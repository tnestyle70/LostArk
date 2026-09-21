#include "ServerGameplayContractTests_Runner.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "ServerNavigation.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;



void CServerGameplayContractRunner::Run_KoukuPushContracts(TESTS& tests, const CGameplayCatalog& catalog)
{
    auto boss = std::make_unique<SERVER_WORLD_ENTITY>(); boss->iPatternSequence = 1u;
    const auto player = [] { SERVER_PLAYER p{}; p.iPlayerId = 1u; p.iNetEntityId = 101u;
        p.iCurrentHp = p.iMaximumHp = 100u; p.isCombatReady = true; p.fPositionX = 1.f; return p; };
    BOSS_PATTERN_LOGIC_RESULT push{}; push.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE;
    push.iPercent = 10u; push.fPushRangeM = 16.f; push.iPushMs = 242u; push.bForcePush = true; push.bPushCanLeaveArena = true;
    std::vector<DAMAGE_EVENT> events;
    for (unsigned state = 0u; state < 4u; ++state)
    {
        auto p = player();
        if (state == 0u) { p.eAction = PLAYER_ACTION_STATE::FEAR; p.iFearEndTick = 200u; p.strFearPresentationId = "push.fear"; }
        if (state == 1u) { p.eAction = PLAYER_ACTION_STATE::KNOCKDOWN; p.iKnockdownEndTick = 200u; }
        if (state == 2u) p.iHitReactionGraceEndTick = 200u;
        if (state == 3u) { p.fKnockbackRemainingSeconds = 1.f; p.fKnockbackSpeed = 1.f; }
        auto ordinary = p; auto normal = push; normal.bForcePush = false;
        CKoukuSaydonLogicRuntime::Apply_Result(ordinary, normal, *boss, catalog, nullptr, 100u, events);
        CKoukuSaydonLogicRuntime::Apply_Result(p, push, *boss, catalog, nullptr, 100u, events);
        tests.Require(ordinary.fKnockbackSpeed <= 1.f && p.iCurrentHp == 90u && p.eAction == PLAYER_ACTION_STATE::NONE &&
            p.iFearEndTick == 0u && p.iKnockdownEndTick == 0u && p.iHitReactionGraceEndTick == 0u &&
            std::abs(p.fKnockbackSpeed * p.fKnockbackRemainingSeconds - 16.f) < .0001f && p.bKnockbackCanLeaveArena,
            "Explicit force push replaces fear/down/grace/previous reaction; ordinary push preserves resistance");
    }
    for (const auto action : {PLAYER_ACTION_STATE::DEAD, PLAYER_ACTION_STATE::FALLING, PLAYER_ACTION_STATE::GRABBED, PLAYER_ACTION_STATE::TRIGGER_MOVE})
    {
        auto p = player(); p.eAction = action;
        if (action == PLAYER_ACTION_STATE::DEAD) p.iCurrentHp = 0u;
        CKoukuSaydonLogicRuntime::Apply_Result(p, push, *boss, catalog, nullptr, 100u, events);
        tests.Require(p.eAction == action && p.fKnockbackRemainingSeconds == 0.f,
            "Force push never replaces death, falling, attachment or world transfer");
    }
    auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
    const auto arenaNavigation = room->m_ServerNavigation;
    room->m_ServerNavigation = CServerNavigation{};
    std::string status; room->m_ServerCollisionSystem.Initialize({}, status); room->m_ServerCollisionSystem.Set_BlockingBodies({});
    for (const float distance : {6.f, 16.f})
    {
        auto p = player(); auto effect = push; effect.fPushRangeM = distance;
        effect.ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD; effect.fPushYawOffsetDegrees = 90.f;
        boss->fYawDegrees = 0.f;
        CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events);
        for (unsigned tick = 0u; tick < 8u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
        tests.Require(std::abs(p.fPositionX - (1.f + distance)) < .0001f && std::abs(p.fPositionZ) < .0001f &&
            p.fKnockbackRemainingSeconds == 0.f && !p.bKnockbackCanLeaveArena && p.iCurrentHp == 90u,
            "Six and sixteen metre pushes consume exact authored distance; laser local X uses yaw offset once");
    }
    {
        auto p = player(); auto effect = push;
        effect.fPushRangeM = 30.f; effect.iPushMs = 1500u; effect.bPushBallistic = true;
        effect.ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT;
        const std::array<float, 2u> contactCenter{0.f, 0.f};
        boss->fPositionX = 50.f;
        CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events, &contactCenter);
        tests.Require(p.eAction == PLAYER_ACTION_STATE::KNOCKDOWN && p.bKnockbackBallistic &&
            p.fKnockbackDirectionX > .999f && std::abs(p.fKnockbackVelocityY - 7.35f) < .0001f,
            "Ballistic result launches away from contacted circle, independently of distant boss position");
        room->Advance_PlayerKnockback(p, .75f);
        tests.Require(std::abs(p.fPositionX - 16.f) < .0001f && std::abs(p.fPositionY - 2.75625f) < .0001f,
            "Thirty metre 1500ms ballistic push reaches expected halfway position and gravity apex");
        room->Advance_PlayerKnockback(p, .75f);
        tests.Require(std::abs(p.fPositionX - 31.f) < .0001f && std::abs(p.fPositionY) < .0001f &&
            !p.bKnockbackBallistic && p.fKnockbackRemainingSeconds == 0.f && p.eAction != PLAYER_ACTION_STATE::FALLING,
            "Ballistic movement lands after its authored distance without a navigation system");
        p = player(); const auto beforeEvents = events.size();
        CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events);
        tests.Require(p.iCurrentHp == 100u && events.size() == beforeEvents,
            "Missing contact centre cannot silently substitute the boss origin");
        boss->fPositionX = 0.f;
    }
    {
        BOSS_PATTERN_DEFINITION repeatPattern{}; repeatPattern.strPatternId = "laser.reentry";
        BOSS_PATTERN_LOGIC_WINDOW repeat{}; repeat.strWindowId = "laser.contact";
        repeat.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; repeat.iDurationMs = 231u;
        repeat.bRearmOnExit = true; repeat.OnSuccess = {push};
        BOSS_LOGIC_REGION r{}; r.strRegionId = "laser.circle"; r.bCircle = true; r.fRadiusM = 4.f;
        r.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD; repeat.CardRegions = {r}; repeatPattern.LogicWindows = {repeat};
        std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, player()}};
        KOUKUSAYDON_LOGIC_LEDGER repeatLedger{}; KOUKUSAYDON_LOGIC_OUTPUT repeatOutput{};
        std::vector<DAMAGE_EVENT> repeatEvents;
        CKoukuSaydonLogicRuntime::Build(repeatPattern, *boss, 100u, repeatLedger);
        const auto update = [&](unsigned tick) { CKoukuSaydonLogicRuntime::Update(*boss, repeatPattern,
            repeatLedger, players, catalog, nullptr, tick, repeatEvents, repeatOutput); };
        update(100u); update(101u);
        players.at(1u).fPositionX = 10.f; update(102u);
        players.at(1u).fPositionX = -1.f; update(103u); update(104u);
        tests.Require(players.at(1u).iCurrentHp == 80u && repeatEvents.size() == 2u &&
            players.at(1u).fKnockbackDirectionX < -.999f && players.at(1u).fKnockbackRemainingSeconds > .24f,
            "Short laser contact rearms on real reentry during previous push and stays latched between exits");
    }
    {
        const auto hookPattern = [](bool grip, bool sweep, bool hidden) {
            BOSS_PATTERN_DEFINITION p{}; p.strPatternId = "hook.body";
            BOSS_PATTERN_LOGIC_WINDOW w{}; w.strWindowId = "hook.capture";
            w.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; w.iDurationMs = 1000u;
            BOSS_PATTERN_LOGIC_RESULT result{}; result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT;
            w.OnSuccess = {result}; BOSS_LOGIC_REGION r{}; r.strRegionId = "hook.tip";
            r.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD; r.fHalfX = .25f; r.fHalfY = .35f; r.fHalfZ = .2f;
            r.WorldTrack.bEnabled = true; r.WorldTrack.iDurationMs = 1000u; r.WorldTrack.fPlaybackSpeed = 1.f;
            for (const unsigned time : {0u, 33u, 1000u})
            {
                BOSS_LOGIC_WORLD_TRANSFORM_KEY key{}; key.iTimeMs = time;
                key.fOffsetX = sweep ? (time ? -2.f : 2.f) : 0.f; key.fOffsetY = 2.2f;
                key.bHasGripPosition = grip; key.GripPosition = {key.fOffsetX, key.fOffsetY, 0.f};
                key.bVisible = !hidden || time != 0u; r.WorldTrack.Keys.push_back(key);
            }
            w.CardRegions = {r}; p.LogicWindows = {w}; return p;
        };
        const auto capture = [&](float x, float y, float z, bool grip, bool sweep, bool hidden) {
            auto p = player(); p.fPositionX = x; p.fPositionY = y; p.fPositionZ = z;
            auto definition = hookPattern(grip, sweep, hidden);
            auto owner = std::make_unique<SERVER_WORLD_ENTITY>(); owner->iNetEntityId = 201u; owner->iPatternSequence = 3u;
            std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, p}};
            KOUKUSAYDON_LOGIC_LEDGER ledger{}; KOUKUSAYDON_LOGIC_OUTPUT output{}; std::vector<DAMAGE_EVENT> damage;
            CKoukuSaydonLogicRuntime::Build(definition, *owner, 100u, ledger);
            CKoukuSaydonLogicRuntime::Update(*owner, definition, ledger, players, catalog, nullptr,
                sweep ? 101u : 100u, damage, output);
            const auto& caught = players.at(1u);
            return caught.eAction == PLAYER_ACTION_STATE::GRABBED && caught.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
        };
        tests.Require(capture(0.f, 1.3f, 0.f, true, false, false), "Hook body contact captures even when the feet miss its vertical box");
        tests.Require(capture(.55f, 1.3f, 0.f, true, false, false), "Hook side contact uses the existing player body radius");
        tests.Require(capture(.7005f, 1.3f, 0.f, true, false, false), "Hook admits the common contact margin at body tangency");
        tests.Require(!capture(.702f, 1.3f, 0.f, true, false, false), "Separated hook and body do not capture");
        tests.Require(!capture(.6f, 1.3f, .6f, true, false, false), "Hook rounded corner test rejects expanded-box false positives");
        tests.Require(!capture(0.f, -1.f, 0.f, true, false, false), "Overhead hook cannot capture a vertically separated body");
        tests.Require(capture(0.f, 1.3f, .55f, true, true, false), "Fast hook sweep catches the body between fixed ticks");
        tests.Require(!capture(0.f, 1.3f, .66f, true, true, false), "Swept hook outside player radius does not capture");
        tests.Require(!capture(0.f, 1.3f, .55f, true, true, true), "Hidden hook intervals do not create swept contact");
        tests.Require(!capture(.55f, 1.3f, 0.f, false, false, false), "Legacy point regions retain their prior footprint");
        tests.Require(capture(0.f, 1.3f, 0.f, false, false, false), "Legacy point capture still works at its centre");
    }
    // Protection intercepts the complete result before either damage or forced movement.
    BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "push.protected";
    BOSS_PATTERN_LOGIC_WINDOW contact{}, zone{}; contact.strWindowId = "push.contact";
    contact.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA; contact.iDurationMs = 1000u; contact.OnSuccess = {push};
    BOSS_LOGIC_REGION circle{}; circle.strRegionId = "push.circle"; circle.bCircle = true; circle.fRadiusM = 4.f;
    circle.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD; contact.CardRegions = {circle}; zone = contact;
    zone.strWindowId = "push.zone"; zone.eKind = BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE; zone.OnSuccess.clear();
    pattern.LogicWindows = {contact, zone}; std::map<PLAYER_ID, SERVER_PLAYER> protectedPlayers{{1u, player()}};
    KOUKUSAYDON_LOGIC_LEDGER ledger{}; KOUKUSAYDON_LOGIC_OUTPUT output{};
    CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
    CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, protectedPlayers, catalog, nullptr, 100u, events, output);
    tests.Require(protectedPlayers.at(1u).iCurrentHp == 100u && protectedPlayers.at(1u).fKnockbackRemainingSeconds == 0.f,
        "Same Pattern invulnerability zone suppresses damage and force push together");
    room->m_ServerNavigation = arenaNavigation;
    SERVER_NAV_POINT center{}; bool foundDeck = arenaNavigation.Is_PointWalkableExact(4.66f, 322.94f) && arenaNavigation.Sample_Position(4.66f, 322.94f, center);
    float lastGroundX = center.x; bool interiorGap = false, returnedGround = false; float gapStartX = center.x;
    for (float x = 4.66f; foundDeck && x <= 132.66f; x += .125f)
    {
        SERVER_NAV_POINT ground{};
        const bool same = arenaNavigation.Is_PointWalkableExact(x, 322.94f) && arenaNavigation.Sample_Position(x, 322.94f, ground);
        if (same) { if (interiorGap) returnedGround = true; lastGroundX = x; }
        else if (!interiorGap) { interiorGap = true; gapStartX = x - .125f; }
    }
    tests.Require(foundDeck && lastGroundX > 4.66f, "Actual Gate 2 navigation provides a bounded outer deck edge");
    if (foundDeck)
    {
        SERVER_NAV_POINT edgeGround{}; arenaNavigation.Sample_Position(lastGroundX, 322.94f, edgeGround);
        for (const float distance : {6.f, 16.f})
        {
            auto p = player(); p.fPositionX = center.x; p.fPositionY = center.y; p.fPositionZ = center.z;
            boss->fPositionX = p.fPositionX; boss->fPositionZ = p.fPositionZ - 1.f;
            auto effect = push; effect.fPushRangeM = distance;
            CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events);
            for (unsigned tick = 0u; tick < 8u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
            tests.Require(p.eAction != PLAYER_ACTION_STATE::FALLING && p.iCurrentHp == 90u &&
                p.fKnockbackRemainingSeconds == 0.f && arenaNavigation.Is_PointWalkableExact(p.fPositionX, p.fPositionZ),
                "Finite six/sixteen metre push from actual centre survives navigation height clamp");
        }
        for (const bool mayExit : {false, true})
        {
            auto p = player(); p.fPositionX = lastGroundX; p.fPositionY = edgeGround.y; p.fPositionZ = 322.94f;
            boss->fPositionX = p.fPositionX - 1.f; boss->fPositionZ = p.fPositionZ;
            auto effect = push; effect.bPushCanLeaveArena = mayExit; effect.fPushRangeM = 6.f;
            CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events);
            room->m_iServerTick = 100u; room->Advance_PlayerKnockback(p, .242f);
            tests.Require(mayExit ? p.eAction == PLAYER_ACTION_STATE::FALLING && p.iFallDeathTick != 0u :
                p.eAction != PLAYER_ACTION_STATE::FALLING && arenaNavigation.Is_PointWalkableExact(p.fPositionX, p.fPositionZ),
                "Only authored arena-exit push crosses the actual deck edge and begins ordinary falling");
            if (mayExit)
            {
                room->Update_PlayerFall(p, 1.f / 30.f, p.iFallDeathTick);
                tests.Require(p.iCurrentHp == 0u && p.eAction == PLAYER_ACTION_STATE::DEAD, "Arena-exit push completes existing falling death lifecycle");
            }
        }
        WORLD_BOOTSTRAP_PLACEMENT wall{}; wall.strPlacementId = "push.wall"; wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
        wall.fPositionX = lastGroundX + .5f; wall.fPositionY = edgeGround.y; wall.fPositionZ = 322.94f;
        wall.fHalfExtentX = .1f; wall.fHalfExtentY = wall.fHalfExtentZ = 10.f; room->m_ServerCollisionSystem.Initialize({wall}, status);
        auto p = player(); p.fPositionX = lastGroundX - 1.f; p.fPositionY = edgeGround.y; p.fPositionZ = 322.94f;
        boss->fPositionX = p.fPositionX - 1.f; boss->fPositionZ = p.fPositionZ;
        CKoukuSaydonLogicRuntime::Apply_Result(p, push, *boss, catalog, nullptr, 100u, events); room->Advance_PlayerKnockback(p, .242f);
        tests.Require(p.eAction != PLAYER_ACTION_STATE::FALLING && p.fPositionX < wall.fPositionX && p.fKnockbackRemainingSeconds == 0.f,
            "Forced arena-exit policy still respects swept static collision walls");
        room->m_ServerCollisionSystem.Initialize({}, status);
        SERVER_BLOCKING_BODY body{}; body.fX = lastGroundX + .5f; body.fZ = 322.94f; body.fRadius = .6f;
        room->m_ServerCollisionSystem.Set_BlockingBodies({body});
        p = player(); p.fPositionX = lastGroundX - 1.f; p.fPositionY = edgeGround.y; p.fPositionZ = 322.94f;
        float slideX = 0.f, slideY = 0.f, slideZ = 0.f; bool slideBlocked = true;
        const bool slideResolves = room->m_ServerCollisionSystem.Resolve_PlayerMove(p, lastGroundX + .125f,
            edgeGround.y, 322.94f, slideX, slideY, slideZ, slideBlocked);
        boss->fPositionX = p.fPositionX - 1.f; boss->fPositionZ = p.fPositionZ;
        CKoukuSaydonLogicRuntime::Apply_Result(p, push, *boss, catalog, nullptr, 100u, events);
        room->Advance_PlayerKnockback(p, .242f);
        tests.Require(slideResolves && !slideBlocked && arenaNavigation.Is_PointWalkableExact(slideX, slideZ) &&
            p.eAction != PLAYER_ACTION_STATE::FALLING && arenaNavigation.Is_PointWalkableExact(p.fPositionX, p.fPositionZ),
            "Body slide returning an attempted exit to ground cannot start falling");
        room->m_ServerCollisionSystem.Set_BlockingBodies({});
        if (interiorGap && returnedGround)
        {
            p = player(); p.fPositionX = gapStartX; p.fPositionY = edgeGround.y; p.fPositionZ = 322.94f;
            boss->fPositionX = p.fPositionX - 1.f; boss->fPositionZ = p.fPositionZ;
            CKoukuSaydonLogicRuntime::Apply_Result(p, push, *boss, catalog, nullptr, 100u, events); room->Advance_PlayerKnockback(p, .242f);
            tests.Require(p.eAction != PLAYER_ACTION_STATE::FALLING, "Interior navigation/height seam with further same-deck ground is not an arena exit");
        }
    }
}

int LostArk::Server::Run_ServerKoukuObjectOverlapContractTests()
{
	TESTS tests;
	const CGameplayCatalog catalog;
	CServerGameplayContractRunner::Run_KoukuPushContracts(tests, catalog);
	Run_KoukuObjectOverlapContracts(tests, catalog);
	Run_KoukuObjectContactContracts(tests, catalog);
	Run_KoukuFearAndCounterContracts(tests, catalog);
	Run_KoukuWorldPlacementContracts(tests, catalog);
	Run_KoukuBoneContactContracts(tests, catalog);
#ifdef _DEBUG
    {
        using namespace LostArk::Shared;
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        // The room constructor loads published placements. This fixture owns exactly two synthetic bosses.
        room->m_WorldEntities.clear();
        auto bossValue = std::make_unique<SERVER_WORLD_ENTITY>();
        bossValue->iNetEntityId = 4000u; bossValue->iPatternSequence = 5u;
        bossValue->strPatternId = "grab.pattern"; bossValue->strActionId = "grab.action";
        bossValue->eKind = WORLD_BOOTSTRAP_KIND::BOSS; bossValue->eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
        bossValue->iCurrentHp = bossValue->iMaximumHp = 1000u;
        room->m_WorldEntities.push_back(*bossValue);
        bossValue->iNetEntityId = 5000u;
        room->m_WorldEntities.push_back(*bossValue);
        auto& boss = room->m_WorldEntities.front();
        for (PLAYER_ID id = 1u; id <= 3u; ++id)
        {
            SERVER_PLAYER player{}; player.iPlayerId = id; player.iNetEntityId = static_cast<NET_ENTITY_ID>(100u + id);
            player.iCurrentHp = player.iMaximumHp = 1000u; player.fPositionZ = 2.f;
            player.fPositionX = id == 1u ? 0.f : 50.f;
            room->m_PlayerIdByEntityId.emplace(player.iNetEntityId, id);
            room->m_Players.emplace(id, player);
        }
        BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss.strPatternId;
        BOSS_PATTERN_LOGIC_WINDOW trigger{};
        trigger.strWindowId = "grab.trigger"; trigger.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
        trigger.bRearmOnExit = true;
        trigger.iStartMs = 2029u; trigger.iDurationMs = 788u; trigger.strHoldLogicOccurrenceId = "grab.hold";
        BOSS_PATTERN_LOGIC_RESULT capture{}; capture.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
        capture.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND; trigger.OnSuccess = {capture};
        BOSS_LOGIC_REGION sector{}; sector.strRegionId = "grab.fan"; sector.bSector = true;
        sector.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT; sector.fRadiusM = 10.f; sector.fHalfAngleDegrees = 45.f;
        trigger.CardRegions = {sector};
        BOSS_PATTERN_LOGIC_WINDOW hold{};
        hold.strWindowId = "grab.hold"; hold.eKind = BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD;
        hold.iStartMs = 2029u; hold.iDurationMs = 5455u;
        pattern.LogicWindows = {trigger, hold};
        CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER member;
        member.strMemberId = "grab.member"; member.iBossEntityId = boss.iNetEntityId;
        member.PatternIds = {pattern.strPatternId}; member.iPatternSequence = boss.iPatternSequence;
        CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, member.LogicLedger);
        room->m_KoukuSaydonPatternAudition.Members.push_back(std::move(member));
        auto& ledger = room->m_KoukuSaydonPatternAudition.Members.front().LogicLedger;
        auto& first = room->m_Players.at(1u); auto& late = room->m_Players.at(2u); auto& other = room->m_Players.at(3u);
        std::vector<DAMAGE_EVENT> events;
        const auto runTick = [&](const std::uint32_t tick) {
            KOUKUSAYDON_LOGIC_OUTPUT output;
            CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, room->m_Players, catalog, nullptr, tick, events, output);
            (void)room->Apply_KoukuLogicOutput(output, boss, tick);
        };
        KOUKUSAYDON_LOGIC_OUTPUT rejected;
        CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, room->m_Players, catalog, nullptr, 161u, events, rejected);
        first.isCombatReady = false;
        (void)room->Apply_KoukuLogicOutput(rejected, boss, 161u);
        tests.Require(rejected.CaptureRequests.size() == 1u && first.eAction == PLAYER_ACTION_STATE::NONE &&
            ledger.Windows.front().Answers[1u] != KOUKUSAYDON_LOGIC_ANSWER::SUCCESS,
            "A room-rejected grab remains unanswered instead of losing the retryable capture candidate");
        first.isCombatReady = true; runTick(162u);
        late.fPositionX = 0.f; runTick(171u);
        tests.Require(first.eAction == PLAYER_ACTION_STATE::GRABBED && late.eAction == PLAYER_ACTION_STATE::GRABBED &&
            first.iAttachmentEndTick == 325u && late.iAttachmentEndTick == 325u &&
            first.iActionStartTick == 162u && late.iActionStartTick == 171u,
            "Early and late captures use the same authored 7484ms Hold endpoint through the actual room attachment transaction");
        const bool duplicateRejected = !room->Capture_PlayerAttachment(first.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 172u, 400u);
        runTick(186u);
        boss.fPositionX = 5.f;
        const bool follows = room->Update_PlayerAttachment(first, 186u) && room->Update_PlayerAttachment(late, 186u);
        tests.Require(duplicateRejected && follows && first.iAttachmentEndTick == 325u && first.fPositionX == 5.f,
            "The short Collider closing cannot release a held player, and duplicate capture cannot extend its deadline");
        const bool otherCaptured = room->Capture_PlayerAttachment(other.iNetEntityId, 5000u,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 186u, 400u);
        const bool heldUntilEnd = room->Update_PlayerAttachment(first, 324u);
        const bool releasedAtEnd = !room->Update_PlayerAttachment(first, 325u) && !room->Update_PlayerAttachment(late, 325u);
        tests.Require(otherCaptured && heldUntilEnd && releasedAtEnd && first.eAction == PLAYER_ACTION_STATE::NONE &&
            first.iAttachmentEndTick == 0u && first.isCombatReady && first.fKnockbackRemainingSeconds == 0.f &&
            first.iKnockdownEndTick == 0u && other.eAction == PLAYER_ACTION_STATE::GRABBED,
            "Hold expiry releases without push/down effects and leaves another boss's held player intact");
        const bool capturedAgain = room->Capture_PlayerAttachment(first.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 330u, 390u);
        ++boss.iPatternSequence;
        const bool staleReleased = !room->Update_PlayerAttachment(first, 331u);
        const bool capturedForDeath = room->Capture_PlayerAttachment(first.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 332u, 390u);
        first.iCurrentHp = 0u;
        const bool deathReleased = !room->Update_PlayerAttachment(first, 333u);
        tests.Require(capturedAgain && staleReleased && capturedForDeath && deathReleased &&
            first.eAction == PLAYER_ACTION_STATE::DEAD && first.iAttachmentEndTick == 0u,
            "An owner sequence change or player death clears a timed attachment without reviving its player");
        first.iCurrentHp = 1000u; first.eAction = PLAYER_ACTION_STATE::FEAR; first.isCombatReady = true;
        first.iActionStartTick = 333u; first.iFearEndTick = 423u; first.strFearPresentationId = "fear.result";
        tests.Require(!room->Capture_PlayerAttachment(first.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 334u, 390u) && first.iFearEndTick == 423u,
            "Capture cannot replace an active fear action or shorten its deadline");
        first.eAction = PLAYER_ACTION_STATE::NONE; first.iFearEndTick = 0u; first.strFearPresentationId.clear();
        const bool capturedForOwnerDeath = room->Capture_PlayerAttachment(first.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 334u, 390u);
        boss.iCurrentHp = 0u;
        const bool ownerDeathReleased = !room->Update_PlayerAttachment(first, 335u);
        tests.Require(capturedForOwnerDeath && ownerDeathReleased && first.eAction == PLAYER_ACTION_STATE::NONE &&
            first.iCurrentHp == 1000u && first.isCombatReady && other.eAction == PLAYER_ACTION_STATE::GRABBED,
            "Boss death releases its held player without damage or affecting another owner");
        boss.iCurrentHp = 1000u;
        const bool capturedForStop = room->Capture_PlayerAttachment(late.iNetEntityId, boss.iNetEntityId,
            PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND, 334u, 390u);
        room->m_iServerTick = 335u;
        room->Clear_KoukuSaydonPatternAudition();
        tests.Require(capturedForStop && late.eAction == PLAYER_ACTION_STATE::NONE && late.iAttachmentEndTick == 0u &&
            other.eAction == PLAYER_ACTION_STATE::GRABBED && room->m_KoukuSaydonPatternAudition.Members.empty(),
            "The Stop/Restart/disconnect cleanup releases its own grab and preserves unrelated boss attachments");
    }
	{
		using namespace LostArk::Shared;
		auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
		const auto arenaNavigation = room->m_ServerNavigation;
		room->m_ServerNavigation = CServerNavigation{};
		std::string collisionStatus;
		room->m_ServerCollisionSystem.Initialize({}, collisionStatus);
		room->m_ServerCollisionSystem.Set_BlockingBodies({});
		auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
		boss->iNetEntityId = 8000u; boss->iPatternSequence = 1u;
		boss->strPatternId = "whirlwind.reentry"; boss->iCurrentHp = 1000u;
		BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss->strPatternId;
		BOSS_PATTERN_LOGIC_WINDOW trigger{};
		trigger.strWindowId = "whirlwind.hit"; trigger.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
		trigger.iDurationMs = 10000u; trigger.bRearmOnExit = true;
		BOSS_LOGIC_REGION region{}; region.strRegionId = "whirlwind.body";
		region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT; region.bCircle = true; region.fRadiusM = 4.f;
		trigger.CardRegions = {region};
		BOSS_PATTERN_LOGIC_RESULT damage{}; damage.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE;
		damage.iPercent = 10u; damage.fPushRangeM = 2.f; damage.iPushMs = 242u;
		trigger.OnSuccess = {damage};
		BOSS_PATTERN_LOGIC_RESULT timeout{}; timeout.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE; timeout.iPercent = 5u;
		trigger.OnTimeout = {timeout}; pattern.LogicWindows = {trigger};
		for (PLAYER_ID id = 1u; id <= 3u; ++id)
		{
			SERVER_PLAYER player{}; player.iPlayerId = id; player.iNetEntityId = 8100u + id;
			player.iCurrentHp = player.iMaximumHp = 1000u; player.isCombatReady = true;
			player.fPositionX = id == 1u ? 1.f : id == 2u ? 9.f : 50.f;
			room->m_Players.emplace(id, player);
		}
		auto& first = room->m_Players.at(1u); auto& second = room->m_Players.at(2u);
		KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
		std::vector<DAMAGE_EVENT> events;
		CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
		const auto update = [&](unsigned tick) {
			CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players, catalog, nullptr, tick, events, output);
		};
		update(100u); update(101u); update(102u);
		tests.Require(first.iCurrentHp == 900u && second.iCurrentHp == 1000u && events.size() == 1u &&
			std::abs(first.fKnockbackDirectionX - 1.f) < .0001f && std::abs(first.fKnockbackDirectionZ) < .0001f &&
			std::abs(first.fKnockbackSpeed * first.fKnockbackRemainingSeconds - 2.f) < .0001f,
			"Reentry trigger hits on first entry, stays latched inside, and arms two-metre boss-away knockback");
		room->Advance_PlayerKnockback(first, .1f);
		room->Advance_PlayerKnockback(first, .1f);
		room->Advance_PlayerKnockback(first, .1f);
		tests.Require(std::abs(first.fPositionX - 3.f) < .0001f && first.fKnockbackRemainingSeconds == 0.f,
			"Existing authoritative knockback mover consumes exactly two metres over the 242ms window");
		first.fPositionX = 6.f; update(110u);
		first.fPositionX = -1.f; second.fPositionX = 0.f; second.fPositionZ = 1.5f; update(111u); update(112u);
		tests.Require(first.iCurrentHp == 800u && second.iCurrentHp == 900u && events.size() == 3u &&
			first.fKnockbackDirectionX < -.9999f && second.fKnockbackDirectionZ > .9999f,
			"Leaving rearms only that player; reentry and another player's first entry each hit once in their own outward direction");
		room->Advance_PlayerKnockback(first, .242f); room->Advance_PlayerKnockback(second, .242f);
		boss->fPositionX = 20.f; update(120u); boss->fPositionX = 0.f; update(121u);
		tests.Require(first.iCurrentHp == 700u && second.iCurrentHp == 800u && events.size() == 5u,
			"Boss-follow collider movement also records a real exit before its next entry hit");
		first.fPositionX = second.fPositionX = 50.f; update(400u); update(401u);
		tests.Require(first.iCurrentHp == 700u && second.iCurrentHp == 800u && room->m_Players.at(3u).iCurrentHp == 950u && events.size() == 6u,
			"Reentry window Timeout runs once only for the player who never entered; previous entrants are not punished on exit");
		pattern.LogicWindows.front().bRearmOnExit = false;
		pattern.LogicWindows.front().bRepeatAfterKnockback = true;
		pattern.LogicWindows.front().iDurationMs = 1000u;
		CKoukuSaydonLogicRuntime::Build(pattern, *boss, 500u, ledger);
		first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
		first.iCurrentHp = first.iMaximumHp = 1000u; first.isCombatReady = true; first.fPositionX = 1.f;
		events.clear(); update(500u);
		room->Advance_PlayerKnockback(first, .1f); update(501u); update(508u);
		tests.Require(first.iCurrentHp == 900u && events.size() == 1u && first.fKnockbackRemainingSeconds > 0.f,
			"Continuous contact suppresses all additional damage while knockback is still moving, even after its minimum hit interval");
		room->Advance_PlayerKnockback(first, .2f); update(509u); update(510u);
		tests.Require(first.iCurrentHp == 800u && events.size() == 2u && first.fKnockbackRemainingSeconds > 0.f,
			"Finishing knockback inside the same collider rearms the next hit without any exit or six duplicated triggers");
		first.fKnockbackRemainingSeconds = first.fKnockbackSpeed = 0.f; update(511u); update(516u);
		tests.Require(first.iCurrentHp == 800u && events.size() == 2u,
			"An early collision stop cannot cause a new contact hit before the authored 242ms interval");
		update(517u);
		tests.Require(first.iCurrentHp == 700u && events.size() == 3u,
			"An inside player can be hit again after the complete knockback interval");
		room->Advance_PlayerKnockback(first, .3f); first.fPositionX = 6.f; update(526u);
		tests.Require(first.iCurrentHp == 700u && events.size() == 3u, "Finished knockback outside the collider does not hit again");
		first.fPositionX = 1.f; update(527u); update(530u); update(540u);
		tests.Require(first.iCurrentHp == 600u && events.size() == 6u,
			"A later entry hits once and the window deadline ends contact processing (two never-entered players receive Timeout)");
		pattern.LogicWindows.front().bRepeatAfterKnockback = false;
		pattern.LogicWindows.front().iDurationMs = 10000u;
		CKoukuSaydonLogicRuntime::Build(pattern, *boss, 200u, ledger);
		first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
		first.iCurrentHp = first.iMaximumHp = 1000u; first.isCombatReady = true; first.fPositionX = 1.f;
		second.fPositionX = 50.f; second.fPositionZ = 0.f; events.clear();
		update(200u); first.fPositionX = 6.f; update(201u); first.fPositionX = 1.f; update(202u);
		tests.Require(first.iCurrentHp == 900u && events.size() == 1u,
			"Existing ENTER_AREA defaults remain one-shot even after leaving and reentering");
		first.fKnockbackRemainingSeconds = first.fKnockbackSpeed = 0.f;
		first.fPositionX = 1.f; first.fPositionZ = 0.f;
		WORLD_BOOTSTRAP_PLACEMENT wall{}; wall.strPlacementId = "whirlwind.wall";
		wall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX; wall.fPositionX = 2.f;
		wall.fHalfExtentX = .1f; wall.fHalfExtentY = wall.fHalfExtentZ = 10.f;
		const bool wallReady = room->m_ServerCollisionSystem.Initialize({wall}, collisionStatus);
		CKoukuSaydonLogicRuntime::Apply_Result(first, damage, *boss, catalog, nullptr, 220u, events);
		room->Advance_PlayerKnockback(first, .242f);
		tests.Require(wallReady && first.fPositionX > 1.f && first.fPositionX < 1.9f &&
			first.fKnockbackRemainingSeconds == 0.f,
			"Whirlwind push stops at the same swept collision wall as ordinary Server knockback");
		room->m_ServerCollisionSystem.Initialize({}, collisionStatus);
		room->m_ServerNavigation = arenaNavigation;
		const auto* spawn = room->Find_Placement("boss.kakulsaydon.g2.kouku");
		bool foundBoundary = false;
		if (spawn && arenaNavigation.Is_Loaded())
		{
			for (unsigned step = 1u; step < 4000u; ++step)
			{
				const float x = spawn->fPositionX + .25f * step;
				if (arenaNavigation.Is_PointWalkableExact(x, spawn->fPositionZ)) continue;
				SERVER_NAV_POINT ground{};
				if (!arenaNavigation.Sample_Position(x - .25f, spawn->fPositionZ, ground)) break;
				first.fPositionX = x - .25f; first.fPositionY = ground.y; first.fPositionZ = spawn->fPositionZ;
				first.fKnockbackRemainingSeconds = first.fKnockbackSpeed = 0.f;
				boss->fPositionX = first.fPositionX - 1.f; boss->fPositionZ = first.fPositionZ;
				const float before = first.fPositionX;
				CKoukuSaydonLogicRuntime::Apply_Result(first, damage, *boss, catalog, nullptr, 230u, events);
				room->Advance_PlayerKnockback(first, .242f);
				foundBoundary = first.fPositionX < before + 2.f && first.fKnockbackRemainingSeconds == 0.f &&
					arenaNavigation.Is_PointWalkableExact(first.fPositionX, first.fPositionZ);
				break;
			}
		}
		tests.Require(foundBoundary, "Whirlwind push preserves navigation bounds through the existing Server mover");
		// Laser uses the same reaction/mover with the boss yaw sampled at each hit.
		auto laserDamage = damage;
		laserDamage.ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD;
		for (const float yaw : {0.f, 90.f, -135.f, 725.f})
		{
			first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
			first.iCurrentHp = first.iMaximumHp = 1000u; first.isCombatReady = true;
			first.fPositionX = 13.f; first.fPositionZ = -7.f;
			boss->fPositionX = -9.f; boss->fPositionZ = 21.f; boss->fYawDegrees = yaw;
			CKoukuSaydonLogicRuntime::Apply_Result(first, laserDamage, *boss, catalog, nullptr, 600u, events);
			const float radians = yaw * .017453292519943295f;
			tests.Require(first.iCurrentHp == 900u && std::abs(first.fKnockbackDirectionX - std::sin(radians)) < .0001f &&
				std::abs(first.fKnockbackDirectionZ - std::cos(radians)) < .0001f &&
				std::abs(first.fKnockbackSpeed * first.fKnockbackRemainingSeconds - 2.f) < .0001f,
				"Laser damage samples current boss forward regardless of player-relative bearing and preserves push distance/time");
		}
		boss->fPositionX = boss->fPositionZ = 0.f; boss->fYawDegrees = 90.f;
		auto& ellipseWindow = pattern.LogicWindows.front();
		ellipseWindow.bRearmOnExit = ellipseWindow.bRepeatAfterKnockback = false;
		ellipseWindow.OnTimeout.clear(); ellipseWindow.OnSuccess = {damage};
		auto& ellipse = ellipseWindow.CardRegions.front();
		ellipse.bCircle = false; ellipse.bSector = true; ellipse.bReverseSector = true;
		ellipse.fRadiusXM = 1.f; ellipse.fRadiusZM = 5.f; ellipse.fHalfAngleDegrees = 45.f;
		for (const auto& sample : std::array<std::array<float, 3u>, 4u>{{ {3.f, 0.f, 1000.f}, {-3.f, 0.f, 900.f}, {0.f, 2.f, 1000.f}, {-6.f, 0.f, 1000.f} }})
		{
			first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
			first.iCurrentHp = first.iMaximumHp = 1000u; first.isCombatReady = true;
			first.fPositionX = sample[0]; first.fPositionZ = sample[1];
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 650u, ledger); update(650u);
			tests.Require(first.iCurrentHp == static_cast<unsigned>(sample[2]),
				"Authoritative Reverse Sector retains its rotated long axis, safe wedge and short-axis/radial exclusions");
		}
		for (const auto& boundary : std::array<std::array<float, 2u>, 2u>{{ {0.f, 900.f}, {180.f, 1000.f} }})
		{
			first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
			first.iCurrentHp = first.iMaximumHp = 1000u; first.isCombatReady = true; first.fPositionX = 3.f;
			ellipse.fHalfAngleDegrees = boundary[0];
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 700u, ledger); update(700u);
			tests.Require(first.iCurrentHp == static_cast<unsigned>(boundary[1]),
				"Reverse safe angle zero hits the full ellipse and 360 leaves an empty hazard");
		}
	}
#endif
	std::cout << "failures : " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
