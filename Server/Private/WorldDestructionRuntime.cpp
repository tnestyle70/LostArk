#include "WorldDestructionRuntime.h"

#include <algorithm>
#include <limits>
#include <set>
#include <tuple>

namespace
{
	using namespace LostArk::Server;

	// 부호 있는 tick 차로 기한을 비교할 수 있도록 저작 breaking 기간을 INT32_MAX 이하로 제한한다.
	constexpr std::uint32_t MAX_ORDERED_TICK_DISTANCE =
		(static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()));

	// graph의 저장 ID는 1~128바이트 ASCII 영숫자/점/밑줄/하이픈으로 제한한다. 존재 참조는 별도 map 검사다.
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

	// 선택적 충돌/navigation ID는 빈 값 또는 같은 stable ID 형식만 허용한다.
	bool Is_OptionalStableId(const std::string& value)
	{
		return value.empty() || Is_StableId(value);
	}

	// 초기 상태로 저장 가능한 지속 상태다. 중간 상태 BREAKING으로 초기화하지 않는다.
	bool Is_PersistentState(const WORLD_DESTRUCTION_STATE state)
	{
		return WORLD_DESTRUCTION_STATE::INTACT == state ||
			WORLD_DESTRUCTION_STATE::FRACTURED == state ||
			WORLD_DESTRUCTION_STATE::DESPAWNED == state;
	}

	// mutation의 도착점은 파괴 또는 제거 상태다. 일반 trigger로 INTACT에 복구하지 않는다.
	bool Is_FinalMutationState(const WORLD_DESTRUCTION_STATE state)
	{
		return WORLD_DESTRUCTION_STATE::FRACTURED == state ||
			WORLD_DESTRUCTION_STATE::DESPAWNED == state;
	}

	// 1~UINT32_MAX를 순환하는 tick에 기간을 더한다. 0은 예약 없음이므로 wrap 때 건너뛴다.
	// startTick이 0이 아니라는 전제는 요청 검사에서 확보한다.
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

	// 같은 tick 또는 부호 있는 차가 양수이면 기한 도달이다. 무제한으로 오래된 tick의 순서를 판정하는 함수는 아니다.
	bool Has_ReachedTick(
		const std::uint32_t currentTick,
		const std::uint32_t targetTick)
	{
		return currentTick == targetTick ||
			static_cast<std::int32_t>(currentTick - targetTick) > 0;
	}

	// stage/impact 연결은 이름 세 개와 stage index를 전부 비교하여 다른 단계의 동명 동작과 섞지 않는다.
	bool Is_ExactActionTuple(
		const WORLD_DESTRUCTION_BINDING_DESCRIPTOR& binding,
		const WORLD_DESTRUCTION_ACTION_TUPLE& action)
	{
		return binding.strPatternId == action.strPatternId &&
			binding.strStageId == action.strStageId &&
			binding.strActionId == action.strActionId &&
			binding.iStageIndex == action.iStageIndex;
	}

	// 이전 후보의 상태와 메모리를 출력에서 제거한다. Runtime의 실제 그룹 상태는 건드리지 않는다.
	void Clear_Transaction(WORLD_DESTRUCTION_TRANSACTION& transaction)
	{
		transaction = {};
	}

	// 검증한 그룹/정의에서 다음 version과 적용 ID를 값으로 복사한다. 호출자는 version 소진을 먼저 검사한다.
	// applyPersistentMutation은 GameRoom이 충돌/navigation flag를 바꿀 시점을 구분한다.
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

// GameRoom 초기화가 bootstrap graph를 넘긴다. 정의 형식/중복/내부 참조를 임시 컨테이너에 검증한 뒤 swap한다.
// 실제 충돌 box와 navigation condition 존재 검사는 GameRoom의 후속 검증 책임이다.
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
	// 한 placement가 서로 다른 파괴 그룹에 중복 소유되지 않도록 graph 전체에 걸쳐 기록한다.
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

	// 벡터는 저작 순서의 평가, map은 commit 때 ID 조회를 맡는다. 동일한 target tuple도 중복 등록하지 않는다.
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

// 방 초기화/audition이 호출한다. epoch 소진과 입력을 먼저 검사하고 그룹 복사본을 초기 상태로 되돌린다.
// 교체 후 중복 적용 기록을 비워 새 epoch에서 같은 패턴 sequence도 다시 사용할 수 있게 한다.
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

// GameRoom의 stage 진입을 공용 trigger 준비로 전달한다. receiver는 없는 종류다.
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

// GameRoom이 확인한 돌진 receiver와 action tuple을 함께 전달한다.
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

// GameRoom이 몸체/공격 도형으로 확인한 벽 접촉을 전달한다. sequence는 중복 키이며 단조 증가를 검증하지 않는다.
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

// 실제 상태는 읽기만 하고 출력 후보를 새로 만든다. 하나의 요청에서 같은 그룹을 두 번 전이시키면 거부한다.
// 성공한 전이가 있으면 READY이며 중복/무변경 binding은 함께 있어도 건너뛸 수 있다.
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
	// contact 파괴는 보스 대기나 패턴 사이에도 들어올 수 있으므로 action tuple을 검사하지 않는다.
	// 정확한 receiver ID와 binding 종류가 일치하는지 아래에서 확인한다.
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
		// 현재 epoch에서 commit된 같은 binding/sequence/source가 있으면 다시 적용하지 않는다.
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

		// 기간 0은 즉시 최종 상태와 외부 flag를 적용하고, 양수 기간은 BREAKING 예약만 먼저 만든다.
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

// GameRoom의 Commit_DueWorldDestruction이 현재 tick을 넘긴다. 기한이 된 BREAKING 그룹만 최종 전이 후보로 모은다.
// 이 후보에는 새 binding 적용 기록이 없고, 실제 외부 flag 적용은 GameRoom의 commit 경로가 담당한다.
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

// GameRoom이 충돌/navigation 후보 준비를 끝낸 뒤 호출한다. 그룹과 중복 기록 복사본을 검증/변경해 끝에서 함께 교체한다.
// 검사 중 false가 나오면 원래 두 컨테이너는 유지되며 외부 충돌/navigation/패킷을 여기서 처리하지 않는다.
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

		// INTACT→BREAKING, BREAKING→최종, 기간 0의 INTACT→최종 중 한 상태 전이만 허용한다.
		// 예약 시작/완료 tick과 persistent flag도 해당 경로의 값이어야 한다.
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

		// trigger 시작에는 같은 index의 binding 적용 기록이 필요하다. 예약 완료에는 새 적용 기록을 추가하지 않는다.
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

// 복제와 벽/pickup 조회가 읽을 상태 복사본을 작성한다. 없는 ID이면 출력 변경 없이 false다.
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

// 전체 동기화가 사용하는 독립 값 목록이다. map의 group ID 순서를 유지하며 내부 요소 참조를 노출하지 않는다.
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
