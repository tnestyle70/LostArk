#include "EncounterPropRuntime.h"

#include <Windows.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string_view>
#include <vector>

namespace
{
	using LostArk::Shared::ENCOUNTER_PROP_STATE;
	using LostArk::Shared::WORLD_ID;

	std::string_view World_ToString(const WORLD_ID worldId)
	{
		switch (worldId)
		{
		case WORLD_ID::BERN: return "BERN";
		case WORLD_ID::VALTAN_ARENA: return "VALTAN_ARENA";
		case WORLD_ID::TRAINING_GROUND: return "TRAINING_GROUND";
		case WORLD_ID::CHARACTER_SELECT_ARENA: return "CHARACTER_SELECT_ARENA";
		default: return {};
		}
	}

	// Same resolution rule the other Server bootstraps use: an explicit root
	// wins, otherwise the DataFiles folder beside the executable.
	std::filesystem::path Resolve_DataRoot()
	{
		std::vector<wchar_t> pathBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			return std::filesystem::path(pathBuffer.data()).lexically_normal();
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
		if (0u == moduleLength || moduleLength >= pathBuffer.size())
			return {};
		return std::filesystem::path(pathBuffer.data()).parent_path().parent_path() /
			L"DataFiles";
	}

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> result;
		std::string_view view(line);
		size_t start = 0;
		while (true)
		{
			const size_t tab = view.find('	', start);
			result.push_back(view.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return result;
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto parsed = std::from_chars(
			value.data(), value.data() + value.size(), outValue);
		return std::errc{} == parsed.ec &&
			parsed.ptr == value.data() + value.size();
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '' == line.back())
			line.pop_back();
	}

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

bool LostArk::Server::Load_EncounterPropSets(
	const LostArk::Shared::WORLD_ID worldId,
	std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR>& outSets,
	std::string& status)
{
	outSets.clear();
	const std::string_view worldName = World_ToString(worldId);
	if (worldName.empty())
	{
		status = "Unknown encounter prop world ID";
		return false;
	}
	const std::filesystem::path path = Resolve_DataRoot() / L"World" /
		std::filesystem::path(std::string(worldName) + ".encounterpropsbootstrap");
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		/* Most worlds own no encounter prop set. An absent file is the normal
		   answer, not a missing dependency. */
		status = "No encounter props for world";
		return true;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		status = "Empty encounter prop bootstrap";
		return false;
	}
	StripCarriageReturn(line);
	const auto header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t revision = 0u;
	std::uint32_t setCount = 0u;
	std::uint32_t slotCount = 0u;
	if (7u != header.size() ||
		"LOSTARK_ENCOUNTER_PROP_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 1u != version ||
		header[2] != worldName ||
		!ParseNumber(header[4], revision) || 0u == revision ||
		!ParseNumber(header[5], setCount) || 0u == setCount || setCount > 8u ||
		!ParseNumber(header[6], slotCount) || 0u == slotCount ||
		slotCount > setCount * LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS)
	{
		status = "Encounter prop bootstrap header is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR> staged;
	std::uint32_t observedSlots = 0u;
	while (std::getline(input, line))
	{
		StripCarriageReturn(line);
		if (line.empty())
			continue;
		const auto fields = SplitTabs(line);
		if (!fields.empty() && "PROPSET" == fields[0])
		{
			ENCOUNTER_PROP_SET_DESCRIPTOR set{};
			if (5u != fields.size() || fields[1].empty() || fields[2].empty() ||
				fields[3].empty() ||
				!ParseNumber(fields[4], set.fCoverRadiusMeters) ||
				!std::isfinite(set.fCoverRadiusMeters) ||
				set.fCoverRadiusMeters <= 0.f)
			{
				status = "Encounter prop set row is invalid";
				return false;
			}
			set.strPropSetId = std::string(fields[1]);
			set.strEncounterId = std::string(fields[2]);
			if (std::any_of(staged.begin(), staged.end(),
				[&set](const ENCOUNTER_PROP_SET_DESCRIPTOR& existing)
				{ return existing.strPropSetId == set.strPropSetId; }))
			{
				status = "Duplicate encounter prop set: " + set.strPropSetId;
				return false;
			}
			staged.push_back(std::move(set));
		}
		else if (!fields.empty() && "PROPSLOT" == fields[0])
		{
			ENCOUNTER_PROP_SLOT_DESCRIPTOR slot{};
			if (5u != fields.size() || fields[1].empty() || fields[2].empty() ||
				!ParseNumber(fields[3], slot.fPositionX) ||
				!ParseNumber(fields[4], slot.fPositionZ) ||
				!std::isfinite(slot.fPositionX) ||
				!std::isfinite(slot.fPositionZ))
			{
				status = "Encounter prop slot row is invalid";
				return false;
			}
			slot.strSlotId = std::string(fields[2]);
			const auto owner = std::find_if(staged.begin(), staged.end(),
				[&fields](const ENCOUNTER_PROP_SET_DESCRIPTOR& candidate)
				{ return candidate.strPropSetId == fields[1]; });
			if (staged.end() == owner ||
				owner->Slots.size() >=
					LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS)
			{
				status = "Encounter prop slot has no set owner: " + slot.strSlotId;
				return false;
			}
			owner->Slots.push_back(std::move(slot));
			++observedSlots;
		}
		else
		{
			status = "Unknown encounter prop bootstrap row";
			return false;
		}
	}
	if (staged.size() != setCount || observedSlots != slotCount ||
		std::any_of(staged.begin(), staged.end(),
			[](const ENCOUNTER_PROP_SET_DESCRIPTOR& set)
			{ return set.Slots.empty(); }))
	{
		status = "Encounter prop bootstrap counts do not match its header";
		return false;
	}
	outSets = std::move(staged);
	status = "Encounter prop bootstrap loaded";
	return true;
}

bool LostArk::Server::CEncounterPropRuntime::Initialize(
	const ENCOUNTER_PROP_SET_DESCRIPTOR& descriptor,
	std::string& status,
	const std::uint32_t initialServerTick)
{
	if (descriptor.strPropSetId.empty() || descriptor.strEncounterId.empty() ||
		descriptor.Slots.empty() ||
		descriptor.Slots.size() > LostArk::Shared::MAX_ENCOUNTER_PROP_SLOTS ||
		!std::isfinite(descriptor.fCoverRadiusMeters) ||
		descriptor.fCoverRadiusMeters <= 0.f ||
		0u == initialServerTick)
	{
		status = "Encounter prop descriptor is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PROP_SLOT_STATE> staged;
	staged.reserve(descriptor.Slots.size());
	for (const ENCOUNTER_PROP_SLOT_DESCRIPTOR& slot : descriptor.Slots)
	{
		const std::string& slotId = slot.strSlotId;
		if (slotId.empty() || !std::isfinite(slot.fPositionX) ||
			!std::isfinite(slot.fPositionZ))
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
		state.fPositionX = slot.fPositionX;
		state.fPositionZ = slot.fPositionZ;
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
