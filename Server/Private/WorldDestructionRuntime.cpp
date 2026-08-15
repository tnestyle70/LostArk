#include "WorldDestructionRuntime.h"

#include <algorithm>
#include <limits>
#include <set>
#include <tuple>

namespace
{
	using namespace LostArk::Server;

	constexpr std::uint32_t MAX_ORDERED_TICK_DISTANCE =
		(static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()));

	bool Is_StableId(const std::string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;

		return std::all_of(value.begin(), value.end(), [](const char valueCharacter)
		{
			const unsigned char character =
				static_cast<unsigned char>(valueCharacter);
			return (character >= 'a' && character <= 'z') ||
				(character >= 'A' && character <= 'Z') ||
				(character >= '0' && character <= '9') ||
				'.' == character || '_' == character || '-' == character;
		});
	}

	bool Is_OptionalStableId(const std::string& value)
	{
		return value.empty() || Is_StableId(value);
	}

	bool Is_PersistentState(const WORLD_DESTRUCTION_STATE state)
	{
		return WORLD_DESTRUCTION_STATE::INTACT == state ||
			WORLD_DESTRUCTION_STATE::FRACTURED == state ||
			WORLD_DESTRUCTION_STATE::DESPAWNED == state;
	}

	bool Is_FinalMutationState(const WORLD_DESTRUCTION_STATE state)
	{
		return WORLD_DESTRUCTION_STATE::FRACTURED == state ||
			WORLD_DESTRUCTION_STATE::DESPAWNED == state;
	}

	std::uint32_t Add_ServerTicksSkippingReservedZero(
		const std::uint32_t startTick,
		const std::uint32_t elapsedTicks)
	{
		constexpr std::uint64_t SERVER_TICK_CARDINALITY =
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)());
		return static_cast<std::uint32_t>(
			((static_cast<std::uint64_t>(startTick - 1u) + elapsedTicks) %
				SERVER_TICK_CARDINALITY) + 1u);
	}

	bool Has_ReachedTick(
		const std::uint32_t currentTick,
		const std::uint32_t targetTick)
	{
		return currentTick == targetTick ||
			static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	bool Is_ExactActionTuple(
		const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding,
		const WORLD_DESTRUCTION_ACTION_TUPLE& action)
	{
		return binding.strPatternId == action.strPatternId &&
			binding.strStageId == action.strStageId &&
			binding.strActionId == action.strActionId &&
			binding.iStageIndex == action.iStageIndex;
	}

	void Clear_Transaction(WORLD_DESTRUCTION_TRANSACTION& transaction)
	{
		transaction = {};
	}

	WORLD_DESTRUCTION_STATE_TRANSITION Make_Transition(
		const WORLD_DESTRUCTION_GROUP_DESCRIPTOR& groupDescriptor,
		const WORLD_DESTRUCTION_STATE groupState,
		const std::uint32_t groupStateVersion,
		const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation,
		const WORLD_DESTRUCTION_STATE nextState,
		const std::uint32_t commitTick,
		const bool applyPersistentMutation)
	{
		WORLD_DESTRUCTION_STATE_TRANSITION transition{};
		transition.strGroupId = groupDescriptor.strGroupId;
		transition.strMutationId = mutation.strMutationId;
		transition.ePreviousState = groupState;
		transition.eNextState = nextState;
		transition.eFinalState = mutation.eFinalState;
		transition.iPreviousStateVersion = groupStateVersion;
		transition.iNextStateVersion = groupStateVersion + 1u;
		transition.iCommitTick = commitTick;
		transition.bApplyPersistentMutation = applyPersistentMutation;
		transition.MemberPlacementIds = groupDescriptor.MemberPlacementIds;
		transition.strCollisionStateId = mutation.strCollisionStateId;
		transition.strNavigationStateId = mutation.strNavigationStateId;
		return transition;
	}
}

bool LostArk::Server::CWorldDestructionRuntime::Initialize(
	const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& descriptorGraph,
	std::string& status,
	const std::uint32_t initialServerTick)
{
	if (m_bInitialized)
	{
		status = "World destruction runtime is already initialized";
		return false;
	}
	if (0u == initialServerTick || descriptorGraph.Groups.empty() ||
		descriptorGraph.Mutations.empty() ||
		descriptorGraph.Bindings.empty())
	{
		status = "World destruction descriptor graph is incomplete";
		return false;
	}

	std::map<std::string, GROUP_RUNTIME> stagedGroups;
	std::set<std::string> ownedPlacementIds;
	for (const WORLD_DESTRUCTION_GROUP_DESCRIPTOR& descriptor :
		descriptorGraph.Groups)
	{
		if (!Is_StableId(descriptor.strGroupId) ||
			descriptor.MemberPlacementIds.empty() ||
			!Is_PersistentState(descriptor.eInitialState))
		{
			status = "World destruction group descriptor is invalid";
			return false;
		}

		GROUP_RUNTIME group{};
		group.Descriptor = descriptor;
		group.eState = descriptor.eInitialState;
		group.iStateVersion = 1u;
		group.iStateStartTick = initialServerTick;
		for (const std::string& placementId : descriptor.MemberPlacementIds)
		{
			if (!Is_StableId(placementId) ||
				!ownedPlacementIds.emplace(placementId).second)
			{
				status = "World destruction placement ownership is invalid";
				return false;
			}
		}
		if (!stagedGroups.emplace(descriptor.strGroupId, std::move(group)).second)
		{
			status = "World destruction group ID is duplicated";
			return false;
		}
	}

	std::map<std::string, WORLD_DESTRUCTION_MUTATION_DESCRIPTOR>
		stagedMutations;
	for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
		descriptorGraph.Mutations)
	{
		if (!Is_StableId(mutation.strMutationId) ||
			!Is_StableId(mutation.strGroupId) ||
			!Is_FinalMutationState(mutation.eFinalState) ||
			mutation.iBreakingDurationTicks > MAX_ORDERED_TICK_DISTANCE ||
			!Is_OptionalStableId(mutation.strCollisionStateId) ||
			!Is_OptionalStableId(mutation.strNavigationStateId) ||
			stagedGroups.end() == stagedGroups.find(mutation.strGroupId))
		{
			status = "World destruction mutation descriptor is invalid";
			return false;
		}
		if (!stagedMutations.emplace(
			mutation.strMutationId, mutation).second)
		{
			status = "World destruction mutation ID is duplicated";
			return false;
		}
	}

	std::vector<WORLD_DESTRUCTION_BINDING_DESCRIPTOR> stagedBindings;
	std::map<std::string, WORLD_DESTRUCTION_BINDING_DESCRIPTOR>
		stagedBindingById;
	std::set<std::tuple<WORLD_DESTRUCTION_TRIGGER_KIND, std::string,
		std::string, std::string, std::uint32_t, std::string, std::string>>
		bindingTargets;
	for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding :
		descriptorGraph.Bindings)
	{
		const bool isContactBinding =
			WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT ==
			binding.eTriggerKind;
		const bool receiverIsValid =
			(WORLD_DESTRUCTION_TRIGGER_KIND::STAGE == binding.eTriggerKind &&
				binding.strImpactReceiverId.empty()) ||
			((WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT ==
				binding.eTriggerKind || isContactBinding) &&
				Is_StableId(binding.strImpactReceiverId));
		const bool scheduleIsValid = isContactBinding ?
			(binding.strPatternId.empty() && binding.strStageId.empty() &&
				binding.strActionId.empty() && 0u == binding.iStageIndex) :
			(Is_StableId(binding.strPatternId) &&
				Is_StableId(binding.strStageId) &&
				Is_StableId(binding.strActionId));
		if (!Is_StableId(binding.strBindingId) ||
			!Is_StableId(binding.strMutationId) ||
			WORLD_DESTRUCTION_TRIGGER_KIND::END == binding.eTriggerKind ||
			!scheduleIsValid || !receiverIsValid ||
			stagedMutations.end() ==
				stagedMutations.find(binding.strMutationId))
		{
			status = "World destruction binding descriptor is invalid";
			return false;
		}

		const auto targetKey = std::make_tuple(
			binding.eTriggerKind, binding.strPatternId, binding.strStageId,
			binding.strActionId, binding.iStageIndex,
			binding.strImpactReceiverId, binding.strMutationId);
		if (!bindingTargets.emplace(targetKey).second ||
			!stagedBindingById.emplace(binding.strBindingId, binding).second)
		{
			status = "World destruction binding target or ID is duplicated";
			return false;
		}
		stagedBindings.push_back(binding);
	}

	m_Groups.swap(stagedGroups);
	m_Mutations.swap(stagedMutations);
	m_Bindings.swap(stagedBindings);
	m_BindingById.swap(stagedBindingById);
	m_AppliedBindingSequences.clear();
	m_iEncounterEpoch = 1u;
	m_bInitialized = true;
	status = "World destruction runtime initialized";
	return true;
}

bool LostArk::Server::CWorldDestructionRuntime::Reset(
	std::string& status,
	const std::uint32_t resetServerTick)
{
	if (!m_bInitialized || 0u == resetServerTick)
	{
		status = "World destruction runtime is not initialized";
		return false;
	}
	if ((std::numeric_limits<std::uint32_t>::max)() == m_iEncounterEpoch)
	{
		status = "World destruction encounter epoch is exhausted";
		return false;
	}
	std::map<std::string, GROUP_RUNTIME> stagedGroups = m_Groups;
	for (auto& [groupId, group] : stagedGroups)
	{
		(void)groupId;
		group.eState = group.Descriptor.eInitialState;
		group.iStateVersion = 1u;
		group.iStateStartTick = resetServerTick;
		group.iCommitTick = 0u;
		group.strPendingMutationId.clear();
	}
	m_Groups.swap(stagedGroups);
	m_AppliedBindingSequences.clear();
	++m_iEncounterEpoch;
	status = "World destruction runtime reset";
	return true;
}

LostArk::Server::WORLD_DESTRUCTION_PREPARE_RESULT
LostArk::Server::CWorldDestructionRuntime::Prepare_StageTrigger(
	const WORLD_DESTRUCTION_ACTION_TUPLE& action,
	const std::uint64_t sourceNetEntityId,
	const std::uint32_t patternSequence,
	const std::uint32_t serverTick,
	WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status) const
{
	return Prepare_Trigger(WORLD_DESTRUCTION_TRIGGER_KIND::STAGE,
		action, {}, sourceNetEntityId, patternSequence, serverTick,
		transaction, status);
}

LostArk::Server::WORLD_DESTRUCTION_PREPARE_RESULT
LostArk::Server::CWorldDestructionRuntime::Prepare_ImpactTrigger(
	const WORLD_DESTRUCTION_ACTION_TUPLE& action,
	const std::string& impactReceiverId,
	const std::uint64_t sourceNetEntityId,
	const std::uint32_t patternSequence,
	const std::uint32_t serverTick,
	WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status) const
{
	return Prepare_Trigger(WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT,
		action, impactReceiverId, sourceNetEntityId, patternSequence, serverTick,
		transaction, status);
}

LostArk::Server::WORLD_DESTRUCTION_PREPARE_RESULT
LostArk::Server::CWorldDestructionRuntime::Prepare_ContactTrigger(
	const std::string& contactCollisionId,
	const std::uint64_t sourceNetEntityId,
	const std::uint32_t contactSequence,
	const std::uint32_t serverTick,
	WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status) const
{
	return Prepare_Trigger(WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT,
		{}, contactCollisionId, sourceNetEntityId, contactSequence, serverTick,
		transaction, status);
}

LostArk::Server::WORLD_DESTRUCTION_PREPARE_RESULT
LostArk::Server::CWorldDestructionRuntime::Prepare_Trigger(
	const WORLD_DESTRUCTION_TRIGGER_KIND triggerKind,
	const WORLD_DESTRUCTION_ACTION_TUPLE& action,
	const std::string& impactReceiverId,
	const std::uint64_t sourceNetEntityId,
	const std::uint32_t patternSequence,
	const std::uint32_t serverTick,
	WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status) const
{
	Clear_Transaction(transaction);
	/* A contact break can happen while the boss is idle, between patterns and
	   during any animation, so it is the one trigger that carries no action
	   tuple to validate or to match. */
	const bool isContactTrigger =
		WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT == triggerKind;
	if (!m_bInitialized || 0u == sourceNetEntityId ||
		0u == patternSequence || 0u == serverTick ||
		WORLD_DESTRUCTION_TRIGGER_KIND::END == triggerKind ||
		(!isContactTrigger && (
			!Is_StableId(action.strPatternId) ||
			!Is_StableId(action.strStageId) ||
			!Is_StableId(action.strActionId))) ||
		(WORLD_DESTRUCTION_TRIGGER_KIND::STAGE != triggerKind &&
			!Is_StableId(impactReceiverId)))
	{
		status = "World destruction trigger request is invalid";
		return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iRequestTick = serverTick;
	bool foundExactBinding = false;
	bool foundDuplicate = false;
	bool foundNoChange = false;
	std::set<std::string> transitionedGroupIds;
	for (const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding : m_Bindings)
	{
		if (binding.eTriggerKind != triggerKind ||
			(!isContactTrigger && !Is_ExactActionTuple(binding, action)) ||
			binding.strImpactReceiverId != impactReceiverId)
		{
			continue;
		}
		foundExactBinding = true;
		const auto applicationKey = std::make_tuple(
			binding.strBindingId, patternSequence, sourceNetEntityId);
		if (m_AppliedBindingSequences.contains(applicationKey))
		{
			foundDuplicate = true;
			continue;
		}

		const auto mutationIt = m_Mutations.find(binding.strMutationId);
		if (m_Mutations.end() == mutationIt)
		{
			Clear_Transaction(transaction);
			status = "World destruction binding mutation is unavailable";
			return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
		}
		const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation =
			mutationIt->second;
		const auto groupIt = m_Groups.find(mutation.strGroupId);
		if (m_Groups.end() == groupIt ||
			WORLD_DESTRUCTION_STATE::INTACT != groupIt->second.eState)
		{
			foundNoChange = true;
			continue;
		}
		if ((std::numeric_limits<std::uint32_t>::max)() ==
			groupIt->second.iStateVersion ||
			!transitionedGroupIds.emplace(mutation.strGroupId).second)
		{
			Clear_Transaction(transaction);
			status = "World destruction trigger cannot allocate one atomic transition";
			return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
		}

		const bool immediate = 0u == mutation.iBreakingDurationTicks;
		const std::uint32_t commitTick = immediate ? serverTick :
			Add_ServerTicksSkippingReservedZero(
				serverTick, mutation.iBreakingDurationTicks);
		transaction.Transitions.push_back(Make_Transition(
			groupIt->second.Descriptor, groupIt->second.eState,
			groupIt->second.iStateVersion, mutation,
			immediate ? mutation.eFinalState :
				WORLD_DESTRUCTION_STATE::BREAKING,
			commitTick, immediate));
		transaction.BindingApplications.push_back({
			binding.strBindingId, binding.strMutationId,
			sourceNetEntityId, patternSequence, binding.eTriggerKind });
	}

	if (!transaction.Transitions.empty())
	{
		status = "World destruction trigger transaction prepared";
		return WORLD_DESTRUCTION_PREPARE_RESULT::READY;
	}
	Clear_Transaction(transaction);
	if (!foundExactBinding)
	{
		status = "World destruction trigger has no exact binding";
		return WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH;
	}
	if (foundDuplicate && !foundNoChange)
	{
		status = "World destruction trigger was already applied";
		return WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST;
	}
	status = "World destruction group has no available state transition";
	return foundDuplicate ? WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST :
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE;
}

LostArk::Server::WORLD_DESTRUCTION_PREPARE_RESULT
LostArk::Server::CWorldDestructionRuntime::Prepare_DueStateCommits(
	const std::uint32_t serverTick,
	WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status) const
{
	Clear_Transaction(transaction);
	if (!m_bInitialized || 0u == serverTick)
	{
		status = "World destruction commit tick is invalid";
		return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iRequestTick = serverTick;
	for (const auto& [groupId, group] : m_Groups)
	{
		(void)groupId;
		if (WORLD_DESTRUCTION_STATE::BREAKING != group.eState ||
			!Has_ReachedTick(serverTick, group.iCommitTick))
		{
			continue;
		}
		if ((std::numeric_limits<std::uint32_t>::max)() ==
			group.iStateVersion)
		{
			Clear_Transaction(transaction);
			status = "World destruction state version is exhausted";
			return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
		}
		const auto mutationIt = m_Mutations.find(group.strPendingMutationId);
		if (m_Mutations.end() == mutationIt)
		{
			Clear_Transaction(transaction);
			status = "World destruction pending mutation is unavailable";
			return WORLD_DESTRUCTION_PREPARE_RESULT::REJECTED;
		}
		transaction.Transitions.push_back(Make_Transition(
			group.Descriptor, group.eState, group.iStateVersion,
			mutationIt->second, mutationIt->second.eFinalState,
			group.iCommitTick, true));
	}

	if (transaction.Transitions.empty())
	{
		Clear_Transaction(transaction);
		status = "World destruction has no due state commits";
		return WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE;
	}
	status = "World destruction due-state transaction prepared";
	return WORLD_DESTRUCTION_PREPARE_RESULT::READY;
}

bool LostArk::Server::CWorldDestructionRuntime::Commit(
	const WORLD_DESTRUCTION_TRANSACTION& transaction,
	std::string& status)
{
	if (!m_bInitialized || transaction.iEncounterEpoch != m_iEncounterEpoch ||
		0u == transaction.iRequestTick || transaction.Transitions.empty())
	{
		status = "World destruction transaction identity is invalid";
		return false;
	}
	if (!transaction.BindingApplications.empty() &&
		transaction.BindingApplications.size() != transaction.Transitions.size())
	{
		status = "World destruction trigger transaction is incomplete";
		return false;
	}

	std::map<std::string, GROUP_RUNTIME> stagedGroups = m_Groups;
	std::set<std::tuple<std::string, std::uint32_t, std::uint64_t>>
		stagedApplications = m_AppliedBindingSequences;
	std::set<std::string> transitionedGroupIds;
	for (std::size_t index = 0u; index < transaction.Transitions.size(); ++index)
	{
		const WORLD_DESTRUCTION_STATE_TRANSITION& transition =
			transaction.Transitions[index];
		const auto groupIt = stagedGroups.find(transition.strGroupId);
		const auto mutationIt = m_Mutations.find(transition.strMutationId);
		if (stagedGroups.end() == groupIt || m_Mutations.end() == mutationIt ||
			!transitionedGroupIds.emplace(transition.strGroupId).second)
		{
			status = "World destruction transaction references an invalid graph node";
			return false;
		}
		GROUP_RUNTIME& group = groupIt->second;
		const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation =
			mutationIt->second;
		if (mutation.strGroupId != transition.strGroupId ||
			group.eState != transition.ePreviousState ||
			group.iStateVersion != transition.iPreviousStateVersion ||
			transition.iNextStateVersion != transition.iPreviousStateVersion + 1u ||
			transition.eFinalState != mutation.eFinalState ||
			transition.MemberPlacementIds != group.Descriptor.MemberPlacementIds ||
			transition.strCollisionStateId != mutation.strCollisionStateId ||
			transition.strNavigationStateId != mutation.strNavigationStateId)
		{
			status = "World destruction transition payload is stale or inconsistent";
			return false;
		}

		const bool startsBreaking =
			WORLD_DESTRUCTION_STATE::INTACT == transition.ePreviousState &&
			WORLD_DESTRUCTION_STATE::BREAKING == transition.eNextState &&
			0u < mutation.iBreakingDurationTicks &&
			!transition.bApplyPersistentMutation &&
			transition.iCommitTick == Add_ServerTicksSkippingReservedZero(
				transaction.iRequestTick, mutation.iBreakingDurationTicks);
		const bool completesBreaking =
			WORLD_DESTRUCTION_STATE::BREAKING == transition.ePreviousState &&
			transition.eNextState == mutation.eFinalState &&
			transition.bApplyPersistentMutation &&
			group.strPendingMutationId == mutation.strMutationId &&
			transition.iCommitTick == group.iCommitTick &&
			Has_ReachedTick(transaction.iRequestTick, group.iCommitTick);
		const bool commitsImmediately =
			WORLD_DESTRUCTION_STATE::INTACT == transition.ePreviousState &&
			transition.eNextState == mutation.eFinalState &&
			0u == mutation.iBreakingDurationTicks &&
			transition.bApplyPersistentMutation &&
			transition.iCommitTick == transaction.iRequestTick;
		if (!startsBreaking && !completesBreaking && !commitsImmediately)
		{
			status = "World destruction transition violates the state machine";
			return false;
		}

		if (!transaction.BindingApplications.empty())
		{
			const WORLD_DESTRUCTION_BINDING_APPLICATION& application =
				transaction.BindingApplications[index];
			const auto bindingIt = m_BindingById.find(application.strBindingId);
			const auto applicationKey = std::make_tuple(
				application.strBindingId, application.iPatternSequence,
				application.iSourceNetEntityId);
			if (m_BindingById.end() == bindingIt ||
				0u == application.iSourceNetEntityId ||
				0u == application.iPatternSequence ||
				application.strMutationId != transition.strMutationId ||
				bindingIt->second.strMutationId != transition.strMutationId ||
				application.eTriggerKind != bindingIt->second.eTriggerKind ||
				!stagedApplications.emplace(applicationKey).second ||
				completesBreaking)
			{
				status = "World destruction binding application is invalid or duplicated";
				return false;
			}
		}
		else if (!completesBreaking)
		{
			status = "World destruction trigger transition has no binding application";
			return false;
		}

		group.eState = transition.eNextState;
		group.iStateVersion = transition.iNextStateVersion;
		group.iStateStartTick = transaction.iRequestTick;
		if (startsBreaking)
		{
			group.iCommitTick = transition.iCommitTick;
			group.strPendingMutationId = transition.strMutationId;
		}
		else
		{
			group.iCommitTick = 0u;
			group.strPendingMutationId.clear();
		}
	}

	m_Groups.swap(stagedGroups);
	m_AppliedBindingSequences.swap(stagedApplications);
	status = "World destruction transaction committed";
	return true;
}

bool LostArk::Server::CWorldDestructionRuntime::Find_GroupState(
	const std::string& groupId,
	WORLD_DESTRUCTION_GROUP_STATE& state) const
{
	const auto groupIt = m_Groups.find(groupId);
	if (m_Groups.end() == groupIt)
		return false;

	state.strGroupId = groupIt->second.Descriptor.strGroupId;
	state.eState = groupIt->second.eState;
	state.iStateVersion = groupIt->second.iStateVersion;
	state.iStateStartTick = groupIt->second.iStateStartTick;
	state.iCommitTick = groupIt->second.iCommitTick;
	state.strPendingMutationId = groupIt->second.strPendingMutationId;
	return true;
}

std::vector<LostArk::Server::WORLD_DESTRUCTION_GROUP_STATE>
LostArk::Server::CWorldDestructionRuntime::Get_GroupStates() const
{
	std::vector<WORLD_DESTRUCTION_GROUP_STATE> states;
	states.reserve(m_Groups.size());
	for (const auto& [groupId, group] : m_Groups)
	{
		(void)groupId;
		WORLD_DESTRUCTION_GROUP_STATE state{};
		state.strGroupId = group.Descriptor.strGroupId;
		state.eState = group.eState;
		state.iStateVersion = group.iStateVersion;
		state.iStateStartTick = group.iStateStartTick;
		state.iCommitTick = group.iCommitTick;
		state.strPendingMutationId = group.strPendingMutationId;
		states.push_back(std::move(state));
	}
	return states;
}
