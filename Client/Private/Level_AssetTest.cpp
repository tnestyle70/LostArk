#include "Level_AssetTest.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "Logic_LanceMaster.h"
#include "MapNavigationContract.h"
#include "Transform.h"
#include "Valtan.h"

CLevel_AssetTest::CLevel_AssetTest(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_AssetTest::~CLevel_AssetTest()
{
}

HRESULT CLevel_AssetTest::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Character()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;
	if (FAILED(Ready_Valtan()))
		return E_FAIL;
	return S_OK;
}

void CLevel_AssetTest::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
#ifdef _DEBUG
	Update_NavigationDebug();
#endif
	Update_ClickMove();
}

HRESULT CLevel_AssetTest::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("Valtan WModel Asset Test"));
#endif
	return S_OK;
}

HRESULT CLevel_AssetTest::Ready_Lights()
{
	LIGHT_DESC light{};
	light.eType = LIGHT::DIRECTIONAL;
	light.vDirection = float4_t(0.5f, -1.f, 0.5f, 0.f);
	light.vDiffuse = float4_t(0.8f, 0.8f, 0.8f, 1.f);
	light.vAmbient = float4_t(0.35f, 0.35f, 0.35f, 1.f);
	light.vSpecular = float4_t(0.5f, 0.5f, 0.5f, 1.f);
	return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_AssetTest::Ready_Layer_Camera(const wstring_t& strLayerTag)
{
	if (nullptr == m_pCharacter)
		return E_FAIL;

	const shared_ptr<CTransform> playerTransform =
		m_pCharacter->Get_Transform();

	if (nullptr == playerTransform)
		return E_FAIL;

	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};

	const vector_t playerPosition =
		playerTransform->Get_State(STATE::POSITION);
	XMStoreFloat3(
		&CameraDesc.vEye,
		playerPosition + XMVectorSet(0.4f, 7.5f, 4.5f, 0.f));
	XMStoreFloat3(
		&CameraDesc.vAt,
		playerPosition + XMVectorSet(0.f, 1.2f, 0.f, 0.f));
	CameraDesc.fFovy = 60.f;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = 90.f;
	CameraDesc.fMouseSensor = 0.1f;

	// 생성 인자는 최초 설정값일 뿐이다. 카메라는 Character가 아니라
	// Layer_Player가 소유한 Transform을 약한 참조로 추적한다.
	CameraDesc.pFollowTarget = playerTransform;
	CameraDesc.vPositionOffset = float3_t(0.4f, 7.5f, 4.5f);
	CameraDesc.vLookOffset = float3_t(0.f, 1.2f, 0.f);
	CameraDesc.fFollowResponse = 18.f;
	// Follow/Free 전환 시에도 이 카메라 인스턴스 하나를 계속 사용한다.
	CameraDesc.isFollowEnabled = true;

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::ASSET_TEST), 
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::ASSET_TEST), strLayerTag, &CameraDesc, &pGameObject)))
		return E_FAIL;

	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(pGameObject);
	if (nullptr == camera)
		return E_FAIL;

	// 실제 수명은 Layer_Camera가 소유한다. Level은 대상 교체용 약한 핸들만 보관한다.
	m_pCamera = camera;

	return S_OK;
}

HRESULT CLevel_AssetTest::Ready_Character()
{
	MAP_NAVIGATION_CONTRACT navigationContract;
	std::string navigationStatus;
	const bool_t navigationReady =
		CMapNavigationContract::Resolve_Active(
			navigationContract, navigationStatus) &&
		navigationContract.runtimeGridAvailable;

	CCharacter::CHARACTER_DESC desc{};
	desc.iPrototypeLevelIndex = ETOUI(LEVEL::ASSET_TEST);
	desc.pSpec = &Spec_LanceMaster;
	desc.pNavigationPrototypeTag = navigationReady ?
		navigationContract.prototypeTag.c_str() : nullptr;
	desc.fSpeedPerSec = 6.f;
	desc.fRotationPerSec = 180.f;
	desc.vPosition = float3_t(151.25f, 22.96835f, -121.75f);

	shared_ptr<CGameObject> pGameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Layer_Player"),
		&desc,
		&pGameObject)))
		return E_FAIL;

	m_pCharacter = dynamic_pointer_cast<CCharacter>(pGameObject);
	return nullptr != m_pCharacter ? S_OK : E_FAIL;
}

HRESULT CLevel_AssetTest::Ready_Valtan()
{
	if (nullptr == m_pCharacter)
		return E_FAIL;

	MAP_NAVIGATION_CONTRACT navigationContract;
	std::string navigationStatus;
	const bool_t navigationReady =
		CMapNavigationContract::Resolve_Active(
			navigationContract, navigationStatus) &&
		navigationContract.runtimeGridAvailable;

	CValtan::VALTAN_DESC desc{};
	desc.fSpeedPerSec = 5.f;
	desc.fRotationPerSec = 180.f;
	desc.pNavigationPrototypeTag = navigationReady ?
		navigationContract.prototypeTag.c_str() : nullptr;
	desc.pTargetTransform = m_pCharacter->Get_Transform();
	desc.vPosition = float3_t(156.25f, 22.99751f, -121.75f);

	shared_ptr<CGameObject> pGameObject;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Valtan"),
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Layer_Valtan"),
		&desc,
		&pGameObject)))
		return E_FAIL;

	m_pValtan = dynamic_pointer_cast<CValtan>(pGameObject);

	return nullptr != m_pValtan ? S_OK : E_FAIL;
}

bool_t CLevel_AssetTest::Bind_CameraToCharacter(
	const shared_ptr<CCharacter>& pCharacter)
{
	// Layer에서 카메라가 제거되었을 수 있으므로 사용할 때만 강한 참조를 얻는다.
	const shared_ptr<CCamera_Free> camera = m_pCamera.lock();
	if (nullptr == camera)
		return false;

	// nullptr는 추적 대상 해제 요청으로 취급한다.
	if (nullptr == pCharacter)
	{
		camera->Set_FollowTarget(nullptr);
		camera->Set_FollowEnabled(false);
		return true;
	}
	// 카메라는 Character 전체가 아니라 위치와 방향을 제공하는 Transform만 추적한다.
	const shared_ptr<CTransform> targetTransform =
		pCharacter->Get_Transform();

	if (nullptr == targetTransform)
		return false;

	camera->Set_FollowTarget(targetTransform);
	camera->Set_FollowEnabled(true);

	return true;
}

void CLevel_AssetTest::Update_ClickMove()
{
	const bool_t isRightMouseDown =
		false == CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);

	if (isRightMouseDown && false == m_bRightMouseDown &&
		nullptr != m_pCharacter)
	{
		float4_t vPickedPosition{};
		if (CGameInstance::Get().Picking(vPickedPosition))
			m_pCharacter->Request_Move(XMLoadFloat4(&vPickedPosition));
	}

	m_bRightMouseDown = isRightMouseDown;
}

#ifdef _DEBUG

void CLevel_AssetTest::Update_NavigationDebug()
{
	if (CGameInstance::Get().Get_DIKeyPressed(DIK_F5))
	{
		m_bNavigationDebugVisible = !m_bNavigationDebugVisible;
		if (nullptr != m_pCharacter)
			m_pCharacter->Set_NavigationDebugVisible(
				m_bNavigationDebugVisible);
		if (nullptr != m_pValtan)
			m_pValtan->Set_NavigationDebugVisible(
				m_bNavigationDebugVisible);
	}
}

#endif

unique_ptr<CLevel_AssetTest> CLevel_AssetTest::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_AssetTest>(new CLevel_AssetTest(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}
