#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace LostArk::Server
{
	struct PLAYER_SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strInputSlot;
		std::string strActionId;
		std::string strDamageProfileId;
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iResourceCost = 0;
		float fMovementDistance = 0.f;
		float fMaximumRange = 0.f;
	};

	struct BOSS_RUNTIME_PROFILE
	{
		std::string strArchetypeId;
		std::string strEncounterId;
		std::uint32_t iMaximumHp = 0;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		std::uint32_t iPhaseTwoHpPercent = 0;
	};

	struct PLAYER_RUNTIME_PROFILE
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumResource = 0;
		float fMoveSpeed = 0.f;
	};

	class CGameplayCatalog final
	{
	public:
		bool Load();

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_Damage(const std::string& damageProfileId) const;

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::unordered_map<LostArk::Shared::SKILL_ID, PLAYER_SKILL_DEFINITION>
			m_Skills;
		std::unordered_map<std::string, BOSS_RUNTIME_PROFILE> m_Bosses;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_RUNTIME_PROFILE> m_Players;
		std::unordered_map<std::string, std::uint32_t> m_DamageByProfileId;
		std::string m_strStatus;
	};
}
