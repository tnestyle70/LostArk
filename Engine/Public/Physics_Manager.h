#pragma once

// 엔진 벡터·HRESULT·DLL 심볼과 소유 포인터를 사용한다. public 헤더에는 PhysX 타입이 필요하지 않다.
#include "Engine_Defines.h"

// actor slot/generation, collision mask와 step 통계를 고정 폭 정수로 표현한다.
#include <cstdint>

NS_BEGIN(Engine)

// STATIC은 고정 actor, DYNAMIC은 simulated/kinematic 전환이 가능한 actor다.
enum class PHYSICS_ACTOR_TYPE : uint8_t
{
	STATIC,
	DYNAMIC,
	END
};

// DYNAMIC의 움직임을 물리 적분 또는 외부 pose 목표로 제어한다. STATIC+KINEMATIC 조합은 거절한다.
enum class PHYSICS_MOTION_MODE : uint8_t
{
	SIMULATED,
	KINEMATIC,
	END
};

// 이 facade가 생성하는 primitive 형상이다. mesh cooking은 이 API의 책임이 아니다.
enum class PHYSICS_SHAPE_TYPE : uint8_t
{
	BOX,
	SPHERE,
	CAPSULE,
	END
};

// 입력 단위는 FORCE=질량×거리/초², IMPULSE=질량×거리/초, VELOCITY_CHANGE=거리/초, ACCELERATION=거리/초²다.
// 뒤 두 모드는 질량과 무관한 속도 변화/가속도로 전달된다. 유효한 이름 있는 enum 값을 사용해야 한다.
enum class PHYSICS_FORCE_MODE : uint8_t
{
	FORCE,
	IMPULSE,
	VELOCITY_CHANGE,
	ACCELERATION,
	END
};

// actor를 소유하지 않는 manager 내부 식별자다. 삭제 시 slot의 generation을 올려 오래된 복사본을 거절한다.
// manager 재생성 사이의 전역 고유 ID는 아니므로 manager 수명을 넘겨 재사용하지 않는다.
struct PHYSICS_ACTOR_HANDLE final
{
	static constexpr uint32_t INVALID_SLOT = UINT32_MAX;

	uint32_t iSlot = INVALID_SLOT;
	uint32_t iGeneration = 0u;

	// 로컬 slot 표식만 검사한다. actor 존재와 generation의 실제 일치는 CPhysics_Manager::Is_ActorValid가 확인한다.
	bool_t Is_Valid() const
	{
		return INVALID_SLOT != iSlot;
	}
	// 이 handle 복사본을 비운다. actor 삭제나 다른 handle 복사본의 변경을 수행하지 않는다.
	void Invalidate()
	{
		iSlot = INVALID_SLOT;
		iGeneration = 0u;
	}
};

// 위치와 XYZW quaternion이다. actor Pose는 월드 좌표, ShapeLocalPose는 actor 상대 좌표로 해석한다.
// 입력 quaternion은 manager가 유한성/영길이를 검사하고 정규화한다.
struct PHYSICS_POSE final
{
	float3_t vPosition = {};
	float4_t vRotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
};

// 생성 시 동기적으로 읽는 값 명세다. 길이는 월드 거리 단위이며 제품 파괴 연출은 m·초 기준 값을 전달한다.
// Level ID는 같은 scene 안에서 일괄 해제할 그룹이다. 각 Level에 별도 PhysX scene을 만드는 값이 아니다.
struct PHYSICS_RIGID_BODY_DESC final
{
	uint32_t iLevelID = UINT32_MAX;
	PHYSICS_ACTOR_TYPE eActorType = PHYSICS_ACTOR_TYPE::DYNAMIC;
	PHYSICS_MOTION_MODE eMotionMode = PHYSICS_MOTION_MODE::SIMULATED;
	PHYSICS_SHAPE_TYPE eShapeType = PHYSICS_SHAPE_TYPE::BOX;
	PHYSICS_POSE Pose = {};
	// actor root 기준의 shape 변환이다. 표현 객체의 pivot을 유지하면서 충돌 형상은 실제 local 경계에 맞춘다.
	PHYSICS_POSE ShapeLocalPose = {};

	// BOX는 축별 반폭, SPHERE는 반지름, CAPSULE은 반지름과 원통 부분 반길이를 사용한다.
	// PhysX capsule의 기본 긴 축은 local X이며 ShapeLocalPose로 방향을 맞출 수 있다.
	float3_t vBoxHalfExtents = { 0.5f, 0.5f, 0.5f };
	f32_t fRadius = 0.5f;
	f32_t fCapsuleHalfHeight = 0.5f;

	// dynamic actor의 shape 부피로 질량/관성을 계산할 밀도와 접촉 재질, 속도 감쇠다.
	f32_t fDensity = 10.f;
	f32_t fStaticFriction = 0.5f;
	f32_t fDynamicFriction = 0.5f;
	f32_t fRestitution = 0.1f;
	f32_t fLinearDamping = 0.05f;
	f32_t fAngularDamping = 0.05f;

	// native gravity는 끄고 manager가 -Y 9.81×scale과 일정 가속도를 step마다 더한다.
	// 잠든/비활성/kinematic actor는 이 수동 가속도 적용에서 제외한다.
	bool_t isGravityEnabled = true;
	f32_t fGravityScale = 1.f;
	float3_t vConstantAcceleration = {};
	bool_t isActive = true;

	// 양쪽 group & 상대 mask가 모두 비영이어야 simulation 접촉을 허용한다. group은0을 거절하며 mask0은 허용한다.
	uint32_t iCollisionGroup = 1u;
	uint32_t iCollisionMask = 0xffffffffu;
};

// static sweep의 월드 접촉 법선과 전체 이동 거리 대비 충돌 지점 비율[0,1]이다.
struct PHYSICS_STATIC_SWEEP_HIT final
{
    float3_t vNormal{};
    f32_t fTravelFraction = 0.f;
};

// 직전 Update 또는 debug 동기 실행의 관측값이다. 누적 잔여 시간은 초, 실행 경과는 ms다.
// iDroppedSteps는 누적 버림 수이며 actor 수에는 잠든/비활성 actor도 포함한다.
struct PHYSICS_STEP_STATS final
{
	uint32_t iSceneActorCount = 0u;
	uint32_t iStepsLastFrame = 0u;
	uint32_t iDroppedSteps = 0u;
	f32_t fAccumulatorSeconds = 0.f;
	f32_t fLastSimulateMs = 0.f;
	bool_t isPaused = false;
};

// Run_ContractTest가 실제로 관측한 높이와 계약별 판정이다. 실패한 단계 이후 항목은 기본값으로 남을 수 있다.
struct PHYSICS_CONTRACT_RESULT final
{
	f32_t fInitialHeight = 0.f;
	f32_t fFinalHeight = 0.f;
	uint32_t iStepsExecuted = 0u;
	bool_t didRejectUnpausedDebugSteps = false;
	bool_t didPreserveDebugAccumulator = false;
	bool_t didRejectInvalidDebugStepCount = false;
	bool_t didRespectShapeLocalPose = false;
	bool_t didSettleOnSupport = false;
	bool_t didInvalidateOnClear = false;
};

// CGameInstance가 소유하는 main-thread 물리 경계다. 소비자는 명세와 slot/generation handle만 사용한다.
// PhysX 메모리·scene·worker dispatcher는 IMPL이 소유하며 public API에 backend 포인터를 노출하지 않는다.
// 외부 호출은 내부 잠금으로 보호하지 않는다. simulate/fetchResults가 끝난 경계에서 main thread가 직렬 호출한다.
class ENGINE_DLL CPhysics_Manager final
{
public:
	// 고정 step은 1/60초다. 일반 프레임은 최대4회 적분하고 초과 정수 step을 버려 잔여 소수 시간만 보관한다.
	static constexpr f32_t FIXED_TIMESTEP = 1.f / 60.f;
	static constexpr uint32_t MAX_STEPS_PER_FRAME = 4u;
	static constexpr uint32_t MAX_DEBUG_STEPS_PER_CALL = 3600u;

private:
	CPhysics_Manager();

public:
	~CPhysics_Manager();

public:
	// Create가 새 manager에 한 번 호출한다. Level별 slot 목록과 backend를 준비하며 초기화된 객체의 재설정 API가 아니다.
	HRESULT Initialize(uint32_t iNumLevels);
	// GameInstance가 객체 Update 후 호출한다. paused는 프레임 시간을 누적하지 않고 예약된 한 step만 실행한다.
	void Update(f32_t fTimeDelta);
	// 지정 Level에 속한 actor를 하나씩 삭제한다. 중간 실패나 할당 예외가 이전 삭제를 되돌리지는 않는다.
	HRESULT Clear(uint32_t iClearLevelID);

	// 시작 시 outHandle을 무효화한다. shape/재질/actor를 준비한 뒤 scene와 Level 목록에 등록하고 handle을 돌려준다.
	// 명시적 준비 실패는 임시 backend 자원을 해제한다. vector 할당 예외를 rollback하는 경계는 별도로 두지 않는다.
	HRESULT Create_Actor(
		const PHYSICS_RIGID_BODY_DESC& Desc,
		PHYSICS_ACTOR_HANDLE& outHandle);
	// scene에서 제거하고 release한 뒤 세대를 올려 slot을 재사용한다. 이미 없어진 handle은 S_FALSE다.
	HRESULT Destroy_Actor(PHYSICS_ACTOR_HANDLE Handle);
	// pose 변경과 dynamic 속도0·기본 eFORCE 누적 초기화를 backend에 요청한다. 검증 실패는 actor를 변경하지 않는다.
	// active/kinematic 조건과 backend 적용 성공은 추가 확인하지 않으며 impulse/velocity-change 누적을 별도로 지우지 않는다.
	HRESULT Reset_Actor(
		PHYSICS_ACTOR_HANDLE Handle,
		const PHYSICS_POSE& Pose);
	// 현재 manager에서 slot·generation과 살아 있는 backend actor가 모두 일치하는지 확인한다.
	bool_t Is_ActorValid(PHYSICS_ACTOR_HANDLE Handle) const;

	// kinematic은 다음 simulation의 목표를 설정하고 그 외에는 global pose를 즉시 바꾼다.
	HRESULT Set_Pose(PHYSICS_ACTOR_HANDLE Handle, const PHYSICS_POSE& Pose);
	// actor root의 현재 월드 pose다. 실패하면 outPose를 보존한다.
	HRESULT Get_Pose(PHYSICS_ACTOR_HANDLE Handle, PHYSICS_POSE& outPose) const;
	// simulated dynamic의 질량 중심 월드 선속도(거리/초)를 지정하고 깨우도록 요청한다.
	HRESULT Set_LinearVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vVelocity);
	// dynamic의 현재 선속도를 읽는다. 실패하면 outVelocity를 보존하며 kinematic 읽기도 허용한다.
	HRESULT Get_LinearVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		float3_t& outVelocity) const;
	// simulated dynamic의 월드 각속도(rad/s)를 지정한다. 유한하지 않은 값은 거절한다.
	HRESULT Set_AngularVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vVelocity);
	// dynamic의 각속도(rad/s)를 읽는다. 실패하면 outVelocity를 보존한다.
	HRESULT Get_AngularVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		float3_t& outVelocity) const;
	// 활성 simulated dynamic에 모드별 입력을 전달한다. static/kinematic/비활성 actor는 거절한다.
	HRESULT Add_Force(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vForce,
		PHYSICS_FORCE_MODE eMode);

	// dynamic의 kinematic flag를 바꾼다. 활성 simulated 전환 시 깨우며 생성 시의 형상/소유 Level은 유지한다.
	HRESULT Set_MotionMode(
		PHYSICS_ACTOR_HANDLE Handle,
		PHYSICS_MOTION_MODE eMode);
	// dynamic slot의 수동 중력 사용 여부를 바꾼다. 활성 simulated라면 변경 효과를 위해 깨운다.
	HRESULT Set_GravityEnabled(
		PHYSICS_ACTOR_HANDLE Handle,
		bool_t isEnabled);
	// dynamic slot에 유한한 비음수 배율을 저장한다. 중력 방향은 -Y이며 일정 가속도는 배율과 독립적이다.
	HRESULT Set_GravityScale(
		PHYSICS_ACTOR_HANDLE Handle,
		f32_t fGravityScale);
	// dynamic slot에 월드 가속도(거리/초²)를 저장한다. 실제 힘 적용은 다음 고정 step에서 이루어진다.
	HRESULT Set_ConstantAcceleration(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vAcceleration);
	// simulation 참여 flag와 slot 상태를 함께 바꾼다. actor를 scene에서 제거하거나 handle을 무효화하지 않는다.
	HRESULT Set_Active(PHYSICS_ACTOR_HANDLE Handle, bool_t isActive);

    // Effect 표현이 simulate/fetchResults 완료 뒤 main thread에서 static scene을 조회한다.
    // world 축에 평행한 box를 start→end로 sweep한다. 시작 시 outHit를 비우므로 false도 기존 출력을 보존하지 않는다.
    bool_t Sweep_StaticBox(const float3_t& vStart, const float3_t& vEnd,
        const float3_t& vHalfExtents, PHYSICS_STATIC_SWEEP_HIT& outHit) const;

	// 저작 도구가 clock 정지 여부를 바꾼다. 잔여 시간은 보존하고 해제 시 예약 single-step을 취소한다.
	void Set_DebugPaused(bool_t isPaused);
	// 현재 clock 정지 여부를 읽는다. manager 구현이 없으면 false다.
	bool_t Is_DebugPaused() const;
	// 정지 상태에서 다음 Update의 한 step을 예약한다. 여러 요청은 bool 하나로 합쳐지며 즉시 적분하지 않는다.
	HRESULT Request_DebugSingleStep();
	// 정지한 저작 timeline의 동기 재구축 경계다. 1~MAX_DEBUG_STEPS_PER_CALL회 simulate/fetchResults를 모두 마친다.
	// accumulator는 건드리지 않고 성공 시 예약 single-step은 해제한다. 일반 실행 중의 호출은 거절한다.
	HRESULT Simulate_DebugSteps(uint32_t iStepCount);
	// manager 소유 통계를 차용한다. 다음 Update/debug step으로 내용이 갱신되며 manager가 없으면 정적 빈 통계를 준다.
	const PHYSICS_STEP_STATS& Get_StepStats() const;
	// Level에 등록된 slot 수다. 잘못된 Level은0이며 활성 상태만을 세는 통계는 아니다.
	uint32_t Get_ActorCount(uint32_t iLevelID) const;

	// 격리 manager에 바닥과 box를 만들고 낙하·shape local pose·debug clock·Clear 뒤 handle 무효화를 검사한다.
	// 출력은 시작 시 초기화하며 중간 실패까지의 관측값을 남긴다. 실제 제품 장면이나 화면을 검사하는 함수는 아니다.
	static HRESULT Run_ContractTest(PHYSICS_CONTRACT_RESULT& outResult);

private:
	// backend 객체·slot 저장소·clock을 단독 소유한다. PhysX 내부 worker와 public 호출의 스레드 책임을 분리한다.
	struct IMPL;
	unique_ptr<IMPL> m_pImpl = { nullptr };

public:
	// 초기화 실패는 임시 manager 소멸로 backend를 정리한 뒤 nullptr를 반환한다. 할당 예외는 전파된다.
	static unique_ptr<CPhysics_Manager> Create(uint32_t iNumLevels);
};

NS_END
