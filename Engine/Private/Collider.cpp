#include "Collider.h"

#include "GameInstance.h"

CCollider::CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CCollider::~CCollider()
{
}

HRESULT CCollider::Initialize_Prototype(COLLIDER eType)
{
	m_eType = eType;

#ifdef _DEBUG
	m_pBatch = make_shared<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
	m_pEffect = make_shared<BasicEffect>(m_pDevice.Get());
	m_pEffect->SetVertexColorEnabled(true);

	const void* pVertexShaderByteCode = { nullptr };
	size_t iLength = {};

	m_pEffect->GetVertexShaderBytecode(&pVertexShaderByteCode, &iLength);

	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pVertexShaderByteCode, iLength, &m_pInputLayout)))
		return E_FAIL;
	
#endif

    return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
	auto		pDesc = static_cast<CBounding::BOUNDING_DESC*>(pArg);

	switch (m_eType)
	{
	case COLLIDER::AABB:
		m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::OBB:
		m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, pDesc);
		break;
	case COLLIDER::SPHERE:
		m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, pDesc);
		break;

	}

    return S_OK;
}

void CCollider::Update(fmatrix_t WorldMatrix)
{
	m_pBounding->Update(WorldMatrix);
}

bool_t CCollider::Intersect(shared_ptr<CCollider> pTargetCollider)
{

	return m_isColl = m_pBounding->Intersect(pTargetCollider->m_eType, pTargetCollider->m_pBounding);
}

#ifdef _DEBUG

HRESULT CCollider::Render()
{
	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	m_pEffect->SetProjection(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(m_pInputLayout.Get());

	m_pEffect->Apply(m_pContext.Get());

	m_pBatch->Begin();

	m_pBounding->Render(m_pBatch);

	m_pBatch->End();

	return S_OK;
}

#endif

unique_ptr<CCollider> CCollider::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eType)
{
	auto pInstance = unique_ptr<CCollider>(new CCollider(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(eType)))
	{
		MSG_BOX("Failed to Created : CCollider");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CCollider::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCollider>(new CCollider(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCollider");
		return nullptr;
	}

	return pInstance;
}
