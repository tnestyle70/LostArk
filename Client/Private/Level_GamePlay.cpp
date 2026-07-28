#include "Level_GamePlay.h"

#include "Camera_Free.h"
#include "GameInstance.h"

CLevel_GamePlay::CLevel_GamePlay(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_GamePlay::~CLevel_GamePlay()
{
}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_GamePlay::Update(f32_t fTimeDelta)
{

	__super::Update(fTimeDelta);		
}

HRESULT CLevel_GamePlay::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("게임플레이레벨입니다."));
#endif

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDirection = float4_t(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = float4_t(0.5f, 0.5f, 0.5f, 1.f);
	LightDesc.vAmbient = float4_t(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vSpecular = float4_t(1.f, 1.f, 1.f, 1.f);

	if (FAILED(CGameInstance::Get().Add_Light(LightDesc)))
		return E_FAIL;

	ZeroMemory(&LightDesc, sizeof LightDesc);
	LightDesc.eType = LIGHT::POINT;
	LightDesc.vPosition = float4_t(20.f, 5.f, 20.f, 1.f);
	LightDesc.fRange = 15.f;
	LightDesc.vDiffuse = float4_t(1.f, 0.7f, 0.f, 1.f);
	LightDesc.vAmbient = float4_t(0.4f, 0.1f, 0.1f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;

	if (FAILED(CGameInstance::Get().Add_Light(LightDesc)))
		return E_FAIL;

	ZeroMemory(&LightDesc, sizeof LightDesc);
	LightDesc.eType = LIGHT::POINT;
	LightDesc.vPosition = float4_t(40.f, 5.f, 20.f, 1.f);
	LightDesc.fRange = 15.f;
	LightDesc.vDiffuse = float4_t(1.f, 0.7f, 0.f, 1.f);
	LightDesc.vAmbient = float4_t(0.4f, 0.1f, 0.1f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;

	if (FAILED(CGameInstance::Get().Add_Light(LightDesc)))
		return E_FAIL;

	SHADOW_LIGHT_DESC		ShadowLightDesc{};
	ShadowLightDesc.vEye = float4_t(5.f, 35.f, 5.f, 1.f);
	ShadowLightDesc.vAt = float4_t(10.f, 0.f, 10.f, 1.f);
	ShadowLightDesc.fFovy = 90.f;
	ShadowLightDesc.fNear = 0.1f;
	ShadowLightDesc.fFar = 1000.f;

	if (FAILED(CGameInstance::Get().Add_Shadow_Light(ShadowLightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring_t& strLayerTag)
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

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::GAMEPLAY), strLayerTag, &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player(const wstring_t& strLayerTag)
{
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Player"),
		ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Monster(const wstring_t& strLayerTag)
{
	for (size_t i = 0; i < 10; i++)
	{
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Monster"),
			ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
			return E_FAIL;
	}
	

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring_t& strLayerTag)
{
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Terrain"),
		ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Sky"),
		ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Snow"),
		ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
		return E_FAIL;

	for (size_t i = 0; i < 10; i++)
	{
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_ForkLift"),
			ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
			return E_FAIL;
	}
	

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Effect(const wstring_t& strLayerTag)
{
	for (size_t i = 0; i < 100; i++)
	{
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Effect"),
			ETOUI(LEVEL::GAMEPLAY), strLayerTag)))
			return E_FAIL;
	}
	

	return S_OK;
}

unique_ptr<CLevel_GamePlay> CLevel_GamePlay::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_GamePlay>(new CLevel_GamePlay(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_GamePlay");
		return nullptr;
	}

	return pInstance;
}
