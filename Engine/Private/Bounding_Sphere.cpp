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

CBounding_Sphere::CBounding_Sphere(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_Sphere::~CBounding_Sphere()
{
}

// CCollider::Initialize가 형상 종류에 맞는 descriptor를 넘긴다. 입력을 보관하지 않고 원본/현재 형상 값을 따로 만든다.
HRESULT CBounding_Sphere::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_Sphere::BOUNDING_SPHERE_DESC*>(pArg);

	m_pOriginalDesc = make_shared<BoundingSphere>(pDesc->vCenter, pDesc->fRadius);
	m_pDesc = make_shared<BoundingSphere>(*m_pOriginalDesc);

	return S_OK;
}
// 로컬 원본에서 월드 구를 다시 계산한다. 변환된 중심/반지름은 이후 교차 검사와 debug 표시가 함께 읽는다.
void CBounding_Sphere::Update(fmatrix_t WorldMatrix)
{

	m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);

}
// 종류에 맞는 실제 대상 형상이 들어온다는 전제다. 지원하지 않는 enum은 초기화한 false를 반환한다.
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

// 마지막 자기 교차 결과가 true면 빨강, false면 초록으로 현재 월드 형상을 그린다. 이 호출에서 다시 교차 검사하지 않는다.
HRESULT CBounding_Sphere::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

// 후보를 shared_ptr로 소유하고 Initialize의 HRESULT 실패를 nullptr로 전달한다. 입력 오류/할당 예외를 모두 HRESULT로 바꾸는 함수는 아니다.
shared_ptr<CBounding_Sphere> CBounding_Sphere::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_Sphere>(new CBounding_Sphere(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_Sphere");
		return nullptr;
	}

	return pInstance;
}
