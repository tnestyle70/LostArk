#pragma once

// 공통 descriptor의 local 중심과 종류별 변환/교차/표시 인터페이스를 상속한다.
#include "Bounding.h"

NS_BEGIN(Engine)

// CCollider의 형상 구현이다. 로컬 원본과 변환된 현재 형상을 따로 소유하여 프레임마다 변환이 누적되지 않게 한다.
class CBounding_AABB final : public CBounding
{
public:
	typedef struct tagBoundingAABBDesc : public CBounding::BOUNDING_DESC
	{
		// 각 축의 전체 길이다. DirectX 형상에는 절반 길이 Extents로 저장한다.
		float3_t		vSize;
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_AABB();

public:
	// 현재 변환 형상을 공유 참조로 반환한다. 복사 snapshot이나 const pointee가 아니므로 외부 변경도 같은 형상에 반영된다.
	shared_ptr<BoundingBox> Get_Desc() const
	{
		return m_pDesc;
	}

public:
	// 선택한 파생 descriptor가 non-null이고 올바른 크기/회전을 가졌다는 전제로 값을 복사한다.
	// 이 AABB descriptor에서 복사하는 값은 중심과 축별 크기이며, 별도 회전 입력은 없다.
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg) override;
	// 행렬의 회전을 버리고 축별 길이와 translation만 적용한다. 회전한 물체 전체를 감싼 최소 AABB 계산과 다르다.
	virtual void Update(fmatrix_t WorldMatrix)  override;
	// 대상 종류에 맞춰 현재 형상끼리 비교하고 자기 충돌 색상만 갱신한다. 대상의 마지막 결과는 바꾸지 않는다.
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) override;
public:
	// 현재 월드 상자를 마지막 교차 결과 색상으로 제출한다. 유효한 Batch와 형상 준비는 호출자 전제다.
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch);

private:
	// OriginalDesc는 로컬 정의, Desc는 Update가 덮어쓰는 월드 판정/표시 값이다.
	shared_ptr<BoundingBox>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingBox>			m_pDesc = { nullptr };

private:
	// AABB끼리의 교차 검사에서 현재 중심±반길이로 축별 닫힌 구간 양끝을 계산한다. 형상은 변경하지 않는다.
	float3_t Compute_Min();
	float3_t Compute_Max();

	// 두 AABB의 x/y/z 구간을 비교하는 내부 helper다. 결과를 멤버에 저장하는 일은 위 종류별 진입점이 한다.
	bool_t Intersect(shared_ptr<CBounding_AABB> pTarget);

public:
	// Collider가 이 파생 타입으로 생성한 공유 소유권을 받는다. 입력 descriptor는 초기화 동안만 사용한다.
	static shared_ptr<CBounding_AABB> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const CBounding::BOUNDING_DESC* pArg);

};

NS_END