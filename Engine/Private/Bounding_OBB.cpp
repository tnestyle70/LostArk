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

CBounding_OBB::CBounding_OBB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CBounding { pDevice, pContext }
{
}

CBounding_OBB::~CBounding_OBB()
{
}

// CCollider::Initialize가 형상 종류에 맞는 descriptor를 넘긴다. 입력을 보관하지 않고 원본/현재 형상 값을 따로 만든다.
HRESULT CBounding_OBB::Initialize(const CBounding::BOUNDING_DESC* pArg)
{
	auto		pDesc = static_cast<const CBounding_OBB::BOUNDING_OBB_DESC*>(pArg);

	float4_t	vQuaternion = {};

	XMStoreFloat4(&vQuaternion,
		XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(pDesc->vDegree.x),
			XMConvertToRadians(pDesc->vDegree.y),
			XMConvertToRadians(pDesc->vDegree.z)));

	m_pOriginalDesc = make_shared<BoundingOrientedBox>(
		pDesc->vCenter,
		float3_t(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f),
		vQuaternion);
	m_pDesc = make_shared<BoundingOrientedBox>(*m_pOriginalDesc);

	return S_OK;
}
// 로컬 OBB 원본에 월드 행렬을 적용한다. 결과는 교차 검사와 debug draw가 함께 소비한다.
void CBounding_OBB::Update(fmatrix_t WorldMatrix)
{

	m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);

}
// 종류에 맞는 실제 대상 형상이 들어온다는 전제다. 지원하지 않는 enum은 초기화한 false를 반환한다.
bool_t CBounding_OBB::Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding)
{
	m_isColl = false;

	switch (eTargetType)
	{
	case COLLIDER::AABB:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_AABB>(pTargetBounding)->Get_Desc());
		break;
	case COLLIDER::OBB:
		m_isColl = Intersect(static_pointer_cast<CBounding_OBB>(pTargetBounding));
		break;
	case COLLIDER::SPHERE:
		m_isColl = m_pDesc->Intersects(*static_pointer_cast<CBounding_Sphere>(pTargetBounding)->Get_Desc());
		break;
	}

	return m_isColl;
}

// 마지막 자기 교차 결과가 true면 빨강, false면 초록으로 현재 월드 형상을 그린다. 이 호출에서 다시 교차 검사하지 않는다.
HRESULT CBounding_OBB::Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch)
{
	DX::Draw(pBatch.get(), *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

// DirectX 모서리 순서에서 같은 꼭짓점에 연결된 세 변을 구해 반으로 줄인다. 그 길이는 투영 반경 계산에 남긴다.
// 크기 0인 축을 Normalize하는 경우를 별도로 처리하지 않으므로 유효한 형상 크기가 필요하다.
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

// 각 상자의 세 축을 후보 분리 축으로 사용한다. 축 투영에서 중심 간격이 두 반경 합보다 크면 분리다.
// 현재는 모서리끼리 외적한 9개 축이 없어 일부 분리된 3D OBB를 겹침으로 볼 수 있다. 이번 정비는 이 계산을 바꾸지 않는다.
bool_t CBounding_OBB::Intersect(shared_ptr<CBounding_OBB> pTarget)
{
	OBB_DESC			OBBDesc[2] = {
		Compute_OBBDesc(),
		pTarget->Compute_OBBDesc()
	};

	// [0]은 중심 차이의 절대 투영 길이, [1]/[2]는 두 상자의 투영 반경이다. 모두 현재 월드 좌표의 길이 단위다.
	f32_t			fLength[3] = {};

	for (uint32_t i = 0; i < 2; i++)
	{
		for (uint32_t j = 0; j < 3; j++)
		{
			fLength[0] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenter) - XMLoadFloat3(&OBBDesc[0].vCenter),
				XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			// 세 반모서리 벡터를 검사 축에 투영한 절댓값의 합이 상자 한쪽의 투영 반경이다.
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

// 후보를 shared_ptr로 소유하고 Initialize의 HRESULT 실패를 nullptr로 전달한다. 입력 오류/할당 예외를 모두 HRESULT로 바꾸는 함수는 아니다.
shared_ptr<CBounding_OBB> CBounding_OBB::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const CBounding::BOUNDING_DESC* pArg)
{
	auto pInstance = shared_ptr<CBounding_OBB>(new CBounding_OBB(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CBounding_OBB");
		return nullptr;
	}

	return pInstance;
}
