#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <string>
#include <vector>

namespace LostArk::Server
{
	/* Per-player verdict inside one judgement window. NONE is "not answered
	yet" for a continuous window and "not judged yet" for an end-tick window. */
	enum class KOUKUSAYDON_LOGIC_ANSWER : std::uint8_t
	{
		NONE,
		SUCCESS,
		FAIL,
		TIMEOUT
	};

	/* Runtime state of one authored Logic window while its pattern runs. Ticks
	are absolute Server ticks derived from the pattern start; the window itself
	never re-reads the catalog after Build. */
	struct KOUKUSAYDON_LOGIC_WINDOW_STATE final
	{
		std::uint32_t iWindowIndex = 0u;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iEndTick = 0u;
		bool bOpened = false;
		bool bClosed = false;
		std::set<std::pair<std::string, std::string>> AppliedWorldMotions;
		std::set<std::string> ContactedWorldOccurrences;
		/* STAGGER_WINDOW measures the health the boss lost since it opened. */
		std::uint32_t iBossHpAtOpen = 0u;
		std::map<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER> Answers;
	};

	/* One authored presentation cue (world sequence or scene profile) waiting
	for its Server tick. */
	struct KOUKUSAYDON_LOGIC_CUE_STATE final
	{
		std::uint32_t iIndex = 0u;
		std::uint32_t iStartTick = 0u;
		bool bStarted = false;
	};

	/* Everything the room keeps for the Logic of the pattern occurrence it is
	auditioning. Built when the pattern begins, discarded when it ends. */
	struct KOUKUSAYDON_LOGIC_LEDGER final
	{
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStartTick = 0u;
		bool bDanceActive = false;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::vector<KOUKUSAYDON_LOGIC_WINDOW_STATE> Windows;
		std::vector<std::uint32_t> ContactWindowOrder;
		std::set<std::tuple<std::string, std::uint32_t, std::string>> ConsumedContactGroups;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> MechanicTriggers;

		[[nodiscard]] bool Is_Active() const noexcept { return !strPatternId.empty(); }
	};

	struct KOUKUSAYDON_LOGIC_WORLD_PLAY final
	{
		std::string strInstanceId;
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u;
		std::string strTargetSequenceInstanceId;
		std::string strOccurrenceId;
		std::uint32_t iStartTick = 0u;
		std::string strTargetWorldOccurrenceId;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};

	/* What one Update tick asks the room to do. The runtime touches players and
	the boss directly; anything that reaches the wire or the audition slot list
	is returned here so the room stays the only owner of those. */
	struct KOUKUSAYDON_LOGIC_OUTPUT final
	{
		std::vector<KOUKUSAYDON_LOGIC_WORLD_PLAY> WorldSequencePlays;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<std::string> FollowupPatternIds;
		bool bEndPatternEarly = false;
		std::string strStatus;
	};

	/* Judges the authored Logic windows of a running KoukuSaydon pattern on the
	fixed Server clock and applies their typed RESULTs to players. It owns no
	socket, no stage transition and no pattern selection. */
	class CKoukuSaydonLogicRuntime final
	{
	public:
		static constexpr std::uint32_t DANCE_POSE_COUNT = 4u;

		static void Assign_EncounterCard(SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID encounterOwnerId, std::uint32_t serverTick);

		static void Build(
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			KOUKUSAYDON_LOGIC_LEDGER& outLedger);
		/* Ends every window without judging it: cards are taken back and the
		shield is lowered. Used on abort, completion and room reset. */
		static void Discard(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			SERVER_WORLD_ENTITY* pBoss);
		static void Update(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput);
		/* One quick-slot press while the DANCE HUD is up. The slot is resolved
		through the player's own replicated layout; the first answer wins. */
		static bool Record_InteractionSlot(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_PLAYER& player,
			LostArk::Shared::INTERACTION_SLOT slot,
			std::string& outStatus);
		/* Recomputes every player's interaction HUD mode, slot layout, madness
		maximum and clown expiry. The dance layout is dealt once per pattern
		occurrence so the judgement and the HUD read the same shuffle. */
		static void Update_PlayerModes(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const KOUKUSAYDON_LOGIC_LEDGER* pActiveLedger,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick);
		static void Apply_Result(
			SERVER_PLAYER& player,
			const BOSS_PATTERN_LOGIC_RESULT& result,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);
		/* The gauge reached its maximum or a RESULT asked for it: the player
		presents the clown body until the hold expires. durationMs 0 takes the
		encounter policy hold. */
		static void Transform_ToClown(
			SERVER_PLAYER& player,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::uint32_t durationMs);
		/* True when a hit from (sourceX, sourceZ) lands inside the boss's
		raised shield arc. The caller reflects it instead of applying it. */
		[[nodiscard]] static bool Is_ShieldReflected(
			const SERVER_WORLD_ENTITY& boss,
			float sourceX,
			float sourceZ) noexcept;
		[[nodiscard]] static bool Has_ReachedTick(
			std::uint32_t serverTick, std::uint32_t targetTick) noexcept;
		[[nodiscard]] static std::uint32_t Add_Ticks(
			std::uint32_t tick, std::uint32_t ticks) noexcept;
		[[nodiscard]] static std::uint32_t Ticks_FromMs(std::uint32_t ms) noexcept;

	private:
		static void Open_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Close_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Apply_Results(
			const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			SERVER_PLAYER* pPlayer,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput);
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Roulette(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Gaze(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static bool Is_Judgeable(const SERVER_PLAYER& player) noexcept;
	};
}
