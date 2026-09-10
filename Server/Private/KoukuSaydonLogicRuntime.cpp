#include "KoukuSaydonLogicRuntime.h"

#include "ServerCombatHitRuntime.h"
#include "PlayerSkillSystem.h"
#include "KoukuSaydonBrain.h"
#include "ServerCollisionSystem.h"
#include "Gameplay/CombatCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr double DEGREES_PER_RADIAN = 57.295779513082320876;
	constexpr float DEFAULT_CLOWN_HOLD_MS = 15000.f;

	/* Stable server-owned card selection for one encounter admission. */
	std::uint64_t Mix(std::uint64_t value) noexcept
	{
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31);
	}

	float Wrap180(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees > 180.f)
			degrees -= 360.f;
		if (degrees <= -180.f)
			degrees += 360.f;
		return degrees;
	}

	float Wrap360(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees < 0.f)
			degrees += 360.f;
		return degrees;
	}

	std::uint32_t Percent_Of(const std::uint32_t value, const std::uint32_t percent) noexcept
	{
		const std::uint64_t scaled =
			(static_cast<std::uint64_t>(value) * percent + 50u) / 100u;
		return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
	}


}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Ticks_FromMs(
	const std::uint32_t ms) noexcept
{
	const std::uint64_t ticks =
		(static_cast<std::uint64_t>(ms) * SERVER_TICK_HZ + 999u) / 1000u;
	return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
		ticks, (std::numeric_limits<std::uint32_t>::max)() - 1u));
}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Add_Ticks(
	const std::uint32_t tick, const std::uint32_t ticks) noexcept
{
	// Tick 0 is the reserved "never" value, so a wrap skips it.
	const std::uint32_t result = tick + ticks;
	return 0u == result ? 1u : result;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Has_ReachedTick(
	const std::uint32_t serverTick, const std::uint32_t targetTick) noexcept
{
	if (0u == targetTick || 0u == serverTick)
		return false;
	return static_cast<std::int32_t>(serverTick - targetTick) >= 0;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_Judgeable(
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp && player.isCombatReady &&
		PLAYER_ACTION_STATE::DEAD != player.eAction &&
		PLAYER_ACTION_STATE::FALLING != player.eAction &&
		PLAYER_ACTION_STATE::GRABBED != player.eAction;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Build(
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t startTick,
	KOUKUSAYDON_LOGIC_LEDGER& outLedger)
{
	KOUKUSAYDON_LOGIC_LEDGER ledger{};
	ledger.strPatternId = pattern.strPatternId;
	ledger.iPatternSequence = boss.iPatternSequence;
	ledger.iPatternStartTick = startTick;
	for (std::size_t index = 0u; index < pattern.LogicWindows.size(); ++index)
	{
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[index];
		KOUKUSAYDON_LOGIC_WINDOW_STATE state{};
		state.iWindowIndex = static_cast<std::uint32_t>(index);
		state.iStartTick = Add_Ticks(startTick, Ticks_FromMs(window.iStartMs));
		state.iEndTick = Add_Ticks(startTick,
			Ticks_FromMs(window.iStartMs + window.iDurationMs));
        if (!window.strHoldLogicOccurrenceId.empty())
        {
            const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
                return row.strWindowId == window.strHoldLogicOccurrenceId && row.eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD;
            });
            if (hold != pattern.LogicWindows.end())
            {
                state.iHoldStartTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs));
                state.iHoldEndTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs + hold->iDurationMs));
            }
        }
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT == window.eKind)
		{
			ledger.bDanceActive = true;
			ledger.eHudMode = LostArk::Shared::KOUKU_HUD_MODE::DANCE;
		}
		ledger.Windows.push_back(std::move(state));
		if (window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT)
			ledger.ContactWindowOrder.push_back(static_cast<std::uint32_t>(index));
	}
	std::sort(ledger.ContactWindowOrder.begin(), ledger.ContactWindowOrder.end(), [&](const auto a, const auto b) {
		const auto& first = pattern.LogicWindows[a]; const auto& second = pattern.LogicWindows[b];
		return first.iContactPriority != second.iContactPriority ? first.iContactPriority > second.iContactPriority : first.strWindowId < second.strWindowId;
	});
	for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
	{
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick, Ticks_FromMs(pattern.MechanicTriggers[index].iStartMs));
		ledger.MechanicTriggers.push_back(cue);
	}
	for (std::size_t index = 0u; index < pattern.WorldSequences.size(); ++index)
	{
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick,
			Ticks_FromMs(pattern.WorldSequences[index].iStartMs));
		ledger.WorldSequences.push_back(cue);
	}
	outLedger = std::move(ledger);
}

namespace
{
	struct LOGIC_REGION_TRANSFORM final
	{
		float centerX = 0.f, centerZ = 0.f, yaw = 0.f;
		float halfX = 0.f, halfZ = 0.f, radius = 0.f;
	};

	bool Resolve_LogicRegionTransform(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t patternElapsedTicks, LOGIC_REGION_TRANSFORM& outTransform) noexcept
	{
		using namespace LostArk::Server;
		float centerX = region.fCenterX, centerZ = region.fCenterZ, yaw = region.fYawDegrees;
		float halfX = region.fHalfX, halfZ = region.fHalfZ, radius = region.fRadiusM;
		if (region.WorldTrack.bEnabled)
		{
			const auto& track = region.WorldTrack;
			if (track.Keys.empty()) return false;
			const std::uint32_t startTicks = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs);
			if (patternElapsedTicks < startTicks) return false;
			const double delayed = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor ?
				static_cast<double>(patternElapsedTicks) * (1000.0 / 30.0) - track.iStartMs - track.iStartDelayMs :
				static_cast<double>(patternElapsedTicks - startTicks) * (1000.0 / 30.0) - track.iStartDelayMs;
			if (delayed < 0.0) return false;
			const double timeMs = (std::min)(static_cast<double>(track.iDurationMs), delayed * track.fPlaybackSpeed);
			const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), timeMs,
				[](const double time, const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) { return time < key.iTimeMs; });
			const auto& left = next == track.Keys.begin() ? track.Keys.front() : *(next - 1);
			const auto& right = next == track.Keys.end() ? left : *next;
			if (!left.bVisible) return false;
			float factor = right.iTimeMs > left.iTimeMs ?
				static_cast<float>((timeMs - left.iTimeMs) / (right.iTimeMs - left.iTimeMs)) : 0.f;
			factor = (std::clamp)(factor, 0.f, 1.f);
			if (track.bSmoothStep) factor = factor * factor * (3.f - 2.f * factor);
			const auto mix = [factor](const float a, const float b) { return a + (b - a) * factor; };
			const float sx = track.fBaselineScaleX * mix(left.fScaleX, right.fScaleX);
			const float sz = track.fBaselineScaleZ * mix(left.fScaleZ, right.fScaleZ);
			if (!(sx > 0.f && sz > 0.f)) return false;
			const float baselineRadians = track.fBaselineYawDegrees * 0.017453292519943295f;
			const float ox = mix(left.fOffsetX, right.fOffsetX), oz = mix(left.fOffsetZ, right.fOffsetZ);
			float qy = right.fRotationY, qw = right.fRotationW;
			float dot = left.fRotationY * qy + left.fRotationW * qw;
			if (dot < 0.f) { qy = -qy; qw = -qw; dot = -dot; }
			dot = (std::clamp)(dot, -1.f, 1.f);
			float leftWeight = 1.f - factor, rightWeight = factor;
			if (dot < 0.9995f)
			{
				const float omega = std::acos(dot), sine = std::sin(omega);
				leftWeight = std::sin((1.f - factor) * omega) / sine;
				rightWeight = std::sin(factor * omega) / sine;
			}
			qy = left.fRotationY * leftWeight + qy * rightWeight;
			qw = left.fRotationW * leftWeight + qw * rightWeight;
			const float qLength = std::sqrt(qy * qy + qw * qw);
			if (!(qLength > 0.f)) return false;
			qy /= qLength; qw /= qLength;
			const float worldYaw = track.fBaselineYawDegrees +
				std::atan2(2.f * qw * qy, 1.f - 2.f * qy * qy) * 57.29577951308232f;
			const float worldRadians = worldYaw * 0.017453292519943295f;
			centerX = track.fBaselineX + std::cos(baselineRadians) * ox + std::sin(baselineRadians) * oz +
				std::cos(worldRadians) * region.fCenterX * sx + std::sin(worldRadians) * region.fCenterZ * sz;
			centerZ = track.fBaselineZ - std::sin(baselineRadians) * ox + std::cos(baselineRadians) * oz -
				std::sin(worldRadians) * region.fCenterX * sx + std::cos(worldRadians) * region.fCenterZ * sz;
			yaw += worldYaw; halfX *= sx; halfZ *= sz; radius *= sx;
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor)
		{
			// A baked bone tip is boss-local. Apply the live authoritative root once,
			// after its track and the authored TARGET_YAW offset have been combined.
			const float radians = boss.fYawDegrees * 0.017453292519943295f;
			const float localX = centerX, localZ = centerZ;
			centerX = boss.fPositionX + std::cos(radians) * localX + std::sin(radians) * localZ;
			centerZ = boss.fPositionZ - std::sin(radians) * localX + std::cos(radians) * localZ;
			yaw += boss.fYawDegrees;
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN == region.eAnchor)
		{
			centerX += boss.fSpawnPositionX;
			centerZ += boss.fSpawnPositionZ;
		}
		outTransform = { centerX, centerZ, yaw, halfX, halfZ, radius };
		return true;
	}

	bool Contains_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_PLAYER& player, const std::uint32_t patternElapsedTicks = 0u) noexcept
	{
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius] = transform;
		const float dx = player.fPositionX - centerX, dz = player.fPositionZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz)) return false;
		const float radians = yaw * 0.017453292519943295f;
		const float localX = std::cos(radians) * dx - std::sin(radians) * dz;
		const float localZ = std::sin(radians) * dx + std::cos(radians) * dz;
		if (!region.bSector && !region.bCircle)
			return std::fabs(localX) <= halfX && std::fabs(localZ) <= halfZ;
		const float distanceSq = dx * dx + dz * dz;
		if (region.bCircle) return distanceSq <= radius * radius;
		if (distanceSq < 0.000001f || distanceSq > radius * radius) return false;
		const float angle = std::atan2(localX, localZ) * 57.29577951308232f;
		// A shared radial edge belongs to exactly one sector.
		return angle >= -region.fHalfAngleDegrees && angle < region.fHalfAngleDegrees;
	}

	bool Intersects_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target,
		const std::uint32_t patternElapsedTicks) noexcept
	{
		using namespace LostArk::Shared::CombatCollision;
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius] = transform;
		if (region.bCircle) return Circles_Overlap(CIRCLE_XZ{ centerX, centerZ, radius }, target);
		const float radians = yaw * 0.017453292519943295f;
		const float forwardX = std::sin(radians), forwardZ = std::cos(radians);
		if (region.bSector)
			return Circle_IntersectsCone(target, centerX, centerZ, forwardX, forwardZ,
				radius, region.fHalfAngleDegrees * 2.f);
		// The shared box starts at its rear edge and extends along local +Z.
		return Circle_IntersectsForwardBox(target, centerX - forwardX * halfZ,
			centerZ - forwardZ * halfZ, forwardX, forwardZ, halfZ * 2.f, halfX);
	}

	/* Puts the player on the region that just caught them. The region rides a
	World Object's transform track, so this is what "hanging from the hook"
	means on the Server: one attachment naming that region, and a deadline the
	room enforces even if this pattern stops running. The player keeps their own
	Y, so the hook drags them across the floor instead of lifting them off it. */
	bool Hang_PlayerOnRegion(LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const std::uint32_t patternElapsedTicks,
		const std::uint32_t windowIndex, const std::uint32_t regionIndex,
		const std::uint32_t releaseTick, const std::uint32_t serverTick) noexcept
	{
		using namespace LostArk::Shared;
		LOGIC_REGION_TRANSFORM transform;
		if (0u == releaseTick || 0u == player.iCurrentHp ||
			PLAYER_ACTION_STATE::DEAD == player.eAction ||
			!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform) ||
			!std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) ||
			!std::isfinite(transform.yaw))
			return false;
		player.iAttachmentOwnerNetEntityId = boss.iNetEntityId;
		player.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
		player.iAttachmentPatternSequence = boss.iPatternSequence;
		player.fAttachmentLocalOffsetX = 0.f;
		player.fAttachmentLocalOffsetY = 0.f;
		player.fAttachmentLocalOffsetZ = 0.f;
		// Caught facing whatever way they were running: keep that on the hook.
		player.fAttachmentYawOffsetDegrees = Wrap180(player.fYawDegrees - transform.yaw);
		player.iAttachmentWindowIndex = windowIndex;
		player.iAttachmentRegionIndex = regionIndex;
		player.iAttachmentReleaseTick = releaseTick;
		player.eAction = PLAYER_ACTION_STATE::GRABBED;
		player.iActionStartTick = serverTick;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.hasReleasedHold = false;
		player.PendingCommand.Clear();
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.isCombatReady = false;
		player.fPositionX = transform.centerX;
		player.fPositionZ = transform.centerZ;
		return true;
	}

	/* One tick of being dragged. False means the hook is no longer there - its
	track ran out or hid it - which is the Server's cue to let the player go. */
	bool Drag_HookedPlayer(LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const std::uint32_t patternElapsedTicks) noexcept
	{
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform) ||
			!std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) ||
			!std::isfinite(transform.yaw))
			return false;
		player.fPositionX = transform.centerX;
		player.fPositionZ = transform.centerZ;
		player.fYawDegrees = Wrap180(transform.yaw + player.fAttachmentYawOffsetDegrees);
		player.hasMoveGoal = false;
		player.isCombatReady = false;
		return true;
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Assign_EncounterCard(
	SERVER_PLAYER& player, const LostArk::Shared::NET_ENTITY_ID encounterOwnerId,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		INVALID_NET_ENTITY_ID == encounterOwnerId)
		return;
	if (MECHANIC_CARD_SYMBOL::NONE != player.eMechanicCardSymbol &&
		MECHANIC_CARD_COLOR::NONE != player.eMechanicCardColor)
		return;
	const std::uint64_t roll = Mix((static_cast<std::uint64_t>(encounterOwnerId) << 32) ^
		(static_cast<std::uint64_t>(player.iPlayerId) << 8) ^ serverTick);
	player.eMechanicCardSymbol = static_cast<MECHANIC_CARD_SYMBOL>(1u + (roll & 3u));
	player.eMechanicCardColor = 0u == (roll & 4u) ? MECHANIC_CARD_COLOR::RED : MECHANIC_CARD_COLOR::BLACK;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Discard(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	SERVER_WORLD_ENTITY* const pBoss)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active())
		return;
	(void)players;
	if (nullptr != pBoss)
	{
		pBoss->bKoukuShieldActive = false;
		pBoss->fKoukuShieldArcDegrees = 0.f;
		pBoss->KoukuShieldRegions.clear();
        (void)CBossCombatRuntime::Set_Flag(pBoss->BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
        CBossCombatRuntime::Clear_PatternOutcomes(*pBoss);
	}
	ledger = {};
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Open_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	const KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	(void)ledger;
	state.bOpened = true;
	state.Answers.clear();
	state.InsidePlayers.clear();
	state.NextContactHitTicks.clear();
	state.iBossHpAtOpen = boss.iCurrentHp;
	switch (window.eKind)
	{
	case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		for (auto& [playerId, player] : players)
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		break;
    case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
        break;
	case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		boss.bKoukuShieldActive = window.fShieldArcDegrees > 0.f;
		boss.fKoukuShieldArcDegrees = window.fShieldArcDegrees;
		boss.fKoukuShieldNormalYawOffsetDegrees = window.fNormalYawOffsetDegrees;
		boss.KoukuShieldRegions = window.CardRegions;
		break;
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
	case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
    case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
		// One world object is judged independently of the party roster.
		break;
	case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
	case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
	default:
		for (auto& [playerId, player] : players)
		{
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		}
		break;
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Close_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	state.bClosed = true;
    if (window.eKind == BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW)
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
	(void)players;
	if (BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind)
	{
		boss.bKoukuShieldActive = false;
		boss.fKoukuShieldArcDegrees = 0.f;
		boss.KoukuShieldRegions.clear();
	}
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Roulette(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	if (!window.CardRegions.empty())
	{
		const BOSS_LOGIC_REGION* matched = nullptr;
		for (const auto& region : window.CardRegions)
			if (Contains_LogicRegion(region, boss, player))
			{
				if (nullptr != matched) return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
				matched = &region;
			}
		if (nullptr == matched) return KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
		return matched->eCardSymbol == player.eMechanicCardSymbol &&
			matched->eCardColor == player.eMechanicCardColor ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	}
	if (window.iSectorCount == 0u ||
		window.SectorSymbols.size() != window.iSectorCount ||
		MECHANIC_CARD_SYMBOL::NONE == player.eMechanicCardSymbol)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float dx = player.fPositionX - window.fCenterX;
	const float dz = player.fPositionZ - window.fCenterZ;
	if (!std::isfinite(dx) || !std::isfinite(dz))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	if (window.fOuterRadiusM > 0.f &&
		dx * dx + dz * dz > window.fOuterRadiusM * window.fOuterRadiusM)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float degrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float relative = Wrap360(degrees - window.fStopYawDegrees);
	const float sectorSpan = 360.f / static_cast<float>(window.iSectorCount);
	std::size_t sector = static_cast<std::size_t>(relative / sectorSpan);
	if (sector >= window.iSectorCount)
		sector = window.iSectorCount - 1u;
	return window.SectorSymbols[sector] == player.eMechanicCardSymbol ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Gaze(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	const float dx = boss.fPositionX - player.fPositionX;
	const float dz = boss.fPositionZ - player.fPositionZ;
	if (!std::isfinite(dx) || !std::isfinite(dz) || !std::isfinite(player.fYawDegrees))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const auto insideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::FAIL : KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
	const auto outsideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float distanceSq = dx * dx + dz * dz;
	// Standing on the boss counts as facing it: no direction to judge.
	if (distanceSq < 0.01f)
		return insideAnswer;
	if (window.fMaxDistanceM > 0.f &&
		distanceSq > window.fMaxDistanceM * window.fMaxDistanceM)
		return outsideAnswer;
	const float toBoss = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float difference = Wrap180(toBoss - player.fYawDegrees);
	return std::fabs(difference) <= window.fHalfAngleDegrees ? insideAnswer : outsideAnswer;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_ShieldReflected(
	const SERVER_WORLD_ENTITY& boss,
	const float sourceX,
	const float sourceZ) noexcept
{
	if (!boss.bKoukuShieldActive || boss.fKoukuShieldArcDegrees <= 0.f)
		return false;
	const auto reflects = [sourceX, sourceZ](const float centerX, const float centerZ,
		const float yaw, const float halfAngle)
	{
		const float dx = sourceX - centerX, dz = sourceZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz) || dx * dx + dz * dz < 0.0001f)
			return false;
		const float direction = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
		return std::fabs(Wrap180(direction - yaw)) <= halfAngle;
	};
	if (!boss.KoukuShieldRegions.empty())
	{
		const float radians = boss.fYawDegrees * 0.017453292519943295f;
		for (const auto& region : boss.KoukuShieldRegions)
		{
			// A ranged attacker is reflected by direction too; the sector radius
			// is its debug drawing extent, not an attack-distance restriction.
			if (!region.bSector || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT)
				continue;
			const float centerX = boss.fPositionX + std::cos(radians) * region.fCenterX + std::sin(radians) * region.fCenterZ;
			const float centerZ = boss.fPositionZ - std::sin(radians) * region.fCenterX + std::cos(radians) * region.fCenterZ;
			if (reflects(centerX, centerZ, boss.fYawDegrees + region.fYawDegrees, region.fHalfAngleDegrees))
				return true;
		}
		return false;
	}
	return reflects(boss.fPositionX, boss.fPositionZ,
		boss.fYawDegrees + boss.fKoukuShieldNormalYawOffsetDegrees, boss.fKoukuShieldArcDegrees * 0.5f);
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Transform_ToClown(
	SERVER_PLAYER& player,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	const std::uint32_t durationMs)
{
	using namespace LostArk::Shared;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction)
		return;
	const std::uint32_t holdMs = 0u != durationMs ? durationMs :
		(nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iClownHoldMs ?
			pMadnessPolicy->iClownHoldMs :
			static_cast<std::uint32_t>(DEFAULT_CLOWN_HOLD_MS));
	player.bKoukuPatternOwnsClown = false;
	player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
	player.iMadnessFormEndTick = Add_Ticks(serverTick, Ticks_FromMs(holdMs));
	player.iCurrentMadness = 0u;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerFear(
    SERVER_PLAYER& player, const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    if (PLAYER_ACTION_STATE::FEAR == player.eAction &&
        (Has_ReachedTick(serverTick, player.iFearEndTick) || player.iCurrentHp == 0u))
    {
        player.eAction = player.iCurrentHp == 0u ? PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
        player.iActionStartTick = 0u;
        player.fActionElapsedSeconds = 0.f;
        player.PendingCommand.Clear();
    }
    if (PLAYER_ACTION_STATE::FEAR != player.eAction)
    {
        player.iFearEndTick = 0u;
        player.strFearPresentationId.clear();
        return false;
    }
    return true;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Result(
	SERVER_PLAYER& player,
	const BOSS_PATTERN_LOGIC_RESULT& result,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	using namespace LostArk::Shared;
	switch (result.eKind)
	{
	case BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE:
	{
		SERVER_WORLD_TO_PLAYER_HIT hit{};
		hit.iRawDamage =
			BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH == result.eKind ?
				(std::max)(1u, player.iCurrentHp) :
				(std::max)(1u, Percent_Of(player.iMaximumHp, result.iPercent));
		hit.fSourceX = boss.fPositionX;
		hit.fSourceZ = boss.fPositionZ;
		hit.fPushRangeM = result.fPushRangeM;
		hit.iPushMs = result.iPushMs;
		hit.iServerTick = serverTick;
		hit.bIgnoreDefense = true;
		hit.bIgnoreCounter = true;
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
			player, hit, catalog, outDamageEvents);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT:
	{
		if (0u == player.iMaximumMadness)
			break;
		const std::uint32_t gain = Percent_Of(player.iMaximumMadness, result.iPercent);
		const std::uint64_t total =
			static_cast<std::uint64_t>(player.iCurrentMadness) + gain;
		player.iCurrentMadness = static_cast<std::uint32_t>(
			(std::min<std::uint64_t>)(total, player.iMaximumMadness));
		if (player.iCurrentMadness >= player.iMaximumMadness)
			Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR:
        if (Is_Judgeable(player) && !player.bPatternBound &&
            PLAYER_ACTION_STATE::GRABBED != player.eAction &&
            PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
            PLAYER_ACTION_STATE::FEAR != player.eAction)
        {
            player.eAction = PLAYER_ACTION_STATE::FEAR;
            player.iActionStartTick = serverTick == 0u ? 1u : serverTick;
            player.iFearEndTick = Add_Ticks(player.iActionStartTick, Ticks_FromMs(result.iDurationMs));
            player.strFearPresentationId = result.strFearPresentationId;
            player.iCurrentSkillId = INVALID_SKILL_ID;
            player.Clear_SkillTarget();
            player.iComboStage = 0u;
            player.hasBufferedComboInput = false;
            player.hasReleasedHold = false;
            player.PendingCommand.Clear();
            player.hasMoveGoal = false;
            player.MovePath.clear();
            player.iMovePathIndex = 0u;
            player.fActionElapsedSeconds = 0.f;
            player.iKnockdownEndTick = 0u;
            player.fKnockbackRemainingSeconds = 0.f;
        }
        break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::CLOWN_TRANSFORM:
		Transform_ToClown(player, pMadnessPolicy, serverTick, result.iDurationMs);
		break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW:
	/* The grab needs the exact region that caught this player, which only the
	ENTER_AREA seam in Update knows, so it is applied there. */
	case BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::NONE:
	default:
		break;
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Results(
	const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	SERVER_PLAYER* const pPlayer,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput)
{
	for (const BOSS_PATTERN_LOGIC_RESULT& result : results)
	{
        if (BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER == result.eKind)
        {
            if (pPlayer && Is_Judgeable(*pPlayer) && pPlayer->eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR && state.iHoldEndTick &&
                Has_ReachedTick(serverTick, state.iHoldStartTick) && !Has_ReachedTick(serverTick, state.iHoldEndTick) &&
                std::none_of(outOutput.CaptureRequests.begin(), outOutput.CaptureRequests.end(), [&](const auto& request) {
                    return request.iPlayerNetEntityId == pPlayer->iNetEntityId;
                }))
                outOutput.CaptureRequests.push_back({pPlayer->iNetEntityId, result.eAttachmentSlot, state.iHoldEndTick, state.iWindowIndex});
            continue;
        }
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind)
		{
			if (state.AppliedWorldMotions.emplace(result.strTargetWorldInstanceId, result.strMotionInstanceId).second)
			{
				KOUKUSAYDON_LOGIC_WORLD_PLAY play;
				play.strInstanceId = result.strMotionInstanceId;
				play.strTargetSequenceInstanceId = result.strTargetWorldInstanceId;
				play.iStartTick = serverTick;
				outOutput.WorldSequencePlays.push_back(std::move(play));
			}
			continue;
		}
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind)
		{
			if (!result.strPatternId.empty())
				outOutput.FollowupPatternIds.push_back(result.strPatternId);
			continue;
		}
		if (nullptr != pPlayer)
		{
			Apply_Result(*pPlayer, result, boss, catalog, pMadnessPolicy,
				serverTick, outDamageEvents);
			continue;
		}
		/* Boss-level windows punish or reward the whole living raid. */
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (Is_Judgeable(player))
				Apply_Result(player, result, boss, catalog, pMadnessPolicy,
					serverTick, outDamageEvents);
		}
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision)
{
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
		return;
	/* Anyone already hanging rides the same authored region that caught them.
	The region is catalog data, so this keeps working after the window that
	judged them closed, and the moment the World Object's track hides the hook
	the deadline collapses to now and the room releases them. */
	for (auto& [hangingPlayerId, hanging] : players)
	{
		(void)hangingPlayerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != hanging.eAction ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP != hanging.eAttachmentSlot ||
			hanging.iAttachmentOwnerNetEntityId != boss.iNetEntityId ||
			hanging.iAttachmentPatternSequence != boss.iPatternSequence ||
			hanging.iAttachmentWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& carrier = pattern.LogicWindows[hanging.iAttachmentWindowIndex];
		if (hanging.iAttachmentRegionIndex >= carrier.CardRegions.size())
			continue;
		if (!Drag_HookedPlayer(hanging, carrier.CardRegions[hanging.iAttachmentRegionIndex],
			boss, serverTick - ledger.iPatternStartTick))
			hanging.iAttachmentReleaseTick = serverTick;
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.MechanicTriggers)
	{
		if (cue.bStarted || cue.iIndex >= pattern.MechanicTriggers.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const auto& trigger = pattern.MechanicTriggers[cue.iIndex];
		if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER == trigger.eKind)
			ledger.eHudMode = trigger.eHudMode;
		else
			outOutput.MechanicTriggers.push_back(trigger);
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.WorldSequences)
	{
		if (cue.bStarted || cue.iIndex >= pattern.WorldSequences.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const BOSS_PATTERN_WORLD_SEQUENCE& sequence = pattern.WorldSequences[cue.iIndex];
		KOUKUSAYDON_LOGIC_WORLD_PLAY play;
		play.strInstanceId = sequence.strInstanceId; play.fPlaybackSpeed = sequence.fPlaybackSpeed;
		play.iDurationMs = sequence.iDurationMs; play.strOccurrenceId = sequence.strOccurrenceId; play.iStartTick = cue.iStartTick;
		play.Placement = sequence.Placement;
		if (!play.Placement)
		{
			play.fPositionOffsetX = sequence.fPositionOffsetX + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionX - sequence.fAnchorPositionX : 0.f);
			play.fPositionOffsetY = sequence.fPositionOffsetY + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionY - sequence.fAnchorPositionY : 0.f);
			play.fPositionOffsetZ = sequence.fPositionOffsetZ + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionZ - sequence.fAnchorPositionZ : 0.f);
		}
		outOutput.WorldSequencePlays.push_back(std::move(play));
	}
    // Capture a charge once at trigger entry, then sample the existing boss-motion
    // path before the body-follow contact regions inspect this tick's position.
    for (auto& state : ledger.Windows)
    {
        if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
            !Has_ReachedTick(serverTick, state.iStartTick)) continue;
        const auto& window = pattern.LogicWindows[state.iWindowIndex];
        if (window.fBossChargeDistanceM <= 0.f || state.bChargeStopped) continue;
        if (!state.bChargeCaptured)
        {
            state.bChargeCaptured = true;
            std::vector<const SERVER_PLAYER*> candidates;
            for (const auto& [id, player] : players)
                if (Is_Judgeable(player)) candidates.push_back(&player);
            if (candidates.empty())
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge has no living target"; continue; }
            const auto* target = candidates[Mix(static_cast<std::uint64_t>(boss.iPatternSequence) << 32u ^
                boss.iNetEntityId ^ state.iStartTick) % candidates.size()];
            const float dx = target->fPositionX - boss.fPositionX, dz = target->fPositionZ - boss.fPositionZ;
            const float length = std::sqrt(dx * dx + dz * dz);
            if (!std::isfinite(length) || length <= .0001f)
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge target has no direction"; continue; }
            BOSS_PATTERN_BOSS_MOTION motion;
            motion.iStartMs = window.iStartMs; motion.iEndMs = window.iStartMs + window.iDurationMs;
            motion.StartPosition = {boss.fPositionX, boss.fPositionY, boss.fPositionZ};
            motion.EndPosition = {boss.fPositionX + dx / length * window.fBossChargeDistanceM,
                boss.fPositionY, boss.fPositionZ + dz / length * window.fBossChargeDistanceM};
            // The authored clip may face a different native axis; its body yaw
            // must not rotate the captured world-space travel vector.
            motion.fYawDegrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN) + window.fChargeYawOffsetDegrees;
            state.ChargeMotion = motion;
            boss.iTargetEntityId = target->iNetEntityId;
            boss.iPatternTargetEntityId = target->iNetEntityId;
            boss.bHasPatternTargetLastPosition = true;
            boss.fPatternTargetLastPositionX = target->fPositionX;
            boss.fPatternTargetLastPositionY = target->fPositionY;
            boss.fPatternTargetLastPositionZ = target->fPositionZ;
        }
        if (!state.ChargeMotion) continue;
        const auto& motion = *state.ChargeMotion;
        const auto position = CKoukuSaydonBrain::Sample_BossMotion(motion, serverTick - ledger.iPatternStartTick);
        SERVER_NAV_POINT destination{position[0], position[1], position[2]};
        if (navigation && !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ,
            destination.x, destination.z, destination))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge stopped at navigation boundary"; continue; }
        bool blocked = false;
        if (collision && !collision->Resolve_CircleMove(boss.fPositionX, boss.fPositionY, boss.fPositionZ,
            destination.x, destination.y, destination.z, boss.fCollisionRadius, boss.fCollisionRadius,
            boss.fCollisionRadius, destination.x, destination.y, destination.z, blocked, boss.iNetEntityId, false))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision resolution failed"; continue; }
        if (navigation && !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ,
            destination.x, destination.z, destination))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision destination is not navigable"; continue; }
        boss.fPositionX = destination.x; boss.fPositionY = destination.y; boss.fPositionZ = destination.z;
        boss.fYawDegrees = motion.fYawDegrees;
        state.bChargeStopped = blocked || Has_ReachedTick(serverTick, state.iEndTick);
        if (blocked) outOutput.strStatus = "boss charge stopped at collision boundary";
    }
	// Resolve all contacts before any duration timeout, including the final tick.
	// A group's priority order is fixed at Build; one card consumes one result per strike.
	std::set<std::string> completedWindows;
	for (const auto index : ledger.ContactWindowOrder)
	{
		auto& state = ledger.Windows[index];
		const auto& window = pattern.LogicWindows[index];
		if (state.bClosed || !Has_ReachedTick(serverTick, state.iStartTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		if (!reachedEnd || serverTick == state.iEndTick)
			for (const auto& target : window.ContactTargets)
			{
				if (state.ContactedWorldOccurrences.contains(target.strWorldOccurrenceId)) continue;
				const auto motionPriority = ledger.AppliedContactMotionPriorities.find({window.strContactGroupId, target.strWorldOccurrenceId});
				if (!window.strContactGroupId.empty() && motionPriority != ledger.AppliedContactMotionPriorities.end() &&
					motionPriority->second > window.iContactPriority) continue;
				const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{ target.fWorldX, target.fWorldZ, target.fRadiusM };
				if (!std::any_of(window.CardRegions.begin(), window.CardRegions.end(), [&](const auto& region) {
					return Intersects_LogicRegion(region, boss, circle, serverTick - ledger.iPatternStartTick);
				})) continue;
				state.ContactedWorldOccurrences.insert(target.strWorldOccurrenceId);
				if (!window.strContactGroupId.empty() && !ledger.ConsumedContactGroups.emplace(
					window.strContactGroupId, state.iStartTick, target.strWorldOccurrenceId).second) continue;
				for (const auto& result : window.OnSuccess)
				{
					if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION)
					{
						const auto motion = std::find_if(result.ContactMotions.begin(), result.ContactMotions.end(),
							[&](const auto& row) { return row.strTargetWorldOccurrenceId == target.strWorldOccurrenceId; });
						if (motion != result.ContactMotions.end() && state.AppliedWorldMotions.emplace(
							target.strWorldOccurrenceId, motion->strMotionInstanceId).second)
						{
							KOUKUSAYDON_LOGIC_WORLD_PLAY play;
							play.strInstanceId = motion->strMotionInstanceId; play.strTargetSequenceInstanceId = target.strWorldInstanceId;
							play.strTargetWorldOccurrenceId = target.strWorldOccurrenceId; play.iStartTick = serverTick;
							outOutput.WorldSequencePlays.push_back(std::move(play));
							if (!window.strContactGroupId.empty())
								ledger.AppliedContactMotionPriorities[{window.strContactGroupId, target.strWorldOccurrenceId}] = window.iContactPriority;
						}
					}
					else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW &&
						(result.strContactTargetWorldOccurrenceId.empty() || result.strContactTargetWorldOccurrenceId == target.strWorldOccurrenceId))
						completedWindows.insert(result.strTargetLogicOccurrenceId);
				}
			}
		if (reachedEnd) Close_Window(boss, window, state, players);
	}
	for (auto& state : ledger.Windows)
	{
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (state.bClosed || window.eKind != BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL ||
			!completedWindows.contains(window.strWindowId) || !Has_ReachedTick(serverTick, state.iStartTick) ||
			(Has_ReachedTick(serverTick, state.iEndTick) && serverTick != state.iEndTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		Close_Window(boss, window, state, players);
		Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog, pMadnessPolicy,
			serverTick, outDamageEvents, outOutput);
		outOutput.bEndPatternEarly |= window.bEndsPatternOnSuccess;
		outOutput.strStatus = "external signal window succeeded";
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (!state.bOpened)
		{
			if (!Has_ReachedTick(serverTick, state.iStartTick))
				continue;
			Open_Window(boss, window, state, ledger, players);
		}
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			break; // Contact phase above owns the complete short window.
		case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
            if (reachedEnd) Close_Window(boss, window, state, players);
            break;
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog, pMadnessPolicy,
					serverTick, outDamageEvents, outOutput);
				outOutput.strStatus = "external signal window timed out";
			}
			break;
        case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        {
            auto& signals = boss.BossCombat.PendingOutcomes;
            const auto counter = std::find_if(signals.begin(), signals.end(), [&](const auto& signal) {
                return signal.iPatternSequence == ledger.iPatternSequence &&
                    signal.strPatternId == boss.strPatternId &&
                    signal.eOutcome == BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT &&
                    Has_ReachedTick(signal.iServerTick, state.iStartTick) &&
                    !Has_ReachedTick(signal.iServerTick, Add_Ticks(state.iEndTick, 1u));
            });
            if (counter != signals.end())
            {
                signals.erase(counter);
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput);
                if (window.bEndsPatternOnSuccess) outOutput.bEndPatternEarly = true;
                outOutput.strStatus = "counter window succeeded";
            }
            else if (reachedEnd)
            {
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput);
            }
            break;
        }
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		{
			const std::uint32_t lost = state.iBossHpAtOpen > boss.iCurrentHp ?
				state.iBossHpAtOpen - boss.iCurrentHp : 0u;
			if (window.iThreshold > 0u && lost >= window.iThreshold)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput);
				if (window.bEndsPatternOnSuccess)
					outOutput.bEndPatternEarly = true;
				outOutput.strStatus = "stagger window succeeded";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput);
				outOutput.strStatus = "stagger window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
		{
			if (!reachedEnd)
				break;
			/* Judge before the cards go back so the card that was dealt is the
			card that is compared. */
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!Is_Judgeable(player))
					continue;
				const KOUKUSAYDON_LOGIC_ANSWER answer =
					BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH == window.eKind ?
						Judge_Roulette(window, boss, player) : Judge_Gaze(window, boss, player);
				state.Answers[playerId] = answer;
				verdicts.emplace_back(playerId, answer);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				Apply_Results(
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout),
					state, &found->second, players, boss, catalog, pMadnessPolicy, serverTick,
					outDamageEvents, outOutput);
			}
			outOutput.strStatus = "end-tick window judged";
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
		{
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ target{
				window.fTargetWorldX, window.fTargetWorldZ, window.fTargetRadiusM };
			const bool inside = !window.strTargetWorldInstanceId.empty() &&
				std::any_of(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) {
						return Intersects_LogicRegion(region, boss, target, serverTick - ledger.iPatternStartTick);
					});
			if (inside)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.bInsideIsFail ? window.OnFail : window.OnSuccess,
					state, nullptr, players, boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput);
				outOutput.strStatus = "object overlap window judged";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput);
				outOutput.strStatus = "object overlap window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
		{
			const bool enter = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA == window.eKind;
			if (!enter && !reachedEnd) break;
			for (auto& [playerId, player] : players)
			{
				if (!Is_Judgeable(player) || (enter && !window.bRearmOnExit && !window.bRepeatAfterKnockback &&
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == state.Answers[playerId]))
					continue;
				const auto caught = std::find_if(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) { return Contains_LogicRegion(region, boss, player, serverTick - ledger.iPatternStartTick); });
				const bool inside = window.CardRegions.end() != caught;
				const bool captureCandidate = inside && enter && !window.OnSuccess.empty() &&
					window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
				if (enter && window.bRepeatAfterKnockback)
				{
					if (inside)
					{
						const auto next = state.NextContactHitTicks.find(playerId);
						if (!CPlayerSkillSystem::Can_ArmPlayerHitReaction(player, serverTick) ||
							(next != state.NextContactHitTicks.end() && !Has_ReachedTick(serverTick, next->second))) continue;
					}
					else
					{
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (enter && window.bRearmOnExit)
				{
					if (inside)
					{
						if (captureCandidate)
						{
							if (state.InsidePlayers.contains(playerId)) continue;
						}
						else if (!state.InsidePlayers.insert(playerId).second) continue;
					}
					else
					{
						state.InsidePlayers.erase(playerId);
						// Timeout belongs only to players who never entered this window.
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (!inside && !reachedEnd) continue;
				// Capture is committed by the room; an ineligible candidate remains retryable.
				if (!captureCandidate)
					state.Answers[playerId] = inside ? (window.bInsideIsFail ? KOUKUSAYDON_LOGIC_ANSWER::FAIL :
						KOUKUSAYDON_LOGIC_ANSWER::SUCCESS) : KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& answered = inside ?
					(window.bInsideIsFail ? window.OnFail : window.OnSuccess) : window.OnTimeout;
				Apply_Results(answered, state, &player, players,
					boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput);
				if (inside && enter && window.bRepeatAfterKnockback && !answered.empty())
					state.NextContactHitTicks[playerId] = Add_Ticks(serverTick, Ticks_FromMs(answered.front().iPushMs));
				/* Only this seam knows which region caught this player, and that
				region is the thing that will carry them, so the grab happens here
				rather than in the typed result switch. */
				if (inside && std::any_of(answered.begin(), answered.end(),
					[](const BOSS_PATTERN_LOGIC_RESULT& result)
					{ return BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT == result.eKind; }))
					(void)Hang_PlayerOnRegion(player, *caught, boss,
						serverTick - ledger.iPatternStartTick, state.iWindowIndex,
						static_cast<std::uint32_t>(caught - window.CardRegions.begin()),
						state.iEndTick, serverTick);
			}
			if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
		{
			if (!reachedEnd)
				break;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!Is_Judgeable(player))
					continue;
				const auto answer = state.Answers.find(playerId);
				verdicts.emplace_back(playerId,
					state.Answers.end() == answer ? KOUKUSAYDON_LOGIC_ANSWER::NONE : answer->second);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results =
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout);
				Apply_Results(results, state, &found->second, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput);
			}
			outOutput.strStatus = "pose window judged";
			break;
		}
		default:
			if (reachedEnd)
				Close_Window(boss, window, state, players);
			break;
		}
	}
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Record_InteractionSlot(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_PLAYER& player,
	const LostArk::Shared::INTERACTION_SLOT slot,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
	{
		outStatus = "no running pattern owns a dance window";
		return false;
	}
	if (!Is_Valid_InteractionSlot(slot))
	{
		outStatus = "slot is out of range";
		return false;
	}
	const std::int8_t pose = player.ModeSkillIndexBySlot[static_cast<std::size_t>(slot)];
	if (pose < 0)
	{
		outStatus = "empty slot";
		return false;
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (!state.bOpened || state.bClosed ||
			state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT != window.eKind)
			continue;
		auto answer = state.Answers.find(player.iPlayerId);
		if (state.Answers.end() == answer)
			answer = state.Answers.emplace(player.iPlayerId, KOUKUSAYDON_LOGIC_ANSWER::NONE).first;
		if (KOUKUSAYDON_LOGIC_ANSWER::NONE != answer->second)
		{
			outStatus = "already answered";
			return false;
		}
		answer->second = static_cast<std::uint32_t>(pose) == window.iPoseIndex ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
		outStatus = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer->second ?
			"pose matched" : "pose mismatched";
		return true;
	}
	outStatus = "no open dance window";
	return false;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerModes(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const KOUKUSAYDON_LOGIC_LEDGER* const pActiveLedger,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	const KOUKU_HUD_MODE ledgerMode = nullptr != pActiveLedger && pActiveLedger->Is_Active() ?
		pActiveLedger->eHudMode : KOUKU_HUD_MODE::NONE;
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		const KOUKU_HUD_MODE patternMode = nullptr != pActiveLedger &&
			0u != player.iKoukuSuppressedPatternSequence &&
			player.iKoukuSuppressedPatternSequence == pActiveLedger->iPatternSequence ?
			KOUKU_HUD_MODE::NONE : ledgerMode;
		if (nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iMaximum)
		{
			player.iMaximumMadness = pMadnessPolicy->iMaximum;
			player.iCurrentMadness = (std::min)(player.iCurrentMadness, player.iMaximumMadness);
			if (player.iCurrentMadness >= player.iMaximumMadness)
				Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		}
		if (PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm &&
			0u != player.iMadnessFormEndTick &&
			Has_ReachedTick(serverTick, player.iMadnessFormEndTick))
		{
			if (KOUKU_HUD_MODE::NONE == patternMode &&
				KOUKU_HUD_MODE::NONE == player.eKoukuAreaHudMode &&
				KOUKU_HUD_MODE::NONE == player.eDebugKoukuHudModeOverride)
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			player.iMadnessFormEndTick = 0u;
		}
		for (std::int8_t& index : player.ModeSkillIndexBySlot)
			index = KOUKU_HUD_SLOT_EMPTY;
		const bool alive = 0u != player.iCurrentHp && PLAYER_ACTION_STATE::DEAD != player.eAction;
		if (!alive)
		{
			player.Clear_KoukuInteractionState();
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			continue;
		}
		KOUKU_HUD_MODE mode = KOUKU_HUD_MODE::NONE != patternMode ?
			patternMode : (KOUKU_HUD_MODE::NONE != player.eKoukuAreaHudMode ?
				player.eKoukuAreaHudMode : player.eDebugKoukuHudModeOverride);
		if (KOUKU_HUD_MODE::NONE != patternMode &&
			PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm)
		{
			player.bKoukuPatternOwnsClown = true;
			player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		}
		else if (KOUKU_HUD_MODE::NONE == patternMode && player.bKoukuPatternOwnsClown)
		{
			player.bKoukuPatternOwnsClown = false;
			if (KOUKU_HUD_MODE::NONE == mode && 0u == player.iMadnessFormEndTick)
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		}
		if (KOUKU_HUD_MODE::NONE == mode && PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm)
			mode = KOUKU_HUD_MODE::POLYMORPH;
		// A mode change cannot reinterpret an interaction animation already in flight.
		if (player.eKoukuHudMode != mode && (PLAYER_ACTION_STATE::INTERACTION == player.eAction ||
			PLAYER_ACTION_STATE::SKILL == player.eAction))
		{
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iComboStage = 0u;
			player.hasBufferedComboInput = false;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
		}
		player.eKoukuHudMode = mode;
		std::uint32_t count = 0u;
		switch (mode)
		{
		case KOUKU_HUD_MODE::DANCE: count = 4u; break;
		case KOUKU_HUD_MODE::POLYMORPH: count = 3u; break;
		case KOUKU_HUD_MODE::MARIO: count = 2u; break;
		case KOUKU_HUD_MODE::MAZE: count = 1u; break;
		case KOUKU_HUD_MODE::NONE: break;
		default: break;
		}
		for (std::uint32_t index = 0u; index < count; ++index)
			player.ModeSkillIndexBySlot[index] = static_cast<std::int8_t>(index);
	}
}

namespace
{
	constexpr LostArk::Shared::MECHANIC_CARD_SYMBOL CARD_MAZE_SUITS[4] = {
		LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND,
	};

	/* Server-owned xorshift seeded from the claim tick and claimant, so a
	deal is reproducible from the room's own clock and nothing else. */
	struct CARD_MAZE_RANDOM final
	{
		std::uint64_t iState;
		std::uint32_t Next() noexcept
		{
			iState ^= iState << 13;
			iState ^= iState >> 7;
			iState ^= iState << 17;
			return static_cast<std::uint32_t>(iState >> 32);
		}
		float Unit() noexcept
		{
			return static_cast<float>(Next() & 0xFFFFFFu) / 16777215.f;
		}
	};

	float DistanceXZ(const float ax, const float az, const float bx, const float bz) noexcept
	{
		const float dx = ax - bx;
		const float dz = az - bz;
		return std::sqrt(dx * dx + dz * dz);
	}
}

void LostArk::Server::CKoukuBingoRuntime::Promote_Lines() noexcept
{
	using namespace LostArk::Shared;
	for (std::int32_t line = 0; line < KOUKU_BINGO_LINE_COUNT; ++line)
	{
		const std::uint32_t lineMask = Kouku_BingoLineMask(line);
		if (0u != lineMask && lineMask == (m_iWhiteMask & lineMask))
			m_iRedMask |= lineMask;
	}
}

void LostArk::Server::CKoukuBingoRuntime::Fill(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	m_iWhiteMask |= cellMask & KOUKU_BINGO_ALL_CELLS_MASK;
	Promote_Lines();
}

void LostArk::Server::CKoukuBingoRuntime::Detonate(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	/* A finished bingo is never disturbed, so red cells drop out first and
	the rest is decided against the board as it stands right now. */
	const std::uint32_t affected =
		(cellMask & KOUKU_BINGO_ALL_CELLS_MASK) & ~m_iRedMask;
	const std::uint32_t cleared = affected & m_iWhiteMask;
	const std::uint32_t lit = affected & ~m_iWhiteMask;
	m_iWhiteMask = (m_iWhiteMask & ~cleared) | lit;
	Promote_Lines();
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Bomb(
	const LostArk::Shared::NET_ENTITY_ID carrier,
	const std::uint32_t detonateTick) noexcept
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == carrier)
		return false;
	std::size_t freeSlot = m_Bombs.size();
	for (std::size_t slot = 0u; slot < m_Bombs.size(); ++slot)
	{
		const BOMB& bomb = m_Bombs[slot];
		if (BINGO_BOMB_PHASE::NONE == bomb.ePhase)
		{
			if (freeSlot == m_Bombs.size())
				freeSlot = slot;
			continue;
		}
		if (BINGO_BOMB_PHASE::MARKED == bomb.ePhase &&
			bomb.iCarrierNetEntityId == carrier)
		{
			return false;
		}
	}
	if (freeSlot == m_Bombs.size())
		return false;
	BOMB& started = m_Bombs[freeSlot];
	started = {};
	started.ePhase = BINGO_BOMB_PHASE::MARKED;
	started.iCarrierNetEntityId = carrier;
	started.iDetonateTick = detonateTick;
	return true;
}

void LostArk::Server::CKoukuBingoRuntime::Plant_Bomb(
	const std::size_t slot, const float x, const float z,
	const std::uint32_t fuseTick) noexcept
{
	using namespace LostArk::Shared;
	if (slot >= m_Bombs.size() ||
		BINGO_BOMB_PHASE::MARKED != m_Bombs[slot].ePhase)
	{
		return;
	}
	BOMB& bomb = m_Bombs[slot];
	bomb.ePhase = BINGO_BOMB_PHASE::PLANTED;
	bomb.iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
	bomb.fPositionX = x;
	bomb.fPositionZ = z;
	bomb.iDetonateTick = fuseTick;
}

void LostArk::Server::CKoukuBingoRuntime::Clear_Bomb(
	const std::size_t slot) noexcept
{
	if (slot < m_Bombs.size())
		m_Bombs[slot] = {};
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Hammer(
	const std::int32_t anchor, const std::uint32_t startTick,
	const std::uint32_t endTick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE != m_Hammer.ePhase ||
		!Is_KoukuBingoHammerAnchor(anchor) || endTick <= startTick)
	{
		return false;
	}
	m_Hammer.iAnchor = anchor;
	m_Hammer.ePhase = BINGO_HAMMER_PHASE::RAISED;
	m_Hammer.iPhaseStartTick = startTick;
	m_Hammer.iPhaseEndTick = endTick;
	return true;
}

bool LostArk::Server::CKoukuBingoRuntime::Advance_Hammer(
	const std::uint32_t tick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE == m_Hammer.ePhase)
		return false;
	if (tick < m_Hammer.iPhaseEndTick)
		return true;
	/* Each phase hands its end tick to the next one, so the three windows
	stay contiguous however late this tick arrives. */
	const auto ticksFor = [](const std::uint32_t milliseconds) noexcept
	{
		return (milliseconds * 30u + 999u) / 1000u;
	};
	m_Hammer.iPhaseStartTick = m_Hammer.iPhaseEndTick;
	if (BINGO_HAMMER_PHASE::RAISED == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::DESCENDING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_DESCEND_MS);
		return true;
	}
	if (BINGO_HAMMER_PHASE::DESCENDING == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::SWEEPING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_SWEEP_MS);
		return true;
	}
	m_Hammer = {};
	return false;
}

bool LostArk::Server::CKoukuBingoRuntime::Is_Safe(
	const float x, const float z) const noexcept
{
	using namespace LostArk::Shared;
	const std::int32_t cell = Kouku_BingoCellAt(x, z);
	if (!Is_KoukuBingoCell(cell))
		return false;
	return 0u != (m_iRedMask & (1u << cell));
}

const char* LostArk::Server::CKoukuCardMazeRuntime::Archetype_ForSuit(
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept
{
	using namespace LostArk::Shared;
	switch (suit)
	{
	case MECHANIC_CARD_SYMBOL::HEART: return "MONSTER_KOUKU_CARD_HEART";
	case MECHANIC_CARD_SYMBOL::SPADE: return "MONSTER_KOUKU_CARD_SPADE";
	case MECHANIC_CARD_SYMBOL::CLUB: return "MONSTER_KOUKU_CARD_CLUB";
	case MECHANIC_CARD_SYMBOL::DIAMOND: return "MONSTER_KOUKU_CARD_DIAMOND";
	default: return "";
	}
}

bool LostArk::Server::CKoukuCardMazeRuntime::Plan(
	const LostArk::Shared::PLAYER_ID claimantId,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation& navigation,
	const std::uint32_t serverTick,
	std::vector<SPAWN_REQUEST>& outSpawns,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	outSpawns.clear();
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = INVALID_PLAYER_ID;
	const auto claimant = players.find(claimantId);
	if (players.end() == claimant || 0u == claimant->second.iCurrentHp)
	{
		outStatus = "Card maze claimant is not a living player";
		return false;
	}
	if (PHASE::HUNTING == m_ePhase)
	{
		outStatus = "Card maze is already running";
		return false;
	}
	/* Hunters are every other living player, in PlayerId order so the same
	room deals the same way whatever the map iteration happens to be. */
	std::vector<PLAYER_ID> hunters;
	for (const auto& [playerId, player] : players)
	{
		if (playerId != claimantId && 0u != player.iCurrentHp)
			hunters.push_back(playerId);
	}
	if (hunters.size() > 3u)
		hunters.resize(3u);
	bool soloTest = false;
#ifdef _DEBUG
	// Only a genuinely one-player room opts into the combined test role.
	soloTest = players.size() == 1u;
#endif
	if (hunters.empty() && !soloTest)
	{
		outStatus = "Card maze needs at least one other living player to hunt";
		return false;
	}
	CARD_MAZE_RANDOM random{
		Mix((static_cast<std::uint64_t>(serverTick) << 32) ^ static_cast<std::uint64_t>(claimantId)) | 1ull };
	MECHANIC_CARD_SYMBOL deck[4] = {
		CARD_MAZE_SUITS[0], CARD_MAZE_SUITS[1], CARD_MAZE_SUITS[2], CARD_MAZE_SUITS[3] };
	for (std::uint32_t index = 3u; index > 0u; --index)
	{
		const std::uint32_t swapWith = random.Next() % (index + 1u);
		std::swap(deck[index], deck[swapWith]);
	}
	PARTICIPANT owner{};
	owner.eRole = CARD_MAZE_ROLE::TELESCOPE;
	// Multiplayer telescope owners remain suitless; Debug solo is a hunter with overhead access.
	if (soloTest)
	{
		owner.eRole = CARD_MAZE_ROLE::HUNTER;
		owner.eSuit = deck[0];
	}
	std::vector<std::pair<PLAYER_ID, MECHANIC_CARD_SYMBOL>> dealt;
	if (soloTest) dealt.emplace_back(claimantId, owner.eSuit);
	for (std::size_t hunterIndex = 0u; hunterIndex < hunters.size(); ++hunterIndex)
		dealt.emplace_back(hunters[hunterIndex], deck[hunterIndex]);
	m_PendingParticipants[claimantId] = owner;
	std::vector<std::pair<float, float>> taken;
	for (const auto& [dealtId, dealtSuit] : dealt)
	{
		if (dealtId != claimantId)
		{
			PARTICIPANT hunter{};
			hunter.eRole = CARD_MAZE_ROLE::HUNTER;
			hunter.eSuit = dealtSuit;
			m_PendingParticipants[dealtId] = hunter;
		}
		for (std::uint32_t target = 0u; target < TARGETS_PER_SUIT; ++target)
		{
			bool placed = false;
			for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS && !placed; ++attempt)
			{
				/* A random point in the maze rectangle projected onto the
				nearest walkable cell: the CardMiro region is 0.5 m, so the
				result is a corridor cell, never the top of a card wall. */
				const float sampleX = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
				const float sampleZ = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
				SERVER_NAV_POINT point{};
				if (!navigation.Project_Point(sampleX, sampleZ, point) ||
					point.x < MAZE_MIN_X || point.x > MAZE_MAX_X ||
					point.z < MAZE_MIN_Z || point.z > MAZE_MAX_Z ||
					std::abs(point.y + .01f) > .5f ||
					DistanceXZ(point.x, point.z, CENTER_X, CENTER_Z) < CENTER_KEEPOUT_M)
				{
					continue;
				}
				bool clear = true;
				for (const auto& [playerId, player] : players)
				{
					(void)playerId;
					if (0u != player.iCurrentHp &&
						DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
					{
						clear = false;
						break;
					}
				}
				for (std::size_t takenIndex = 0u; clear && takenIndex < taken.size(); ++takenIndex)
				{
					if (DistanceXZ(point.x, point.z, taken[takenIndex].first, taken[takenIndex].second) < SPAWN_SPACING_M)
						clear = false;
				}
				if (!clear)
					continue;
				std::vector<SERVER_NAV_POINT> route;
				if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
					DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
				SPAWN_REQUEST request{};
				request.eSuit = dealtSuit;
				request.fPositionX = point.x;
				request.fPositionY = point.y;
				request.fPositionZ = point.z;
				request.fYawDegrees = 360.f * random.Unit();
				outSpawns.push_back(request);
				taken.emplace_back(point.x, point.z);
				placed = true;
			}
			if (!placed)
			{
				outStatus = "Card maze found no free corridor cell for a target";
				m_PendingParticipants.clear();
				outSpawns.clear();
				return false;
			}
		}
	}
	m_iPendingTelescopeOwner = claimantId;
	outStatus = "Card maze planned";
	return true;
}

void LostArk::Server::CKoukuCardMazeRuntime::Register_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId,
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit)
{
	m_Targets[entityId] = suit;
}

void LostArk::Server::CKoukuCardMazeRuntime::Commit(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	m_Participants = m_PendingParticipants;
	m_iTelescopeOwner = m_iPendingTelescopeOwner;
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::HUNTING;
	for (const auto& [playerId, participant] : m_Participants)
	{
		const auto player = players.find(playerId);
		if (players.end() != player)
		{
			player->second.CardMaze = {};
			player->second.eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::MAZE;
			player->second.CardMaze.flags = playerId == m_iTelescopeOwner ? 1u : 0u;
			Apply_ToPlayer(participant, player->second);
		}
	}
}

void LostArk::Server::CKoukuCardMazeRuntime::Abort()
{
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId) const noexcept
{
	return m_Targets.end() != m_Targets.find(entityId);
}

void LostArk::Server::CKoukuCardMazeRuntime::Apply_ToPlayer(
	const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	player.eCardMazeRole = participant.eRole;
	player.eCardMazeSuit = participant.eSuit;
	player.iCardMazeKills = participant.iKills;
	player.iCardMazeKillTarget = MECHANIC_CARD_SYMBOL::NONE != participant.eSuit ? KILL_TARGET : 0u;
}

LostArk::Server::CKoukuCardMazeRuntime::HIT_OUTCOME
LostArk::Server::CKoukuCardMazeRuntime::On_TargetHit(
	SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, const bool killed)
{
	using namespace LostArk::Shared;
	HIT_OUTCOME outcome{};
	const auto registered = m_Targets.find(target.iNetEntityId);
	const auto participant = m_Participants.find(hunter.iPlayerId);
	/* Only a dealt suit striking its own targets advances anything: a wrong
	suit, or the suitless owner, takes the damage the room already applied
	and nothing more. */
	if (PHASE::HUNTING != m_ePhase || m_Targets.end() == registered ||
		m_Participants.end() == participant ||
		participant->second.eSuit != registered->second)
	{
		return outcome;
	}
	if (!m_bMarchStarted)
	{
		m_bMarchStarted = true;
		outcome.bStartMarch = true;
	}
	if (killed && m_CountedKills.insert(target.iNetEntityId).second &&
		participant->second.iKills < KILL_TARGET)
	{
		++participant->second.iKills;
		outcome.bKillCounted = true;
		if (KILL_TARGET == participant->second.iKills)
		{
			outcome.bHunterComplete = true;
			bool allComplete = true;
			for (const auto& [playerId, other] : m_Participants)
			{
				(void)playerId;
				if (MECHANIC_CARD_SYMBOL::NONE != other.eSuit && other.iKills < KILL_TARGET)
				{
					allComplete = false;
					break;
				}
			}
			// Three kills only unlock a personal exit; central arrival completes the run.
			(void)allComplete;
		}
		Apply_ToPlayer(participant->second, hunter);
	}
	return outcome;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	if (PHASE::INACTIVE == m_ePhase)
		return false;
	m_Participants.erase(playerId);
	if (playerId == m_iTelescopeOwner)
		m_iTelescopeOwner = INVALID_PLAYER_ID;
	if (PHASE::HUNTING != m_ePhase)
		return false;
	std::size_t hunters = 0u;
	std::size_t hunting = 0u;
	for (const auto& [id, participant] : m_Participants)
	{
		(void)id;
		if (MECHANIC_CARD_SYMBOL::NONE == participant.eSuit)
			continue;
		++hunters;
		if (participant.iKills < KILL_TARGET)
			++hunting;
	}
	if (0u == hunters)
		return true;
	(void)hunting;
	return false;
}

bool LostArk::Server::CKoukuCardMazeRuntime::In_SafeZone(float x, float z) noexcept
{
	return DistanceXZ(x, z, CENTER_X, CENTER_Z) <= CENTER_KEEPOUT_M;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Can_Hit(
	const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const
{
	const auto p = m_Participants.find(player.iPlayerId);
	const auto t = m_Targets.find(targetId);
	return m_ePhase == PHASE::HUNTING && p != m_Participants.end() && t != m_Targets.end() &&
		!p->second.escaped && p->second.iKills < KILL_TARGET && p->second.eSuit == t->second;
}

void LostArk::Server::CKoukuCardMazeRuntime::Retire_Target(LostArk::Shared::NET_ENTITY_ID id)
{
	m_Targets.erase(id);
	m_CountedKills.erase(id);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Sample_Corridor(
	LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
	std::uint32_t seed, SPAWN_REQUEST& out) const
{
	CARD_MAZE_RANDOM random{ Mix(static_cast<std::uint64_t>(seed) ^ (static_cast<std::uint64_t>(suit) << 32)) | 1ull };
	for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS; ++attempt)
	{
		const float x = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
		const float z = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
		SERVER_NAV_POINT point{};
		if (!navigation.Is_PointWalkableExact(x, z) || !navigation.Sample_Position(x, z, point) ||
			std::abs(point.y + .01f) > .5f || In_SafeZone(point.x, point.z)) continue;
		bool clear = true;
		for (const auto& [id, player] : players)
		{
			(void)id;
			if (player.iCurrentHp && DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
			{ clear = false; break; }
		}
		for (const auto& entity : entities)
			if (entity.iCurrentHp && Is_Target(entity.iNetEntityId) &&
				DistanceXZ(point.x, point.z, entity.fPositionX, entity.fPositionZ) < SPAWN_SPACING_M) clear = false;
		if (!clear) continue;
		std::vector<SERVER_NAV_POINT> route;
		if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
			DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
		out = {suit, point.x, point.y, point.z, 360.f * random.Unit()};
		return true;
	}
	return false;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset_Progress(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || p->second.escaped) return;
	p->second.iKills = 0u;
	p->second.escaped = false;
	player.CardMaze.flags &= static_cast<std::uint8_t>(~6u);
	player.CardMaze.exitX = player.CardMaze.exitY = player.CardMaze.exitZ = 0.f;
	Apply_ToPlayer(p->second, player);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Toggle_Telescope(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || !In_SafeZone(player.fPositionX, player.fPositionZ) ||
		(p->second.eRole != LostArk::Shared::CARD_MAZE_ROLE::TELESCOPE && !p->second.escaped &&
			!Is_SoloHunter(player.iPlayerId))) return false;
	player.CardMaze.flags ^= 1u;
	player.hasMoveGoal = false;
	player.MovePath.clear();
	return true;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_SoloHunter(
	LostArk::Shared::PLAYER_ID playerId) const noexcept
{
	const auto p = m_Participants.find(playerId);
	return playerId == m_iTelescopeOwner && m_Participants.size() == 1u && p != m_Participants.end() &&
		p->second.eRole == LostArk::Shared::CARD_MAZE_ROLE::HUNTER;
}

void LostArk::Server::CKoukuCardMazeRuntime::Mark_Escaped(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end()) return;
	p->second.escaped = true;
	player.CardMaze.flags = static_cast<std::uint8_t>((player.CardMaze.flags | 2u) & ~5u);
}

bool LostArk::Server::CKoukuCardMazeRuntime::All_LivingCentral(
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const
{
	bool anyLiving = false;
	for (const auto& [id, participant] : m_Participants)
	{
		const auto p = players.find(id);
		if (p == players.end() || !p->second.iCurrentHp) continue;
		anyLiving = true;
		if (!In_SafeZone(p->second.fPositionX, p->second.fPositionZ) || p->second.CardMaze.transferStartTick ||
			(participant.eSuit != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE && !participant.escaped)) return false;
	}
	return anyLiving;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	for (const auto& [playerId, participant] : m_Participants)
	{
		(void)participant;
		const auto player = players.find(playerId);
		if (players.end() != player)
			player->second.Clear_CardMazeState();
	}
	m_Participants.clear();
	m_PendingParticipants.clear();
	m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::INACTIVE;
}
