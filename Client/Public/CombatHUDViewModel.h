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
		/* Class identity gauge. A maximum of 0 means the class has none and the
		HUD draws nothing for it. */
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		bool isCombatReady = true;
		LostArk::Shared::PLAYER_ACTION_STATE eAction =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		std::vector<HUD_SKILL_STATE> Skills;
	};

	struct HUD_BOSS_STATE
	{
		bool isValid = false;
		std::string strArchetypeId;
		std::string strDisplayName;
		std::uint32_t iCurrentHp = 0;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumHealthBars = 0;
		std::uint8_t iPhase = 1;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::IDLE;
		std::string strActionId;
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
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
		/* Room-shared raid Esther gauge straight from the world snapshot. A
		maximum of 0 means this world has no Esther and the HUD draws nothing. */
		void Apply_EstherGauge(
			std::uint32_t gauge,
			std::uint32_t gaugeMaximum)
		{
			m_iEstherGauge = gauge;
			m_iEstherGaugeMaximum = gaugeMaximum;
		}
		void Reset_RuntimeState();

		/* Debug-only: lets the HUD Layout Tool preview the boss bar with sample numbers without
		requiring a live Valtan encounter (real Server snapshot). Enabling stamps fixed sample data
		into m_Boss every call; disabling drops back to isValid=false so real Apply_Boss() snapshots
		take over normally the next time one arrives. Never touches Server truth. */
		void Debug_Set_Boss_Preview(bool enable);

		/* Debug-only: lets the HUD Layout Tool preview the Esther gauge fill/label with a sample
		ratio without a live Valtan encounter. Enabling stamps a fixed sample gauge and marks the
		player valid (RenderEstherGauge requires isValid); disabling zeroes the maximum, which is
		all RenderEstherGauge checks to skip drawing. Never touches Server truth. */
		void Debug_Set_Esther_Preview(bool enable);

		const HUD_PLAYER_STATE& Get_Player() const { return m_Player; }
		const HUD_BOSS_STATE& Get_Boss() const { return m_Boss; }
		std::uint32_t Get_EstherGauge() const { return m_iEstherGauge; }
		std::uint32_t Get_EstherGaugeMaximum() const
		{
			return m_iEstherGaugeMaximum;
		}
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
			LostArk::Shared::PLAYER_STANCE_ID eDefaultStance =
				LostArk::Shared::PLAYER_STANCE_ID::NONE;
		};

		struct BOSS_PROFILE_DEFINITION
		{
			std::string strDisplayName;
			std::uint32_t iMaximumHealthBars = 0;
		};

		void Build_PlayerSkills(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>* pCooldowns);

		/* Skill definitions live in CPlayerSkillCatalog because the input
		controller reads the same rows; only the boss names are HUD-only. */
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_PROFILE_DEFINITION> m_PlayerProfiles;
		std::unordered_map<std::string, BOSS_PROFILE_DEFINITION> m_BossProfiles;
		HUD_PLAYER_STATE m_Player;
		HUD_BOSS_STATE m_Boss;
		std::vector<HUD_DAMAGE_EVENT> m_DamageEvents;
		std::uint32_t m_iEstherGauge = 0;
		std::uint32_t m_iEstherGaugeMaximum = 0;
		std::string m_strStatus;
	};
}
