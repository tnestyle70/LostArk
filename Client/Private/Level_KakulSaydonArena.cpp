#include "Level_KakulSaydonArena.h"
#include "WorldSequenceObject.h"
#include "ActorCatalog.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "Npc.h"
#include "Model.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"

#include "Camera_Free.h"
#include "CameraTool.h"
#include "MapTool.h"
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "Profiler.h"
#include "KakulArenaHiddenPlacements.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuMadnessGaugeView.h"
#include "MvpAwardCatalog.h"
#include "MvpResultView.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "ValtanCinematicCameraController.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "ProjectDataRoot.h"
#include "UILayoutRuntime.h"
#include "UIInputRouter.h"
#include "MainApp.h"
#include "HitAreaWire.h"
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
#include <sstream>
#include <iomanip>
#include <limits>
#include <unordered_set>
#include <unordered_map>

namespace
{
	std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> WorldPlacementFromCue(
		const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play)
	{
		if (!play.bHasPlacement) return {};
		return CWorldSequencePlayer::OBJECT_PLACEMENT{
			{play.fWorldPositionX, play.fWorldPositionY, play.fWorldPositionZ},
			{play.fWorldRotationXDegrees, play.fWorldRotationYDegrees, play.fWorldRotationZDegrees},
			{play.fWorldScaleX, play.fWorldScaleY, play.fWorldScaleZ}};
	}

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
	constexpr size_t CAMERA_SHOT_MAX_COUNT = 128u;
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
	/* The framing the telescope owner holds while the card maze runs; it
	   follows the Server role rather than a box or a sequence. */
	constexpr const char* CARD_MAZE_TELESCOPE_SHOT_ID = "cardmaze.telescope";
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
			keyframes->Get_Array().empty() ||
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
			if (!Has_ShotProperties(entry,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }, { "up" }))
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
			if (const auto* up = entry.Find("up"))
			{
				if (!Read_Float3(up, 1.f, keyframe.vUp))
				{ outStatus = "Camera up must be a finite direction: " + shotId; return false; }
				const auto forward = XMVector3Normalize(XMLoadFloat3(&keyframe.vLookAt) - XMLoadFloat3(&keyframe.vEye));
				if (XMVectorGetX(XMVector3LengthSq(XMVector3Cross(XMLoadFloat3(&keyframe.vUp), forward))) < 0.000001f)
				{ outStatus = "Camera up must not be parallel to its view: " + shotId; return false; }
				keyframe.hasUp = true;
			}
			keyframe.iTimeMs = timeMs;
			keyframe.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
			previousTimeMs = timeMs;
			outCue.Keyframes.push_back(std::move(keyframe));
		}
		if (outCue.Keyframes.size() > 1u && outCue.Keyframes.back().iTimeMs != durationMs)
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
	Stop_CompositionCamera(true);
	Clear_EntranceTriggerMarkers();
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	Debug_StopWorldObjectPreview();
	Debug_StopCompositionWorldPreview();
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
	for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(Make_WorldSequenceTargets(), true);
	m_OwnedWorldCues.clear();
	if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(Make_WorldSequenceTargets(), true);
	m_pMarioBombPlayer.reset();
	m_MarioBombEmitters.clear();
	m_SequencePlayer.Clear();
	m_pMapLightAuthoringOverride.reset();
	m_pMapLightPresentation.reset();
	m_DeployRuntime.Clear();
	m_MapRuntime.Clear();
}


bool_t Client::CLevel_KakulSaydonArena::Create_CompositionPreviewActor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::shared_ptr<CNpc>& outActor,
    std::string& status)
{
    CWorldGameplayDocument world;
    if (!world.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
            std::string(KAKULSAYDON_AREA_ID) / "Gameplay.world.json"),
            std::string(KAKULSAYDON_AREA_ID), status)) return false;
    const auto* placement = world.Find(pattern.strTargetBossPlacementId);
    if (!placement || placement->eKind != WORLD_PLACEMENT_KIND::BOSS ||
        CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(placement->placementId) != pattern.strActorProfileId)
    { status = "Bundle preview target/model is unavailable: " + pattern.strTargetBossPlacementId; return false; }
    const auto* actor = CActorCatalog::Find_Boss(placement->archetypeId);
    const auto level = ETOUI(LEVEL::KAKULSAYDON_ARENA);
    if (!actor || FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
        m_pDevice, m_pContext, level, placement->archetypeId)))
    { status = "Bundle preview boss model admission failed: " + placement->archetypeId; return false; }
    CNpc::NPC_DESC desc{};
    desc.iPrototypeLevelIndex = level;
    desc.strModelTag = CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(placement->archetypeId);
    desc.strShaderTag = L"Prototype_Component_Shader_VtxAnimMeshBinary";
    desc.pIdleClip = actor->presentationClips.idle.c_str();
    desc.vPosition = placement->position;
    desc.fYawDegree = placement->yawDegrees;
    // The Server preserves the live facing even when only position is reset.
    // Start preview from that same authoritative pose, rather than an unrelated spawn yaw.
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    m_Replication.Collect_KoukuPresentationViews(bosses, players);
    for (const auto& live : bosses)
    {
        if (live.strArchetypeId != placement->archetypeId || live.iOwnerBossNetEntityId ||
            !live.Snapshot.iCurrentHp) continue;
        desc.fYawDegree = live.Snapshot.fYawDegrees;
        if (!pattern.bResetBossToSpawn)
            desc.vPosition = {live.Snapshot.fPositionX, live.Snapshot.fPositionY, live.Snapshot.fPositionZ};
        break;
    }
    if (pattern.ResetBossYawDegrees) desc.fYawDegree = float(*pattern.ResetBossYawDegrees);
    if (pattern.BossMotion)
    {
        const auto& motion = *pattern.BossMotion;
        desc.vPosition = {float(motion.StartPosition[0]), float(motion.StartPosition[1]), float(motion.StartPosition[2])};
        desc.fYawDegree = float(motion.fYawDegrees);
    }
    desc.bSuppressRootMotion = true;
    desc.strWeaponModelTag = CKoukuSaydonPresentationAssetService::Get_WeaponModelPrototypeTag(placement->archetypeId);
    if (!desc.strWeaponModelTag.empty()) desc.pWeaponSocketBone = CKoukuSaydonPresentationAssetService::Get_WeaponSocketBone();
    std::shared_ptr<CGameObject> object;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(level,
        CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag(), level,
        L"Layer_KoukuCompositionPreview", &desc, &object)))
    { status = "Bundle preview actor clone failed: " + placement->placementId; return false; }
    const auto npc = std::dynamic_pointer_cast<CNpc>(object);
    if (!npc || !npc->Get_Model() || !npc->Get_Transform())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(level, L"Layer_KoukuCompositionPreview", object);
        status = "Bundle preview actor has no model/transform: " + placement->placementId;
        return false;
    }
    npc->Get_Model()->Set_AnimPaused(true);
    outActor = npc;
    return true;
}

void Client::CLevel_KakulSaydonArena::Release_CompositionPreviewActor(const std::shared_ptr<CNpc>& actor)
{
    if (actor) CGameInstance::Get().Remove_GameObject_from_Layer(
        ETOUI(LEVEL::KAKULSAYDON_ARENA), L"Layer_KoukuCompositionPreview", actor);
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Debug_BeginCompositionWorldPreview(
	const std::string& patternId, std::vector<COMPOSITION_WORLD_PREVIEW_CUE> cues,
	std::string& status, const CWorldSequenceDocument* sourceDocument)
{
	if (cues.empty())
	{
		Debug_StopCompositionWorldPreview();
		if (!m_CompositionWorldPreviewDeployStates.empty() || !m_CompositionWorldPreviewArenaVisibility.empty())
		{ status = "Previous WORLD preview state could not be restored."; return false; }
		return true;
	}
	auto targets = Make_WorldSequenceTargets();
	const auto& document = sourceDocument ? *sourceDocument : m_SequencePlayer.Get_Document();
	std::map<std::string, COMPOSITION_WORLD_PREVIEW_PLAYBACK> staged;
	std::vector<CWorldSequencePlayer*> stagedPlayers;
	std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> placementBindings;
	bool previewsResourceBook = false;
	for (const auto& cue : cues)
	{
		const auto* instance = document.Find_Instance(cue.instanceId);
		const char* rejection = nullptr;
		if (nullptr == instance) rejection = "instance is missing from the loaded Area document";
		else if (!instance->enabled) rejection = "instance is disabled";
		else if (0u == cue.durationMs) rejection = "duration is zero";
		else if (!std::isfinite(cue.playbackSpeed) || cue.playbackSpeed <= 0.f)
			rejection = "playback speed must be finite and positive";
		else if (!Is_StableId(cue.occurrenceId)) rejection = "occurrence ID is invalid";
		else if (staged.contains(cue.occurrenceId)) rejection = "occurrence ID is duplicated";
		if (rejection)
		{
			status = "WORLD preview " + cue.occurrenceId + ": " + rejection +
				" [instance=" + cue.instanceId + ", revision=" + std::to_string(document.Get_Revision()) +
				", document=" + (sourceDocument ? "supplied snapshot" : "runtime") + "]";
			return false;
		}
		if (!Can_StartCompositionWorld(cue.instanceId, status, &document)) return false;
		for (const auto& binding : instance->bindings)
		{
			if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
				binding.targetId == "world.object.kouku.popup.book") previewsResourceBook = true;
			if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
				!placementBindings.emplace(binding.targetKind, binding.targetId).second)
			{
				status = "WORLD preview occurrences share a mutable map/deploy target: " + cue.occurrenceId;
				return false;
			}
		}
		auto player = make_unique<CWorldSequencePlayer>();
		stagedPlayers.push_back(player.get());
		staged.emplace(cue.occurrenceId, COMPOSITION_WORLD_PREVIEW_PLAYBACK{cue, std::move(player)});
	}
	if (!CWorldSequencePlayer::Set_DocumentBatch(document, targets, stagedPlayers, status)) return false;
	for (auto& [id, playback] : staged)
	{
		auto& player = *playback.player;
		const auto& cue = playback.cue;
		if (!player.Prepare_InstanceResources(cue.instanceId, targets))
		{
			status = "WORLD preview " + cue.occurrenceId + ": " + player.Get_Status();
			return false;
		}
		if (!player.Validate_ObjectPlacement(cue.instanceId, cue.placement, status)) return false;
	}
	Debug_StopCompositionWorldPreview();
	if (!m_CompositionWorldPreviewDeployStates.empty() || !m_CompositionWorldPreviewArenaVisibility.empty())
	{ status = "Previous WORLD preview state could not be restored."; return false; }

	std::vector<COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE> previousDeployStates;
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> visibleDeployStates;
	bool_t previewsCutsceneSet = false;
	for (const auto& [kind, target] : placementBindings)
	{
		WORLD_SEQUENCE_BINDING binding;
		binding.targetKind = kind;
		binding.targetId = target;
		uint64_t targetId = 0u;
		if (!CWorldSequencePlayer::Try_ParseTargetId(binding, targetId))
		{ status = "WORLD preview target identity is invalid."; return false; }
		if (kind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
		{
			const auto object = m_DeployRuntime.Find(targetId);
			if (!object || object->Is_AnimationAuthoringPreviewActive())
			{ status = "WORLD preview Deploy target is missing or already owned."; return false; }
			previousDeployStates.push_back({targetId, object->Get_State(), DEPLOY_PROP_STATE::INTACT});
			visibleDeployStates.emplace_back(targetId, DEPLOY_PROP_STATE::INTACT);
		}
		else if (kind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
			KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= targetId && targetId < KAKULSAYDON_CUTSCENE_SET_END_ID)
			previewsCutsceneSet = true;
	}
	if (previewsResourceBook)
	{
		if (placementBindings.contains({WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
			std::to_string(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID)}))
		{ status = "WORLD preview cannot own both the restored and legacy popup book."; return false; }
		const auto book = m_DeployRuntime.Find(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID);
		if (book)
		{
			if (book->Is_AnimationAuthoringPreviewActive())
			{ status = "WORLD preview legacy popup book is already owned."; return false; }
			previousDeployStates.push_back({KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID,
				book->Get_State(), DEPLOY_PROP_STATE::DESPAWNED});
			visibleDeployStates.emplace_back(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
		}
	}
	std::vector<std::pair<uint64_t, bool_t>> previousArenaVisibility;
	if (previewsCutsceneSet)
	{
		// Reuse the Level's exclusive unfolded/standing arena membership. The
		// World player reveals its bound copy; only the standing copy is borrowed.
		for (auto& entry : m_MapRuntime.Get_MutablePlacements())
		{
			if (placementBindings.contains({WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT,
				std::to_string(entry.record.placementId)}) ||
				std::find(KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.begin(), KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end(),
					entry.record.placementId) == KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end()) continue;
			bool_t visible = false;
			if (!CMapPlacementRuntime::Try_GetRuntimeVisible(entry, visible))
			{ status = "WORLD preview could not capture standing arena visibility."; return false; }
			previousArenaVisibility.emplace_back(entry.record.placementId, visible);
		}
	}
	m_CompositionWorldPreviewDeployStates = std::move(previousDeployStates);
	m_CompositionWorldPreviewArenaVisibility = std::move(previousArenaVisibility);
	m_bCompositionWorldPreviewStandingArenaVisible = false;
	if (!visibleDeployStates.empty() && !m_DeployRuntime.Set_States(visibleDeployStates))
	{
		status = "WORLD preview could not apply its Deploy states: " + m_DeployRuntime.Get_Status();
		Debug_StopCompositionWorldPreview();
		return false;
	}
	for (const auto& previous : m_CompositionWorldPreviewArenaVisibility)
	{
		auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), previous.first);
		if (!entry || !CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
		{
			status = "WORLD preview could not hide the standing arena.";
			Debug_StopCompositionWorldPreview();
			return false;
		}
	}
	m_CompositionWorldPreviewCues = std::move(staged);
	m_strCompositionWorldPreviewPattern = patternId;
	m_bCompositionWorldPreviewClockBound = false;
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_HasVisibleCompositionWorldBox(const std::string_view occurrenceId) const
{
    const auto found = m_CompositionWorldPreviewCues.find(std::string(occurrenceId));
    float4x4_t pivot;
    return found != m_CompositionWorldPreviewCues.end() &&
        found->second.player->Try_GetObjectPivot(found->second.cue.instanceId, pivot);
}

bool_t Client::CLevel_KakulSaydonArena::Debug_SetCompositionWorldPlacement(
	const std::string& occurrenceId, const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement,
	std::string& status)
{
	const auto found = m_CompositionWorldPreviewCues.find(occurrenceId);
	if (found == m_CompositionWorldPreviewCues.end())
	{ status = "WORLD preview occurrence is unavailable: " + occurrenceId; return false; }
	auto& playback = found->second;
	if (!playback.player->Validate_ObjectPlacement(playback.cue.instanceId, placement, status)) return false;
	if (playback.player->Is_Playing(playback.cue.instanceId) &&
		!playback.player->Set_ObjectPlacement(playback.cue.instanceId, placement, Make_WorldSequenceTargets()))
	{ status = playback.player->Get_Status(); return false; }
	playback.cue.placement = placement;
	return true;
}

void Client::CLevel_KakulSaydonArena::Debug_StopCompositionWorldPreview()
{
	auto targets = Make_WorldSequenceTargets();
	for (auto& [id, playback] : m_CompositionWorldPreviewCues)
		playback.player->Stop_All(targets, true);
	m_CompositionWorldPreviewCues.clear();
	// Release the animation borrow before restoring state: Set_State rejects
	// writes while an animation preview owns the model. Respect later external state.
	for (auto row = m_CompositionWorldPreviewDeployStates.begin(); row != m_CompositionWorldPreviewDeployStates.end();)
	{
		const auto object = m_DeployRuntime.Find(row->placementId);
		if (object && object->Get_State() == row->appliedState &&
			object->Get_State() != row->previousState &&
			!m_DeployRuntime.Set_State(row->placementId, row->previousState))
		{
			OutputDebugStringA(("[KoukuWorldPreview] Deploy restore pending: " + m_DeployRuntime.Get_Status() + "\n").c_str());
			++row;
		}
		else row = m_CompositionWorldPreviewDeployStates.erase(row);
	}
	for (auto row = m_CompositionWorldPreviewArenaVisibility.begin(); row != m_CompositionWorldPreviewArenaVisibility.end();)
	{
		auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), row->first);
		bool_t visible = false;
		if (entry && (!CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, visible) ||
			(visible == m_bCompositionWorldPreviewStandingArenaVisible && visible != row->second &&
				!CMapPlacementRuntime::Set_RuntimeVisible(*entry, row->second))))
		{
			OutputDebugStringA("[KoukuWorldPreview] Standing arena visibility restore pending.\n");
			++row;
		}
		else row = m_CompositionWorldPreviewArenaVisibility.erase(row);
	}
	m_strCompositionWorldPreviewPattern.clear();
	m_bCompositionWorldPreviewClockBound = false;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_SampleCompositionWorldPreview(
	const std::string& patternId, const bool_t playing, const uint32_t clockMs, std::string& status)
{
	status.clear();
	if (m_CompositionWorldPreviewCues.empty()) return true;
	if (!playing || patternId != m_strCompositionWorldPreviewPattern)
	{
		// A pending Animation target admission takes one frame. Wait for that
		// first matching clock; after binding, a changed owner releases WORLD.
		if (m_bCompositionWorldPreviewClockBound) Debug_StopCompositionWorldPreview();
		return true;
	}
	m_bCompositionWorldPreviewClockBound = true;
	auto targets = Make_WorldSequenceTargets();
	bool_t cutsceneMapPending = false;
	for (auto& [id, playback] : m_CompositionWorldPreviewCues)
	{
		const auto& cue = playback.cue;
		auto& player = *playback.player;
		targets.objectEmissionAnchor = cue.emissionAnchor;
		if (!cue.actorProfileId.empty())
			targets.bossAnchor = [&cue](const std::string& archetype, const std::string& bone,
				CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
			{
				ANIMATION_MODEL_TARGET_VIEW view;
				if (archetype != cue.bossArchetypeId ||
					CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(CAnimationTargetService::Resolve_AssetName()) != cue.actorProfileId ||
					!CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::BODY, view))
				{ status = "World Object Boss anchor is waiting for its matching Model View actor: " + archetype; return false; }
				return CWorldSequencePlayer::Resolve_BossBoneAnchor(view.Model, view.BoneRoot, bone, out, status);
			};
		const auto span = player.Get_InstanceElapsedSpanMs(cue.instanceId, cue.playbackSpeed, cue.durationMs);
		const auto* instance = player.Get_Document().Find_Instance(cue.instanceId);
		if (instance && (clockMs < cue.startMs || clockMs - cue.startMs < span))
			for (const auto& binding : instance->bindings)
			{
				uint64_t targetId = 0u;
				if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
					CWorldSequencePlayer::Try_ParseTargetId(binding, targetId) &&
					KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= targetId &&
					targetId < KAKULSAYDON_CUTSCENE_SET_END_ID)
					cutsceneMapPending = true;
			}
		if (clockMs < cue.startMs || clockMs - cue.startMs >= span)
		{
			player.Stop_All(targets, true);
			continue;
		}
		if (!player.Is_Playing(cue.instanceId) &&
			!player.Play(cue.instanceId, targets, cue.playbackSpeed, cue.positionOffset, cue.durationMs, cue.placement))
		{
			status = "WORLD preview failed: " + cue.occurrenceId + ": " + player.Get_Status();
			OutputDebugStringA(("[KoukuWorldPreview] " + status + "\n").c_str());
			Debug_StopCompositionWorldPreview();
			return false;
		}
		if (!player.Seek_InstanceToMs(cue.instanceId,
			static_cast<f32_t>(clockMs - cue.startMs), targets))
		{
			status = "WORLD preview failed: " + cue.occurrenceId + ": " + player.Get_Status();
			OutputDebugStringA(("[KoukuWorldPreview] " + status + "\n").c_str());
			Debug_StopCompositionWorldPreview();
			return false;
		}
	}
	// Finished unfold boxes release their animated copies before the real arena
	// (including its placement lighting) is shown. Scrubbing reverses the swap.
	const bool_t showStandingArena = !cutsceneMapPending;
	if (!m_CompositionWorldPreviewArenaVisibility.empty() &&
		showStandingArena != m_bCompositionWorldPreviewStandingArenaVisible)
	{
		std::vector<MAP_RUNTIME_PLACED_ENTRY*> changed;
		for (const auto& previous : m_CompositionWorldPreviewArenaVisibility)
		{
			auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), previous.first);
			if (!entry || !CMapPlacementRuntime::Set_RuntimeVisible(*entry, showStandingArena))
			{
				for (auto* applied : changed)
					(void)CMapPlacementRuntime::Set_RuntimeVisible(*applied, m_bCompositionWorldPreviewStandingArenaVisible);
				status = "WORLD preview could not switch the unfolded and standing arena.";
				Debug_StopCompositionWorldPreview();
				return false;
			}
			changed.push_back(entry);
		}
		m_bCompositionWorldPreviewStandingArenaVisible = showStandingArena;
	}
	return true;
}
#endif

HRESULT Client::CLevel_KakulSaydonArena::Initialize()
{
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope initializeScope(pProfiler, "Level.Kouku.Initialize");
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::KAKULSAYDON_ARENA);
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.MapPlacementCommit");
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
	}
	/* Loader admits this Area's deploy prototypes before activation. Clone the
	   prepared models here; repeating admission would both stall this frame
	   and reject the duplicate prototype tags. */
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.DeployCommit");
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
	}
	if (!Reload_MapLights())
	{
		m_DeployRuntime.Clear();m_MapRuntime.Clear();return E_FAIL;
	}
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.InitialVisibility");
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

		// Resolve the initial visibility batch once. emplace preserves the same
		// first-match identity as Find_Placement without 483 full vector scans.
		std::unordered_map<uint64_t, MAP_RUNTIME_PLACED_ENTRY*> placementIndex;
		auto& placements = m_MapRuntime.Get_MutablePlacements();
		placementIndex.reserve(placements.size());
		for (auto& entry : placements)
			placementIndex.emplace(entry.record.placementId, &entry);
		const auto FindInitialPlacement = [&placementIndex](const uint64_t placementId)
		{
			const auto found = placementIndex.find(placementId);
			return found != placementIndex.end() ? found->second : nullptr;
		};

		/* The finale reveals each of these on its own keyframe. Hiding them now
		   costs nothing if the sequence never runs, and a placement the runtime
		   cannot address is reported rather than silently left standing. */
		for (const uint64_t placementId : KAKULSAYDON_CIRCUS_FINALE_PLACEMENT_IDS)
		{
			MAP_RUNTIME_PLACED_ENTRY* const entry = FindInitialPlacement(placementId);
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
			MAP_RUNTIME_PLACED_ENTRY* const entry = FindInitialPlacement(placementId);
			if (nullptr == entry ||
				!CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
			{
				OutputDebugStringA((
					"[Level_KakulSaydonArena][Arena] placement not hidden: " +
					std::to_string(placementId) + "\n").c_str());
			}
		}
	}
	/* A missing or rejected sequence document only costs the scripted props
	   their animation. The arena itself, its Server contracts and every other
	   placement stay enterable, so report the loss instead of blocking entry. */
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.WorldSequence.Load");
		if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::KAKULSAYDON_ARENA),
			CWorldSequenceObject::PROTOTYPE_TAG, CWorldSequenceObject::Create(m_pDevice, m_pContext))))
			return E_FAIL;
		auto sequenceTargets = Make_WorldSequenceTargets();
		if (!m_SequencePlayer.Load_PreparedArea(pEntry->pMapAreaId, sequenceTargets))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][WorldSequence] " +
				m_SequencePlayer.Get_Status() + "\n").c_str());
		}
		else
		{
			CProfilerScope prepareScope(pProfiler, "Level.Kouku.JokerCards.Prewarm");
			// The encounter's six normal cards and one joker reuse these hidden clones.
			// Prepare during arena entry, before the first Pattern 13 spawn frame.
			if (!m_SequencePlayer.Prewarm_ObjectInstances("world.object.instance.kouku.card", 6u, sequenceTargets) ||
				!m_SequencePlayer.Prewarm_ObjectInstances("world.object.instance.kouku.joker_card", 1u, sequenceTargets))
				OutputDebugStringA(("[Level_KakulSaydonArena][JokerPrewarm] " +
					m_SequencePlayer.Get_Status() + "\n").c_str());
		}
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
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.SelfMotion.Load");
		if (!m_MapRuntime.Load_SelfMotions(std::string(KAKULSAYDON_AREA_ID)))
		{
			OutputDebugStringA(
				"[Level_KakulSaydonArena] Self-motion document was rejected.\n");
		}
	}

	{
		CProfilerScope scope(pProfiler, "Level.Kouku.UI.Create");
		m_pTriggerMoveFadeView = std::make_unique<CUILayoutRuntime>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
			L"UI/KakulFade/KakulFadeUI.json");
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", false);
		m_pMadnessGaugeView = std::make_unique<CKoukuMadnessGaugeView>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));

		m_pDeadSceneView = std::make_unique<CUILayoutRuntime>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
			L"UI/DeadScene/DeadSceneUI.json");
		m_pDeadSceneView->Set_AllSlotsVisible(false);
	}

	/* Built hidden; only the F1 Developer Tools show it so far. */
	m_pMvpResultView = std::make_unique<CMvpResultView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));

	/* KoukuSaydon's own document rather than the one Valtan drives. Every layer of
	   epicGateCommanderClearSuccess_Set02 animates its position, size and alpha frame by
	   frame, and the light layers are authored white with each Set's colorTransform
	   supplying the raid colour (Kouku pulls blue to 0, which is what makes it gold), so
	   the whole Set is carried as a keyframe document instead of fixed rects.
	   epicgatecommonclear.gfx's MainTimeline places the frame at translateX -6400 twips on
	   a 1920x1080 stage, so local x maps to x - 320 and then the usual 2/3 onto 1280x720;
	   that mapping is already baked into the generated keys. */
	m_pRaidClearView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
		L"UI/RaidClear/RaidClear_Kouku_Layout.json");
	m_pRaidClearView->Set_AllSlotsVisible(false);

	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.Replication.Initialize");
		if (!m_Replication.Initialize(replicationDesc))
			return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_pWorldEntityCommandSink = make_shared<CNetworkWorldEntityCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
#ifdef _DEBUG
	m_PlayerController.Set_DebugMarioJumpEnabled(true);
#endif
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.Controller.Prepare");
		if (!m_PlayerController.Initialize_TargetingPreview(
				ETOUI(LEVEL::KAKULSAYDON_ARENA)) ||
			!m_PlayerController.Initialize_ClickMoveEffect(
				ETOUI(LEVEL::KAKULSAYDON_ARENA)))
		{
			return E_FAIL;
		}
	}

	(void)Load_EntranceTriggerMarkers();

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
	const auto& mapLights=m_pMapLightAuthoringOverride?m_pMapLightAuthoringOverride:m_pMapLightPresentation;
	if(mapLights && !mapLights->Submit_Frame()) OutputDebugStringA((mapLights->Get_Status()+"\n").c_str());
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
#ifdef _DEBUG
	if (m_bDebugGazeView && localCharacter && localCharacter->Get_Transform())
	{
		HIT_AREA_SHAPE cone{};
		cone.iAreaType = 3;
		cone.iAreaRange = static_cast<int32_t>(m_fDebugGazeDistance * 100.f);
		cone.iAreaAngle = static_cast<int32_t>(m_fDebugGazeHalfAngle * 2.f);
		CHitAreaWire::Draw(*localCharacter->Get_Transform()->Get_WorldMatrixPtr(), cone,
			40u | (220u << 8u) | (255u << 16u) | (255u << 24u));
	}
#endif
	Update_DeadScene(fTimeDelta);
	Update_RaidClear(fTimeDelta);
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Update(fTimeDelta);

#ifdef _DEBUG
	/* Gate spawn replies arrive one per requested placement. They are Debug
	   status only; the presentation itself follows the reliable spawn stream. */
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT spawnResult{};
	std::uint64_t spawnRequestToken = 0u;
	while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(spawnResult, &spawnRequestToken))
	{
		const auto pending = m_DebugGatePendingPlacements.find(spawnResult.strPlacementId);
		if (pending == m_DebugGatePendingPlacements.end() ||
			spawnRequestToken == 0u || pending->second != spawnRequestToken)
			continue;
		m_DebugGatePendingPlacements.erase(pending);
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
		if (LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::SPAWNED != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ACTIVATED != spawnResult.eResult)
			m_bDebugGateFailed = true;
		m_strDebugGateStatus += "\n" + spawnResult.strPlacementId + ": " + pResult;
	}
	if (Is_DebugGatePending())
	{
		m_fDebugGatePendingSeconds += fTimeDelta;
		if (m_fDebugGatePendingSeconds >= 15.f)
		{
			Debug_RetireGateActivation(
				"Server Gate activation timed out before all spawn and movement approvals arrived.");
		}
	}
	if (Is_DebugGatePending() && m_DebugGatePendingPlacements.empty() &&
		!m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		if (!m_bDebugGateFailed && m_PlayerController.Did_DebugPlayerPlacementSucceed())
		{
			const KAKUL_DEBUG_GATE& gate = Get_DebugGates()[m_iPendingDebugGate];
			CCombatHUDViewModel::Get().Set_BossFocusArchetype(
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			CCombatHUDViewModel::Get().Set_BossHidden(m_bSequenceCombatPending || nullptr == gate.pHudFocusArchetypeId);
			CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss(
				nullptr != gate.pAuditionPlacementId ? gate.pAuditionPlacementId : "",
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			m_iActiveDebugGate = m_iPendingDebugGate;
			// Commit the gimmick mode only after the Server-approved gate move.
			using LostArk::Shared::KOUKU_HUD_MODE;
			(void)m_PlayerController.Request_DebugKoukuHudMode(
				m_iActiveDebugGate == 3u ? KOUKU_HUD_MODE::MARIO :
				(m_iActiveDebugGate == 7u ? KOUKU_HUD_MODE::MAZE : KOUKU_HUD_MODE::NONE));
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

	auto targets = Make_WorldSequenceTargets();
	const auto& pendingRun = m_Replication.Get_KoukuBundleState();
	if (pendingRun.iRunEpoch > m_iLatestWorldRunEpoch)
	{
		for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(targets, true);
		m_OwnedWorldCues.clear(); m_PendingOwnedWorldCues.clear();
		m_StoppedWorldOwners.clear(); m_ConsumedWorldCueIds.clear();
		m_iLatestWorldRunEpoch = pendingRun.iRunEpoch;
	}
	// Product presentation is prepared by MainApp after the first arena update.
	// Keep reliable cues until the matching pinned revision is ready or their span expires.
	auto pendingWorldCues = std::move(m_PendingOwnedWorldCues);
	m_PendingOwnedWorldCues.clear();
	for (const auto& play : pendingWorldCues) Consume_OwnedWorldCue(play, targets);
	/* The Server decided these started; this level only resolves each stable
	   instance ID against what it loaded and plays the presentation. */
	for (const auto& play : m_Replication.Consume_WorldSequencePlays())
	{
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP_OWNER ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::FINISH_OWNER ||
			play.iRunEpoch != 0u)
		{
			Consume_OwnedWorldCue(play, targets);
			continue;
		}
		const std::string& instanceId = play.strSequenceInstanceId;
#ifdef _DEBUG
		Debug_StopWorldObjectPreview();
		Debug_StopCompositionWorldPreview();
#endif
		std::string status;
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY &&
			!play.strTargetSequenceInstanceId.empty())
		{
			if (!m_SequencePlayer.Apply_ObjectMotion(play.strTargetSequenceInstanceId, instanceId, targets))
				OutputDebugStringA(("[Level_KakulSaydonArena][WorldMotion] " +
					m_SequencePlayer.Get_Status() + "\n").c_str());
			continue;
		}
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::REPLAY ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP)
		{
			m_SequencePlayer.Stop_Instance(instanceId, targets, true);
			for (const auto& link : KAKULSAYDON_PAPER_BRIDGE_LINKS)
			{
				if (link.bridgeSequenceInstanceId != instanceId) continue;
				m_SequencePlayer.Stop_Instance(std::string(link.leverSequenceInstanceId), targets, true);
				m_RaisedPaperBridges.erase(link.bridgePlacementId);
				m_DeployRuntime.Set_State(link.bridgePlacementId, DEPLOY_PROP_STATE::DESPAWNED);
			}
			if (instanceId == KAKULSAYDON_CUTSCENE_SEQUENCE_ID)
			{
				for (const auto& instance : m_SequencePlayer.Get_Document().Get_Instances())
					if (instance.instanceId.starts_with(KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX))
						m_SequencePlayer.Stop_Instance(instance.instanceId, targets, true);
				m_bCutsceneBossVisible = false;
				Apply_CutsceneSetVisible(false);
				m_DeployRuntime.Set_State(KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
			}
			const auto activeShot = std::find_if(m_CameraShots.begin(), m_CameraShots.end(),
				[&](const KAKUL_CAMERA_SHOT& shot) { return shot.strShotId == m_strActiveCameraShotId &&
					shot.strSequenceInstanceId == instanceId; });
			if (activeShot != m_CameraShots.end()) Release_CameraShot();
			if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP) continue;
		}
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::REPLAY)
			Retire_EntranceTriggerMarker(instanceId);
		if (!Start_ServerRequestedSequence(instanceId, play.fPlaybackSpeed,
			float3_t(play.fPositionOffsetX, play.fPositionOffsetY, play.fPositionOffsetZ), targets, status, play.iDurationMs,
			WorldPlacementFromCue(play)))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][WorldSequence] " + instanceId +
				": " + status + "\n").c_str());
		}
	}
	// Persistent terminal state also closes owners for reconnect/late packet ordering.
	const auto& runState = m_Replication.Get_KoukuBundleState();
	using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
	const auto stopOwner = [&](const std::string& memberId, const RUN_STATE state)
	{
		LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY stop;
		stop.eOperation = state == RUN_STATE::COMPLETED ? LostArk::Shared::WORLD_SEQUENCE_OPERATION::FINISH_OWNER :
			LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP_OWNER;
		stop.iRunEpoch = runState.iRunEpoch; stop.strMemberId = memberId;
		Consume_OwnedWorldCue(stop, targets);
	};
	if (runState.iRunEpoch)
	{
		if (runState.eState == RUN_STATE::COMPLETED || runState.eState == RUN_STATE::ABORTED) stopOwner({}, runState.eState);
		else for (const auto& member : runState.Members)
			if (member.eState == RUN_STATE::COMPLETED || member.eState == RUN_STATE::ABORTED) stopOwner(member.strMemberId, member.eState);
	}
	m_SequencePlayer.Update(fTimeDelta, targets);
	Update_CardMazePresentation(fTimeDelta);
	Update_MarioBallBouncePresentation(fTimeDelta);
	Update_MarioBallPresentation(fTimeDelta);
	Update_MarioBombPresentation(fTimeDelta);
	if (m_bWorldObjectReloadPending && !m_SequencePlayer.Has_ActiveInstances())
	{
		std::string status;
		if (!Reload_WorldObjectRuntime(status))
			OutputDebugStringA(("[WorldObjectReload] " + status + "\n").c_str());
	}
	for (auto cue = m_OwnedWorldCues.begin(); cue != m_OwnedWorldCues.end();)
	{
		float seconds = 0.f;
		auto& value = cue->second;
		if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			m_Replication.Get_LastServerTick(), value.startTick, 30.f, seconds))
			value.clockMs = (std::max)(seconds * 1000.f, value.clockMs + fTimeDelta * 1000.f);
		else value.clockMs += fTimeDelta * 1000.f;
		auto cueTargets = targets;
		cueTargets.objectEmissionAnchor = value.emissionAnchor;
		(void)value.player->Seek_InstanceToMs(value.sequenceId, value.clockMs, cueTargets);
		value.player->Update(0.f, cueTargets);
		if (!value.player->Has_ActiveInstances()) cue = m_OwnedWorldCues.erase(cue);
		else ++cue;
	}
	Update_CutsceneBossRetire(targets);
	Update_CompositionCamera(fTimeDelta);
	Update_CameraShots(fTimeDelta);
	// Consume this frame's Server-started camera sequence before accepting input.
	// A completed shot may keep following the player and must not block controls.
	const bool_t isCameraTrackPlaying = std::any_of(
		m_CameraShots.begin(), m_CameraShots.end(),
		[this](const KAKUL_CAMERA_SHOT& shot)
		{
			return shot.hasCameraTrack && !shot.strSequenceInstanceId.empty() &&
				m_SequencePlayer.Is_Playing(shot.strSequenceInstanceId);
		});
	bool_t sequenceInputReady = true;
#ifdef _DEBUG
	sequenceInputReady = !m_bSequenceCombatPending && !Is_DebugGatePending();
#endif
	m_PlayerController.Update(
		sequenceInputReady && nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() && !isCameraTrackPlaying,
		sequenceInputReady && nullptr != m_pCamera && !m_pCamera->Is_FollowRequested() &&
		!m_pCamera->Is_PresentationOverrideActive());
	Update_TriggerMoveFade(fTimeDelta);
	Update_EntranceTriggerMarkers(fTimeDelta);
	m_MapRuntime.Update_SelfMotions(fTimeDelta);
	if (nullptr != m_pMadnessGaugeView)
	{
		m_pMadnessGaugeView->Update(fTimeDelta, localCharacter,
			CCombatHUDViewModel::Get().Get_KoukuGimmick());
	}
	Update_StatusEffectText(fTimeDelta);
}

void Client::CLevel_KakulSaydonArena::Update_StatusEffectText(const f32_t fTimeDelta)
{
	/* "gongpo" (fear). The word and its colour are retail data, not a code
	   decision: EFTable_GameMsg tip.name.skillbuffdmgfont_<buffId> spells it and
	   EFTable_SkillBuff.FontColor gives 0x8041D9 on all 66 fear buff rows that show
	   one. Those rows also carry FontShow 1, which is the movie motion the view
	   draws. Written with universal character names so this file keeps the
	   ASCII bytes its codepage needs, exactly like the card maze suit names below. */
	static const std::wstring FEAR_WORD = L"\uACF5\uD3EC";
	constexpr std::uint32_t FEAR_COLOR_RGB = 0x8041D9u;

	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
	Collect_KoukuPresentationViews(bosses, players);
	for (const KOUKU_CARD_PRESENTATION_VIEW& view : players)
	{
		if (LostArk::Shared::PLAYER_ACTION_STATE::FEAR != view.Snapshot.eAction ||
			0u == view.Snapshot.iCurrentHp || 0u == view.Snapshot.iActionStartTick)
		{
			continue;
		}
		CStatusEffectTextView::REQUEST request{};
		request.iOwnerEntityId = view.Snapshot.iNetEntityId;
		/* The Server owns the window, so its start tick is the occurrence: one
		   word per FEAR, and a second FEAR pops a second word. */
		request.iOccurrenceKey = view.Snapshot.iActionStartTick;
		request.strWord = FEAR_WORD;
		request.iColorRgb = FEAR_COLOR_RGB;
		request.pAnchor = view.pCharacter;
		m_StatusEffectTextView.Submit(request);
	}

#ifdef _DEBUG
	/* F1 preview: the same word over the local character with no Server truth,
	   keyed by the button's own serial so repeated presses keep firing. */
	const std::uint32_t previewSerial =
		CCombatHUDViewModel::Get().Get_StatusEffectTextPreviewSerial();
	const auto previewAnchor = m_Replication.Get_LocalCharacter();
	/* Only consume the serial once there is a character to hang the word on, so
	   a press made before the local character is up is not swallowed. */
	if (previewSerial != m_iStatusEffectTextPreviewSerial && nullptr != previewAnchor)
	{
		m_iStatusEffectTextPreviewSerial = previewSerial;
		CStatusEffectTextView::REQUEST request{};
		request.iOwnerEntityId = 0u;
		request.iOccurrenceKey = previewSerial;
		request.strWord = FEAR_WORD;
		request.iColorRgb = FEAR_COLOR_RGB;
		request.pAnchor = previewAnchor;
		m_StatusEffectTextView.Submit(request);
	}
#endif

	m_StatusEffectTextView.Update(fTimeDelta);
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
	// Older documents borrow Deploy 7. Recovered documents clone the same
    // book through an explicit material-bound World Object; keep only its owner visible.
    bool borrowsBook = false;
    for (const auto& instance : m_SequencePlayer.Get_Document().Get_Instances())
        if (instance.instanceId.starts_with(KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX))
            for (const auto& binding : instance.bindings)
                if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT &&
                    binding.targetId == std::to_string(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID)) borrowsBook = true;
	if (!targets.pDeployRuntime->Set_State(
		KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, borrowsBook ? DEPLOY_PROP_STATE::INTACT : DEPLOY_PROP_STATE::DESPAWNED))
	{
		outStatus = "Cutscene book could not be revealed: " +
			targets.pDeployRuntime->Get_Status();
		return false;
	}
	Apply_CutsceneSetVisible(true);
	if (!targets.pDeployRuntime->Set_State(KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::INTACT))
	{
		Apply_CutsceneSetVisible(false);
		outStatus = "Cutscene boss could not be revealed: " + targets.pDeployRuntime->Get_Status();
		return false;
	}

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
	const std::string& instanceId, const f32_t playbackSpeed, const float3_t& positionOffset,
	const CWorldSequencePlayer::TARGET_SET& targets,
	std::string& outStatus, const uint32_t durationMs,
	const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement)
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

	if (!m_SequencePlayer.Play(instanceId, targets, playbackSpeed, positionOffset, durationMs, placement))
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

	auto targets = Make_WorldSequenceTargets();

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

void Client::CLevel_KakulSaydonArena::Update_DeadScene(
	const f32_t fTimeDelta)
{
	if (nullptr == m_pDeadSceneView)
		return;

	m_pDeadSceneView->Update(fTimeDelta);

	using LostArk::Shared::PLAYER_ACTION_STATE;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t isDead = player.isValid &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;

	/* Real Render_DeadScene's own whole-screen AddRectFilled(IM_COL32(0,0,0,160)), now a real
	slot (DeadScene_Dim, White1x1 tinted) instead of a raw ImGui draw call. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Dim", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_PanelBg", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_WingedArch", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Effect", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveButton", isDead);
	/* Spectate is not wired to any server/client command yet -- these two slots exist only
	so the button and its border can be positioned in the HUD Layout Tool. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateButton", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateBorder", isDead);
	/* Tool-authoring placeholders only (mark where RenderDeadSceneText's labels land) --
	never shown in real gameplay, regardless of death state. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_TitleTextMarker", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveMessageMarker", false);
	if (!isDead)
	{
		CCombatHUDViewModel::Get().Set_DeadSceneTextRects({});
		return;
	}

	/* RenderDeadSceneText() (CMainApp, after EndFrame()) has no access to this Level's
	m_pDeadSceneView -- push the live, Tool-editable rects through the same Level -> ViewModel ->
	UI path the rest of the combat HUD uses instead of hand-copying these numbers into
	MainApp.cpp, which is exactly what went stale and made the title/button text drift off after
	the panel was repositioned in the Tool. The "부활"/"관전하기" labels are drawn ON their own
	buttons, so those two read the button slots' own rects directly -- DeadScene_ReviveMessageMarker
	is a separate free-standing box above the revive button, unrelated to that label. */
	{
		HUD_DEADSCENE_TEXT_RECTS textRects;
		textRects.isValid =
			m_pDeadSceneView->Get_SlotRect("DeadScene_TitleTextMarker",
				textRects.fTitleX, textRects.fTitleY,
				textRects.fTitleWidth, textRects.fTitleHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_ReviveButton",
				textRects.fReviveTextX, textRects.fReviveTextY,
				textRects.fReviveTextWidth, textRects.fReviveTextHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_SpectateButton",
				textRects.fSpectateX, textRects.fSpectateY,
				textRects.fSpectateWidth, textRects.fSpectateHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_ReviveMessageMarker",
				textRects.fMessageX, textRects.fMessageY,
				textRects.fMessageWidth, textRects.fMessageHeight);
		CCombatHUDViewModel::Get().Set_DeadSceneTextRects(textRects);
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pDeadSceneView->Get_SlotRect(
		"DeadScene_ReviveButton", fX, fY, fWidth, fHeight))
	{
		return;
	}
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fResolutionWidth = m_pDeadSceneView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pDeadSceneView->Get_ResolutionHeight();
	if (Router.Is_Hovered(fX, fY, fWidth, fHeight, fResolutionWidth, fResolutionHeight))
	{
		Router.Claim_Mouse_This_Frame();
		if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fResolutionWidth, fResolutionHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_PlayerController.Request_Revive();
		}
	}
}

HRESULT Client::CLevel_KakulSaydonArena::Render()
{
	const HRESULT drawn = __super::Render();
	if (FAILED(drawn))
		return drawn;
#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(
		TEXT("KoukuSaydon arena loading complete"));
#endif
	/* Drawn last so it sits over the scene. The text only reports what the
	   Server is offering -- pressing the shown key submits a command and the Server
	   decides, so nothing here can move the player by itself. */
	const std::string& offered =
		CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
	if (!offered.empty())
	{
		/* ASCII only: this file carries no other non-ASCII byte and has no BOM,
		   so a UTF-8 Korean literal here is read back in the system codepage. */
		const tchar_t* const PROMPT = 0u != CCombatHUDViewModel::Get().Get_Player().iMarioStage
			? TEXT("[ Up ]") : TEXT("[ G ]");
		const float2_t size = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), PROMPT);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), PROMPT,
			float2_t(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.62f),
			Colors::White, 0.f, float2_t(size.x * 0.5f, size.y * 0.5f), 1.f);
	}
	/* Card maze: the suit this player hunts and the count, or the telescope
	   role. ASCII for the same codepage reason as the prompt above. */
	const HUD_KOUKU_GIMMICK_STATE& maze = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	if (LostArk::Shared::CARD_MAZE_ROLE::NONE != maze.eCardMazeRole ||
		CCombatHUDViewModel::Get().Get_Player().eKoukuHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE)
	{
		std::wstring text;
		if (LostArk::Shared::CARD_MAZE_ROLE::NONE == maze.eCardMazeRole)
			text = L"[ Q ] Strike the telescope at the maze center (G is not used)";
		if (maze.CardMaze.flags & 1u) text = L"[ TELESCOPE ON ]";
		else if (maze.CardMaze.flags & 2u) text = L"[ ESCAPED / HAMMER TELESCOPE TO VIEW ]";
		/* The Debug solo owner hunts as well, so both parts can show at once.
		   The suit name is Korean, written with universal character names so this
		   file keeps the ASCII bytes its codepage needs; the YoonGasiIIM sprite
		   font carries every Hangul syllable used here. */
		if (LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE != maze.eCardMazeSuit)
		{
			if (!text.empty())
				text += L" ";
			switch (maze.eCardMazeSuit)
			{
			/* hateu */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART:
				text += L"\uD558\uD2B8"; break;
			/* seupeideu */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE:
				text += L"\uC2A4\uD398\uC774\uB4DC"; break;
			/* keullobeo */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB:
				text += L"\uD074\uB85C\uBC84"; break;
			/* daia */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND:
				text += L"\uB2E4\uC774\uC544"; break;
			/* munyang */
			default:
				text += L"\uBB38\uC591"; break;
			}
			/* "<suit> jogak x N": the shards this hunter has collected. */
			text += L" \uC870\uAC01 x " + std::to_wstring(maze.iCardMazeKills);
		}
		if (maze.CardMaze.flags & 4u)
			text += L" EXIT (" + std::to_wstring(static_cast<int>(maze.CardMaze.exitX)) + L", " +
				std::to_wstring(static_cast<int>(maze.CardMaze.exitZ)) + L")";
		const float2_t mazeSize = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), text.c_str());
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), text.c_str(),
			float2_t(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.68f),
			Colors::White, 0.f, float2_t(mazeSize.x * 0.5f, mazeSize.y * 0.5f), 1.f);
	}
	/* Mario: a colour's curse lifts when its last source ball pops. Korean by
	   universal character names for the same codepage reason as above. */
	if (m_iMarioCurseNoticeColor >= 0 && m_iMarioCurseNoticeColor < 3)
	{
		/* "<ppalgan|paran|noran> inhyeong-ui jeoju haeje": the red/blue/yellow
		   doll's curse is released. */
		static constexpr const tchar_t* NOTICES[3] = {
			TEXT("\uBE68\uAC04 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C"),
			TEXT("\uD30C\uB780 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C"),
			TEXT("\uB178\uB780 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C") };
		const tchar_t* const notice = NOTICES[m_iMarioCurseNoticeColor];
		const vector_t tint = 0 == m_iMarioCurseNoticeColor ? Colors::Red :
			1 == m_iMarioCurseNoticeColor ? Colors::DeepSkyBlue : Colors::Gold;
		const float2_t noticeSize = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), notice);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), notice,
			float2_t(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f),
			tint, 0.f, float2_t(noticeSize.x * 0.5f, noticeSize.y * 0.5f), 2.f);
	}
	/* Floating status words last, over the scene and over the two prompts above,
	   the way the retail damage-text canvas sits on its own top layer. */
	m_StatusEffectTextView.Render();
	/* Award page labels sit over everything else this Level draws, the status
	   words included. Its own image layers are CUI_Sprite objects on Layer_UI,
	   so they need no call. */
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Render();
	return drawn;
}

namespace
{
	/* EFTable_Mvp.StatType, for the contributions KoukuSaydon's group tracks. */
	constexpr int32_t MVP_STAT_DAMAGE = 1;
	constexpr int32_t MVP_STAT_STAGGER = 3;
	constexpr int32_t MVP_STAT_HEAL = 4;
	constexpr int32_t MVP_STAT_BATTLE_ITEM = 9;
	constexpr int32_t MVP_STAT_COUNTER = 11;
	constexpr int32_t MVP_STAT_SUPPORT_DAMAGE = 13;

	/* KoukuSaydon is a four-player raid -- the award page seats one MVP and three
	   party columns -- so the four-player cutoffs apply. */
	constexpr int32_t MVP_PARTY_SIZE = 4;

	/* EFTable_ZoneEpicGate.GroupId for KoukuSaydon; Valtan is 101, and the
	   headline follows whichever raid is handed in. SecondaryKey 0 on that row is
	   the normal difficulty, 2 the hard one. */
	constexpr int32_t KOUKU_RAID_GROUP_ID = 103;
	constexpr const char* KOUKU_DIFFICULTY_ID = "normal";

	/* The reference capture shows no guild line under any of the four names:
	   MvpResultFrame fills guildNameTF only when the character has a guild, so
	   the sample leaves it empty instead of printing a stand-in word. */
	const wstring_t PREVIEW_GUILD;

	Client::MVP_AWARD_PARTICIPANT Make_PreviewParticipant(
		const wchar_t* const pName,
		vector<Client::MVP_AWARD_CONTRIBUTION> Contributions,
		vector<int32_t> Medals)
	{
		Client::MVP_AWARD_PARTICIPANT Participant;
		Participant.strCharacterName = pName;
		Participant.strGuildName = PREVIEW_GUILD;
		Participant.Contributions = std::move(Contributions);
		Participant.Medals = std::move(Medals);
		for (const Client::MVP_AWARD_CONTRIBUTION& Contribution
			: Participant.Contributions)
			Participant.fTotalScore += Contribution.fScore;
		return Participant;
	}

	/* Sample page for the F1 preview.

	   The shares, scores and medal requests below are made-up sample play. Who
	   ends up as the MVP, which rows each card gets, which title each row shows
	   and which medals survive are all decided by CMvpAwardCatalog from
	   Data/UI/MVP/MvpAwards.json -- nothing here states a title.

	   The sample deliberately gives two of the three columns \uC900 \uD53C\uD574 as their
	   best contribution so the one-damage-title-per-page rule is visible:
	   Berserker takes it and Sorceress falls through to \uBC30\uD2C0\uC544\uC774\uD15C. Medal 16 is
	   requested and dropped, because group 220000 cannot award it. */
	Client::MVP_RESULT_DATA Build_MvpResultPreviewData(const int32_t iGate)
	{
		const vector<Client::MVP_AWARD_PARTICIPANT> Participants = {
			Make_PreviewParticipant(L"Test",
				{ { MVP_STAT_DAMAGE, 4250.f, 42.5f, L"42.5%" },
				  { MVP_STAT_STAGGER, 1655.f, 33.1f, L"33.1%" },
				  { MVP_STAT_COUNTER, 248.f, 24.8f, L"11" } },
				{ 1, 9, 13 }),
			Make_PreviewParticipant(L"Berserker",
				{ { MVP_STAT_DAMAGE, 2830.f, 28.3f, {} },
				  { MVP_STAT_STAGGER, 1530.f, 30.6f, {} } },
				{ 2, 9 }),
			Make_PreviewParticipant(L"Bard",
				{ { MVP_STAT_SUPPORT_DAMAGE, 2260.f, 22.6f, {} },
				  { MVP_STAT_HEAL, 1230.f, 41.0f, {} } },
				{ 14, 16, 17 }),
			Make_PreviewParticipant(L"Sorceress",
				{ { MVP_STAT_DAMAGE, 1520.f, 15.2f, {} },
				  { MVP_STAT_BATTLE_ITEM, 210.f, 21.0f, {} } },
				{ 5 }),
		};

		/* The headline is not a written-out string any more: the difficulty, the
		   raid name and the gate come out of MvpContentNames.json with their own
		   colours, so a different gate or a different raid reads correctly
		   without touching this. */
		const Client::CMvpAwardCatalog& Awards = Client::CMvpAwardCatalog::Get();
		return Awards.Compose_Page(
			Awards.Build_ContentName(
				KOUKU_RAID_GROUP_ID, iGate, KOUKU_DIFFICULTY_ID),
			Participants, MVP_PARTY_SIZE);
	}
}

namespace
{
	/* epicgatecommonclear.gfx runs at 40fps and every Set variant is 309 frames, so elapsed
	   seconds * 40 is the Set's own current frame and the keyframe document plays on the
	   same clock. EpicGateCommonClearFrame picks its variant by an integer the client hands
	   it -- result_<ClearNoticeImage> -- and EFTable_ZoneEpicGate gives KoukuSaydon 103,
	   which is epicGateCommanderClearSuccess_Set02, the Set this document was built from. */
	constexpr f32_t CLEAR_FPS = 40.f;
	/* The Set sprite is authored 309 frames and fades itself out over 300..308, but retail
	   never gets there: in the reference capture the clear screen is still at full strength
	   when it is cut outright, and the award page starts in the same instant. Anchoring the
	   capture to the document (its light enters at f108, crest f118, caption f126) puts
	   document frame 1 at capture frame 1038.5 and the cut at capture 1401, i.e. f243.
	   Document-to-document sequencing lives in the client's C++ and is not in the .gfx, so
	   this one number is measured rather than extracted. */
	constexpr f32_t CLEAR_END_FRAME = 243.f;

	/* Layer entry, position, size, alpha and tint all live in the keyframe document now,
	   so nothing is listed here. The one thing the document cannot carry is the caption:
	   the clear title is a DefineEditText (char 364, 66pt, scale 1.3 settling to 1.0 over
	   frames 126..136) and text is drawn by CMainApp::RenderRaidClearText from a rect.
	   HUD_RAIDCLEAR_TEXT_RECTS has no alpha field, so the caption is gated on at the frame
	   its own alphaMultTerm leaves 0; its 126..136 scale-in is not reproduced yet. */
	constexpr f32_t CLEAR_CAPTION_IN_FRAME = 126.f;
}

int32_t Client::CLevel_KakulSaydonArena::Current_GateNumber() const
{
	return (NO_ACTIVE_DEBUG_GATE == m_iActiveDebugGate)
		? 1 : static_cast<int32_t>(m_iActiveDebugGate) + 1;
}

void Client::CLevel_KakulSaydonArena::Update_RaidClear(const f32_t fTimeDelta)
{
	if (nullptr == m_pRaidClearView || m_fRaidClearElapsedSeconds < 0.f)
		return;

	const f32_t fPrevious = m_fRaidClearElapsedSeconds;
	m_fRaidClearElapsedSeconds += fTimeDelta;
	const f32_t fFrame = m_fRaidClearElapsedSeconds * CLEAR_FPS;
	const bool_t isShowing = fFrame < CLEAR_END_FRAME;

	if (0.f == fPrevious)
	{
		m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_Frame", true);
		m_pRaidClearView->Play_KeyframeAnimation("RaidClear_Kouku_Frame", "intro");
	}
	m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_Frame", isShowing);
	/* Authoring-only marker; the caption itself is drawn from the text pass. */
	m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_TitleTextBox", false);
	m_pRaidClearView->Update(fTimeDelta);

	/* The light enters first, the crest lands on it, the caption follows. */
	HUD_RAIDCLEAR_TEXT_RECTS TextRects;
	TextRects.isValid = isShowing &&
		fFrame >= CLEAR_CAPTION_IN_FRAME &&
		m_pRaidClearView->Get_SlotRect("RaidClear_Kouku_TitleTextBox",
			TextRects.fTitleX, TextRects.fTitleY,
			TextRects.fTitleWidth, TextRects.fTitleHeight);
	CCombatHUDViewModel::Get().Set_RaidClearTextRects(TextRects);

	/* callbackFrameActionEnd: the document hides itself at its last frame and hands the
	   screen to whatever comes next. */
	if (fPrevious * CLEAR_FPS < CLEAR_END_FRAME && fFrame >= CLEAR_END_FRAME)
	{
		m_pRaidClearView->Set_AllSlotsVisible(false);
		if (nullptr != m_pMvpResultView)
			m_pMvpResultView->Show(
				Build_MvpResultPreviewData(Current_GateNumber()));
	}
}

void Client::CLevel_KakulSaydonArena::Debug_Play_ClearThenMvp()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Hide();
	m_fRaidClearElapsedSeconds = 0.f;
}

void Client::CLevel_KakulSaydonArena::Debug_Show_MvpResult()
{
	if (nullptr == m_pMvpResultView)
		return;
	m_pMvpResultView->Show(Build_MvpResultPreviewData(Current_GateNumber()));
}

void Client::CLevel_KakulSaydonArena::Debug_Hide_MvpResult()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Hide();
	m_fRaidClearElapsedSeconds = -1.f;
	if (nullptr != m_pRaidClearView)
		m_pRaidClearView->Set_AllSlotsVisible(false);
}

bool_t Client::CLevel_KakulSaydonArena::Debug_Is_MvpResultVisible() const
{
	return nullptr != m_pMvpResultView && m_pMvpResultView->Is_Visible();
}

bool_t Client::CLevel_KakulSaydonArena::Load_StageMarkers(
	std::string& outStatus)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.StageMarkers.Load");
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

bool_t Client::CLevel_KakulSaydonArena::Try_Get_AuthoringForwardPlacement(
	float3_t& outPosition, std::string& outStatus) const
{
	const auto character = m_Replication.Get_LocalCharacter();
	const auto transform = character ? character->Get_Transform() : nullptr;
	if (!transform) { outStatus = "WORLD placement requires the replicated local player."; return false; }
	const vector_t origin = transform->Get_State(STATE::POSITION);
	vector_t forward = XMVectorSetW(XMVectorSetY(transform->Get_State(STATE::LOOK), 0.f), 0.f);
	const float lengthSquared = XMVectorGetX(XMVector3LengthSq(forward));
	if (!std::isfinite(lengthSquared) || lengthSquared <= .000001f)
	{ outStatus = "The local player has no finite horizontal facing for WORLD placement."; return false; }
	forward = XMVector3Normalize(forward);
	float3_t candidate;
	XMStoreFloat3(&candidate, origin + forward * 3.25f);
	if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y) || !std::isfinite(candidate.z))
	{ outStatus = "The local player's WORLD placement is not finite."; return false; }
	float3_t sampled;
	if (character->Try_SampleTargetGround(candidate.x, candidate.z, sampled) &&
		std::isfinite(sampled.x) && std::isfinite(sampled.y) && std::isfinite(sampled.z))
		candidate = sampled;
	outPosition = candidate;
	outStatus = "Placed ahead of the current player; the saved world position remains fixed.";
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
			float3_t(-2.45f, 1.32f, 945.17f),
			"BOSS_KAKULSAYDON_G1_SAYDON", "boss.kakulsaydon.g1.saydon", nullptr },
		// 2관문 - 대형 세이튼, 쿠크 (HUD and audition follow Kouku)
		KAKUL_DEBUG_GATE{ "2" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEB\x8C\x80\xED\x98\x95" " " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC" ", " "\xEC\xBF\xA0\xED\x81\xAC",
			{ { "boss.kakulsaydon.g2.big-saydon", "boss.kakulsaydon.g2.kouku" } },
			float3_t(3.38f, 10.56f, 323.92f),
			"BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", nullptr },
		// 3관문 - 세이튼
		KAKUL_DEBUG_GATE{ "3" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.g3.saydon", nullptr } },
			float3_t(-2.45f, 1.32f, 945.17f),
			"BOSS_KAKULSAYDON_G3_SAYDON", "boss.kakulsaydon.g3.saydon", nullptr },
		// 1마리오 - player only
		KAKUL_DEBUG_GATE{ "1" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4" " (" "\xED\x94\x8C\xEB\xA0\x88\xEC\x9D\xB4\xEC\x96\xB4\xEB\xA7\x8C" ")",
			{ { nullptr, nullptr } },
			float3_t(-1150.f, -11.52f, -909.28f),
			nullptr, nullptr, nullptr },
		// Mario2/3/4_go destinations use their published detail navigation grids.
		KAKUL_DEBUG_GATE{ "2" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1434.48999f, -9.02000999f, -1175.96997f), nullptr, nullptr, nullptr },
		KAKUL_DEBUG_GATE{ "3" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1889.68994f, -11.5299997f, -1646.20996f), nullptr, nullptr, nullptr },
		KAKUL_DEBUG_GATE{ "4" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1632.57f, -20.49f, -1400.92f), nullptr, nullptr, nullptr },
		// Card maze: main's admitted Debug entry destination.
		KAKUL_DEBUG_GATE{ "\xEC\xB9\xB4\xEB\x93\x9C\xEB\xAF\xB8\xEB\xA1\x9C", { { nullptr, nullptr } }, float3_t(0.09f, -0.01f, 1351.48f),
			nullptr, nullptr, nullptr },
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
	m_fDebugGatePendingSeconds = 0.f;
	m_bDebugGateFailed = false;
	m_DebugGatePendingPlacements.clear();
	std::size_t spawnRequests = 0u;
	for (const char_t* pPlacementId : gate.BossPlacementIds)
	{
		if (nullptr == pPlacementId)
			continue;
		std::uint64_t requestToken = 0u;
		if (!m_pWorldEntityCommandSink->Request_SpawnWorldEntity(pPlacementId, &requestToken))
		{
			m_bDebugGateFailed = true;
			outStatus = m_strDebugGateStatus =
				label + ": spawn command was rejected for " + pPlacementId;
			return false;
		}
		m_DebugGatePendingPlacements.emplace(pPlacementId, requestToken);
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

void Client::CLevel_KakulSaydonArena::Debug_RetireGateActivation(const std::string& reason)
{
	if (!Is_DebugGatePending()) return;
	const std::string finalReason = reason;
	m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_DebugGatePendingPlacements.clear();
	m_fDebugGatePendingSeconds = 0.f;
	m_bDebugGateFailed = true;
	m_strDebugGateStatus = finalReason;
	m_PlayerController.Retire_DebugPlayerPlacementRequest(finalReason);
	CCombatHUDViewModel::Get().Set_BossFocusArchetype("");
	CCombatHUDViewModel::Get().Set_BossHidden(true);
	CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss("", "");
	CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(false);
}

void Client::CLevel_KakulSaydonArena::Debug_SetSequenceCombatPending(const bool_t pending)
{
	m_bSequenceCombatPending = pending;
	if (!pending) m_bSequenceCombatFadeHeld = false;
	CCombatHUDViewModel::Get().Set_BossHidden(pending || m_iActiveDebugGate == NO_ACTIVE_DEBUG_GATE);
	if (!Is_DebugGatePending())
		CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(pending);
}

void Client::CLevel_KakulSaydonArena::Debug_HoldSequenceCombatFade()
{
	m_bSequenceCombatFadeHeld = true;
	m_fTriggerMoveFadeAlpha = 1.f;
	if (m_pTriggerMoveFadeView)
	{
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", true);
		m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen", float4_t(0.f, 0.f, 0.f, 1.f));
	}
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
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.Camera.Create");
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::KOUKU_SAYDON,
		m_FollowCameraProfile, m_strFollowCameraProfileStatus))
	{
		OutputDebugStringA(("[Level_KakulSaydonArena][FollowCamera] " +
			m_strFollowCameraProfileStatus + "\n").c_str());
	}
	const float3_t positionOffset = m_FollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
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
			approvedSpawn.fPositionX + positionOffset.x,
			approvedSpawn.fPositionY + positionOffset.y,
			approvedSpawn.fPositionZ + positionOffset.z);
		initialAt = float3_t(
			approvedSpawn.fPositionX + lookOffset.x,
			approvedSpawn.fPositionY + lookOffset.y,
			approvedSpawn.fPositionZ + lookOffset.z);
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = initialEye;
	cameraDesc.vAt = initialAt;
	cameraDesc.fFovy = m_FollowCameraProfile.fovYDegrees;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = g_KakulSaydonFreeCameraSpeed;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = positionOffset;
	cameraDesc.vLookOffset = lookOffset;
	cameraDesc.fFollowResponse = m_FollowCameraProfile.followResponse;
	cameraDesc.fFollowRollDegrees = m_FollowCameraProfile.rotationDegrees.z;
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

bool_t Client::CLevel_KakulSaydonArena::Set_FollowCameraProfile(
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
	outStatus = "Applied to this map's follow camera. Save to keep these settings.";
	m_strFollowCameraProfileStatus = outStatus;
	return true;
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
	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);
	return true;
}

namespace
{
	std::filesystem::path Camera_AuthoringPath()
	{
		return CProjectDataRoot::Resolve(L"Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json");
	}
	std::string Camera_JsonText(const DATA_JSON_VALUE& value, const unsigned depth = 0u)
	{
		if (value.Is_String()) return "\"" + CDataJson::Escape(value.Get_String()) + "\"";
		if (value.Is_Number()) { std::ostringstream stream; stream << std::setprecision(17) << value.Get_Number(); return stream.str(); }
		if (value.Is_Boolean()) return value.Get_Boolean() ? "true" : "false";
		if (value.Is_Null()) return "null";
		const bool object = value.Is_Object();
		std::string text = object ? "{" : "[";
		bool first = true;
		const auto append = [&](const std::string& item) {
			text += first ? "\n" : ",\n"; first = false;
			text += std::string((depth + 1u) * 2u, ' ') + item;
		};
		if (object)
		{
			std::set<std::string> written;
			for (const auto& key : value.Get_ObjectInsertionOrder())
				if (const auto* child = value.Find(key); child && written.insert(key).second)
					append("\"" + CDataJson::Escape(key) + "\": " + Camera_JsonText(*child, depth + 1u));
			for (const auto& [key, child] : value.Get_Object())
				if (written.insert(key).second) append("\"" + CDataJson::Escape(key) + "\": " + Camera_JsonText(child, depth + 1u));
		}
		else for (const auto& child : value.Get_Array()) append(Camera_JsonText(child, depth + 1u));
		if (!first) text += "\n" + std::string(depth * 2u, ' ');
		return text + (object ? "}" : "]");
	}
	DATA_JSON_VALUE Camera_TrackJson(const VALTAN_CINEMATIC_CAMERA_CUE& cue);
	DATA_JSON_VALUE Camera_ShotJson(const CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT& shot,
		const DATA_JSON_VALUE* existing)
	{
		using J = DATA_JSON_VALUE;
		const auto vec = [](const float3_t& v) { return J::Array({ J::Number(v.x), J::Number(v.y), J::Number(v.z) }); };
		J::OBJECT fields = existing ? existing->Get_Object() : J::OBJECT{};
		fields["shotId"] = J::String(shot.strShotId);
		fields["displayName"] = J::String(shot.strDisplayName);
		fields["sequenceInstanceId"] = J::String(shot.strSequenceInstanceId);
		fields["box"] = J::Object({ {"center", vec(shot.vCenter)}, {"halfExtents", vec(shot.vHalfExtents)}, {"yawDegrees", J::Number(shot.fYawDegrees)} });
		fields["eye"] = vec(shot.vEye); fields["lookAt"] = vec(shot.vLookAt);
		fields["fovYDegrees"] = J::Number(shot.fFovYDegrees);
		fields["blendInMs"] = J::Number(shot.iBlendInMs); fields["blendOutMs"] = J::Number(shot.iBlendOutMs);
		fields["defaultHoldMs"] = J::Number(shot.iDefaultHoldMs); fields["priority"] = J::Number(shot.iPriority);
		fields["activation"] = J::String(shot.bPatternOnly ? "PATTERN_ONLY" : "AUTO");
		fields["transitionEasing"] = J::String(shot.eTransitionEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? "LINEAR" : "SMOOTHSTEP");
		if (shot.followsPlayer) fields["follow"] = J::Object({ {"eyeOffset", vec(shot.vFollowEyeOffset)}, {"lookAtOffset", vec(shot.vFollowLookAtOffset)} });
		else fields.erase("follow");
		if (shot.hasCameraTrack) fields["cameraTrack"] = Camera_TrackJson(shot.CameraTrack);
		else fields.erase("cameraTrack");
		return J::Object(std::move(fields), existing ? existing->Get_ObjectInsertionOrder() : std::vector<std::string>{});
	}
	DATA_JSON_VALUE Camera_TrackJson(const VALTAN_CINEMATIC_CAMERA_CUE& cue)
	{
		using J = DATA_JSON_VALUE;
		const auto vec = [](const float3_t& v) { return J::Array({J::Number(v.x), J::Number(v.y), J::Number(v.z)}); };
		J::ARRAY keys;
		for (const auto& key : cue.Keyframes)
		{
			J::OBJECT fields{{"sceneId", J::String(key.strSceneId)}, {"timeMs", J::Number(key.iTimeMs)},
				{"eye", vec(key.vEye)}, {"lookAt", vec(key.vLookAt)}, {"fovYDegrees", J::Number(key.fFovYDegrees)}};
			if (key.hasUp) fields["up"] = vec(key.vUp);
			keys.push_back(J::Object(std::move(fields)));
		}
		return J::Object({{"durationMs", J::Number(cue.iDurationMs)},
			{"interpolation", J::String(cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR ? "LINEAR" :
				cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM ? "CATMULL_ROM" : "INVALID")},
			{"easing", J::String(cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? "LINEAR" :
				cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP ? "SMOOTHSTEP" :
				cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::HOLD ? "HOLD" : "INVALID")}, {"keyframes", J::Array(std::move(keys))}});
	}
	std::string Camera_EmptyDocument()
	{
		return "{\"schema\":\"lostark.camera-shots\",\"formatVersion\":1,\"areaId\":\"LV_LUT_MIDNIGHTC_ED\",\"revision\":1,\"shots\":[]}";
	}
}

Client::VALTAN_CINEMATIC_CAMERA_CUE Client::CLevel_KakulSaydonArena::CameraShot_ToCue(const KAKUL_CAMERA_SHOT& shot)
{
	auto cue = shot.hasCameraTrack ? shot.CameraTrack : VALTAN_CINEMATIC_CAMERA_CUE{};
	cue.strCueId = shot.strShotId;
	cue.iTransitionInMs = shot.iBlendInMs; cue.iTransitionOutMs = shot.iBlendOutMs;
	if (!shot.hasCameraTrack)
	{
		cue.iDurationMs = (std::clamp)(shot.iBlendInMs + shot.iDefaultHoldMs, 1u, CAMERA_TRACK_MAX_DURATION_MS);
		cue.Keyframes = {{shot.strShotId + ".p1", 0u, shot.vEye, shot.vLookAt, shot.fFovYDegrees}};
	}
	return cue;
}

bool_t Client::CLevel_KakulSaydonArena::Stage_PatternCameraTracks(const std::string_view baseline,
	const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues, const std::map<std::string, std::string>& names,
	std::string& outText, std::string& outStatus)
{
	using J = DATA_JSON_VALUE;
	std::vector<KAKUL_CAMERA_SHOT> original;
	J root;
	if (!Parse_CameraShots(baseline, original, outStatus) || !CDataJson::Parse(baseline, root, outStatus)) return false;
	auto rows = root.Find("shots")->Get_Array();
	std::set<std::string> ids;
	bool changed = false;
	for (const auto& cue : cues)
	{
		const auto name = names.find(cue.strCueId);
		if (!ids.insert(cue.strCueId).second || name == names.end() || cue.eTrackingMode != VALTAN_CINEMATIC_TRACKING_MODE::WORLD ||
			cue.fShakeAmplitude != 0.f || cue.iShakeDurationMs != 0u)
		{ outStatus = "Pattern Camera requires a unique shot, display name and WORLD track."; return false; }
		const auto old = std::find_if(original.begin(), original.end(), [&](const auto& shot) { return shot.strShotId == cue.strCueId; });
		if (old != original.end() && !old->bPatternOnly)
		{ outStatus = "Automatic Area cameras stay in their existing Map Tool owner."; return false; }
		const bool trackChanged = old == original.end() ||
			Camera_JsonText(Camera_TrackJson(CameraShot_ToCue(*old))) != Camera_JsonText(Camera_TrackJson(cue));
		if (!trackChanged && old->strDisplayName == name->second && old->iBlendInMs == cue.iTransitionInMs &&
			old->iBlendOutMs == cue.iTransitionOutMs) continue;
		KAKUL_CAMERA_SHOT shot = old != original.end() ? *old : KAKUL_CAMERA_SHOT{};
		shot.strShotId = cue.strCueId; shot.strDisplayName = name->second; shot.bPatternOnly = true;
		shot.iBlendInMs = cue.iTransitionInMs; shot.iBlendOutMs = cue.iTransitionOutMs;
		if (trackChanged || old->iBlendInMs != cue.iTransitionInMs)
			shot.iDefaultHoldMs = cue.iDurationMs > cue.iTransitionInMs ? cue.iDurationMs - cue.iTransitionInMs : 0u;
		if (old == original.end())
		{
			shot.vHalfExtents = {1.f, 1.f, 1.f};
			shot.iDefaultHoldMs = cue.iDurationMs > cue.iTransitionInMs ? cue.iDurationMs - cue.iTransitionInMs : 0u;
		}
		if (trackChanged && !cue.Keyframes.empty())
		{ shot.vEye = cue.Keyframes.front().vEye; shot.vLookAt = cue.Keyframes.front().vLookAt; shot.fFovYDegrees = cue.Keyframes.front().fFovYDegrees; }
		const auto target = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.Find("shotId")->Get_String() == cue.strCueId; });
		auto json = Camera_ShotJson(shot, target == rows.end() ? nullptr : &*target);
		auto fields = json.Get_Object();
		if (trackChanged) { fields.erase("follow"); fields["cameraTrack"] = Camera_TrackJson(cue); }
		json = J::Object(std::move(fields), json.Get_ObjectInsertionOrder());
		if (target == rows.end()) rows.push_back(std::move(json)); else *target = std::move(json);
		changed = true;
	}
	std::erase_if(rows, [&](const auto& row) {
		const auto id = row.Find("shotId")->Get_String();
		const auto old = std::find_if(original.begin(), original.end(), [&](const auto& shot) { return shot.strShotId == id; });
		const bool removed = old != original.end() && old->bPatternOnly && !ids.contains(id);
		changed |= removed; return removed;
	});
	if (!changed) { outText = std::string(baseline); return true; }
	const double revision = root.Find("revision")->Get_Number();
	if (revision >= 4294967295.0) { outStatus = "Camera revision is exhausted."; return false; }
	auto fields = root.Get_Object(); fields["shots"] = J::Array(std::move(rows)); fields["revision"] = J::Number(revision + 1.0);
	auto text = Camera_JsonText(J::Object(std::move(fields), root.Get_ObjectInsertionOrder())) + "\n";
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus)) return false;
	outText = std::move(text); return true;
}

bool_t Client::CLevel_KakulSaydonArena::Save_CameraShotSource(const std::string_view expectedSource,
	const std::string& text, std::string& outStatus)
{
	if (!m_DirtyCameraShotIds.empty())
	{ outStatus = "Save the Action Workbench Camera draft before saving from Cinematic Camera Tool."; return false; }
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus) ||
		!CMapTool::Save_CameraShotDocumentAtomic(Camera_AuthoringPath(), std::string(expectedSource), text, outStatus)) return false;
	m_AuthoringCameraShots = std::move(staged); m_strCameraAuthoringBaseline = text; m_bCameraAuthoringLoaded = true;
	outStatus = "Saved Kouku Area Camera source. Preview is ready; publish before Complete Play.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Ensure_CameraShotAuthoring(std::string& outStatus)
{
	if (m_bCameraAuthoringLoaded) return true;
	if (m_bCameraAuthoringLoadAttempted)
	{
		outStatus = m_strCameraAuthoringLoadFailure;
		return false;
	}
	return Reload_CameraShotAuthoring(outStatus);
}

bool_t Client::CLevel_KakulSaydonArena::Reload_CameraShotAuthoring(std::string& outStatus)
{
	if (!m_DirtyCameraShotIds.empty())
	{ outStatus = "Save the Camera draft before Reload Cameras; unsaved shots were preserved."; return false; }
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Kouku.CameraAuthoring.Load");
	m_bCameraAuthoringLoadAttempted = true;
	const auto fail = [&](std::string reason) {
		m_strCameraAuthoringLoadFailure = std::move(reason);
		outStatus = m_strCameraAuthoringLoadFailure;
		return false;
	};
	const auto path = Camera_AuthoringPath();
	std::error_code error;
	std::string text;
	if (std::filesystem::exists(path, error))
	{
		const auto bytes = std::filesystem::file_size(path, error);
		if (error || bytes > 2u * 1024u * 1024u)
			return fail("Camera authoring source exceeds 2 MiB or cannot be read.");
		if (bytes == 0u) return fail("Camera authoring source is empty or unreadable.");
		std::ifstream input(path, std::ios::binary);
		if (!input) return fail("Cannot open Camera authoring source.");
		text.resize(static_cast<std::size_t>(bytes));
		input.read(text.data(), static_cast<std::streamsize>(text.size()));
		if (!input || input.peek() != std::char_traits<char>::eof())
			return fail("Camera authoring source changed while reading or is unreadable.");
	}
	else if (error) return fail("Cannot inspect Camera authoring source.");
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text.empty() ? Camera_EmptyDocument() : text, staged, outStatus))
		return fail(outStatus);
	m_AuthoringCameraShots = std::move(staged);
	m_strCameraAuthoringBaseline = std::move(text);
	m_bCameraAuthoringLoaded = true;
	m_strCameraAuthoringLoadFailure.clear();
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Create_CameraShot(const std::string_view name,
	std::string& outShotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	if (m_AuthoringCameraShots.size() >= CAMERA_SHOT_MAX_COUNT)
	{ outStatus = "Camera shot limit is 128."; return false; }
	KAKUL_CAMERA_SHOT shot;
	for (uint32_t ordinal = 1u; ordinal <= CAMERA_SHOT_MAX_COUNT + 1u; ++ordinal)
	{
		shot.strShotId = "camera.kouku.pattern." + std::to_string(ordinal);
		if (std::none_of(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
			[&](const auto& item) { return item.strShotId == shot.strShotId; })) break;
	}
	shot.strDisplayName = std::string(name);
	shot.bPatternOnly = true; shot.eTransitionEasing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
	shot.iBlendInMs = 500u; shot.iBlendOutMs = 500u; shot.vHalfExtents = float3_t(1.f, 1.f, 1.f);
	VALTAN_CINEMATIC_CAMERA_POSE pose;
	if (!CCameraTool::Capture_ViewPose(pose)) { outStatus = "Current Camera pose is unavailable."; return false; }
	shot.vEye = pose.vEye; shot.vLookAt = pose.vLookAt; shot.fFovYDegrees = pose.fFovYDegrees;
	shot.CameraTrack = CameraShot_ToCue(shot);
	shot.CameraTrack.Keyframes.front().vUp = pose.vUp;
	shot.CameraTrack.Keyframes.front().hasUp = true;
	shot.hasCameraTrack = true;
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object(); fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	outShotId = shot.strShotId;
	m_DirtyCameraShotIds.insert(shot.strShotId);
	m_AuthoringCameraShots.push_back(std::move(shot));
	outStatus = "Camera created in the authoring draft. Set Camera Pos, then Save Camera.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Update_CameraShot(const KAKUL_CAMERA_SHOT& shot, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto found = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == shot.strShotId; });
	if (found == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	// Validate the exact track being committed, including times and orientation.
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object();
	fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	auto adjusted = shot;
	if (adjusted.followsPlayer && !found->followsPlayer)
	{
		const auto character = m_Replication.Get_LocalCharacter();
		if (!character || !character->Get_Transform()) { outStatus = "PLAYER anchor requires the local replicated Character."; return false; }
		float3_t position; XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
		adjusted.vFollowEyeOffset = float3_t(shot.vEye.x - position.x, shot.vEye.y - position.y, shot.vEye.z - position.z);
		adjusted.vFollowLookAtOffset = float3_t(shot.vLookAt.x - position.x, shot.vLookAt.y - position.y, shot.vLookAt.z - position.z);
	}
	*found = adjusted; m_DirtyCameraShotIds.insert(shot.strShotId);
	outStatus = "Camera draft changed. Save Camera writes the Area source.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Capture_CameraShot(const std::string_view shotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto found = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == shotId; });
	if (found == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	VALTAN_CINEMATIC_CAMERA_POSE pose;
	if (!CCameraTool::Capture_ViewPose(pose)) { outStatus = "Current Camera pose is unavailable."; return false; }
	auto shot = *found;
	shot.vEye = pose.vEye; shot.vLookAt = pose.vLookAt; shot.fFovYDegrees = pose.fFovYDegrees;
	shot.hasCameraTrack = false;
	if (shot.followsPlayer)
	{
		const auto character = m_Replication.Get_LocalCharacter();
		if (!character || !character->Get_Transform()) { outStatus = "PLAYER capture requires the local replicated Character."; return false; }
		float3_t position; XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
		shot.vFollowEyeOffset = float3_t(pose.vEye.x - position.x, pose.vEye.y - position.y, pose.vEye.z - position.z);
		shot.vFollowLookAtOffset = float3_t(pose.vLookAt.x - position.x, pose.vLookAt.y - position.y, pose.vLookAt.z - position.z);
	}
	return Update_CameraShot(shot, outStatus);
}

bool_t Client::CLevel_KakulSaydonArena::Save_CameraShots(std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	if (m_DirtyCameraShotIds.empty()) { outStatus = "Camera source is already saved."; return true; }
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(m_strCameraAuthoringBaseline.empty() ? Camera_EmptyDocument() : m_strCameraAuthoringBaseline, root, outStatus)) return false;
	auto fields = root.Get_Object(); auto rows = root.Find("shots")->Get_Array();
	for (const auto& shot : m_AuthoringCameraShots)
	{
		if (!m_DirtyCameraShotIds.contains(shot.strShotId)) continue;
		auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.Find("shotId")->Get_String() == shot.strShotId; });
		if (found == rows.end()) rows.push_back(Camera_ShotJson(shot, nullptr));
		else *found = Camera_ShotJson(shot, &*found);
	}
	const double revision = root.Find("revision")->Get_Number();
	if (revision >= 4294967295.0) { outStatus = "Camera revision is exhausted."; return false; }
	fields["shots"] = DATA_JSON_VALUE::Array(std::move(rows));
	fields["revision"] = DATA_JSON_VALUE::Number(revision + 1.0);
	const auto text = Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields), root.Get_ObjectInsertionOrder())) + "\n";
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus) ||
		!CMapTool::Save_CameraShotDocumentAtomic(Camera_AuthoringPath(), m_strCameraAuthoringBaseline, text, outStatus)) return false;
	m_AuthoringCameraShots = std::move(staged); m_strCameraAuthoringBaseline = text; m_DirtyCameraShotIds.clear();
	outStatus = "Camera saved to the Area source; Preview is ready. Publish the Area before Complete Play.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Load_CameraShots(
	std::string& outStatus)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.CameraShots.Load");
	const std::filesystem::path path = Find_CameraShotDocument();
	std::error_code fileError;
	if (!std::filesystem::is_regular_file(path, fileError) || fileError)
	{
		outStatus = "KoukuSaydon camera shot document is absent; follow view only.";
		return true;
	}
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (fileError || 0u == fileBytes || fileBytes > 2u * 1024u * 1024u)
	{
		outStatus = "KoukuSaydon camera shot document is empty or exceeds 2 MiB.";
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

	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus)) return false;
	Release_CameraShot();
	m_CameraShots = std::move(staged);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Parse_CameraShots(
	const std::string_view text, std::vector<KAKUL_CAMERA_SHOT>& outShots, std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 2u * 1024u * 1024u;
	limits.iMaximumDepth = 12u;
	// The document supports 128 shots with 64 keys each, including eye/lookAt/up.
	// Keep the byte/depth and per-shot/key bounds; 4096 values rejected valid tracks.
	limits.iMaximumValues = 128u * 1024u;
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
			{ "cameraTrack", "follow", "displayName", "defaultHoldMs", "transitionEasing", "activation" }))
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
		shot.strDisplayName = shot.strShotId;
		if (const auto* name = value.Find("displayName"))
		{
			if (!name->Is_String() || name->Get_String().empty() || name->Get_String().size() > 128u || name->Get_String().find('\0') != std::string::npos ||
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name->Get_String().data(),
					static_cast<int>(name->Get_String().size()), nullptr, 0) <= 0)
			{ outStatus = "Camera displayName requires 1..128 UTF-8 bytes."; return false; }
			shot.strDisplayName = name->Get_String();
		}
		if (const auto* hold = value.Find("defaultHoldMs"))
			if (!Read_Uint(hold, 600000u, shot.iDefaultHoldMs) || shot.iBlendInMs + shot.iDefaultHoldMs > 600000u ||
				shot.iBlendInMs + shot.iDefaultHoldMs == 0u)
			{ outStatus = "Camera entry plus default hold must be 1..600000 ms."; return false; }
		if (const auto* easing = value.Find("transitionEasing"))
		{
			if (!easing->Is_String() || (easing->Get_String() != "LINEAR" && easing->Get_String() != "SMOOTHSTEP"))
			{ outStatus = "Camera transitionEasing must be LINEAR or SMOOTHSTEP."; return false; }
			shot.eTransitionEasing = easing->Get_String() == "LINEAR" ?
				VALTAN_CINEMATIC_CAMERA_EASING::LINEAR : VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		}
		if (const auto* activation = value.Find("activation"))
		{
			if (!activation->Is_String() || (activation->Get_String() != "AUTO" && activation->Get_String() != "PATTERN_ONLY"))
			{ outStatus = "Camera activation must be AUTO or PATTERN_ONLY."; return false; }
			shot.bPatternOnly = activation->Get_String() == "PATTERN_ONLY";
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

	outShots = std::move(stagedShots);
	outStatus = "KoukuSaydon camera shots loaded: " +
		std::to_string(outShots.size());
	return true;
}

const Client::CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT*
Client::CLevel_KakulSaydonArena::Find_ActiveCameraShot(
	const float3_t& vPosition) const
{
	const KAKUL_CAMERA_SHOT* best = nullptr;
	for (const KAKUL_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.bPatternOnly) continue;
		const bool_t isHeldNow = shot.strShotId == m_strActiveCameraShotId;
		bool_t isActive = false;
		if (shot.strShotId == CARD_MAZE_TELESCOPE_SHOT_ID)
		{
			isActive = (CCombatHUDViewModel::Get().Get_KoukuGimmick().CardMaze.flags & 1u) != 0u;
		}
		else if (!shot.strSequenceInstanceId.empty())
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

void Client::CLevel_KakulSaydonArena::Update_CardMazePresentation(f32_t dt)
{
	const auto tick = m_Replication.Get_LastServerTick();
	if (m_iCardMazeLastSnapshotTick != tick)
	{ m_iCardMazeLastSnapshotTick = tick; m_fCardMazeSnapshotSeconds = 0.f; }
	else m_fCardMazeSnapshotSeconds = (std::min)(.1f, m_fCardMazeSnapshotSeconds + dt);
	const auto& state = CCombatHUDViewModel::Get().Get_KoukuGimmick().CardMaze;
	const bool playing = state.marchStartTick && state.marchCycleMs;
	if (!playing && !m_bCardMazeMarchPlaying) return;
	const auto targets = Make_WorldSequenceTargets();
	const float elapsed = playing ? std::fmod(
		float(tick - state.marchStartTick) * (1000.f / 30.f) + m_fCardMazeSnapshotSeconds * 1000.f,
		float(state.marchCycleMs)) : 0.f;
	for (const auto& instance : m_SequencePlayer.Get_Document().Get_Instances())
	{
		if (!instance.instanceId.starts_with("cardmiro.march.instance.")) continue;
		const auto* sequence = m_SequencePlayer.Get_Document().Find_Template(instance.templateId);
		const bool active = playing && instance.enabled && sequence &&
			elapsed >= float(instance.startDelayMs) && elapsed < float(instance.startDelayMs + sequence->durationMs);
		if (!active) { m_SequencePlayer.Stop_Instance(instance.instanceId, targets, true); continue; }
		if (!m_SequencePlayer.Is_Playing(instance.instanceId) && !m_SequencePlayer.Play(instance.instanceId, targets)) continue;
		(void)m_SequencePlayer.Seek_InstanceToMs(instance.instanceId, elapsed, targets);
	}
	m_bCardMazeMarchPlaying = playing;
}

bool_t Client::CLevel_KakulSaydonArena::Load_EntranceTriggerMarkers()
{
	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
		std::string(KAKULSAYDON_AREA_ID) / "Gameplay.world.json"),
		std::string(KAKULSAYDON_AREA_ID), status))
	{
		OutputDebugStringA(("[KoukuEntranceMarker] " + status + "\n").c_str());
		return false;
	}
	static constexpr std::array<std::string_view, 5> triggerIds = {
		"jump.1", "jump.2", "jump.3", "paper.1", "paper.2" };
	std::vector<ENTRANCE_TRIGGER_MARKER> staged;
	for (const std::string_view id : triggerIds)
	{
		const auto* placement = document.Find(std::string(id));
		if (!placement || placement->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
			placement->triggerEvents.size() != 1u)
		{
			OutputDebugStringA(("[KoukuEntranceMarker] Invalid trigger: " +
				std::string(id) + "\n").c_str());
			return false;
		}
		if (!placement->isEnabled) continue;
		const auto& event = placement->triggerEvents.front();
		if (event.eKind != WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER &&
			event.eKind != WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE)
			return false;
		ENTRANCE_TRIGGER_MARKER marker;
		marker.placementId = placement->placementId;
		if (placement->isTriggerOnce && event.eKind == WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE)
			marker.sequenceInstanceId = event.targetId;
		// Trigger_Box::Rebuild_Bounds uses placement.position as its exact center.
		// No authoring position copy, ground guess, or collider-local offset.
		XMStoreFloat4x4(&marker.rootWorld, XMMatrixTranslation(
			placement->position.x, placement->position.y, placement->position.z));
		staged.push_back(std::move(marker));
	}
	Clear_EntranceTriggerMarkers();
	m_EntranceTriggerMarkers = std::move(staged);
	return true;
}

void Client::CLevel_KakulSaydonArena::Clear_EntranceTriggerMarkers()
{
	for (auto& marker : m_EntranceTriggerMarkers)
		CEffectPresentationService::Stop_WorldRoot(marker.handle);
	m_EntranceTriggerMarkers.clear();
}

void Client::CLevel_KakulSaydonArena::Retire_EntranceTriggerMarker(
	const std::string& sequenceInstanceId)
{
	for (auto& marker : m_EntranceTriggerMarkers)
	{
		if (marker.sequenceInstanceId.empty() || marker.sequenceInstanceId != sequenceInstanceId)
			continue;
		CEffectPresentationService::Stop_WorldRoot(marker.handle);
		marker.handle = {};
		marker.retired = true;
	}
}

void Client::CLevel_KakulSaydonArena::Update_EntranceTriggerMarkers(const f32_t deltaSeconds)
{
	if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f) return;
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		Clear_EntranceTriggerMarkers();
		return;
	}
	for (auto& marker : m_EntranceTriggerMarkers)
	{
		if (marker.retired) continue;
		const bool_t firstSample = !marker.started;
		if (firstSample)
		{
			// Defer creation until the Level is current; Loader already prepared
			// the shared World target. Optional failures are isolated once.
			marker.started = true;
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
			desc.iLevelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
			desc.strPlacementId = "kouku.entrance.trigger." + marker.placementId;
			desc.strEffectAssetId = "effect.world.move_destination";
			desc.RootWorld = marker.rootWorld;
			desc.bExternallySampled = true;
			std::string status;
			if (!CEffectPresentationService::Spawn_LevelPlacement(desc, marker.handle, status))
			{
				marker.retired = true;
				OutputDebugStringA(("[KoukuEntranceMarker] " + marker.placementId +
					": " + status + "\n").c_str());
				continue;
			}
		}
		else marker.seconds = std::fmod(marker.seconds + deltaSeconds, 7.f);
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
			[root = marker.rootWorld](f32_t, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& sample,
				std::string& status)
			{
				sample.RootWorld = root;
				sample.SourceAnchorWorlds.clear();
				status.clear();
				return true;
			};
		if (!CEffectPresentationService::Seek_WorldRoot(marker.handle,
			marker.seconds, provider, firstSample))
		{
			CEffectPresentationService::Stop_WorldRoot(marker.handle);
			marker.handle = {};
			marker.retired = true;
			OutputDebugStringA(("[KoukuEntranceMarker] Sample failed: " +
				marker.placementId + "\n").c_str());
		}
	}
}

void Client::CLevel_KakulSaydonArena::Update_TriggerMoveFade(
	const f32_t fTimeDelta)
{
	if (nullptr == m_pTriggerMoveFadeView)
		return;

#ifdef _DEBUG
	if (m_bSequenceCombatFadeHeld)
	{
		Debug_HoldSequenceCombatFade();
		return;
	}
#endif
	using LostArk::Shared::PLAYER_ACTION_STATE;
	const auto& maze = CCombatHUDViewModel::Get().Get_KoukuGimmick().CardMaze;
	if (maze.transferStartTick)
	{
		const float ticks = float(m_Replication.Get_LastServerTick() - maze.transferStartTick) + m_fCardMazeSnapshotSeconds * 30.f;
		m_fTriggerMoveFadeAlpha = ticks < 12.f ? std::clamp(ticks / 12.f, 0.f, 1.f) :
			ticks < 24.f ? 1.f : std::clamp((36.f - ticks) / 12.f, 0.f, 1.f);
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", m_fTriggerMoveFadeAlpha > 0.f);
		m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen", float4_t(0.f, 0.f, 0.f, m_fTriggerMoveFadeAlpha));
		return;
	}
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

bool_t Client::CLevel_KakulSaydonArena::Try_GetCompositionWorldPivot(
 const std::string_view instanceId, float4x4_t& out, const std::string_view occurrenceId,
 const std::uint32_t emissionIndex) const
{
#ifdef _DEBUG
 if (!m_CompositionWorldPreviewCues.empty())
 {
  const CWorldSequencePlayer* selected = nullptr;
  for (const auto& [id, playback] : m_CompositionWorldPreviewCues)
   if ((occurrenceId.empty() || id == occurrenceId) && playback.cue.instanceId == instanceId &&
    playback.player->Is_Playing(playback.cue.instanceId))
   {
    if (selected) return false;
    selected = playback.player.get();
   }
  return selected && selected->Try_GetSequencePivot(std::string(instanceId), out, emissionIndex);
 }
#endif
 return m_SequencePlayer.Try_GetSequencePivot(std::string(instanceId), out, emissionIndex);
}

bool_t Client::CLevel_KakulSaydonArena::Sample_CompositionCamera(
	const std::string_view shotId, const float seconds, const float3_t& offset,
	const std::string_view ownerKey, const uint32_t durationMs, const bool_t preview)
{
	constexpr uint64_t owner = 0x4b4f554b55434f4dull;
	if (!m_pCamera || ownerKey.empty() || !std::isfinite(seconds) || seconds < 0.f || durationMs == 0u ||
		!std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(offset.z)) return false;
	if (preview) { std::string status; if (!Ensure_CameraShotAuthoring(status)) return false; }
	const auto& shots = preview ? m_AuthoringCameraShots : m_CameraShots;
	const auto found = std::find_if(shots.begin(), shots.end(), [shotId](const auto& shot) { return shot.strShotId == shotId; });
	if (found == shots.end() || durationMs < found->iBlendInMs) return false;
	auto& transition = m_CompositionCamera;
	if (transition.cancelledOwnerKey == ownerKey) return false;
	if (!preview && !m_pCamera->Is_FollowEnabled()) return false;
	if (transition.ownerKey != ownerKey || transition.returning || seconds + 0.01f < transition.lastSeconds)
	{
		VALTAN_CINEMATIC_CAMERA_POSE current;
		if (!CCameraTool::Capture_ViewPose(current)) return false;
		if (!m_pCamera->Begin_PresentationOverride(owner, Engine::CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW)) return false;
		// Taking over an Area shot keeps the displayed pose but drops its stale owner state.
		Release_CameraShot();
		transition.ownerKey = std::string(ownerKey);
		transition.cancelledOwnerKey.clear();
		transition.fromPose = current;
		transition.entryPose = current;
		transition.appliedPose = current;
		transition.blendOutMs = found->iBlendOutMs;
		transition.easing = found->eTransitionEasing;
		transition.returnSeconds = 0.f;
		transition.returning = false;
		transition.followAtStart = m_pCamera->Is_FollowEnabled();
	}
	if (!m_pCamera->Is_PresentationOverrideOwnedBy(owner) ||
		m_pCamera->Is_FollowEnabled() != transition.followAtStart)
	{ Stop_CompositionCamera(true); return false; }
	VALTAN_CINEMATIC_CAMERA_POSE target{ found->vEye, found->vLookAt, found->fFovYDegrees };
	if (found->hasCameraTrack)
	{
		if (!CValtanCinematicCameraController::Sample_Cue(found->CameraTrack, seconds, target)) return false;
	}
	else if (found->followsPlayer)
	{
		const auto character = m_Replication.Get_LocalCharacter();
		if (!character || !character->Get_Transform()) return false;
		float3_t player; XMStoreFloat3(&player, character->Get_Transform()->Get_State(STATE::POSITION));
		target.vEye = float3_t(player.x + found->vFollowEyeOffset.x, player.y + found->vFollowEyeOffset.y, player.z + found->vFollowEyeOffset.z);
		target.vLookAt = float3_t(player.x + found->vFollowLookAtOffset.x, player.y + found->vFollowLookAtOffset.y, player.z + found->vFollowLookAtOffset.z);
	}
	target.vEye.x += offset.x; target.vEye.y += offset.y; target.vEye.z += offset.z;
	target.vLookAt.x += offset.x; target.vLookAt.y += offset.y; target.vLookAt.z += offset.z;
	VALTAN_CINEMATIC_CAMERA_POSE applied = target;
	if (found->iBlendInMs && !CValtanCinematicCameraController::Sample_BoundedTransition(
		transition.fromPose, target, found->iBlendInMs, seconds, applied, found->eTransitionEasing)) return false;
	if (!(applied.hasUp ? m_pCamera->Apply_PresentationPoseWithUp(owner, applied.vEye, applied.vLookAt, applied.vUp, applied.fFovYDegrees) :
		m_pCamera->Apply_PresentationPose(owner, applied.vEye, applied.vLookAt, applied.fFovYDegrees))) return false;
	transition.appliedPose = applied;
	transition.lastSeconds = seconds;
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Resolve_CompositionFollowPose(VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	const auto character = m_Replication.Get_LocalCharacter();
	if (!character || !character->Get_Transform()) return false;
	float3_t position; XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
	const auto eyeOffset = m_FollowCameraProfile.positionOffset;
	const auto lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
	outPose.vEye = float3_t(position.x + eyeOffset.x, position.y + eyeOffset.y, position.z + eyeOffset.z);
	outPose.vLookAt = float3_t(position.x + lookOffset.x, position.y + lookOffset.y, position.z + lookOffset.z);
	outPose.fFovYDegrees = m_FollowCameraProfile.fovYDegrees;
	const auto rotation = m_FollowCameraProfile.rotationDegrees;
	const auto basis = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
	XMStoreFloat3(&outPose.vUp, basis.r[1]); outPose.hasUp = true;
	return true;
}

void Client::CLevel_KakulSaydonArena::Stop_CompositionCamera(const bool_t force)
{
	auto& transition = m_CompositionCamera;
	if (transition.ownerKey.empty()) return;
	if (force)
	{
		transition.cancelledOwnerKey = transition.ownerKey;
		if (m_pCamera)
		{
			auto pose = transition.appliedPose;
			if (m_pCamera->Is_FollowEnabled())
			{
				(void)Resolve_CompositionFollowPose(pose);
				(void)m_pCamera->End_PresentationOverrideToPose(0x4b4f554b55434f4dull, pose.vEye, pose.vLookAt, pose.fFovYDegrees);
			}
			else (void)m_pCamera->End_PresentationOverride(0x4b4f554b55434f4dull);
		}
		transition.ownerKey.clear(); transition.returning = false;
		return;
	}
	if (!transition.returning)
	{
		transition.fromPose = transition.appliedPose;
		transition.returning = true; transition.returnSeconds = 0.f;
	}
}

void Client::CLevel_KakulSaydonArena::Update_CompositionCamera(const f32_t timeDelta)
{
	constexpr uint64_t owner = 0x4b4f554b55434f4dull;
	auto& transition = m_CompositionCamera;
	if (transition.ownerKey.empty()) return;
	if (!m_pCamera || !m_pCamera->Is_PresentationOverrideOwnedBy(owner) ||
		m_pCamera->Is_FollowEnabled() != transition.followAtStart)
	{ Stop_CompositionCamera(true); return; }
	if (!transition.returning || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
	VALTAN_CINEMATIC_CAMERA_POSE target = transition.entryPose;
	if (transition.followAtStart && !Resolve_CompositionFollowPose(target)) { Stop_CompositionCamera(true); return; }
	transition.returnSeconds += timeDelta;
	VALTAN_CINEMATIC_CAMERA_POSE applied = target;
	if (transition.blendOutMs && !CValtanCinematicCameraController::Sample_BoundedTransition(
		transition.fromPose, target, transition.blendOutMs, transition.returnSeconds, applied, transition.easing))
	{ Stop_CompositionCamera(true); return; }
	if (!(applied.hasUp ? m_pCamera->Apply_PresentationPoseWithUp(owner, applied.vEye, applied.vLookAt, applied.vUp, applied.fFovYDegrees) :
		m_pCamera->Apply_PresentationPose(owner, applied.vEye, applied.vLookAt, applied.fFovYDegrees)))
	{ Stop_CompositionCamera(true); return; }
	transition.appliedPose = applied;
	if (transition.returnSeconds * 1000.f >= float(transition.blendOutMs))
	{
		if (transition.followAtStart) (void)m_pCamera->End_PresentationOverrideToPose(owner, target.vEye, target.vLookAt, target.fFovYDegrees);
		else (void)m_pCamera->End_PresentationOverride(owner);
		if (transition.followAtStart)
			(void)m_pCamera->Set_FollowPose(m_FollowCameraProfile.positionOffset, CArenaCameraProfile::LookOffset(m_FollowCameraProfile),
				m_FollowCameraProfile.rotationDegrees.z, m_FollowCameraProfile.fovYDegrees, m_FollowCameraProfile.followResponse);
		transition.ownerKey.clear(); transition.returning = false;
	}
}

void Client::CLevel_KakulSaydonArena::Update_CameraShots(const f32_t fTimeDelta)
{
	if (!m_CompositionCamera.ownerKey.empty()) return;
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

	// The same tuned framing is the destination when a camera shot ends.
	const float3_t positionOffset = m_FollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
	const float3_t followEye(
		position.x + positionOffset.x,
		position.y + positionOffset.y,
		position.z + positionOffset.z);
	const float3_t followLook(
		position.x + lookOffset.x,
		position.y + lookOffset.y,
		position.z + lookOffset.z);

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
			m_fCameraFovFrom = m_FollowCameraProfile.fovYDegrees;
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
		m_fCameraFovTo = m_FollowCameraProfile.fovYDegrees;
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

bool_t Client::CLevel_KakulSaydonArena::Reload_MapLights()
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.MapLights.Load");
	auto staged=std::make_shared<CMapLightPresentationRuntime>();
	if(!staged->Load_Runtime(std::string(KAKULSAYDON_AREA_ID)))
	{OutputDebugStringA(("[Level_KakulSaydonArena] "+staged->Get_Status()+"\n").c_str());return false;}
	OutputDebugStringA((staged->Get_Status()+"\n").c_str());
	m_pMapLightPresentation=std::move(staged);return true;
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Ready_DebugStageEntryTriggers(
	const std::string& areaId)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.DebugTriggers.Load");
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


void Client::CLevel_KakulSaydonArena::Consume_OwnedWorldCue(
    const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play,
    const CWorldSequencePlayer::TARGET_SET& targets)
{
    using LostArk::Shared::WORLD_SEQUENCE_OPERATION;
    if (play.iRunEpoch < m_iLatestWorldRunEpoch) return;
    if (play.iRunEpoch > m_iLatestWorldRunEpoch)
    {
        for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(targets, true);
        m_OwnedWorldCues.clear();
        m_PendingOwnedWorldCues.clear();
        m_StoppedWorldOwners.clear();
        m_ConsumedWorldCueIds.clear();
        m_iLatestWorldRunEpoch = play.iRunEpoch;
    }
    const auto owner = std::to_string(play.iRunEpoch) + ":" + play.strMemberId;
    const auto runOwner = std::to_string(play.iRunEpoch) + ":";
    if (play.eOperation == WORLD_SEQUENCE_OPERATION::STOP_OWNER ||
        play.eOperation == WORLD_SEQUENCE_OPERATION::FINISH_OWNER)
    {
        m_StoppedWorldOwners.insert(owner);
        for (auto cue = m_OwnedWorldCues.begin(); cue != m_OwnedWorldCues.end();)
            if (cue->second.runEpoch == play.iRunEpoch &&
                (play.strMemberId.empty() || cue->second.memberId == play.strMemberId))
            {
                if (play.eOperation == WORLD_SEQUENCE_OPERATION::FINISH_OWNER &&
                    cue->second.player->Get_LongestElapsedSpanMs() > cue->second.durationMs)
                { ++cue; continue; }
                cue->second.player->Stop_All(targets, true); cue = m_OwnedWorldCues.erase(cue);
            }
            else ++cue;
        return;
    }
    if (m_StoppedWorldOwners.contains(owner) || m_StoppedWorldOwners.contains(runOwner)) return;
    const std::string key = owner + ":" + play.strCueId;
    if (m_ConsumedWorldCueIds.contains(key)) return; // reliable resend is idempotent
    if (m_ConsumedWorldCueIds.size() >= 65536u) { OutputDebugStringA("[KoukuWORLD] Run cue capacity exceeded.\n"); return; }
    float seconds = 0.f;
    const auto tick = (std::max)(play.iServerTick, m_Replication.Get_LastServerTick());
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(tick, play.iStartTick, 30.f, seconds)) return;
    const float ageMs = seconds * 1000.f;
    if (!play.strTargetSequenceInstanceId.empty() && play.iDurationMs && ageMs >= play.iDurationMs) return;
    if (play.strTargetSequenceInstanceId.empty() && ageMs >= m_SequencePlayer.Get_InstanceElapsedSpanMs(
        play.strSequenceInstanceId, play.fPlaybackSpeed, play.iDurationMs)) return;
    if (!play.strTargetSequenceInstanceId.empty())
    {
        OWNED_WORLD_CUE* target = nullptr;
        for (auto& [id, cue] : m_OwnedWorldCues)
            if (cue.runEpoch == play.iRunEpoch && cue.memberId == play.strMemberId &&
                cue.sequenceId == play.strTargetSequenceInstanceId &&
                (play.strTargetCueId.empty() || cue.cueId == play.strTargetCueId))
            {
                if (target) { OutputDebugStringA("[KoukuWORLD] Ambiguous owned motion target.\n"); return; }
                target = &cue;
            }
        if (!target) { OutputDebugStringA("[KoukuWORLD] Owned motion target is unavailable.\n"); return; }
        float motionSeconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(play.iStartTick, target->startTick, 30.f, motionSeconds)) return;
        const float present = (std::max)(target->clockMs, motionSeconds * 1000.f + ageMs);
        auto motionTargets = targets;
        motionTargets.objectEmissionAnchor = target->emissionAnchor;
        if (!target->player->Seek_InstanceToMs(target->sequenceId, motionSeconds * 1000.f, motionTargets) ||
            !target->player->Apply_ObjectMotion(target->sequenceId, play.strSequenceInstanceId, motionTargets))
            OutputDebugStringA(("[KoukuWORLD] " + target->player->Get_Status() + "\n").c_str());
        m_ConsumedWorldCueIds.insert(key);
        target->clockMs = present;
        (void)target->player->Seek_InstanceToMs(target->sequenceId, present, motionTargets);
        return;
    }
    for (auto old = m_OwnedWorldCues.begin(); old != m_OwnedWorldCues.end();)
    {
        float elapsed = 0.f;
        if (old->second.durationMs && CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            tick, old->second.startTick, 30.f, elapsed) && elapsed * 1000.f >= old->second.player->Get_LongestElapsedSpanMs())
        { old->second.player->Stop_All(targets, true); old = m_OwnedWorldCues.erase(old); }
        else ++old;
    }
    std::string status;
    if (!Can_StartCompositionWorld(play.strSequenceInstanceId, status))
    { OutputDebugStringA(("[KoukuWORLD] " + status + "\n").c_str()); return; }
    auto cueTargets = targets;
    const auto& run = m_Replication.Get_KoukuBundleState();
    const auto member = std::find_if(run.Members.begin(), run.Members.end(),
        [&](const auto& value) { return value.strMemberId == play.strMemberId; });
    if (run.iRunEpoch != play.iRunEpoch || member == run.Members.end() || !m_WorldEmissionResolver ||
        !m_WorldEmissionResolver(run.iPinnedSourceRevision, member->strPatternId,
            play.strOccurrenceId, cueTargets.objectEmissionAnchor))
    {
        const bool pending = std::any_of(m_PendingOwnedWorldCues.begin(), m_PendingOwnedWorldCues.end(),
            [&](const auto& value) { return value.iRunEpoch == play.iRunEpoch &&
                value.strMemberId == play.strMemberId && value.strCueId == play.strCueId; });
        if (!pending && m_PendingOwnedWorldCues.size() < 1024u) m_PendingOwnedWorldCues.push_back(play);
        else if (!pending) OutputDebugStringA("[KoukuWORLD] Pending presentation cue capacity exceeded.\n");
        return;
    }
    auto player = std::make_shared<CWorldSequencePlayer>();
    const auto placement = WorldPlacementFromCue(play);
    if (!player->Set_Document(m_SequencePlayer.Get_Document(), targets, status) ||
        !player->Prepare_InstanceResources(play.strSequenceInstanceId, targets) ||
        !player->Play(play.strSequenceInstanceId, cueTargets, play.fPlaybackSpeed,
            float3_t(play.fPositionOffsetX, play.fPositionOffsetY, play.fPositionOffsetZ), play.iDurationMs, placement) ||
        !player->Seek_InstanceToMs(play.strSequenceInstanceId, ageMs, cueTargets))
    {
        player->Stop_All(targets, true);
        OutputDebugStringA(("[KoukuWORLD] " + (status.empty() ? player->Get_Status() : status) + "\n").c_str());
        return;
    }
    OWNED_WORLD_CUE cue;
    cue.runEpoch = play.iRunEpoch; cue.startTick = play.iStartTick; cue.durationMs = play.iDurationMs;
    cue.memberId = play.strMemberId; cue.cueId = play.strCueId; cue.occurrenceId = play.strOccurrenceId; cue.sequenceId = play.strSequenceInstanceId;
    cue.emissionAnchor = std::move(cueTargets.objectEmissionAnchor);
    cue.clockMs = ageMs; cue.player = std::move(player);
    m_OwnedWorldCues.emplace(key, std::move(cue));
    m_ConsumedWorldCueIds.insert(key);
}

bool_t Client::CLevel_KakulSaydonArena::Try_GetOwnedCompositionWorldPivot(
    std::uint32_t runEpoch, const std::string& memberId, const std::string& sequenceId,
    const std::string& cueId, float4x4_t& out, const std::uint32_t emissionIndex) const
{
    const OWNED_WORLD_CUE* found = nullptr;
    for (const auto& [id, cue] : m_OwnedWorldCues)
        if (cue.runEpoch == runEpoch && cue.memberId == memberId && cue.sequenceId == sequenceId &&
            (cueId.empty() || cue.occurrenceId == cueId))
        {
            if (found) return false;
            found = &cue;
        }
    return found && found->player->Try_GetSequencePivot(sequenceId, out, emissionIndex);
}


bool_t Client::CLevel_KakulSaydonArena::Can_StartCompositionWorld(
    const std::string& instanceId, std::string& status, const CWorldSequenceDocument* sourceDocument) const
{
    const auto& document = sourceDocument ? *sourceDocument : m_SequencePlayer.Get_Document();
    const auto* source = document.Find_Instance(instanceId);
    if (!source) { status = "WORLD instance is unavailable: " + instanceId; return false; }
    std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> sharedTargets;
    for (const auto& binding : source->bindings)
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
            sharedTargets.emplace(binding.targetKind, binding.targetId);
    if (sharedTargets.empty()) return true;
    const auto conflicts = [&](const CWorldSequencePlayer& player)
    {
        for (const auto& active : player.Get_Document().Get_Instances())
            if (player.Is_Playing(active.instanceId))
                for (const auto& binding : active.bindings)
                    if (sharedTargets.contains({binding.targetKind, binding.targetId})) return true;
        return false;
    };
    if (conflicts(m_SequencePlayer))
    { status = "WORLD map/deploy target is already owned by an active level cue: " + instanceId; return false; }
    for (const auto& [id, cue] : m_OwnedWorldCues)
        if (conflicts(*cue.player))
        { status = "WORLD map/deploy target is already owned by another run/member cue: " + instanceId; return false; }
    return true;
}
