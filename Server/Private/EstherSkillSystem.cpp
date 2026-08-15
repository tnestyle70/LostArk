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
	std::string& outArchetypeId)
{
	if (!m_isEnabled)
		return ESTHER_USE_REJECTION::DISABLED_WORLD;
	// Valtan roster order is Sillian, Wei, Bahuntur. Only Sillian's summon
	// assets are cooked; the other two slots stay rejected until their
	// extraction lands and must not fall back to slot 1.
	if (1u != slotIndex)
		return ESTHER_USE_REJECTION::UNSUPPORTED_SLOT;
	if (m_iGauge < GAUGE_MAXIMUM)
		return ESTHER_USE_REJECTION::GAUGE_NOT_FULL;

	Reset();
	outArchetypeId = "NPC_59030";
	return ESTHER_USE_REJECTION::NONE;
}
