#include "EstherSkillSystem.h"

#include <cmath>

void LostArk::Server::CEstherSkillSystem::Initialize(
	const LostArk::Shared::WORLD_ID worldId)
{
	m_isEnabled = LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId;
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
	const ESTHER_ROSTER_ENTRY* pEntry = nullptr;
	for (const ESTHER_ROSTER_ENTRY& entry : ESTHER_ROSTER)
	{
		if (entry.iSlotIndex == slotIndex)
		{
			pEntry = &entry;
			break;
		}
	}
	if (nullptr == pEntry)
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	if (m_iGauge < GAUGE_MAXIMUM)
		return ESTHER_USE_REJECTION::GAUGE_NOT_FULL;

	Reset();
	outEntry = pEntry;
	return ESTHER_USE_REJECTION::NONE;
}
