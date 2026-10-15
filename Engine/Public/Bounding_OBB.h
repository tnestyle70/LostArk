#pragma once

// 공통 중심 descriptor와 변환/교차/표시 경계를 상속하고 DirectX 회전 상자를 소유한다.
#include "Bounding.h"

NS_BEGIN(Engine)

// CCollider의 형상 구현이다. 로컬 원본과 변환된 현재 형상을 따로 소유하여 프레임마다 변환이 누적되지 않게 한다.
class CBounding_OBB final : public CBounding
{
public:
	typedef struct tagBoundingOBBDesc : public CBounding::BOUNDING_DESC
	{
		// 각 축의 전체 길이다. DirectX 형상에는 절반 길이 Extents로 저장한다.
		float3_t		vSize;
		// x/y/z는 pitch/yaw/roll의 degree 입력이다. Initialize가 radian으로 변환하여 quaternion을 만든다.
		float3_t		vDegree;
	}BOUNDING_OBB_DESC;

	// 현재 월드 OBB를 비교할 임시 값이다. CenterDir은 반길이를 포함한 모서리 벡터, AlignDir은 그 단위 방향이다.
	typedef struct tagOBBDesc
	{
		float3_t		vCenter;
		float3_t		vCenterDir[3];
		float3_t		vAlignDir[3];
	}OBB_DESC;
private:
	CBounding_OBB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_OBB();

public:
	// 현재 변환 형상을 공유 참조로 반환한다. 복사 snapshot이나 const pointee가 아니므로 외부 변경도 같은 형상에 반영된다.
	shared_ptr<BoundingOrientedBox> Get_Desc() const
	{
		return m_pDesc;
	}

public:
	// 선택한 파생 descriptor가 non-null이고 올바른 크기/회전을 가졌다는 전제로 값을 복사한다.
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg) override;
	// 로컬 원본에 전달된 world 행렬을 적용해 현재 회전 상자를 덮어쓴다. 이전 프레임 변환을 누적하지 않는다.
	virtual void Update(fmatrix_t WorldMatrix)  override;
	// OBB끼리는 아래 6축 helper를 사용하고 다른 종류는 DirectX 교차 검사에 위임한다. 자기 마지막 결과만 저장한다.
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) override;
public:
	// 현재 회전 상자를 마지막 교차 결과 색상으로 제출한다. 그리기 호출이 충돌을 새로 판정하지 않는다.
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch);

private:
	// OriginalDesc는 로컬 정의, Desc는 Update가 덮어쓰는 월드 판정/표시 값이다.
	shared_ptr<BoundingOrientedBox>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingOrientedBox>			m_pDesc = { nullptr };

private:
	// 현재 상자의 모서리에서 반모서리 벡터와 단위 축을 지역 값으로 만든다. 아래 투영 반경 계산이 소비한다.
	OBB_DESC Compute_OBBDesc();
	// 현재 구현은 두 상자의 축 6개만 검사한다. 모서리 교차 축 9개를 생략하므로 일반 3D OBB의 완전한 SAT 검사가 아니다.
	bool_t Intersect(shared_ptr<CBounding_OBB> pTarget);
public:
	// Collider가 파생 형상과 로컬 정의의 공유 소유권을 받는다. descriptor 포인터는 결과에 보관하지 않는다.
	static shared_ptr<CBounding_OBB> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const CBounding::BOUNDING_DESC* pArg);

};

NS_END