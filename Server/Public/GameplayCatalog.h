#pragma once

#include "Gameplay/AttackHitTemplate.h"

#include "Network/PacketMessages.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	// 게시자와 Server 로더가 함께 사용하는 gameplay bootstrap 버전이다. 로더는 이 버전과 다른 입력을 거부한다.
	// 그래 이 bootstrap의 불일치 때문에 컴파일 에러 나고 뭐 그런 경우가 많았는데,
	// 왜 이게 강제가 되어야 하고, 이 bootstrap에 대한 것이 필요한지 조차, 그 구조가 왜 그렇게 설계가 되어야 하는지도 좀 
	// 의문이 들었다. 왜 bootstrap이 필요한 구조인 거야? 
	inline constexpr std::uint32_t GAMEPLAY_BOOTSTRAP_VERSION =
		LostArk::Shared::GAMEPLAY_BOOTSTRAP_FORMAT_VERSION;

	// 클립에 저작된 누적 이동의 한 표본이다. 플레이어 스킬과 보스 stage가 각자의 경과 ms로 샘플링한다.
	// 전방/옆/위 변위는 m이며, 현재 위치에 표본 전체를 반복 더하지 않고 구간 차이를 소비한다.
	struct ROOT_MOTION_SAMPLE
	{
		std::uint32_t iTimeMs = 0;
		float fForward = 0.f;
		float fLateral = 0.f;
		// 선택적인 네 번째 성분이다. 수평 이동만 읽는 소비자는 이 수직 값을 사용하지 않는다.
		float fUp = 0.f;
	};

	// 스킬 시계에 예약할 타격 정의다. PLAYER_SKILL_DEFINITION 또는 combo stage가 목록을 소유하고 PlayerSkillSystem이 소비한다.
	// 반복 횟수와 예약 시간은 정의이며, 이미 소비한 타격과 대상별 적용 기록은 실행 상태 쪽에서 관리한다.
	struct PLAYER_SKILL_HIT final
	{
		// 0은 기존 복합 타격이고 독립 결과 채널을 쓰는 데이터는 아래 종류를 구분한다.
		// 1은 피해/부위/밀기, 2는 무력화, 3은 카운터다. 소비자는 해당 채널의 값만 적용한다.
		std::uint32_t iResultKind = 0u;
		std::uint32_t iTimeMs = 0;
		std::uint32_t iRepeatCount = 1;
		std::uint32_t iRepeatMs = 0;
		// AreaType 1은 원/링(range, inner), 2는 전방 box(range 길이, width), 3은 부채꼴(range, angle, inner)이다.
		std::uint32_t iAreaType = 0;
		float fRange = 0.f;
		float fAngleDegrees = 0.f;
		float fWidth = 0.f;
		float fHeight = 0.f;
		float fOffset = 0.f;
		float fInner = 0.f;
		std::uint32_t iMaxTargets = 0;
		// 이 타격의 밀기 기간은 ms, 거리는 m다. 기간 0이면 이동하지 않으며 음수 거리는 공격 원점 쪽으로 당긴다.
		std::uint32_t iPushMs = 0;
		float fPushRange = 0.f;
		std::uint32_t iDurationMs = 0;
	};

	enum class PLAYER_PROJECTILE_KIND : std::uint8_t
	{
		MISSILE,
		FIXAREA,
		GRENADE,
		TRACE
	};

	// 전투 오브젝트 자체가 적용할 타격이다. contact는 접촉 대상별 반복 횟수/간격을, timed는 생성 후 iTimeMs 일정을 사용한다.
	// 도형 원점과 전방은 시전자의 현재 자세가 아니라 해당 오브젝트의 자세를 기준으로 한다.
	struct PLAYER_PROJECTILE_HIT final
	{
		bool isContact = false;
		PLAYER_SKILL_HIT Hit;
	};

	// 시전자 자세에서 전방/오른쪽 offset으로 생성할지, 거리 제한을 거친 조준 지점에 생성할지 정한다.
	enum class PLAYER_PROJECTILE_ORIGIN : std::uint8_t
	{
		CASTER,
		AIM
	};

	// 스킬 action의 iTimeMs에 생성하는 오브젝트 정의다. 이동 속도는 m/s, 거리와 반지름은 m, 수명은 ms다.
	// FIXAREA는 원점에 머무르며 나머지 이동 종류의 실제 경로는 CombatObjectRuntime이 해석한다.
	struct PLAYER_SKILL_PROJECTILE final
	{
		std::uint32_t iTimeMs = 0;
		PLAYER_PROJECTILE_KIND eKind = PLAYER_PROJECTILE_KIND::MISSILE;
		PLAYER_PROJECTILE_ORIGIN eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
		float fOffsetForward = 0.f;
		float fOffsetRight = 0.f;
		float fSpeed = 0.f;
		float fMinDistance = 0.f;
		float fMaxDistance = 0.f;
		std::uint32_t iLifeMs = 0;
		float fRadius = 0.f;
		std::vector<PLAYER_PROJECTILE_HIT> Hits;
	};

	// 실행 중인 action이 다음 입력을 허용하는 ms 구간이다. 스킬/이동/회피는 각자 다른 목록을 사용한다.
	// 콤보 stage가 있으면 해당 stage의 시계를 기준으로 검사한다.
	struct PLAYER_CANCEL_WINDOW final
	{
		std::uint32_t iStartMs = 0;
		std::uint32_t iEndMs = 0;
	};

	enum class PLAYER_CANCEL_INPUT : std::uint8_t
	{
		SKILL,
		MOVE,
		DODGE,
	};

	// 콤보/홀드/카운터의 한 단계 정의다. 단계 전환 때 실행 시계를 초기화하므로 타격/이동/취소 시간도 이 단계 기준이다.
	struct PLAYER_COMBO_STAGE final
	{
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		// 피해 발생 시각과 다음 action/stage로 넘어가는 시각을 별도로 저장한다.
		std::uint32_t iComboAdvanceMs = 0;
		std::uint32_t iInputOpenMs = 0;
		std::uint32_t iInputCloseMs = 0;
		// stage 전환 때 action 시계를 초기화하므로 이동/타격/취소 구간도 각 stage가 소유한다.
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
		std::vector<PLAYER_CANCEL_WINDOW> SkillCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> MoveCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> DodgeCancelWindows;
	};

	// stable SKILL_ID로 조회할 승인된 스킬 정의다. 입력 slot/재사용 시간/자원 비용과 판정·이동 목록을 함께 보관한다.
	// 실행 중 HP, cooldown, combo 진행 상태는 PlayerSkillSystem이 갱신하는 SERVER_PLAYER가 소유한다.
	struct PLAYER_SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strInputSlot;
		std::string strActionId;
		std::string strDamageProfileId;
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iResourceCost = 0;
		// Try_Start가 스킬 시작을 승인할 때 한 번 차감하는 직업 identity 비용이다. 부족하면 시작을 거부하며 0이면 차감하지 않는다.
		std::uint32_t iIdentityCost = 0;
		// 이 프로젝트가 조정하는 보스 상호작용 값이다. 0이면 해당 채널을 제공하지 않으며 일반 HP 피해와 분리한다.
		std::uint32_t iStaggerDamage = 0;
		std::uint32_t iPartDamage = 0;
		std::uint32_t iCounterPower = 0;
		float fMovementDistance = 0.f;
		// 추출한 RootMotion 표본을 고치지 않고 소비하는 수평 이동량에 곱하는 배율이다.
		// 1이면 기존 변위를 유지하고 action 길이는 바꾸지 않아 배율이 크면 같은 시간에 더 멀리 이동한다.
		float fRootMotionScale = 1.f;
		// 가디언 나이트의 불씨 획득/소비와 소켓 잠금 정의다. 다른 스킬은 기본 0/false를 사용한다.
		// 현재 불씨와 잠긴 소켓 수는 이 정의가 아니라 SERVER_PLAYER가 소유한다.
		std::uint32_t iEmberGain = 0;
		std::uint32_t iEmberCost = 0;
		bool locksEmberSocket = false;
		float fMaximumRange = 0.f;
		LostArk::Shared::SKILL_TARGET_INTENT_KIND eTargetIntent =
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fTargetMaximumRange = 0.f;
		bool requiresWalkableTarget = false;
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eSetsStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		std::vector<PLAYER_COMBO_STAGE> ComboStages;
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
		std::vector<PLAYER_CANCEL_WINDOW> SkillCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> MoveCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> DodgeCancelWindows;
	};

	// 기존 갑옷 plate 정의다. iPlateIndex는 게시 순서와 Client 부위 표시를 연결한다.
	// iDurability는 타격 횟수가 아닌 내구도 양이며 기존 경로는 GROGGY에서 소비한다.
	// typed Parts가 있으면 별도의 damage condition으로 판정한 결과를 기존 plate에 반영한다.
	// valtan boss의 armor에 대한 정보를 가지는 구조체이다.
	struct BOSS_ARMOR_PLATE
	{
		std::uint32_t iPlateIndex = 0;
		std::uint32_t iDurability = 0;
		std::uint32_t iDefense = 0;
	};

	enum class BOSS_PHASE_POLICY_KIND : std::uint8_t
	{
		// HP 비율로 phase를 정하는 정책이다. 기준 percent는 BOSS_PHASE_POLICY의 값으로 저장한다.
		HEALTH_PERCENT_THRESHOLD,
		// 저작 stage의 SET_GAMEPLAY_PHASE 명령으로 phase를 전환한다. profile에 같은 pattern/stage 연결을 중복 저장하지 않는다.
		AUTHORED_PATTERN_EVENT
	};

	struct BOSS_PHASE_POLICY final
	{
		BOSS_PHASE_POLICY_KIND eKind =
			BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD;
		std::uint32_t iThresholdPercent = 0;
	};

	// archetype ID로 찾는 보스 기본 수치다. entity 생성 시 복사할 HP/방어/이동/부위 설정이며 현재 전투 상태는 아니다.
	struct BOSS_RUNTIME_PROFILE
	{
		std::string strArchetypeId;
		std::string strEncounterId;
		std::uint32_t iMaximumHp = 0;
		// 피해 산정에 사용할 별도 기준 HP다. 0이면 실제 최대 HP를 기준으로 삼는다.
		std::uint32_t iDamageReferenceHp = 0;
		std::uint32_t Get_DamageReferenceHp() const noexcept
		{
			return iDamageReferenceHp ? iDamageReferenceHp : iMaximumHp;
		}
		std::uint32_t iMaximumHealthBars = 0;
		// 이 보스의 피해 profile을 수치로 바꿀 때 사용하는 공격력이다.
		std::uint32_t iAttackPower = 0;
		// Server가 대상의 중심 거리로 판정할 때 원형 몸체의 가장자리까지 포함하기 위한 반지름이다.
		float fCollisionRadius = 0.f;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		BOSS_PHASE_POLICY PhasePolicy;
		// 기존 plate가 없는 보스는 이 목록의 방어 기여가 없다. typed 부위/보호막/무적 등 다른 정책까지 없다는 뜻은 아니다.
		std::vector<BOSS_ARMOR_PLATE> ArmorPlates;
	};

	enum class BOSS_PATTERN_SELECTION
	{
		NORMAL,
		HEALTH_BAR,
		// 일반 가중치나 체력줄 자동 선택 후보에서 제외하는 저작 패턴이다.
		// 명시적 ID 요청/저작 후속 연결 등 승인된 경로가 실행하며 이 값만으로 실행을 시작하지 않는다.
		AUDITION_ONLY
	};

	// 일반 패턴 후보가 요구하는 갑옷 상태다. ANY는 상태를 제한하지 않고 plate가 없는 상태는 STRIPPED 쪽으로 판정한다.
	enum class BOSS_PATTERN_ARMOR_REQUIREMENT
	{
		ANY,
		ARMORED,
		STRIPPED
	};

	// 패턴 후보의 phase 조건이다. phase를 계산하거나 변경하는 책임은 boss profile의 정책과 stage 명령에 있다.
	enum class BOSS_PATTERN_PHASE_REQUIREMENT
	{
		ANY,
		PHASE_ONE,
		PHASE_TWO
	};

	enum class BOSS_PATTERN_HIT_SHAPE
	{
		NONE,
		CIRCLE,
		RING,
		CONE,
		BOX,
		CROSS,
		SIX_DIRECTIONS
	};

	enum class BOSS_COMBAT_OBJECT_KIND : std::uint8_t
	{
		FIXED_AREA,
		MISSILE
	};

	enum class BOSS_COMBAT_OBJECT_ORIGIN_POLICY : std::uint8_t
	{
		BOSS_POSITION,
		LOCKED_TARGET_UNTIL_FIRST_PULSE,
		// 생성 시 승인한 살아 있는 플레이어마다 대상을 잠근 오브젝트를 만든다. 생성 이후 참가자 변화가 기존 생성 개수를 바꾸지는 않는다.
		LOCKED_TARGET_PER_ALIVE_PLAYER
	};

	enum class BOSS_COMBAT_OBJECT_DIRECTION_POLICY : std::uint8_t
	{
		NONE,
		PATTERN_FACING_AT_SPAWN,
		RADIAL_INWARD,
		// 방사형 생성의 각 순번이 다음 생성 슬롯을 향하게 한다. 마지막 순번의 다음 슬롯은 0이다.
		NEXT_RADIAL_SLOT
	};

	enum class BOSS_COMBAT_OBJECT_HIT_TRIGGER : std::uint8_t
	{
		CONTACT,
		TIMED
	};

	// 독립 보스 전투 오브젝트의 접촉/예약 타격 정의다. 반복 소비와 현재 위치는 CCombatObjectRuntime이 따로 소유한다.
	struct BOSS_COMBAT_OBJECT_HIT final
	{
		std::string strHitId;
		BOSS_COMBAT_OBJECT_HIT_TRIGGER eTrigger =
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		std::uint32_t iAtMs = 0;
		std::uint32_t iRepeatCount = 1;
		std::uint32_t iRepeatIntervalMs = 0;
		BOSS_PATTERN_HIT_SHAPE eHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		float fHitOuterRadius = 0.f;
		float fHitInnerRadius = 0.f;
		float fHitAngleDegrees = 0.f;
		float fHitLength = 0.f;
		float fHitHalfWidth = 0.f;
		std::string strDamageProfileId;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0;
	};

	// 전투 오브젝트의 같은 수명 시계에서 발생하는 표시 pulse다. 도형이나 피해 profile은 가지지 않는다.
	// GameRoom이 stable event ID를 복제하고 Client가 실제 연출을 찾는다.
	// 오브젝트 수명 중 게시된 Client visual ID를 알릴 시점이다. 이 행만으로 HP 피해를 적용하지 않는다.
	struct BOSS_COMBAT_OBJECT_PRESENTATION_PULSE final
	{
		std::string strPresentationEventId;
		std::uint32_t iAtMs = 0;
	};

	// 소유 보스의 지정 action 타격 도형과 연쇄 오브젝트를 연결한다. CCombatObjectRuntime::Apply_OwnerHits가 지연을 계산해 활성화한다.
	// 다음 보스 패턴을 고르는 분기가 아니라 오브젝트의 경고/예약 타격 시점을 여는 정의다.
	struct BOSS_COMBAT_OBJECT_OWNER_HIT_CHAIN final
	{
		std::string strTriggerActionId;
		std::string strArmedPresentationEventId;
		std::uint32_t iDelayMs = 0u;
	};

	// 전투 오브젝트 archetype 정의다. 공유 Catalog가 소유하고 각 실행 인스턴스는 생성 시 필요한 값과 진행 상태를 갖는다.
	struct BOSS_COMBAT_OBJECT_DEFINITION final
	{
		std::string strEncounterId;
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		BOSS_COMBAT_OBJECT_KIND eKind = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA;
		BOSS_COMBAT_OBJECT_ORIGIN_POLICY eOriginPolicy =
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION;
		BOSS_COMBAT_OBJECT_DIRECTION_POLICY eDirectionPolicy =
			BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE;
		float fOffsetForwardM = 0.f;
		float fOffsetRightM = 0.f;
		float fSpeedMps = 0.f;
		float fMaximumDistanceM = 0.f;
		std::uint32_t iMovementStartDelayMs = 0u;
		bool bExpireOnDistanceEnd = true;
		float fCoverRadiusM = 0.f;
		std::uint32_t iLifeMs = 0;
		std::uint32_t iExpectedEventCount = 0;
		BOSS_COMBAT_OBJECT_OWNER_HIT_CHAIN OwnerHitChain;
		std::vector<BOSS_COMBAT_OBJECT_HIT> Hits;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackTemplates;
		std::vector<BOSS_COMBAT_OBJECT_PRESENTATION_PULSE> PresentationPulses;
	};

	enum class BOSS_PATTERN_STAGE_KIND
	{
		WINDUP,
		ACTIVE,
		RECOVERY,
		// 무력화 노출 단계다. 기존 갑옷 내구도 경로가 이 상태를 요구하며 typed 부위 조건은 별도다.
		// 복제 action 상태는 PATTERN_ACTIVE로 연결한다.
		GROGGY,
		// 부위 파괴에 대한 반응 stage다. 복제 action 상태는 PATTERN_RECOVERY로 연결한다.
		PART_BREAK
	};

	// 일반 시간 순서 진행에서 건너뛰고 사건 분기로 진입할 stage 종류인지 판별한다.
	// 실제 벽 충돌/부위 파괴 결과와 다음 action ID의 연결은 Branches가 소유한다.
	// Brain의 순차 진행이 건너뛸 GROGGY/PART_BREAK 단계를 식별한다.
	inline bool Is_EventEnteredStage(const BOSS_PATTERN_STAGE_KIND kind)
	{
		return BOSS_PATTERN_STAGE_KIND::GROGGY == kind ||
			BOSS_PATTERN_STAGE_KIND::PART_BREAK == kind;
	}
	enum class BOSS_PATTERN_CATEGORY : std::uint8_t
	{
		NORMAL,
		IMPORTANT,
		MECHANIC
	};

	enum class BOSS_PATTERN_TARGET_POLICY : std::uint8_t
	{
		NONE,
		NEAREST_EACH_TICK,
		LOCK_NEAREST_ON_START,
		LOCK_RANDOM_ALIVE_ON_START,
		// 패턴 시작 전 보스 방향의 뒤쪽에서 승인 대상을 하나 고른다. 후보가 없으면 일반 생존 대상 풀로 돌아간다.
		LOCK_RANDOM_ALIVE_BEHIND_ON_START
	};

	enum class BOSS_PATTERN_AIM_POLICY : std::uint8_t
	{
		NONE,
		TRACK_TARGET_EACH_TICK,
		LOCK_FACING_ON_START,
		FACE_MOTION_ANCHOR
	};

	enum class BOSS_PATTERN_STAGE_OUTCOME : std::uint8_t
	{
		TIMEOUT,
		COUNTER_HIT,
		STAGGER_BROKEN,
		HEALTH_DAMAGE_THRESHOLD_REACHED,
		WALL_CONTACT,
		PART_DESTROYED,
		PROP_DESTROYED,
		SUMMON_DEAD,
		ALL_PLAYERS_GRABBED,
		ANY_PLAYER_GRABBED,
		NAVIGATION_BLOCKED
	};

	enum class BOSS_PATTERN_BOSS_RESPONSE_KIND : std::uint8_t
	{
		NONE,
		// 방어/무적/보호막 처리 후 실제 감소한 HP를 누적한다. 현재 occurrence의 기준 도달 결과는 한 번만 발행한다.
		ACCUMULATED_HEALTH_DAMAGE
	};

	enum class BOSS_PATTERN_COUNTER_PROXY_KIND : std::uint8_t
	{
		NONE,
		BOSS_LOCAL_CIRCLE,
		// 공격 시전 원점이 보스의 닫힌 전방 반평면에 있는지 검사한다. 저작 각은 180도이며 반지름 제한을 두지 않는다.
		BOSS_FORWARD_ARC
	};

	// stage 결과와 후속 stage/실패/완료를 연결한다. 같은 결과의 실제 소비는 Brain의 분기 선택 순서를 따른다.
	struct BOSS_PATTERN_STAGE_BRANCH final
	{
		BOSS_PATTERN_STAGE_OUTCOME eOutcome =
			BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
		// 같은 패턴의 다음 action ID다. vector index가 아니며 cross-pattern 목표와 동시에 사용하지 않는다.
		std::string strNextActionId;
		// 같은 encounter의 후속 패턴 ID다. 로더가 대상 없는 AUDITION_ONLY 연결을 검증한다.
		// Brain은 현재 종료를 먼저 기록하고 다음 tick에 고정한 정의 세대의 후속을 시작한다.
		std::string strNextPatternId;
	};

	enum class BOSS_PATTERN_STAGE_ACTION_TRIGGER : std::uint8_t
	{
		ENTER,
		EXIT
	};

	enum class BOSS_PATTERN_STAGE_ACTION_KIND : std::uint8_t
	{
		SET_BOSS_FLAG,
		SET_STAGGER_GAUGE,
		SET_SHIELD,
		SET_PLAYER_BIND,
		SET_PLAYER_SILENCE,
		SET_GAMEPLAY_PHASE,
		SPAWN_COMBAT_OBJECT,
		SPAWN_COMBAT_OBJECT_VOLLEY,
		// GameRoom이 기존 대상 선택 경로에서 살아 있는 대상을 다시 잠근다. 현재 pattern 실행을 새로 만들지는 않는다.
		RETARGET_RANDOM_ALIVE,
		// 이 보스에 부착된 플레이어를 해제한다. 해제 모드를 구분하여 단순 해제와 날리기 입력이 섞이지 않게 한다.
		RELEASE_GRABBED_PLAYERS,
		DAMAGE_GRABBED_PLAYERS,
		EXECUTE_GRABBED_PLAYERS,
		RETURN_TO_ARENA_CENTER,
		// FinishPattern이 이번 종료에 예약한 추적 대기만 소모한다. 순서 커서와 Debug 완료 대기는 별도 상태다.
		SUPPRESS_INTER_STEP_PURSUIT
	};

	enum class BOSS_GRABBED_RELEASE_MODE : std::uint8_t
	{
		NONE,
		HOLD,
		OPPOSITE_KNOCKBACK,
		ARENA_EJECTION
	};

	enum class BOSS_COMBAT_OBJECT_VOLLEY_POLICY : std::uint8_t
	{
		NONE,
		PER_ALIVE_PLAYER,
		// 현재 Server 보스 자세를 중심으로 방사형 생성한다. 저작 각도는 보스 yaw에 상대적이다.
		BOSS_RELATIVE,
		// 방 입장 때 보관한 보스 spawn 위치를 아레나 중심으로 쓴다. 각도는 월드 기준이므로 현재 보스 방향을 더하지 않는다.
		ARENA_CENTER
	};

	enum class BOSS_COMBAT_OBJECT_LAYOUT_KIND : std::uint8_t
	{
		SINGLE,
		RADIAL
	};

	enum class BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY : std::uint8_t
	{
		NONE,
		// 방 입장 시 navigation에 투영한 spawn 위치다. 패턴 실행 중 움직이는 현재 보스 위치와 구분한다.
		BOSS_SPAWN_POSITION
	};

	// 한 번의 stage action이 생성할 묶음 배치다. 원점/방향 정책과 발사 간격은 실제 오브젝트 생성 경로가 해석한다.
	struct BOSS_COMBAT_OBJECT_VOLLEY final
	{
		BOSS_COMBAT_OBJECT_VOLLEY_POLICY ePolicy =
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE;
		std::uint32_t iCountPerResolvedTarget = 1u;
		BOSS_COMBAT_OBJECT_LAYOUT_KIND eLayout =
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
		float fRadiusM = 0.f;
		float fStartAngleDegrees = 0.f;
		float fAngleStepDegrees = 0.f;
		bool bAllowOverlap = false;
		std::uint32_t iMaximumTotalObjects = 1u;
		// stage 시계로 예약하는 전체 wave 수다. 첫 offset이 0이면 첫 wave를 stage ENTER에서 생성한다.
		std::uint32_t iSpawnCount = 1u;
		std::uint32_t iFirstSpawnOffsetMs = 0u;
		std::uint32_t iSpawnIntervalMs = 0u;
		std::uint32_t iArenaRandomCount = 0u;
		float fArenaRandomRadiusM = 0.f;
		float fArenaHeightToleranceM = 0.f;
		BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY eArenaAnchorPolicy =
			BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
	};

	// stage ENTER 또는 EXIT에서 GameRoom에 요청할 동작이다. 보스 Brain의 단계 진행과 플레이어/prop 변경 책임을 나눈다.
	struct BOSS_PATTERN_STAGE_ACTION final
	{
		BOSS_PATTERN_STAGE_ACTION_TRIGGER eTrigger =
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
		BOSS_PATTERN_STAGE_ACTION_KIND eKind =
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
		std::string strTargetId;
		std::uint32_t iValue = 0;
		std::uint32_t iDurationMs = 0;
		BOSS_GRABBED_RELEASE_MODE eReleaseMode =
			BOSS_GRABBED_RELEASE_MODE::NONE;
		float fReleaseSpeedMps = 0.f;
		// ARENA_EJECTION에서 보스 방향에 더하는 해제 각도다. 0은 기본 후방 해제이며 HOLD/OPPOSITE_KNOCKBACK은 0을 유지한다.
		float fReleaseYawOffsetDegrees = 0.f;
		BOSS_COMBAT_OBJECT_VOLLEY Volley;
	};

	enum class BOSS_PATTERN_PLAYER_RESPONSE : std::uint8_t
	{
		DAMAGE,
		CAPTURE
	};

	enum class BOSS_PATTERN_HIT_ANCHOR_KIND : std::uint8_t
	{
		BOSS_CURRENT,
		STAGE_ORIGIN
	};

	enum class BOSS_PATTERN_HIT_ACTIVATION_KIND : std::uint8_t
	{
		PULSE_SCHEDULE,
		ACTIVE_WINDOW
	};

	enum class BOSS_PATTERN_PART_DAMAGE_POLICY : std::uint8_t
	{
		NORMAL,
		// 부위 공격력이 양수이면 현재 저작 damage condition에 맞는 첫 부위를 파괴하는 정책이다.
		DESTROY_FIRST_ELIGIBLE
	};

	enum class BOSS_PATTERN_STAGE_MOTION_KIND : std::uint8_t
	{
		NONE,
		FORWARD,
		TO_ARENA_CENTER,
		PORTAL_TARGET_RUSH,
		PORTAL_CROSS_ARENA
	};

	struct BOSS_PATTERN_STAGE_MOTION final
	{
		BOSS_PATTERN_STAGE_MOTION_KIND eKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		std::uint32_t iRetargetDelayMs = 0u;
		float fSpeedMps = 0.f;
		float fDistance = 0.f;
		std::uint32_t iCornerIndex = 0u;
		float fHalfExtentsX = 0.f;
		float fHalfExtentsZ = 0.f;
		// stage의 누적 루트모션 표본이다. 값이 있으면 해당 이동 경로에서 고정 거리 보간보다 표본을 우선 사용한다.
		// 계산한 이동은 GameRoom의 navigation/충돌 검사를 거치므로 표본의 위치를 무조건 확정하지 않는다.
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
	};

	// Kouku의 판정 종류다. 종료 시 판정, 매 tick 접촉, 보스 단위 무력화, 플레이어 입력 등 종류에 따라 처리 시점이 다르다.
	// 종류 값만으로 결과를 적용하지 않고 LogicWindow의 시간/영역/결과 목록을 함께 소비한다.
	enum class BOSS_PATTERN_LOGIC_KIND : std::uint8_t
	{
		CARD_DICE_BIND,
		ROULETTE_CARD_MATCH,
		GAZE_REAL_BOSS,
		POSE_INPUT,
		STAGGER_WINDOW,
		AREA_OVERLAP,
		ENTER_AREA,
		OBJECT_OVERLAP,
		OBJECT_CONTACT,
		EXTERNAL_SIGNAL,
		COUNTER_WINDOW,
        ATTACHMENT_HOLD,
		PATTERN_COMPLETION_COUNT,
		INVULNERABILITY_ZONE,
		BINGO_COMPLETED_LINES,
		BOSS_DAMAGE_REDUCTION
	};

	enum class BOSS_PATTERN_LOGIC_RESULT_KIND : std::uint8_t
	{
		NONE,
		INSTANT_DEATH,
		MAX_HP_PERCENT_DAMAGE,
		MADNESS_GAUGE_ADD_PERCENT,
		CLOWN_TRANSFORM,
		FEAR,
		FOLLOWUP_PATTERN,
		PLAY_WORLD_OBJECT_MOTION,
		PLAY_CONTACT_WORLD_OBJECT_MOTION,
		COMPLETE_LOGIC_WINDOW,
		CAPTURE_PLAYER,
		// 플레이어를 판정한 World Object 영역에 잡아 이동을 따라가게 한다.
		// 게시 검증은 ENTER_AREA의 WorldTrack 영역 연결을 요구하며 단순 정적 영역 포획과 구분한다.
		GRAB_TO_WORLD_OBJECT,
		MARIO_ENTER,
		FIXED_DAMAGE,
		PLAYER_INVULNERABILITY
	};

	struct BOSS_LOGIC_CONTACT_MOTION final
	{
		std::string strTargetWorldOccurrenceId;
		std::string strMotionInstanceId;
	};

	enum class BOSS_LOGIC_PUSH_DIRECTION : std::uint8_t
	{
		AWAY_FROM_BOSS,
		BOSS_FORWARD,
		AWAY_FROM_CONTACT
	};

	// Kouku 판정의 성공/실패/시간 만료 결과 목록 요소다. eKind에 해당하는 값만 결과 적용 경로가 사용한다.
	struct BOSS_PATTERN_LOGIC_RESULT final
	{
		BOSS_PATTERN_LOGIC_RESULT_KIND eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::NONE;
		std::uint32_t iPercent = 0u;
		std::uint32_t iDurationMs = 0u;
		std::string strPatternId;
		std::string strTargetWorldInstanceId;
		std::string strMotionInstanceId;
		std::vector<BOSS_LOGIC_CONTACT_MOTION> ContactMotions;
		std::string strTargetLogicOccurrenceId;
		std::string strContactTargetWorldOccurrenceId;
		std::string strFearPresentationId;
		// MARIO_ENTER에서만 사용한다. 1~4는 명시 stage이고 0은 현재 방의 순번을 따른다.
		std::uint8_t iMarioEntryStage = 0u;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        // 전방/위/오른쪽 m 오프셋이다. 부착 연출 위치를 전달하는 값이다.
        std::array<float, 3u> GripLocalOffset{};
		float fPushRangeM = 0.f;
		float fPushHeightM = 0.f;
		std::uint32_t iPushMs = 0u;
		//Collider를 통해서 보스 기준으로  어느 정도 밀려날 것인지를 정하기 위한 struct BOSS_PATTERN_LOGIC_RESULT의 
		//구조체 변수이다. AWAY_FROM_BOSS의 경우 boss의 위치를 기준으로 플레이어의 방향을 구하는 벡터를 쏘고, 해당
		//벡터를 기준으로 플레이어를 해당하는 방향과 collider에 정해진 속도만큼 날려보낸다. 
		// 이 enum은 밀기 방향을 고른다. 이동 거리 fPushRangeM의 단위는 m, 지속 시간 iPushMs의 단위는 ms다.
		// 방향과 거리/기간을 받은 실제 피격·이동 소비자가 위치 변화를 계산하며 Collider가 속도 값을 직접 소유하지는 않는다.
		BOSS_LOGIC_PUSH_DIRECTION ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_BOSS;
		bool bForcePush = false;
		bool bPushCanLeaveArena = false;
		bool bPushBallistic = false;
		float fPushYawOffsetDegrees = 0.f;
		// 기존 aggregate 초기화 순서를 유지하도록 뒤에 둔 고정 피해량이다.
		std::uint32_t iDamageAmount = 0u;
	};

	// 영역의 기준 자세를 고르는 정책이다. WORLD와 보스 현재/등장/판정창 시작 또는 별도 captureStart 기준을 구분한다.
	enum class BOSS_LOGIC_REGION_ANCHOR : std::uint8_t
	{
		WORLD,
		BOSS_CURRENT,
		BOSS_SPAWN,
		BOSS_START
	};
	// Resolve_LogicRegionTransform이 읽는 track 내부 ms 표본이다. offset은 m, Y/W는 yaw 전용 quaternion 성분이다.
	// offset/scale은 선형 보간하고 회전은 quaternion의 짧은 경로를 따른다. 가시성은 해당 구간의 왼쪽 key 값을 쓴다.
	// GripPosition이 있으면 월드 위치를 직접 보간하며 보스 기준 변환을 다시 적용하지 않는다.
	struct BOSS_LOGIC_WORLD_TRANSFORM_KEY final
	{
		std::uint32_t iTimeMs = 0u;
		float fOffsetX = 0.f,
			fOffsetY = 0.f,
			fOffsetZ = 0.f;
		float fRotationY = 0.f,
			fRotationW = 1.f;
		float fScaleX = 1.f,
			fScaleY = 1.f,
			fScaleZ = 1.f;
		bool bVisible = true;
		bool bHasGripPosition = false;
		// 월드 m 단위로 구운 물리 잡기 지점이다. bHasGripPosition이 있을 때 사용한다.
		std::array<float, 3u> GripPosition{};
	};
	// 판정 영역이 World Object의 저작 이동을 따라가도록 baseline과 key 목록을 보관한다. vector가 표본을 소유한다.
	// bSmoothStep은 보간 비율에 적용한다. baseline scale은 영역 중심/크기에 적용하며 key translation 자체에는 곱하지 않는다.
	// BOSS_CURRENT는 pattern 경과 ms에서 시작/delay를 빼고, 다른 기준은 시작을 30Hz tick으로 올림한 뒤 경과를 계산한다.
	struct BOSS_LOGIC_WORLD_TRANSFORM_TRACK final
	{
		bool bEnabled = false;
		std::uint32_t iStartMs = 0u,
			iStartDelayMs = 0u,
			iDurationMs = 0u;
		float fPlaybackSpeed = 1.f;
		bool bSmoothStep = false;
		float fBaselineX = 0.f,
			fBaselineY = 0.f,
			fBaselineZ = 0.f;
		float fBaselineYawDegrees = 0.f;
		float fBaselineScaleX = 1.f,
			fBaselineScaleY = 1.f,
			fBaselineScaleZ = 1.f;
		std::vector<BOSS_LOGIC_WORLD_TRANSFORM_KEY> Keys;
	};
	// LogicRuntime의 영역 판정 입력이다. 중심/길이는 m, yaw/부채꼴 각도는 degree이며 도형 flag에 맞는 크기만 소비한다.
	// WORLD는 보스 변환을 더하지 않는다. BOSS_CURRENT는 현재 보스 XZ/yaw, BOSS_SPAWN은 등장 XZ 가산만 사용한다.
	// 두 보스 기준의 직접 Y 가산은 cylinder 또는 linearMotion일 때만 수행한다.
	// BOSS_START는 지정 시각에 도달한 호출의 현재 보스 자세와 지정 시각의 track 표본을 조합해 WORLD 도형으로 고정하고 Y도 보완한다.
	struct BOSS_LOGIC_REGION final
	{
		std::string strRegionId;
		BOSS_LOGIC_REGION_ANCHOR eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
		bool bSector = false;
		bool bReverseSector = false;
		bool bCircle = false;
		bool bCylinder = false;
		bool bLinearMotion = false;
		std::uint32_t iMotionStartMs = 0u,
			iMotionDurationMs = 0u;
		// 미설정이면 해당 region 판정창의 시작 시각을 사용한다.
		std::uint32_t iAnchorCaptureStartMs = 0xffffffffu;
		std::array<float, 3u> EndPositionOffset{};
		std::array<float, 3u> EndScale{ 1.f, 1.f, 1.f };
		BOSS_LOGIC_WORLD_TRANSFORM_TRACK WorldTrack;
		float fCenterX = 0.f,
			fCenterY = 0.f,
			fCenterZ = 0.f;
		float fYawDegrees = 0.f;
		float fHalfX = 1.f,
			fHalfY = 1.f,
			fHalfZ = 1.f;
		float fRadiusM = 1.f,
			fInnerRadiusM = 0.f,
			fHalfAngleDegrees = 45.f;
		// 부채꼴의 선택적인 X/Z 반경이다. 0이면 기본 반경을 사용한다.
		float fRadiusXM = 0.f,
			fRadiusZM = 0.f;
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardSymbol = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		LostArk::Shared::MECHANIC_CARD_COLOR eCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;
	};

	// 접촉을 확인할 World occurrence의 stable ID와 월드 XZ 원이다. 표시 instance ID와 개별 occurrence ID를 함께 구분한다.
	struct BOSS_LOGIC_CONTACT_TARGET final
	{
		std::string strWorldOccurrenceId;
		std::string strWorldInstanceId;
		float fWorldX = 0.f,
			fWorldZ = 0.f,
			fRadiusM = 0.f;
	};

	// Workbench에서 게시한 Kouku 판정창 하나다. iStartMs/iDurationMs는 pattern 시계 기준이며 eKind가 사용할 값을 결정한다.
	// 시간·영역·대상과 결과 정의를 담고, 현재 완료 여부/중복 접촉/고정 영역은 LogicRuntime의 ledger가 소유한다.
	struct BOSS_PATTERN_LOGIC_WINDOW final
	{
		std::string strWindowId;
		std::vector<std::string> PatternIds;
		std::uint32_t iCompletionCount = 0u;
		BOSS_PATTERN_LOGIC_KIND eKind = BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iSectorCount = 0u;
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fOuterRadiusM = 0.f;
		float fStopYawDegrees = 0.f;
		std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> SectorSymbols;
		float fHalfAngleDegrees = 0.f;
		float fMaxDistanceM = 0.f;
        bool bGazeDuringWindow = false;
		std::uint32_t iPoseIndex = 0u;
		std::uint32_t iThreshold = 0u;
		float fShieldArcDegrees = 0.f;
		float fNormalYawOffsetDegrees = 0.f;
		std::vector<BOSS_LOGIC_REGION> CardRegions;
		bool bInsideIsFail = false;
		bool bRearmOnExit = false;
		bool bRepeatAfterKnockback = false;
		std::uint32_t iRepeatIntervalMs = 0u;
		bool bEndsPatternOnSuccess = false;
		// 부모 일정이 자식 창을 잘라 끝낸 경우 결과 판정 없이 닫는다.
		bool bCancelAtEnd = false;
		std::string strOwnerWorldOccurrenceId;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnSuccess;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnFail;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnTimeout;
		std::string strTargetWorldInstanceId;
		float fTargetWorldX = 0.f,
			fTargetWorldZ = 0.f;
		float fTargetRadiusM = 0.f;
		std::vector<BOSS_LOGIC_CONTACT_TARGET> ContactTargets;
		std::string strContactGroupId;
		std::uint32_t iContactPriority = 0u;
		bool bHasContactGroup = false;
		float fBossChargeDistanceM = 0.f;
        float fChargeYawOffsetDegrees = 0.f;
        std::string strHoldLogicOccurrenceId;
	};

	enum class BOSS_PATTERN_MECHANIC_TRIGGER_KIND : std::uint8_t
	{
		HUD_ENTER,
		REAL_GAZE_TELEPORT,
		CARD_MAZE_HIDE_NEXT,
		CARD_MAZE_ENTER,
		ALBION_BLUE_CIRCLE,
		SUMMON_PATTERNS,
		SHOWTIME_PLAYER_TARGETS,
		BOSS_TELEPORT_XZ,
		BOSS_TELEPORT_FACE_CENTER,
		MARIO_PHASE2_PLAYERS,
		BOSS_TRACK_TARGET,
		ALBION_AIRBORNE,
		CROSS_DIRECTION_CLONES,
		PURSUIT_PROJECTILES,
		BINGO_BOARD,
		BINGO_DETONATION,
		BOSS_TELEPORT_GROUNDED,
		CARD_MAZE_STAGE_PLAYERS,
		CARD_RAIN_SOLDIERS,
		BOSS_RANDOM_TARGET,
		BOSS_RANDOM_TARGET_PRESENTATION
	};

	enum class ALBION_AIRBORNE_PHASE : std::uint8_t
	{
		NONE,
		JUMP,
		SELECT_PLAYER,
		APPEAR_PLAYER,
		DISAPPEAR,
		CENTER,
		SLAM
	};

	inline constexpr std::size_t KOUKU_SUMMON_MAX_PATTERN_SPAWNS = 16u;
	enum class BOSS_PATTERN_SUMMON_ANCHOR_KIND : std::uint8_t
	{
		BOSS,
		MAP
	};

	struct BOSS_PATTERN_SUMMON_PATTERN_SPAWN final
	{
		std::string strSpawnId;
		std::string strPatternId;
		BOSS_PATTERN_SUMMON_ANCHOR_KIND eAnchorKind = BOSS_PATTERN_SUMMON_ANCHOR_KIND::BOSS;
		// MAP은 절대 월드 위치/방향, BOSS는 소유자 기준 위치/방향으로 해석한다.
		std::array<float, 3u> PositionOffset{};
		float fYawOffsetDegrees = 0.f;
	};

	// Showtime 무작위 발사 묶음의 판정 template과 Client visual ID다. 표시 수명과 실제 명중 여부는 별도 상태로 처리한다.
	struct BOSS_SHOWTIME_RANDOM_VOLLEY final
	{
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> Hits;
		std::string strClientVisualId;
		std::uint32_t iLifetimeMs = 0u;
	};

	// 고정 폭탄 하나를 소유자의 최종 추적 부채꼴과 한 번 대조하기 위한 정의다.
	struct BOSS_SHOWTIME_BOMB_RESOLUTION final
	{
		std::uint32_t iBodyStartMs = 0u;
		std::string strBodyVisualId,
			strExplosionVisualId;
		std::uint32_t iExplosionLifetimeMs = 0u;
		std::array<float, 3u> BodyPosition{};
		BOSS_LOGIC_REGION FanRegion;
	};

	// 빙고 폭탄의 표식/낙하/폭발 일정을 ms로 저장한다. 로더가 아래 범위와 30Hz 변환 뒤의 슬롯 상한을 함께 검사한다.
	struct BOSS_BINGO_BOARD_SETTINGS final
	{
		bool bWindowOnly = false;
		std::uint32_t iFirstBombDelayMs = 30000u,
			iBombIntervalMs = 20000u;
		std::uint32_t iBombMarkMs = 6000u,
			iBombDropDelayMs = 2000u,
			iBombFuseMs = 4000u;
		std::uint32_t iInitialMarkedCells = 2u;
		// 값만 검사하고 상태를 변경하지 않는다. ms 합과 올림한 tick 합을 모두 검사하여 네 interval 범위에 들어오는지 확인한다.
		[[nodiscard]] bool Is_Valid() const noexcept
		{
			const auto ticks = [](const std::uint32_t ms)
			{
				return (std::uint64_t(ms) * 30u + 999u) / 1000u;
			};
			return iFirstBombDelayMs <= 600000u &&
				iBombIntervalMs && iBombIntervalMs <= 600000u &&
				iBombMarkMs && iBombMarkMs <= 600000u &&
				iBombDropDelayMs && iBombDropDelayMs <= 600000u &&
				iBombFuseMs >= 250u && iBombFuseMs <= 80000u &&
				iInitialMarkedCells <= 25u &&
				std::uint64_t(iBombMarkMs) + iBombDropDelayMs + iBombFuseMs <= std::uint64_t(iBombIntervalMs) * 4u &&
				ticks(iBombMarkMs) + ticks(iBombDropDelayMs) + ticks(iBombFuseMs) <= ticks(iBombIntervalMs) * 4u;
		}
	};

	// pattern 시계의 iStartMs에 GameRoom이 실행할 기믹 요청이다. eKind에 따라 HUD/이동/소환/빙고 입력을 선택한다.
	// 이 정의는 요청 데이터이며 현재 대상 선택, 생성 entity, 예약 횟수는 방의 실행 상태가 관리한다.
	// 더 못 쪼갤까? 왜 이렇게 구성이 되어야만 할까?
	struct BOSS_PATTERN_MECHANIC_TRIGGER final
	{
		std::string strTriggerId;
		BOSS_PATTERN_MECHANIC_TRIGGER_KIND eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		float fTeleportX = 0.f;
		float fTeleportY = 0.f;
		float fTeleportZ = 0.f;
		// 게시자가 MAP 이펙트 occurrence에서 해석한 순서별 월드 진입 목적지다.
		std::vector<std::array<float, 3u>> PlayerEntryPositions;
		std::string strClonePatternId;
		std::vector<std::uint32_t> ClockHours;
		float fFaceCenterYawOffsetDegrees = 0.f;
		std::uint32_t iCountPerPlayer = 0u;
		float fPlayerEffectRadiusM = 0.f;
		std::uint32_t iEffectLifetimeMs = 0u;
		std::uint32_t iArenaRandomCount = 0u;
		float fArenaRandomRadiusM = 0.f;
		float fArenaHeightToleranceM = 0.f;
		float fArenaMinimumSpacingM = 0.f;
		bool bRandomPlayerOnly = false;
		// 게시된 시각 그룹의 stable ID다. Server는 실제 이펙트 asset 경로를 해석하지 않는다.
		std::string strFixedVisualId;
		std::string strTrackingVisualId;
		std::uint32_t iFixedLifetimeMs = 0u;
		std::uint32_t iSpawnIntervalMs = 0u;
		float fFollowSpeedScale = 0.f;
		std::vector<BOSS_SHOWTIME_RANDOM_VOLLEY> RandomVolleys;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> FixedHits,
			TrackingHits,
			ProjectileHits;
		std::uint32_t iRandomSpawnIntervalMs = 0u;
		float fRandomArenaRadiusM = 0.f,
			fRandomArenaHeightToleranceM = 0.f;
		std::string strRandomAnchorKind = "BOSS_SPAWN";
		float fRandomScaleMin = 1.f,
			fRandomScaleMax = 1.f;
		ALBION_AIRBORNE_PHASE eAirbornePhase = ALBION_AIRBORNE_PHASE::NONE;
		float fAirborneHeightM = 0.f;
		std::uint32_t iAirborneDurationMs = 0u;
		bool bCaptureAirborneTargetPosition = false;
		std::string strSelectedEffectVisualId;
		std::uint32_t iSelectedEffectLifetimeMs = 0u;
		std::vector<BOSS_PATTERN_SUMMON_PATTERN_SPAWN> PatternSpawns;
		std::vector<std::string> DirectionPatternIds;
		std::string strCloneEndStageId;
		std::string strRealPatternId;
		std::vector<std::string> ProjectileVisualIds;
		std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> ProjectileCardSymbols;
		std::string strContactVisualId;
		// 0이면 거리 상한을 두지 않고 수명에 따른 이동 종료를 사용한다.
		float fProjectileMaxDistanceM = 0.f;
		float fProjectileSpeedMps = 0.f,
			fProjectileContactRadiusM = 0.f,
			fProjectileSpawnRadiusM = 0.f;
		std::uint32_t iProjectileLifetimeMs = 0u,
			iProjectileCountPerWave = 0u;
		bool bProjectileHoming = false;
		std::array<std::uint32_t, 3u> SoldierCounts{ 1u, 1u, 1u };
		float fSoldierSpawnRadiusMinM = 3.f,
			fSoldierSpawnRadiusMaxM = 6.f;
		// 0이면 profile 기본값을 사용한다. 양수 고정 피해는 방어 계산을 우회하며 다른 보호 효과는 공용 피격 경로가 처리한다.
		std::uint32_t iSoldierMaxHp = 0u,
			iSoldierDamage = 0u;
		bool bHasSoldierTuning = false;
		std::optional<BOSS_SHOWTIME_BOMB_RESOLUTION> ShowtimeBomb;
		// 저작 Object scale을 반영한 월드 m 반길이다. 경로 전방/가로 두 축을 저장한다.
		std::optional<std::array<float, 2u>> BingoHammerHalfExtentsM;
		std::optional<BOSS_BINGO_BOARD_SETTINGS> BingoBoardSettings;
		// 게시자가 animation/다른 활성 lane이 없는 빙고 제어 전용 패턴인지 검증하여 전달한 값이다.
		bool bBingoControlOnly = false;
	};

	// World sequence가 플레이어를 받칠 수 있는 tick 구간과 월드 XZ 원/높이 정의다. 표시 event ID와는 다른 서버 판정 입력이다.
	struct BOSS_PATTERN_WORLD_SUPPORT_WINDOW final
	{
		std::uint32_t iStartOffsetTicks = 0u;
		std::uint32_t iEndOffsetTicks = 0u;
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fHeightY = 0.f;
		float fRadiusM = 0.f;
	};

	// World sequence의 명시 배치 변환이다. 위치는 월드 m, 회전은 degree, scale은 축별 배율이다.
	struct BOSS_PATTERN_WORLD_PLACEMENT final
	{
		float fPositionX = 0.f,
			fPositionY = 0.f,
			fPositionZ = 0.f;
		float fRotationXDegrees = 0.f,
			fRotationYDegrees = 0.f,
			fRotationZDegrees = 0.f;
		float fScaleX = 1.f,
			fScaleY = 1.f,
			fScaleZ = 1.f;
	};

	// World 표시와 함께 Server가 만들 전투 몸체의 HP/중심/반지름이다. 실제 entity ID와 현재 HP는 실행 인스턴스에 둔다.
	struct BOSS_PATTERN_WORLD_COMBAT_BODY final
	{
		std::uint32_t iMaximumHp = 0u;
		float fCenterX = 0.f,
			fCenterY = 0.f,
			fCenterZ = 0.f;
		float fRadiusM = 0.f;
	};

	// pattern 시계에 맞춰 Client World 표시를 요청하고 선택적으로 Server 받침/전투 몸체 정의를 연결한다.
	// SupportWindows의 offset tick은 이 cue의 시작 tick 기준이다. 받침 위치는 position offset과 선택적인 spawn-anchor 차이를 더한다.
	struct BOSS_PATTERN_WORLD_SEQUENCE final
	{
		std::uint32_t iDurationMs = 0u;
		std::string strInstanceId;
		std::string strOccurrenceId;
		std::uint32_t iStartMs = 0u;
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		bool bAnchorBossSpawn = false;
		float fAnchorPositionX = 0.f;
		float fAnchorPositionY = 0.f;
		float fAnchorPositionZ = 0.f;
		std::vector<BOSS_PATTERN_WORLD_SUPPORT_WINDOW> SupportWindows;
		bool bAuthoredMadness = false;
		std::optional<BOSS_PATTERN_WORLD_COMBAT_BODY> CombatBody;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};

	// pattern 일정에 연결한 Client scene profile ID와 유지/전환 ms다. Server는 렌더링 자원을 직접 해석하지 않는다.
	struct BOSS_PATTERN_SCENE_PROFILE final
	{
		std::string strProfileId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iBlendMs = 0u;
	};

	// encounter의 광기 정책이다. 최대값 0은 기본값을 유지하고 hold ms는 광대 형태 유지 시간을 정한다.
	struct BOSS_ENCOUNTER_MADNESS_POLICY final
	{
		std::uint32_t iMaximum = 0u;
		std::uint32_t iClownHoldMs = 0u;
		std::uint32_t iDamageGainPercent = 100u;
		std::uint32_t iBallGainPercent = 10u,
			iBallMultiplierPercent = 200u;
		float fBallRadiusM = 2.f;
		std::uint32_t iDollGainPercent = 10u,
			iDollMultiplierPercent = 200u;
		float fDollRadiusM = 4.f;
		std::uint32_t iSpecialIntervalMs = 1000u;
	};

	// Brain이 순서/분기로 진입할 단계 정의다. 타격·이동·카운터·결과 연결을 보관하고 경과 시간/소비 횟수는 보스 실행 상태에 둔다.
	struct BOSS_PATTERN_STAGE_DEFINITION
	{
		std::string strStageId;
		std::string strActionId;
		std::string strDamageProfileId;
		BOSS_PATTERN_STAGE_KIND eStageKind =
			BOSS_PATTERN_STAGE_KIND::WINDUP;
		BOSS_PATTERN_HIT_SHAPE eHitShape =
			BOSS_PATTERN_HIT_SHAPE::NONE;
		// DAMAGE는 일반 타격이다. CAPTURE도 타격을 먼저 처리한 뒤 살아 있는 대상의 typed 부착을 GameRoom에 요청한다.
		BOSS_PATTERN_PLAYER_RESPONSE ePlayerResponse =
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		BOSS_PATTERN_PART_DAMAGE_POLICY ePartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		// 카운터 허용 stage에서 시전자 원점을 추가로 제한한다. 원형 offset은 보스 전방/오른쪽 m 기준이다.
		// 전방 반평면 정책은 arc 각도만 사용한다.
		bool bHasCounterProxy = false;
		BOSS_PATTERN_COUNTER_PROXY_KIND eCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
		float fCounterProxyForwardOffsetM = 0.f;
		float fCounterProxyRightOffsetM = 0.f;
		float fCounterProxyRadiusM = 0.f;
		float fCounterProxyArcDegrees = 0.f;
		BOSS_PATTERN_BOSS_RESPONSE_KIND eBossResponseKind =
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE;
		std::uint32_t iBossResponseThreshold = 0u;
		// stage에만 적용하는 Y 오프셋이다. 다음 stage 경계에서 복원하며 pattern 단위 오프셋과 별도로 관리한다.
		float fVerticalOffsetM = 0.f;
		std::uint32_t iDurationMs = 0;
		float fHitOuterRadius = 0.f;
		float fHitInnerRadius = 0.f;
		float fHitAngleDegrees = 0.f;
		float fHitLength = 0.f;
		float fHitHalfWidth = 0.f;
		std::uint32_t iHitCount = 0;
		std::uint32_t iHitIntervalMs = 0;
		// stage 시작부터 첫 타격까지의 ms다. 균등 반복 경로는 이 값에서 iHitIntervalMs 간격으로 진행한다.
		std::uint32_t iHitDelayMs = 0;
		// 균등 간격으로 표현하지 못하는 타격의 stage 기준 ms 목록이다. 비어 있으면 delay/count/interval 경로를 사용한다.
		std::vector<std::uint32_t> HitOffsetsMs;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackContacts;
		// 판정 원점을 Effect 연출과 분리한다. 기본값은 현재 보스 자세이며 STAGE_ORIGIN은 stage 진입 자세를 사용한다.
		BOSS_PATTERN_HIT_ANCHOR_KIND eHitAnchorKind =
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		float fHitAnchorForwardOffsetM = 0.f;
		float fHitAnchorRightOffsetM = 0.f;
		float fHitAnchorYawOffsetDegrees = 0.f;
		// ACTIVE_WINDOW는 활성 구간 매 tick에 접촉을 평가하고 허용된 대상을 기록한다.
		// PULSE_SCHEDULE은 count/delay/offset 일정의 판정 기회를 소비한다.
		BOSS_PATTERN_HIT_ACTIVATION_KIND eHitActivationKind =
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		std::uint32_t iHitActivationStartMs = 0u;
		std::uint32_t iHitActivationLifetimeMs = 0u;
		// 이 stage 타격의 밀기 거리 m와 기간 ms다. 음수 거리는 판정 원점 쪽으로 당긴다.
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0;
		// 공격 도형을 벽 파괴 contact 후보로 사용할지 정한다. 모든 HP 피해가 벽을 부수는 것은 아니다.
		bool bWallContact = false;
		// GameRoom이 이 stage의 돌진 벽 접촉을 검사하도록 한다. 실제 이동은 Motion/RootMotion 경로가 계산한다.
		// 충돌 뒤 stage는 저작된 WALL_CONTACT 분기에서 결정하며 반드시 바로 다음 GROGGY라는 뜻은 아니다.
		bool bChargeImpact = false;
		// 현재 stage가 encounter prop 엄폐 검사를 건너뛸지 정한다. 이 flag의 허용 여부는 로더의 저작 계약이 검사한다.
		bool bPiercesCover = false;
		BOSS_PATTERN_STAGE_MOTION Motion;
		// stage에만 적용할 추적/조준 종료 조건이다. 설정되지 않은 항목은 pattern의 기본 조준 정책을 따른다.
		bool bTrackNearestTarget = false;
		bool bTrackPatternTarget = false;
		bool bHasAimEnd = false;
		std::uint32_t iAimEndMs = 0u;
		bool bHasAimResponseScale = false;
		float fAimResponseScale = 1.f;
		// 이 stage 진입에서 부술 encounter prop set/slot ID다. prop은 생성한 패턴보다 오래 살 수 있어 파괴 연결을 별도로 저장한다.
		std::string strPropBreakSetId;
		std::vector<std::string> PropBreakSlotIds;
		std::vector<BOSS_PATTERN_STAGE_BRANCH> Branches;
		std::vector<BOSS_PATTERN_STAGE_ACTION> Actions;
	};

	enum class BOSS_PATTERN_MOTION_KIND : std::uint8_t
	{
		NONE,
		LEAP_TO_ANCHOR,
		// 저장해 둔 마지막 대상 위치가 있으면 그 위치를 착지점으로 사용하고 없으면 저작 anchor로 돌아간다.
		LEAP_TO_TARGET
	};

	// Server가 계산할 패턴 단위 도약의 기준 위치와 시간 정의다. stage 일반 이동과 별도로 소비한다.
	// 착지 X/Y/Z는 월드 m, 꼭대기 높이는 기준 Y에 더하는 m다.
	struct BOSS_PATTERN_MOTION
	{
		BOSS_PATTERN_MOTION_KIND eKind = BOSS_PATTERN_MOTION_KIND::NONE;
		std::string strAnchorId;
		float fLandingX = 0.f;
		float fLandingY = 0.f;
		float fLandingZ = 0.f;
		float fApexHeight = 0.f;
		// 하강 이동을 맡은 stage index다. 이륙과 이 stage 사이 구간은 꼭대기 높이를 유지한다.
		std::uint32_t iTravelStageIndex = 1u;
		// 이륙/이동 stage 안에서 도약이 실제로 진행되는 ms 구간이다. 예고/회복 시간을 도약 길이와 구분한다.
		std::uint32_t iTakeoffStartMs = 0u;
		std::uint32_t iTakeoffEndMs = 0u;
		std::uint32_t iTravelStartMs = 0u;
		std::uint32_t iTravelEndMs = 0u;
		bool bMoveToAnchorBeforeTakeoff = false;
	};

	enum class BOSS_PATTERN_ROTATION_SELECTION_MODE : std::uint8_t
	{
		// Candidates를 선택 풀로 사용하며 후보별 weight/enabled를 유지한다.
		// 거리/phase/갑옷/재사용 대기/연속 사용 제한은 pattern 정의와 함께 검사한다.
		WEIGHTED_POOL,
		// 저작 순서의 각 항목을 한 번씩 도입한 뒤 encounter의 가중치 선택으로 연결한다.
		ORDERED_INTRO_THEN_WEIGHTED,
		// 해당 체력 구간에서 PatternIds의 순번을 반복한다. 같은 ID가 여러 번 있어도 순번을 합치지 않는다.
		ORDERED_LOOP
	};

	// 관리되는 가중 pool의 패턴 ID와 선택 가중치다. 실행 당시 거리/재사용 등의 조건은 Brain이 추가 검사한다.
	struct BOSS_PATTERN_ROTATION_CANDIDATE final
	{
		std::string strPatternId;
		std::uint32_t iSelectionWeight = 0u;
		bool bEnabled = false;
	};

	// 특정 gameplay phase와 체력줄 구간에 속하는 rotation의 선택 창이다. 실제 포함 비교는 Catalog 조회 함수가 수행한다.
	struct BOSS_PATTERN_ROTATION_WINDOW final
	{
		std::string strWindowId;
		std::string strSelectionSetId;
		std::uint32_t iGameplayPhase = 0u;
		std::uint32_t iFromHealthBar = 0u;
		std::uint32_t iToHealthBar = 0u;
		std::uint32_t iExpectedCandidateCount = 0u;

		// window ID의 존재만 검사한다. phase/체력줄/후보 수가 올바른지는 bootstrap 로더의 별도 검증 책임이다.
		[[nodiscard]] bool Is_Defined() const noexcept
		{
			return !strWindowId.empty();
		}
	};

	// 체력 기믹 사이의 일반 패턴 선택 구간이다. 체력줄이 감소하므로 높은 from에서 낮은 to 방향으로 해석한다.
	struct BOSS_PATTERN_ROTATION_DEFINITION
	{
		std::string strEncounterId;
		std::string strRotationId;
		BOSS_PATTERN_ROTATION_SELECTION_MODE eSelectionMode =
			BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_INTRO_THEN_WEIGHTED;
		std::uint32_t iFromHealthBar = 0;
		std::uint32_t iToHealthBar = 0;
		std::uint32_t iExpectedStepCount = 0;
		// WEIGHTED_POOL은 Window/Candidates를, 순서형 모드는 PatternIds를 사용한다. 로더가 모드별 필드 조합을 검사한다.
		BOSS_PATTERN_ROTATION_WINDOW Window;
		std::vector<BOSS_PATTERN_ROTATION_CANDIDATE> Candidates;
		std::vector<std::string> PatternIds;
	};

	enum class BOSS_PATTERN_SEQUENCE_MODE : std::uint8_t
	{
		// 저작 항목을 순서대로 한 번 소비하고 마지막 뒤에는 대기한다. 일반 가중치 선택으로 자동 복귀하지 않는다.
		ORDERED_ONCE_THEN_IDLE,
		// sequence는 Play All용으로 유지하고 자동 전투는 체력 기믹과 rotation을 사용한다.
		HEALTH_BAR_ROTATIONS
	};

	// 순서가 있는 패턴 ID와 사이 추적 시간을 소유한다. eMode가 자동 순서 실행 또는 체력줄/rotation 사용을 구분한다.
	struct BOSS_PATTERN_SEQUENCE_DEFINITION final
	{
		std::string strEncounterId;
		std::string strSequenceId;
		// 자동 시작에서 기존 intro 전에 선택할 수 있는 입장 연출 패턴 ID다.
		std::string strEntranceCinematicPatternId;
		BOSS_PATTERN_SEQUENCE_MODE eMode =
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
		std::uint32_t iInterStepPursuitMs = 0u;
		std::uint32_t iInterStepPursuitTicks = 0u;
		std::uint32_t iExpectedStepCount = 0u;
		std::vector<std::string> PatternIds;
		// 마지막을 제외한 패턴 occurrence 뒤의 추적 대기다. 로더는 이전 단일 기간 형식도 항목별 목록으로 확장한다.
		std::vector<std::uint32_t> TransitionPursuitMs;
		std::vector<std::uint32_t> TransitionPursuitTicks;
	};

	enum class BOSS_PATTERN_FINALE_KIND : std::uint8_t
	{
		NONE,
		GHOST_PORTAL_LOOP
	};

	// 패턴이 시작할 피날레 보조 보스/portal 생성 정책이다. 동시에 살아 있는 개체와 다음 생성 tick은 실행 상태에서 관리한다.
	struct BOSS_PATTERN_FINALE final
	{
		BOSS_PATTERN_FINALE_KIND eKind = BOSS_PATTERN_FINALE_KIND::NONE;
		std::string strGhostArchetypeId;
		std::vector<std::string> GhostPatternIds;
		float fSpawnHalfExtentsX = 0.f;
		float fSpawnHalfExtentsZ = 0.f;
		std::uint32_t iMaximumActiveGhosts = 0u;
		// 0이면 보조 유령이 사라진 다음 fixed tick의 기존 보충 경로를 사용한다.
		std::uint32_t iAuxiliarySpawnIntervalMs = 0u;
		std::uint32_t iPortalSpawnIntervalMs = 7900u;
	};

	// 묶음 실행에서 누가 어떤 패턴을 언제 시작할지 지정한다. member ID는 같은 패턴의 서로 다른 실행을 구분한다.
	struct BOSS_PATTERN_BUNDLE_MEMBER final
	{
		std::string strMemberId;
		std::string strPatternId;
		std::string strTargetBossPlacementId;
		std::uint32_t iStartOffsetMs = 0u;
	};
	// 관문/encounter에 속한 병렬 패턴 묶음 정의다. GameRoom의 member scheduler가 목록과 시작 offset을 소비한다.
	struct BOSS_PATTERN_BUNDLE_DEFINITION final
	{
		std::string strBundleId;
		std::string strEncounterId;
		std::string strGateId;
		std::vector<BOSS_PATTERN_BUNDLE_MEMBER> Members;
	};


	// 관문 진행 목록의 한 항목이다. bBundle에 따라 target ID를 패턴 또는 묶음으로 해석하고 완료 뒤 대기 시간을 적용한다.
	struct KOUKU_RAID_FLOW_ENTRY final
	{
		std::string strEntryId,
			strTargetId;
		bool bBundle = false;
		std::uint32_t iWaitAfterMs = 0u;
	};
	// 관문 진입/종료 연출의 플레이어 도착 위치다. 슬롯과 pattern 시계의 시작 ms를 보관한다.
	struct KOUKU_RAID_ARRIVAL final
	{
		std::string strOccurrenceId;
		bool bClear = false;
		std::uint32_t iSlot = 0u,
			iStartMs = 0u;
		std::array<float, 3u> Position{};
	};
	// 연속된 항목 범위를 체력줄 조건까지 반복한다. bTransitionAtGroupEnd는 조건을 만족해도 그룹 끝까지 진행할지 정한다.
	struct KOUKU_RAID_FLOW_GROUP final
	{
		std::string strGroupId;
		std::uint32_t iFirstEntry = 0u,
			iLastEntry = 0u;
		std::optional<std::uint32_t> RepeatUntilHealthBars;
		bool bTransitionAtGroupEnd = false;
	};
	// GameRoom의 Kouku 관문 진행이 읽는 전체 일정이다. 아래 조회 함수는 정의를 바꾸지 않고 다음 index만 계산한다.
	struct KOUKU_RAID_GATE_DEFINITION final
	{
		std::string strEncounterId,
			strGateId,
			strFlowId,
			strSequenceCompositionId;
		std::string strIntroPatternId,
			strClearPatternId,
			strPrimaryBossPlacementId;
		std::string strEntrySequenceInstanceId;
		std::string strLoopStartEntryId;
		std::string strBingoSpecialPatternId;
		std::uint32_t iSequenceRevision = 0u,
			iIntroDurationMs = 0u,
			iClearDurationMs = 0u,
			iExpectedEntryCount = 0u;
		std::vector<KOUKU_RAID_FLOW_ENTRY> Entries;
		std::vector<KOUKU_RAID_FLOW_GROUP> EntryGroups;
		std::vector<KOUKU_RAID_ARRIVAL> Arrivals;
		// 아직 시작하지 않은 그룹의 첫 항목에서 체력 조건이 이미 충족되면 그 범위를 건너뛴다.
		// HP 비율은 정수 곱으로 비교하고, 반환값 Entries.size()는 다음 항목이 없다는 표식이다.
		std::uint32_t Resolve_ReadyEntry(std::uint32_t current, std::uint32_t hp,
			std::uint32_t maximumHp, std::uint32_t maximumBars) const noexcept
		{
			// 큰 피해가 여러 체력 경계를 넘겨도 아직 시작하지 않은 반복 구간만 건너뛴다.
			// 현재 index가 그룹의 첫 항목일 때만 넘기므로 사이의 독립 기믹 항목을 통째로 생략하지 않는다.
			for (const auto& group : EntryGroups)
				if (group.RepeatUntilHealthBars &&
					current == group.iFirstEntry && maximumHp && maximumBars &&
					static_cast<std::uint64_t>(hp) * maximumBars <= static_cast<std::uint64_t>(*group.RepeatUntilHealthBars) * maximumHp)
					current = group.iLastEntry + 1u;
			return current < Entries.size() ?
				current : static_cast<std::uint32_t>(Entries.size());
		}
		// GameRoom이 현재 occurrence와 동적 후속 실행을 모두 끝낸 뒤 다음 순번을 계산할 때 호출한다.
		std::uint32_t Resolve_NextCompletedEntry(std::uint32_t current, std::uint32_t hp,
			std::uint32_t maximumHp, std::uint32_t maximumBars) const noexcept
		{
			if (current >= Entries.size())
				return static_cast<std::uint32_t>(Entries.size());
			for (const auto& group : EntryGroups)
			{
				if (!group.RepeatUntilHealthBars ||
					current < group.iFirstEntry || current > group.iLastEntry)
					continue;
				const bool reached = maximumHp && maximumBars &&
					static_cast<std::uint64_t>(hp) * maximumBars <= static_cast<std::uint64_t>(*group.RepeatUntilHealthBars) * maximumHp;
				if (reached && (!group.bTransitionAtGroupEnd || current == group.iLastEntry))
					return Resolve_ReadyEntry(group.iLastEntry + 1u, hp, maximumHp, maximumBars);
				if (current == group.iLastEntry)
					return group.iFirstEntry;
				break;
			}
			return Resolve_ReadyEntry(current + 1u, hp, maximumHp, maximumBars);
		}
	};

	// BossMotion 경로의 pattern 기준 ms/월드 위치 표본이다. KoukuBrain이 인접 표본을 보간해 실제 보스 위치를 갱신한다.
	struct BOSS_PATTERN_BOSS_MOTION_KEY
	{
		std::uint32_t iTimeMs = 0u;
		std::array<float, 3u> Position{};
	};

	// Kouku 보스의 pattern 기준 시작/끝 ms와 월드 이동 정의다. key가 있는 경우 중간 경로도 함께 보관한다.
	struct BOSS_PATTERN_BOSS_MOTION
	{
		std::uint32_t iStartMs = 0u;
		std::uint32_t iEndMs = 1u;
		std::array<float, 3u> StartPosition{};
		std::array<float, 3u> EndPosition{};
		float fYawDegrees = 0.f;
		std::vector<BOSS_PATTERN_BOSS_MOTION_KEY> Keys;
	};

    // 보존된 Parent가 member scheduler에 예약하는 자식 occurrence다. ID/시작/기간을 유지하고 animation stage로 강제 합치지 않는다.
    struct BOSS_PATTERN_PARENT_CHILD final
    {
        std::string strOccurrenceId;
        std::string strPatternId;
        std::uint32_t iStartMs = 0u;
        std::uint32_t iDurationMs = 0u;
    };

	// encounter별 보스 패턴 하나의 읽기 전용 정의다. 선택 조건, stage 목록과 Kouku의 독립 저작 lane을 한 stable pattern ID로 연결한다.
	// 현재 선택/단계/타격 소비/기믹 결과는 각 Brain과 GameRoom 실행 상태에 두므로 이 정의와 수명을 구분한다.
	struct BOSS_PATTERN_DEFINITION
	{
		std::string strEncounterId;
		std::string strPatternId;
		std::string strGateId;
		std::string strTargetBossPlacementId;
		std::string strActionId;
		bool bAuthoringMasterManaged = false;
		BOSS_PATTERN_MOTION Motion;
		BOSS_PATTERN_FINALE Finale;
		// 패턴 진입 기준 Y에 한 번 더하는 값이다. 종료 정리에서 stage 오프셋과 분리해 복원한다.
		float fVerticalOffsetM = 0.f;
		BOSS_PATTERN_SELECTION eSelection = BOSS_PATTERN_SELECTION::NORMAL;
		BOSS_PATTERN_ARMOR_REQUIREMENT eArmorRequirement =
			BOSS_PATTERN_ARMOR_REQUIREMENT::ANY;
		BOSS_PATTERN_PHASE_REQUIREMENT ePhaseRequirement =
			BOSS_PATTERN_PHASE_REQUIREMENT::ANY;
		// 이 패턴 실행 동안 외부 player hit 경로가 보스 무적을 적용하도록 하는 정책이다.
		bool bInvulnerableWhileRunning = false;
		BOSS_PATTERN_CATEGORY eCategory = BOSS_PATTERN_CATEGORY::NORMAL;
		BOSS_PATTERN_TARGET_POLICY eTargetPolicy =
			BOSS_PATTERN_TARGET_POLICY::NONE;
		BOSS_PATTERN_AIM_POLICY eAimPolicy = BOSS_PATTERN_AIM_POLICY::NONE;
		std::uint32_t iMinimumPhase = 1;
		std::uint32_t iMaximumPhase = 1;
		std::uint32_t iMinimumHealthBar = 0;
		std::uint32_t iMaximumHealthBar = 0;
		std::uint32_t iTriggerHealthBar = 0;
		std::uint32_t iTriggerOrder = 0;
		std::uint32_t iSelectionWeight = 0;
		std::uint32_t iMaximumConsecutiveUses = 0;
		float fMinimumRange = 0.f;
		float fMaximumRange = 0.f;
		// Valtan 원본 주 action의 metadata다. 원본 단위 범위와 Server가 m 단위로 조정한 선택 범위는 서로 다른 값이다.
		std::uint32_t iSourcePrimaryActionId = 0;
		std::uint32_t iSourceShapeCount = 0;
		std::uint32_t iSourceCooldownMs = 0;
		std::uint32_t iSourceCooldownTicks = 0;
		std::uint32_t iSourceRangeUnits = 0;
		std::uint32_t iSourceApproachUnits = 0;
		std::uint32_t iSourceTurnDegrees = 0;
		std::uint32_t iExpectedStageCount = 0;
		// Kouku audition에서 이 패턴을 재생할 수 있는 보스 archetype 목록이다. 실제 살아 있는 대상 승인은 GameRoom이 수행한다.
		std::vector<std::string> AuditionBossArchetypeIds;
		std::vector<BOSS_PATTERN_STAGE_DEFINITION> Stages;
		// stage 시계 옆에서 Kouku Logic/기믹/World/Scene 연출을 평가할 게시 정의다. 실제 진행 상태는 entity의 ledger가 소유한다.
		std::vector<BOSS_PATTERN_LOGIC_WINDOW> LogicWindows;
		bool bResetBossToSpawn = false;
		bool bFixedTimelineClock = false;
        // 0이면 stage 길이 합을 사용한다. 독립 occurrence의 끝이 더 늦다고 stage 시간을 늘리지는 않는다.
        std::uint32_t iTimelineDurationMs = 0u;
        // 보존한 Parent의 자식 목록이다. 확장한 animation stage 대신 기존 멤버 예약 경로로 실행한다.
        std::vector<BOSS_PATTERN_PARENT_CHILD> ParentChildren;
        std::string strParentLoopStartOccurrenceId;
		std::optional<float> ResetBossYawDegrees;
		std::optional<BOSS_PATTERN_BOSS_MOTION> BossMotion;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<BOSS_PATTERN_WORLD_SEQUENCE> WorldSequences;
		std::vector<BOSS_PATTERN_SCENE_PROFILE> SceneProfiles;
	};

	enum class BOSS_PART_DAMAGE_CONDITION : std::uint8_t
	{
		ALWAYS,
		GROGGY_ONLY
	};

	// 보스의 typed 부위 정의다. entity의 현재 내구도/파괴 상태와 분리하며 GROGGY 제한은 각 부위의 eDamageCondition으로 정한다.
	struct BOSS_PART_DEFINITION final
	{
		std::string strBossArchetypeId;
		std::string strPartId;
		std::uint32_t iStateMask = 0;
		std::uint32_t iMaximumDurability = 0;
		std::uint32_t iDamageReductionPercent = 0;
		BOSS_PART_DAMAGE_CONDITION eDamageCondition =
			BOSS_PART_DAMAGE_CONDITION::ALWAYS;
	};

	enum class VALTAN_TIMELINE_ARENA_STATE : std::uint8_t
	{
		FRESH,
		ORDINARY_WALLS_GONE,
		ALL_WALLS_GONE,
		FLOOR84_GONE,
		FLOOR84_AND_30_GONE,
		// Debug 프리셋이다. 제품 진행에서는 84줄 이후 30줄 지형 상태로 넘어간다.
		FLOOR30_GONE
	};

	enum class VALTAN_TIMELINE_PROP_STATE : std::uint8_t
	{
		HIDDEN,
		FOUR_PILLARS_INTACT
	};

	enum class VALTAN_TIMELINE_ENTRY_TYPE : std::uint8_t
	{
		MECHANIC,
		NORMAL
	};

	// Timeline의 한 행에서 실행할 pattern ID와 반복 횟수다. 실행은 기존 ValtanBrain의 패턴 경로를 사용한다.
	struct VALTAN_TIMELINE_PATTERN_ACTION final
	{
		std::string strPatternId;
		std::uint32_t iRepeat = 0;
	};

	// Debug에서 선택할 시간 순서 occurrence다. 한 행에 여러 패턴을 묶어도 각 action은 기존 CValtanBrain 경로로 실행한다.
	struct VALTAN_TIMELINE_ROW final
	{
		std::string strRowId;
		// strRowId에서 FNV-1a32로 만든 wire 명령 키다. 화면 순번과 분리하여 저작 순서를 바꿔도 같은 ID를 유지한다.
		std::uint32_t iCommandId = 0;
		std::uint32_t iOrdinal = 0;
		std::uint32_t iSectionHealthBar = 0;
		VALTAN_TIMELINE_ENTRY_TYPE eEntryType =
			VALTAN_TIMELINE_ENTRY_TYPE::NORMAL;
		VALTAN_TIMELINE_ARENA_STATE eArenaState =
			VALTAN_TIMELINE_ARENA_STATE::FRESH;
		VALTAN_TIMELINE_PROP_STATE ePropState =
			VALTAN_TIMELINE_PROP_STATE::HIDDEN;
		std::vector<VALTAN_TIMELINE_PATTERN_ACTION> PatternActions;
	};

	// 편집 도구/Server가 공유할 발탄 발생 순서 목록이다. 행의 wire command ID와 표시 순번은 서로 다른 값이다.
	struct VALTAN_TIMELINE_DEFINITION final
	{
		std::string strEncounterId;
		std::string strTimelineId;
		std::vector<VALTAN_TIMELINE_ROW> Rows;
	};

	// 직업별 기본 수치와 자원 정책이다. 승인된 직업을 적용할 때 실행 player 상태를 초기화하고 이후 증감은 그 상태에 기록한다.
	struct PLAYER_RUNTIME_PROFILE
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumResource = 0;
		// 스킬 시전 중이 아닐 때 fixed tick 누산기로 회복하는 초당 자원량이다. 실제 벽시계 시간을 직접 읽는 필드는 아니다.
		std::uint32_t iResourceRegenPerSecond = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		// 치명타 확률 percent와 치명타 피해 배율 percent다. 확률 0이면 치명타 추첨이 발생하지 않는다.
		std::uint32_t iCriticalChancePercent = 0;
		std::uint32_t iCriticalDamagePercent = 200;
		float fMoveSpeed = 0.f;
		// 방어 stance를 유지하는 동안 기본 이동 속도에 곱하는 값이다. 1이면 속도를 바꾸지 않는다.
		float fDefenseStanceMoveSpeedScale = 1.f;
		// 직업 identity의 최대값이다. 0이면 해당 profile의 identity 갱신 경로는 즉시 반환한다.
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityRegenPerSecond = 0;
		// 게이지를 사용하는 stance를 유지할 때 초당 소모한다. 0에 도달하면 기본 stance로 복귀한다.
		std::uint32_t iIdentityDrainPerSecond = 0;
		// stance 변경 스킬의 처리 시점에 충분한 게이지가 있을 때 한 번 차감한다. 부족하면 현재 구현은 차감 없이 전환한다.
		std::uint32_t iIdentityStanceSwitchCost = 0;
		// 자동 회복 중 최대값에 도달하면 0으로 순환하는 게이지다.
		// 현재 Update_Identity는 그 지점에서 게이지만 되돌리며 별도 한 바퀴 완료 효과를 발행하지 않는다.
		std::uint32_t iIdentityCyclic = 0;
		LostArk::Shared::PLAYER_STANCE_ID eDefaultStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
	};

	// 가디언 나이트의 identity/불씨 정책이다. 기본 stance의 명중은 게이지를 채우고 용 stance는 설정한 기간으로 소모한다.
	// 최대 소켓 수에서 잠긴 수를 빼 불씨 용량을 정하고, 스킬이 소비한 불씨당 추가 피해 percent를 계산한다.
	struct GUARDIAN_EMBER_PROFILE
	{
		std::uint32_t iGaugeGainPerHit = 0;
		std::uint32_t iDragonDurationMs = 0;
		std::uint32_t iMaximumSockets = 0;
		std::uint32_t iDamageBonusPercentPerOrb = 0;
	};

	// Server가 게시 bootstrap을 읽어 소유하는 gameplay 정의 집합이다. GameRoom/Brain/PlayerSkillSystem은 stable ID로 조회한다.
	// Catalog 요소를 가리키는 조회 포인터/참조는 이 객체의 컨테이너를 차용한다. 성공한 재로드/교체/파괴 뒤에는 재사용하면 안 된다.
	// 내부 잠금은 없으며 호출자는 활성 catalog의 수명과 교체 시점을 관리한다. 실패 사유 문자열은 조회 정의와 별도로 갱신될 수 있다.
	class CGameplayCatalog final
	{
	public:
		// 관문 공통 무력화 최대치를 값으로 돌려준다. 현재 보스의 남은 무력화량은 entity 실행 상태에 있다.
		[[nodiscard]] std::uint32_t Get_RaidStaggerMaximum() const noexcept
		{
			return m_iRaidStaggerMaximum;
		}
		// Server 시작 로드가 게시 Gameplay.bootstrap과 선택적 numeric receipt를 임시 Catalog에 검증하고 성공 시 교체한다.
		// false이면 기존 정의를 유지하되 Get_Status의 실패 설명은 바뀔 수 있다.
		bool Load();
		// 숫자 편집 목록과 별도 revision을 만든다. 값 수정과 연출/동작의 identity를 분리하는 Balance Test 입력이다.
		bool Build_NumericBalanceSnapshot(
			std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries,
			LostArk::Shared::GameplayDataRevision& numericRevision,
			std::string& status) const;
		// active의 현재 숫자와 changes의 before 값을 대조한 뒤 허용 수치만 바꾼 bootstrap을 재검증한다.
		// 성공할 때 이 Catalog와 bootstrapBytes를 함께 갱신하며 choreography/presentation 연결은 유지한다.
		bool Load_NumericBalancePatch(const CGameplayCatalog& active,
			const std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE>& changes,
			std::string& bootstrapBytes);
		// choreography의 구조 위에 numeric에서 같은 키의 값을 대조하여 숫자 patch 경로로 재검증한다.
		// 차이가 없으면 choreography를 복사하며, 숫자 Catalog 전체로 구조 정의를 교체하는 함수는 아니다.
		bool Load_NumericBalanceValues(const CGameplayCatalog& choreography,
			const CGameplayCatalog& numeric);
		// 로드 때 보존한 Kouku/그 외 원문 행으로 header와 count를 다시 붙인다. 현재 struct를 범용 직렬화하는 함수는 아니다.
		[[nodiscard]] std::string Export_BootstrapBytes() const;
		// 전체 bootstrap과 숫자 칸을 마스킹한 행의 digest를 계산한다. 결과는 receipt 검증에 쓰이며 Catalog 정의를 바꾸지 않는다.
		bool Build_NumericBalanceReceiptHashes(
			LostArk::Shared::GameplayDataRevision& bootstrapHash,
			LostArk::Shared::GameplayDataRevision& nonNumericRowsHash,
			std::string& status) const;

		// 게시된 Kouku 행과 activeGameplay의 그 외 행을 합쳐 검증한다. 현재 process의 다른 보스/플레이어 정의를 디스크 값으로 덮지 않는다.
		bool Load_PublishedKoukuProduct(const CGameplayCatalog& activeGameplay);
		// Debug audition 한 번에 사용할 메모리 후보를 검증한다. 이 함수가 디스크에 publish하지는 않는다.
		bool Load_DraftKoukuProduct(const CGameplayCatalog& activeGameplay,
			const std::string& rows, const LostArk::Shared::GameplayDataRevision& expectedRowsRevision);
		// 정규 절대 경로의 Gameplay.bootstrap 후보를 읽고 예상 content hash를 대조한다.
		// 승인되면 ActiveRevision은 child 파일 hash 대신 인자로 검증한 parent revision을 사용한다.
		bool Load_FromBootstrap(
			const std::filesystem::path& bootstrapPath,
			const LostArk::Shared::GameplayDataRevision& expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision& parentRevision);
		// Balance와 스킬 초기화가 순회할 정의 map의 const 참조다. 순서는 unordered_map 계약을 따른다.
		[[nodiscard]] const auto& Get_Skills() const noexcept
		{
			return m_Skills;
		}
		// 포인터를 반환하는 Find_*는 stable ID 조회이며 없으면 nullptr다. 입력으로 실행 entity의 현재 상태를 바꾸지 않는다.
		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& combatObjectArchetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		// 표시 순번이 아닌 command ID로 행을 찾는다. command ID 0 또는 없는 encounter/행이면 nullptr다.
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId,
			std::uint32_t commandId) const;
		// rotation 조회의 phase 필터는 Window가 정의된 관리 구간에만 적용한다.
		// from 줄은 포함하고 to 줄은 제외하는 구간을 찾는다. 해당 구간이 없으면 nullptr다.
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId,
			std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		// encounter의 sequence 정의를 조회한다. Brain이 eMode를 보고 순서 전용 실행과 체력 rotation 실행을 구분한다.
		const BOSS_PATTERN_SEQUENCE_DEFINITION* Find_BossPatternSequence(
			const std::string& encounterId) const;
		const KOUKU_RAID_GATE_DEFINITION* Find_KoukuRaidGate(const std::string& gateId) const;
		const BOSS_PATTERN_BUNDLE_DEFINITION* Find_BossPatternBundle(const std::string& bundleId) const;
		// Kouku 저작 composition revision 숫자다. bootstrap content hash와 별개이며 다른 encounter 조회는 0이다.
		[[nodiscard]] std::uint32_t Find_KoukuSaydonProductSourceRevision(
			const std::string& encounterId) const noexcept;
		// non-Kouku 행의 revision이 유효하고 같은지 비교한다. Kouku reload가 기존 플레이어/보스 balance와 Valtan 기준을 유지하는 데 쓴다.
		[[nodiscard]] bool Has_SameNonKoukuGameplay(
			const CGameplayCatalog& other) const noexcept;
		// 해당 encounter의 광기 정책이다. 없으면 nullptr를 반환한다.
		[[nodiscard]] const BOSS_ENCOUNTER_MADNESS_POLICY* Find_KoukuMadnessPolicy(
			const std::string& encounterId) const noexcept;
		// encounter의 intro pattern ID 참조를 돌려준다. 없는 경우 정적인 빈 문자열 참조다.
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		// 해당 class의 불씨 정책을 찾는다. 등록된 정책이 없으면 nullptr다.
		const GUARDIAN_EMBER_PROFILE* Find_EmberProfile(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		// 공격력에 적용할 피해 비율 percent다. 알 수 없는 profile ID는 0을 반환한다.
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		// 피해 수치 정의다. coefficient/addend가 있으면 공격력 * basis-point 계수 / 10000 + 고정 가산을 사용한다.
		// 두 값이 모두 0이면 기존 iRatePercent 계산으로 돌아간다.
		struct DAMAGE_PROFILE final
		{
			std::uint32_t iRatePercent = 0;
			std::uint32_t iAttackCoefficientBp = 0;
			std::uint32_t iDamageAddend = 0;
			// 플레이어 피해 추첨에서 기준값 주변에 적용할 변동 percent다. 0이면 그 변동을 적용하지 않는다.
			std::uint32_t iDamageSpreadPercent = 0;
			// 스킬 전체 시전의 보스 체력줄 기준 피해량이다. 개별 sub-hit 분배는 스킬 소비자가 맡는다.
			std::uint32_t iBossHealthBarDamage = 0;
		};
		[[nodiscard]] const DAMAGE_PROFILE* Find_DamageProfile(
			const std::string& damageProfileId) const;

		// 스킬이 buff를 줄 대상을 구분한다. 지속 시간/비율과 대상 정의를 게시 데이터에서 함께 읽는다.
		enum class SKILL_BUFF_TARGET : std::uint8_t
		{
			SELF,
			ALLY,
			ENEMY
		};
		struct SKILL_BUFF_DEFINITION final
		{
			std::uint32_t iSkillId = 0;
			std::uint32_t iBuffId = 0;
			SKILL_BUFF_TARGET eTarget = SKILL_BUFF_TARGET::SELF;
			std::uint32_t iDurationMs = 0;
			std::int32_t iDamageDealtPercent = 0;
			std::int32_t iDamageTakenPercent = 0;
			std::int32_t iAttackSpeedPercent = 0;
			// 시전자 class profile의 최대 HP 중 이 percent를 보호막 양으로 계산한다.
			std::uint32_t iShieldPercentOfMaxHp = 0;
			// 일반 MONSTER의 동작을 멈출 ms다. CServerBuffRuntime은 보스에 같은 stun을 적용하지 않는다.
			std::uint32_t iStunMs = 0;
			// 일반 치명 피해에서 HP 1을 남기고 부여할 무적 ms다. 공용 피격 경로가 발동 시 사망 방지 buff를 소비한다.
			std::uint32_t iDeathDenyInvulnerableMs = 0;
		};
		[[nodiscard]] const std::vector<SKILL_BUFF_DEFINITION>* Find_SkillBuffs(
			std::uint32_t skillId) const;
		[[nodiscard]] const SKILL_BUFF_DEFINITION* Find_SkillBuff(
			std::uint32_t buffId) const;
		// 계수/가산값 중 하나라도 있으면 uint64 중간값으로 계산하고 1~UINT32_MAX로 제한한다. 둘 다 0이면 아래 percent 경로를 쓴다.
		// 치명타/편차/방어/대상 HP 변경은 이 함수 밖의 피격 경로 책임이다.
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, const DAMAGE_PROFILE& profile);

		// 공격력 * percent를 정수 피해로 바꾼다. 둘 중 하나가 0이면 0이며 그 외는 1~UINT32_MAX로 제한한다.
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, std::uint32_t damageRatePercent);
		// 프로젝트 공통 방어식이다. rawDamage * 100 / (100 + defense)를 사용하며 양수 피해는 최소 1이다.
		// 특정 타격이 방어를 우회할지는 호출 경로의 정책이 정한다.
		static std::uint32_t Apply_Defense(
			std::uint32_t rawDamage, std::uint32_t defense);

		// 검증한 활성 gameplay 세대다. 명시 candidate 로드에서는 child bootstrap hash와 별도인 parent revision을 노출한다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept
		{
			return m_ActiveRevision;
		}
		// bootstrap이 승인한 Client 표시 세대의 content identity다. Server가 Client asset을 로드하는 경로는 아니다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ValtanPresentationGenerationId() const noexcept
		{
			return m_ValtanPresentationGenerationId;
		}
		// 최근 로드/검증 상태 문자열의 차용 참조다. 다음 검증 호출 때 내용이 바뀔 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	private:
		// 크기 제한과 읽기 완료를 검사한 뒤 공용 byte parser로 전달한다. 외부 path 선택과 revision 입력은 공개 Load 경로가 정한다.
		bool Load_BootstrapPath(
			const std::filesystem::path& bootstrapPath,
			const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision* parentRevision);
		// 모든 로드 경로가 사용하는 parse/참조 검증/commit 경계다. 주요 정의 map은 실패 시 rollback한다.
		// 현재 raid 무력화 최대치 멤버는 이 rollback에 포함되지 않으므로 모든 멤버 보존을 전제로 활성 객체에 직접 재로드하면 안 된다.
		// 공개 Load와 ServerApp의 후보 로드처럼 별도 Catalog에서 검증한다. 아래 부분 parser의 출력도 실패 시 일부 작성될 수 있다.
		bool Load_BootstrapBytes(
			const std::string& bootstrapBytes,
			const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision* parentRevision);
		// 스킬/stage가 같은 packed RootMotion 형식과 시간 순서 검사를 공유한다. 실패 이유는 m_strStatus에 남긴다.
		bool Parse_RootMotionSamples(
			std::string_view packed,
			std::uint32_t sampleCount,
			std::uint32_t limitMs,
			std::vector<ROOT_MOTION_SAMPLE>& outSamples);
		// 정렬된 타격 목록과 반복 합계 상한을 검사한다. 출력은 먼저 비우며 실패 시 이미 읽은 일부 타격이 남을 수 있다.
		bool Parse_SkillHits(
			std::string_view packed,
			std::uint32_t hitCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_HIT>& outHits);
		// 시작:종료 ms 구간을 순서와 겹침 조건에 맞춰 읽는다. 파서 출력은 실패 시 부분 작성될 수 있다.
		bool Parse_CancelWindows(
			std::string_view packed,
			std::uint32_t windowCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_CANCEL_WINDOW>& outWindows);
		// areaType/range/angle/width/height/offset/inner/maxTargets/pushMs/pushRange 형식을 공통 해석한다.
		bool Parse_HitShapeExtent(
			const std::string_view* fields,
			PLAYER_SKILL_HIT& outHit);
		// SKILLPROJ/SKILLSTAGEPROJ의 index/생성 시각/이동/타격 목록을 읽는다.
		// 행 순서로 추가하며 잘못된 생성 시각 순서나 index를 거부한다.
		bool Parse_SkillProjectile(
			const std::vector<std::string_view>& fields,
			std::size_t firstField,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_PROJECTILE>& outProjectiles);

		// 각 map이 정의의 값과 하위 vector/string을 소유한다. stable ID 조회를 위해 해시 map을 쓰며 실행 순서는 별도 vector 정의로 보관한다.
		std::unordered_map<LostArk::Shared::SKILL_ID, PLAYER_SKILL_DEFINITION>
			m_Skills;
		std::unordered_map<std::string, BOSS_RUNTIME_PROFILE> m_Bosses;
		std::unordered_map<std::string, std::vector<BOSS_PART_DEFINITION>>
			m_BossParts;
		std::unordered_map<std::string, std::vector<BOSS_PATTERN_DEFINITION>>
			m_BossPatterns;
		std::unordered_map<std::string, BOSS_COMBAT_OBJECT_DEFINITION>
			m_BossCombatObjects;
		std::unordered_map<std::string, VALTAN_TIMELINE_DEFINITION>
			m_ValtanTimelines;
		std::unordered_map<std::string, std::string> m_IntroPatternIdByEncounter;
		std::unordered_map<std::string,
			std::vector<BOSS_PATTERN_ROTATION_DEFINITION>>
			m_BossPatternRotations;
		std::unordered_map<std::string, BOSS_PATTERN_SEQUENCE_DEFINITION>
			m_BossPatternSequences;
		std::unordered_map<std::string, BOSS_PATTERN_BUNDLE_DEFINITION> m_BossPatternBundles;
		std::unordered_map<std::string, KOUKU_RAID_GATE_DEFINITION> m_KoukuRaidGates;
		std::uint32_t m_iKoukuSaydonProductSourceRevision = 0u;
		std::uint32_t m_iRaidStaggerMaximum = 0u;
		std::unordered_map<std::string, BOSS_ENCOUNTER_MADNESS_POLICY>
			m_KoukuMadnessPolicies;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_RUNTIME_PROFILE> m_Players;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			GUARDIAN_EMBER_PROFILE> m_EmberProfiles;
		std::unordered_map<std::string, std::uint32_t>
			m_DamageRatePercentByProfileId;
		std::unordered_map<std::string, DAMAGE_PROFILE> m_DamageProfileById;
		std::unordered_map<std::uint32_t, std::vector<SKILL_BUFF_DEFINITION>>
			m_SkillBuffsBySkillId;
		std::unordered_map<std::uint32_t, SKILL_BUFF_DEFINITION> m_SkillBuffById;
		// 전체 활성 세대, Kouku 외 내용의 digest, Client presentation 세대를 구분해 재로드/입장 계약에 사용한다.
		LostArk::Shared::GameplayDataRevision m_ActiveRevision{};
		LostArk::Shared::GameplayDataRevision m_NonKoukuGameplayRevision{};
		LostArk::Shared::GameplayDataRevision
			m_ValtanPresentationGenerationId{};
		// domain별 원문 행을 보존하여 Kouku 전용 재로드와 numeric patch가 무관한 domain을 그대로 유지하도록 한다.
		std::string m_KoukuBootstrapRows;
		std::string m_NonKoukuBootstrapRows;
		std::string m_strStatus;
	};

	// 공용 피격 경로가 기본 단축키 Q/W/E/R 중 counter power가 있는 스킬인지 확인한다. 이 함수 자체는 카운터 성공을 판정하지 않는다.
	inline bool Is_PrimaryCounterSkill(const PLAYER_SKILL_DEFINITION& skill)
	{
		return 0u != skill.iCounterPower &&
			("Q" == skill.strInputSlot || "W" == skill.strInputSlot ||
			 "E" == skill.strInputSlot || "R" == skill.strInputSlot);
	}

	// SPACE에 배치된 ACTIVE/HOLD를 회피 스킬로 분류한다. 이 함수는 분류만 하며 입력 취소 금지는 호출자가 적용한다.
	inline bool Is_DodgeSkill(const PLAYER_SKILL_DEFINITION& skill)
	{
		return "SPACE" == skill.strInputSlot &&
			(LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE == skill.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::HOLD == skill.eSkillKind);
	}

	// 현재 action 또는 combo stage의 입력별 취소 구간을 조회한다. 시간 입력은 초이며 ms로 환산해 [시작, 종료)로 검사한다.
	// 목록이 없거나 경과 시간이 음수/NaN이면 false다. 실행 상태나 다음 스킬은 이 함수에서 변경하지 않는다.
	inline bool Is_InsideCancelWindow(
		const PLAYER_SKILL_DEFINITION& running,
		const std::uint32_t comboStage,
		const float actionElapsedSeconds,
		const PLAYER_CANCEL_INPUT input)
	{
		const std::size_t stageIndex =
			0u == comboStage ? 0u : comboStage - 1u;
		const bool hasStage =
			(LostArk::Shared::PLAYER_SKILL_KIND::COMBO == running.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::HOLD == running.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::COUNTER == running.eSkillKind) &&
			stageIndex < running.ComboStages.size();
		const auto select = [input](const auto& owner)
			-> const std::vector<PLAYER_CANCEL_WINDOW>&
		{
			switch (input)
			{
			case PLAYER_CANCEL_INPUT::MOVE:
				return owner.MoveCancelWindows;
			case PLAYER_CANCEL_INPUT::DODGE:
				return owner.DodgeCancelWindows;
			default:
				return owner.SkillCancelWindows;
			}
		};
		const std::vector<PLAYER_CANCEL_WINDOW>& windows = hasStage ?
			select(running.ComboStages[stageIndex]) : select(running);
		if (windows.empty() || !(actionElapsedSeconds >= 0.f))
			return false;
		const double elapsedMs =
			static_cast<double>(actionElapsedSeconds) * 1000.0;
		for (const PLAYER_CANCEL_WINDOW& window : windows)
		{
			if (elapsedMs >= static_cast<double>(window.iStartMs) &&
				elapsedMs < static_cast<double>(window.iEndMs))
			{
				return true;
			}
		}
		return false;
	}
}
