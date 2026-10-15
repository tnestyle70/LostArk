#pragma once

// 공통 local 중심과 변환/교차/표시 인터페이스를 상속하고 DirectX 구 형상을 소유한다.
#include "Bounding.h"

NS_BEGIN(Engine)

// CCollider의 형상 구현이다. 로컬 원본과 변환된 현재 형상을 따로 소유하여 프레임마다 변환이 누적되지 않게 한다.
class CBounding_Sphere final : public CBounding
{
public:
	typedef struct tagBoundingSphereDesc : public CBounding::BOUNDING_DESC
	{
		// 로컬 반지름이다. 음수/NaN 등 수치 검증은 이 형상 생성자에서 수행하지 않는다.
		f32_t		fRadius;
	}BOUNDING_SPHERE_DESC;
private:
	CBounding_Sphere(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_Sphere();
public:
	// 현재 변환 형상을 공유 참조로 반환한다. 복사 snapshot이나 const pointee가 아니므로 외부 변경도 같은 형상에 반영된다.
	shared_ptr<BoundingSphere> Get_Desc() const
	{
		return m_pDesc;
	}
public:
	// 선택한 파생 descriptor가 non-null이고 올바른 크기/회전을 가졌다는 전제로 값을 복사한다.
	// 이 구 descriptor에서 복사하는 값은 중심과 반지름이며, 별도 회전 입력은 없다.
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg) override;
	// DirectX BoundingSphere::Transform으로 중심과 반지름을 월드로 변환한다.
	virtual void Update(fmatrix_t WorldMatrix)  override;
	// 대상의 현재 AABB/OBB/구와 DirectX 교차 검사를 수행하고 자기 마지막 판정만 저장한다.
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) override;

public:
	// 현재 월드 구를 마지막 교차 결과 색상으로 Batch에 제출한다.
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch);

private:
	// OriginalDesc는 로컬 정의, Desc는 Update가 덮어쓰는 월드 판정/표시 값이다.
	shared_ptr<BoundingSphere>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingSphere>			m_pDesc = { nullptr };

public:
	// Collider가 구 형상의 공유 소유권을 받는다. 중심/반지름은 복사하므로 descriptor는 호출 뒤 유지할 필요가 없다.
	static shared_ptr<CBounding_Sphere> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const CBounding::BOUNDING_DESC* pArg);

};

NS_END