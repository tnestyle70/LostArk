#include "Level_KakulSaydonArena.h"

#include "Camera_Free.h"
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "KakulArenaHiddenPlacements.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuMadnessGaugeView.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "ValtanCinematicCameraController.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "ProjectDataRoot.h"
#include "UILayoutRuntime.h"
#include "Transform.h"
#include "Trigger_Box.h"
#include "WorldGameplayDocument.h"

#include <algorithm>
#include <cstring>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <unordered_set>

namespace
{
	// Each arena remembers its own chosen speed for this process session.
	f32_t g_KakulSaydonFreeCameraSpeed = CCamera_Free::DEFAULT_ARENA_MOVE_SPEED;
	constexpr std::string_view KAKULSAYDON_AREA_ID =
		"LV_LUT_MIDNIGHTC_ED";
	/* Runtime placement IDs from the authored deploy placements. Rows 1 and 4
	   are the paper stage bridges; rows 2 and 3 are the levers that raise them
	   and stay visible from the first frame. */
	constexpr std::array<uint64_t, 2> KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS = {
		1ull,
		4ull,
	};
	/* One lever raises exactly one bridge. The authored sequence instances play
	   the lever pull and the bridge unfold, so the level only decides when. */
	struct PAPER_BRIDGE_LINK final
	{
		uint64_t leverPlacementId;
		uint64_t bridgePlacementId;
		std::string_view leverSequenceInstanceId;
		std::string_view bridgeSequenceInstanceId;
	};
	constexpr std::array<PAPER_BRIDGE_LINK, 2> KAKULSAYDON_PAPER_BRIDGE_LINKS = {
		PAPER_BRIDGE_LINK{ 2ull, 1ull,
			"world.sequence.instance.3", "world.sequence.instance.1" },
		PAPER_BRIDGE_LINK{ 3ull, 4ull,
			"world.sequence.instance.6", "world.sequence.instance.5" },
	};
	/* Map placement IDs the circus finale raises. They are authored standing so
	   the Map Tool can edit them in place, so the level suppresses them here
	   instead of letting the arena open with the finale already assembled. The
	   paper wall is deliberately absent: it must stand until the sequence
	   topples it. */
	constexpr std::array<uint64_t, 22> KAKULSAYDON_CIRCUS_FINALE_PLACEMENT_IDS = {
		8ull, 10ull, 11ull, 12ull, 13ull, 14ull, 15ull, 16ull, 18ull,
		19ull, 20ull, 21ull, 23ull, 24ull, 25ull, 26ull, 27ull, 28ull,
		/* The four stage curtains sweep in at the end of the finale, so they
		   stay hidden with the rest instead of framing an empty plaza. */
		33ull, 35ull, 38ull, 39ull,
	};
	constexpr std::string_view STAGE_MARKER_SCHEMA =
		"lostark.kakul-stage-markers-runtime";
	constexpr std::string_view STAGE_SEMANTIC_STATUS =
		"SOURCE_LEVEL_ID_ONLY";

	constexpr std::string_view CAMERA_SHOT_SCHEMA = "lostark.camera-shots";
	constexpr size_t CAMERA_SHOT_MAX_COUNT = 64u;
	constexpr uint32_t CAMERA_SHOT_MAX_BLEND_MS = 10000u;
	constexpr uint32_t CAMERA_SHOT_MAX_PRIORITY = 1000u;
	constexpr f32_t CAMERA_SHOT_MAX_HALF_EXTENT = 1000.f;
	constexpr f32_t CAMERA_SHOT_MAX_COORDINATE = 100000.f;
	/* A shot is released only once the Character stands this far outside its
	   box, so walking the boundary cannot flip the camera every frame. */
	constexpr f32_t CAMERA_SHOT_EXIT_MARGIN = 0.5f;
	/* A cue longer than the cutscene it rides is authoring nonsense, and a
	   key list longer than this is past what one shot can be read as. */
	constexpr uint32_t CAMERA_TRACK_MAX_DURATION_MS = 120000u;
	constexpr size_t CAMERA_TRACK_MAX_KEYFRAMES = 64u;
	constexpr f32_t CAMERA_TRACK_MIN_LOOK_DISTANCE = 0.01f;
	/* Distinct from the Bern and Valtan cinematic owners so the engine's
	   single-owner override never confuses this arena with theirs. */
	constexpr uint64_t KAKULSAYDON_CAMERA_SHOT_OWNER_ID = 0x4B414B554C534854ull;
	/* The pop-up book cutscene and the boss prop it stages. The boss is
	   presentation only, so it leaves the arena when this sequence ends. */
	constexpr const char* KAKULSAYDON_CUTSCENE_SEQUENCE_ID =
		"world.sequence.instance.original_kouku";
	constexpr uint64_t KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID = 5ull;
	constexpr uint64_t KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID = 7ull;
	/* Every instance whose id starts with this belongs to the same show. */
	constexpr const char* KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX =
		"world.sequence.instance.original_";
	/* The unfolding copy of the tent. These placements are hidden while the
	   arena stands and take over for the length of the cutscene. */
	constexpr uint64_t KAKULSAYDON_CUTSCENE_SET_FIRST_ID = 41ull;
	constexpr uint64_t KAKULSAYDON_CUTSCENE_SET_END_ID = 300ull;
	/* The follow camera this level installs. Reused when a shot hands the
	   camera back so the released pose matches the follow pose exactly. */
	const float3_t KAKULSAYDON_FOLLOW_POSITION_OFFSET(0.4f, 7.5f, 4.5f);
	const float3_t KAKULSAYDON_FOLLOW_LOOK_OFFSET(0.f, 1.2f, 0.f);
	constexpr f32_t KAKULSAYDON_FOLLOW_FOV_DEGREES = 60.f;

	const Client::DATA_JSON_VALUE* Required(
		const Client::DATA_JSON_VALUE& object,
		const char* name,
		const Client::DATA_JSON_TYPE type)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool Has_ExactProperties(
		const Client::DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		for (const std::string_view name : names)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		return true;
	}

	/* The shot object carries optional blocks. Counting the required names
	   and allowing only the known optional ones rejects unknown properties
	   just as strictly as one exact list per combination would. */
	bool Has_ShotProperties(
		const Client::DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> required,
		const std::initializer_list<std::string_view> optional)
	{
		if (!object.Is_Object())
			return false;
		size_t known = 0u;
		for (const std::string_view name : required)
		{
			if (nullptr == object.Find(name))
				return false;
			++known;
		}
		for (const std::string_view name : optional)
		{
			if (nullptr != object.Find(name))
				++known;
		}
		return object.Get_Object().size() == known;
	}

	bool Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > 128u ||
			value == "." || value == "..")
		{
			return false;
		}
		return std::all_of(value.begin(), value.end(), [](const unsigned char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
		});
	}

	bool Is_DisplayText(const std::string_view value)
	{
		return !value.empty() && value.size() <= 512u &&
			std::none_of(value.begin(), value.end(), [](const unsigned char c)
			{
				return c < 0x20u;
			});
	}

	std::filesystem::path Find_CameraShotDocument()
	{
		return Client::CMapAssetCatalog::Get_MapDataRoot() /
			(std::filesystem::path(std::string(KAKULSAYDON_AREA_ID)).wstring() +
				L".camerashots.json");
	}

	bool Read_Float3(
		const Client::DATA_JSON_VALUE* value,
		const f32_t limit,
		float3_t& out)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		f32_t parts[3]{};
		for (size_t index = 0; index < 3u; ++index)
		{
			const Client::DATA_JSON_VALUE& part = value->Get_Array()[index];
			if (!part.Is_Number() || !std::isfinite(part.Get_Number()) ||
				std::abs(part.Get_Number()) > limit)
			{
				return false;
			}
			parts[index] = static_cast<f32_t>(part.Get_Number());
		}
		out = float3_t(parts[0], parts[1], parts[2]);
		return true;
	}

	bool Read_Uint(
		const Client::DATA_JSON_VALUE* value,
		const uint32_t maximum,
		uint32_t& out)
	{
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 || number > maximum ||
			std::floor(number) != number)
		{
			return false;
		}
		out = static_cast<uint32_t>(number);
		return true;
	}

	/* Same yawed box test the Server applies to trigger boxes, so a shot
	   authored with the trigger tools covers the ground it appears to. */
	bool Read_CameraTrack(
		const Client::DATA_JSON_VALUE& value,
		const std::string& shotId,
		Client::VALTAN_CINEMATIC_CAMERA_CUE& outCue,
		std::string& outStatus)
	{
		if (!Has_ExactProperties(value,
			{ "durationMs", "interpolation", "easing", "keyframes" }))
		{
			outStatus = "KoukuSaydon camera track shape is invalid: " + shotId;
			return false;
		}
		const Client::DATA_JSON_VALUE* interpolation =
			Required(value, "interpolation", Client::DATA_JSON_TYPE::STRING);
		const Client::DATA_JSON_VALUE* easing =
			Required(value, "easing", Client::DATA_JSON_TYPE::STRING);
		const Client::DATA_JSON_VALUE* keyframes =
			Required(value, "keyframes", Client::DATA_JSON_TYPE::ARRAY);
		uint32_t durationMs = 0u;
		if (nullptr == interpolation || nullptr == easing || nullptr == keyframes ||
			!Read_Uint(value.Find("durationMs"), CAMERA_TRACK_MAX_DURATION_MS,
				durationMs) ||
			0u == durationMs ||
			keyframes->Get_Array().size() < 2u ||
			keyframes->Get_Array().size() > CAMERA_TRACK_MAX_KEYFRAMES)
		{
			outStatus = "KoukuSaydon camera track values are invalid: " + shotId;
			return false;
		}
		if ("LINEAR" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
		}
		else if ("CATMULL_ROM" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		}
		else
		{
			outStatus = "KoukuSaydon camera track interpolation is unknown: " + shotId;
			return false;
		}
		if ("LINEAR" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		else if ("SMOOTHSTEP" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		else if ("HOLD" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
		else
		{
			outStatus = "KoukuSaydon camera track easing is unknown: " + shotId;
			return false;
		}
		outCue.strCueId = shotId;
		outCue.strPatternId.clear();
		outCue.strStageId.clear();
		outCue.strStageActionId.clear();
		outCue.iStageIndex = 0u;
		outCue.iDurationMs = durationMs;
		outCue.iTransitionInMs = 0u;
		outCue.iTransitionOutMs = 0u;
		/* The cutscene has no replicated actor to track; the shot is authored
		   in world space and the level's own blend owns the hand-over. */
		outCue.eTrackingMode = Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
		outCue.vTrackingOrigin = float3_t(0.f, 0.f, 0.f);
		outCue.fShakeAmplitude = 0.f;
		outCue.iShakeDurationMs = 0u;
		outCue.Keyframes.clear();
		outCue.Keyframes.reserve(keyframes->Get_Array().size());
		uint32_t previousTimeMs = 0u;
		std::unordered_set<std::string> sceneIds;
		for (const Client::DATA_JSON_VALUE& entry : keyframes->Get_Array())
		{
			if (!Has_ExactProperties(entry,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "KoukuSaydon camera keyframe shape is invalid: " + shotId;
				return false;
			}
			Client::VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			const Client::DATA_JSON_VALUE* sceneId =
				Required(entry, "sceneId", Client::DATA_JSON_TYPE::STRING);
			const Client::DATA_JSON_VALUE* fov =
				Required(entry, "fovYDegrees", Client::DATA_JSON_TYPE::NUMBER);
			uint32_t timeMs = 0u;
			if (nullptr == sceneId || !Is_StableId(sceneId->Get_String()) ||
				!sceneIds.emplace(sceneId->Get_String()).second ||
				!Read_Uint(entry.Find("timeMs"), durationMs, timeMs) ||
				!Read_Float3(entry.Find("eye"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vEye) ||
				!Read_Float3(entry.Find("lookAt"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vLookAt) ||
				nullptr == fov || !std::isfinite(fov->Get_Number()) ||
				fov->Get_Number() <= 1.0 || fov->Get_Number() >= 179.0)
			{
				outStatus = "KoukuSaydon camera keyframe values are invalid: " + shotId;
				return false;
			}
			if (outCue.Keyframes.empty())
			{
				if (0u != timeMs)
				{
					outStatus = "KoukuSaydon camera track must start at 0ms: " + shotId;
					return false;
				}
			}
			else if (timeMs <= previousTimeMs)
			{
				outStatus = "KoukuSaydon camera keyframes must advance: " + shotId;
				return false;
			}
			const f32_t dx = keyframe.vLookAt.x - keyframe.vEye.x;
			const f32_t dy = keyframe.vLookAt.y - keyframe.vEye.y;
			const f32_t dz = keyframe.vLookAt.z - keyframe.vEye.z;
			if (CAMERA_TRACK_MIN_LOOK_DISTANCE >
				std::sqrt(dx * dx + dy * dy + dz * dz))
			{
				outStatus = "KoukuSaydon camera keyframe has no view direction: " +
					shotId;
				return false;
			}
			keyframe.strSceneId = sceneId->Get_String();
			keyframe.iTimeMs = timeMs;
			keyframe.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
			previousTimeMs = timeMs;
			outCue.Keyframes.push_back(std::move(keyframe));
		}
		if (outCue.Keyframes.back().iTimeMs != durationMs)
		{
			outStatus = "KoukuSaydon camera track must end at its duration: " + shotId;
			return false;
		}
		return true;
	}

	bool Contains_CameraShot(
		const Client::CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT& shot,
		const float3_t& position,
		const f32_t margin)
	{
		const f32_t deltaX = position.x - shot.vCenter.x;
		const f32_t deltaZ = position.z - shot.vCenter.z;
		const f32_t yaw = XMConvertToRadians(shot.fYawDegrees);
		const f32_t cosine = std::cos(yaw);
		const f32_t sine = std::sin(yaw);
		const f32_t localX = cosine * deltaX - sine * deltaZ;
		const f32_t localZ = sine * deltaX + cosine * deltaZ;
		return std::abs(localX) <= shot.vHalfExtents.x + margin &&
			std::abs(position.y - shot.vCenter.y) <= shot.vHalfExtents.y + margin &&
			std::abs(localZ) <= shot.vHalfExtents.z + margin;
	}

	float3_t Lerp_Float3(const float3_t& from, const float3_t& to, const f32_t t)
	{
		return float3_t(
			from.x + (to.x - from.x) * t,
			from.y + (to.y - from.y) * t,
			from.z + (to.z - from.z) * t);
	}

	std::filesystem::path Find_StageMarkerDocument()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};

		const std::filesystem::path moduleDirectory =
			std::filesystem::path(modulePath).parent_path();
		const std::filesystem::path fileName =
			L"KAKULSAYDON_ARENA.stagemarkers.json";
		const std::filesystem::path adjacent = moduleDirectory /
			L"DataFiles" / L"World" / fileName;
		if (std::filesystem::is_regular_file(adjacent))
			return adjacent;
		const std::filesystem::path parent = moduleDirectory.parent_path() /
			L"DataFiles" / L"World" / fileName;
		return std::filesystem::is_regular_file(parent) ? parent : adjacent;
	}
}

Client::CLevel_KakulSaydonArena*
	Client::CLevel_KakulSaydonArena::s_pActiveInstance = nullptr;

Client::CLevel_KakulSaydonArena::CLevel_KakulSaydonArena(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
	s_pActiveInstance = this;
}

Client::CLevel_KakulSaydonArena::~CLevel_KakulSaydonArena()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	// The gate focus is this arena's session state; the next level starts neutral.
	CCombatHUDViewModel::Get().Clear_BossFocus();
#endif
	m_PlayerController.Set_LocalCharacter(nullptr);
	m_PlayerController.Set_CommandSink(nullptr);
	m_Replication.Reset();
	m_pWorldEntityCommandSink.reset();
	m_pPlayerCommandSink.reset();
	m_pCameraTarget.reset();
	m_pCamera.reset();
	m_SequencePlayer.Clear();
	m_DeployRuntime.Clear();
	m_MapRuntime.Clear();
}

HRESULT Client::CLevel_KakulSaydonArena::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::KAKULSAYDON_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		KAKULSAYDON_AREA_ID != pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}
	/* The product loader contract for this arena stages only the map and the
	   server player bundle, so the deploy models Load_Area clones are admitted
	   here through the same CDeployPropRuntime entry the loader uses for every
	   other Area. Failing closed keeps the arena out rather than entering it
	   with levers and bridges that can never appear. */
	std::string deployPrototypeStatus;
	if (!CDeployPropRuntime::Ensure_AreaPrototypes(
		m_pDevice,
		m_pContext,
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId,
		deployPrototypeStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][DeployProp] " +
			deployPrototypeStatus + "\n").c_str());
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	if (!m_DeployRuntime.Load_Area(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		pEntry->pMapAreaId))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][DeployProp] " +
			m_DeployRuntime.Get_Status() + "\n").c_str());
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	/* Levers stay INTACT so the player can find them. Each paper stage bridge
	   only exists once its lever is pulled, so suppress it here rather than
	   waiting for the first sequence frame and flashing an unfolded bridge. */
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> hiddenBridges;
	hiddenBridges.reserve(KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS.size());
	for (const uint64_t placementId : KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS)
		hiddenBridges.emplace_back(placementId, DEPLOY_PROP_STATE::DESPAWNED);
	if (!m_DeployRuntime.Set_States(hiddenBridges))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][PaperBridge] " +
			m_DeployRuntime.Get_Status() + "\n").c_str());
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	/* The finale reveals each of these on its own keyframe. Hiding them now
	   costs nothing if the sequence never runs, and a placement the runtime
	   cannot address is reported rather than silently left standing. */
	for (const uint64_t placementId : KAKULSAYDON_CIRCUS_FINALE_PLACEMENT_IDS)
	{
		MAP_RUNTIME_PLACED_ENTRY* const entry = CWorldSequencePlayer::Find_Placement(
			m_MapRuntime.Get_MutablePlacements(), placementId);
		if (nullptr == entry ||
			!CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][CircusFinale] placement not hidden: " +
				std::to_string(placementId) + "\n").c_str());
		}
	}

	/* The pop-up book cutscene raises the tent arena, so it must not already be
	   standing when the level opens. The generated list is every placement
	   within 80m of the roulette floor; the circus plaza is 800m away and never
	   overlaps it. */
	for (const uint64_t placementId : KAKUL_ARENA_HIDDEN_PLACEMENT_IDS)
	{
		MAP_RUNTIME_PLACED_ENTRY* const entry = CWorldSequencePlayer::Find_Placement(
			m_MapRuntime.Get_MutablePlacements(), placementId);
		if (nullptr == entry ||
			!CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][Arena] placement not hidden: " +
				std::to_string(placementId) + "\n").c_str());
		}
	}

	/* A missing or rejected sequence document only costs the scripted props
	   their animation. The arena itself, its Server contracts and every other
	   placement stay enterable, so report the loss instead of blocking entry. */
	CWorldSequencePlayer::TARGET_SET sequenceTargets{};
	sequenceTargets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
	sequenceTargets.pCatalog = &m_MapRuntime.Get_Catalog();
	sequenceTargets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
	sequenceTargets.pDeployRuntime = &m_DeployRuntime;
	if (!m_SequencePlayer.Load_Area(pEntry->pMapAreaId, sequenceTargets))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][WorldSequence] " +
			m_SequencePlayer.Get_Status() + "\n").c_str());
	}

	std::string stageStatus;
	if (!Load_StageMarkers(stageStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] " + stageStatus + "\n").c_str());
		return E_FAIL;
	}

	/* A missing or rejected shot document costs the arena its authored camera
	   only. Entry never depends on it, so report and keep the follow view. */
	if (!Load_CameraShots(m_strCameraShotStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][CameraShot] " +
			m_strCameraShotStatus + "\n").c_str());
	}

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	CClientReplication::DESC replicationDesc{};
	/* Built here rather than on first use so a trigger move never waits on a
	   JSON load, and hidden immediately because Render() can run before the
	   first Update() on the frame this Level is activated. */
	/* Most of this stage's life is EFActorMotion rather than matinee: cards
	   rocking, floor pieces turning. An absent document is not an error. */
	if (!m_MapRuntime.Load_SelfMotions(std::string(KAKULSAYDON_AREA_ID)))
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Self-motion document was rejected.\n");
	}

	m_pTriggerMoveFadeView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
		L"UI/KakulFade/KakulFadeUI.json");
	m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", false);
	m_pMadnessGaugeView = std::make_unique<CKoukuMadnessGaugeView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));

	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	if (!m_Replication.Initialize(replicationDesc))
		return E_FAIL;

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_pWorldEntityCommandSink = make_shared<CNetworkWorldEntityCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(LEVEL::KAKULSAYDON_ARENA)) ||
		!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(LEVEL::KAKULSAYDON_ARENA)))
	{
		return E_FAIL;
	}

#ifdef _DEBUG
	/* A missing wire is a missing wire, not a reason to keep the arena
	   shut, so this reports and carries on. */
	if (!Ready_DebugStageEntryTriggers(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Debug stage entry Trigger Box "
			"presentation failed.\n");
	}
#endif

	return S_OK;
}

void Client::CLevel_KakulSaydonArena::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::KAKULSAYDON_ARENA))
	{
		return;
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Failed to apply replication event.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-kakul-saydon.network-connection-lost",
			"KoukuSaydon replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Lobby recovery request was rejected; retrying.\n");
	}

	if (!Bind_CameraToLocalCharacter())
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Failed to bind local character camera.\n");
	}
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	// Avatar replacement keeps the same Server player and command sequences.
	m_PlayerController.Rebind_LocalCharacter(localCharacter);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled(),
		nullptr != m_pCamera && !m_pCamera->Is_FollowRequested() &&
		!m_pCamera->Is_PresentationOverrideActive());

#ifdef _DEBUG
	/* Gate spawn replies arrive one per requested placement. They are Debug
	   status only; the presentation itself follows the reliable spawn stream. */
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT spawnResult{};
	while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(spawnResult))
	{
		const char_t* pResult = "unsupported result";
		switch (spawnResult.eResult)
		{
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::SPAWNED:
			pResult = "spawned"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS:
			pResult = "already exists"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ACTIVATED:
			pResult = "activated"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::REJECTED:
			pResult = "rejected by Server"; break;
		default: break;
		}
		if (0u != m_DebugGatePendingPlacements.erase(spawnResult.strPlacementId) &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::SPAWNED != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ACTIVATED != spawnResult.eResult)
			m_bDebugGateFailed = true;
		m_strDebugGateStatus += "\n" + spawnResult.strPlacementId + ": " + pResult;
	}
	if (Is_DebugGatePending() && m_DebugGatePendingPlacements.empty() &&
		!m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		if (!m_bDebugGateFailed && m_PlayerController.Did_DebugPlayerPlacementSucceed())
		{
			const KAKUL_DEBUG_GATE& gate = Get_DebugGates()[m_iPendingDebugGate];
			CCombatHUDViewModel::Get().Set_BossFocusArchetype(
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			CCombatHUDViewModel::Get().Set_BossHidden(nullptr == gate.pHudFocusArchetypeId);
			CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss(
				nullptr != gate.pAuditionPlacementId ? gate.pAuditionPlacementId : "",
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			m_iActiveDebugGate = m_iPendingDebugGate;
			m_strDebugGateStatus += "\nGate activation confirmed by Server.";
		}
		else
		{
			m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
			m_strDebugGateStatus += "\nGate activation failed; correct the reported cause and retry.";
		}
		m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
		CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(false);
	}
#endif

	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
	targets.pCatalog = &m_MapRuntime.Get_Catalog();
	targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
	targets.pDeployRuntime = &m_DeployRuntime;
	/* The Server decided these started; this level only resolves each stable
	   instance ID against what it loaded and plays the presentation. */
	for (const std::string& instanceId :
		m_Replication.Consume_WorldSequencePlays())
	{
		std::string status;
		if (!Start_ServerRequestedSequence(instanceId, targets, status))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][WorldSequence] " + instanceId +
				": " + status + "\n").c_str());
		}
	}
	m_SequencePlayer.Update(fTimeDelta, targets);
	Update_CutsceneBossRetire(targets);
	Update_CameraShots(fTimeDelta);
	Update_TriggerMoveFade(fTimeDelta);
	m_MapRuntime.Update_SelfMotions(fTimeDelta);
	if (nullptr != m_pMadnessGaugeView)
	{
		m_pMadnessGaugeView->Update(fTimeDelta, localCharacter,
			CCombatHUDViewModel::Get().Get_KoukuGimmick());
	}
}

bool_t Client::CLevel_KakulSaydonArena::Start_PopupBookCutscene(
	const CWorldSequencePlayer::TARGET_SET& targets,
	std::string& outStatus)
{
	if (!targets.Is_Complete())
	{
		outStatus = "Cutscene targets are not ready";
		return false;
	}
	/* The book carries the unfold animation, so it has to be on the map
	   before its sequence samples the first frame. */
	if (!targets.pDeployRuntime->Set_State(
		KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, DEPLOY_PROP_STATE::INTACT))
	{
		outStatus = "Cutscene book could not be revealed: " +
			targets.pDeployRuntime->Get_Status();
		return false;
	}
	Apply_CutsceneSetVisible(true);

	const size_t prefixLength = strlen(KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX);
	size_t started = 0u;
	std::string rejected;
	for (const WORLD_SEQUENCE_INSTANCE& instance :
		m_SequencePlayer.Get_Document().Get_Instances())
	{
		if (instance.instanceId.size() < prefixLength ||
			0 != instance.instanceId.compare(0, prefixLength,
				KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX))
		{
			continue;
		}
		if (m_SequencePlayer.Play(instance.instanceId, targets))
		{
			++started;
			continue;
		}
		if (!rejected.empty())
			rejected += ", ";
		rejected += instance.instanceId.substr(prefixLength);
	}
	if (0u == started)
	{
		Apply_CutsceneSetVisible(false);
		outStatus = "Cutscene could not start: " + rejected;
		return false;
	}
	m_bCutsceneBossVisible = true;
	outStatus = rejected.empty() ? "Cutscene started" :
		"Cutscene started without " + rejected;
	return true;
}

void Client::CLevel_KakulSaydonArena::Apply_CutsceneSetVisible(
	const bool_t cutsceneVisible)
{
	if (m_bCutsceneSetVisible == cutsceneVisible)
		return;
	m_bCutsceneSetVisible = cutsceneVisible;
	/* The unfolding copy and the standing arena occupy the same space, so
	   exactly one of them is on screen at a time. */
	for (MAP_RUNTIME_PLACED_ENTRY& entry :
		m_MapRuntime.Get_MutablePlacements())
	{
		const uint64_t placementId = entry.record.placementId;
		const bool_t isCutsceneSet =
			KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= placementId &&
			placementId < KAKULSAYDON_CUTSCENE_SET_END_ID;
		if (isCutsceneSet)
		{
			(void)CMapPlacementRuntime::Set_RuntimeVisible(
				entry, cutsceneVisible);
			continue;
		}
		const bool_t isHiddenArena = std::find(
			KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.begin(),
			KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end(),
			placementId) != KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end();
		if (isHiddenArena)
		{
			(void)CMapPlacementRuntime::Set_RuntimeVisible(
				entry, !cutsceneVisible);
		}
	}
}

void Client::CLevel_KakulSaydonArena::Update_CutsceneBossRetire(
	const CWorldSequencePlayer::TARGET_SET& targets)
{
	if (!targets.Is_Complete())
		return;
	const bool_t playing =
		m_SequencePlayer.Is_Playing(KAKULSAYDON_CUTSCENE_SEQUENCE_ID);
	if (playing)
	{
		m_bCutsceneBossVisible = true;
		return;
	}
	if (!m_bCutsceneBossVisible)
		return;
	/* One retire per cutscene: the flag clears whether or not the prop was
	   still there, so a missing prop never retries every frame. */
	m_bCutsceneBossVisible = false;
	/* The show is over: the arena the cutscene built takes over from the
	   unfolding copy, and the presentation boss leaves with it. */
	Apply_CutsceneSetVisible(false);
	if (!targets.pDeployRuntime->Set_State(
		KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][Cutscene] boss retire failed: " +
			targets.pDeployRuntime->Get_Status() + "\n").c_str());
	}
}

bool_t Client::CLevel_KakulSaydonArena::Start_ServerRequestedSequence(
	const std::string& instanceId,
	const CWorldSequencePlayer::TARGET_SET& targets,
	std::string& outStatus)
{
	/* A bridge unfold is more than its sequence: the Deploy prop must leave
	   DESPAWNED first. Route those through the bridge contract so the reveal
	   and the animation stay one decision. */
	const auto link = std::find_if(
		KAKULSAYDON_PAPER_BRIDGE_LINKS.begin(),
		KAKULSAYDON_PAPER_BRIDGE_LINKS.end(),
		[&instanceId](const PAPER_BRIDGE_LINK& value)
		{
			return value.bridgeSequenceInstanceId == instanceId;
		});
	if (KAKULSAYDON_PAPER_BRIDGE_LINKS.end() != link)
		return Request_PaperBridgeUnfold(link->leverPlacementId, outStatus);

	/* The pop-up book show is authored as several instances but the Server
	   names only one of them, so that name starts the whole show. */
	if (KAKULSAYDON_CUTSCENE_SEQUENCE_ID == instanceId)
		return Start_PopupBookCutscene(targets, outStatus);

	if (!m_SequencePlayer.Play(instanceId, targets))
	{
		outStatus = m_SequencePlayer.Get_Status();
		return false;
	}
	outStatus = "World sequence started";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_PaperBridgeUnfold(
	const uint64_t leverPlacementId,
	std::string& outStatus)
{
	const auto link = std::find_if(
		KAKULSAYDON_PAPER_BRIDGE_LINKS.begin(),
		KAKULSAYDON_PAPER_BRIDGE_LINKS.end(),
		[leverPlacementId](const PAPER_BRIDGE_LINK& value)
		{
			return value.leverPlacementId == leverPlacementId;
		});
	if (KAKULSAYDON_PAPER_BRIDGE_LINKS.end() == link)
	{
		outStatus = "Unknown paper lever placement";
		return false;
	}
	if (!m_RaisedPaperBridges.insert(link->bridgePlacementId).second)
	{
		outStatus = "Paper bridge is already raised";
		return true;
	}

	CWorldSequencePlayer::TARGET_SET targets{};
	targets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
	targets.pCatalog = &m_MapRuntime.Get_Catalog();
	targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
	targets.pDeployRuntime = &m_DeployRuntime;

	/* Reveal before the first sample so the unfold plays from its own opening
	   frame. A failed reveal leaves the bridge hidden and stays retryable. */
	if (!m_DeployRuntime.Set_State(
		link->bridgePlacementId, DEPLOY_PROP_STATE::INTACT))
	{
		m_RaisedPaperBridges.erase(link->bridgePlacementId);
		outStatus = m_DeployRuntime.Get_Status();
		return false;
	}
	if (!m_SequencePlayer.Play(
			std::string(link->bridgeSequenceInstanceId), targets))
	{
		m_DeployRuntime.Set_State(
			link->bridgePlacementId, DEPLOY_PROP_STATE::DESPAWNED);
		m_RaisedPaperBridges.erase(link->bridgePlacementId);
		outStatus = m_SequencePlayer.Get_Status();
		return false;
	}
	/* The lever pull is decoration on top of the bridge contract: losing it
	   must not undo a bridge that is already unfolding. */
	if (!m_SequencePlayer.Play(
		std::string(link->leverSequenceInstanceId), targets))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][PaperLever] " +
			m_SequencePlayer.Get_Status() + "\n").c_str());
	}
	outStatus = "Paper bridge unfold started";
	return true;
}

HRESULT Client::CLevel_KakulSaydonArena::Render()
{
	const HRESULT drawn = __super::Render();
	if (FAILED(drawn))
		return drawn;
	/* Drawn last so it sits over the scene. The text only reports what the
	   Server is offering -- pressing G submits a command and the Server
	   decides, so nothing here can move the player by itself. */
	const std::string& offered =
		CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
	if (!offered.empty())
	{
		/* ASCII only: this file carries no other non-ASCII byte and has no BOM,
		   so a UTF-8 Korean literal here is read back in the system codepage. */
		static const tchar_t* const PROMPT = TEXT("[ G ]");
		const float2_t size = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), PROMPT);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), PROMPT,
			float2_t(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.62f),
			Colors::White, 0.f, float2_t(size.x * 0.5f, size.y * 0.5f), 1.f);
	}
	return drawn;
}

bool_t Client::CLevel_KakulSaydonArena::Load_StageMarkers(
	std::string& outStatus)
{
	const std::filesystem::path path = Find_StageMarkerDocument();
	std::error_code fileError;
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (path.empty() || fileError || 0u == fileBytes || fileBytes > 256u * 1024u)
	{
		outStatus = "KoukuSaydon StageMarkers document is missing or exceeds 256 KiB.";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaydon StageMarkers document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaydon StageMarkers document could not be read completely.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 256u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 4096u;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "worldId", "areaId", "revision",
				"semanticStatus", "stages" }))
	{
		outStatus = "KoukuSaydon StageMarkers root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* world = Required(root, "worldId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* area = Required(root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* semanticStatus = Required(
		root, "semanticStatus", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* stages = Required(root, "stages", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || STAGE_MARKER_SCHEMA != schema->Get_String() ||
		nullptr == version || version->Get_Number() != 1.0 ||
		nullptr == world || world->Get_String() != "KAKULSAYDON_ARENA" ||
		nullptr == area || KAKULSAYDON_AREA_ID != area->Get_String() ||
		nullptr == revision || !std::isfinite(revision->Get_Number()) ||
		revision->Get_Number() < 1.0 ||
		std::floor(revision->Get_Number()) != revision->Get_Number() ||
		nullptr == semanticStatus ||
		STAGE_SEMANTIC_STATUS != semanticStatus->Get_String() ||
		nullptr == stages || stages->Get_Array().empty() ||
		stages->Get_Array().size() > 64u)
	{
		outStatus = "KoukuSaydon StageMarkers header is invalid.";
		return false;
	}

	std::vector<KAKUL_STAGE_MARKER> stagedMarkers;
	std::unordered_set<std::string> stagedIds;
	std::unordered_set<std::string> stagedPlacementIds;
	stagedMarkers.reserve(stages->Get_Array().size());
	for (const DATA_JSON_VALUE& value : stages->Get_Array())
	{
		if (!Has_ExactProperties(value,
			{ "stageId", "placementId", "displayNameKo", "sourceLevelId" }))
		{
			outStatus = "KoukuSaydon StageMarkers stage has unexpected properties.";
			return false;
		}
		const DATA_JSON_VALUE* stageId = Required(value, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* placementId = Required(value, "placementId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* displayName = Required(value, "displayNameKo", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceLevelId = Required(value, "sourceLevelId", DATA_JSON_TYPE::STRING);
		if (nullptr == stageId || nullptr == placementId || nullptr == displayName ||
			nullptr == sourceLevelId ||
			!Is_StableId(stageId->Get_String()) ||
			!stageId->Get_String().starts_with("stage.kakul.") ||
			stageId->Get_String() != placementId->Get_String() ||
			!Is_DisplayText(displayName->Get_String()) ||
			!Is_StableId(sourceLevelId->Get_String()) ||
			!stagedIds.emplace(stageId->Get_String()).second ||
			!stagedPlacementIds.emplace(placementId->Get_String()).second)
		{
			outStatus = "KoukuSaydon StageMarkers stage identity or evidence is invalid.";
			return false;
		}
		stagedMarkers.push_back({
			stageId->Get_String(), placementId->Get_String(),
			displayName->Get_String(), sourceLevelId->Get_String() });
	}

	m_StageMarkers = std::move(stagedMarkers);
	m_StageMarkerPlacementIds = std::move(stagedPlacementIds);
	outStatus = "KoukuSaydon StageMarkers loaded.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Try_Get_AuthoringPreviewPlacement(
	float3_t& outPosition, std::string& outStatus) const
{
	const shared_ptr<CCharacter> localCharacter = m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		outStatus = "Waiting for the replicated local player in the KoukuSaydon arena.";
		return false;
	}
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
	{
		outStatus = "The replicated local player has no transform for preview placement.";
		return false;
	}
	const vector_t playerPosition = transform->Get_State(STATE::POSITION);
	float3_t position{};
	XMStoreFloat3(&position, playerPosition);
	if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
		!std::isfinite(position.z))
	{
		outStatus = "The replicated local player position is not finite.";
		return false;
	}

	vector_t screenRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pCamera)
	{
		const shared_ptr<CTransform> cameraTransform = dynamic_pointer_cast<CTransform>(
			m_pCamera->Get_Component(g_strTransformComTag));
		if (nullptr != cameraTransform)
		{
			vector_t candidate = cameraTransform->Get_State(STATE::RIGHT);
			candidate = XMVectorSetW(XMVectorSetY(candidate, 0.f), 0.f);
			const f32_t lengthSquared = XMVectorGetX(XMVector3LengthSq(candidate));
			if (std::isfinite(lengthSquared) && lengthSquared > 0.000001f)
				screenRight = XMVector3Normalize(candidate);
		}
	}

	constexpr f32_t PREVIEW_OFFSET_METERS = 3.25f;
	for (const f32_t direction : std::array<f32_t, 2>{ 1.f, -1.f })
	{
		float3_t candidate{};
		XMStoreFloat3(&candidate,
			playerPosition + screenRight * (PREVIEW_OFFSET_METERS * direction));
		float3_t sampled{};
		if (localCharacter->Try_SampleTargetGround(candidate.x, candidate.z, sampled) &&
			std::isfinite(sampled.x) && std::isfinite(sampled.y) && std::isfinite(sampled.z))
		{
			outPosition = sampled;
			outStatus = direction > 0.f ?
				"replicated local player / camera-right / Navigation" :
				"replicated local player / camera-left / Navigation";
			return true;
		}
	}

	// Navigation is optional for this collision-off view; retain the player's height.
	XMStoreFloat3(&outPosition, playerPosition + screenRight * PREVIEW_OFFSET_METERS);
	outStatus = "replicated local player / camera-right / unclamped";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport(
	const std::uint32_t requestSequence,
	const std::string_view placementId,
	std::string& outStatus)
{
	if (0u == requestSequence || placementId.empty())
	{
		outStatus = "KoukuSaydon stage teleport request identity is invalid.";
		return false;
	}
	if (m_StageMarkerPlacementIds.empty())
	{
		outStatus = "KoukuSaydon StageMarkers are not authored; teleport is isolated.";
		return false;
	}
	if (!m_StageMarkerPlacementIds.contains(std::string(placementId)))
	{
		outStatus = "KoukuSaydon stage marker placement ID is not authored.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_StageTeleport(
			requestSequence, placementId))
	{
		outStatus = "KoukuSaydon stage teleport command was rejected.";
		return false;
	}
	outStatus = "KoukuSaydon stage teleport command submitted.";
	return true;
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Set_DebugCameraSpeed(const f32_t metersPerSecond)
{
	if (nullptr == m_pCamera || !m_pCamera->Set_FreeMoveSpeed(metersPerSecond))
		return false;
	g_KakulSaydonFreeCameraSpeed = metersPerSecond;
	return true;
}

const std::array<Client::CLevel_KakulSaydonArena::KAKUL_DEBUG_GATE, 9>&
Client::CLevel_KakulSaydonArena::Get_DebugGates()
{
	/* Boss positions are the disabled placements in
	   Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json; only the player
	   position, the HUD focus and the audition target are Client Debug
	   values. Labels are UTF-8 byte escapes so the source encoding never
	   changes them. */
	static const std::array<KAKUL_DEBUG_GATE, 9> gates = { {
		// 1관문 - 세이튼
		KAKUL_DEBUG_GATE{ "1" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.g1.saydon", nullptr } },
			float3_t(-2.84f, 1.32f, 941.02f),
			"BOSS_KAKULSAYDON_G1_SAYDON", "boss.kakulsaydon.g1.saydon", nullptr },
		// 2관문 - 대형 세이튼, 쿠크 (HUD and audition follow Kouku)
		KAKUL_DEBUG_GATE{ "2" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEB\x8C\x80\xED\x98\x95" " " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC" ", " "\xEC\xBF\xA0\xED\x81\xAC",
			{ { "boss.kakulsaydon.g2.big-saydon", "boss.kakulsaydon.g2.kouku" } },
			float3_t(3.38f, 10.56f, 323.92f),
			"BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", nullptr },
		// 3관문 - 세이튼
		KAKUL_DEBUG_GATE{ "3" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.g3.saydon", nullptr } },
			float3_t(-2.84f, 1.32f, 941.02f),
			"BOSS_KAKULSAYDON_G3_SAYDON", "boss.kakulsaydon.g3.saydon", nullptr },
		// 1마리오 - player only
		KAKUL_DEBUG_GATE{ "1" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4" " (" "\xED\x94\x8C\xEB\xA0\x88\xEC\x9D\xB4\xEC\x96\xB4\xEB\xA7\x8C" ")",
			{ { nullptr, nullptr } },
			float3_t(-1150.f, -11.52f, -909.28f),
			nullptr, nullptr, nullptr },
		// 2마리오 ~ 카드미로: no navigation yet
		KAKUL_DEBUG_GATE{ "2" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } }, float3_t(0.f, 0.f, 0.f),
			nullptr, nullptr, "navigation " "\xEB\xAF\xB8\xEB\xB3\xB4\xEC\x9C\xA0\xEB\xA1\x9C" " " "\xEB\xB3\xB4\xEB\xA5\x98" },
		KAKUL_DEBUG_GATE{ "3" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } }, float3_t(0.f, 0.f, 0.f),
			nullptr, nullptr, "navigation " "\xEB\xAF\xB8\xEB\xB3\xB4\xEC\x9C\xA0\xEB\xA1\x9C" " " "\xEB\xB3\xB4\xEB\xA5\x98" },
		KAKUL_DEBUG_GATE{ "4" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } }, float3_t(0.f, 0.f, 0.f),
			nullptr, nullptr, "navigation " "\xEB\xAF\xB8\xEB\xB3\xB4\xEC\x9C\xA0\xEB\xA1\x9C" " " "\xEB\xB3\xB4\xEB\xA5\x98" },
		KAKUL_DEBUG_GATE{ "\xEC\xB9\xB4\xEB\x93\x9C\xEB\xAF\xB8\xEB\xA1\x9C", { { nullptr, nullptr } }, float3_t(0.f, 0.f, 0.f),
			nullptr, nullptr, "navigation " "\xEB\xAF\xB8\xEB\xB3\xB4\xEC\x9C\xA0\xEB\xA1\x9C" " " "\xEB\xB3\xB4\xEB\xA5\x98" },
		// 빙고 - 앵콜을 외친 쿠크세이튼 (Saydon holding the hammer)
		KAKUL_DEBUG_GATE{ "\xEB\xB9\x99\xEA\xB3\xA0" " - " "\xEC\x95\xB5\xEC\xBD\x9C\xEC\x9D\x84" " " "\xEC\x99\xB8\xEC\xB9\x9C" " " "\xEC\xBF\xA0\xED\x81\xAC\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.bingo.saydon", nullptr } },
			float3_t(-3.4f, 0.f, 1147.44f),
			"BOSS_KAKULSAYDON_BINGO_SAYDON", "boss.kakulsaydon.bingo.saydon", nullptr },
	} };
	return gates;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_ActivateGate(
	const size_t gateIndex, std::string& outStatus)
{
	const auto& gates = Get_DebugGates();
	if (gateIndex >= gates.size())
	{
		outStatus = m_strDebugGateStatus = "Unknown KoukuSaydon gate index.";
		return false;
	}
	const KAKUL_DEBUG_GATE& gate = gates[gateIndex];
	const std::string label = nullptr != gate.pLabel ? gate.pLabel : "gate";
	if (nullptr != gate.pDeferredReason)
	{
		outStatus = m_strDebugGateStatus = label + ": " + gate.pDeferredReason;
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		nullptr == m_Replication.Get_LocalCharacter())
	{
		outStatus = m_strDebugGateStatus =
			label + ": the replicated local player or command sink is unavailable.";
		return false;
	}
	/* Pre-check before any command leaves: a gate change while the previous
	   player move is still unanswered would replace the bosses but leave the
	   player at the old gate. Refusing here keeps boss, HUD and player on the
	   gate that is already in flight. */
	if (Is_DebugGatePending() || m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		outStatus = m_strDebugGateStatus =
			label + ": the previous gate's player move is still awaiting the Server; wait for its reply.";
		return false;
	}
	if (0u == m_iNextDebugGateRequestSequence)
	{
		outStatus = m_strDebugGateStatus =
			label + ": gate request sequence is exhausted; restart the Client.";
		return false;
	}
	const std::uint32_t requestSequence = m_iNextDebugGateRequestSequence;
	if ((std::numeric_limits<std::uint32_t>::max)() == m_iNextDebugGateRequestSequence)
		m_iNextDebugGateRequestSequence = 0u;
	else
		++m_iNextDebugGateRequestSequence;

	/* These ordered commands have separate Server results. Only their
	   confirmed success commits the active gate, HUD and audition target. */
	if (!m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(requestSequence))
	{
		outStatus = m_strDebugGateStatus = label + ": despawn command was rejected.";
		return false;
	}
	CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(true);
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_iPendingDebugGate = gateIndex;
	m_bDebugGateFailed = false;
	m_DebugGatePendingPlacements.clear();
	std::size_t spawnRequests = 0u;
	for (const char_t* pPlacementId : gate.BossPlacementIds)
	{
		if (nullptr == pPlacementId)
			continue;
		if (!m_pWorldEntityCommandSink->Request_SpawnWorldEntity(pPlacementId))
		{
			m_bDebugGateFailed = true;
			outStatus = m_strDebugGateStatus =
				label + ": spawn command was rejected for " + pPlacementId;
			return false;
		}
		m_DebugGatePendingPlacements.emplace(pPlacementId);
		++spawnRequests;
	}
	const bool_t teleportSubmitted = m_PlayerController.Request_DebugTeleportToPosition(
		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA,
		gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z);
	m_bDebugGateFailed = !teleportSubmitted;
	char_t summary[256]{};
	sprintf_s(summary,
		": despawn + %zu spawn request(s) sent; player -> (%.2f, %.2f, %.2f) %s",
		spawnRequests, gate.vPlayerPosition.x, gate.vPlayerPosition.y,
		gate.vPlayerPosition.z, teleportSubmitted ? "submitted" : "not submitted");
	m_strDebugGateStatus = label + summary;
	if (!teleportSubmitted)
	{
		m_strDebugGateStatus += " (" +
			m_PlayerController.Get_DebugPlayerPlacementStatus() + ")";
	}
	outStatus = m_strDebugGateStatus;
	return teleportSubmitted;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_DespawnArenaBosses(std::string& outStatus)
{
	if (Is_DebugGatePending() || m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		outStatus = m_strDebugGateStatus = "Wait for the pending gate request before despawning.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink || 0u == m_iNextDebugGateRequestSequence)
	{
		outStatus = m_strDebugGateStatus =
			"Despawn requires the command sink and an available request sequence.";
		return false;
	}
	const std::uint32_t requestSequence = m_iNextDebugGateRequestSequence;
	if ((std::numeric_limits<std::uint32_t>::max)() == m_iNextDebugGateRequestSequence)
		m_iNextDebugGateRequestSequence = 0u;
	else
		++m_iNextDebugGateRequestSequence;
	if (!m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(requestSequence))
	{
		outStatus = m_strDebugGateStatus = "Despawn command was rejected.";
		return false;
	}
	CCombatHUDViewModel::Get().Clear_BossFocus();
	CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss("", "");
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	outStatus = m_strDebugGateStatus =
		"Despawn of Debug-activated arena bosses submitted; HUD focus and audition target reset.";
	return true;
}
#endif

HRESULT Client::CLevel_KakulSaydonArena::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	float3_t minimum{};
	float3_t maximum{};
	float3_t focus(0.f, 0.f, 0.f);
	f32_t span = 80.f;
	if (m_MapRuntime.Try_Get_PlacementBounds(minimum, maximum))
	{
		focus = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		span = (std::clamp)(
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z),
			40.f,
			5000.f);
	}

	const f32_t distance = (std::max)(40.f, span * 0.7f);
	float3_t initialEye(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	float3_t initialAt = focus;
	LostArk::Shared::S2C_PLAYER_SPAWNED approvedSpawn{};
	if (CNetworkManager::Get().Try_Get_LocalSpawn(approvedSpawn))
	{
		initialEye = float3_t(
			approvedSpawn.fPositionX + 0.4f,
			approvedSpawn.fPositionY + 7.5f,
			approvedSpawn.fPositionZ + 4.5f);
		initialAt = float3_t(
			approvedSpawn.fPositionX,
			approvedSpawn.fPositionY + 1.2f,
			approvedSpawn.fPositionZ);
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = initialEye;
	cameraDesc.vAt = initialAt;
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = g_KakulSaydonFreeCameraSpeed;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = KAKULSAYDON_FOLLOW_POSITION_OFFSET;
	cameraDesc.vLookOffset = KAKULSAYDON_FOLLOW_LOOK_OFFSET;
	cameraDesc.fFollowResponse = 0.f;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			strLayerTag,
			gameObject);
		return E_FAIL;
	}
	return S_OK;
}

bool_t Client::CLevel_KakulSaydonArena::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;
	m_pCameraTarget = localCharacter;
	m_pCamera->Set_PositionOffset(KAKULSAYDON_FOLLOW_POSITION_OFFSET);
	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Load_CameraShots(
	std::string& outStatus)
{
	m_CameraShots.clear();
	Release_CameraShot();

	const std::filesystem::path path = Find_CameraShotDocument();
	std::error_code fileError;
	if (!std::filesystem::is_regular_file(path, fileError) || fileError)
	{
		outStatus = "KoukuSaydon camera shot document is absent; follow view only.";
		return true;
	}
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (fileError || 0u == fileBytes || fileBytes > 256u * 1024u)
	{
		outStatus = "KoukuSaydon camera shot document is empty or exceeds 256 KiB.";
		return false;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaydon camera shot document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaydon camera shot document could not be read completely.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 256u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 4096u;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "areaId", "revision", "shots" }))
	{
		outStatus = "KoukuSaydon camera shot root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* area = Required(root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* shots = Required(root, "shots", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || CAMERA_SHOT_SCHEMA != schema->Get_String() ||
		nullptr == version || version->Get_Number() != 1.0 ||
		nullptr == area || KAKULSAYDON_AREA_ID != area->Get_String() ||
		nullptr == revision || !std::isfinite(revision->Get_Number()) ||
		revision->Get_Number() < 1.0 ||
		std::floor(revision->Get_Number()) != revision->Get_Number() ||
		nullptr == shots || shots->Get_Array().size() > CAMERA_SHOT_MAX_COUNT)
	{
		outStatus = "KoukuSaydon camera shot header is invalid.";
		return false;
	}

	std::vector<KAKUL_CAMERA_SHOT> stagedShots;
	std::unordered_set<std::string> stagedIds;
	stagedShots.reserve(shots->Get_Array().size());
	for (const DATA_JSON_VALUE& value : shots->Get_Array())
	{
		if (!Has_ShotProperties(value,
			{ "shotId", "sequenceInstanceId", "box", "eye", "lookAt",
				"fovYDegrees", "blendInMs", "blendOutMs", "priority" },
			{ "cameraTrack", "follow" }))
		{
			outStatus = "KoukuSaydon camera shot has unexpected properties.";
			return false;
		}
		KAKUL_CAMERA_SHOT shot;
		const DATA_JSON_VALUE* shotId = Required(value, "shotId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* box = Required(value, "box", DATA_JSON_TYPE::OBJECT);
		if (nullptr == shotId || !Is_StableId(shotId->Get_String()) ||
			!stagedIds.emplace(shotId->Get_String()).second ||
			nullptr == box ||
			!Has_ExactProperties(*box, { "center", "halfExtents", "yawDegrees" }))
		{
			outStatus = "KoukuSaydon camera shot identity or box is invalid.";
			return false;
		}
		shot.strShotId = shotId->Get_String();
		const DATA_JSON_VALUE* sequenceId =
			Required(value, "sequenceInstanceId", DATA_JSON_TYPE::STRING);
		if (nullptr == sequenceId ||
			(!sequenceId->Get_String().empty() &&
				!Is_StableId(sequenceId->Get_String())))
		{
			outStatus = "KoukuSaydon camera shot sequence binding is invalid: " +
				shot.strShotId;
			return false;
		}
		shot.strSequenceInstanceId = sequenceId->Get_String();

		const DATA_JSON_VALUE* yaw = Required(*box, "yawDegrees", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* fov = Required(value, "fovYDegrees", DATA_JSON_TYPE::NUMBER);
		if (!Read_Float3(box->Find("center"), CAMERA_SHOT_MAX_COORDINATE, shot.vCenter) ||
			!Read_Float3(box->Find("halfExtents"), CAMERA_SHOT_MAX_HALF_EXTENT,
				shot.vHalfExtents) ||
			shot.vHalfExtents.x <= 0.f || shot.vHalfExtents.y <= 0.f ||
			shot.vHalfExtents.z <= 0.f ||
			nullptr == yaw || !std::isfinite(yaw->Get_Number()) ||
			std::abs(yaw->Get_Number()) > 360.0 ||
			!Read_Float3(value.Find("eye"), CAMERA_SHOT_MAX_COORDINATE, shot.vEye) ||
			!Read_Float3(value.Find("lookAt"), CAMERA_SHOT_MAX_COORDINATE, shot.vLookAt) ||
			nullptr == fov || !std::isfinite(fov->Get_Number()) ||
			fov->Get_Number() <= 1.0 || fov->Get_Number() >= 179.0 ||
			!Read_Uint(value.Find("blendInMs"), CAMERA_SHOT_MAX_BLEND_MS, shot.iBlendInMs) ||
			!Read_Uint(value.Find("blendOutMs"), CAMERA_SHOT_MAX_BLEND_MS, shot.iBlendOutMs) ||
			!Read_Uint(value.Find("priority"), CAMERA_SHOT_MAX_PRIORITY, shot.iPriority))
		{
			outStatus = "KoukuSaydon camera shot values are out of range: " +
				shot.strShotId;
			return false;
		}
		shot.fYawDegrees = static_cast<f32_t>(yaw->Get_Number());
		shot.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
		const DATA_JSON_VALUE* cameraTrack = value.Find("cameraTrack");
		if (nullptr != cameraTrack)
		{
			if (DATA_JSON_TYPE::OBJECT != cameraTrack->Get_Type() ||
				!Read_CameraTrack(*cameraTrack, shot.strShotId,
					shot.CameraTrack, outStatus))
			{
				return false;
			}
			shot.hasCameraTrack = true;
		}
		const DATA_JSON_VALUE* follow = value.Find("follow");
		if (nullptr != follow)
		{
			if (DATA_JSON_TYPE::OBJECT != follow->Get_Type() ||
				!Has_ExactProperties(*follow, { "eyeOffset", "lookAtOffset" }) ||
				!Read_Float3(follow->Find("eyeOffset"),
					CAMERA_SHOT_MAX_COORDINATE, shot.vFollowEyeOffset) ||
				!Read_Float3(follow->Find("lookAtOffset"),
					CAMERA_SHOT_MAX_COORDINATE, shot.vFollowLookAtOffset))
			{
				outStatus = "KoukuSaydon camera shot follow offsets are invalid: " +
					shot.strShotId;
				return false;
			}
			const float3_t followForward(
				shot.vFollowLookAtOffset.x - shot.vFollowEyeOffset.x,
				shot.vFollowLookAtOffset.y - shot.vFollowEyeOffset.y,
				shot.vFollowLookAtOffset.z - shot.vFollowEyeOffset.z);
			if (followForward.x * followForward.x +
				followForward.y * followForward.y +
				followForward.z * followForward.z <= 0.000001f)
			{
				outStatus = "KoukuSaydon camera shot follow offsets coincide: " +
					shot.strShotId;
				return false;
			}
			shot.followsPlayer = true;
		}
		/* A pose whose eye sits on its own target has no direction, and the
		   engine would reject it every frame. Refuse it at load instead. */
		const float3_t forward(
			shot.vLookAt.x - shot.vEye.x,
			shot.vLookAt.y - shot.vEye.y,
			shot.vLookAt.z - shot.vEye.z);
		if (forward.x * forward.x + forward.y * forward.y +
			forward.z * forward.z <= 0.000001f)
		{
			outStatus = "KoukuSaydon camera shot eye and lookAt coincide: " +
				shot.strShotId;
			return false;
		}
		stagedShots.push_back(std::move(shot));
	}

	m_CameraShots = std::move(stagedShots);
	outStatus = "KoukuSaydon camera shots loaded: " +
		std::to_string(m_CameraShots.size());
	return true;
}

const Client::CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT*
Client::CLevel_KakulSaydonArena::Find_ActiveCameraShot(
	const float3_t& vPosition) const
{
	const KAKUL_CAMERA_SHOT* best = nullptr;
	for (const KAKUL_CAMERA_SHOT& shot : m_CameraShots)
	{
		const bool_t isHeldNow = shot.strShotId == m_strActiveCameraShotId;
		bool_t isActive = false;
		if (!shot.strSequenceInstanceId.empty())
		{
			/* The sequence starts the shot on the frame its trigger fires, even
			   though the party is still far from the box. Once the sequence
			   ends the box keeps the framing until they walk on to the next
			   stage, so the camera does not snap back mid scene. */
			isActive = m_SequencePlayer.Is_Playing(shot.strSequenceInstanceId);
			if (!isActive && isHeldNow)
			{
				isActive = Contains_CameraShot(
					shot, vPosition, CAMERA_SHOT_EXIT_MARGIN);
			}
		}
		else
		{
			isActive = Contains_CameraShot(shot, vPosition,
				isHeldNow ? CAMERA_SHOT_EXIT_MARGIN : 0.f);
		}
		if (!isActive)
			continue;
		if (nullptr == best || shot.iPriority > best->iPriority)
			best = &shot;
	}
	return best;
}

void Client::CLevel_KakulSaydonArena::Release_CameraShot()
{
	if (m_bCameraShotHeld && nullptr != m_pCamera)
	{
		m_pCamera->End_PresentationOverride(
			KAKULSAYDON_CAMERA_SHOT_OWNER_ID);
	}
	m_bCameraShotHeld = false;
	m_strActiveCameraShotId.clear();
	m_fCameraBlendSeconds = 0.f;
	m_fCameraBlendElapsed = 0.f;
}

void Client::CLevel_KakulSaydonArena::Update_TriggerMoveFade(
	const f32_t fTimeDelta)
{
	if (nullptr == m_pTriggerMoveFadeView)
		return;

	using LostArk::Shared::PLAYER_ACTION_STATE;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t isMoving = player.isValid &&
		PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction;

	/* Every movePlayer trigger uses TRIGGER_MOVE, hops and stage transition
	   alike, so speed is what tells them apart: a 4-6 m hop stays under
	   15 m/s even with its arc, the 1.2 km transition runs at hundreds. */
	constexpr f32_t TRANSITION_SPEED_METRES_PER_SECOND = 40.f;
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	const shared_ptr<CTransform> transform =
		nullptr != localCharacter ? localCharacter->Get_Transform() : nullptr;
	if (nullptr == transform)
	{
		m_bTriggerMoveFadeHasLastPosition = false;
		m_bTriggerMoveFadeArmed = false;
	}
	else
	{
		float3_t position{};
		XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
		if (m_bTriggerMoveFadeHasLastPosition && isMoving &&
			fTimeDelta > 0.f)
		{
			const f32_t dx = position.x - m_vTriggerMoveFadeLastPosition.x;
			const f32_t dy = position.y - m_vTriggerMoveFadeLastPosition.y;
			const f32_t dz = position.z - m_vTriggerMoveFadeLastPosition.z;
			const f32_t speed =
				std::sqrt(dx * dx + dy * dy + dz * dz) / fTimeDelta;
			if (speed > TRANSITION_SPEED_METRES_PER_SECOND)
				m_bTriggerMoveFadeArmed = true;
		}
		m_vTriggerMoveFadeLastPosition = position;
		m_bTriggerMoveFadeHasLastPosition = true;
	}
	if (!isMoving)
		m_bTriggerMoveFadeArmed = false;

	/* Darkening is near-instant so the first frames of the transition are
	   covered; the arrival is revealed gently instead of snapping. */
	constexpr f32_t DARKEN_SECONDS = 0.08f;
	constexpr f32_t BRIGHTEN_SECONDS = 0.4f;
	const f32_t fStep = m_bTriggerMoveFadeArmed
		? fTimeDelta / DARKEN_SECONDS
		: -fTimeDelta / BRIGHTEN_SECONDS;
	m_fTriggerMoveFadeAlpha =
		std::clamp(m_fTriggerMoveFadeAlpha + fStep, 0.f, 1.f);

	const bool_t bVisible = m_fTriggerMoveFadeAlpha > 0.f;
	m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", bVisible);
	if (!bVisible)
		return;
	/* Set_SlotAlpha would rewrite RGB to white, which is the opposite of a
	   blackout, so the tint is written whole. */
	m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen",
		float4_t(0.f, 0.f, 0.f, m_fTriggerMoveFadeAlpha));
}

void Client::CLevel_KakulSaydonArena::Update_CameraShots(const f32_t fTimeDelta)
{
	if (nullptr == m_pCamera || m_CameraShots.empty())
		return;
	if (!m_pCamera->Is_FollowEnabled())
	{
		/* The free camera owns the view while it is on. */
		Release_CameraShot();
		return;
	}
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	const shared_ptr<CTransform> transform =
		nullptr != localCharacter ? localCharacter->Get_Transform() : nullptr;
	float3_t position{};
	if (nullptr != transform)
		XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
	else if (!m_bCameraShotHeld)
	{
		/* Without a Character there is no follow pose to hand back to, so a
		   shot may only start once the local player exists. */
		return;
	}

	/* The pose the follow camera would hold this frame. This level installs a
	   zero response follow, so it lands exactly here and a released shot hands
	   over without a step. */
	const float3_t followEye(
		position.x + KAKULSAYDON_FOLLOW_POSITION_OFFSET.x,
		position.y + KAKULSAYDON_FOLLOW_POSITION_OFFSET.y,
		position.z + KAKULSAYDON_FOLLOW_POSITION_OFFSET.z);
	const float3_t followLook(
		position.x + KAKULSAYDON_FOLLOW_LOOK_OFFSET.x,
		position.y + KAKULSAYDON_FOLLOW_LOOK_OFFSET.y,
		position.z + KAKULSAYDON_FOLLOW_LOOK_OFFSET.z);

	const KAKUL_CAMERA_SHOT* shot = Find_ActiveCameraShot(position);
	const std::string shotId = nullptr != shot ? shot->strShotId : std::string();
	if (shotId != m_strActiveCameraShotId)
	{
		uint32_t blendMs = 0u;
		if (nullptr != shot)
		{
			blendMs = shot->iBlendInMs;
		}
		else
		{
			const auto previous = std::find_if(
				m_CameraShots.begin(), m_CameraShots.end(),
				[this](const KAKUL_CAMERA_SHOT& value)
				{
					return value.strShotId == m_strActiveCameraShotId;
				});
			blendMs = m_CameraShots.end() != previous ?
				previous->iBlendOutMs : 0u;
		}
		/* Freeze the starting pose once per hand-over. Advancing both the
		   start and the ratio would shorten every blend. */
		if (m_bCameraShotHeld)
		{
			m_vCameraEyeFrom = m_vCameraEyeApplied;
			m_vCameraLookFrom = m_vCameraLookApplied;
			m_fCameraFovFrom = m_fCameraFovApplied;
		}
		else
		{
			m_vCameraEyeFrom = followEye;
			m_vCameraLookFrom = followLook;
			m_fCameraFovFrom = KAKULSAYDON_FOLLOW_FOV_DEGREES;
		}
		m_strActiveCameraShotId = shotId;
		m_fCameraBlendSeconds = static_cast<f32_t>(blendMs) / 1000.f;
		m_fCameraBlendElapsed = 0.f;
	}

	if (nullptr != shot)
	{
		m_vCameraEyeTo = shot->vEye;
		m_vCameraLookTo = shot->vLookAt;
		m_fCameraFovTo = shot->fFovYDegrees;
		if (shot->followsPlayer && nullptr != transform)
		{
			/* The side scrolling stages keep this framing and slide it with
			   the Character, so the backdrop stays behind the run line. */
			m_vCameraEyeTo = float3_t(
				position.x + shot->vFollowEyeOffset.x,
				position.y + shot->vFollowEyeOffset.y,
				position.z + shot->vFollowEyeOffset.z);
			m_vCameraLookTo = float3_t(
				position.x + shot->vFollowLookAtOffset.x,
				position.y + shot->vFollowLookAtOffset.y,
				position.z + shot->vFollowLookAtOffset.z);
		}
		f32_t cueElapsedMs = 0.f;
		VALTAN_CINEMATIC_CAMERA_POSE cuePose{};
		if (shot->hasCameraTrack &&
			!shot->strSequenceInstanceId.empty() &&
			m_SequencePlayer.Try_GetElapsedMs(
				shot->strSequenceInstanceId, cueElapsedMs) &&
			CValtanCinematicCameraController::Sample_Cue(
				shot->CameraTrack, cueElapsedMs / 1000.f, cuePose))
		{
			/* The cue owns the framing for as long as the cutscene runs. The
			   authored single pose stays as the fallback so a rejected sample
			   never leaves the camera holding a stale frame. */
			m_vCameraEyeTo = cuePose.vEye;
			m_vCameraLookTo = cuePose.vLookAt;
			m_fCameraFovTo = cuePose.fFovYDegrees;
		}
	}
	else
	{
		if (!m_bCameraShotHeld)
			return;
		if (nullptr == transform)
		{
			Release_CameraShot();
			return;
		}
		m_vCameraEyeTo = followEye;
		m_vCameraLookTo = followLook;
		m_fCameraFovTo = KAKULSAYDON_FOLLOW_FOV_DEGREES;
	}

	if (!m_bCameraShotHeld)
	{
		if (!m_pCamera->Begin_PresentationOverride(
			KAKULSAYDON_CAMERA_SHOT_OWNER_ID))
		{
			/* A cinematic outranks an authored shot; try again once it ends. */
			m_strActiveCameraShotId.clear();
			return;
		}
		m_bCameraShotHeld = true;
	}

	f32_t ratio = 1.f;
	if (m_fCameraBlendSeconds > 0.f)
	{
		m_fCameraBlendElapsed = (std::min)(
			m_fCameraBlendSeconds,
			m_fCameraBlendElapsed + (std::max)(0.f, fTimeDelta));
		const f32_t linear = m_fCameraBlendElapsed / m_fCameraBlendSeconds;
		ratio = linear * linear * (3.f - 2.f * linear);
	}
	const bool_t isBlendFinished = m_fCameraBlendSeconds <= 0.f ||
		m_fCameraBlendElapsed >= m_fCameraBlendSeconds;
	const float3_t eye = Lerp_Float3(m_vCameraEyeFrom, m_vCameraEyeTo, ratio);
	const float3_t lookAt = Lerp_Float3(m_vCameraLookFrom, m_vCameraLookTo, ratio);
	const f32_t fov = m_fCameraFovFrom +
		(m_fCameraFovTo - m_fCameraFovFrom) * ratio;
	if (!m_pCamera->Apply_PresentationPose(
		KAKULSAYDON_CAMERA_SHOT_OWNER_ID, eye, lookAt, fov))
	{
		/* Ownership was taken or the pose was rejected: fall back rather than
		   hold a stale frame. */
		m_bCameraShotHeld = false;
		Release_CameraShot();
		return;
	}
	m_vCameraEyeApplied = eye;
	m_vCameraLookApplied = lookAt;
	m_fCameraFovApplied = fov;

	/* The hand-back finishes only once the blend has fully played. */
	if (nullptr == shot && isBlendFinished)
		Release_CameraShot();
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Ready_DebugStageEntryTriggers(
	const std::string& areaId)
{
	/* Arena-side entrances only. Every stage also carries its own trigger
	   boxes a kilometre away, and drawing those here would say nothing
	   about where a player is supposed to stand when the arena opens. */
	static constexpr std::string_view STAGE_ENTRY_SUFFIX = "_go";

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
			"[Level_KakulSaydonArena] Debug gameplay document is "
			"unavailable: " + documentPath.string() + "\n").c_str());
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] Debug gameplay document rejected: " +
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
				ETOUI(LEVEL::KAKULSAYDON_ARENA),
				TEXT("Layer_DebugWorldGameplay"),
				static_pointer_cast<CGameObject>(triggerBox));
		}
		staged.clear();
	};

	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		const bool_t isStageEntryTrigger =
			placement.isEnabled &&
			WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind &&
			1u == placement.triggerEvents.size() &&
			WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER ==
				placement.triggerEvents.front().eKind &&
			placement.placementId.ends_with(STAGE_ENTRY_SUFFIX);
		if (!isStageEntryTrigger)
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = placement.placementId;
		desc.position = placement.position;
		desc.halfExtents = placement.halfExtents;
		desc.yawDegrees = placement.yawDegrees;
		desc.isEnabled = true;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			TEXT("Prototype_GameObject_TriggerBox"),
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			TEXT("Layer_DebugWorldGameplay"),
			&desc,
			&gameObject)))
		{
			rollback();
			OutputDebugStringA((
				"[Level_KakulSaydonArena] Debug Trigger Box clone failed: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::KAKULSAYDON_ARENA),
				TEXT("Layer_DebugWorldGameplay"),
				gameObject);
			rollback();
			OutputDebugStringA((
				"[Level_KakulSaydonArena] Debug Trigger Box type mismatch: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		triggerBox->Set_AuthoringVisible(true);
		staged.push_back(std::move(triggerBox));
	}

	m_DebugStageEntryTriggers = std::move(staged);
	OutputDebugStringA((
		"[Level_KakulSaydonArena] Debug stage entry Trigger Boxes ready: " +
		std::to_string(m_DebugStageEntryTriggers.size()) + "\n").c_str());
	return true;
}
#endif

unique_ptr<Client::CLevel_KakulSaydonArena>
Client::CLevel_KakulSaydonArena::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_KakulSaydonArena>(
		new CLevel_KakulSaydonArena(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
