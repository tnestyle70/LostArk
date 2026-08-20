#include "EncounterPropRuntime.h"

#include <algorithm>
#include <limits>

namespace
{
	using LostArk::Shared::ENCOUNTER_PROP_STATE;

	bool Is_ForwardTick(
		const std::uint32_t candidate,
		const std::uint32_t reference)
	{
		if (candidate == reference)
			return true;
		const std::uint32_t forward = candidate - reference;
		return forward < ((std::numeric_limits<std::uint32_t>::max)() / 2u);
	}
}

bool LostArk::Server::CEncounterPropRuntime::Initialize(
	const ENCOUNTER_PROP_SET_DESCRIPTOR& descriptor,
	std::string& status,
	const std::uint32_t initialServerTick)
{
	if (descriptor.strPropSetId.empty() || descriptor.strEncounterId.empty() ||
		descriptor.SlotIds.empty() ||
		descriptor.SlotIds.size() > LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS ||
		0u == initialServerTick)
	{
		status = "Encounter prop descriptor is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PROP_SLOT_STATE> staged;
	staged.reserve(descriptor.SlotIds.size());
	for (const std::string& slotId : descriptor.SlotIds)
	{
		if (slotId.empty())
		{
			status = "Encounter prop slot identity is invalid";
			return false;
		}
		if (std::any_of(staged.begin(), staged.end(),
			[&slotId](const ENCOUNTER_PROP_SLOT_STATE& existing)
			{
				return existing.strSlotId == slotId;
			}))
		{
			status = "Duplicate encounter prop slot: " + slotId;
			return false;
		}
		ENCOUNTER_PROP_SLOT_STATE state{};
		state.strSlotId = slotId;
		state.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		state.iStateVersion = 1u;
		state.iStateStartTick = initialServerTick;
		state.iOccurrenceSequence = 0u;
		staged.push_back(std::move(state));
	}
	/* The wire carries slots in one canonical order, so the codec can reject a
	   set that two servers would disagree about. */
	std::sort(staged.begin(), staged.end(),
		[](const ENCOUNTER_PROP_SLOT_STATE& left,
			const ENCOUNTER_PROP_SLOT_STATE& right)
		{
			return left.strSlotId < right.strSlotId;
		});

	m_Descriptor = descriptor;
	m_Slots = std::move(staged);
	m_iEncounterEpoch = 1u;
	m_iOccurrenceSequence = 0u;
	m_bInitialized = true;
	status = "Encounter prop runtime initialized";
	return true;
}

bool LostArk::Server::CEncounterPropRuntime::Reset(
	std::string& status,
	const std::uint32_t resetServerTick)
{
	if (!m_bInitialized || 0u == resetServerTick)
	{
		status = "Encounter prop runtime is not initialized";
		return false;
	}
	for (ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		slot.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		slot.iStateVersion = 1u;
		slot.iStateStartTick = resetServerTick;
		slot.iOccurrenceSequence = 0u;
	}
	m_iEncounterEpoch = (std::numeric_limits<std::uint32_t>::max)() ==
		m_iEncounterEpoch ? 1u : m_iEncounterEpoch + 1u;
	m_iOccurrenceSequence = 0u;
	status = "Encounter prop runtime reset";
	return true;
}

LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT
LostArk::Server::CEncounterPropRuntime::Prepare_Transition(
	const ENCOUNTER_PROP_STATE fromState,
	const ENCOUNTER_PROP_STATE toState,
	const std::uint32_t occurrenceSequence,
	const std::uint32_t serverTick,
	ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick || 0u == occurrenceSequence)
	{
		status = "Encounter prop transition input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	if (!Is_ForwardTick(occurrenceSequence, m_iOccurrenceSequence))
	{
		status = "Encounter prop occurrence sequence went backwards";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = occurrenceSequence;
	for (const ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		/* Already in the destination state for this same occurrence: the edge
		   arrived twice and the second one must not emit anything. */
		if (slot.eState == toState &&
			slot.iOccurrenceSequence == occurrenceSequence)
		{
			continue;
		}
		if (slot.eState != fromState)
		{
			status = "Encounter prop slot is not in the expected state: " +
				slot.strSlotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		ENCOUNTER_PROP_SLOT_STATE next = slot;
		next.eState = toState;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() ==
			slot.iStateVersion ? 1u : slot.iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		next.iOccurrenceSequence = occurrenceSequence;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "Encounter prop transition is already applied";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop transition prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT
LostArk::Server::CEncounterPropRuntime::Prepare_Spawn(
	const std::uint32_t occurrenceSequence,
	const std::uint32_t serverTick,
	ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status) const
{
	/* A new cycle only starts from a strictly newer occurrence, which is what
	   lets the same four slots be reused without a rewound destruction group. */
	if (0u != m_iOccurrenceSequence &&
		occurrenceSequence <= m_iOccurrenceSequence)
	{
		transaction = ENCOUNTER_PROP_TRANSACTION{};
		status = "Encounter prop spawn repeated an occurrence";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	return Prepare_Transition(
		ENCOUNTER_PROP_STATE::HIDDEN, ENCOUNTER_PROP_STATE::INTACT,
		occurrenceSequence, serverTick, transaction, status);
}

LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT
LostArk::Server::CEncounterPropRuntime::Prepare_Break(
	const std::uint32_t occurrenceSequence,
	const std::uint32_t serverTick,
	ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status) const
{
	return Prepare_Transition(
		ENCOUNTER_PROP_STATE::INTACT, ENCOUNTER_PROP_STATE::BREAKING,
		occurrenceSequence, serverTick, transaction, status);
}

LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT
LostArk::Server::CEncounterPropRuntime::Prepare_BreakSlots(
	const std::vector<std::string>& slotIds,
	const std::uint32_t occurrenceSequence,
	const std::uint32_t serverTick,
	ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick || 0u == occurrenceSequence ||
		slotIds.empty())
	{
		status = "Encounter prop partial break input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	if (!Is_ForwardTick(occurrenceSequence, m_iOccurrenceSequence))
	{
		status = "Encounter prop occurrence sequence went backwards";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}

	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = occurrenceSequence;
	for (const std::string& slotId : slotIds)
	{
		/* A repeated id in one edge would stage the same slot twice and commit
		   two version bumps for a single event. */
		const bool duplicated = std::count(
			slotIds.begin(), slotIds.end(), slotId) > 1;
		if (duplicated)
		{
			status = "Encounter prop partial break names a slot twice: " + slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		const auto found = std::find_if(m_Slots.begin(), m_Slots.end(),
			[&slotId](const ENCOUNTER_PROP_SLOT_STATE& candidate)
			{ return candidate.strSlotId == slotId; });
		if (m_Slots.end() == found)
		{
			status = "Encounter prop partial break names an unknown slot: " +
				slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		/* Already shattered on this occurrence: the edge arrived twice, or a
		   player skill got there first, and neither may emit a second event. */
		if (ENCOUNTER_PROP_STATE::BREAKING == found->eState &&
			found->iOccurrenceSequence == occurrenceSequence)
		{
			continue;
		}
		if (ENCOUNTER_PROP_STATE::INTACT != found->eState)
		{
			status = "Encounter prop slot is not raised: " + slotId;
			return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
		}
		ENCOUNTER_PROP_SLOT_STATE next = *found;
		next.eState = ENCOUNTER_PROP_STATE::BREAKING;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() ==
			found->iStateVersion ? 1u : found->iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		next.iOccurrenceSequence = occurrenceSequence;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "Encounter prop partial break is already applied";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop partial break prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

LostArk::Server::ENCOUNTER_PROP_PREPARE_RESULT
LostArk::Server::CEncounterPropRuntime::Prepare_DueRemoval(
	const std::uint32_t serverTick,
	const std::uint32_t breakingTicks,
	ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status) const
{
	transaction = ENCOUNTER_PROP_TRANSACTION{};
	if (!m_bInitialized || 0u == serverTick)
	{
		status = "Encounter prop removal input is invalid";
		return ENCOUNTER_PROP_PREPARE_RESULT::REJECTED;
	}
	transaction.iEncounterEpoch = m_iEncounterEpoch;
	transaction.iCommitTick = serverTick;
	transaction.iOccurrenceSequence = m_iOccurrenceSequence;
	for (const ENCOUNTER_PROP_SLOT_STATE& slot : m_Slots)
	{
		if (ENCOUNTER_PROP_STATE::BREAKING != slot.eState)
			continue;
		const std::uint32_t elapsed = serverTick - slot.iStateStartTick;
		if (!Is_ForwardTick(serverTick, slot.iStateStartTick) ||
			elapsed < breakingTicks)
		{
			continue;
		}
		ENCOUNTER_PROP_SLOT_STATE next = slot;
		next.eState = ENCOUNTER_PROP_STATE::HIDDEN;
		next.iStateVersion = (std::numeric_limits<std::uint32_t>::max)() ==
			slot.iStateVersion ? 1u : slot.iStateVersion + 1u;
		next.iStateStartTick = serverTick;
		transaction.Slots.push_back(std::move(next));
	}
	if (transaction.Slots.empty())
	{
		status = "No encounter prop removal is due";
		return ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE;
	}
	status = "Encounter prop removal prepared";
	return ENCOUNTER_PROP_PREPARE_RESULT::READY;
}

bool LostArk::Server::CEncounterPropRuntime::Commit(
	const ENCOUNTER_PROP_TRANSACTION& transaction,
	std::string& status)
{
	if (!m_bInitialized || transaction.Slots.empty() ||
		transaction.iEncounterEpoch != m_iEncounterEpoch ||
		0u == transaction.iCommitTick)
	{
		status = "Encounter prop transaction does not match the live runtime";
		return false;
	}
	/* Validate the whole batch first: a slot that moved underneath this
	   transaction invalidates all of it, so no cycle is ever half applied. */
	for (const ENCOUNTER_PROP_SLOT_STATE& staged : transaction.Slots)
	{
		const auto live = std::find_if(m_Slots.begin(), m_Slots.end(),
			[&staged](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return slot.strSlotId == staged.strSlotId;
			});
		if (m_Slots.end() == live ||
			staged.iStateVersion != live->iStateVersion + 1u)
		{
			status = "Encounter prop slot changed before commit: " +
				staged.strSlotId;
			return false;
		}
	}
	for (const ENCOUNTER_PROP_SLOT_STATE& staged : transaction.Slots)
	{
		const auto live = std::find_if(m_Slots.begin(), m_Slots.end(),
			[&staged](const ENCOUNTER_PROP_SLOT_STATE& slot)
			{
				return slot.strSlotId == staged.strSlotId;
			});
		*live = staged;
	}
	if (0u != transaction.iOccurrenceSequence)
		m_iOccurrenceSequence = transaction.iOccurrenceSequence;
	status = "Encounter prop transaction committed";
	return true;
}
