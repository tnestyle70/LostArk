#pragma once

#include "Network/NetworkIds.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "WorldBootstrap.h"
#include "ServerNavigation.h"
#include "NpcBehaviorRuntime.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace LostArk::Server
{
	struct KOUKUSAYDON_LOGIC_LEDGER;
	// Kouku의 기존 stage 루트모션 소비자가 한 pattern sequence의 수직 단계를 함께 관리한다.
	struct SERVER_ALBION_AIRBORNE_STATE final
	{
		ALBION_AIRBORNE_PHASE ePhase = ALBION_AIRBORNE_PHASE::NONE;
		std::uint32_t iPatternSequence = 0u, iStartMs = 0u, iDurationMs = 0u, iSourceStageIndex = 0u;
		LostArk::Shared::NET_ENTITY_ID iSelectedPlayer = LostArk::Shared::INVALID_NET_ENTITY_ID;
		bool bHasSelectedGround = false;
		SERVER_NAV_POINT SelectedGround{};
		float fJumpHeightM = 0.f, fPhaseHeightM = 0.f, fStartHeightM = 0.f;
		float fSourceUpAtStart = 0.f, fSourceUpMinimum = 0.f, fLandingProgress = 0.f;
	};

	// 패턴 시작 시 Brain이 준비 tick을 기록하고 일반 후보 선택이 남은 재사용 대기를 읽는다.
	struct SERVER_BOSS_PATTERN_COOLDOWN final
	{
		// 같은 원본 주 action ID를 공유하는 패턴은 재사용 대기 family도 공유한다.
		// 원본 ID가 0인 정의는 stable pattern ID로 구분한다.
		std::uint32_t iSourcePrimaryActionId = 0;
		std::string strPatternId;
		std::uint32_t iReadyTick = 0;
	};

	// 체력 기믹 한 발생의 생명주기다. 실행 큐와 완료/실패 기록을 별도 상태로 표현한다.
	enum class SERVER_BOSS_MECHANIC_STATE : std::uint8_t
	{
		QUEUED,
		ACTIVE,
		COMPLETED,
		FAILED_REQUIRES_RESET
	};

	// 재시도 가능한 대상 부재와 정의/commit 실패를 구분하는 원인이다.
	enum class SERVER_BOSS_MECHANIC_FAILURE : std::uint8_t
	{
		NONE,
		MISSING_PATTERN_DEFINITION,
		NO_VALID_TARGET,
		INVALID_RUNNING_DEFINITION,
		STAGE_TRANSITION_PREFLIGHT,
		STAGE_TRANSITION_COMMIT,
		BOSS_DIED
	};

	// 체력 경계를 통과할 때 stable pattern ID로 생성하는 기믹 발생 기록이다.
	// PendingPatternIds는 실행 대기열이고, 이 기록은 대기/진행/완료/reset 필요 상태를 구분한다.
	// 대상 부재로 다시 예약해도 같은 기록을 사용하여 체력 경계를 중복 발생시키지 않는다.
	struct SERVER_BOSS_MECHANIC_OCCURRENCE final
	{
		std::string strPatternId;
		// 체력 경계를 평가한 정의 revision을 저장한다.
		// 실행 차례가 왔을 때 최신 Catalog의 같은 ID로 바꾸어 해석하지 않도록 GameRoom이 세대를 유지한다.
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision;
		SERVER_BOSS_MECHANIC_STATE eState =
			SERVER_BOSS_MECHANIC_STATE::QUEUED;
		SERVER_BOSS_MECHANIC_FAILURE eFailure =
			SERVER_BOSS_MECHANIC_FAILURE::NONE;
		std::uint32_t iTriggerHealthBar = 0u;
		std::uint32_t iQueuedTick = 0u;
		std::uint32_t iStartedTick = 0u;
		std::uint32_t iFinishedTick = 0u;
		std::uint32_t iPatternSequence = 0u;
	};

	// 기존 갑옷 표시/방어 경로가 읽는 상태다. plate index는 생성 시 profile의 값을 복사한다.
	// 정의된 typed part가 있으면 파괴 결과를 이 상태에도 반영하고, 없으면 GROGGY의 기존 내구도 경로를 사용한다.
	// 남은 내구도가 0인 plate의 방어값은 HP 피해 계산에 더하지 않는다.
	struct SERVER_BOSS_ARMOR_PLATE_STATE final
	{
		std::uint32_t iPlateIndex = 0;
		std::uint32_t iRemainingDurability = 0;
		std::uint32_t iDefense = 0;
	};

	// 외부 snapshot에 반영할 큰 행동 상태다. 패턴 내부의 세부 action ID와 stage는 별도 필드로 보관한다.
	enum class SERVER_ENTITY_ACTION
	{
		IDLE,
		CHASE,
		PATTERN_WINDUP,
		PATTERN_ACTIVE,
		PATTERN_RECOVERY,
		DEAD
	};

	enum class SERVER_DEPENDENT_BOSS_ROLE : std::uint8_t
	{
		NONE,
		AUXILIARY,
		PORTAL_RUNNER
	};

	enum class SERVER_BOSS_PATTERN_TERMINAL_RESULT : std::uint8_t
	{
		NONE,
		COMPLETED,
		ABORTED
	};

	// FinishPattern과 공개 실패 경로가 남긴 최근 종료 결과다. 다음 패턴 예약과 Debug 완료 판단이 소비한다.
	struct SERVER_BOSS_PATTERN_TERMINAL_RECEIPT final
	{
		std::uint32_t iPatternSequence = 0u;
		// 일반 실행에서는 현재 sequence와 같고, 후속 패턴은 최초 선택된 실행의 root sequence를 유지한다.
		// Debug 실행 소유자는 이 값을 이용해 후속 연쇄의 마지막 종료까지 같은 작업으로 묶는다.
		std::uint32_t iRootPatternSequence = 0u;
		SERVER_BOSS_PATTERN_TERMINAL_RESULT eResult =
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::NONE;
	};

	// 결과 분기가 예약한 다음 패턴이다. 완료한 원본 sequence와 정의 revision을 함께 보관한다.
	// SelectPattern은 일치하는 완료 기록을 확인한 뒤 실행하여 다른 실행의 결과나 최신 정의와 섞이지 않게 한다.
	struct SERVER_BOSS_PATTERN_FOLLOWUP final
	{
		std::string strPatternId;
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		std::uint32_t iSourcePatternSequence = 0u;
		std::uint32_t iRootPatternSequence = 0u;
		std::uint8_t iDepth = 0u;

		// 예약 ID의 존재만 검사한다. revision과 완료 기록의 유효성은 실제 선택 경로가 검사한다.
		[[nodiscard]] bool Is_Pending() const noexcept
		{
			return !strPatternId.empty();
		}
	};

	// 한 보스에 대한 플레이어별 MVP 입력이다. 전투 기여와 생존 시간은 서로 다른 경로에서 누적한다.
	struct SERVER_MVP_LEDGER_ROW final
	{
		LostArk::Shared::PLAYER_ID iPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		std::uint64_t iDamage = 0u;
		std::uint64_t iStagger = 0u;
		std::uint32_t iCounterCount = 0u;
		std::uint32_t iAliveTicks = 0u;
		std::uint32_t iFightTicks = 0u;
		// 보상 평가에 사용할 세부 기록이다. 아래 HP 비율 단위는 천분율이다.
		std::uint64_t iPartDamage = 0u;
		std::uint32_t iFinishingBlows = 0u;
		std::uint32_t iDamagingHitsTaken = 0u;
		std::uint32_t iLowestHpPermille = 1000u;
		std::uint32_t iLastCounterTick = 0u;
		std::uint32_t iMinCounterGapTicks = 0u;
		// 이전 전투 tick의 HP와 비교해 감소한 tick을 센다. 개별 피격 event 개수와는 다를 수 있다.
		std::uint32_t iLastHp = 0u;
		bool bHpSeen = false;
		// KNOCKDOWN 상태로 바뀐 순간만 세고 같은 상태의 다음 tick에는 더하지 않는다.
		std::uint32_t iKnockdowns = 0u;
		bool bWasKnockedDown = false;
	};

	// 피격/MVP 집계가 player ID의 기존 행을 찾고 없으면 기본 행을 추가한다.
	// 반환 참조는 ledger vector 요소이므로 이후 재할당이 가능한 추가를 넘겨 보관하지 않는다.
	SERVER_MVP_LEDGER_ROW& Find_Or_Add_MvpLedgerRow(
		std::vector<SERVER_MVP_LEDGER_ROW>& ledger, LostArk::Shared::PLAYER_ID playerId);

	// GameRoom이 소유하는 월드 entity의 가변 실행 상태다. Catalog의 불변 정의와 구분한다.
	// server의 world entity와 gamecatalog의 불변 정의가 어떤 차이를 가지고 있는 걸까?
	// Brain, 전투, 이동, 복제가 같은 방의 simulation 흐름에서 읽고 갱신하며 이 struct 자체에는 잠금이 없다.
	// 값 복사는 단계별 사전 검사/commit 준비에도 사용하므로 원시 포인터 대신 ID와 소유형 상태를 보관한다.
	struct SERVER_WORLD_ENTITY
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		// 종속 보스의 소유 보스 ID다. 기존 Brain을 사용하되 주 보스의 HUD/피격 대상을 대신하지 않는다.
		// 한 패턴을 끝낸 뒤에도 소유 관계를 식별하는 데 사용한다.
		LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		// Server 내부 진행 역할이다. AUXILIARY는 종속 Brain으로 저작 공격을 수행한다.
		// PORTAL_RUNNER는 navigation과 별도로 준비한 삼각 경계의 한 구간을 보간한다.
		SERVER_DEPENDENT_BOSS_ROLE eDependentBossRole =
			SERVER_DEPENDENT_BOSS_ROLE::NONE;
		BOSS_PATTERN_SEQUENCE_DEFINITION DependentPatternSequence;
		// 유령 단계의 주 Valtan은 HP/피격/HUD/보상 소유권을 유지한다.
		// 전면 패턴 목록과 포탈 발생 시계를 분리하여 각각 진행한다.
		BOSS_PATTERN_SEQUENCE_DEFINITION GhostPhasePatternSequence;
		bool bGhostPhasePatternLoopActive = false;
		bool bKoukuGazeClone = false;
		bool bKoukuSummonClone = false;
		std::vector<std::size_t> KoukuSummonStartedTriggers;
		std::uint32_t iKoukuCloneOwnerSequence = 0u;
		std::uint32_t iKoukuCloneEndTick = 0u;
		// 방향 연출은 별도로 stage한 자식 상태를 const shared_ptr로 보관한다.
		// 부모 entity의 패턴/HP/identity/logic 기록은 유지하며, GameRoom이 자식 복사본을 갱신해 교체한다.
		std::shared_ptr<const SERVER_WORLD_ENTITY> KoukuDirectionPlayback;
		// Kouku 독립 contact 실행의 ledger를 공유 소유한다. entity 복사만으로 ledger 내용까지 독립 복제되지는 않는다.
		std::shared_ptr<KOUKUSAYDON_LOGIC_LEDGER> KoukuContactLedger;
		std::uint32_t iKoukuDirectionEndTick = 0u;
		bool bKoukuDirectionPlaybackComplete = false;
		// 보조 유령 발생의 sequence와 다음 spawn tick이다.
		// 주 보스의 패턴 커서와 포탈 발생 sequence를 대신 사용하지 않도록 별도로 둔다.
		std::uint32_t iGhostAuxiliaryOccurrenceSequence = 0u;
		std::uint32_t iGhostAuxiliaryNextSpawnTick = 0u;
		std::uint32_t iGhostPortalLastSpawnTick = 0u;
		std::uint32_t iGhostPortalOccurrenceSequence = 0u;
		// 명시적 Debug 재배치의 진행 상태다. 제품 유령 순환의 공격 완료마다 자동으로 여는 상태는 아니다.
		bool bGhostRepositionPending = false;
		std::uint32_t iGhostReappearTick = 0u;
		std::uint32_t iGhostRelocationSequence = 0u;
		// 한 tick의 후보에서 같은 바닥 높이의 재배치 지점을 못 찾으면 현재 위치를 유지한다.
		// 다음 tick 재시도를 표시하여 이동 위치를 못 찾은 일을 방 전체의 치명 오류로 처리하지 않는다.
		bool bGhostRelocationRetryPending = false;
		std::uint32_t iGhostRelocationRetryTick = 0u;
		// 재배치 정리는 자신이 설정한 숨김/무적 플래그만 해제한다.
		// 다른 기믹이 이미 소유한 무적 상태를 함께 지우지 않기 위한 기록이다.
		bool bGhostRelocationOwnsHiddenFlag = false;
		bool bGhostRelocationOwnsInvulnerableFlag = false;
		// 저장 문서의 placement/archetype/encounter ID와 현재 실행 pattern/stage/action ID를 값으로 보관한다.
		std::string strPlacementId;
		std::string strArchetypeId;
		std::string strEncounterId;
		std::string strPatternId;
		std::string strPatternStageId;
		std::string strActionId;
		std::string strDamageProfileId;
		std::string strSpawnGroupId;
		WORLD_BOOTSTRAP_KIND eKind = WORLD_BOOTSTRAP_KIND::END;
		SERVER_ENTITY_ACTION eAction = SERVER_ENTITY_ACTION::IDLE;
		// 월드 위치는 m, yaw는 degree다. 이동과 판정은 이 Server 상태를 사용하고 Client는 복제된 값을 표현한다.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		float fActionElapsedSeconds = 0.f;
		float fPatternMinimumRange = 0.f;
		float fPatternMaximumRange = 0.f;
		float fPatternForcedMotionSpeed = 0.f;
		// stage 진입 시 복사한 누적 루트모션 샘플이다. 이동 tick마다 Catalog에 다시 접근하지 않는다.
		// Valtan의 일반 이동에서는 이 샘플이 고정 전진 속도보다 우선하며 포탈/도약 이동은 별도 분기가 우선한다.
		std::vector<ROOT_MOTION_SAMPLE> PatternStageRootMotion;
		SERVER_ALBION_AIRBORNE_STATE AlbionAirborne;
		bool bPortalMotionActive = false;
		// PORTAL_TARGET_RUSH는 stage 시작에 대상 방향과 끝점을 잠근다.
		// 표시는 이 경로로 두 포탈을 배치하고, 저작 지연 시간은 실제 이동/판정 시작을 제어한다.
		bool bPortalRushTargetLocked = false;
		std::uint32_t iPortalRushRetargetDelayMs = 0u;
		float fPortalRushSpeedMps = 0.f;
		float fPortalRushDistanceM = 0.f;
		std::vector<LostArk::Shared::NET_ENTITY_ID> PortalStageHitTargets;
		// ACTIVE_WINDOW에서 실제 LANDED 결과가 난 대상만 기록한다. stage마다 초기화하여 같은 구간의 중복 피해를 막는다.
		std::vector<LostArk::Shared::NET_ENTITY_ID> PatternActiveWindowHitTargets;
		float fPortalStartX = 0.f;
		float fPortalStartZ = 0.f;
		float fPortalEndX = 0.f;
		float fPortalEndZ = 0.f;
		// 이전 포탈 예약 판정의 Server 위치다. BOX 판정은 현재 위치까지의 전진 거리도 도형에 포함한다.
		// 지난 tick의 전체 위치 이력을 재생하는 저장소는 아니다.
		float fPortalLastHitSampleX = 0.f;
		float fPortalLastHitSampleZ = 0.f;
		// 생성 시 확정한 위치다. 원형 저작 위치가 navigation에 투영된 경우에는 그 확정 위치를 저장한다.
		// spawn 복귀, 아레나 기준 배치와 안전 지점 탐색이 사용하며 현재 보스 위치와 구분한다.
		float fSpawnPositionX = 0.f;
		float fSpawnPositionY = 0.f;
		float fSpawnPositionZ = 0.f;
		// Mario 몬스터 순찰의 축과 spawn 기준 부호 있는 거리 경계다. MonsterBrain이 왕복 위치를 계산한다.
		std::uint8_t iMarioPatrolStage = 0u;
		float fMarioPatrolAxisX = 0.f;
		float fMarioPatrolAxisZ = 0.f;
		float fMarioPatrolMinimum = 0.f;
		float fMarioPatrolMaximum = 0.f;
		bool bMarioPatrolForward = true;
		// 현재 도약을 시작한 Server 위치다. Brain이 도약 곡선으로 위치를 갱신하고 Client에는 transform이 복제된다.
		float fLeapOriginX = 0.f;
		float fLeapOriginY = 0.f;
		float fLeapOriginZ = 0.f;
		float fLeapApexHeight = 0.f;
		// 현재 도약 착지점이다. LEAP_TO_TARGET은 저장한 대상 위치를 사용하고 나머지 도약은 저작 anchor를 사용한다.
		// 대상 위치를 확보하지 못한 경우도 저작 anchor를 사용한다.
		float fLeapLandingX = 0.f;
		float fLeapLandingY = 0.f;
		float fLeapLandingZ = 0.f;
		// 패턴에 저작된 도약 높이다. 도약을 소유하지 않는 패턴은 0이다.
		float fPatternLeapApexHeight = 0.f;
		// 저작된 하강 stage index다. 첫 이륙 stage와 그 사이의 stage는 정점 유지 구간을 구성할 수 있다.
		std::uint32_t iPatternLeapTravelStageIndex = 1u;
		// stage 전체 시간과 실제 상승/하강 구간을 구분하는 ms 경계다.
		// StageWindowRatio가 이 구간 안의 진행률을 계산한다.
		std::uint32_t iPatternLeapTakeoffStartMs = 0u;
		std::uint32_t iPatternLeapTakeoffEndMs = 0u;
		std::uint32_t iPatternLeapTravelStartMs = 0u;
		std::uint32_t iPatternLeapTravelEndMs = 0u;
		bool bPatternMoveToAnchorBeforeTakeoff = false;
		// 입장 공격과 입장 연출의 소비 여부다. 플레이어별 상태가 아니므로 늦게 참가한 플레이어마다 다시 실행하지 않는다.
		bool bIntroPatternConsumed = false;
		bool bEntranceCinematicConsumed = false;
		// Debug 순서 재생 여부다. 재생이 실행 대기열을 관리하는 동안 제품의 후속 패턴 예약과 섞이지 않게 한다.
		bool bScriptedPatternPlayback = false;
		// 입장/체력 기믹 시험은 제품 자동 sequence를 잠시 양보시킨다.
		// ARM_HEALTH_BAR 대기와 시험 완료 후 대기를 서로 다른 플래그로 구분한다.
		bool bAutomaticPatternSequenceAuditionOverride = false;
		bool bAutomaticPatternSequenceAuditionHold = false;
		// 선택한 순서형 항목이 실행 중임을 뜻한다. 커서는 선택 순간이 아니라 정상 FinishPattern에서 진행한다.
		// 대상 부재로 취소하면 같은 항목을 재시도하고 다른 실행 실패는 reset 필요 상태를 남긴다.
		bool bAutomaticPatternSequenceStepRunning = false;
		// 기존 pause 상태를 읽는 GameRoom/Debug 경계가 남아 있는 필드다.
		// 현재 ValtanBrain은 대상 부재를 NO_VALID_TARGET 종료/재예약으로 처리하며 이 플래그를 true로 올리지 않는다.
		// 따라서 이 필드만 보고 stage와 판정 커서를 정지 보존하는 기능이 현재 동작한다고 해석하지 않는다.
		bool bAutomaticPatternSequencePausedForRevive = false;
		std::uint32_t iAutomaticPatternSequencePauseLastTick = 0u;
		// 순서형 비마지막 항목을 완료한 뒤의 저작 추적 대기 tick이다.
		// 남은 시간이 있는 동안 일반 CHASE를 사용하되 다음 순서형 항목을 다른 후보로 바꾸지 않는다.
		std::uint32_t iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
		std::uint32_t iAutomaticPatternSequencePursuitTicksRemaining = 0u;
		std::uint32_t iPatternTelegraphMs = 0;
		std::uint32_t iPatternActiveMs = 0;
		std::uint32_t iPatternRecoveryMs = 0;
		std::uint32_t iPatternSequence = 0;
		std::uint32_t iPatternStartTick = 0;
		SERVER_BOSS_PATTERN_TERMINAL_RECEIPT PatternTerminalReceipt;
		SERVER_BOSS_PATTERN_FOLLOWUP PendingPatternFollowup;
		// 직접 선택한 실행은 0이고, 결과 분기를 통해 다른 패턴에 들어갈 때마다 깊이를 증가시킨다.
		std::uint8_t iPatternFollowupDepth = 0u;
		std::uint32_t iPatternFollowupRootSequence = 0u;
		// GameRoom이 실제 잡기 처형을 확정한 sequence/stage다.
		// 마지막 살아 있는 대상을 처형한 뒤에도 해당 stage의 남은 시간을 진행할 근거로 사용한다.
		std::uint32_t iGrabExecutionCommittedPatternSequence = 0u;
		std::uint32_t iGrabExecutionCommittedStageIndex = 0u;
		std::uint32_t iPatternStageIndex = 0;
		// 생성 및 패턴 실행 경계에서 고정한 정의 revision이다. 실제 불변 Catalog의 보관은 GameRoom이 맡는다.
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		// 제품 sequence가 첫 항목부터 종료 대기까지 사용할 정의 revision이다.
		// 기존 순번을 새 목록에 적용하지 않도록 실행 정의를 고정하며 Debug Flow는 별도 고정을 사용한다.
		LostArk::Shared::GameplayDataRevision ProductSequencePinnedDefinitionRevision{};
		std::uint32_t iPatternStageDurationMs = 0;
		// stage를 처음 평가할 0이 아닌 Server tick이다. 0은 미설정이며 UINT32_MAX 다음 tick은 1이다.
		std::uint32_t iPatternStageFirstEvaluationTick = 0;
		std::uint64_t iPatternAimLastElapsedTicks = 0u;
		// 현재 action에서 commit한 spawn wave 수다.
		// ENTER의 첫 wave와 이후 예약 wave 모두 전체 생성 transaction 성공 뒤에만 횟수를 진행한다.
		std::uint32_t iAppliedPatternStageSpawnWaveCount = 0;
		BOSS_PATTERN_STAGE_MOTION_KIND ePatternStageMotionKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		BOSS_PATTERN_HIT_SHAPE ePatternHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		BOSS_PATTERN_PLAYER_RESPONSE ePatternPlayerResponse =
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT ePatternAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		BOSS_PATTERN_PART_DAMAGE_POLICY ePatternPartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		bool bPatternHasCounterProxy = false;
		BOSS_PATTERN_COUNTER_PROXY_KIND ePatternCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
		float fPatternCounterProxyForwardOffsetM = 0.f;
		float fPatternCounterProxyRightOffsetM = 0.f;
		float fPatternCounterProxyRadiusM = 0.f;
		float fPatternCounterProxyArcDegrees = 0.f;
		BOSS_PATTERN_BOSS_RESPONSE_KIND ePatternBossResponseKind =
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE;
		std::uint32_t iPatternBossResponseThreshold = 0u;
		std::uint32_t iPatternBossResponseAccumulatedHealthDamage = 0u;
		bool bPatternBossResponsePublished = false;
		// 높이 보정은 실행 시작 때 저장한 기준 Y에 더한다. 이전 보정 결과에 다시 더해 누적하지 않는다.
		// stage 보정을 먼저 제거한 뒤 pattern 보정을 제거할 수 있도록 각각 기준값을 둔다.
		bool bPatternVerticalOffsetApplied = false;
		float fPatternVerticalBaseY = 0.f;
		bool bPatternStageVerticalOffsetApplied = false;
		float fPatternStageVerticalBaseY = 0.f;
		float fPatternHitOuterRadius = 0.f;
		float fPatternHitInnerRadius = 0.f;
		float fPatternHitAngleDegrees = 0.f;
		float fPatternHitLength = 0.f;
		float fPatternHitHalfWidth = 0.f;
		// stage 진입 시 복사한 예약 판정 수와 ms 간격/지연이다.
		// iAppliedPatternHitCount는 명중 수가 아니라 이미 처리한 기회 수이며 miss여도 증가한다.
		std::uint32_t iPatternHitCount = 0;
		std::uint32_t iPatternHitIntervalMs = 0;
		std::uint32_t iPatternHitDelayMs = 0;
		std::uint32_t iAppliedPatternHitCount = 0;
		BOSS_PATTERN_HIT_ANCHOR_KIND ePatternHitAnchorKind =
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		float fPatternHitAnchorForwardOffsetM = 0.f;
		float fPatternHitAnchorRightOffsetM = 0.f;
		float fPatternHitAnchorYawOffsetDegrees = 0.f;
		BOSS_PATTERN_HIT_ACTIVATION_KIND ePatternHitActivationKind =
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		std::uint32_t iPatternHitActivationStartMs = 0u;
		std::uint32_t iPatternHitActivationLifetimeMs = 0u;
		// stage 이동 전의 기준 위치/방향이다. 판정 anchor가 STAGE_ORIGIN일 때 이 값을 사용한다.
		float fPatternStageOriginX = 0.f;
		float fPatternStageOriginZ = 0.f;
		float fPatternStageOriginYawDegrees = 0.f;
		// Kouku 루트모션 원점은 spawn 복귀와 ENTER 대상 갱신을 마친 뒤 확정한다.
		float fPatternStageOriginY = 0.f;
		float fPatternStageRootGroundY = 0.f;
		bool bPatternStageRootOriginCaptured = false;
		// 명시적 지상 순간이동 뒤에는 이 패턴이 끝날 때까지 navigation 기준 높이를 사용한다.
		bool bPatternRootGrounded = false;
		std::uint32_t iPatternStageRootLastTick = 0u;
		bool bPatternWallContact = false;
		// 현재 stage의 공격이 엄폐물을 관통하는지 정한다. true이면 엄폐 선분 검사를 건너뛴다.
		bool bPatternPiercesCover = false;
		// 현재 stage의 기본 플레이어 밀기 거리(m)다. 음수는 공격 원점 쪽으로 끌어당긴다.
		// 개별 AttackContacts가 있으면 그 contact의 밀기 정의가 이 기본값을 덮는다.
		float fPatternPushRangeM = 0.f;
		std::uint32_t iPatternPushMs = 0;
		bool bPatternKnockdown = false;
		std::uint32_t iPatternDownMs = 0;
		// 현재 GROGGY stage 여부다. stage 전환/패턴 정리와 갑옷 파괴 응답이 이 상태를 갱신한다.
		bool bPatternGroggy = false;
		// 현재 stage가 벽 충돌 처리를 요구하는 돌진인지 나타낸다.
		// 실제 충돌 뒤의 목적 stage는 저작된 WALL_CONTACT 분기로 결정한다.
		bool bPatternChargeImpact = false;
		// 패턴 실행 동안의 무적 정책이다. 외부 player hit 경로가 먼저 검사하고 패턴 종료 시 해제한다.
		bool bPatternInvulnerable = false;
		// Kouku logic window의 무력화 입력과 방어 상태다.
		// 방패의 반사 대상은 저작 region 또는 방향 범위를 통해 검사하며 window 종료 시 해당 상태를 정리한다.
		std::uint32_t iKoukuItemStaggerMaximum = 0u;
		std::uint32_t iKoukuItemStaggerCredit = 0u;
		std::uint32_t iKoukuDamageReductionWindows = 0u;
		bool bKoukuShieldActive = false;
		float fKoukuShieldArcDegrees = 0.f;
		float fKoukuShieldNormalYawOffsetDegrees = 0.f;
		std::vector<BOSS_LOGIC_REGION> KoukuShieldRegions;
		// 종료 뒤 유지되는 logic 소유자에 대한 weak_ptr다. 실제 소유와 만료 시각은 GameRoom이 관리한다.
		std::vector<std::weak_ptr<SERVER_WORLD_ENTITY>> KoukuRetainedLogicOwners;
		// 피해 경로가 갑옷 파괴 후 남기는 반응 요청이다.
		// Brain이 다음 결과 분기 또는 PART_BREAK 진입을 처리하며 피해 계산 중에 직접 stage를 바꾸지 않는다.
		bool bPendingArmorBreakReaction = false;
		std::uint32_t iActionStartTick = 0;
		// 플레이어 스킬이 이 entity에 적용한 buff/debuff 상태다. 지속 시간과 피해 증폭 등의 소비는 전투 경로가 맡는다.
		std::vector<LostArk::Shared::ACTIVE_BUFF> ActiveBuffs;
		// 일반 몬스터가 다시 동작할 수 있는 기준 tick이다. MonsterBrain은 그 tick 전까지 동작을 멈춘다.
		std::uint32_t iStunEndTick = 0;
		std::uint32_t iCurrentHp = 1;
		std::uint32_t iMaximumHp = 1;
		// 체력줄 피해량 계산의 기준 HP다. 0이면 실제 최대 HP를 사용한다.
		std::uint32_t iDamageReferenceHp = 0;
		std::uint32_t iMaximumHealthBars = 1;
		std::uint32_t iLastEvaluatedHealthBar = 1;
		std::uint8_t iPhase = 1;
		// 마지막 체력 경계 평가에 사용한 방 내부 정의 세대다.
		// 실행 중인 패턴의 PinnedDefinitionRevision과 새 체력 기믹 평가의 활성 세대는 다를 수 있다.
		// 고정된 제품 sequence는 기존 실행 정의로 평가하므로 모든 hot reload가 즉시 이 값에 반영되지는 않는다.
		std::uint16_t iLastHealthMechanicGenerationEpoch = 0u;
		SERVER_BOSS_COMBAT_STATE BossCombat;
		float fEngageDistance = 0.f;
		float fTargetReleaseDistance = 0.f;
		float fMoveSpeed = 0.f;
		float fTurnSpeedDegreesPerSecond = 0.f;
		float fMoveAcceleration = 0.f;
		float fMoveDeceleration = 0.f;
		float fArrivalSlowRadius = 0.f;
		float fCurrentMoveSpeed = 0.f;
		float fCollisionRadius = 0.f;
		float fAttackRange = 0.f;
		std::uint32_t iAttackPower = 0;
		// 기믹 몬스터의 고정 기본 피해다. 0이면 일반 공격력 경로를 사용한다.
		std::uint32_t iAttackFixedDamage = 0u;
		std::uint32_t iDefense = 0;
		std::uint32_t iDeadDespawnMs = 0;
		// 보스 사망 보상을 이미 지급했는지 기록한다. 사망 entity가 남은 다음 tick에도 같은 보상을 반복 지급하지 않는다.
		bool bLootGranted = false;
		// 플레이어 공격 결과와 방의 전투 tick이 함께 쌓는 MVP 기록이다.
		// 최초 기여 행이 생긴 뒤부터 살아 있는 주 보스의 전투 시간을 집계한다.
		std::vector<SERVER_MVP_LEDGER_ROW> MvpLedger;
		// 몬스터가 피격될 때 저작된 밀기 거리에 적용하는 배율이다. 0이면 그 경로의 밀림을 만들지 않는다.
		float fHitKnockbackScale = 0.f;
		// 몬스터 공격이 플레이어에게 적용할 밀기 거리(m)다. 음수는 몬스터 쪽으로 끌어당긴다.
		float fAttackPushRangeM = 0.f;
		std::uint32_t iAttackPushMs = 0;
		bool bAttackKnockdown = false;
		std::uint32_t iAttackDownMs = 0;
		// 진행 중인 피격 이동의 XZ 단위 방향, m/s 속도와 남은 초다.
		// 음수 속도는 반대 방향으로 당기며 새 피격은 기존 이동을 누적하지 않고 새 구간으로 갱신한다.
		float fKnockbackDirectionX = 0.f;
		float fKnockbackDirectionZ = 0.f;
		float fKnockbackSpeed = 0.f;
		float fKnockbackRemainingSeconds = 0.f;
		// 에스더 소환은 일반 Brain 대신 정해진 타격 시간을 진행한다.
		// 타격 종료 뒤 소환 entity를 정리하므로 일반 몬스터 사망 후 despawn 시간과 구분한다.
		bool isEstherSummon = false;
		std::uint32_t iEstherStrikeMs = 0;
		LostArk::Shared::ESTHER_ID eEstherId = LostArk::Shared::ESTHER_ID::NONE;
		LostArk::Shared::PLAYER_ID iEstherCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		std::uint32_t iEstherAppliedHitMask = 0u;
		bool bEstherSupportApplied = false;
		std::uint32_t iNextPathReplanTick = 0;
		BOSS_PHASE_POLICY PhasePolicy;
		// 생성 시 PhasePolicy의 임계값을 복사해 두는 기존 fixture 필드다. 제품 선택은 typed PhasePolicy를 읽는다.
		std::uint32_t iPhaseTwoHpPercent = 0;
		// 생성 시 profile의 저작 plate 순서대로 복사한다. 갑옷이 없는 entity는 빈 vector다.
		std::vector<SERVER_BOSS_ARMOR_PLATE_STATE> ArmorPlates;
		bool hasAppliedPatternDamage = false;
		std::string strLastPatternId;
		// 활성 자동 sequence/rotation의 ID와 순서 커서다. 순서형 실행은 정상 종료 때 진행한다.
		// 가중치 선택에서는 ID를 진단에 쓰고 커서를 순서 실행처럼 진행하지 않는다.
		std::string strRotationId;
		std::uint32_t iRotationStepIndex = 0;
		std::uint32_t iConsecutivePatternUses = 0;
		std::vector<SERVER_BOSS_PATTERN_COOLDOWN> PatternCooldowns;
		// 강제 실행 stable ID 대기열이다. 정의/발생 상태는 Catalog와 MechanicOccurrences에서 별도로 확인한다.
		std::vector<std::string> PendingPatternIds;
		std::vector<std::string> TriggeredPatternIds;
		// 체력 기믹 발생 기록의 소유 저장소다. 원소를 가리키는 임시 포인터는 vector 변경을 넘겨 보관하지 않는다.
		std::vector<SERVER_BOSS_MECHANIC_OCCURRENCE> MechanicOccurrences;
		bool bMechanicLedgerRequiresReset = false;
		LostArk::Shared::NET_ENTITY_ID iTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		// 패턴의 저작 대상 ID다. 전투 유지 여부를 확인할 최근접 대상과 구분한다.
		// LOCK은 시작 대상 ID를 유지하지만 stage별 최근접 조준이나 명시적 retarget이 대상을 갱신할 수 있다.
		LostArk::Shared::NET_ENTITY_ID iPatternTargetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		// 마지막으로 유효했던 대상의 Server 위치다.
		// 대상 부재 후에도 예약된 도약/포탈 등의 stage action이 확정 위치를 사용할 수 있게 값으로 저장한다.
		bool bHasPatternTargetLastPosition = false;
		float fPatternTargetLastPositionX = 0.f;
		float fPatternTargetLastPositionY = 0.f;
		float fPatternTargetLastPositionZ = 0.f;
		// Kouku 대상 연출이 끝난 시점에 방향을 잠그기 전의 본체 yaw를 보관한다.
		// 다음 선택이나 패턴 종료가 원래 yaw를 복구하고 저장값을 비운다. sequence가 같은 경우만 복구한다.
		std::optional<float> KoukuPresentationAimRestoreYawDegrees;
		std::uint32_t iKoukuPresentationAimSequence = 0u;
		// GameRoom의 조준 처리와 복구 함수가 현재 sequence의 저장 yaw가 있는지만 검사한다.
		[[nodiscard]] bool Has_KoukuPresentationAim() const noexcept
		{
			return KoukuPresentationAimRestoreYawDegrees.has_value() &&
				iKoukuPresentationAimSequence == iPatternSequence;
		}
		// Kouku Brain의 시작/종료와 GameRoom 연출 종료가 호출한다.
		// 현재 sequence와 일치할 때만 yaw를 복구하지만 저장값과 sequence는 항상 비운다.
		void Restore_KoukuPresentationAim() noexcept
		{
			if (Has_KoukuPresentationAim())
				fYawDegrees = *KoukuPresentationAimRestoreYawDegrees;
			KoukuPresentationAimRestoreYawDegrees.reset();
			iKoukuPresentationAimSequence = 0u;
		}
		float fLastPathGoalX = 0.f;
		float fLastPathGoalZ = 0.f;
		// navigation이 만든 경로 점을 entity가 소유한다. iMovePathIndex는 다음에 소비할 점의 위치이며 저장 ID가 아니다.
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;
		SERVER_NPC_BEHAVIOR_STATE NpcBehavior;
	};
}
