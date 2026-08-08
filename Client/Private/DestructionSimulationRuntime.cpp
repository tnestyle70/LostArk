#include "DestructionSimulationRuntime.h"

#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "GameInstance.h"
#include "RigidBody.h"
#include "WorldDestructionDocument.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
	constexpr f32_t PREVIEW_GROUND_HALF_THICKNESS = 0.25f;
	constexpr f32_t MINIMUM_SHAPE_HALF_EXTENT = 0.01f;

	const Client::DEPLOY_RUNTIME_ENTRY* Find_Entry(
		const Client::CDeployPropRuntime& runtime,
		const uint64_t placementId)
	{
		const auto& entries = runtime.Get_Entries();
		const auto found = std::find_if(entries.begin(), entries.end(),
			[placementId](const Client::DEPLOY_RUNTIME_ENTRY& entry)
			{
				return entry.placement.runtimePlacementId == placementId;
			});
		return found == entries.end() ? nullptr : &*found;
	}

	bool_t Is_FinitePose(
		const float3_t& position,
		const float4_t& rotation)
	{
		return std::isfinite(position.x) && std::isfinite(position.y) &&
			std::isfinite(position.z) && std::isfinite(rotation.x) &&
			std::isfinite(rotation.y) && std::isfinite(rotation.z) &&
			std::isfinite(rotation.w);
	}
}

class Client::CDestructionSimulationRuntime::CPhysicsAdapter final
{
public:
	~CPhysicsAdapter()
	{
		Clear_Ground();
		End_ExclusiveClock();
	}

	bool_t Begin_ExclusiveClock(std::string& outStatus)
	{
		if (m_ownsClock)
			return true;
		m_pManager = CGameInstance::Get().Get_PhysicsManager();
		if (nullptr == m_pManager)
		{
			outStatus = "Physics manager is unavailable for destruction preview";
			return false;
		}
		m_wasPaused = m_pManager->Get_StepStats().isPaused;
		m_pManager->Set_DebugPaused(true);
		m_ownsClock = true;
		return true;
	}

	void End_ExclusiveClock()
	{
		Engine::CPhysics_Manager* current =
			CGameInstance::Get().Get_PhysicsManager();
		if (m_ownsClock && nullptr != m_pManager && current == m_pManager)
			current->Set_DebugPaused(m_wasPaused);
		m_pManager = nullptr;
		m_ownsClock = false;
		m_wasPaused = false;
	}

	bool_t Simulate_Steps(
		const uint32_t stepCount,
		std::string& outStatus)
	{
		Engine::CPhysics_Manager* current =
			CGameInstance::Get().Get_PhysicsManager();
		if (!m_ownsClock || nullptr == m_pManager || current != m_pManager ||
			0u == stepCount ||
			FAILED(current->Simulate_DebugSteps(stepCount)))
		{
			outStatus = "PhysX rejected synchronous destruction preview steps";
			return false;
		}
		return true;
	}

	bool_t Create_DynamicBox(
		const uint32_t levelId,
		const float3_t& position,
		const float4_t& rotation,
		const float3_t& shapeLocalCentre,
		const float3_t& halfExtents,
		const float3_t& linearVelocity,
		const f32_t gravityScale,
		shared_ptr<Engine::CRigidBody>& outBody,
		std::string& outStatus)
	{
		Engine::PHYSICS_RIGID_BODY_DESC desc{};
		desc.iLevelID = levelId;
		desc.eActorType = Engine::PHYSICS_ACTOR_TYPE::DYNAMIC;
		desc.eMotionMode = Engine::PHYSICS_MOTION_MODE::SIMULATED;
		desc.eShapeType = Engine::PHYSICS_SHAPE_TYPE::BOX;
		desc.Pose.vPosition = position;
		desc.Pose.vRotationQuaternion = rotation;
		desc.ShapeLocalPose.vPosition = shapeLocalCentre;
		desc.vBoxHalfExtents = {
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.x),
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.y),
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.z)
		};
		desc.fDensity = 10.f;
		desc.fStaticFriction = 0.6f;
		desc.fDynamicFriction = 0.5f;
		desc.fRestitution = 0.15f;
		desc.fLinearDamping = 0.05f;
		desc.fAngularDamping = 0.1f;
		desc.isGravityEnabled = gravityScale > 0.f;
		desc.fGravityScale = gravityScale;
		desc.isActive = true;

		shared_ptr<Engine::CRigidBody> staged =
			Engine::CRigidBody::Create_Runtime(desc);
		if (nullptr == staged ||
			FAILED(staged->Set_LinearVelocity(linearVelocity)))
		{
			if (nullptr != staged)
				staged->Destroy_Actor();
			outStatus = "PhysX rejected a destruction debris actor";
			return false;
		}
		outBody = std::move(staged);
		return true;
	}

	bool_t Create_Ground(
		const uint32_t levelId,
		const float3_t& centre,
		const f32_t topHeight,
		const float2_t& halfExtents,
		std::string& outStatus)
	{
		Engine::PHYSICS_RIGID_BODY_DESC desc{};
		desc.iLevelID = levelId;
		desc.eActorType = Engine::PHYSICS_ACTOR_TYPE::STATIC;
		desc.eMotionMode = Engine::PHYSICS_MOTION_MODE::SIMULATED;
		desc.eShapeType = Engine::PHYSICS_SHAPE_TYPE::BOX;
		desc.Pose.vPosition = {
			centre.x,
			topHeight - PREVIEW_GROUND_HALF_THICKNESS,
			centre.z
		};
		desc.vBoxHalfExtents = {
			halfExtents.x,
			PREVIEW_GROUND_HALF_THICKNESS,
			halfExtents.y
		};
		desc.isGravityEnabled = false;
		desc.fGravityScale = 0.f;
		desc.isActive = true;
		m_pGround = Engine::CRigidBody::Create_Runtime(desc);
		if (nullptr == m_pGround)
		{
			outStatus = "PhysX rejected the tool-only preview ground";
			return false;
		}
		return true;
	}

	void Clear_Ground()
	{
		if (nullptr != m_pGround)
			m_pGround->Destroy_Actor();
		m_pGround.reset();
	}

private:
	shared_ptr<Engine::CRigidBody> m_pGround;
	Engine::CPhysics_Manager* m_pManager = nullptr;
	bool_t m_ownsClock = false;
	bool_t m_wasPaused = false;
};

struct Client::CDestructionSimulationRuntime::ELEMENT_RUNTIME final
{
	DESTRUCTION_SIMULATION_ELEMENT Desc;
	shared_ptr<CDeployPropObject> pObject;
	shared_ptr<Engine::CRigidBody> pBody;
	float3_t vInitialPosition{};
	float4_t vInitialRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vShapeLocalCentre{};
	float3_t vHalfExtents = { 0.5f, 0.5f, 0.5f };
	float3_t vCurrentPosition{};
	float4_t vCurrentRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vLinearVelocity{};
	f32_t fActivatedAtSeconds = 0.f;
	DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
		DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
};

Client::CDestructionSimulationRuntime::CDestructionSimulationRuntime()
	: m_pPhysics(std::make_unique<CPhysicsAdapter>())
{
}

Client::CDestructionSimulationRuntime::~CDestructionSimulationRuntime()
{
	Clear();
}

bool_t Client::CDestructionSimulationRuntime::Validate_Stage(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const std::string& selectedGroupId,
	const CWorldDestructionDocument& destructionDocument,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus) const
{
	if (!CDestructionSimulationDocument::Validate_Profile(profile, outStatus))
		return false;
	if (profile.groupId != selectedGroupId)
	{
		outStatus = "Simulation profile does not belong to the selected group";
		return false;
	}
	const DESTRUCTION_GROUP* group = destructionDocument.Find_Group(
		selectedGroupId);
	if (nullptr == group || group->memberPlacementIds.empty())
	{
		outStatus = "Selected destruction group is missing or empty";
		return false;
	}
	if (profile.Elements.size() != group->memberPlacementIds.size())
	{
		outStatus = "Simulation profile must include every selected group member";
		return false;
	}
	for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
	{
		if (group->memberPlacementIds.end() == std::find(
			group->memberPlacementIds.begin(), group->memberPlacementIds.end(),
			element.sourceRuntimePlacementId))
		{
			outStatus = "Simulation element is outside the selected group: " +
				element.elementId;
			return false;
		}
		const shared_ptr<CDeployPropObject> object = deployRuntime.Find(
			element.sourceRuntimePlacementId);
		float3_t localCentre{};
		float3_t halfExtents{};
		if (nullptr == object || !object->Is_Destructible() ||
			!object->Get_PhysicsPreviewLocalBounds(
				localCentre, halfExtents))
		{
			outStatus = "Simulation element has no loaded destructible bounds: " +
				element.elementId;
			return false;
		}
	}
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Stage_Profile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const std::string& selectedGroupId,
	const CWorldDestructionDocument& destructionDocument,
	CDeployPropRuntime& deployRuntime,
	const uint32_t levelId,
	std::string& outStatus)
{
	if (levelId >= ETOUI(LEVEL::END) ||
		!Validate_Stage(profile, selectedGroupId, destructionDocument,
			deployRuntime, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Destruction simulation stage input is invalid";
		return false;
	}

	std::vector<ELEMENT_RUNTIME> stagedElements;
	stagedElements.reserve(profile.Elements.size());
	float3_t groupCentre{};
	for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
	{
		const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
			deployRuntime, element.sourceRuntimePlacementId);
		if (nullptr == entry || nullptr == entry->object)
		{
			outStatus = "DeployProp runtime changed during simulation staging";
			return false;
		}
		float3_t localCentre{};
		float3_t boundsHalfExtents{};
		if (!entry->object->Get_PhysicsPreviewLocalBounds(
			localCentre, boundsHalfExtents))
		{
			outStatus = "DeployProp bounds changed during simulation staging";
			return false;
		}

		ELEMENT_RUNTIME runtime;
		runtime.Desc = element;
		runtime.pObject = entry->object;
		runtime.vInitialPosition = {
			entry->placement.position.x + element.vSpawnOffset.x,
			entry->placement.position.y + element.vSpawnOffset.y,
			entry->placement.position.z + element.vSpawnOffset.z
		};
		runtime.vInitialRotation = entry->placement.rotationQuaternion;
		runtime.vShapeLocalCentre = localCentre;
		runtime.vHalfExtents = boundsHalfExtents;
		runtime.vCurrentPosition = runtime.vInitialPosition;
		runtime.vCurrentRotation = runtime.vInitialRotation;
		runtime.vLinearVelocity = {
			element.vDirection.x * element.fSpeedMetersPerSecond,
			element.vDirection.y * element.fSpeedMetersPerSecond,
			element.vDirection.z * element.fSpeedMetersPerSecond
		};
		groupCentre.x += entry->placement.position.x;
		groupCentre.y += entry->placement.position.y;
		groupCentre.z += entry->placement.position.z;
		stagedElements.push_back(std::move(runtime));
	}
	const f32_t inverseCount = 1.f /
		static_cast<f32_t>(stagedElements.size());
	groupCentre.x *= inverseCount;
	groupCentre.y *= inverseCount;
	groupCentre.z *= inverseCount;

	/* Actor creation uses the global Engine scene, but this preview owns its
	   clock while staged. Auto stepping stays paused and Controller performs
	   every exact 1/60 step synchronously. */
	Clear();
	unique_ptr<CPhysicsAdapter> stagedPhysics =
		std::make_unique<CPhysicsAdapter>();
	if (!stagedPhysics->Begin_ExclusiveClock(outStatus))
		return false;
	if (profile.isPreviewGroundEnabled &&
		!stagedPhysics->Create_Ground(
			levelId, groupCentre, profile.fPreviewGroundHeight,
			profile.vPreviewGroundHalfExtents, outStatus))
	{
		stagedPhysics->End_ExclusiveClock();
		return false;
	}

	m_pPhysics = std::move(stagedPhysics);
	m_Profile = profile;
	m_Elements = std::move(stagedElements);
	m_pDeployRuntime = &deployRuntime;
	m_iLevelId = levelId;
	m_isStaged = true;
	m_isPhysicsPaused = true;
	m_Status = "Staged destruction simulation: " + profile.profileId;
	if (!Reset(outStatus))
	{
		Clear();
		return false;
	}
	m_Status = "Staged destruction simulation: " + profile.profileId;
	outStatus = m_Status;
	return true;
}

void Client::CDestructionSimulationRuntime::Destroy_Actors()
{
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (nullptr != runtime.pBody)
			runtime.pBody->Destroy_Actor();
		runtime.pBody.reset();
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_PhysicsPreviewActive())
		{
			runtime.pObject->End_PhysicsPreview();
		}
	}
}

void Client::CDestructionSimulationRuntime::Clear()
{
	Destroy_Actors();
	if (nullptr != m_pPhysics)
	{
		m_pPhysics->Clear_Ground();
		m_pPhysics->End_ExclusiveClock();
	}
	m_Profile = {};
	m_Elements.clear();
	m_Frame = {};
	m_pDeployRuntime = nullptr;
	m_iLevelId = ETOUI(LEVEL::END);
	m_fSampleTimeSeconds = 0.f;
	m_eScope = DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS;
	m_SelectedElementId.clear();
	m_Status = "Destruction simulation is not staged";
	m_isStaged = false;
	m_isPhysicsPaused = true;
	if (nullptr == m_pPhysics)
		m_pPhysics = std::make_unique<CPhysicsAdapter>();
}

bool_t Client::CDestructionSimulationRuntime::Reset(
	std::string& outStatus)
{
	if (!m_isStaged)
	{
		outStatus = "Destruction simulation reset requires a staged profile";
		return false;
	}
	Destroy_Actors();
	m_fSampleTimeSeconds = 0.f;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		runtime.vCurrentPosition = runtime.vInitialPosition;
		runtime.vCurrentRotation = runtime.vInitialRotation;
		runtime.vLinearVelocity = {
			runtime.Desc.vDirection.x *
				runtime.Desc.fSpeedMetersPerSecond,
			runtime.Desc.vDirection.y *
				runtime.Desc.fSpeedMetersPerSecond,
			runtime.Desc.vDirection.z *
				runtime.Desc.fSpeedMetersPerSecond
		};
		runtime.fActivatedAtSeconds = 0.f;
		runtime.eState = Is_ElementInScope(runtime.Desc.elementId) ?
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING :
			DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED;
	}
	Rebuild_Frame();
	m_Status = "Reset destruction simulation to 0 s";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Is_ElementInScope(
	const std::string& elementId) const
{
	return DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == m_eScope ||
		(DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == m_eScope &&
			elementId == m_SelectedElementId);
}

bool_t Client::CDestructionSimulationRuntime::Set_Scope(
	const DESTRUCTION_SIMULATION_SCOPE scope,
	const std::string& selectedElementId,
	std::string& outStatus)
{
	if (!m_isStaged || DESTRUCTION_SIMULATION_SCOPE::END == scope)
	{
		outStatus = "Destruction simulation scope is invalid";
		return false;
	}
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == scope)
	{
		const bool_t exists = std::any_of(m_Elements.begin(), m_Elements.end(),
			[&selectedElementId](const ELEMENT_RUNTIME& runtime)
			{
				return runtime.Desc.elementId == selectedElementId;
			});
		if (!exists)
		{
			outStatus = "Solo debris element is not in the staged profile";
			return false;
		}
	}

	const DESTRUCTION_SIMULATION_SCOPE previousScope = m_eScope;
	const std::string previousSelection = m_SelectedElementId;
	m_eScope = scope;
	m_SelectedElementId =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == scope ?
			selectedElementId : std::string{};
	if (!Reset(outStatus))
	{
		m_eScope = previousScope;
		m_SelectedElementId = previousSelection;
		return false;
	}
	m_Status = DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == scope ?
		"Preview scope: All Debris" :
		"Preview scope: Solo " + selectedElementId;
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Activate_Element(
	ELEMENT_RUNTIME& runtime,
	std::string& outStatus)
{
	if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != runtime.eState ||
		nullptr == runtime.pObject || nullptr == m_pPhysics)
	{
		outStatus = "Destruction debris activation state is invalid";
		return false;
	}
	shared_ptr<Engine::CRigidBody> stagedBody;
	if (!m_pPhysics->Create_DynamicBox(
		m_iLevelId,
		runtime.vInitialPosition,
		runtime.vInitialRotation,
		runtime.vShapeLocalCentre,
		runtime.vHalfExtents,
		runtime.vLinearVelocity,
		runtime.Desc.fGravityScale,
		stagedBody,
		outStatus))
	{
		return false;
	}
	if (!runtime.pObject->Begin_PhysicsPreview(
		DEPLOY_PROP_STATE::FRACTURED) ||
		!runtime.pObject->Apply_PhysicsPreviewPose(
			runtime.vInitialPosition, runtime.vInitialRotation))
	{
		stagedBody->Destroy_Actor();
		if (runtime.pObject->Is_PhysicsPreviewActive())
			runtime.pObject->End_PhysicsPreview();
		outStatus = "DeployProp rejected its destruction preview pose";
		return false;
	}
	runtime.pBody = std::move(stagedBody);
	runtime.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE;
	runtime.fActivatedAtSeconds =
		DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
			runtime.Desc.Trigger.eKind ?
			runtime.Desc.Trigger.fTimeSeconds : m_fSampleTimeSeconds;
	return true;
}

void Client::CDestructionSimulationRuntime::Expire_Element(
	ELEMENT_RUNTIME& runtime)
{
	if (nullptr != runtime.pBody)
		runtime.pBody->Destroy_Actor();
	runtime.pBody.reset();
	if (nullptr != runtime.pObject &&
		runtime.pObject->Is_PhysicsPreviewActive())
	{
		runtime.pObject->Set_State(DEPLOY_PROP_STATE::DESPAWNED);
	}
	runtime.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED;
}

bool_t Client::CDestructionSimulationRuntime::Advance_Timeline(
	const f32_t fixedDeltaSeconds,
	std::string& outStatus)
{
	if (!m_isStaged || !std::isfinite(fixedDeltaSeconds) ||
		fixedDeltaSeconds <= 0.f || fixedDeltaSeconds > 0.1f)
	{
		outStatus = "Destruction simulation fixed step is invalid";
		return false;
	}
	if (Is_Finished())
	{
		outStatus = "Destruction simulation is already finished";
		return false;
	}

	const f32_t previousTime = m_fSampleTimeSeconds;
	m_fSampleTimeSeconds = (std::min)(m_Profile.fDurationSeconds,
		m_fSampleTimeSeconds + fixedDeltaSeconds);
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == runtime.eState)
		{
			const DESTRUCTION_SIMULATION_TRIGGER& trigger = runtime.Desc.Trigger;
			const bool_t shouldActivate =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE == trigger.eKind ||
				(DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
					trigger.eKind &&
					previousTime <= trigger.fTimeSeconds &&
					m_fSampleTimeSeconds >= trigger.fTimeSeconds);
			if (shouldActivate && !Activate_Element(runtime, outStatus))
				return false;
		}
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState &&
			(nullptr == runtime.pObject ||
				!runtime.pObject->Advance_PhysicsPreviewAnimation(
					fixedDeltaSeconds)))
		{
			outStatus = "DeployProp rejected its fixed destruction animation tick";
			return false;
		}
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState &&
			m_fSampleTimeSeconds - runtime.fActivatedAtSeconds >=
				runtime.Desc.fLifetimeSeconds)
		{
			Expire_Element(runtime);
		}
	}
	if (m_fSampleTimeSeconds >= m_Profile.fDurationSeconds)
	{
		for (ELEMENT_RUNTIME& runtime : m_Elements)
		{
			if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState ||
				DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == runtime.eState)
			{
				Expire_Element(runtime);
			}
		}
	}
	Rebuild_Frame();
	m_Status = "Destruction simulation sampled at " +
		std::to_string(m_fSampleTimeSeconds) + " s";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Post_Physics_Update(
	std::string& outStatus)
{
	if (!m_isStaged)
		return true;
	bool_t succeeded = true;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE != runtime.eState ||
			nullptr == runtime.pBody)
		{
			continue;
		}
		Engine::PHYSICS_POSE pose{};
		float3_t velocity{};
		if (FAILED(runtime.pBody->Get_Pose(pose)) ||
			FAILED(runtime.pBody->Get_LinearVelocity(velocity)) ||
			!Is_FinitePose(pose.vPosition, pose.vRotationQuaternion) ||
			nullptr == runtime.pObject ||
			!runtime.pObject->Apply_PhysicsPreviewPose(
				pose.vPosition, pose.vRotationQuaternion))
		{
			succeeded = false;
			continue;
		}
		runtime.vCurrentPosition = pose.vPosition;
		runtime.vCurrentRotation = pose.vRotationQuaternion;
		runtime.vLinearVelocity = velocity;
	}

	Rebuild_Frame();
	if (!succeeded)
	{
		m_Status = "One or more destruction preview poses could not be pulled";
		outStatus = m_Status;
		return false;
	}
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Notify_Collision(
	const std::string_view receiverCollisionId,
	std::string& outStatus)
{
	if (!m_isStaged || receiverCollisionId.empty())
	{
		outStatus = "Collision preview needs a staged receiver ID";
		return false;
	}
	size_t activated = 0u;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != runtime.eState ||
			DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT !=
				runtime.Desc.Trigger.eKind ||
			runtime.Desc.Trigger.receiverCollisionId != receiverCollisionId)
		{
			continue;
		}
		if (!Activate_Element(runtime, outStatus))
			return false;
		++activated;
	}
	if (0u == activated)
	{
		outStatus = "No waiting debris element matches collision receiver: " +
			std::string(receiverCollisionId);
		return false;
	}
	Rebuild_Frame();
	m_Status = "Collision preview activated " + std::to_string(activated) +
		" debris elements";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Set_PhysicsPaused(
	const bool_t isPaused,
	std::string& outStatus)
{
	if (!m_isStaged)
	{
		outStatus = "Physics pause requires a staged simulation";
		return false;
	}
	/* The Engine manager deliberately remains debug-paused for the entire
	   staged lifetime. This flag is playback state only; Step_Once drives the
	   scene through Simulate_DebugSteps so no second clock can advance it. */
	m_isPhysicsPaused = isPaused;
	outStatus = isPaused ? "Destruction physics paused" :
		"Destruction physics playing";
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Simulate_PhysicsSteps(
	const uint32_t stepCount,
	std::string& outStatus)
{
	if (!m_isStaged || nullptr == m_pPhysics || 0u == stepCount ||
		!m_pPhysics->Simulate_Steps(stepCount, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Destruction simulation cannot advance PhysX";
		return false;
	}
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Is_Finished() const
{
	return m_isStaged &&
		m_fSampleTimeSeconds >= m_Profile.fDurationSeconds;
}

void Client::CDestructionSimulationRuntime::Rebuild_Frame()
{
	m_Frame = {};
	m_Frame.profileId = m_Profile.profileId;
	m_Frame.groupId = m_Profile.groupId;
	m_Frame.fSampleTimeSeconds = m_fSampleTimeSeconds;
	m_Frame.fDurationSeconds = m_Profile.fDurationSeconds;
	m_Frame.eScope = m_eScope;
	m_Frame.selectedElementId = m_SelectedElementId;
	m_Frame.Elements.reserve(m_Elements.size());
	for (const ELEMENT_RUNTIME& runtime : m_Elements)
	{
		DESTRUCTION_SIMULATION_ELEMENT_FRAME frame;
		frame.elementId = runtime.Desc.elementId;
		frame.sourceRuntimePlacementId =
			runtime.Desc.sourceRuntimePlacementId;
		frame.eState = runtime.eState;
		frame.vWorldPosition = runtime.vCurrentPosition;
		frame.vWorldRotationQuaternion = runtime.vCurrentRotation;
		frame.vLinearVelocity = runtime.vLinearVelocity;
		frame.fNormalizedLife =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState ?
				(std::clamp)(
					(m_fSampleTimeSeconds - runtime.fActivatedAtSeconds) /
						runtime.Desc.fLifetimeSeconds,
					0.f, 1.f) :
			DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED == runtime.eState ?
				1.f : 0.f;
		m_Frame.Elements.push_back(std::move(frame));
	}
}
