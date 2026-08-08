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
	constexpr f32_t PI = 3.14159265358979323846f;
	constexpr f32_t DEBRIS_DIRECTION_SPREAD_RADIANS = 28.f * PI / 180.f;
	constexpr f32_t DEBRIS_UPWARD_SPEED_METERS_PER_SECOND = 3.25f;
	constexpr f32_t DEBRIS_MIN_SPEED_SCALE = 0.8f;
	constexpr f32_t DEBRIS_MAX_SPEED_SCALE = 1.2f;
	constexpr f32_t DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND = 5.f;
	constexpr f32_t DEBRIS_MAX_ANGULAR_SPEED_RADIANS_PER_SECOND = 11.f;
	constexpr f32_t DEBRIS_MIN_VISUAL_SCALE = 0.8f;
	constexpr f32_t DEBRIS_MAX_VISUAL_SCALE = 1.2f;
	std::string Build_FragmentId(
		const std::string& elementId,
		const uint32_t pieceIndex)
	{
		return elementId + ".fragment." +
			(pieceIndex < 10u ? "0" : "") + std::to_string(pieceIndex);
	}

	uint64_t Hash_Bytes(uint64_t seed, const std::string_view value)
	{
		constexpr uint64_t FNV_PRIME = 1099511628211ull;
		for (const unsigned char byte : value)
		{
			seed ^= static_cast<uint64_t>(byte);
			seed *= FNV_PRIME;
		}
		return seed;
	}

	uint64_t Build_DebrisSeed(
		const std::string& profileId,
		const std::string& elementId,
		const uint32_t pieceIndex)
	{
		uint64_t seed = 14695981039346656037ull;
		seed = Hash_Bytes(seed, profileId);
		seed = Hash_Bytes(seed, elementId);
		for (uint32_t shift = 0u; shift < 32u; shift += 8u)
		{
			seed ^= static_cast<uint64_t>((pieceIndex >> shift) & 0xffu);
			seed *= 1099511628211ull;
		}
		return 0u == seed ? 0x9e3779b97f4a7c15ull : seed;
	}

	uint64_t Next_Random(uint64_t& state)
	{
		state ^= state >> 12u;
		state ^= state << 25u;
		state ^= state >> 27u;
		return state * 2685821657736338717ull;
	}

	f32_t Random_Unit(uint64_t& state)
	{
		return static_cast<f32_t>((Next_Random(state) >> 40u) & 0xffffffu) /
			static_cast<f32_t>(0xffffffu);
	}

	f32_t Random_Signed(uint64_t& state)
	{
		return Random_Unit(state) * 2.f - 1.f;
	}

	float3_t Rotate_Vector(
		const float3_t& value,
		const float4_t& rotation)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Rotate(
			XMLoadFloat3(&value),
			XMQuaternionNormalize(XMLoadFloat4(&rotation))));
		return result;
	}

	float3_t InverseRotate_Vector(
		const float3_t& value,
		const float4_t& rotation)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3InverseRotate(
			XMLoadFloat3(&value),
			XMQuaternionNormalize(XMLoadFloat4(&rotation))));
		return result;
	}

	float3_t Normalize_Vector(const float3_t& value)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&value)));
		return result;
	}

	float3_t Build_SpreadDirection(
		const float3_t& authoredDirection,
		uint64_t& randomState)
	{
		const vector_t forward = XMVector3Normalize(
			XMLoadFloat3(&authoredDirection));
		const f32_t upDot = std::abs(XMVectorGetX(XMVector3Dot(
			forward, XMVectorSet(0.f, 1.f, 0.f, 0.f))));
		const vector_t reference = upDot > 0.95f ?
			XMVectorSet(1.f, 0.f, 0.f, 0.f) :
			XMVectorSet(0.f, 1.f, 0.f, 0.f);
		const vector_t right = XMVector3Normalize(
			XMVector3Cross(reference, forward));
		const vector_t secondary = XMVector3Normalize(
			XMVector3Cross(forward, right));
		const f32_t angle = DEBRIS_DIRECTION_SPREAD_RADIANS *
			std::sqrt(Random_Unit(randomState));
		const f32_t azimuth = 2.f * PI * Random_Unit(randomState);
		const vector_t radial = right * std::cos(azimuth) +
			secondary * std::sin(azimuth);
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(
			forward * std::cos(angle) + radial * std::sin(angle)));
		return result;
	}

	float4_t Build_RandomRotation(uint64_t& randomState)
	{
		float4_t result{};
		XMStoreFloat4(&result, XMQuaternionNormalize(
			XMQuaternionRotationRollPitchYaw(
				Random_Signed(randomState) * PI,
				Random_Signed(randomState) * PI,
				Random_Signed(randomState) * PI)));
		return result;
	}

	float3_t Build_RandomAngularVelocity(uint64_t& randomState)
	{
		float3_t axis = {
			Random_Signed(randomState),
			Random_Signed(randomState),
			Random_Signed(randomState)
		};
		const f32_t lengthSquared = axis.x * axis.x + axis.y * axis.y +
			axis.z * axis.z;
		if (lengthSquared <= 0.000001f)
			axis = { 0.f, 1.f, 0.f };
		else
			axis = Normalize_Vector(axis);
		const f32_t speed = DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND +
			(DEBRIS_MAX_ANGULAR_SPEED_RADIANS_PER_SECOND -
				DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND) *
			Random_Unit(randomState);
		return { axis.x * speed, axis.y * speed, axis.z * speed };
	}

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

const std::vector<Client::DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC>&
Client::CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs()
{
	static const std::vector<DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC> specs = {
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone001",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone002",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone004",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone010",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f
		}
	};
	return specs;
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
		const float3_t& angularVelocity,
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
			FAILED(staged->Set_LinearVelocity(linearVelocity)) ||
			FAILED(staged->Set_AngularVelocity(angularVelocity)))
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
	struct DEBRIS_PIECE_RUNTIME final
	{
		std::string fragmentId;
		std::string modelAssetId;
		uint32_t iVisualIndex = UINT32_MAX;
		shared_ptr<Engine::CRigidBody> pBody;
		float3_t vInitialPosition{};
		float4_t vInitialRotation = { 0.f, 0.f, 0.f, 1.f };
		float3_t vShapeLocalCentre{};
		float3_t vHalfExtents = { 0.05f, 0.05f, 0.05f };
		float3_t vInitialLinearVelocity{};
		float3_t vInitialAngularVelocity{};
		float3_t vCurrentPosition{};
		float4_t vCurrentRotation = { 0.f, 0.f, 0.f, 1.f };
		float3_t vCurrentLinearVelocity{};
		f32_t fActivatedAtSeconds = 0.f;
		DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
	};

	DESTRUCTION_SIMULATION_ELEMENT Desc;
	shared_ptr<CDeployPropObject> pObject;
	float3_t vInitialPosition{};
	float4_t vInitialRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vShapeLocalCentre{};
	float3_t vHalfExtents = { 0.5f, 0.5f, 0.5f };
	float3_t vCurrentPosition{};
	float4_t vCurrentRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vLinearVelocity{};
	std::vector<DEBRIS_PIECE_RUNTIME> DebrisPieces;
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

	const auto& modelSpecs = Get_ProjectAuthoredDebrisModelSpecs();
	if (modelSpecs.empty())
	{
		stagedPhysics->Clear_Ground();
		stagedPhysics->End_ExclusiveClock();
		outStatus = "Project-authored debris model recipe is empty";
		return false;
	}

	auto releaseStagedDebris = [&stagedElements]()
	{
		for (ELEMENT_RUNTIME& runtime : stagedElements)
		{
			if (nullptr != runtime.pObject &&
				runtime.pObject->Is_DebrisPreviewActive())
			{
				runtime.pObject->End_DebrisPreview();
			}
		}
	};
	for (ELEMENT_RUNTIME& runtime : stagedElements)
	{
		CDeployPropObject::DEBRIS_PREVIEW_DESC previewDesc;
		previewDesc.suppressSource = false;
		previewDesc.instances.reserve(
			PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT);
		std::vector<uint64_t> randomStates;
		randomStates.reserve(PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT);
		for (uint32_t pieceIndex = 0u;
			pieceIndex < PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT;
			++pieceIndex)
		{
			uint64_t randomState = Build_DebrisSeed(
				profile.profileId, runtime.Desc.elementId, pieceIndex);
			const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& modelSpec =
				modelSpecs[pieceIndex % modelSpecs.size()];
			CDeployPropObject::DEBRIS_PREVIEW_INSTANCE_DESC instanceDesc;
			instanceDesc.modelPrototypeTag = modelSpec.prototypeTag;
			instanceDesc.uniformScale = DEBRIS_MIN_VISUAL_SCALE +
					(DEBRIS_MAX_VISUAL_SCALE - DEBRIS_MIN_VISUAL_SCALE) *
					Random_Unit(randomState);
			previewDesc.instances.push_back(std::move(instanceDesc));
			randomStates.push_back(randomState);
		}

		std::string debrisError;
		if (!runtime.pObject->Begin_DebrisPreview(previewDesc, debrisError) ||
			runtime.pObject->Get_DebrisPreviewInstanceCount() !=
				PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT)
		{
			releaseStagedDebris();
			stagedPhysics->Clear_Ground();
			stagedPhysics->End_ExclusiveClock();
			outStatus = "Project-authored debris prototypes are unavailable";
			if (!debrisError.empty())
				outStatus += ": " + debrisError;
			return false;
		}

		runtime.DebrisPieces.reserve(
			PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT);
		const float3_t localImpactDirection = InverseRotate_Vector(
			runtime.Desc.vDirection, runtime.vInitialRotation);
		for (uint32_t pieceIndex = 0u;
			pieceIndex < PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT;
			++pieceIndex)
		{
			ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME piece;
			const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& modelSpec =
				modelSpecs[pieceIndex % modelSpecs.size()];
			piece.fragmentId = Build_FragmentId(
				runtime.Desc.elementId, pieceIndex);
			piece.modelAssetId = modelSpec.assetId;
			piece.iVisualIndex = pieceIndex;
			if (!runtime.pObject->Get_DebrisPreviewLocalBounds(
				pieceIndex, piece.vShapeLocalCentre, piece.vHalfExtents))
			{
				releaseStagedDebris();
				stagedPhysics->Clear_Ground();
				stagedPhysics->End_ExclusiveClock();
				outStatus = "Project-authored debris model has no usable bounds";
				return false;
			}

			uint64_t randomState = randomStates[pieceIndex];
			float3_t localSpawn = {
				runtime.vShapeLocalCentre.x + runtime.vHalfExtents.x *
					Random_Signed(randomState) * 0.75f,
				runtime.vShapeLocalCentre.y + runtime.vHalfExtents.y *
					Random_Signed(randomState) * 0.75f,
				runtime.vShapeLocalCentre.z + runtime.vHalfExtents.z *
					Random_Signed(randomState) * 0.75f
			};
			/* Bias roots toward the authored impact-facing surface. This keeps
			   the stones distributed over the visible wall volume without
			   making every actor emerge from the placement origin. */
			const f32_t surfaceFactor = 0.65f +
				Random_Unit(randomState) * 0.3f;
			const f32_t absX = std::abs(localImpactDirection.x);
			const f32_t absY = std::abs(localImpactDirection.y);
			const f32_t absZ = std::abs(localImpactDirection.z);
			if (absX >= absY && absX >= absZ)
			{
				localSpawn.x = runtime.vShapeLocalCentre.x +
					(localImpactDirection.x < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.x * surfaceFactor;
			}
			else if (absY >= absZ)
			{
				localSpawn.y = runtime.vShapeLocalCentre.y +
					(localImpactDirection.y < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.y * surfaceFactor;
			}
			else
			{
				localSpawn.z = runtime.vShapeLocalCentre.z +
					(localImpactDirection.z < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.z * surfaceFactor;
			}
			const float3_t worldSpawnOffset = Rotate_Vector(
				localSpawn, runtime.vInitialRotation);
			piece.vInitialPosition = {
				runtime.vInitialPosition.x + worldSpawnOffset.x,
				runtime.vInitialPosition.y + worldSpawnOffset.y,
				runtime.vInitialPosition.z + worldSpawnOffset.z
			};
			piece.vInitialRotation = Build_RandomRotation(randomState);
			const float3_t spreadDirection = Build_SpreadDirection(
				runtime.Desc.vDirection, randomState);
			const f32_t speedScale = DEBRIS_MIN_SPEED_SCALE +
				(DEBRIS_MAX_SPEED_SCALE - DEBRIS_MIN_SPEED_SCALE) *
				Random_Unit(randomState);
			const f32_t upwardSpeed =
				DEBRIS_UPWARD_SPEED_METERS_PER_SECOND *
				(0.8f + Random_Unit(randomState) * 0.4f);
			piece.vInitialLinearVelocity = {
				spreadDirection.x * runtime.Desc.fSpeedMetersPerSecond *
					speedScale,
				spreadDirection.y * runtime.Desc.fSpeedMetersPerSecond *
					speedScale + upwardSpeed,
				spreadDirection.z * runtime.Desc.fSpeedMetersPerSecond *
					speedScale
			};
			piece.vInitialAngularVelocity =
				Build_RandomAngularVelocity(randomState);
			piece.vCurrentPosition = piece.vInitialPosition;
			piece.vCurrentRotation = piece.vInitialRotation;
			piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
			runtime.DebrisPieces.push_back(std::move(piece));
		}
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
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			if (nullptr != piece.pBody)
				piece.pBody->Destroy_Actor();
			piece.pBody.reset();
			if (nullptr != runtime.pObject &&
				runtime.pObject->Is_DebrisPreviewActive())
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					piece.iVisualIndex,
					piece.vCurrentPosition,
					piece.vCurrentRotation,
					false);
			}
		}
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_PhysicsPreviewActive())
		{
			runtime.pObject->End_PhysicsPreview();
		}
	}
}

void Client::CDestructionSimulationRuntime::Release_DebrisPreviews()
{
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_DebrisPreviewActive())
		{
			runtime.pObject->End_DebrisPreview();
		}
	}
}

void Client::CDestructionSimulationRuntime::Clear()
{
	Destroy_Actors();
	Release_DebrisPreviews();
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
	m_SelectedFragmentId.clear();
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
		if (nullptr == runtime.pObject ||
			!runtime.pObject->Is_DebrisPreviewActive() ||
			runtime.pObject->Get_DebrisPreviewInstanceCount() !=
				runtime.DebrisPieces.size())
		{
			outStatus = "Destruction debris preview resources changed during reset";
			return false;
		}
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			piece.vCurrentPosition = piece.vInitialPosition;
			piece.vCurrentRotation = piece.vInitialRotation;
			piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
			piece.fActivatedAtSeconds = 0.f;
			piece.eState = Is_FragmentInScope(piece.fragmentId) ?
				DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING :
				DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED;
			if (!runtime.pObject->Apply_DebrisPreviewPose(
				piece.iVisualIndex,
				piece.vInitialPosition,
				piece.vInitialRotation,
				false))
			{
				outStatus = "DeployProp rejected a hidden debris reset pose";
				return false;
			}
		}
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
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == m_eScope)
		return true;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == m_eScope)
		return elementId == m_SelectedElementId;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT != m_eScope)
		return false;
	const auto element = std::find_if(m_Elements.begin(), m_Elements.end(),
		[&elementId](const ELEMENT_RUNTIME& runtime)
		{
			return runtime.Desc.elementId == elementId;
		});
	return m_Elements.end() != element && std::any_of(
		element->DebrisPieces.begin(), element->DebrisPieces.end(),
		[this](const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
		{
			return piece.fragmentId == m_SelectedFragmentId;
		});
}

bool_t Client::CDestructionSimulationRuntime::Is_FragmentInScope(
	const std::string& fragmentId) const
{
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == m_eScope)
		return true;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == m_eScope)
		return fragmentId == m_SelectedFragmentId;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED != m_eScope)
		return false;
	const auto element = std::find_if(m_Elements.begin(), m_Elements.end(),
		[this](const ELEMENT_RUNTIME& runtime)
		{
			return runtime.Desc.elementId == m_SelectedElementId;
		});
	return m_Elements.end() != element && std::any_of(
		element->DebrisPieces.begin(), element->DebrisPieces.end(),
		[&fragmentId](const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
		{
			return piece.fragmentId == fragmentId;
		});
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
	std::string selectedOwnerElementId;
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
		selectedOwnerElementId = selectedElementId;
	}
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
	{
		for (const ELEMENT_RUNTIME& runtime : m_Elements)
		{
			const bool_t found = std::any_of(
				runtime.DebrisPieces.begin(), runtime.DebrisPieces.end(),
				[&selectedElementId](
					const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
				{
					return piece.fragmentId == selectedElementId;
				});
			if (found)
			{
				selectedOwnerElementId = runtime.Desc.elementId;
				break;
			}
		}
		if (selectedOwnerElementId.empty())
		{
			outStatus = "Solo debris fragment is not in the staged profile";
			return false;
		}
	}

	const DESTRUCTION_SIMULATION_SCOPE previousScope = m_eScope;
	const std::string previousSelection = m_SelectedElementId;
	const std::string previousFragmentSelection = m_SelectedFragmentId;
	m_eScope = scope;
	m_SelectedElementId = selectedOwnerElementId;
	m_SelectedFragmentId =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope ?
			selectedElementId : std::string{};
	if (!Reset(outStatus))
	{
		m_eScope = previousScope;
		m_SelectedElementId = previousSelection;
		m_SelectedFragmentId = previousFragmentSelection;
		return false;
	}
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == scope)
		m_Status = "Preview scope: All Debris";
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
		m_Status = "Preview scope: Solo Fragment " + selectedElementId;
	else
		m_Status = "Preview scope: Solo Wall " + selectedElementId;
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Activate_Element(
	ELEMENT_RUNTIME& runtime,
	std::string& outStatus)
{
	if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != runtime.eState ||
		nullptr == runtime.pObject || nullptr == m_pPhysics ||
		!runtime.pObject->Is_DebrisPreviewActive() ||
		runtime.pObject->Get_DebrisPreviewInstanceCount() !=
			runtime.DebrisPieces.size() ||
		runtime.DebrisPieces.empty())
	{
		outStatus = "Destruction debris activation state is invalid";
		return false;
	}

	std::vector<std::pair<size_t, shared_ptr<Engine::CRigidBody>>>
		stagedBodies;
	stagedBodies.reserve(runtime.DebrisPieces.size());
	for (size_t pieceIndex = 0u;
		pieceIndex < runtime.DebrisPieces.size(); ++pieceIndex)
	{
		const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[pieceIndex];
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != piece.eState)
			continue;
		shared_ptr<Engine::CRigidBody> stagedBody;
		if (!m_pPhysics->Create_DynamicBox(
			m_iLevelId,
			piece.vInitialPosition,
			piece.vInitialRotation,
			piece.vShapeLocalCentre,
			piece.vHalfExtents,
			piece.vInitialLinearVelocity,
			piece.vInitialAngularVelocity,
			runtime.Desc.fGravityScale,
			stagedBody,
			outStatus))
		{
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			return false;
		}
		stagedBodies.emplace_back(pieceIndex, std::move(stagedBody));
	}
	if (stagedBodies.empty())
	{
		outStatus = "Destruction debris activation has no fragment in scope";
		return false;
	}

	if (!runtime.pObject->Begin_PhysicsPreview(
		DEPLOY_PROP_STATE::FRACTURED))
	{
		for (auto& staged : stagedBodies)
			staged.second->Destroy_Actor();
		if (runtime.pObject->Is_PhysicsPreviewActive())
			runtime.pObject->End_PhysicsPreview();
		outStatus = "DeployProp rejected its fractured source preview state";
		return false;
	}

	for (size_t index = 0u; index < runtime.DebrisPieces.size(); ++index)
	{
		ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[index];
		const bool_t isVisible =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == piece.eState;
		if (!runtime.pObject->Apply_DebrisPreviewPose(
			piece.iVisualIndex,
			piece.vInitialPosition,
			piece.vInitialRotation,
			isVisible))
		{
			for (const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& hiddenPiece :
				runtime.DebrisPieces)
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					hiddenPiece.iVisualIndex,
					hiddenPiece.vInitialPosition,
					hiddenPiece.vInitialRotation,
					false);
			}
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			runtime.pObject->End_PhysicsPreview();
			outStatus = "DeployProp rejected an initial debris proxy pose";
			return false;
		}
		piece.vCurrentPosition = piece.vInitialPosition;
		piece.vCurrentRotation = piece.vInitialRotation;
		piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
	}
	runtime.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE;
	runtime.fActivatedAtSeconds =
		DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
			runtime.Desc.Trigger.eKind ?
			runtime.Desc.Trigger.fTimeSeconds : m_fSampleTimeSeconds;
	for (auto& staged : stagedBodies)
	{
		ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[staged.first];
		piece.pBody = std::move(staged.second);
		piece.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE;
		piece.fActivatedAtSeconds = runtime.fActivatedAtSeconds;
	}
	return true;
}

void Client::CDestructionSimulationRuntime::Expire_Element(
	ELEMENT_RUNTIME& runtime)
{
	for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
		runtime.DebrisPieces)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED == piece.eState)
			continue;
		if (nullptr != piece.pBody)
			piece.pBody->Destroy_Actor();
		piece.pBody.reset();
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_DebrisPreviewActive())
		{
				runtime.pObject->Apply_DebrisPreviewPose(
				piece.iVisualIndex,
				piece.vCurrentPosition,
				piece.vCurrentRotation,
				false);
		}
		piece.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED;
	}
	/* The proxy stones are transient, but the source wall remains in its
	   fractured preview state until Reset/Clear restores the exact authored
	   state and animation cursor. */
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
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE != runtime.eState)
		{
			continue;
		}
		if (nullptr == runtime.pObject ||
			!runtime.pObject->Is_DebrisPreviewActive())
		{
			succeeded = false;
			continue;
		}

		float3_t aggregatePosition{};
		float3_t aggregateVelocity{};
		size_t sampledPieces = 0u;
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE != piece.eState)
				continue;
			Engine::PHYSICS_POSE pose{};
			float3_t velocity{};
			if (nullptr == piece.pBody ||
				FAILED(piece.pBody->Get_Pose(pose)) ||
				FAILED(piece.pBody->Get_LinearVelocity(velocity)) ||
				!Is_FinitePose(pose.vPosition, pose.vRotationQuaternion) ||
				!std::isfinite(velocity.x) || !std::isfinite(velocity.y) ||
				!std::isfinite(velocity.z) ||
				!runtime.pObject->Apply_DebrisPreviewPose(
					piece.iVisualIndex,
					pose.vPosition,
					pose.vRotationQuaternion,
					true))
			{
				succeeded = false;
				continue;
			}
			piece.vCurrentPosition = pose.vPosition;
			piece.vCurrentRotation = pose.vRotationQuaternion;
			piece.vCurrentLinearVelocity = velocity;
			aggregatePosition.x += pose.vPosition.x;
			aggregatePosition.y += pose.vPosition.y;
			aggregatePosition.z += pose.vPosition.z;
			aggregateVelocity.x += velocity.x;
			aggregateVelocity.y += velocity.y;
			aggregateVelocity.z += velocity.z;
			if (0u == sampledPieces)
				runtime.vCurrentRotation = pose.vRotationQuaternion;
			++sampledPieces;
		}
		if (0u == sampledPieces)
		{
			succeeded = false;
			continue;
		}
		const f32_t inverseCount = 1.f /
			static_cast<f32_t>(sampledPieces);
		runtime.vCurrentPosition = {
			aggregatePosition.x * inverseCount,
			aggregatePosition.y * inverseCount,
			aggregatePosition.z * inverseCount
		};
		runtime.vLinearVelocity = {
			aggregateVelocity.x * inverseCount,
			aggregateVelocity.y * inverseCount,
			aggregateVelocity.z * inverseCount
		};
	}

	Rebuild_Frame();
	if (!succeeded)
	{
		m_Status = "One or more debris proxy poses could not be pulled";
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
	m_Frame.selectedFragmentId = m_SelectedFragmentId;
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
		frame.Fragments.reserve(runtime.DebrisPieces.size());
		for (uint32_t pieceIndex = 0u;
			pieceIndex < runtime.DebrisPieces.size(); ++pieceIndex)
		{
			const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
				runtime.DebrisPieces[pieceIndex];
			DESTRUCTION_SIMULATION_FRAGMENT_FRAME fragmentFrame;
			fragmentFrame.fragmentId = piece.fragmentId;
			fragmentFrame.modelAssetId = piece.modelAssetId;
			fragmentFrame.pieceIndex = pieceIndex;
			fragmentFrame.eState = piece.eState;
			fragmentFrame.vWorldPosition = piece.vCurrentPosition;
			fragmentFrame.vWorldRotationQuaternion = piece.vCurrentRotation;
			fragmentFrame.vLinearVelocity = piece.vCurrentLinearVelocity;
			fragmentFrame.fNormalizedLife =
				DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == piece.eState ?
					(std::clamp)(
						(m_fSampleTimeSeconds - piece.fActivatedAtSeconds) /
							runtime.Desc.fLifetimeSeconds,
						0.f, 1.f) :
				DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED == piece.eState ?
					1.f : 0.f;
			frame.Fragments.push_back(std::move(fragmentFrame));
		}
		m_Frame.Elements.push_back(std::move(frame));
	}
}
