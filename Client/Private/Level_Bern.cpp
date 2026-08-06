#include "Level_Bern.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "Transform.h"
#include "Trigger_Box.h"
#include "WorldGameplayDocument.h"

#include <algorithm>

CLevel_Bern::CLevel_Bern(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{}

CLevel_Bern::~CLevel_Bern()
{}

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

	if (FAILED(CMapPlacementRuntime::Ensure_DefaultLight()) ||
		FAILED(Ready_Layer_Camera(
			TEXT("Layer_Camera"))))
	{
		return E_FAIL;
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;

	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::BERN);

	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::BERN);

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

#ifdef _DEBUG
	if (!Ready_DebugLevelChangeTriggers(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_Bern] Debug changeLevel Trigger Box presentation failed.\n");
	}
#endif

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

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();

	m_PlayerController.Set_LocalCharacter(
		localCharacter);

	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled());

}

HRESULT CLevel_Bern::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(
		g_hWnd,
		TEXT("Bern Castle Network Player Test"));
#endif

	return S_OK;
}

HRESULT CLevel_Bern::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
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

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};

	/*
	 * Player가 Spawn되기 전까지는 기존 베른성 전체 맵 카메라다.
	 */
	cameraDesc.vEye = float3_t(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);

	cameraDesc.vAt = focus;

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

	cameraDesc.fFollowResponse = 18.f;
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
