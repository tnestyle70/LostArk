#pragma once

#include "Engine_Defines.h"

#include <cstdint>

NS_BEGIN(Engine)

enum class PHYSICS_ACTOR_TYPE : uint8_t
{
	STATIC,
	DYNAMIC,
	END
};

enum class PHYSICS_MOTION_MODE : uint8_t
{
	SIMULATED,
	KINEMATIC,
	END
};

enum class PHYSICS_SHAPE_TYPE : uint8_t
{
	BOX,
	SPHERE,
	CAPSULE,
	END
};

enum class PHYSICS_FORCE_MODE : uint8_t
{
	FORCE,
	IMPULSE,
	VELOCITY_CHANGE,
	ACCELERATION,
	END
};

struct PHYSICS_ACTOR_HANDLE final
{
	static constexpr uint32_t INVALID_SLOT = UINT32_MAX;

	uint32_t iSlot = INVALID_SLOT;
	uint32_t iGeneration = 0u;

	bool_t Is_Valid() const { return INVALID_SLOT != iSlot; }
	void Invalidate()
	{
		iSlot = INVALID_SLOT;
		iGeneration = 0u;
	}
};

struct PHYSICS_POSE final
{
	float3_t vPosition = {};
	float4_t vRotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
};

struct PHYSICS_RIGID_BODY_DESC final
{
	uint32_t iLevelID = UINT32_MAX;
	PHYSICS_ACTOR_TYPE eActorType = PHYSICS_ACTOR_TYPE::DYNAMIC;
	PHYSICS_MOTION_MODE eMotionMode = PHYSICS_MOTION_MODE::SIMULATED;
	PHYSICS_SHAPE_TYPE eShapeType = PHYSICS_SHAPE_TYPE::BOX;
	PHYSICS_POSE Pose = {};
	/* Shape transform relative to the actor root. This preserves a render
	   object's authored pivot while collision uses its actual local bounds. */
	PHYSICS_POSE ShapeLocalPose = {};

	/* BOX uses half extents. SPHERE uses radius. CAPSULE uses radius and
	   half-height; PhysX capsules are aligned to their local X axis. */
	float3_t vBoxHalfExtents = { 0.5f, 0.5f, 0.5f };
	f32_t fRadius = 0.5f;
	f32_t fCapsuleHalfHeight = 0.5f;

	f32_t fDensity = 10.f;
	f32_t fStaticFriction = 0.5f;
	f32_t fDynamicFriction = 0.5f;
	f32_t fRestitution = 0.1f;
	f32_t fLinearDamping = 0.05f;
	f32_t fAngularDamping = 0.05f;

	bool_t isGravityEnabled = true;
	f32_t fGravityScale = 1.f;
	float3_t vConstantAcceleration = {};
	bool_t isActive = true;

	uint32_t iCollisionGroup = 1u;
	uint32_t iCollisionMask = 0xffffffffu;
};

struct PHYSICS_STEP_STATS final
{
	uint32_t iSceneActorCount = 0u;
	uint32_t iStepsLastFrame = 0u;
	uint32_t iDroppedSteps = 0u;
	f32_t fAccumulatorSeconds = 0.f;
	f32_t fLastSimulateMs = 0.f;
	bool_t isPaused = false;
};

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

/* Main-thread PhysX facade. Public consumers use only descriptors and
   generation-checked handles; PhysX headers and pointers remain in the
   private implementation. */
class ENGINE_DLL CPhysics_Manager final
{
public:
	static constexpr f32_t FIXED_TIMESTEP = 1.f / 60.f;
	static constexpr uint32_t MAX_STEPS_PER_FRAME = 4u;
	static constexpr uint32_t MAX_DEBUG_STEPS_PER_CALL = 3600u;

private:
	CPhysics_Manager();

public:
	~CPhysics_Manager();

public:
	HRESULT Initialize(uint32_t iNumLevels);
	void Update(f32_t fTimeDelta);
	HRESULT Clear(uint32_t iClearLevelID);

	HRESULT Create_Actor(
		const PHYSICS_RIGID_BODY_DESC& Desc,
		PHYSICS_ACTOR_HANDLE& outHandle);
	HRESULT Destroy_Actor(PHYSICS_ACTOR_HANDLE Handle);
	HRESULT Reset_Actor(
		PHYSICS_ACTOR_HANDLE Handle,
		const PHYSICS_POSE& Pose);
	bool_t Is_ActorValid(PHYSICS_ACTOR_HANDLE Handle) const;

	HRESULT Set_Pose(PHYSICS_ACTOR_HANDLE Handle, const PHYSICS_POSE& Pose);
	HRESULT Get_Pose(PHYSICS_ACTOR_HANDLE Handle, PHYSICS_POSE& outPose) const;
	HRESULT Set_LinearVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vVelocity);
	HRESULT Get_LinearVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		float3_t& outVelocity) const;
	HRESULT Set_AngularVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vVelocity);
	HRESULT Get_AngularVelocity(
		PHYSICS_ACTOR_HANDLE Handle,
		float3_t& outVelocity) const;
	HRESULT Add_Force(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vForce,
		PHYSICS_FORCE_MODE eMode);

	HRESULT Set_MotionMode(
		PHYSICS_ACTOR_HANDLE Handle,
		PHYSICS_MOTION_MODE eMode);
	HRESULT Set_GravityEnabled(
		PHYSICS_ACTOR_HANDLE Handle,
		bool_t isEnabled);
	HRESULT Set_GravityScale(
		PHYSICS_ACTOR_HANDLE Handle,
		f32_t fGravityScale);
	HRESULT Set_ConstantAcceleration(
		PHYSICS_ACTOR_HANDLE Handle,
		const float3_t& vAcceleration);
	HRESULT Set_Active(PHYSICS_ACTOR_HANDLE Handle, bool_t isActive);

	void Set_DebugPaused(bool_t isPaused);
	bool_t Is_DebugPaused() const;
	HRESULT Request_DebugSingleStep();
	/* Synchronous timeline rebuild seam. Valid only while paused. Every step
	   completes simulate/fetchResults before the call returns, does not touch
	   the accumulator, and is bounded to MAX_DEBUG_STEPS_PER_CALL. */
	HRESULT Simulate_DebugSteps(uint32_t iStepCount);
	const PHYSICS_STEP_STATS& Get_StepStats() const;
	uint32_t Get_ActorCount(uint32_t iLevelID) const;

	/* Engine-only executable harness entry. It creates an isolated manager,
	   drops one box onto one support, and verifies handle invalidation. */
	static HRESULT Run_ContractTest(PHYSICS_CONTRACT_RESULT& outResult);

private:
	struct IMPL;
	unique_ptr<IMPL> m_pImpl = { nullptr };

public:
	static unique_ptr<CPhysics_Manager> Create(uint32_t iNumLevels);
};

NS_END
