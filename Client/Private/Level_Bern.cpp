#include <WinSock2.h>
#include <dinput.h>
#include "imgui.h"
#include "UITextOcclusion.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Level_Bern.h"

#include "ActorCatalog.h"
#include "Camera_Free.h"
#include "CombatHUDViewModel.h"
#include "EffectFailureDiagnostic.h"
#include "Effect_PresentationService.h"
#include "InteractKeyPromptView.h"
#include "Character.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ItemCatalog.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "CharacterRoster.h"
#include "CharacterSelectionState.h"
#include "MainApp.h"
#include "MapLightPresentationRuntime.h"
#include "MapEffectPresentationRuntime.h"
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
	// Main-thread presentation state outlives Level instances, not the Client process.
	bool_t s_hasPresentedBernEntranceThisSession = false;
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
	Clear_AnchorMarker();
	if (nullptr != m_pMapEffectPresentation)
	{
		m_pMapEffectPresentation->Clear();
		m_pMapEffectPresentation.reset();
	}
	if (nullptr != m_pMapLightPresentation)
	{
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
	}

	if (m_bBernBgmStarted)
		CGameInstance::Get().Stop_Music();

	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
}

HRESULT CLevel_Bern::Initialize()
{
	const auto FailActivation = [](const std::string_view source, const std::string_view detail)
	{
		// Preserve the refusing stage before MainApp reports its generic create failure.
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_LEVEL_CREATE_FAILED,
			source, detail, E_FAIL);
		return E_FAIL;
	};
	if (FAILED(__super::Initialize()))
		return FailActivation("bern.level-initialize", "Base level initialization failed.");

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
		return FailActivation("bern.map-area", m_MapRuntime.Get_Status());
	}

	// Keep the provider local until the rest of Level initialization commits.
	auto mapLightPresentation = make_shared<CMapLightPresentationRuntime>();
	if (!mapLightPresentation->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA(("[Level_Bern][MapLight] " +
			mapLightPresentation->Get_Status() + "\n").c_str());
		m_MapRuntime.Clear();
		return FailActivation("bern.map-light", mapLightPresentation->Get_Status());
	}

	if (FAILED(Ready_Layer_Camera(
			TEXT("Layer_Camera"), pEntry->pMapAreaId)))
	{
		return FailActivation("bern.camera", "Bern camera layer creation failed.");
	}

	auto mapEffectPresentation = make_shared<CMapEffectPresentationRuntime>();
	std::string mapEffectStatus;
	if (!mapEffectPresentation->Load_AmbientArea(
			ETOUI(LEVEL::BERN), pEntry->pMapAreaId, mapEffectStatus))
	{
		OutputDebugStringA(("[Level_Bern][MapEffect] " + mapEffectStatus + "\n").c_str());
		return FailActivation("bern.map-effect", mapEffectStatus);
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
		return FailActivation("bern.replication", "Approved Bern replication initialization failed.");
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	m_PlayerController.Set_ItemTargetResolver([this](const float3_t& origin, const float3_t& direction)
	{ return m_Replication.Find_ItemTargetPlayerFromRay(origin, direction); });
	if (!m_PlayerController.Initialize_TargetingPreview(ETOUI(LEVEL::BERN)))
		return FailActivation("bern.targeting-preview", "Player targeting preview initialization failed.");
	if (!m_PlayerController.Initialize_ClickMoveEffect(ETOUI(LEVEL::BERN)))
		return FailActivation("bern.click-move-effect", "Player click-move Effect initialization failed.");

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
	(void)Ready_ShipNpcs(pEntry->pMapAreaId);
	if (!Ready_ServiceNpcs(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Bern] Repair/shop NPC positions unavailable; right-click "
			"interaction is disabled.\n");
	}
	m_pValtanEntryView = std::make_unique<CRaidEntryPreviewView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::BERN));
	m_pInteractPrompt = std::make_unique<CInteractKeyPromptView>();
	m_pInteractPrompt->Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::BERN), pEntry->pMapAreaId);
	{
		std::vector<std::string> markerTargets;
		std::string markerStatus;
		if (!CEffectPresentationService::Queue_ProductTargets_Priority(
			{ "effect.bern.anchor.marker.marker.full.restore" }, markerTargets, markerStatus))
			Write_EffectFailureDiagnostic("AnchorMarker.Bern", "prepare isolated: " + markerStatus);
	}

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

	m_PartyInteraction.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::BERN));
	m_ChatBubbleView.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::BERN));
	m_SystemMenuButtons.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::BERN));

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

	m_pMapLightPresentation = std::move(mapLightPresentation);
	m_pMapEffectPresentation = std::move(mapEffectPresentation);
	m_bMapLightSubmissionFailureReported = false;
	OutputDebugStringA(("[Level_Bern][MapLight] " +
		m_pMapLightPresentation->Get_Status() + "\n").c_str());
	return S_OK;
}

void CLevel_Bern::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_bReturningToCharacterSelect)
		return;
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(LEVEL::BERN))
	{
		return;
	}

	if (nullptr != m_pMapEffectPresentation)
		m_pMapEffectPresentation->Update_LevelPresentation(fTimeDelta);

	if (nullptr != m_pMapLightPresentation &&
		!m_pMapLightPresentation->Submit_Frame() &&
		!m_bMapLightSubmissionFailureReported)
	{
		m_bMapLightSubmissionFailureReported = true;
		OutputDebugStringA(("[Level_Bern][MapLight] " +
			m_pMapLightPresentation->Get_Status() + "\n").c_str());
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
	if (Is_ValtanEntryModalOpen())
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PartyInteraction.Register_TextOccluders();
	/* The raid entry window's dim backdrop covers the whole screen. */
	if (Is_ValtanEntryModalOpen())
	{
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		CUITextOcclusion::Get().Add_Occluder(UI_TEXT_LAYER::MODAL, 0.f, 0.f, vViewport.x, vViewport.y);
	}
	if (m_PartyInteraction.Update(
		m_Replication, m_pPlayerCommandSink, m_NameplatePlayers,
		!Is_ValtanEntryModalOpen() &&
			nullptr != m_pCamera && m_pCamera->Is_FollowEnabled()))
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
#ifdef _DEBUG
	Update_ValtanEntryDebugPreviewKey();
#endif
	Update_SystemMenuButtons();
	Try_Send_CharacterRestore();
	Update_ValtanEntryInteraction();
	Advance_ValtanEntryWalk();
	Poll_RaidEntryVote();
	Poll_ColosseumQueueState();
	Update_ItemUpgradeNpcInteraction();
	Advance_ItemUpgradeNpcWalk();
	Update_ShipNpcInteraction();
	Advance_ShipNpcWalk();
	Update_ShipCamera(fTimeDelta);
	Update_ServiceNpcInteraction();
	Advance_ServiceNpcWalk();
	if (Is_ValtanEntryModalOpen())
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	if (nullptr != m_pInteractPrompt)
		m_pInteractPrompt->Update(fTimeDelta, localCharacter,
			CCombatHUDViewModel::Get().Get_InteractPromptTriggerId(),
			nullptr != m_pCamera && !m_pCamera->Is_PresentationOverrideActive());
	Update_AnchorMarker(fTimeDelta);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled(),
		nullptr != m_pCamera && !m_pCamera->Is_FollowRequested() &&
			!m_pCamera->Is_PresentationOverrideActive());

}

bool_t CLevel_Bern::Ready_EntranceCinematic()
{
	if (s_hasPresentedBernEntranceThisSession)
	{
		m_bEntranceCinematicDone = true;
		return true;
	}
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
	/* ESC skips the remainder through the same end path that restores the
	   follow camera; the press edge keeps a held key from re-triggering. */
	const bool_t isEscapeDown = 0 != (
		CGameInstance::Get().Get_DIKeyState(DIK_ESCAPE) & 0x80);
	const bool_t wasEscapePressed =
		isEscapeDown && !m_wasEscapeDownForEntranceSkip;
	m_wasEscapeDownForEntranceSkip = isEscapeDown;
	if (wasEscapePressed)
	{
		s_hasPresentedBernEntranceThisSession = true;
		End_EntranceCinematic();
		return;
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
	// Failed loading, ownership or pose application must not consume the first visit.
	s_hasPresentedBernEntranceThisSession = true;
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

	m_PlayerNameplateView.Render(m_NameplatePlayers, &m_Replication.Get_PartyRoster());
	m_ChatBubbleView.Render(m_Replication, m_NameplatePlayers);
	m_PartyInteraction.Render(m_pPlayerCommandSink);

	/* Render_ValtanEntryModal() itself is called from CMainApp::Render(), after
	   the combat HUD renders -- see its declaration comment in Level_Bern.h.
	   Only the LOA font label pass stays here. */
	Render_ValtanEntryModalText();
	if (nullptr != m_pInteractPrompt)
		m_pInteractPrompt->Render_Text();

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
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::BERN,
		m_FollowCameraProfile, m_strFollowCameraProfileStatus))
	{
		OutputDebugStringA(("[Level_Bern][FollowCamera] " +
			m_strFollowCameraProfileStatus + "\n").c_str());
	}
	const float3_t positionOffset = m_FollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
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
	const auto applyPlayerFraming = [&initialEye, &initialAt, &positionOffset, &lookOffset](
		const float3_t& position)
	{
		initialEye = float3_t(
			position.x + positionOffset.x,
			position.y + positionOffset.y,
			position.z + positionOffset.z);
		initialAt = float3_t(
			position.x + lookOffset.x,
			position.y + lookOffset.y,
			position.z + lookOffset.z);
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

	cameraDesc.fFovy = m_FollowCameraProfile.fovYDegrees;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar =
		(std::max)(2000.f, span * 8.f);

	cameraDesc.fSpeedPerSec = CCamera_Free::DEFAULT_ARENA_MOVE_SPEED;

	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	/*
	 * Player Spawn 이후 사용할 Follow Camera 설정이다.
	 * Initialize 시점에는 Player가 없으므로 비활성화한다.
	 */
	cameraDesc.pFollowTarget = nullptr;

	cameraDesc.vPositionOffset = positionOffset;
	cameraDesc.vLookOffset = lookOffset;
	cameraDesc.fFollowResponse = m_FollowCameraProfile.followResponse;
	cameraDesc.fFollowRollDegrees = m_FollowCameraProfile.rotationDegrees.z;
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

bool_t CLevel_Bern::Set_FollowCameraProfile(
	const ARENA_CAMERA_PROFILE& profile,
	std::string& outStatus)
{
	if (!CArenaCameraProfile::Validate(profile, outStatus))
		return false;
	if (nullptr == m_pCamera || !m_pCamera->Set_FollowPose(
		profile.positionOffset, CArenaCameraProfile::LookOffset(profile),
		profile.rotationDegrees.z, profile.fovYDegrees, profile.followResponse))
	{
		outStatus = "The active follow camera could not apply these settings.";
		return false;
	}
	m_FollowCameraProfile = profile;
	if (const auto character = Get_LocalCharacter())
		CCharacter::Set_MapPresentationSizeProfile(profile);
	outStatus = "Applied to this map's follow camera. Save to keep these settings.";
	m_strFollowCameraProfileStatus = outStatus;
	return true;
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
	CCharacter::Set_MapPresentationSizeProfile(m_FollowCameraProfile);
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();

	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;

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

	// The only NPC inside the castle interior; Handle_ConfirmNpcEntry names the same placement.
	static constexpr const char* COLOSSEUM_NPC_PLACEMENT_ID = "npc.bern.25184_1.2";
	VALTAN_ENTRY_NPC colosseumNpc{};
	bool_t hasColosseumNpc = false;
	std::vector<VALTAN_ENTRY_NPC> staged;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind)
			continue;
		if (placement.placementId == COLOSSEUM_NPC_PLACEMENT_ID)
		{
			colosseumNpc = { placement.placementId, placement.position, true };
			hasColosseumNpc = true;
			continue;
		}
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
	// After the guides, so the Debug O-key preview (front()) keeps naming a Valtan guide.
	if (hasColosseumNpc)
		staged.push_back(std::move(colosseumNpc));

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
	if (Is_ValtanEntryModalOpen() || !isRightMousePressed ||
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
	if (!m_isWalkingToValtanEntryNpc || Is_ValtanEntryModalOpen())
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
	if (nullptr != m_pValtanEntryView)
	{
		if (npcIt->isColosseum)
		{
			// Retail's timer dialog: accept / decline within 15 seconds, then the wait window.
			m_pValtanEntryView->Open_ColosseumOffer();
			/* Retail's loading avatar stands in the relaxed pose, not the class's battle idle: play the
			customizing idle now so the frame captured at the transfer already holds it. */
			if (const shared_ptr<CCharacter> pLocal = Get_LocalCharacter();
				nullptr != pLocal && nullptr != pLocal->Get_Spec())
			{
				if (const char_t* pClip = pLocal->Get_Spec()->AnimationClips[ETOUI(CHARACTER_ANIM::CUSTOMIZING_IDLE)])
					(void)pLocal->Set_Animation(pClip, true);
			}
		}
		else
			m_pValtanEntryView->Open();
	}
#ifdef _DEBUG
	OutputDebugStringA("[Level_Bern][ValtanEntryText] modal opened this frame\n");
#endif
}

bool_t CLevel_Bern::Ready_ServiceNpcs(const std::string& areaId)
{
	// The two placements carrying the anvil symbol (Minimap_Symbol_158) in
	// Data/UI/WorldMap/WorldMapNpcSymbols.json -- retail's repair service marker, as opposed
	// to npc.bern.schmidt's hammer, which is the item upgrade NPC above.
	static constexpr const char* REPAIR_NPC_PLACEMENT_IDS[] =
	{
		"npc.bern.src.31",
		"npc.bern.src.48",
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

	std::vector<SERVICE_NPC> staged;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind)
			continue;
		const bool_t isRepair = std::any_of(
			std::begin(REPAIR_NPC_PLACEMENT_IDS),
			std::end(REPAIR_NPC_PLACEMENT_IDS),
			[&placement](const char* pId)
			{
				return placement.placementId == pId;
			});
		if (isRepair)
			staged.push_back({ placement.placementId, placement.position, NPC_SERVICE::REPAIR });
		else if (nullptr != CItemCatalog::Find_ShopByNpc(placement.placementId))
			staged.push_back({ placement.placementId, placement.position, NPC_SERVICE::SHOP });
	}

	m_ServiceNpcs = std::move(staged);
	return !m_ServiceNpcs.empty();
}

void CLevel_Bern::Update_ServiceNpcInteraction()
{
	const bool_t isRightMouseDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDownForServiceNpcInteract;
	m_wasRightMouseDownForServiceNpcInteract = isRightMouseDown;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (m_ServiceNpcs.empty() || !isRightMousePressed ||
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
	f32_t fBestRayParameter = FLT_MAX;
	const SERVICE_NPC* pHit = nullptr;
	for (const SERVICE_NPC& npc : m_ServiceNpcs)
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
	m_strServiceNpcPlacementId = pHit->strPlacementId;
	m_isWalkingToServiceNpc = true;

	/* Stop just inside interaction range, on the side the character is already standing --
	same approach as Update_ItemUpgradeNpcInteraction. */
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr != transform)
	{
		const vector_t vNpcPos = XMLoadFloat3(&pHit->vPosition);
		const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
		vector_t vTowardCharacter = XMVectorSubtract(vCharacterPos, vNpcPos);
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
				vNpcPos, XMVectorScale(vTowardCharacter, INTERACTION_RADIUS * 0.7f)));
			goal.y = pHit->vPosition.y;
		}
		m_PlayerController.Request_MoveToPoint(goal);
	}
}

void CLevel_Bern::Advance_ServiceNpcWalk()
{
	if (!m_isWalkingToServiceNpc)
		return;

	const shared_ptr<CCharacter> localCharacter = m_Replication.Get_LocalCharacter();
	const auto npcIt = std::find_if(
		m_ServiceNpcs.begin(), m_ServiceNpcs.end(),
		[this](const SERVICE_NPC& npc)
		{
			return npc.strPlacementId == m_strServiceNpcPlacementId;
		});
	if (nullptr == localCharacter || m_ServiceNpcs.end() == npcIt)
	{
		m_isWalkingToServiceNpc = false;
		return;
	}
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return;

	// Same footprint Update_ServiceNpcInteraction stops the character at.
	constexpr f32_t INTERACTION_RADIUS = 3.f;
	const vector_t vCharacterPos = transform->Get_State(STATE::POSITION);
	const vector_t vDelta = XMVectorSubtract(
		vCharacterPos, XMLoadFloat3(&npcIt->vPosition));
	const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
		XMVectorSetY(vDelta, 0.f)));
	if (fDistanceSq > INTERACTION_RADIUS * INTERACTION_RADIUS)
		return;

	m_isWalkingToServiceNpc = false;
	if (CMainApp* pMainApp = CMainApp::Get_Active())
	{
		if (NPC_SERVICE::SHOP == npcIt->eService)
			pMainApp->Open_ShopWindow(npcIt->strPlacementId);
		else
			pMainApp->Open_RepairWindow();
	}
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


bool_t CLevel_Bern::Ready_ShipNpcs(const std::string& areaId)
{
	m_ShipNpcPositions.clear();
	m_iWalkingToShipNpc = -1;

	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") / areaId / "Gameplay.world.json");
	std::error_code pathError;
	if (documentPath.empty() ||
		!std::filesystem::is_regular_file(documentPath, pathError) || pathError)
	{
		Write_EffectFailureDiagnostic("ship.npc.ready",
			"area=" + areaId + " count=0 reason=world document missing: " + documentPath.string());
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
	{
		Write_EffectFailureDiagnostic("ship.npc.ready",
			"area=" + areaId + " count=0 reason=world document rejected: " + status);
		return false;
	}

	constexpr const char* SHIP_NPC_ARCHETYPE_PREFIX = "NPC_SHIP_";
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC == placement.eKind && placement.isEnabled &&
			0 == placement.archetypeId.rfind(SHIP_NPC_ARCHETYPE_PREFIX, 0))
		{
			m_ShipNpcPositions.push_back(placement.position);
		}
	}
	Write_EffectFailureDiagnostic("ship.npc.ready",
		"area=" + areaId + " count=" + std::to_string(m_ShipNpcPositions.size()) +
		" doc=" + documentPath.string());
	return !m_ShipNpcPositions.empty();
}

void CLevel_Bern::Update_ShipNpcInteraction()
{
	const bool_t isRightMouseDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	const bool_t isRightMousePressed =
		isRightMouseDown && !m_wasRightMouseDownForShipNpcInteract;
	m_wasRightMouseDownForShipNpcInteract = isRightMouseDown;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (m_ShipNpcPositions.empty() || !isRightMousePressed ||
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

	/* The NPC nearest to the click ray wins when two of them stand close together. */
	constexpr f32_t NPC_CLICK_RADIUS = 1.5f;
	int32_t iPicked = -1;
	f32_t fBestDistanceSq = NPC_CLICK_RADIUS * NPC_CLICK_RADIUS;
	for (size_t i = 0; i < m_ShipNpcPositions.size(); ++i)
	{
		const vector_t vNpcPos = XMLoadFloat3(&m_ShipNpcPositions[i]);
		const f32_t fRayParameter = XMVectorGetX(XMVector3Dot(
			XMVectorSubtract(vNpcPos, rayOrigin), rayDirection));
		if (fRayParameter < 0.f)
			continue;
		const vector_t vClosestPoint = XMVectorAdd(
			rayOrigin, XMVectorScale(rayDirection, fRayParameter));
		const f32_t fDistanceSq = XMVectorGetX(XMVector3LengthSq(
			XMVectorSubtract(vNpcPos, vClosestPoint)));
		if (fDistanceSq <= fBestDistanceSq)
		{
			fBestDistanceSq = fDistanceSq;
			iPicked = static_cast<int32_t>(i);
		}
	}
	if (iPicked < 0)
		return;

	m_PlayerController.Suppress_MoveClickThisFrame();
	CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	m_iWalkingToShipNpc = iPicked;
	Write_EffectFailureDiagnostic("ship.npc.clicked",
		"index=" + std::to_string(iPicked) + " npc=" +
		std::to_string(m_ShipNpcPositions[static_cast<size_t>(iPicked)].x) + "," +
		std::to_string(m_ShipNpcPositions[static_cast<size_t>(iPicked)].y) + "," +
		std::to_string(m_ShipNpcPositions[static_cast<size_t>(iPicked)].z));

	/* Stop just inside interaction range on the side the character already stands. */
	const float3_t& npcPosition = m_ShipNpcPositions[static_cast<size_t>(iPicked)];
	const vector_t vNpcPos = XMLoadFloat3(&npcPosition);
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr != transform)
	{
		vector_t vTowardCharacter = XMVectorSubtract(transform->Get_State(STATE::POSITION), vNpcPos);
		vTowardCharacter = XMVectorSetY(vTowardCharacter, 0.f);
		constexpr f32_t INTERACTION_RADIUS = 3.f;
		float3_t goal{};
		if (XMVectorGetX(XMVector3LengthSq(vTowardCharacter)) < 0.01f)
		{
			goal = npcPosition;
		}
		else
		{
			vTowardCharacter = XMVector3Normalize(vTowardCharacter);
			XMStoreFloat3(&goal, XMVectorAdd(
				vNpcPos, XMVectorScale(vTowardCharacter, INTERACTION_RADIUS * 0.7f)));
			goal.y = npcPosition.y;
		}
		m_PlayerController.Request_MoveToPoint(goal);
	}
}

void CLevel_Bern::Advance_ShipNpcWalk()
{
	if (m_iWalkingToShipNpc < 0)
		return;

	const shared_ptr<CCharacter> localCharacter = m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter ||
		static_cast<size_t>(m_iWalkingToShipNpc) >= m_ShipNpcPositions.size())
	{
		m_iWalkingToShipNpc = -1;
		return;
	}
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return;

	constexpr f32_t INTERACTION_RADIUS = 3.f;
	const vector_t vDelta = XMVectorSubtract(transform->Get_State(STATE::POSITION),
		XMLoadFloat3(&m_ShipNpcPositions[static_cast<size_t>(m_iWalkingToShipNpc)]));
	if (XMVectorGetX(XMVector3LengthSq(XMVectorSetY(vDelta, 0.f))) >
		INTERACTION_RADIUS * INTERACTION_RADIUS)
	{
		return;
	}

	m_iWalkingToShipNpc = -1;
	if (CMainApp* pMainApp = CMainApp::Get_Active())
	{
		Write_EffectFailureDiagnostic("ship.window.open",
			"requested: the character is inside the ship NPC interaction radius");
		pMainApp->Open_ShipWindow();
	}
}

void CLevel_Bern::Clear_AnchorMarker()
{
	if (0u != m_iAnchorMarkerHandle)
	{
		EFFECT_WORLD_ROOT_HANDLE handle{};
		handle.iValue = m_iAnchorMarkerHandle;
		CEffectPresentationService::Stop_WorldRoot(handle);
	}
	m_iAnchorMarkerHandle = 0u;
	m_bAnchorMarkerPlaced = false;
}

/* The golden anchor volume on the water in front of the dock trigger (EFTable_VoyageAnchorVolume
   -> Prop ITR_10297 DockingVolume). It is placed only where the authored viewer document has a
   dock trigger, at that trigger, so it can never mark a place the Server does not offer. */
void CLevel_Bern::Update_AnchorMarker(const f32_t fTimeDelta)
{
	if (m_bAnchorMarkerPlaced || nullptr == m_pInteractPrompt || m_iAnchorMarkerAttempts >= 40u)
		return;
	m_fAnchorMarkerRetrySeconds -= fTimeDelta;
	if (m_fAnchorMarkerRetrySeconds > 0.f)
		return;
	float3_t vCenter{};
	f32_t fYawDegrees = 0.f;
	if (!m_pInteractPrompt->Try_Get_DockPoint(vCenter, fYawDegrees))
	{
		m_iAnchorMarkerAttempts = 40u;
		return;
	}
	m_fAnchorMarkerRetrySeconds = 0.5f;
	++m_iAnchorMarkerAttempts;
	EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
	desc.iLevelIndex = ETOUI(LEVEL::BERN);
	desc.strPlacementId = "bern.anchor.marker";
	desc.strEffectAssetId = "effect.bern.anchor.marker.marker.full.restore";
	desc.bOwnerSustainedSourceLoops = true;
	XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(fYawDegrees)) *
		XMMatrixTranslation(vCenter.x, vCenter.y, vCenter.z));
	EFFECT_WORLD_ROOT_HANDLE staged;
	std::string status;
	if (!CEffectPresentationService::Spawn_LevelPlacement(desc, staged, status))
	{
		if (m_iAnchorMarkerAttempts == 1u || m_iAnchorMarkerAttempts == 40u)
			Write_EffectFailureDiagnostic("AnchorMarker.Bern", "spawn waiting: " + status);
		return;
	}
	CEffectPresentationService::Commit_PendingWorldRootSpawns({ staged });
	if (!CEffectPresentationService::Update_WorldRoot(staged, desc.RootWorld))
	{
		Write_EffectFailureDiagnostic("AnchorMarker.Bern", "activation failed: " + CEffectPresentationService::Get_Status());
		CEffectPresentationService::Stop_WorldRoot(staged);
		return;
	}
	m_iAnchorMarkerHandle = staged.iValue;
	m_bAnchorMarkerPlaced = true;
	char line[192]{};
	snprintf(line, sizeof(line), "placed asset=%s pos=(%.2f, %.2f, %.2f) yaw=%.1f attempts=%u",
		desc.strEffectAssetId.c_str(), vCenter.x, vCenter.y, vCenter.z, fYawDegrees, m_iAnchorMarkerAttempts);
	Write_EffectFailureDiagnostic("AnchorMarker.Bern", line);
}

void CLevel_Bern::Update_ShipCamera(const f32_t fTimeDelta)
{
	if (nullptr == m_pCamera)
		return;

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const VEHICLE_ACTOR_ENTRY* pVehicle = (player.isValid && 0u != player.iVehicleId) ?
		CActorCatalog::Find_Vehicle(player.iVehicleId) : nullptr;
	const bool_t onShip = nullptr != pVehicle && pVehicle->isShip;
	const ARENA_CAMERA_PROFILE& base = m_FollowCameraProfile;
	if (!onShip)
	{
		if (!m_bShipCameraActive)
			return;
		const float3_t look = CArenaCameraProfile::LookOffset(base);
		if (m_pCamera->Set_FollowPose(base.positionOffset, look, base.rotationDegrees.z,
			base.fovYDegrees, base.followResponse))
		{
			m_bShipCameraActive = false;
			Write_EffectFailureDiagnostic("ship.camera.applied",
				"restored the map camera: distance=" + std::to_string(base.focusDistance) +
				" fovY=" + std::to_string(base.fovYDegrees));
		}
		return;
	}

	/* EFTable_CameraSetting 1001 zoom steps (VoyageShip.Camera_Ocean of every base ship), kept in
	   source units in Data/Camera/Bern.camera.json "shipCamera". The source draws each ship mesh at
	   its LookInfo scale (0.26-0.5); this project draws it at 1, so ZoomDist and RelativeZ are divided
	   by that scale and the retail framing of the ridden ship is kept. Pitch and yaw use the (x, z, -y)
	   basis of the 2026-09-14 restoration and the FOV is horizontal at 16:9. */
	const ARENA_SHIP_CAMERA& lens = base.shipCamera;
	f32_t meshScale = 1.f;
	for (const ARENA_SHIP_MESH_SCALE& row : lens.shipMeshScales)
		if (row.vehicleId == player.iVehicleId)
			meshScale = row.scale;
	bool_t inAnchorVolume = false;
	if (const shared_ptr<CTransform> pTarget = m_pCamera->Get_FollowTarget())
	{
		float3_t position{};
		XMStoreFloat3(&position, pTarget->Get_State(STATE::POSITION));
		inAnchorVolume = lens.anchorVolume.z > 0.f &&
			std::abs(position.x - lens.anchorVolume.x) <= lens.anchorVolume.z &&
			std::abs(position.z - lens.anchorVolume.y) <= lens.anchorVolume.z;
	}
	const bool_t boarding = !m_bShipCameraActive;
	const uint32_t previousStep = m_iShipZoomStep;
	const uint32_t stepCount = static_cast<uint32_t>(lens.zoomSteps.size());
	// Crossing the anchor volume (and boarding) takes that region's step, as the retail
	// view closes in near a port and opens out at sea; the wheel moves between steps.
	if (boarding || inAnchorVolume != m_bShipInAnchorVolume)
		m_iShipZoomStep = (std::min)((inAnchorVolume ? lens.anchorStep : lens.openSeaStep), stepCount) - 1u;
	m_bShipInAnchorVolume = inAnchorVolume;
	const int32_t wheel = CGameInstance::Get().Get_DIMouseMove(DIMM::WHEEL);
	if (!boarding && 0 != wheel && GetForegroundWindow() == g_hWnd &&
		!CGameInstance::Get().IsMouseInputBlocked() && !ImGui::GetIO().WantCaptureMouse &&
		!CUIInputRouter::Get().Is_MouseClaimedThisFrame() && !m_pCamera->Is_PresentationOverrideActive())
	{
		// Wheel up is ZoomIn: step 1 open sea -> 2 coast -> 3 ship.
		if (wheel > 0 && m_iShipZoomStep + 1u < stepCount)
			++m_iShipZoomStep;
		else if (wheel < 0 && m_iShipZoomStep > 0u)
			--m_iShipZoomStep;
	}

	const ARENA_SHIP_CAMERA_STEP& step = lens.zoomSteps[m_iShipZoomStep];
	const SHIP_LENS target{
		step.zoomDistCm * 0.01f / meshScale,
		-step.sourcePitchDegrees,
		step.sourceYawDegrees + 90.f,
		step.fovXDegrees,
		step.relativeZCm * 0.01f / meshScale };
	if (boarding)
		m_ShipLens = target;
	else if (std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
	{
		// The source eases the view with its interpolation ratio; the target step's ratio drives it.
		const f32_t alpha = -std::expm1(-step.interpolationRatio * fTimeDelta);
		m_ShipLens.distanceMeters += (target.distanceMeters - m_ShipLens.distanceMeters) * alpha;
		m_ShipLens.pitchDegrees += (target.pitchDegrees - m_ShipLens.pitchDegrees) * alpha;
		m_ShipLens.yawDegrees += (target.yawDegrees - m_ShipLens.yawDegrees) * alpha;
		m_ShipLens.fovXDegrees += (target.fovXDegrees - m_ShipLens.fovXDegrees) * alpha;
		m_ShipLens.focusOffsetYMeters += (target.focusOffsetYMeters - m_ShipLens.focusOffsetYMeters) * alpha;
	}

	const f32_t fovY = XMConvertToDegrees(2.f * atanf(
		tanf(XMConvertToRadians(m_ShipLens.fovXDegrees) * 0.5f) * 9.f / 16.f));
	const auto rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(m_ShipLens.pitchDegrees), XMConvertToRadians(m_ShipLens.yawDegrees), 0.f);
	float3_t forward;
	XMStoreFloat3(&forward, XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotation));
	const float3_t shipLook(0.f, m_ShipLens.focusOffsetYMeters, 0.f);
	const float3_t eye(
		shipLook.x - forward.x * m_ShipLens.distanceMeters,
		shipLook.y - forward.y * m_ShipLens.distanceMeters,
		shipLook.z - forward.z * m_ShipLens.distanceMeters);
	const bool_t applied = boarding ?
		m_pCamera->Set_FollowPose(eye, shipLook, base.rotationDegrees.z, fovY, lens.followResponse) :
		m_pCamera->Set_FollowLens(eye, shipLook, fovY);
	/* One line per boarding and per step change, so a framing complaint can be told apart
	   from a lens that never applied or a step the anchor volume never chose. */
	if (boarding || previousStep != m_iShipZoomStep)
	{
		Write_EffectFailureDiagnostic("ship.camera.applied",
			"vehicle=" + std::to_string(player.iVehicleId) +
			" applied=" + std::string(applied ? "1" : "0") +
			" step=" + std::to_string(m_iShipZoomStep + 1u) +
			" anchorVolume=" + std::string(inAnchorVolume ? "1" : "0") +
			" meshScale=" + std::to_string(meshScale) +
			" targetDistance=" + std::to_string(target.distanceMeters) +
			" targetPitch=" + std::to_string(target.pitchDegrees) +
			" targetFovX=" + std::to_string(target.fovXDegrees) +
			" fovY=" + std::to_string(fovY));
	}
	if (boarding && applied)
		m_bShipCameraActive = true;
}

bool_t CLevel_Bern::Is_ValtanEntryModalOpen() const
{
	return nullptr != m_pValtanEntryView && m_pValtanEntryView->Is_Open();
}

void CLevel_Bern::Render_ValtanEntryModalText()
{
	if (nullptr != m_pValtanEntryView)
		m_pValtanEntryView->RenderText();
}

void CLevel_Bern::Render_ValtanEntryModal()
{
	if (nullptr == m_pValtanEntryView)
		return;

	m_pValtanEntryView->Render();

	if (nullptr == m_pPlayerCommandSink)
		return;
	const CRaidEntryPreviewView::RAID_ENTRY_INTENT intent =
		m_pValtanEntryView->Consume_Intent();
	if (CRaidEntryPreviewView::RAID_ENTRY_INTENT::COLOSSEUM_JOIN == intent.eKind)
	{
		// Colosseum NPC: the offer was accepted. The Server queues the player, decides the teams once the
		// head count is met and stages the BERN -> COLOSSEUM transfer; the Client only follows
		// S2C_ENTER_ACCEPTED (Pump_ServerApprovedWorldTransfer). A refused send closes the wait window.
		CLevelTransitionService::Clear_ColosseumMatch();
		if (!m_pPlayerCommandSink->Request_ColosseumQueueJoin(
			m_iNextNpcEntryConfirmSequence++, m_strValtanEntryNpcPlacementId))
			m_pValtanEntryView->Close_ColosseumWait();
	}
	else if (CRaidEntryPreviewView::RAID_ENTRY_INTENT::COLOSSEUM_LEAVE == intent.eKind)
	{
		(void)m_pPlayerCommandSink->Request_ColosseumQueueLeave(m_iNextNpcEntryConfirmSequence++);
	}
	else if (CRaidEntryPreviewView::RAID_ENTRY_INTENT::PROPOSE == intent.eKind)
	{
		// 입장하기 -> 파티 전원 수락 투표 발의(솔로는 인원 1명 투표). 즉시 전송하지 않고
		// 서버가 파티 전원에게 프롬프트를 보낸다.
		m_pPlayerCommandSink->Request_RaidEntryPropose(
			m_iNextNpcEntryConfirmSequence++,
			m_strValtanEntryNpcPlacementId,
			intent.eTarget);
	}
	else if (CRaidEntryPreviewView::RAID_ENTRY_INTENT::RESPOND == intent.eKind)
	{
		m_pPlayerCommandSink->Request_RaidEntryRespond(
			m_iNextNpcEntryConfirmSequence++,
			intent.iProposalId,
			intent.bAccepted);
	}
}

void CLevel_Bern::Update_SystemMenuButtons()
{
	/* Hidden while the entrance cinematic plays or the raid entry window is up. */
	const bool_t bEntranceCinematic = m_bEntranceCinematicApplied && !m_bEntranceCinematicDone;
	const CSystemMenuButtonsView::INTENT eIntent =
		m_SystemMenuButtons.Update(!bEntranceCinematic && !Is_ValtanEntryModalOpen());
	if (m_SystemMenuButtons.Is_PointerOver())
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	CMainApp* pMainApp = CMainApp::Get_Active();
	if (nullptr == pMainApp)
		return;
	switch (eIntent)
	{
	case CSystemMenuButtonsView::INTENT::OPEN_OPTIONS:
		pMainApp->Open_SystemOptionsWindow();
		break;
	case CSystemMenuButtonsView::INTENT::RETURN_TO_CHARACTER_SELECT:
		if (pMainApp->Return_ToCharacterSelect())
			m_bReturningToCharacterSelect = true;
		break;
	default:
		break;
	}
}

void CLevel_Bern::Try_Send_CharacterRestore()
{
	/* Once the local character is standing the Server has admitted this entry. A character with
	no saved state just keeps the Server's fresh start. */
	if (m_bCharacterRestoreSent || nullptr == m_Replication.Get_LocalCharacter())
		return;
	CHARACTER_WORLD_STATE State{};
	if (!CCharacterSelectionState::Try_Get_ActiveWorldState(State))
	{ m_bCharacterRestoreSent = true; return; }
	if (CNetworkManager::Get().Send_RestoreCharacter(
		1u, State.Items, State.iSilver, State.iGold, State.iHonorTitleId))
	{
		CCharacterSelectionState::Mark_RestoreRequested(1u);
		m_bCharacterRestoreSent = true;
	}
}

void CLevel_Bern::Poll_ColosseumQueueState()
{
	if (nullptr == m_pValtanEntryView || nullptr == m_pPlayerCommandSink)
		return;
	LostArk::Shared::S2C_COLOSSEUM_QUEUE_STATE state{};
	while (m_pPlayerCommandSink->Consume_ColosseumQueueState(state))
	{
		m_pValtanEntryView->Set_ColosseumQueueState(state);
	}
}

void CLevel_Bern::Poll_RaidEntryVote()
{
	if (nullptr == m_pValtanEntryView)
		return;
	using namespace LostArk::Shared;
	// 프롬프트 수신 -> 수락/거절 창을 연다(파티원은 입장 UI를 안 열었어도 이 창만 뜬다).
	S2C_RAID_ENTRY_PROMPT prompt{};
	if (m_Replication.Try_Consume_RaidEntryPrompt(prompt))
		m_pValtanEntryView->Open_VoteConfirm(prompt.iProposalId, prompt.eTarget);
	// 종료 통지 -> 거절/타임아웃/취소면 창을 닫고 Bern에 남는다. ALL_ACCEPTED는 서버가
	// 이어서 S2C_ENTER_ACCEPTED를 보내 Pump_ServerApprovedWorldTransfer가 레벨을 전환한다.
	S2C_RAID_ENTRY_VOTE vote{};
	if (m_Replication.Try_Consume_RaidEntryVote(vote) && vote.bClosed &&
		RAID_ENTRY_VOTE_RESULT::ALL_ACCEPTED != vote.eResult)
	{
		m_pValtanEntryView->Close_VoteConfirm();
	}
}

#ifdef _DEBUG
void CLevel_Bern::Update_ValtanEntryDebugPreviewKey()
{
	if (Is_ValtanEntryModalOpen() || ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_TextInputActive())
		return;
	const bool_t isODown =
		0 != (CGameInstance::Get().Get_DIKeyState(DIK_O) & 0x80);
	const bool_t wasOPressed = isODown && !m_wasODownForValtanEntryDebugPreview;
	m_wasODownForValtanEntryDebugPreview = isODown;
	if (!wasOPressed || nullptr == m_pValtanEntryView)
		return;
	m_strValtanEntryNpcPlacementId = m_ValtanEntryNpcs.empty() ?
		std::string() : m_ValtanEntryNpcs.front().strPlacementId;
	m_isWalkingToValtanEntryNpc = false;
	m_pValtanEntryView->Open();
}

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
		/* The debug layer is an authored-location aid, not a product trigger
		   implementation. Bern's live boxes are movePlayer boxes, so filtering
		   this to changeLevel made every useful deck-transfer marker disappear. */
		const bool_t isEnabledTrigger =
			placement.isEnabled &&
			WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind;
		if (!isEnabledTrigger)
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

		/* These are only non-interactive Debug-layer clones retained for
		   location diagnostics.  They are not Map Tool outlines: Map Tool owns
		   its own Layer_TriggerBoxes instances and explicitly reveals those
		   only while F1 -> World Gameplay is open.  Keeping this runtime copy
		   visible made every enabled gameplay trigger leak into normal Bern
		   play in a Debug build. */
		triggerBox->Set_AuthoringVisible(false);
		staged.push_back(std::move(triggerBox));
	}

	m_DebugLevelChangeTriggers = std::move(staged);
	OutputDebugStringA((
		"[Level_Bern] Debug enabled Trigger Boxes ready: " +
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
