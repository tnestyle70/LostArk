#include "Level.h"

CLevel::CLevel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CLevel::~CLevel()
{

}

HRESULT CLevel::Initialize()
{
	return S_OK;
}

void CLevel::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	[[maybe_unused]] volatile f32_t fDebugTimeDelta = fTimeDelta;
#endif
}

HRESULT CLevel::Render()
{
	return S_OK;
}
