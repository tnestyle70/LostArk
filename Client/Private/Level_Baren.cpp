#include "Level_Baren.h"

CLevel_Baren::CLevel_Baren(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{pDevice, pContext}
{}

CLevel_Baren::~CLevel_Baren()
{}

HRESULT CLevel_Baren::Initialize()
{
	return S_OK;
}

void CLevel_Baren::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_Baren::Render()
{
	return S_OK;
}

HRESULT CLevel_Baren::Ready_Lights()
{
	return S_OK;
}

HRESULT CLevel_Baren::Ready_Layer_Camera(const wstring_t& strLayerTag)
{
	return S_OK;
}

unique_ptr<CLevel_Baren> CLevel_Baren::Create(ComPtr<ID3D11Device> pDevice, 
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Baren>(new CLevel_Baren(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
		return nullptr;
	
	return pInstance;
}
