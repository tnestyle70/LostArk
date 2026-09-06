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
		/* KoukuSaydon madness gauge and the avatar form the Server replicates
		for it. A maximum of 0 means no Saydon encounter owns the gauge. */
		std::uint32_t iCurrentMadness = 0;
		std::uint32_t iMaximumMadness = 0;
		LostArk::Shared::PLAYER_MADNESS_FORM eMadnessForm =
			LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;
		bool isCombatReady = true;
		/* Pattern status is replicated by the Server. Bind affects locomotion/action
		state while silence is projected through the existing quick-slot cooldown
		mask; neither state is authored or inferred by the HUD. */
		bool isPatternBound = false;
		std::uint32_t iPatternBindEndTick = 0;
		std::uint32_t iSilenceEndTick = 0;
		std::uint32_t iSilenceDurationTicks = 0;
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
		std::uint32_t iResponseProgress = 0;
		std::uint32_t iResponseThreshold = 0;
		/* Replicated ground position of the boss entity, for gauges drawn at the boss
		in world space (CBossImmuneGaugeView). False for the Debug boss preview, which
		has no entity. */
		bool hasPosition = false;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		std::uint32_t iServerTick = 0;
		LostArk::Shared::WORLD_ENTITY_ACTION eAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::IDLE;
		std::string strActionId;
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStageIndex = 0;
		std::uint32_t iActionStartTick = 0;
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	};

	struct HUD_DAMAGE_EVENT
	{
		std::uint32_t iServerTick = 0;
		LostArk::Shared::DAMAGE_EVENT Event;
	};

	/* One raid Esther strike -> one full-screen cutin movie. iGeneration bumps
	per request so the presentation consumer replays even the same archetype. */
	struct HUD_ESTHER_CUTIN_REQUEST
	{
		std::uint32_t iGeneration = 0;
		std::string strArchetypeId;
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

	/* Reference-resolution rect for the Raid Clear headline CMainApp::RenderRaidClearText() draws
	after EndFrame() -- same reason/split as HUD_DEADSCENE_TEXT_RECTS above.
	CLevel_ValtanArena::Update_RaidClear() fills this from its own m_pRaidClearView
	(Get_SlotRect("RaidClear_TitleTextBox")) every frame the overlay is showing. */
	struct HUD_RAIDCLEAR_TEXT_RECTS
	{
		bool isValid = false;
		float fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
		/* RaidClear_ReturnButton's own rect + the "돌아가기" label drawn over it.
		Deliberately its OWN validity flag, not shared with isValid above: the
		button only appears AFTER the celebration overlay's reveal/hold timeline
		finishes and every fading slot has already hidden itself -- the two
		states are mutually exclusive in time, never both true at once. */
		bool isButtonValid = false;
		float fButtonX = 0.f, fButtonY = 0.f, fButtonWidth = 0.f, fButtonHeight = 0.f;
	};

	/* Reference-resolution rect + already-localized "OO을(를) 획득하였습니다" text
	CMainApp::RenderItemAnnounceText() draws after EndFrame(). The text itself (not just a rect,
	unlike RaidClear's static headline) is built once per shown item by
	CLevel_ValtanArena::Update_ItemAnnounce() from the real inventory diff and item catalog, so
	this ViewModel stays a plain read-only mirror -- it never picks an item or a particle itself. */
	struct HUD_ITEMANNOUNCE_TEXT_RECTS
	{
		bool isValid = false;
		float fTextX = 0.f, fTextY = 0.f, fTextWidth = 0.f, fTextHeight = 0.f;
		/* Kept as two separate pieces (not one combined string) so
		CMainApp::RenderItemAnnounceText() can draw the item name in its own grade
		color and the "을(를) 획득하였습니다" suffix in plain white. */
		std::wstring strItemName;
		std::wstring strSuffix;
	};

	/* KoukuSaydon arena HUD mode. Retail switches the quick-slot frame into an
	"interaction" layout for the clown polymorph, the Mario side-scroll, the dance
	time and the card maze: the class emblem swaps to an interaction emblem and the
	Q..F row keeps only that mode's skills. Which mode is active is Server truth
	(or the Debug preview until the Server contract exists); the HUD only draws it. */
	enum class HUD_KOUKU_HUD_MODE : std::uint8_t
	{
		NONE,
		POLYMORPH,
		MARIO,
		DANCE,
		MAZE,
	};

	/* Q W E R A S D F in this order -- the same order Update_SkillIcons walks. */
	constexpr std::size_t HUD_KOUKU_SLOT_COUNT = 8;

	struct HUD_KOUKU_GIMMICK_STATE
	{
		bool isValid = false;
		/* Madness (retail ZoneContentsGauge 3708100): 0..iMadnessMaximum, drawn under
		the local character by CKoukuMadnessGaugeView. */
		std::uint32_t iMadnessGauge = 0;
		std::uint32_t iMadnessMaximum = 100;
		HUD_KOUKU_HUD_MODE eHudMode = HUD_KOUKU_HUD_MODE::NONE;
		/* Per quick slot (Q..F): index into the active mode's skill list from
		KoukuHudModes.json, or -1 for an empty slot. The dance mode shuffles this;
		the HUD never decides the order itself. */
		std::int8_t ModeSkillIndexBySlot[HUD_KOUKU_SLOT_COUNT] =
			{ -1, -1, -1, -1, -1, -1, -1, -1 };
		/* Server-tick cooldowns for the mode skills sitting in each slot, same
		semantics as HUD_SKILL_STATE (end <= server tick means ready). */
		std::uint32_t CooldownEndTicks[HUD_KOUKU_SLOT_COUNT] = {};
		std::uint32_t CooldownDurationTicks[HUD_KOUKU_SLOT_COUNT] = {};
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
			std::uint32_t serverTick,
			const std::string& archetypeId,
			const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot);
		/* Apply_Boss only runs while a BOSS-kind entity is still present in the current
		S2C_WORLD_SNAPSHOT, so despawning it (it simply stops appearing in future snapshots) never
		calls Apply_Boss again -- m_Boss stayed isValid with stale HP forever, so the boss health
		bar HUD never disappeared. CClientReplication::Apply_WorldEntityDespawn calls this
		explicitly for a despawned BOSS-kind entity instead. */
		void Clear_Boss() { m_Boss = {}; }
		/* The interact-gated trigger box the Server is offering this player, or
		   empty when none. Read-only view: the Server decides both when it
		   appears and when it goes away. */
		void Set_InteractPromptTriggerId(const std::string& triggerPlacementId)
		{
			m_strInteractPromptTriggerId = triggerPlacementId;
		}
		const std::string& Get_InteractPromptTriggerId() const
		{
			return m_strInteractPromptTriggerId;
		}
		/* Debug arena focus. While a focus archetype is set, Apply_Boss keeps only
		that boss, so a room holding several primary bosses shows the one the F1
		gate button chose. An empty focus restores "last primary boss wins". */
		void Set_BossFocusArchetype(const std::string& archetypeId);
		void Clear_BossFocus() { m_strBossFocusArchetype.clear(); m_bBossHidden = false; }
		void Set_BossHidden(bool hidden) { m_bBossHidden = hidden; if (hidden) m_Boss = {}; }
		const std::string& Get_BossFocusArchetype() const { return m_strBossFocusArchetype; }
		/* Despawn edge of a non-Valtan primary boss: drops the bar only when the
		bar currently shows that archetype. */
		void Clear_BossIfArchetype(const std::string& archetypeId);
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
		void Apply_EstherCutinAction(const std::string& archetypeId)
		{
			++m_EstherCutinRequest.iGeneration;
			m_EstherCutinRequest.strArchetypeId = archetypeId;
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

		void Set_RaidClearTextRects(const HUD_RAIDCLEAR_TEXT_RECTS& rects)
		{
			m_RaidClearTextRects = rects;
		}
		const HUD_RAIDCLEAR_TEXT_RECTS& Get_RaidClearTextRects() const
		{
			return m_RaidClearTextRects;
		}

		void Set_ItemAnnounceTextRects(const HUD_ITEMANNOUNCE_TEXT_RECTS& rects)
		{
			m_ItemAnnounceTextRects = rects;
		}
		const HUD_ITEMANNOUNCE_TEXT_RECTS& Get_ItemAnnounceTextRects() const
		{
			return m_ItemAnnounceTextRects;
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

#ifdef _DEBUG
		/* The explicit F1 override never replaces the replicated player values.
		Disabling it exposes the latest snapshot immediately. */
		void Debug_Set_KoukuGimmickPreview(const HUD_KOUKU_GIMMICK_STATE& state)
		{
			m_KoukuGimmickPreview = state;
		}
		bool Is_KoukuGimmickPreviewEnabled() const
		{
			return m_KoukuGimmickPreview.isValid;
		}
#endif
		HUD_KOUKU_GIMMICK_STATE Get_KoukuGimmick() const;

		const HUD_PLAYER_STATE& Get_Player() const { return m_Player; }
		/* Display-only attack power from Data/Balance/PlayerProfiles.json for the character info
		window; false when the class has no profile row. The Server stays the only authority. */
		bool Try_Get_ProfileAttackPower(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass,
			std::uint32_t& outAttackPower) const
		{
			const auto it = m_PlayerProfiles.find(characterClass);
			if (it == m_PlayerProfiles.end())
				return false;
			outAttackPower = it->second.iAttackPower;
			return true;
		}
		const HUD_BOSS_STATE& Get_Boss() const { return m_Boss; }
		/* Set from either the raw incoming WORLD_ENTITY_SNAPSHOT.eAction or the
		reliable DEAD despawn. The latter is the normal terminal edge because the
		Server may remove the boss before producing a final DEAD snapshot. The latch
		stays true only for this replication/world lifetime; Reset_RuntimeState clears
		it before another Level session can observe it. */
		void Set_BossDeadRaw(bool isDead) { m_bBossDeadRaw = m_bBossDeadRaw || isDead; }
		bool Get_BossDeadRaw() const { return m_bBossDeadRaw; }
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
		std::string m_strInteractPromptTriggerId;
		bool m_bBossDeadRaw = false;
		std::string m_strBossFocusArchetype;
		bool m_bBossHidden = false;
		std::vector<HUD_DAMAGE_EVENT> m_DamageEvents;
		std::uint32_t m_iEstherGauge = 0;
		std::uint32_t m_iEstherGaugeMaximum = 0;
		HUD_ESTHER_CUTIN_REQUEST m_EstherCutinRequest;
		HUD_DEADSCENE_TEXT_RECTS m_DeadSceneTextRects;
		HUD_RAIDCLEAR_TEXT_RECTS m_RaidClearTextRects;
		HUD_ITEMANNOUNCE_TEXT_RECTS m_ItemAnnounceTextRects;
#ifdef _DEBUG
		HUD_KOUKU_GIMMICK_STATE m_KoukuGimmickPreview;
#endif
		LostArk::Shared::S2C_INVENTORY_SNAPSHOT m_Inventory{};
		std::string m_strStatus;
	};
}
