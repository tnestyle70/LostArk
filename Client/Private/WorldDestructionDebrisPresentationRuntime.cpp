#include "WorldDestructionDebrisPresentationRuntime.h"

#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "DestructionSimulationRuntime.h"
#include "GameInstance.h"
#include "Model.h"
#include "RigidBody.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace
{
	constexpr f32_t PI = 3.14159265358979323846f;
	constexpr f32_t MINIMUM_SHAPE_HALF_EXTENT = 0.01f;
	constexpr f32_t UPWARD_SPEED_METERS_PER_SECOND = 3.25f;
	constexpr f32_t MINIMUM_SPEED_SCALE = 0.8f;
	constexpr f32_t MAXIMUM_SPEED_SCALE = 1.2f;
	constexpr f32_t MINIMUM_ANGULAR_SPEED = 5.f;
	constexpr f32_t MAXIMUM_ANGULAR_SPEED = 11.f;
	constexpr uint32_t PRODUCT_DEBRIS_COLLISION_GROUP = 1u << 2u;

	bool_t Is_Finite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t Is_UnitDirection(const float3_t& value)
	{
		if (!Is_Finite(value))
			return false;
		const f32_t lengthSquared = value.x * value.x +
			value.y * value.y + value.z * value.z;
		return std::isfinite(lengthSquared) &&
			lengthSquared > 0.000001f;
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

	uint64_t Build_Seed(
		const Client::WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		const uint64_t sourcePlacementId,
		const uint32_t pieceIndex)
	{
		uint64_t seed = 14695981039346656037ull;
		seed = Hash_Bytes(seed, cue.groupId);
		const uint64_t values[] = {
			cue.eventSequence, cue.randomSeed, sourcePlacementId, pieceIndex
		};
		for (const uint64_t value : values)
		{
			for (uint32_t shift = 0u; shift < 64u; shift += 8u)
			{
				seed ^= (value >> shift) & 0xffu;
				seed *= 1099511628211ull;
			}
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

	float3_t Normalize_Vector(const float3_t& value)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&value)));
		return result;
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

	float3_t Build_SpreadDirection(
		const float3_t& authoredDirection,
		const f32_t spreadDegrees,
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
		/* An authored spread outside the half-sphere would fold the cone back
		   on itself, so it is clamped rather than trusted. */
		const f32_t spreadRadians =
			(std::clamp)(spreadDegrees, 0.f, 180.f) * PI / 180.f;
		const f32_t angle = spreadRadians *
			std::sqrt(Random_Unit(randomState));
		const f32_t azimuth = 2.f * PI * Random_Unit(randomState);
		const vector_t radial = right * std::cos(azimuth) +
			secondary * std::sin(azimuth);
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(
			forward * std::cos(angle) + radial * std::sin(angle)));
		return result;
	}

	float3_t Build_AngularVelocity(uint64_t& randomState)
	{
		float3_t axis = {
			Random_Signed(randomState),
			Random_Signed(randomState),
			Random_Signed(randomState)
		};
		const f32_t lengthSquared = axis.x * axis.x + axis.y * axis.y +
			axis.z * axis.z;
		axis = lengthSquared <= 0.000001f ?
			float3_t(0.f, 1.f, 0.f) : Normalize_Vector(axis);
		const f32_t speed = MINIMUM_ANGULAR_SPEED +
			(MAXIMUM_ANGULAR_SPEED - MINIMUM_ANGULAR_SPEED) *
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

	std::vector<const Client::DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>
	Find_Recipe(const std::string& sourceAssetId)
	{
		std::vector<const Client::DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>
			result;
		const auto& specs = Client::CDestructionSimulationRuntime::
			Get_ProjectAuthoredDebrisModelSpecs();
		for (const auto& spec : specs)
		{
			if (spec.sourceDeployAssetId == sourceAssetId)
				result.push_back(&spec);
		}
		return result;
	}
}

Client::CWorldDestructionDebrisPresentationRuntime::
~CWorldDestructionDebrisPresentationRuntime()
{
	Clear();
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::Initialize(
	const uint32_t levelId,
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11DeviceContext> context,
	CDeployPropRuntime& deployRuntime,
	std::string& outError)
{
	outError.clear();
	if (levelId >= ETOUI(LEVEL::END) || nullptr == device ||
		nullptr == context || !deployRuntime.Is_Loaded())
	{
		outError = "World destruction debris initialization is invalid";
		return false;
	}

	Clear();
	m_LevelId = levelId;
	m_Device = std::move(device);
	m_Context = std::move(context);
	m_pDeployRuntime = &deployRuntime;
	if (!Admit_ModelPrototypes(outError))
	{
		Clear();
		return false;
	}
	m_Status = "World destruction debris presentation is ready";
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::
Admit_ModelPrototypes(std::string& outError)
{
	const auto& specs =
		CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs();
	if (specs.empty())
	{
		outError = "Project-authored destruction debris recipe is empty";
		return false;
	}

	std::vector<std::pair<wstring_t, unique_ptr<CPrototype>>> staged;
	std::unordered_set<wstring_t> uniqueTags;
	for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : specs)
	{
		if (spec.sourceDeployAssetId.empty())
			continue;
		if (spec.prototypeTag.empty() || spec.assetId.empty() ||
			!std::isfinite(spec.fUniformScale) || spec.fUniformScale <= 0.f ||
			!Is_Finite(spec.vSourceLocalPivotMeters) ||
			!uniqueTags.emplace(spec.prototypeTag).second)
		{
			outError = "Project-authored destruction debris recipe is invalid";
			return false;
		}

		const shared_ptr<CModel> existing = dynamic_pointer_cast<CModel>(
			CGameInstance::Get().Clone_Prototype(
				m_LevelId, spec.prototypeTag));
		if (nullptr != existing)
		{
			if (existing->Is_Skinned() || !existing->Has_LocalBounds())
			{
				outError = "Destruction debris prototype tag collision";
				return false;
			}
			continue;
		}

		const std::filesystem::path modelPath =
			CRuntimeAssetRoot::Resolve(spec.assetId).lexically_normal();
		std::error_code modelError;
		if (modelPath.empty() || !std::filesystem::is_regular_file(
			modelPath, modelError) || modelError)
		{
			outError = "Destruction debris model is missing: " + spec.assetId;
			return false;
		}
		const f32_t modelScale = 0.01f * spec.fUniformScale;
		auto model = CModel::Create(
			m_Device,
			m_Context,
			MODEL::NONANIM,
			modelPath.string().c_str(),
			XMMatrixScaling(modelScale, modelScale, modelScale));
		if (nullptr == model || model->Is_Skinned() ||
			!model->Has_LocalBounds())
		{
			outError = "Destruction debris model decode failed: " + spec.assetId;
			return false;
		}
		unique_ptr<CPrototype> prototype = std::move(model);
		staged.emplace_back(spec.prototypeTag, std::move(prototype));
	}

	if (uniqueTags.empty())
	{
		outError = "No exact Valtan destruction debris recipe is available";
		return false;
	}
	if (!staged.empty() && FAILED(CGameInstance::Get().Add_Prototypes(
		m_LevelId, std::move(staged))))
	{
		outError = "Destruction debris prototype batch commit failed";
		return false;
	}
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::Validate_Cue(
	const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
	std::string& outError) const
{
	if (!Is_Initialized() || cue.groupId.empty() ||
		0u == cue.eventSequence || cue.emitters.empty())
	{
		outError = "World destruction debris cue is invalid";
		return false;
	}
	std::unordered_set<uint64_t> placementIds;
	for (const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitter : cue.emitters)
	{
		if (0u == emitter.sourceRuntimePlacementId ||
			!placementIds.emplace(emitter.sourceRuntimePlacementId).second ||
			!Is_Finite(emitter.spawnOffset) ||
			!Is_UnitDirection(emitter.direction) ||
			!std::isfinite(emitter.speedMetersPerSecond) ||
			emitter.speedMetersPerSecond <= 0.f ||
			!std::isfinite(emitter.gravityScale) || emitter.gravityScale < 0.f ||
			!std::isfinite(emitter.lifetimeSeconds) ||
			emitter.lifetimeSeconds <= 0.f)
		{
			outError = "World destruction debris emitter is invalid";
			return false;
		}
		for (const uint64_t aliasId : emitter.suppressionAliasPlacementIds)
		{
			if (0u == aliasId || !placementIds.emplace(aliasId).second)
			{
				outError = "World destruction debris placement is duplicated";
				return false;
			}
		}
	}
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::
Validate_RuntimeTargets(
	const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
	std::string& outError) const
{
	for (const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitter : cue.emitters)
	{
		const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
			*m_pDeployRuntime, emitter.sourceRuntimePlacementId);
		if (nullptr == entry || nullptr == entry->object ||
			!entry->object->Is_Destructible() ||
			!entry->object->Is_StaticDeployModel() ||
			entry->object->Is_DestructionDebrisPresentationActive() ||
			entry->object->Is_TransientDestructionSuppressed() ||
			Find_Recipe(entry->placement.assetId).size() != ACTORS_PER_EMITTER)
		{
			outError = "Destruction debris source/recipe is unavailable: " +
				std::to_string(emitter.sourceRuntimePlacementId);
			return false;
		}
		for (const uint64_t aliasId : emitter.suppressionAliasPlacementIds)
		{
			const DEPLOY_RUNTIME_ENTRY* aliasEntry = Find_Entry(
				*m_pDeployRuntime, aliasId);
			if (nullptr == aliasEntry || nullptr == aliasEntry->object ||
				!aliasEntry->object->Is_Destructible() ||
				!aliasEntry->object->Is_StaticDeployModel() ||
				aliasEntry->object->Is_DestructionDebrisPresentationActive() ||
				aliasEntry->object->Is_TransientDestructionSuppressed())
			{
				outError = "Destruction debris alias is unavailable: " +
					std::to_string(aliasId);
				return false;
			}
		}
	}
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::Play_Cue(
	const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
	const uint32_t maxEmitters,
	std::string& outError)
{
	outError.clear();
	if (!Validate_Cue(cue, outError))
		return false;
	const auto cueKey = std::make_pair(cue.groupId, cue.eventSequence);
	if (m_AcceptedCueKeys.contains(cueKey))
	{
		m_Status = "Duplicate destruction debris cue ignored: " + cue.groupId;
		return true;
	}
	/* Validate every referenced placement before budget selection. Even walls
	   whose cosmetic actors would be omitted must not conceal a corrupt cue. */
	if (!Validate_RuntimeTargets(cue, outError))
		return false;

	std::vector<uint64_t> admittedPlacementIds;
	if (!Build_DeterministicAdmissionPlan(
		cue, Get_ActiveActorCount(), maxEmitters, admittedPlacementIds,
		outError))
	{
		return false;
	}
	std::vector<EMITTER_RUNTIME> stagedEmitters;
	stagedEmitters.reserve(admittedPlacementIds.size());
	for (const uint64_t placementId : admittedPlacementIds)
	{
		const auto found = std::find_if(
			cue.emitters.begin(), cue.emitters.end(),
			[placementId](const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitter)
			{
				return emitter.sourceRuntimePlacementId == placementId;
			});
		if (found == cue.emitters.end())
		{
			outError = "Deterministic debris admission lost an emitter";
			for (EMITTER_RUNTIME& staged : stagedEmitters)
				Release_Emitter(staged);
			return false;
		}
		EMITTER_RUNTIME staged;
		if (!Stage_Emitter(cue, *found, staged, outError))
		{
			for (EMITTER_RUNTIME& previous : stagedEmitters)
				Release_Emitter(previous);
			m_Status = "Cosmetic destruction debris cue failed: " + outError;
			return false;
		}
		stagedEmitters.push_back(std::move(staged));
	}
	for (EMITTER_RUNTIME& staged : stagedEmitters)
		m_Emitters.push_back(std::move(staged));
	m_AcceptedCueKeys.emplace(cueKey);
	m_Status = "World destruction debris cue started: " + cue.groupId +
		" (" + std::to_string(admittedPlacementIds.size()) + "/" +
		std::to_string(cue.emitters.size()) + " complete emitters)";
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::Stage_Emitter(
	const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
	const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitterCue,
	EMITTER_RUNTIME& outEmitter,
	std::string& outError)
{
	const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
		*m_pDeployRuntime, emitterCue.sourceRuntimePlacementId);
	if (nullptr == entry || nullptr == entry->object ||
		!entry->object->Is_Destructible() ||
		!entry->object->Is_StaticDeployModel())
	{
		outError = "Destruction debris source placement is unavailable";
		return false;
	}
	const auto recipe = Find_Recipe(entry->placement.assetId);
	if (recipe.size() != ACTORS_PER_EMITTER)
	{
		outError = "Exact 12-piece debris recipe is unavailable for " +
			entry->placement.assetId;
		return false;
	}

	EMITTER_RUNTIME staged;
	staged.groupId = cue.groupId;
	staged.eventSequence = cue.eventSequence;
	staged.sourceRuntimePlacementId = emitterCue.sourceRuntimePlacementId;
	staged.lifetimeSeconds = emitterCue.lifetimeSeconds;
	staged.source = entry->object;
	for (const uint64_t aliasId : emitterCue.suppressionAliasPlacementIds)
	{
		const DEPLOY_RUNTIME_ENTRY* aliasEntry = Find_Entry(
			*m_pDeployRuntime, aliasId);
		if (nullptr == aliasEntry || nullptr == aliasEntry->object ||
			!aliasEntry->object->Is_Destructible() ||
			!aliasEntry->object->Is_StaticDeployModel())
		{
			outError = "Destruction debris suppression alias is unavailable";
			return false;
		}
		staged.aliases.push_back(aliasEntry->object);
	}

	CDeployPropObject::DESTRUCTION_DEBRIS_PRESENTATION_DESC presentation;
	presentation.suppressSource = true;
	presentation.instances.reserve(recipe.size());
	for (const auto* spec : recipe)
	{
		presentation.instances.push_back({
			spec->prototypeTag,
			entry->placement.uniformScale
		});
	}
	if (!staged.source->Begin_DestructionDebrisPresentation(
		presentation, outError))
	{
		return false;
	}

	auto rollback = [&staged]()
	{
		for (auto& fragment : staged.fragments)
		{
			if (nullptr != fragment.body)
				fragment.body->Destroy_Actor();
		}
		for (const auto& alias : staged.aliases)
		{
			if (nullptr != alias)
				alias->End_TransientDestructionSuppression();
		}
		if (nullptr != staged.source)
			staged.source->End_DestructionDebrisPresentation();
	};

	for (const auto& alias : staged.aliases)
	{
		if (!alias->Begin_TransientDestructionSuppression(outError))
		{
			rollback();
			return false;
		}
	}

	for (uint32_t pieceIndex = 0u; pieceIndex < recipe.size(); ++pieceIndex)
	{
		float3_t shapeLocalCentre{};
		float3_t halfExtents{};
		if (!staged.source->Get_DestructionDebrisPresentationLocalBounds(
			pieceIndex, shapeLocalCentre, halfExtents))
		{
			outError = "Destruction debris model has no usable bounds";
			rollback();
			return false;
		}

		const auto& spec = *recipe[pieceIndex];
		const float3_t localPivot = {
			spec.vSourceLocalPivotMeters.x * entry->placement.uniformScale,
			spec.vSourceLocalPivotMeters.y * entry->placement.uniformScale,
			spec.vSourceLocalPivotMeters.z * entry->placement.uniformScale
		};
		const float3_t rotatedPivot = Rotate_Vector(
			localPivot, entry->placement.rotationQuaternion);
		const float3_t rootPosition = {
			entry->placement.position.x + emitterCue.spawnOffset.x +
				rotatedPivot.x,
			entry->placement.position.y + emitterCue.spawnOffset.y +
				rotatedPivot.y,
			entry->placement.position.z + emitterCue.spawnOffset.z +
				rotatedPivot.z
		};
		const float4_t rootRotation = entry->placement.rotationQuaternion;

		uint64_t randomState = Build_Seed(
			cue, emitterCue.sourceRuntimePlacementId, pieceIndex);
		const float3_t spreadDirection = Build_SpreadDirection(
			emitterCue.direction, emitterCue.spreadDegrees, randomState);
		const f32_t speedScale = MINIMUM_SPEED_SCALE +
			(MAXIMUM_SPEED_SCALE - MINIMUM_SPEED_SCALE) *
			Random_Unit(randomState);
		const f32_t upwardSpeed = UPWARD_SPEED_METERS_PER_SECOND *
			(0.8f + Random_Unit(randomState) * 0.4f);
		const float3_t linearVelocity = {
			spreadDirection.x * emitterCue.speedMetersPerSecond * speedScale,
			spreadDirection.y * emitterCue.speedMetersPerSecond * speedScale +
				upwardSpeed,
			spreadDirection.z * emitterCue.speedMetersPerSecond * speedScale
		};
		const float3_t angularVelocity =
			Build_AngularVelocity(randomState);

		Engine::PHYSICS_RIGID_BODY_DESC desc{};
		desc.iLevelID = m_LevelId;
		desc.eActorType = Engine::PHYSICS_ACTOR_TYPE::DYNAMIC;
		desc.eMotionMode = Engine::PHYSICS_MOTION_MODE::SIMULATED;
		desc.eShapeType = Engine::PHYSICS_SHAPE_TYPE::BOX;
		desc.Pose.vPosition = rootPosition;
		desc.Pose.vRotationQuaternion = rootRotation;
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
		desc.isGravityEnabled = emitterCue.gravityScale > 0.f;
		desc.fGravityScale = emitterCue.gravityScale;
		desc.isActive = true;
		/* Product debris is cosmetic in this slice. A non-zero group keeps the
		   descriptor valid, while a zero mask prevents gameplay/world contact. */
		desc.iCollisionGroup = PRODUCT_DEBRIS_COLLISION_GROUP;
		desc.iCollisionMask = 0u;
		shared_ptr<Engine::CRigidBody> body =
			Engine::CRigidBody::Create_Runtime(desc);
		if (nullptr == body || FAILED(body->Set_LinearVelocity(linearVelocity)) ||
			FAILED(body->Set_AngularVelocity(angularVelocity)) ||
			!staged.source->Apply_DestructionDebrisPresentationPose(
				pieceIndex, rootPosition, rootRotation, true))
		{
			if (nullptr != body)
				body->Destroy_Actor();
			outError = "PhysX rejected cosmetic destruction debris";
			rollback();
			return false;
		}
		staged.fragments.push_back({ pieceIndex, std::move(body) });
	}

	outEmitter = std::move(staged);
	return true;
}

bool_t Client::CWorldDestructionDebrisPresentationRuntime::
Post_Physics_Update(const f32_t deltaSeconds, std::string& outError)
{
	outError.clear();
	if (!Is_Initialized() || !std::isfinite(deltaSeconds) ||
		deltaSeconds < 0.f)
	{
		outError = "World destruction debris update input is invalid";
		return false;
	}

	for (size_t emitterIndex = 0u; emitterIndex < m_Emitters.size();)
	{
		EMITTER_RUNTIME& emitter = m_Emitters[emitterIndex];
		emitter.elapsedSeconds += deltaSeconds;
		if (emitter.elapsedSeconds >= emitter.lifetimeSeconds)
		{
			Release_Emitter(emitter);
			m_Emitters.erase(m_Emitters.begin() + emitterIndex);
			continue;
		}

		bool_t emitterFailed = false;
		for (const FRAGMENT_RUNTIME& fragment : emitter.fragments)
		{
			Engine::PHYSICS_POSE pose{};
			if (nullptr == fragment.body ||
				FAILED(fragment.body->Get_Pose(pose)) ||
				nullptr == emitter.source ||
				!emitter.source->Apply_DestructionDebrisPresentationPose(
					fragment.visualIndex,
					pose.vPosition,
					pose.vRotationQuaternion,
					true))
			{
				emitterFailed = true;
				break;
			}
		}
		if (emitterFailed)
		{
			const uint64_t failedPlacement =
				emitter.sourceRuntimePlacementId;
			Release_Emitter(emitter);
			m_Emitters.erase(m_Emitters.begin() + emitterIndex);
			outError = "Cosmetic debris update failed for placement " +
				std::to_string(failedPlacement);
			m_Status = outError;
			return false;
		}
		++emitterIndex;
	}
	return true;
}

void Client::CWorldDestructionDebrisPresentationRuntime::
Release_Emitter(EMITTER_RUNTIME& emitter)
{
	for (FRAGMENT_RUNTIME& fragment : emitter.fragments)
	{
		if (nullptr != fragment.body)
			fragment.body->Destroy_Actor();
		fragment.body.reset();
	}
	for (const shared_ptr<CDeployPropObject>& alias : emitter.aliases)
	{
		if (nullptr != alias)
			alias->End_TransientDestructionSuppression();
	}
	if (nullptr != emitter.source)
		emitter.source->End_DestructionDebrisPresentation();
	emitter.fragments.clear();
	emitter.aliases.clear();
	emitter.source.reset();
}

void Client::CWorldDestructionDebrisPresentationRuntime::Clear()
{
	Reset_Presentation();
	m_pDeployRuntime = nullptr;
	m_LevelId = ETOUI(LEVEL::END);
	m_Device.Reset();
	m_Context.Reset();
	m_Status = "World destruction debris is not initialized";
}

void Client::CWorldDestructionDebrisPresentationRuntime::Reset_Presentation()
{
	for (EMITTER_RUNTIME& emitter : m_Emitters)
		Release_Emitter(emitter);
	m_Emitters.clear();
	m_AcceptedCueKeys.clear();
	m_Status = Is_Initialized() ?
		"World destruction debris presentation was reset" :
		"World destruction debris is not initialized";
}

uint32_t Client::CWorldDestructionDebrisPresentationRuntime::
Get_ActiveActorCount() const
{
	size_t count = 0u;
	for (const EMITTER_RUNTIME& emitter : m_Emitters)
		count += emitter.fragments.size();
	return static_cast<uint32_t>((std::min)(
		count, static_cast<size_t>(UINT32_MAX)));
}
