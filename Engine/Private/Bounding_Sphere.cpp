#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "debugDraw.h"

CBounding_Sphere::CBounding_Sphere(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_Sphere::~CBounding_Sphere()
{
}

HRESULT CBounding_Sphere::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_Sphere::BOUNDING_SPHERE_DESC*>(pArg);

	m_pOriginalDesc = make_shared<BoundingSphere>(pDesc->vCenter, pDesc->fRadius);
	m_pDesc = make_shared<BoundingSphere>(*m_pOriginalDesc);

	return S_OK;
}
void CBounding_Sphere::Update(fmatrix_t WorldMatrix)
{
	
	m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);

}
bool_t CBounding_Sphere::Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding)
{
	m_isColl = false;

	switch (eTargetType)
	{
	case COLLIDER::AABB:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_AABB>(pTargetBounding)->Get_Desc());
		break;
	case COLLIDER::OBB:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_OBB>(pTargetBounding)->Get_Desc());
		break;
	case COLLIDER::SPHERE:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_Sphere>(pTargetBounding)->Get_Desc());
		break;
	}

	return m_isColl;
}
#ifdef _DEBUG

HRESULT CBounding_Sphere::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

shared_ptr<CBounding_Sphere> CBounding_Sphere::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_Sphere>(new CBounding_Sphere(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_Sphere");
		return nullptr;
	}

	return pInstance;
}
