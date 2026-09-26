#include "ServerCombatHitRuntime.h"

#include "BossCombatRuntime.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;

	std::uint32_t SumIntactLegacyArmorDefense(
		const LostArk::Server::SERVER_WORLD_ENTITY& target)
	{
		std::uint32_t total = 0u;
		for (const auto& plate : target.ArmorPlates)
		{
			if (0u != plate.iRemainingDurability)
				total += plate.iDefense;
		}
		return total;
	}

	bool ConsumeLegacyArmorDurability(
		LostArk::Server::SERVER_WORLD_ENTITY& target,
		const std::uint32_t damage)
	{
		if (0u == damage)
			return false;
		for (auto& plate : target.ArmorPlates)
		{
			if (0u == plate.iRemainingDurability)
				continue;
			plate.iRemainingDurability =
				damage >= plate.iRemainingDurability ?
					0u : plate.iRemainingDurability - damage;
			return 0u == plate.iRemainingDurability;
		}
		return false;
	}

	void MirrorTypedPartBreakToLegacyArmor(
		LostArk::Server::SERVER_WORLD_ENTITY& target,
		const std::uint32_t destroyedPartMask)
	{
		for (auto& plate : target.ArmorPlates)
		{
			if (plate.iPlateIndex >= 32u ||
				0u == (destroyedPartMask & (1u << plate.iPlateIndex)))
			{
				continue;
			}
			plate.iRemainingDurability = 0u;
		}
	}

	void PushDamageEvent(
		const LostArk::Shared::NET_ENTITY_ID targetId,
		const std::uint32_t amount,
		const float x,
		const float y,
		const float z,
		const bool outgoing,
		std::vector<LostArk::Shared::DAMAGE_EVENT>& events,
		const LostArk::Shared::PLAYER_ID sourcePlayerId = LostArk::Shared::INVALID_PLAYER_ID,
		const std::uint32_t staggerAmount = 0u,
		const bool counterSuccess = false,
		const bool critical = false,
		const LostArk::Shared::DAMAGE_HIT_FLAG hitFlag = LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL)
	{
		/* A counter or stagger-only hit still reaches the combat analyzer. */
		if ((0u == amount && 0u == staggerAmount && !counterSuccess) ||
			events.size() >= LostArk::Shared::MAX_DAMAGE_EVENTS)
			return;
		LostArk::Shared::DAMAGE_EVENT event{};
		event.iTargetNetEntityId = targetId;
		event.iAmount = amount;
		event.fPositionX = x;
		event.fPositionY = y;
		event.fPositionZ = z;
		event.isOutgoing = outgoing;
		event.iSourcePlayerId = sourcePlayerId;
		event.iStaggerAmount = staggerAmount;
		event.isCounterSuccess = counterSuccess;
		event.eHitFlag = hitFlag;
		if (critical && 0u != amount && hitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL)
			event.eHitFlag = LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL;
		events.push_back(event);
	}
}

namespace
{
	/* The room's fixed rate; a duration in milliseconds becomes whole ticks. */
	constexpr std::uint32_t BUFF_TICK_HZ = 30u;

	void GrantBuff(
		std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs,
		const std::uint32_t buffId,
		const std::uint32_t endTick)
	{
		for (LostArk::Shared::ACTIVE_BUFF& existing : buffs)
		{
			if (existing.iBuffId == buffId)
			{
				/* A recast refreshes rather than stacks, as the original does. */
				existing.iEndTick = (std::max)(existing.iEndTick, endTick);
				return;
			}
		}
		if (buffs.size() >= LostArk::Shared::MAX_ACTIVE_BUFFS)
			return;
		LostArk::Shared::ACTIVE_BUFF granted{};
		granted.iBuffId = buffId;
		granted.iEndTick = endTick;
		buffs.push_back(granted);
	}
}

void LostArk::Server::CServerBuffRuntime::Apply_SkillBuffs(
	const CGameplayCatalog& catalog,
	const std::uint32_t skillId,
	SERVER_PLAYER& caster,
	std::vector<SERVER_PLAYER*>& allies,
	std::vector<SERVER_WORLD_ENTITY*>& enemies,
	const std::uint32_t serverTick)
{
	const std::vector<CGameplayCatalog::SKILL_BUFF_DEFINITION>* definitions =
		catalog.Find_SkillBuffs(skillId);
	if (nullptr == definitions)
		return;
	/* A shield is worth a share of the caster's maximum HP, as the original reads
	it off the buff's caster rather than off whoever receives it. */
	const PLAYER_RUNTIME_PROFILE* casterProfile =
		catalog.Find_Player(caster.eCharacterClass);
	const std::uint32_t casterMaximumHp =
		nullptr == casterProfile ? 0u : casterProfile->iMaximumHp;
	const auto grantShield = [&](SERVER_PLAYER& holder, const std::uint32_t percent)
	{
		if (0u == percent || 0u == casterMaximumHp)
			return;
		const std::uint64_t shield =
			static_cast<std::uint64_t>(casterMaximumHp) * percent / 100ull;
		holder.iShield = static_cast<std::uint32_t>((std::max<std::uint64_t>)(
			holder.iShield, (std::min<std::uint64_t>)(
				shield, (std::numeric_limits<std::uint32_t>::max)())));
	};
	for (const CGameplayCatalog::SKILL_BUFF_DEFINITION& definition : *definitions)
	{
		const std::uint32_t endTick = serverTick +
			(definition.iDurationMs * BUFF_TICK_HZ + 999u) / 1000u;
		switch (definition.eTarget)
		{
		case CGameplayCatalog::SKILL_BUFF_TARGET::SELF:
			GrantBuff(caster.ActiveBuffs, definition.iBuffId, endTick);
			grantShield(caster, definition.iShieldPercentOfMaxHp);
			break;
		case CGameplayCatalog::SKILL_BUFF_TARGET::ALLY:
			/* The original counts the caster among the party it protects. */
			GrantBuff(caster.ActiveBuffs, definition.iBuffId, endTick);
			grantShield(caster, definition.iShieldPercentOfMaxHp);
			for (SERVER_PLAYER* ally : allies)
			{
				if (nullptr == ally || ally == &caster || 0u == ally->iCurrentHp)
					continue;
				GrantBuff(ally->ActiveBuffs, definition.iBuffId, endTick);
				grantShield(*ally, definition.iShieldPercentOfMaxHp);
			}
			break;
		case CGameplayCatalog::SKILL_BUFF_TARGET::ENEMY:
			for (SERVER_WORLD_ENTITY* enemy : enemies)
			{
				if (nullptr == enemy || 0u == enemy->iCurrentHp ||
					!LostArk::Shared::Is_PlayerDamageableWorldArchetype(enemy->strArchetypeId))
					continue;
				GrantBuff(enemy->ActiveBuffs, definition.iBuffId, endTick);
				/* A boss keeps its own pattern clock, so it takes the mark but
				never the stun, which is how the original treats commanders. */
				if (0u != definition.iStunMs &&
					WORLD_BOOTSTRAP_KIND::MONSTER == enemy->eKind)
				{
					const std::uint32_t stunEnd = serverTick +
						(definition.iStunMs * BUFF_TICK_HZ + 999u) / 1000u;
					enemy->iStunEndTick = (std::max)(enemy->iStunEndTick, stunEnd);
				}
			}
			break;
		}
	}
}

void LostArk::Server::CServerBuffRuntime::Expire(
	std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs,
	const std::uint32_t serverTick)
{
	buffs.erase(
		std::remove_if(buffs.begin(), buffs.end(),
			[serverTick](const LostArk::Shared::ACTIVE_BUFF& buff)
			{
				return buff.iEndTick <= serverTick;
			}),
		buffs.end());
}

namespace
{
	std::uint32_t Death_DenyInvulnerableMs(
		const LostArk::Server::CGameplayCatalog& catalog,
		const LostArk::Server::SERVER_PLAYER& player)
	{
		for (const LostArk::Shared::ACTIVE_BUFF& buff : player.ActiveBuffs)
		{
			const LostArk::Server::CGameplayCatalog::SKILL_BUFF_DEFINITION*
				definition = catalog.Find_SkillBuff(buff.iBuffId);
			if (nullptr != definition && 0u != definition->iDeathDenyInvulnerableMs)
				return definition->iDeathDenyInvulnerableMs;
		}
		return 0u;
	}

	void Consume_DeathDeny(
		const LostArk::Server::CGameplayCatalog& catalog,
		LostArk::Server::SERVER_PLAYER& player)
	{
		player.ActiveBuffs.erase(
			std::remove_if(player.ActiveBuffs.begin(), player.ActiveBuffs.end(),
				[&catalog](const LostArk::Shared::ACTIVE_BUFF& buff)
				{
					const LostArk::Server::CGameplayCatalog::SKILL_BUFF_DEFINITION*
						definition = catalog.Find_SkillBuff(buff.iBuffId);
					return nullptr != definition &&
						0u != definition->iDeathDenyInvulnerableMs;
				}),
			player.ActiveBuffs.end());
	}
}

void LostArk::Server::CServerBuffRuntime::Settle_Shield(
	const CGameplayCatalog& catalog,
	SERVER_PLAYER& player)
{
	if (0u == player.iShield)
		return;
	for (const LostArk::Shared::ACTIVE_BUFF& buff : player.ActiveBuffs)
	{
		const CGameplayCatalog::SKILL_BUFF_DEFINITION* definition =
			catalog.Find_SkillBuff(buff.iBuffId);
		if (nullptr != definition && 0u != definition->iShieldPercentOfMaxHp)
			return;
	}
	player.iShield = 0u;
}

std::int32_t LostArk::Server::CServerBuffRuntime::Damage_DealtPercent(
	const CGameplayCatalog& catalog,
	const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs)
{
	std::int32_t percent = 0;
	for (const LostArk::Shared::ACTIVE_BUFF& buff : buffs)
	{
		const CGameplayCatalog::SKILL_BUFF_DEFINITION* definition =
			catalog.Find_SkillBuff(buff.iBuffId);
		if (nullptr != definition)
			percent += definition->iDamageDealtPercent;
	}
	return percent;
}

std::int32_t LostArk::Server::CServerBuffRuntime::Damage_TakenPercent(
	const CGameplayCatalog& catalog,
	const std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs)
{
	std::int32_t percent = 0;
	for (const LostArk::Shared::ACTIVE_BUFF& buff : buffs)
	{
		const CGameplayCatalog::SKILL_BUFF_DEFINITION* definition =
			catalog.Find_SkillBuff(buff.iBuffId);
		if (nullptr != definition)
			percent += definition->iDamageTakenPercent;
	}
	return percent;
}

std::uint32_t LostArk::Server::CServerBuffRuntime::Scale_Damage(
	const std::uint32_t damage,
	const std::int32_t percent)
{
	if (0u == damage || 0 == percent)
		return damage;
	/* A stack of mitigation can never make a hit heal, so the floor is 1. */
	const std::int64_t scaled =
		static_cast<std::int64_t>(damage) * (100 + percent) / 100;
	if (scaled < 1)
		return 1u;
	return static_cast<std::uint32_t>((std::min<std::int64_t>)(
		scaled, static_cast<std::int64_t>((std::numeric_limits<std::uint32_t>::max)())));
}

LostArk::Server::SERVER_MVP_LEDGER_ROW& LostArk::Server::Find_Or_Add_MvpLedgerRow(
	std::vector<SERVER_MVP_LEDGER_ROW>& ledger, const LostArk::Shared::PLAYER_ID playerId)
{
	for (SERVER_MVP_LEDGER_ROW& row : ledger)
	{
		if (row.iPlayerId == playerId)
			return row;
	}
	SERVER_MVP_LEDGER_ROW& row = ledger.emplace_back();
	row.iPlayerId = playerId;
	return row;
}

bool LostArk::Server::CServerCombatHitRuntime::Is_PlayerDamageableWorldTarget(
	const SERVER_WORLD_ENTITY& target) noexcept
{
	return (WORLD_BOOTSTRAP_KIND::BOSS == target.eKind ||
		WORLD_BOOTSTRAP_KIND::MONSTER == target.eKind ||
		WORLD_BOOTSTRAP_KIND::WORLD_OBJECT == target.eKind) &&
		LostArk::Shared::Is_PlayerDamageableWorldArchetype(target.strArchetypeId) &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == target.iOwnerBossNetEntityId &&
		SERVER_ENTITY_ACTION::DEAD != target.eAction && 0u != target.iCurrentHp;
}

LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_PlayerToWorld(
	SERVER_WORLD_ENTITY& target,
	const SERVER_PLAYER_TO_WORLD_HIT& hit,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	if (!Is_PlayerDamageableWorldTarget(target) ||
		((WORLD_BOOTSTRAP_KIND::MONSTER == target.eKind || WORLD_BOOTSTRAP_KIND::WORLD_OBJECT == target.eKind) && hit.iRawDamage == 0u) ||
		(target.strSpawnGroupId == "cardmaze.targets" && hit.iSkillId != 56411u) ||
		LostArk::Shared::INVALID_SKILL_ID == hit.iSkillId)
	{
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	}

	std::uint32_t damage = 0u;
	std::uint32_t staggerDealt = 0u;
	bool counterTriggered = false;
	if (WORLD_BOOTSTRAP_KIND::BOSS == target.eKind)
	{
		/* Product Valtan still carries the original armour plates because pattern
		selection and older snapshots consume them.  Resolve that defense exactly
		once, then tell the typed runtime that the HP amount is already reduced.
		The typed part state remains the sole part-durability authority whenever
		it is authored. */
		if (target.bPatternInvulnerable)
			return SERVER_COMBAT_HIT_RESULT::ABSORBED;
		const bool hasLegacyArmor = !target.ArmorPlates.empty();
		const bool hasTypedParts = !target.BossCombat.Parts.empty();
		BOSS_INCOMING_HIT incoming{};
		incoming.iSourcePlayerId = hit.iSourcePlayerId;
		incoming.iSkillId = hit.iSkillId;
		incoming.iRawDamage = hasLegacyArmor && !hit.bHealthDamagePreResolved ?
			CGameplayCatalog::Apply_Defense(
				hit.iRawDamage, SumIntactLegacyArmorDefense(target)) :
			hit.iRawDamage;
		incoming.iStaggerDamage = hit.iStaggerDamage;
		incoming.iPartDamage = hasTypedParts ? hit.iPartDamage : 0u;
		incoming.iCounterPower = hit.iCounterPower;
		incoming.iServerTick = hit.iServerTick;
		incoming.bHealthDamagePreResolved = hasLegacyArmor || hit.bHealthDamagePreResolved;
		incoming.fSourceX = hit.fSourceX;
		incoming.fSourceZ = hit.fSourceZ;
		const BOSS_HIT_RESULT bossHit =
			CBossCombatRuntime::Apply_PlayerHit(target, incoming);
		damage = bossHit.iHealthDamage;
		PushDamageEvent(target.iNetEntityId, bossHit.iShieldDamage,
			target.fPositionX, target.fPositionY, target.fPositionZ, true, outDamageEvents,
			hit.iSourcePlayerId, 0u, false, false, LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB);
		staggerDealt = bossHit.iStaggerDamage;
		counterTriggered = bossHit.bCounterTriggered;
		if (LostArk::Shared::INVALID_PLAYER_ID != hit.iSourcePlayerId &&
			(0u != damage || 0u != staggerDealt || counterTriggered || 0u != bossHit.iPartDamage))
		{
			SERVER_MVP_LEDGER_ROW& row =
				Find_Or_Add_MvpLedgerRow(target.MvpLedger, hit.iSourcePlayerId);
			row.iDamage += damage;
			row.iStagger += staggerDealt;
			row.iPartDamage += bossHit.iPartDamage;
			if (counterTriggered)
			{
				++row.iCounterCount;
				if (0u != row.iLastCounterTick && hit.iServerTick > row.iLastCounterTick)
				{
					const std::uint32_t gapTicks = hit.iServerTick - row.iLastCounterTick;
					if (0u == row.iMinCounterGapTicks || gapTicks < row.iMinCounterGapTicks)
						row.iMinCounterGapTicks = gapTicks;
				}
				row.iLastCounterTick = hit.iServerTick;
			}
			/* The hit that took the boss's last health. */
			if (0u != damage && 0u == target.iCurrentHp)
				++row.iFinishingBlows;
		}
		if (bossHit.bPartDestroyed)
		{
			MirrorTypedPartBreakToLegacyArmor(
				target, bossHit.iDestroyedPartMask);
			(void)CBossCombatRuntime::Set_Flag(
				target.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY, false);
			target.bPatternGroggy = false;
			target.bPendingArmorBreakReaction = true;
		}
		else if (!hasTypedParts && target.bPatternGroggy &&
			ConsumeLegacyArmorDurability(target, damage))
		{
			/* Compatibility for bosses/fixtures that have not authored typed parts.
			Product Valtan never enters this branch. */
			target.bPatternGroggy = false;
			target.bPendingArmorBreakReaction = true;
		}
	}
	else
	{
		damage = target.strSpawnGroupId == "cardmaze.targets" ? target.iCurrentHp :
			CGameplayCatalog::Apply_Defense(hit.iRawDamage, target.iDefense);
		target.iCurrentHp = damage >= target.iCurrentHp ?
			0u : target.iCurrentHp - damage;
	}
	/* A card maze soldier is a gimmick token, not a damage race: the rule
	above removes its whole HP in one blow, so a floating number would read as
	the soldier's max HP over every corpse. The hit still reports KILLED and
	the maze still counts it; only the presentation number is withheld. */
	if (target.strSpawnGroupId != "cardmaze.targets")
	{
		PushDamageEvent(
			target.iNetEntityId, damage,
			target.fPositionX, target.fPositionY, target.fPositionZ,
			true, outDamageEvents,
			hit.iSourcePlayerId, staggerDealt, counterTriggered, hit.bCritical);
	}

	const float pushDistance = 0u == hit.iPushMs ?
		0.f : hit.fPushRangeM * target.fHitKnockbackScale;
	if (0u != damage && WORLD_BOOTSTRAP_KIND::MONSTER == target.eKind &&
		0.f != pushDistance && 0u != target.iCurrentHp)
	{
		float directionX = target.fPositionX - hit.fSourceX;
		float directionZ = target.fPositionZ - hit.fSourceZ;
		const float length = std::sqrt(
			directionX * directionX + directionZ * directionZ);
		if (length < 0.0001f)
		{
			directionX = hit.fFallbackDirectionX;
			directionZ = hit.fFallbackDirectionZ;
		}
		else
		{
			directionX /= length;
			directionZ /= length;
		}
		const float durationSeconds =
			static_cast<float>(hit.iPushMs) * MILLISECONDS_TO_SECONDS;
		target.fKnockbackDirectionX = directionX;
		target.fKnockbackDirectionZ = directionZ;
		target.fKnockbackSpeed = pushDistance / durationSeconds;
		target.fKnockbackRemainingSeconds = durationSeconds;
	}
	if (0u != target.iCurrentHp)
		return SERVER_COMBAT_HIT_RESULT::LANDED;
	target.eAction = SERVER_ENTITY_ACTION::DEAD;
	target.iActionStartTick = 0u == hit.iServerTick ? 1u : hit.iServerTick;
	target.MovePath.clear();
	return SERVER_COMBAT_HIT_RESULT::KILLED;
}

void LostArk::Server::CServerCombatHitRuntime::Add_MadnessGauge(
	SERVER_PLAYER& target, const double gain)
{
	using namespace LostArk::Shared;
	if (!target.iMaximumMadness || !target.iCurrentHp || !target.isCombatReady ||
		target.eMadnessForm != PLAYER_MADNESS_FORM::NORMAL ||
		!std::isfinite(gain) || gain <= 0.) return;
	const double total = target.iCurrentMadness + target.dMadnessRemainder + gain;
	const double clamped = (std::min)(total, static_cast<double>(target.iMaximumMadness));
	target.iCurrentMadness = static_cast<std::uint32_t>(clamped + 1e-9);
	target.dMadnessRemainder = (std::max)(0., clamped - target.iCurrentMadness);
}

LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_WorldToPlayer(
	SERVER_PLAYER& target,
	const SERVER_WORLD_TO_PLAYER_HIT& hit,
	const CGameplayCatalog& catalog,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents)
{
	using namespace LostArk::Shared;
	if (hit.bUsePushDirection && (!std::isfinite(hit.fPushDirectionX) || !std::isfinite(hit.fPushDirectionZ) ||
		hit.fPushDirectionX * hit.fPushDirectionX + hit.fPushDirectionZ * hit.fPushDirectionZ < .000001f))
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	const bool ownedCapture = Is_CurrentBossHandCapture(target, hit.iCaptureOwnerId,
		hit.iCapturePatternSequence, hit.iServerTick);
	if (0u == target.iCurrentHp || (!hit.bEncounterWipe && !ownedCapture && !target.isCombatReady) ||
		PLAYER_ACTION_STATE::DEAD == target.eAction ||
		(!hit.bEncounterWipe && (PLAYER_ACTION_STATE::FALLING == target.eAction ||
		(PLAYER_ACTION_STATE::GRABBED == target.eAction && !ownedCapture))))
	{
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	}
	if (!hit.bEncounterWipe && hit.iServerTick < target.iInvulnerableEndTick)
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;
	if (!hit.bEncounterWipe && !hit.bIgnoreCounter &&
		CPlayerSkillSystem::Try_Counter(target, catalog, hit.iServerTick))
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;

	const PLAYER_RUNTIME_PROFILE* playerProfile =
		catalog.Find_Player(target.eCharacterClass);
	const std::uint32_t mitigated = hit.bIgnoreDefense ? hit.iRawDamage :
		CGameplayCatalog::Apply_Defense(
			hit.iRawDamage, nullptr == playerProfile ? 0u : playerProfile->iDefense);
	/* A guardian's protection reduces what the hit finally takes off. */
	const std::uint32_t damage = hit.bEncounterWipe ? target.iCurrentHp : CServerBuffRuntime::Scale_Damage(
		mitigated,
		CServerBuffRuntime::Damage_TakenPercent(catalog, target.ActiveBuffs));
	/* The shield takes the hit first and only what it cannot hold reaches HP. */
	std::uint32_t throughShield = damage;
	if (!hit.bEncounterWipe && 0u != target.iShield && 0u != throughShield)
	{
		const std::uint32_t absorbed = (std::min)(target.iShield, throughShield);
		target.iShield -= absorbed;
		throughShield -= absorbed;
		PushDamageEvent(target.iNetEntityId, absorbed,
			target.fPositionX, target.fPositionY, target.fPositionZ, false, outDamageEvents,
			INVALID_PLAYER_ID, 0u, false, false, DAMAGE_HIT_FLAG::ABSORB);
	}
	if (!hit.bEncounterWipe && throughShield >= target.iCurrentHp &&
		0u != Death_DenyInvulnerableMs(catalog, target))
	{
		/* The original leaves the holder alive and untouchable for a moment
		instead of killing it; the buff is spent by clearing what armed it. */
		const std::uint32_t invulnerableMs =
			Death_DenyInvulnerableMs(catalog, target);
		throughShield = target.iCurrentHp - 1u;
		target.iInvulnerableEndTick = hit.iServerTick +
			(invulnerableMs * BUFF_TICK_HZ + 999u) / 1000u;
		Consume_DeathDeny(catalog, target);
	}
	const std::uint32_t hpBefore = target.iCurrentHp;
	target.iCurrentHp = throughShield >= target.iCurrentHp ?
		0u : target.iCurrentHp - throughShield;
	// Count only HP actually removed after mitigation, shield and death-deny.
	// The room enables this policy only for Kouku; client snapshots never set it.
	if (target.iMadnessDamageGainPercent && target.iMaximumHp)
		Add_MadnessGauge(target, static_cast<double>(hpBefore - target.iCurrentHp) *
			target.iMaximumMadness / target.iMaximumHp * target.iMadnessDamageGainPercent / 100.);
	PushDamageEvent(
		target.iNetEntityId, hpBefore - target.iCurrentHp,
		target.fPositionX, target.fPositionY, target.fPositionZ,
		false, outDamageEvents);
	if (0u == target.iCurrentHp)
	{
		if (hit.bEncounterWipe)
		{
			target.iShield = 0u; target.iInvulnerableEndTick = 0u;
			target.ActiveBuffs.clear(); target.Clear_Attachment();
		}
		target.eAction = PLAYER_ACTION_STATE::DEAD;
		target.iCurrentSkillId = INVALID_SKILL_ID;
		target.Clear_SkillTarget();
		target.iActionStartTick = 0u == hit.iServerTick ? 1u : hit.iServerTick;
		target.hasMoveGoal = false;
		target.MovePath.clear();
		return SERVER_COMBAT_HIT_RESULT::KILLED;
	}
	CPlayerSkillSystem::Arm_PlayerHitReaction(
		target,
		hit.bUsePushDirection ? target.fPositionX - hit.fPushDirectionX : hit.fSourceX,
		hit.bUsePushDirection ? target.fPositionZ - hit.fPushDirectionZ : hit.fSourceZ,
		hit.fPushRangeM,
		hit.iPushMs,
		hit.bKnockdown,
		hit.iDownMs,
		hit.iServerTick, hit.bForcePush, hit.bPushCanLeaveArena, hit.bPushBallistic, hit.fPushHeightM);
	return SERVER_COMBAT_HIT_RESULT::LANDED;
}
