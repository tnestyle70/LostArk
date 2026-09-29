#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "ItemCatalog.h"
#include "ServerCombatHitRuntime.h"
#include "KoukuSaydonLogicRuntime.h"
#include "WorldBootstrap.h"
#include "Network/PacketReader.h"

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
    {
        // Exercise the real use command, trajectory, part authority and four observer
        // packets. Only the current recovery window and cooldown clock are fixture inputs.
        auto arena = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
        std::vector<std::shared_ptr<CClientSession>> observers;
        bool admitted = arena->Is_Ready();
        for (unsigned index = 0; index < 4u && admitted; ++index)
        {
            auto session = std::make_shared<CClientSession>(91000u + index, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            session->m_isSendRunning.store(true);
            arena->Handle_Register(session);
            observers.push_back(session);
            C2S_ENTER_WORLD entry;
            entry.eWorldId = WORLD_ID::VALTAN_ARENA;
            entry.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
            entry.strNickName = "BombArmor" + std::to_string(index + 1u);
            admitted = arena->Join(session->Get_SessionId(), entry);
        }
        const auto* placement = arena->Find_Placement("boss.valtan.center");
        const auto* patterns = arena->m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
        const BOSS_PATTERN_STAGE_DEFINITION* recovery = nullptr;
        std::uint32_t recoveryIndex = 0u;
        if (patterns) for (const auto& pattern : *patterns)
            if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
                for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
                    if (pattern.Stages[index].strActionId == "valtan.attack.dash-charge.recovery")
                    { recovery = &pattern.Stages[index]; recoveryIndex = static_cast<std::uint32_t>(index); }
        auto stagedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
        const bool built = admitted && placement && recovery &&
            arena->Build_WorldEntity(*placement, 500001u, *stagedBoss);
        tests.Require(built && arena->Count_HumanPlayers() == 4u &&
            stagedBoss->BossCombat.iAlivePartMask == 3u && stagedBoss->ArmorPlates.size() == 2u &&
            recovery->ePartDamagePolicy == BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE,
            "Four admitted players use the published Valtan parts and dash recovery policy");
        if (built)
        {
            arena->m_WorldEntities.clear();
            arena->m_WorldEntities.push_back(std::move(*stagedBoss));
            auto& boss = arena->m_WorldEntities.front();
            auto& thrower = arena->m_Players.at(observers.front()->Get_PlayerId());
            SERVER_NAV_POINT source;
            const bool positioned = arena->m_ServerNavigation.Sample_SurfacePosition(
                boss.fPositionX, boss.fPositionZ + 4.f, source);
            thrower.fPositionX = source.x; thrower.fPositionY = source.y; thrower.fPositionZ = source.z;
            thrower.isCombatReady = true;
            tests.Require(positioned && arena->Grant_Item(thrower, "BATTLE_DESTRUCTION_BOMB", 5u),
                "Destruction bombs enter a real inventory at a navigable throwing position");
            const auto remaining = [&]() {
                const auto item = std::find_if(thrower.Inventory.begin(), thrower.Inventory.end(),
                    [](const auto& row) { return row.strItemId == "BATTLE_DESTRUCTION_BOMB"; });
                return item == thrower.Inventory.end() ? 0u : item->iQuantity;
            };
            const auto initialQuantity = remaining();
            std::uint32_t sequence = 1u;
            for (unsigned attempt = 0u; attempt < 3u && positioned; ++attempt)
            {
                for (const auto& observer : observers)
                { observer->m_OutboundFrames.clear(); observer->m_iQueuedOutboundBytes = 0u; }
                arena->m_iServerTick = 100u + attempt * 1000u;
                arena->m_TickBossCombatEvents.clear();
                boss.BossCombat.PendingOutcomes.clear();
                boss.bPendingArmorBreakReaction = false;
                boss.strPatternId = "VALTAN_DASH_CHARGE";
                boss.strPatternStageId = recovery->strStageId;
                boss.strActionId = recovery->strActionId;
                boss.iPatternSequence = attempt + 1u;
                boss.iPatternStageIndex = recoveryIndex;
                boss.iPatternStageDurationMs = recovery->iDurationMs;
                boss.iActionStartTick = arena->m_iServerTick;
                boss.eAction = SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
                boss.ePatternPartDamagePolicy = recovery->ePartDamagePolicy;
                boss.bPatternGroggy = attempt != 0u;
                (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY, attempt != 0u);
                const auto beforeMask = boss.BossCombat.iAlivePartMask;
                std::uint32_t expectedBroken = 0u;
                if (attempt) for (const auto& part : boss.BossCombat.Parts)
                    if (part.iStateMask & beforeMask) { expectedBroken = part.iStateMask; break; }
                C2S_USE_ITEM request;
                request.strItemId = "BATTLE_DESTRUCTION_BOMB";
                request.hasGroundTarget = true;
                request.iRequestSequence = sequence++;
                request.fTargetX = thrower.fPositionX + 100.f; request.fTargetZ = thrower.fPositionZ;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt && arena->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                    "Out-of-range destruction throws preserve inventory and create no projectile");
                request.iRequestSequence = sequence++;
                request.fTargetX = boss.fPositionX; request.fTargetZ = boss.fPositionZ;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt - 1u && arena->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
                    "Accepted destruction command consumes one item and launches one server projectile");
                arena->Handle_UseItem(thrower.iSessionId, request);
                request.iRequestSequence = sequence++;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt - 1u && arena->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
                    "Duplicate and cooldown destruction requests cannot duplicate the throw");
                damageEvents.clear();
                for (unsigned step = 0u; step < 60u && !arena->m_CombatObjectRuntime.Get_LiveObjects().empty(); ++step)
                {
                    ++arena->m_iServerTick;
                    arena->m_CombatObjectRuntime.Update(arena->m_Players, arena->m_WorldEntities,
                        arena->m_GameplayCatalog, 1.f / 30.f, arena->m_iServerTick, damageEvents);
                }
                tests.Require(arena->m_CombatObjectRuntime.Get_LiveObjects().empty() &&
                    boss.BossCombat.iAlivePartMask == (beforeMask & ~expectedBroken) &&
                    boss.bPendingArmorBreakReaction == (expectedBroken != 0u),
                    "Destruction contact breaks exactly one eligible plate only while groggy");
                arena->Drain_BossCombatEvents();
                const bool broadcast = arena->Broadcast_CombatObjectLifecycle();
                arena->Broadcast_WorldSnapshot();
                bool parity = broadcast;
                for (const auto& observer : observers)
                {
                    unsigned spawns = 0u, impacts = 0u, despawns = 0u, snapshots = 0u;
                    for (const auto& frame : observer->m_OutboundFrames)
                    {
                        CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                        if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED)
                        {
                            S2C_COMBAT_OBJECT_SPAWNED message;
                            parity &= Read_Message(reader, message) && message.strClientVisualId == "battle.item.destruction_bomb";
                            ++spawns;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT)
                        {
                            S2C_COMBAT_OBJECT_PRESENTATION_EVENT message;
                            parity &= Read_Message(reader, message) && message.strHitId == "battle.item.impact" &&
                                message.eKind == COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE;
                            ++impacts;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED)
                        {
                            S2C_COMBAT_OBJECT_DESPAWNED message;
                            parity &= Read_Message(reader, message);
                            ++despawns;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT)
                        {
                            S2C_WORLD_SNAPSHOT message;
                            parity &= Read_Message(reader, message);
                            const auto entity = std::find_if(message.Entities.begin(), message.Entities.end(),
                                [&](const auto& row) { return row.iNetEntityId == boss.iNetEntityId; });
                            parity &= entity != message.Entities.end() && entity->BossCombat.iAlivePartMask == boss.BossCombat.iAlivePartMask &&
                                entity->iBrokenArmorMask == static_cast<std::uint8_t>(3u & ~boss.BossCombat.iAlivePartMask) &&
                                message.BossCombatEvents.size() == (expectedBroken ? 1u : 0u);
                            if (expectedBroken && message.BossCombatEvents.size() == 1u)
                                parity &= message.BossCombatEvents.front().eKind == BOSS_COMBAT_EVENT_KIND::PART_BROKEN &&
                                    message.BossCombatEvents.front().iPartMask == expectedBroken;
                            ++snapshots;
                        }
                    }
                    parity &= spawns == 1u && impacts == 1u && despawns == 1u && snapshots == 1u;
                }
                tests.Require(parity, "Four observers decode one flight, impact, removal and matching armor break snapshot");
            }
            tests.Require(!boss.BossCombat.iAlivePartMask && std::all_of(boss.ArmorPlates.begin(), boss.ArmorPlates.end(),
                [](const auto& plate) { return !plate.iRemainingDurability; }),
                "Two separate groggy windows remove both typed and legacy armor plates");
        }
        for (const auto& observer : observers) observer->Request_Close();
    }
    std::cout << "failures : " << tests.failures << '\n';
    return tests.failures ? 1 : 0;
}
