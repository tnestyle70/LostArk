#pragma once

// 게임 객체에 붙는 Component의 장치 참조와 초기화 인터페이스를 상속한다. 행렬 상태는 각 Transform이 값으로 소유한다.
#include "Component.h"
// 호출 동안 Shader와 Navigation을 공유 참조하고 생성 결과의 소유권을 전달한다.
#include <memory>

/* 월드 공간에서의 상태를 담당할 월드변환행렬을 보관한다. */
/* 이 행렬의 상태를 제어하는 다양한 기능을 보유한다. */
// 행벡터 규약의 RIGHT/UP/LOOK/POSITION 행을 저장한다. 기저는 w=0, 위치는 w=1이다.
// 일반 객체는 월드 행렬로 쓰고 PartObject는 부모와 합성하기 전 local 행렬로도 사용한다.

NS_BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		// 이동 속도는 위치 단위/초, 회전 속도는 degree/초다. 시간 입력은 초를 사용한다.
		f32_t		fSpeedPerSec = {};
		f32_t		fRotationPerSec = {};
	}TRANSFORM_DESC;
private:
	CTransform(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CTransform();

public:
	// 저장 행렬을 XMLoad로 계산 타입에 읽고 요청한 행을 값으로 반환한다.
	// STATE는 유효한 네 행 중 하나여야 한다. END/임의 값의 범위 검사는 하지 않는다.
	vector_t Get_State(STATE eState)
	{
		return XMLoadFloat4x4(&m_WorldMatrix).r[ETOUI(eState)];
	}

	// Transform 소유 메모리를 차용한다. Part가 이 주소를 보관하면 부모 Transform의 수명이 더 길어야 한다.
	const float4x4_t* Get_WorldMatrixPtr()
	{
		return &m_WorldMatrix;
	}

	// 세 기저의 길이를 값으로 반환한다. 축의 음수 부호나 shear를 복원하는 행렬 분해는 수행하지 않는다.
	const float3_t Get_Scaled();

	// 계산 결과를 XMStore로 해당 저장 행에 복사한다. w와 local/world 좌표 의미는 호출자가 맞춘다.
	void Set_State(STATE eState, fvector_t vState)
	{
		XMStoreFloat4(reinterpret_cast<float4_t*>(&m_WorldMatrix.m[ETOUI(eState)][0]), vState);
	}

public:
	// 직접 생성 단계에서는 S_OK만 반환한다. 실제 단위 행렬 설정은 이어 호출하는 Initialize가 수행한다.
	virtual HRESULT Initialize_Prototype() override;
	// GameObject 초기화가 차용 descriptor의 속도를 복사하고 단위 행렬로 초기화한다. nullptr이면 기존 속도를 유지한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// 객체의 그리기 준비가 현재 행렬을 지정 shader 변수에 전달한다. 유효한 shader/변수명이 전제이며 바인딩 HRESULT를 반환한다.
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName);

public:
	// Scale은 각 축 길이를 지정값으로 설정하고, Scaling은 현재 축에 배율을 곱한다. 위치는 보존한다.
	void Scale(f32_t fScaleX = 1.f, f32_t fScaleY = 1.f, f32_t fScaleZ = 1.f);
	void Scaling(f32_t fScaleX = 1.f, f32_t fScaleY = 1.f, f32_t fScaleZ = 1.f);
	// 기저 방향으로 속도 * 초만큼 이동한다. Navigation 검사는 Go_Straight의 선택 인자 경로에만 있다.
	void Go_Straight(f32_t fTimeDelta, shared_ptr<class CNavigation> pNavigation = nullptr);
	void Go_Backward(f32_t fTimeDelta);
	void Go_Left(f32_t fTimeDelta);
	void Go_Right(f32_t fTimeDelta);

	// Rotation은 기존 방향을 교체한다. 입력 degree를 계산 시 radian으로 변환한다.
	void Rotation(fvector_t vAxis, f32_t fDegree);
	void Rotation(f32_t fDegreeX, f32_t fDegreeY, f32_t fDegreeZ);
	// Turn은 degree/초인 회전 속도와 초 단위 delta로 현재 방향에 회전을 누적한다.
	void Turn(fvector_t vAxis, f32_t fTimeDelta);

	// +Y를 위로 두고 목표를 향한다. 같은 위치/수직 방향 등 퇴화 입력은 호출자가 피해야 한다.
	void LookAt(fvector_t vAt);
	// 제한 거리 밖이면 한 프레임 분량을 이동한다. 목표까지의 남은 거리로 보폭을 제한하지는 않는다.
	void Chase(fvector_t vGoal, f32_t fTimeDelta, f32_t fLimitDistance = 0.1f);

private:
	// 객체가 소유하는 저장 타입이다. 각 계산은 XMLoad → DirectXMath 연산 → XMStore로 반영한다.
	float4x4_t					m_WorldMatrix = {};
	f32_t						m_fSpeedPerSec = {};
	f32_t						m_fRotationPerSec = {};

public:
	// GameObject가 자신의 Transform을 직접 만들 때 공유 소유권을 받는다. Clone 경로로 등록해 재사용하지 않는다.
	static shared_ptr<CTransform> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 현재 구현은 항상 nullptr다. 개별 Transform 준비에는 위 Create와 Initialize를 사용한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END