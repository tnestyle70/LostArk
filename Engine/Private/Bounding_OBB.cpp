#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "debugDraw.h"

CBounding_OBB::CBounding_OBB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_OBB::~CBounding_OBB()
{
}

HRESULT CBounding_OBB::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_OBB::BOUNDING_OBB_DESC*>(pArg);

	float4_t	vQuaternion = {};

	XMStoreFloat4(&vQuaternion,
		XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vDegree.x), XMConvertToRadians(pDesc->vDegree.y), XMConvertToRadians(pDesc->vDegree.z)));


	m_pOriginalDesc = make_shared<BoundingOrientedBox>(pDesc->vCenter, float3_t(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f), vQuaternion);
	m_pDesc = make_shared<BoundingOrientedBox>(*m_pOriginalDesc);

	return S_OK;
}
void CBounding_OBB::Update(fmatrix_t WorldMatrix)
{

	m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);

}
bool_t CBounding_OBB::Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding)
{
	m_isColl = false;

	switch (eTargetType)
	{
	case COLLIDER::AABB:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_AABB>(pTargetBounding)->Get_Desc());
		break;
	case COLLIDER::OBB:
		// m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_OBB>(pTargetBounding)->Get_Desc());
		m_isColl = Intersect(static_pointer_cast<CBounding_OBB>(pTargetBounding));
		break;
	case COLLIDER::SPHERE:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_Sphere>(pTargetBounding)->Get_Desc());
		break;
	}

	return m_isColl;
}
#ifdef _DEBUG

HRESULT CBounding_OBB::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

CBounding_OBB::OBB_DESC CBounding_OBB::Compute_OBBDesc()
{
	OBB_DESC			OBBDesc{};

	float3_t		vPoints[8] = {};
	m_pDesc->GetCorners(vPoints);

	OBBDesc.vCenter = m_pDesc->Center;
	XMStoreFloat3(&OBBDesc.vCenterDir[0],
		(XMLoadFloat3(&vPoints[5]) - XMLoadFloat3(&vPoints[4])) * 0.5f);
	XMStoreFloat3(&OBBDesc.vCenterDir[1],
		(XMLoadFloat3(&vPoints[7]) - XMLoadFloat3(&vPoints[4])) * 0.5f);
	XMStoreFloat3(&OBBDesc.vCenterDir[2],
		(XMLoadFloat3(&vPoints[0]) - XMLoadFloat3(&vPoints[4])) * 0.5f);

	for (uint32_t i = 0; i < 3; i++)
	{
		XMStoreFloat3(&OBBDesc.vAlignDir[i],
			XMVector3Normalize(XMLoadFloat3(&OBBDesc.vCenterDir[i])));
	}


	return OBBDesc;
}

bool_t CBounding_OBB::Intersect(shared_ptr<CBounding_OBB> pTarget)
{
	OBB_DESC			OBBDesc[2] = {
		Compute_OBBDesc(), 
		pTarget->Compute_OBBDesc()
	};

	f32_t			fLength[3] = {};

	for (uint32_t i = 0; i < 2; i++)
	{
		for (uint32_t j = 0; j < 3; j++)
		{
			fLength[0] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenter) - XMLoadFloat3(&OBBDesc[0].vCenter),
				XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			fLength[1] =
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[0]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[1]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[2]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			fLength[2] =
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[0]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[1]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[2]),
					XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));


			if (fLength[0] > fLength[1] + fLength[2])
				return false;
		}
	}

	return true;
}

shared_ptr<CBounding_OBB> CBounding_OBB::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_OBB>(new CBounding_OBB(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_OBB");
		return nullptr;
	}

	return pInstance;
}
