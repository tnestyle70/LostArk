// 엔진 물리 명세/handle과 main-thread facade를 구현한다.
#include "Physics_Manager.h"

// PhysX 객체 생성·primitive 형상·동기 simulation을 구현 안에서만 사용한다.
#include <PxPhysicsAPI.h>

// Level slot 제거와 경과 시간 측정, 유한성·고정 step 잔여 계산을 사용한다.
#include <algorithm>
#include <chrono>
#include <cmath>
// 내부 저장소에 필요한 표준 이동/컨테이너 선언을 포함한다.
#include <utility>
#include <vector>

using namespace physx;

namespace
{
	// foundation보다 오래 사는 allocator/error callback이다. backend가 이 전역 객체를 참조한다.
	PxDefaultAllocator g_PhysicsAllocator;
	PxDefaultErrorCallback g_PhysicsErrorCallback;

	// 월드 위치·속도·가속도 입력의 NaN/무한대를 검사한다.
	bool IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	// quaternion 네 성분의 유한성을 검사한다. 정규화 가능 여부는 TryMakeTransform에서 추가 확인한다.
	bool IsFinite(const float4_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z) && std::isfinite(value.w);
	}

	// 좌표축/단위를 바꾸지 않고 엔진 벡터의 XYZ를 backend로 전달한다.
	PxVec3 ToPxVec3(const float3_t& value)
	{
		return PxVec3(value.x, value.y, value.z);
	}

	// backend 결과를 같은 좌표계/단위의 엔진 벡터로 복사한다.
	float3_t ToFloat3(const PxVec3& value)
	{
		return float3_t(value.x, value.y, value.z);
	}

	// 위치와 quaternion을 검사하고 회전을 정규화한다. outPose에 대입한 뒤 backend 유효성 결과를 반환한다.
	bool TryMakeTransform(const PHYSICS_POSE& Pose, PxTransform& outPose)
	{
		if (!IsFinite(Pose.vPosition) ||
			!IsFinite(Pose.vRotationQuaternion))
		{
			return false;
		}

		PxQuat rotation(
			Pose.vRotationQuaternion.x,
			Pose.vRotationQuaternion.y,
			Pose.vRotationQuaternion.z,
			Pose.vRotationQuaternion.w);
		if (!rotation.isFinite() || rotation.magnitudeSquared() <= 1.e-8f)
			return false;

		rotation.normalize();
		outPose = PxTransform(ToPxVec3(Pose.vPosition), rotation);
		return outPose.isValid();
	}

	// actor root의 backend 변환을 엔진 pose로 복사한다. shape local offset을 합치지 않는다.
	PHYSICS_POSE ToPhysicsPose(const PxTransform& Pose)
	{
		PHYSICS_POSE result;
		result.vPosition = ToFloat3(Pose.p);
		result.vRotationQuaternion = float4_t(
			Pose.q.x, Pose.q.y, Pose.q.z, Pose.q.w);
		return result;
	}

	// simulation 접촉 쌍의 양방향 group/mask를 검사한다. 하나라도 맞지 않으면 제거하고 통과 시 기본 접촉만 요청한다.
	PxFilterFlags PhysicsFilterShader(
		PxFilterObjectAttributes,
		PxFilterData filterData0,
		PxFilterObjectAttributes,
		PxFilterData filterData1,
		PxPairFlags& pairFlags,
		const void*,
		PxU32)
	{
		if (0u == (filterData0.word0 & filterData1.word1) ||
			0u == (filterData1.word0 & filterData0.word1))
		{
			return PxFilterFlag::eKILL;
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	// 공개 force mode를 backend에 대응한다. 정의되지 않은 값은 기본 FORCE로 가므로 호출자는 이름 있는 enum 값을 사용한다.
	PxForceMode::Enum ToPxForceMode(PHYSICS_FORCE_MODE eMode)
	{
		switch (eMode)
		{
		case PHYSICS_FORCE_MODE::FORCE:
			return PxForceMode::eFORCE;
		case PHYSICS_FORCE_MODE::IMPULSE:
			return PxForceMode::eIMPULSE;
		case PHYSICS_FORCE_MODE::VELOCITY_CHANGE:
			return PxForceMode::eVELOCITY_CHANGE;
		case PHYSICS_FORCE_MODE::ACCELERATION:
			return PxForceMode::eACCELERATION;
		default:
			return PxForceMode::eFORCE;
		}
	}
}

// backend와 Level별 actor 소유 목록, generation 저장소, 고정 step clock을 한 manager에 묶는다.
struct CPhysics_Manager::IMPL
{
	// actor는 release 책임이 있는 포인터다. 삭제 후에도 slot/generation은 남고 FreeSlots를 통해 재사용한다.
	struct ACTOR_SLOT final
	{
		PxRigidActor* pActor = nullptr;
		uint32_t iGeneration = 1u;
		uint32_t iLevelID = PHYSICS_ACTOR_HANDLE::INVALID_SLOT;
		bool_t isGravityEnabled = true;
		f32_t fGravityScale = 1.f;
		float3_t vConstantAcceleration = {};
		bool_t isActive = true;
	};

	// backend 자원은 생성 역순으로 Release에서 정리한다. PVD는 Debug에서만 연결을 시도하는 진단용이다.
	PxFoundation* pFoundation = nullptr;
	PxPvd* pPvd = nullptr;
	PxPvdTransport* pPvdTransport = nullptr;
	PxPhysics* pPhysics = nullptr;
	PxDefaultCpuDispatcher* pDispatcher = nullptr;
	PxScene* pScene = nullptr;

	// ActorSlots는 소유 기록, FreeSlots는 빈 slot, LevelActorSlots는 일괄 해제용 비소유 index 목록이다.
	std::vector<ACTOR_SLOT> ActorSlots;
	std::vector<uint32_t> FreeSlots;
	std::vector<std::vector<uint32_t>> LevelActorSlots;

	// 잔여 초와 저작 clock 상태다. single-step은 횟수 큐가 아닌 한 번의 예약 flag다.
	f32_t fAccumulator = 0.f;
	bool_t isPaused = false;
	bool_t isSingleStepRequested = false;
	PHYSICS_STEP_STATS StepStats = {};

	// handle slot 범위와 generation을 함께 확인한다. 오래된 handle이 재사용된 actor를 조작하지 않게 한다.
	ACTOR_SLOT* Find(PHYSICS_ACTOR_HANDLE Handle)
	{
		if (!Handle.Is_Valid() || Handle.iSlot >= ActorSlots.size())
			return nullptr;

		ACTOR_SLOT& Slot = ActorSlots[Handle.iSlot];
		return nullptr != Slot.pActor &&
			Slot.iGeneration == Handle.iGeneration ? &Slot : nullptr;
	}

	// 조회 경로에서도 같은 세대 검사를 수행한다. 반환 포인터는 vector 재할당/삭제 전에만 차용한다.
	const ACTOR_SLOT* Find(PHYSICS_ACTOR_HANDLE Handle) const
	{
		if (!Handle.Is_Valid() || Handle.iSlot >= ActorSlots.size())
			return nullptr;

		const ACTOR_SLOT& Slot = ActorSlots[Handle.iSlot];
		return nullptr != Slot.pActor &&
			Slot.iGeneration == Handle.iGeneration ? &Slot : nullptr;
	}

	// 살아 있는 backend actor를 센다. sleep/active 여부와 무관한 scene 소유 수다.
	void RefreshActorCount()
	{
		uint32_t count = 0u;
		for (const ACTOR_SLOT& Slot : ActorSlots)
		{
			if (nullptr != Slot.pActor)
				++count;
		}
		StepStats.iSceneActorCount = count;
	}

	// 활성·깨어 있는 simulated dynamic만 수동 중력과 일정 가속도를 적용한다.
	// 중력은 (0,-9.81,0)×scale이고 force mode는 ACCELERATION이므로 질량으로 나누지 않는다.
	void ApplyAccelerations()
	{
		const PxVec3 GRAVITY(0.f, -9.81f, 0.f);

		for (ACTOR_SLOT& Slot : ActorSlots)
		{
			if (nullptr == Slot.pActor || !Slot.isActive)
				continue;

			PxRigidDynamic* pDynamic = Slot.pActor->is<PxRigidDynamic>();
			if (nullptr == pDynamic ||
				pDynamic->getRigidBodyFlags().isSet(
					PxRigidBodyFlag::eKINEMATIC) ||
				pDynamic->isSleeping())
			{
				continue;
			}

			PxVec3 acceleration = ToPxVec3(Slot.vConstantAcceleration);
			if (Slot.isGravityEnabled)
				acceleration += GRAVITY * Slot.fGravityScale;

			if (!acceleration.isZero())
				pDynamic->addForce(
					acceleration, PxForceMode::eACCELERATION, false);
		}
	}

	// 가속도를 제출하고 1/60초 simulation의 결과 수집까지 동기적으로 마친다. 호출 반환 뒤 pose를 읽을 수 있다.
	void Step()
	{
		ApplyAccelerations();
		pScene->simulate(CPhysics_Manager::FIXED_TIMESTEP);
		pScene->fetchResults(true);
	}

	// actor를 scene에서 떼어 release한 뒤 scene→dispatcher→physics→PVD/transport→foundation 순으로 정리한다.
	void Release()
	{
		for (ACTOR_SLOT& Slot : ActorSlots)
		{
			if (nullptr != Slot.pActor)
			{
				if (nullptr != pScene && nullptr != Slot.pActor->getScene())
					pScene->removeActor(*Slot.pActor);
				Slot.pActor->release();
				Slot.pActor = nullptr;
			}
		}
		ActorSlots.clear();
		FreeSlots.clear();
		LevelActorSlots.clear();

		if (nullptr != pScene)
		{
			pScene->release();
			pScene = nullptr;
		}
		if (nullptr != pDispatcher)
		{
			pDispatcher->release();
			pDispatcher = nullptr;
		}
		if (nullptr != pPhysics)
		{
			pPhysics->release();
			pPhysics = nullptr;
		}
		if (nullptr != pPvd)
		{
			if (pPvd->isConnected())
				pPvd->disconnect();
			pPvd->release();
			pPvd = nullptr;
		}
		if (nullptr != pPvdTransport)
		{
			pPvdTransport->release();
			pPvdTransport = nullptr;
		}
		if (nullptr != pFoundation)
		{
			pFoundation->release();
			pFoundation = nullptr;
		}
	}
};

// backend 객체 생성 전에 소유 구현과 빈 clock/slot 저장소를 준비한다.
CPhysics_Manager::CPhysics_Manager()
	: m_pImpl { make_unique<IMPL>() }
{
}

// 초기화 중간 실패로 일부 backend만 존재해도 보유 포인터를 확인하면서 정리한다.
CPhysics_Manager::~CPhysics_Manager()
{
	if (nullptr != m_pImpl)
		m_pImpl->Release();
}

// Create가 새 객체에서 한 번 호출한다. backend 단계별 명시적 실패는 만들어진 자원을 정리한다.
// 이미 초기화된 객체의 재호출이나 vector 할당 예외를 트랜잭션으로 처리하는 경로는 아니다.
HRESULT CPhysics_Manager::Initialize(uint32_t iNumLevels)
{
	if (0u == iNumLevels || nullptr == m_pImpl)
		return E_INVALIDARG;

	// 오래 사는 callback을 foundation에 연결한다. 이 단계의 실패는 후속 객체를 만들지 않는다.
	m_pImpl->pFoundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		g_PhysicsAllocator,
		g_PhysicsErrorCallback);
	if (nullptr == m_pImpl->pFoundation)
		return E_FAIL;

#ifdef _DEBUG
	// Debug PVD는 로컬 진단 endpoint다. 연결 실패를 제품 Server 접속이나 물리 초기화 실패로 취급하지 않는다.
	m_pImpl->pPvd = PxCreatePvd(*m_pImpl->pFoundation);
	if (nullptr != m_pImpl->pPvd)
	{
		m_pImpl->pPvdTransport = PxDefaultPvdSocketTransportCreate(
			"127.0.0.1", 5425, 10u);
		if (nullptr != m_pImpl->pPvdTransport)
		{
			m_pImpl->pPvd->connect(
				*m_pImpl->pPvdTransport,
				PxPvdInstrumentationFlag::eALL);
		}
	}
#endif

	m_pImpl->pPhysics = PxCreatePhysics(
		PX_PHYSICS_VERSION,
		*m_pImpl->pFoundation,
		PxTolerancesScale(),
		true,
		m_pImpl->pPvd);
	if (nullptr == m_pImpl->pPhysics)
	{
		m_pImpl->Release();
		return E_FAIL;
	}

	// backend simulation 작업을 수행할 worker 둘을 준비한다. public API를 여러 스레드에서 호출해도 된다는 뜻은 아니다.
	m_pImpl->pDispatcher = PxDefaultCpuDispatcherCreate(2u);
	if (nullptr == m_pImpl->pDispatcher)
	{
		m_pImpl->Release();
		return E_FAIL;
	}

	// 하나의 scene에 Level별 actor를 함께 넣는다. dynamic 생성에서 native gravity를 끄고 slot별 가속도로 대체한다.
	PxSceneDesc SceneDesc(m_pImpl->pPhysics->getTolerancesScale());
	SceneDesc.gravity = PxVec3(0.f, -9.81f, 0.f);
	SceneDesc.cpuDispatcher = m_pImpl->pDispatcher;
	SceneDesc.filterShader = PhysicsFilterShader;
	m_pImpl->pScene = m_pImpl->pPhysics->createScene(SceneDesc);
	if (nullptr == m_pImpl->pScene)
	{
		m_pImpl->Release();
		return E_FAIL;
	}

#ifdef _DEBUG
	if (PxPvdSceneClient* pPvdClient =
		m_pImpl->pScene->getScenePvdClient())
	{
		pPvdClient->setScenePvdFlag(
			PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pPvdClient->setScenePvdFlag(
			PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pPvdClient->setScenePvdFlag(
			PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
#endif

	m_pImpl->LevelActorSlots.resize(iNumLevels);
	return S_OK;
}

// GameInstance의 객체 Update와 Post_Physics_Update 사이에서 호출한다. 유한한 비음수 프레임 시간만 일반 clock에 더한다.
void CPhysics_Manager::Update(f32_t fTimeDelta)
{
	if (nullptr == m_pImpl || nullptr == m_pImpl->pScene)
		return;

	PHYSICS_STEP_STATS& Stats = m_pImpl->StepStats;
	Stats.iStepsLastFrame = 0u;
	Stats.fLastSimulateMs = 0.f;
	Stats.isPaused = m_pImpl->isPaused;

	const auto begin = std::chrono::steady_clock::now();
	if (m_pImpl->isPaused)
	{
		// 정지한 저작 clock은 프레임 시간을 누적하지 않는다. 예약 하나는 고정 step 한 번이며 기존 잔여 시간은 유지한다.
		if (m_pImpl->isSingleStepRequested)
		{
			m_pImpl->Step();
			Stats.iStepsLastFrame = 1u;
			m_pImpl->isSingleStepRequested = false;
		}
	}
	else if (std::isfinite(fTimeDelta) && fTimeDelta >= 0.f)
	{
		m_pImpl->isSingleStepRequested = false;
		m_pImpl->fAccumulator += fTimeDelta;
		// 한 프레임의 simulation 작업을 최대4회로 제한한다. 남은 정수 step은 아래에서 버리고 소수 잔여만 유지한다.
		while (m_pImpl->fAccumulator >= FIXED_TIMESTEP &&
			Stats.iStepsLastFrame < MAX_STEPS_PER_FRAME)
		{
			m_pImpl->Step();
			m_pImpl->fAccumulator -= FIXED_TIMESTEP;
			++Stats.iStepsLastFrame;
		}

		// 누적 지연 전체를 다음 프레임으로 넘기지 않는다. 버린 횟수 통계를 늘리고 fmod로 한 step 미만 시간을 남긴다.
		if (m_pImpl->fAccumulator >= FIXED_TIMESTEP)
		{
			Stats.iDroppedSteps += static_cast<uint32_t>(
				m_pImpl->fAccumulator / FIXED_TIMESTEP);
			m_pImpl->fAccumulator = std::fmod(
				m_pImpl->fAccumulator, FIXED_TIMESTEP);
		}
	}

	const std::chrono::duration<f32_t, std::milli> elapsed =
		std::chrono::steady_clock::now() - begin;
	Stats.fLastSimulateMs = elapsed.count();
	Stats.fAccumulatorSeconds = m_pImpl->fAccumulator;
	m_pImpl->RefreshActorCount();
}

// Level의 뒤쪽 slot부터 제거한다. 이미 수행한 삭제를 복구하는 일괄 rollback은 없다.
HRESULT CPhysics_Manager::Clear(uint32_t iClearLevelID)
{
	if (nullptr == m_pImpl ||
		iClearLevelID >= m_pImpl->LevelActorSlots.size())
	{
		return E_INVALIDARG;
	}

	auto& LevelSlots = m_pImpl->LevelActorSlots[iClearLevelID];
	while (!LevelSlots.empty())
	{
		const uint32_t iSlot = LevelSlots.back();
		if (iSlot >= m_pImpl->ActorSlots.size())
		{
			LevelSlots.pop_back();
			continue;
		}

		const IMPL::ACTOR_SLOT& Slot = m_pImpl->ActorSlots[iSlot];
		const PHYSICS_ACTOR_HANDLE Handle { iSlot, Slot.iGeneration };
		if (FAILED(Destroy_Actor(Handle)))
			return E_FAIL;
	}

	m_pImpl->RefreshActorCount();
	return S_OK;
}

// CRigidBody/도구가 명세를 전달한다. 출력 handle은 먼저 비우고 backend 준비·slot 등록 이후에만 채운다.
HRESULT CPhysics_Manager::Create_Actor(
	const PHYSICS_RIGID_BODY_DESC& Desc,
	PHYSICS_ACTOR_HANDLE& outHandle)
{
	outHandle.Invalidate();
	if (nullptr == m_pImpl || nullptr == m_pImpl->pPhysics ||
		nullptr == m_pImpl->pScene ||
		Desc.iLevelID >= m_pImpl->LevelActorSlots.size() ||
		PHYSICS_ACTOR_TYPE::END == Desc.eActorType ||
		PHYSICS_MOTION_MODE::END == Desc.eMotionMode ||
		PHYSICS_SHAPE_TYPE::END == Desc.eShapeType ||
		!std::isfinite(Desc.fDensity) || Desc.fDensity <= 0.f ||
		!std::isfinite(Desc.fStaticFriction) ||
		Desc.fStaticFriction < 0.f ||
		!std::isfinite(Desc.fDynamicFriction) ||
		Desc.fDynamicFriction < 0.f ||
		!std::isfinite(Desc.fRestitution) ||
		Desc.fRestitution < 0.f || Desc.fRestitution > 1.f ||
		!std::isfinite(Desc.fLinearDamping) ||
		Desc.fLinearDamping < 0.f ||
		!std::isfinite(Desc.fAngularDamping) ||
		Desc.fAngularDamping < 0.f ||
		!std::isfinite(Desc.fGravityScale) ||
		Desc.fGravityScale < 0.f ||
		!IsFinite(Desc.vConstantAcceleration) ||
		0u == Desc.iCollisionGroup)
	{
		return E_INVALIDARG;
	}

	if (PHYSICS_ACTOR_TYPE::STATIC == Desc.eActorType &&
		PHYSICS_MOTION_MODE::KINEMATIC == Desc.eMotionMode)
	{
		return E_INVALIDARG;
	}

	// actor root와 shape local 변환을 따로 검증한다. shape offset을 actor의 표현 pivot에 합쳐 저장하지 않는다.
	PxTransform ActorPose(PxIdentity);
	PxTransform ShapeLocalPose(PxIdentity);
	if (!TryMakeTransform(Desc.Pose, ActorPose) ||
		!TryMakeTransform(Desc.ShapeLocalPose, ShapeLocalPose))
		return E_INVALIDARG;

	// 재질과 actor를 임시 준비한다. 이후 명시적 실패 분기마다 자기 단계에서 획득한 참조를 해제한다.
	PxMaterial* pMaterial = m_pImpl->pPhysics->createMaterial(
		Desc.fStaticFriction,
		Desc.fDynamicFriction,
		Desc.fRestitution);
	if (nullptr == pMaterial)
		return E_FAIL;

	PxRigidActor* pActor = nullptr;
	if (PHYSICS_ACTOR_TYPE::STATIC == Desc.eActorType)
		pActor = m_pImpl->pPhysics->createRigidStatic(ActorPose);
	else
		pActor = m_pImpl->pPhysics->createRigidDynamic(ActorPose);

	if (nullptr == pActor)
	{
		pMaterial->release();
		return E_FAIL;
	}

	// 명세가 선택한 primitive만 생성한다. 길이는 반폭/반지름/반길이이며 유한한 양수여야 한다.
	PxShape* pShape = nullptr;
	switch (Desc.eShapeType)
	{
	case PHYSICS_SHAPE_TYPE::BOX:
		if (!IsFinite(Desc.vBoxHalfExtents) ||
			Desc.vBoxHalfExtents.x <= 0.f ||
			Desc.vBoxHalfExtents.y <= 0.f ||
			Desc.vBoxHalfExtents.z <= 0.f)
		{
			break;
		}
		pShape = m_pImpl->pPhysics->createShape(
			PxBoxGeometry(ToPxVec3(Desc.vBoxHalfExtents)),
			*pMaterial,
			true);
		break;
	case PHYSICS_SHAPE_TYPE::SPHERE:
		if (!std::isfinite(Desc.fRadius) || Desc.fRadius <= 0.f)
			break;
		pShape = m_pImpl->pPhysics->createShape(
			PxSphereGeometry(Desc.fRadius), *pMaterial, true);
		break;
	case PHYSICS_SHAPE_TYPE::CAPSULE:
		if (!std::isfinite(Desc.fRadius) || Desc.fRadius <= 0.f ||
			!std::isfinite(Desc.fCapsuleHalfHeight) ||
			Desc.fCapsuleHalfHeight <= 0.f)
		{
			break;
		}
		pShape = m_pImpl->pPhysics->createShape(
			PxCapsuleGeometry(Desc.fRadius, Desc.fCapsuleHalfHeight),
			*pMaterial,
			true);
		break;
	default:
		break;
	}

	// shape가 재질 참조를 보유하므로 준비용 material 참조를 반환한다. shape 미생성은 actor도 해제한다.
	pMaterial->release();
	if (nullptr == pShape)
	{
		pActor->release();
		return E_INVALIDARG;
	}

	// simulation/query 필터와 actor 상대 shape pose를 설정한다. attach 뒤에는 actor가 shape 참조를 보유한다.
	const PxFilterData FilterData(
		Desc.iCollisionGroup,
		Desc.iCollisionMask,
		0u,
		0u);
	pShape->setSimulationFilterData(FilterData);
	pShape->setQueryFilterData(FilterData);
	pShape->setLocalPose(ShapeLocalPose);
	const bool attached = pActor->attachShape(*pShape);
	pShape->release();
	if (!attached)
	{
		pActor->release();
		return E_FAIL;
	}

	// dynamic에만 damping·kinematic mode·질량/관성을 적용한다. 개별 gravity scale은 이후 고정 step에서 소비한다.
	if (PxRigidDynamic* pDynamic = pActor->is<PxRigidDynamic>())
	{
		pDynamic->setLinearDamping(Desc.fLinearDamping);
		pDynamic->setAngularDamping(Desc.fAngularDamping);
		pDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		pDynamic->setRigidBodyFlag(
			PxRigidBodyFlag::eKINEMATIC,
			PHYSICS_MOTION_MODE::KINEMATIC == Desc.eMotionMode);
		if (!PxRigidBodyExt::updateMassAndInertia(
			*pDynamic, Desc.fDensity))
		{
			pActor->release();
			return E_FAIL;
		}
	}

	pActor->setActorFlag(
		PxActorFlag::eDISABLE_SIMULATION,
		!Desc.isActive);

	// backend 준비 뒤 빈 slot을 재사용하거나 저장소를 늘린다. 이 구간의 vector 할당 예외를 따로 rollback하지는 않는다.
	uint32_t iSlot = 0u;
	if (m_pImpl->FreeSlots.empty())
	{
		iSlot = static_cast<uint32_t>(m_pImpl->ActorSlots.size());
		m_pImpl->ActorSlots.emplace_back();
	}
	else
	{
		iSlot = m_pImpl->FreeSlots.back();
		m_pImpl->FreeSlots.pop_back();
	}

	IMPL::ACTOR_SLOT& Slot = m_pImpl->ActorSlots[iSlot];
	Slot.pActor = pActor;
	Slot.iLevelID = Desc.iLevelID;
	Slot.isGravityEnabled = Desc.isGravityEnabled;
	Slot.fGravityScale = Desc.fGravityScale;
	Slot.vConstantAcceleration = Desc.vConstantAcceleration;
	Slot.isActive = Desc.isActive;

	// scene와 Level 소유 목록에 등록을 마친 뒤 외부에 handle을 공개한다.
	m_pImpl->pScene->addActor(*pActor);
	m_pImpl->LevelActorSlots[Desc.iLevelID].push_back(iSlot);
	outHandle = PHYSICS_ACTOR_HANDLE { iSlot, Slot.iGeneration };
	m_pImpl->RefreshActorCount();
	return S_OK;
}

// 현재 세대가 맞는 actor만 제거한다. 삭제 후 generation을 증가시키고0을 건너뛰어 이전 handle을 무효화한다.
HRESULT CPhysics_Manager::Destroy_Actor(PHYSICS_ACTOR_HANDLE Handle)
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	if (nullptr == pSlot)
		return S_FALSE;

	if (nullptr != m_pImpl->pScene && nullptr != pSlot->pActor->getScene())
		m_pImpl->pScene->removeActor(*pSlot->pActor);
	pSlot->pActor->release();
	pSlot->pActor = nullptr;

	if (pSlot->iLevelID < m_pImpl->LevelActorSlots.size())
	{
		auto& LevelSlots = m_pImpl->LevelActorSlots[pSlot->iLevelID];
		LevelSlots.erase(
			std::remove(LevelSlots.begin(), LevelSlots.end(), Handle.iSlot),
			LevelSlots.end());
	}

	pSlot->iLevelID = PHYSICS_ACTOR_HANDLE::INVALID_SLOT;
	pSlot->isGravityEnabled = true;
	pSlot->fGravityScale = 1.f;
	pSlot->vConstantAcceleration = {};
	pSlot->isActive = true;
	++pSlot->iGeneration;
	if (0u == pSlot->iGeneration)
		pSlot->iGeneration = 1u;
	m_pImpl->FreeSlots.push_back(Handle.iSlot);
	m_pImpl->RefreshActorCount();
	return S_OK;
}

// root pose 변경과 dynamic 속도0을 backend에 요청한다. clearForce/clearTorque는 기본 eFORCE 누적만 대상으로 한다.
// active/kinematic 추가 검사나 backend 적용 성공 확인은 없고 impulse/velocity-change 누적을 별도로 지우지는 않는다.
HRESULT CPhysics_Manager::Reset_Actor(
	PHYSICS_ACTOR_HANDLE Handle,
	const PHYSICS_POSE& Pose)
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxTransform ActorPose(PxIdentity);
	if (nullptr == pSlot || !TryMakeTransform(Pose, ActorPose))
		return E_INVALIDARG;

	pSlot->pActor->setGlobalPose(ActorPose, false);
	if (PxRigidDynamic* pDynamic = pSlot->pActor->is<PxRigidDynamic>())
	{
		pDynamic->setLinearVelocity(PxVec3(PxZero), false);
		pDynamic->setAngularVelocity(PxVec3(PxZero), false);
		pDynamic->clearForce();
		pDynamic->clearTorque();
		if (!pDynamic->getRigidBodyFlags().isSet(
			PxRigidBodyFlag::eKINEMATIC))
		{
			pDynamic->wakeUp();
		}
	}
	return S_OK;
}

// slot 표식만 믿지 않고 현재 manager에 동일 세대의 actor가 있는지 확인한다.
bool_t CPhysics_Manager::Is_ActorValid(PHYSICS_ACTOR_HANDLE Handle) const
{
	return nullptr != m_pImpl && nullptr != m_pImpl->Find(Handle);
}

// kinematic은 다음 step의 목표 pose를 지정한다. static/simulated는 global pose를 바로 설정한다.
HRESULT CPhysics_Manager::Set_Pose(
	PHYSICS_ACTOR_HANDLE Handle,
	const PHYSICS_POSE& Pose)
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxTransform ActorPose(PxIdentity);
	if (nullptr == pSlot || !TryMakeTransform(Pose, ActorPose))
		return E_INVALIDARG;

	if (PxRigidDynamic* pDynamic = pSlot->pActor->is<PxRigidDynamic>();
		nullptr != pDynamic && pDynamic->getRigidBodyFlags().isSet(
			PxRigidBodyFlag::eKINEMATIC))
	{
		pDynamic->setKinematicTarget(ActorPose);
	}
	else
	{
		pSlot->pActor->setGlobalPose(ActorPose, true);
	}
	return S_OK;
}

// 검증을 통과한 뒤에만 outPose를 바꾸므로 실패한 조회는 기존 출력 값을 유지한다.
HRESULT CPhysics_Manager::Get_Pose(
	PHYSICS_ACTOR_HANDLE Handle,
	PHYSICS_POSE& outPose) const
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	const IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	if (nullptr == pSlot)
		return E_INVALIDARG;

	outPose = ToPhysicsPose(pSlot->pActor->getGlobalPose());
	return S_OK;
}

// simulated dynamic의 월드 선속도를 설정한다. active flag는 이 wrapper에서 추가 검사하지 않는다.
HRESULT CPhysics_Manager::Set_LinearVelocity(
	PHYSICS_ACTOR_HANDLE Handle,
	const float3_t& vVelocity)
{
	if (nullptr == m_pImpl || !IsFinite(vVelocity))
		return E_INVALIDARG;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic || pDynamic->getRigidBodyFlags().isSet(
		PxRigidBodyFlag::eKINEMATIC))
	{
		return E_INVALIDARG;
	}

	pDynamic->setLinearVelocity(ToPxVec3(vVelocity), true);
	return S_OK;
}

// dynamic의 질량 중심 선속도를 읽는다. kinematic 조회도 허용하며 실패는 출력을 보존한다.
HRESULT CPhysics_Manager::Get_LinearVelocity(
	PHYSICS_ACTOR_HANDLE Handle,
	float3_t& outVelocity) const
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	const IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	const PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	outVelocity = ToFloat3(pDynamic->getLinearVelocity());
	return S_OK;
}

// 유한한 월드 각속도(rad/s)를 simulated dynamic에 전달한다. static/kinematic에는 쓰지 않는다.
HRESULT CPhysics_Manager::Set_AngularVelocity(
	PHYSICS_ACTOR_HANDLE Handle,
	const float3_t& vVelocity)
{
	if (nullptr == m_pImpl || !IsFinite(vVelocity))
		return E_INVALIDARG;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic || pDynamic->getRigidBodyFlags().isSet(
		PxRigidBodyFlag::eKINEMATIC))
	{
		return E_INVALIDARG;
	}

	pDynamic->setAngularVelocity(ToPxVec3(vVelocity), true);
	return S_OK;
}

// dynamic의 현재 각속도를 읽고 성공 경로에서만 outVelocity에 대입한다.
HRESULT CPhysics_Manager::Get_AngularVelocity(
	PHYSICS_ACTOR_HANDLE Handle,
	float3_t& outVelocity) const
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	const IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	const PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	outVelocity = ToFloat3(pDynamic->getAngularVelocity());
	return S_OK;
}

// 활성 simulated dynamic인지 확인하고 모드별 입력을 누적한다. 즉시 pose를 직접 이동시키는 API는 아니다.
HRESULT CPhysics_Manager::Add_Force(
	PHYSICS_ACTOR_HANDLE Handle,
	const float3_t& vForce,
	PHYSICS_FORCE_MODE eMode)
{
	if (nullptr == m_pImpl || !IsFinite(vForce) ||
		PHYSICS_FORCE_MODE::END == eMode)
	{
		return E_INVALIDARG;
	}

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic || !pSlot->isActive ||
		pDynamic->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC))
	{
		return E_INVALIDARG;
	}

	pDynamic->addForce(ToPxVec3(vForce), ToPxForceMode(eMode), true);
	return S_OK;
}

// dynamic의 kinematic flag를 변경한다. 다시 활성 simulated가 된 actor는 다음 적분을 위해 깨운다.
HRESULT CPhysics_Manager::Set_MotionMode(
	PHYSICS_ACTOR_HANDLE Handle,
	PHYSICS_MOTION_MODE eMode)
{
	if (nullptr == m_pImpl || PHYSICS_MOTION_MODE::END == eMode)
		return E_INVALIDARG;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	const bool isKinematic = PHYSICS_MOTION_MODE::KINEMATIC == eMode;
	pDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematic);
	if (!isKinematic && pSlot->isActive)
		pDynamic->wakeUp();
	return S_OK;
}

// slot의 수동 중력 사용 여부를 변경한다. native gravity flag를 켜는 경로가 아니다.
HRESULT CPhysics_Manager::Set_GravityEnabled(
	PHYSICS_ACTOR_HANDLE Handle,
	bool_t isEnabled)
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	pSlot->isGravityEnabled = isEnabled;
	if (pSlot->isActive && !pDynamic->getRigidBodyFlags().isSet(
		PxRigidBodyFlag::eKINEMATIC))
	{
		pDynamic->wakeUp();
	}
	return S_OK;
}

// 유한한0 이상 배율을 보관하고 활성 simulated를 깨운다. 이미 저장된 일정 가속도에는 곱하지 않는다.
HRESULT CPhysics_Manager::Set_GravityScale(
	PHYSICS_ACTOR_HANDLE Handle,
	f32_t fGravityScale)
{
	if (nullptr == m_pImpl || !std::isfinite(fGravityScale) ||
		fGravityScale < 0.f)
	{
		return E_INVALIDARG;
	}

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	pSlot->fGravityScale = fGravityScale;
	if (pSlot->isActive && !pDynamic->getRigidBodyFlags().isSet(
		PxRigidBodyFlag::eKINEMATIC))
	{
		pDynamic->wakeUp();
	}
	return S_OK;
}

// 일정 가속도를 값으로 저장한다. step의 ApplyAccelerations가 중력과 합쳐 소비한다.
HRESULT CPhysics_Manager::Set_ConstantAcceleration(
	PHYSICS_ACTOR_HANDLE Handle,
	const float3_t& vAcceleration)
{
	if (nullptr == m_pImpl || !IsFinite(vAcceleration))
		return E_INVALIDARG;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	PxRigidDynamic* pDynamic = nullptr != pSlot ?
		pSlot->pActor->is<PxRigidDynamic>() : nullptr;
	if (nullptr == pDynamic)
		return E_INVALIDARG;

	pSlot->vConstantAcceleration = vAcceleration;
	if (pSlot->isActive && !pDynamic->getRigidBodyFlags().isSet(
		PxRigidBodyFlag::eKINEMATIC))
	{
		pDynamic->wakeUp();
	}
	return S_OK;
}

// simulation disable flag와 slot의 참여 상태를 맞춘다. scene 등록·Level 소속·handle은 유지한다.
HRESULT CPhysics_Manager::Set_Active(
	PHYSICS_ACTOR_HANDLE Handle,
	bool_t isActive)
{
	if (nullptr == m_pImpl)
		return E_FAIL;

	IMPL::ACTOR_SLOT* pSlot = m_pImpl->Find(Handle);
	if (nullptr == pSlot)
		return E_INVALIDARG;

	pSlot->pActor->setActorFlag(
		PxActorFlag::eDISABLE_SIMULATION,
		!isActive);
	pSlot->isActive = isActive;
	if (isActive)
	{
		if (PxRigidDynamic* pDynamic =
			pSlot->pActor->is<PxRigidDynamic>();
			nullptr != pDynamic && !pDynamic->getRigidBodyFlags().isSet(
				PxRigidBodyFlag::eKINEMATIC))
		{
			pDynamic->wakeUp();
		}
	}
	return S_OK;
}

// Effect_Playback의 표현 충돌이 호출한다. static actor만 대상으로 이동하는 축 정렬 box의 첫 blocking hit를 구한다.
// 시작부터 outHit를 초기화하며 입력 오류/충돌 없음/유효하지 않은 법선 모두 false다.
bool_t CPhysics_Manager::Sweep_StaticBox(const float3_t& vStart, const float3_t& vEnd,
	const float3_t& vHalfExtents, PHYSICS_STATIC_SWEEP_HIT& outHit) const
{
	outHit = {};
	if (!m_pImpl || !m_pImpl->pScene)
		return false;
	const PxVec3 Start = ToPxVec3(vStart), End = ToPxVec3(vEnd);
	const PxVec3 Extent = ToPxVec3(vHalfExtents);
	if (!Start.isFinite() || !End.isFinite() || !Extent.isFinite() ||
		Extent.x < 0.f || Extent.y < 0.f || Extent.z < 0.f)
			return false;
	// 월드 거리와 단위 이동 방향을 구한다. 길이가 거의0이면 sweep하지 않는다.
	const PxVec3 Delta = End - Start;
	const float Distance = Delta.magnitude();
	if (Distance <= 1.e-6f)
		return false;
	PxSweepBuffer Hit;
	// 0 반폭은 backend용 최소1e-4로 보정한다. 회전은 identity이며 별도의 gameplay collision mask는 적용하지 않는다.
	const PxBoxGeometry Box(PxMax(Extent.x, 1.e-4f),
		PxMax(Extent.y, 1.e-4f), PxMax(Extent.z, 1.e-4f));
	const PxQueryFilterData Filter(PxQueryFlag::eSTATIC);
	if (!m_pImpl->pScene->sweep(Box, PxTransform(Start), Delta / Distance,
		Distance, Hit, PxHitFlag::eNORMAL | PxHitFlag::ePOSITION, Filter) ||
		!Hit.hasBlock)
			return false;
	const auto& Block = Hit.block;
	if (!Block.normal.isFinite() || Block.normal.magnitudeSquared() < 1.e-8f)
		return false;
	// 접촉 법선과 충돌 이동거리/전체거리 비율을 전달한다. 소비자가 반사·남은 이동 처리를 결정한다.
	outHit.vNormal = float3_t(Block.normal.x, Block.normal.y, Block.normal.z);
	outHit.fTravelFraction = PxClamp(Block.distance / Distance, 0.f, 1.f);
	return true;
}

// 프레임 clock을 멈추거나 재개한다. accumulator는 유지하고 재개할 때 single-step 예약만 지운다.
void CPhysics_Manager::Set_DebugPaused(bool_t isPaused)
{
	if (nullptr == m_pImpl)
		return;
	m_pImpl->isPaused = isPaused;
	if (!isPaused)
		m_pImpl->isSingleStepRequested = false;
	m_pImpl->StepStats.isPaused = isPaused;
}

// 현재 정지 상태를 값으로 반환한다.
bool_t CPhysics_Manager::Is_DebugPaused() const
{
	return nullptr != m_pImpl && m_pImpl->isPaused;
}

// 정지 중 다음 Update에서 한 번 실행할 flag를 세운다. 반복 요청으로 step 수가 누적되지는 않는다.
HRESULT CPhysics_Manager::Request_DebugSingleStep()
{
	if (nullptr == m_pImpl || !m_pImpl->isPaused)
		return E_FAIL;
	m_pImpl->isSingleStepRequested = true;
	return S_OK;
}

// 저작 timeline 재구축이 정지 상태에서만 호출한다. 최대3600회이며 모든 결과를 동기 수집한 뒤 통계를 갱신한다.
HRESULT CPhysics_Manager::Simulate_DebugSteps(uint32_t iStepCount)
{
	if (nullptr == m_pImpl || nullptr == m_pImpl->pScene ||
		!m_pImpl->isPaused || 0u == iStepCount ||
		iStepCount > MAX_DEBUG_STEPS_PER_CALL)
	{
		return E_INVALIDARG;
	}

	const auto begin = std::chrono::steady_clock::now();
	for (uint32_t i = 0u; i < iStepCount; ++i)
		m_pImpl->Step();

	const std::chrono::duration<f32_t, std::milli> elapsed =
		std::chrono::steady_clock::now() - begin;
	m_pImpl->StepStats.iStepsLastFrame = iStepCount;
	m_pImpl->StepStats.fLastSimulateMs = elapsed.count();
	m_pImpl->StepStats.fAccumulatorSeconds = m_pImpl->fAccumulator;
	m_pImpl->StepStats.isPaused = true;
	m_pImpl->isSingleStepRequested = false;
	m_pImpl->RefreshActorCount();
	return S_OK;
}

// manager 통계를 읽기 전용으로 차용한다. 구현이 없을 때의 정적 빈 값도 수정하면 안 된다.
const PHYSICS_STEP_STATS& CPhysics_Manager::Get_StepStats() const
{
	static const PHYSICS_STEP_STATS EMPTY_STATS = {};
	return nullptr != m_pImpl ? m_pImpl->StepStats : EMPTY_STATS;
}

// Level의 등록 slot 수를 반환한다. 잘못된 ID는 빈 Level과 마찬가지로0이다.
uint32_t CPhysics_Manager::Get_ActorCount(uint32_t iLevelID) const
{
	if (nullptr == m_pImpl || iLevelID >= m_pImpl->LevelActorSlots.size())
		return 0u;
	return static_cast<uint32_t>(
		m_pImpl->LevelActorSlots[iLevelID].size());
}

// 별도 manager로 제한된 물리 계약을 확인하는 기존 검사다. 출력은 관측한 단계까지 기록하며 임시 actor는 manager 소멸로 정리한다.
HRESULT CPhysics_Manager::Run_ContractTest(
	PHYSICS_CONTRACT_RESULT& outResult)
{
	outResult = {};
	auto pManager = CPhysics_Manager::Create(2u);
	if (nullptr == pManager)
		return E_FAIL;

	// 지면 top은 Y=0, box shape의 중심은 actor root에서 Y+0.5다. 착지 시 root와 shape 높이를 구분해 확인한다.
	PHYSICS_RIGID_BODY_DESC SupportDesc;
	SupportDesc.iLevelID = 1u;
	SupportDesc.eActorType = PHYSICS_ACTOR_TYPE::STATIC;
	SupportDesc.Pose.vPosition = float3_t(0.f, -0.5f, 0.f);
	SupportDesc.vBoxHalfExtents = float3_t(5.f, 0.5f, 5.f);
	SupportDesc.isGravityEnabled = false;

	PHYSICS_RIGID_BODY_DESC BoxDesc;
	BoxDesc.iLevelID = 1u;
	BoxDesc.Pose.vPosition = float3_t(0.f, 3.f, 0.f);
	BoxDesc.ShapeLocalPose.vPosition = float3_t(0.f, 0.5f, 0.f);
	BoxDesc.vBoxHalfExtents = float3_t(0.5f, 0.5f, 0.5f);
	BoxDesc.fDensity = 10.f;

	PHYSICS_ACTOR_HANDLE SupportHandle;
	PHYSICS_ACTOR_HANDLE BoxHandle;
	if (FAILED(pManager->Create_Actor(SupportDesc, SupportHandle)) ||
		FAILED(pManager->Create_Actor(BoxDesc, BoxHandle)))
	{
		return E_FAIL;
	}

	PHYSICS_POSE Pose;
	if (FAILED(pManager->Get_Pose(BoxHandle, Pose)))
		return E_FAIL;
	outResult.fInitialHeight = Pose.vPosition.y;

	// 반 step의 잔여 시간을 만든 뒤 동기 저작 재구축이 이를 소비하지 않는지 확인한다.
	// 일반 실행 상태에서 debug 동기 step 요청을 거절하는지도 별도로 확인한다.
	pManager->Update(FIXED_TIMESTEP * 0.5f);
	const f32_t fAccumulatorBefore =
		pManager->Get_StepStats().fAccumulatorSeconds;
	outResult.didRejectUnpausedDebugSteps =
		FAILED(pManager->Simulate_DebugSteps(1u));

	pManager->Set_DebugPaused(true);
	if (FAILED(pManager->Simulate_DebugSteps(180u)))
		return E_FAIL;
	outResult.iStepsExecuted =
		pManager->Get_StepStats().iStepsLastFrame;
	outResult.didPreserveDebugAccumulator = std::fabs(
		pManager->Get_StepStats().fAccumulatorSeconds -
		fAccumulatorBefore) <= 1.e-7f;
	outResult.didRejectInvalidDebugStepCount =
		FAILED(pManager->Simulate_DebugSteps(0u)) &&
		FAILED(pManager->Simulate_DebugSteps(
			MAX_DEBUG_STEPS_PER_CALL + 1u));

	float3_t vVelocity;
	if (FAILED(pManager->Get_Pose(BoxHandle, Pose)) ||
		FAILED(pManager->Get_LinearVelocity(BoxHandle, vVelocity)))
	{
		return E_FAIL;
	}

	// 180step 뒤 actor root 높이와 shape 중심 높이, 수직 속도를 각각 허용 범위와 대조한다.
	outResult.fFinalHeight = Pose.vPosition.y;
	outResult.didRespectShapeLocalPose =
		outResult.fFinalHeight >= -0.05f &&
		outResult.fFinalHeight <= 0.15f;
	const f32_t fShapeCenterHeight = outResult.fFinalHeight + 0.5f;
	outResult.didSettleOnSupport =
		fShapeCenterHeight >= 0.45f &&
		fShapeCenterHeight <= 0.65f &&
		std::fabs(vVelocity.y) <= 0.25f;

	// 같은 Level을 비운 뒤 이전 두 handle이 모두 거절되는지와 남은 actor 수를 확인한다.
	if (FAILED(pManager->Clear(1u)))
		return E_FAIL;
	outResult.didInvalidateOnClear =
		!pManager->Is_ActorValid(SupportHandle) &&
		!pManager->Is_ActorValid(BoxHandle) &&
		0u == pManager->Get_ActorCount(1u);

	return outResult.didRejectUnpausedDebugSteps &&
		outResult.didPreserveDebugAccumulator &&
		outResult.didRejectInvalidDebugStepCount &&
		outResult.didRespectShapeLocalPose &&
		outResult.didSettleOnSupport &&
		outResult.didInvalidateOnClear &&
		180u == outResult.iStepsExecuted ? S_OK : E_FAIL;
}

// 새 manager를 준비해 소유권을 넘긴다. 초기화 실패한 지역 객체는 소멸하며 보유 backend를 해제한다.
unique_ptr<CPhysics_Manager> CPhysics_Manager::Create(uint32_t iNumLevels)
{
	auto pInstance = unique_ptr<CPhysics_Manager>(new CPhysics_Manager());
	if (FAILED(pInstance->Initialize(iNumLevels)))
		return nullptr;
	return pInstance;
}
