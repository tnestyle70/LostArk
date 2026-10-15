#pragma once

// 기존 Prototype/Clone 초기화 경로와 장치 참조를 사용하는 Component 기반 클래스다.
#include "Component.h"
// PhysX 포인터 대신 엔진의 물리 생성 명세·pose·세대가 포함된 actor handle을 사용한다.
#include "Physics_Manager.h"

NS_BEGIN(Engine)

// 물리 actor 하나의 해제 책임을 Component 수명에 연결한다. 실제 actor 메모리와 세대 검사는 CPhysics_Manager가 맡는다.
// CTransform은 소유하지 않는다. 사용자가 갱신 전에 pose를 전달하고 물리 계산 뒤 결과를 읽어 표현 객체에 적용한다.
// 파괴 연출/저작 도구는 Create_Runtime으로 GameObject 없이 사용할 수 있다. 모든 조작은 manager의 main-thread 계약을 따른다.
class ENGINE_DLL CRigidBody final : public CComponent
{
private:
	CRigidBody(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// 기반 Component만 복제한다. 원형의 actor handle은 복사하지 않아 서로 독립적으로 생성/해제한다.
	CRigidBody(const CRigidBody& Prototype);

public:
	// 남아 있는 actor의 제거를 요청한다. 먼저 Level Clear로 사라진 actor는 세대 검사로 다시 해제하지 않는다.
	virtual ~CRigidBody();

public:
	// 기반 Component 원형 준비만 수행한다. 원형 자체는 actor를 생성하지 않는다.
	virtual HRESULT Initialize_Prototype() override;
	// pArg가 있으면 PHYSICS_RIGID_BODY_DESC를 호출 중 읽어 actor를 만든다. nullptr는 actor 없이 성공한다.
	virtual HRESULT Initialize(void* pArg) override;

	// 이미 유효한 actor가 있으면 E_FAIL이다. manager의 생성 실패는 이 객체의 handle을 무효화한다.
	HRESULT Create_Actor(const PHYSICS_RIGID_BODY_DESC& Desc);
	// actor 제거를 요청한 뒤 반환 HRESULT와 무관하게 로컬 handle을 무효화한다. 이미 빈 handle은 S_FALSE다.
	HRESULT Destroy_Actor();
	// pose 변경과 dynamic 속도0·기본 eFORCE 누적 초기화를 요청한다. active/kinematic 추가 검사는 없으며 충격량 누적은 별도로 비우지 않는다.
	HRESULT Reset_Actor(const PHYSICS_POSE& Pose);
	// 로컬 slot 표식뿐 아니라 현재 manager의 actor 존재와 generation까지 검사한다.
	bool_t Is_Valid() const;
	// 비소유 handle의 복사본이다. 복사본을 받았다고 actor의 수명이 연장되지는 않는다.
	PHYSICS_ACTOR_HANDLE Get_Handle() const
	{
		return m_Handle;
	}

	// kinematic은 다음 step의 목표 pose를, 그 외 actor는 즉시 global pose를 설정한다. CTransform에는 직접 쓰지 않는다.
	HRESULT Set_Pose(const PHYSICS_POSE& Pose);
	// 현재 actor root의 월드 pose를 읽는다. manager/handle 오류 시 출력은 보존된다.
	HRESULT Get_Pose(PHYSICS_POSE& outPose) const;
	// simulated dynamic actor의 질량 중심 선속도를 월드 거리/초 단위로 지정한다. static/kinematic은 거절된다.
	HRESULT Set_LinearVelocity(const float3_t& vVelocity);
	// dynamic actor의 현재 선속도를 읽는다. 읽기 실패면 출력 값을 보존한다.
	HRESULT Get_LinearVelocity(float3_t& outVelocity) const;
	// simulated dynamic actor의 월드 각속도(rad/s)를 지정한다. 값의 유한성과 actor 종류는 manager가 검사한다.
	HRESULT Set_AngularVelocity(const float3_t& vVelocity);
	// dynamic actor의 각속도(rad/s)를 읽는다. 읽기 실패면 출력 값을 보존한다.
	HRESULT Get_AngularVelocity(float3_t& outVelocity) const;
	// 활성 simulated dynamic actor에 지정 모드의 힘/충격량/속도 변화/가속도를 전달한다. 질량 적용은 모드에 따라 다르다.
	HRESULT Add_Force(
		const float3_t& vForce,
		PHYSICS_FORCE_MODE eMode = PHYSICS_FORCE_MODE::FORCE);
	// 시간 적분 전의 힘이 아닌 충격량(질량×거리/초)을 IMPULSE 모드로 제출한다.
	HRESULT Apply_Impulse(const float3_t& vImpulse)
	{
		return Add_Force(vImpulse, PHYSICS_FORCE_MODE::IMPULSE);
	}

	// dynamic actor를 simulated/kinematic으로 전환한다. static actor는 거절되며 활성 simulated 전환은 깨운다.
	HRESULT Set_MotionMode(PHYSICS_MOTION_MODE eMode);
	// manager가 step마다 적용할 개별 중력의 사용 여부를 바꾼다. dynamic actor만 허용한다.
	HRESULT Set_GravityEnabled(bool_t isEnabled);
	// 개별 중력에 곱할 유한한 0 이상 배율을 설정한다. 일정 가속도는 이 배율과 별도로 적용된다.
	HRESULT Set_GravityScale(f32_t fGravityScale);
	// 매 step 적용할 월드 가속도(거리/초²)를 저장한다. 즉시 pose를 이동시키지는 않는다.
	HRESULT Set_ConstantAcceleration(const float3_t& vAcceleration);
	// actor를 삭제하지 않고 simulation 참여 flag를 바꾼다. handle과 소유 관계는 유지한다.
	HRESULT Set_Active(bool_t isActive);

public:
	// 독립 Component를 복제하고 전달 명세로 초기화한다. 실패한 임시 객체는 소멸하며 actor 정리도 요청한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	// 장치 참조를 가진 원형을 만든다. actor 생성은 이후 Clone/Initialize가 담당한다.
	static unique_ptr<CRigidBody> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	// 파괴 연출/저작 도구용 actor 소유자를 만든다. GPU 장치가 없어도 동작하며 명세는 호출 중만 읽는다.
	static shared_ptr<CRigidBody> Create_Runtime(
		const PHYSICS_RIGID_BODY_DESC& Desc);

private:
	// manager의 slot/generation이다. manager 포인터나 CTransform을 장기간 보관하지 않는다.
	PHYSICS_ACTOR_HANDLE m_Handle = {};
};

NS_END
