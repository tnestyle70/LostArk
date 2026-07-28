#include "Component.h"

CComponent::CComponent(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CPrototype { pDevice, pContext }
{
}

CComponent::~CComponent()
{
}

HRESULT CComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CComponent::Initialize(void* pArg)
{
	return S_OK; 
}
