#include "Level_KakulSaydonArena.h"

#include "Camera_Free.h"
#include "Character.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "KakulArenaHiddenPlacements.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "Transform.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unordered_set>

namespace
{
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
	/* Distinct from the Bern and Valtan cinematic owners so the engine's
	   single-owner override never confuses this arena with theirs. */
	constexpr uint64_t KAKULSAYDON_CAMERA_SHOT_OWNER_ID = 0x4B414B554C534854ull;
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
			"KoukuSaton replication observed a disconnected Server session.");
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
	m_PlayerController.Set_LocalCharacter(localCharacter);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled());

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
	Update_CameraShots(fTimeDelta);
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
	return __super::Render();
}

bool_t Client::CLevel_KakulSaydonArena::Load_StageMarkers(
	std::string& outStatus)
{
	const std::filesystem::path path = Find_StageMarkerDocument();
	std::error_code fileError;
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (path.empty() || fileError || 0u == fileBytes || fileBytes > 256u * 1024u)
	{
		outStatus = "KoukuSaton StageMarkers document is missing or exceeds 256 KiB.";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaton StageMarkers document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaton StageMarkers document could not be read completely.";
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
		outStatus = "KoukuSaton StageMarkers root is invalid: " + parseError;
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
		outStatus = "KoukuSaton StageMarkers header is invalid.";
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
			outStatus = "KoukuSaton StageMarkers stage has unexpected properties.";
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
			outStatus = "KoukuSaton StageMarkers stage identity or evidence is invalid.";
			return false;
		}
		stagedMarkers.push_back({
			stageId->Get_String(), placementId->Get_String(),
			displayName->Get_String(), sourceLevelId->Get_String() });
	}

	m_StageMarkers = std::move(stagedMarkers);
	m_StageMarkerPlacementIds = std::move(stagedPlacementIds);
	outStatus = "KoukuSaton StageMarkers loaded.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport(
	const std::uint32_t requestSequence,
	const std::string_view placementId,
	std::string& outStatus)
{
	if (0u == requestSequence || placementId.empty())
	{
		outStatus = "KoukuSaton stage teleport request identity is invalid.";
		return false;
	}
	if (m_StageMarkerPlacementIds.empty())
	{
		outStatus = "KoukuSaton StageMarkers are not authored; teleport is isolated.";
		return false;
	}
	if (!m_StageMarkerPlacementIds.contains(std::string(placementId)))
	{
		outStatus = "KoukuSaton stage marker placement ID is not authored.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_StageTeleport(
			requestSequence, placementId))
	{
		outStatus = "KoukuSaton stage teleport command was rejected.";
		return false;
	}
	outStatus = "KoukuSaton stage teleport command submitted.";
	return true;
}

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
	cameraDesc.fSpeedPerSec = (std::max)(20.f, span * 0.08f);
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
		outStatus = "KoukuSaton camera shot document is absent; follow view only.";
		return true;
	}
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (fileError || 0u == fileBytes || fileBytes > 256u * 1024u)
	{
		outStatus = "KoukuSaton camera shot document is empty or exceeds 256 KiB.";
		return false;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaton camera shot document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaton camera shot document could not be read completely.";
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
		outStatus = "KoukuSaton camera shot root is invalid: " + parseError;
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
		outStatus = "KoukuSaton camera shot header is invalid.";
		return false;
	}

	std::vector<KAKUL_CAMERA_SHOT> stagedShots;
	std::unordered_set<std::string> stagedIds;
	stagedShots.reserve(shots->Get_Array().size());
	for (const DATA_JSON_VALUE& value : shots->Get_Array())
	{
		if (!Has_ExactProperties(value,
			{ "shotId", "sequenceInstanceId", "box", "eye", "lookAt",
				"fovYDegrees", "blendInMs", "blendOutMs", "priority" }))
		{
			outStatus = "KoukuSaton camera shot has unexpected properties.";
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
			outStatus = "KoukuSaton camera shot identity or box is invalid.";
			return false;
		}
		shot.strShotId = shotId->Get_String();
		const DATA_JSON_VALUE* sequenceId =
			Required(value, "sequenceInstanceId", DATA_JSON_TYPE::STRING);
		if (nullptr == sequenceId ||
			(!sequenceId->Get_String().empty() &&
				!Is_StableId(sequenceId->Get_String())))
		{
			outStatus = "KoukuSaton camera shot sequence binding is invalid: " +
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
			outStatus = "KoukuSaton camera shot values are out of range: " +
				shot.strShotId;
			return false;
		}
		shot.fYawDegrees = static_cast<f32_t>(yaw->Get_Number());
		shot.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
		/* A pose whose eye sits on its own target has no direction, and the
		   engine would reject it every frame. Refuse it at load instead. */
		const float3_t forward(
			shot.vLookAt.x - shot.vEye.x,
			shot.vLookAt.y - shot.vEye.y,
			shot.vLookAt.z - shot.vEye.z);
		if (forward.x * forward.x + forward.y * forward.y +
			forward.z * forward.z <= 0.000001f)
		{
			outStatus = "KoukuSaton camera shot eye and lookAt coincide: " +
				shot.strShotId;
			return false;
		}
		stagedShots.push_back(std::move(shot));
	}

	m_CameraShots = std::move(stagedShots);
	outStatus = "KoukuSaton camera shots loaded: " +
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
