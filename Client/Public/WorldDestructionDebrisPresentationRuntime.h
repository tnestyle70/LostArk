#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

NS_BEGIN(Engine)
class CRigidBody;
NS_END

NS_BEGIN(Client)

class CDeployPropObject;
class CDeployPropRuntime;

struct WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE final
{
	uint64_t sourceRuntimePlacementId = 0u;
	std::vector<uint64_t> suppressionAliasPlacementIds;
	float3_t spawnOffset{};
	float3_t direction{};
	f32_t speedMetersPerSecond = 0.f;
	f32_t gravityScale = 1.f;
	f32_t lifetimeSeconds = 0.f;
};

/* One transient cue accompanies an authoritative INTACT -> BREAKING edge.
   It is deliberately insufficient to mutate gameplay or persistent map state. */
struct WORLD_DESTRUCTION_DEBRIS_CUE final
{
	std::string groupId;
	uint64_t eventSequence = 0u;
	uint64_t randomSeed = 0u;
	std::vector<WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE> emitters;
};

class CWorldDestructionDebrisPresentationRuntime final
{
public:
	static constexpr uint32_t ACTORS_PER_EMITTER = 12u;
	/* The 109 phase transition breaks all 21 authored groups on one tick. The
	budget holds one complete recipe for each of them, because admitting a
	partial recipe would visibly delete two thirds of a wall instead of
	fracturing it, and starving later groups would leave whole ring sectors
	vanishing with no debris at all. */
	static constexpr uint32_t MAX_ACTIVE_ACTORS = 21u * ACTORS_PER_EMITTER;

public:
	CWorldDestructionDebrisPresentationRuntime() = default;
	~CWorldDestructionDebrisPresentationRuntime();

	CWorldDestructionDebrisPresentationRuntime(
		const CWorldDestructionDebrisPresentationRuntime&) = delete;
	CWorldDestructionDebrisPresentationRuntime& operator=(
		const CWorldDestructionDebrisPresentationRuntime&) = delete;

	bool_t Initialize(
		uint32_t levelId,
		ComPtr<ID3D11Device> device,
		ComPtr<ID3D11DeviceContext> context,
		CDeployPropRuntime& deployRuntime,
		std::string& outError);
	/* maxEmitters is the caller's fair share for this cue. A frame that drains
	   one group passes the whole remaining budget; a frame that drains the 109
	   batch divides it, so no group can spend what its peers still need. */
	bool_t Play_Cue(
		const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		uint32_t maxEmitters,
		std::string& outError);
	/* Pure admission seam used by contract harnesses. It validates stable IDs,
	   sorts emitters by placement ID, and selects only complete 12-piece
	   recipes within both the global budget and this cue's share. Defined here
	   because it touches no Engine state, so the frontend harness can link it
	   without pulling in the PhysX-backed runtime. */
	static bool_t Build_DeterministicAdmissionPlan(
		const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		const uint32_t activeActorCount,
		const uint32_t maxEmitters,
		std::vector<uint64_t>& outSourcePlacementIds,
		std::string& outError)
	{
		outSourcePlacementIds.clear();
		outError.clear();
		if (cue.groupId.empty() || 0u == cue.eventSequence ||
			cue.emitters.empty() || activeActorCount > MAX_ACTIVE_ACTORS ||
			0u != activeActorCount % ACTORS_PER_EMITTER || 0u == maxEmitters)
		{
			outError = "Destruction debris admission input is invalid";
			return false;
		}
		std::set<uint64_t> projectedIds;
		std::vector<uint64_t> orderedIds;
		orderedIds.reserve(cue.emitters.size());
		for (const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitter : cue.emitters)
		{
			if (0u == emitter.sourceRuntimePlacementId ||
				!projectedIds.emplace(emitter.sourceRuntimePlacementId).second)
			{
				outError = "Destruction debris admission has a duplicate source";
				return false;
			}
			orderedIds.push_back(emitter.sourceRuntimePlacementId);
			for (const uint64_t aliasId : emitter.suppressionAliasPlacementIds)
			{
				if (0u == aliasId || !projectedIds.emplace(aliasId).second)
				{
					outError =
						"Destruction debris admission has a duplicate alias";
					return false;
				}
			}
		}
		std::sort(orderedIds.begin(), orderedIds.end());
		const uint32_t emitterBudget = (std::min)(
			(MAX_ACTIVE_ACTORS - activeActorCount) / ACTORS_PER_EMITTER,
			maxEmitters);
		const size_t selectedCount = (std::min)(
			orderedIds.size(), static_cast<size_t>(emitterBudget));
		outSourcePlacementIds.assign(
			orderedIds.begin(), orderedIds.begin() + selectedCount);
		return true;
	}
	/* Even split of the remaining emitter slots across a drained batch, with
	   at least one slot each so a large batch still shows every group. */
	static uint32_t Resolve_CueEmitterShare(
		const uint32_t activeActorCount,
		const size_t pendingCueCount)
	{
		if (0u == pendingCueCount || activeActorCount >= MAX_ACTIVE_ACTORS)
			return 1u;
		const uint32_t freeEmitters =
			(MAX_ACTIVE_ACTORS - activeActorCount) / ACTORS_PER_EMITTER;
		const uint32_t share =
			freeEmitters / static_cast<uint32_t>(pendingCueCount);
		return 0u == share ? 1u : share;
	}
	/* Call from the owning Level after Engine PostPhysics. dt advances only
	   presentation lifetime; PhysX itself remains on the normal engine clock. */
	bool_t Post_Physics_Update(
		f32_t deltaSeconds,
		std::string& outError);
	/* FULL_SYNC generation replacement keeps admitted level prototypes and the
	   Deploy runtime binding, while dropping every transient actor/overlay and
	   dedupe key from the previous generation. */
	void Reset_Presentation();
	void Clear();

	bool_t Is_Initialized() const { return m_pDeployRuntime != nullptr; }
	uint32_t Get_ActiveActorCount() const;
	const std::string& Get_Status() const { return m_Status; }

private:
	struct FRAGMENT_RUNTIME final
	{
		uint32_t visualIndex = UINT32_MAX;
		shared_ptr<Engine::CRigidBody> body;
	};

	struct EMITTER_RUNTIME final
	{
		std::string groupId;
		uint64_t eventSequence = 0u;
		uint64_t sourceRuntimePlacementId = 0u;
		f32_t elapsedSeconds = 0.f;
		f32_t lifetimeSeconds = 0.f;
		shared_ptr<CDeployPropObject> source;
		std::vector<shared_ptr<CDeployPropObject>> aliases;
		std::vector<FRAGMENT_RUNTIME> fragments;
	};

	bool_t Admit_ModelPrototypes(std::string& outError);
	bool_t Validate_Cue(
		const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		std::string& outError) const;
	bool_t Validate_RuntimeTargets(
		const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		std::string& outError) const;
	bool_t Stage_Emitter(
		const WORLD_DESTRUCTION_DEBRIS_CUE& cue,
		const WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE& emitterCue,
		EMITTER_RUNTIME& outEmitter,
		std::string& outError);
	void Release_Emitter(EMITTER_RUNTIME& emitter);

private:
	uint32_t m_LevelId = ETOUI(LEVEL::END);
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	CDeployPropRuntime* m_pDeployRuntime = nullptr;
	std::vector<EMITTER_RUNTIME> m_Emitters;
	std::set<std::pair<std::string, uint64_t>> m_AcceptedCueKeys;
	std::string m_Status = "World destruction debris is not initialized";
};

NS_END
