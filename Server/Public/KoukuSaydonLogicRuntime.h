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
	/* 판정 window의 플레이어별 응답이다. NONE은 연속 window에서는 미응답, 종료 tick 판정에서는 아직 판정 전이다. */
	enum class KOUKUSAYDON_LOGIC_ANSWER : std::uint8_t
	{
		NONE,
		SUCCESS,
		FAIL,
		TIMEOUT
	};

	/* Build가 패턴 시작 tick을 기준으로 만든 window 진행 상태다. Update가 저작 pattern과 이 상태를 함께 소비한다. */
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
		// 주기적으로 접촉하는 공·인형의 WORLD 소유자를 구분한다. 나머지 피해는 HP 기반 광기 증가를 따른다.
		std::uint8_t iMadnessSource = 0u;
		std::uint32_t iMadnessContactIntervalMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER> Answers;
		std::set<LostArk::Shared::PLAYER_ID> InsidePlayers;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> NextContactHitTicks;
		std::map<std::string, BOSS_LOGIC_REGION> FrozenRegions;
		LostArk::Shared::NET_ENTITY_ID iFreePlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::NET_ENTITY_ID> BoundPlayers;
	};

	/* 예정된 Server tick을 기다리는 저작 world sequence·scene profile cue다. */
	struct KOUKUSAYDON_LOGIC_CUE_STATE final
	{
		std::uint32_t iIndex = 0u;
		std::uint32_t iStartTick = 0u;
		bool bStarted = false;
	};

	struct KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE final
	{
		std::uint32_t iTriggerIndex = 0u;
		std::uint32_t iStartTick = 0u, iEndTick = 0u, iNextFixedTick = 0u;
		std::uint32_t iLastUpdateTick = 0u;
		std::uint32_t iNextRandomTick = 0u, iRandomWaveOrdinal = 0u;
		std::uint32_t iBombStartTick = 0u;
		LostArk::Shared::COMBAT_OBJECT_ID iBombBodyObjectId = 0u;
		std::optional<bool> BombInsideAtEnd;
		LostArk::Shared::NET_ENTITY_ID iSelectedTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::optional<std::array<float, 3u>> LastSelectedTargetPosition;
		bool bClosed = false;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::COMBAT_OBJECT_ID> TrackingObjects;
	};

	/* 룸이 패턴 발생별로 소유하는 Logic 진행 기록이다. 패턴 시작 때 만들고 종료 때 버린다. */
	struct KOUKUSAYDON_LOGIC_LEDGER final
	{
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStartTick = 0u;
		bool bDanceActive = false;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> ScheduledDanceIntervals;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::vector<KOUKUSAYDON_LOGIC_WINDOW_STATE> Windows;
		std::set<std::string> RetiredWorldOccurrences;
		std::uint32_t iBingoLineJudgementTick = 0u, iBingoCompletedLines = 0u;
		bool bBingoLineRewardApplied = false;
		std::vector<std::uint32_t> ContactWindowOrder;
		std::set<std::tuple<std::string, std::uint32_t, std::string>> ConsumedContactGroups;
		// 이번 패턴에서 먼저 적용된 높은 우선순위의 카드 상태를 낮은 우선순위 반응이 덮지 못하게 한다.
		std::map<std::pair<std::string, std::string>, std::uint32_t> AppliedContactMotionPriorities;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> MechanicTriggers;
		std::vector<KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE> PlayerTargetWindows;
		bool bTrackingTargetReached = false;
		// 입장 시 roster를 world X 내림차순, 동률이면 PlayerId 순으로 한 번 캡처한다.
		bool bCardMazeEntryRosterCaptured = false;
		std::vector<LostArk::Shared::PLAYER_ID> CardMazeEntryPlayers;
		std::size_t iCardMazeNextHiddenPlayer = 0u;

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
		bool bAuthoredMadness = false;
		std::optional<BOSS_PATTERN_WORLD_COMBAT_BODY> CombatBody;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};

    struct KOUKUSAYDON_LOGIC_CAPTURE_REQUEST final
    {
        LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        std::uint32_t iHoldEndTick = 0u;
        std::uint32_t iWindowIndex = 0u;
    };

	/* Update가 룸에 요청하는 송신·후속 패턴·포획 결과다. 플레이어·보스 상태는 직접 갱신하지만 송신과 실행 목록 소유권은 룸에 둔다. */
	struct KOUKUSAYDON_LOGIC_OUTPUT final
	{
		std::vector<KOUKUSAYDON_LOGIC_WORLD_PLAY> WorldSequencePlays;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<std::string> FollowupPatternIds;
        std::vector<KOUKUSAYDON_LOGIC_CAPTURE_REQUEST> CaptureRequests;
		bool bEndPatternEarly = false;
		// 패턴을 끝내는 counter의 검증된 착지가 commit된 뒤에만 설정한다.
		bool bCounterSuccessLanded = false;
		bool bStaggerSuccess = false;
		std::optional<bool> BingoLineCompletion;
		std::string strStatus;
	};

	/* 고정 Server tick에서 저작 Logic의 RESULT를 판정·적용한다. */
	// 쿠크 패턴의 Logic window와 Server tick을 비교해 플레이어·보스 상태와 룸에 돌려줄 결과를 계산한다.
	// 발생별 진행 상태는 호출자가 넘기는 ledger에 보관하고, 송신·패턴 선택·stage 전환은 룸이 맡는다.
	class CKoukuSaydonLogicRuntime final
	{
	public:
        // Only physical contact hazards are offered to companion avoidance.
        [[nodiscard]] static float Predict_ContactRisk(const SERVER_WORLD_ENTITY& boss,
            const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
            const SERVER_PLAYER& probe, std::uint32_t serverTick, std::uint32_t horizonMs);

		static constexpr std::uint32_t DANCE_POSE_COUNT = 4u;

		static void Assign_EncounterCard(SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID encounterOwnerId, std::uint32_t serverTick,
			std::uint8_t unavailableSymbols = 0u);

		static bool Can_EnterMarioEntry(const SERVER_PLAYER& player) noexcept;
		// MARIO_ENTER만 성공 결과로 가진 영역 진입 window 또는 PATTERN_COMPLETION_COUNT chain root를 찾는다.
		static bool Has_MarioEntry(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 영역이 있는 첫 Mario 진입 결과를 반환한다. chain root만으로 진입하는 패턴이면 nullptr다.
		static const BOSS_PATTERN_LOGIC_RESULT* Find_MarioEntryResult(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 자식 패턴 실행 중에도 룸이 보관한 root 영역으로 검사한다. honorWindowEnd가 true일 때만 저작 종료 시각에 portal을 닫는다.
		static bool Is_InsideMarioEntry(const BOSS_PATTERN_DEFINITION& root,
			const SERVER_WORLD_ENTITY& anchor, const SERVER_PLAYER& player,
			std::uint32_t elapsedTicks, bool honorWindowEnd = false, std::uint8_t* matchedStage = nullptr);
		// 접촉 판정과 같은 저작 영역 변환과 타원 부채꼴 경계를 사용한다.
		[[nodiscard]] static bool Is_PointInsideRegion(const BOSS_LOGIC_REGION& region,
			const SERVER_WORLD_ENTITY& boss, float worldX, float worldZ) noexcept;
		// 현재 권위 mechanic 구간을 읽어 snapshot의 gauge로 투영한다.
		static void Project_MechanicGauge(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::uint32_t serverTick, LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot);
		static void Build(
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			KOUKUSAYDON_LOGIC_LEDGER& outLedger);
		/* 추가 판정 없이 ledger를 비우고 보스 방어·결과 및 이 발생이 소유한 플레이어 bind를 정리한다. */
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
		/* 플레이어에게 복제한 slot 배치로 DANCE 입력을 해석한다. 열린 window의 첫 응답만 기록한다. */
		static bool Record_InteractionSlot(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_PLAYER& player,
			LostArk::Shared::INTERACTION_SLOT slot,
			std::string& outStatus);
		/* 플레이어 HUD·slot·광기 상한·광대 만료를 갱신한다. dance slot은 패턴 발생별 한 번 섞어 HUD와 판정이 같은 배치를 읽는다. */
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
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const std::array<float, 2u>* pContactCenter = nullptr, bool encounterWipe = true);
		/* 살아 있는 일반 플레이어를 광대로 바꾼다. durationMs가 0이면 encounter 유지 시간, 없으면 기본 시간을 쓴다. */
		static void Transform_ToClown(
			SERVER_PLAYER& player,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::uint32_t durationMs);
		/* 공격 원점 방향이 활성 방패 각도 안이면 true다. 호출자는 원래 피해 대신 반사를 처리한다. */
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
		static void Capture_CardMazeEntryRoster(KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Reveal_CardMazeEntryPlayers(const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static bool Stage_CardMazeEntryPlayers(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static bool Enter_CardMaze(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static void Capture_StartRegions(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window, KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::uint32_t patternElapsedTicks);
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
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const std::set<LostArk::Shared::PLAYER_ID>& invulnerablePlayers,
			const std::array<float, 2u>* pContactCenter = nullptr,
			bool spatialContact = false);
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Roulette(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Gaze(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static bool Is_Judgeable(const SERVER_PLAYER& player) noexcept;
		static bool Can_ReceiveSpatialContact(const SERVER_PLAYER& player) noexcept;
	};

	/* 룸이 소유하는 빙고 상태다. 폭탄·망치 단계와 보드 진행을 값으로 보관한다. */
	/* 25비트 white는 모든 해골 칸, red는 완성된 행·열의 영구 칸이다. 보상 소비 여부는 별도 Server line mask로 관리한다. */
	class CKoukuBingoRuntime final
	{
	public:
		void Reset() noexcept
		{
			m_iWhiteMask = 0u;
			m_iRedMask = 0u;
			m_iConsumedLineMask = 0u;
			m_Bombs = {};
			m_Hammer = {};
		}
		/* 보드 안의 지정 칸을 채우고 완성된 행·열을 승격한다. 보드 밖 비트는 무시한다. */
		void Fill(std::uint32_t cellMask) noexcept;
		/* red 칸은 유지하고 폭발 범위의 나머지 해골 칸을 반전한 뒤 행·열을 승격한다. white에는 red도 포함하며 대각선은 승격하지 않는다. */
		void Detonate(std::uint32_t cellMask) noexcept;
		[[nodiscard]] std::uint32_t Get_WhiteMask() const noexcept { return m_iWhiteMask; }
		[[nodiscard]] std::uint32_t Get_RedMask() const noexcept { return m_iRedMask; }
		[[nodiscard]] std::uint32_t Count_CompletedRowsAndColumns() const noexcept;
		[[nodiscard]] std::uint32_t Count_UnconsumedRowsAndColumns() const noexcept;
		/* 행·열의 고정 순서로 미소비 완성 줄을 count개 소비한다. count가 0이거나 줄이 부족하면 상태를 유지한다. */
		bool Consume_CompletedRowsAndColumns(std::uint32_t count) noexcept;
		/* 해당 위치의 보드 칸이 완성된 red 줄에 속하는지 검사한다. 전멸 공격의 안전 구역 판정에 사용한다. */
		[[nodiscard]] bool Is_Safe(float x, float z) const noexcept;

		/* 폭탄 slot은 운반자 ID와 예정 tick을 보관한다. 실제 플레이어 조회와 낙하 위치 결정은 룸이 담당한다. */
		struct BOMB final
		{
			LostArk::Shared::BINGO_BOMB_PHASE ePhase =
				LostArk::Shared::BINGO_BOMB_PHASE::NONE;
			LostArk::Shared::NET_ENTITY_ID iCarrierNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iDetonateTick = 0u;
			// 머리 표식이 사라진 뒤 지면 생성 전까지 slot을 예약하는 tick이다.
			std::uint32_t iPlantTick = 0u;
			std::uint32_t iMarkOrdinal = 0u;
			// 폭탄마다 시간을 캡처해 뒤의 보드 설정 변경이 대기 중인 낙하·폭발을 바꾸지 못하게 한다.
			std::uint32_t iDropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS;
			std::uint32_t iFuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS;
		};
		using BOMB_SLOTS = std::array<BOMB,
			static_cast<std::size_t>(LostArk::Shared::KOUKU_BINGO_MAX_BOMBS)>;

		/* 운반자 ID가 유효하고 빈 slot이 있으면 표식한다. 같은 운반자의 MARKED가 이미 있으면 거절해 표식 시간을 재시작하지 않는다. */
		bool Start_Bomb(LostArk::Shared::NET_ENTITY_ID carrier,
			std::uint32_t detonateTick, std::uint32_t markOrdinal = 0u,
			std::uint32_t dropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS,
			std::uint32_t fuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS) noexcept;
		/* MARKED 또는 낙하 예약 slot을 지정 위치에 설치하고 fuseTick을 폭발 tick으로 저장한다. */
		void Plant_Bomb(std::size_t slot, float x, float z,
			std::uint32_t fuseTick) noexcept;
		void Queue_BombPlant(std::size_t slot, float x, float z, std::uint32_t plantTick) noexcept;
		void Clear_Bomb(std::size_t slot) noexcept;
		[[nodiscard]] const BOMB_SLOTS& Get_Bombs() const noexcept { return m_Bombs; }

		/* 망치가 없을 때 유효한 anchor와 시간으로 시작한다. anchor 선택은 룸이 맡는다. */
		bool Start_Hammer(std::int32_t anchor, std::uint32_t startTick,
			std::uint32_t endTick) noexcept;
		/* 도달한 종료 tick에 망치를 한 단계 전진시키고 마지막 단계 뒤에는 비운다. 망치가 남아 있으면 true다. */
		bool Advance_Hammer(std::uint32_t tick) noexcept;
		[[nodiscard]] const LostArk::Shared::BINGO_HAMMER_SNAPSHOT&
			Get_Hammer() const noexcept { return m_Hammer; }

	private:
		/* Promotes every line that is now completely white. */
		void Promote_Lines() noexcept;

		std::uint32_t m_iWhiteMask = 0u;
		std::uint32_t m_iRedMask = 0u;
		std::uint32_t m_iConsumedLineMask = 0u;
		BOMB_SLOTS m_Bombs{};
		LostArk::Shared::BINGO_HAMMER_SNAPSHOT m_Hammer{};
	};

	// 미로의 역할·문양·처치 진행과 대기 참가자를 소유한다. entity 생성·피격·제거와 송신은 룸이 담당한다.
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

		static constexpr std::uint8_t KILL_TARGET = 1u;
		static constexpr std::uint32_t TARGETS_PER_SUIT = 1u;
		/* 일반 망치의 피해 판정은 입력 후 30Hz 기준 30tick이다. Hammer_HitTickOffset은 interactionIndex 1에만 12tick을 적용한다. */
		static constexpr std::uint32_t HAMMER_HIT_TICK_OFFSET = 30u;
		static constexpr std::uint32_t Hammer_HitTickOffset(std::uint32_t interactionIndex) noexcept
		{ return 1u == interactionIndex ? 12u : HAMMER_HIT_TICK_OFFSET; }
		static constexpr float HAMMER_RANGE_M = 2.4f;
		/* cos(60 degrees): the swing covers a 120 degree arc in front. */
		static constexpr float HAMMER_HALF_ANGLE_COS = 0.5f;
		/* 일반 망치 입력 피해량이다. 미로 표적의 피격 adapter는 별도로 남은 HP 전부를 소비한다. */
		static constexpr std::uint32_t HAMMER_RAW_DAMAGE = 100u;
		/* MAZE HUD의 망치 skill ID다. 미로 표적은 다른 skill의 타격을 거절한다. */
		static constexpr LostArk::Shared::SKILL_ID HAMMER_SKILL_ID = 56411u;
		static constexpr const char* SPAWN_GROUP_TAG = "cardmaze.targets";
		/* The box a hammer swing must reach to take the telescope. */
		static constexpr const char* TELESCOPE_PLACEMENT_ID = "cardmaze.telescope";
		/* 입장 30tick 뒤 망원경 위에 생성하는 광대 상자다. 별도 group으로 일반 몬스터 피해를 받고 파괴 뒤 망원경을 연다. */
		static constexpr const char* CLOWN_BOX_ARCHETYPE_ID = "MONSTER_KOUKU_CLOWN_BOX";
		static constexpr const char* CLOWN_BOX_SPAWN_GROUP_TAG = "cardmaze.clownbox";
		static constexpr std::uint32_t CLOWN_BOX_SPAWN_DELAY_TICKS = 30u;
		/* CardMiro 영역의 world m 경계다. 0.5m 격자 116×114칸과 망원경 중심 좌표에 대응한다. */
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

		/* 역할·문양·표적 위치를 대기 상태로 준비한다. spawn 성공 뒤 Commit이 플레이어에 적용하고 실패 시 Abort로 버린다. */
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
		/* 진행 중인 사냥에서 문양을 가진 참가자가 모두 나갔으면 true를 반환해 룸의 종료 정리를 요청한다. */
		bool Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		bool Can_Hit(const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Retire_Target(LostArk::Shared::NET_ENTITY_ID targetId);
		bool Sample_Corridor(LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
			std::uint32_t seed, SPAWN_REQUEST& out, bool avoidPersonalExits = false) const;
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
