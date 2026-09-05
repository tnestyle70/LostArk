#include "KoukuSaydonBrain.h"

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace
{
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;

	std::uint32_t Next_ServerTick(const std::uint32_t tick) noexcept
	{
		return tick == (std::numeric_limits<std::uint32_t>::max)() ?
			1u : tick + 1u;
	}

	std::uint64_t Stage_ElapsedTicks(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick) noexcept
	{
		const std::uint32_t firstTick = boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		const std::uint64_t age = serverTick >= firstTick ?
			static_cast<std::uint64_t>(serverTick - firstTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - firstTick) +
			static_cast<std::uint64_t>(serverTick);
		return age + 1u;
	}

	bool Has_ElapsedMilliseconds(
		const std::uint64_t elapsedTicks,
		const std::uint32_t durationMs) noexcept
	{
		return elapsedTicks * MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ;
	}

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

	bool Is_ZeroMotion(
		const LostArk::Server::BOSS_PATTERN_STAGE_MOTION& motion) noexcept
	{
		using namespace LostArk::Server;
		return BOSS_PATTERN_STAGE_MOTION_KIND::NONE == motion.eKind &&
			0u == motion.iRetargetDelayMs && 0.f == motion.fSpeedMps &&
			0.f == motion.fDistance && 0u == motion.iCornerIndex &&
			0.f == motion.fHalfExtentsX && 0.f == motion.fHalfExtentsZ &&
			motion.RootMotion.empty();
	}
}

bool LostArk::Server::CKoukuSaydonBrain::Is_GateOneBoss(
	const LostArk::Shared::WORLD_ID worldId,
	const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == boss.strArchetypeId &&
		KOUKUSAYDON_G1_BOSS_PLACEMENT_ID == boss.strPlacementId;
}

bool LostArk::Server::CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
	const BOSS_PATTERN_DEFINITION& pattern,
	std::string& status)
{
	if (KOUKUSAYDON_G1_ENCOUNTER_ID != pattern.strEncounterId ||
		pattern.strPatternId.empty() ||
		BOSS_PATTERN_SELECTION::AUDITION_ONLY != pattern.eSelection ||
		BOSS_PATTERN_TARGET_POLICY::NONE != pattern.eTargetPolicy ||
		BOSS_PATTERN_AIM_POLICY::NONE != pattern.eAimPolicy ||
		BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind ||
		!pattern.Motion.strAnchorId.empty() ||
		0.f != pattern.Motion.fLandingX || 0.f != pattern.Motion.fLandingY ||
		0.f != pattern.Motion.fLandingZ || 0.f != pattern.Motion.fApexHeight ||
		pattern.Motion.bMoveToAnchorBeforeTakeoff ||
		BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind ||
		!pattern.Finale.strGhostArchetypeId.empty() ||
		!pattern.Finale.GhostPatternIds.empty() ||
		0.f != pattern.fVerticalOffsetM || pattern.bInvulnerableWhileRunning ||
		pattern.Stages.empty() ||
		pattern.Stages.size() != pattern.iExpectedStageCount)
	{
		status = "KoukuSaydon pattern is not an animation-only audition definition";
		return false;
	}

	std::unordered_set<std::string> stageIds;
	std::unordered_set<std::string> actionIds;
	for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
	{
		const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern.Stages[index];
		const bool supportedKind =
			BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::ACTIVE == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::RECOVERY == stage.eStageKind;
		const bool validTimeoutBranch = stage.Branches.empty() ||
			(1u == stage.Branches.size() &&
			 BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == stage.Branches.front().eOutcome &&
			 stage.Branches.front().strNextPatternId.empty() &&
			 stage.Branches.front().strNextActionId ==
				(index + 1u < pattern.Stages.size() ?
					pattern.Stages[index + 1u].strActionId : std::string{}));
		if (stage.strStageId.empty() || stage.strActionId.empty() ||
			!stageIds.insert(stage.strStageId).second ||
			!actionIds.insert(stage.strActionId).second ||
			!supportedKind || 0u == stage.iDurationMs ||
			!stage.strDamageProfileId.empty() ||
			BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT !=
				stage.eHitAnchorKind ||
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE !=
				stage.eHitActivationKind ||
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE != stage.ePlayerResponse ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE != stage.eAttachmentSlot ||
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL != stage.ePartDamagePolicy ||
			stage.bHasCounterProxy ||
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE != stage.eCounterProxyKind ||
			0.f != stage.fCounterProxyForwardOffsetM ||
			0.f != stage.fCounterProxyRightOffsetM ||
			0.f != stage.fCounterProxyRadiusM ||
			0.f != stage.fCounterProxyArcDegrees ||
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE != stage.eBossResponseKind ||
			0u != stage.iBossResponseThreshold || 0.f != stage.fVerticalOffsetM ||
			0.f != stage.fHitOuterRadius || 0.f != stage.fHitInnerRadius ||
			0.f != stage.fHitAngleDegrees || 0.f != stage.fHitLength ||
			0.f != stage.fHitHalfWidth || 0u != stage.iHitCount ||
			0u != stage.iHitIntervalMs || 0u != stage.iHitDelayMs ||
			!stage.HitOffsetsMs.empty() ||
			0.f != stage.fHitAnchorForwardOffsetM ||
			0.f != stage.fHitAnchorRightOffsetM ||
			0.f != stage.fHitAnchorYawOffsetDegrees ||
			0u != stage.iHitActivationStartMs ||
			0u != stage.iHitActivationLifetimeMs ||
			0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
			stage.bKnockdown || 0u != stage.iDownMs || stage.bWallContact ||
			stage.bChargeImpact || stage.bPiercesCover ||
			!Is_ZeroMotion(stage.Motion) || !stage.strPropBreakSetId.empty() ||
			!stage.PropBreakSlotIds.empty() || !stage.Actions.empty() ||
			!validTimeoutBranch)
		{
			status = "KoukuSaydon pattern contains unsupported combat, motion, action, or branch data";
			return false;
		}
	}
	status.clear();
	return true;
}

std::uint32_t
LostArk::Server::CKoukuSaydonBrain::Resolve_ProductSourceRevision(
	const CGameplayCatalog& catalog) noexcept
{
	return catalog.Find_KoukuSaydonProductSourceRevision(
		std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
}

const LostArk::Server::BOSS_PATTERN_DEFINITION*
LostArk::Server::CKoukuSaydonBrain::Find_AnimationOnlyPattern(
	const CGameplayCatalog& catalog,
	const std::string_view patternId,
	std::string& status)
{
	const auto* patterns = catalog.Find_BossPatterns(
		std::string{ KOUKUSAYDON_G1_ENCOUNTER_ID });
	if (nullptr == patterns)
	{
		status = "KoukuSaydon encounter patterns are missing";
		return nullptr;
	}
	const auto found = std::find_if(patterns->begin(), patterns->end(),
		[patternId](const BOSS_PATTERN_DEFINITION& pattern)
		{
			return pattern.strPatternId == patternId;
		});
	if (patterns->end() == found)
	{
		status = "KoukuSaydon pattern ID is unknown";
		return nullptr;
	}
	return Validate_AnimationOnlyPattern(*found, status) ? &*found : nullptr;
}

void LostArk::Server::CKoukuSaydonBrain::Enter_Stage(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage,
	const std::uint32_t stageIndex,
	const std::uint32_t serverTick,
	const bool evaluatesOnEntryTick)
{
	boss.iPatternStageIndex = stageIndex;
	boss.strPatternStageId = stage.strStageId;
	boss.strActionId = stage.strActionId;
	boss.iPatternStageDurationMs = stage.iDurationMs;
	boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ?
		serverTick : Next_ServerTick(serverTick);
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = To_ServerAction(stage.eStageKind);
}

bool LostArk::Server::CKoukuSaydonBrain::Begin_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (!revision.Is_Valid() || 0u == serverTick ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction ||
		!boss.strPatternId.empty() ||
		KOUKUSAYDON_G1_ENCOUNTER_ID != boss.strEncounterId ||
		!Validate_AnimationOnlyPattern(pattern, status))
	{
		if (status.empty())
			status = "KoukuSaydon pattern cannot begin from the current boss state";
		return false;
	}
	boss.PinnedDefinitionRevision = revision;
	boss.PatternTerminalReceipt = {};
	boss.strPatternId = pattern.strPatternId;
	boss.iPatternSequence = boss.iPatternSequence ==
		(std::numeric_limits<std::uint32_t>::max)() ? 1u :
		boss.iPatternSequence + 1u;
	Enter_Stage(boss, pattern.Stages.front(), 0u, serverTick, true);
	status.clear();
	return true;
}

void LostArk::Server::CKoukuSaydonBrain::Finish_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick,
	const SERVER_BOSS_PATTERN_TERMINAL_RESULT result)
{
	(void)serverTick;
	if (!boss.strPatternId.empty())
	{
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.eResult = result;
	}
	boss.strPatternId.clear();
	boss.strPatternStageId.clear();
	boss.strActionId.clear();
	boss.strDamageProfileId.clear();
	boss.iPatternStageIndex = 0u;
	boss.iPatternStageDurationMs = 0u;
	boss.iPatternStageFirstEvaluationTick = 0u;
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = 0u == boss.iCurrentHp ?
		SERVER_ENTITY_ACTION::DEAD : SERVER_ENTITY_ACTION::IDLE;
}

LostArk::Server::KOUKUSAYDON_BRAIN_UPDATE_RESULT
LostArk::Server::CKoukuSaydonBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (boss.strPatternId.empty())
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
	}
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		Finish_Pattern(boss, serverTick,
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		status = "KoukuSaydon boss died during pattern playback";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD;
	}
	const BOSS_PATTERN_DEFINITION* pattern =
		Find_AnimationOnlyPattern(catalog, boss.strPatternId, status);
	if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size() ||
		pattern->Stages[boss.iPatternStageIndex].strStageId !=
			boss.strPatternStageId ||
		pattern->Stages[boss.iPatternStageIndex].strActionId != boss.strActionId)
	{
		Finish_Pattern(boss, serverTick,
			SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		if (status.empty())
			status = "KoukuSaydon running stage no longer matches its pinned definition";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION;
	}
	const std::uint64_t elapsedTicks = Stage_ElapsedTicks(boss, serverTick);
	boss.fActionElapsedSeconds = static_cast<float>(elapsedTicks) /
		static_cast<float>(SERVER_TICK_HZ);
	const BOSS_PATTERN_STAGE_DEFINITION& stage =
		pattern->Stages[boss.iPatternStageIndex];
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
	Finish_Pattern(boss, serverTick,
		SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
	status.clear();
	return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
}

void LostArk::Server::CKoukuSaydonBrain::Abort_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick) const
{
	Finish_Pattern(
		boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
}
