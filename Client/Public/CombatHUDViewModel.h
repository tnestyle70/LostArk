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

	class CCombatHUDViewModel final
	{
	public:
		static CCombatHUDViewModel& Get();

		bool Initialize_Definitions();
		void Apply_LocalPlayer(
			std::uint32_t serverTick,
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
		void Apply_Boss(
			const std::string& archetypeId,
			const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot);
		void Reset_RuntimeState();

		const HUD_PLAYER_STATE& Get_Player() const { return m_Player; }
		const HUD_BOSS_STATE& Get_Boss() const { return m_Boss; }
		const std::string& Get_Status() const { return m_strStatus; }

	private:
		/* Skill definitions live in CPlayerSkillCatalog because the input
		controller reads the same rows; only the boss names are HUD-only. */
		std::unordered_map<std::string, std::string> m_BossDisplayNames;
		HUD_PLAYER_STATE m_Player;
		HUD_BOSS_STATE m_Boss;
		std::string m_strStatus;
	};
}
