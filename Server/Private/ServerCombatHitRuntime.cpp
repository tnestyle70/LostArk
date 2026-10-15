#include "ServerCombatHitRuntime.h"

#include "BossCombatRuntime.h"
#include "ColosseumCombatPolicy.h"
#include "PlayerSkillSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	constexpr float MILLISECONDS_TO_SECONDS = 0.001f;

	// 살아 있는 기존 갑옷판의 방어값을 합한다. typed part 감소와 중복 적용하지 않도록 상위 피해 계산에서 사용한다.
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

	// typed Parts가 없는 경로에서 첫 남은 갑옷판의 내구도를 줄인다. true는 이번에 그 판이 0이 되었음을 뜻한다.
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

	// typed 파괴 mask를 기존 표시/선택용 갑옷판에 반영한다. index 32 이상은 비트 이동하지 않고 건너뛴다.
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

	// 양방향 피해 경로가 공용 표시 event를 만든다. 출력 개수 상한에 닿으면 이 event만 추가하지 않는다.
	// 이 함수의 생략은 앞서 적용한 HP나 전투 상태를 rollback하지 않는다.
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
		const LostArk::Shared::DAMAGE_HIT_FLAG hitFlag = LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL,
		const bool staggerSuccess = false,
		const LostArk::Shared::MARIO_HIT_SOURCE marioHitSource = LostArk::Shared::MARIO_HIT_SOURCE::NONE)
	{
		// HP 피해가 0이어도 무력화/카운터/무적 결과는 표시 분석에 필요하므로 별도로 허용한다.
		if ((0u == amount && 0u == staggerAmount && !counterSuccess && !staggerSuccess &&
			hitFlag != LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE) ||
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
		event.isStaggerSuccess = staggerSuccess;
		event.eMarioHitSource = marioHitSource;
		event.eHitFlag = hitFlag;
		if (critical && 0u != amount && hitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL)
			event.eHitFlag = LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL;
		events.push_back(event);
	}
}

namespace
{
	// buff의 ms 기간을 방의 30Hz 정수 tick으로 올림 환산한다.
	constexpr std::uint32_t BUFF_TICK_HZ = 30u;

	// 보유 vector에서 같은 buff ID의 기한을 연장하거나 새 행을 추가한다. 최대 보유량이면 새 행만 추가하지 않는다.
	void GrantBuff(
		std::vector<LostArk::Shared::ACTIVE_BUFF>& buffs,
		const std::uint32_t buffId,
		const std::uint32_t endTick)
	{
		for (LostArk::Shared::ACTIVE_BUFF& existing : buffs)
		{
			if (existing.iBuffId == buffId)
			{
				// 같은 buff 재적용은 중첩 행을 늘리지 않고 더 늦은 종료 tick을 유지한다.
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

// GameRoom이 준비한 대상 포인터를 호출 중 차용한다. buff 정의가 없으면 변경 없이 반환한다.
// 보호막은 기존 값과 새 값 중 큰 값을 사용한다. 각 수신자 갱신은 즉시 적용되어 전체 rollback은 제공하지 않는다.
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
	// 보호막 비율은 수신자가 아니라 시전자 class profile의 최대 HP를 기준으로 계산한다.
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
			// ALLY는 시전자 자신에게도 적용한 뒤 목록의 다른 살아 있는 아군에 적용한다.
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
				// 보스는 자체 pattern 시간을 사용하므로 buff는 부여하되 이 경로의 stun은 MONSTER에만 적용한다.
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

// PlayerSkillSystem의 PvP 명중 경로가 호출한다. 상대 경기/팀 조건을 다시 확인한 뒤 ENEMY buff만 부여한다.
void LostArk::Server::CServerBuffRuntime::Apply_ColosseumEnemyBuffs(
    const CGameplayCatalog& catalog, const std::uint32_t skillId,
    const SERVER_PLAYER& caster, SERVER_PLAYER& target, const std::uint32_t serverTick,
    const SERVER_COLOSSEUM_COMBAT_CONTEXT& context)
{
    if (!Is_ColosseumOpponent(context, caster, target))
        return;
    const auto* definitions = catalog.Find_SkillBuffs(skillId);
    if (!definitions)
        return;
    for (const auto& definition : *definitions)
        if (definition.eTarget == CGameplayCatalog::SKILL_BUFF_TARGET::ENEMY)
            GrantBuff(target.ActiveBuffs, definition.iBuffId,
                serverTick + (definition.iDurationMs * BUFF_TICK_HZ + 999u) / 1000u);
}

// 방의 buff 갱신이 만료된 행을 제거한다. 삭제 뒤 vector의 기존 참조/iterator를 유지하면 안 된다.
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
	// 치명 피해 전에 첫 사망 방지 buff의 무적 ms를 찾는다. 만료 행 제거는 호출 전 tick 처리의 책임이다.
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

	// 사망 방지 발동 뒤 해당 속성을 가진 보유 buff를 모두 지운다. 하나의 ID만 지우는 처리가 아니다.
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

// buff 만료 처리 뒤 보호막 부여 buff가 남아 있는지 확인한다. 남은 buff가 없다면 보호막 잔량도 제거한다.
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

// 스킬 피해 계산이 보유 buff 정의의 공격 피해 비율을 합산한다. 목록과 Catalog는 읽기만 한다.
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

// 피격/PvP 정책이 보유 buff 정의의 피격 피해 비율을 합산한다. 없는 buff 정의는 건너뛴다.
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

// 피해량에 합산 percent를 정수 계산으로 적용한다. 입력 피해 0은 0을 유지하고 양수 결과는 1 이상으로 제한한다.
std::uint32_t LostArk::Server::CServerBuffRuntime::Scale_Damage(
	const std::uint32_t damage,
	const std::int32_t percent)
{
	if (0u == damage || 0 == percent)
		return damage;
	// 피해 감소가 커도 양수 피해를 회복량으로 바꾸지 않도록 최소 1로 제한한다.
	const std::int64_t scaled =
		static_cast<std::int64_t>(damage) * (100 + percent) / 100;
	if (scaled < 1)
		return 1u;
	return static_cast<std::uint32_t>((std::min<std::int64_t>)(
		scaled, static_cast<std::int64_t>((std::numeric_limits<std::uint32_t>::max)())));
}

// 피격과 방의 MVP tick이 공유하는 player ID 행 조회다. 없으면 기본 행을 생성한다.
// 반환 참조는 vector 요소이므로 이후 추가로 재할당될 수 있다.
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

// 타격/버프 경로가 사용할 월드 피해 대상 조건이다. 종속 보스는 주 보스와 같은 피해 대상 집합에 넣지 않는다.
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

// PlayerSkillSystem과 combat object가 허용한 타격을 전달한다. 여기서는 대상 참가 조건을 다시 확인한다.
// HP/무력화/카운터/부위 결과, MVP, 표시 event와 일반 몬스터의 밀림/사망을 연결한다.
// 실제 도형/반복 판정은 호출자가 맡으며 LANDED는 결과 처리됨을 뜻하고 HP 양수 피해를 보장하지 않는다.
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
	bool staggerBroken = false;
	bool damageReduced = false;
	if (WORLD_BOOTSTRAP_KIND::BOSS == target.eKind)
	{
		// 기존 ArmorPlates는 선택과 snapshot이 계속 소비하므로 방어를 먼저 한 번 계산한다.
		// CBossCombatRuntime에는 이미 정산한 HP 입력으로 전달해 typed 부위 감소를 이중 적용하지 않는다.
		// typed Parts가 정의되어 있으면 부위 내구도는 typed 경로가 변경하고 기존 plate에는 결과를 반영한다.
		const auto invulnerable = [&]()
		{
			if (hit.iRawDamage)
				PushDamageEvent(target.iNetEntityId, 0u, target.fPositionX, target.fPositionY, target.fPositionZ,
					true, outDamageEvents, hit.iSourcePlayerId, 0u, false, false,
					LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE);
			return SERVER_COMBAT_HIT_RESULT::ABSORBED;
		};
		if (target.bPatternInvulnerable || (hit.iStaggerMaximumDivisor &&
			CBossCombatRuntime::Has_Flag(target.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE)))
			return invulnerable();
		const bool hasLegacyArmor = !target.ArmorPlates.empty();
		const bool hasTypedParts = !target.BossCombat.Parts.empty();
		const bool requiresDestructionBomb = target.strArchetypeId == "BOSS_VALTAN" &&
			target.iOwnerBossNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID &&
			target.iPhase < 3u;
		const bool canDamageArmor = !requiresDestructionBomb || hit.bDestructionBombSource;
		BOSS_INCOMING_HIT incoming{};
		incoming.iSourcePlayerId = hit.iSourcePlayerId;
		incoming.iSkillId = hit.iSkillId;
		incoming.iRawDamage = hasLegacyArmor && !hit.bHealthDamagePreResolved ?
			CGameplayCatalog::Apply_Defense(
				hit.iRawDamage, SumIntactLegacyArmorDefense(target)) :
			hit.iRawDamage;
		// HP 또는 독립 무력화 피해를 계산하기 전에 갑옷 피해 감소를 한 번 정산한다.
		if (!hasLegacyArmor && !hit.bHealthDamagePreResolved)
		{
			std::uint32_t reduction = 0u;
			for (const auto& part : target.BossCombat.Parts)
				if (target.BossCombat.iAlivePartMask & part.iStateMask)
					reduction = (std::min)(100u, reduction + part.iDamageReductionPercent);
			const auto raw = incoming.iRawDamage;
			incoming.iRawDamage = static_cast<std::uint32_t>(static_cast<std::uint64_t>(raw) * (100u - reduction) / 100u);
			if (raw && !incoming.iRawDamage && reduction < 100u)
				incoming.iRawDamage = 1u;
		}
		SERVER_WORLD_ENTITY* staggerOwner = target.iKoukuItemStaggerMaximum ? &target : nullptr;
		bool marioReduction = target.iKoukuDamageReductionWindows != 0u;
		for (const auto& retained : target.KoukuRetainedLogicOwners)
			if (const auto owner = retained.lock())
			{
				if (!staggerOwner && owner->iKoukuItemStaggerMaximum)
					staggerOwner = owner.get();
				marioReduction |= owner->iKoukuDamageReductionWindows != 0u;
			}
		const auto maximum = staggerOwner ? staggerOwner->iKoukuItemStaggerMaximum : target.BossCombat.iStaggerMaximum;
		const auto amount = hit.bGuideSource || hit.bStaggerDisabled ? 0u : hit.iStaggerMaximumDivisor ?
			maximum / hit.iStaggerMaximumDivisor + (maximum % hit.iStaggerMaximumDivisor ? 1u : 0u) :
			Uses_RaidStaggerPolicy(target) ? incoming.iRawDamage / 1000u : hit.iStaggerDamage;
		incoming.iStaggerDamage = staggerOwner ? 0u : amount;
		std::uint32_t itemStaggerCredit = 0u;
		if (staggerOwner && amount && !CBossCombatRuntime::Has_Flag(target.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE))
		{
			itemStaggerCredit = (std::min)(amount, maximum - (std::min)(maximum, staggerOwner->iKoukuItemStaggerCredit));
			staggerOwner->iKoukuItemStaggerCredit += itemStaggerCredit;
		}
		if (marioReduction && incoming.iRawDamage)
		{
			incoming.iRawDamage = (std::max)(1u, incoming.iRawDamage / 1000u);
			damageReduced = true;
		}
		if (hit.bHealthDamageDisabled)
		{
			incoming.iRawDamage = 0u;
			damageReduced = false;
		}
		incoming.iPartDamage = hasTypedParts && !hit.bGuideSource && canDamageArmor ? hit.iPartDamage : 0u;
		incoming.bDestroyAllEligibleParts = requiresDestructionBomb && hit.bDestructionBombSource && !hit.bGuideSource;
		incoming.iCounterPower = hit.bGuideSource ? 0u : hit.iCounterPower;
		incoming.bCounterFromPrimarySlot = !hit.bGuideSource && hit.bCounterFromPrimarySlot;
		incoming.iServerTick = hit.iServerTick;
		incoming.bHealthDamagePreResolved = true;
		incoming.fSourceX = hit.fSourceX;
		incoming.fSourceZ = hit.fSourceZ;
		const BOSS_HIT_RESULT bossHit =
			CBossCombatRuntime::Apply_PlayerHit(target, incoming);
		damage = bossHit.iHealthDamage;
		PushDamageEvent(target.iNetEntityId, bossHit.iShieldDamage,
			target.fPositionX, target.fPositionY, target.fPositionZ, true, outDamageEvents,
			hit.iSourcePlayerId, 0u, false, false, LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB);
		staggerDealt = bossHit.iStaggerDamage + itemStaggerCredit;
		counterTriggered = bossHit.bCounterTriggered;
		staggerBroken = bossHit.bStaggerBroken;
		if (!hit.bGuideSource && LostArk::Shared::INVALID_PLAYER_ID != hit.iSourcePlayerId &&
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
			// 이번 타격으로 보스 HP가 처음 0이 되었으면 마무리 횟수를 기록한다.
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
		else if (!hit.bGuideSource && !hasTypedParts && canDamageArmor && target.bPatternGroggy &&
			ConsumeLegacyArmorDurability(target, damage))
		{
			// typed 부위가 없는 보스/fixture만 기존 내구도 감소를 사용한다. typed Parts가 있으면 이 분기로 들어오지 않는다.
			target.bPatternGroggy = false;
			target.bPendingArmorBreakReaction = true;
		}
	}
	else
	{
		damage = hit.bHealthDamageDisabled ? 0u : target.strSpawnGroupId == "cardmaze.targets" ? target.iCurrentHp :
			CGameplayCatalog::Apply_Defense(hit.iRawDamage, target.iDefense);
		target.iCurrentHp = damage >= target.iCurrentHp ?
			0u : target.iCurrentHp - damage;
	}
	// cardmaze.targets는 허용된 타격 한 번에 HP를 제거한다.
	// KILLED와 미로 기믹 처리는 유지하되 최대 HP가 피해 숫자로 반복 표시되지 않도록 일반 피해 event는 생략한다.
	if (target.strSpawnGroupId != "cardmaze.targets")
	{
		PushDamageEvent(
			target.iNetEntityId, damage,
			target.fPositionX, target.fPositionY, target.fPositionZ,
			true, outDamageEvents,
			hit.iSourcePlayerId, staggerDealt, counterTriggered, hit.bCritical,
			damageReduced ? (hit.bCritical ? LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL_DAMAGE_REDUCED :
				LostArk::Shared::DAMAGE_HIT_FLAG::DAMAGE_REDUCED) : LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL, staggerBroken);
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

// 살아 있고 전투 가능한 NORMAL 상태에만 광기를 더한다. 정수 게이지와 소수 잔여량을 분리해 작은 증가를 보존한다.
void LostArk::Server::CServerCombatHitRuntime::Add_MadnessGauge(
	SERVER_PLAYER& target, const double gain)
{
	using namespace LostArk::Shared;
	if (target.Is_Guide() || !target.iMaximumMadness || !target.iCurrentHp || !target.isCombatReady ||
		target.eMadnessForm != PLAYER_MADNESS_FORM::NORMAL ||
		!std::isfinite(gain) || gain <= 0.)
		return;
	const double total = target.iCurrentMadness + target.dMadnessRemainder + gain;
	const double clamped = (std::min)(total, static_cast<double>(target.iMaximumMadness));
	target.iCurrentMadness = static_cast<std::uint32_t>(clamped + 1e-9);
	target.dMadnessRemainder = (std::max)(0., clamped - target.iCurrentMadness);
}

namespace
{
	// 월드 피격 피해에 최대 ±10%의 대칭 정수 변동을 준다. target의 표본 sequence는 진행하지만 HP는 바꾸지 않는다.
	// 나머지 연산의 편향을 줄이려고 불완전한 앞 구간 표본은 버린 뒤 같은 폭으로 나눈다.
	std::uint32_t SampleIncomingDamage(LostArk::Server::SERVER_PLAYER& target,
		const std::uint32_t damage, const std::uint32_t tick)
	{
		if (damage == 0u)
			return 0u;
		// 정산된 피해를 중심으로 좌우 같은 폭의 정수 변동 구간을 만든다.
		// UINT32_MAX 근처에서는 양쪽 폭을 함께 줄여 한쪽만 잘리는 편향을 피한다.
		const std::uint32_t radius = (std::min)(damage / 10u,
			(std::numeric_limits<std::uint32_t>::max)() - damage);
		std::uint64_t state = ++target.iIncomingDamageSampleSerial * 0x9e3779b97f4a7c15ull ^
			(static_cast<std::uint64_t>(target.iNetEntityId) << 32u) ^ tick;
		if (radius == 0u)
			return damage;
		// Server의 tick/entity/타격 sequence로 재현 가능한 SplitMix64 입력을 만든다. 보안 난수 용도는 아니다.
		const auto next = [&state]()
		{
			std::uint64_t value = (state += 0x9e3779b97f4a7c15ull);
			value = (value ^ (value >> 30u)) * 0xbf58476d1ce4e5b9ull;
			value = (value ^ (value >> 27u)) * 0x94d049bb133111ebull;
			return value ^ (value >> 31u);
		};
		const std::uint64_t width = static_cast<std::uint64_t>(radius) * 2u + 1u;
		const std::uint64_t rejectedPrefix = (0ull - width) % width;
		std::uint64_t sample;
		do
		{
			sample = next();
		} while (sample < rejectedPrefix);
		return damage - radius + static_cast<std::uint32_t>(sample % width);
	}
}

// 동일 활성 경기, 유효한 다른 팀, 생존/참가 상태를 검사한다. 거리나 공격 도형의 승인은 포함하지 않는다.
bool LostArk::Server::CServerCombatHitRuntime::Is_EnemyPlayer(const SERVER_PLAYER& source, const SERVER_PLAYER& target) noexcept
{
	return source.iColosseumMatchId && source.iColosseumMatchId == target.iColosseumMatchId &&
		source.bColosseumCombatActive && target.bColosseumCombatActive &&
        source.bColosseumParticipant && target.bColosseumParticipant && source.isCombatReady && target.isCombatReady && source.iColosseumTeam < 2u &&
		target.iColosseumTeam < 2u && source.iColosseumTeam != target.iColosseumTeam &&
		source.iCurrentHp && target.iCurrentHp && source.iPlayerId != target.iPlayerId;
}

// 기존 player hit 입력을 월드→플레이어 공용 경로로 연결한다. HP 채널이 금지된 입력은 거부한다.
// 이번 호출이 추가한 event에만 공격자 ID/치명타 표시를 적용하고 사망하면 killer ID를 기록한다.
LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_PlayerToPlayer(const SERVER_PLAYER& source, SERVER_PLAYER& target,
	const SERVER_PLAYER_TO_WORLD_HIT& hit, const CGameplayCatalog& catalog,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& events)
{
	if (!Is_EnemyPlayer(source, target) || hit.bHealthDamageDisabled)
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	SERVER_WORLD_TO_PLAYER_HIT incoming{};
	incoming.iRawDamage = hit.iRawDamage;
	incoming.fSourceX = hit.fSourceX;
	incoming.fSourceZ = hit.fSourceZ;
	incoming.fPushRangeM = hit.fPushRangeM;
	incoming.iPushMs = hit.iPushMs;
	incoming.iServerTick = hit.iServerTick;
	const std::size_t first = events.size();
	const SERVER_COLOSSEUM_COMBAT_CONTEXT context{ LostArk::Shared::WORLD_ID::COLOSSEUM, source.iColosseumMatchId, true, {} };
    const SERVER_COLOSSEUM_RESOLVED_DAMAGE resolved{ &context, &source, hit.bCritical };
	const auto result = Apply_WorldToPlayer(target, incoming, catalog, events, &resolved);
	for (std::size_t index = first; index < events.size(); ++index)
	{
		events[index].iSourcePlayerId = source.iPlayerId;
		if (hit.bCritical && events[index].eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL)
			events[index].eHitFlag = LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL;
	}
	if (result == SERVER_COMBAT_HIT_RESULT::KILLED)
		target.iColosseumKillerId = source.iPlayerId;
	return result;
}

// Brain/환경/contact와 PvP adapter가 호출한다. 대상/부착 소유를 검사한 뒤 보호 효과를 순서대로 처리한다.
// 일반 피해는 방어와 변동을 거쳐 보호막/사망 방지/HP에 적용하고, lethal은 현재 HP 전체를 제거한다.
// 피해 이후 event/광기/내구도/사망 상태를 갱신하며 살아 있으면 기존 스킬 반응 경로에 밀기/넘어짐을 맡긴다.
LostArk::Server::SERVER_COMBAT_HIT_RESULT
LostArk::Server::CServerCombatHitRuntime::Apply_WorldToPlayer(
	SERVER_PLAYER& target,
	const SERVER_WORLD_TO_PLAYER_HIT& hit,
	const CGameplayCatalog& catalog,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
    const SERVER_COLOSSEUM_RESOLVED_DAMAGE* pPvPResolved)
{
	using namespace LostArk::Shared;
    if (pPvPResolved && (!pPvPResolved->pContext || !pPvPResolved->pSource ||
        !Is_ColosseumOpponent(*pPvPResolved->pContext, *pPvPResolved->pSource, target)))
        return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
    const PLAYER_ID damageSource = pPvPResolved ? pPvPResolved->pSource->iPlayerId : INVALID_PLAYER_ID;
	const bool lethal = hit.bEncounterWipe || hit.bInstantDeath;
	if (!lethal && hit.bUsePushDirection && (!std::isfinite(hit.fPushDirectionX) || !std::isfinite(hit.fPushDirectionZ) ||
		hit.fPushDirectionX * hit.fPushDirectionX + hit.fPushDirectionZ * hit.fPushDirectionZ < .000001f))
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	const bool ownedCapture = Is_CurrentBossHandCapture(target, hit.iCaptureOwnerId,
		hit.iCapturePatternSequence, hit.iServerTick);
	if ((target.Is_Guide() && hit.bEncounterWipe) || 0u == target.iCurrentHp || (!lethal && !ownedCapture && !target.isCombatReady) ||
		PLAYER_ACTION_STATE::DEAD == target.eAction ||
		(!lethal && (PLAYER_ACTION_STATE::FALLING == target.eAction ||
		(PLAYER_ACTION_STATE::GRABBED == target.eAction && !ownedCapture))))
	{
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	}
	// 즉사와 전멸 판정은 아래의 개인 생존 효과를 우회한다.
	if (!lethal && target.Has_TimeStop(hit.iServerTick))
		return SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED;
	if (!lethal && target.Has_HolyCharmProtection(hit.iServerTick))
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;
	const bool estherGuarded = !lethal && hit.iServerTick < target.iEstherGuardEndTick;
	const bool ronaunBlocked = !lethal && target.bRonaunGuard && hit.bEstherGuardBlockable;
	if (ronaunBlocked)
		target.bRonaunGuard = false;
	if ((estherGuarded || ronaunBlocked) && hit.bEstherGuardBlockable)
	{
		// 다른 무적이 겹쳐도 빙고에서 사용하는 보호 표시의 접촉/pulse tick을 함께 남긴다.
		target.iInvulnerabilityZoneContactTick = hit.iServerTick;
		target.iInvulnerabilityZonePulseTick = hit.iServerTick;
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;
	}
	if (!lethal && hit.iServerTick < target.iInvulnerableEndTick)
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;
	if (!lethal && !hit.bIgnoreCounter &&
		CPlayerSkillSystem::Try_Counter(target, catalog, hit.iServerTick))
		return SERVER_COMBAT_HIT_RESULT::ABSORBED;

	const PLAYER_RUNTIME_PROFILE* playerProfile =
		catalog.Find_Player(target.eCharacterClass);
	const std::uint32_t mitigated = hit.bIgnoreDefense ? hit.iRawDamage :
		CGameplayCatalog::Apply_Defense(
			hit.iRawDamage, nullptr == playerProfile ? 0u : playerProfile->iDefense);
	// 일반 월드 피해에는 보유 buff와 에스더 보호의 피해 증감률을 적용한다.
	// PvP 입력은 상위 경기 정책이 buff/피해 변동/치명타/방어를 이미 계산한 값이다.
    // pPvPResolved가 없으면 기존 월드 피해의 방어/피해 변동 경로를 사용한다.
	const std::uint32_t damage = lethal ? target.iCurrentHp : pPvPResolved ? hit.iRawDamage : SampleIncomingDamage(target,
		CServerBuffRuntime::Scale_Damage(mitigated,
			CServerBuffRuntime::Damage_TakenPercent(catalog, target.ActiveBuffs) +
				(estherGuarded ? target.iEstherGuardDamageTakenPercent : 0)), hit.iServerTick);
	// 일반 피해는 보호막을 먼저 줄이고 남은 양만 HP로 전달한다. lethal은 보호막을 우회한다.
	std::uint32_t throughShield = damage;
	if (!lethal && 0u != target.iShield && 0u != throughShield)
	{
		const std::uint32_t absorbed = (std::min)(target.iShield, throughShield);
		target.iShield -= absorbed;
		throughShield -= absorbed;
		PushDamageEvent(target.iNetEntityId, absorbed,
			target.fPositionX, target.fPositionY, target.fPositionZ, false, outDamageEvents,
			damageSource, 0u, false, false, DAMAGE_HIT_FLAG::ABSORB, false, hit.eMarioHitSource);
	}
	if (!lethal && throughShield >= target.iCurrentHp &&
		0u != Death_DenyInvulnerableMs(catalog, target))
	{
		// 사망 방지 buff가 있으면 HP 1을 남기고 정해진 시간 무적을 연다.
		// 사망 방지 속성을 가진 buff를 소비하여 같은 buff로 다음 치명타를 반복 막지 않게 한다.
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
	// 광기는 방어/보호막/사망 방지 이후 실제 감소한 HP만으로 계산한다.
	// 적용 비율은 방이 설정한 Server 상태를 사용한다.
	if (target.iMadnessDamageGainPercent && target.iMaximumHp)
		Add_MadnessGauge(target, static_cast<double>(hpBefore - target.iCurrentHp) *
			target.iMaximumMadness / target.iMaximumHp * target.iMadnessDamageGainPercent / 100.);
	PushDamageEvent(
		target.iNetEntityId, hpBefore - target.iCurrentHp,
		target.fPositionX, target.fPositionY, target.fPositionZ,
		false, outDamageEvents, damageSource, 0u, false, pPvPResolved && pPvPResolved->bCritical, DAMAGE_HIT_FLAG::NORMAL, false, hit.eMarioHitSource);
	// 실제 HP를 잃은 인간 플레이어만 장비 내구도를 소모한다. 안내 NPC에는 적용하지 않는다.
	if (target.Is_Human() && hpBefore > target.iCurrentHp)
		target.Wear_Durability(5u);
	if (0u == target.iCurrentHp)
	{
        if (pPvPResolved)
            target.iColosseumKillerId = pPvPResolved->pSource->iPlayerId;
		target.iKoukuBingoLineProtectionEndTick = target.iEstherZoneProtectionEndTick = 0u;
		target.iTimeStopEndTick = 0u;
		target.iHolyCharmProtectionEndTick = 0u;
		if (lethal)
		{
			target.iShield = 0u;
			target.iInvulnerableEndTick = 0u;
			target.ActiveBuffs.clear();
			target.Clear_Attachment();
		}
		target.bRonaunGuard = false;
		target.iRonaunGrantTick = 0u;
		target.iEstherGuardEndTick = 0u;
		target.eAction = PLAYER_ACTION_STATE::DEAD;
		target.iCurrentSkillId = INVALID_SKILL_ID;
		target.Clear_SkillTarget();
		target.iActionStartTick = 0u == hit.iServerTick ? 1u : hit.iServerTick;
		target.hasMoveGoal = false;
		target.MovePath.clear();
		return SERVER_COMBAT_HIT_RESULT::KILLED;
	}
	if (estherGuarded)
		return SERVER_COMBAT_HIT_RESULT::LANDED;
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
