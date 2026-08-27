#include "WorldDestructionProjectionRuntime.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <unordered_set>

namespace
{
	using namespace Client;
	using namespace LostArk::Shared;

	bool_t Is_StableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= MAX_STABLE_NETWORK_ID_BYTES &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool_t Map_State(
		const WORLD_DESTRUCTION_RUNTIME_STATE source,
		const bool_t isSuppressionAlias,
		DEPLOY_PROP_STATE& outState)
	{
		switch (source)
		{
		case WORLD_DESTRUCTION_RUNTIME_STATE::INTACT:
		case WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING:
			outState = DEPLOY_PROP_STATE::INTACT;
			return true;
		case WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED:
			outState = isSuppressionAlias ?
				DEPLOY_PROP_STATE::DESPAWNED : DEPLOY_PROP_STATE::FRACTURED;
			return true;
		case WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED:
			outState = DEPLOY_PROP_STATE::DESPAWNED;
			return true;
		default:
			return false;
		}
	}

	bool_t Is_ValidState(const WORLD_DESTRUCTION_STATE_WIRE& state)
	{
		if (!Is_StableId(state.strGroupId) || 0u == state.iStateVersion ||
			0u == state.iStateStartTick ||
			static_cast<uint8_t>(state.eState) >=
				static_cast<uint8_t>(WORLD_DESTRUCTION_RUNTIME_STATE::END))
		{
			return false;
		}
		if (WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING == state.eState)
		{
			const uint32_t span =
				static_cast<uint32_t>(state.iCommitTick - state.iStateStartTick);
			return 0u != state.iCommitTick && 0u != span && span < 0x80000000u;
		}
		return 0u == state.iCommitTick;
	}

	bool_t Is_SameState(
		const WORLD_DESTRUCTION_STATE_WIRE& left,
		const WORLD_DESTRUCTION_STATE_WIRE& right)
	{
		return left.strGroupId == right.strGroupId &&
			left.eState == right.eState &&
			left.iStateVersion == right.iStateVersion &&
			left.iStateStartTick == right.iStateStartTick &&
			left.iCommitTick == right.iCommitTick;
	}

	bool_t Is_StrictlyNextVersion(
		const uint32_t current,
		const uint32_t candidate)
	{
		return current != (std::numeric_limits<uint32_t>::max)() &&
			candidate == current + 1u;
	}

	bool_t Is_ForwardTick(const uint32_t candidate, const uint32_t previous)
	{
		return 0u != candidate && candidate != previous &&
			static_cast<uint32_t>(candidate - previous) < 0x80000000u;
	}
}

bool_t Client::CWorldDestructionProjectionRuntime::Stage_Full(
	const CWorldDestructionProjectionDocument& document,
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& message,
	WORLD_DESTRUCTION_PROJECTION_TRANSACTION& outTransaction,
	std::string& outStatus) const
{
	if (!document.Is_Ready() ||
		message.strCombatRuntimeRevision != document.Get_CombatRuntimeRevision() ||
		0u == message.iServerTick || 0u == message.iEncounterEpoch ||
		message.GroupStates.size() != document.Get_Groups().size())
	{
		outStatus = "World destruction full sync header does not match projection";
		return false;
	}
	if (m_isSynchronized &&
		(message.iEncounterEpoch == m_iEncounterEpoch ||
			!Is_ForwardTick(message.iEncounterEpoch, m_iEncounterEpoch)))
	{
		outStatus = "World destruction full sync did not advance the encounter epoch";
		return false;
	}

	WORLD_DESTRUCTION_PROJECTION_TRANSACTION staged;
	staged.strCombatRuntimeRevision = message.strCombatRuntimeRevision;
	staged.iEncounterEpoch = message.iEncounterEpoch;
	staged.iServerTick = message.iServerTick;
	staged.iLastEventSequence = 0u;
	staged.GroupStates.reserve(message.GroupStates.size());

	std::unordered_set<uint64_t> placementIds;
	for (size_t index = 0u; index < message.GroupStates.size(); ++index)
	{
		const WORLD_DESTRUCTION_STATE_WIRE& state = message.GroupStates[index];
		const WORLD_DESTRUCTION_PROJECTION_GROUP& group =
			document.Get_Groups()[index];
		if (!Is_ValidState(state) || state.strGroupId != group.strGroupId ||
			static_cast<uint8_t>(state.eState) >=
				static_cast<uint8_t>(WORLD_DESTRUCTION_RUNTIME_STATE::END))
		{
			outStatus = "World destruction full sync group is invalid";
			return false;
		}

		for (const uint64_t placementId : group.MemberPlacementIds)
		{
			DEPLOY_PROP_STATE presentationState = DEPLOY_PROP_STATE::INTACT;
			const bool_t isSuppressionAlias = std::binary_search(
				group.SuppressionAliasPlacementIds.begin(),
				group.SuppressionAliasPlacementIds.end(), placementId);
			if (!Map_State(
				state.eState, isSuppressionAlias, presentationState))
			{
				outStatus = "World destruction full sync state is unsupported";
				return false;
			}
			if (0u == placementId || !placementIds.insert(placementId).second)
			{
				outStatus = "World destruction projection membership is invalid";
				return false;
			}
			staged.PlacementStates.emplace_back(placementId, presentationState);
		}
		staged.GroupStates.push_back(state);
	}

	if (staged.PlacementStates.empty())
	{
		outStatus = "World destruction projection produced no placement states";
		return false;
	}
	outTransaction = std::move(staged);
	outStatus = "Staged persistent world destruction full sync";
	return true;
}

bool_t Client::CWorldDestructionProjectionRuntime::Stage_Delta(
	const CWorldDestructionProjectionDocument& document,
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& message,
	WORLD_DESTRUCTION_PROJECTION_TRANSACTION& outTransaction,
	std::string& outStatus) const
{
	if (!m_isSynchronized || !document.Is_Ready() ||
		message.strCombatRuntimeRevision != document.Get_CombatRuntimeRevision() ||
		message.strCombatRuntimeRevision != m_strCombatRuntimeRevision ||
		0u == message.iServerTick || message.iEncounterEpoch != m_iEncounterEpoch ||
		m_GroupStates.size() != document.Get_Groups().size() ||
		(message.ChangedStates.empty() && message.LiveEvents.empty()) ||
		(message.iServerTick != m_iServerTick &&
			!Is_ForwardTick(message.iServerTick, m_iServerTick)))
	{
		outStatus = "World destruction delta header does not match synchronized state";
		return false;
	}

	WORLD_DESTRUCTION_PROJECTION_TRANSACTION staged;
	staged.strCombatRuntimeRevision = message.strCombatRuntimeRevision;
	staged.iEncounterEpoch = message.iEncounterEpoch;
	staged.iServerTick = message.iServerTick;
	staged.iLastEventSequence = m_iLastEventSequence;
	staged.GroupStates = m_GroupStates;
	std::unordered_set<uint64_t> placementIds;
	std::string previousChangedGroupId;
	for (const WORLD_DESTRUCTION_STATE_WIRE& changed : message.ChangedStates)
	{
		if (!Is_ValidState(changed) ||
			(!previousChangedGroupId.empty() &&
				!(previousChangedGroupId < changed.strGroupId)))
		{
			outStatus = "World destruction delta state order is invalid";
			return false;
		}
		previousChangedGroupId = changed.strGroupId;

		const WORLD_DESTRUCTION_PROJECTION_GROUP* group =
			document.Find_Group(changed.strGroupId);
		if (nullptr == group)
		{
			outStatus = "World destruction delta group is unknown";
			return false;
		}
		const size_t groupIndex = static_cast<size_t>(group -
			document.Get_Groups().data());
		if (groupIndex >= staged.GroupStates.size() ||
			staged.GroupStates[groupIndex].strGroupId != group->strGroupId)
		{
			outStatus = "World destruction synchronized group order diverged";
			return false;
		}

		const WORLD_DESTRUCTION_STATE_WIRE& current =
			staged.GroupStates[groupIndex];
		if (changed.iStateVersion == current.iStateVersion)
		{
			if (!Is_SameState(changed, current))
			{
				outStatus = "World destruction duplicate version changed content";
				return false;
			}
			continue;
		}
		if (!Is_StrictlyNextVersion(
			current.iStateVersion, changed.iStateVersion))
		{
			outStatus = "World destruction delta skipped a state version";
			return false;
		}

		for (const uint64_t placementId : group->MemberPlacementIds)
		{
			DEPLOY_PROP_STATE presentationState = DEPLOY_PROP_STATE::INTACT;
			const bool_t isSuppressionAlias = std::binary_search(
				group->SuppressionAliasPlacementIds.begin(),
				group->SuppressionAliasPlacementIds.end(), placementId);
			if (!Map_State(
				changed.eState, isSuppressionAlias, presentationState))
			{
				outStatus = "World destruction delta state is unsupported";
				return false;
			}
			if (!placementIds.insert(placementId).second)
			{
				outStatus = "World destruction delta membership is duplicated";
				return false;
			}
			staged.PlacementStates.emplace_back(placementId, presentationState);
		}
		staged.GroupStates[groupIndex] = changed;
	}

	for (const WORLD_DESTRUCTION_EVENT_WIRE& event : message.LiveEvents)
	{
		if (event.iEventSequence <= staged.iLastEventSequence)
			continue;
		const WORLD_DESTRUCTION_PROJECTION_GROUP* group =
			document.Find_Group(event.strGroupId);
		const auto changedState = std::find_if(
			message.ChangedStates.begin(), message.ChangedStates.end(),
			[&event](const WORLD_DESTRUCTION_STATE_WIRE& state)
			{
				return state.strGroupId == event.strGroupId;
			});
		if (nullptr == group || group->strMutationId != event.strMutationId ||
			event.iServerTick != message.iServerTick ||
			changedState == message.ChangedStates.end() ||
			changedState->eState != WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING ||
			changedState->iStateStartTick != event.iServerTick)
		{
			outStatus = "World destruction live event does not match a breaking transition";
			return false;
		}
		staged.iLastEventSequence = event.iEventSequence;
		staged.LiveEvents.push_back(event);
	}

	outTransaction = std::move(staged);
	outStatus = outTransaction.PlacementStates.empty() ?
		"Staged duplicate world destruction delta" :
		"Staged persistent world destruction delta";
	return true;
}

void Client::CWorldDestructionProjectionRuntime::Commit(
	WORLD_DESTRUCTION_PROJECTION_TRANSACTION transaction)
{
	m_strCombatRuntimeRevision =
		std::move(transaction.strCombatRuntimeRevision);
	m_iEncounterEpoch = transaction.iEncounterEpoch;
	m_iServerTick = transaction.iServerTick;
	m_iLastEventSequence = transaction.iLastEventSequence;
	m_GroupStates = std::move(transaction.GroupStates);
	m_isSynchronized = true;
}

void Client::CWorldDestructionProjectionRuntime::Reset()
{
	m_strCombatRuntimeRevision.clear();
	m_iEncounterEpoch = 0u;
	m_iServerTick = 0u;
	m_iLastEventSequence = 0u;
	m_GroupStates.clear();
	m_isSynchronized = false;
}
