#include "Level_KakulSaydonArena.h"

#include "Camera_Free.h"
#include "Character.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unordered_set>

namespace
{
	constexpr std::string_view KAKULSAYDON_AREA_ID =
		"LV_LUT_MIDNIGHTC_ED";
	constexpr std::string_view STAGE_MARKER_SCHEMA =
		"lostark.kakul-stage-markers-runtime";
	constexpr std::string_view STAGE_SEMANTIC_STATUS =
		"SOURCE_LEVEL_ID_ONLY";

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
	std::string stageStatus;
	if (!Load_StageMarkers(stageStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] " + stageStatus + "\n").c_str());
		return E_FAIL;
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
			"KakulSaydon replication observed a disconnected Server session.");
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
		outStatus = "Kakul StageMarkers document is missing or exceeds 256 KiB.";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Kakul StageMarkers document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "Kakul StageMarkers document could not be read completely.";
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
		outStatus = "Kakul StageMarkers root is invalid: " + parseError;
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
		outStatus = "Kakul StageMarkers header is invalid.";
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
			outStatus = "Kakul StageMarkers stage has unexpected properties.";
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
			outStatus = "Kakul StageMarkers stage identity or evidence is invalid.";
			return false;
		}
		stagedMarkers.push_back({
			stageId->Get_String(), placementId->Get_String(),
			displayName->Get_String(), sourceLevelId->Get_String() });
	}

	m_StageMarkers = std::move(stagedMarkers);
	m_StageMarkerPlacementIds = std::move(stagedPlacementIds);
	outStatus = "Kakul StageMarkers loaded.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport(
	const std::uint32_t requestSequence,
	const std::string_view placementId,
	std::string& outStatus)
{
	if (0u == requestSequence || placementId.empty())
	{
		outStatus = "Kakul stage teleport request identity is invalid.";
		return false;
	}
	if (m_StageMarkerPlacementIds.empty())
	{
		outStatus = "Kakul StageMarkers are not authored; teleport is isolated.";
		return false;
	}
	if (!m_StageMarkerPlacementIds.contains(std::string(placementId)))
	{
		outStatus = "Kakul stage marker placement ID is not authored.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_StageTeleport(
			requestSequence, placementId))
	{
		outStatus = "Kakul stage teleport command was rejected.";
		return false;
	}
	outStatus = "Kakul stage teleport command submitted.";
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
	cameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
	cameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
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
	m_pCamera->Set_PositionOffset(float3_t(0.4f, 7.5f, 4.5f));
	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);
	return true;
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
