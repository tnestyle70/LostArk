#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

LostArk::Server::CGameplayCatalogGenerations::CGameplayCatalogGenerations()
{
	m_Generations.reserve(MAX_GENERATION_COUNT);
}

bool LostArk::Server::CGameplayCatalogGenerations::Load()
{
	auto initial = std::make_shared<CGameplayCatalog>();
	if (nullptr == initial || !initial->Load())
	{
		m_strStatus = nullptr == initial ?
			"Gameplay catalog allocation failed" : initial->Get_Status();
		return false;
	}
	return Initialize(std::move(initial));
}

bool LostArk::Server::CGameplayCatalogGenerations::Initialize(
	const std::shared_ptr<const CGameplayCatalog>& initialGeneration)
{
	if (nullptr == initialGeneration ||
		!initialGeneration->Get_ActiveRevision().Is_Valid())
	{
		m_strStatus = "Initial gameplay generation is invalid";
		return false;
	}
	m_Generations.clear();
	m_Generations.push_back(initialGeneration);
	m_pActiveGeneration = initialGeneration;
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
	m_iActiveGenerationEpoch = 1u;
	m_strStatus = "Initialized immutable gameplay generation";
	return true;
}

bool LostArk::Server::CGameplayCatalogGenerations::Stage(
	const std::uint32_t transactionSequence,
	const LostArk::Shared::GameplayDataRevision& baseRevision,
	const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
	std::string& status)
{
	if (0u == transactionSequence || nullptr == m_pActiveGeneration ||
		(std::numeric_limits<std::uint16_t>::max)() ==
			m_iActiveGenerationEpoch ||
		m_pActiveGeneration->Get_ActiveRevision() != baseRevision ||
		nullptr == candidateGeneration ||
		!candidateGeneration->Get_ActiveRevision().Is_Valid() ||
		candidateGeneration->Get_ActiveRevision() == baseRevision)
	{
		status = "Room gameplay generation stage identity is invalid";
		return false;
	}
	if (nullptr != m_pStagedGeneration)
	{
		if (m_iStagedTransactionSequence == transactionSequence &&
			m_pStagedGeneration->Get_ActiveRevision() ==
				candidateGeneration->Get_ActiveRevision())
		{
			return true;
		}
		status = "Room already owns a different staged gameplay generation";
		return false;
	}
	const bool alreadyRetained = nullptr != Resolve(
		candidateGeneration->Get_ActiveRevision());
	if (!alreadyRetained && m_Generations.size() >= MAX_GENERATION_COUNT)
	{
		status = "Room gameplay generation capacity is exhausted";
		return false;
	}
	m_pStagedGeneration = alreadyRetained ?
		std::shared_ptr<const CGameplayCatalog>{} : candidateGeneration;
	if (alreadyRetained)
	{
		for (const auto& generation : m_Generations)
		{
			if (nullptr != generation && generation->Get_ActiveRevision() ==
				candidateGeneration->Get_ActiveRevision())
			{
				m_pStagedGeneration = generation;
				break;
			}
		}
	}
	m_iStagedTransactionSequence = transactionSequence;
	status.clear();
	return nullptr != m_pStagedGeneration;
}

bool LostArk::Server::CGameplayCatalogGenerations::Commit(
	const std::uint32_t transactionSequence) noexcept
{
	if (0u == transactionSequence ||
		transactionSequence != m_iStagedTransactionSequence ||
		nullptr == m_pStagedGeneration)
	{
		return false;
	}
	const bool alreadyRetained = std::any_of(
		m_Generations.begin(), m_Generations.end(),
		[this](const std::shared_ptr<const CGameplayCatalog>& generation)
		{
			return nullptr != generation &&
				generation->Get_ActiveRevision() ==
				m_pStagedGeneration->Get_ActiveRevision();
		});
	if (!alreadyRetained)
	{
		if (m_Generations.size() >= m_Generations.capacity())
			return false;
		m_Generations.push_back(m_pStagedGeneration);
	}
	m_pActiveGeneration = std::move(m_pStagedGeneration);
	m_iStagedTransactionSequence = 0u;
	++m_iActiveGenerationEpoch;
	m_strStatus = "Committed immutable gameplay generation";
	return true;
}

void LostArk::Server::CGameplayCatalogGenerations::Abort(
	const std::uint32_t transactionSequence) noexcept
{
	if (0u != transactionSequence &&
		transactionSequence != m_iStagedTransactionSequence)
	{
		return;
	}
	m_pStagedGeneration.reset();
	m_iStagedTransactionSequence = 0u;
}

void LostArk::Server::CGameplayCatalogGenerations::Collect_Garbage(
	const std::vector<LostArk::Shared::GameplayDataRevision>& livePins)
{
	m_Generations.erase(
		std::remove_if(
			m_Generations.begin(), m_Generations.end(),
			[this, &livePins](
				const std::shared_ptr<const CGameplayCatalog>& generation)
			{
				if (nullptr == generation || generation == m_pActiveGeneration ||
					generation == m_pStagedGeneration)
				{
					return false;
				}
				return livePins.end() == std::find(
					livePins.begin(), livePins.end(),
					generation->Get_ActiveRevision());
			}),
		m_Generations.end());
}

const LostArk::Server::CGameplayCatalog*
LostArk::Server::CGameplayCatalogGenerations::Resolve(
	const LostArk::Shared::GameplayDataRevision& revision) const noexcept
{
	if (!revision.Is_Valid())
		return nullptr;
	for (const auto& generation : m_Generations)
	{
		if (nullptr != generation &&
			generation->Get_ActiveRevision() == revision)
		{
			return generation.get();
		}
	}
	if (nullptr != m_pStagedGeneration &&
		m_pStagedGeneration->Get_ActiveRevision() == revision)
	{
		return m_pStagedGeneration.get();
	}
	return nullptr;
}

const LostArk::Server::CGameplayCatalog&
LostArk::Server::CGameplayCatalogGenerations::Active() const noexcept
{
	static const CGameplayCatalog EMPTY_CATALOG{};
	return nullptr == m_pActiveGeneration ? EMPTY_CATALOG : *m_pActiveGeneration;
}

const LostArk::Server::PLAYER_SKILL_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_Skill(
	const LostArk::Shared::SKILL_ID skillId) const
{
	return Active().Find_Skill(skillId);
}

const LostArk::Server::BOSS_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalogGenerations::Find_Boss(
	const std::string& archetypeId) const
{
	return Active().Find_Boss(archetypeId);
}

const std::vector<LostArk::Server::BOSS_PART_DEFINITION>*
LostArk::Server::CGameplayCatalogGenerations::Find_BossParts(
	const std::string& archetypeId) const
{
	return Active().Find_BossParts(archetypeId);
}

const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>*
LostArk::Server::CGameplayCatalogGenerations::Find_BossPatterns(
	const std::string& encounterId) const
{
	return Active().Find_BossPatterns(encounterId);
}

const LostArk::Server::BOSS_COMBAT_OBJECT_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_BossCombatObject(
	const std::string& archetypeId) const
{
	return Active().Find_BossCombatObject(archetypeId);
}

const LostArk::Server::VALTAN_TIMELINE_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimeline(
	const std::string& encounterId) const
{
	return Active().Find_ValtanTimeline(encounterId);
}

const LostArk::Server::VALTAN_TIMELINE_ROW*
LostArk::Server::CGameplayCatalogGenerations::Find_ValtanTimelineRow(
	const std::string& encounterId, const std::uint32_t commandId) const
{
	return Active().Find_ValtanTimelineRow(encounterId, commandId);
}

const LostArk::Server::BOSS_PATTERN_ROTATION_DEFINITION*
LostArk::Server::CGameplayCatalogGenerations::Find_BossPatternRotation(
	const std::string& encounterId, const std::uint32_t gameplayPhase,
	const std::uint32_t healthBar) const
{
	return Active().Find_BossPatternRotation(
		encounterId, gameplayPhase, healthBar);
}

const std::string&
LostArk::Server::CGameplayCatalogGenerations::Find_IntroPatternId(
	const std::string& encounterId) const
{
	return Active().Find_IntroPatternId(encounterId);
}

const LostArk::Server::PLAYER_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalogGenerations::Find_Player(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	return Active().Find_Player(characterClass);
}

std::uint32_t
LostArk::Server::CGameplayCatalogGenerations::Find_DamageRatePercent(
	const std::string& damageProfileId) const
{
	return Active().Find_DamageRatePercent(damageProfileId);
}

const LostArk::Shared::GameplayDataRevision&
LostArk::Server::CGameplayCatalogGenerations::Get_ActiveRevision() const noexcept
{
	return Active().Get_ActiveRevision();
}

const std::string&
LostArk::Server::CGameplayCatalogGenerations::Get_Status() const noexcept
{
	return m_strStatus.empty() ? Active().Get_Status() : m_strStatus;
}
