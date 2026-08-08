#include "Physics_Manager.h"

#ifdef new
#pragma push_macro("new")
#undef new
#define LOSTARK_RESTORE_DEBUG_NEW
#endif
#include <PxPhysicsAPI.h>
#ifdef LOSTARK_RESTORE_DEBUG_NEW
#pragma pop_macro("new")
#undef LOSTARK_RESTORE_DEBUG_NEW
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>
#include <vector>

using namespace physx;

namespace
{
	PxDefaultAllocator g_PhysicsAllocator;
	PxDefaultErrorCallback g_PhysicsErrorCallback;

	bool IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool IsFinite(const float4_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z) && std::isfinite(value.w);
	}

	PxVec3 ToPxVec3(const float3_t& value)
	{
		return PxVec3(value.x, value.y, value.z);
	}

	float3_t ToFloat3(const PxVec3& value)
	{
		return float3_t(value.x, value.y, value.z);
	}

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

	PHYSICS_POSE ToPhysicsPose(const PxTransform& Pose)
	{
		PHYSICS_POSE result;
		result.vPosition = ToFloat3(Pose.p);
		result.vRotationQuaternion = float4_t(
			Pose.q.x, Pose.q.y, Pose.q.z, Pose.q.w);
		return result;
	}

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

struct CPhysics_Manager::IMPL
{
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

	PxFoundation* pFoundation = nullptr;
	PxPvd* pPvd = nullptr;
	PxPvdTransport* pPvdTransport = nullptr;
	PxPhysics* pPhysics = nullptr;
	PxDefaultCpuDispatcher* pDispatcher = nullptr;
	PxScene* pScene = nullptr;

	std::vector<ACTOR_SLOT> ActorSlots;
	std::vector<uint32_t> FreeSlots;
	std::vector<std::vector<uint32_t>> LevelActorSlots;

	f32_t fAccumulator = 0.f;
	bool_t isPaused = false;
	bool_t isSingleStepRequested = false;
	PHYSICS_STEP_STATS StepStats = {};

	ACTOR_SLOT* Find(PHYSICS_ACTOR_HANDLE Handle)
	{
		if (!Handle.Is_Valid() || Handle.iSlot >= ActorSlots.size())
			return nullptr;

		ACTOR_SLOT& Slot = ActorSlots[Handle.iSlot];
		return nullptr != Slot.pActor &&
			Slot.iGeneration == Handle.iGeneration ? &Slot : nullptr;
	}

	const ACTOR_SLOT* Find(PHYSICS_ACTOR_HANDLE Handle) const
	{
		if (!Handle.Is_Valid() || Handle.iSlot >= ActorSlots.size())
			return nullptr;

		const ACTOR_SLOT& Slot = ActorSlots[Handle.iSlot];
		return nullptr != Slot.pActor &&
			Slot.iGeneration == Handle.iGeneration ? &Slot : nullptr;
	}

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

	void Step()
	{
		ApplyAccelerations();
		pScene->simulate(CPhysics_Manager::FIXED_TIMESTEP);
		pScene->fetchResults(true);
	}

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

CPhysics_Manager::CPhysics_Manager()
	: m_pImpl { make_unique<IMPL>() }
{
}

CPhysics_Manager::~CPhysics_Manager()
{
	if (nullptr != m_pImpl)
		m_pImpl->Release();
}

HRESULT CPhysics_Manager::Initialize(uint32_t iNumLevels)
{
	if (0u == iNumLevels || nullptr == m_pImpl)
		return E_INVALIDARG;

	m_pImpl->pFoundation = PxCreateFoundation(
		PX_PHYSICS_VERSION,
		g_PhysicsAllocator,
		g_PhysicsErrorCallback);
	if (nullptr == m_pImpl->pFoundation)
		return E_FAIL;

#ifdef _DEBUG
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

	m_pImpl->pDispatcher = PxDefaultCpuDispatcherCreate(2u);
	if (nullptr == m_pImpl->pDispatcher)
	{
		m_pImpl->Release();
		return E_FAIL;
	}

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
		/* A paused authoring clock does not absorb frame time. One request is
		   exactly one fixed step and preserves the pre-pause residual. */
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
		while (m_pImpl->fAccumulator >= FIXED_TIMESTEP &&
			Stats.iStepsLastFrame < MAX_STEPS_PER_FRAME)
		{
			m_pImpl->Step();
			m_pImpl->fAccumulator -= FIXED_TIMESTEP;
			++Stats.iStepsLastFrame;
		}

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

	PxTransform ActorPose(PxIdentity);
	PxTransform ShapeLocalPose(PxIdentity);
	if (!TryMakeTransform(Desc.Pose, ActorPose) ||
		!TryMakeTransform(Desc.ShapeLocalPose, ShapeLocalPose))
		return E_INVALIDARG;

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

	pMaterial->release();
	if (nullptr == pShape)
	{
		pActor->release();
		return E_INVALIDARG;
	}

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

	m_pImpl->pScene->addActor(*pActor);
	m_pImpl->LevelActorSlots[Desc.iLevelID].push_back(iSlot);
	outHandle = PHYSICS_ACTOR_HANDLE { iSlot, Slot.iGeneration };
	m_pImpl->RefreshActorCount();
	return S_OK;
}

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

bool_t CPhysics_Manager::Is_ActorValid(PHYSICS_ACTOR_HANDLE Handle) const
{
	return nullptr != m_pImpl && nullptr != m_pImpl->Find(Handle);
}

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

void CPhysics_Manager::Set_DebugPaused(bool_t isPaused)
{
	if (nullptr == m_pImpl)
		return;
	m_pImpl->isPaused = isPaused;
	if (!isPaused)
		m_pImpl->isSingleStepRequested = false;
	m_pImpl->StepStats.isPaused = isPaused;
}

bool_t CPhysics_Manager::Is_DebugPaused() const
{
	return nullptr != m_pImpl && m_pImpl->isPaused;
}

HRESULT CPhysics_Manager::Request_DebugSingleStep()
{
	if (nullptr == m_pImpl || !m_pImpl->isPaused)
		return E_FAIL;
	m_pImpl->isSingleStepRequested = true;
	return S_OK;
}

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

const PHYSICS_STEP_STATS& CPhysics_Manager::Get_StepStats() const
{
	static const PHYSICS_STEP_STATS EMPTY_STATS = {};
	return nullptr != m_pImpl ? m_pImpl->StepStats : EMPTY_STATS;
}

uint32_t CPhysics_Manager::Get_ActorCount(uint32_t iLevelID) const
{
	if (nullptr == m_pImpl || iLevelID >= m_pImpl->LevelActorSlots.size())
		return 0u;
	return static_cast<uint32_t>(
		m_pImpl->LevelActorSlots[iLevelID].size());
}

HRESULT CPhysics_Manager::Run_ContractTest(
	PHYSICS_CONTRACT_RESULT& outResult)
{
	outResult = {};
	auto pManager = CPhysics_Manager::Create(2u);
	if (nullptr == pManager)
		return E_FAIL;

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

	/* Leave a real sub-step residual, then prove that a synchronous authoring
	   rebuild neither consumes it nor accepts calls from the runtime clock. */
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

	outResult.fFinalHeight = Pose.vPosition.y;
	outResult.didRespectShapeLocalPose =
		outResult.fFinalHeight >= -0.05f &&
		outResult.fFinalHeight <= 0.15f;
	const f32_t fShapeCenterHeight = outResult.fFinalHeight + 0.5f;
	outResult.didSettleOnSupport =
		fShapeCenterHeight >= 0.45f &&
		fShapeCenterHeight <= 0.65f &&
		std::fabs(vVelocity.y) <= 0.25f;

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

unique_ptr<CPhysics_Manager> CPhysics_Manager::Create(uint32_t iNumLevels)
{
	auto pInstance = unique_ptr<CPhysics_Manager>(new CPhysics_Manager());
	if (FAILED(pInstance->Initialize(iNumLevels)))
		return nullptr;
	return pInstance;
}
