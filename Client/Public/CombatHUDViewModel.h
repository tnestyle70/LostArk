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
		LostArk::Shared::SKILL_ID iCurrentSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::PLAYER_STANCE_ID eStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		/* 0 outside a staged action, 1-based stage index while one runs -- combo
		stages, and start/loop/end for a HOLD skill. The Server owns it; the UI
		must not count stages itself. Paired with iActionStartTick (a changed
		start tick is how the client learns the current stage began) so a HOLD
		skill's charge gauge can derive real per-stage elapsed time via
		CActionPresentationTimeline::Try_ResolveActionAgeSeconds, the same way
		combo animation timing already does. */
		std::uint8_t iComboStage = 0;
		std::uint32_t iActionStartTick = 0;
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
		std::uint32_t iBossCombatStateRevision = 0;
		std::uint32_t iAlivePartMask = 0;
		std::uint16_t iBossCombatFlags = 0;
		std::uint32_t iCurrentStagger = 0;
		std::uint32_t iMaximumStagger = 0;
		std::uint32_t iCurrentShield = 0;
		std::uint32_t iMaximumShield = 0;
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

	struct HUD_ESTHER_CUTIN_REQUEST
	{
		std::uint32_t iGeneration = 0;
		std::string strArchetypeId;
		std::vector<std::string> Clips;
	};

	/* Reference-resolution rects for the DeadScene labels CMainApp::RenderDeadSceneText() draws
	after EndFrame() -- see that function's comment for why the text can't be drawn where the
	DeadScene panel/button images themselves are drawn. CLevel_ValtanArena::Update_DeadScene()
	fills this from its own m_pDeadSceneView (Get_SlotRect) every frame the player is dead, so
	repositioning a slot in the HUD Layout Tool moves its text along with it instead of leaving a
	hand-copied constant in MainApp.cpp to drift out of sync. */
	struct HUD_DEADSCENE_TEXT_RECTS
	{
		bool isValid = false;
		float fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
		float fReviveTextX = 0.f, fReviveTextY = 0.f, fReviveTextWidth = 0.f, fReviveTextHeight = 0.f;
		float fSpectateX = 0.f, fSpectateY = 0.f, fSpectateWidth = 0.f, fSpectateHeight = 0.f;
		/* DeadScene_ReviveMessageMarker -- a free-standing message box positioned above the
		revive button, independent of the "부활" label drawn on the button itself (fReviveText*
		above). No string is drawn against this yet; RenderDeadSceneText only has the rect wired
		until the actual message copy is decided. */
		float fMessageX = 0.f, fMessageY = 0.f, fMessageWidth = 0.f, fMessageHeight = 0.f;
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
		/* Apply_Boss only runs while a BOSS-kind entity is still present in the current
		S2C_WORLD_SNAPSHOT, so despawning it (it simply stops appearing in future snapshots) never
		calls Apply_Boss again -- m_Boss stayed isValid with stale HP forever, so the boss health
		bar HUD never disappeared. CClientReplication::Apply_WorldEntityDespawn calls this
		explicitly for a despawned BOSS-kind entity instead. */
		void Clear_Boss() { m_Boss = {}; }
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
		void Apply_EstherCutinAction(
			const std::string& archetypeId,
			const std::vector<std::string>& clips)
		{
			++m_EstherCutinRequest.iGeneration;
			m_EstherCutinRequest.strArchetypeId = archetypeId;
			m_EstherCutinRequest.Clips = clips;
		}
		void Reset_RuntimeState();

		void Set_DeadSceneTextRects(const HUD_DEADSCENE_TEXT_RECTS& rects)
		{
			m_DeadSceneTextRects = rects;
		}
		const HUD_DEADSCENE_TEXT_RECTS& Get_DeadSceneTextRects() const
		{
			return m_DeadSceneTextRects;
		}

		/* Debug-only inventory slice. CClientReplication pushes its
		replace-in-full inventory state here the same way it pushes
		Apply_LocalPlayer, so the level-agnostic F1 debug panel can read it
		without reaching into whichever Level currently owns the live
		CClientReplication instance. */
		void Apply_Inventory(
			const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& snapshot)
		{
			m_Inventory = snapshot;
		}
		const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& Get_Inventory() const
		{
			return m_Inventory;
		}

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
		const HUD_ESTHER_CUTIN_REQUEST& Get_EstherCutinRequest() const
		{
			return m_EstherCutinRequest;
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
		HUD_ESTHER_CUTIN_REQUEST m_EstherCutinRequest;
		HUD_DEADSCENE_TEXT_RECTS m_DeadSceneTextRects;
		LostArk::Shared::S2C_INVENTORY_SNAPSHOT m_Inventory{};
		std::string m_strStatus;
	};
}
