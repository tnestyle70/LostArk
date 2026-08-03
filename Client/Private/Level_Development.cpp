#include "Level_Development.h"

#include "AnimationTargetService.h"
#include "Camera_Free.h"
#include "Character.h"
#include "ClientLaunchOptions.h"
#include "GameInstance.h"
#include "LevelCatalog.h"
#include "Logic_LanceMaster.h"
#include "NetworkPlayerCommandSink.h"
#include "OfflinePlayerPreview.h"
#include "SceneTransitionService.h"
#include "Transform.h"

CLevel_Development::CLevel_Development(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_Development::~CLevel_Development()
{
	CAnimationTargetService::Unbind(m_pCharacter);
}

HRESULT CLevel_Development::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_SCENARIO scenario =
		CClientLaunchOptions::Get().eScenario;
	const LEVEL_CATALOG_ENTRY* pEntry =
		CLevelCatalog::Find(scenario);
	if (nullptr == pEntry ||
		pEntry->eLevel != LEVEL::DEVELOPMENT)
	{
		return E_INVALIDARG;
	}
	m_isNetworkTraining =
		CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND == scenario;
	if (CLIENT_SCENARIO::DEVELOPMENT_MAP == scenario || m_isNetworkTraining)
	{
		if (pEntry->strMapAreaId.empty() ||
			!m_MapRuntime.Load_Area(
				ETOUI(LEVEL::DEVELOPMENT),
				pEntry->strMapAreaId,
				pEntry->MapLoadScope))
		{
			OutputDebugStringA((
				"[Level_Development] " +
				m_MapRuntime.Get_Status() +
				"\n").c_str());
			return E_FAIL;
		}
	}
	else if (CLIENT_SCENARIO::DEVELOPMENT_CHARACTER == scenario)
	{
		if (FAILED(Ready_Character()))
			return E_FAIL;
	}

	if (FAILED(Ready_Lights()) ||
		FAILED(Ready_Camera(TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	if (m_isNetworkTraining)
	{
		CClientReplication::DESC replicationDesc{};
		replicationDesc.pDevice = m_pDevice;
		replicationDesc.pContext = m_pContext;
		replicationDesc.iPrototypeLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
		replicationDesc.iLayerLevelIndex = ETOUI(LEVEL::DEVELOPMENT);
		replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
		replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
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
				OutputDebugStringA((
					"[Level_Development] " + previewStatus + "\n").c_str());
				m_MapRuntime.Clear();
				return E_FAIL;
			}
		}
		else
		{
			m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
			m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
		}
	}

	return S_OK;
}

void CLevel_Development::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (m_isNetworkTraining)
	{
		if (!m_Replication.Update())
			OutputDebugStringA(
				"[Level_Development] Failed to apply training replication.\n");
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
				"[Level_Development] Lobby recovery request was rejected; retrying.\n");
		}
		Bind_CameraToLocalCharacter();
		const shared_ptr<CCharacter> localCharacter =
			m_Replication.Get_LocalCharacter();
		m_PlayerController.Set_LocalCharacter(localCharacter);
		const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
		m_PlayerController.Set_GameplayInputEnabled(
			!CClientLaunchOptions::Get().isOfflinePreview &&
			(nullptr == camera || camera->Is_FollowEnabled()));
		m_PlayerController.Update();
	}
	else
	{
		Update_ClickMove();
	}
}

HRESULT CLevel_Development::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("LostArk Development Lab"));
#endif
	return S_OK;
}

HRESULT CLevel_Development::Ready_Lights()
{
	return CMapPlacementRuntime::Ensure_DefaultLight();
}

HRESULT CLevel_Development::Ready_Camera(
	const wstring_t& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	cameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 4000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	if (nullptr != m_pCharacter)
	{
		const shared_ptr<CTransform> pTransform =
			m_pCharacter->Get_Transform();
		cameraDesc.pFollowTarget = pTransform;
		cameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
		cameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
		cameraDesc.fFollowResponse = 18.f;
		cameraDesc.isFollowEnabled = nullptr != pTransform;
	}

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::DEVELOPMENT),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::DEVELOPMENT),
		strLayerTag,
		&cameraDesc,
		&pGameObject)))
	{
		return E_FAIL;
	}

	const shared_ptr<CCamera_Free> pCamera =
		dynamic_pointer_cast<CCamera_Free>(pGameObject);
	if (nullptr == pCamera)
		return E_FAIL;

	m_pCamera = pCamera;
	return S_OK;
}

HRESULT CLevel_Development::Ready_Character()
{
	CCharacter::CHARACTER_DESC characterDesc{};
	characterDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::DEVELOPMENT);
	characterDesc.pSpec = &Spec_LanceMaster;
	characterDesc.vPosition = float3_t(0.f, 0.f, 0.f);
	characterDesc.isLocallyControlled = true;

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::DEVELOPMENT),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::DEVELOPMENT),
		TEXT("Layer_Player"),
		&characterDesc,
		&pGameObject)))
	{
		return E_FAIL;
	}

	m_pCharacter = dynamic_pointer_cast<CCharacter>(pGameObject);
	if (nullptr == m_pCharacter)
		return E_FAIL;

	CAnimationTargetService::Bind(m_pCharacter);
	return S_OK;
}

void CLevel_Development::Update_ClickMove()
{
	if (nullptr == m_pCharacter)
		return;

	const bool_t isRightMouseDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr != camera && !camera->Is_FollowEnabled())
	{
		m_wasRightMouseDown = isRightMouseDown;
		return;
	}
	if (isRightMouseDown && !m_wasRightMouseDown)
	{
		float4_t pickedPosition{};
		if (CGameInstance::Get().Picking(pickedPosition))
			m_pCharacter->Request_Move(XMLoadFloat4(&pickedPosition));
	}
	m_wasRightMouseDown = isRightMouseDown;
}

bool_t CLevel_Development::Bind_CameraToLocalCharacter()
{
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr == camera)
		return false;
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		camera->Set_FollowTarget(nullptr);
		camera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;
	m_pCameraTarget = localCharacter;
	camera->Set_PositionOffset(float3_t(0.4f, 7.5f, 4.5f));
	camera->Set_FollowTarget(transform);
	camera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_Development> CLevel_Development::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Development>(
		new CLevel_Development(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}
