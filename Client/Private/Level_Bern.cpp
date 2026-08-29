#include "imgui.h"

#include "Level_Bern.h"

#include "Camera_Free.h"
#include "Character.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "PlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "Trigger_Box.h"
#include "ValtanCinematicCameraController.h"
#include "WorldGameplayDocument.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	constexpr const wchar_t* BERN_CASTLE_BGM_ASSET_ID =
		L"Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav";

	/* Bern entrance cinematic: an authoring-owned single camera cue that plays
	   once on entry. The validation mirrors the pattern-free death-cue rules of
	   the Valtan cinematic camera document so both consume the same sampler. */
	constexpr uint64_t BERN_ENTRANCE_CINEMATIC_OWNER_ID = 0x4245524E43494E45ull;
	constexpr const char_t* BERN_ENTRANCE_CAMERA_SCHEMA =
		"lostark.level-entrance-camera";
	constexpr uint32_t BERN_ENTRANCE_CAMERA_FORMAT_VERSION = 1u;
	constexpr uint32_t BERN_ENTRANCE_MAX_DURATION_MS = 60000u;
	constexpr size_t BERN_ENTRANCE_MAX_KEYFRAME_COUNT = 64u;
	constexpr f32_t BERN_ENTRANCE_MAX_SHAKE_AMPLITUDE = 2.f;
	constexpr uint32_t BERN_ENTRANCE_MAX_SHAKE_DURATION_MS = 1000u;
	constexpr f32_t BERN_ENTRANCE_MAX_WORLD_COORDINATE = 100000.f;

	bool_t Is_EntranceStableId(const std::string& value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const char_t character)
			{
				return (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					'_' == character || '-' == character || '.' == character;
			});
	}

	bool_t Is_ExactEntranceObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Read_EntranceString(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_EntranceUnsigned(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number() ||
			value->Was_FloatingPointToken())
		{
			return false;
		}
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) ||
			std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Read_EntranceFloat3(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		float3_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		f32_t components[3]{};
		for (size_t index = 0u; index < 3u; ++index)
		{
			const DATA_JSON_VALUE& component = value->Get_Array()[index];
			if (!component.Is_Number() ||
				!std::isfinite(component.Get_Number()) ||
				std::abs(component.Get_Number()) >
					BERN_ENTRANCE_MAX_WORLD_COORDINATE)
			{
				return false;
			}
			components[index] = static_cast<f32_t>(component.Get_Number());
		}
		outValue = float3_t(components[0], components[1], components[2]);
		return true;
	}

	bool_t Parse_BernEntranceCamera(
		const std::filesystem::path& path,
		VALTAN_CINEMATIC_CAMERA_CUE& outCue,
		std::string& outStatus)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input.is_open())
		{
			outStatus = "entrance camera document is unreadable";
			return false;
		}
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
		{
			outStatus = "entrance camera document read failed";
			return false;
		}

		DATA_JSON_VALUE root;
		std::string parseError;
		DATA_JSON_PARSE_LIMITS limits{};
		limits.iMaximumBytes = 256u * 1024u;
		limits.iMaximumDepth = 16u;
		limits.iMaximumValues = 4096u;
		if (!CDataJson::Parse(buffer.str(), root, parseError, limits))
		{
			outStatus = "entrance camera parse failed: " + parseError;
			return false;
		}
		if (!Is_ExactEntranceObject(root,
			{ "schema", "formatVersion", "levelId", "provenance", "cue" }))
		{
			outStatus = "entrance camera root has unexpected properties";
			return false;
		}
		std::string schema;
		std::string levelId;
		std::string provenance;
		uint32_t formatVersion = 0u;
		if (!Read_EntranceString(root, "schema", schema) ||
			BERN_ENTRANCE_CAMERA_SCHEMA != schema ||
			!Read_EntranceUnsigned(root, "formatVersion",
				BERN_ENTRANCE_CAMERA_FORMAT_VERSION, formatVersion) ||
			BERN_ENTRANCE_CAMERA_FORMAT_VERSION != formatVersion ||
			!Read_EntranceString(root, "levelId", levelId) ||
			"BERN" != levelId ||
			!Read_EntranceString(root, "provenance", provenance) ||
			"PROJECT_AUTHORED" != provenance)
		{
			outStatus = "entrance camera header is invalid";
			return false;
		}

		const DATA_JSON_VALUE* cueValue = root.Find("cue");
		if (nullptr == cueValue || !Is_ExactEntranceObject(*cueValue,
			{ "cueId", "durationMs", "interpolation", "easing",
				"shakeAmplitude", "shakeDurationMs", "keyframes" }))
		{
			outStatus = "entrance camera cue has unexpected properties";
			return false;
		}
		VALTAN_CINEMATIC_CAMERA_CUE cue;
		std::string interpolation;
		std::string easing;
		if (!Read_EntranceString(*cueValue, "cueId", cue.strCueId) ||
			!Is_EntranceStableId(cue.strCueId) ||
			!Read_EntranceUnsigned(*cueValue, "durationMs",
				BERN_ENTRANCE_MAX_DURATION_MS, cue.iDurationMs) ||
			0u == cue.iDurationMs ||
			!Read_EntranceString(*cueValue, "interpolation", interpolation) ||
			!Read_EntranceString(*cueValue, "easing", easing))
		{
			outStatus = "entrance camera cue identity is invalid";
			return false;
		}
		if ("LINEAR" == interpolation)
			cue.eInterpolation = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
		else if ("CATMULL_ROM" == interpolation)
			cue.eInterpolation =
				VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		else
		{
			outStatus = "entrance camera interpolation is unsupported";
			return false;
		}
		if ("LINEAR" == easing)
			cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		else if ("SMOOTHSTEP" == easing)
			cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		else if ("HOLD" == easing)
			cue.eEasing = VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
		else
		{
			outStatus = "entrance camera easing is unsupported";
			return false;
		}
		const DATA_JSON_VALUE* amplitude = cueValue->Find("shakeAmplitude");
		if (nullptr == amplitude || !amplitude->Is_Number() ||
			!std::isfinite(amplitude->Get_Number()) ||
			amplitude->Get_Number() < 0.0 ||
			amplitude->Get_Number() >
				static_cast<double>(BERN_ENTRANCE_MAX_SHAKE_AMPLITUDE) ||
			!Read_EntranceUnsigned(*cueValue, "shakeDurationMs",
				BERN_ENTRANCE_MAX_SHAKE_DURATION_MS, cue.iShakeDurationMs))
		{
			outStatus = "entrance camera shake is invalid";
			return false;
		}
		cue.fShakeAmplitude = static_cast<f32_t>(amplitude->Get_Number());
		if ((cue.fShakeAmplitude > 0.f) != (0u != cue.iShakeDurationMs) ||
			cue.iShakeDurationMs > cue.iDurationMs)
		{
			outStatus = "entrance camera shake pair is invalid";
			return false;
		}

		const DATA_JSON_VALUE* keyframes = cueValue->Find("keyframes");
		if (nullptr == keyframes || !keyframes->Is_Array() ||
			keyframes->Get_Array().size() < 2u ||
			keyframes->Get_Array().size() > BERN_ENTRANCE_MAX_KEYFRAME_COUNT)
		{
			outStatus = "entrance camera keyframe array is invalid";
			return false;
		}
		std::unordered_set<std::string> sceneIds;
		uint32_t previousTime = 0u;
		for (size_t index = 0u; index < keyframes->Get_Array().size(); ++index)
		{
			const DATA_JSON_VALUE& keyframeValue =
				keyframes->Get_Array()[index];
			if (!Is_ExactEntranceObject(keyframeValue,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "entrance camera keyframe has unexpected properties";
				return false;
			}
			VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			const DATA_JSON_VALUE* fov = keyframeValue.Find("fovYDegrees");
			if (!Read_EntranceString(keyframeValue, "sceneId",
					keyframe.strSceneId) ||
				!Is_EntranceStableId(keyframe.strSceneId) ||
				!sceneIds.insert(keyframe.strSceneId).second ||
				!Read_EntranceUnsigned(keyframeValue, "timeMs",
					cue.iDurationMs, keyframe.iTimeMs) ||
				!Read_EntranceFloat3(keyframeValue, "eye", keyframe.vEye) ||
				!Read_EntranceFloat3(keyframeValue, "lookAt",
					keyframe.vLookAt) ||
				nullptr == fov || !fov->Is_Number() ||
				!std::isfinite(fov->Get_Number()) ||
				fov->Get_Number() < 10.0 || fov->Get_Number() > 120.0 ||
				(0u == index && 0u != keyframe.iTimeMs) ||
				(index > 0u && keyframe.iTimeMs <= previousTime))
			{
				outStatus = "entrance camera keyframe is invalid";
				return false;
			}
			keyframe.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
			const vector_t eye = XMLoadFloat3(&keyframe.vEye);
			const vector_t lookAt = XMLoadFloat3(&keyframe.vLookAt);
			if (XMVectorGetX(XMVector3LengthSq(lookAt - eye)) <= 0.000001f)
			{
				outStatus = "entrance camera eye and lookAt must differ";
				return false;
			}
			previousTime = keyframe.iTimeMs;
			cue.Keyframes.push_back(keyframe);
		}
		if (previousTime != cue.iDurationMs)
		{
			outStatus =
				"entrance camera final keyframe must match cue duration";
			return false;
		}
		outCue = std::move(cue);
		outStatus = "entrance camera cue is ready";
		return true;
	}
}

CLevel_Bern* CLevel_Bern::s_pActiveInstance = nullptr;

CLevel_Bern::CLevel_Bern(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
	s_pActiveInstance = this;
}

CLevel_Bern::~CLevel_Bern()
{
	End_EntranceCinematic();

	if (m_bBernBgmStarted)
		CGameInstance::Get().Stop_Music();

	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
}

HRESULT CLevel_Bern::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::BERN);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::BERN),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_Bern] " +
			m_MapRuntime.Get_Status() +
			"\n").c_str());
		return E_FAIL;
	}

	if (FAILED(Ready_Layer_Camera(
			TEXT("Layer_Camera"), pEntry->pMapAreaId)))
	{
		return E_FAIL;
	}

	(void)Ready_EntranceCinematic();

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;

	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::BERN);

	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::BERN);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;

	replicationDesc.strPlayerLayerTag =
		TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag =
		TEXT("Layer_WorldEntity");

	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (!m_PlayerController.Initialize_TargetingPreview(ETOUI(LEVEL::BERN)))
		return E_FAIL;
	if (!m_PlayerController.Initialize_ClickMoveEffect(ETOUI(LEVEL::BERN)))
		return E_FAIL;

	if (!Ready_ValtanEntryNpcs(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Bern] Valtan-entry guide NPC positions unavailable; "
			"right-click entry interaction is disabled.\n");
	}
	if (!Ready_ItemUpgradeNpc(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Bern] Item Upgrade NPC (npc.bern.schmidt) position "
			"unavailable; right-click interaction is disabled.\n");
	}
	m_pValtanEntryView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/Bern/BernValtanEntry_Layout.json");

	/* First-ever CGameInstance::Draw_Text call for Font_YoonGasiIIM in this
	level appeared to render nothing the first time the Valtan-entry popup
	opened (confirmed determinate rect/measure that frame -- see
	Render_ValtanEntryModalText), then worked normally every time after.
	Off-screen warm-up draw so whatever GPU-side lazy init that first call
	does happens here instead of on the popup's actual first appearance.
	A single space (the original warm-up string here) turned out not to
	actually fix this: DirectXTK's SpriteFont::DrawString skips the
	SpriteBatch::Draw() call entirely for a whitespace glyph whose sprite
	sheet subrect is 1x1 or smaller (true for the space glyph in most
	fonts, this one included), so Begin()/End() ran with zero queued
	sprites and never touched whatever the real first Draw() call lazily
	sets up. Warming up with the exact real string ("레이드 입장") instead
	guarantees at least one non-degenerate glyph quad is actually queued
	and drawn. */
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), L"\xB808\xC774\xB4DC \xC785\xC7A5",
		float2_t(-1000.f, -1000.f), Colors::White, 0.f,
		float2_t(0.5f, 0.5f), 1.f);

	m_PartyInteraction.Initialize(m_pDevice, m_pContext);

#ifdef _DEBUG
	if (!Ready_DebugLevelChangeTriggers(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Bern] Debug changeLevel Trigger Box presentation failed.\n");
	}
#endif

	const std::filesystem::path musicPath =
		CRuntimeAssetRoot::Resolve(BERN_CASTLE_BGM_ASSET_ID);
	if (!musicPath.empty() && std::filesystem::is_regular_file(musicPath) &&
		SUCCEEDED(CGameInstance::Get().Play_Music(
			musicPath.wstring(), 1.f, true)))
	{
		m_bBernBgmStarted = true;
	}
	else
	{
#ifdef _DEBUG
		OutputDebugStringA(
			"[Level_Bern] Bern Castle BGM was isolated because the exact "
			"runtime WAV could not be played.\n");
#endif
	}

	return S_OK;
}

void CLevel_Bern::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(LEVEL::BERN))
	{
		return;
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Bern] Failed to apply replication event.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-bern.network-connection-lost",
			"Bern replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_Bern] Lobby recovery request was rejected; retrying.\n");
	}

	if (!Bind_CameraToLocalCharacter())
	{
		OutputDebugStringA(
			"[Level_Bern] Failed to bind local character camera.\n");
	}

	Update_EntranceCinematic(fTimeDelta);

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();

	m_PlayerController.Set_LocalCharacter(
		localCharacter);

	/* Popup ownership is added before either world picking or gameplay input.
	Never clear another consumer's block (for example MapTool's LMB owner). */
	if (m_isValtanEntryModalOpen)
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	if (m_PartyInteraction.Update(
		m_Replication, m_pPlayerCommandSink, m_NameplatePlayers,
		!m_isValtanEntryModalOpen &&
			nullptr != m_pCamera && m_pCamera->Is_FollowEnabled()))
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	Update_ValtanEntryInteraction();
	Advance_ValtanEntryWalk();
	Update_ItemUpgradeNpcInteraction();
	Advance_ItemUpgradeNpcWalk();
	if (m_isValtanEntryModalOpen)
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}

	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled());

}

bool_t CLevel_Bern::Ready_EntranceCinematic()
{
	const std::filesystem::path path = CProjectDataRoot::Resolve(
		L"Encounters/Bern/BernEntranceCamera.json");
	std::error_code fileError;
	if (path.empty() || !std::filesystem::is_regular_file(path, fileError))
		return false;
	std::string status;
	if (!Parse_BernEntranceCamera(path, m_EntranceCameraCue, status))
	{
		OutputDebugStringA((
			"[Level_Bern] Entrance cinematic was isolated: " +
			status + "\n").c_str());
		m_EntranceCameraCue = VALTAN_CINEMATIC_CAMERA_CUE{};
		return false;
	}
	m_hasEntranceCameraCue = true;
	return true;
}

void CLevel_Bern::Update_EntranceCinematic(const f32_t fTimeDelta)
{
	if (!m_hasEntranceCameraCue || m_bEntranceCinematicDone ||
		nullptr == m_pCamera)
	{
		return;
	}
	if (!m_bEntranceCinematicApplied)
	{
		m_bEntranceRestoreFollowRequested = m_pCamera->Is_FollowRequested();
		m_pEntranceRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
		if (!m_pCamera->Begin_PresentationOverride(
			BERN_ENTRANCE_CINEMATIC_OWNER_ID,
			CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
		{
			m_pCamera->Set_FollowTarget(m_pEntranceRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(m_bEntranceRestoreFollowRequested);
			m_bEntranceCinematicDone = true;
			return;
		}
		m_bEntranceCinematicApplied = true;
		m_fEntranceCinematicSeconds = 0.f;
	}
	else if (nullptr != m_pCamera->Get_FollowTarget())
	{
		/* Replication rebinds the follow camera once the local character
		   spawns; keep the newest target for the restore and strip it while
		   the entrance override owns the view. */
		m_pEntranceRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
	}
	if (std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
		m_fEntranceCinematicSeconds += (std::min)(fTimeDelta, 0.1f);
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!CValtanCinematicCameraController::Sample_Cue(
		m_EntranceCameraCue, m_fEntranceCinematicSeconds, pose) ||
		!m_pCamera->Apply_PresentationPose(
			BERN_ENTRANCE_CINEMATIC_OWNER_ID,
			pose.vEye, pose.vLookAt, pose.fFovYDegrees))
	{
		End_EntranceCinematic();
		return;
	}
	if (m_fEntranceCinematicSeconds >=
		static_cast<f32_t>(m_EntranceCameraCue.iDurationMs) * 0.001f)
	{
		End_EntranceCinematic();
	}
}

void CLevel_Bern::End_EntranceCinematic()
{
	if (m_bEntranceCinematicApplied && nullptr != m_pCamera)
	{
		(void)m_pCamera->End_PresentationOverride(
			BERN_ENTRANCE_CINEMATIC_OWNER_ID);
		m_pCamera->Set_FollowTarget(m_pEntranceRestoreTarget.lock());
		m_pCamera->Set_FollowEnabled(m_bEntranceRestoreFollowRequested);
	}
	m_pEntranceRestoreTarget.reset();
	m_bEntranceRestoreFollowRequested = false;
	m_bEntranceCinematicApplied = false;
	m_bEntranceCinematicDone = true;
}

HRESULT CLevel_Bern::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_PlayerNameplateView.Render(m_NameplatePlayers);
	m_ChatBubbleView.Render(m_Replication, m_NameplatePlayers);
	m_PartyInteraction.Render(m_pPlayerCommandSink);

	/* The ImGui popup draws the panel and button art; the LOA font pass that
	labels them has to follow it inside this same Render call. */
	Render_ValtanEntryModal();
	Render_ValtanEntryModalText();

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(
		TEXT("Bern Castle Network Player Test"));
#endif

	return S_OK;
}

HRESULT CLevel_Bern::Ready_Layer_Camera(
	const wstring_t& strLayerTag,
	const std::string& areaId)
{
	float3_t minimum{};
	float3_t maximum{};
	float3_t focus(0.f, 0.f, 0.f);

	f32_t span = 80.f;

	if (m_MapRuntime.Try_Get_PlacementBounds(
		minimum,
		maximum))
	{
		focus = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);

		span = (std::max)(
			maximum.x - minimum.x,
			maximum.z - minimum.z);

		span = (std::clamp)(
			span,
			40.f,
			5000.f);
	}

	const f32_t distance =
		(std::max)(40.f, span * 0.7f);
	float3_t initialEye(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	float3_t initialAt = focus;
	const auto applyPlayerFraming = [&initialEye, &initialAt](
		const float3_t& position)
	{
		initialEye = float3_t(
			position.x + 0.4f,
			position.y + 7.5f,
			position.z + 4.5f);
		initialAt = float3_t(
			position.x,
			position.y + 1.2f,
			position.z);
	};
	LostArk::Shared::S2C_PLAYER_SPAWNED approvedSpawn{};
	if (CNetworkManager::Get().Try_Get_LocalSpawn(approvedSpawn))
	{
		applyPlayerFraming(float3_t(
			approvedSpawn.fPositionX,
			approvedSpawn.fPositionY,
			approvedSpawn.fPositionZ));
	}
	else
	{
		/* ENTER_ACCEPTED can activate Bern before the later player spawn frame is
		consumed. Frame that short window from the same authored spawn document
		the Server publishes instead of from the 50,000-placement map bounds. */
		const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
			std::filesystem::path("Worlds") / areaId / "Gameplay.world.json");
		CWorldGameplayDocument document;
		std::string status;
		if (!documentPath.empty() && document.Load(documentPath, areaId, status))
		{
			const auto& placements = document.Get_Placements();
			const auto authoredSpawn = std::find_if(
				placements.begin(), placements.end(),
				[](const WORLD_GAMEPLAY_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind;
				});
			if (placements.end() != authoredSpawn)
				applyPlayerFraming(authoredSpawn->position);
		}
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};

	/*
	 * Server spawn이 이미 도착했으면 그 위치를 사용하고, 아직이면 authored
	 * player spawn을 사용한다. map bounds는 문서도 없을 때의 마지막 fallback이다.
	 */
	cameraDesc.vEye = initialEye;

	cameraDesc.vAt = initialAt;

	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar =
		(std::max)(2000.f, span * 8.f);

	cameraDesc.fSpeedPerSec =
		(std::max)(20.f, span * 0.08f);

	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	/*
	 * Player Spawn 이후 사용할 Follow Camera 설정이다.
	 * Initialize 시점에는 Player가 없으므로 비활성화한다.
	 */
	cameraDesc.pFollowTarget = nullptr;

	cameraDesc.vPositionOffset =
		float3_t(0.4f, 7.5f, 4.5f);

	cameraDesc.vLookOffset =
		float3_t(0.f, 1.2f, 0.f);

	cameraDesc.fFollowResponse = 0.f;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::BERN),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::BERN),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera =
		dynamic_pointer_cast<CCamera_Free>(
			gameObject);

	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::BERN),
			strLayerTag,
			gameObject);

		return E_FAIL;
	}

	return S_OK;
}

bool_t CLevel_Bern::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();

	/*
	 * 아직 Local Spawn Event가 도착하지 않았거나
	 * Local Character가 Despawn된 상태다.
	 */
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();

		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);

		return true;
	}

	/*
	 * 이미 같은 Character에 연결되어 있으면 매 프레임
	 * Camera Target을 다시 설정하지 않는다.
	 */
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();

	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;

	m_pCamera->Set_PositionOffset(
		float3_t(0.4f, 7.5f, 4.5f));

	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);

	return true;
}

bool_t CLevel_Bern::Ready_ValtanEntryNpcs(const std::string& areaId)
{
	// The two guide NPCs that used to sit beside an automatic changeLevel
	// triggerBox (now disabled -- see Data/Worlds/LV_BER_BERNCASTLE/
	// Gameplay.world.json's trigger.bern.to-valtan / valtan placements). Server
	// Handle_ConfirmNpcEntry carries the same two IDs for its own authority
	// check, so both sides name the same real placements instead of one
	// inferring the pairing from geometry.
	static constexpr const char* GUIDE_NPC_PLACEMENT_IDS[] =
	{
		"npc.bern.beda.guide",
		"npc.bern.aylara",
	};

	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") / areaId / "Gameplay.world.json");
	std::error_code pathError;
	if (documentPath.empty() ||
		!std::filesystem::is_regular_file(documentPath, pathError) || pathError)
	{
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
		return false;

	std::vector<VALTAN_ENTRY_NPC> staged;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind)
			continue;
		const bool_t isGuide = std::any_of(
			std::begin(GUIDE_NPC_PLACEMENT_IDS),
			std::end(GUIDE_NPC_PLACEMENT_IDS),
			[&placement](const char* pId)
			{
				return placement.placementId == pId;
			});
		if (!isGuide)
			continue;
		staged.push_back({ placement.placementId, placement.position });
	}

	m_ValtanEntryNpcs = std::move(staged);
	return !m_ValtanEntryNpcs.empty();
}

void CLevel_Bern::Update_ValtanEntryInteraction()
{
	const bool_t isRightMouseDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDownForNpcInteract;
	m_wasRightMouseDownForNpcInteract = isRightMouseDown;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (m_isValtanEntryModalOpen || !isRightMousePressed ||
		0 == (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80) ||
		m_ValtanEntryNpcs.empty() || nullptr == localCharacter ||
		nullptr == m_pCamera || !m_pCamera->Is_FollowEnabled())
	{
		return;
	}

	/* World-ray-vs-sphere pick against each NPC's real position, the same
	technique CPartyInteractionView's right-click-a-player pick uses -- the
	NPC has to be clickable from anywhere on screen, not just while already
	standing next to it. NPC_CLICK_RADIUS is a generous clickable capsule;
	INTERACTION_RADIUS (Advance_ValtanEntryWalk, same value
	Handle_ConfirmNpcEntry re-validates server-side) is the much smaller
	distance the character actually has to walk into before the window
	opens. */
	vector_t rayOrigin{}, rayDirection{};
	if (!CPlayerController::Try_PickWorldRay(rayOrigin, rayDirection))
		return;
	rayDirection = XMVector3Normalize(rayDirection);

	constexpr f32_t NPC_CLICK_RADIUS = 1.5f;
	f32_t fBestRayParameter = FLT_MAX;
	const VALTAN_ENTRY_NPC* pHit = nullptr;
	for (const VALTAN_ENTRY_NPC& npc : m_ValtanEntryNpcs)
	{
		const vector_t vNpcPos = XMLoadFloat3(&npc.vPosition);
		const f32_t fRayParameter = XMVectorGetX(XMVector3Dot(
			XMVectorSubtract(vNpcPos, rayOrigin), rayDirection));
		if (fRayParameter < 0.f)
			continue;

		const vector_t vClosestPoint = XMVectorAdd(
			rayOrigin, XMVectorScale(rayDirection, fRayParameter));
		const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
			XMVectorSubtract(vNpcPos, vClosestPoint)));
		if (fDistanceSq > NPC_CLICK_RADIUS * NPC_CLICK_RADIUS)
			continue;

		if (fRayParameter < fBestRayParameter)
		{
			fBestRayParameter = fRayParameter;
			pHit = &npc;
		}
	}
	if (nullptr == pHit)
		return;

	m_PlayerController.Suppress_MoveClickThisFrame();
	CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	m_strValtanEntryNpcPlacementId = pHit->strPlacementId;
	m_isWalkingToValtanEntryNpc = true;

	/* Stop just inside interaction range, on the side the character is
	already standing, instead of walking exactly onto the NPC's own point. */
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr != transform)
	{
		const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
		vector_t vTowardCharacter = XMVectorSubtract(vCharacterPos,
			XMLoadFloat3(&pHit->vPosition));
		vTowardCharacter = XMVectorSetY(vTowardCharacter, 0.f);
		constexpr f32_t INTERACTION_RADIUS = 3.f;
		float3_t goal{};
		if (XMVectorGetX(XMVector3LengthSq(vTowardCharacter)) < 0.01f)
		{
			goal = pHit->vPosition;
		}
		else
		{
			vTowardCharacter = XMVector3Normalize(vTowardCharacter);
			XMStoreFloat3(&goal, XMVectorAdd(
				XMLoadFloat3(&pHit->vPosition),
				XMVectorScale(vTowardCharacter, INTERACTION_RADIUS * 0.7f)));
			goal.y = pHit->vPosition.y;
		}
		m_PlayerController.Request_MoveToPoint(goal);
	}
}

void CLevel_Bern::Advance_ValtanEntryWalk()
{
	if (!m_isWalkingToValtanEntryNpc || m_isValtanEntryModalOpen)
		return;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	const auto npcIt = std::find_if(
		m_ValtanEntryNpcs.begin(), m_ValtanEntryNpcs.end(),
		[this](const VALTAN_ENTRY_NPC& npc)
		{
			return npc.strPlacementId == m_strValtanEntryNpcPlacementId;
		});
	if (nullptr == localCharacter || m_ValtanEntryNpcs.end() == npcIt)
	{
		m_isWalkingToValtanEntryNpc = false;
		return;
	}

	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return;

	// Same footprint Handle_ConfirmNpcEntry re-validates server-side.
	constexpr f32_t INTERACTION_RADIUS = 3.f;
	const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
	const vector_t vDelta = XMVectorSubtract(
		vCharacterPos, XMLoadFloat3(&npcIt->vPosition));
	const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
		XMVectorSetY(vDelta, 0.f)));
	if (fDistanceSq > INTERACTION_RADIUS * INTERACTION_RADIUS)
		return;

	m_isWalkingToValtanEntryNpc = false;
	m_isValtanEntryModalOpen = true;
	m_hasValtanEntryModalJustOpened = true;
#ifdef _DEBUG
	OutputDebugStringA("[Level_Bern][ValtanEntryText] modal opened this frame\n");
#endif
}

bool_t CLevel_Bern::Ready_ItemUpgradeNpc(const std::string& areaId)
{
	// Real placement confirmed in Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json:
	// npc.bern.schmidt, archetype NPC_SCHMIDT, (143.069, 46.8328629, -104.165001).
	constexpr const char* ITEM_UPGRADE_NPC_PLACEMENT_ID = "npc.bern.schmidt";

	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") / areaId / "Gameplay.world.json");
	std::error_code pathError;
	if (documentPath.empty() ||
		!std::filesystem::is_regular_file(documentPath, pathError) || pathError)
	{
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
		return false;

	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind ||
			placement.placementId != ITEM_UPGRADE_NPC_PLACEMENT_ID)
		{
			continue;
		}
		m_vItemUpgradeNpcPosition = placement.position;
		m_hasItemUpgradeNpc = true;
		return true;
	}
	return false;
}

void CLevel_Bern::Update_ItemUpgradeNpcInteraction()
{
	const bool_t isRightMouseDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDownForItemUpgradeNpcInteract;
	m_wasRightMouseDownForItemUpgradeNpcInteract = isRightMouseDown;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (!m_hasItemUpgradeNpc || !isRightMousePressed ||
		0 == (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80) ||
		nullptr == localCharacter ||
		nullptr == m_pCamera || !m_pCamera->Is_FollowEnabled())
	{
		return;
	}

	vector_t rayOrigin{}, rayDirection{};
	if (!CPlayerController::Try_PickWorldRay(rayOrigin, rayDirection))
		return;
	rayDirection = XMVector3Normalize(rayDirection);

	constexpr f32_t NPC_CLICK_RADIUS = 1.5f;
	const vector_t vNpcPos = XMLoadFloat3(&m_vItemUpgradeNpcPosition);
	const f32_t fRayParameter = XMVectorGetX(XMVector3Dot(
		XMVectorSubtract(vNpcPos, rayOrigin), rayDirection));
	if (fRayParameter < 0.f)
		return;
	const vector_t vClosestPoint = XMVectorAdd(
		rayOrigin, XMVectorScale(rayDirection, fRayParameter));
	const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
		XMVectorSubtract(vNpcPos, vClosestPoint)));
	if (fDistanceSq > NPC_CLICK_RADIUS * NPC_CLICK_RADIUS)
		return;

	m_PlayerController.Suppress_MoveClickThisFrame();
	CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	m_isWalkingToItemUpgradeNpc = true;

	/* Stop just inside interaction range, on the side the character is already
	standing, instead of walking exactly onto the NPC's own point -- same
	approach as Update_ValtanEntryInteraction. */
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr != transform)
	{
		const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
		vector_t vTowardCharacter = XMVectorSubtract(vCharacterPos, vNpcPos);
		vTowardCharacter = XMVectorSetY(vTowardCharacter, 0.f);
		constexpr f32_t INTERACTION_RADIUS = 3.f;
		float3_t goal{};
		if (XMVectorGetX(XMVector3LengthSq(vTowardCharacter)) < 0.01f)
		{
			goal = m_vItemUpgradeNpcPosition;
		}
		else
		{
			vTowardCharacter = XMVector3Normalize(vTowardCharacter);
			XMStoreFloat3(&goal, XMVectorAdd(
				vNpcPos, XMVectorScale(vTowardCharacter, INTERACTION_RADIUS * 0.7f)));
			goal.y = m_vItemUpgradeNpcPosition.y;
		}
		m_PlayerController.Request_MoveToPoint(goal);
	}
}

void CLevel_Bern::Advance_ItemUpgradeNpcWalk()
{
	if (!m_isWalkingToItemUpgradeNpc)
		return;

	const shared_ptr<CCharacter> localCharacter = m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_isWalkingToItemUpgradeNpc = false;
		return;
	}
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return;

	// Same footprint Update_ItemUpgradeNpcInteraction stops the character at.
	constexpr f32_t INTERACTION_RADIUS = 3.f;
	const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
	const vector_t vDelta = XMVectorSubtract(
		vCharacterPos, XMLoadFloat3(&m_vItemUpgradeNpcPosition));
	const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
		XMVectorSetY(vDelta, 0.f)));
	if (fDistanceSq > INTERACTION_RADIUS * INTERACTION_RADIUS)
		return;

	m_isWalkingToItemUpgradeNpc = false;
	if (CMainApp* pMainApp = CMainApp::Get_Active())
		pMainApp->Open_ItemUpgradeWindow();
}

void CLevel_Bern::Render_ValtanEntryModal()
{
	if (nullptr == m_pValtanEntryView)
		return;

	if (m_hasValtanEntryModalJustOpened)
	{
		ImGui::OpenPopup("ValtanEntryConfirm");
		m_hasValtanEntryModalJustOpened = false;
	}

	if (!m_isValtanEntryModalOpen)
		return;

	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(pViewport->WorkPos);
	ImGui::SetNextWindowSize(pViewport->WorkSize);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoSavedSettings;

	/* BeginPopupModal draws its own full-viewport dim rect before returning,
	   independent of ImGuiWindowFlags_NoBackground (which only covers the
	   popup window itself) -- StyleColorsDark's default ModalWindowDimBg is a
	   light translucent grey, which reads as a wash of white over the game
	   behind it. Suppressed since this popup only wants its own panel art
	   visible, not a dimmed backdrop -- same fix as CPartyInteractionView's
	   two modals. */
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	const bool_t isModalOpen =
		ImGui::BeginPopupModal("ValtanEntryConfirm", nullptr, flags);
	ImGui::PopStyleColor();
	if (!isModalOpen)
	{
		m_isValtanEntryModalOpen = false;
		return;
	}

	const auto Fn_ToScreen = [pViewport](f32_t fX, f32_t fY)
	{
		const f32_t fScaleX = pViewport->WorkSize.x / 1280.f;
		const f32_t fScaleY = pViewport->WorkSize.y / 720.f;
		return ImVec2(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
	};
	const auto Fn_HitTest = [](const ImVec2& corner0, const ImVec2& corner1)
	{
		const ImVec2 mouse = ImGui::GetMousePos();
		return mouse.x >= corner0.x && mouse.x < corner1.x &&
			mouse.y >= corner0.y && mouse.y < corner1.y;
	};

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelW = 0.f, fPanelH = 0.f;
	if (m_pValtanEntryView->Get_SlotRect(
		"ValtanEntry_Panel", fPanelX, fPanelY, fPanelW, fPanelH))
	{
		ID3D11ShaderResourceView* pPanel = m_pValtanEntryView->Load_Texture(
			"UI/ClassSelect/Common/CreateCharacterModalPanel.png");
		if (nullptr != pPanel)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pPanel),
				Fn_ToScreen(fPanelX, fPanelY),
				Fn_ToScreen(fPanelX + fPanelW, fPanelY + fPanelH));
		}
	}

	/* ValtanEntry_TitleTextBox/DescTextBox are position-only markers for
	   Render_ValtanEntryModalText() -- no background image, text only. */

	struct MODAL_BUTTON
	{
		const char* pSlotId;
		bool_t isConfirm;
	};
	static constexpr MODAL_BUTTON BUTTONS[2] =
	{
		{ "ValtanEntry_ConfirmButton", true },
		{ "ValtanEntry_CancelButton", false },
	};

	bool_t confirmClicked = false;
	bool_t cancelClicked = false;
	for (const MODAL_BUTTON& button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pValtanEntryView->Get_SlotRect(button.pSlotId, fX, fY, fW, fH))
			continue;
		const ImVec2 corner0 = Fn_ToScreen(fX, fY);
		const ImVec2 corner1 = Fn_ToScreen(fX + fW, fY + fH);
		const bool_t isHovered = Fn_HitTest(corner0, corner1);
		ID3D11ShaderResourceView* pTexture = m_pValtanEntryView->Load_Texture(
			isHovered ?
				"UI/ClassSelect/Common/NormalButtonHover.png" :
				"UI/ClassSelect/Common/NormalButton.png");
		if (nullptr != pTexture)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pTexture), corner0, corner1);
		}
		if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			CMainApp::Play_UIButtonClickSound();
			if (button.isConfirm)
				confirmClicked = true;
			else
				cancelClicked = true;
		}
	}

	/* Drawn after the buttons (not before) so the button art doesn't paint
	   over these and hide them. */
	struct MODAL_ICON_SLOT
	{
		const char* pSlotId;
		const char* pTexturePath;
	};
	static constexpr MODAL_ICON_SLOT ICON_SLOTS[2] =
	{
		{ "ValtanEntry_AcceptIcon", "UI/Bern/Accept.png" },
		{ "ValtanEntry_DeclineIcon", "UI/Bern/Decline.png" },
	};
	for (const MODAL_ICON_SLOT& icon : ICON_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pValtanEntryView->Get_SlotRect(icon.pSlotId, fX, fY, fW, fH))
			continue;
		ID3D11ShaderResourceView* pTexture =
			m_pValtanEntryView->Load_Texture(icon.pTexturePath);
		if (nullptr != pTexture)
		{
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(pTexture),
				Fn_ToScreen(fX, fY), Fn_ToScreen(fX + fW, fY + fH));
		}
	}

	if (cancelClicked)
	{
		m_isValtanEntryModalOpen = false;
		ImGui::CloseCurrentPopup();
	}
	else if (confirmClicked)
	{
		if (nullptr != m_pPlayerCommandSink)
		{
			m_pPlayerCommandSink->Request_ConfirmNpcEntry(
				m_iNextNpcEntryConfirmSequence++,
				m_strValtanEntryNpcPlacementId);
		}
		m_isValtanEntryModalOpen = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void CLevel_Bern::Render_ValtanEntryModalText()
{
	if (!m_isValtanEntryModalOpen || nullptr == m_pValtanEntryView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vViewportSize.x / 1280.f;
	const float textScaleY = vViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto Fn_DrawCentered = [&](f32_t fCenterX, f32_t fCenterY,
		const wchar_t* pLabel, f32_t fTargetHeight, const fvector_t& vColor)
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ?
			(fTargetHeight / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleW = 0.f, fTitleH = 0.f;
	const bool_t bTitleRectFound = m_pValtanEntryView->Get_SlotRect(
		"ValtanEntry_TitleTextBox", fTitleX, fTitleY, fTitleW, fTitleH);
#ifdef _DEBUG
	{
		const float2_t vMeasuredDebug = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), L"\xB808\xC774\xB4DC \xC785\xC7A5");
		OutputDebugStringA((
			"[Level_Bern][ValtanEntryText] rectFound=" +
			std::to_string(bTitleRectFound) +
			" rect=(" + std::to_string(fTitleX) + "," +
			std::to_string(fTitleY) + "," + std::to_string(fTitleW) + "," +
			std::to_string(fTitleH) + ") measured=(" +
			std::to_string(vMeasuredDebug.x) + "," +
			std::to_string(vMeasuredDebug.y) + ") viewport=(" +
			std::to_string(CGameInstance::Get().Get_ViewportSize().x) + "," +
			std::to_string(CGameInstance::Get().Get_ViewportSize().y) +
			")\n").c_str());
	}
#endif
	if (bTitleRectFound)
	{
		// "레이드 입장"
		Fn_DrawCentered(fTitleX + fTitleW * 0.5f, fTitleY + fTitleH * 0.5f,
			L"\xB808\xC774\xB4DC \xC785\xC7A5",
			24.f, Colors::White);
	}

	f32_t fDescX = 0.f, fDescY = 0.f, fDescW = 0.f, fDescH = 0.f;
	if (m_pValtanEntryView->Get_SlotRect(
		"ValtanEntry_DescTextBox", fDescX, fDescY, fDescW, fDescH))
	{
		// "부활한 마수의 심장으로 이동하시겠습니까?"
		Fn_DrawCentered(fDescX + fDescW * 0.5f, fDescY + fDescH * 0.5f,
			L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5\xC73C\xB85C "
			L"\xC774\xB3D9\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?",
			18.f, Colors::White);
	}

	/* Label sits to the right of the icon inside the same button, not centered
	   on the whole button -- Get_SlotRect gives both rects so no hardcoded
	   offset is needed. */
	struct MODAL_BUTTON_LABEL
	{
		const char_t* pButtonSlotId;
		const char_t* pIconSlotId;
		const wchar_t* pLabel;
	};
	const MODAL_BUTTON_LABEL BUTTON_LABELS[] =
	{
		{ "ValtanEntry_ConfirmButton", "ValtanEntry_AcceptIcon", L"\xC218\xB77D" }, // "수락"
		{ "ValtanEntry_CancelButton", "ValtanEntry_DeclineIcon", L"\xAC70\xC808" }, // "거절"
	};
	for (const MODAL_BUTTON_LABEL& Label : BUTTON_LABELS)
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonW = 0.f, fButtonH = 0.f;
		f32_t fIconX = 0.f, fIconY = 0.f, fIconW = 0.f, fIconH = 0.f;
		if (!m_pValtanEntryView->Get_SlotRect(
				Label.pButtonSlotId, fButtonX, fButtonY, fButtonW, fButtonH) ||
			!m_pValtanEntryView->Get_SlotRect(
				Label.pIconSlotId, fIconX, fIconY, fIconW, fIconH))
		{
			continue;
		}
		const f32_t fIconRight = fIconX + fIconW;
		const f32_t fButtonRight = fButtonX + fButtonW;
		Fn_DrawCentered(
			(fIconRight + fButtonRight) * 0.5f, fButtonY + fButtonH * 0.5f,
			Label.pLabel, fButtonH * 0.48f, Colors::White);
	}
}

#ifdef _DEBUG
bool_t CLevel_Bern::Ready_DebugLevelChangeTriggers(
	const std::string& areaId)
{
	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") /
		areaId /
		"Gameplay.world.json");
	std::error_code pathError;
	if (documentPath.empty() ||
		!std::filesystem::is_regular_file(documentPath, pathError) ||
		pathError)
	{
		OutputDebugStringA((
			"[Level_Bern] Debug gameplay document is unavailable: " +
			documentPath.string() + "\n").c_str());
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
	{
		OutputDebugStringA((
			"[Level_Bern] Debug gameplay document rejected: " +
			status + "\n").c_str());
		return false;
	}

	std::vector<shared_ptr<CTrigger_Box>> staged;
	const auto rollback = [&staged]()
	{
		for (const shared_ptr<CTrigger_Box>& triggerBox : staged)
		{
			if (nullptr == triggerBox)
				continue;
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::BERN),
				TEXT("Layer_DebugWorldGameplay"),
				static_pointer_cast<CGameObject>(triggerBox));
		}
		staged.clear();
	};

	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		const bool_t isLevelChangeTrigger =
			placement.isEnabled &&
			WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind &&
			1u == placement.triggerEvents.size() &&
			WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL ==
				placement.triggerEvents.front().eKind;
		if (!isLevelChangeTrigger)
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = placement.placementId;
		desc.position = placement.position;
		desc.halfExtents = placement.halfExtents;
		desc.yawDegrees = placement.yawDegrees;
		desc.isEnabled = true;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::BERN),
			TEXT("Prototype_GameObject_TriggerBox"),
			ETOUI(LEVEL::BERN),
			TEXT("Layer_DebugWorldGameplay"),
			&desc,
			&gameObject)))
		{
			rollback();
			OutputDebugStringA((
				"[Level_Bern] Debug Trigger Box clone failed: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::BERN),
				TEXT("Layer_DebugWorldGameplay"),
				gameObject);
			rollback();
			OutputDebugStringA((
				"[Level_Bern] Debug Trigger Box type mismatch: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		triggerBox->Set_AuthoringVisible(true);
		staged.push_back(std::move(triggerBox));
	}

	m_DebugLevelChangeTriggers = std::move(staged);
	OutputDebugStringA((
		"[Level_Bern] Debug changeLevel Trigger Boxes ready: " +
		std::to_string(m_DebugLevelChangeTriggers.size()) + "\n").c_str());
	return true;
}
#endif

unique_ptr<CLevel_Bern> CLevel_Bern::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CLevel_Bern>(
			new CLevel_Bern(
				pDevice,
				pContext));

	if (FAILED(instance->Initialize()))
		return nullptr;

	return instance;
}
