#include "Level_Bern.h"

#include "Camera_Free.h"
#include "Character.h"
#include "ClientLaunchOptions.h"
#include "GameInstance.h"
#include "LevelCatalog.h"
#include "Level_Loading.h"
#include "NetworkPlayerCommandSink.h"
#include "OfflinePlayerPreview.h"
#include "SceneTransitionService.h"
#include "Transform.h"

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

	const LEVEL_CATALOG_ENTRY* pEntry =
		CLevelCatalog::Find(CLIENT_SCENARIO::WORLD_BERN);
	if (nullptr == pEntry || pEntry->strMapAreaId.empty() ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::BERN),
			pEntry->strMapAreaId,
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

	if (CClientLaunchOptions::Get().isOfflinePreview)
	{
		std::string previewStatus;
		if (!COfflinePlayerPreview::Spawn(
			m_Replication,
			pEntry->strMapAreaId,
			previewStatus))
		{
			OutputDebugStringA(("[Level_Bern] " + previewStatus + "\n").c_str());
			m_MapRuntime.Clear();
			return E_FAIL;
		}
	}
	else
	{
		m_pPlayerCommandSink =
			make_shared<CNetworkPlayerCommandSink>();
		m_PlayerController.Set_CommandSink(
			m_pPlayerCommandSink);
	}

	return S_OK;
}

void CLevel_Bern::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Bern] Failed to apply replication event.\n");
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
		if (CSceneTransitionService::Request(
			LEVEL::LOBBY,
			CLIENT_SCENARIO::FRONT_LOBBY,
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

	m_PlayerController.Set_GameplayInputEnabled(
		!CClientLaunchOptions::Get().isOfflinePreview &&
		(nullptr == m_pCamera || m_pCamera->Is_FollowEnabled()));
	m_PlayerController.Update();

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
