#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace LostArk::Server
{
	enum class WORLD_DESTRUCTION_STATE : std::uint8_t
	{
		INTACT,
		BREAKING,
		FRACTURED,
		DESPAWNED,
		END
	};

	enum class WORLD_DESTRUCTION_TRIGGER_KIND : std::uint8_t
	{
		STAGE,
		BOSS_IMPACT,
		/* Geometry, not schedule. The wall falls to whichever collider actually
		   reaches it, so this binding names a receiver and deliberately carries
		   no pattern, stage or action to match against. */
		COLLIDER_CONTACT,
		END
	};

	enum class WORLD_DESTRUCTION_PREPARE_RESULT : std::uint8_t
	{
		READY,
		NO_MATCH,
		DUPLICATE_REQUEST,
		NO_CHANGE,
		REJECTED
	};

	struct WORLD_DESTRUCTION_GROUP_DESCRIPTOR final
	{
		std::string strGroupId;
		std::vector<std::string> MemberPlacementIds;
		WORLD_DESTRUCTION_STATE eInitialState =
			WORLD_DESTRUCTION_STATE::INTACT;
	};

	struct WORLD_DESTRUCTION_MUTATION_DESCRIPTOR final
	{
		std::string strMutationId;
		std::string strGroupId;
		WORLD_DESTRUCTION_STATE eFinalState =
			WORLD_DESTRUCTION_STATE::FRACTURED;
		std::uint32_t iBreakingDurationTicks = 0u;
		std::string strCollisionStateId;
		std::string strNavigationStateId;
	};

	struct WORLD_DESTRUCTION_BINDING_DESCRIPTOR final
	{
		std::string strBindingId;
		std::string strMutationId;
		WORLD_DESTRUCTION_TRIGGER_KIND eTriggerKind =
			WORLD_DESTRUCTION_TRIGGER_KIND::STAGE;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::uint32_t iStageIndex = 0u;
		std::string strImpactReceiverId;
	};

	struct WORLD_DESTRUCTION_DESCRIPTOR_GRAPH final
	{
		std::vector<WORLD_DESTRUCTION_GROUP_DESCRIPTOR> Groups;
		std::vector<WORLD_DESTRUCTION_MUTATION_DESCRIPTOR> Mutations;
		std::vector<WORLD_DESTRUCTION_BINDING_DESCRIPTOR> Bindings;
	};

	struct WORLD_DESTRUCTION_ACTION_TUPLE final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::uint32_t iStageIndex = 0u;
	};

	struct WORLD_DESTRUCTION_BINDING_APPLICATION final
	{
		std::string strBindingId;
		std::string strMutationId;
		std::uint64_t iSourceNetEntityId = 0u;
		std::uint32_t iPatternSequence = 0u;
		WORLD_DESTRUCTION_TRIGGER_KIND eTriggerKind =
			WORLD_DESTRUCTION_TRIGGER_KIND::END;
	};

	struct WORLD_DESTRUCTION_STATE_TRANSITION final
	{
		std::string strGroupId;
		std::string strMutationId;
		WORLD_DESTRUCTION_STATE ePreviousState =
			WORLD_DESTRUCTION_STATE::END;
		WORLD_DESTRUCTION_STATE eNextState =
			WORLD_DESTRUCTION_STATE::END;
		WORLD_DESTRUCTION_STATE eFinalState =
			WORLD_DESTRUCTION_STATE::END;
		std::uint32_t iPreviousStateVersion = 0u;
		std::uint32_t iNextStateVersion = 0u;
		std::uint32_t iCommitTick = 0u;
		bool bApplyPersistentMutation = false;
		std::vector<std::string> MemberPlacementIds;
		std::string strCollisionStateId;
		std::string strNavigationStateId;
	};

	struct WORLD_DESTRUCTION_TRANSACTION final
	{
		std::uint32_t iEncounterEpoch = 0u;
		std::uint32_t iRequestTick = 0u;
		std::vector<WORLD_DESTRUCTION_BINDING_APPLICATION>
			BindingApplications;
		std::vector<WORLD_DESTRUCTION_STATE_TRANSITION> Transitions;
	};

	struct WORLD_DESTRUCTION_GROUP_STATE final
	{
		std::string strGroupId;
		WORLD_DESTRUCTION_STATE eState =
			WORLD_DESTRUCTION_STATE::END;
		std::uint32_t iStateVersion = 0u;
		std::uint32_t iStateStartTick = 0u;
		std::uint32_t iCommitTick = 0u;
		std::string strPendingMutationId;
	};

	class CWorldDestructionRuntime final
	{
	public:
		bool Initialize(
			const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& descriptorGraph,
			std::string& status,
			std::uint32_t initialServerTick = 1u);
		bool Reset(
			std::string& status,
			std::uint32_t resetServerTick = 1u);

		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_StageTrigger(
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_ImpactTrigger(
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			const std::string& impactReceiverId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		/* One collider actually reached one wall. The contact sequence only has
		to move forward so the same contact is not applied twice; destruction is
		one-way, so a repeat on an already broken wall answers NO_CHANGE. */
		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_ContactTrigger(
			const std::string& contactCollisionId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t contactSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_DueStateCommits(
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		bool Commit(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status);

		bool Find_GroupState(
			const std::string& groupId,
			WORLD_DESTRUCTION_GROUP_STATE& state) const;
		std::vector<WORLD_DESTRUCTION_GROUP_STATE> Get_GroupStates() const;

		bool Is_Initialized() const noexcept { return m_bInitialized; }
		std::uint32_t Get_EncounterEpoch() const noexcept
		{
			return m_iEncounterEpoch;
		}

	private:
		struct GROUP_RUNTIME final
		{
			WORLD_DESTRUCTION_GROUP_DESCRIPTOR Descriptor;
			WORLD_DESTRUCTION_STATE eState =
				WORLD_DESTRUCTION_STATE::END;
			std::uint32_t iStateVersion = 0u;
			std::uint32_t iStateStartTick = 0u;
			std::uint32_t iCommitTick = 0u;
			std::string strPendingMutationId;
		};

		WORLD_DESTRUCTION_PREPARE_RESULT Prepare_Trigger(
			WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
			const WORLD_DESTRUCTION_ACTION_TUPLE& action,
			const std::string& impactReceiverId,
			std::uint64_t sourceNetEntityId,
			std::uint32_t patternSequence,
			std::uint32_t serverTick,
			WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::string& status) const;

		bool m_bInitialized = false;
		std::uint32_t m_iEncounterEpoch = 0u;
		std::map<std::string, GROUP_RUNTIME> m_Groups;
		std::map<std::string, WORLD_DESTRUCTION_MUTATION_DESCRIPTOR>
			m_Mutations;
		std::vector<WORLD_DESTRUCTION_BINDING_DESCRIPTOR> m_Bindings;
		std::map<std::string, WORLD_DESTRUCTION_BINDING_DESCRIPTOR>
			m_BindingById;
		std::set<std::tuple<std::string, std::uint32_t, std::uint64_t>>
			m_AppliedBindingSequences;
	};
}
