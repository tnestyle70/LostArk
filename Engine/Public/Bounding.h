#pragma once

// 형상/행렬의 DirectX 타입, COLLIDER 종류와 COM·공유 포인터 타입을 사용한다.
#include "Engine_Defines.h"

// 공개 헤더는 debug draw 타입을 포인터로만 다룬다. 구체 DirectXTK 정의는 각 구현 파일에서 읽는다.
namespace DirectX { inline namespace DX11 { template<typename TVertex> class PrimitiveBatch; struct VertexPositionColor; class BasicEffect; } }

NS_BEGIN(Engine)

// CCollider가 소유하는 로컬 형상→월드 형상 변환/교차/그리기의 공통 경계다.
// 교차 질의는 마지막 결과를 변경하며 thread-safe 읽기 전용 연산이 아니다.
class CBounding abstract
{
public:
	// vCenter는 형상의 로컬 중심이다. 크기와 함께 소비 객체의 좌표 단위를 따라야 한다.
	typedef struct tagBoundingDesc
	{
		float3_t		vCenter;
	}BOUNDING_DESC;

protected:
	CBounding(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding();

public:
	// 기본 구현은 상태 변경 없이 S_OK다. 실제 형상 클래스가 descriptor를 복사하므로 이후 입력 메모리를 보관하지 않는다.
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg);
	// Collider가 전달한 local→world 행렬로 현재 형상을 갱신한다. 실제 변환 정책은 AABB/OBB/구마다 다르다.
	virtual void Update(fmatrix_t WorldMatrix) = 0;
	// eTargetType과 실제 파생 객체 타입의 일치, non-null 입력을 호출자가 보장한다. 내부 static cast는 동적 검증이 아니다.
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) = 0;

public:
	// Collider가 Begin한 Batch에 현재 형상을 제출한다. Effect와 View/Projection 설정 및 Batch 종료는 Collider가 담당한다.
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch) = 0;

protected:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };
	// 자기 마지막 교차 결과다. Render의 빨강/초록 선택에 쓰며 충돌한 모든 상대 목록을 저장하지 않는다.
	bool_t								m_isColl = { false };
};

NS_END