#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "ServerNavigation.h"

#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CServerCollisionSystem;
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
        std::uint32_t iHoldStartTick = 0u, iHoldEndTick = 0u;
		bool bOpened = false;
		bool bClosed = false;
		bool bChargeCaptured = false;
		bool bChargeStopped = false;
		std::optional<BOSS_PATTERN_BOSS_MOTION> ChargeMotion;
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
		// A lower-priority reaction cannot replace a card state already won in this pattern.
		std::map<std::pair<std::string, std::string>, std::uint32_t> AppliedContactMotionPriorities;
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

    struct KOUKUSAYDON_LOGIC_CAPTURE_REQUEST final
    {
        LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        std::uint32_t iHoldEndTick = 0u;
        std::uint32_t iWindowIndex = 0u;
    };

	/* What one Update tick asks the room to do. The runtime touches players and
	the boss directly; anything that reaches the wire or the audition slot list
	is returned here so the room stays the only owner of those. */
	struct KOUKUSAYDON_LOGIC_OUTPUT final
	{
		std::vector<KOUKUSAYDON_LOGIC_WORLD_PLAY> WorldSequencePlays;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<std::string> FollowupPatternIds;
        std::vector<KOUKUSAYDON_LOGIC_CAPTURE_REQUEST> CaptureRequests;
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
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const CServerNavigation* navigation = nullptr,
			const CServerCollisionSystem* collision = nullptr);
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
		static bool Update_PlayerFear(SERVER_PLAYER& player, std::uint32_t serverTick);
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

	/* One card maze run: who claimed the telescope, the suit each hunter was
	dealt and how many of that suit they have felled. It owns no entity and no
	socket -- the room spawns, hits and despawns on its verdicts -- and it is
	reset with the run, never by the roulette that shares the suit enum. */
	/* The bingo board. Two 25-bit masks are the whole of it: a white bit is a
	skull, a red bit is a skull on a line that has completed. Red never clears
	until the board resets, so a finished line stays safe. */
	class CKoukuBingoRuntime final
	{
	public:
		void Reset() noexcept
		{
			m_iWhiteMask = 0u;
			m_iRedMask = 0u;
			m_Bombs = {};
			m_Hammer = {};
		}
		/* Paints the given cells white, then promotes every line that is now
		complete. Bits outside the board are ignored rather than refused so a
		caller cannot half-apply a fill. */
		void Fill(std::uint32_t cellMask) noexcept;
		/* One bomb blast. Empty cells in the mask light up, white cells that
		are not part of a completed line go back out, and red cells are left
		alone. Every cell is judged against the board as it stood before the
		blast, so neighbours inside one cross cannot cancel each other by
		order. Completed lines promote afterwards, same as Fill. */
		void Detonate(std::uint32_t cellMask) noexcept;
		[[nodiscard]] std::uint32_t Get_WhiteMask() const noexcept { return m_iWhiteMask; }
		[[nodiscard]] std::uint32_t Get_RedMask() const noexcept { return m_iRedMask; }
		/* True while this position stands on a completed line. The wipe attack
		is the consumer; until then only the contract test asks. */
		[[nodiscard]] bool Is_Safe(float x, float z) const noexcept;

		/* One bomb slot. The room owns player lookup, so the runtime keeps the
		carrier's id and asks to be told where that carrier is when the mark
		expires. */
		struct BOMB final
		{
			LostArk::Shared::BINGO_BOMB_PHASE ePhase =
				LostArk::Shared::BINGO_BOMB_PHASE::NONE;
			LostArk::Shared::NET_ENTITY_ID iCarrierNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iDetonateTick = 0u;
		};
		using BOMB_SLOTS = std::array<BOMB,
			static_cast<std::size_t>(LostArk::Shared::KOUKU_BINGO_MAX_BOMBS)>;

		/* Marks a carrier if it has no live bomb and a slot is free. Returns
		false rather than replacing an existing mark, so a double press cannot
		restart someone else's clock. */
		bool Start_Bomb(LostArk::Shared::NET_ENTITY_ID carrier,
			std::uint32_t detonateTick) noexcept;
		/* Turns a MARKED slot into a PLANTED bomb at this position, burning
		until the given tick. */
		void Plant_Bomb(std::size_t slot, float x, float z,
			std::uint32_t fuseTick) noexcept;
		void Clear_Bomb(std::size_t slot) noexcept;
		[[nodiscard]] const BOMB_SLOTS& Get_Bombs() const noexcept { return m_Bombs; }

		/* Starts the hammer on one anchor if none is running. The room rolls
		the anchor, because the runtime owns no randomness. */
		bool Start_Hammer(std::int32_t anchor, std::uint32_t startTick,
			std::uint32_t endTick) noexcept;
		/* Moves the hammer into its next phase, or clears it after the
		sweep. Returns true while a hammer is still running. */
		bool Advance_Hammer(std::uint32_t tick) noexcept;
		[[nodiscard]] const LostArk::Shared::BINGO_HAMMER_SNAPSHOT&
			Get_Hammer() const noexcept { return m_Hammer; }

	private:
		/* Promotes every line that is now completely white. */
		void Promote_Lines() noexcept;

		std::uint32_t m_iWhiteMask = 0u;
		std::uint32_t m_iRedMask = 0u;
		BOMB_SLOTS m_Bombs{};
		LostArk::Shared::BINGO_HAMMER_SNAPSHOT m_Hammer{};
	};

	class CKoukuCardMazeRuntime final
	{
	public:
		enum class PHASE : std::uint8_t
		{
			INACTIVE,
			HUNTING,
			COMPLETE
		};

		struct SPAWN_REQUEST final
		{
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
		};

		/* What one landed hammer hit changed. */
		struct HIT_OUTCOME final
		{
			bool bStartMarch = false;
			bool bKillCounted = false;
			bool bHunterComplete = false;
			bool bAllComplete = false;
		};

		static constexpr std::uint8_t KILL_TARGET = 3u;
		static constexpr std::uint32_t TARGETS_PER_SUIT = 1u;
		/* Where in the 3000 ms hammer press the head lands, in 30 Hz ticks. */
		static constexpr std::uint32_t HAMMER_HIT_TICK_OFFSET = 12u;
		static constexpr float HAMMER_RANGE_M = 2.4f;
		/* cos(60 degrees): the swing covers a 120 degree arc in front. */
		static constexpr float HAMMER_HALF_ANGLE_COS = 0.5f;
		/* Generic hammer input amount; the maze target hit adapter consumes all remaining HP. */
		static constexpr std::uint32_t HAMMER_RAW_DAMAGE = 100u;
		/* The MAZE HUD's authored hammer id; maze targets reject other skill ids. */
		static constexpr LostArk::Shared::SKILL_ID HAMMER_SKILL_ID = 56411u;
		static constexpr const char* SPAWN_GROUP_TAG = "cardmaze.targets";
		/* The box a hammer swing must reach to take the telescope. */
		static constexpr const char* TELESCOPE_PLACEMENT_ID = "cardmaze.telescope";
		/* The CardMiro navigation region: 116 x 114 cells of 0.5 m, and the
		centre cell the telescope stands on. */
		static constexpr float MAZE_MIN_X = -24.97f;
		static constexpr float MAZE_MAX_X = 33.03f;
		static constexpr float MAZE_MIN_Z = 1323.35f;
		static constexpr float MAZE_MAX_Z = 1380.35f;
		static constexpr float CENTER_X = 0.28f;
		static constexpr float CENTER_Z = 1351.65f;
		static constexpr float CENTER_KEEPOUT_M = 5.f;
		static constexpr float SPAWN_SPACING_M = 3.f;
		static constexpr float PLAYER_KEEPOUT_M = 4.f;
		static constexpr std::uint32_t SAMPLE_ATTEMPTS = 256u;

		static const char* Archetype_ForSuit(LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept;

		/* Chooses roles, suits and target positions for one claim. Nothing is
		applied to players until Commit; a failed spawn calls Abort instead. */
		bool Plan(
			LostArk::Shared::PLAYER_ID claimantId,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation& navigation,
			std::uint32_t serverTick,
			std::vector<SPAWN_REQUEST>& outSpawns,
			std::string& outStatus);
		void Register_Target(LostArk::Shared::NET_ENTITY_ID entityId,
			LostArk::Shared::MECHANIC_CARD_SYMBOL suit);
		void Commit(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		void Abort();
		[[nodiscard]] bool Is_Target(LostArk::Shared::NET_ENTITY_ID entityId) const noexcept;
		/* A hammer hit the room already landed on a registered target. */
		HIT_OUTCOME On_TargetHit(SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, bool killed);
		/* True when no hunter is left and the room must tear the run down. */
		bool Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		bool Can_Hit(const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Retire_Target(LostArk::Shared::NET_ENTITY_ID targetId);
		bool Sample_Corridor(LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
			std::uint32_t seed, SPAWN_REQUEST& out) const;
		void Reset_Progress(SERVER_PLAYER& player);
		bool Toggle_Telescope(SERVER_PLAYER& player);
		[[nodiscard]] bool Is_SoloHunter(LostArk::Shared::PLAYER_ID playerId) const noexcept;
		void Mark_Escaped(SERVER_PLAYER& player);
		bool All_LivingCentral(const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const;
		static bool In_SafeZone(float x, float z) noexcept;
		void Complete() { m_ePhase = PHASE::COMPLETE; }
		[[nodiscard]] PHASE Get_Phase() const noexcept { return m_ePhase; }
		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_TelescopeOwner() const noexcept { return m_iTelescopeOwner; }
		[[nodiscard]] const std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL>&
			Get_Targets() const noexcept { return m_Targets; }

	private:
		struct PARTICIPANT final
		{
			LostArk::Shared::CARD_MAZE_ROLE eRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			std::uint8_t iKills = 0u;
			bool escaped = false;
		};
		static void Apply_ToPlayer(const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept;

		PHASE m_ePhase = PHASE::INACTIVE;
		LostArk::Shared::PLAYER_ID m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_Participants;
		LostArk::Shared::PLAYER_ID m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_PendingParticipants;
		/* Raised targets by entity id and the suit they show. */
		std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL> m_Targets;
		/* Each felled target counts once, however long its body stays. */
		std::set<LostArk::Shared::NET_ENTITY_ID> m_CountedKills;
		bool m_bMarchStarted = false;
	};
}
