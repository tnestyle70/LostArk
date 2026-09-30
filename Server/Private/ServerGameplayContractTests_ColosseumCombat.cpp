#include "ServerGameplayContractTests_Runner.h"
#include "ColosseumCombatPolicy.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>

using namespace LostArk::Server;
using namespace LostArk::Shared;
using namespace ServerGameplayContractDetail;

namespace
{
    SERVER_PLAYER Player(const std::uint32_t id, const std::uint8_t team)
    {
        SERVER_PLAYER player{};
        player.iPlayerId = id; player.iNetEntityId = id + 10000u;
        player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        player.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
        player.iCurrentHp = player.iMaximumHp = 600000u;
        player.iCurrentResource = player.iMaximumResource = 10000u;
        player.iColosseumMatchId = 77u; player.iColosseumTeam = team;
        player.iColosseumDamageReferenceHp = 600000u;
        player.bColosseumParticipant = true; player.isCombatReady = true;
        return player;
    }

    struct FIXTURE final
    {
        explicit FIXTURE(const CGameplayCatalog& source, const int path)
            : catalog(std::make_unique<CGameplayCatalog>(source))
        {
            skill = const_cast<PLAYER_SKILL_DEFINITION*>(catalog->Find_Skill(34040u));
            auto* profile = const_cast<PLAYER_RUNTIME_PROFILE*>(catalog->Find_Player(CHARACTER_CLASS_ID::LANCE_MASTER));
            damage = skill ? const_cast<CGameplayCatalog::DAMAGE_PROFILE*>(catalog->Find_DamageProfile(skill->strDamageProfileId)) : nullptr;
            if (!skill || !profile || !damage) return;
            profile->iAttackPower = 10003u; profile->iCriticalChancePercent = 0u; profile->iDefense = 0u;
            damage->iAttackCoefficientBp = 10000u; damage->iDamageAddend = 0u;
            damage->iDamageSpreadPercent = 0u; damage->iBossHealthBarDamage = 0u;
            skill->RootMotion.clear(); skill->ComboStages.clear(); skill->Hits.clear(); skill->Projectiles.clear();
            skill->eSkillKind = PLAYER_SKILL_KIND::ACTIVE; skill->fMovementDistance = 0.f;
            skill->eRequiredStance = PLAYER_STANCE_ID::NONE;
            skill->iActionDurationMs = 600u; skill->iHitTimeMs = 100u;
            skill->iResourceCost = skill->iIdentityCost = 0u;
            skill->iStaggerDamage = skill->iCounterPower = skill->iPartDamage = 0u;
            skill->fMaximumRange = 4.f; skill->strInputSlot = "Q";
            PLAYER_SKILL_HIT shape{};
            shape.iTimeMs = 100u; shape.iRepeatCount = 3u; shape.iRepeatMs = 100u;
            shape.iAreaType = 1u; shape.fRange = 4.f; shape.fHeight = 2.f;
            shape.iResultKind = 1u;
            if (path == 0) skill->Hits.push_back(shape);
            else if (path < 3)
            {
                PLAYER_SKILL_PROJECTILE projectile{};
                projectile.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
                projectile.eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
                projectile.iLifeMs = 600u; projectile.fRadius = 4.f;
                PLAYER_PROJECTILE_HIT hit{}; hit.Hit = shape; hit.isContact = path == 2;
                projectile.Hits.push_back(hit); skill->Projectiles.push_back(projectile);
            }
            ready = true;
        }
        std::unique_ptr<CGameplayCatalog> catalog;
        PLAYER_SKILL_DEFINITION* skill = nullptr;
        CGameplayCatalog::DAMAGE_PROFILE* damage = nullptr;
        bool ready = false;
    };

    struct CAST final
    {
        SERVER_PLAYER caster = Player(100u, 0u), enemy = Player(101u, 1u), ally = Player(102u, 0u);
        SERVER_PLAYER spectator = Player(103u, 1u), otherMatch = Player(104u, 1u), high = Player(105u, 1u);
        std::array<SERVER_PLAYER*, 6> players{ &caster, &enemy, &ally, &spectator, &otherMatch, &high };
        SERVER_COLOSSEUM_COMBAT_CONTEXT context{WORLD_ID::COLOSSEUM, 77u, true, players};
        CPlayerSkillSystem runtime;
        std::vector<SERVER_WORLD_ENTITY> world;
        std::vector<DAMAGE_EVENT> events;
        bool started = false;
        CAST()
        {
            enemy.fPositionZ = ally.fPositionZ = spectator.fPositionZ = otherMatch.fPositionZ = high.fPositionZ = 1.f;
            spectator.bColosseumParticipant = false; otherMatch.iColosseumMatchId = 78u;
            high.fPositionY = 10.f;
        }
        void Start(const FIXTURE& fixture)
        {
            caster.eCharacterClass = fixture.skill->eCharacterClass;
            C2S_USE_SKILL command{};
            command.iSkillId = fixture.skill->iSkillId; command.iClientSequence = 1u; command.fAimZ = 3.f;
            started = runtime.Try_Start(caster, command, *fixture.catalog, 1u);
        }
        void Step(const FIXTURE& fixture, const std::uint32_t tick, const bool withContext = true)
        {
            if (started) runtime.Update(caster, world, *fixture.catalog, nullptr, nullptr,
                1.f / 30.f, tick, events, withContext ? &context : nullptr);
        }
        void Finish(const FIXTURE& fixture, const bool withContext = true)
        {
            Start(fixture);
            for (std::uint32_t tick = 2u; tick < 35u; ++tick) Step(fixture, tick, withContext);
        }
    };

    bool Near(const float left, const float right) { return std::fabs(left - right) < .001f; }

    void Contracts(TESTS& tests, const CGameplayCatalog& source)
    {
        for (int path = 0; path < 4; ++path)
        {
            FIXTURE fixture(source, path);
            tests.Require(fixture.ready, "PvP fixture resolves published player/skill/damage definitions");
            if (!fixture.ready) continue;
            CAST cast; cast.Finish(fixture);
            tests.Require(cast.started && cast.enemy.iMaximumHp - cast.enemy.iCurrentHp == 10003u &&
                cast.events.size() == (path == 3 ? 1u : 3u) && cast.enemy.iIncomingDamageSampleSerial == 0u,
                "Caster, timed projectile, contact projectile and fallback preserve one cast damage without a second incoming roll");
            tests.Require(cast.caster.iCurrentHp == 600000u && cast.ally.iCurrentHp == 600000u &&
                cast.spectator.iCurrentHp == 600000u && cast.otherMatch.iCurrentHp == 600000u && cast.high.iCurrentHp == 600000u &&
                std::all_of(cast.events.begin(), cast.events.end(), [&](const auto& event) {
                    return event.iSourcePlayerId == cast.caster.iPlayerId && event.iTargetNetEntityId == cast.enemy.iNetEntityId && !event.isOutgoing;
                }), "PvP admits only opposing participating player bodies in this match and preserves source attribution");

            for (int guard = 0; guard != 9; ++guard)
            {
                CAST blocked;
                if (guard == 1) blocked.context.eWorldId = WORLD_ID::VALTAN_ARENA;
                if (guard == 2) blocked.context.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
                if (guard == 3) blocked.context.eWorldId = WORLD_ID::BERN;
                if (guard == 4) blocked.context.bMatchActive = false;
                if (guard == 5) blocked.context.iMatchId = 0u;
                if (guard == 6) blocked.caster.bColosseumParticipant = false;
                if (guard == 7) blocked.enemy.iColosseumTeam = 0u;
                if (guard == 8) blocked.enemy.eAction = PLAYER_ACTION_STATE::DEAD;
                blocked.Finish(fixture, guard != 0);
                tests.Require(blocked.started && blocked.enemy.iCurrentHp == 600000u && blocked.events.empty() &&
                    blocked.enemy.fKnockbackRemainingSeconds == 0.f,
                    "Null, raid/world, inactive, zero match, spectator, same-team and dead-target contexts cannot deal PvP damage or CC");
            }
            for (const bool artist : {false, true})
            {
                fixture.damage->iBossHealthBarDamage = 1u;
                fixture.skill->strInputSlot = artist ? "T" : "Q";
                if (artist) fixture.skill->eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
                CAST bars;
                bars.enemy.iColosseumDamageReferenceHp = 600001u;
                bars.enemy.iMaximumHp = bars.enemy.iCurrentHp = 150001u;
                bars.Finish(fixture);
                tests.Require(bars.started && 150001u - bars.enemy.iCurrentHp == (artist ? 750u : 3751u),
                    "40-bar HP keeps ceil(original full HP/160) cast damage, then Artist T scales once before sub-hit sharing");
            }
            fixture.skill->eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            fixture.skill->strInputSlot = "Q";
            for (const auto reference : {741285439u, 169u})
            {
                fixture.damage->iBossHealthBarDamage = reference == 169u ? 15u : 1u;
                const auto expected = reference == 169u ? 16u : 4633034u;
                CAST quarter;
                quarter.enemy.iColosseumDamageReferenceHp = reference;
                const auto maximum = reference / 4u + (reference % 4u != 0u ? 1u : 0u);
                quarter.enemy.iMaximumHp = quarter.enemy.iCurrentHp = maximum;
                quarter.Finish(fixture);
                tests.Require(quarter.started && maximum - quarter.enemy.iCurrentHp == expected,
                    "Caster/contact/timed/fallback retain original full-HP damage independently of quarter-HP rounding");
            }
            CAST missingReference; missingReference.enemy.iColosseumDamageReferenceHp = 0u;
            missingReference.Finish(fixture);
            tests.Require(missingReference.started && missingReference.enemy.iCurrentHp == 600000u && missingReference.events.empty(),
                "Bar damage rejects a missing arena reference instead of reconstructing it from rounded current HP");
        }
        for (int path = 0; path < 3; ++path)
        {
            FIXTURE fixture(source, path);
            if (!fixture.ready) continue;
            auto& shape = path ? fixture.skill->Projectiles.front().Hits.front().Hit : fixture.skill->Hits.front();
            shape.iRepeatCount = 1u; shape.iDurationMs = 350u; shape.iMaxTargets = 1u;
            CAST cap; cap.high.fPositionY = 0.f; cap.high.fPositionZ = 2.f; cap.Finish(fixture);
            tests.Require(cap.started && cap.events.size() == 1u && cap.enemy.iCurrentHp == 589997u && cap.high.iCurrentHp == 600000u,
                "Authored max-target limit and persistent hit ledger prevent repeated window/contact damage");

            CAST stopped; stopped.Start(fixture);
            for (std::uint32_t tick = 2u; tick < 35u; ++tick)
            {
                if (tick == 3u) stopped.context.bMatchActive = false;
                stopped.Step(fixture, tick);
            }
            tests.Require(stopped.started && stopped.events.empty() && stopped.enemy.iCurrentHp == 600000u,
                "Closing the match context stops already spawned projectiles and pending caster hits");
        }
        for (const char* slot : {"Q", "V", "ALT_V"})
        {
            FIXTURE fixture(source, 0); if (!fixture.ready) continue;
            fixture.skill->strInputSlot = slot;
            auto& hit = fixture.skill->Hits.front();
            hit.iRepeatCount = 1u; hit.fPushRange = -2.f; hit.iPushMs = 500u;
            CAST cast; cast.Finish(fixture);
            const bool strong = fixture.skill->strInputSlot == "ALT_V", soft = fixture.skill->strInputSlot == "V";
            const float range = strong ? 16.f : soft ? 5.1f : 2.f;
            const float seconds = strong ? 1.5f : soft ? 2.161f : .5f;
            tests.Require(cast.started && Near(cast.enemy.fKnockbackSpeed * cast.enemy.fKnockbackRemainingSeconds, range) &&
                Near(cast.enemy.fKnockbackRemainingSeconds, seconds) && !cast.enemy.bKnockbackCanLeaveArena &&
                cast.enemy.bKnockbackBallistic == (strong || soft) &&
                (strong || soft ? cast.enemy.fKnockbackDirectionZ > 0.f : cast.enemy.fKnockbackDirectionZ < 0.f),
                "PvP preserves authored pulls, uses ALT_V 16m/1500ms and V 5.1m/2161ms, and never permits arena escape");
        }
        {
            FIXTURE fixture(source, 0);
            auto* warlord = const_cast<PLAYER_RUNTIME_PROFILE*>(fixture.catalog->Find_Player(CHARACTER_CLASS_ID::WARLORD));
            auto* actualSkill = const_cast<PLAYER_SKILL_DEFINITION*>(fixture.catalog->Find_Skill(17170u));
            auto* damage = actualSkill ? const_cast<CGameplayCatalog::DAMAGE_PROFILE*>(fixture.catalog->Find_DamageProfile(actualSkill->strDamageProfileId)) : nullptr;
            tests.Require(warlord && actualSkill && damage, "Published Warlord V resolves its actual collider and enemy stun buff");
            if (warlord && actualSkill && damage)
            {
                fixture.skill = actualSkill;
                warlord->iAttackPower = 1000u; warlord->iCriticalChancePercent = 0u;
                damage->iAttackCoefficientBp = 10000u; damage->iDamageAddend = 0u;
                damage->iDamageSpreadPercent = damage->iBossHealthBarDamage = 0u;
                for (const bool invulnerable : {false, true})
                {
                    CAST cast; if (invulnerable) cast.enemy.iInvulnerableEndTick = 1000u;
                    cast.Start(fixture);
                    for (std::uint32_t tick = 2u; tick < 150u; ++tick) cast.Step(fixture, tick);
                    const auto buff = std::find_if(cast.enemy.ActiveBuffs.begin(), cast.enemy.ActiveBuffs.end(),
                        [](const auto& value) { return value.iBuffId == 171705u; });
                    tests.Require(cast.started && cast.ally.ActiveBuffs.empty() &&
                        (invulnerable ? buff == cast.enemy.ActiveBuffs.end() && cast.enemy.iKnockdownEndTick == 0u :
                            buff != cast.enemy.ActiveBuffs.end() && cast.enemy.iKnockdownEndTick >= cast.enemy.iActionStartTick + 120u),
                        "Actual Warlord V grants enemy stun only on an admitted collider hit and retains four seconds alongside arena push");
                }
            }
        }
        {
            FIXTURE fixture(source, 0); if (!fixture.ready) return;
            fixture.skill->Hits.front().iRepeatCount = 1u;
            auto* profile = const_cast<PLAYER_RUNTIME_PROFILE*>(fixture.catalog->Find_Player(CHARACTER_CLASS_ID::ARTIST));
            tests.Require(profile != nullptr, "Artist T fixture resolves the published Artist profile");
            if (profile)
            {
                profile->iAttackPower = 10003u; profile->iCriticalChancePercent = 0u;
                fixture.skill->eCharacterClass = CHARACTER_CLASS_ID::ARTIST; fixture.skill->strInputSlot = "T";
                CAST cast; cast.Finish(fixture);
                // Whole-cast integer scaling truncates 10003 / 5 to 2000 HP.
                tests.Require(cast.started && cast.enemy.iCurrentHp == 598000u &&
                    cast.events.size() == 1u && cast.events.front().iAmount == 2000u,
                    "Raw Artist T PvP damage is one fifth while global damage and skill profiles remain unchanged");
            }
            fixture.skill->eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; fixture.skill->strInputSlot = "Q";
            for (int protection = 0; protection < 3; ++protection)
            {
                CAST cast;
                if (protection == 0) cast.enemy.iShield = 10003u;
                if (protection == 1) cast.enemy.iInvulnerableEndTick = 100u;
                if (protection == 2) cast.enemy.iTimeStopEndTick = 100u;
                cast.Finish(fixture);
                tests.Require(cast.started && cast.enemy.iCurrentHp == 600000u &&
                    (protection != 0 || cast.enemy.iShield == 0u),
                    "Arena damage shares existing shield, invulnerability and time-stop protection");
            }
            CAST lethal; lethal.enemy.iCurrentHp = 10u; lethal.Finish(fixture);
            tests.Require(lethal.started && lethal.enemy.iCurrentHp == 0u && lethal.enemy.eAction == PLAYER_ACTION_STATE::DEAD &&
                lethal.events.size() == 1u && lethal.events.front().iAmount == 10u,
                "Arena lethal damage commits existing death and reports only HP actually removed");
        }
        // Identical world hit inputs must retain the existing incoming variation,
        // HP/CC result and attribution even when arena metadata is present on a player.
        for (const std::uint32_t tick : {1u, 30u, 1000u})
        {
            auto raid = Player(401u, 0u), withMetadata = raid;
            raid.iColosseumMatchId = 0u; raid.iColosseumTeam = 255u; raid.bColosseumParticipant = false;
            SERVER_WORLD_TO_PLAYER_HIT incoming{};
            incoming.iRawDamage = 10003u; incoming.iServerTick = tick;
            incoming.fSourceZ = -1.f; incoming.fPushRangeM = 3.f; incoming.iPushMs = 500u;
            std::vector<DAMAGE_EVENT> first, second;
            const auto a = CServerCombatHitRuntime::Apply_WorldToPlayer(raid, incoming, source, first);
            const auto b = CServerCombatHitRuntime::Apply_WorldToPlayer(withMetadata, incoming, source, second, nullptr);
            tests.Require(a == b && raid.iCurrentHp == withMetadata.iCurrentHp && raid.iIncomingDamageSampleSerial == 1u &&
                withMetadata.iIncomingDamageSampleSerial == 1u && Near(raid.fKnockbackSpeed, withMetadata.fKnockbackSpeed) &&
                Near(raid.fKnockbackRemainingSeconds, withMetadata.fKnockbackRemainingSeconds) && first.size() == second.size() &&
                !first.empty() && first.front().iAmount == second.front().iAmount && first.front().iSourcePlayerId == INVALID_PLAYER_ID,
                "Default/null arena option leaves raid incoming damage, variation, CC and source attribution identical");
        }
    }
}

int CServerGameplayContractRunner::Run_ColosseumCombat()
{
    int result = 1;
    const auto execute = [](void* output)
    {
        std::cout << std::unitbuf;
        TESTS tests; auto catalog = std::make_unique<CGameplayCatalog>();
        if (!catalog->Load()) { std::cout << catalog->Get_Status() << '\n'; return; }
        Contracts(tests, *catalog);
        std::cout << "failures : " << tests.failures << '\n';
        *static_cast<int*>(output) = tests.failures == 0 ? 0 : 1;
    };
    return Run_WithContractWorkerStack(execute, &result) ? result : 1;
}
