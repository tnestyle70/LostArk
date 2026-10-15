#pragma once

/* 충돌체 정보를 의미한다. */
/* 구, 회전이없는박스, 회전이 있는 박스 */
// 보충: 아래 종류별 bounding을 소유하며 Client가 사용하는 Engine 충돌 컴포넌트다. Server의 피해 확정과 별개이며 debug 표시에서 같은 형상을 사용한다.
// 기존 원형 복제로 게임 객체에 부착되는 Component 계약과 기기/문맥 참조를 상속한다.
#include "Component.h"
// 회전 없는 상자의 descriptor와 형상 생성/교차 구현을 연결한다.
#include "Bounding_AABB.h"
// 회전 상자의 descriptor와 형상 생성/교차 구현을 연결한다.
#include "Bounding_OBB.h"
// 중심과 반지름을 갖는 구 descriptor 및 형상 구현을 연결한다.
#include "Bounding_Sphere.h"

// 공개 헤더에서는 DirectXTK 객체를 shared_ptr 멤버로만 보관하므로 여기서는 선언만 둔다. 실제 생성과 사용은 CPP에서 한다.
namespace DirectX { inline namespace DX11 { template<typename TVertex> class PrimitiveBatch; struct VertexPositionColor; class BasicEffect; } }

NS_BEGIN(Engine)

// 객체가 소유하는 충돌 형상 컴포넌트다. Loader의 형상별 prototype을 Clone하여 로컬 정의를 넣는다.
// Character/Npc/Valtan은 갱신한 형상을 debug draw에 사용한다. Server의 피해 확정 경로와는 별개다.
// clone끼리 GPU debug 자원을 공유하므로 갱신/그리기는 같은 렌더 스레드 흐름에서 호출한다.
class ENGINE_DLL CCollider final : public CComponent
{
private:
	CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCollider();

public:
	// 형상 종류와 공유 debug 그리기 자원을 준비한다. 개별 bounding은 아직 만들지 않는다.
	virtual HRESULT Initialize_Prototype(COLLIDER eType);
	// 선택한 종류에 맞는 BOUNDING_*_DESC를 받아 clone 전용 형상을 만든다. 포인터/종류/수치 검증은 호출자 전제다.
	virtual HRESULT Initialize(void* pArg) override;
public:
	// 로컬 원본에 월드 행렬을 적용하여 현재 판정 형상을 덮어쓴다. 마지막 충돌 색상은 초기화하지 않는다.
	void Update(fmatrix_t WorldMatrix);

	// 양쪽의 갱신된 형상을 비교하고 자기 마지막 판정 결과만 저장한다. 대상의 표시 상태까지 바꾸지는 않는다.
	bool_t Intersect(shared_ptr<CCollider> pTargetCollider);

public:
	// 현재 월드 형상을 View/Projection으로 그린다. 피해나 이동 보정을 적용하는 함수는 아니다.
	virtual HRESULT Render() override;

private:
	COLLIDER				m_eType = { COLLIDER::END };
	// clone 초기화가 교체하는 형상 소유 참조다. 유효한 Initialize 이후에만 Update/Intersect/Render를 호출한다.
	shared_ptr<CBounding>	m_pBounding = { nullptr };
	bool_t					m_isColl = { false };

private:
	// prototype 복사로 공유하는 debug 자원이다. BasicEffect 설정과 immediate context 접근은 내부 잠금으로 보호하지 않는다.
	shared_ptr<PrimitiveBatch<VertexPositionColor>>			m_pBatch = { nullptr };
	shared_ptr<BasicEffect>									m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout>								m_pInputLayout = { nullptr };

public:
	// prototype 생성 실패는 nullptr이며 지역 소유 객체가 부분 생성된 자원도 해제한다.
	static unique_ptr<CCollider> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eType);
	// prototype을 복사한 뒤 전달된 로컬 정의로 형상을 만든다. 유효한 종류에서는 원본과 형상 상태를 분리한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END