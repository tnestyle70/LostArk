#include "ServerGameplayContractTests_Runner.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "KoukuSaydonBrain.h"
#include "ServerApp.h"
#include "PlayerSkillSystem.h"
#include "Gameplay/KoukuMarioBombContract.h"
#include "ServerCombatHitRuntime.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
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



namespace
{
    void Run_SplitBallCylinderDamageContracts(TESTS& tests, const CGameplayCatalog& catalog)
    {
        auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
        boss->iPatternSequence = 1u; boss->iCurrentHp = 100u;
        BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "split.ball.cylinder.contract";
        for (unsigned emission = 0u; emission < 2u; ++emission)
        {
            BOSS_PATTERN_LOGIC_WINDOW window{};
            window.strWindowId = "split.ball.explosion." + std::to_string(emission);
            window.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
            window.iStartMs = 100u; window.iDurationMs = 34u;
            BOSS_PATTERN_LOGIC_RESULT damage{};
            damage.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE; damage.iPercent = 10u;
            window.OnSuccess = {damage};
            BOSS_LOGIC_REGION cylinder{}; cylinder.strRegionId = window.strWindowId + ".cylinder";
            cylinder.bCylinder = true; cylinder.fRadiusM = 2.f;
            cylinder.fHalfX = cylinder.fHalfZ = 2.f; cylinder.fCenterY = cylinder.fHalfY = 1.f;
            auto& track = cylinder.WorldTrack; track.bEnabled = true;
            track.iStartMs = window.iStartMs; track.iDurationMs = window.iDurationMs;
            track.fBaselineX = 10.f + emission; track.fBaselineY = 3.f; track.fBaselineZ = 20.f;
            BOSS_LOGIC_WORLD_TRANSFORM_KEY key{}; track.Keys.push_back(key);
            key.iTimeMs = window.iDurationMs - 1u; track.Keys.push_back(key);
            // Published explosion tracks are visible through 33ms and hidden at 34ms.
            key.iTimeMs = window.iDurationMs; key.bVisible = false; track.Keys.push_back(key);
            window.CardRegions = {cylinder}; pattern.LogicWindows.push_back(window);
        }
        std::map<PLAYER_ID, SERVER_PLAYER> players;
        const auto addPlayer = [&](PLAYER_ID id, float x, float y, float z) {
            auto& player = players[id]; player.iPlayerId = id; player.iNetEntityId = 100u + id;
            player.iCurrentHp = player.iMaximumHp = 13200u; player.isCombatReady = true;
            player.fPositionX = x; player.fPositionY = y; player.fPositionZ = z;
        };
        addPlayer(1u, 8.f, 3.f, 20.f);       // Inside only the first explosion.
        addPlayer(2u, 10.5f, 3.f, 20.f);    // Inside both independent explosions.
        addPlayer(3u, 8.1f, 3.f, 21.9f);   // Inside the enclosing box corner, outside both circles.
        addPlayer(4u, 10.5f, 7.f, 20.f);    // Same XZ overlap, above both cylinders.
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
        std::vector<DAMAGE_EVENT> events;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
        const auto update = [&](unsigned tick) {
            CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
        };
        update(102u);
        tests.Require(events.empty() && players.at(1u).iCurrentHp == 13200u && players.at(2u).iCurrentHp == 13200u,
            "Split-ball damage stays inactive before the authored explosion window");
        update(103u);
        tests.Require(players.at(1u).iCurrentHp == 11880u,
            "One split-ball WORLD cylinder applies 1320 damage from ten percent of 13200 maximum HP through the authoritative result consumer");
        tests.Require(players.at(2u).iCurrentHp == 10560u,
            "Two independent overlapping split-ball explosion windows stack to 2640 damage in one Server tick");
        tests.Require(players.at(3u).iCurrentHp == 13200u && players.at(4u).iCurrentHp == 13200u,
            "Split-ball cylinder rejects the enclosing box corner and finite-height misses after WORLD-track placement");
        tests.Require(events.size() == 3u && std::all_of(events.begin(), events.end(),
            [](const DAMAGE_EVENT& event) { return event.iAmount == 1320u && !event.isOutgoing; }),
            "Split-ball hits each emit an independent incoming 1320 damage event");
        update(103u); players.at(1u).fPositionX = 50.f; update(104u);
        players.at(1u).fPositionX = 8.f; update(104u);
        tests.Require(events.size() == 3u && players.at(1u).iCurrentHp == 11880u && players.at(2u).iCurrentHp == 10560u,
            "A split-ball explosion hits each player once despite duplicate ticks, continued overlap or reentry");
        // One-shot ENTER_AREA judges the ceil-rounded end tick before closing.
        // Keep the outside player still until that final contact sample is consumed.
        update(105u);
        const bool closedWithoutDamage = events.size() == 3u && players.at(3u).iCurrentHp == 13200u &&
            ledger.Windows.size() == 2u && std::all_of(ledger.Windows.begin(), ledger.Windows.end(),
                [](const auto& state) { return state.bClosed; });
        players.at(3u).fPositionX = 10.5f; players.at(3u).fPositionZ = 20.f;
        update(106u); update(107u);
        tests.Require(closedWithoutDamage && events.size() == 3u && players.at(3u).iCurrentHp == 13200u,
            "The 34ms explosion hides before its rounded 30Hz closing tick, closes without damage, and rejects subsequent entry");
    }

    void Run_ColliderMotionAndTickContracts(TESTS& tests, const CGameplayCatalog& catalog)
    {
        auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
        boss->iPatternSequence = 1u; boss->iCurrentHp = 100u;
        BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "collider.motion.contract";
        BOSS_PATTERN_LOGIC_WINDOW window{}; window.strWindowId = "column.contact";
        window.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
        window.iStartMs = 100u; window.iDurationMs = 1000u;
        BOSS_PATTERN_LOGIC_RESULT damage{};
        damage.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE; damage.iPercent = 10u;
        window.OnSuccess = {damage};
        BOSS_LOGIC_REGION column{}; column.strRegionId = "column";
        column.bCylinder = true; column.fRadiusM = 1.f; column.fHalfY = .2f;
        column.fCenterY = -3.f; column.bLinearMotion = true;
        column.iMotionStartMs = 100u; column.iMotionDurationMs = 1000u;
        column.EndPositionOffset = {0.f, 8.f, 0.f}; column.EndScale = {1.f, 1.f, 1.f};
        window.CardRegions = {column}; pattern.LogicWindows = {window};
        std::map<PLAYER_ID, SERVER_PLAYER> players;
        const auto resetPlayer = [&](PLAYER_ID id, float x, float y) {
            auto& player = players[id]; player = {};
            player.iPlayerId = id; player.iNetEntityId = 100u + id;
            player.iCurrentHp = player.iMaximumHp = 100u; player.isCombatReady = true;
            player.fPositionX = x; player.fPositionY = y;
        };
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
        std::vector<DAMAGE_EVENT> events;
        const auto build = [&] {
            events.clear(); output = {};
            CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
        };
        const auto update = [&](unsigned tick) {
            CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
        };
        resetPlayer(1u, 0.f, 0.f); resetPlayer(2u, 3.f, 0.f); resetPlayer(3u, 0.f, 20.f);
        build(); update(102u); update(103u); update(108u);
        tests.Require(events.empty(), "A rising cylinder has no contact before birth or while its top remains below the player's body");
        update(118u); update(119u); update(132u); update(133u);
        tests.Require(players.at(1u).iCurrentHp == 90u && players.at(2u).iCurrentHp == 100u &&
            players.at(3u).iCurrentHp == 100u && events.size() == 1u,
            "A rising cylinder intersects the player's body once at its interpolated height and rejects radial and vertical misses");

        auto& region = pattern.LogicWindows.front().CardRegions.front();
        region.fCenterY = 5.f; region.EndPositionOffset = {0.f, -8.f, 0.f};
        resetPlayer(1u, 0.f, 0.f); build(); update(103u);
        tests.Require(events.empty(), "A descending cylinder starts above the player instead of becoming an infinite-height circle");
        update(118u); update(132u);
        tests.Require(players.at(1u).iCurrentHp == 90u && events.size() == 1u,
            "A descending cylinder uses the same lifetime interpolation and once-per-player trigger");

        region.fCenterY = -5.f; region.fHalfY = .05f; region.iMotionDurationMs = 33u;
        pattern.LogicWindows.front().iDurationMs = 33u;
        region.EndPositionOffset = {0.f, 10.f, 0.f};
        resetPlayer(1u, 0.f, 0.f); build(); update(103u); update(104u);
        tests.Require(players.at(1u).iCurrentHp == 90u && events.size() == 1u,
            "A thin cylinder crossing the entire player body between two Server ticks still hits through its bounded sweep");
        region.iMotionDurationMs = 1000u; pattern.LogicWindows.front().iDurationMs = 1000u;

        region.fCenterY = .5f; region.fHalfY = .5f; region.fRadiusM = .1f;
        region.EndPositionOffset = {0.f, 3.5f, 0.f}; region.EndScale = {30.f, 8.f, 30.f};
        players.clear(); resetPlayer(1u, 2.f, 5.f); build(); update(103u); update(108u);
        tests.Require(events.empty(), "Growing column keeps a narrow and low start volume");
        update(128u); update(132u);
        tests.Require(players.at(1u).iCurrentHp == 90u && events.size() == 1u,
            "Independent end size and end position grow a column upward from its fixed floor and expand its radius");

        region.bLinearMotion = false; region.fCenterY = 1.f; region.fHalfY = 1.f; region.fRadiusM = 2.f;
        auto& contact = pattern.LogicWindows.front(); contact.iRepeatIntervalMs = 200u;
        players.clear(); resetPlayer(1u, 0.f, 0.f); resetPlayer(2u, 20.f, 0.f);
        build(); update(103u); update(103u); update(108u);
        tests.Require(players.at(1u).iCurrentHp == 90u && events.size() == 1u,
            "Tick contact damages immediately on entry and does not repeat on a duplicate or premature Server tick");
        update(109u); players.at(2u).fPositionX = 0.f; update(110u); update(114u);
        tests.Require(players.at(1u).iCurrentHp == 80u && players.at(2u).iCurrentHp == 90u && events.size() == 3u,
            "Tick contact owns an independent authored interval for each player");
        players.at(1u).fPositionX = 20.f; update(115u); update(116u);
        tests.Require(players.at(1u).iCurrentHp == 80u && players.at(2u).iCurrentHp == 80u && events.size() == 4u,
            "A contact tick damages only players still inside and does not punish an exited player");
        update(133u); update(140u);
        tests.Require(events.size() == 4u, "Collider lifetime closes periodic contact without a final out-of-window damage tick");

        contact.iRepeatIntervalMs = 0u; region.fRadiusM = 1.f;
        region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT; region.fCenterX = 0.f;
        region.fCenterZ = 3.f; region.fCenterY = 1.f; boss->fPositionX = 10.f;
        boss->fPositionY = 7.f; boss->fPositionZ = 20.f; boss->fYawDegrees = 90.f;
        players.clear(); resetPlayer(1u, 13.f, 7.f); players.at(1u).fPositionZ = 20.f;
        resetPlayer(2u, 13.f, 0.f); players.at(2u).fPositionZ = 20.f;
        build(); update(103u);
        tests.Require(players.at(1u).iCurrentHp == 90u && players.at(2u).iCurrentHp == 100u,
            "Boss-follow cylinder combines authoritative root yaw and height exactly once");
        region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_START;
        resetPlayer(1u, 1000.f, 7.f); players.at(1u).fPositionZ = 20.f;
        build(); update(103u);
        boss->fPositionX = 100.f; boss->fPositionY = 50.f; boss->fPositionZ = 200.f; boss->fYawDegrees = 0.f;
        players.at(1u).fPositionX = 13.f; update(104u); update(105u);
        tests.Require(players.at(1u).iCurrentHp == 90u && players.at(2u).iCurrentHp == 100u && events.size() == 1u,
            "A fixed boss-anchored cylinder freezes birth position, yaw and height while the boss moves to another location");
        region.iAnchorCaptureStartMs = 0u;
        boss->fPositionX = 10.f; boss->fPositionY = 7.f; boss->fPositionZ = 20.f; boss->fYawDegrees = 90.f;
        resetPlayer(1u, 13.f, 7.f); players.at(1u).fPositionZ = 20.f;
        build(); update(100u);
        tests.Require(events.empty() && !ledger.Windows.front().bOpened && !ledger.Windows.front().FrozenRegions.empty(),
            "A fixed Effect captures its authoritative basis before the later Collider window opens");
        boss->fPositionX = 100.f; boss->fPositionY = 50.f; boss->fPositionZ = 200.f; boss->fYawDegrees = 0.f;
        update(103u);
        tests.Require(players.at(1u).iCurrentHp == 90u && events.size() == 1u,
            "The later Collider uses its linked Effect birth basis after the boss has moved and turned");
        contact.OnSuccess.front().eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER;
        contact.OnSuccess.front().iMarioEntryStage = 1u;
        resetPlayer(1u, boss->fPositionX, boss->fPositionY);
        players.at(1u).fPositionZ = boss->fPositionZ + 3.f;
        players.at(1u).eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
        tests.Require(CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(pattern, *boss, players.at(1u), 3u),
            "A fixed boss-anchored Mario entry resolves against its room-owned frozen entry anchor");
        players.at(1u).fPositionY -= 10.f;
        tests.Require(!CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(pattern, *boss, players.at(1u), 3u),
            "Cylinder Mario entry preserves finite-height exclusion through its specialized admission consumer");
    }
}

void CServerGameplayContractRunner::Run_KoukuPushContracts(TESTS& tests, const CGameplayCatalog& catalog)
{
    {
        S2C_WORLD_SNAPSHOT message{}; message.iServerTick = 100u; message.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
        (void)Try_Parse_GameplayDataRevision(std::string(64u, 'a'), message.ActiveGameplayRevision);
        PLAYER_SNAPSHOT state{}; state.iNetEntityId = 101u; state.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        state.eAction = PLAYER_ACTION_STATE::KNOCKDOWN; state.iActionStartTick = 99u; state.isKnockbackAirborne = true;
        message.Players.push_back(state);
        CPacketWriter writer;
        const bool wrote = Write_Message(writer, message);
        S2C_WORLD_SNAPSHOT decoded{}; CPacketReader reader(writer.Get_Buffer());
        tests.Require(wrote && Read_Message(reader, decoded) && decoded.Players.size() == 1u &&
            decoded.Players.front().isKnockbackAirborne && decoded.Players.front().eAction == PLAYER_ACTION_STATE::KNOCKDOWN,
            "Protocol 110 round-trips authoritative airborne knockdown without changing the action identity");
        message.Players.front().eAction = PLAYER_ACTION_STATE::NONE; message.Players.front().iActionStartTick = 0u;
        CPacketWriter invalidWriter;
        tests.Require(!Write_Message(invalidWriter, message), "Airborne hit-reaction state is rejected outside KNOCKDOWN");
        message.Players.front().eAction = PLAYER_ACTION_STATE::KNOCKDOWN; message.Players.front().iActionStartTick = 99u;
        message.Players.front().isKnockbackAirborne = false;
        CPacketWriter landedWriter;
        tests.Require(Write_Message(landedWriter, message), "The same knockdown occurrence accepts a grounded landing phase");
    }
    {
        auto movementRoom = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
        movementRoom->m_ServerNavigation = CServerNavigation{};
        movementRoom->m_WorldEntities.clear();
        std::string movementStatus;
        movementRoom->m_ServerCollisionSystem.Initialize({}, movementStatus);
        const SERVER_BLOCKING_BODY body{0.f, 0.f, 1.2f, .9f, .9f, 901u};
        movementRoom->m_ServerCollisionSystem.Set_BlockingBodies({body});
        SERVER_PLAYER initial{}; initial.iPlayerId = 1u; initial.iNetEntityId = 101u;
        initial.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        initial.iCurrentHp = initial.iMaximumHp = 100u; initial.isCombatReady = true;
        initial.fPositionX = -5.f;
        auto& mover = movementRoom->m_Players[1u]; mover = initial;
        const auto advance = [&](unsigned count) {
            for (unsigned tick = 0u; tick < count; ++tick) {
                movementRoom->Update_Players(1.f / 30.f); ++movementRoom->m_iServerTick;
            }
        };
        (void)movementRoom->Commit_MoveGoal(mover, 0.f, 0.f);
        advance(120u);
        const auto stopX = mover.fPositionX, stopZ = mover.fPositionZ;
        const auto stoppedRadius = std::hypot(stopX, stopZ);
        std::cout << "body-goal stop: hasGoal=" << mover.hasMoveGoal << " x=" << stopX << " z=" << stopZ << " radius=" << stoppedRadius << " hp=" << mover.iCurrentHp << " action=" << unsigned(mover.eAction) << '\n';
        tests.Require(!mover.hasMoveGoal && mover.MovePath.empty() && stoppedRadius >= 1.649f && stoppedRadius < 2.f,
            "A click inside the contacted boss body ends the move at its boundary instead of rerouting forever");
        advance(60u);
        tests.Require(mover.fPositionX == stopX && mover.fPositionZ == stopZ && !mover.hasMoveGoal,
            "The released body click stays stopped on subsequent authoritative movement ticks");
        (void)movementRoom->Commit_MoveGoal(mover, -5.f, 0.f); advance(120u);
        tests.Require(!mover.hasMoveGoal && std::hypot(mover.fPositionX + 5.f, mover.fPositionZ) < .2f,
            "A fresh click can move away immediately after a blocked body destination");
        mover = initial;
        (void)movementRoom->Commit_MoveGoal(mover, 5.f, 0.f); advance(180u);
        tests.Require(!mover.hasMoveGoal && std::hypot(mover.fPositionX - 5.f, mover.fPositionZ) < .2f,
            "A destination beyond the body preserves the existing tangent slide and reaches the other side");
        mover = initial; mover.hasMoveGoal = true; mover.fMoveGoalX = 0.f; mover.fMoveGoalZ = 0.f;
        tests.Require(!movementRoom->m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(mover, -4.8f, 0.f, 0.f, 0.f),
            "A body containing the goal does not stop an approach before that body is contacted");
        mover.fPositionX = -1.7f;
        tests.Require(movementRoom->m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(mover, -1.5f, 0.f, 0.f, 0.f),
            "The body goal query recognizes the actual sweep into a same-floor occupied destination");
        tests.Require(!movementRoom->m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(mover, -1.5f, 0.f, 0.f, 10.f),
            "A body on a different destination floor does not end the movement goal");
        mover.fMoveGoalX = 5.f;
        tests.Require(!movementRoom->m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(mover, -1.5f, 0.f, 0.f, 0.f),
            "Contact alone cannot discard a walkable destination outside the body");
    }
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
        tests.Require(ordinary.fKnockbackSpeed <= 1.f && p.iCurrentHp == 90u && p.eAction == PLAYER_ACTION_STATE::KNOCKDOWN &&
            p.iFearEndTick == 0u && p.iKnockdownEndTick > 100u && p.iHitReactionGraceEndTick == 0u &&
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
    {
        namespace fs = std::filesystem;
        std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
        const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), DWORD(buffer.size()));
        if (configured && configured < buffer.size()) dataRoot = buffer.data();
        else { GetModuleFileNameW(nullptr, buffer.data(), DWORD(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
        std::ifstream input(dataRoot / L"Gameplay/Gameplay.bootstrap", std::ios::binary);
        std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!bytes.empty() && bytes.back() != '\n') bytes += '\n';
        const std::string encounter = "ENCOUNTER_KAKULSAYDON_G1", id = "KAKULSAYDON_G1_COLLIDER_CONTRACT";
        const std::string prefix = encounter + "\t" + id;
        bytes += "PATTERN\t" + prefix + "\t" + id + ".action\tAUDITION_ONLY\t0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0\n";
        bytes += "PATTERNBOSS\t" + prefix + "\tBOSS_KAKULSAYDON_G1_SAYDON\n";
        bytes += "PATTERNPOLICY\t" + prefix + "\tNORMAL\t1\t1\tNONE\tNONE\n";
        bytes += "PATTERNSOURCE\t" + prefix + "\t1\t0\t0\t0\t0\t0\t0\n";
        bytes += "PATTERNSTAGE\t" + prefix + "\t0\tSTAGE_1\t" + id + ".stage.1\tACTIVE\t5000\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
        bytes += "PATTERNSTAGEBRANCH\t" + prefix + "\t" + id + ".stage.1\tTIMEOUT\t-\n";
        bytes += "PATTERNTARGET\t" + id + "\tGATE1\tboss.kakulsaydon.g1.saydon\n";
        bytes += "PATTERNLOGIC\t" + prefix + "\t0\tcolumn.hit\tENTER_AREA\t100\t1000\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\n";
        bytes += "PATTERNLOGICTICK\t" + prefix + "\tcolumn.hit\t200\n";
        bytes += "PATTERNLOGICREGION\t" + prefix + "\tcolumn.hit\t0\tcolumn.body\tBOSS_START\tCYLINDER\t1\t0.25\t3\t0\t2\t0.25\t2\t4\t45\tNONE\tNONE\n";
        bytes += "PATTERNLOGICREGIONCAPTURE\t" + prefix + "\tcolumn.hit\t0\t0\n";
        bytes += "PATTERNLOGICREGIONMOTION\t" + prefix + "\tcolumn.hit\t0\t0\t3.75\t4\t1.5\t16\t1.5\n";
        bytes += "PATTERNLOGICOUTCOME\t" + prefix + "\tcolumn.hit\tSUCCESS\t0\tMAX_HP_PERCENT_DAMAGE\t10\t0\t-\n";
        bytes += "PATTERNLOGICPUSH\t" + prefix + "\tcolumn.hit\tSUCCESS\t0\t0\t1500\tAWAY_FROM_BOSS\t0\t0\t0\t1\t4\n";
        const auto headerEnd = bytes.find('\n'), headerCount = bytes.rfind('\t', headerEnd);
        bytes.replace(headerCount + 1u, headerEnd - headerCount - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
        const auto directory = fs::temp_directory_path() / (L"LostArkColliderContract-" + std::to_wstring(GetCurrentProcessId()));
        std::error_code error; fs::create_directories(directory, error); const auto path = directory / L"Gameplay.bootstrap";
        auto generation = std::make_shared<CGameplayCatalog>(); GameplayDataRevision revision;
        const auto load = [&](const std::string& content) {
            { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(content.data(), content.size()); }
            GameplayDataRevision hash;
            return !error && CServerApp::Hash_GameplayFileForAdmission(path, hash, status) &&
                generation->Load_FromBootstrap(fs::canonical(path), hash, hash);
        };
        const bool loaded = load(bytes);
        if (!loaded) std::cout << "[ColliderCatalog] " << generation->Get_Status() << '\n';
        const auto* parsed = loaded ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, id, status) : nullptr;
        bool fields = parsed && parsed->LogicWindows.size() == 1u &&
            parsed->LogicWindows.front().CardRegions.size() == 1u && parsed->LogicWindows.front().OnSuccess.size() == 1u;
        if (fields) {
            const auto& w = parsed->LogicWindows.front(); const auto& r = w.CardRegions.front(); const auto& hit = w.OnSuccess.front();
            fields = w.iRepeatIntervalMs == 200u && r.bCylinder && r.bLinearMotion &&
                r.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_START && r.iAnchorCaptureStartMs == 0u && r.iMotionStartMs == 100u && r.iMotionDurationMs == 1000u &&
                r.EndPositionOffset == std::array<float, 3u>{0.f, 3.75f, 4.f} && r.EndScale == std::array<float, 3u>{1.5f, 16.f, 1.5f} &&
                hit.fPushRangeM == 0.f && hit.fPushHeightM == 4.f && hit.bPushBallistic && !hit.bPushCanLeaveArena;
        }
        tests.Require(fields, "Actual hashed bootstrap admission consumes cylinder, birth anchor, motion, contact tick and pure vertical launch fields");
        if (fields) {
            std::string rising = bytes;
            const auto start = rising.find("PATTERNATTACKHIT\tENCOUNTER_KAKULSAYDON_G1\tKAKULSAYDON_G1_PATTERN_39\t");
            auto end = start == std::string::npos ? start : rising.find('\n', start);
            if (end != std::string::npos && end > start && rising[end - 1u] == '\r') --end;
            bool admitsRise = end != std::string::npos;
            if (admitsRise) {
                const auto line = rising.substr(start, end - start);
                const auto columns = std::count(line.begin(), line.end(), '\t') + 1;
                if (columns == 25) rising.insert(end, "\t3\t1200");
                else if (columns == 27 || columns == 28 || columns == 30) {
                    // Retain optional horizontal push/force/direction columns.
                    std::istringstream source(line); std::string field; std::vector<std::string> values;
                    while (std::getline(source, field, '\t')) values.push_back(field);
                    values[25] = "3"; values[26] = "1200";
                    std::string replacement;
                    for (const auto& value : values) { if (!replacement.empty()) replacement += '\t'; replacement += value; }
                    rising.replace(start, end - start, replacement);
                } else admitsRise = false;
            }
            admitsRise = admitsRise && load(rising);
            const auto* albion = admitsRise ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, "KAKULSAYDON_G1_PATTERN_39", status) : nullptr;
            bool foundRise = false;
            if (albion) for (const auto& trigger : albion->MechanicTriggers)
                for (const auto& hit : trigger.FixedHits)
                    foundRise |= hit.fRiseHeightM == 3.0 && hit.iPushMs == 1200u;
            tests.Require(foundRise, "Native bootstrap admits optional Albion rise height and flight duration on the existing attack template");
            tests.Require(load(bytes), "Legacy attack rows still admit with no rise reaction and preserve other Collider fields");
            revision = generation->Get_ActiveRevision();
            std::string invalid = bytes; const auto motion = invalid.rfind("\t1.5\t16\t1.5\n");
            invalid.replace(motion, std::string("\t1.5\t16\t1.5\n").size(), "\t1.5\t0\t1.5\n");
            tests.Require(!load(invalid) && generation->Get_ActiveRevision() == revision,
                "Malformed end-size supplement preserves the previously admitted gameplay generation");
            invalid = bytes;
            const std::string circleTail = "\t4\t45\tNONE\tNONE\n";
            invalid.replace(invalid.rfind(circleTail), circleTail.size(), "\t4\t45\tNONE\tNONE\t1\n");
            tests.Require(!load(invalid) && generation->Get_ActiveRevision() == revision,
                "Native cylinder admission rejects unsupported hollow geometry and preserves its previous catalog");
            room->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch = 1u;
            room->m_KoukuSaydonPatternAudition.Request.Scope.strEncounterId = encounter;
            room->m_KoukuSaydonPatternAudition.Request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
            room->m_KoukuSaydonPatternAudition.pProductGeneration = generation;
            room->m_KoukuSaydonPatternAudition.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*generation);
            room->m_KoukuSaydonPatternAudition.PinnedGameplayRevision = revision;
            tests.Require(room->Resolve_KoukuProductCatalog() == generation.get(), "Gate 1 fixture retains its exact admitted Product generation and revision");
            auto member = std::make_unique<CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER>(); member->PatternIds = {id};
            room->m_KoukuSaydonPatternAudition.Members.push_back(std::move(*member));
            WORLD_BOOTSTRAP_PLACEMENT fence{}; fence.strPlacementId = "gate1.flight.fence";
            fence.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX; fence.fPositionX = 3.f;
            fence.fHalfExtentX = .1f; fence.fHalfExtentY = fence.fHalfExtentZ = 10.f;
            room->m_ServerCollisionSystem.Initialize({fence}, status);
            auto p = player(); auto flight = push; flight.fPushRangeM = 6.f; flight.iPushMs = 1000u;
            flight.bPushBallistic = flight.bPushCanLeaveArena = true; flight.fPushHeightM = 4.f;
            CKoukuSaydonLogicRuntime::Apply_Result(p, flight, *boss, catalog, nullptr, 100u, events);
            for (unsigned tick = 0u; tick < 31u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
            tests.Require(p.fPositionX > 1.f && p.fPositionX < 2.9f && std::abs(p.fPositionY) < .001f &&
                !p.bKnockbackBallistic && p.eAction != PLAYER_ACTION_STATE::FALLING,
                "Single-pattern Gate 1 audition resolves its pinned member gate and fences a legacy arena-exit launch with empty request scope");
            room->m_KoukuSaydonPatternAudition = {};
            room->m_ServerCollisionSystem.Initialize({}, status);
        }
        fs::remove(path, error); fs::remove(directory, error);
    }
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
        for (const float height : {.5f, 4.f}) for (const float distance : {0.f, 6.f})
        {
            p = player(); auto bounded = effect;
            bounded.fPushRangeM = distance; bounded.iPushMs = 1000u;
            bounded.fPushHeightM = height; bounded.bPushCanLeaveArena = false;
            CKoukuSaydonLogicRuntime::Apply_Result(p, bounded, *boss, catalog, nullptr, 100u, events, &contactCenter);
            room->Advance_PlayerKnockback(p, .5f);
            tests.Require(std::abs(p.fPositionY - height) < .0001f && std::abs(p.fPositionX - (1.f + distance * .5f)) < .0001f,
                "Authored launch height reaches its apex halfway through the authored flight duration");
            room->Advance_PlayerKnockback(p, .5f);
            tests.Require(std::abs(p.fPositionY) < .0001f && std::abs(p.fPositionX - (1.f + distance)) < .0001f &&
                !p.bKnockbackBallistic && p.eAction != PLAYER_ACTION_STATE::FALLING,
                "Bounded ballistic launch returns to its floor after independent distance/height/time tuning");
        }
        WORLD_BOOTSTRAP_PLACEMENT fence{}; fence.strPlacementId = "ballistic.fence";
        fence.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX; fence.fPositionX = 3.f;
        fence.fHalfExtentX = .1f; fence.fHalfExtentY = fence.fHalfExtentZ = 10.f;
        const bool fenceReady = room->m_ServerCollisionSystem.Initialize({fence}, status);
        p = player(); auto fenced = effect; fenced.fPushRangeM = 6.f; fenced.iPushMs = 1000u;
        fenced.fPushHeightM = 4.f; fenced.bPushCanLeaveArena = false;
        CKoukuSaydonLogicRuntime::Apply_Result(p, fenced, *boss, catalog, nullptr, 100u, events, &contactCenter);
        for (unsigned tick = 0u; tick < 31u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
        tests.Require(fenceReady && p.fPositionX > 1.f && p.fPositionX < 2.9f && std::abs(p.fPositionY) < .001f &&
            p.fKnockbackRemainingSeconds == 0.f && p.eAction != PLAYER_ACTION_STATE::FALLING,
            "A bounded ballistic launch is fenced in XZ while completing its rise and fall");
        room->m_ServerCollisionSystem.Initialize({}, status);
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
        namespace Bomb = LostArk::Shared::KoukuMarioBomb;
        for (const auto& binding : Bomb::BINDINGS)
        {
            const auto* marker = room->Find_Placement(binding.marker);
            const auto* arrival = room->Find_Placement(binding.arrival);
            const auto* exit = room->Find_Placement(binding.exit);
            const bool ready = marker && arrival && exit && arrival->TriggerActions.size() == 1u;
            tests.Require(ready, "Mario bomb contact consumes the actual published disabled marker and lane");
            if (!ready) continue;
            const auto& lane = arrival->TriggerActions.front();
            const auto distance = [&](float x, float z) { return std::hypot(double(x) - marker->fPositionX, double(z) - marker->fPositionZ); };
            const bool towardArrival = distance(lane.fTargetX, lane.fTargetZ) > distance(exit->fPositionX, exit->fPositionZ);
            const float endX = towardArrival ? lane.fTargetX : exit->fPositionX;
            const float endZ = towardArrival ? lane.fTargetZ : exit->fPositionZ;
            const double length = distance(endX, endZ);
            const auto duration = static_cast<unsigned>(std::ceil(length / Bomb::SPEED_MPS * 1000.));
            const auto seed = Bomb::Seed(binding.marker), phase = seed % Bomb::INTERVAL_MS;
            for (unsigned generation : {100u, 101u})
            {
                const auto birthMs = generation * Bomb::INTERVAL_MS + phase;
                const auto tick = (birthMs + 1000u) * 30u / 1000u + 1u;
                const float fraction = static_cast<float>((double(tick) * (1000. / 30.) - birthMs) / duration);
                auto victim = player(); victim.iMarioStage = binding.stage;
                victim.fPositionX = marker->fPositionX + (endX - marker->fPositionX) * fraction;
                victim.fPositionZ = marker->fPositionZ + (endZ - marker->fPositionZ) * fraction;
                victim.fPositionY = marker->fPositionY;
                const auto expectedX = victim.fPositionX, expectedZ = victim.fPositionZ;
                room->m_TickDamageEvents.clear();
                room->Update_MarioBombContacts(victim, tick);
                room->Update_MarioBombContacts(victim, tick);
                room->Update_MarioBombContacts(victim, tick + 1u);
                tests.Require(victim.iCurrentHp == 90u && room->m_TickDamageEvents.size() == 1u &&
                    victim.eAction == PLAYER_ACTION_STATE::KNOCKDOWN && victim.bKnockbackBallistic &&
                    victim.fKnockbackRemainingSeconds > .99f,
                    "Every Mario emitter generation hits once and launches the player through authoritative knockdown");
                room->Advance_PlayerKnockback(victim, .5f);
                tests.Require(std::abs(victim.fPositionY - marker->fPositionY - 2.f) < .001f,
                    "Mario bomb launch reaches its two-metre apex during the knockdown animation");
                room->Advance_PlayerKnockback(victim, .5f);
                tests.Require(std::abs(victim.fPositionY - marker->fPositionY) < .001f &&
                    std::abs(std::hypot(victim.fPositionX - expectedX, victim.fPositionZ - expectedZ) - 4.f) < .001f &&
                    victim.fKnockbackRemainingSeconds == 0.f,
                    "Mario bomb stops at its four-metre endpoint and lands instead of continuing to drift");
                auto airborne = player(); airborne.iMarioStage = binding.stage;
                airborne.fPositionX = expectedX; airborne.fPositionZ = expectedZ; airborne.fPositionY = marker->fPositionY + 4.f;
                room->Update_MarioBombContacts(airborne, tick);
                tests.Require(airborne.iCurrentHp == 100u, "A jump above the flying bomb's volume avoids its contact");
            }
        }
        auto noPush = SERVER_WORLD_TO_PLAYER_HIT{};
        Configure_MarioHazardLaunch(noPush);
        tests.Require(!noPush.bPushBallistic, "Unconfigured ordinary attacks do not acquire Mario launch motion");
    }
    // P85 uses the same first-contact path for both visible beam occurrences.
    for (const unsigned startMs : {3083u, 5222u})
    for (const unsigned entryDelayTicks : {0u, 1u, 20u, 45u})
    for (const auto priorAction : {PLAYER_ACTION_STATE::NONE, PLAYER_ACTION_STATE::FEAR, PLAYER_ACTION_STATE::KNOCKDOWN})
    {
        BOSS_PATTERN_DEFINITION laser{}; laser.strPatternId = "bazooka.first.contact";
        BOSS_PATTERN_LOGIC_WINDOW contact{}; contact.strWindowId = "bazooka.contact";
        contact.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
        contact.iStartMs = startMs; contact.iDurationMs = 1634u;
        auto hit = push; hit.fPushRangeM = 12.f; hit.iPushMs = 1200u;
        hit.bForcePush = hit.bPushBallistic = true; hit.fPushHeightM = 3.f;
        hit.ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD; hit.fPushYawOffsetDegrees = 90.f;
        contact.OnSuccess = {hit};
        BOSS_LOGIC_REGION region{}; region.strRegionId = "bazooka.box";
        region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
        region.fCenterX = 8.4f; region.fCenterY = .35f;
        region.fHalfX = 7.951f; region.fHalfY = .509209f; region.fHalfZ = .314871f;
        contact.CardRegions = {region}; laser.LogicWindows = {contact};
        std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, player()}};
        auto& victim = players.at(1u); victim.fPositionX = 8.4f;
        victim.eAction = priorAction; victim.iHitReactionGraceEndTick = 10000u;
        victim.fKnockbackRemainingSeconds = priorAction == PLAYER_ACTION_STATE::KNOCKDOWN ? 1.f : 0.f;
        KOUKUSAYDON_LOGIC_LEDGER ledger{}; KOUKUSAYDON_LOGIC_OUTPUT output{};
        std::vector<DAMAGE_EVENT> damage;
        CKoukuSaydonLogicRuntime::Build(laser, *boss, 100u, ledger);
        const auto birth = 100u + (startMs * 30u + 999u) / 1000u;
        const auto update = [&](unsigned tick) { CKoukuSaydonLogicRuntime::Update(*boss, laser,
            ledger, players, catalog, nullptr, tick, damage, output); };
        update(birth - 1u);
        const bool inactiveBeforeBirth = damage.empty() && victim.iCurrentHp == 100u;
        if (entryDelayTicks) { victim.fPositionZ = 20.f; update(birth); victim.fPositionZ = 0.f; }
        update(birth + entryDelayTicks); update(birth + entryDelayTicks + 1u);
        tests.Require(inactiveBeforeBirth && victim.iCurrentHp == 90u && damage.size() == 1u &&
            victim.eAction == PLAYER_ACTION_STATE::KNOCKDOWN && victim.bKnockbackBallistic &&
            victim.fKnockbackRemainingSeconds > 1.19f && victim.fKnockbackDirectionX > .999f,
            "Both bazooka shots hit initial overlap or later visible entry once, replacing fear/down/grace with the authored launch");
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
        const auto capture = [&](float x, float y, float z, bool grip, bool sweep, bool hidden, bool clown = false) {
            auto p = player(); p.fPositionX = x; p.fPositionY = y; p.fPositionZ = z;
            if (clown) p.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
            auto definition = hookPattern(grip, sweep, hidden);
            auto owner = std::make_unique<SERVER_WORLD_ENTITY>(); owner->iNetEntityId = 201u; owner->iPatternSequence = 3u;
            std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, p}};
            KOUKUSAYDON_LOGIC_LEDGER ledger{}; KOUKUSAYDON_LOGIC_OUTPUT output{}; std::vector<DAMAGE_EVENT> damage;
            CKoukuSaydonLogicRuntime::Build(definition, *owner, 100u, ledger);
            CKoukuSaydonLogicRuntime::Update(*owner, definition, ledger, players, catalog, nullptr,
                sweep ? 101u : 100u, damage, output);
            const auto& caught = players.at(1u);
            return caught.eAction == PLAYER_ACTION_STATE::GRABBED && caught.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP &&
                caught.iNetEntityId == p.iNetEntityId && caught.eMadnessForm == p.eMadnessForm;
        };
        tests.Require(capture(0.f, 1.3f, 0.f, true, false, false), "Hook body contact captures even when the feet miss its vertical box");
        tests.Require(capture(0.f, 1.3f, 0.f, true, false, false, true), "Hook captures a transformed Mario entrant through the same authoritative player body");
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
    {
        // These are the installed baked source tracks, including P33's split carrier.
        // The synthetic result tests use an empty catalog; the room owns the
        // admitted Product generation containing these installed hook tracks.
        const auto& sourceCatalog = room->m_GameplayCatalog.Active();
        const auto* patterns = sourceCatalog.Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
        tests.Require(patterns != nullptr, "Hook source regression reads the room's admitted Product catalog");
        std::size_t checked = 0u, ascents = 0u;
        if (patterns) for (const auto& source : *patterns)
        {
            if (source.strPatternId != "KAKULSAYDON_G1_PATTERN_18" &&
                source.strPatternId != "KAKULSAYDON_G1_PATTERN_19" &&
                source.strPatternId != "KAKULSAYDON_G1_PATTERN_33") continue;
            for (const auto& window : source.LogicWindows) for (const auto& region : window.CardRegions)
            {
                const auto& track = region.WorldTrack;
                if (!track.bEnabled || track.Keys.empty() || !track.Keys.front().bHasGripPosition) continue;
                const bool rises = track.Keys.back().GripPosition[1] > 10.f;
                auto last = track.Keys.size() - 1u;
                while (last && !track.Keys[last].bVisible) --last;
                // P18/P19's staggered starts shift the baked 30 Hz samples. These
                // measured times are independent expectations, not a copy of the resolver.
                const auto phase = track.iStartMs % 1000u;
                const auto ascentMs = source.strPatternId == "KAKULSAYDON_G1_PATTERN_33" ? 1701u :
                    (phase == 175u || phase == 875u ? 6542u : phase == 350u ? 6550u : phase == 525u ? 6558u : 6567u);
                const auto releaseMs = rises ? ascentMs : track.Keys[last].iTimeMs;
                tests.Require(region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD,
                    "Installed hook grip tracks are baked in world space");
                const auto boundary = std::find_if(track.Keys.begin(), track.Keys.end(),
                    [releaseMs](const auto& key) { return key.iTimeMs == releaseMs; });
                tests.Require(boundary != track.Keys.end(), "Installed hook retains the measured source ascent boundary");
                if (boundary == track.Keys.end()) continue;
                SERVER_NAV_POINT floor{};
                // Independent expected floor uses the ordinary spawn projection.
                // Some baked grip endpoints lie over unbaked cells whose height
                // cannot be used as a same-level reference.
                const bool grounded = arenaNavigation.Project_Point(boundary->GripPosition[0],
                    boundary->GripPosition[2], floor, boundary->GripPosition[1]);
                tests.Require(grounded && std::abs(floor.y - boundary->GripPosition[1]) <= 1.8f &&
                    std::hypot(floor.x - boundary->GripPosition[0], floor.z - boundary->GripPosition[2]) <= 1.8f &&
                    arenaNavigation.Is_InSameNavigationGrid(boundary->GripPosition[0], boundary->GripPosition[2], floor.x, floor.z),
                    "Every installed hook release boundary admits the nearby arena floor, never its final airborne height");
                if (!grounded) continue;
                BOSS_PATTERN_DEFINITION definition{}; definition.strPatternId = "hook.release.source";
                auto carrier = window; carrier.OnSuccess.clear(); carrier.OnFail.clear(); carrier.CardRegions = {region};
                definition.LogicWindows = {carrier};
                auto owner = std::make_unique<SERVER_WORLD_ENTITY>(); owner->iNetEntityId = 201u; owner->iPatternSequence = 3u;
                auto hanging = player(); hanging.eAction = PLAYER_ACTION_STATE::GRABBED;
                hanging.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
                hanging.iAttachmentOwnerNetEntityId = owner->iNetEntityId;
                hanging.iAttachmentPatternSequence = owner->iPatternSequence;
                hanging.iAttachmentWindowIndex = hanging.iAttachmentRegionIndex = 0u;
                hanging.fPositionX = boundary->GripPosition[0]; hanging.fPositionY = boundary->GripPosition[1];
                hanging.fPositionZ = boundary->GripPosition[2]; hanging.iAttachmentReleaseTick = 100000u;
                std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, hanging}};
                KOUKUSAYDON_LOGIC_LEDGER sourceLedger{}; KOUKUSAYDON_LOGIC_OUTPUT sourceOutput{};
                std::vector<DAMAGE_EVENT> sourceDamage;
                CKoukuSaydonLogicRuntime::Build(definition, *owner, 100u, sourceLedger);
                const auto releaseTick = 100u + CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs) +
                    static_cast<std::uint32_t>(std::ceil((releaseMs / double(track.fPlaybackSpeed) + track.iStartDelayMs) * .03));
                const auto update = [&](std::uint32_t tick, const CServerNavigation* nav) {
                    CKoukuSaydonLogicRuntime::Update(*owner, definition, sourceLedger, players, sourceCatalog,
                        nullptr, tick, sourceDamage, sourceOutput, nav);
                };
                update(releaseTick - 1u, &arenaNavigation);
                tests.Require(players.at(1u).iAttachmentReleaseTick > releaseTick,
                    "Hook still carries its player on the tick before the source release boundary");
                update(releaseTick, &arenaNavigation);
                auto& dropped = players.at(1u);
                tests.Require(dropped.iAttachmentReleaseTick == releaseTick && dropped.fPositionX == floor.x &&
                    dropped.fPositionY == floor.y && dropped.fPositionZ == floor.z,
                    "Hook source boundary stages a ground position and releases before its final ascent");
                tests.Require(room->Release_PlayerAttachment(dropped, owner->iNetEntityId, 0.f, 0u, false, 0u, releaseTick) &&
                    dropped.eAction == PLAYER_ACTION_STATE::NONE && dropped.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::NONE &&
                    dropped.isCombatReady && dropped.fPositionY == floor.y,
                    "Existing room release restores movement/combat on the admitted hook endpoint floor");
                update(releaseTick + 15u, &arenaNavigation);
                tests.Require(dropped.fPositionY == floor.y, "Released player no longer follows the upward hook");
                if (rises && ascents == 0u)
                {
                    dropped = hanging;
                    CServerNavigation unavailable;
                    update(releaseTick, &unavailable);
                    tests.Require(!CKoukuSaydonLogicRuntime::Has_ReachedTick(releaseTick + 1u, dropped.iAttachmentReleaseTick) &&
                        dropped.eAction == PLAYER_ACTION_STATE::GRABBED && dropped.fPositionY == boundary->GripPosition[1],
                        "Missing floor retains the boundary through the next room pre-logic attachment update");
                    update(releaseTick + 1u, &arenaNavigation);
                    tests.Require(dropped.iAttachmentReleaseTick == releaseTick + 1u && dropped.fPositionY == floor.y,
                        "Restored floor admission releases the pinned hook endpoint on retry");

                    // Level horizontal travel must finish before the final vertical exit.
                    auto& flatWindow = definition.LogicWindows.front();
                    flatWindow.iStartMs = 0u; flatWindow.iDurationMs = 3000u;
                    auto& flatTrack = flatWindow.CardRegions.front().WorldTrack;
                    flatTrack.iStartMs = flatTrack.iStartDelayMs = 0u;
                    flatTrack.iDurationMs = 3000u; flatTrack.fPlaybackSpeed = 1.f;
                    flatTrack.Keys.assign(4u, *boundary);
                    for (std::size_t index = 0u; index < flatTrack.Keys.size(); ++index)
                        flatTrack.Keys[index].iTimeMs = static_cast<std::uint32_t>(index) * 1000u;
                    flatTrack.Keys[0].GripPosition[0] += 5.f;
                    flatTrack.Keys[1].GripPosition[0] += 2.f;
                    flatTrack.Keys[3].GripPosition[1] += 5.f;
                    dropped = hanging;
                    owner->fPositionX = 500.f; owner->fPositionY = 50.f;
                    owner->fPositionZ = -300.f; owner->fYawDegrees = 90.f;
                    CKoukuSaydonLogicRuntime::Build(definition, *owner, 100u, sourceLedger);
                    update(159u, &arenaNavigation);
                    tests.Require(dropped.iAttachmentReleaseTick > 160u,
                        "A flat horizontal hook approach does not release before its last level key");
                    update(160u, &arenaNavigation);
                    tests.Require(dropped.iAttachmentReleaseTick == 160u && dropped.fPositionX == floor.x &&
                        dropped.fPositionY == floor.y && dropped.fPositionZ == floor.z,
                        "Vertical exit releases at the level approach endpoint without applying the moving boss basis");
                    for (auto& key : flatTrack.Keys) key.GripPosition[1] += 10.f;
                    dropped = hanging;
                    CKoukuSaydonLogicRuntime::Build(definition, *owner, 100u, sourceLedger);
                    update(160u, &arenaNavigation);
                    tests.Require(!CKoukuSaydonLogicRuntime::Has_ReachedTick(161u, dropped.iAttachmentReleaseTick) &&
                        dropped.fPositionY == boundary->GripPosition[1] + 10.f,
                        "A nearby XZ projection onto a different-height deck cannot release the hook player");
                }
                ++checked; if (rises) ++ascents;
            }
        }
        tests.Require(checked == 66u && ascents == 51u,
            "All 66 installed P18/P19/P33 carriers include 51 terminal-ascent releases and 15 split-track endings");
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
            auto effect = push; effect.fPushRangeM = distance; effect.bPushCanLeaveArena = false;
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
                for (unsigned tick = 0u; tick < 60u && p.iCurrentHp; ++tick)
                    room->Update_PlayerFall(p, 1.f / 30.f, 102u + tick);
                tests.Require(p.iCurrentHp == 0u && p.eAction == PLAYER_ACTION_STATE::DEAD &&
                    p.fPositionY <= p.fFallDeathPlaneY, "Arena-exit push dies only after crossing its arena fall plane");
            }
        }
        for (const std::uint8_t gate : {std::uint8_t(1u), std::uint8_t(3u)})
        {
            room->m_GateProgress.iCurrentGate = gate;
            for (const bool ballistic : {false, true})
            {
                auto p = player(); p.fPositionX = lastGroundX; p.fPositionY = edgeGround.y; p.fPositionZ = 322.94f;
                boss->fPositionX = p.fPositionX - 1.f; boss->fPositionZ = p.fPositionZ;
                auto effect = push; effect.fPushRangeM = 6.f; effect.iPushMs = 1000u;
                effect.bPushBallistic = ballistic; effect.fPushHeightM = ballistic ? 3.f : 0.f;
                CKoukuSaydonLogicRuntime::Apply_Result(p, effect, *boss, catalog, nullptr, 100u, events);
                for (unsigned tick = 0u; tick < 31u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
                tests.Require(p.iCurrentHp == 90u && p.eAction != PLAYER_ACTION_STATE::FALLING &&
                    p.fKnockbackRemainingSeconds == 0.f && arenaNavigation.Is_PointWalkableExact(p.fPositionX, p.fPositionZ) &&
                    std::abs(p.fPositionY - edgeGround.y) <= 1.f,
                    "Gate 1 and Gate 3 fence both ordinary and ballistic authored arena-exit pushes at the supported deck");
            }
        }
        room->m_GateProgress.iCurrentGate = 0u;
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
    {
        SERVER_NAV_POINT chair{}, arenaFloor{}; bool foundChair = false, foundFloor = false;
        for (float z = 307.25f; z < 332.5f && !(foundChair && foundFloor); z += .5f)
            for (float x = -4.25f; x < 20.5f; x += .5f)
            {
                SERVER_NAV_POINT surface{};
                if (!arenaNavigation.Sample_SurfacePosition(x, z, surface)) continue;
                if (!foundChair && std::abs(surface.y - 6.51f) < .02f) { chair = surface; foundChair = true; }
                if (!foundFloor && std::abs(surface.y - 10.56f) < .02f) { arenaFloor = surface; foundFloor = true; }
            }
        tests.Require(foundChair && foundFloor, "Published Gate 2 navigation retains the 6.51-metre chair below its 10.56-metre combat floor");
        if (foundChair && foundFloor)
        {
            room->m_GateProgress.iCurrentGate = 2u;
            auto p = player(); p.fPositionX = chair.x; p.fPositionZ = chair.z; p.fPositionY = arenaFloor.y;
            p.bKnockbackBallistic = p.bKnockbackCanLeaveArena = true;
            p.fKnockbackLaunchY = p.fKnockbackSupportY = arenaFloor.y;
            p.fKnockbackRemainingSeconds = 2.f; p.fKnockbackVelocityY = -6.f; p.fKnockbackGravityMps2 = 12.f;
            for (unsigned tick = 0u; tick < 60u && p.iCurrentHp; ++tick)
            {
                room->m_iServerTick = 100u + tick;
                room->Advance_PlayerKnockback(p, 1.f / 30.f);
                room->Update_PlayerFall(p, 1.f / 30.f, 101u + tick);
            }
            tests.Require(!p.iCurrentHp && p.eAction == PLAYER_ACTION_STATE::DEAD && p.fPositionY > chair.y &&
                std::abs(p.fFallDeathPlaneY - (arenaFloor.y - 1.f)) < .001f,
                "Gate 2 descent dies below the combat support before a lower chair can become a safe landing");
            p = player(); p.fPositionX = arenaFloor.x; p.fPositionZ = arenaFloor.z; p.fPositionY = arenaFloor.y;
            p.bKnockbackBallistic = p.bKnockbackCanLeaveArena = true;
            p.fKnockbackLaunchY = p.fKnockbackSupportY = arenaFloor.y;
            p.fKnockbackRemainingSeconds = 1.f; p.fKnockbackVelocityY = 3.f; p.fKnockbackGravityMps2 = 6.f;
            for (unsigned tick = 0u; tick < 40u; ++tick) room->Advance_PlayerKnockback(p, 1.f / 30.f);
            tests.Require(p.iCurrentHp && !p.bKnockbackBallistic && std::abs(p.fPositionY - arenaFloor.y) < .001f,
                "Gate 2 launch landing back on its original supported floor remains alive");
            room->m_GateProgress.iCurrentGate = 0u;
        }
    }
    {
        auto p = player(); p.fPositionY = 20.f;
        room->Begin_PlayerFall(p, 0.f, 100u);
        p.fPositionY = 15.01f;
        room->Update_PlayerFall(p, 0.f, 1000u);
        tests.Require(p.iCurrentHp == 100u && p.eAction == PLAYER_ACTION_STATE::FALLING &&
            std::abs(p.fFallDeathPlaneY - 15.f) < .0001f,
            "Kouku stays alive above its five-metre death plane even after the old timer deadline");
        p.fPositionY = 15.f;
        room->Update_PlayerFall(p, 0.f, 1001u);
        tests.Require(p.iCurrentHp == 0u && p.eAction == PLAYER_ACTION_STATE::DEAD,
            "Kouku dies exactly when its feet reach the support height minus five metres");
        p = player(); p.fPositionY = 17.f; p.bKnockbackBallistic = true; p.fKnockbackLaunchY = p.fKnockbackSupportY = 20.f;
        room->Begin_PlayerFall(p, 0.f, 100u);
        tests.Require(std::abs(p.fFallDeathPlaneY - 15.f) < .0001f,
            "Ballistic handoff retains its original launch floor instead of lowering the death plane again");
        p = player(); p.fPositionY = 20.f;
        CPlayerSkillSystem::Arm_PlayerHitReaction(p, 0.f, 0.f, 0.f, 1500u, false, 0u, 100u, true, true, true, 3.f);
        p.fPositionY = 23.f;
        CPlayerSkillSystem::Arm_PlayerHitReaction(p, 0.f, 0.f, 0.f, 1500u, false, 0u, 120u, true, true, true, 3.f);
        room->Begin_PlayerFall(p, 0.f, 140u);
        tests.Require(std::abs(p.fFallDeathPlaneY - 15.f) < .0001f,
            "Repeated airborne force pushes keep the first support floor instead of raising the death plane");
        const auto nav = room->m_ServerNavigation;
        room->m_ServerNavigation = CServerNavigation{};
        p = player(); p.fPositionY = 15.01f; p.fKnockbackLaunchY = p.fKnockbackSupportY = 20.f;
        p.bKnockbackBallistic = p.bKnockbackCanLeaveArena = true;
        p.fKnockbackRemainingSeconds = 1.f; p.fKnockbackVelocityY = -1.f;
        room->Advance_PlayerKnockback(p, 1.f / 30.f);
        tests.Require(p.iCurrentHp == 0u && p.eAction == PLAYER_ACTION_STATE::DEAD && !p.bKnockbackBallistic,
            "A ballistic descent crosses the same death plane before considering a lower landing floor");
        room->m_ServerNavigation = nav;
    }
    {
        constexpr SESSION_ID session = 80771u;
        constexpr PLAYER_ID id = 80772u;
        room->m_PlayerIdBySessionId[session] = id;
        for (std::uint8_t gate = 1u; gate <= 4u; ++gate)
        {
            room->m_GateProgress.iCurrentGate = gate;
            auto p = player(); p.iPlayerId = id; p.iSessionId = session;
            p.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            p.iCurrentHp = 0u; p.eAction = PLAYER_ACTION_STATE::DEAD;
            p.fPositionX = 10000.f; p.fPositionY = -100.f; p.fPositionZ = 10000.f;
            p.bKnockbackBallistic = p.bKnockbackCanLeaveArena = true;
            p.fKnockbackRemainingSeconds = 1.f; p.fKnockbackVelocityY = -10.f;
            SERVER_NAV_POINT entry{}, expected{}; float yaw = 0.f;
            const bool resolves = room->Resolve_KoukuRevivePosition(p, entry, yaw) &&
                room->m_ServerNavigation.Project_Point(entry.x, entry.z, expected, entry.y);
            room->m_Players[id] = p;
            C2S_REVIVE_PLAYER revive{}; revive.iClientSequence = gate;
            room->Handle_RevivePlayer(session, revive);
            const auto& revived = room->m_Players.at(id);
            tests.Require(resolves && revived.iCurrentHp == revived.iMaximumHp && revived.isCombatReady &&
                revived.eAction == PLAYER_ACTION_STATE::NONE && !revived.bKnockbackBallistic &&
                !revived.bKnockbackCanLeaveArena && revived.fKnockbackRemainingSeconds == 0.f &&
                std::abs(revived.fPositionX - expected.x) < .01f &&
                std::abs(revived.fPositionY - expected.y) < .01f && std::abs(revived.fPositionZ - expected.z) < .01f,
                "Revive returns to the actual current Gate 1/2/3/Bingo start and clears all flight state");
        }
        room->m_GateProgress.iCurrentGate = 0u;
        auto p = player(); p.iPlayerId = id; p.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
        p.iCurrentHp = 0u; p.eAction = PLAYER_ACTION_STATE::DEAD; p.fPositionY = -100.f;
        p.strSpawnPlacementId = "missing.spawn"; room->m_Players[id] = p;
        C2S_REVIVE_PLAYER revive{}; revive.iClientSequence = 10u;
        room->Handle_RevivePlayer(session, revive);
        tests.Require(room->m_Players.at(id).iCurrentHp == 0u && room->m_Players.at(id).fPositionY == -100.f &&
            room->m_Players.at(id).eAction == PLAYER_ACTION_STATE::DEAD,
            "An unresolved Kouku revive destination preserves the dead player without partial healing");
        room->m_Players.erase(id); room->m_PlayerIdBySessionId.erase(session);
    }

    {
        ATTACK_HIT_TEMPLATE hit{}; hit.strHitId = "albion.rise";
        hit.fRadiusM = 2.0; hit.fRiseHeightM = 3.0; hit.iPushMs = 1200u;
        auto invalid = hit; invalid.iPushMs = 0u;
        tests.Require(!Validate_AttackHitTemplates({invalid}), "A rise height without a bounded flight duration is rejected");
        invalid = hit; invalid.fRiseHeightM = 0.0;
        tests.Require(!Validate_AttackHitTemplates({invalid}), "A flight duration without a rise height is rejected");
        const auto savedNav = room->m_ServerNavigation;
        room->m_ServerNavigation = CServerNavigation{};
        for (const bool contact : {false, true})
        {
            hit.strTrigger = contact ? "CONTACT" : "TIMED"; hit.iEndMs = contact ? 2000u : 0u;
            BOSS_COMBAT_OBJECT_DEFINITION definition{}; definition.strEncounterId = "albion.encounter";
            definition.strOwnerPatternId = "albion.pattern"; definition.strOwnerStageActionId = "albion.trigger";
            definition.strCombatObjectArchetypeId = "combatobject.kouku.albion.bluecircle";
            definition.strClientVisualId = "combatvisual.kouku.albion.bluecircle";
            definition.iLifeMs = 3000u; definition.AttackTemplates = {hit};
            SERVER_WORLD_ENTITY owner{}; owner.strEncounterId = definition.strEncounterId;
            owner.iNetEntityId = 998u; owner.iPatternSequence = 1u; owner.iCurrentHp = 100u;
            (void)Try_Parse_GameplayDataRevision(std::string(64u, 'a'), owner.PinnedDefinitionRevision);
            CCombatObjectRuntime runtime; auto transaction = runtime.Begin_Transaction();
            const bool staged = runtime.Stage_BossCombatObject(transaction, owner, nullptr, definition, nullptr, catalog, 1u, 100u, status);
            tests.Require(staged && runtime.Commit(std::move(transaction)), "Existing combat object staging admits a rising Albion timed/contact hit");
            auto p = player(); p.fPositionX = 0.f;
            std::map<PLAYER_ID, SERVER_PLAYER> targets{{1u, p}};
            std::vector<SERVER_WORLD_ENTITY> owners{owner}; std::vector<DAMAGE_EVENT> damage;
            runtime.Update(targets, owners, catalog, 1.f / 30.f, 101u, damage);
            runtime.Update(targets, owners, catalog, 1.f / 30.f, 102u, damage);
            auto& caught = targets.at(1u);
            tests.Require(caught.iCurrentHp == 90u && damage.size() == 1u && caught.bKnockbackBallistic &&
                !caught.bKnockbackCanLeaveArena && caught.fKnockbackSpeed == 0.f,
                "Albion timed/contact damage launches once vertically through the shared world-hit consumer");
            caught.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            caught.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
            C2S_USE_SKILL standup{}; standup.iClientSequence = 1u; standup.iSkillId = 34030u;
            standup.fAimX = caught.fPositionX + 1.f;
            tests.Require(room->m_GameplayCatalog.Find_Skill(34030u) != nullptr &&
                !CPlayerSkillSystem{}.Try_Start(caught, standup, room->m_GameplayCatalog, 103u),
                "Stand-up input cannot cancel authoritative ballistic motion while airborne");
            for (unsigned tick = 0u; tick < 18u; ++tick) room->Advance_PlayerKnockback(caught, 1.f / 30.f);
            tests.Require(std::abs(caught.fPositionY - 3.f) < .001f && caught.fPositionX == 0.f,
                "Authored Albion rise reaches three metres halfway through its 1.2-second flight");
            for (unsigned tick = 0u; tick < 19u; ++tick) room->Advance_PlayerKnockback(caught, 1.f / 30.f);
            tests.Require(std::abs(caught.fPositionY) < .001f && caught.fKnockbackRemainingSeconds == 0.f &&
                caught.iCurrentHp == 90u && caught.eAction == PLAYER_ACTION_STATE::KNOCKDOWN && !caught.bKnockbackBallistic &&
                caught.iKnockdownEndTick > 138u, "Authored Albion rise lands in a retained down pose without a second damage tick");
        }
        room->m_ServerNavigation = savedNav;
    }

}

int LostArk::Server::Run_ServerKoukuObjectOverlapContractTests()
{
	TESTS tests;
	const CGameplayCatalog catalog;
	CServerGameplayContractRunner::Run_KoukuPushContracts(tests, catalog);
	Run_SplitBallCylinderDamageContracts(tests, catalog);
	Run_ColliderMotionAndTickContracts(tests, catalog);
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
        // Duration contact uses the same per-player clock as trigger contact.
        auto& repeated = pattern.LogicWindows.front();
        repeated.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
        repeated.iDurationMs = 500u; repeated.iRepeatIntervalMs = 100u;
        repeated.bRearmOnExit = repeated.bRepeatAfterKnockback = false;
        BOSS_PATTERN_LOGIC_RESULT fixed; fixed.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; fixed.iDamageAmount = 100u;
        BOSS_PATTERN_LOGIC_RESULT madness; madness.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT; madness.iPercent = 2u;
        repeated.OnSuccess = {fixed, madness}; repeated.OnFail.clear(); repeated.OnTimeout.clear();
        auto& rectangle = repeated.CardRegions.front(); rectangle = {};
        rectangle.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD; rectangle.fHalfX = rectangle.fHalfZ = 2.f;
        first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
        first.iCurrentHp = first.iMaximumHp = 2000u; first.iMaximumMadness = 100u; first.isCombatReady = true;
        first.iMadnessDamageGainPercent = 100u;
        second.fPositionX = 50.f; events.clear();
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 800u, ledger);
        update(800u); update(801u); update(802u);
        tests.Require(first.iCurrentHp == 1900u && first.iCurrentMadness == 2u && events.size() == 1u,
            "Duration flame contact hits immediately then respects the authored hundred-millisecond interval");
        update(803u); first.fPositionX = 20.f; update(806u); first.fPositionX = 0.f; update(809u);
        update(812u); update(815u); update(818u);
        tests.Require(first.iCurrentHp == 1600u && first.iCurrentMadness == 8u && events.size() == 4u,
            "Duration contact applies damage and madness together, excludes exits and expires without an extra end-tick hit");
        // Gauge belongs to a landed damage verdict, regardless of result order.
        repeated.iDurationMs = 900u; repeated.iRepeatIntervalMs = 300u;
        madness.iPercent = 1u; repeated.OnSuccess = {madness, fixed};
        const auto resetFlame = [&] {
            first = SERVER_PLAYER{}; first.iPlayerId = 1u; first.iNetEntityId = 8101u;
            first.iCurrentHp = first.iMaximumHp = 2000u; first.iMaximumMadness = 100u;
            first.iMadnessDamageGainPercent = 100u; first.isCombatReady = true;
            events.clear(); CKoukuSaydonLogicRuntime::Build(pattern, *boss, 900u, ledger);
        };
        resetFlame();
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 1700u && first.iCurrentMadness == 3u &&
            first.dMadnessRemainder == 0. && events.size() == 3u && first.iMadnessDamageGainPercent == 100u,
            "Three explicit breath ticks preserve 100 damage each and add exactly 3 percent madness without automatic gain");
        resetFlame(); first.iShield = 1000u;
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 2000u && first.iShield == 700u && first.iCurrentMadness == 0u,
            "Shield-absorbed breath damage cannot add explicit contact madness");
        resetFlame(); first.iInvulnerableEndTick = 1000u;
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 2000u && first.iCurrentMadness == 0u && events.empty(),
            "Invulnerability blocks breath damage and its explicit madness as one verdict");
        madness.iPercent = 0u; repeated.OnSuccess = {fixed, madness};
        resetFlame(); first.iCurrentMadness = 3u;
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 1700u && first.iCurrentMadness == 3u &&
            first.dMadnessRemainder == 0. && events.size() == 3u && first.iMadnessDamageGainPercent == 100u,
            "Explicit zero madness preserves flame-floor damage and the prior three-percent gauge while suppressing automatic gain");
        resetFlame(); first.iShield = 1000u;
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 2000u && first.iShield == 700u && first.iCurrentMadness == 0u,
            "Explicit zero madness preserves shield absorption without gaining gauge");
        resetFlame(); first.iInvulnerableEndTick = 1000u;
        for (unsigned tick = 900u; tick <= 930u; ++tick) update(tick);
        tests.Require(first.iCurrentHp == 2000u && first.iCurrentMadness == 0u && events.empty(),
            "Explicit zero madness preserves invulnerability without contact damage or gauge");
        repeated.OnSuccess = {fixed}; resetFlame(); update(900u);
        tests.Require(first.iCurrentHp == 1900u && first.iCurrentMadness == 5u && first.iMadnessDamageGainPercent == 100u,
            "An ordinary damage verdict without an explicit madness result retains automatic HP-proportional gain");


	}
#endif
	std::cout << "failures : " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
