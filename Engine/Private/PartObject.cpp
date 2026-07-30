#include "PartObject.h"

#include "GameInstance.h"

CPartObject::CPartObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CPartObject::~CPartObject()
{
}

HRESULT CPartObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPartObject::Initialize(void* pArg)
{
	auto		pDesc = static_cast<PARTOBJECT_DESC*>(pArg);
	m_pParentMatrix = pDesc->pParentMatrix;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CPartObject::Priority_Update(f32_t fTimeDelta)
{
}

void CPartObject::Update(f32_t fTimeDelta)
{
}

void CPartObject::Late_Update(f32_t fTimeDelta)
{
}

HRESULT CPartObject::Render()
{
	return S_OK;
}

HRESULT CPartObject::Update_CombinedWorldMatrix(fmatrix_t ChildMatrix)
{
	XMStoreFloat4x4(&m_CombinedWorldMatrix, ChildMatrix * XMLoadFloat4x4(m_pParentMatrix));

	return S_OK;
}

HRESULT CPartObject::Bind_WorldMatrix(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_CombinedWorldMatrix);	
}
