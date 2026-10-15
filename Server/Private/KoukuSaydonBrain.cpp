#include "KoukuSaydonBrain.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace
{
	// Server 시계는 초당 30 tick이다. 저작 시간 ms와 비교할 때 정수 곱을 사용하여 stage마다 float 반올림이 끼지 않게 한다.
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;

	// 0은 미설정 tick이므로 UINT32_MAX 다음은 1이다. stage 전환 뒤 첫 평가 tick을 예약할 때 사용한다.
	std::uint32_t Next_ServerTick(const std::uint32_t tick) noexcept
	{
		return tick == (std::numeric_limits<std::uint32_t>::max)() ? 1u : tick + 1u;
	}

	// 일반 stage 시계는 첫 평가 tick 자체를 1 tick으로 센다. 시작 tick/입력 tick이 0이면 아직 평가하지 않은 것으로 본다.
	std::uint64_t Stage_ElapsedTicks(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) noexcept
	{
		const std::uint32_t firstTick = boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		const std::uint64_t age = serverTick >= firstTick
			? static_cast<std::uint64_t>(serverTick - firstTick)
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - firstTick) +
				static_cast<std::uint64_t>(serverTick);
		return age + 1u;
	}

	// 경과 tick과 기간 ms를 64비트 곱으로 비교한다. 30Hz에서 기간에 도달하는 첫 tick부터 true다.
	bool Has_ElapsedMilliseconds(const std::uint64_t elapsedTicks, const std::uint32_t durationMs) noexcept
	{
		return elapsedTicks * MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ;
	}

	// 지원하는 세 stage 종류를 복제되는 action 상태로 바꾼다. 다른 종류는 IDLE이며 정의 validator가 먼저 지원 종류를 제한한다.
	LostArk::Server::SERVER_ENTITY_ACTION To_ServerAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_KIND kind) noexcept
	{
		using namespace LostArk::Server;
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_KIND::WINDUP:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		case BOSS_PATTERN_STAGE_KIND::ACTIVE:
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		case BOSS_PATTERN_STAGE_KIND::RECOVERY:
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		default:
			return SERVER_ENTITY_ACTION::IDLE;
		}
	}

	// 일반 이동 옵션이 비어 있는 animation root motion만 승인한다. 키가 있다면 2~512개, 시작 0/끝 duration, 시간 엄격 증가를 요구한다.
	// 좌표는 source 상대 변위이며 finite/범위를 확인한다. navigation 경로의 실제 유효성은 여기서 검사하지 않는다.
	bool Is_AnimationRootMotion(
		const LostArk::Server::BOSS_PATTERN_STAGE_MOTION& motion, const std::uint32_t durationMs) noexcept
	{
		using namespace LostArk::Server;
		if (BOSS_PATTERN_STAGE_MOTION_KIND::NONE != motion.eKind || 0u != motion.iRetargetDelayMs ||
			0.f != motion.fSpeedMps || 0.f != motion.fDistance || 0u != motion.iCornerIndex ||
			0.f != motion.fHalfExtentsX || 0.f != motion.fHalfExtentsZ)
			return false;
		if (motion.RootMotion.empty())
			return true;
		const auto& samples = motion.RootMotion;
		if (samples.size() < 2u || samples.size() > 512u || samples.front().iTimeMs != 0u ||
			samples.back().iTimeMs != durationMs || samples.front().fForward != 0.f ||
			samples.front().fLateral != 0.f || samples.front().fUp != 0.f)
			return false;
		for (std::size_t index = 0u; index < samples.size(); ++index)
		{
			const auto& sample = samples[index];
			if (!std::isfinite(sample.fForward) || !std::isfinite(sample.fLateral) ||
				!std::isfinite(sample.fUp) || std::abs(sample.fForward) > 100000.f ||
				std::abs(sample.fLateral) > 100000.f || std::abs(sample.fUp) > 100000.f ||
				(index && sample.iTimeMs <= samples[index - 1u].iTimeMs))
				return false;
		}
		return true;
	}
}

// PartyWorld의 G1 주 보스 조회에 사용한다. owner가 없는 정확한 world/encounter/archetype/placement 조합만 true다.
bool LostArk::Server::CKoukuSaydonBrain::Is_GateOneBoss(
	const LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == boss.strArchetypeId &&
		KOUKUSAYDON_G1_BOSS_PLACEMENT_ID == boss.strPlacementId;
}

// GameRoom이 다룰 쿠크 아레나 주 보스를 분류한다. 호출자가 가진 entity의 필드만 읽으며 alive/실행 소유 여부는 별도다.
bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBoss(
	const LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		boss.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

// GateProgress의 활성화 후보를 분류한다. disabled placement를 찾는 조건이며 실제 spawn 성공이나 definition 전체 유효성은 뜻하지 않는다.
bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBossPlacement(
	const LostArk::Shared::WORLD_ID worldId, const WORLD_BOOTSTRAP_PLACEMENT& placement) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId && !placement.isEnabled &&
		WORLD_BOOTSTRAP_KIND::BOSS == placement.eKind &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == placement.strEncounterId &&
		placement.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

// Catalog 검증과 GameRoom 승인/조회가 공용으로 사용한다. pattern은 읽기 전용이고 첫 실패의 status를 반환한다.
// 이름은 animation-only지만 현재는 LogicWindow/MechanicTrigger의 공격도 검증한다. 아래 제한은 stage 직접 공격과 지원하지 않는 조합에 대한 것이다.
// 외부 참조의 존재, navigation, Catalog 전체 무결성은 호출 계층의 추가 검사에 남는다.
bool LostArk::Server::CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
	const BOSS_PATTERN_DEFINITION& pattern, std::string& status)
{
	if (KOUKUSAYDON_G1_ENCOUNTER_ID != pattern.strEncounterId || pattern.strPatternId.empty() ||
		BOSS_PATTERN_SELECTION::AUDITION_ONLY != pattern.eSelection ||
		BOSS_PATTERN_TARGET_POLICY::NONE != pattern.eTargetPolicy ||
		BOSS_PATTERN_AIM_POLICY::NONE != pattern.eAimPolicy ||
		BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind || !pattern.Motion.strAnchorId.empty() ||
		0.f != pattern.Motion.fLandingX || 0.f != pattern.Motion.fLandingY || 0.f != pattern.Motion.fLandingZ ||
		0.f != pattern.Motion.fApexHeight || pattern.Motion.bMoveToAnchorBeforeTakeoff ||
		BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind || !pattern.Finale.strGhostArchetypeId.empty() ||
		!pattern.Finale.GhostPatternIds.empty() || 0.f != pattern.fVerticalOffsetM ||
		pattern.bInvulnerableWhileRunning || pattern.Stages.empty() ||
		pattern.Stages.size() != pattern.iExpectedStageCount)
	{
		status = "KoukuSaydon pattern is not an animation-only audition definition";
		return false;
	}

	// root motion과 다른 보스 이동 작성 방식이 동시에 좌표를 소유하지 못하도록 충돌하는 조합을 거부한다.
	const bool hasRootMotion = std::any_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) {
		return !stage.Motion.RootMotion.empty();
	});
	if (hasRootMotion &&
		(pattern.BossMotion ||
		 std::any_of(
			 pattern.LogicWindows.begin(),
			 pattern.LogicWindows.end(),
			 [](const auto& window) {
				 return window.fBossChargeDistanceM > 0.f;
			 }) ||
		 std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(), [](const auto& trigger) {
			 return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT;
		 })))
	{
		status = "KoukuSaydon animation root motion conflicts with authored movement";
		return false;
	}
	// stage/action ID의 중복 검사용 지역 set이다. 실행 순서는 pattern.Stages vector가 그대로 보유한다.
	std::unordered_set<std::string> stageIds;
	std::unordered_set<std::string> actionIds;
	for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
	{
		const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern.Stages[index];
		// 현재 stage action은 비어 있거나 지정된 ENTER 대상 재선택 한 개만 허용한다. action을 여기서 실행하지 않는다.
		const bool supportedActions =
			stage.Actions.empty() || (stage.Actions.size() == 1u && !pattern.BossMotion && [&]() {
				const auto& action = stage.Actions.front();
				return action.eTrigger == BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER &&
					action.eKind == BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE &&
					(action.strTargetId == "boss.target.pattern" ||
					 action.strTargetId == "boss.target.nearest") &&
					action.iValue == 1u && action.iDurationMs == 0u &&
					action.eReleaseMode == BOSS_GRABBED_RELEASE_MODE::NONE && action.fReleaseSpeedMps == 0.f &&
					action.fReleaseYawOffsetDegrees == 0.f &&
					action.Volley.ePolicy == BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE;
			}());
		const bool supportedKind = BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::ACTIVE == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::RECOVERY == stage.eStageKind;
		// 분기가 있으면 다음 stage action으로 가는 TIMEOUT 한 개여야 한다. 마지막 stage는 next action이 비어 있어야 한다.
		const bool validTimeoutBranch = stage.Branches.empty() ||
			(1u == stage.Branches.size() &&
			 BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == stage.Branches.front().eOutcome &&
			 stage.Branches.front().strNextPatternId.empty() &&
			 stage.Branches.front().strNextActionId ==
				 (index + 1u < pattern.Stages.size() ? pattern.Stages[index + 1u].strActionId : std::string{}));
		// 식별자/종류/기간 뒤에 stage 직접 피해·타격·반복·이동·분기 필드의 지원 범위를 차례로 검사한다.
		if (stage.strStageId.empty() || stage.strActionId.empty() ||
			!stageIds.insert(stage.strStageId).second || !actionIds.insert(stage.strActionId).second ||
			!supportedKind || 0u == stage.iDurationMs || !stage.strDamageProfileId.empty() ||
			BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT != stage.eHitAnchorKind ||
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE != stage.eHitActivationKind ||
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE != stage.ePlayerResponse ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE != stage.eAttachmentSlot ||
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL != stage.ePartDamagePolicy || stage.bHasCounterProxy ||
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE != stage.eCounterProxyKind ||
			0.f != stage.fCounterProxyForwardOffsetM || 0.f != stage.fCounterProxyRightOffsetM ||
			0.f != stage.fCounterProxyRadiusM || 0.f != stage.fCounterProxyArcDegrees ||
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE != stage.eBossResponseKind ||
			0u != stage.iBossResponseThreshold || 0.f != stage.fVerticalOffsetM ||
			0.f != stage.fHitOuterRadius || 0.f != stage.fHitInnerRadius || 0.f != stage.fHitAngleDegrees ||
			0.f != stage.fHitLength || 0.f != stage.fHitHalfWidth || 0u != stage.iHitCount ||
			0u != stage.iHitIntervalMs || 0u != stage.iHitDelayMs || !stage.HitOffsetsMs.empty() ||
			0.f != stage.fHitAnchorForwardOffsetM || 0.f != stage.fHitAnchorRightOffsetM ||
			0.f != stage.fHitAnchorYawOffsetDegrees || 0u != stage.iHitActivationStartMs ||
			0u != stage.iHitActivationLifetimeMs || 0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
			stage.bKnockdown || 0u != stage.iDownMs || stage.bWallContact || stage.bChargeImpact ||
			stage.bPiercesCover || !Is_AnimationRootMotion(stage.Motion, stage.iDurationMs) ||
			!stage.strPropBreakSetId.empty() || !stage.PropBreakSlotIds.empty() || !supportedActions ||
			!validTimeoutBranch)
		{
			status = "KoukuSaydon pattern contains unsupported combat, motion, action, or branch data";
			return false;
		}
	}
	// 고정 시계에서는 누적 경계를 30Hz tick으로 올림했을 때 각 stage가 적어도 한 tick 간격을 갖는지 검사한다.
	std::uint64_t patternDurationMs = 0u;
	for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
	{
		if (pattern.bFixedTimelineClock &&
			((patternDurationMs + stage.iDurationMs) * SERVER_TICK_HZ + 999u) / 1000u <=
				(patternDurationMs * SERVER_TICK_HZ + 999u) / 1000u)
		{
			status = "Parent stage boundaries must occupy distinct Server ticks";
			return false;
		}
		patternDurationMs += stage.iDurationMs;
	}
	// 독립 row 수명 검증은 timeline 기간까지 보지만, Brain의 stage 완료 시계는 Stages의 길이 합만 사용한다.
	patternDurationMs = (std::max)(patternDurationMs, std::uint64_t(pattern.iTimelineDurationMs));
	// BossMotion은 stage root와 별도인 절대 월드 위치 경로다. 범위와 키의 시작/종료 일치·시간 증가를 검사한다.
	if (pattern.BossMotion)
	{
		const auto& motion = *pattern.BossMotion;
		bool valid = !pattern.bResetBossToSpawn && !pattern.ResetBossYawDegrees &&
			motion.iStartMs < motion.iEndMs && motion.iEndMs <= patternDurationMs &&
			(!motion.Keys.empty() || motion.StartPosition[1] == motion.EndPosition[1]) &&
			std::isfinite(motion.fYawDegrees) && std::abs(motion.fYawDegrees) <= 360.f;
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			valid = valid && std::isfinite(motion.StartPosition[axis]) &&
				std::isfinite(motion.EndPosition[axis]) && std::abs(motion.StartPosition[axis]) <= 100000.f &&
				std::abs(motion.EndPosition[axis]) <= 100000.f;
		if (!motion.Keys.empty())
		{
			valid = valid && motion.Keys.size() >= 2u && motion.Keys.size() <= 512u &&
				motion.Keys.front().iTimeMs == motion.iStartMs && motion.Keys.back().iTimeMs == motion.iEndMs &&
				motion.Keys.front().Position == motion.StartPosition &&
				motion.Keys.back().Position == motion.EndPosition;
			for (std::size_t i = 0u; i < motion.Keys.size(); ++i)
				valid = valid && (i == 0u || motion.Keys[i - 1u].iTimeMs < motion.Keys[i].iTimeMs) &&
					std::all_of(
							motion.Keys[i].Position.begin(), motion.Keys[i].Position.end(), [](float value) {
								return std::isfinite(value) && std::abs(value) <= 100000.f;
							});
		}
		if (!valid)
		{
			status = "KoukuSaydon Boss Motion interval, keys, base height or reset policy is invalid";
			return false;
		}
	}
	// trigger별 필요한 필드와 다른 종류에 남아 있으면 안 되는 필드를 구분한다. 이 반복은 실제 trigger 소비가 아닌 정의 검증이다.
	std::unordered_set<std::string> triggerIds;
	for (const auto& trigger : pattern.MechanicTriggers)
	{
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
		{
			std::unordered_set<std::string> directions(
				trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end());
			if (pattern.BossMotion || trigger.DirectionPatternIds.size() != 4u || directions.size() != 4u ||
				directions.count("") || directions.count(pattern.strPatternId) ||
				trigger.strCloneEndStageId.empty() ||
				(!trigger.strRealPatternId.empty() && !directions.count(trigger.strRealPatternId)) ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f ||
				trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() ||
				!trigger.ClockHours.empty() || !trigger.PatternSpawns.empty() || trigger.iCountPerPlayer ||
				trigger.iEffectLifetimeMs || trigger.iArenaRandomCount || trigger.bRandomPlayerOnly)
			{
				status = "Cross direction requires four distinct child Patterns and one clone end Stage";
				return false;
			}
		}
		else if (
			!trigger.DirectionPatternIds.empty() || !trigger.strCloneEndStageId.empty() ||
			!trigger.strRealPatternId.empty())
		{
			status = "Only cross direction owns directional Pattern references";
			return false;
		}

		// Albion 공중 phase의 높이 m/기간 ms와 선택 effect 조합을 제한한다. 선행 JUMP와 SLAM의 남은 root 하강 구간도 아래에서 확인한다.
		using Air = ALBION_AIRBORNE_PHASE;
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
		{
			const auto phase = trigger.eAirbornePhase;
			const bool jump = phase == Air::JUMP, appear = phase == Air::APPEAR_PLAYER;
			if (phase == Air::NONE || phase > Air::SLAM || pattern.BossMotion ||
				!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM < 0.f ||
				trigger.fAirborneHeightM > 100000.f || ((jump || appear) != (trigger.fAirborneHeightM > 0.f)) ||
				(trigger.bCaptureAirborneTargetPosition && phase != Air::SELECT_PLAYER) ||
				trigger.strSelectedEffectVisualId.empty() != (trigger.iSelectedEffectLifetimeMs == 0u) ||
				(!trigger.strSelectedEffectVisualId.empty() && !trigger.bCaptureAirborneTargetPosition) ||
				std::uint64_t(trigger.iStartMs) + trigger.iSelectedEffectLifetimeMs > patternDurationMs ||
				(jump ? (trigger.iAirborneDurationMs > 600000u ||
						 std::uint64_t(trigger.iStartMs) + trigger.iAirborneDurationMs > patternDurationMs)
					  : trigger.iAirborneDurationMs != 0u) ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE ||
				!trigger.strClonePatternId.empty() || !trigger.ClockHours.empty() ||
				trigger.fFaceCenterYawOffsetDegrees != 0.f || trigger.iCountPerPlayer ||
				trigger.fPlayerEffectRadiusM != 0.f || trigger.iEffectLifetimeMs || trigger.iArenaRandomCount ||
				trigger.bRandomPlayerOnly || !trigger.PatternSpawns.empty() ||
				!std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) ||
				!std::isfinite(trigger.fTeleportZ) || std::abs(trigger.fTeleportX) > 100000.f ||
				std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f ||
				(phase != Air::CENTER &&
				 (trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f)))
			{
				status = "Albion airborne phase has invalid or unrelated values";
				return false;
			}
			if (phase != Air::JUMP && phase != Air::SELECT_PLAYER &&
				!std::any_of(
					pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(), [&](const auto& row) {
						return row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
							row.eAirbornePhase == Air::JUMP && row.iStartMs <= trigger.iStartMs;
					}))
			{
				status = "Albion airborne phase requires its initial jump";
				return false;
			}
			if (phase == Air::SLAM)
			{
				std::uint64_t start = 0u;
				const BOSS_PATTERN_STAGE_DEFINITION* source = nullptr;
				for (const auto& stage : pattern.Stages)
				{
					if (trigger.iStartMs < start + stage.iDurationMs)
					{
						source = &stage;
						break;
					}
					start += stage.iDurationMs;
				}
				if (!source || source->Motion.RootMotion.empty())
				{
					status = "Albion slam requires source root motion";
					return false;
				}
				const double age = double(trigger.iStartMs - start);
				const float up = Sample_StageRootMotion(source->Motion.RootMotion, age).fUp;
				float minimum = up;
				for (const auto& sample : source->Motion.RootMotion)
					if (sample.iTimeMs >= age)
						minimum = (std::min)(minimum, sample.fUp);
				if (up - minimum <= .000001f)
				{
					status = "Albion slam has no remaining source descent";
					return false;
				}
			}
		}
		else if (
			trigger.eAirbornePhase != Air::NONE || trigger.fAirborneHeightM != 0.f ||
			trigger.iAirborneDurationMs != 0u || trigger.bCaptureAirborneTargetPosition ||
			!trigger.strSelectedEffectVisualId.empty() || trigger.iSelectedEffectLifetimeMs != 0u)
		{
			status = "Non-Albion trigger carries airborne values";
			return false;
		}
		// Showtime 폭탄은 마지막 회전 추적과 BOSS_CURRENT 부채꼴을 함께 사용한다. 고정/폭발 표시 ID와 수명도 검사한다.
		if (trigger.ShowtimeBomb)
		{
			const auto& bomb = *trigger.ShowtimeBomb;
			const auto& fan = bomb.FanRegion;
			if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
				trigger.fFollowSpeedScale != 0.f || trigger.iDurationMs <= 34u ||
				bomb.iBodyStartMs >= std::uint64_t(trigger.iStartMs) + trigger.iDurationMs ||
				bomb.strBodyVisualId.empty() || bomb.strExplosionVisualId.empty() ||
				bomb.strBodyVisualId == bomb.strExplosionVisualId || !bomb.iExplosionLifetimeMs ||
				bomb.iExplosionLifetimeMs > 600000u ||
				std::any_of(
					bomb.BodyPosition.begin(),
					bomb.BodyPosition.end(),
					[](float value) {
						return !std::isfinite(value) || std::abs(value) > 100000.f;
					}) ||
				fan.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || !fan.bSector || fan.bReverseSector ||
				fan.bCircle || fan.bCylinder || fan.bLinearMotion || fan.WorldTrack.bEnabled ||
				fan.fInnerRadiusM != 0.f || !std::isfinite(fan.fCenterX) || std::abs(fan.fCenterX) > 1000.f ||
				!std::isfinite(fan.fCenterY) || std::abs(fan.fCenterY) > 1000.f ||
				!std::isfinite(fan.fCenterZ) || std::abs(fan.fCenterZ) > 1000.f ||
				!std::isfinite(fan.fYawDegrees) || std::abs(fan.fYawDegrees) > 360.f ||
				!std::isfinite(fan.fRadiusM) || fan.fRadiusM <= 0.f || !std::isfinite(fan.fRadiusXM) ||
				fan.fRadiusXM <= 0.f || fan.fRadiusXM > 1000.f || !std::isfinite(fan.fRadiusZM) ||
				fan.fRadiusZM <= 0.f || fan.fRadiusZM > 1000.f || !std::isfinite(fan.fHalfAngleDegrees) ||
				fan.fHalfAngleDegrees <= 0.f || fan.fHalfAngleDegrees >= 180.f)
			{
				status = "Showtime bomb requires a fixed MAP visual and the final rotate-only boss fan";
				return false;
			}
		}
		// 공격 template은 사용할 trigger 종류와 visual 소유자가 맞아야 한다. 종류별 실제 수명 ms를 Shared validator에 넘긴다.
		// FixedHits의 수명은 Albion effect, 선택 대상 effect, 일반 fixed visual 순서로 선택한다.
		const bool hitShowtime = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
		const bool hitPursuit = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
		const bool hitAlbion = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
		const bool hitSelected = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
			trigger.eAirbornePhase == Air::SELECT_PLAYER && trigger.bCaptureAirborneTargetPosition &&
			!trigger.strSelectedEffectVisualId.empty();
		if ((!hitShowtime && !trigger.TrackingHits.empty()) ||
			(!hitShowtime && !hitAlbion && !hitSelected && !trigger.FixedHits.empty()) ||
			(!hitPursuit && !trigger.ProjectileHits.empty()) ||
			(!hitAlbion && !hitSelected && !trigger.FixedHits.empty() && trigger.strFixedVisualId.empty()) ||
			(!trigger.TrackingHits.empty() && trigger.strTrackingVisualId.empty()) ||
			!LostArk::Shared::Validate_AttackHitTemplates(
				trigger.FixedHits,
				hitAlbion		  ? trigger.iEffectLifetimeMs
					: hitSelected ? trigger.iSelectedEffectLifetimeMs
								  : trigger.iFixedLifetimeMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(trigger.TrackingHits, trigger.iDurationMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(
				trigger.ProjectileHits,
				trigger.iProjectileLifetimeMs ? trigger.iProjectileLifetimeMs : 600000u) ||
			std::any_of(trigger.RandomVolleys.begin(), trigger.RandomVolleys.end(), [](const auto& volley) {
				return !LostArk::Shared::Validate_AttackHitTemplates(volley.Hits, volley.iLifetimeMs);
			}))
		{
			status = "Dynamic attack templates have invalid owners, shapes or hit windows";
			return false;
		}
		// 추적 발사체의 표시 목록/카드 수와 이동·반지름·수명을 제한한다. 다른 trigger에는 발사체 전용 필드가 남지 않아야 한다.
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES)
		{
			const auto count = trigger.ProjectileVisualIds.size();
			if (!count || count > 4u || trigger.strContactVisualId.empty() ||
				(!trigger.ProjectileCardSymbols.empty() &&
				 (trigger.ProjectileCardSymbols.size() != count ||
				  std::any_of(
					  trigger.ProjectileCardSymbols.begin(),
					  trigger.ProjectileCardSymbols.end(),
					  [](const auto symbol) {
						  return symbol <= LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE ||
							  symbol >= LostArk::Shared::MECHANIC_CARD_SYMBOL::END;
					  }))) ||
				std::any_of(
					trigger.ProjectileVisualIds.begin(),
					trigger.ProjectileVisualIds.end(),
					[](const auto& id) {
						return id.empty();
					}) ||
				!std::isfinite(trigger.fProjectileSpeedMps) || trigger.fProjectileSpeedMps < .01f ||
				trigger.fProjectileSpeedMps > 100.f || !std::isfinite(trigger.fProjectileContactRadiusM) ||
				trigger.fProjectileContactRadiusM < .01f || trigger.fProjectileContactRadiusM > 10.f ||
				!std::isfinite(trigger.fProjectileSpawnRadiusM) || trigger.fProjectileSpawnRadiusM < 0.f ||
				trigger.fProjectileSpawnRadiusM > 100.f || !std::isfinite(trigger.fProjectileMaxDistanceM) ||
				trigger.fProjectileMaxDistanceM < 0.f || trigger.fProjectileMaxDistanceM > 1000.f ||
				trigger.iProjectileLifetimeMs > 600000u || trigger.iSpawnIntervalMs > 600000u ||
				!trigger.iProjectileCountPerWave || trigger.iProjectileCountPerWave > 16u ||
				(trigger.iProjectileLifetimeMs == 0u &&
				 (!trigger.bProjectileHoming || trigger.iSpawnIntervalMs != 0u ||
				  trigger.fProjectileMaxDistanceM != 0.f)))
			{
				status = "Pursuit projectile visual, motion or lifetime contract is invalid";
				return false;
			}
		}
		else if (
			!trigger.ProjectileVisualIds.empty() || !trigger.ProjectileCardSymbols.empty() ||
			!trigger.strContactVisualId.empty() || trigger.fProjectileSpeedMps != 0.f ||
			trigger.fProjectileMaxDistanceM != 0.f || trigger.fProjectileContactRadiusM != 0.f ||
			trigger.fProjectileSpawnRadiusM != 0.f || trigger.iProjectileLifetimeMs != 0u ||
			trigger.iProjectileCountPerWave != 0u || trigger.bProjectileHoming)
		{
			status = "Only pursuit projectiles own projectile values";
			return false;
		}
		// Showtime random volley가 있으면 anchor/크기/간격/수명과 각 visual을 검사한다. 없을 때는 관련 필드도 기본값이어야 한다.
		const bool hasRandomVolleys = !trigger.RandomVolleys.empty();
		if (hasRandomVolleys
				? (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
				   trigger.RandomVolleys.size() > 32u ||
				   (trigger.strRandomAnchorKind != "BOSS_SPAWN" && trigger.strRandomAnchorKind != "BOSS") ||
				   !std::isfinite(trigger.fRandomScaleMin) || !std::isfinite(trigger.fRandomScaleMax) ||
				   trigger.fRandomScaleMin < .01f || trigger.fRandomScaleMax > 10.f ||
				   trigger.fRandomScaleMax < trigger.fRandomScaleMin || trigger.iRandomSpawnIntervalMs == 0u ||
				   trigger.iRandomSpawnIntervalMs > 600000u || !std::isfinite(trigger.fRandomArenaRadiusM) ||
				   trigger.fRandomArenaRadiusM <= 0.f || trigger.fRandomArenaRadiusM > 1000.f ||
				   !std::isfinite(trigger.fRandomArenaHeightToleranceM) ||
				   trigger.fRandomArenaHeightToleranceM <= 0.f || trigger.fRandomArenaHeightToleranceM > 10.f ||
				   std::any_of(
					   trigger.RandomVolleys.begin(),
					   trigger.RandomVolleys.end(),
					   [](const auto& volley) {
						   return volley.strClientVisualId.empty() || volley.iLifetimeMs == 0u ||
							   volley.iLifetimeMs > 600000u;
					   }))
				: (trigger.iRandomSpawnIntervalMs != 0u || trigger.fRandomArenaRadiusM != 0.f ||
				   trigger.fRandomArenaHeightToleranceM != 0.f || trigger.strRandomAnchorKind != "BOSS_SPAWN" ||
				   trigger.fRandomScaleMin != 1.f || trigger.fRandomScaleMax != 1.f))
		{
			status = "Showtime random volley pool, cadence or arena contract is invalid";
			return false;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
			((trigger.strFixedVisualId == trigger.strTrackingVisualId &&
			  (!hasRandomVolleys || !trigger.strFixedVisualId.empty())) ||
			 trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) ||
			 trigger.iFixedLifetimeMs > 600000u || trigger.iSpawnIntervalMs == 0u ||
			 trigger.iSpawnIntervalMs > 600000u || !std::isfinite(trigger.fFollowSpeedScale) ||
			 trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
		{
			status = "Showtime player-target visual or timing contract is invalid";
			return false;
		}
		if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
			 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET ||
			 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION) &&
			(trigger.iDurationMs > 600000u || !trigger.strFixedVisualId.empty() ||
			 !trigger.strTrackingVisualId.empty() || trigger.iFixedLifetimeMs != 0u ||
			 trigger.iSpawnIntervalMs != 0u ||
			 (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
			  trigger.fFollowSpeedScale != 0.f) ||
			 // scale 0은 회전만 허용하고, 양수 scale은 추적 대상 player 속도에 대한 이동 비율로 사용한다.
			 !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < 0.f ||
			 (trigger.fFollowSpeedScale != 0.f &&
			  (trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f)) ||
			 trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f ||
			 trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() ||
			 !trigger.ClockHours.empty() || trigger.fFaceCenterYawOffsetDegrees != 0.f ||
			 trigger.iCountPerPlayer != 0u || trigger.fPlayerEffectRadiusM != 0.f ||
			 trigger.iEffectLifetimeMs != 0u || trigger.iArenaRandomCount != 0u || trigger.bRandomPlayerOnly ||
			 !trigger.PatternSpawns.empty()))
		{
			status = "Boss tracking duration must not create visuals or carry another mechanic's values";
			return false;
		}
		// 소환 요청은 spawn ID 중복, 자기 패턴 참조, anchor별 좌표 범위부터 검사한다. 실제 child 존재와 owner 조합 검증은 Catalog/GameRoom이 이어서 수행한다.
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
		{
			std::unordered_set<std::string> spawnIds;
			if (trigger.PatternSpawns.empty() || trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
			{
				status = "Summon Pattern trigger requires one to sixteen spawns";
				return false;
			}
			for (const auto& spawn : trigger.PatternSpawns)
			{
				if (spawn.strSpawnId.empty() || !spawnIds.insert(spawn.strSpawnId).second ||
					spawn.strPatternId.empty() || spawn.strPatternId == pattern.strPatternId ||
					!std::isfinite(spawn.fYawOffsetDegrees) || std::abs(spawn.fYawOffsetDegrees) > 360.f ||
					(spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::BOSS &&
					 spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP) ||
					std::any_of(
						spawn.PositionOffset.begin(), spawn.PositionOffset.end(), [&](const float value) {
							return !std::isfinite(value) ||
								std::abs(value) >
								(spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP ? 100000.f : 1000.f);
						}))
				{
					status = "Summon Pattern spawn identity or transform is invalid";
					return false;
				}
			}
		}
		if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS) !=
				!trigger.PlayerEntryPositions.empty() ||
			trigger.PlayerEntryPositions.size() > 4u ||
			std::any_of(
				trigger.PlayerEntryPositions.begin(),
				trigger.PlayerEntryPositions.end(),
				[](const auto& position) {
					return std::any_of(position.begin(), position.end(), [](const float value) {
						return !std::isfinite(value) || std::abs(value) > 100000.f;
					});
				}))
		{
			status = "Card maze staging requires one to four finite destinations on its own trigger";
			return false;
		}
		// 모든 trigger의 ID 중복과 시작/종료 범위를 검사한다. teleport는 BossMotion과 병행할 수 없다.
		if (trigger.strTriggerId.empty() || !triggerIds.insert(trigger.strTriggerId).second ||
			trigger.iStartMs >= patternDurationMs || 0u == trigger.iDurationMs ||
			static_cast<std::uint64_t>(trigger.iStartMs) + trigger.iDurationMs > patternDurationMs ||
			((BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS == trigger.eKind) &&
			 (pattern.BossMotion || !std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) ||
			  !std::isfinite(trigger.fTeleportZ) || std::abs(trigger.fTeleportX) > 100000.f ||
			  std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f)) ||
			(BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT == trigger.eKind &&
			 (pattern.BossMotion || trigger.strClonePatternId.empty() || trigger.ClockHours.size() != 3u)))
		{
			status = "KoukuSaydon mechanic trigger is invalid";
			return false;
		}
	}
	// LogicWindow의 kind별 입력/결과 허용 범위와 판정 시간을 검사한다. 실행 시 플레이어 판정 기록은 LogicRuntime이 소유한다.
	std::unordered_set<std::string> windowIds;
	for (const BOSS_PATTERN_LOGIC_WINDOW& window : pattern.LogicWindows)
	{
		const std::uint64_t endMs = static_cast<std::uint64_t>(window.iStartMs) + window.iDurationMs;
		const bool endTickKind = BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS == window.eKind;
		if (window.bGazeDuringWindow &&
			(window.eKind != BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS || !window.bInsideIsFail ||
			 !window.OnSuccess.empty() || !window.OnTimeout.empty()))
		{
			status = "Gaze during window requires facing FAIL and only Fail outcomes";
			return false;
		}
		bool valuesValid = true;
		if (((window.bRearmOnExit || window.bRepeatAfterKnockback) &&
			 window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA) ||
			(window.iRepeatIntervalMs && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
			 window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) ||
			(window.bRearmOnExit && window.bRepeatAfterKnockback) || window.iRepeatIntervalMs > 600000u ||
			(window.iRepeatIntervalMs && (window.bRearmOnExit || window.bRepeatAfterKnockback)))
		{
			status = "ENTER_AREA accepts one contact repeat policy";
			return false;
		}
		if (window.bRepeatAfterKnockback &&
			(window.OnSuccess.size() != 1u ||
			 window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			 window.OnSuccess.front().fPushRangeM <= 0.f || window.OnSuccess.front().iPushMs == 0u))
		{
			status = "Repeat after knockback requires one damage Success with positive knockback";
			return false;
		}
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT:
			valuesValid = !window.PatternIds.empty() && window.PatternIds.size() <= 16u &&
				window.iCompletionCount > 0u && window.iCompletionCount <= window.PatternIds.size() &&
				window.CardRegions.empty() && window.OnFail.empty() && window.OnTimeout.empty() &&
				window.OnSuccess.size() <= 1u &&
				(window.OnSuccess.empty() ||
				 window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN);
			break;
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
			valuesValid = window.CardRegions.size() == 8u ||
				(window.iSectorCount >= 2u && window.SectorSymbols.size() == window.iSectorCount &&
				 window.fOuterRadiusM > 0.f);
			break;
		case BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE:
			valuesValid =
				window.iThreshold <= 4u && !window.CardRegions.empty() && window.OnSuccess.empty() &&
				window.OnFail.empty() && window.OnTimeout.empty() && !window.bEndsPatternOnSuccess &&
				std::all_of(window.CardRegions.begin(), window.CardRegions.end(), [](const auto& region) {
					return region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && !region.WorldTrack.bEnabled;
				});
			break;
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
			valuesValid = !window.CardRegions.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
			valuesValid = !window.CardRegions.empty() && !window.strTargetWorldInstanceId.empty() &&
				std::isfinite(window.fTargetWorldX) && std::isfinite(window.fTargetWorldZ) &&
				std::abs(window.fTargetWorldX) <= 100000.f && std::abs(window.fTargetWorldZ) <= 100000.f &&
				window.fTargetRadiusM >= .01f && window.fTargetRadiusM <= 1000.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			valuesValid = !window.CardRegions.empty() && !window.ContactTargets.empty() &&
				window.ContactTargets.size() <= 64u && !window.OnSuccess.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
			break;
		case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
		case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
			valuesValid = window.CardRegions.empty() && window.OnSuccess.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
			break;
		case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			valuesValid = window.CardRegions.empty() && window.OnFail.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
			valuesValid = window.fHalfAngleDegrees > 0.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
			valuesValid = window.iPoseIndex < 4u;
			break;
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
			valuesValid = window.iThreshold > 0u;
			break;
		case BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES:
			valuesValid = window.iThreshold >= 1u && window.iThreshold <= 10u && window.CardRegions.empty() &&
				!window.bInsideIsFail && !window.bEndsPatternOnSuccess && window.OnTimeout.empty();
			break;
		default:
			valuesValid = false;
			break;
		}
		// ENTER_AREA 돌진은 BossMotion과 겹치지 않아야 한다. 서로 겹친 돌진창과 그 기간의 ENTER action도 배제한다.
		if (!std::isfinite(window.fBossChargeDistanceM) || window.fBossChargeDistanceM < 0.f ||
			window.fBossChargeDistanceM > 1000.f || !std::isfinite(window.fChargeYawOffsetDegrees) ||
			std::abs(window.fChargeYawOffsetDegrees) > 360.f ||
			(window.fChargeYawOffsetDegrees != 0.f && window.fBossChargeDistanceM <= 0.f) ||
			(window.fBossChargeDistanceM > 0.f &&
			 (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || pattern.BossMotion)))
			valuesValid = false;
		if (window.fBossChargeDistanceM > 0.f)
		{
			for (const auto& other : pattern.LogicWindows)
				if (&other != &window && other.fBossChargeDistanceM > 0.f && other.iStartMs < endMs &&
					window.iStartMs < std::uint64_t(other.iStartMs) + other.iDurationMs)
					valuesValid = false;
			std::uint64_t stageStartMs = 0u;
			for (const auto& stage : pattern.Stages)
			{
				if (!stage.Actions.empty() && window.iStartMs <= stageStartMs && stageStartMs < endMs)
					valuesValid = false;
				stageStartMs += stage.iDurationMs;
			}
		}
		// 플레이어 capture는 sole success 결과여야 한다. 이를 유지할 ATTACHMENT_HOLD 창이 실제 시간 구간을 덮는지 대조한다.
		const auto captureCount =
			std::count_if(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
				return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
			});
		if (captureCount > 1 || (captureCount == 1 && window.OnSuccess.size() != 1u) ||
			(captureCount == 1) != !window.strHoldLogicOccurrenceId.empty())
			valuesValid = false;
		if (!window.strHoldLogicOccurrenceId.empty())
		{
			const auto hold =
				std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
					return row.strWindowId == window.strHoldLogicOccurrenceId;
				});
			if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || hold == pattern.LogicWindows.end() ||
				hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD || hold->iStartMs > window.iStartMs ||
				std::uint64_t(hold->iStartMs) + hold->iDurationMs < endMs)
				valuesValid = false;
		}
		// success/fail/timeout 결과 배열을 같은 규칙으로 검사한다. 값 단위는 피해 percent/양, 이동 m, 지속 ms이고 종류별 조합 제한이 다르다.
		const auto resultsValid = [&window](const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results) {
			if (results.size() > 4u)
				return false;
			for (const BOSS_PATTERN_LOGIC_RESULT& result : results)
			{
				if (result.iPercent > 100u ||
					(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
					 result.iPercent == 0u))
					return false;
				if (!std::isfinite(result.fPushRangeM) || result.fPushRangeM < 0.f ||
					result.fPushRangeM > (result.bPushBallistic ? 100.f : 20.f) ||
					(result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_BOSS &&
					 result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD &&
					 result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD &&
					 result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					result.iPushMs > (result.bPushBallistic ? 5000u : 600000u) ||
					(result.bPushBallistic && result.iPushMs < 100u) || !std::isfinite(result.fPushHeightM) ||
					result.fPushHeightM < 0.f || result.fPushHeightM > 100.f ||
					(result.fPushHeightM > 0.f && !result.bPushBallistic) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT &&
					 ((result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					  &results == &window.OnTimeout || window.CardRegions.empty() ||
					  (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
					   window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP))) ||
					((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) != (result.iPushMs > 0u)) ||
					((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) &&
					 (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
					  result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE)) ||
					((result.bForcePush || result.bPushCanLeaveArena) && result.fPushRangeM <= 0.f &&
					 result.fPushHeightM <= 0.f) ||
					!std::isfinite(result.fPushYawOffsetDegrees) ||
					std::abs(result.fPushYawOffsetDegrees) > 360.f ||
					(result.fPushYawOffsetDegrees != 0.f &&
					 (result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD ||
					  (result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f))))
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE
						? (result.iDamageAmount == 0u || result.iDamageAmount > 1000000000u ||
						   result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())
						: result.iDamageAmount != 0u)
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY &&
					(!result.iDurationMs || result.iDurationMs > 600000u || result.iPercent ||
					 !result.strPatternId.empty()))
					return false;
				const bool worldMotion =
					BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind;
				if ((worldMotion &&
					 (result.strTargetWorldInstanceId.empty() || result.strMotionInstanceId.empty() ||
					  result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())) ||
					(!worldMotion &&
					 (!result.strTargetWorldInstanceId.empty() || !result.strMotionInstanceId.empty())))
					return false;
				const bool contactMotion =
					result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
				const bool contactSignal =
					result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
				if (((contactMotion || contactSignal) &&
					 window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT) ||
					(window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && !contactMotion &&
					 !contactSignal) ||
					(contactMotion && result.ContactMotions.size() != window.ContactTargets.size()) ||
					(contactSignal && result.strTargetLogicOccurrenceId.empty()))
					return false;
				const bool capture = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
				if (capture
						? (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA ||
						   &results != &window.OnSuccess ||
						   result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
						   result.iPercent || result.iDurationMs ||
						   std::any_of(
							   result.GripLocalOffset.begin(),
							   result.GripLocalOffset.end(),
							   [](const float value) {
								   return !std::isfinite(value) || std::abs(value) > 10.f;
							   }))
						: (result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ||
						   result.GripLocalOffset != std::array<float, 3u>{}))
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR &&
					(result.iDurationMs == 0u || result.iDurationMs > 600000u || result.iPercent != 0u ||
					 result.strFearPresentationId.empty()))
					return false;
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::NONE == result.eKind)
					return false;
				// 보스 단위 완료 결과만 후속 pattern을 넘긴다. 플레이어별 판정창은 여기서 보스 패턴 전환을 요청할 수 없다.
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind &&
					BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT != window.eKind)
					return false;
			}
			return true;
		};
		// grip 위치를 담은 track은 단일 물체 잡기 결과를 가진 WORLD ENTER_AREA 창의 제약을 추가로 지켜야 한다.
		for (const auto& region : window.CardRegions)
		{
			const auto& track = region.WorldTrack;
			const bool hasGrip = std::any_of(track.Keys.begin(), track.Keys.end(), [](const auto& key) {
				return key.bHasGripPosition;
			});
			if (!hasGrip)
				continue;
			if (!track.bEnabled || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::WORLD ||
				window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || window.OnSuccess.size() != 1u ||
				window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT ||
				!window.OnFail.empty() || !window.OnTimeout.empty() || region.bCircle || region.bSector ||
				track.iStartMs != window.iStartMs || track.iDurationMs < window.iDurationMs ||
				track.iStartDelayMs || track.fPlaybackSpeed != 1.f || track.bSmoothStep ||
				track.fBaselineX != 0.f || track.fBaselineY != 0.f || track.fBaselineZ != 0.f ||
				track.fBaselineYawDegrees != 0.f || track.fBaselineScaleX != 1.f ||
				track.fBaselineScaleY != 1.f || track.fBaselineScaleZ != 1.f ||
				track.Keys.front().iTimeMs != 0u || track.Keys.back().iTimeMs != track.iDurationMs ||
				std::any_of(
					track.Keys.begin(),
					track.Keys.end(),
					[](const auto& key) {
						return !key.bHasGripPosition ||
							std::any_of(key.GripPosition.begin(), key.GripPosition.end(), [](float x) {
								return !std::isfinite(x) || std::abs(x) > 100000.f;
							});
					}))
			{
				status =
					"Physical hook grip requires a complete identity-baseline ENTER_AREA BOX track and sole world-hook result.";
				return false;
			}
		}
		// 앞선 종류별 검사 뒤 공통 window ID/기간/범위 및 세 결과 배열을 최종 확인한다.
		if (window.strWindowId.empty() || !windowIds.insert(window.strWindowId).second ||
			0u == window.iDurationMs || endMs > patternDurationMs || !valuesValid ||
			(window.bInsideIsFail && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
			 BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
			 BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS != window.eKind) ||
			std::any_of(
				window.CardRegions.begin(),
				window.CardRegions.end(),
				[&](const BOSS_LOGIC_REGION& region) {
					return region.WorldTrack.bEnabled &&
						(region.WorldTrack.Keys.empty() ||
						 (BOSS_PATTERN_LOGIC_KIND::ENTER_AREA != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT != window.eKind) ||
						 region.WorldTrack.iStartMs > window.iStartMs);
				}) ||
			!resultsValid(window.OnSuccess) || !resultsValid(window.OnFail) ||
			!resultsValid(window.OnTimeout) || (endTickKind && !window.OnTimeout.empty()) ||
			(BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind && !window.OnFail.empty()))
		{
			status =
				"KoukuSaydon pattern logic window is out of the pattern lifetime or carries invalid values: " +
				pattern.strPatternId + "/" + window.strWindowId;
			return false;
		}
	}
	// World cue는 combat body, placement override 등 선택한 기능의 필수 ID/수치와 금지 조합을 확인한다. 실제 world instance 조회는 별도다.
	for (const BOSS_PATTERN_WORLD_SEQUENCE& sequence : pattern.WorldSequences)
	{
		if (sequence.CombatBody)
		{
			const auto& body = *sequence.CombatBody;
			if (sequence.strOccurrenceId.empty() || sequence.bAnchorBossSpawn ||
				!sequence.SupportWindows.empty() || !body.iMaximumHp || body.iMaximumHp > 1000000000u ||
				!std::isfinite(body.fRadiusM) || body.fRadiusM <= .001f || body.fRadiusM > 1000.f ||
				!std::isfinite(body.fCenterX) || !std::isfinite(body.fCenterY) ||
				!std::isfinite(body.fCenterZ) || std::abs(body.fCenterX) > 100000.f ||
				std::abs(body.fCenterY) > 100000.f || std::abs(body.fCenterZ) > 100000.f)
			{
				status = "World combat body is outside the fixed owned cue contract";
				return false;
			}
		}
		if (sequence.Placement)
		{
			const auto& placement = *sequence.Placement;
			const float values[] = {
				placement.fPositionX,
				placement.fPositionY,
				placement.fPositionZ,
				placement.fRotationXDegrees,
				placement.fRotationYDegrees,
				placement.fRotationZDegrees,
				placement.fScaleX,
				placement.fScaleY,
				placement.fScaleZ};
			bool valid = !sequence.strOccurrenceId.empty() && sequence.SupportWindows.empty() &&
				!sequence.bAnchorBossSpawn && sequence.fPositionOffsetX == 0.f &&
				sequence.fPositionOffsetY == 0.f && sequence.fPositionOffsetZ == 0.f &&
				sequence.fAnchorPositionX == 0.f && sequence.fAnchorPositionY == 0.f &&
				sequence.fAnchorPositionZ == 0.f;
			for (std::size_t index = 0u; index < 9u; ++index)
				valid = valid && std::isfinite(values[index]) &&
					(index < 3u		  ? std::abs(values[index]) <= 100000.f
						 : index < 6u ? std::abs(values[index]) <= 36000.f
									  : values[index] >= .001f && values[index] <= 1000.f);
			if (!valid)
			{
				status = "KoukuSaydon absolute World placement or occurrence ownership is invalid";
				return false;
			}
		}
		if (sequence.strInstanceId.empty() || sequence.iStartMs > patternDurationMs ||
			sequence.iDurationMs > 600000u ||
			!(sequence.fPlaybackSpeed >= 0.05f && sequence.fPlaybackSpeed <= 16.f))
		{
			status = "KoukuSaydon pattern world sequence cue is invalid";
			return false;
		}
	}
	// scene profile cue는 비어 있지 않은 ID와 pattern timeline 안의 시작 시각을 요구한다. profile 존재를 여기서 조회하지 않는다.
	for (const BOSS_PATTERN_SCENE_PROFILE& profile : pattern.SceneProfiles)
	{
		if (profile.strProfileId.empty() || profile.iStartMs > patternDurationMs)
		{
			status = "KoukuSaydon pattern scene profile cue is invalid";
			return false;
		}
	}
	status.clear();
	return true;
}

// Catalog의 소환 참조 검사와 방향 분신 선택에서 사용한다. child 기본 계약을 확인한 뒤 owner와 함께 실행 가능한 제한을 적용한다.
// allowActorLocalAirborne은 지정된 JUMP/SLAM만 추가 허용하며 실제 actor 생성/수명 관리는 이 함수 밖에 있다.
bool LostArk::Server::CKoukuSaydonBrain::Validate_SummonedPattern(
	const BOSS_PATTERN_DEFINITION& owner,
	const BOSS_PATTERN_DEFINITION& child,
	std::string& status,
	const bool allowActorLocalAirborne)
{
	if (!Validate_AnimationOnlyPattern(child, status))
		return false;
	if (owner.strPatternId == child.strPatternId || owner.strEncounterId != child.strEncounterId ||
		owner.strGateId.empty() || owner.strGateId != child.strGateId ||
		child.eCategory != BOSS_PATTERN_CATEGORY::MECHANIC ||
		owner.strTargetBossPlacementId != child.strTargetBossPlacementId ||
		owner.AuditionBossArchetypeIds.empty() ||
		owner.AuditionBossArchetypeIds.size() != child.AuditionBossArchetypeIds.size() ||
		!std::all_of(
			owner.AuditionBossArchetypeIds.begin(),
			owner.AuditionBossArchetypeIds.end(),
			[&](const auto& archetype) {
				return std::find(
						   child.AuditionBossArchetypeIds.begin(),
						   child.AuditionBossArchetypeIds.end(),
						   archetype) != child.AuditionBossArchetypeIds.end();
			}) ||
		child.BossMotion || child.bResetBossToSpawn || child.ResetBossYawDegrees ||
		std::any_of(
			child.LogicWindows.begin(),
			child.LogicWindows.end(),
			[](const auto& window) {
				if (window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP &&
					window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
					return true;
				const auto unsupported = [](const auto& result) {
					return result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE &&
						result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
						result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
				};
				return std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), unsupported) ||
					std::any_of(window.OnFail.begin(), window.OnFail.end(), unsupported) ||
					std::any_of(window.OnTimeout.begin(), window.OnTimeout.end(), unsupported);
			}) ||
		std::any_of(
			child.MechanicTriggers.begin(),
			child.MechanicTriggers.end(),
			[&](const auto& trigger) {
				if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
					trigger.iDurationMs > 0u && trigger.iDurationMs <= 34u && trigger.fFollowSpeedScale == 0.f)
					return false;
				return !allowActorLocalAirborne ||
					trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
					(trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP &&
					 trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
					trigger.bCaptureAirborneTargetPosition || !trigger.strSelectedEffectVisualId.empty();
			}) ||
		!child.WorldSequences.empty() || !child.SceneProfiles.empty() ||
		std::any_of(child.Stages.begin(), child.Stages.end(), [](const auto& stage) {
			return !stage.Actions.empty();
		}))
	{
		status =
			"Summoned Pattern requires another same-body, same-Gate leaf with only supported actor-local motion and no world, player or reset actions: " +
			child.strPatternId;
		return false;
	}
	status.clear();
	return true;
}

// Catalog의 참조 검사와 BossSimulation의 분신 준비가 호출한다. 네 child의 정의/종료 stage를 모두 확인한 뒤에만 출력을 교체한다.
// 평가 중인 Catalog 정의는 차용하며, 실패는 status만 바꾸고 selected/durations는 유지한다.
// 기분나빠 패턴 기준으로 4방향 이동에 대한 위치를 먼저 계산한 이후에, 중앙과 가까운 위치를 진짜 세이튼으로 설정을 한다. 
bool LostArk::Server::CKoukuSaydonBrain::Select_CrossDirection(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& parent,
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
	const CGameplayCatalog& catalog,
	std::size_t& selected,
	std::array<std::uint32_t, 4u>& cloneDurationsMs,
	std::string& status)
{
	//trigger의 kind가 cross direction clones가 아니거나, trigger
	if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES ||
		trigger.DirectionPatternIds.size() != 4u || trigger.strCloneEndStageId.empty())
	{
		status = "Cross direction requires four Patterns and a clone end Stage";
		return false;
	}
	//staged duration? 4개의 방향을 저장하는 거 아닌가? 왜 이름이 duration이지?
	std::array<std::uint32_t, 4u> stagedDurations{};

	std::size_t stagedSelection = 0u;
	//이게 이제 가장 중앙와 가장 가까운 값을 저장하는 곳이다. 
	//왜 double로 선언이 된 거지?
	double bestDistance = (std::numeric_limits<double>::max)();
	//왜 yawDegrees에 0.0174 이런 식의 소수점 값을 붙인 거지?
	const double yaw = boss.fYawDegrees * 0.01745329251994329577;

	std::unordered_set<std::string> identities;

	for (std::size_t index = 0u; index < 4u; ++index)
	{
		const auto* child = Find_AnimationOnlyPattern(catalog, trigger.DirectionPatternIds[index], status);

		if (!identities.insert(trigger.DirectionPatternIds[index]).second || !child ||
			!Validate_SummonedPattern(parent, *child, status))
			return false;

		double forward = 0., lateral = 0.;
		std::uint64_t duration = 0u;
		bool foundEnd = false;
		for (const auto& stage : child->Stages)
		{
			duration += stage.iDurationMs;
			if (!foundEnd)
			{
				const auto endpoint = Sample_StageRootMotion(stage.Motion.RootMotion, stage.iDurationMs);
				forward += endpoint.fForward;
				lateral += endpoint.fLateral;
				if (stage.strStageId == trigger.strCloneEndStageId)
				{
					foundEnd = true;
					stagedDurations[index] = static_cast<std::uint32_t>(duration);
				}
			}
		}
		if (!child->bFixedTimelineClock || !foundEnd || duration > trigger.iDurationMs ||
			stagedDurations[index] >= duration)
		{
			status =
				"Cross direction child requires a clone end Stage before its full ending, within the Logic duration";
			return false;
		}
		const double x = boss.fPositionX + lateral * std::cos(yaw) + forward * std::sin(yaw);
		const double z = boss.fPositionZ - lateral * std::sin(yaw) + forward * std::cos(yaw);
		const double dx = x - boss.fSpawnPositionX, dz = z - boss.fSpawnPositionZ;
		const double distance = dx * dx + dz * dz;
		if (!std::isfinite(distance) || forward * forward + lateral * lateral < .000001)
		{
			status = "Cross direction child has no finite directional movement at the clone end Stage";
			return false;
		}
		// 거리 비교는 월드 XZ의 제곱 거리다. clone end stage까지 합산한 변위를 owner의 현재 yaw와 위치로 옮긴 뒤 spawn XZ와 비교한다.
		// 현재 최솟값보다 0.00001을 초과하는 차이로 작을 때만 교체하므로 같은 거리와 작은 차이는 저작된 앞/뒤/좌/우 순서를 유지한다.
		if (distance < bestDistance - .00001)
		{
			bestDistance = distance;
			stagedSelection = index;
		}
	}
	// 명시 real ID가 있으면 거리 선택보다 우선한다. 이 경로도 앞서 네 후보 전체의 유효성을 확인한 뒤 적용한다.
	if (!trigger.strRealPatternId.empty())
	{
		const auto fixed = std::find(
			trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end(), trigger.strRealPatternId);
		if (fixed == trigger.DirectionPatternIds.end())
		{
			status = "Cross direction real Pattern is not a candidate";
			return false;
		}
		stagedSelection = static_cast<std::size_t>(fixed - trigger.DirectionPatternIds.begin());
	}
	selected = stagedSelection;
	cloneDurationsMs = stagedDurations;
	status.clear();
	return true;
}

// GameRoom의 Play All 승인에서 호출한다. 먼저 저작 sequence의 각 참조를 검증하고 이후 몸체/관문/placement로 걸러낸다.
// 따라서 최종 선택에서 제외될 패턴도 이 루프에 도달하면 존재/지원 검증은 통과해야 한다.
bool LostArk::Server::CKoukuSaydonBrain::Select_AnimationOnlySequence(
	const std::vector<BOSS_PATTERN_DEFINITION>& definitions,
	const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
	const std::string_view bossArchetypeId,
	std::vector<std::string>& outPatternIds,
	std::vector<std::uint32_t>& outTransitionTicks,
	std::string& status,
	const std::string_view gateId,
	const std::string_view targetBossPlacementId)
{
	if (sequence.PatternIds.empty() || sequence.PatternIds.size() != sequence.iExpectedStepCount ||
		sequence.TransitionPursuitTicks.size() + 1u != sequence.PatternIds.size())
	{
		status = "KoukuSaydon Product pattern sequence is unavailable";
		return false;
	}
	std::vector<std::string> selectedPatterns;
	std::vector<std::uint32_t> selectedTransitions;
	std::size_t previousSelectedIndex = 0u;
	for (std::size_t index = 0u; index < sequence.PatternIds.size(); ++index)
	{
		const std::string& patternId = sequence.PatternIds[index];
		const auto pattern = std::find_if(
			definitions.begin(), definitions.end(), [&patternId](const BOSS_PATTERN_DEFINITION& candidate) {
				return candidate.strPatternId == patternId;
			});
		if (definitions.end() == pattern)
		{
			status = "KoukuSaydon Product sequence names an unknown pattern: " + patternId;
			return false;
		}
		if (!Validate_AnimationOnlyPattern(*pattern, status))
			return false;
		const bool admitted =
			pattern->AuditionBossArchetypeIds.empty() ? KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == bossArchetypeId
													  : pattern->AuditionBossArchetypeIds.end() !=
				std::find(pattern->AuditionBossArchetypeIds.begin(),
						  pattern->AuditionBossArchetypeIds.end(),
						  bossArchetypeId);
		if (!admitted || (!gateId.empty() && !pattern->strGateId.empty() && pattern->strGateId != gateId) ||
			(!targetBossPlacementId.empty() && !pattern->strTargetBossPlacementId.empty() &&
			 pattern->strTargetBossPlacementId != targetBossPlacementId))
			continue;
		// 새로 남길 패턴 앞에 바로 이전에 남긴 패턴의 transition을 넣는다. 건너뛴 row들의 transition 합은 사용하지 않는다.
		if (!selectedPatterns.empty())
			selectedTransitions.push_back(sequence.TransitionPursuitTicks[previousSelectedIndex]);
		selectedPatterns.push_back(patternId);
		previousSelectedIndex = index;
	}
	if (selectedPatterns.empty())
	{
		status =
			"KoukuSaydon Play All has no Product pattern for the target boss: " + std::string{bossArchetypeId};
		return false;
	}
	// 성공 결과만 출력 vector로 이동한다. 앞선 모든 false 경로는 호출자의 기존 선택을 유지한다.
	outPatternIds = std::move(selectedPatterns);
	outTransitionTicks = std::move(selectedTransitions);
	status.clear();
	return true;
}

// GameRoom/GateProgress가 게시 원본 revision을 확인한다. 새 revision을 생성하거나 파일을 읽는 함수는 아니다.
std::uint32_t LostArk::Server::CKoukuSaydonBrain::Resolve_ProductSourceRevision(
	const CGameplayCatalog& catalog) noexcept
{
	return catalog.Find_KoukuSaydonProductSourceRevision(std::string{KOUKUSAYDON_G1_ENCOUNTER_ID});
}

// G1 encounter의 Catalog vector를 차용해 선형 검색한다. 성공 포인터의 수명은 호출자가 유지하는 Catalog 세대에 달려 있다.
const LostArk::Server::BOSS_PATTERN_DEFINITION* LostArk::Server::CKoukuSaydonBrain::Find_AnimationOnlyPattern(
	const CGameplayCatalog& catalog, const std::string_view patternId, std::string& status)
{
	const auto* patterns = catalog.Find_BossPatterns(std::string{KOUKUSAYDON_G1_ENCOUNTER_ID});
	if (nullptr == patterns)
	{
		status = "KoukuSaydon encounter patterns are missing";
		return nullptr;
	}
	const auto found =
		std::find_if(patterns->begin(), patterns->end(), [patternId](const BOSS_PATTERN_DEFINITION& pattern) {
			return pattern.strPatternId == patternId;
		});
	if (patterns->end() == found)
	{
		status = "KoukuSaydon pattern ID is unknown";
		return nullptr;
	}
	return Validate_AnimationOnlyPattern(*found, status) ? &*found : nullptr;
}

// Begin/Update가 공용으로 쓰는 stage 상태 교체다. root sample을 boss에 복사하고 원점/마지막 적용 tick을 초기화한다.
// 첫 stage는 진입 tick부터, 일반적인 다음 stage는 다음 tick부터 평가하도록 firstEvaluationTick을 정한다.
void LostArk::Server::CKoukuSaydonBrain::Enter_Stage(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage,
	const std::uint32_t stageIndex,
	const std::uint32_t serverTick,
	const bool evaluatesOnEntryTick)
{
	boss.PatternStageRootMotion = stage.Motion.RootMotion;
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginY = 0.f;
	boss.fPatternStageRootGroundY = 0.f;
	boss.iPatternStageIndex = stageIndex;
	boss.strPatternStageId = stage.strStageId;
	boss.strActionId = stage.strActionId;
	boss.iPatternStageDurationMs = stage.iDurationMs;
	boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ? serverTick : Next_ServerTick(serverTick);
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = To_ServerAction(stage.eStageKind);
}

// GameRoom은 승인 사전 점검/예약 시작에서 boss 복사본으로 호출하고 별도 준비 후 live 상태에 반영한다.
// 명시적인 false 경로는 boss를 바꾸기 전에 끝난다. 성공 뒤 수행할 spawn reset/ENTER action/Logic 기록은 GameRoom 책임이다.
bool LostArk::Server::CKoukuSaydonBrain::Begin_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (!revision.Is_Valid() || 0u == serverTick || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction || !boss.strPatternId.empty() ||
		KOUKUSAYDON_G1_ENCOUNTER_ID != boss.strEncounterId || !Validate_AnimationOnlyPattern(pattern, status))
	{
		if (status.empty())
			status = "KoukuSaydon pattern cannot begin from the current boss state";
		return false;
	}
	// 새 occurrence의 접촉/방향/공중/소환 시작 기록을 비우고 revision과 sequence를 고정한다. sequence도 예약값 0을 건너뛴다.
	boss.Restore_KoukuPresentationAim();
	boss.PinnedDefinitionRevision = revision;
	boss.KoukuContactLedger.reset();
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.PatternTerminalReceipt = {};
	boss.bPatternRootGrounded = false;
	boss.AlbionAirborne = {};
	boss.KoukuSummonStartedTriggers.clear();
	boss.strPatternId = pattern.strPatternId;
	boss.iPatternStartTick = serverTick;
	boss.iPatternSequence =
		boss.iPatternSequence == (std::numeric_limits<std::uint32_t>::max)() ? 1u : boss.iPatternSequence + 1u;
	Enter_Stage(boss, pattern.Stages.front(), 0u, serverTick, true);
	Apply_BossMotion(boss, pattern, serverTick);
	status.clear();
	return true;
}

// Update/Abort/Complete가 실행을 종료할 때 공용으로 사용한다. 진행 중인 ID가 있을 때만 terminal receipt를 새로 기록한다.
// Brain의 진행 상태만 비우며 외부 소환/플레이어/월드 cue 정리는 GameRoom이 수행한다. pinned revision과 sequence는 남는다.
void LostArk::Server::CKoukuSaydonBrain::Finish_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick, const SERVER_BOSS_PATTERN_TERMINAL_RESULT result)
{
	(void)serverTick;
	if (!boss.strPatternId.empty())
	{
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.eResult = result;
	}
	boss.Restore_KoukuPresentationAim();
	boss.PatternStageRootMotion.clear();
	boss.KoukuContactLedger.reset();
	boss.bPatternRootGrounded = false;
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.AlbionAirborne = {};
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginX = boss.fPatternStageOriginY = boss.fPatternStageOriginZ = 0.f;
	boss.fPatternStageOriginYawDegrees = boss.fPatternStageRootGroundY = 0.f;
	boss.strPatternId.clear();
	boss.iPatternStartTick = 0u;
	boss.strPatternStageId.clear();
	boss.strActionId.clear();
	boss.strDamageProfileId.clear();
	boss.iPatternStageIndex = 0u;
	boss.iPatternStageDurationMs = 0u;
	boss.iPatternStageFirstEvaluationTick = 0u;
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = 0u == boss.iCurrentHp ? SERVER_ENTITY_ACTION::DEAD : SERVER_ENTITY_ACTION::IDLE;
}

// GameRoom이 이번 tick의 이동/Logic 처리를 마친 뒤 호출한다. 정의 불일치나 사망은 receipt를 남기고 실행을 중단한다.
// 여기서는 받은 Catalog에서 조회하며, 실행 revision에 맞는 Catalog를 선택하는 일은 GameRoom이 담당한다.
// 보충: 위 순서는 주 보스의 일반 tick 기준이다. 방향 child는 별도 복사본에서 root 이동과 actor contact 판정 뒤 Update를 호출한다.
LostArk::Server::KOUKUSAYDON_BRAIN_UPDATE_RESULT LostArk::Server::CKoukuSaydonBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick,
	std::string& status) const
{
	// pattern ID가 없으면 HP 검사보다 먼저 IDLE을 반환한다. 실행 중인 경우에만 아래 사망/정의 오류 종료가 적용된다.
	if (boss.strPatternId.empty())
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
	}
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		status = "KoukuSaydon boss died during pattern playback";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD;
	}
	const BOSS_PATTERN_DEFINITION* pattern = Find_AnimationOnlyPattern(catalog, boss.strPatternId, status);
	if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size() ||
		pattern->Stages[boss.iPatternStageIndex].strStageId != boss.strPatternStageId ||
		pattern->Stages[boss.iPatternStageIndex].strActionId != boss.strActionId)
	{
		Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		if (status.empty())
			status = "KoukuSaydon running stage no longer matches its pinned definition";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION;
	}
	Apply_BossMotion(boss, *pattern, serverTick);
	if (pattern->bFixedTimelineClock)
	{
		// 각 stage마다 올림한 기간을 누적하지 않고 pattern 공통 시계의 누적 ms 경계와 비교한다.
		// Logic/이펙트와 같은 시작 시계를 사용하며, 지연된 호출은 이미 지난 여러 stage 경계를 한 번에 넘길 수 있다.
		const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
			? serverTick - boss.iPatternStartTick
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
				serverTick;
		std::uint64_t startMs = 0u;
		std::uint32_t selected = 0u;
		while (selected < pattern->Stages.size() &&
			   ticks * MILLISECONDS_PER_SECOND >=
				   (startMs + pattern->Stages[selected].iDurationMs) * SERVER_TICK_HZ)
			startMs += pattern->Stages[selected++].iDurationMs;
		if (selected == pattern->Stages.size())
		{
			Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
			status.clear();
			return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
		}
		const bool changed = selected != boss.iPatternStageIndex;
		if (changed)
			Enter_Stage(boss, pattern->Stages[selected], selected, serverTick, false);
		const auto offset = static_cast<std::uint32_t>(
			(startMs * SERVER_TICK_HZ + MILLISECONDS_PER_SECOND - 1u) / MILLISECONDS_PER_SECOND);
		boss.iActionStartTick = boss.iPatternStartTick + offset;
		if (boss.iActionStartTick < boss.iPatternStartTick)
			++boss.iActionStartTick;
		if (boss.iActionStartTick == 0u)
			boss.iActionStartTick = 1u;
		boss.fActionElapsedSeconds =
			static_cast<float>((std::max)(0.0,
										  static_cast<double>(ticks) / SERVER_TICK_HZ -
											  static_cast<double>(startMs) / MILLISECONDS_PER_SECOND));
		status.clear();
		return changed ? KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED
					   : KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint64_t elapsedTicks = Stage_ElapsedTicks(boss, serverTick);
	boss.fActionElapsedSeconds = static_cast<float>(elapsedTicks) / static_cast<float>(SERVER_TICK_HZ);
	const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern->Stages[boss.iPatternStageIndex];
	// 일반 시계는 이 호출에서 stage를 최대 한 번 전환한다. 지연분을 여러 stage로 소비하는 고정 시계와 구분한다.
	if (!Has_ElapsedMilliseconds(elapsedTicks, stage.iDurationMs))
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint32_t nextStage = boss.iPatternStageIndex + 1u;
	if (nextStage < pattern->Stages.size())
	{
		Enter_Stage(boss, pattern->Stages[nextStage], nextStage, serverTick, false);
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED;
	}
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
	status.clear();
	return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
}

// GameRoom의 Logic 전처리와 Brain Update 양쪽에서 호출한다. 동일한 pattern 경과 시각의 절대 XYZ/yaw를 다시 써 누적 이동을 만들지 않는다.
// 이 함수 자체는 navigation/collision을 조회하지 않는다. GameRoom 승인은 key가 있으면 시작 지면만, 없으면 시작/끝과 XZ 경로를 확인한다.
void LostArk::Server::CKoukuSaydonBrain::Apply_BossMotion(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, const std::uint32_t serverTick) noexcept
{
	if (!pattern.BossMotion || !boss.iPatternStartTick || boss.strPatternId != pattern.strPatternId)
		return;
	const auto& motion = *pattern.BossMotion;
	if (motion.iEndMs <= motion.iStartMs)
		return;
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
		? serverTick - boss.iPatternStartTick
		: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
			serverTick;
	const auto position = Sample_BossMotion(motion, static_cast<std::uint32_t>(ticks));
	boss.fPositionX = position[0];
	boss.fPositionY = position[1];
	boss.fPositionZ = position[2];
	boss.fYawDegrees = motion.fYawDegrees;
}

// BossMotion key가 있으면 endpoint clamp와 선형 보간을 사용한다. 없으면 시작/종료 XZ만 보간하고 Y는 시작 높이를 유지한다.
// 키 순서와 유효 범위는 사전 validator의 계약이며 함수 내부에서 전체 키를 재검증하지 않는다.
std::array<float, 3u> LostArk::Server::CKoukuSaydonBrain::Sample_BossMotion(
	const BOSS_PATTERN_BOSS_MOTION& motion, const std::uint32_t elapsedTicks) noexcept
{
	if (motion.iEndMs <= motion.iStartMs)
		return motion.StartPosition;
	const double timeMs = static_cast<double>(elapsedTicks) * 1000.0 / SERVER_TICK_HZ;
	if (!motion.Keys.empty())
	{
		if (timeMs <= motion.Keys.front().iTimeMs)
			return motion.Keys.front().Position;
		if (timeMs >= motion.Keys.back().iTimeMs)
			return motion.Keys.back().Position;
		const auto next =
			std::upper_bound(motion.Keys.begin(), motion.Keys.end(), timeMs, [](double time, const auto& key) {
				return time < key.iTimeMs;
			});
		const auto& previous = *(next - 1);
		const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
		std::array<float, 3u> position{};
		for (std::size_t axis = 0u; axis < position.size(); ++axis)
			position[axis] = static_cast<float>(
				previous.Position[axis] + (next->Position[axis] - previous.Position[axis]) * alpha);
		return position;
	}
	const double alpha =
		(std::clamp)((timeMs - motion.iStartMs) / static_cast<double>(motion.iEndMs - motion.iStartMs),
					 0.0,
					 1.0);
	return {
		static_cast<float>(motion.StartPosition[0] + (motion.EndPosition[0] - motion.StartPosition[0]) * alpha),
		motion.StartPosition[1],
		static_cast<float>(
			motion.StartPosition[2] + (motion.EndPosition[2] - motion.StartPosition[2]) * alpha)};
}

// stage 로컬 시간 ms의 상대 F/L/U 변위를 구한다. 정렬된 키 사이를 선형 보간하며 입력 시간 밖에서는 끝점을 쓴다.
LostArk::Server::ROOT_MOTION_SAMPLE LostArk::Server::CKoukuSaydonBrain::Sample_StageRootMotion(
	const std::vector<ROOT_MOTION_SAMPLE>& samples, const double timeMs) noexcept
{
	if (samples.empty() || !std::isfinite(timeMs))
		return {};
	if (timeMs <= samples.front().iTimeMs)
		return samples.front();
	if (timeMs >= samples.back().iTimeMs)
		return samples.back();
	const auto next = std::upper_bound(
		samples.begin(), samples.end(), timeMs, [](const double time, const ROOT_MOTION_SAMPLE& sample) {
			return time < sample.iTimeMs;
		});
	const auto& previous = *(next - 1);
	const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
	return {
		static_cast<std::uint32_t>(timeMs),
		static_cast<float>(previous.fForward + (next->fForward - previous.fForward) * alpha),
		static_cast<float>(previous.fLateral + (next->fLateral - previous.fLateral) * alpha),
		static_cast<float>(previous.fUp + (next->fUp - previous.fUp) * alpha)};
}

// 30Hz pattern 경과 tick을 ms로 바꾼다. UINT32_MAX에서 1로 넘어가는 wrap을 포함하며 유효 시작 tick은 호출자가 보장한다.
double LostArk::Server::CKoukuSaydonBrain::Pattern_ElapsedMs(
	const SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) noexcept
{
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
		? serverTick - boss.iPatternStartTick
		: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
			serverTick;
	return double(ticks) * 1000.0 / SERVER_TICK_HZ;
}

// 고정 시계는 pattern 공통 시간에서 이전 stage들의 저작 ms를 뺀다. 일반 시계는 첫 평가 tick을 포함하는 stage 시계를 쓴다.
double LostArk::Server::CKoukuSaydonBrain::Stage_RootTimeMs(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick) noexcept
{
	if (!pattern.bFixedTimelineClock)
		return double(Stage_ElapsedTicks(boss, serverTick)) * 1000.0 / SERVER_TICK_HZ;
	std::uint64_t start = 0u;
	for (std::uint32_t index = 0u; index < boss.iPatternStageIndex && index < pattern.Stages.size(); ++index)
		start += pattern.Stages[index].iDurationMs;
	return (std::max)(0.0, Pattern_ElapsedMs(boss, serverTick) - double(start));
}

// BossSimulation의 공중 phase 전환과 아래 root 이동이 사용한다. 높이는 현재 지면 위 m이고 SLAM 진행률은 역행하지 않게 누적한다.
// state/outHeight는 참조 출력이므로 false라도 변경될 수 있다. root 이동 호출자는 지역 airborneState를 넘겨 최종 성공 때 반영한다.
bool LostArk::Server::CKoukuSaydonBrain::Sample_AlbionAirborneHeight(
	SERVER_ALBION_AIRBORNE_STATE& state,
	const double patternMs,
	const std::uint32_t stageIndex,
	const float sourceUp,
	float& outHeight) noexcept
{
	using Phase = ALBION_AIRBORNE_PHASE;
	switch (state.ePhase)
	{
	case Phase::JUMP:
		if (state.iDurationMs == 0u)
		{
			outHeight = state.fJumpHeightM;
			break;
		}
		outHeight = static_cast<float>(
			state.fStartHeightM +
			(state.fJumpHeightM - state.fStartHeightM) *
				(std::clamp)((patternMs - state.iStartMs) / state.iDurationMs, 0.0, 1.0));
		break;
	case Phase::APPEAR_PLAYER:
		outHeight = stageIndex == state.iSourceStageIndex ? state.fPhaseHeightM + sourceUp : 0.f;
		break;
	case Phase::DISAPPEAR:
	case Phase::CENTER:
		outHeight = state.fJumpHeightM;
		break;
	case Phase::SLAM:
		if (state.fSourceUpAtStart - state.fSourceUpMinimum <= .000001f)
			return false;
		if (stageIndex != state.iSourceStageIndex)
			state.fLandingProgress = 1.f;
		else
			state.fLandingProgress =
				(std::max)(state.fLandingProgress,
						   (std::clamp)((state.fSourceUpAtStart - sourceUp) /
											(state.fSourceUpAtStart - state.fSourceUpMinimum),
										0.f,
										1.f));
		outHeight = state.fStartHeightM * (1.f - state.fLandingProgress);
		break;
	default:
		return false;
	}
	return std::isfinite(outHeight) && std::abs(outHeight) <= 100000.f;
}

// GameRoom이 Logic 전에 호출한다. navigation/collision은 Room 소유의 차용 입력이고, 현재 root 기준 시각의 후보 XYZ를 검사한다.
// 최종 좌표는 끝에서 반영하지만 최초 origin capture는 뒤 검사가 실패해도 남을 수 있다. true에는 경계 위치 유지와 no-op도 포함된다.
// 보충: 주 보스는 일반 tick의 Logic 전에, 방향 child는 접촉 판정 전에 사용한다. Commit_KoukuAlbionAirborne의 SLAM 전환도 staged 상태로 호출한다.
bool LostArk::Server::CKoukuSaydonBrain::Apply_StageRootMotion(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick,
	const CServerNavigation& navigation,
	const CServerCollisionSystem& collision,
	std::string& status)
{
	status.clear();
	const bool airborne = boss.AlbionAirborne.iPatternSequence == boss.iPatternSequence &&
		boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::NONE &&
		(boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::SLAM ||
		 boss.iPatternStageIndex == boss.AlbionAirborne.iSourceStageIndex);
	// 이동할 것이 없거나 이미 처리한 tick이면 true다. 이 조기 반환은 아래 serverTick/현재 stage 검사보다 먼저 실행된다.
	if ((!airborne && boss.PatternStageRootMotion.empty()) || boss.strPatternId.empty() ||
		boss.iPatternStageRootLastTick == serverTick)
		return true;
	if (serverTick == 0u || boss.strPatternId != pattern.strPatternId ||
		boss.iPatternStageIndex >= pattern.Stages.size())
	{
		status = "KoukuSaydon root motion lost its running stage";
		return false;
	}
	// GameRoom이 spawn reset과 stage ENTER action을 반영한 뒤 첫 root 이동에서 원점을 capture한다.
	SERVER_NAV_POINT originGround;
	if (!boss.bPatternStageRootOriginCaptured)
	{
		if (!std::isfinite(boss.fPositionX) || !std::isfinite(boss.fPositionY) ||
			!std::isfinite(boss.fPositionZ) || !std::isfinite(boss.fYawDegrees) ||
			!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, originGround))
		{
			status = "KoukuSaydon root motion origin is not navigable";
			return false;
		}
		boss.fPatternStageOriginX = boss.fPositionX;
		// 명시적으로 착지한 뒤 다음 stage를 capture할 때는 현재 지면을 Y 기준으로 써 이전 clip의 잔여 Up을 누적하지 않는다.
		// 착지 표시가 없는 경우에는 현재 boss Y를 기준으로 유지한다.
		boss.fPatternStageOriginY = boss.bPatternRootGrounded ? originGround.y : boss.fPositionY;
		boss.fPatternStageOriginZ = boss.fPositionZ;
		boss.fPatternStageOriginYawDegrees = boss.fYawDegrees;
		boss.fPatternStageRootGroundY = originGround.y;
		boss.bPatternStageRootOriginCaptured = true;
	}
	// 고정 origin과 현재 stage sample로 후보를 만든다. origin을 매 tick 갱신해 상대 변위를 다시 누적하지 않는다.
	const double elapsedMs = Stage_RootTimeMs(boss, pattern, serverTick);
	const auto sample = Sample_StageRootMotion(boss.PatternStageRootMotion, elapsedMs);
	constexpr double RADIANS_PER_DEGREE = 3.14159265358979323846 / 180.0;
	const double yaw = boss.fPatternStageOriginYawDegrees * RADIANS_PER_DEGREE;
	// 아래 변환은 lateral을 model X, forward를 model Z로 읽고 capture한 yaw로 월드 XZ에 회전한다.
	// 시각적 전방은 보스 몸체별 yaw 보정도 소비하므로 필드명 forward를 곧바로 화면 전방으로 해석하지 않는다.
	// 관련 추적 방향 보정은 GameRoom_KoukuTargeting.cpp의 Update_KoukuPlayerTargets에서 forwardYawOffset을 사용한다.
	SERVER_NAV_POINT destination{
		static_cast<float>(
			boss.fPatternStageOriginX + sample.fLateral * std::cos(yaw) + sample.fForward * std::sin(yaw)),
		boss.fPatternStageOriginY + sample.fUp,
		static_cast<float>(
			boss.fPatternStageOriginZ - sample.fLateral * std::sin(yaw) + sample.fForward * std::cos(yaw))};
	// SLAM은 현재 확정된 목표/중앙의 XZ를 유지한다. source의 가로 흔들림을 보스 월드 이동에 더하지 않는다.
	if (airborne && boss.AlbionAirborne.ePhase == ALBION_AIRBORNE_PHASE::SLAM)
	{
		destination.x = boss.fPositionX;
		destination.z = boss.fPositionZ;
	}
	if (!std::isfinite(destination.x) || !std::isfinite(destination.y) || !std::isfinite(destination.z))
	{
		status = "KoukuSaydon root motion destination is invalid";
		return false;
	}
	SERVER_NAV_POINT ground;
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(
			boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		// 현재 위치에서 요청 위치까지의 구간을 이분 탐색해 마지막으로 승인된 navigation 지점을 찾는다.
		// 최대 40회이며 남은 구간 길이가 1mm 이하가 되면 멈춘다. stage 시계와 capture 원점은 변경하지 않는다.
		// 다음 호출도 원점+해당 시각의 sample을 요청하므로 매 tick 이동량을 계속 누적하는 방식은 아니다.
		const double segmentX = double(destination.x) - double(boss.fPositionX);
		const double segmentZ = double(destination.z) - double(boss.fPositionZ);
		const double segmentLength = std::sqrt(segmentX * segmentX + segmentZ * segmentZ);
		constexpr double CLAMP_RESOLUTION_M = 0.001;
		double low = 0.0, high = 1.0;
		bool found = false;
		SERVER_NAV_POINT clamped{};
		for (unsigned iteration = 0u; iteration < 40u && (high - low) * segmentLength > CLAMP_RESOLUTION_M;
			 ++iteration)
		{
			const double mid = (low + high) * 0.5;
			const float candidateX = static_cast<float>(boss.fPositionX + segmentX * mid);
			const float candidateZ = static_cast<float>(boss.fPositionZ + segmentZ * mid);
			SERVER_NAV_POINT candidate{};
			if (navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, candidateX, candidateZ) &&
				navigation.Resolve_TraversalStep(
					boss.fPositionX, boss.fPositionZ, candidateX, candidateZ, candidate))
			{
				low = mid;
				clamped = candidate;
				clamped.x = candidateX;
				clamped.z = candidateZ;
				found = true;
			}
			else
				high = mid;
		}
		// 승인된 구간을 찾지 못했거나 이동 거리가 1mm 미만이면 XYZ는 그대로 두고 이번 tick을 처리한 것으로 기록한다.
		if (!found || low * segmentLength < CLAMP_RESOLUTION_M)
		{
			boss.iPatternStageRootLastTick = serverTick;
			return true; // 경계에서 위치를 유지해도 GameRoom은 이어서 stage 시계를 진행한다.
		}
		destination.x = clamped.x;
		destination.z = clamped.z;
		ground = clamped;
	}
	// navigation의 목적지 지면 높이 차를 더한다. source Up은 capture 시점의 stage 원점에 대한 상대 높이 m다.
	SERVER_NAV_POINT startGround;
	if (!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, startGround))
	{
		status = "KoukuSaydon root motion lost its current ground";
		return false;
	}
	const float targetGroundY = ground.y;
	destination.y += targetGroundY - boss.fPatternStageRootGroundY;
	if (boss.bPatternRootGrounded)
		destination.y = (std::max)(destination.y, targetGroundY);
	auto airborneState = boss.AlbionAirborne;
	// airborne 계산은 복사본에서 수행한다. 아래 검사 실패로 실제 boss의 착지 진행도까지 먼저 갱신하지 않도록 한다.
	if (airborne)
	{
		float height = 0.f;
		float airborneSourceUp = sample.fUp;
		if (airborneState.ePhase == ALBION_AIRBORNE_PHASE::SLAM &&
			boss.iPatternStageIndex == airborneState.iSourceStageIndex)
		{
			std::uint64_t stageStart = 0u;
			for (std::uint32_t index = 0u; index < boss.iPatternStageIndex; ++index)
				stageStart += pattern.Stages[index].iDurationMs;
			const double triggerAge = (std::max)(0.0, double(airborneState.iStartMs) - double(stageStart));
			// 지연된 호출도 SLAM 시작부터 현재 시각 사이에 지나온 root Up 최솟값을 고려한다. 과거 위치로 충돌을 재실행하지는 않는다.
			for (const auto& key : boss.PatternStageRootMotion)
				if (key.iTimeMs >= triggerAge && key.iTimeMs <= elapsedMs)
					airborneSourceUp = (std::min)(airborneSourceUp, key.fUp);
		}
		if (!Sample_AlbionAirborneHeight(
				airborneState,
				Pattern_ElapsedMs(boss, serverTick),
				boss.iPatternStageIndex,
				airborneSourceUp,
				height))
		{
			status = "Albion airborne height lost its validated phase";
			return false;
		}
		destination.y = targetGroundY + height;
	}
	const SERVER_NAV_POINT proposed = destination;
	bool blocked = false;
	if (!collision.Resolve_CircleMove(
			boss.fPositionX,
			boss.fPositionY,
			boss.fPositionZ,
			destination.x,
			destination.y,
			destination.z,
			boss.fCollisionRadius,
			boss.fCollisionRadius,
			boss.fCollisionRadius,
			destination.x,
			destination.y,
			destination.z,
			blocked,
			boss.iNetEntityId,
			false))
	{
		status = "KoukuSaydon root motion collision resolution failed";
		return false;
	}
	// 몸체 sweep 후의 XZ도 navigation으로 다시 검사한다. 승인되지 않으면 현재 좌표를 유지하고 이 tick을 소비한다.
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(
			boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		boss.iPatternStageRootLastTick = serverTick;
		return true;
	}
	if (blocked)
	{
		// 충돌로 잘린 XYZ의 경로 비율에서 예상 지면 높이를 계산하고 실제 resolved XZ의 지면과 차이를 보정한다.
		// 보간된 source Up은 유지하며 필요한 높이 보정도 별도 body sweep으로 확인한다.
		const double dx = proposed.x - boss.fPositionX, dy = proposed.y - boss.fPositionY,
					 dz = proposed.z - boss.fPositionZ;
		const double squaredDistance = dx * dx + dy * dy + dz * dz;
		const double ratio = squaredDistance > 1e-12
			? (std::clamp)(((destination.x - boss.fPositionX) * dx + (destination.y - boss.fPositionY) * dy +
							(destination.z - boss.fPositionZ) * dz) /
							   squaredDistance,
						   0.0,
						   1.0)
			: 0.0;
		const float terrainCorrection =
			ground.y - static_cast<float>(startGround.y + (targetGroundY - startGround.y) * ratio);
		if (std::abs(terrainCorrection) > .00001f)
		{
			bool correctionBlocked = false;
			SERVER_NAV_POINT corrected;
			if (!collision.Resolve_CircleMove(
					destination.x,
					destination.y,
					destination.z,
					destination.x,
					destination.y + terrainCorrection,
					destination.z,
					boss.fCollisionRadius,
					boss.fCollisionRadius,
					boss.fCollisionRadius,
					corrected.x,
					corrected.y,
					corrected.z,
					correctionBlocked,
					boss.iNetEntityId,
					false))
			{
				status = "KoukuSaydon root ground correction failed collision validation";
				return false;
			}
			if (correctionBlocked)
			{
				boss.iPatternStageRootLastTick = serverTick;
				return true;
			}
			destination = corrected;
		}
	}
	// navigation과 몸체 sweep의 검사 후에만 최종 XYZ를 함께 대입한다. 앞선 원점 capture와 no-op tick 기록은 별도 상태 변경이다.
	boss.fPositionX = destination.x;
	boss.fPositionY = destination.y;
	boss.fPositionZ = destination.z;
	if (airborne)
		boss.AlbionAirborne = airborneState;
	boss.iPatternStageRootLastTick = serverTick;
	return true;
}

// GameRoom의 취소/실행 정리에서 호출한다. 실행 ID가 있으면 ABORTED receipt를 쓰고 없어도 내부 진행 필드는 정리한다.
void LostArk::Server::CKoukuSaydonBrain::Abort_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) const
{
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
}

// Logic 결과/연쇄 종료가 요청한 조기 완료다. 실행 중인 ID가 있을 때만 COMPLETED로 정리하며 나머지 stage는 진행하지 않는다.
void LostArk::Server::CKoukuSaydonBrain::Complete_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) const
{
	if (boss.strPatternId.empty())
		return;
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
}
