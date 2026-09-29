#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "ItemCatalog.h"
#include "ServerCombatHitRuntime.h"
#include "KoukuSaydonLogicRuntime.h"
#include "WorldBootstrap.h"

#include <algorithm>
#include <memory>
#include <vector>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_BattleItemsOnly()
{
    TESTS tests;
    auto room = std::make_unique<CGameRoom>(WORLD_ID::BERN);
    tests.Require(room->Is_Ready(), "Battle items load published catalogs in a real room");
    if (!room->Is_Ready()) { std::cout << room->Get_Status() << '\n'; return 1; }
    std::vector<std::shared_ptr<CClientSession>> sessions;
    for (unsigned id = 1; id <= 3; ++id)
    {
        auto& player = room->m_Players[id];
        player.iPlayerId = id; player.iNetEntityId = 100u + id; player.iSessionId = 9000u + id;
        player.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
        player.iCurrentHp = player.iMaximumHp = 10000u;
        player.isCombatReady = true; player.fPositionX = static_cast<float>(id);
        room->m_PlayerIdBySessionId[player.iSessionId] = id;
        auto session = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true);
        room->m_Sessions[player.iSessionId] = session;
        sessions.push_back(std::move(session));
    }
    room->m_PartyIdByPlayerId[1u] = room->m_PartyIdByPlayerId[2u] = 1u;
    room->m_PartyMembersByPartyId[1u] = {1u, 2u};
    auto& caster = room->m_Players.at(1u);
    auto& ally = room->m_Players.at(2u);
    for (const char* id : {"BATTLE_DESTRUCTION_BOMB", "BATTLE_WHIRLWIND_GRENADE", "BATTLE_HOLY_CHARM", "BATTLE_TIME_STOP_POTION"})
        tests.Require(room->Grant_Item(caster, id, 5u), "Every battle item can enter the authoritative inventory");
    const auto quantity = [&](const char* id) {
        const auto found = std::find_if(caster.Inventory.begin(), caster.Inventory.end(),
            [&](const auto& item) { return item.strItemId == id; });
        return found == caster.Inventory.end() ? 0u : found->iQuantity;
    };
    room->m_iServerTick = 100u;
    C2S_USE_ITEM charm; charm.iRequestSequence = 1u; charm.strItemId = "BATTLE_HOLY_CHARM";
    charm.iTargetPlayerNetEntityId = 103u;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 5u, "Holy charm rejects another party without consuming an item");
    ally.eAction = PLAYER_ACTION_STATE::FEAR; ally.iFearEndTick = 500u;
    charm.iRequestSequence = 2u; charm.iTargetPlayerNetEntityId = ally.iNetEntityId;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 4u && ally.eAction != PLAYER_ACTION_STATE::FEAR &&
        ally.Has_HolyCharmProtection(101u) && !ally.Has_HolyCharmProtection(191u),
        "Holy charm immediately cleanses its chosen ally and grants exactly three seconds");
    room->Handle_UseItem(caster.iSessionId, charm);
    ++charm.iRequestSequence;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 4u, "Duplicate and cooldown uses cannot spend twice");

    std::vector<DAMAGE_EVENT> damageEvents;
    SERVER_WORLD_TO_PLAYER_HIT lethal;
    lethal.iRawDamage = 100000u; lethal.iServerTick = 102u;
    lethal.bIgnoreDefense = lethal.bIgnoreCounter = true;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(ally, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(ally.iCurrentHp == 10000u && damageEvents.empty(), "Holy protection blocks lethal collision damage");
    auto expiredAlly = ally; lethal.iServerTick = 191u;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(expiredAlly, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(!expiredAlly.iCurrentHp, "Holy protection ends at the authoritative expiry tick");

    C2S_USE_ITEM stop; stop.iRequestSequence = 4u; stop.strItemId = "BATTLE_TIME_STOP_POTION";
    room->Handle_UseItem(caster.iSessionId, stop);
    tests.Require(quantity("BATTLE_TIME_STOP_POTION") == 4u && caster.Has_TimeStop(101u) && !caster.Has_TimeStop(191u),
        "Time stop consumes one item and protects only its caster for three seconds");
    damageEvents.clear(); lethal.iServerTick = 102u;
    const auto avoided = CServerCombatHitRuntime::Apply_WorldToPlayer(caster, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(avoided == SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED && caster.iCurrentHp == 10000u && damageEvents.empty(),
        "Time stop removes its caster from lethal collision hit admission");
    auto stoppedWipe = caster; auto protectedWipe = ally; lethal.bEncounterWipe = true;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(stoppedWipe, lethal, room->m_GameplayCatalog, damageEvents);
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(protectedWipe, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(!stoppedWipe.iCurrentHp && !protectedWipe.iCurrentHp,
        "Explicit encounter failure wipes bypass both battle-item protections");

    const auto* whirlwind = room->m_ItemCatalog.Find_Item("BATTLE_WHIRLWIND_GRENADE");
    tests.Require(whirlwind && whirlwind->BattleUse.iDamageRatePercent == 0u,
        "Whirlwind catalog gives no HP damage");
    if (whirlwind)
    {
        caster.iTimeStopEndTick = 0u;
        caster.fPositionX = caster.fPositionY = caster.fPositionZ = 0.f;
        auto entities = std::make_unique<std::vector<SERVER_WORLD_ENTITY>>(1u);
        auto& boss = entities->front();
        boss.iNetEntityId = 500u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strArchetypeId = "BOSS_VALTAN";
        boss.iCurrentHp = boss.iMaximumHp = 100000u;
        boss.fPositionZ = 2.f; boss.fCollisionRadius = 1.f;
        boss.BossCombat.iStaggerMaximum = 1000u;
        CCombatObjectRuntime runtime;
        for (unsigned throwIndex = 0; throwIndex < 3u; ++throwIndex)
        {
            auto transaction = runtime.Begin_Transaction(); std::string status;
            const auto tick = 200u + throwIndex * 40u;
            const bool staged = runtime.Stage_BattleItemProjectile(transaction, caster, whirlwind->BattleUse,
                0.f, 0.f, 4.f, room->m_GameplayCatalog, tick, status);
            tests.Require(staged && runtime.Commit(std::move(transaction)), "Whirlwind stages a real replicated projectile");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
            std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
            std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
            runtime.Drain_Lifecycle(spawned, pulses, despawned);
            tests.Require(spawned.size() == 1u && spawned.front().strClientVisualId == "battle.item.whirlwind_grenade",
                "Observers receive the canonical flight identity");
            const auto previous = boss.BossCombat.iStaggerCurrent;
            damageEvents.clear();
            for (unsigned step = 1; step <= 35u && !runtime.Get_LiveObjects().empty(); ++step)
                runtime.Update(room->m_Players, *entities, room->m_GameplayCatalog, 1.f / 30.f, tick + step, damageEvents);
            runtime.Drain_Lifecycle(spawned, pulses, despawned);
            tests.Require(runtime.Get_LiveObjects().empty() && pulses.size() == 1u && despawned.size() == 1u &&
                pulses.front().strHitId == "battle.item.impact", "Bomb contact emits one impact then removes its projectile");
            tests.Require(boss.iCurrentHp == 100000u && boss.BossCombat.iStaggerCurrent == (std::min)(1000u, previous + 334u),
                "Each whirlwind contributes one third of maximum stagger with zero HP loss");
            tests.Require(std::all_of(damageEvents.begin(), damageEvents.end(), [](const auto& event) { return event.iAmount == 0u; }),
                "Whirlwind combat events never report fabricated HP damage");
        }
        tests.Require(boss.BossCombat.iStaggerCurrent == 1000u, "Three whirlwinds fill a non-divisible stagger maximum exactly");
    }
    {
        auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
        boss->iNetEntityId = 600u; boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss->strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
        boss->iCurrentHp = boss->iMaximumHp = 100000u;
        boss->strPatternId = "battle.item.stagger.window"; boss->iPatternSequence = 1u;
        BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = boss->strPatternId;
        BOSS_PATTERN_LOGIC_WINDOW window;
        window.strWindowId = "stagger"; window.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
        window.iDurationMs = 5000u; window.iThreshold = 1000u; window.bEndsPatternOnSuccess = true;
        pattern.LogicWindows.push_back(window);
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 500u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 500u, damageEvents, output);
        tests.Require(boss->iKoukuItemStaggerMaximum == 1000u,
            "Kouku stagger window admits its own full threshold for item credit");
        SERVER_PLAYER_TO_WORLD_HIT hit;
        hit.iSourcePlayerId = caster.iPlayerId; hit.iSkillId = 32311u; hit.iStaggerMaximumDivisor = 3u;
        for (unsigned count = 1u; count <= 3u; ++count)
        {
            hit.iServerTick = 500u + count;
            (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
            BOSS_COMBAT_SNAPSHOT gauge;
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(*boss, pattern, ledger, hit.iServerTick, gauge);
            tests.Require(boss->iCurrentHp == 100000u && gauge.iMaximumMechanicGauge == 1000u &&
                gauge.iCurrentMechanicGauge == 1000u - (std::min)(1000u, count * 334u),
                "Kouku HUD decreases by a maximum-third while boss HP stays unchanged");
            CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
                room->m_GameplayCatalog, nullptr, hit.iServerTick, damageEvents, output);
        }
        tests.Require(ledger.Windows.front().bClosed && output.bStaggerSuccess && output.bEndPatternEarly &&
            boss->iCurrentHp == 100000u && !boss->iKoukuItemStaggerMaximum && !boss->iKoukuItemStaggerCredit,
            "Third whirlwind completes the actual Kouku window and clears only that window's credit");
        ++boss->iPatternSequence;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 700u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 700u, damageEvents, output);
        auto skill = hit; skill.iStaggerMaximumDivisor = 0u; skill.iRawDamage = 100u; skill.iServerTick = 701u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, skill, damageEvents);
        hit.iServerTick = 702u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
        BOSS_COMBAT_SNAPSHOT mixed;
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(*boss, pattern, ledger, 702u, mixed);
        tests.Require(boss->iCurrentHp == 99900u && mixed.iCurrentMechanicGauge == 566u,
            "Existing skill HP contribution and separate whirlwind credit combine without extra HP damage");
        BOSS_PATTERN_LOGIC_RESULT fear;
        fear.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR; fear.iDurationMs = 3000u;
        fear.strFearPresentationId = "battle.item.fear";
        ally.iHolyCharmProtectionEndTick = 900u; ally.eAction = PLAYER_ACTION_STATE::NONE;
        CKoukuSaydonLogicRuntime::Apply_Result(ally, fear, *boss, room->m_GameplayCatalog, nullptr, 800u, damageEvents);
        tests.Require(ally.eAction != PLAYER_ACTION_STATE::FEAR, "Holy charm blocks new Kouku fear during protection");
        CKoukuSaydonLogicRuntime::Apply_Result(ally, fear, *boss, room->m_GameplayCatalog, nullptr, 900u, damageEvents);
        tests.Require(ally.eAction == PLAYER_ACTION_STATE::FEAR, "Kouku fear resumes exactly after holy protection expires");
    }
    std::cout << "failures : " << tests.failures << '\n';
    return tests.failures ? 1 : 0;
}
