#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Client
{
	struct HUD_SKILL_STATE
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::string strInputSlot;
		std::string strDisplayName;
		std::string strActionId;
		std::uint32_t iCooldownDurationTicks = 0;
		std::uint32_t iCooldownEndTick = 0;
		std::uint32_t iDamage = 0;

		bool Is_Ready(std::uint32_t serverTick) const
		{
			return iCooldownEndTick <= serverTick;
		}
	};

	struct HUD_PLAYER_STATE
	{
		bool isValid = false;
		bool isPreview = false;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iServerTick = 0;
		std::uint32_t iCurrentHp = 0;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iCurrentResource = 0;
		std::uint32_t iMaximumResource = 0;
		LostArk::Shared::PLAYER_ACTION_STATE eAction =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		std::vector<HUD_SKILL_STATE> Skills;
	};

	struct HUD_BOSS_STATE
	{
		bool isValid = false;
		std::string strArchetypeId;
		std::string strDisplayName;
		std::uint32_t iCurrentHp = 0;
		std::uint32_t iMaximumHp = 0;
		std::uint8_t iPhase = 1;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::IDLE;
		std::string strActionId;
	};

	struct HUD_DAMAGE_EVENT
	{
		std::uint32_t iServerTick = 0;
		LostArk::Shared::DAMAGE_EVENT Event;
	};

	class CCombatHUDViewModel final
	{
	public:
		static CCombatHUDViewModel& Get();

		bool Initialize_Definitions();
		bool Apply_CharacterPreview(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass);
		void Apply_LocalPlayer(
			std::uint32_t serverTick,
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
		void Apply_Boss(
			const std::string& archetypeId,
			const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot);
		void Apply_DamageEvents(
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::DAMAGE_EVENT>& events);
		void Reset_RuntimeState();

		const HUD_PLAYER_STATE& Get_Player() const { return m_Player; }
		const HUD_BOSS_STATE& Get_Boss() const { return m_Boss; }
		const std::vector<HUD_DAMAGE_EVENT>& Get_DamageEvents() const
		{
			return m_DamageEvents;
		}
		const std::string& Get_Status() const { return m_strStatus; }

	private:
		struct PLAYER_PROFILE_DEFINITION
		{
			std::uint32_t iMaximumHp = 0;
			std::uint32_t iMaximumResource = 0;
			/* Display-only multiplicand for skill damage rates. The server keeps
			its own copy in the gameplay bootstrap and is the only authority. */
			std::uint32_t iAttackPower = 0;
		};

		void Build_PlayerSkills(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>* pCooldowns);

		/* Skill definitions live in CPlayerSkillCatalog because the input
		controller reads the same rows; only the boss names are HUD-only. */
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_PROFILE_DEFINITION> m_PlayerProfiles;
		std::unordered_map<std::string, std::string> m_BossDisplayNames;
		HUD_PLAYER_STATE m_Player;
		HUD_BOSS_STATE m_Boss;
		std::vector<HUD_DAMAGE_EVENT> m_DamageEvents;
		std::string m_strStatus;
	};
}
