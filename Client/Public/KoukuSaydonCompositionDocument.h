#pragma once

// Logic 정의가 값으로 소유하는 고정·추적·투사체·volley 피해 템플릿을 제공한다.
#include "Gameplay/AttackHitTemplate.h"

// 네 프로필의 읽기 전용 Action 참조 문서를 함께 보관하는 값 타입을 제공한다.
#include "KoukuSaydonAnimationActionDocument.h"

// XYZ 값, 고정 종류 목록과 네 프로필 참조를 크기가 정해진 배열로 보관한다.
#include <array>
// 저장 revision·ID 순번·ms 시각에 고정 폭 정수를 사용한다.
#include <cstdint>
// 문서 객체의 소유 경로와 경로를 받는 Reload 인터페이스를 선언한다.
#include <filesystem>
// 생략 가능한 대상 재선택·배치·보스 이동 등과 값이 명시된 상태를 구분한다.
#include <optional>
// 문서 ID·리소스 경로·상태 메시지와 원본 JSON 문자열을 소유한다.
#include <string>
// 문자열 검사 입력과 고정 표에서 돌려주는 문자열을 복사 없이 차용한다.
#include <string_view>
// 가변 길이 정의·배치·결과 목록을 각 문서 값 안에서 소유한다.
#include <vector>

namespace Client
{
	// 하나의 단계에 배치한 애니메이션이다. 참조 원본의 identity와 실제 재생 구간을 함께 저장한다.
	// 시작 오프셋과 길이는 ms이며, 재생 속도는 원본 시간에 곱하는 배율이다.
	struct KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strProfileId;
		// Action 0도 유효한 참조다. 물리 클립을 직접 지정할 때는 Action 0과 RAW 단계를 사용한다.
		std::uint32_t iSourceActionId = 0u;
		std::string strSourceStageId;
		std::string strSourceSlotId;
		std::string strReferenceRevision;
		std::string strRuntimeClip;
		std::uint32_t iStartOffsetMs = 0u;
		std::uint32_t iSourceStartMs = 0u;
		// 0은 원본 클립의 끝을 뜻하므로 종료 시점을 저장하지 않던 문서와 호환된다.
		std::uint32_t iSourceEndMs = 0u;
        // 미리보기에서 계산한 포즈 시작 시점이다. 저작 occurrence에는 직렬화하지 않는다.
        std::uint32_t iPoseStartMs = UINT32_MAX;
		std::uint32_t iPlayMs = 0u;
		std::uint32_t iBlendInMs = 0u;
		f32_t fPlayRate = 1.f;
		std::string strEndPolicy;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE&) const = default;
	};

	// 저작 Logic occurrence의 시간 구간에서 파생한 재생용 블렌드 정보다.
	// Source와 Target의 시간 오프셋은 단계 내부 시간이 아니라 패턴 시작 기준 시간이다.
	struct KOUKU_SAYDON_ANIMATION_BLEND_WINDOW final
	{
		std::string strLogicOccurrenceId;
		std::uint32_t iStartMs = 0u, iDurationMs = 0u;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE Source, Target;
		bool operator==(const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW&) const = default;
	};

	// 패턴이 순서대로 실행하는 단계다. 단계 길이와 애니메이션 목록, 진입 시 대상 재선택 정책을 소유한다.
	struct KOUKU_SAYDON_COMPOSITION_STAGE final
	{
		std::string strStageId;
		std::string strActionId;
		std::string strStageKind;
		std::uint32_t iDurationMs = 0u;
		std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE>
			AnimationOccurrences;
		bool bRetargetOnEnter = false;
        // 생략하면 RANDOM_ALIVE 정책을 유지한다. 명시한 값은 저작 문서 저장 시 보존한다.
        std::optional<std::string> RetargetTarget;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_STAGE&) const = default;
	};

	// DURATION의 판정 종류와 RESULT의 결과 종류는 Server가 소비하는 문자열 계약이다.
	// 이름만 만든 정의는 종류를 비워 둘 수 있으며 완성된 실행 정의와 구분해 DRAFT로 편집한다.
	inline constexpr std::array<const char_t*, 20u> KOUKU_SAYDON_JUDGEMENT_KINDS = {
		"CARD_DICE_BIND", "ROULETTE_CARD_MATCH", "GAZE_REAL_BOSS", "POSE_INPUT", "STAGGER_WINDOW", "COUNTER_WINDOW", "AREA_OVERLAP", "OBJECT_OVERLAP", "EXTERNAL_SIGNAL", "ATTACHMENT_HOLD", "PATTERN_COMPLETION_COUNT", "SHOWTIME_PLAYER_TARGETS", "BOSS_TRACK_TARGET", "CROSS_DIRECTION_CLONES", "PURSUIT_PROJECTILES", "BINGO_BOARD", "INVULNERABILITY_ZONE", "BINGO_COMPLETED_LINES", "BOSS_RANDOM_TARGET", "BOSS_DAMAGE_REDUCTION" };
	inline constexpr std::array<const char_t*, 14u> KOUKU_SAYDON_OUTCOME_KINDS = {
		"INSTANT_DEATH", "MAX_HP_PERCENT_DAMAGE", "FIXED_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT",
		"CLOWN_TRANSFORM", "FEAR", "FOLLOWUP_PATTERN", "PLAY_WORLD_OBJECT_MOTION",
		"PLAY_CONTACT_WORLD_OBJECT_MOTION", "COMPLETE_LOGIC_WINDOW", "CAPTURE_PLAYER", "GRAB_TO_WORLD_OBJECT", "MARIO_ENTER", "PLAYER_INVULNERABILITY" };
	inline constexpr std::array<const char_t*, 4u> KOUKU_SAYDON_CARD_SYMBOLS = {
		"HEART", "SPADE", "CLUB", "DIAMOND" };
	inline constexpr std::size_t KOUKU_SAYDON_MAX_OUTCOMES_PER_SLOT = 4u;
	inline constexpr std::uint32_t KOUKU_SAYDON_DANCE_POSE_COUNT = 4u;

	enum class KOUKU_SAYDON_OUTCOME_SLOT : std::uint8_t
	{
		SUCCESS,
		FAIL,
		TIMEOUT,
	};

	// 판정 창이 끝나는 시점에 결과를 정하는 종류를 구분한다. 결과 슬롯별 허용 여부는 아래 함수가 별도로 판단한다.
	inline bool_t Kouku_IsEndTickJudgement(const std::string_view judgementKind)
	{
		return judgementKind == "ROULETTE_CARD_MATCH" ||
			judgementKind == "GAZE_REAL_BOSS" || judgementKind == "AREA_OVERLAP";
	}
	// Workbench의 결과 연결과 문서 검증이 공유하는 허용 규칙이다. 주어진 판정 종류와 결과 슬롯만 읽는다.
	// 구간 중 시선 판정은 FAIL만, 종료 판정이나 접촉 판정은 각 종류가 실제로 낼 수 있는 결과만 연결한다.
	inline bool_t Kouku_IsOutcomeSlotAllowed(
		const std::string_view judgementKind,
		const KOUKU_SAYDON_OUTCOME_SLOT slot, const bool_t gazeDuringWindow = false)
	{
        if (judgementKind == "GAZE_REAL_BOSS" && gazeDuringWindow)
			return slot == KOUKU_SAYDON_OUTCOME_SLOT::FAIL;
		if (judgementKind == "CARD_DICE_BIND" ||
			judgementKind == "ATTACHMENT_HOLD" ||
			judgementKind == "SHOWTIME_PLAYER_TARGETS" ||
			judgementKind == "BOSS_TRACK_TARGET" ||
			judgementKind == "BOSS_RANDOM_TARGET" ||
			judgementKind == "CROSS_DIRECTION_CLONES" ||
			judgementKind == "PURSUIT_PROJECTILES" ||
			judgementKind == "BINGO_BOARD" ||
			judgementKind == "INVULNERABILITY_ZONE" ||
			judgementKind == "BOSS_DAMAGE_REDUCTION")
			return false;
		if (judgementKind == "PATTERN_COMPLETION_COUNT")
			return slot == KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS;
		if (KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT == slot)
			return judgementKind != "GAZE_REAL_BOSS" && judgementKind != "OBJECT_CONTACT";
		if (KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot)
			return judgementKind != "STAGGER_WINDOW" && judgementKind != "COUNTER_WINDOW" && judgementKind != "ENTER_AREA" &&
				judgementKind != "OBJECT_CONTACT" && judgementKind != "EXTERNAL_SIGNAL";
		return true;
	}

	// 접촉한 World occurrence에 어떤 motion을 실행할지 연결한다. 실제 대상은 stable ID로 다시 찾는다.
	struct KOUKU_SAYDON_CONTACT_MOTION final
	{
		std::string strTargetWorldOccurrenceId;
		std::string strMotionInstanceId;
		bool operator==(const KOUKU_SAYDON_CONTACT_MOTION&) const = default;
	};

	// 문서가 소유하는 재사용 Logic 정의다. ID, 이름과 타입을 먼저 만들고 종류별 값을 나중에 채울 수 있다.
	// 배치 시간과 결과 연결은 occurrence가 소유한다. 이 정의를 수정하면 해당 ID를 참조하는 배치들이 공유한다.
	struct KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION final
	{
		std::string strLogicId;
		std::string strDisplayName;
		std::string strLogicType;
		// DURATION 정의의 값이다. strJudgementKind에 해당하는 필드만 해석한다.
		std::string strJudgementKind;
        // BINGO_BOARD 정책은 occurrence들이 공유한다. WINDOW 모드는 각 배치 상자의 시간 구간을 사용한다.
        std::string strBingoActiveMode = "ENCOUNTER";
        std::uint32_t iBingoFirstBombDelayMs = 30000u;
        std::uint32_t iBingoBombIntervalMs = 20000u;
        std::uint32_t iBingoBombMarkMs = 6000u;
        std::uint32_t iBingoBombDropDelayMs = 2000u;
        std::uint32_t iBingoBombFuseMs = 4000u;
        std::uint32_t iBingoInitialMarkedCells = 2u;
		std::string strFixedSelectionGroupId;
		std::string strTrackingPresentationOccurrenceId;
		std::uint32_t iSpawnIntervalMs = 0u;
		double fFollowSpeedScale = 0.0;
        // 선택적인 회전 폭탄 시험에서 참조하는 배치 ID다. 해당 배치는 계속 편집 가능한 템플릿으로 남는다.
        std::string strBombPresentationOccurrenceId;
        std::string strBombExplosionPresentationOccurrenceId;
        std::string strBombSectorPresentationOccurrenceId;
        double fBombSectorRadiusM = 0.0;
        double fBombSectorHalfAngleDegrees = 0.0;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> FixedHits, TrackingHits, ProjectileHits;
		std::vector<std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>> RandomVolleyHits;
        // 이펙트 리소스 ID다. 투사체의 이동, 접촉과 수명은 Server가 결정한다.
        std::vector<std::string> PursuitVisualIds;
        std::vector<std::string> PursuitCardSymbols; // 선택적인 표시 슬롯의 카드 문양이다. 문양이 맞는 플레이어는 해당 카드 피해에서 제외된다.
        std::string strContactVisualId;
        double fPursuitMaxDistanceM = 0.0; // 0이면 별도 거리 제한 없이 수명이나 접촉으로 이동을 끝낸다.
        double fPursuitSpeedMps = 0.0, fContactRadiusM = 0.0, fSpawnRadiusM = 0.0;
        std::uint32_t iPursuitLifetimeMs = 0u, iCountPerWave = 0u;
        bool_t bPursuitHoming = false;
		// 완전한 이펙트 묶음을 순서대로 저장한다. Server는 아레나 생성 간격마다 한 묶음씩 순환한다.
		std::vector<std::vector<std::string>> RandomVolleyOccurrenceSets;
		std::uint32_t iRandomSpawnIntervalMs = 0u;
		double fRandomArenaRadiusM = 0.0;
		double fRandomArenaHeightToleranceM = 0.0;
		std::string strRandomAnchorKind = "BOSS_SPAWN";
		double fRandomScaleMin = 1.0, fRandomScaleMax = 1.0;
		// 앞, 뒤, 왼쪽, 오른쪽 순서의 패턴 후보다. 실제 방향 선택은 Server가 한다.
		std::vector<std::string> DirectionPatternIds;
		std::string strCloneEndStageId;
		std::string strSummonOccurrenceId;
		std::vector<std::string> PatternIds;
		std::uint32_t iCompletionCount = 0u;
		std::uint32_t iSectorCount = 0u;
		std::vector<std::string> SectorSymbols;
		std::vector<std::string> RegionIds;
		double fCenterX = 0.0;
		double fCenterZ = 0.0;
		double fOuterRadiusM = 0.0;
		std::string strWorldSequenceInstanceId;
		double fHalfAngleDegrees = 0.0;
		double fMaxDistanceM = 0.0;
        bool_t bGazeDuringWindow = false;
		std::uint32_t iPoseIndex = 0u;
		std::uint32_t iThreshold = 0u;
		double fShieldArcDegrees = 0.0;
		bool_t bEndsPatternOnSuccess = false;
		double fNormalYawOffsetDegrees = 0.0;
		std::string strInsideOutcome = "SUCCESS";
		// RESULT 정의의 값이다. strOutcomeKind에 따라 피해, 상태 변화 또는 후속 동작에 사용한다.
		std::string strOutcomeKind;
		std::uint32_t iPercent = 0u;
		std::uint32_t iDamageAmount = 0u; // FIXED_DAMAGE의 HP 값이다. 최대 HP에 대한 비율과 구분한다.
		std::uint32_t iDurationMs = 0u;
		double fPushRangeM = 0.0;
		double fPushHeightM = 0.0;
		std::string strPushDirection = "AWAY_FROM_BOSS";
		std::uint32_t iPushMs = 0u;
		bool_t bForcePush = false;
		bool_t bPushCanLeaveArena = false;
		bool_t bPushBallistic = false;
		double fPushYawOffsetDegrees = 0.0;
		std::string strFollowupPatternId;
		std::uint32_t iMarioEntryStage = 0u; // MARIO_ENTER에서만 사용한다. 0이면 현재 방의 진행 횟수를 따른다.
		std::string strSceneProfileId;
		std::string strEffectResourceId;
		std::string strLightResourceId;
		std::string strSoundResourceId; // FEAR에서 선택적으로 재생하는 SOUND다. iEffectDelayMs 이후 시작한다.
		std::uint32_t iEffectDelayMs = 0u;
		std::string strAttachmentSlot;
		// 소유자의 전방, 위쪽, 오른쪽 기준으로 기록한 m 단위 오프셋이다.
		std::array<double, 3u> GripLocalOffset{};
		std::string strTargetWorldInstanceId;
		std::string strMotionInstanceId;
		double fTargetRadiusM = 0.0;
		// CARD_RAIN_SOLDIERS의 기본값은 클럽, 하트, 다이아몬드 병사 생성 수를 유지한다.
		std::array<std::uint32_t, 3u> SoldierCounts{1u, 1u, 1u};
		// 0이면 몬스터 프로필 값을 사용한다. 카드 비 병사에만 별도 값을 적용한다.
		std::uint32_t iSoldierMaxHp = 0u, iSoldierDamage = 0u;
		double fSpawnRadiusMinM = 3.0, fSpawnRadiusMaxM = 6.0;
		// TRIGGER 정의의 값이다. 게시 단계에서 Server의 기믹 cue로 변환한다.
		std::string strTriggerKind;
		// Collider의 피해와 넉백 접촉 기본값을 구분하는 저작용 역할이다.
		std::string strColliderDamageContactRole;
		// 같은 패턴의 MAP 이펙트 ID를 순서대로 저장한다. 게시 단계에서 해당 배치의 위치를 해석한다.
		std::vector<std::string> PlayerEntryEffectOccurrenceIds;
		std::uint32_t iCountPerPlayer = 0u;
		double fPlayerEffectRadiusM = 0.0;
		std::uint32_t iEffectLifetimeMs = 0u;
		std::uint32_t iArenaRandomCount = 0u;
		double fArenaRandomRadiusM = 0.0;
		double fArenaHeightToleranceM = 0.0;
		double fArenaMinimumSpacingM = 0.0;
		bool_t bRandomPlayerOnly = false;
		bool_t bRearmOnExit = false;
		bool_t bRepeatAfterKnockback = false;
		std::uint32_t iRepeatIntervalMs = 0u;
		double fBossChargeDistanceM = 0.0;
        double fChargeYawOffsetDegrees = 0.0;
		std::string strHudMode;
		std::array<double, 3> TeleportPosition{};
		std::string strAirbornePhase;
		std::string strAirborneTargetPositionPolicy = "APPEAR";
		std::string strSelectedEffectGroupId;
        std::uint32_t iSelectedFlightMs = 0u;
        double fSelectedFlightArcHeightM = 0.0;
        std::array<double, 3u> SelectedFlightSourceOffset{0.0, 0.0, 0.0};
		double fAirborneHeightM = 0.0;
		std::uint32_t iAirborneDurationMs = 0u;
		std::string strClonePatternId;
		std::vector<std::uint32_t> ClockHours;
		double fFaceCenterYawOffsetDegrees = 0.0;
		std::vector<std::string> TargetWorldOccurrenceIds;
		std::string strContactGroupId;
		std::uint32_t iContactPriority = 0u;
		std::vector<KOUKU_SAYDON_CONTACT_MOTION> ContactMotions;
		std::string strTargetLogicOccurrenceId;
		std::string strContactTargetWorldOccurrenceId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION&) const = default;
	};

	// Workbench와 Validate_Shape가 결과 연결을 소유할 수 있는 Logic 종류인지 확인한다.
	// 결과 슬롯이 없는 지속 정책은 제외하고 구간 판정 및 접촉 trigger의 연결만 허용한다.
	inline bool_t Kouku_LogicOwnsOutcomes(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return (logic.strLogicType == "DURATION" &&
			logic.strJudgementKind != "CARD_DICE_BIND" &&
			logic.strJudgementKind != "ATTACHMENT_HOLD" &&
			logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS" &&
			logic.strJudgementKind != "BOSS_TRACK_TARGET" &&
			logic.strJudgementKind != "BOSS_RANDOM_TARGET" &&
			logic.strJudgementKind != "CROSS_DIRECTION_CLONES" &&
			logic.strJudgementKind != "PURSUIT_PROJECTILES" &&
			logic.strJudgementKind != "BINGO_BOARD" &&
			logic.strJudgementKind != "INVULNERABILITY_ZONE" &&
			logic.strJudgementKind != "BOSS_DAMAGE_REDUCTION") ||
			(logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" ||
				logic.strTriggerKind == "OBJECT_CONTACT"));
	}
	// Workbench의 콜라이더 연결 UI와 문서 검증이 같은 규칙을 사용한다.
	// 자체 정책으로 동작하는 종류는 제외하며, 결과 연결 허용 여부와는 서로 다른 조건이다.
	inline bool_t Kouku_LogicAcceptsColliders(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return (logic.strLogicType == "DURATION" &&
			logic.strJudgementKind != "CARD_DICE_BIND" &&
			logic.strJudgementKind != "PATTERN_COMPLETION_COUNT" &&
			logic.strJudgementKind != "EXTERNAL_SIGNAL" &&
			logic.strJudgementKind != "COUNTER_WINDOW" &&
			logic.strJudgementKind != "ATTACHMENT_HOLD" &&
			logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS" &&
			logic.strJudgementKind != "BOSS_TRACK_TARGET" &&
			logic.strJudgementKind != "BOSS_RANDOM_TARGET" &&
			logic.strJudgementKind != "CROSS_DIRECTION_CLONES" &&
			logic.strJudgementKind != "PURSUIT_PROJECTILES" &&
			logic.strJudgementKind != "BINGO_BOARD") ||
			(logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" ||
				logic.strTriggerKind == "OBJECT_CONTACT"));
	}
	// 결과 연결 검사가 사용할 종류 문자열을 입력 정의에서 차용한다. 입력 Logic보다 오래 보관하지 않는다.
	inline const std::string& Kouku_LogicOutcomeKind(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
	{
		return logic.strLogicType == "TRIGGER" ? logic.strTriggerKind : logic.strJudgementKind;
	}

	// 시퀀스의 플레이어 슬롯과 도착 월드 위치를 기록한다. 클라이언트에서 즉시 플레이어를 이동시키는 명령은 아니다.
	struct KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL final
	{
		std::uint32_t iPlayerSlot = 0u;
		std::array<double, 3> Position{};
		bool operator==(const KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL&) const = default;
	};

	// 타임라인에 배치한 Logic 하나다. 시간 구간은 패턴 시작 기준이므로 여러 단계와 반복 클립에 걸칠 수 있다.
	struct KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strLogicId;
		bool_t bEnabled = true;
		// 확장된 실행 문서에서 종료 시 취소가 필요한지 계산한 값이다. 저작 문서에는 저장하지 않는다.
		bool_t bCancelAtEnd = false;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		// 배치가 결과 연결을 소유한다. 각 슬롯의 RESULT Logic ID는 최대 네 개이며 목록 순서로 적용한다.
		// 같은 정의를 재사용해도 성공, 실패, 시간 만료의 연결은 배치마다 다를 수 있다.
		std::vector<std::string> OnSuccessLogicIds;
		std::vector<std::string> OnFailLogicIds;
		std::vector<std::string> OnTimeoutLogicIds;
		// 시퀀스에서 요청하는 플레이어 배치다. 실제 적용은 현재 Server 방의 승인을 거친다.
		std::optional<KOUKU_SAYDON_ROOM_PLAYER_ARRIVAL> RoomPlayerArrival;
		// ENTER_AREA 포획이 유지될 기한을 같은 패턴의 ATTACHMENT_HOLD occurrence로 연결한다.
		std::string strHoldLogicOccurrenceId;

		// 편집기가 슬롯에 해당하는 ID 목록을 직접 수정하도록 차용한다. occurrence의 수명과 컨테이너 변경에 유의한다.
		std::vector<std::string>& Outcomes(const KOUKU_SAYDON_OUTCOME_SLOT slot)
		{
			return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? OnSuccessLogicIds :
				(KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? OnFailLogicIds : OnTimeoutLogicIds);
		}
		// 검증과 표시에서 같은 슬롯 선택 규칙으로 읽기 전용 목록을 차용한다.
		const std::vector<std::string>& Outcomes(const KOUKU_SAYDON_OUTCOME_SLOT slot) const
		{
			return KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS == slot ? OnSuccessLogicIds :
				(KOUKU_SAYDON_OUTCOME_SLOT::FAIL == slot ? OnFailLogicIds : OnTimeoutLogicIds);
		}

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE&) const = default;
	};

	// 재사용할 소환 정책이다. 실제 생성 시각과 수명은 각 occurrence가 소유한다.
	struct KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION final
	{
		std::string strSummonId;
		std::string strDisplayName;
		std::string strSummonKind;
		std::vector<std::string> DirectionPatternIds;
		std::string strCloneEndStageId;
		std::string strRealPatternId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION&) const = default;
	};

    // 해석을 끝낸 읽기 전용 실행 구간이다. 숨겨진 Logic 정의로 저작 문서에 저장하지 않는다.
    struct KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW final
    {
        std::string strOccurrenceId;
        std::uint32_t iStartMs = 0u;
        std::uint32_t iDurationMs = 0u;
        std::vector<std::string> DirectionPatternIds;
        std::string strCloneEndStageId;
        std::string strRealPatternId;
    };

    struct KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN final
    {
        std::string strSpawnId;
        std::string strPatternId;
        // BOSS는 소유자 기준 오프셋과 회전, MAP은 절대 월드 위치와 회전이다.
        std::string strAnchorKind = "BOSS";
        std::array<double, 3u> PositionOffset{};
        double fYawOffsetDegrees = 0.0;
        bool operator==(const KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN&) const = default;
    };

	// 소환 배치 하나다. iStartMs는 패턴 시작 기준 생성 시각이고 iDurationMs는 생성 이후 수명이다.
	struct KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strSummonId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
        // 비어 있으면 이름만 지정했던 기존 Summon 동작을 유지한다.
        std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_PATTERN_SPAWN> PatternSpawns;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE&) const = default;
	};

	// Area에서 저작한 월드 시퀀스 인스턴스를 참조한다. 배치는 패턴 시계와 자신의 재생 속도로 이를 실행한다.
	struct KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION final
	{
		std::string strWorldId;
		std::string strDisplayName;
		std::string strSequenceInstanceId;
		// 선택적인 부모 Object다. sequenceInstanceId는 배치를 추가할 때 선택한 초기 상태를 지정한다.
		std::string strObjectResourceId;
		std::array<double, 3u> PositionOffset{};
		std::string strAnchorKind = "NONE";
		std::array<double, 3u> AnchorPosition{};
		std::string strCompanionEffectResourceId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION&) const = default;
	};

	// 복제한 Object occurrence의 절대 월드 배치다. 저장된 motion 자체의 좌표는 계속 로컬 기준이다.
	struct KOUKU_SAYDON_WORLD_PLACEMENT final
	{
		std::array<double, 3u> Position{};
		std::array<double, 3u> RotationDegrees{};
		std::array<double, 3u> Scale{ 1.0, 1.0, 1.0 };
		bool operator==(const KOUKU_SAYDON_WORLD_PLACEMENT&) const = default;
	};

	// World 정의를 타임라인에 배치한다. 생성 시점, 수명, 재생 속도와 선택적인 월드 배치 변환을 소유한다.
	struct KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strWorldId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		f32_t fPlaybackSpeed = 1.f;

		std::optional<KOUKU_SAYDON_WORLD_PLACEMENT> Placement;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE&) const = default;
	};

	// 렌더링 프로필 참조를 재사용한다. 각 배치가 패턴 시계에서 적용 구간과 블렌드 시간을 정한다.
	struct KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION final
	{
		std::string strSceneProfileId;
		std::string strDisplayName;
		std::string strRenderingProfileId;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION&) const = default;
	};

	// 공유 렌더링 프로필을 적용할 시간 구간이다. 적용 길이와 블렌드 길이는 ms이며 원본 프로필 값은 이 구조에 복제하지 않는다.
	struct KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strSceneProfileId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iBlendMs = 0u;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE&) const = default;
	};

	enum class KOUKU_SAYDON_PRESENTATION_KIND : std::uint8_t
	{
		EFFECT, SOUND, CAMERA, COLLIDER, LIGHT, WORLD, SCENE_PROFILE, SUBTITLE
	};

	// 이펙트, 사운드, 카메라, 콜라이더, 조명 등은 stable resource ID로 정의를 참조한다.
	// WORLD와 SCENE_PROFILE은 별도 정의 계열을 유지하며 여기의 종류 값은 단발성 리소스 미리보기에도 사용한다.
	struct KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE final
	{
		std::string strResourceId;
		std::string strDisplayName;
		KOUKU_SAYDON_PRESENTATION_KIND eKind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
		std::string strAssetId;
		// 선택적인 KoukuSaydon 사운드 카탈로그 이벤트다. 허용된 각 변형은 하나의 완전한 원본 이벤트를 담는다.
		std::string strSoundEvent;
		std::string strSubtitleText;
		std::string strSubtitlePosition = "NORMAL";
		std::string strResourceKind = "GROUP";
		std::string strElementId;
		std::string strDefaultAnchorKind = "BOSS";
		std::uint32_t iDurationMs = 1000u;
		std::string strShape = "BOX";
		std::string strColliderKind = "GEOMETRY";
		std::array<double, 3u> HalfExtents{ 1.0, 1.0, 1.0 };
		double fRadiusM = 3.0, fInnerRadiusM = 0.0;
		double fHalfAngleDegrees = 45.0;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE&) const = default;
	};

	// 타임라인 배치마다 적용할 값이다. 배치 조정은 공유 이펙트 그룹의 자식이나 원본 leaf, 카메라 shot, 음원 파일을 고치지 않는다.
	struct KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strResourceId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 1000u;
		std::array<double, 3u> PositionOffset{};
		std::array<double, 3u> RotationDegrees{};
		std::array<double, 3u> Scale{ 1.0, 1.0, 1.0 };
		// LINEAR는 이 배치의 수명 동안 콜라이더 위치와 크기를 보간한다.
		std::string strColliderMotion = "STATIC";
        std::string strAnchorPresentationOccurrenceId;
		std::array<double, 3u> ColliderEndPositionOffset{};
		std::array<double, 3u> ColliderEndScale{ 1.0, 1.0, 1.0 };
		std::uint32_t iFadeInMs = 0u;
		std::uint32_t iFadeOutMs = 0u;
		double fDissolveStart = 0.85;
		double fDissolveEnd = 1.0;
		double fVolume = 1.0;
		std::uint32_t iSoundSourceStartMs = 0u;
		// 공유 이펙트 에셋을 수정하지 않고 원본 재생 시작 시점만 건너뛴다.
		std::uint32_t iEffectSourceStartMs = 0u;
        // 선택적인 시간 변환 키다. 각 쌍은 배치 내부 경과 ms와 공유 V1/V2 원본의 절대 ms를 담는다.
        // MAP 이펙트에만 허용한다. 원본 에셋과 원본 입자 이력 자체는 수정하지 않는다.
        std::vector<std::array<double, 2u>> EffectSourceTimeKeys;
        // 재생부가 검증된 시간 키를 보간해 원본 시각 ms를 구한다. 키가 없으면 시작 오프셋을 더한다.
        double Effect_SourceTimeMs(double elapsedMs) const noexcept;
        // 원본 시각을 배치 경과 ms로 되돌린다. 구간 밖의 capture 경계를 보존하기 위해 끝 구간으로 외삽한다.
        double Effect_ElapsedTimeMs(double sourceMs) const noexcept;
        // 키 개수, 양 축의 엄격한 증가, 수명과 MAP 전용 조건을 검사한다. 시간 변환 호출 전에 만족해야 한다.
        bool Has_ValidEffectSourceTimeKeys(std::uint32_t resourceDurationMs) const noexcept;
		double fBrightnessMultiplier = 1.0;
		bool_t bFollowBoss = true;
		// 반복 횟수를 늘리지 않고 V1 원본 시계를 이 배치의 재생 구간에 맞춘다.
		bool_t bFitEffectToDuration = false;
		// 이 배치가 살아 있는 동안 원본의 loop-zero emitter를 원래 속도로 계속 실행한다.
		bool_t bLoopEffectToDuration = false;
		// 활성화된 마리오 2단계 trigger에서 파생한 제품용 표시 대상 정보다.
		bool_t bSuppressLocalMario = false;
		bool_t bDebugRender = true;
		std::string strBone;
		std::string strBoneTarget = "BODY";
		// TARGET_YAW는 보스의 방향 기준을 유지하고 본에서는 위치만 가져온다.
		// BONE은 본의 회전도 사용하므로 이펙트가 해당 본을 따라 회전한다.
		std::string strBoneRotation = "TARGET_YAW";
		// 재사용 도형과 별개로 각 영역 배치가 자신의 카드 정보를 소유한다.
		std::string strRegionId;
		std::string strCardSymbol = "NONE";
		std::string strCardColor = "NONE";
		std::string strAnchorKind = "BOSS";
		std::string strWorldId;
		std::string strLogicOccurrenceId;
		std::string strWorldOccurrenceId;
        // 선택적인 WORLD 이펙트 루트다. 지정한 본 변환 다음, Object 배치 변환 전에 적용한다.
        std::string strWorldEffectTrackId;
		// 이 World 배치의 저작 emission 목록에서 따라갈 행이다. 단일 emitter는 0을 사용한다.
		std::uint32_t iWorldEmissionIndex = 0u;
		// 패턴 내부 이펙트 또는 BOSS 콜라이더의 선택 그룹 정보다. 런타임 변환은 계속 각 occurrence가 소유한다.
		std::string strSelectionGroupId;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE&) const = default;
	};

	// 모든 플레이어에 적용할 광기 최댓값과 광대 유지 시간이다. projector가 실행 제품 데이터에 복사한다.
	struct KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY final
	{
		std::uint32_t iMaximum = 100u;
		std::uint32_t iClownHoldMs = 15000u;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY&) const = default;
	};

	// DURATION은 구간 판정, TRIGGER는 지정한 시점의 기믹 동작, RESULT는 연결된 판정 결과를 표현한다.
	inline constexpr std::array<const char_t*, 3u> KOUKU_SAYDON_LOGIC_TYPES = {
		"DURATION", "TRIGGER", "RESULT" };

	// 패턴 시작 기준 시각 ms와 절대 월드 위치 표본이다. 정렬과 중복 시간 검증 뒤 보간에 사용한다.
	struct KOUKU_SAYDON_BOSS_MOTION_KEY final
	{
		std::uint32_t iTimeMs = 0u;
		std::array<double, 3u> Position{};
		bool operator==(const KOUKU_SAYDON_BOSS_MOTION_KEY&) const = default;
	};

	// 보스의 절대 월드 기준 위치와 방향이다. 애니메이션 자체의 수직 포즈와 구분한다.
	struct KOUKU_SAYDON_BOSS_MOTION final
	{
		std::uint32_t iStartMs = 0u;
		std::uint32_t iEndMs = 1u;
		std::array<double, 3u> StartPosition{};
		std::array<double, 3u> EndPosition{};
		double fYawDegrees = 0.0;
		// 키가 없으면 시작점과 끝점 사이의 직선 이동을 사용한다. 키를 쓰면 절대 위치 표본을 2~512개 저장한다.
		std::vector<KOUKU_SAYDON_BOSS_MOTION_KEY> Keys;
		bool operator==(const KOUKU_SAYDON_BOSS_MOTION&) const = default;
	};

	// 부모 패턴에 배치한 자식 패턴 참조다. stable occurrence ID로 반복 배치들을 서로 구분한다.
	struct KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strPatternId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 1000u;
		bool_t bRepeat = false;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE&) const = default;
	};

	// 편집할 패턴 한 개의 단계와 여러 타임라인 목록을 값으로 소유한다.
	// ID는 저장 연결에 사용하고 vector 위치는 편집 중 바뀔 수 있으므로 영구 참조로 저장하지 않는다.
	struct KOUKU_SAYDON_COMPOSITION_PATTERN final
	{
		std::string strPatternId;
		std::string strActorProfileId;
		std::string strGateId = "GATE1";
		std::string strTargetBossPlacementId;
		// 패턴 자체의 폴더 위치다. 번들 소속은 별도의 Members가 관리한다.
		std::string strFolderId;
		std::string strDisplayName;
		std::string strAuthoringStatus;
		std::string strCategory;
		std::uint32_t iNextStageOrdinal = 1u;
		std::uint32_t iNextAnimationOrdinal = 1u;
		std::uint32_t iNextLogicOccurrenceOrdinal = 1u;
		std::uint32_t iNextSummonOccurrenceOrdinal = 1u;
		std::uint32_t iNextWorldOccurrenceOrdinal = 1u;
		std::uint32_t iNextSceneProfileOccurrenceOrdinal = 1u;
		std::uint32_t iNextPresentationOccurrenceOrdinal = 1u;
		std::uint32_t iNextPatternOccurrenceOrdinal = 1u;
		// 0이면 기존 문서처럼 단계 길이의 합을 패턴 길이로 사용한다.
		std::uint32_t iDurationMs = 0u;
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN_OCCURRENCE> PatternOccurrences;
		// Server가 실행하는 자식 시퀀스다. 앞부분을 한 번 실행한 뒤 지정한 자식부터 끝까지 반복한다.
		std::string strLoopStartPatternOccurrenceId;
		bool_t bPlayChildrenSequentially = false;
		// 입장 시퀀스용이다. 재생이 끝나면 F1과 같은 Server 관문 입장을 요청한다.
		bool_t bEnterCombatOnFinish = false;
		bool_t bResetBossToSpawn = false;
		std::optional<double> ResetBossYawDegrees;
		std::optional<KOUKU_SAYDON_BOSS_MOTION> BossMotion;
		double fAnimationRootVerticalScale = 1.0;
		double fAnimationRootHorizontalScale = 1.0;
		std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> Stages;
		// 실행 스냅샷에서만 사용한다. Try_ResolveAnimationBlendWindows가 Logic 정의와 배치에서 계산하며 Serialize는 저장하지 않는다.
		std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> AnimationBlendWindows;
		std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE> LogicOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_OCCURRENCE> SummonOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> WorldOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> PresentationOccurrences;
		// 잘못된 패턴도 편집 목록에서 확인할 수 있고 무관한 저장 때문에 사라지지 않게 한다.
		// 편집기의 오류 상태와 보존한 JSON이다. Serialize는 오류 행의 보존 JSON을 다시 기록한다.
		std::string strLoadError;
		std::string strPreservedJson;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_PATTERN&) const = default;
	};

	// 시간 구간 밖에서는 시작점이나 끝점을 유지한다. false이면 출력 위치와 방향을 변경하지 않는다.
	[[nodiscard]] bool Sample_KoukuSaydonBossMotion(
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, double patternTimeMs,
		std::array<double, 3u>& outPosition, double& outYawDegrees) noexcept;

	// 관문 안의 편집 폴더다. 목록 정리와 선택적인 실행 타임라인 참조를 보관한다.
	struct KOUKU_SAYDON_COMPOSITION_FOLDER final
	{
		std::string strFolderId;
		std::string strGateId;
		std::string strDisplayName;
		// 선택적인 실행 타임라인이다. 같은 폴더 안에 있는 일반 패턴 ID를 참조한다.
		std::string strTimelinePatternId;
		std::string strLoadError;
		std::string strPreservedJson;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_FOLDER&) const = default;
	};

	// 번들에 포함한 패턴과 시작 오프셋 ms를 연결한다. 같은 패턴의 여러 배치는 member ID로 구분한다.
	struct KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER final
	{
		std::string strMemberId;
		std::string strPatternId;
		std::uint32_t iStartOffsetMs = 0u;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER&) const = default;
	};

	// 여러 패턴 배치를 묶고 번들 자체의 장면 프로필과 표현 배치를 보관한다.
	// 오류 행의 원본 JSON도 보존하여 다른 정상 행의 편집과 저장을 계속할 수 있게 한다.
	struct KOUKU_SAYDON_COMPOSITION_BUNDLE final
	{
		std::string strBundleId;
		std::string strGateId;
		std::string strFolderId;
		std::string strDisplayName;
		std::string strAuthoringStatus = "DRAFT";
		std::uint32_t iNextMemberOrdinal = 1u;
		std::uint32_t iNextSceneProfileOccurrenceOrdinal = 1u;
		std::uint32_t iNextPresentationOccurrenceOrdinal = 1u;
		std::vector<KOUKU_SAYDON_COMPOSITION_BUNDLE_MEMBER> Members;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> PresentationOccurrences;
		std::string strLoadError;
		std::string strPreservedJson;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_BUNDLE&) const = default;
	};

	// 관문 재생 순서의 한 항목이다. 종류와 대상 ID, 실행 뒤 대기 시간 ms를 저장한다.
	struct KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY final
	{
		std::string strEntryId;
		std::string strKind;
		std::string strTargetId;
		std::uint32_t iWaitAfterMs = 0u;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY&) const = default;
	};

	// 시작과 끝 entry ID로 반복 구간을 지정한다. 선택적인 체력줄 조건과 전환 경계를 함께 저장한다.
	struct KOUKU_SAYDON_COMPOSITION_FLOW_GROUP final
	{
		std::string strGroupId, strDisplayName, strStartEntryId, strEndEntryId;
		std::optional<std::uint32_t> RepeatUntilHealthBars;
		bool bTransitionAtGroupEnd = false;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_FLOW_GROUP&) const = default;
	};

	// 관문별 패턴과 번들의 실행 순서 및 반복 구간을 소유한다. 대상 정의는 문서 안의 ID로 연결한다.
	struct KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW final
	{
		std::string strFlowId;
		std::string strGateId;
		std::string strDisplayName;
		std::string strLoopStartEntryId;
		std::string strBingoSpecialPatternId;
		std::vector<KOUKU_SAYDON_COMPOSITION_FLOW_ENTRY> Entries;
		std::vector<KOUKU_SAYDON_COMPOSITION_FLOW_GROUP> EntryGroups;
		bool operator==(const KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW&) const = default;
	};

	// 저작 JSON의 메모리 표현이다. 재사용 정의 목록과 패턴, 폴더, 번들, 관문 실행 순서를 함께 보관한다.
	// next...Ordinal은 ID 발급을 위한 증가값이며 현재 vector 크기나 배열 인덱스와 같은 뜻이 아니다.
	struct KOUKU_SAYDON_COMPOSITION_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 3u;
		std::uint32_t iRevision = 1u;
		std::string strCompositionId;
		std::string strEncounterId;
		std::string strBossArchetypeId;
		std::string strBossPlacementId;
		std::string strAreaId;
		std::uint32_t iFixedTickHz = 30u;
		std::uint32_t iNextPatternOrdinal = 1u;
		std::uint32_t iNextFolderOrdinal = 1u;
		std::uint32_t iNextBundleOrdinal = 1u;
		std::uint32_t iNextLogicOrdinal = 1u;
		std::uint32_t iNextSummonOrdinal = 1u;
		std::uint32_t iNextWorldOrdinal = 1u;
		std::uint32_t iNextSceneProfileOrdinal = 1u;
		std::uint32_t iNextPresentationResourceOrdinal = 1u;
		KOUKU_SAYDON_COMPOSITION_MADNESS_POLICY MadnessPolicy;
		std::vector<std::string> PlayAllPatternIds;
		std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION> Logics;
		std::vector<KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION> Summons;
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION> Worlds;
		std::vector<KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION> SceneProfiles;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> PresentationResources;
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN> Patterns;
		std::vector<KOUKU_SAYDON_COMPOSITION_FOLDER> Folders;
		std::vector<KOUKU_SAYDON_COMPOSITION_BUNDLE> Bundles;
		// 관문별 재생 순서는 이미 저장된 패턴과 번들의 ID를 참조한다.
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN_FLOW> PatternFlows;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_DOCUMENT&) const = default;
	};

	// 네 프로필의 선택적 애니메이션 참조와 읽은 원본 bytes를 함께 보관한다.
	// 참조 실패는 해당 프로필만 빈 값으로 남기며 조합 문서의 저장 권한과 분리한다.
	struct KOUKU_SAYDON_ACTION_REFERENCE_SET final
	{
		std::array<KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT, 4u>
			Documents;
		std::array<std::string, 4u> SourceBytes;

		bool operator==(const KOUKU_SAYDON_ACTION_REFERENCE_SET&) const = default;
	};

	// 조합 문서의 읽기, 검증과 원자적 저장을 담당한다. Workbench의 미저장 draft는 별도 소유다.
	// 애니메이션 참조는 리소스 탐색을 돕는 선택 입력이며 참조 로드 실패가 패턴 목록과 조합 저장을 막지는 않는다.
	class CKoukuSaydonCompositionDocument final
	{
	public:
		CKoukuSaydonCompositionDocument() = default;
		explicit CKoukuSaydonCompositionDocument(std::filesystem::path path);

		// 재생부가 배치 경과 시간과 속도를 원본 클립 시각 ms로 바꿀 때 사용한다.
		// 반복이면 구간 안에서 순환하고 아니면 끝에 고정한다. 유효하지 않은 입력은 false이며 출력은 보존한다.
		static bool_t Try_SampleAnimationSourceMs(std::uint32_t sourceStartMs,
            std::uint32_t sourceEndMs, double elapsedMs, double playRate,
            double nativeDurationMs, bool_t loop, double& outSourceMs);
        // 타임라인 편집이 애니메이션 상자의 앞 또는 뒤 경계를 deltaMs만큼 조정한다.
        // 원본 클립 범위와 단계 길이에 맞춰 임시 복사본을 검증한 뒤 적용하며 실패하면 occurrence를 보존한다.
        static bool_t Trim_AnimationWindow(
            KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
            std::int64_t deltaMs, bool_t front, std::uint32_t stageDurationMs,
            std::uint32_t nativeDurationMs);
		// 기본 패턴 저작 JSON 경로를 프로젝트 Data 루트에서 구한다. 이 함수 자체는 파일을 읽거나 게시하지 않는다.
		static std::filesystem::path Resolve_Path();
		// 별도 시퀀스 저작 문서 경로다. 제품 패턴 publisher의 입력 정본과 구분한다.
		static std::filesystem::path Resolve_SequencePath();
		// 저작 문서가 지원하는 고정 프로필 목록에 있는지 검사한다.
		static bool_t Is_KnownProfile(std::string_view profileId);
		// UTF-8 자막의 길이와 금지 제어 문자, 태그 구분 문자를 검사한다. 개행은 허용한다.
		static bool_t Is_ValidSubtitleText(std::string_view text);
		// 지원하는 네 관문 ID 중 하나인지 검사한다.
		static bool_t Is_KnownGate(std::string_view gateId);
		// 관문과 actor 조합에 대응하는 보스 placement ID를 고정 표에서 차용한다. 대응 항목이 없으면 빈 view다.
		static std::string_view Resolve_DefaultPlacementId(std::string_view gateId, std::string_view actorProfileId);
		// placement ID에 대응하는 archetype ID를 고정 표에서 차용한다. 모르는 ID이면 빈 view다.
		static std::string_view Resolve_BossArchetypeId(std::string_view placementId);
		// placement ID에 대응하는 actor 프로필을 고정 표에서 차용한다. 모르는 ID이면 빈 view다.
		static std::string_view Resolve_ActorProfileForPlacement(std::string_view placementId);
		// 원본 프로필 07은 actor 05를 공유한다. 모르는 프로필이면 빈 view를 반환한다.
		static std::string_view Resolve_ActorProfileId(std::string_view sourceProfileId);
		// Reload와 저장 검증이 호출한다. JSON을 임시 문서로 읽고 구조 검증이 끝난 뒤 outDocument를 교체한다.
		// 실패하면 기존 출력 문서를 보존한다. 격리 가능한 오류 패턴 등의 행은 원본 JSON과 오류를 함께 유지한다.
		static bool_t Parse_Text(
			std::string_view text,
			KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument,
			std::string& outStatus);
		// 각 프로필의 참조 문서를 별도로 읽는다. 실패한 프로필은 빈 값과 상태 메시지로 남긴다.
		// 부분 참조 결과를 outReferences에 교체하고 true를 반환하므로 true가 모든 참조 로드 성공을 뜻하지는 않는다.
		static bool_t Load_ImmutableActionReferences(
			KOUKU_SAYDON_ACTION_REFERENCE_SET& outReferences,
			std::string& outStatus);
        // 방향 분신의 자식 패턴에서 허용한 접촉 판정, 피해 결과와 짧은 방향 전환만 사용하는지 검사한다.
        static bool_t Validate_CrossDirectionChildLogic(
            const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
            const KOUKU_SAYDON_COMPOSITION_PATTERN& child, std::string& outStatus);
        // Summon 또는 기존 Logic으로 표현한 방향 분신 구간을 공통 실행 구간으로 해석한다.
        // 자식 패턴과 시간 충돌까지 검증하며 실패하면 outWindows를 보존한다.
        static bool_t Try_ResolveCrossDirectionWindows(
            const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
            const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
            std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW>& outWindows,
            std::string& outStatus);
        // 소환 자식이 같은 actor와 관문의 독립적인 패턴인지 검사한다. 중첩 소환과 공유 상태 제어는 제한한다.
        static bool_t Validate_SummonPatternTarget(
            const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
            const KOUKU_SAYDON_COMPOSITION_PATTERN& owner,
            std::string_view targetPatternId, std::string& outStatus);
		// 빙고 보스용 유한 부모와 자식을 검사하고 활성 BINGO_DETONATION trigger가 전체에서 정확히 하나인지 확인한다.
		static bool_t Validate_BingoSpecialPatternTarget(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
			std::string_view patternId, std::string& outStatus);
		// 편집 가능한 문서 범위를 검사한다. 현재 구현은 선택적 references를 검사 조건에 사용하지 않는다.
		// 이 성공은 Server 게시와 실제 전투 실행 성공을 대신하지 않는다.
		static bool_t Validate(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
			const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
			std::string& outStatus);
		// 원본을 변경하지 않고 실행용 문서 복사본을 만든다. 일반 자식 펼치기 분기에서는 occurrence별 Logic 정의와 참조 ID도 확장한다.
		// 파생 ID와 빈 구간을 채우는 idle 단계는 저작 데이터가 아니므로 이 결과를 원본으로 저장하지 않는다.
		static bool_t Try_ExpandPatternDocument(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& source,
			std::string_view patternId,
			KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument,
			std::string& outStatus);
		// 패턴의 ANIMATION_BLEND Logic 구간과 연속 포즈 소유자를 연결한다. 모호하거나 겹치는 전환은 거절한다.
		// 성공한 정렬 결과만 outWindows에 교체하므로 실패하면 기존 출력을 보존한다.
		static bool_t Try_ResolveAnimationBlendWindows(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
			const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
			std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& outWindows,
			std::string& outStatus);
		// 저작 문서의 JSON 문자열을 만든다. 디스크 저장이나 Server 게시를 하지 않으며 검증은 호출자가 수행한다.
		static std::string Serialize(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);

		// 저장된 경로가 있으면 그 경로, 없으면 기본 경로로 Reload_FromPath를 호출한다.
		bool_t Reload(std::string& outStatus);
		// 경로 계약, 읽기, 파싱과 검증이 성공하면 LastGood, 참조, 기준 bytes와 generation을 함께 갱신한다.
		// 실패하면 LastGood과 기존 경로를 보존하되 Fresh를 false로 만들고 오류 상태를 갱신한다.
		bool_t Reload_FromPath(
			const std::filesystem::path& path,
			std::string& outStatus);
		// candidate는 로드한 revision을 유지해야 한다. 파일 잠금 아래 최신 디스크와 LastGood을 대조한다.
		// stable ID와 필드가 독립적인 변경만 병합하며 실제 충돌은 디스크와 draft를 보존하고 거절한다.
		// 최신 revision을 증가시키고 교체 직전 bytes를 다시 비교한다. 원자적 교체와 재열기가 성공해야 LastGood을 갱신한다.
		// 교체 성공 뒤 재열기에 실패할 수 있으므로 false가 항상 디스크 무변경을 뜻하지는 않는다.
		bool_t Save_Atomic(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
			std::string& outStatus);

		// 성공적으로 읽거나 저장한 문서가 있는지 확인한다. Fresh와 달리 최근 실패 이후에도 true일 수 있다.
		[[nodiscard]] bool_t Has_LastGood() const noexcept
		{
			return m_bHasLastGood;
		}
		// 마지막 Reload 또는 Save에서 확인한 상태다. 매 조회마다 디스크 변경을 감시하는 함수는 아니다.
		[[nodiscard]] bool_t Is_Fresh() const noexcept
		{
			return m_bFresh;
		}
		// Reload 또는 저장 후 재열기가 성공할 때 증가하는 메모리 세대다. JSON revision과 구분한다.
		[[nodiscard]] std::uint64_t Get_Generation() const noexcept
		{
			return m_iGeneration;
		}
		// 아래 const 참조 조회는 문서 객체의 멤버를 차용한다. 이후 Reload나 Save가 해당 내용을 교체할 수 있다.
		[[nodiscard]] const std::filesystem::path& Get_Path() const noexcept
		{
			return m_Path;
		}
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT& Get_LastGood() const
		{
			return m_LastGood;
		}
		[[nodiscard]] const KOUKU_SAYDON_ACTION_REFERENCE_SET& Get_References() const
		{
			return m_References;
		}
		[[nodiscard]] const std::string& Get_Status() const noexcept
		{
			return m_strStatus;
		}

	private:
		// 문서 객체가 마지막 정상 문서, 선택적 참조와 디스크 비교용 bytes를 소유한다.
		// Workbench draft와 분리하여 검증 실패와 동시 저장 충돌에서 마지막 정상 문서를 보존한다.
		std::filesystem::path m_Path;
		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_LastGood;
		KOUKU_SAYDON_ACTION_REFERENCE_SET m_References;
		std::string m_strBaselineSourceBytes;
		std::string m_strStatus;
		std::uint64_t m_iGeneration = 0u;
		bool_t m_bHasLastGood = false;
		bool_t m_bFresh = false;
	};
}
