#include "Level_AssetTest.h"

#include "Camera_Free.h"
#include "GameInstance.h"

CLevel_AssetTest::CLevel_AssetTest(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{}

CLevel_AssetTest::~CLevel_AssetTest()
{}

HRESULT CLevel_AssetTest::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;
	return S_OK;
}

void CLevel_AssetTest::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_AssetTest::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

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
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	CameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	CameraDesc.fFovy = 60.f;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = 90.f;
	CameraDesc.fMouseSensor = 0.1f;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::ASSET_TEST), strLayerTag, &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CLevel_AssetTest> CLevel_AssetTest::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_AssetTest>(new CLevel_AssetTest(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}
