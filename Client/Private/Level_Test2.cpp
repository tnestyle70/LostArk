#include "Level_Test2.h"

#include "Camera_Free.h"
#include "GameInstance.h"
#include "LanceMaster.h"

CLevel_Test2::CLevel_Test2(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{}

CLevel_Test2::~CLevel_Test2()
{}

HRESULT CLevel_Test2::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;
	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;
	return S_OK;
}

void CLevel_Test2::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_Test2::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Test2::Ready_Lights()
{
	LIGHT_DESC light{};
	light.eType = LIGHT::DIRECTIONAL;
	light.vDirection = float4_t(0.5f, -1.f, 0.5f, 0.f);
	light.vDiffuse = float4_t(0.8f, 0.8f, 0.8f, 1.f);
	light.vAmbient = float4_t(0.35f, 0.35f, 0.35f, 1.f);
	light.vSpecular = float4_t(0.5f, 0.5f, 0.5f, 1.f);
	return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_Test2::Ready_Layer_Camera(const wstring_t& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	CameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	CameraDesc.fFovy = 60.f;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = 90.f;
	CameraDesc.fMouseSensor = 0.1f;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::TEST_LEVEL2), strLayerTag, &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Test2::Ready_Layer_Player(const wstring_t& strLayerTag)
{
	CLanceMaster::LANCEMASTER_DESC		LanceMasterDesc{};
	LanceMasterDesc.vPosition = float3_t(0.f, 0.f, 0.f);

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_GameObject_LanceMaster"),
		ETOUI(LEVEL::TEST_LEVEL2), strLayerTag, &LanceMasterDesc)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CLevel_Test2> CLevel_Test2::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Test2>(new CLevel_Test2(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}
