#include "EstherSkillSystem.h"

#include <cmath>

const LostArk::Server::ESTHER_ROSTER_ENTRY*
LostArk::Server::Find_EstherDefinition(
	const LostArk::Shared::ESTHER_ID estherId)
{
	for (const ESTHER_ROSTER_ENTRY& entry : ESTHER_DEFINITIONS)
	{
		if (entry.eEstherId == estherId)
			return &entry;
	}
	return nullptr;
}

void LostArk::Server::CEstherSkillSystem::Initialize(
	const LostArk::Shared::WORLD_ID worldId)
{
	m_isEnabled =
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId;
	m_Roster = LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId ?
		ESTHER_ROSTER_KOUKUSAYDON : ESTHER_ROSTER_VALTAN;
	Reset();
}

void LostArk::Server::CEstherSkillSystem::Update(
	const float fixedDeltaSeconds,
	const bool hasPlayers)
{
	if (!m_isEnabled || !hasPlayers ||
		!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		m_iGauge >= GAUGE_MAXIMUM)
	{
		return;
	}

	m_fRegenRemainder += REGEN_PER_SECOND * fixedDeltaSeconds;
	const float wholePoints = std::floor(m_fRegenRemainder);
	m_fRegenRemainder -= wholePoints;
	const std::uint32_t gained = static_cast<std::uint32_t>(wholePoints);
	m_iGauge = (GAUGE_MAXIMUM - m_iGauge <= gained) ?
		GAUGE_MAXIMUM : m_iGauge + gained;
}

void LostArk::Server::CEstherSkillSystem::Reset()
{
	m_iGauge = 0u;
	m_fRegenRemainder = 0.f;
}

LostArk::Server::ESTHER_USE_REJECTION
LostArk::Server::CEstherSkillSystem::Try_Consume(
	const std::uint8_t slotIndex,
	const ESTHER_ROSTER_ENTRY*& outEntry)
{
	if (!m_isEnabled)
		return ESTHER_USE_REJECTION::DISABLED_WORLD;
	if (slotIndex < LostArk::Shared::MIN_ESTHER_SLOT_INDEX ||
		slotIndex > LostArk::Shared::MAX_ESTHER_SLOT_INDEX)
	{
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	}
	const ESTHER_ROSTER_ENTRY* pEntry = Find_EstherDefinition(
		m_Roster[slotIndex - LostArk::Shared::MIN_ESTHER_SLOT_INDEX]);
	if (nullptr == pEntry)
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	if (m_iGauge < GAUGE_MAXIMUM)
		return ESTHER_USE_REJECTION::GAUGE_NOT_FULL;

	Reset();
	outEntry = pEntry;
	return ESTHER_USE_REJECTION::NONE;
}
