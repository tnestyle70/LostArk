#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "debugDraw.h"

CBounding_AABB::CBounding_AABB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_AABB::~CBounding_AABB()
{
}

HRESULT CBounding_AABB::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_AABB::BOUNDING_AABB_DESC*>(pArg);

	m_pOriginalDesc = make_shared<BoundingBox>(pDesc->vCenter, float3_t(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f));
	m_pDesc = make_shared<BoundingBox>(*m_pOriginalDesc);

	return S_OK;
}
void CBounding_AABB::Update(fmatrix_t WorldMatrix)
{
	matrix_t		TransformMatrix = WorldMatrix;
	TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[0]);
	TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[1]);
	TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(TransformMatrix.r[2]);

	m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);

}
bool_t CBounding_AABB::Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding)
{
	m_isColl = false;

	switch (eTargetType)
	{
	case COLLIDER::AABB:
		// m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_AABB>(pTargetBounding)->Get_Desc());		
		m_isColl = Intersect(static_pointer_cast<CBounding_AABB>(pTargetBounding));
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

HRESULT CBounding_AABB::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

float3_t CBounding_AABB::Compute_Min()
{
	return float3_t(m_pDesc->Center.x - m_pDesc->Extents.x, 
		m_pDesc->Center.y - m_pDesc->Extents.y, 
		m_pDesc->Center.z - m_pDesc->Extents.z);
}

float3_t CBounding_AABB::Compute_Max()
{
	return float3_t(m_pDesc->Center.x + m_pDesc->Extents.x,
		m_pDesc->Center.y + m_pDesc->Extents.y,
		m_pDesc->Center.z + m_pDesc->Extents.z);
}

bool_t CBounding_AABB::Intersect(shared_ptr<CBounding_AABB> pTarget)
{
	float3_t		vSourMin = Compute_Min();
	float3_t		vDestMin = pTarget->Compute_Min();

	float3_t		vSourMax = Compute_Max();
	float3_t		vDestMax = pTarget->Compute_Max();

	if (max(vSourMin.x, vDestMin.x) > min(vSourMax.x, vDestMax.x))
		return false;
	if (max(vSourMin.y, vDestMin.y) > min(vSourMax.y, vDestMax.y))
		return false;
	if (max(vSourMin.z, vDestMin.z) > min(vSourMax.z, vDestMax.z))
		return false;

	return true;
}

shared_ptr<CBounding_AABB> CBounding_AABB::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_AABB>(new CBounding_AABB(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_AABB");
		return nullptr;
	}

	return pInstance;
}
