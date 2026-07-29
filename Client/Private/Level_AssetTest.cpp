#include "Level_AssetTest.h"

#include "BinaryAssetObject.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"

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
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	// Missing shared assets must leave the test level and F1 diagnostics available.
	Ready_BinaryAsset();
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

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("Binary Asset Test - F1 opens decoder diagnostics"));
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
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.vEye = float3_t(-5.f, 5.f, -5.f);
	CameraDesc.vAt = float3_t(0.f, 0.f, 0.f);
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

HRESULT CLevel_AssetTest::Ready_BinaryAsset()
{
	CBinaryAssetObject::BINARY_ASSET_DESC asset{};
	asset.asset.assetRoot = CRuntimeAssetRoot::Get();
	const filesystem::path characterRoot = CRuntimeAssetRoot::Resolve(L"Character/MN_RPBF_01");
	if (filesystem::exists(characterRoot / L"MN_RPBF_01.wmesh"))
	{
		asset.asset.meshPath = characterRoot / L"MN_RPBF_01.wmesh";
		asset.asset.materialPath = characterRoot / L"MN_RPBF_01.wmat";
		asset.asset.skeletonPath = characterRoot / L"MN_RPBF_01.wskel";
		asset.asset.defaultAnimationName = "idle_battle_1";

		const filesystem::path animationRoot = characterRoot / L"anims";
		if (filesystem::exists(animationRoot))
		{
			for (const filesystem::directory_entry& entry : filesystem::directory_iterator(animationRoot))
			{
				if (entry.is_regular_file() && L".wanim" == entry.path().extension())
					asset.asset.animationPaths.push_back(entry.path());
			}
			sort(asset.asset.animationPaths.begin(), asset.asset.animationPaths.end());
		}
		asset.scale = 0.01f;
	}
	else
	{
		const filesystem::path annieRoot = CRuntimeAssetRoot::Resolve(L"LoL/Annie");
		asset.asset.meshPath = annieRoot / L"annie.wmesh";
		asset.asset.materialPath = annieRoot / L"annie.wmat";
		asset.asset.fallbackDiffusePath = annieRoot / L"annie_base_2012_cm.dds";
		asset.scale = 0.02f;
	}
	asset.position = float3_t(0.f, 0.f, 0.f);

	return CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_BinaryAsset"),
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Layer_BinaryAsset"),
		&asset);
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
