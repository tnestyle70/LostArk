// AABB 대상의 실제 파생 타입과 현재 BoundingBox를 교차 검사에서 사용한다.
#include "Bounding_AABB.h"
// Collider에서 시작한 Batch에 debug 도형 정점을 제출하는 실제 타입을 사용한다.
#include "DirectXTK/PrimitiveBatch.h"
// debug 도형 Batch의 위치/색상 정점 형식을 사용한다.
#include "DirectXTK/VertexTypes.h"
// OBB 대상의 실제 파생 타입과 현재 BoundingOrientedBox를 교차 검사에서 사용한다.
#include "Bounding_OBB.h"
// 구 대상의 실제 파생 타입과 현재 BoundingSphere를 교차 검사에서 사용한다.
#include "Bounding_Sphere.h"
// 현재 형상을 위치/색상 Batch 정점으로 만드는 DX::Draw를 호출한다.
#include "debugDraw.h"

CBounding_AABB::CBounding_AABB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_AABB::~CBounding_AABB()
{
}

// CCollider::Initialize가 형상 종류에 맞는 descriptor를 넘긴다. 입력을 보관하지 않고 원본/현재 형상 값을 따로 만든다.
HRESULT CBounding_AABB::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_AABB::BOUNDING_AABB_DESC*>(pArg);

	m_pOriginalDesc = make_shared<BoundingBox>(
		pDesc->vCenter,
		float3_t(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f));
	m_pDesc = make_shared<BoundingBox>(*m_pOriginalDesc);

	return S_OK;
}
// WorldMatrix의 basis 행을 각 축 길이만 가진 양의 직교축으로 바꾼다. translation은 유지하고 회전/축 부호는 버린다.
// 따라서 중심 offset도 회전하지 않는다. 이 정책을 회전 OBB를 감싸는 AABB 생성과 혼동하지 않는다.
void CBounding_AABB::Update(fmatrix_t WorldMatrix)
{
	matrix_t		TransformMatrix = WorldMatrix;
	TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[0]);
	TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(TransformMatrix.r[1]);
	TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(TransformMatrix.r[2]);

	m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);

}
// 종류에 맞는 실제 대상 형상이 들어온다는 전제다. 지원하지 않는 enum은 초기화한 false를 반환한다.
bool_t CBounding_AABB::Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding)
{
	m_isColl = false;

	switch (eTargetType)
	{
	case COLLIDER::AABB:
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

// 마지막 자기 교차 결과가 true면 빨강, false면 초록으로 현재 월드 형상을 그린다. 이 호출에서 다시 교차 검사하지 않는다.
HRESULT CBounding_AABB::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

// 현재 중심 - 반길이가 축별 최소 좌표다. 아래 최대 좌표와 함께 겹치는 구간이 있는지 비교한다.
float3_t CBounding_AABB::Compute_Min()
{
	return float3_t(m_pDesc->Center.x - m_pDesc->Extents.x,
		m_pDesc->Center.y - m_pDesc->Extents.y,
		m_pDesc->Center.z - m_pDesc->Extents.z);
}

// 현재 중심에 반길이를 더해 각 축 구간의 최댓값을 얻는다. Min과 같은 월드 좌표 단위를 사용한다.
float3_t CBounding_AABB::Compute_Max()
{
	return float3_t(m_pDesc->Center.x + m_pDesc->Extents.x,
		m_pDesc->Center.y + m_pDesc->Extents.y,
		m_pDesc->Center.z + m_pDesc->Extents.z);
}

// x/y/z 중 하나라도 구간이 분리되면 즉시 false다. 엄격한 > 비교이므로 면/모서리가 닿는 경우는 true다.
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

// 후보를 shared_ptr로 소유하고 Initialize의 HRESULT 실패를 nullptr로 전달한다. 입력 오류/할당 예외를 모두 HRESULT로 바꾸는 함수는 아니다.
shared_ptr<CBounding_AABB> CBounding_AABB::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_AABB>(new CBounding_AABB(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_AABB");
		return nullptr;
	}

	return pInstance;
}
