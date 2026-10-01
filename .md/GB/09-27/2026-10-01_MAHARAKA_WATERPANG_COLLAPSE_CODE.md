# G06/G07 워터팡 붕괴·점프 도착 연결 — 적용 코드 전문

2026-10-01. 최신 main b83d646be에 통합한 SEQUENCES_PLAN G06/G07의 코드 부록이다.
신규 C++ 파일/project/filter 등록은 없다. 기존 main의 `--maharaka-ai-contract-test`를 재사용한다.
Shared 시계 → Client 원본 WorldSequence → Server 지지/낙하/점프/AI → 기존 집중 검사 순서다.
main의 파티 입장·경기 종료·복귀·원본 물총 검사도 해당 파일 전문에 보존했다.

## C:/Users/USER/source/졸업팀폴/LostArk/Shared/Public/Gameplay/MaharakaWaterpangContract.h

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace LostArk::Shared
{
    // The existing stage instance identifies the entire admitted Waterpang intro.
    // S2C_WORLD_SEQUENCE_PLAY carries its future start on the 30 Hz Server clock.
    inline constexpr const char* MAHARAKA_WATERPANG_INTRO_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.intro15.stage";
    // Server spawn and Client level-entry preload consume the same NPC roster.
    inline constexpr std::array<const char*, 8> MAHARAKA_WATERPANG_AI_NPCS = {
        "NPC_MHP_RESIDENT_8FC2DB56F0AA5175", "NPC_MHP_RESIDENT_7D37CE489AF57466",
        "NPC_MHP_RESIDENT_6C0BF3F0C656EBAB", "NPC_MHP_RESIDENT_48E52BAC20260E4C",
        "NPC_BEDA", "NPC_AYLARA", "NPC_FORMAN", "NPC_SCHMIDT" };
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TICK_HZ = 30u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COUNTDOWN_TICKS = 300u;
    // Navigation detail region that bakes the arena deck and its jump pier. It spans the whole
    // island grid (so a player can walk out of the pool), hence "inside the region" alone no
    // longer means "on the arena": the hazards also test the arena footprint below.
    inline constexpr const char* MAHARAKA_WATERPANG_REGION_ID = "WaterpangEntry";
    // The 22 m square the region used to cover when it was the arena window (half-open, metres).
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MIN_X_M = 64.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MAX_X_M = 86.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MIN_Z_M = -995.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MAX_Z_M = -973.f;
    constexpr bool Is_MaharakaWaterpangArenaFootprint(const float x, const float z) noexcept
    {
        return x >= MAHARAKA_WATERPANG_ARENA_MIN_X_M && x < MAHARAKA_WATERPANG_ARENA_MAX_X_M &&
            z >= MAHARAKA_WATERPANG_ARENA_MIN_Z_M && z < MAHARAKA_WATERPANG_ARENA_MAX_Z_M;
    }

    /* Match cast from 57011 DeployData: actor 3 (NPC 570911, water cannon) and
       actor 22 (NPC 570941, big mokoko), converted to runtime metres. */
    inline constexpr float MAHARAKA_WATERPANG_CANNON_X = 75.044f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_Z = -984.307f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_YAW_DEGREES = 223.59f;
    inline constexpr float MAHARAKA_WATERPANG_MOKOMOKO_X = 81.836f;
    inline constexpr float MAHARAKA_WATERPANG_MOKOMOKO_Z = -991.514f;

    /* Rotating cannon, SkillEffect 422560330: a 1500 cm box pulled back 750 cm is
       one straight line through the cannon, i.e. two opposite 7.5 m jets, 70 cm
       wide, re-applied every 0.4 s. A hit pushes 500-520 cm over 535-555 ms and
       knocks down. One turn takes 19.2 s ("rotates") or 9.5 s ("rotates fast");
       the 1.5 s rectangle decal precedes the jets (actions 4225613-16). */
    inline constexpr float MAHARAKA_WATERPANG_CANNON_HALF_LENGTH_M = 7.5f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_HALF_WIDTH_M = 0.35f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_HIT_INTERVAL_TICKS = 12u;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_PUSH_M = 5.1f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_PUSH_MS = 545u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS = 45u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_END_TICKS = 30u;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_SECONDS_PER_TURN = 19.2f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_FAST_SECONDS_PER_TURN = 9.5f;

    /* Arena deck measured on the published WaterpangEntry navigation: a disc of
       ~7.45 m about the cannon at y 22.3-22.43 (the pool around it is 20.48); the
       jump1/jump2 piers leave it at the jump box bearings. */
    inline constexpr float MAHARAKA_WATERPANG_DECK_RADIUS_M = 7.5f;
    inline constexpr float MAHARAKA_WATERPANG_DECK_MIN_Y_M = 21.5f;
    // A launch never ends over a pier: bearings within this of a jump box turn aside.
    inline constexpr float MAHARAKA_WATERPANG_PIER_HALF_ANGLE_DEGREES = 12.f;

    /* The crossing triggers (movePlayer arcs from a pier or the pool onto the deck,
       Gameplay.world.json jump1, jump2, jump3). Landing from one grants this many
       ticks of invulnerability, so a player who just crossed is not pushed off by
       the hazard that is already running. One second at the 30 Hz Server clock. */
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_JUMP_INVULNERABLE_TICKS = 1u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr std::array<std::string_view, 3u> MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS{ "jump1", "jump2", "jump3" };

    constexpr bool Is_MaharakaWaterpangJumpTrigger(const std::string_view triggerPlacementId) noexcept
    {
        for (const std::string_view id : MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS)
            if (id == triggerPlacementId) return true;
        return false;
    }

    /* Big mokoko waterfall, arena action 4225612 (6 s clip att_battle_1_01, the
       Waterpang variant of 4225601; decal SkillDecal 1106 for 2.3 s). The source
       hits a 180 degree half disc 3.2 m ahead of the mokoko in two waves (inner
       1.2-2.5 m at 2.30 s, outer 2.5-5.9 m at 2.46 s) and launches 580-600 cm over
       1814-1834 ms at 200 cm height with FallDown. Project rule: the waves cover
       the whole deck (inner = within 2.5 m of that origin), and every hit body
       is launched away from the arena centre far enough to leave the deck; the
       launch always ends in the Waterpang fall (jump box revive). No damage.
       Project rule (revised): only the yellow centre disc is hit; the ring of
       purple blocks around it and the piers are clear. */
    /* Radius of the yellow disc about the cannon: the placed mesh
       BG_OCN_ETC_FLOOR01_SM_LNH_OVR_8EB1D0D03456 (Data/Maps/Authoring/LV_OCN_EVENTIS_MHP,
       scale 0.95 x 1.1875 x 0.95) has a top face of radius 4.90 m and a widest
       vertex at 5.01 m. The 16 purple ring blocks start outside it (block centres
       at 6.15 m, deck edge 7.5 m). Raise or lower this one value to widen or
       narrow the waterfall; the mokoko telegraph decal is built from it too. */
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M = 5.f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_TICKS = 180u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_HIT_TICK = 69u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_SECOND_HIT_TICK = 74u;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M = 3.2f;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_INNER_WAVE_M = 2.5f;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_LAUNCH_M = 5.9f;
    // Every launch ends at least this far from the cannon (deck 7.5 m + margin).
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_EXIT_RADIUS_M = 9.f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_LAUNCH_MS = 1824u;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_LAUNCH_HEIGHT_M = 2.f;

    enum class MAHARAKA_WATERPANG_EVENT_KIND : std::uint8_t
    {
        CANNON,      // GameMsg se_announce_52
        CANNON_FAST, // se_announce_53
        WATERFALL    // se_announce_51
    };

    struct MAHARAKA_WATERPANG_EVENT final
    {
        std::uint32_t iStartSeconds;
        MAHARAKA_WATERPANG_EVENT_KIND eKind;
        std::uint32_t iFireSeconds; // cannon jets only
    };

    // One cannon turn or waterfall, measured from the notice tick to its last tick.
    constexpr std::uint32_t Get_MaharakaWaterpangEventTicks(const MAHARAKA_WATERPANG_EVENT& event) noexcept
    {
        return MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind ?
            MAHARAKA_WATERPANG_WATERFALL_TICKS :
            MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + event.iFireSeconds * MAHARAKA_WATERPANG_TICK_HZ +
                MAHARAKA_WATERPANG_CANNON_END_TICKS;
    }

    /* Schedule (project tuning, not a source timer): the hazards alternate strictly,
       one cannon turn, then one waterfall, then a cannon turn again. The idle time
       between the end of one hazard and the start of the next is this one value;
       a start always lands on a whole second, so the real idle time is up to 1 s
       longer. Lower it for a faster loop; two hazards never overlap. The old
       table waited 22-37 s between two cannons and had waterfalls only three times. */
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_EVENT_GAP_SECONDS = 8u;
    // First hazard: the countdown and the intro cutscene own the cast until then.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS = 20u;
    // Project match duration: three minutes of play after the intro owns the stage.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_MATCH_SECONDS = 180u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_MATCH_END_TICKS =
        (MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS + MAHARAKA_WATERPANG_MATCH_SECONDS) * MAHARAKA_WATERPANG_TICK_HZ;
    // Reuse SCENE03B Matinee42/Data157: eighteen ring tiles, 5 s, held until match STOP.
    // The requested 60 s remaining is match time, not countdown/intro time.
    inline constexpr const char* MAHARAKA_WATERPANG_COLLAPSE_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.collapse";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_START_TICKS =
        MAHARAKA_WATERPANG_MATCH_END_TICKS - 60u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_DURATION_MS = 5000u;
    // Gameplay transition: the outward spread precedes the steep descent. Remove support at 2 s.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS =
        MAHARAKA_WATERPANG_COLLAPSE_START_TICKS + 2u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr float MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M = 4.4f;
    constexpr bool Has_MaharakaWaterpangCollapsedFloor(const std::int32_t elapsedTicks) noexcept
    {
        return elapsedTicks >= static_cast<std::int32_t>(MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS);
    }
    constexpr bool Is_MaharakaWaterpangMissingRing(const std::int32_t elapsedTicks,
        const float x, const float z) noexcept
    {
        const float dx = x - MAHARAKA_WATERPANG_CANNON_X, dz = z - MAHARAKA_WATERPANG_CANNON_Z;
        const float radiusSquared = dx * dx + dz * dz;
        return Has_MaharakaWaterpangCollapsedFloor(elapsedTicks) &&
            radiusSquared > MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M * MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M &&
            radiusSquared <= MAHARAKA_WATERPANG_DECK_RADIUS_M * MAHARAKA_WATERPANG_DECK_RADIUS_M;
    }
    inline constexpr std::size_t MAHARAKA_WATERPANG_AI_COUNT = 20u;

    constexpr std::uint32_t Get_MaharakaWaterpangNextStartSeconds(
        const std::uint32_t startSeconds, const MAHARAKA_WATERPANG_EVENT& event) noexcept
    {
        const std::uint32_t idleUntilTicks = startSeconds * MAHARAKA_WATERPANG_TICK_HZ +
            Get_MaharakaWaterpangEventTicks(event) + MAHARAKA_WATERPANG_EVENT_GAP_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
        return (idleUntilTicks + MAHARAKA_WATERPANG_TICK_HZ - 1u) / MAHARAKA_WATERPANG_TICK_HZ;
    }

    /* Cannon order, speeds and the 50/50 turn direction follow trigger chain
       6000-6009 (normal 15 s x4, longer 20 s x2, then fast 15 s, 10 s, 10 s). The
       timers of that chain are condition-driven on the Server, so absolute times
       are this project schedule. A waterfall follows every cannon turn. */
    inline constexpr std::array<MAHARAKA_WATERPANG_EVENT, 9u> MAHARAKA_WATERPANG_CANNON_SEQUENCE{ {
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 20u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 20u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 15u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 10u },
        { 0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 10u } } };

    struct MAHARAKA_WATERPANG_SCHEDULE_PLAN final
    {
        std::array<MAHARAKA_WATERPANG_EVENT, 18u> rows{};
        std::uint32_t iTailStartSeconds = 0u; // first start after the last row
    };

    constexpr MAHARAKA_WATERPANG_SCHEDULE_PLAN Make_MaharakaWaterpangSchedule() noexcept
    {
        MAHARAKA_WATERPANG_SCHEDULE_PLAN plan{};
        std::uint32_t start = MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS;
        for (std::size_t index = 0u; index < MAHARAKA_WATERPANG_CANNON_SEQUENCE.size(); ++index)
        {
            MAHARAKA_WATERPANG_EVENT cannon = MAHARAKA_WATERPANG_CANNON_SEQUENCE[index];
            cannon.iStartSeconds = start;
            plan.rows[index * 2u] = cannon;
            start = Get_MaharakaWaterpangNextStartSeconds(start, cannon);
            const MAHARAKA_WATERPANG_EVENT waterfall{ start, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL, 0u };
            plan.rows[index * 2u + 1u] = waterfall;
            start = Get_MaharakaWaterpangNextStartSeconds(start, waterfall);
        }
        plan.iTailStartSeconds = start;
        return plan;
    }

    inline constexpr MAHARAKA_WATERPANG_SCHEDULE_PLAN MAHARAKA_WATERPANG_SCHEDULE_BUILT =
        Make_MaharakaWaterpangSchedule();
    inline constexpr std::array<MAHARAKA_WATERPANG_EVENT, 18u> MAHARAKA_WATERPANG_SCHEDULE =
        MAHARAKA_WATERPANG_SCHEDULE_BUILT.rows;

    /* After the table a fast 10 s turn and a waterfall repeat, still alternating
       (trigger 1700 cadence). Offsets and the period follow the same gap. */
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_START_SECONDS = MAHARAKA_WATERPANG_SCHEDULE_BUILT.iTailStartSeconds;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_FIRE_SECONDS = 10u;
    inline constexpr MAHARAKA_WATERPANG_EVENT MAHARAKA_WATERPANG_TAIL_CANNON{
        0u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, MAHARAKA_WATERPANG_TAIL_FIRE_SECONDS };
    inline constexpr MAHARAKA_WATERPANG_EVENT MAHARAKA_WATERPANG_TAIL_WATERFALL{
        0u, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL, 0u };
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_WATERFALL_OFFSET_SECONDS =
        Get_MaharakaWaterpangNextStartSeconds(0u, MAHARAKA_WATERPANG_TAIL_CANNON);
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_PERIOD_SECONDS =
        Get_MaharakaWaterpangNextStartSeconds(MAHARAKA_WATERPANG_TAIL_WATERFALL_OFFSET_SECONDS, MAHARAKA_WATERPANG_TAIL_WATERFALL);

    struct MAHARAKA_WATERPANG_EVENT_SAMPLE final
    {
        MAHARAKA_WATERPANG_EVENT_KIND eKind = MAHARAKA_WATERPANG_EVENT_KIND::CANNON;
        std::uint32_t iOccurrence = 0u;   // table row, then tail rows after it
        std::uint32_t iElapsedTicks = 0u; // since the event (notice) started
        std::uint32_t iDurationTicks = 0u;
        bool bClockwise = true;
        bool bFiring = false;             // cannon jets are live
        float fCannonYawDegrees = 0.f;    // one jet; the other is +180
    };

    /* Fills the sample of one event elapsedTicks after its own start. The turn
       direction hashes hashTick with the occurrence, so Server and Client agree
       without replicating it. */
    inline bool Complete_MaharakaWaterpangSample(const MAHARAKA_WATERPANG_EVENT& event,
        const std::uint32_t occurrence, const std::uint32_t elapsedTicks, const std::uint32_t hashTick,
        MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        out = {};
        out.eKind = event.eKind;
        out.iOccurrence = occurrence;
        out.iElapsedTicks = elapsedTicks;
        out.iDurationTicks = Get_MaharakaWaterpangEventTicks(event);
        if (MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind) return true;
        const std::uint32_t mix = (hashTick * 2654435761u) ^ ((occurrence + 1u) * 40503u);
        out.bClockwise = 0u == ((mix >> 13u) & 1u);
        const std::uint32_t fireTicks = event.iFireSeconds * MAHARAKA_WATERPANG_TICK_HZ;
        out.bFiring = out.iElapsedTicks >= MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS &&
            out.iElapsedTicks < MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + fireTicks;
        const float fired = out.iElapsedTicks <= MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS ? 0.f :
            static_cast<float>((out.iElapsedTicks < MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + fireTicks ?
                out.iElapsedTicks : MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + fireTicks) -
                MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS) / static_cast<float>(MAHARAKA_WATERPANG_TICK_HZ);
        const float period = MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST == event.eKind ?
            MAHARAKA_WATERPANG_CANNON_FAST_SECONDS_PER_TURN : MAHARAKA_WATERPANG_CANNON_SECONDS_PER_TURN;
        /* The source jets (projectile Loop_800 and the FX_Turn WaterBomb bursts)
           lie on their particle system's source Y axis, which the actor-root
           snapshot basis (-90) places at the cannon facing + 90 degrees. The
           Client roots every jet visual on this same line. */
        const float turned = 360.f * fired / period;
        out.fCannonYawDegrees = MAHARAKA_WATERPANG_CANNON_YAW_DEGREES + 90.f + (out.bClockwise ? turned : -turned);
        return true;
    }

    /* Samples the event running ticksSinceStart after the match start tick. The
       direction draw hashes the room's start tick, so Server and Client agree
       without replicating it. */
    inline bool Sample_MaharakaWaterpangEvent(const std::int32_t ticksSinceStart,
        const std::uint32_t matchStartTick, MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        if (ticksSinceStart < 0) return false;
        const auto now = static_cast<std::uint32_t>(ticksSinceStart);
        MAHARAKA_WATERPANG_EVENT event{};
        std::uint32_t occurrence = 0u;
        bool found = false;
        for (; occurrence < MAHARAKA_WATERPANG_SCHEDULE.size(); ++occurrence)
        {
            const auto& row = MAHARAKA_WATERPANG_SCHEDULE[occurrence];
            const std::uint32_t start = row.iStartSeconds * MAHARAKA_WATERPANG_TICK_HZ;
            if (now >= start && now - start < Get_MaharakaWaterpangEventTicks(row)) { event = row; found = true; break; }
        }
        const std::uint32_t tailStart = MAHARAKA_WATERPANG_TAIL_START_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
        if (!found && now >= tailStart)
        {
            const std::uint32_t period = MAHARAKA_WATERPANG_TAIL_PERIOD_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
            const std::uint32_t cycle = (now - tailStart) / period;
            const std::uint32_t within = (now - tailStart) % period;
            const std::uint32_t cycleStartSeconds = MAHARAKA_WATERPANG_TAIL_START_SECONDS +
                cycle * MAHARAKA_WATERPANG_TAIL_PERIOD_SECONDS;
            const std::uint32_t waterfallOffset = MAHARAKA_WATERPANG_TAIL_WATERFALL_OFFSET_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
            const auto tableRows = static_cast<std::uint32_t>(MAHARAKA_WATERPANG_SCHEDULE.size());
            if (within < Get_MaharakaWaterpangEventTicks(MAHARAKA_WATERPANG_TAIL_CANNON))
            {
                event = MAHARAKA_WATERPANG_TAIL_CANNON;
                event.iStartSeconds = cycleStartSeconds;
                occurrence = tableRows + cycle * 2u;
                found = true;
            }
            else if (within >= waterfallOffset &&
                within - waterfallOffset < Get_MaharakaWaterpangEventTicks(MAHARAKA_WATERPANG_TAIL_WATERFALL))
            {
                event = MAHARAKA_WATERPANG_TAIL_WATERFALL;
                event.iStartSeconds = cycleStartSeconds + MAHARAKA_WATERPANG_TAIL_WATERFALL_OFFSET_SECONDS;
                occurrence = tableRows + cycle * 2u + 1u;
                found = true;
            }
        }
        if (!found) return false;
        return Complete_MaharakaWaterpangSample(event, occurrence,
            now - event.iStartSeconds * MAHARAKA_WATERPANG_TICK_HZ, matchStartTick, out);
    }

    /* Debug F1 forced events (Debug Server only). The Server broadcasts one of these
       ids as S2C_WORLD_SEQUENCE_PLAY with iStartTick; while it runs it replaces the
       scheduled sample on the Server and on every Client, then the schedule resumes. */
    inline constexpr const char* MAHARAKA_WATERPANG_DEBUG_WATERFALL_INSTANCE =
        "maharaka.waterpang.debug.waterfall";
    inline constexpr const char* MAHARAKA_WATERPANG_DEBUG_CANNON_INSTANCE =
        "maharaka.waterpang.debug.cannon";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_CANNON_FIRE_SECONDS = 15u;
    // Start a few ticks ahead so every Client holds the event before its first tick.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_LEAD_TICKS = 6u;
    // Keeps forced occurrences apart from schedule rows and tail cycles.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_OCCURRENCE_BASE = 0x80000000u;

    inline bool Find_MaharakaWaterpangDebugKind(const std::string_view instanceId,
        MAHARAKA_WATERPANG_EVENT_KIND& out) noexcept
    {
        if (instanceId == MAHARAKA_WATERPANG_DEBUG_WATERFALL_INSTANCE) { out = MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL; return true; }
        if (instanceId == MAHARAKA_WATERPANG_DEBUG_CANNON_INSTANCE) { out = MAHARAKA_WATERPANG_EVENT_KIND::CANNON; return true; }
        return false;
    }

    inline MAHARAKA_WATERPANG_EVENT Make_MaharakaWaterpangDebugEvent(const MAHARAKA_WATERPANG_EVENT_KIND kind) noexcept
    {
        return { 0u, kind, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == kind ? 0u : MAHARAKA_WATERPANG_DEBUG_CANNON_FIRE_SECONDS };
    }

    // The forced event nowTick - startTick into it; false before its start and after its end.
    inline bool Sample_MaharakaWaterpangDebugEvent(const MAHARAKA_WATERPANG_EVENT_KIND kind,
        const std::uint32_t startTick, const std::uint32_t nowTick, MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        const auto elapsed = static_cast<std::int32_t>(nowTick - startTick);
        const MAHARAKA_WATERPANG_EVENT event = Make_MaharakaWaterpangDebugEvent(kind);
        if (elapsed < 0 || static_cast<std::uint32_t>(elapsed) >= Get_MaharakaWaterpangEventTicks(event)) return false;
        return Complete_MaharakaWaterpangSample(event,
            MAHARAKA_WATERPANG_DEBUG_OCCURRENCE_BASE | (startTick & 0x7fffffffu),
            static_cast<std::uint32_t>(elapsed), startTick, out);
    }

    /* Water Pro MK-1 (EFTable_Prop 15000) as the Waterpang arena arms it. The
       [Maharaka] action variants (skill id + 2) own the clips and the hit rows:
       GADGET.loa actions 57002 / 56912 / 56932, SkillEffect 570020-570021,
       569120-569121 and 569320-569321, Projectile 570020 / 569120 / 569320. The
       four quick slots are the prop's own list: Q connected shot (56900), W water
       bomb (56910), E speed up (56920) and the prop's default attack (56930) on R.
       Q uses the requested MK2 fan action/projectile while retaining stable
       skill 56900, the existing 3 s cooldown (source MK2 is 5 s), and the selected
       prop appearance. W/R retain their MK1 sources. Speed and MaxDistance are
       separate reflected projectile fields; the action's "Effect" notify fires
       every ray of the fan simultaneously. The source hit rows also carry
       HP damage and cold stacks (buff 569302, 8 stacks kill); neither is applied
       because the extracted data does not give the Waterpang stat adjustment they
       need, so a hit only pushes and staggers the body using the room's
       existing shared human/AI knockback tuning. Casting never locks movement (prop MoveSkillEnable = 1). */
    enum class MAHARAKA_WATERGUN_KIND : std::uint8_t
    {
        MISSILE,    // straight projectile from the caster along its facing
        GRENADE,    // thrown to the aimed ground point, bursts on landing
        SPEED_BUFF  // self buff, no projectile
    };

    struct MAHARAKA_WATERGUN_SKILL final
    {
        std::uint32_t iSkillId;
        char cInputSlot;              // Q W E R
        MAHARAKA_WATERGUN_KIND eKind;
        std::uint32_t iCooldownMs;    // EFTable_Skill.Cooltime
        std::uint32_t iActionMs;      // clip length: no second cast before it ends
        std::uint32_t iSpawnMs;       // action "Effect" notify: the shot leaves the muzzle
        std::uint32_t iAttackClip;    // watergun_att_N; 0 = no clip
        float fMaxRangeM;             // EFTable_Skill.MaxRange
        float fSpeedMps;              // Projectile speed
        float fLifeSeconds;           // Projectile lifetime; missile reach is also bounded by MaxDistance
        float fHitRadiusM;            // hit SkillEffect AreaRange
        float fPushRangeM;            // hit SkillEffect push range
        std::uint32_t iPushMs;        // hit SkillEffect push time
        std::uint32_t iBuffMs;        // SPEED_BUFF duration (SkillBuff 569200)
        float fBuffSpeedScale;        // SPEED_BUFF move speed factor (+30 %)
        float fProjectileMaxDistanceM; // Projectile MaxDistance, independent of Speed and skill aim range
        float fLaunchRightM;           // source Effect local Y; all fan rays share one muzzle position
        std::uint32_t iProjectileCount;
        float fSpreadDegrees;          // centre, positive, negative yaw for simultaneous source fan rows
    };

    inline constexpr std::array<MAHARAKA_WATERGUN_SKILL, 4u> MAHARAKA_WATERGUN_SKILLS{ {
        // Stable Q input/skill ID keeps its cooldown; its requested fan uses source MK2
        // action 57002 / projectile 570020 (Att4, one notify, 0/+30/-30 degrees).
        { 56900u, 'Q', MAHARAKA_WATERGUN_KIND::MISSILE, 3000u, 1667u, 704u, 4u, 7.f, 10.f, 1.5f, 1.f, 0.18f, 5u, 0u, 1.f, 3.3f, .20f, 3u, 30.f },
        { 56910u, 'W', MAHARAKA_WATERGUN_KIND::GRENADE, 8000u, 1000u, 200u, 5u, 7.f, 10.f, 6.f, 1.05f, 0.2f, 200u, 0u, 1.f, 8.f, 0.f, 1u, 0.f },
        { 56920u, 'E', MAHARAKA_WATERGUN_KIND::SPEED_BUFF, 7000u, 0u, 0u, 0u, 0.f, 0.f, 0.f, 0.f, 0.f, 0u, 5000u, 1.3f, 0.f, 0.f, 0u, 0.f },
        { 56930u, 'R', MAHARAKA_WATERGUN_KIND::MISSILE, 0u, 1000u, 402u, 1u, 4.f, 10.f, 1.5f, 1.f, 0.18f, 5u, 0u, 1.f, 3.f, .11f, 1u, 0.f } } };

    struct MAHARAKA_WATERGUN_LAUNCH final
    {
        float fX, fY, fZ;
        float fYawDegrees;
        float fDirX, fDirZ;
    };

    // Both room collision and replicated presentation consume the same source muzzle
    // and ray basis. The fan rotates direction, never the shared local launch offset.
    inline MAHARAKA_WATERGUN_LAUNCH Sample_MaharakaWaterGunLaunch(
        const MAHARAKA_WATERGUN_SKILL& skill, const std::uint32_t projectileIndex,
        const float x, const float y, const float z, const float ownerYawDegrees) noexcept
    {
        constexpr float radiansPerDegree = 0.01745329251994329577f;
        const float offset = projectileIndex == 1u ? skill.fSpreadDegrees :
            projectileIndex == 2u ? -skill.fSpreadDegrees : 0.f;
        MAHARAKA_WATERGUN_LAUNCH result{x, y, z, ownerYawDegrees + offset, 0.f, 0.f};
        result.fDirX = std::sin(result.fYawDegrees * radiansPerDegree);
        result.fDirZ = std::cos(result.fYawDegrees * radiansPerDegree);
        if (skill.eKind == MAHARAKA_WATERGUN_KIND::MISSILE)
        {
            const float ownerX = std::sin(ownerYawDegrees * radiansPerDegree);
            const float ownerZ = std::cos(ownerYawDegrees * radiansPerDegree);
            result.fX += ownerX * .70f + ownerZ * skill.fLaunchRightM;
            result.fZ += ownerZ * .70f - ownerX * skill.fLaunchRightM;
            result.fY += .75f;
        }
        return result;
    }

    // Stable gameplay identities; only Client presentation maps these to resources.
    inline constexpr const char* MaharakaWaterGunProjectileArchetype(const std::uint32_t skillId) noexcept
    {
        switch (skillId)
        {
        case 56900u: return "maharaka.watergun.burst";
        case 56910u: return "maharaka.watergun.bomb";
        case 56930u: return "maharaka.watergun.single";
        default: return nullptr;
        }
    }
    inline constexpr bool Is_MaharakaWaterGunProjectile(const std::string_view id) noexcept
    {
        return id == "maharaka.watergun.burst" || id == "maharaka.watergun.bomb" ||
            id == "maharaka.watergun.single";
    }
    inline constexpr std::uint32_t MAHARAKA_WATERGUN_SPEED_BUFF_ID = 569200u;

    // A struck body must stand within this height of the shot to be hit.
    inline constexpr float MAHARAKA_WATERGUN_HIT_HEIGHT_M = 1.5f;

    inline const MAHARAKA_WATERGUN_SKILL* Find_MaharakaWaterGunSkill(const std::uint32_t skillId) noexcept
    {
        for (const MAHARAKA_WATERGUN_SKILL& skill : MAHARAKA_WATERGUN_SKILLS)
            if (skill.iSkillId == skillId) return &skill;
        return nullptr;
    }

    inline const MAHARAKA_WATERGUN_SKILL* Find_MaharakaWaterGunSkillBySlot(const char inputSlot) noexcept
    {
        for (const MAHARAKA_WATERGUN_SKILL& skill : MAHARAKA_WATERGUN_SKILLS)
            if (skill.cInputSlot == inputSlot) return &skill;
        return nullptr;
    }

    inline constexpr std::uint32_t Get_MaharakaWaterGunTicks(const std::uint32_t milliseconds) noexcept
    {
        return (milliseconds * MAHARAKA_WATERPANG_TICK_HZ + 999u) / 1000u;
    }
}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/MaharakaWaterpangPresentation.h

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#pragma once
#include "Client_Defines.h"
#include "WorldSequencePlayer.h"
#include "ValtanCinematicCameraController.h"
#include "Network/PacketMessages.h"
#include "Gameplay/MaharakaWaterpangContract.h"

namespace Client
{
class CCamera_Free;
// Level-owned presentation only. A Server reservation is the sole start authority.
class CMaharakaWaterpangPresentation final
{
public:
    static constexpr const wchar_t* WATER_GUN_PROTOTYPE_TAG = L"Prototype_Component_Model_MaharakaWaterGun";
    static HRESULT Ensure_WaterGunPrototype(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, uint32_t levelIndex, std::string& status);
    bool Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera);
    void Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play);
    void Update(float delta, uint32_t serverTick, bool editing);
    void Render() const;
    void Stop();
    void Suspend_ForAuthoring();
    ~CMaharakaWaterpangPresentation();
    const std::string& Get_Status() const { return m_Status; }
private:
    struct CUT { uint32_t startMs = 0; VALTAN_CINEMATIC_CAMERA_CUE cue; };
    bool Load_Camera();
    bool Start_World(float elapsedMs);
    // Match hazards: the authored attack sequence of each Server phase.
    void Update_Attacks();
    void Update_Collapse(); // Original ring motion, independent of the alternating attack phase.
    bool Show_Actor(size_t actor, const char* instanceId, float offsetMs, uint32_t occurrence);
    // The running Debug forced event first, else the match schedule (the Server samples the same way).
    bool Sample_Now(LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event) const;
    // Returns the cast to its placements after a forced event that ran without a match.
    void End_ForcedOnly();
    std::string Describe_EffectTracks(const std::string& instanceId, bool turned) const;
    void Trace_JetAxes(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event);
    void Trace_Waterfall(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event);
    CWorldSequencePlayer::TARGET_SET m_Targets;
    CWorldSequencePlayer m_World;
    std::weak_ptr<CCamera_Free> m_Camera;
    std::vector<CUT> m_Cuts;
    std::vector<std::string> m_Instances;
    uint32_t m_DurationMs = 0, m_StartTick = 0, m_LastTick = 0;
    float m_ClockMs = 0.f;
    float m_ReadinessWait = 0.f;
    float m_WorldClockMs = 0.f;
    int m_Countdown = 0;
    int m_Notice = -1; // MAHARAKA_WATERPANG_EVENT_KIND of the running match event
    std::string m_ActorInstances[2]; // instance currently posing the mokoko / cannon
    uint32_t m_ActorOccurrences[2] = { UINT32_MAX, UINT32_MAX }; // event occurrence each instance plays
    // Debug F1 forced event from the Server; m_ForcedOnly = the cast was posed for it without a match.
    bool m_Forced = false, m_ForcedOnly = false;
    LostArk::Shared::MAHARAKA_WATERPANG_EVENT_KIND m_ForcedKind = LostArk::Shared::MAHARAKA_WATERPANG_EVENT_KIND::CANNON;
    uint32_t m_ForcedStartTick = 0, m_ForcedOnlyBaseTick = 0;
    bool m_AttackPrepared = false, m_AttackDisabled = false;
    bool m_CollapseStarted = false, m_CollapseFailed = false;
    // Effect-only turn that puts the loop instance's bursts and jet on the Server
    // jet line, about the held cannon pivot. Shared so effect providers never
    // borrow this object.
    struct CANNON_TURN { float yawDegrees = 0.f; float pivotX = 0.f, pivotZ = 0.f; };
    std::shared_ptr<CANNON_TURN> m_CannonTurn = std::make_shared<CANNON_TURN>();
    float m_CannonObjectYawDegrees = 0.f; // held cannon object yaw of the loop templates
    float m_LastAxisTraceMs = -1.e9f;
    uint32_t m_WaterfallTraceOccurrence = UINT32_MAX;
    bool m_Ready = false, m_Scheduled = false, m_Started = false, m_Failed = false;
    std::string m_Status;
};
}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MaharakaWaterpangPresentation.cpp

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#include "MaharakaWaterpangPresentation.h"
#include "ActorCatalog.h"
#include "Model.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "Camera_Free.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EffectFailureDiagnostic.h"
#include "GameInstance.h"
#include "UILabelFont.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>

using namespace Client;
namespace
{
constexpr uint64_t CAMERA_OWNER = 0x4d48505741544552ull;
constexpr const char* AREA = "LV_OCN_EVENTIS_MHP";
constexpr const char* INTRO = "cutscene.maharaka.waterpang.source.intro15";
// Intro instances hold the match pose; the attack instances borrow the same objects.
constexpr const char* MOKOMOKO_HOLD = "world.sequence.instance.maharaka.waterpang.source.intro15.mokomoko";
constexpr const char* CANNON_HOLD = "world.sequence.instance.maharaka.waterpang.source.intro15.cannon";
/* Stable instance IDs per Server phase. Everything these sequences show or
   play - clips, effects with their anchors and offsets, telegraphs, sounds - is
   authored data, edited in MapTool Camera under the attack preview cutscenes. */
constexpr const char* MOKOMOKO_ATTACK = "world.sequence.instance.maharaka.waterpang.attack.mokomoko";
constexpr const char* CANNON_START = "world.sequence.instance.maharaka.waterpang.attack.cannon.start";
constexpr const char* CANNON_LOOP_CW = "world.sequence.instance.maharaka.waterpang.attack.cannon.loop.cw";
constexpr const char* CANNON_LOOP_CCW = "world.sequence.instance.maharaka.waterpang.attack.cannon.loop.ccw";
constexpr const char* CANNON_END = "world.sequence.instance.maharaka.waterpang.attack.cannon.end";
constexpr const char* ATTACK_INSTANCES[] = { MOKOMOKO_ATTACK, CANNON_START, CANNON_LOOP_CW, CANNON_LOOP_CCW, CANNON_END };
// Heading of a direction mod 180: both branches of a line share one axis.
float Line_Axis(DirectX::FXMVECTOR direction)
{
    const float degrees=std::fmod(DirectX::XMConvertToDegrees(std::atan2(
        DirectX::XMVectorGetX(direction),DirectX::XMVectorGetZ(direction)))+360.f,180.f);
    return degrees<0.f ? degrees+180.f : degrees;
}
const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing camera field: ") + key);
    return *value;
}
std::string Text(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_String() || value.Get_String().empty()) throw std::runtime_error("Invalid camera string");
    return value.Get_String();
}
double Number(const DATA_JSON_VALUE& row, const char* key, double lo, double hi)
{
    const auto& value = Field(row,key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number()<lo || value.Get_Number()>hi)
        throw std::runtime_error(std::string("Invalid camera number: ") + key);
    return value.Get_Number();
}
uint32_t Milliseconds(const DATA_JSON_VALUE& row, const char* key)
{
    const double value = Number(row,key,0,600000);
    if (value != std::floor(value)) throw std::runtime_error("Non-integral camera clock");
    return static_cast<uint32_t>(value);
}
const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& value = Field(row,key);
    if (!value.Is_Array()) throw std::runtime_error("Invalid camera array");
    return value.Get_Array();
}
float3_t Vector(const DATA_JSON_VALUE& row, const char* key)
{
    const auto& a = Array(row,key);
    if (a.size()!=3) throw std::runtime_error("Invalid camera vector");
    for (const auto& v:a) if (!v.Is_Number() || !std::isfinite(v.Get_Number()) || std::abs(v.Get_Number())>100000)
        throw std::runtime_error("Invalid camera coordinate");
    return {static_cast<float>(a[0].Get_Number()),static_cast<float>(a[1].Get_Number()),static_cast<float>(a[2].Get_Number())};
}
}

bool CMaharakaWaterpangPresentation::Load_Camera()
{
    try
    {
        const auto path = CMapAssetCatalog::Get_MapDataRoot()/L"LV_OCN_EVENTIS_MHP.camerashots.json";
        if (std::filesystem::file_size(path)>2097152u) throw std::runtime_error("Camera document exceeds limit");
        std::ifstream input(path, std::ios::binary); std::ostringstream bytes; bytes<<input.rdbuf();
        DATA_JSON_VALUE root; std::string error;
        if (!input || !CDataJson::Parse(bytes.str(),root,error)) throw std::runtime_error("Camera parse failed: "+error);
        if (Text(root,"schema")!="lostark.camera-shots" || Number(root,"formatVersion",1,1)!=1 || Text(root,"areaId")!=AREA)
            throw std::runtime_error("Wrong Waterpang camera document");
        const DATA_JSON_VALUE* scene = nullptr;
        std::set<std::string> seen;
        for (const auto& row:Array(root,"cutscenes"))
        {
            const auto id = Text(row,"cutsceneId");
            if (!seen.insert(id).second) throw std::runtime_error("Duplicate cutscene");
            if (id==INTRO) scene=&row;
        }
        if (!scene) throw std::runtime_error("Waterpang intro15 is missing");
        const uint32_t duration = Milliseconds(*scene,"durationMs");
        if (!duration) throw std::runtime_error("Empty Waterpang intro");
        std::vector<std::string> instances; seen.clear();
        for (const auto& id:Array(*scene,"worldInstanceIds"))
        {
            if (!id.Is_String() || !seen.insert(id.Get_String()).second || !m_World.Get_Document().Find_Instance(id.Get_String()))
                throw std::runtime_error("Missing/duplicate Waterpang world instance");
            instances.push_back(id.Get_String());
        }
        if (instances.empty()) throw std::runtime_error("Empty Waterpang cast");
        std::map<std::string,const DATA_JSON_VALUE*> shots;
        for (const auto& row:Array(root,"shots"))
            if (!shots.emplace(Text(row,"shotId"), &row).second) throw std::runtime_error("Duplicate camera shot");
        std::vector<CUT> cuts;
        for (const auto& row:Array(*scene,"cameraCuts"))
        {
            CUT cut; cut.startMs=Milliseconds(row,"startMs");
            if (cut.startMs>=duration || (!cuts.empty() && cut.startMs<=cuts.back().startMs)) throw std::runtime_error("Camera cut order invalid");
            auto shot=shots.find(Text(row,"shotId"));
            if (shot==shots.end()) throw std::runtime_error("Camera shot reference missing");
            const auto& track=Field(*shot->second,"cameraTrack");
            auto& cue=cut.cue; cue.strCueId=shot->first; cue.iDurationMs=Milliseconds(track,"durationMs");
            if (!cue.iDurationMs) throw std::runtime_error("Empty camera track");
            const auto interpolation=Text(track,"interpolation"), easing=Text(track,"easing");
            if (interpolation=="LINEAR") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
            else if (interpolation=="CATMULL_ROM") cue.eInterpolation=VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
            else throw std::runtime_error("Unknown camera interpolation");
            if (easing=="LINEAR") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
            else if (easing=="SMOOTHSTEP") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
            else if (easing=="HOLD") cue.eEasing=VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
            else throw std::runtime_error("Unknown camera easing");
            for (const auto& k:Array(track,"keyframes"))
            {
                VALTAN_CINEMATIC_CAMERA_KEYFRAME key; key.strSceneId=Text(k,"sceneId"); key.iTimeMs=Milliseconds(k,"timeMs");
                if (key.iTimeMs>cue.iDurationMs || (!cue.Keyframes.empty() && key.iTimeMs<=cue.Keyframes.back().iTimeMs))
                    throw std::runtime_error("Invalid camera key clock");
                key.vEye=Vector(k,"eye"); key.vLookAt=Vector(k,"lookAt"); key.fFovYDegrees=static_cast<float>(Number(k,"fovYDegrees",1,179));
                if (k.Find("up")) { key.vUp=Vector(k,"up"); key.hasUp=true; }
                cue.Keyframes.push_back(key);
            }
            if (cue.Keyframes.empty() || cue.Keyframes.size()>512) throw std::runtime_error("Camera key count invalid");
            VALTAN_CINEMATIC_CAMERA_POSE pose;
            if (!CValtanCinematicCameraController::Sample_Cue(cue,0,pose)) throw std::runtime_error("Camera sampler rejected cue");
            cuts.push_back(std::move(cut));
        }
        if (cuts.empty() || cuts.front().startMs!=0) throw std::runtime_error("Missing initial camera cut");
        m_Cuts=std::move(cuts); m_Instances=std::move(instances); m_DurationMs=duration;
        return true;
    }
    catch (const std::exception& error) { m_Status=error.what(); return false; }
}

bool CMaharakaWaterpangPresentation::Initialize(const CWorldSequencePlayer::TARGET_SET& targets, std::shared_ptr<CCamera_Free> camera)
{
    m_Targets=targets; m_Camera=camera; m_Targets.objectPreparationOwner=&m_World;
    /* The loop sequences' effects (the bursts and the projectile jet tracks) sit
       on RotY(snapshot basis) * RotY(track yaw) * RotY(held object yaw). Turn only
       those effects (never the model: its head already spins in the clip) about
       the held pivot by the Server's turned angle, so they stay on the Server jet
       line. Their placement itself is the authored track data. */
    const auto previous=m_Targets.objectEffectPostTransform;
    m_Targets.objectEffectPostTransform=[previous,turn=m_CannonTurn](const std::string& instanceId,
        f32_t sourceMs, float4x4_t& out, std::string& status)
    {
        DirectX::XMStoreFloat4x4(&out,DirectX::XMMatrixIdentity());
        if (previous && !previous(instanceId,sourceMs,out,status)) return false;
        if (instanceId!=CANNON_LOOP_CW && instanceId!=CANNON_LOOP_CCW) return true;
        const DirectX::XMMATRIX spin=
            DirectX::XMMatrixTranslation(-turn->pivotX,0.f,-turn->pivotZ)*
            DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(turn->yawDegrees))*
            DirectX::XMMatrixTranslation(turn->pivotX,0.f,turn->pivotZ);
        DirectX::XMStoreFloat4x4(&out,DirectX::XMLoadFloat4x4(&out)*spin);
        return true;
    };
    if (!m_World.Load_PreparedArea(AREA,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    if (!Load_Camera()) return false;
    for (const auto& id:m_Instances)
        if (!m_World.Prepare_InstanceResources(id,m_Targets)) { m_Status=m_World.Get_Status(); return false; }
    // Attack presentation is optional: a missing instance keeps the intro, the
    // notice and the Server hazards working.
    const auto casts=[this](const char* id) { return std::find(m_Instances.begin(),m_Instances.end(),id)!=m_Instances.end(); };
    m_AttackDisabled=!casts(MOKOMOKO_HOLD) || !casts(CANNON_HOLD);
    for (const char* id:ATTACK_INSTANCES)
        if (!m_World.Get_Document().Find_Instance(id)) m_AttackDisabled=true;
    // Both loop templates must hold one pure-yaw cannon pose; the effect turn needs it.
    bool loopPoseRead=false;
    for (const char* id:{CANNON_LOOP_CW,CANNON_LOOP_CCW})
    {
        const auto* instance=m_World.Get_Document().Find_Instance(id);
        const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr;
        if (!sequence || sequence->tracks.empty() || sequence->tracks.front().keys.empty()) { m_AttackDisabled=true; continue; }
        const auto& key=sequence->tracks.front().keys.front();
        const auto& q=key.rotationQuaternion;
        const float yaw=DirectX::XMConvertToDegrees(2.f*std::atan2(q.y,q.w));
        if (std::abs(q.x)>1.e-4f || std::abs(q.z)>1.e-4f ||
            (loopPoseRead && (std::abs(yaw-m_CannonObjectYawDegrees)>1.e-3f ||
             key.positionOffset.x!=m_CannonTurn->pivotX || key.positionOffset.z!=m_CannonTurn->pivotZ)))
        { m_AttackDisabled=true; continue; }
        m_CannonObjectYawDegrees=yaw; m_CannonTurn->pivotX=key.positionOffset.x; m_CannonTurn->pivotZ=key.positionOffset.z;
        loopPoseRead=true;
    }
    if (m_AttackDisabled) OutputDebugStringA("[Waterpang] attack presentation instances are missing\n");
    m_Ready=true; return true;
}

bool CMaharakaWaterpangPresentation::Show_Actor(size_t actor, const char* instanceId, float offsetMs, uint32_t occurrence)
{
    std::string& current=m_ActorInstances[actor];
    if (current==instanceId && m_ActorOccurrences[actor]==occurrence) return true;
    if (!current.empty()) m_World.Stop_Instance(current,m_Targets,false);
    current=instanceId; m_ActorOccurrences[actor]=occurrence;
    if (m_World.Play(instanceId,m_Targets) && m_World.Seek_InstanceToMs(instanceId,offsetMs,m_Targets,true)) return true;
    m_Status=m_World.Get_Status();
    OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
    return false;
}

std::string CMaharakaWaterpangPresentation::Describe_EffectTracks(const std::string& instanceId, bool turned) const
{
    /* Each authored effect track of the instance: its line axis (mod 180) from the
       track yaw, the held object yaw when inherited, the effect turn when applied
       and the document's snapshot basis, i.e. the composition the World player
       and Effect_Playback use. Head followers ride the clip bone instead. */
    const auto* instance=m_World.Get_Document().Find_Instance(instanceId);
    const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr;
    if (!sequence) return "none";
    // The held object's authored yaw, which tracks inheriting its rotation take.
    float objectYaw=0.f;
    if (!sequence->tracks.empty() && !sequence->tracks.front().keys.empty())
    {
        const auto& q=sequence->tracks.front().keys.front().rotationQuaternion;
        objectYaw=DirectX::XMConvertToDegrees(2.f*std::atan2(q.y,q.w));
    }
    std::string text;
    for (const auto& track:sequence->effectTracks)
    {
        const auto document=CEffectCatalog::Find_Loaded(track.resourceId);
        bool root=false, head=false;
        float basis=0.f;
        if (document)
            for (const auto& element:document->Elements)
            {
                const auto& attachment=element.ActionCueAttachment;
                if (attachment.bEnabled && attachment.bFollow) head=true;
                else if (!root) { root=true; basis=attachment.bEnabled ? attachment.fSnapshotRootSourceBasisYawDegrees : 0.f; }
            }
        char row[160];
        if (!document) std::snprintf(row,sizeof(row),"%s=unprepared ",track.effectTrackId.c_str());
        else if (!root) std::snprintf(row,sizeof(row),"%s=head ",track.effectTrackId.c_str());
        else
        {
            const float yaw=basis+track.rotationDegrees.y+(track.inheritObjectRotation ? objectYaw : 0.f)+
                (turned ? m_CannonTurn->yawDegrees : 0.f);
            std::snprintf(row,sizeof(row),"%s=%.2f%s ",track.effectTrackId.c_str(),
                Line_Axis(DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.f,0.f,1.f,0.f),
                    DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(yaw)))),head ? "+head" : "");
        }
        text+=row;
    }
    return text.empty() ? "none" : text;
}

void CMaharakaWaterpangPresentation::Trace_JetAxes(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event)
{
    /* Once a second while firing: the Server line and every loop effect track's
       line axis from the authored data in use, plus whether the body turns. */
    m_LastAxisTraceMs=m_ClockMs;
    const DirectX::XMVECTOR localZ=DirectX::XMVectorSet(0.f,0.f,1.f,0.f);
    const bool loop=m_ActorInstances[1]==CANNON_LOOP_CW || m_ActorInstances[1]==CANNON_LOOP_CCW;
    float model=-1.f;
    float4x4_t pivot{};
    // The effect root is the held template pose (pure yaw); the model sample only
    // shows whether the cannon body itself turns.
    if (loop && m_World.Try_GetObjectPivot(m_ActorInstances[1],pivot))
        model=Line_Axis(DirectX::XMVector3TransformNormal(localZ,DirectX::XMLoadFloat4x4(&pivot)));
    // The head bone spins by its clip: att_battle_3_02 clockwise, 3_03 counter-clockwise.
    const char* headSpin=m_ActorInstances[1]==CANNON_LOOP_CW ? "cw" : (m_ActorInstances[1]==CANNON_LOOP_CCW ? "ccw" : "none");
    const std::string tracks=loop ? Describe_EffectTracks(m_ActorInstances[1],true) : std::string("none");
    char line[512];
    std::snprintf(line,sizeof(line),
        "occurrence=%u server=%.2f tracks=[%s] modelFacing=%.2f effectTurn=%.2f serverSpin=%s headSpin=%s instance=%s",
        event.iOccurrence,Line_Axis(DirectX::XMVector3TransformNormal(localZ,DirectX::XMMatrixRotationY(
            DirectX::XMConvertToRadians(event.fCannonYawDegrees)))),tracks.c_str(),model,m_CannonTurn->yawDegrees,
        event.bClockwise ? "cw" : "ccw",headSpin,loop ? "loop" : "none");
    Write_EffectFailureDiagnostic("maharaka.waterpang.axis",line);
}

void CMaharakaWaterpangPresentation::Update_Attacks()
{
    using namespace LostArk::Shared;
    // The intro owns both actors until its HOLD pose; the first event starts later.
    if (m_AttackDisabled || m_ClockMs<static_cast<float>(m_DurationMs)) return;
    if (!m_AttackPrepared)
    {
        // Instance resources include their V1 effect tracks, which prepare asynchronously.
        m_AttackPrepared=true;
        for (const char* id:ATTACK_INSTANCES)
            if (!m_World.Prepare_InstanceResources(id,m_Targets)) { m_AttackPrepared=false; break; }
    }
    MAHARAKA_WATERPANG_EVENT_SAMPLE event{};
    const bool live=Sample_Now(event);
    const bool waterfall=live && MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL==event.eKind;
    const bool cannonEvent=live && !waterfall;
    const float tickMs=1000.f/static_cast<float>(MAHARAKA_WATERPANG_TICK_HZ);
    const float eventMs=static_cast<float>(event.iElapsedTicks)*tickMs;
    const float telegraphMs=static_cast<float>(MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS)*tickMs;
    const float fireMs=cannonEvent ? static_cast<float>(event.iDurationTicks-MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS-
        MAHARAKA_WATERPANG_CANNON_END_TICKS)*tickMs : 0.f;
    const float holdMs=static_cast<float>(m_DurationMs);
    const char* mokoko=MOKOMOKO_HOLD; float mokokoMs=holdMs;
    const char* cannon=CANNON_HOLD; float cannonMs=holdMs;
    if (m_AttackPrepared)
    {
        if (waterfall) { mokoko=MOKOMOKO_ATTACK; mokokoMs=eventMs; }
        else if (cannonEvent && eventMs<telegraphMs) { cannon=CANNON_START; cannonMs=eventMs; }
        else if (cannonEvent && eventMs<telegraphMs+fireMs)
        {
            cannon=event.bClockwise ? CANNON_LOOP_CW : CANNON_LOOP_CCW; cannonMs=eventMs-telegraphMs;
        }
        else if (cannonEvent) { cannon=CANNON_END; cannonMs=eventMs-telegraphMs-fireMs; }
    }
    const uint32_t mokokoOccurrence=mokoko==MOKOMOKO_HOLD ? UINT32_MAX : event.iOccurrence;
    const uint32_t cannonOccurrence=cannon==CANNON_HOLD ? UINT32_MAX : event.iOccurrence;
    if (!Show_Actor(0,mokoko,mokokoMs,mokokoOccurrence) || !Show_Actor(1,cannon,cannonMs,cannonOccurrence))
    {
        // Fall back to the held intro pose and stop trying this session.
        m_AttackDisabled=true;
        (void)Show_Actor(0,MOKOMOKO_HOLD,holdMs,UINT32_MAX);
        (void)Show_Actor(1,CANNON_HOLD,holdMs,UINT32_MAX);
        return;
    }
    if (cannonEvent && event.bFiring && m_ClockMs-m_LastAxisTraceMs>=1000.f)
        Trace_JetAxes(event);
    if (waterfall && event.iOccurrence!=m_WaterfallTraceOccurrence &&
        event.iElapsedTicks>=MAHARAKA_WATERPANG_WATERFALL_HIT_TICK)
        Trace_Waterfall(event);
}

void CMaharakaWaterpangPresentation::Trace_Waterfall(const LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event)
{
    /* Once per cast at the first Server wave tick: the yellow-disc hit volume, the
       held mokoko object's facing (its +X carries the water columns) and the source
       wave origin 3.2 m ahead of it, every authored effect track (the yellow-disc
       telegraph and the ground splash stations) and the authored sound rows. */
    using namespace LostArk::Shared;
    m_WaterfallTraceOccurrence=event.iOccurrence;
    const auto heading=[](DirectX::FXMVECTOR direction)
    { return DirectX::XMConvertToDegrees(std::atan2(DirectX::XMVectorGetX(direction),DirectX::XMVectorGetZ(direction))); };
    const float sectorDegrees=DirectX::XMConvertToDegrees(std::atan2(
        MAHARAKA_WATERPANG_CANNON_X-MAHARAKA_WATERPANG_MOKOMOKO_X,MAHARAKA_WATERPANG_CANNON_Z-MAHARAKA_WATERPANG_MOKOMOKO_Z));
    float facing=0.f, originDistance=-1.f, originAngle=0.f;
    float4x4_t pivot{};
    const bool posed=m_World.Try_GetObjectPivot(MOKOMOKO_ATTACK,pivot);
    if (posed)
    {
        const DirectX::XMVECTOR forward=DirectX::XMVector3Normalize(DirectX::XMVectorSet(pivot._11,0.f,pivot._13,0.f));
        facing=heading(forward);
        // Source wave origin (4225612 AreaOffsetX 320 cm) along the object +X.
        const float x=pivot._41+DirectX::XMVectorGetX(forward)*MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M-MAHARAKA_WATERPANG_MOKOMOKO_X;
        const float z=pivot._43+DirectX::XMVectorGetZ(forward)*MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M-MAHARAKA_WATERPANG_MOKOMOKO_Z;
        originDistance=std::sqrt(x*x+z*z);
        originAngle=DirectX::XMConvertToDegrees(std::atan2(x,z))-sectorDegrees;
        originAngle=std::fmod(originAngle+540.f,360.f)-180.f;
    }
    std::string sounds;
    const auto* instance=m_World.Get_Document().Find_Instance(MOKOMOKO_ATTACK);
    if (const auto* sequence=instance ? m_World.Get_Document().Find_Template(instance->templateId) : nullptr)
        for (const auto& row:sequence->soundTracks)
            sounds+=row.soundTrackId+"@"+std::to_string(row.startMs)+"+"+std::to_string(row.durationMs)+"ms:"+
                row.assetId.substr(row.assetId.find_last_of('/')+1u)+" ";
    const std::string tracks=Describe_EffectTracks(MOKOMOKO_ATTACK,false);
    char line[768];
    std::snprintf(line,sizeof(line),
        "occurrence=%u tick=%u hit=disc(r<=%.1fm,waves@%u/%u) facingToCentre=%.2f tracks=[%s] modelFacing=%s%.2f waveOrigin=%.2fm/%+.1f sounds=[%s] instance=%s",
        event.iOccurrence,event.iElapsedTicks,MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M,MAHARAKA_WATERPANG_WATERFALL_HIT_TICK,
        MAHARAKA_WATERPANG_WATERFALL_SECOND_HIT_TICK,sectorDegrees,tracks.c_str(),posed ? "" : "unposed:",facing,
        originDistance,originAngle,sounds.empty() ? "none" : sounds.c_str(),m_ActorInstances[0].c_str());
    Write_EffectFailureDiagnostic("maharaka.waterpang.waterfall",line);
}

void CMaharakaWaterpangPresentation::Accept(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play)
{
    using namespace LostArk::Shared;
    if (play.strSequenceInstanceId == MAHARAKA_WATERPANG_INTRO_INSTANCE && play.eOperation == WORLD_SEQUENCE_OPERATION::STOP)
    { Stop(); return; }
    MAHARAKA_WATERPANG_EVENT_KIND forcedKind{};
    if (m_Ready && play.eOperation==WORLD_SEQUENCE_OPERATION::PLAY && play.iStartTick &&
        Find_MaharakaWaterpangDebugKind(play.strSequenceInstanceId,forcedKind))
    {
        // Debug F1 forced event: a newer press replaces the running one (new occurrence).
        m_Forced=true; m_ForcedKind=forcedKind; m_ForcedStartTick=play.iStartTick;
        if (!m_LastTick || static_cast<int32_t>(play.iServerTick-m_LastTick)>0) m_LastTick=play.iServerTick;
        return;
    }
    if (!m_Ready || play.strSequenceInstanceId!=MAHARAKA_WATERPANG_INTRO_INSTANCE || play.eOperation!=WORLD_SEQUENCE_OPERATION::PLAY || !play.iStartTick) return;
    if (m_Scheduled) return; // Same room reservation must not restart on duplicate delivery.
    // The reservation ends a Debug forced event on the Server; the intro poses the cast afresh.
    m_Forced=false;
    End_ForcedOnly();
    m_StartTick=play.iStartTick; m_LastTick=play.iServerTick;
    m_ClockMs=static_cast<float>(static_cast<int32_t>(play.iServerTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    m_Scheduled=true;
}

bool CMaharakaWaterpangPresentation::Start_World(float elapsedMs)
{
    // Admission can arrive before async NPC presentation is ready. Do not start
    // half a cast; wait for every exact replacement's owner before taking poses.
    for (const auto& id:m_Instances)
        for (const auto& binding:m_World.Get_Document().Find_Instance(id)->bindings)
            if (!binding.previewNpcPlacementId.empty() && (!m_Targets.previewNpc || !m_Targets.previewNpc(binding.previewNpcPlacementId))) return false;
    for (const auto& id:m_Instances)
    {
        if (!m_World.Play(id,m_Targets) || !m_World.Seek_InstanceToMs(id,elapsedMs,m_Targets,true))
        {
            m_Status=m_World.Get_Status(); m_World.Stop_All(m_Targets,true); m_Failed=true;
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str()); return false;
        }
    }
    m_WorldClockMs=elapsedMs; m_Started=true;
    m_ActorInstances[0]=MOKOMOKO_HOLD; m_ActorInstances[1]=CANNON_HOLD;
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    return true;
}

bool CMaharakaWaterpangPresentation::Sample_Now(LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& event) const
{
    using namespace LostArk::Shared;
    if (m_Forced && Sample_MaharakaWaterpangDebugEvent(m_ForcedKind,m_ForcedStartTick,m_LastTick,event)) return true;
    return m_Scheduled && Sample_MaharakaWaterpangEvent(static_cast<int32_t>(m_LastTick-m_StartTick),m_StartTick,event);
}

void CMaharakaWaterpangPresentation::Update_Collapse()
{
    using namespace LostArk::Shared;
    const auto elapsed = static_cast<int32_t>(m_LastTick - m_StartTick);
    if (!m_Scheduled || m_CollapseStarted || m_CollapseFailed ||
        elapsed < static_cast<int32_t>(MAHARAKA_WATERPANG_COLLAPSE_START_TICKS)) return;
    const auto& document = m_World.Get_Document();
    const auto* stage = document.Find_Instance(MAHARAKA_WATERPANG_INTRO_INSTANCE);
    const auto* collapse = document.Find_Instance(MAHARAKA_WATERPANG_COLLAPSE_INSTANCE);
    const auto* sequence = collapse ? document.Find_Template(collapse->templateId) : nullptr;
    // Never replace the ring by a name/position guess, or consume a partial binding set.
    bool valid = stage && collapse && collapse->enabled && sequence &&
        sequence->durationMs == MAHARAKA_WATERPANG_COLLAPSE_DURATION_MS &&
        stage->bindings.size() == 18u && collapse->bindings.size() == stage->bindings.size();
    if (valid)
        for (const auto& binding : collapse->bindings)
            valid = valid && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
                std::any_of(stage->bindings.begin(), stage->bindings.end(), [&](const auto& original)
                { return original.slotId == binding.slotId && original.targetKind == binding.targetKind &&
                    original.targetId == binding.targetId; });
    if (!valid)
        m_Status = "Waterpang collapse requires the original eighteen stage bindings and 5000 ms motion";
    else if (!m_World.Prepare_InstanceResources(MAHARAKA_WATERPANG_COLLAPSE_INSTANCE, m_Targets))
        m_Status = m_World.Get_Status();
    else
    {
        // Hand back the intro's baseline before acquiring the same placements. Other actors keep playing.
        m_World.Stop_Instance(MAHARAKA_WATERPANG_INTRO_INSTANCE, m_Targets, true);
        const float offset = static_cast<float>(elapsed - MAHARAKA_WATERPANG_COLLAPSE_START_TICKS) *
            1000.f / MAHARAKA_WATERPANG_TICK_HZ;
        if (m_World.Play(MAHARAKA_WATERPANG_COLLAPSE_INSTANCE, m_Targets) &&
            m_World.Seek_InstanceToMs(MAHARAKA_WATERPANG_COLLAPSE_INSTANCE, offset, m_Targets, true))
        {
            m_CollapseStarted = true;
            return;
        }
        m_Status = m_World.Get_Status();
        m_World.Stop_Instance(MAHARAKA_WATERPANG_COLLAPSE_INSTANCE, m_Targets, true);
    }
    m_CollapseFailed = true;
    Write_EffectFailureDiagnostic("maharaka.waterpang.collapse", m_Status);
}

void CMaharakaWaterpangPresentation::End_ForcedOnly()
{
    if (!m_ForcedOnly) return;
    m_ForcedOnly=false;
    if (m_Started) m_World.Stop_All(m_Targets,true);
    m_Started=false; m_Notice=-1; m_ReadinessWait=0.f;
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
}

void CMaharakaWaterpangPresentation::Update(float delta, uint32_t serverTick, bool editing)
{
    using namespace LostArk::Shared;
    if (!m_Ready || m_Failed || (!m_Scheduled && !m_Forced)) return;
    if (static_cast<int32_t>(serverTick-m_LastTick)>0) m_LastTick=serverTick;
    // A forced event past its end hands the cast back to the schedule (or to nothing).
    MAHARAKA_WATERPANG_EVENT_SAMPLE forcedSample{};
    if (m_Forced && static_cast<int32_t>(m_LastTick-m_ForcedStartTick)>=0 &&
        !Sample_MaharakaWaterpangDebugEvent(m_ForcedKind,m_ForcedStartTick,m_LastTick,forcedSample))
        m_Forced=false;
    if (m_Scheduled)
        m_ClockMs=static_cast<float>(static_cast<int32_t>(m_LastTick-m_StartTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    else
    {
        // No match: a forced event poses the held cast without countdown, intro camera or schedule.
        if (!m_Forced) { End_ForcedOnly(); return; }
        if (!m_ForcedOnly) { m_ForcedOnly=true; m_ForcedOnlyBaseTick=m_LastTick; }
        m_ClockMs=static_cast<float>(m_DurationMs)+
            static_cast<float>(static_cast<int32_t>(m_LastTick-m_ForcedOnlyBaseTick))*1000.f/MAHARAKA_WATERPANG_TICK_HZ;
    }
    // A frozen/disconnected Server cannot locally advance the match to its start.
    m_Countdown=m_ClockMs<0 ? static_cast<int>(std::ceil(-m_ClockMs/1000.f)) : 0;
    // The Server applies the same shared schedule; the notice follows its clock.
    MAHARAKA_WATERPANG_EVENT_SAMPLE event{};
    const bool live=Sample_Now(event);
    m_Notice=live ? static_cast<int>(event.eKind) : -1;
    // Before the world samples this frame: RotY(-90) * RotY(object) * RotY(turn) == RotY(Server jet yaw).
    if (live && MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL!=event.eKind)
        m_CannonTurn->yawDegrees=event.fCannonYawDegrees+90.f-m_CannonObjectYawDegrees;
    const auto camera=m_Camera.lock();
    if (editing) { Suspend_ForAuthoring(); return; }
    if (m_ClockMs<0) return;
    if (!m_Started && !Start_World(m_ClockMs))
    {
        m_ReadinessWait += delta;
        if (!m_Failed && m_ReadinessWait >= 5.f && !m_Scheduled)
        {
            // A forced event alone never disables the match presentation; drop just it.
            m_Forced=false; m_ReadinessWait=0.f;
            m_Status="Waterpang forced event dropped: NPC presentation not ready";
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
            return;
        }
        if (!m_Failed && m_ReadinessWait >= 5.f)
        {
            m_Failed=true; m_Status="Waterpang NPC presentation readiness timed out";
            OutputDebugStringA(("[Waterpang] "+m_Status+"\n").c_str());
        }
        return;
    }
    m_World.Update((std::max)(0.f,m_ClockMs-m_WorldClockMs)*.001f,m_Targets);
    m_WorldClockMs=m_ClockMs;
    Update_Attacks();
    Update_Collapse();
    if (!camera) return;
    if (m_ClockMs>=m_DurationMs) { camera->End_PresentationOverride(CAMERA_OWNER); return; }
    const CUT* cut=&m_Cuts.front();
    for (const auto& row:m_Cuts) if (row.startMs<=m_ClockMs) cut=&row;
    VALTAN_CINEMATIC_CAMERA_POSE pose;
    if (CValtanCinematicCameraController::Sample_Cue(cut->cue,(m_ClockMs-cut->startMs)*.001f,pose) &&
        camera->Begin_PresentationOverride(CAMERA_OWNER,Engine::CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
    {
        if (pose.hasUp) camera->Apply_PresentationPoseWithUp(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.vUp,pose.fFovYDegrees);
        else camera->Apply_PresentationPose(CAMERA_OWNER,pose.vEye,pose.vLookAt,pose.fFovYDegrees);
    }
}

void CMaharakaWaterpangPresentation::Render() const
{
    const auto viewport=Engine::CGameInstance::Get().Get_ViewportSize();
    const float scale=(std::min)(viewport.x/1280.f,viewport.y/720.f);
    // EFTable_GameMsg se_announce_52 / 53 / 51, in MAHARAKA_WATERPANG_EVENT_KIND order.
    static const wchar_t* const NOTICES[]={
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC6CC\uD130\uCE90\uB17C\uC774 \uD68C\uC804\uD569\uB2C8\uB2E4.",
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC6CC\uD130\uCE90\uB17C\uC774 \uBE60\uB974\uAC8C \uD68C\uC804\uD569\uB2C8\uB2E4.",
        L"\uBAA8\uCF54\uBAA8\uCF54 \uC5B4\uD2B8\uB809\uC158\uC5D0\uC11C \uBB3C\uBCBC\uB77D\uC774 \uC3DF\uC544\uC9D1\uB2C8\uB2E4."};
    if (m_Notice>=0 && m_Notice<3)
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"),NOTICES[m_Notice],viewport.x*.5f,viewport.y*.18f,22.f*scale,DirectX::Colors::White);
    if (m_Scheduled && !m_Failed && m_LastTick)
    {
        using namespace LostArk::Shared;
        const auto elapsed = static_cast<int32_t>(m_LastTick - m_StartTick);
        const auto begin = MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
        if (elapsed >= static_cast<int32_t>(begin))
        {
            const auto ticks = elapsed < static_cast<int32_t>(MAHARAKA_WATERPANG_MATCH_END_TICKS) ?
                MAHARAKA_WATERPANG_MATCH_END_TICKS - static_cast<uint32_t>(elapsed) : 0u;
            const auto seconds = (ticks + MAHARAKA_WATERPANG_TICK_HZ - 1u) / MAHARAKA_WATERPANG_TICK_HZ;
            wchar_t remaining[64]{};
            std::swprintf(remaining, 64, L"Waterpang  %02u:%02u", seconds / 60u, seconds % 60u);
            UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), remaining, viewport.x * .5f, viewport.y * .11f, 22.f * scale, DirectX::Colors::White);
        }
    }
    if (!m_Countdown || m_Failed) return;
    const std::wstring text=L"\uC6CC\uD130\uD321 \uC544\uB808\uB098\uAC00 "+std::to_wstring(m_Countdown)+L"\uCD08 \uB4A4\uC5D0 \uC2DC\uC791\uD569\uB2C8\uB2E4";
    UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"),text.c_str(),viewport.x*.5f,viewport.y*.25f,26.f*scale,DirectX::Colors::Yellow);
}

void CMaharakaWaterpangPresentation::Stop()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    m_World.Stop_All(m_Targets,true); m_Scheduled=false; m_Started=false; m_Countdown=0; m_Notice=-1;
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    m_Forced=m_ForcedOnly=false;
    m_CollapseStarted=m_CollapseFailed=false;
}
void CMaharakaWaterpangPresentation::Suspend_ForAuthoring()
{
    if (const auto camera=m_Camera.lock()) camera->End_PresentationOverride(CAMERA_OWNER);
    if (m_Started) m_World.Stop_All(m_Targets,true);
    m_ActorInstances[0].clear(); m_ActorInstances[1].clear();
    m_ActorOccurrences[0]=m_ActorOccurrences[1]=UINT32_MAX;
    m_Started=false; m_Countdown=0; m_ReadinessWait=0;
    m_CollapseStarted=m_CollapseFailed=false;
    // Keep the Server reservation. Closing the tool seeks to the current room
    // time (including HOLD after the intro), rather than replaying the event.
}
CMaharakaWaterpangPresentation::~CMaharakaWaterpangPresentation() { Stop(); }

HRESULT Client::CMaharakaWaterpangPresentation::Ensure_WaterGunPrototype(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, const uint32_t iLevelIndex, std::string& status)
	{
		if (nullptr != CGameInstance::Get().Clone_Prototype(iLevelIndex, WATER_GUN_PROTOTYPE_TAG))
			return S_FALSE;
		Engine::MODEL_ASSET_LOAD_DESC load;
		if (!CActorCatalog::Build_ModelLoadDescription("Character/Maharaka/WaterGun/ITR_02164/ITR_02164.wmodel", load, status))
			return E_FAIL;
		/* Cooked in UE centimetres with axes (x, z, y). The prop bone frame is
		   (x, y, -z) and already carries the class cm-to-m basis. */
		auto model = Engine::CModel::Create(pDevice, pContext, MODEL::NONANIM, load,
			XMMatrixRotationX(XMConvertToRadians(-90.f)));
		if (!model || !model->Get_NumMeshes())
		{
			status = "water gun model failed to load";
			return E_FAIL;
		}
		std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
		staged.emplace_back(WATER_GUN_PROTOTYPE_TAG, std::move(model));
		if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		{
			status = "water gun prototype commit failed";
			return E_FAIL;
		}
		return S_OK;
	}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ColosseumCombatPolicy.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

namespace
{
	constexpr float KOUKU_FALL_DEPTH_M = 5.f;
	constexpr float WATERPANG_FALL_DEPTH_M = 2.f;
	// The casino chairs sit above the generic five-metre fall plane. A body
	// leaving its support by more than the forced-move step limit is out.
	constexpr float KOUKU_CASINO_FALL_DEPTH_M = 1.f;

	enum class FORCED_SURFACE_RESULT
	{
		SUPPORTED,
		BLOCKED,
		FALL
	};

	/* Forced motion follows physical support, not the walking graph. A
	   non-walkable surface can support a pushed body, while a lower deck must
	   never become an instantaneous landing. Collision has already resolved
	   this straight segment; no nearest-cell projection is permitted here. */
	FORCED_SURFACE_RESULT Trace_ForcedSurface(
		const LostArk::Server::CServerNavigation& navigation,
		const LostArk::Server::SERVER_NAV_POINT& from,
		const float toX, const float toZ,
		LostArk::Server::SERVER_NAV_POINT& outPoint)
	{
		outPoint = from;
		const float distance = std::hypot(toX - from.x, toZ - from.z);
		const float sampleStep = std::clamp(navigation.Get_CellSize() * 0.5f, 0.01f, 0.25f);
		if (!std::isfinite(distance) || !std::isfinite(from.y) ||
			!std::isfinite(sampleStep) || distance / sampleStep > 4096.f)
		{
			return FORCED_SURFACE_RESULT::BLOCKED;
		}
		/* Keep forced support bounded even if an older navigation policy uses
		   zero to permit arbitrary walking height changes. */
		const float authoredStep = navigation.Get_MaximumTraversalStepHeight();
		const float maximumStep = authoredStep > 0.f ? (std::min)(authoredStep, 1.f) : 1.f;
		const auto count = static_cast<std::uint32_t>((std::max)(1.f, std::ceil(distance / sampleStep)));
		for (std::uint32_t sample = 0u; sample <= count; ++sample)
		{
			const float ratio = static_cast<float>(sample) / static_cast<float>(count);
			const float x = from.x + (toX - from.x) * ratio;
			const float z = from.z + (toZ - from.z) * ratio;
			LostArk::Server::SERVER_NAV_POINT ground{};
			if (!navigation.Sample_SurfacePosition(x, z, ground) ||
				!std::isfinite(ground.y) || ground.y < outPoint.y - maximumStep)
			{
				outPoint.x = x;
				outPoint.z = z;
				return FORCED_SURFACE_RESULT::FALL;
			}
			if (ground.y > outPoint.y + maximumStep)
				return FORCED_SURFACE_RESULT::BLOCKED;
			outPoint = ground;
		}
		return FORCED_SURFACE_RESULT::SUPPORTED;
	}
}

float LostArk::Server::CGameRoom::Resolve_StanceMoveSpeedScale(
	const SERVER_PLAYER& player) const
{
	const PLAYER_RUNTIME_PROFILE* profile =
		m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile ||
		!CPlayerSkillSystem::Is_HoldingGaugedStance(player, *profile))
	{
		return 1.f;
	}
	return profile->fDefenseStanceMoveSpeedScale;
}

void LostArk::Server::CGameRoom::Refresh_PlayerBlockingBodies()
{
	std::vector<SERVER_BLOCKING_BODY> bodies;
	bodies.reserve(m_WorldEntities.size());
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon ||
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
			SERVER_ENTITY_ACTION::DEAD == entity.eAction ||
			(WORLD_BOOTSTRAP_KIND::NPC != entity.eKind &&
			 0u == entity.iCurrentHp))
		{
			continue;
		}
		/* Same body the skill hit test uses: monsters carry their profile
		radius, the boss reads its profile, and town NPCs use the shared upright
		player-sized body until the catalog owns a dedicated gameplay radius. */
		float radius = entity.fCollisionRadius;
		float centerY = entity.fPositionY + radius;
		float halfHeight = radius;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			if (const BOSS_RUNTIME_PROFILE* bossProfile =
				m_GameplayCatalog.Find_Boss(entity.strArchetypeId))
			{
				radius = bossProfile->fCollisionRadius;
			}
		}
		else if (WORLD_BOOTSTRAP_KIND::NPC == entity.eKind)
		{
			using namespace LostArk::Shared::WorldCollision;
			radius = PLAYER_HALF_EXTENT_X;
			centerY = entity.fPositionY + PLAYER_CENTER_OFFSET_Y;
			halfHeight = PLAYER_HALF_EXTENT_Y;
		}
		else if (WORLD_BOOTSTRAP_KIND::MONSTER != entity.eKind)
		{
			continue;
		}
		if (radius <= 0.f)
			continue;
		if (WORLD_BOOTSTRAP_KIND::NPC != entity.eKind)
		{
			centerY = entity.fPositionY + radius;
			halfHeight = radius;
		}
		bodies.push_back(SERVER_BLOCKING_BODY{
			entity.fPositionX, entity.fPositionZ, radius,
			centerY, halfHeight, entity.iNetEntityId });
	}
	m_ServerCollisionSystem.Set_BlockingBodies(std::move(bodies));
}

void LostArk::Server::CGameRoom::Begin_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	const float fallDepth = m_eWorldId == WORLD_ID::MAHARAKA ? WATERPANG_FALL_DEPTH_M :
		(Resolve_KoukuFallCenter(player) ? KOUKU_CASINO_FALL_DEPTH_M : KOUKU_FALL_DEPTH_M);
	player.fFallDeathPlaneY = (player.bKnockbackBallistic ? player.fKnockbackSupportY : player.fPositionY) - fallDepth;
	player.eAction = PLAYER_ACTION_STATE::FALLING;
	player.bKoukuFallDeath = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA && !player.iMarioStage;
	player.bWaterpangFall = false;
	player.bWaterpangLaunch = false;
	player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
	player.iFallDeathTick = Add_ServerTicksSkippingReservedZero(
		player.iActionStartTick, FALL_DEATH_TICKS);
	player.fFallVelocityY = 0.f;
	/* Everything the fall interrupts is cleared here instead of inside each
	system, so no half-finished action can resume when the body lands dead. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0;
	player.iSpawnedProjectileMask = 0;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	/* The ejection/ballistic phase ends at this boundary.  Keep the ordinary
	   FALLING integrator authoritative after the edge crossing; leaving either
	   typed flight flag set would make Update_PlayerFall return early forever
	   and the player could never reach the dead-zone deadline. */
	player.bArenaEjectionActive = false;
	player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
	player.bKnockbackCanLeaveArena = false;
	player.bKnockbackBallistic = false;
	player.fKnockbackVelocityY = 0.f;
	player.fKnockbackLaunchY = 0.f;
	player.iKnockdownEndTick = 0u;
	player.Clear_Attachment();
	/* Every boss and monster gate already refuses a player that is not combat
	ready, so this one flag removes the falling body from acquisition and from
	area damage without editing four separate target filters. */
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	/* The edge that opens the hole is also the first tick of the descent, so
	the body integrates here instead of hanging one tick at the old height and
	broadcasting a FALLING snapshot that has not moved. The deadline was just
	set a full FALL_DEATH_TICKS away, so it cannot be due on this tick. */
	player.fFallVelocityY -=
		FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
	player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
}

bool LostArk::Server::CGameRoom::Capture_PlayerAttachment(
	const LostArk::Shared::NET_ENTITY_ID playerEntityId,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
	const std::uint32_t serverTick, const std::uint32_t holdEndTick, const std::uint32_t sourcePatternSequence)
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == playerEntityId ||
		INVALID_NET_ENTITY_ID == ownerEntityId ||
		playerEntityId == ownerEntityId || 0u == serverTick ||
        (holdEndTick && (Has_ReachedServerTick(serverTick, holdEndTick) || holdEndTick - serverTick > 18001u)) ||
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != slot)
	{
		return false;
	}

	const auto playerId = m_PlayerIdByEntityId.find(playerEntityId);
	if (m_PlayerIdByEntityId.end() == playerId)
		return false;
	const auto playerIter = m_Players.find(playerId->second);
	if (m_Players.end() == playerIter)
		return false;
	const auto liveOwner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	auto* owner = sourcePatternSequence && m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, sourcePatternSequence) :
		(liveOwner == m_WorldEntities.end() ? nullptr : &*liveOwner);
	if (nullptr == owner ||
		WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
		SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
		0u == owner->iCurrentHp || 0u == owner->iPatternSequence ||
		!std::isfinite(owner->fPositionX) ||
		!std::isfinite(owner->fPositionY) ||
		!std::isfinite(owner->fPositionZ) ||
		!std::isfinite(owner->fYawDegrees))
	{
		return false;
	}

	SERVER_PLAYER& player = playerIter->second;
	if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
	{
		return player.iAttachmentOwnerNetEntityId == ownerEntityId &&
			player.eAttachmentSlot == slot &&
			player.iAttachmentPatternSequence == owner->iPatternSequence &&
            player.iAttachmentEndTick == holdEndTick;
	}
	if (0u == player.iCurrentHp || !player.isCombatReady || player.Has_TimeStop(serverTick) ||
        PLAYER_ACTION_STATE::FEAR == player.eAction ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		!std::isfinite(player.fPositionX) ||
		!std::isfinite(player.fPositionY) ||
		!std::isfinite(player.fPositionZ) ||
		!std::isfinite(player.fYawDegrees))
	{
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float deltaX = player.fPositionX - owner->fPositionX;
	const float deltaZ = player.fPositionZ - owner->fPositionZ;
	const float localX = deltaX * cosine - deltaZ * sine;
	const float localY = player.fPositionY - owner->fPositionY;
	const float localZ = deltaX * sine + deltaZ * cosine;
	const float localYaw = Wrap_Degrees(
		player.fYawDegrees - owner->fYawDegrees);
	if (!std::isfinite(localX) || !std::isfinite(localY) ||
		!std::isfinite(localZ) || !std::isfinite(localYaw))
	{
		return false;
	}

	/* Capture interrupts one complete action transaction. Projectiles and
	combat objects cannot remain owned by a body whose input is now frozen. */
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0u;
	player.iSpawnedProjectileMask = 0u;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.Clear_Attachment();
	player.iAttachmentOwnerNetEntityId = ownerEntityId;
	player.eAttachmentSlot = slot;
	player.iAttachmentPatternSequence = owner->iPatternSequence;
    player.iAttachmentEndTick = holdEndTick;
	player.fAttachmentLocalOffsetX = localX;
	player.fAttachmentLocalOffsetY = localY;
	player.fAttachmentLocalOffsetZ = localZ;
	player.fAttachmentYawOffsetDegrees = localYaw;
	player.eAction = PLAYER_ACTION_STATE::GRABBED;
	player.iActionStartTick = serverTick;
	player.isCombatReady = false;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	return true;
}

bool LostArk::Server::CGameRoom::Release_PlayerAttachment(
	SERVER_PLAYER& player,
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
		player.iAttachmentOwnerNetEntityId != ownerEntityId ||
		0u == serverTick || !std::isfinite(pushRangeM))
	{
		return false;
	}

	// Every hook exit, including cancellation and an owner disappearing, must
	// land before input is unlocked. A dodge must never start from an air pose.
	if (player.iCurrentHp && !player.iMarioStage && Resolve_CurrentKoukuGate() == 3u &&
		player.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP)
	{
		SERVER_NAV_POINT landing{};
		constexpr float maximumDistance = 2.f * WorldCollision::PLAYER_HALF_EXTENT_Y;
		const auto clearFloor = [&] {
			return std::isfinite(landing.x) && std::isfinite(landing.y) && std::isfinite(landing.z) &&
				m_ServerNavigation.Is_PointWalkableExact(landing.x, landing.z, landing.y) &&
				m_ServerCollisionSystem.Is_PlayerPositionClear(landing.x, landing.y, landing.z, player.iNetEntityId);
		};
		const auto nearbyFloor = [&] {
			return clearFloor() && std::abs(landing.y - player.fPositionY) <= maximumDistance &&
				std::hypot(landing.x - player.fPositionX, landing.z - player.fPositionZ) <= maximumDistance &&
				m_ServerNavigation.Is_InSameNavigationGrid(player.fPositionX, player.fPositionZ, landing.x, landing.z);
		};
		bool grounded = m_ServerNavigation.Is_Loaded() &&
			std::isfinite(player.fPositionX) && std::isfinite(player.fPositionY) && std::isfinite(player.fPositionZ) &&
			((m_ServerNavigation.Project_PointOnSameLevel(player.fPositionX, player.fPositionZ, landing, player.fPositionY) && nearbyFloor()) ||
			 (m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, landing, player.fPositionY) && nearbyFloor()));
		if (!grounded)
		{
			SERVER_NAV_POINT start{}; float yaw = 0.f;
			grounded = m_ServerNavigation.Is_Loaded() && Resolve_KoukuRevivePosition(player, start, yaw) &&
				m_ServerNavigation.Project_Point(start.x, start.z, landing, start.y) && clearFloor() &&
				std::abs(landing.y - start.y) <= maximumDistance &&
				std::hypot(landing.x - start.x, landing.z - start.z) <= maximumDistance &&
				m_ServerNavigation.Is_InSameNavigationGrid(start.x, start.z, landing.x, landing.z);
		}
		if (!grounded)
		{
			player.isCombatReady = false;
			m_strStatus = "Kouku hook release is waiting for a safe arena floor";
			return false;
		}
		player.fPositionX = landing.x;
		player.fPositionY = landing.y;
		player.fPositionZ = landing.z;
		player.fFallVelocityY = 0.f;
		player.iFallDeathTick = 0u;
	}

	float sourceX = player.fPositionX;
	float sourceZ = player.fPositionZ;
	const auto owner = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	if (m_WorldEntities.end() != owner &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionZ))
	{
		sourceX = owner->fPositionX;
		sourceZ = owner->fPositionZ;
	}

	player.Clear_Attachment();
	player.eAction = 0u == player.iCurrentHp ?
		PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackRemainingSeconds = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = 0u != player.iCurrentHp;
	if (0u != player.iCurrentHp)
	{
		CPlayerSkillSystem::Arm_PlayerHitReaction(
			player, sourceX, sourceZ, pushRangeM, pushMs,
			knockdown, downMs, serverTick);
	}
	return true;
}

std::size_t LostArk::Server::CGameRoom::Release_PlayerAttachments(
	const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
	const float pushRangeM,
	const std::uint32_t pushMs,
	const bool knockdown,
	const std::uint32_t downMs,
	const std::uint32_t serverTick)
{
	std::size_t released = 0u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (Release_PlayerAttachment(
			player, ownerEntityId, pushRangeM, pushMs,
			knockdown, downMs, serverTick))
		{
			++released;
		}
	}
	return released;
}

bool LostArk::Server::CGameRoom::Update_PlayerAttachment(
	SERVER_PLAYER& player,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != player.eAction)
	{
		if (INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::NONE != player.eAttachmentSlot ||
			(0u != player.iAttachmentPatternSequence || 0u != player.iAttachmentEndTick))
		{
			player.Clear_Attachment();
		}
		return false;
	}

	const NET_ENTITY_ID ownerEntityId =
		player.iAttachmentOwnerNetEntityId;
    if (player.iCurrentHp == 0u || (player.iAttachmentEndTick && Has_ReachedServerTick(serverTick, player.iAttachmentEndTick)))
    {
        (void)Release_PlayerAttachment(player, ownerEntityId, 0.f, 0u, false, 0u, serverTick ? serverTick : 1u);
        return player.eAction == PLAYER_ACTION_STATE::GRABBED;
    }
	const auto body = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[ownerEntityId](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId == ownerEntityId;
		});
	const auto* owner = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ?
		Find_KoukuOccurrenceOwner(ownerEntityId, player.iAttachmentPatternSequence) :
		(body == m_WorldEntities.end() ? nullptr : &*body);
	const bool liveOwner = nullptr != owner &&
		WORLD_BOOTSTRAP_KIND::BOSS == owner->eKind &&
		SERVER_ENTITY_ACTION::DEAD != owner->eAction &&
		0u != owner->iCurrentHp && 0u != owner->iPatternSequence &&
		player.iAttachmentPatternSequence == owner->iPatternSequence;
	/* A World Object carries this player, not a boss bone. The KoukuSaydon logic
	runtime writes the transform from the authored region every tick it runs, so
	the pose it wrote stands; all that is owned here is the deadline that region
	gave us and the ordinary release once it passes. */
	if (PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP == player.eAttachmentSlot)
	{
		if (!liveOwner || 0u == player.iAttachmentReleaseTick ||
			CKoukuSaydonLogicRuntime::Has_ReachedTick(
				serverTick, player.iAttachmentReleaseTick))
		{
			(void)Release_PlayerAttachment(
				player, ownerEntityId, 0.f, 0u, false, 0u,
				0u == serverTick ? 1u : serverTick);
			return player.eAction == PLAYER_ACTION_STATE::GRABBED;
		}
		player.isCombatReady = false;
		return true;
	}
	const bool validOwner = liveOwner &&
		PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND == player.eAttachmentSlot &&
		std::isfinite(owner->fPositionX) &&
		std::isfinite(owner->fPositionY) &&
		std::isfinite(owner->fPositionZ) &&
		std::isfinite(owner->fYawDegrees) &&
		std::isfinite(player.fAttachmentLocalOffsetX) &&
		std::isfinite(player.fAttachmentLocalOffsetY) &&
		std::isfinite(player.fAttachmentLocalOffsetZ) &&
		std::isfinite(player.fAttachmentYawOffsetDegrees);
	if (!validOwner)
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	const float yawRadians = owner->fYawDegrees * DEGREES_TO_RADIANS;
	const float sine = std::sin(yawRadians);
	const float cosine = std::cos(yawRadians);
	const float nextX = owner->fPositionX +
		player.fAttachmentLocalOffsetX * cosine +
		player.fAttachmentLocalOffsetZ * sine;
	const float nextY = owner->fPositionY +
		player.fAttachmentLocalOffsetY;
	const float nextZ = owner->fPositionZ -
		player.fAttachmentLocalOffsetX * sine +
		player.fAttachmentLocalOffsetZ * cosine;
	const float nextYaw = Wrap_Degrees(
		owner->fYawDegrees + player.fAttachmentYawOffsetDegrees);
	if (!std::isfinite(nextX) || !std::isfinite(nextY) ||
		!std::isfinite(nextZ) || !std::isfinite(nextYaw))
	{
		(void)Release_PlayerAttachment(
			player, ownerEntityId, 0.f, 0u, false, 0u,
			0u == serverTick ? 1u : serverTick);
		return false;
	}

	player.fPositionX = nextX;
	player.fPositionY = nextY;
	player.fPositionZ = nextZ;
	player.fYawDegrees = nextYaw;
	player.isCombatReady = false;
	return true;
}

/* Owns the whole falling life cycle of one player inside one tick: it starts
a fall when the authored ground under the player is gone, advances a running
fall, and turns it into the ordinary death the revive path already
understands. Returning true is what keeps trigger motion, skills and movement
from running at all this tick. */
bool LostArk::Server::CGameRoom::Restore_PatternBoundPlayer(
	SERVER_PLAYER& player)
{
	float restoreX = player.fPatternBindRestoreX;
	float restoreY = player.fPatternBindRestoreY;
	float restoreZ = player.fPatternBindRestoreZ;
	bool resolved = std::isfinite(restoreX) && std::isfinite(restoreY) &&
		std::isfinite(restoreZ);
	if (m_ServerNavigation.Is_Loaded())
	{
		resolved = false;
		SERVER_NAV_POINT ground{};
		if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPatternBindRestoreX,
				player.fPatternBindRestoreZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPatternBindRestoreX;
			restoreY = ground.y;
			restoreZ = player.fPatternBindRestoreZ;
			resolved = true;
		}
		else if (std::isfinite(player.fPatternBindRestoreX) &&
			std::isfinite(player.fPatternBindRestoreZ) &&
			(m_ServerNavigation.Project_PointOnSameLevel(
			player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground) ||
			m_ServerNavigation.Project_Point(
				player.fPatternBindRestoreX, player.fPatternBindRestoreZ, ground)) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = ground.x;
			restoreY = ground.y;
			restoreZ = ground.z;
			resolved = true;
		}
		if (!resolved && std::isfinite(player.fPositionX) &&
			std::isfinite(player.fPositionZ) &&
			m_ServerNavigation.Is_PointWalkableExact(
				player.fPositionX, player.fPositionZ) &&
			m_ServerNavigation.Sample_Position(
				player.fPositionX, player.fPositionZ, ground) &&
			std::isfinite(ground.x) && std::isfinite(ground.y) &&
			std::isfinite(ground.z))
		{
			restoreX = player.fPositionX;
			restoreY = ground.y;
			restoreZ = player.fPositionZ;
			resolved = true;
		}
		if (!resolved && !player.strSpawnPlacementId.empty())
		{
			const WORLD_BOOTSTRAP_PLACEMENT* spawn =
				Find_Placement(player.strSpawnPlacementId);
			if (nullptr != spawn &&
				m_ServerNavigation.Is_PointWalkableExact(
					spawn->fPositionX, spawn->fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					spawn->fPositionX, spawn->fPositionZ, ground) &&
				std::isfinite(ground.x) && std::isfinite(ground.y) &&
				std::isfinite(ground.z))
			{
				restoreX = spawn->fPositionX;
				restoreY = ground.y;
				restoreZ = spawn->fPositionZ;
				resolved = true;
			}
		}
	}
	if (!resolved)
		return false;
	const float restoreYaw = std::isfinite(player.fPatternBindRestoreYawDegrees) ?
		player.fPatternBindRestoreYawDegrees :
		(std::isfinite(player.fYawDegrees) ? player.fYawDegrees : 0.f);
	player.fPositionX = restoreX;
	player.fPositionY = restoreY;
	player.fPositionZ = restoreZ;
	player.fYawDegrees = restoreYaw;
	player.eAction = 0u == player.iCurrentHp ?
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD :
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	player.isCombatReady = 0u != player.iCurrentHp &&
		player.bPatternBindRestoreCombatReady;
	player.Clear_PatternBindStatus();
	return true;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const
{
	if (m_eWorldId != LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA || player.iMarioStage ||
		player.eKoukuAreaHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE) return nullptr;
	// Refinement grid rectangles are only bake coverage, not arena bounds.
	// The casino continues onto the base grid. Classify the separated authored
	// stages by their spawn anchors, not by Gate2Fine's small rectangle.
	const WORLD_BOOTSTRAP_PLACEMENT* nearest = nullptr;
	float distance = (std::numeric_limits<float>::max)();
	for (const char* id : { "stage.kakul.sl01", "stage.kakul.sl02", "stage.kakul.sl03",
		"stage.kakul.sl04", "stage.kakul.sl05" })
	{
		const auto* marker = Find_Placement(id);
		if (!marker || marker->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN) continue;
		const float dx = player.fPositionX - marker->fPositionX;
		const float dz = player.fPositionZ - marker->fPositionZ;
		const float candidate = dx * dx + dz * dz;
		if (candidate < distance) { nearest = marker; distance = candidate; }
	}
	return nearest && nearest->strPlacementId == "stage.kakul.sl03" ? nearest : nullptr;
}

bool LostArk::Server::CGameRoom::Try_KoukuWalkOffFloor(
	SERVER_PLAYER& player, const float x, const float z,
	const float fixedDeltaSeconds, const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || !m_ServerNavigation.Is_Loaded() ||
		!player.iCurrentHp || player.TriggerMove.isActive || player.bArenaEjectionActive ||
		(player.iVehicleId == ANCIENT_SEA_VEHICLE_ID && player.eAction == PLAYER_ACTION_STATE::VEHICLE_SKILL &&
		 player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)) return false;
	const auto* gate2 = Resolve_KoukuFallCenter(player);
	const bool casino = nullptr != gate2;
	if (!player.iMarioStage && !casino) return false;
	SERVER_NAV_POINT surface{};
	// Obstacles keep their physical support. A blocked walking cell alone
	// must never be interpreted as a hole.
	const auto supported = [&](const float px, const float pz) {
		return m_ServerNavigation.Sample_SurfacePosition(px, pz, surface) &&
			surface.y >= player.fPositionY - m_ServerNavigation.Get_MaximumTraversalStepHeight();
	};
	if (supported(x, z)) return false;
	float resolvedX{}, resolvedY{}, resolvedZ{};
	bool blocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(player, x, player.fPositionY, z,
		resolvedX, resolvedY, resolvedZ, blocked) || blocked ||
		supported(resolvedX, resolvedZ)) return false;
	if (casino && !player.iMarioStage)
	{
		SERVER_NAV_POINT center{};
		if (!m_ServerNavigation.Project_Point(gate2->fPositionX, gate2->fPositionZ, center, gate2->fPositionY)) return false;
		player.KoukuFallRevivePosition = std::array<float, 3u>{ center.x, center.y, center.z };
	}
	player.fPositionX = resolvedX;
	player.fPositionZ = resolvedZ;
	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

bool LostArk::Server::CGameRoom::Update_PlayerFall(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds,
	const std::uint32_t updateTick)
{
	using namespace LostArk::Shared;
	if ((player.bArenaEjectionActive ||
		(player.bKnockbackBallistic && player.fKnockbackRemainingSeconds > 0.f)) && 0u != player.iCurrentHp)
		return false;
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence && player.iCurrentHp && player.eAction == PLAYER_ACTION_STATE::FALLING)
	{
		SERVER_NAV_POINT start{}, ground{}; float yaw = 0.f;
		if (Resolve_KoukuRevivePosition(player, start, yaw) &&
			(!m_ServerNavigation.Is_Loaded() || m_ServerNavigation.Project_Point(start.x, start.z, ground, start.y)))
		{
			if (!m_ServerNavigation.Is_Loaded()) ground = start;
			player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
			player.fFallVelocityY = 0.f; player.iFallDeathTick = 0u;
			player.eAction = PLAYER_ACTION_STATE::NONE; player.isCombatReady = true;
		}
		return true;
	}
	if (PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.fFallVelocityY -=
			FALL_GRAVITY_METERS_PER_SECOND_SQUARED * fixedDeltaSeconds;
		player.fPositionY += player.fFallVelocityY * fixedDeltaSeconds;
		/* Signed difference so a wrapped tick counter keeps ordering, the same
		rule the cooldown deadlines use. */
		const std::int32_t sinceDeadline = static_cast<std::int32_t>(
			updateTick - player.iFallDeathTick);
		const bool reachedDeath = (m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA ||
			(m_eWorldId == WORLD_ID::MAHARAKA && player.bWaterpangFall)) ?
			(!std::isfinite(player.fFallDeathPlaneY) || player.fPositionY <= player.fFallDeathPlaneY) : sinceDeadline >= 0;
		if (!std::isfinite(player.fPositionY) || reachedDeath)
		{
			/* A Waterpang fall shows its descent, then returns the player alive on
			   the jump box nearest the fall so they can jump back onto the arena. */
			if (player.bWaterpangFall && player.iCurrentHp)
			{
				const WORLD_BOOTSTRAP_PLACEMENT* nearest = nullptr;
				float nearestDistance = (std::numeric_limits<float>::max)();
				for (const char* id : { "jump1", "jump2" })
				{
					const auto* box = Find_Placement(id);
					if (!box) continue;
					const float dx = player.fPositionX - box->fPositionX;
					const float dz = player.fPositionZ - box->fPositionZ;
					if (dx * dx + dz * dz < nearestDistance) { nearest = box; nearestDistance = dx * dx + dz * dz; }
				}
				SERVER_NAV_POINT ground{};
				if (nearest && m_ServerNavigation.Project_Point(
					nearest->fPositionX, nearest->fPositionZ, ground, nearest->fPositionY))
				{
					player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
					player.fFallVelocityY = 0.f; player.iFallDeathTick = 0u;
					player.eAction = PLAYER_ACTION_STATE::NONE; player.iActionStartTick = 0u;
					player.isCombatReady = true; player.bWaterpangFall = false;
					return true;
				}
			}
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u == updateTick ? 1u : updateTick;
			player.fFallVelocityY = 0.f;
			player.iFallDeathTick = 0u;
			Update_MarioControlState(player);
		}
		return true;
	}
	// Authored jumps and entry/exit transfers own their airborne trajectory.
	if (player.TriggerMove.isActive ||
		(player.iVehicleId == ANCIENT_SEA_VEHICLE_ID && player.eAction == PLAYER_ACTION_STATE::VEHICLE_SKILL &&
		 player.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED)) return false;
	// The static island nav still contains the pre-match ring. During collapse it is
	// no longer physical support; use the same fall/revive path as Waterpang knockback.
	if (m_eWorldId == WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro && player.iCurrentHp &&
		player.fPositionY >= MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
		Is_MaharakaWaterpangMissingRing(static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick),
			player.fPositionX, player.fPositionZ))
	{
		Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		player.bWaterpangFall = true;
		return true;
	}
	if (gateFence || !m_ServerNavigation.Is_Loaded() ||
		0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::DEAD == player.eAction ||
		!m_ServerNavigation.Is_PointInVoidRegion(
			player.fPositionX, player.fPositionZ))
	{
		return false;
	}

	Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
	return true;
}

void LostArk::Server::CGameRoom::Update_Players(const float fixedDeltaSeconds)
{
	std::vector<SERVER_PLAYER*> colosseumPlayers;
	SERVER_COLOSSEUM_COMBAT_CONTEXT colosseumContext{ m_eWorldId, m_iColosseumMatchId,
		m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::ACTIVE, {} };
	if (m_iColosseumMatchId != 0u)
	{
		colosseumPlayers.reserve(m_Players.size());
		for (auto& [id, player] : m_Players) colosseumPlayers.push_back(&player);
		colosseumContext.Players = colosseumPlayers;
	}
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
        if (player.iColosseumMatchId && !player.bColosseumCombatActive &&
            !(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING && player.Is_Human())) continue;
		// Bern personal guides hold their exact pose while the owner is away or sailing.
		if (m_eWorldId == LostArk::Shared::WORLD_ID::BERN && player.Is_Guide() && !player.isCombatReady) continue;
		if (player.CardMaze.transferStartTick && player.iCurrentHp) continue;
		const auto ownsLivePatternOccurrence =
			[this](const LostArk::Shared::NET_ENTITY_ID ownerEntityId,
				const std::uint32_t patternSequence)
			{
				return std::any_of(
					m_WorldEntities.begin(), m_WorldEntities.end(),
					[ownerEntityId, patternSequence](
						const SERVER_WORLD_ENTITY& entity)
					{
						return entity.iNetEntityId == ownerEntityId &&
							entity.iPatternSequence == patternSequence &&
							0u != entity.iCurrentHp &&
							SERVER_ENTITY_ACTION::DEAD != entity.eAction;
					});
			};
		if (!player.iCurrentHp || !player.Has_TimeStop(updateTick)) player.iTimeStopEndTick = 0u;
		if (!player.iCurrentHp || !player.Has_HolyCharmProtection(updateTick)) player.iHolyCharmProtectionEndTick = 0u;
        (void)CKoukuSaydonLogicRuntime::Update_PlayerFear(player, updateTick);
		Update_MarioControlState(player);
		Update_MarioBombContacts(player, updateTick);
		Update_MarioBouncingBallContacts(player, updateTick);
		Update_MaharakaWaterpangHazards(player, updateTick);
		if (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA && player.iCurrentHp &&
			!player.iMarioStage && !player.TriggerMove.isActive &&
			player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FALLING)
		{
			const auto* center = Resolve_KoukuFallCenter(player);
			if (center)
			{
				SERVER_NAV_POINT ground{};
				if (m_ServerNavigation.Project_Point(center->fPositionX, center->fPositionZ, ground, center->fPositionY))
					player.KoukuFallRevivePosition = std::array<float, 3u>{ground.x, ground.y, ground.z};
			}
			else if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::NONE)
				player.KoukuFallRevivePosition.reset();
		}
		/* A song that ended any way but its own timeout (a hit, a bind, death) never
		lands the player later. */
		if (0u != player.iSquareHoleId &&
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG != player.eAction)
		{
			player.iSquareHoleId = 0u;
		}
		if (0u == player.iCurrentHp ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			/* A lethal hit does not strand the replicated body five metres above
			the arena. Restore the admitted pose first, while preserving DEAD and
			combat-disabled state, then release both occurrence owners. */
			if (player.bPatternBound)
				(void)Restore_PatternBoundPlayer(player);
			player.Clear_SilenceStatus();
		}
		else
		{
			if (player.bPatternBound &&
				(Has_ReachedServerTick(updateTick, player.iPatternBindEndTick) ||
				 !ownsLivePatternOccurrence(
					player.iPatternBindOwnerNetEntityId,
					player.iPatternBindSequence)))
			{
				(void)Restore_PatternBoundPlayer(player);
			}
			if (0u != player.iSilenceEndTick &&
				Has_ReachedServerTick(updateTick, player.iSilenceEndTick))
			{
				player.Clear_SilenceStatus();
			}
		}
		if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
				m_ValtanTimelineAudition.ePhase)
		{
			const bool driverMayPlay =
				player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId &&
				VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
					m_ValtanTimelineAudition.ePhase;
			if (!driverMayPlay)
			{
				m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
				Freeze_TimelineAuditionPlayer(player);
				continue;
			}
		}
		if (player.bPatternBound)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			player.PendingCommand.Clear();
			// Binding owns the input lock; it must not make a living captive immune
			// to the same authoritative hit tests used for other participants.
			player.isCombatReady = player.bPatternBindRestoreCombatReady;
			continue;
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::FEAR == player.eAction) continue;
		if (Update_PlayerAttachment(player, updateTick))
			continue;
		if (Update_PlayerFall(player, fixedDeltaSeconds, updateTick))
			continue;
		if (player.Has_TimeStop(updateTick))
		{
			CServerBuffRuntime::Expire(player.ActiveBuffs, updateTick);
			continue;
		}
		const std::string authoredMoveSource = player.TriggerMove.strSourcePlacementId;
		// Preserve the saved jump boxes. Only a live match with a missing destination
		// extends its airborne crossing inward, so returning players do not fall forever.
		if (m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro &&
			player.TriggerMove.isActive && LostArk::Shared::Is_MaharakaWaterpangJumpTrigger(authoredMoveSource) &&
			LostArk::Shared::Is_MaharakaWaterpangMissingRing(
				static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick),
				player.TriggerMove.fTargetX, player.TriggerMove.fTargetZ))
		{
			using namespace LostArk::Shared;
			const float dx = player.TriggerMove.fTargetX - MAHARAKA_WATERPANG_CANNON_X;
			const float dz = player.TriggerMove.fTargetZ - MAHARAKA_WATERPANG_CANNON_Z;
			const float scale = MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M / std::hypot(dx, dz);
			SERVER_NAV_POINT landing{};
			if (m_ServerNavigation.Sample_Position(MAHARAKA_WATERPANG_CANNON_X + dx * scale,
				MAHARAKA_WATERPANG_CANNON_Z + dz * scale, landing, player.TriggerMove.fTargetY) &&
				landing.y >= MAHARAKA_WATERPANG_DECK_MIN_Y_M)
			{
				player.TriggerMove.fTargetX = landing.x;
				player.TriggerMove.fTargetY = landing.y;
				player.TriggerMove.fTargetZ = landing.z;
			}
		}
		if (m_ServerTriggerSystem.Update_PlayerMotion(
			player, fixedDeltaSeconds))
		{
			if (authoredMoveSource.empty())
				Project_MarioRailPoint(player, player.fPositionX, player.fPositionZ);
			if (0u != player.iMarioStage && !player.TriggerMove.isActive && !authoredMoveSource.empty())
				(void)Configure_MarioRail(player, authoredMoveSource);
			Update_MarioControlState(player);
			if (!player.TriggerMove.isActive && !authoredMoveSource.empty())
				Complete_KoukuMarioReturn(player, authoredMoveSource, updateTick);
			/* Landing from a Waterpang crossing (jump1/jump2/jump3) protects the player for a
			   moment; the existing invulnerable tick already absorbs cannon, waterfall and
			   water gun hits, so none of them push or drop the player while it runs. */
			if (LostArk::Shared::WORLD_ID::MAHARAKA == m_eWorldId && !player.TriggerMove.isActive &&
				player.iCurrentHp &&
				LostArk::Shared::Is_MaharakaWaterpangJumpTrigger(authoredMoveSource))
			{
				const std::uint32_t protectUntil = Add_ServerTicksSkippingReservedZero(
					updateTick, LostArk::Shared::MAHARAKA_WATERPANG_JUMP_INVULNERABLE_TICKS);
				if (!player.iInvulnerableEndTick || Has_ReachedServerTick(protectUntil, player.iInvulnerableEndTick))
					player.iInvulnerableEndTick = protectUntil;
			}
            if (!player.TriggerMove.isActive && player.Is_Human()) Guide_AnchorArrived(player);
			continue;
		}
		const bool wasKnockbackActive =
			player.fKnockbackRemainingSeconds > 0.f;
		Advance_PlayerKnockback(player, fixedDeltaSeconds);
		if (wasKnockbackActive)
			continue;
		if (LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN == player.eAction &&
			static_cast<std::int32_t>(
				updateTick - player.iKnockdownEndTick) >= 0)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.iKnockdownEndTick = 0u;
			player.iHitReactionGraceEndTick = player.bPushOnlyHitReaction ? 0u :
				updateTick + PLAYER_HIT_REACTION_GRACE_TICKS;
			player.bPushOnlyHitReaction = false;
			player.PendingCommand.Clear();
		}
		/* The Esther call is a fixed-length lock, not a balance skill: the
		roster owns the summon, this block only releases the caster once the
		call clip has run out. Signed difference keeps ordering across a
		wrapped tick counter. */
		const bool estherCastElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::ESTHER_CAST == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + ESTHER_CAST_TICKS)) >= 0;
		/* The square-hole lock is the song plus a black hold: the Client screen is fully
		black when the song ticks run out, and the player lands inside the hold. */
		const bool squareHoleSongElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::SQUAREHOLE_SONG == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + SQUAREHOLE_LOCK_TICKS)) >= 0;
		if (squareHoleSongElapsed)
			Finish_SquareHoleSong(player);
		/* An escape teleport borrows the same INTERACTION lock and start tick,
		so a swing is judged only for a real hammer press. */
		const bool mazeHammerPress =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MAZE == player.eKoukuHudMode &&
			0u == player.CardMaze.transferStartTick;
		/* The maze hammer lands part-way through its press: judge the swing
		once, on that tick, against the run's targets in front of the player. */
		if (mazeHammerPress &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::Hammer_HitTickOffset(player.iCurrentSkillId))
		{
			Resolve_CardMazeHammerHit(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			LostArk::Shared::KOUKU_HUD_MODE::MARIO == player.eKoukuHudMode &&
			0u == player.iCurrentSkillId &&
			updateTick == player.iActionStartTick + CKoukuCardMazeRuntime::HAMMER_HIT_TICK_OFFSET)
		{
			Resolve_MarioHammerHit(player, updateTick);
		}
		/* A KoukuSaydon interaction press is the same kind of lock, but it ends
		with the clip the pressed slot was authored on so the Client is never
		left holding a frozen last frame. An escape teleport borrows this action
		with no slot of its own, and INVALID_SKILL_ID is 0 -- the same value as
		slot 0 -- so it is separated by its transfer tick, not by the skill id. */
		const std::uint32_t interactionTicks =
			0u == player.CardMaze.transferStartTick ?
			CKoukuSaydonLogicRuntime::Ticks_FromMs(
				LostArk::Shared::Kouku_InteractionActionMs(
					player.eKoukuHudMode, player.iCurrentSkillId)) :
			KOUKU_INTERACTION_TICKS;
		const bool interactionElapsed =
			LostArk::Shared::PLAYER_ACTION_STATE::INTERACTION == player.eAction &&
			static_cast<std::int32_t>(updateTick -
				(player.iActionStartTick + interactionTicks)) >= 0;
		if (estherCastElapsed || squareHoleSongElapsed || interactionElapsed)
		{
			player.eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
			player.PendingCommand.Clear();
		}
		CServerBuffRuntime::Expire(player.ActiveBuffs, updateTick);
		CServerBuffRuntime::Settle_Shield(m_GameplayCatalog.Active(), player);
		Update_VehicleSkill(player, fixedDeltaSeconds);
		m_PlayerSkillSystem.Update(
			player,
			m_WorldEntities,
			m_GameplayCatalog,
			m_ServerNavigation.Is_Loaded() ? &m_ServerNavigation : nullptr,
			&m_ServerCollisionSystem,
			fixedDeltaSeconds,
			updateTick,
			m_TickDamageEvents,
			m_iColosseumMatchId != 0u ? &colosseumContext : nullptr);
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction &&
			PLAYER_PENDING_COMMAND_KIND::NONE != player.PendingCommand.eKind)
		{
			Commit_PendingPlayerCommand(player, updateTick);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction)
			continue;
		Update_MarioMoveGoal(player, updateTick);
		if (!player.hasMoveGoal)
			continue;
		/* The route was pulled taut from where the player stood when it was
		built, and the player has moved since. Pull it again from here and aim
		at the farthest waypoint still in sight. Without this the player keeps
		facing the first waypoint - a neighbouring cell centre whose bearing has
		little to do with the goal's - and every rebuild puts that cell on the
		other side, which is what made a held right mouse shake. */
		if (!player.MovePath.empty() && m_ServerNavigation.Is_Loaded())
		{
			for (std::size_t candidate = player.MovePath.size();
				candidate > player.iMovePathIndex + 1u; --candidate)
			{
				const SERVER_NAV_POINT& ahead = player.MovePath[candidate - 1u];
				if (m_ServerNavigation.Has_LineOfSight(
					player.fPositionX, player.fPositionZ, ahead.x, ahead.z,
					player.fPositionY))
				{
					player.iMovePathIndex = candidate - 1u;
					break;
				}
			}
		}
		float targetX = player.fMoveGoalX;
		float targetY = player.fPositionY;
		float targetZ = player.fMoveGoalZ;
		if (player.iMovePathIndex < player.MovePath.size())
		{
			const SERVER_NAV_POINT& pathPoint =
				player.MovePath[player.iMovePathIndex];
			targetX = pathPoint.x;
			targetY = pathPoint.y;
			targetZ = pathPoint.z;
		}
		/* A smoothed path's intermediate points are corners, not arrivals. Only
		the last one is where the player stops and has to end up facing. */
		const bool targetIsDestination =
			player.iMovePathIndex + 1u >= player.MovePath.size();
		const float deltaX = targetX - player.fPositionX;
		const float deltaZ = targetZ - player.fPositionZ;
		const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
		const bool reachedPathPoint = distance <= MOVE_STOP_DISTANCE;
		float proposedX = targetX;
		float proposedY = targetY;
		float proposedZ = targetZ;
		if (!reachedPathPoint)
		{
			const float desiredYaw =
				std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
			const float yawDifference =
				Wrap_Degrees(desiredYaw - player.fYawDegrees);
			const float maxYawStep =
				(LostArk::Shared::INVALID_VEHICLE_ID != player.iVehicleId ?
					VEHICLE_TURN_DEGREES_PER_SECOND :
					PLAYER_TURN_DEGREES_PER_SECOND) * fixedDeltaSeconds;
			/* Close to the destination the turn radius no longer fits, so facing
			snaps rather than orbiting the point. A corner is not a destination:
			snapping there made every path bend read as an instant pivot. */
			if (0u != player.iMarioStage ||
				(targetIsDestination && distance <= DIRECT_BEARING_DISTANCE) ||
				std::abs(yawDifference) <= maxYawStep)
			{
				player.fYawDegrees = desiredYaw;
			}
			else
			{
				player.fYawDegrees = Wrap_Degrees(player.fYawDegrees +
					(yawDifference > 0.f ? maxYawStep : -maxYawStep));
			}
			const float moveDistance = (std::min)(
				Resolve_PlayerMoveSpeed(player) *
					fixedDeltaSeconds,
				distance);
			const float moveRatio = moveDistance / distance;
			// Movement follows the requested path immediately; facing catches up
			// independently so an opposite click does not first walk sideways.
			const float stepX = deltaX * moveRatio;
			const float stepZ = deltaZ * moveRatio;
			proposedX = player.fPositionX + stepX;
			proposedY = player.fPositionY +
				(targetY - player.fPositionY) * moveRatio;
			proposedZ = player.fPositionZ + stepZ;
		}
		Project_MarioRailPoint(player, proposedX, proposedZ);
		if (Try_KoukuWalkOffFloor(player, proposedX, proposedZ, fixedDeltaSeconds, updateTick))
			continue;
		/* A smoothed path can skip many authored cells. Never interpolate Y toward
		the distant waypoint: doing so raises the player while XZ is still on the
		lower deck and lets a later height check see an already-raised player.
		Resolve both XZ positions against navigation and take only its ground Y. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT proposedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				proposedX,
				proposedZ,
				proposedGround,
				player.fPositionY))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			proposedY = proposedGround.y;
		}

		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_PlayerMove(
			player,
			proposedX,
			proposedY,
			proposedZ,
			resolvedX,
			resolvedY,
			resolvedZ,
			wasBlocked))
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		Project_MarioRailPoint(player, resolvedX, resolvedZ);
		/* Body collision may slide XZ away from the point checked above. Validate
		the final slide destination too and ground it before committing any
		authoritative coordinate. */
		if (m_ServerNavigation.Is_Loaded())
		{
			SERVER_NAV_POINT resolvedGround{};
			if (!m_ServerNavigation.Resolve_TraversalStep(
				player.fPositionX,
				player.fPositionZ,
				resolvedX,
				resolvedZ,
				resolvedGround,
				player.fPositionY))
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0u;
				continue;
			}
			resolvedY = resolvedGround.y;
		}
		// Successful tangent slides clear wasBlocked; a deflected step still
		// reached the body and must finish a goal inside that same body.
		const bool reachedGoalBody =
			(wasBlocked || resolvedX != proposedX || resolvedZ != proposedZ) &&
			m_ServerCollisionSystem.Is_PlayerMoveBlockedAtGoalBody(player,
				proposedX, proposedY, proposedZ,
				player.MovePath.empty() ? player.fPositionY : player.MovePath.back().y);
		player.fPositionX = resolvedX;
		player.fPositionY = resolvedY;
		player.fPositionZ = resolvedZ;
		if (reachedGoalBody)
		{
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0u;
			continue;
		}
		if (wasBlocked)
		{
			/* The body sweep met something the navigation grid does not carry:
			a collisionBox, or another body. Route around it and keep the goal.
			Dropping the goal here is what made a held right mouse shake next
			to an obstacle: every brush cancelled the move, and the re-send
			50 ms later started a fresh search from a start cell that had
			moved, so the first waypoint jumped from side to side.

			The rebuild is rate limited because brushing reports blocked on
			many ticks in a row. Between rebuilds the move is left alone and
			the existing slide carries it along the obstacle. */
			const bool mayReroute = m_ServerNavigation.Is_Loaded() &&
				updateTick - player.iMoveRerouteTick >= MOVE_REROUTE_MIN_TICKS;
			if (!mayReroute)
				continue;
			player.iMoveRerouteTick = updateTick;
			std::vector<SERVER_NAV_POINT> reroute;
			if (m_ServerNavigation.Find_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY))
			{
				m_ServerNavigation.Smooth_Path(
					player.fPositionX,
					player.fPositionZ,
					player.fMoveGoalX,
					player.fMoveGoalZ,
					reroute,
					player.fPositionY);
				player.MovePath = std::move(reroute);
				player.iMovePathIndex = 0;
				continue;
			}
			player.hasMoveGoal = false;
			player.MovePath.clear();
			player.iMovePathIndex = 0;
			continue;
		}
		if (reachedPathPoint)
		{
			if (player.iMovePathIndex < player.MovePath.size())
				++player.iMovePathIndex;
			if (player.iMovePathIndex >= player.MovePath.size())
			{
				player.hasMoveGoal = false;
				player.MovePath.clear();
				player.iMovePathIndex = 0;
			}
			continue;
		}
	}
}

bool LostArk::Server::CGameRoom::Resolve_ArenaCenter(
	const SERVER_WORLD_ENTITY& boss, SERVER_NAV_POINT& point)
{
	const bool exact = m_ServerNavigation.Is_PointWalkableExact(
		boss.fSpawnPositionX, boss.fSpawnPositionZ) &&
		m_ServerNavigation.Sample_Position(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point);
	if (!m_ServerNavigation.Is_Loaded() ||
		(!exact && !m_ServerNavigation.Project_PointOnSameLevel(
			boss.fSpawnPositionX, boss.fSpawnPositionZ, point)) ||
		!m_ServerNavigation.Is_PointWalkableExact(point.x, point.z) ||
		std::fabs(point.y - boss.fSpawnPositionY) > 1.5f ||
		std::hypot(point.x - boss.fSpawnPositionX,
			point.z - boss.fSpawnPositionZ) > 8.f)
	{
		m_strStatus = "Arena center has no nearby walkable same-level recovery point";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Prepare_ArenaEjection(
	SERVER_PLAYER& staged,
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick)
{
	if (BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION != action.eReleaseMode ||
		!m_ServerNavigation.Is_Loaded() ||
		!std::isfinite(action.fReleaseSpeedMps) || action.fReleaseSpeedMps <= 0.f ||
		action.fReleaseSpeedMps > 50.f || 0u == action.iDurationMs ||
		action.iDurationMs > 5000u ||
		!std::isfinite(action.fReleaseYawOffsetDegrees) ||
		std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
		!std::isfinite(boss.fYawDegrees))
	{
		m_strStatus = "Arena ejection policy or navigation is invalid";
		return false;
	}
	const float yaw = (boss.fYawDegrees + action.fReleaseYawOffsetDegrees) *
		DEGREES_TO_RADIANS;
	const float directionX = -std::sin(yaw);
	const float directionZ = -std::cos(yaw);
	constexpr float maximumDistance = 128.f;
	constexpr float outsideMargin = 2.f;
	const float sampleStep = std::clamp(m_ServerNavigation.Get_CellSize(), 0.1f, 0.5f);
	float lastArenaGround = 0.f;
	/* Scan past small holes and seams. An interior missing cell is not the arena
	   exterior; the endpoint lies beyond the last same-deck ground on this ray. */
	for (float distance = 0.f; distance <= maximumDistance; distance += sampleStep)
	{
		const float x = staged.fPositionX + directionX * distance;
		const float z = staged.fPositionZ + directionZ * distance;
		SERVER_NAV_POINT ground{};
		if (m_ServerNavigation.Is_PointWalkableExact(x, z) &&
			m_ServerNavigation.Sample_Position(x, z, ground) &&
			std::fabs(ground.y - boss.fSpawnPositionY) <= 1.5f)
			lastArenaGround = distance;
	}
	const float minimumDistance = action.fReleaseSpeedMps *
		(static_cast<float>(action.iDurationMs) / 1000.f);
	const float distance = (std::max)(minimumDistance, lastArenaGround + outsideMargin);
	if (!std::isfinite(distance) || distance > maximumDistance ||
		!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
	{
		m_strStatus = "Arena ejection has no bounded exterior destination";
		return false;
	}
	if (0u == staged.iCurrentHp)
		return true;
	staged.fKnockbackDirectionX = directionX;
	staged.fKnockbackDirectionZ = directionZ;
	staged.fKnockbackSpeed = action.fReleaseSpeedMps;
	staged.fKnockbackRemainingSeconds = distance / action.fReleaseSpeedMps;
	staged.bArenaEjectionActive = true;
	staged.iEjectionOwnerNetEntityId = boss.iNetEntityId;
	staged.isCombatReady = false;
	return true;
}

void LostArk::Server::CGameRoom::Advance_PlayerKnockback(
	SERVER_PLAYER& player, const float fixedDeltaSeconds)
{
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f ||
		player.fKnockbackRemainingSeconds <= 0.f)
	{
		return;
	}
	if (0u == player.iCurrentHp ||
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction)
	{
		player.Clear_Attachment();
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.bWaterpangLaunch = false;
		return;
	}
	const float step = (std::min)(
		fixedDeltaSeconds, player.fKnockbackRemainingSeconds);
	float desiredX = player.fPositionX +
		player.fKnockbackDirectionX * player.fKnockbackSpeed * step;
	float desiredZ = player.fPositionZ +
		player.fKnockbackDirectionZ * player.fKnockbackSpeed * step;
	const auto gate = Resolve_CurrentKoukuGate();
	const bool gateFence = !player.iMarioStage && (gate == 1u || gate == 3u);
	if (gateFence) player.bKnockbackCanLeaveArena = false;
	// Once the match has started, a push may carry a player off the Waterpang arena.
	const std::uint32_t knockbackTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
	const bool waterpangMatch = m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA &&
		((m_MaharakaWaterpangIntro && Has_ReachedServerTick(knockbackTick, m_MaharakaWaterpangIntro->iStartTick)) ||
		 Is_MaharakaWaterpangDebugEventLive(knockbackTick));
	const bool waterpangPush = waterpangMatch &&
		LostArk::Shared::Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) &&
		m_ServerNavigation.Is_PointWalkableInRegion(LostArk::Shared::MAHARAKA_WATERPANG_REGION_ID,
			player.fPositionX, player.fPositionZ, player.fPositionY);
	if (waterpangPush) player.bKnockbackCanLeaveArena = true;
	if (player.bKnockbackBallistic)
	{
		// A bounded launch keeps the authored Y arc while its ground footprint
		// obeys the same navigation and fence collision as ordinary movement.
		const bool bounded = !player.bKnockbackCanLeaveArena && !player.bWaterpangLaunch;
		if (bounded)
		{
			SERVER_NAV_POINT reachable{desiredX, player.fKnockbackSupportY, desiredZ};
			bool clamped = false;
			if (m_ServerNavigation.Is_Loaded())
				CPlayerSkillSystem::Clamp_StepToWalkable(m_ServerNavigation, player.fPositionX, player.fPositionZ,
					desiredX, desiredZ, reachable, clamped, player.fKnockbackSupportY);
			SERVER_PLAYER groundBody = player;
			groundBody.fPositionY = player.fKnockbackSupportY;
			float x = player.fPositionX, y = player.fKnockbackSupportY, z = player.fPositionZ;
			bool blocked = false;
			if (!m_ServerCollisionSystem.Resolve_PlayerMove(groundBody, reachable.x, reachable.y, reachable.z, x, y, z, blocked))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT valid{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Has_LineOfSight(player.fPositionX, player.fPositionZ, x, z) ||
				 !m_ServerNavigation.Resolve_TraversalStep(player.fPositionX, player.fPositionZ, x, z, valid)))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			SERVER_NAV_POINT support{};
			if (m_ServerNavigation.Is_Loaded() &&
				(!m_ServerNavigation.Sample_SurfacePosition(x, z, support) ||
				 std::abs(support.y - player.fKnockbackSupportY) > 1.f))
			{ x = player.fPositionX; z = player.fPositionZ; blocked = true; }
			desiredX = x; desiredZ = z;
			if (clamped || blocked) player.fKnockbackSpeed = 0.f;
		}
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fPositionY += player.fKnockbackVelocityY * step -
			0.5f * player.fKnockbackGravityMps2 * step * step;
		player.fKnockbackVelocityY -= player.fKnockbackGravityMps2 * step;
		player.fKnockbackRemainingSeconds = (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		/* A Waterpang waterfall launch never lands on the deck, a pier or the pool
		   floor it crosses: its flight always ends in the Waterpang fall. */
		if (player.bWaterpangLaunch)
		{
			if (player.fKnockbackRemainingSeconds > 0.00001f)
				return;
			const float velocityY = player.fKnockbackVelocityY;
			const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, 0.f, tick);
			player.fFallVelocityY = velocityY;
			player.bWaterpangFall = true;
			return;
		}
		const float fallDepth = Resolve_KoukuFallCenter(player) ? KOUKU_CASINO_FALL_DEPTH_M : KOUKU_FALL_DEPTH_M;
		if (!bounded && m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
			player.fPositionY <= player.fKnockbackSupportY - fallDepth)
		{
			const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, 0.f, tick);
			(void)Update_PlayerFall(player, 0.f, tick);
			return;
		}
		SERVER_NAV_POINT floor{ desiredX, player.fKnockbackLaunchY, desiredZ };
		bool hasFloor = !m_ServerNavigation.Is_Loaded() ||
			m_ServerNavigation.Sample_SurfacePosition(desiredX, desiredZ, floor);
		/* A ballistic player may cross an overlapping upper deck while leaving an
		   arena.  That deck is not a landing surface for a flight launched from
		   below it: accepting it would snap Y upward and turn the dead-zone fall
		   into a nav teleport.  Lower floors remain valid and are handled by the
		   normal gravity/dead-zone path. */
		constexpr float maximumLandingRiseM = 0.01f;
		if (hasFloor && std::isfinite(floor.y) &&
			floor.y > player.fKnockbackLaunchY + maximumLandingRiseM)
			hasFloor = false;
		if (bounded && (!hasFloor || std::abs(floor.y - player.fKnockbackSupportY) > 1.f))
		{
			floor = {desiredX, player.fKnockbackSupportY, desiredZ};
			hasFloor = true;
		}
		if (hasFloor && m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && m_MaharakaWaterpangIntro &&
			floor.y >= LostArk::Shared::MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
			LostArk::Shared::Is_MaharakaWaterpangMissingRing(
				static_cast<std::int32_t>(knockbackTick - m_MaharakaWaterpangIntro->iStartTick), desiredX, desiredZ))
			hasFloor = false;
		if (player.fKnockbackVelocityY <= 0.f && hasFloor && player.fPositionY <= floor.y + 0.0001f)
		{
			player.fPositionY = floor.y;
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = player.fKnockbackVelocityY = 0.f;
			player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = false;
			const auto landingTick = Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u);
			const auto recoveryEnd = Add_ServerTicksSkippingReservedZero(landingTick,
				CKoukuSaydonLogicRuntime::Ticks_FromMs(PLAYER_HIT_LANDING_RECOVERY_MS));
			if (player.eAction == LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN &&
				static_cast<std::int32_t>(recoveryEnd - player.iKnockdownEndTick) > 0)
				player.iKnockdownEndTick = recoveryEnd;
		}
		else if (player.fKnockbackRemainingSeconds <= 0.00001f)
		{
			if (hasFloor)
			{
				// A lower deck takes longer than the nominal same-height flight.
				// Keep descending at the endpoint without snapping down to that deck.
				player.fKnockbackSpeed = 0.f;
				player.fKnockbackRemainingSeconds = fixedDeltaSeconds;
			}
			else
			{
				const float velocityY = player.fKnockbackVelocityY;
				// A launch leaves the deck before it ends, so the match alone decides.
				const bool waterpangFall = waterpangMatch && player.bKnockbackCanLeaveArena;
				const std::uint32_t tick = (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
				Begin_PlayerFall(player, 0.f, tick);
				player.fFallVelocityY = velocityY;
				player.bWaterpangFall = waterpangFall;
			}
		}
		return;
	}
	Project_MarioRailPoint(player, desiredX, desiredZ);
	// Ordinary/arena pushes must reach their existing bounded or swept-surface
	// mover. Only an authored Mario exit uses the rail's walking-floor check.
	if (player.iMarioStage && player.bKnockbackCanLeaveArena &&
		Try_KoukuWalkOffFloor(player, desiredX, desiredZ, fixedDeltaSeconds,
			Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u))) return;
	if (player.bArenaEjectionActive)
	{
		player.fPositionX = desiredX;
		player.fPositionZ = desiredZ;
		player.fKnockbackRemainingSeconds =
			(std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&player](const SERVER_WORLD_ENTITY& boss)
			{ return boss.iNetEntityId == player.iEjectionOwnerNetEntityId; });
		if (player.fKnockbackRemainingSeconds <= 0.00001f ||
			owner == m_WorldEntities.end() || 0u == owner->iCurrentHp ||
			SERVER_ENTITY_ACTION::DEAD == owner->eAction)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
				1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
		}
		return;
	}
	if (m_ServerNavigation.Is_Loaded() &&
		(m_eWorldId == LostArk::Shared::WORLD_ID::VALTAN_ARENA || waterpangPush ||
		 (m_eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
		  player.bKnockbackCanLeaveArena && !player.iMarioStage)))
	{
		/* Valtan hits and authored Kouku arena-exit hits cross walking boundaries. Keep actual walls
		   and bodies authoritative, but do not route or clamp the displacement
		   to walkable cells. A push is a straight sweep, not walking avoidance
		   around another body; this also gives support one exact segment. */
		using namespace LostArk::Shared::WorldCollision;
		float resolvedX = player.fPositionX;
		float resolvedY = player.fPositionY;
		float resolvedZ = player.fPositionZ;
		bool wasBlocked = false;
		if (!m_ServerCollisionSystem.Resolve_CircleMove(
			player.fPositionX, player.fPositionY, player.fPositionZ,
			desiredX, player.fPositionY, desiredZ,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y, PLAYER_CENTER_OFFSET_Y,
			resolvedX, resolvedY, resolvedZ, wasBlocked,
			player.iNetEntityId, false))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		SERVER_NAV_POINT supported{};
		const FORCED_SURFACE_RESULT support = Trace_ForcedSurface(
			m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ},
			resolvedX, resolvedZ, supported);
		player.fPositionX = supported.x;
		player.fPositionY = supported.y;
		player.fPositionZ = supported.z;
		if (FORCED_SURFACE_RESULT::FALL == support)
		{
			const std::uint32_t updateTick =
				(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
			Begin_PlayerFall(player, fixedDeltaSeconds, updateTick);
			player.bWaterpangFall = waterpangPush;
			return;
		}
		player.fKnockbackRemainingSeconds = wasBlocked || FORCED_SURFACE_RESULT::BLOCKED == support ?
			0.f : (std::max)(0.f, player.fKnockbackRemainingSeconds - step);
		if (player.fKnockbackRemainingSeconds <= 0.f)
		{
			player.fKnockbackSpeed = 0.f;
			player.bKnockbackCanLeaveArena = false;
		}
		return;
	}
	SERVER_NAV_POINT reachable{ desiredX, player.fPositionY, desiredZ };
	bool wasClamped = false;
	if (m_ServerNavigation.Is_Loaded())
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			m_ServerNavigation,
			player.fPositionX,
			player.fPositionZ,
			desiredX,
			desiredZ,
			reachable,
			wasClamped,
			player.fPositionY);
	}
	Project_MarioRailPoint(player, reachable.x, reachable.z);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!player.bMarioRailReady || !m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, reachable.x, reachable.z, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		reachable.y = railGround.y;
	}
	float resolvedX = player.fPositionX;
	float resolvedY = player.fPositionY;
	float resolvedZ = player.fPositionZ;
	bool wasBlocked = false;
	if (!m_ServerCollisionSystem.Resolve_PlayerMove(
		player,
		reachable.x,
		reachable.y,
		reachable.z,
		resolvedX,
		resolvedY,
		resolvedZ,
		wasBlocked))
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		return;
	}
	Project_MarioRailPoint(player, resolvedX, resolvedZ);
	if (0u != player.iMarioStage)
	{
		SERVER_NAV_POINT railGround{};
		if (!m_ServerNavigation.Resolve_TraversalStep(
			player.fPositionX, player.fPositionZ, resolvedX, resolvedZ, railGround))
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = railGround.y;
	}
	if (gateFence && m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT supported{};
		const auto support = Trace_ForcedSurface(m_ServerNavigation,
			{player.fPositionX, player.fPositionY, player.fPositionZ}, resolvedX, resolvedZ, supported);
		if (support != FORCED_SURFACE_RESULT::SUPPORTED)
		{
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			return;
		}
		resolvedY = supported.y;
	}
	player.fPositionX = resolvedX;
	player.fPositionY = resolvedY;
	player.fPositionZ = resolvedZ;
	player.fKnockbackRemainingSeconds = (wasClamped || wasBlocked) ?
		0.f : player.fKnockbackRemainingSeconds - step;
	if (player.fKnockbackRemainingSeconds <= 0.f)
	{
		player.fKnockbackRemainingSeconds = 0.f;
		player.fKnockbackSpeed = 0.f;
		player.bKnockbackCanLeaveArena = false;
	}
}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/GameRoom_MaharakaAI.cpp

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketWriter.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>

using namespace LostArk::Server;
using namespace LostArk::Shared;
namespace
{
    constexpr float PI = 3.14159265359f;
    bool reached(std::uint32_t now, std::uint32_t end) { return !end || static_cast<std::int32_t>(now - end) >= 0; }
    std::uint32_t nextTick(std::uint32_t now, std::uint32_t delta) { const auto value = now + delta; return value ? value : 1u; }
    float roll(std::uint32_t seed) { seed ^= seed >> 16; seed *= 0x7feb352du; seed ^= seed >> 15; seed *= 0x846ca68bu; seed ^= seed >> 16; return (seed & 0xffffu) / 65535.f; }
    std::filesystem::path tuningPath()
    {
        wchar_t executable[32768]{};
        const auto count = GetModuleFileNameW(nullptr, executable, 32768);
        for (auto base : { std::filesystem::current_path(), count && count < 32768 ? std::filesystem::path(executable).parent_path() : std::filesystem::path{} })
            for (unsigned depth = 0; !base.empty() && depth < 8; ++depth, base = base.parent_path())
                if (std::filesystem::is_regular_file(base / L"AGENTS.md") && std::filesystem::is_directory(base / L"Data"))
                    return base / L"Data/AI/MaharakaWaterpangAI.json";
        return {};
    }
    std::string readFile(const std::filesystem::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) throw std::runtime_error("AI source JSON is unavailable");
        stream.seekg(0, std::ios::end);
        if (stream.tellg() > 16384) throw std::runtime_error("AI source JSON exceeds 16 KiB");
        stream.seekg(0); return { std::istreambuf_iterator<char>(stream), {} };
    }
    MAHARAKA_AI_TUNING parseTuning(const std::string& text)
    {
        // This contract is a flat JSON object of ASCII schema keys and finite numbers.
        // Match the whole input; duplicate/unknown keys and trailing data are rejected.
        const std::regex row(R"json(\s*"([A-Za-z][A-Za-z0-9]*)"\s*:\s*("[A-Za-z0-9._-]+"|-?(?:0|[1-9][0-9]*)(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)\s*)json");
        std::map<std::string, std::string> values;
        std::size_t at = text.find_first_not_of(" \r\n\t");
        if (at == std::string::npos || text[at++] != '{') throw std::runtime_error("AI JSON object expected");
        for (;;)
        {
            std::smatch match; const auto remainder = text.substr(at);
            if (!std::regex_search(remainder, match, row, std::regex_constants::match_continuous) ||
                !values.emplace(match[1].str(), match[2].str()).second) throw std::runtime_error("Invalid or duplicate AI JSON field");
            at += match.length();
            if (at == text.size()) throw std::runtime_error("Unclosed AI JSON object");
            const char separator = text[at++];
            if (separator == '}') break;
            if (separator != ',') throw std::runtime_error("Invalid AI JSON separator");
        }
        if (text.find_first_not_of(" \r\n\t", at) != std::string::npos || values.size() != 12u ||
            values.at("schema") != "\"lostark.maharaka-waterpang-ai\"" || values.at("formatVersion") != "1")
            throw std::runtime_error("Unsupported AI JSON schema");
        auto number = [&](const char* key) { std::size_t used = 0; const auto& s = values.at(key); const double v = std::stod(s, &used); if (used != s.size() || !std::isfinite(v)) throw std::runtime_error("Invalid AI number"); return v; };
        auto integer = [&](const char* key) { const double v = number(key); if (v < 0 || v > UINT32_MAX || v != std::floor(v)) throw std::runtime_error("Invalid AI integer"); return static_cast<std::uint32_t>(v); };
        MAHARAKA_AI_TUNING t;
        t.iRevision = integer("revision"); t.iBotCount = integer("botCount"); t.iDecisionTicks = integer("decisionTicks");
        t.iMoveRetargetTicks = integer("moveRetargetTicks"); t.iSkillIntervalTicks = integer("skillIntervalTicks");
        t.fTargetRangeM = static_cast<float>(number("targetRangeM")); t.fMoveProbability = static_cast<float>(number("moveProbability"));
        t.fAggression = static_cast<float>(number("aggression")); t.fKnockbackRangeM = static_cast<float>(number("knockbackRangeM"));
        t.iKnockbackMs = integer("knockbackMs");
        if (!Is_Valid_MaharakaAITuning(t)) throw std::runtime_error("AI tuning value outside bounds");
        return t;
    }
    std::string serializeTuning(const MAHARAKA_AI_TUNING& t)
    {
        std::ostringstream s; s << std::setprecision(9) << "{\n  \"schema\": \"lostark.maharaka-waterpang-ai\",\n  \"formatVersion\": 1,\n  \"revision\": " << t.iRevision
            << ",\n  \"botCount\": " << t.iBotCount << ",\n  \"decisionTicks\": " << t.iDecisionTicks << ",\n  \"moveRetargetTicks\": " << t.iMoveRetargetTicks
            << ",\n  \"skillIntervalTicks\": " << t.iSkillIntervalTicks << ",\n  \"targetRangeM\": " << t.fTargetRangeM
            << ",\n  \"moveProbability\": " << t.fMoveProbability << ",\n  \"aggression\": " << t.fAggression
            << ",\n  \"knockbackRangeM\": " << t.fKnockbackRangeM << ",\n  \"knockbackMs\": " << t.iKnockbackMs << "\n}\n";
        return s.str();
    }
    void loadTuning(bool& loaded, MAHARAKA_AI_TUNING& tuning, std::string& original)
    {
        if (loaded) return;
        loaded = true;
        try { auto bytes = readFile(tuningPath()); const auto candidate = parseTuning(bytes); tuning = candidate; original = std::move(bytes); }
        catch (const std::exception& e) { std::cout << "[WaterpangAI] keeping defaults: " << e.what() << '\n'; }
    }
}

void CGameRoom::Handle_MaharakaAITuning(SESSION_ID sessionId, const C2S_MAHARAKA_AI_TUNING& request)
{
    const auto session = Find_Session(sessionId);
    if (!session || !m_PlayerIdBySessionId.contains(sessionId)) return;
    loadTuning(m_bMaharakaAITuningLoaded, m_MaharakaAITuning, m_strMaharakaAISourceBytes);
    S2C_MAHARAKA_AI_TUNING result; result.iRequestSequence = request.iRequestSequence;
    result.strStatus = "Current Server AI tuning";
    if (m_eWorldId != WORLD_ID::MAHARAKA) { result.eResult = MAHARAKA_AI_RESULT::WRONG_WORLD; result.strStatus = "Enter Maharaka first"; }
    else if (request.eOperation != MAHARAKA_AI_OPERATION::GET)
    {
        auto staged = request.Tuning;
        if (request.iExpectedRevision != m_MaharakaAITuning.iRevision) { result.eResult = MAHARAKA_AI_RESULT::REVISION_CONFLICT; result.strStatus = "Revision changed; Get current values before applying"; }
        else if (!Is_Valid_MaharakaAITuning(staged) || m_MaharakaAITuning.iRevision == UINT32_MAX) { result.eResult = MAHARAKA_AI_RESULT::INVALID_VALUE; result.strStatus = "AI tuning is outside supported bounds"; }
        else
        {
            staged.iRevision = m_MaharakaAITuning.iRevision + 1u;
            try
            {
                if (request.eOperation == MAHARAKA_AI_OPERATION::SAVE)
                {
                    const auto path = tuningPath();
                    if (path.empty() || m_strMaharakaAISourceBytes.empty() || readFile(path) != m_strMaharakaAISourceBytes)
                        throw std::runtime_error("Source JSON changed or is unavailable; current Server values preserved");
                    const auto payload = serializeTuning(staged);
                    if (!Is_Valid_MaharakaAITuning(parseTuning(payload))) throw std::runtime_error("AI save validation failed");
                    auto temporary = path; temporary += L".tmp." + std::to_wstring(GetCurrentProcessId());
                    { std::ofstream output(temporary, std::ios::binary | std::ios::trunc); output.write(payload.data(), payload.size()); output.flush(); if (!output) throw std::runtime_error("AI temporary write failed"); }
                    if (readFile(path) != m_strMaharakaAISourceBytes) throw std::runtime_error("Concurrent AI source save; current values preserved");
                    auto backup = path; backup += L".backup";
                    if (!ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), REPLACEFILE_WRITE_THROUGH, nullptr, nullptr)) throw std::runtime_error("Atomic AI source replacement failed");
                    m_strMaharakaAISourceBytes = payload;
                }
                m_MaharakaAITuning = staged;
                for (auto& [id, state] : m_MaharakaWaterpangAI) { (void)id; state.iNextThinkTick = state.iNextMoveTick = state.iNextShotTick = 0; }
                result.strStatus = request.eOperation == MAHARAKA_AI_OPERATION::SAVE ? "Saved source and applied on Server" : "Applied on Server; source file unchanged";
            }
            catch (const std::exception& e) { result.eResult = MAHARAKA_AI_RESULT::SAVE_FAILED; result.strStatus = e.what(); }
        }
    }
    result.Tuning = m_MaharakaAITuning;
    CPacketWriter writer;
    if (!Write_Message(writer, result) || !session->Send_Frame(PACKET_TYPE::S2C_MAHARAKA_AI_TUNING, writer.Get_Buffer())) session->Request_Close();
}

bool CGameRoom::Spawn_MaharakaWaterpangAI()
{
    const auto firstSlot = static_cast<std::uint32_t>(m_MaharakaWaterpangAI.size());
    const auto count = m_MaharakaAITuning.iBotCount - firstSlot;
    if (m_Players.size() + count > MAX_WORLD_SNAPSHOT_PLAYERS || !m_ServerNavigation.Is_Loaded()) return false;
    std::vector<SERVER_PLAYER> staged;
    const bool collapsed = m_MaharakaWaterpangIntro && Has_MaharakaWaterpangCollapsedFloor(
        static_cast<std::int32_t>(m_iServerTick - m_MaharakaWaterpangIntro->iStartTick));
    for (std::uint32_t slot = firstSlot; slot < m_MaharakaAITuning.iBotCount; ++slot)
    {
        SERVER_PLAYER ai; ai.eControlKind = PLAYER_CONTROL_KIND::WATERPANG_AI;
        ai.iPlayerId = m_iNextWaterpangAIPlayerId + slot - firstSlot; ai.iNetEntityId = m_iNextNetEntityId + slot - firstSlot;
        if (!ai.iPlayerId || !ai.iNetEntityId || m_Players.contains(ai.iPlayerId)) return false;
        ai.eCharacterClass = slot % 2u ? CHARACTER_CLASS_ID::LANCE_MASTER : CHARACTER_CLASS_ID::GUARDIANKNIGHT;
        const auto* profile = m_GameplayCatalog.Find_Player(ai.eCharacterClass);
        if (!profile) return false;
        ai.strNickName = "Waterpang AI " + std::to_string(slot + 1u);
        ai.strSpawnPlacementId = "waterpang.ai." + std::to_string(slot);
        ai.iCurrentHp = ai.iMaximumHp = profile->iMaximumHp;
        ai.iCurrentResource = ai.iMaximumResource = profile->iMaximumResource;
        ai.iMaximumIdentity = profile->iMaximumIdentity; ai.eStance = profile->eDefaultStance;
        ai.fMoveSpeed = profile->fMoveSpeed; ai.isCombatReady = true;
        CPlayerSkillSystem::Reset_Gauges(ai, m_GameplayCatalog);
        if (slot < 12u)
        {
            const std::string prefix = slot % 2u ? "AVATAR_LANCEMASTER_MOKOKO_036" : "AVATAR_GUARDIANKNIGHT_MOKOKO_036";
            const auto variant = slot / 2u; const auto set = prefix + (variant ? "-" + std::to_string(variant) : "");
            for (const auto& [suffix, equip] : { std::pair{"_HEAD", EQUIPMENT_SLOT::AVATAR_HEAD}, std::pair{"_OUTFIT", EQUIPMENT_SLOT::AVATAR_OUTFIT} })
            {
                const auto item = set + suffix;
                if (!m_ItemCatalog.Find_Item(item)) return false;
                ai.Inventory.push_back({item, 1u, equip});
            }
        }
        else ai.strWaterpangNpcArchetypeId = MAHARAKA_WATERPANG_AI_NPCS[slot - 12u];
        const float angle = (slot % 10u) * PI / 5.f + (slot / 10u) * PI / 10.f;
        const float radius = collapsed ? (slot < 10u ? 2.8f : 4.4f) : (slot < 10u ? 3.4f : 6.f);
        SERVER_NAV_POINT point;
        if (!Find_GuideLanding(ai, MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * radius, 22.4f,
            MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * radius, point) || point.y < MAHARAKA_WATERPANG_DECK_MIN_Y_M ||
            std::hypot(point.x - MAHARAKA_WATERPANG_CANNON_X, point.z - MAHARAKA_WATERPANG_CANNON_Z) > 7.f) return false;
        if (std::any_of(staged.begin(), staged.end(), [&](const auto& other) { return std::hypot(point.x - other.fPositionX, point.z - other.fPositionZ) < .75f; })) return false;
        ai.fPositionX = point.x; ai.fPositionY = point.y; ai.fPositionZ = point.z;
        ai.fYawDegrees = angle * 180.f / PI + 180.f;
        staged.push_back(std::move(ai));
    }
    for (auto& ai : staged)
    {
        MAHARAKA_WATERPANG_AI state; state.iSlot = static_cast<std::uint32_t>(m_MaharakaWaterpangAI.size()); state.iSkillSlot = state.iSlot % 4u;
        const auto id = ai.iPlayerId;
        m_PlayerIdByEntityId.emplace(ai.iNetEntityId, id); m_MaharakaWaterpangAI.emplace(id, state);
        m_Players.emplace(id, std::move(ai)); Broadcast_Spawned(m_Players.at(id), INVALID_SESSION_ID);
    }
    m_iNextNetEntityId += count; m_iNextWaterpangAIPlayerId += count;
    std::cout << "[WaterpangAI] spawned " << count << " contestants\n";
    return true;
}

void CGameRoom::Clear_MaharakaWaterpangAI(const std::uint32_t keepCount)
{
    for (auto it = m_MaharakaWaterpangAI.begin(); it != m_MaharakaWaterpangAI.end();)
    {
        if (it->second.iSlot < keepCount) { ++it; continue; }
        const auto id = it->first; it = m_MaharakaWaterpangAI.erase(it);
        auto actor = m_Players.find(id); if (actor == m_Players.end()) continue;
        m_CombatObjectRuntime.Cancel_Source(actor->second.iNetEntityId); m_ServerTriggerSystem.Remove_Player(id);
        Broadcast_Despawned(actor->second.iNetEntityId, PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
        m_PlayerIdByEntityId.erase(actor->second.iNetEntityId); m_Players.erase(actor);
    }
}

bool CGameRoom::Finish_MaharakaWaterpangMatch()
{
    if (!m_MaharakaWaterpangIntro) return false;
    struct RETURN_DESTINATION { PLAYER_ID id; SERVER_NAV_POINT point; float yaw; };
    std::vector<RETURN_DESTINATION> destinations;
    for (const auto& [id, player] : m_Players)
    {
        if (!player.Is_Human() || !player.bWaterpangParticipant) continue;
        const auto* spawn = Find_Placement(player.strSpawnPlacementId);
        SERVER_NAV_POINT point;
        // Stage every participant's own admission spawn before sending STOP or mutating the room.
        if (!spawn || !spawn->isEnabled || !player.iMaximumHp ||
            !Find_GuideLanding(player, spawn->fPositionX, spawn->fPositionY, spawn->fPositionZ, point) ||
            Is_MaharakaWaterpangArenaFootprint(point.x, point.z) ||
            std::any_of(destinations.begin(), destinations.end(), [&](const auto& other)
            { return std::abs(point.y - other.point.y) < 1.5f && std::hypot(point.x - other.point.x, point.z - other.point.z) < .75f; }))
        {
            m_strStatus = "Waterpang return pending: admission spawn landing unavailable for " + player.strSpawnPlacementId;
            return false;
        }
        destinations.push_back({id, point, spawn->fYawDegrees});
    }
    S2C_WORLD_SEQUENCE_PLAY stopped = *m_MaharakaWaterpangIntro;
    stopped.eOperation = WORLD_SEQUENCE_OPERATION::STOP; stopped.iServerTick = m_iServerTick;
    CPacketWriter writer; if (!Write_Message(writer, stopped)) return false;
    for (const auto& [id, player] : m_Players)
    {
        (void)id; if (auto session = Find_Session(player.iSessionId); session && !session->Send_Frame(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer())) session->Request_Close();
    }
    m_MaharakaWaterGunShots.clear(); Clear_MaharakaWaterpangAI();
    for (const auto& destination : destinations)
    {
        auto& player = m_Players.at(destination.id);
        Reset_PlayerForDebugTeleport(player);
        player.bWaterpangParticipant = player.bWaterpangFall = player.bWaterpangLaunch = false;
        // The replicated latest-cast pair must become zero together, including after its clip ended.
        player.iWaterGunSkillId = player.iWaterGunCastTick = player.iWaterGunCastEndTick = player.iWaterGunSpeedEndTick = 0u;
        player.fWaterGunSpeedScale = 1.f; player.iWaterpangCannonHitTick = 0u;
        for (const auto& skill : MAHARAKA_WATERGUN_SKILLS) player.CooldownEndTickBySkillId.erase(skill.iSkillId);
        player.bKnockbackBallistic = player.bKnockbackCanLeaveArena = player.bArenaEjectionActive = false;
        player.iEjectionOwnerNetEntityId = INVALID_NET_ENTITY_ID;
        player.bPushOnlyHitReaction = false;
        player.fKnockbackVelocityY = player.fKnockbackLaunchY = player.fKnockbackSupportY = 0.f;
        player.fKnockbackGravityMps2 = SERVER_PLAYER::KNOCKBACK_GRAVITY_MPS2;
        player.iCurrentHp = player.iMaximumHp;
        player.fPositionX = destination.point.x; player.fPositionY = destination.point.y; player.fPositionZ = destination.point.z;
        player.fYawDegrees = destination.yaw;
    }
    m_MaharakaWaterpangIntro.reset(); m_MaharakaWaterpangDebugEvent.reset(); m_iWaterpangAIRetryTick = 0;
    m_ServerTriggerSystem.Reset_SequenceActivation(MAHARAKA_WATERPANG_INTRO_INSTANCE);
    std::cout << "[WaterpangAI] three-minute match finished; returned " << destinations.size() << " humans to island\n";
    return true;
}

void CGameRoom::Update_MaharakaWaterpangMatch(const std::uint32_t updateTick)
{
    if (m_eWorldId != WORLD_ID::MAHARAKA) return;
    loadTuning(m_bMaharakaAITuningLoaded, m_MaharakaAITuning, m_strMaharakaAISourceBytes);
    if (!m_MaharakaWaterpangIntro) return;
    for (auto& [id, player] : m_Players)
    {
        (void)id;
        if (player.Is_Human() && (Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ) ||
            player.bWaterpangFall || player.bWaterpangLaunch ||
            (player.TriggerMove.isActive && Is_MaharakaWaterpangArenaFootprint(player.TriggerMove.fTargetX, player.TriggerMove.fTargetZ))))
            player.bWaterpangParticipant = true;
    }
    const auto elapsed = static_cast<std::int32_t>(updateTick - m_MaharakaWaterpangIntro->iStartTick);
    if (elapsed >= static_cast<std::int32_t>(MAHARAKA_WATERPANG_MATCH_END_TICKS)) { (void)Finish_MaharakaWaterpangMatch(); return; }
    if (m_MaharakaWaterpangAI.size() != m_MaharakaAITuning.iBotCount && reached(updateTick, m_iWaterpangAIRetryTick))
    {
        // Admit additions atomically and preserve existing contestants if navigation fails.
        Clear_MaharakaWaterpangAI(m_MaharakaAITuning.iBotCount);
        if (m_MaharakaWaterpangAI.size() < m_MaharakaAITuning.iBotCount && !Spawn_MaharakaWaterpangAI()) std::cout << "[WaterpangAI] spawn admission pending: navigation, profile, avatar or room capacity\n";
        m_iWaterpangAIRetryTick = nextTick(updateTick, 30u);
    }
    if (elapsed < static_cast<std::int32_t>(MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS * MAHARAKA_WATERPANG_TICK_HZ)) return;
    const auto& tuning = m_MaharakaAITuning;
    const bool collapsed = Has_MaharakaWaterpangCollapsedFloor(elapsed);
    for (auto& [id, state] : m_MaharakaWaterpangAI)
    {
        auto& ai = m_Players.at(id);
        if (!reached(updateTick, state.iNextThinkTick)) continue;
        state.iNextThinkTick = nextTick(updateTick, tuning.iDecisionTicks);
        if (!ai.iCurrentHp || ai.eAction != PLAYER_ACTION_STATE::NONE || ai.TriggerMove.isActive || ai.fKnockbackRemainingSeconds > 0.f || ai.bWaterpangFall) continue;
        const float radius = std::hypot(ai.fPositionX - MAHARAKA_WATERPANG_CANNON_X, ai.fPositionZ - MAHARAKA_WATERPANG_CANNON_Z);
        if (radius > 7.25f || ai.fPositionY < MAHARAKA_WATERPANG_DECK_MIN_Y_M)
        {
            SERVER_NAV_POINT landing; const float angle = state.iSlot * PI * .618f;
            const float landingRadius = collapsed ? MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M : 5.5f;
            if (Find_GuideLanding(ai, MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * landingRadius, 22.4f, MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * landingRadius, landing) && landing.y >= MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
                (!collapsed || std::hypot(landing.x - MAHARAKA_WATERPANG_CANNON_X, landing.z - MAHARAKA_WATERPANG_CANNON_Z) <= MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M))
            {
                ai.TriggerMove = {}; ai.TriggerMove.isActive = true; ai.TriggerMove.strSourcePlacementId = "waterpang.ai.rejoin";
                ai.TriggerMove.fStartX = ai.fPositionX; ai.TriggerMove.fStartY = ai.fPositionY; ai.TriggerMove.fStartZ = ai.fPositionZ;
                ai.TriggerMove.fTargetX = landing.x; ai.TriggerMove.fTargetY = landing.y; ai.TriggerMove.fTargetZ = landing.z;
                ai.TriggerMove.fDurationSeconds = 1.2f; ai.TriggerMove.fArcHeight = 2.5f; ai.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
                ai.iActionStartTick = updateTick; ai.hasMoveGoal = false; ai.MovePath.clear();
            }
            continue;
        }
        const SERVER_PLAYER* target = nullptr; float distance = tuning.fTargetRangeM;
        for (const auto& [otherId, other] : m_Players)
        {
            if (id == otherId || !other.iCurrentHp || other.Is_Guide() || other.eAction == PLAYER_ACTION_STATE::FALLING ||
                !Is_MaharakaWaterpangArenaFootprint(other.fPositionX, other.fPositionZ) || std::abs(ai.fPositionY - other.fPositionY) > 1.5f) continue;
            const float d = std::hypot(ai.fPositionX - other.fPositionX, ai.fPositionZ - other.fPositionZ);
            if (d < distance) { distance = d; target = &other; }
        }
        if (reached(updateTick, state.iNextMoveTick))
        {
            state.iNextMoveTick = nextTick(updateTick, tuning.iMoveRetargetTicks);
            if (roll(updateTick ^ id) < tuning.fMoveProbability)
            {
                const float angle = roll(updateTick + id * 37u) * 2.f * PI;
                const float range = collapsed ? 2.2f + roll(updateTick ^ (id * 77u)) * 2.1f :
                    2.8f + roll(updateTick ^ (id * 77u)) * 3.6f;
                C2S_MOVE command; command.iClientSequence = ++state.iSequence;
                command.fGoalX = MAHARAKA_WATERPANG_CANNON_X + std::sin(angle) * range;
                command.fGoalZ = MAHARAKA_WATERPANG_CANNON_Z + std::cos(angle) * range;
                Execute_PlayerMove(ai, command);
            }
        }
        if (target && reached(updateTick, state.iNextShotTick))
        {
            state.iNextShotTick = nextTick(updateTick, tuning.iSkillIntervalTicks + state.iSlot % 7u);
            if (roll(updateTick ^ (id * 313u)) < tuning.fAggression)
                for (unsigned attempt = 0; attempt < 4; ++attempt)
                {
                    const auto& skill = MAHARAKA_WATERGUN_SKILLS[state.iSkillSlot++ % MAHARAKA_WATERGUN_SKILLS.size()];
                    C2S_USE_SKILL command; command.iClientSequence = ++state.iSequence; command.iSkillId = skill.iSkillId;
                    command.fAimX = target->fPositionX; command.fAimZ = target->fPositionZ;
                    if (Try_StartMaharakaWaterGunSkill(ai, command)) break;
                }
        }
    }
}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/ServerGameplayContractTests_MaharakaAI.cpp

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <set>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_MaharakaAI()
{
    TESTS tests;

    // Exercise the actual G/vote/admission transaction without a listening socket.
    for (const unsigned partySize : {1u, 2u, 4u})
    {
        auto source = std::make_shared<CGameRoom>(WORLD_ID::BERN);
        auto island = std::make_shared<CGameRoom>(WORLD_ID::MAHARAKA);
        auto app = std::make_unique<CServerApp>();
        app->m_SharedGameRooms.emplace(WORLD_ID::BERN, source);
        app->m_SharedGameRooms.emplace(WORLD_ID::MAHARAKA, island);
        std::vector<std::shared_ptr<CClientSession>> peers;
        const auto clear = [&]() {
            for (auto& peer : peers) {
                peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0;
                peer->m_OutboundMetrics.iCurrentQueuedByteCount = 0;
                peer->m_OutboundMetrics.iCurrentQueuedFrameCount = 0;
            }
        };
        bool ready = source->Is_Ready() && island->Is_Ready();
        for (unsigned i = 0; i < partySize && ready; ++i)
        {
            auto peer = std::make_shared<CClientSession>(99701u + i, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true); source->Handle_Register(peer);
            C2S_ENTER_WORLD entry; entry.eWorldId = WORLD_ID::BERN;
            entry.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
            entry.strNickName = "IslandParty" + std::to_string(i);
            ready = source->Join(peer->Get_SessionId(), entry);
            peers.push_back(peer);
            app->m_Sessions.emplace(peer->Get_SessionId(), peer);
            CServerApp::SESSION_GAMEPLAY_BINDING binding;
            binding.eWorldId = WORLD_ID::BERN; binding.pSimulation = source;
            app->m_GameplayBindingBySessionId.emplace(peer->Get_SessionId(), binding);
            clear();
        }
        tests.Require(ready, "Island party fixtures admit the complete human roster");
        if (!ready) continue;
        auto& leader = source->m_Players.at(peers.front()->Get_PlayerId());
        for (unsigned i = 1; i < partySize; ++i)
        {
            C2S_PARTY_INVITE invite;
            invite.iTargetNetEntityId = source->m_Players.at(peers[i]->Get_PlayerId()).iNetEntityId;
            source->Handle_PartyInvite(leader.iSessionId, invite);
            C2S_PARTY_INVITE_RESPOND response;
            response.iFromNetEntityId = leader.iNetEntityId; response.bAccepted = true;
            source->Handle_PartyInviteRespond(peers[i]->Get_SessionId(), response);
        }
        clear();
        const auto* dock = source->Find_Placement("island.dock.to.maharaka");
        tests.Require(dock != nullptr, "Island entry uses the published Bern G dock");
        if (!dock) continue;
        for (unsigned i = 0; i < partySize; ++i)
        {
            auto& player = source->m_Players.at(peers[i]->Get_PlayerId());
            player.fPositionX = dock->fPositionX; player.fPositionY = dock->fPositionY; player.fPositionZ = dock->fPositionZ;
            player.iVehicleId = 8200u; player.bShipDockValid = true;
            player.fShipDockX = 10.f + i; player.fShipDockY = 1.f;
            player.fShipDockZ = 20.f + i; player.fShipDockYawDegrees = 30.f;
            player.Inventory.clear(); player.Purse.iSilver = 0u; player.Purse.iGold = 0u;
        }
        C2S_INTERACT_TRIGGER interact; interact.iRequestSequence = 501u;
        interact.strTriggerPlacementId = dock->strPlacementId;
        source->Handle_InteractTrigger(leader.iSessionId, interact);
        tests.Require(source->m_RaidEntryProposals.size() == 1u && source->m_PendingWorldTransfers.empty(),
            "G opens one island confirmation for all members before any departure");
        if (source->m_RaidEntryProposals.empty()) continue;
        C2S_RAID_ENTRY_RESPOND answer;
        answer.iProposalId = source->m_RaidEntryProposals.front().iProposalId; answer.bAccepted = false;
        source->Handle_RaidEntryRespond(peers.back()->Get_SessionId(), answer);
        tests.Require(source->m_RaidEntryProposals.empty() && source->m_PendingWorldTransfers.empty() &&
            source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "An island vote decline preserves every member in Bern");
        C2S_RAID_ENTRY_PROPOSE propose; propose.iRequestSequence = 502u;
        propose.eTarget = RAID_ENTRY_TARGET::MAHARAKA; propose.strNpcPlacementId = dock->strPlacementId;
        source->Handle_RaidEntryPropose(leader.iSessionId, propose);
        if (source->m_RaidEntryProposals.empty()) { tests.Require(false, "Island confirmation can be proposed again"); continue; }
        answer.iProposalId = source->m_RaidEntryProposals.front().iProposalId; answer.bAccepted = true;
        for (const auto& peer : peers) source->Handle_RaidEntryRespond(peer->Get_SessionId(), answer);
        SERVER_WORLD_TRANSFER_REQUEST transfer;
        const bool staged = source->Try_DequeueWorldTransfer(transfer);
        tests.Require(staged && transfer.PartyBatchSessionIds.size() == partySize &&
            source->m_MaharakaShipReturnBySession.size() == partySize,
            "Every island traveler, including solo, uses the bounded batch and retains its own ship");
        if (!staged) continue;
        clear();
        const std::array<std::uint8_t, 1> payload{1u};
        for (unsigned i = 0; i < CClientSession::MAX_OUTBOUND_FRAME_COUNT; ++i)
            (void)peers.back()->Send_Frame(PACKET_TYPE::S2C_CHAT, payload);
        CServerApp::SESSION_WORLD_TRANSFER_FAILURE failure;
        tests.Require(!app->Transfer_SessionWorld(source, transfer, failure) &&
            failure.ePartyResult == PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY &&
            source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "One full outbound queue rejects island admission without partial room or party mutation");
        clear();
        const bool entered = app->Transfer_SessionWorld(source, transfer, failure);
        tests.Require(entered && source->Count_HumanPlayers() == 0u && island->Count_HumanPlayers() == partySize,
            "All island travelers commit together after reliable preparation");
        if (!entered) continue;
        tests.Require(partySize == 1u ? island->m_PartyMembersByPartyId.empty() :
            island->m_PartyMembersByPartyId.size() == 1u && island->m_PartyMembersByPartyId.begin()->second.size() == partySize,
            "Island entry preserves the complete party roster");
        for (unsigned i = 0; i < partySize; ++i)
        {
            const auto& player = island->m_Players.at(peers[i]->Get_PlayerId());
            tests.Require(player.strNickName == "IslandParty" + std::to_string(i) && player.Inventory.empty() &&
                player.Purse.iSilver == 0u && player.Purse.iGold == 0u &&
                app->m_GameplayBindingBySessionId.at(peers[i]->Get_SessionId()).pSimulation == island,
                "Nickname, intentionally empty inventory/purse and authoritative session binding survive island entry");
        }
        clear();
        auto& islandLeader = island->m_Players.at(peers.front()->Get_PlayerId());
        const auto* exit = island->Find_Placement("island.exit.to.bern");
        tests.Require(exit != nullptr, "Island return uses the published G exit");
        if (!exit) continue;
        islandLeader.fPositionX = exit->fPositionX; islandLeader.fPositionY = exit->fPositionY; islandLeader.fPositionZ = exit->fPositionZ;
        interact.iRequestSequence = 503u; interact.strTriggerPlacementId = exit->strPlacementId;
        island->Handle_InteractTrigger(islandLeader.iSessionId, interact);
        tests.Require(island->m_RaidEntryProposals.size() == 1u, "Island G return also confirms with the party");
        if (island->m_RaidEntryProposals.empty()) continue;
        answer.iProposalId = island->m_RaidEntryProposals.front().iProposalId;
        for (const auto& peer : peers) island->Handle_RaidEntryRespond(peer->Get_SessionId(), answer);
        const bool returnStaged = island->Try_DequeueWorldTransfer(transfer);
        tests.Require(returnStaged && transfer.strSpawnPlacementOverrideId == "island.return.sea.landing",
            "Party return keeps the authored sea landing");
        clear();
        const bool returned = returnStaged && app->Transfer_SessionWorld(island, transfer, failure);
        tests.Require(returned && source->Count_HumanPlayers() == partySize && island->Count_HumanPlayers() == 0u,
            "Party return commits every member back into Bern");
        if (returned) for (unsigned i = 0; i < partySize; ++i)
        {
            const auto& player = source->m_Players.at(peers[i]->Get_PlayerId());
            tests.Require(player.iVehicleId == 8200u && player.bShipDockValid &&
                player.fShipDockX == 10.f + i && player.Inventory.empty() && player.Purse.iSilver == 0u,
                "Each returning member restores its own ship and empty inventory without fresh grants");
        }
    }
    for (const auto world : {WORLD_ID::MAHARAKA, WORLD_ID::VALTAN_ARENA, WORLD_ID::KAKULSAYDON_ARENA})
    {
        auto fallRoom = std::make_unique<CGameRoom>(world);
        SERVER_PLAYER falling; falling.iCurrentHp = falling.iMaximumHp = 100u;
        falling.fPositionX = MAHARAKA_WATERPANG_CANNON_X;
        falling.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z; falling.fPositionY = 22.4f;
        if (world == WORLD_ID::KAKULSAYDON_ARENA) falling.iMarioStage = 1u;
        fallRoom->Begin_PlayerFall(falling, 0.f, 100u);
        const bool water = world == WORLD_ID::MAHARAKA;
        falling.bWaterpangFall = water;
        tests.Require(std::abs(falling.fFallDeathPlaneY - (water ? 20.4f : 17.4f)) < .001f,
            "Only Maharaka changes the fall depth to two meters; both raid fall planes retain five");
        falling.fPositionY = falling.fFallDeathPlaneY + .01f;
        (void)fallRoom->Update_PlayerFall(falling, 0.f, 101u);
        tests.Require(falling.eAction == PLAYER_ACTION_STATE::FALLING && falling.iCurrentHp == 100u,
            "Above the fall plane no world resolves the fall early");
        falling.fPositionY = falling.fFallDeathPlaneY - .01f;
        (void)fallRoom->Update_PlayerFall(falling, 0.f, 102u);
        tests.Require(water ? falling.eAction == PLAYER_ACTION_STATE::NONE && falling.iCurrentHp == 100u :
            world == WORLD_ID::VALTAN_ARENA ? falling.eAction == PLAYER_ACTION_STATE::FALLING && falling.iCurrentHp == 100u :
            falling.eAction == PLAYER_ACTION_STATE::DEAD && falling.iCurrentHp == 0u,
            "Waterpang returns alive at two meters, Valtan keeps its deadline and Kouku keeps its original height death");
        if (world == WORLD_ID::VALTAN_ARENA)
        {
            (void)fallRoom->Update_PlayerFall(falling, 0.f, 145u);
            tests.Require(falling.eAction == PLAYER_ACTION_STATE::DEAD && falling.iCurrentHp == 0u,
                "Valtan still resolves falling only at its original 45-tick deadline");
        }
    }

    Run_WorldPlayback(tests);
    auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
    tests.Require(room->Is_Ready(), "Waterpang AI fixture loads the published Maharaka room");
    if (!room->Is_Ready()) return 1;
    std::vector<std::shared_ptr<CClientSession>> sessions;
    const auto drain = [&]() { for (auto& session : sessions) { session->m_OutboundFrames.clear(); session->m_iQueuedOutboundBytes = 0; session->m_OutboundMetrics.iCurrentQueuedByteCount = 0; session->m_OutboundMetrics.iCurrentQueuedFrameCount = 0; } };
    for (unsigned i = 0; i < 4; ++i)
    {
        auto session = std::make_shared<CClientSession>(99501u + i, INVALID_SOCKET, CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true); room->Handle_Register(session);
        C2S_ENTER_WORLD enter; enter.eWorldId = WORLD_ID::MAHARAKA; enter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER; enter.strNickName = "Waterpang contract";
        tests.Require(room->Join(session->Get_SessionId(), enter), "Human admission succeeds beside optional Waterpang contestants");
        sessions.push_back(session); drain();
    }
    if (room->Count_HumanPlayers() != 4) return 1;
    C2S_MAHARAKA_AI_TUNING request; request.iRequestSequence = 1;
    CPacketWriter requestWriter; C2S_MAHARAKA_AI_TUNING decoded;
    const bool written = Write_Message(requestWriter, request); CPacketReader requestReader(requestWriter.Get_Buffer());
    tests.Require(written && Read_Message(requestReader, decoded) && !requestReader.Get_RemainingSize(), "AI typed GET request round trips");
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    const auto revision = room->m_MaharakaAITuning.iRevision;
    request.eOperation = MAHARAKA_AI_OPERATION::APPLY; request.iExpectedRevision = revision;
    request.Tuning = room->m_MaharakaAITuning; request.Tuning.fMoveProbability = 1.f; request.Tuning.fAggression = 1.f;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1 && room->m_MaharakaAITuning.fAggression == 1.f, "AI APPLY commits the new revision and live decision values");
    request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iBotCount == 20 && room->m_MaharakaAITuning.iRevision == revision + 1, "Stale tuning CAS preserves the live roster and revision");
    request.iExpectedRevision = revision + 1; request.Tuning.fAggression = std::numeric_limits<float>::quiet_NaN();
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 1, "Nonfinite AI tuning is rejected before mutation");
    // Exercise the real SAVE path in an isolated authoring root, without touching team data.
    const auto originalCwd = std::filesystem::current_path();
    const auto isolated = originalCwd / "out/WaterpangEffects20260930/ai-save-fixture";
    std::filesystem::create_directories(isolated / "Data/AI");
    { std::ofstream marker(isolated / "AGENTS.md"); marker << "isolated test root\n"; }
    const auto source = isolated / "Data/AI/MaharakaWaterpangAI.json";
    { std::ofstream stream(source, std::ios::binary); stream << room->m_strMaharakaAISourceBytes; }
    std::filesystem::current_path(isolated);
    request.eOperation = MAHARAKA_AI_OPERATION::SAVE; request.Tuning = room->m_MaharakaAITuning;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && std::filesystem::is_regular_file(source.string() + ".backup"), "AI SAVE validates and atomically replaces source with a recoverable backup");
    { std::ofstream stream(source, std::ios::app); stream << " \n"; }
    request.iExpectedRevision = revision + 2; request.Tuning.iBotCount = 0;
    room->Handle_MaharakaAITuning(sessions[0]->Get_SessionId(), request); drain();
    tests.Require(room->m_MaharakaAITuning.iRevision == revision + 2 && room->m_MaharakaAITuning.iBotCount == 20, "External source edit rejects SAVE and preserves active tuning");
    std::filesystem::current_path(originalCwd);

    S2C_WORLD_SEQUENCE_PLAY intro; intro.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
    intro.strSequenceInstanceId = MAHARAKA_WATERPANG_INTRO_INSTANCE; intro.iStartTick = 100;
    room->m_MaharakaWaterpangIntro = intro; room->m_iServerTick = 100;
    room->Update_MaharakaWaterpangMatch(100);
    std::set<std::string> expectedNames;
    for (unsigned slot = 1u; slot <= 20u; ++slot) expectedNames.insert("Waterpang AI " + std::to_string(slot));
    const auto numberedRoster = [&]()
    {
        std::set<std::string> names;
        for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
        {
            const auto& player = room->m_Players.at(id);
            if (player.strNickName != "Waterpang AI " + std::to_string(state.iSlot + 1u) ||
                !names.insert(player.strNickName).second) return false;
        }
        return names == expectedNames;
    };
    tests.Require(numberedRoster(), "Twenty Waterpang AI have unique stable slot names 1 through 20 across avatar and NPC appearances");
    bool spawnNames = true;
    for (const auto& session : sessions)
    {
        std::set<std::string> observedNames;
        for (const auto& frame : session->m_OutboundFrames)
        {
            if (frame.ePacketType != PACKET_TYPE::S2C_PLAYER_SPAWNED) continue;
            CPacketReader reader(std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES));
            S2C_PLAYER_SPAWNED spawned;
            if (!Read_Message(reader, spawned) || reader.Get_RemainingSize()) { spawnNames = false; continue; }
            if (spawned.eControlKind == PLAYER_CONTROL_KIND::WATERPANG_AI) observedNames.insert(spawned.strNickName);
        }
        spawnNames = spawnNames && observedNames == expectedNames;
    }
    tests.Require(spawnNames, "Actual reliable spawn frames deliver all twenty AI nicknames unchanged to every human session");
    drain();
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->m_Players.size() == 24 && room->Count_HumanPlayers() == 4, "Twenty AI fit the real navigation and retain all four human slots");
    if (room->m_MaharakaWaterpangAI.size() != 20) return 1;
    unsigned avatars = 0, npcs = 0;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        tests.Require(player.Is_WaterpangAI() && !player.Is_Human() && player.iSessionId == INVALID_SESSION_ID && player.fPositionY >= MAHARAKA_WATERPANG_DECK_MIN_Y_M, "AI uses native deck support without a fake session");
        if (player.strWaterpangNpcArchetypeId.empty()) avatars += player.Inventory.size() == 2;
        else ++npcs;
    }
    tests.Require(avatars == 12 && npcs == 8, "Roster carries twelve real avatar loadouts and eight immutable NPC identities");
    room->m_iServerTick = 100 + 20 * 30;
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick); drain();
    bool moved = false, usedSkill = false;
    for (const auto& [id, state] : room->m_MaharakaWaterpangAI)
    {
        const auto& player = room->m_Players.at(id);
        moved = moved || player.iLastMoveSequence != 0;
        usedSkill = usedSkill || player.iWaterGunSkillId != 0;
    }
    tests.Require(moved && usedSkill, "Actual AI decision loop submits admitted movement and Waterpang skills");
    for (unsigned i = 0; i < 15; ++i) { room->Tick(1.f / 30.f); drain(); }
    tests.Require(room->m_MaharakaWaterpangAI.size() == 20 && room->Count_HumanPlayers() == 4, "Actual room ticks preserve AI ownership and human roster");
    tests.Require(numberedRoster(), "Movement and skill updates preserve every AI slot nickname");
    std::array<SERVER_NAV_POINT, 4> admitted;
    for (unsigned i = 0; i < sessions.size(); ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        admitted[i] = {player.fPositionX, player.fPositionY, player.fPositionZ};
    }
    auto& human = room->m_Players.at(sessions[0]->Get_PlayerId());
    // Isolate collapse probes from the main branch\'s independent return/snapshot regression.
    {
        const auto beforeCollapseHuman=human;
        const auto beforeCollapseTick=room->m_iServerTick;
        tests.Require(MAHARAKA_WATERPANG_MATCH_END_TICKS - MAHARAKA_WATERPANG_COLLAPSE_START_TICKS == 60u * 30u &&
            MAHARAKA_WATERPANG_COLLAPSE_START_TICKS == (20u + 120u) * 30u,
            "Original ring collapse starts with exactly one minute remaining, excluding countdown and intro");
        const auto missingTick = intro.iStartTick + MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS;
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 22.4f; human.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
        human.eAction = PLAYER_ACTION_STATE::NONE; human.TriggerMove = {};
        human.bKnockbackBallistic = human.bArenaEjectionActive = false;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick - 1u),
            "Ring keeps support before the collapse gameplay transition");
        tests.Require(room->Update_PlayerFall(human, 1.f / 30.f, missingTick) &&
            human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::FALLING && human.fPositionY < 22.4f,
            "A stationary contestant on the collapsed ring begins the existing Waterpang fall");
        for (unsigned step=1;step<=60u && human.bWaterpangFall;++step)
            room->Update_PlayerFall(human, 1.f / 30.f, missingTick+step);
        tests.Require(human.iCurrentHp && !human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::NONE,
            "Collapsed ring fall returns the contestant alive to a jump pier");
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick + 46u),
            "Side jump piers do not collapse with the outer ring");
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 4.f; human.fPositionY = 22.4f; human.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick),
            "The yellow centre remains supported after ring collapse");
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 20.48f;
        tests.Require(!room->Update_PlayerFall(human, 1.f / 30.f, missingTick),
            "Ordinary exploration below the arena is not a collapsed-deck fall");
        human.TriggerMove.isActive = true; human.TriggerMove.strSourcePlacementId = "jump1";
        human.TriggerMove.fStartX = human.fPositionX; human.TriggerMove.fStartY = human.fPositionY; human.TriggerMove.fStartZ = human.fPositionZ;
        human.TriggerMove.fTargetX = human.fPositionX; human.TriggerMove.fTargetY = 22.4f; human.TriggerMove.fTargetZ = human.fPositionZ;
        human.TriggerMove.fDurationSeconds = 1.f; human.TriggerMove.fArcHeight = 2.f;
        human.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
        room->m_iServerTick = missingTick;
        room->Update_Players(1.f / 30.f); drain();
        tests.Require(human.TriggerMove.isActive && std::hypot(
            human.TriggerMove.fTargetX - MAHARAKA_WATERPANG_CANNON_X,
            human.TriggerMove.fTargetZ - MAHARAKA_WATERPANG_CANNON_Z) < MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M,
            "G jump targets the remaining centre without modifying authored boxes");
        for (const auto name : MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS)
        {
            const auto* box=room->Find_Placement(std::string(name));
            tests.Require(box && box->TriggerActions.size()==1u,"Collapsed match keeps each published jump action");
            if (!box || box->TriggerActions.size()!=1u) continue;
            human.TriggerMove={}; human.eAction=PLAYER_ACTION_STATE::NONE;
            human.bWaterpangFall=human.bKnockbackBallistic=human.bArenaEjectionActive=false;
            human.fKnockbackRemainingSeconds=0.f;
            human.fPositionX=box->fPositionX; human.fPositionY=box->fPositionY; human.fPositionZ=box->fPositionZ;
            CServerTriggerSystem jump; jump.Set_WorldId(WORLD_ID::MAHARAKA);
            std::string status;
            tests.Require(jump.Initialize({*box},status),"Post-collapse G trigger initializes from the real published box");
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
            const auto activate=[](WORLD_TRIGGER_ACTION_KIND,const std::string&){return true;};
            tests.Require(jump.Activate_Here(human.iPlayerId,room->m_Players,missingTick,transfers,activate)==1u,
                "Post-collapse G input activates the saved crossing");
            const auto& authored=box->TriggerActions.front();
            room->m_iServerTick=missingTick;
            room->Update_Players(1.f/30.f); drain();
            const auto landing=human.TriggerMove;
            const bool onMissingRing=Is_MaharakaWaterpangMissingRing(MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS,
                authored.fTargetX,authored.fTargetZ);
            tests.Require(landing.isActive && (onMissingRing ? std::abs(std::hypot(
                landing.fTargetX-MAHARAKA_WATERPANG_CANNON_X,landing.fTargetZ-MAHARAKA_WATERPANG_CANNON_Z)-
                MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M)<.001f :
                landing.fTargetX==authored.fTargetX && landing.fTargetY==authored.fTargetY && landing.fTargetZ==authored.fTargetZ),
                "Collapse preserves safe edited landings and adapts only missing outer-ring destinations");
            for (unsigned tick=1;tick<45;++tick)
            {
                room->m_iServerTick=missingTick+tick;
                room->Update_Players(1.f/30.f); drain();
            }
            tests.Require(!human.TriggerMove.isActive && human.iCurrentHp && !human.bWaterpangFall &&
                human.eAction!=PLAYER_ACTION_STATE::FALLING && human.fPositionY>=MAHARAKA_WATERPANG_DECK_MIN_Y_M &&
                std::hypot(human.fPositionX-landing.fTargetX,human.fPositionZ-landing.fTargetZ)<.01f,
                "Every real G jump lands alive and stays supported after collapse");
        }
        human.TriggerMove = {}; human.eAction = PLAYER_ACTION_STATE::NONE;
        human.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 6.f; human.fPositionY = 22.5f;
        human.bKnockbackBallistic = true; human.bKnockbackCanLeaveArena = true;
        human.fKnockbackRemainingSeconds = 1.f / 30.f; human.fKnockbackVelocityY = -6.f;
        human.fKnockbackLaunchY = human.fKnockbackSupportY = 22.4f; human.fKnockbackSpeed = 0.f;
        room->Advance_PlayerKnockback(human, 1.f / 30.f);
        tests.Require(human.bWaterpangFall && human.eAction == PLAYER_ACTION_STATE::FALLING,
            "Airborne knockback cannot land on the old baked ring height");
        human=beforeCollapseHuman;
        room->m_iServerTick=beforeCollapseTick;
    }
    auto& fallen = room->m_Players.at(sessions[1]->Get_PlayerId());
    auto& launched = room->m_Players.at(sessions[2]->Get_PlayerId());
    auto& visitor = room->m_Players.at(sessions[3]->Get_PlayerId());
    for (unsigned i = 0; i < 3u; ++i)
    {
        auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        player.fPositionX = MAHARAKA_WATERPANG_CANNON_X + (i == 0u ? 4.f : i == 1u ? -4.f : 0.f);
        player.fPositionY = 22.4f; player.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z + (i == 2u ? 4.f : 0.f);
    }
    C2S_USE_SKILL shot; shot.iClientSequence = human.iLastSkillSequence + 1u;
    shot.iSkillId = MAHARAKA_WATERGUN_SKILLS.front().iSkillId;
    shot.fAimX = fallen.fPositionX; shot.fAimZ = fallen.fPositionZ;
    tests.Require(room->Try_StartMaharakaWaterGunSkill(human, shot) && human.iWaterGunCastTick != 0u,
        "A real human Waterpang cast precedes the match-end snapshot regression");
    room->Update_MaharakaWaterpangMatch(room->m_iServerTick);
    tests.Require(human.bWaterpangParticipant && fallen.bWaterpangParticipant && launched.bWaterpangParticipant && !visitor.bWaterpangParticipant,
        "Match membership retains arena humans without capturing island visitors");
    // One contestant has already returned to a jump box; another is still in flight at expiry.
    fallen.fPositionX = 66.f; fallen.fPositionY = 20.48f; fallen.fPositionZ = -998.f;
    fallen.iCurrentHp = 0u; fallen.eAction = PLAYER_ACTION_STATE::DEAD;
    launched.fPositionX = 90.f; launched.fPositionY = 25.f; launched.fPositionZ = -984.f;
    launched.bWaterpangLaunch = launched.bWaterpangFall = launched.bKnockbackBallistic = launched.bKnockbackCanLeaveArena = true;
    launched.fKnockbackRemainingSeconds = 1.f; launched.fKnockbackVelocityY = 5.f;
    launched.iFallDeathTick = room->m_iServerTick + 10u; launched.eAction = PLAYER_ACTION_STATE::FALLING;
    human.fWaterGunSpeedScale = 1.8f; human.iWaterGunSpeedEndTick = room->m_iServerTick + 100u;
    human.iWaterpangCannonHitTick = room->m_iServerTick;
    for (const auto& skill : MAHARAKA_WATERGUN_SKILLS) human.CooldownEndTickBySkillId[skill.iSkillId] = room->m_iServerTick + 100u;
    const auto spawnId = fallen.strSpawnPlacementId;
    fallen.strSpawnPlacementId = "missing.waterpang.return.spawn";
    const auto shotCount = room->m_MaharakaWaterGunShots.size();
    drain();
    tests.Require(!room->Finish_MaharakaWaterpangMatch() && room->m_MaharakaWaterpangIntro &&
        room->m_MaharakaWaterpangAI.size() == 20u && room->m_MaharakaWaterGunShots.size() == shotCount &&
        human.iWaterGunCastTick != 0u && human.bWaterpangParticipant && fallen.iCurrentHp == 0u &&
        std::abs(human.fPositionX - (MAHARAKA_WATERPANG_CANNON_X + 4.f)) < .001f &&
        sessions[0]->m_OutboundFrames.empty(),
        "One invalid return destination preserves all positions, AI, casts and the sequence transaction");
    fallen.strSpawnPlacementId = spawnId;
    room->m_iServerTick = intro.iStartTick + MAHARAKA_WATERPANG_MATCH_END_TICKS - 1u;
    const auto encodeFailures = room->m_PerformanceMetrics.iSnapshotEncodeFailureCount;
    room->Tick(1.f / 30.f);
    tests.Require(!room->m_MaharakaWaterpangIntro && !room->m_MaharakaWaterpangDebugEvent &&
        room->m_MaharakaWaterpangAI.empty() && room->m_MaharakaWaterGunShots.empty() && room->Count_HumanPlayers() == 4u,
        "Actual expiry tick clears the match and all AI while retaining the connected humans");
    {
        const auto returnedHuman=human;
        human.fPositionX=MAHARAKA_WATERPANG_CANNON_X+6.f;
        human.fPositionY=22.4f; human.fPositionZ=MAHARAKA_WATERPANG_CANNON_Z;
        tests.Require(!room->Update_PlayerFall(human,1.f/30.f,room->m_iServerTick),
            "Match reset restores the ring's normal support without stale collapse state");
        human=returnedHuman;
    }
    room->Refresh_PlayerBlockingBodies();
    for (unsigned i = 0; i < 3u; ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        const auto* spawn = room->Find_Placement(player.strSpawnPlacementId);
        SERVER_NAV_POINT returnGround{}, markerGround{};
        const bool returnNavigable = room->m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, returnGround, player.fPositionY) &&
            std::abs(returnGround.y - player.fPositionY) < .05f;
        const bool returnClear = room->m_ServerCollisionSystem.Is_PlayerPositionClear(player.fPositionX, player.fPositionY, player.fPositionZ, player.iNetEntityId);
        const bool markerNavigable = spawn && room->m_ServerNavigation.Sample_Position(spawn->fPositionX, spawn->fPositionZ, markerGround, spawn->fPositionY) &&
            std::abs(markerGround.y - spawn->fPositionY) <= 2.f;
        const bool markerClear = spawn && room->m_ServerCollisionSystem.Is_PlayerPositionClear(spawn->fPositionX, spawn->fPositionY, spawn->fPositionZ, player.iNetEntityId);
        const float markerDistance = spawn ? std::hypot(player.fPositionX - spawn->fPositionX, player.fPositionZ - spawn->fPositionZ) : 1000.f;
        std::cout << "[WaterpangLanding] participant=" << i << " markerDistance=" << markerDistance
            << " markerNav/clear=" << markerNavigable << '/' << markerClear
            << " returnNav/clear=" << returnNavigable << '/' << returnClear << '\n';
        tests.Require(spawn && markerDistance <= 3.001f && returnNavigable && returnClear &&
            !Is_MaharakaWaterpangArenaFootprint(player.fPositionX, player.fPositionZ),
            "Each admission marker returns onto nearby authoritative navigation outside every blocking body");
        tests.Require(markerDistance < .4f || !markerNavigable || !markerClear,
            "A displaced landing is justified by the authored marker failing navigation or actual collision admission");
        std::cout << "[WaterpangReturn] participant=" << i << " spawn=" << player.strSpawnPlacementId
            << " admitted=" << admitted[i].x << ',' << admitted[i].y << ',' << admitted[i].z
            << " returned=" << player.fPositionX << ',' << player.fPositionY << ',' << player.fPositionZ
            << " hp=" << player.iCurrentHp << '/' << player.iMaximumHp << " action=" << static_cast<unsigned>(player.eAction)
            << " participant/fall/launch/ballistic/leave/trigger=" << player.bWaterpangParticipant << '/' << player.bWaterpangFall
            << '/' << player.bWaterpangLaunch << '/' << player.bKnockbackBallistic << '/' << player.bKnockbackCanLeaveArena
            << '/' << player.TriggerMove.isActive << " fallTick=" << player.iFallDeathTick
            << " cast=" << player.iWaterGunSkillId << '/' << player.iWaterGunCastTick << '/' << player.iWaterGunCastEndTick
            << " speed=" << player.iWaterGunSpeedEndTick << '/' << player.fWaterGunSpeedScale << '\n';
        tests.Require(player.iCurrentHp == player.iMaximumHp &&
            player.eAction == PLAYER_ACTION_STATE::NONE && !player.bWaterpangParticipant &&
            !player.bWaterpangFall && !player.bWaterpangLaunch && !player.bKnockbackBallistic &&
            !player.bKnockbackCanLeaveArena && !player.TriggerMove.isActive && !player.iFallDeathTick &&
            !player.iWaterGunSkillId && !player.iWaterGunCastTick && !player.iWaterGunCastEndTick &&
            !player.iWaterGunSpeedEndTick && player.fWaterGunSpeedScale == 1.f,
            "Every participant returns alive to its own admission spawn with all Waterpang motion and cast state cleared");
    }
    tests.Require(visitor.fPositionX == admitted[3].x && visitor.fPositionY == admitted[3].y && visitor.fPositionZ == admitted[3].z,
        "An island visitor remains at the existing location");
    for (const auto& skill : MAHARAKA_WATERGUN_SKILLS)
        tests.Require(!human.CooldownEndTickBySkillId.contains(skill.iSkillId), "Match return clears only the Waterpang skill cooldowns");
    bool stopped = false;
    unsigned snapshotRecipients = 0u;
    for (const auto& session : sessions)
    {
        bool snapshotReceived = false;
        for (const auto& frame : session->m_OutboundFrames)
        {
            CPacketReader reader{std::span<const std::uint8_t>{frame.Bytes}.subspan(PACKET_HEADER_BYTES)};
            if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
            {
                S2C_WORLD_SEQUENCE_PLAY play;
                stopped = stopped || (Read_Message(reader, play) && play.eOperation == WORLD_SEQUENCE_OPERATION::STOP);
            }
            if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT)
            {
                S2C_WORLD_SNAPSHOT snapshot;
                snapshotReceived = Read_Message(reader, snapshot) && !reader.Get_RemainingSize() && snapshot.Players.size() == 4u &&
                    std::all_of(snapshot.Players.begin(), snapshot.Players.end(), [](const auto& player)
                    { return player.iWaterGunSkillId == 0u && player.iWaterGunCastTick == 0u && !player.isWaterpangArmed; });
            }
        }
        snapshotRecipients += snapshotReceived ? 1u : 0u;
    }
    tests.Require(stopped && snapshotRecipients == 4u && room->m_PerformanceMetrics.iSnapshotEncodeFailureCount == encodeFailures,
        "STOP and a valid post-return world snapshot reach every client without the stale cast-tick failure");
    drain();
    std::array<SERVER_NAV_POINT, 3> beforeMove;
    for (unsigned i = 0; i < beforeMove.size(); ++i)
    {
        auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        beforeMove[i] = {player.fPositionX, player.fPositionY, player.fPositionZ};
        bool submitted = false;
        for (unsigned direction = 0; direction < 8u && !submitted; ++direction)
        {
            const float angle = direction * 3.14159265359f / 4.f;
            SERVER_NAV_POINT goal{};
            const float x = player.fPositionX + .75f * std::cos(angle), z = player.fPositionZ + .75f * std::sin(angle);
            if (!room->m_ServerNavigation.Sample_Position(x, z, goal, player.fPositionY) ||
                !room->m_ServerNavigation.Has_LineOfSight(player.fPositionX, player.fPositionZ, goal.x, goal.z, player.fPositionY) ||
                !room->m_ServerCollisionSystem.Is_PlayerPositionClear(goal.x, goal.y, goal.z, player.iNetEntityId)) continue;
            float sweepX, sweepY, sweepZ; bool blocked = false;
            if (!room->m_ServerCollisionSystem.Resolve_PlayerMove(player, goal.x, goal.y, goal.z, sweepX, sweepY, sweepZ, blocked) || blocked) continue;
            C2S_MOVE move; move.iClientSequence = player.iLastMoveSequence + 1u;
            move.fGoalX = goal.x; move.fGoalZ = goal.z;
            room->Execute_PlayerMove(player, move);
            submitted = player.hasMoveGoal;
        }
        tests.Require(submitted, "Every returned participant admits a movement command along an actual free navigation route");
    }
    room->Tick(1.f / 30.f); drain();
    for (unsigned i = 0; i < beforeMove.size(); ++i)
    {
        const auto& player = room->m_Players.at(sessions[i]->Get_PlayerId());
        tests.Require(std::hypot(player.fPositionX - beforeMove[i].x, player.fPositionZ - beforeMove[i].z) > .005f,
            "Every returned participant advances on the next authoritative room tick");
    }
    const auto* jump = room->Find_Placement("jump3");
    tests.Require(jump != nullptr, "Published Waterpang re-entry jump exists");
    if (jump)
    {
        human.hasMoveGoal = false; human.MovePath.clear();
        human.fPositionX = jump->fPositionX; human.fPositionY = jump->fPositionY; human.fPositionZ = jump->fPositionZ;
        std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
        const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& target)
        { return room->Activate_TriggerTarget(kind, target); };
        const bool jumped = room->m_ServerTriggerSystem.Activate_Interact(human.iPlayerId, jump->strPlacementId,
            room->m_Players, room->m_iServerTick + 1u, transfers, activate);
        tests.Require(jumped && human.TriggerMove.isActive, "The same player can activate the real G jump after match return");
        for (unsigned i = 0; i < 45u; ++i) { room->Tick(1.f / 30.f); drain(); }
        tests.Require(room->m_MaharakaWaterpangIntro && room->m_MaharakaWaterpangIntro->iStartTick >
            intro.iStartTick + MAHARAKA_WATERPANG_MATCH_END_TICKS && room->m_MaharakaWaterpangAI.size() == 20u,
            "Re-entry landing reserves a new countdown and admits the next AI roster without leaving the room");
        tests.Require(numberedRoster(), "The next match respawns unique AI names from 1 through 20 without stale or duplicate suffixes");
    }
    for (const auto& session : sessions) room->Leave(session->Get_SessionId(), PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
    return tests.failures ? 1 : 0;
}
```

## C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/ServerGameplayContractTests_WorldPlayback.cpp

변경 종류: 기존 파일 수정. 적용 위치와 책임은 PLAN G06/G07을 따른다.

```cpp
#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerTriggerSystem.h"
#include "Gameplay/MaharakaWaterpangContract.h"
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

int LostArk::Server::CServerGameplayContractRunner::Run_WorldPlayback(TESTS& tests)
{
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
        CCombatObjectRuntime runtime;
        SERVER_PLAYER owner; owner.iPlayerId=1001u; owner.iNetEntityId=1002u;
        owner.fPositionX=10.f; owner.fPositionY=20.f; owner.fPositionZ=30.f; owner.fYawDegrees=90.f;
        std::string status;
        for (const auto skill : {56900u, 56910u, 56930u})
        {
            auto staged=runtime.Begin_Transaction();
            tests.Require(runtime.Stage_WaterGunPresentation(staged,owner,skill,room->m_GameplayCatalog,500u,status),
                "Watergun stages a source-owned projectile");
            if (staged.Objects.empty()) continue;
            const auto id=staged.Objects.front().iCombatObjectId;
            tests.Require(runtime.Get_LiveObjects().empty() && runtime.Commit(std::move(staged)),
                "Staging never exposes a partial projectile");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
            std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
            std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
            runtime.Drain_Lifecycle(spawned,events,despawned);
            const float expectedX=skill==56910u?10.f:10.7f;
            const float expectedY=skill==56910u?20.f:20.75f;
            const float expectedZ=skill==56910u?30.f:skill==56900u?29.8f:29.89f;
            tests.Require(spawned.size()==1u && std::abs(spawned[0].fPositionX-expectedX)<.001f &&
                std::abs(spawned[0].fPositionY-expectedY)<.001f && std::abs(spawned[0].fPositionZ-expectedZ)<.001f,
                "Source forward/right/up launch offset is converted once at yaw90");
            tests.Require(!runtime.Finish_WaterGunPresentation(id,999u,500u,501u,true) &&
                !runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,499u,501u,true) &&
                runtime.Get_LiveObjects().size()==1u,"Wrong owner or cast cannot finish a live watergun object");
            tests.Require(runtime.Set_OwnedVisualPosition(id,owner.iNetEntityId,500u,15.f,20.f,34.f),
                "Watergun flight consumes the authoritative room pose");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> late;
            runtime.Build_LiveSpawnMessages(510u,late);
            tests.Require(late.size()==1u && late[0].iSpawnTick==500u && late[0].iServerTick==510u &&
                late[0].fPositionX==15.f && late[0].fPositionZ==34.f,
                "Late join keeps cast age and current projectile position");
            tests.Require(runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,500u,511u,true) &&
                !runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,500u,512u,true),
                "A projectile impact finishes exactly once");
            runtime.Drain_Lifecycle(spawned,events,despawned);
            tests.Require(events.size()==1u && despawned.size()==1u && runtime.Get_LiveObjects().empty() &&
                events[0].strHitId=="maharaka.watergun.impact" && events[0].fPositionX==15.f &&
                events[0].fPositionZ==34.f && events[0].PinnedDefinitionRevision==room->m_GameplayCatalog.Get_ActiveRevision(),
                "Impact and despawn carry the last authoritative pose and pinned revision");
        }
        auto invalid=runtime.Begin_Transaction();
        tests.Require(!runtime.Stage_WaterGunPresentation(invalid,owner,56920u,room->m_GameplayCatalog,520u,status) &&
            invalid.Objects.empty() && invalid.Spawned.empty(),"Speed buff cannot create a water projectile");
        auto expired=runtime.Begin_Transaction();
        const bool staged=runtime.Stage_WaterGunPresentation(expired,owner,56900u,room->m_GameplayCatalog,530u,status);
        const auto id=staged?expired.Objects.front().iCombatObjectId:0u;
        tests.Require(staged && runtime.Commit(std::move(expired)) &&
            runtime.Finish_WaterGunPresentation(id,owner.iNetEntityId,530u,560u,false),
            "A missed watergun projectile expires without impact");
        std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
        std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
        std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
        runtime.Drain_Lifecycle(spawned,events,despawned);
        tests.Require(events.empty() && despawned.size()==1u,"Miss expiry emits no fabricated hit");
        // Independent source fields, rather than the old misread MaxDistance-as-Speed:
        // EFSequenceSummonsProjectile Speed=1000cm/s; Q570020/R569320 distance=330/300cm.
        const auto* fan = Find_MaharakaWaterGunSkillBySlot('Q');
        const auto* bomb = Find_MaharakaWaterGunSkillBySlot('W');
        const auto* single = Find_MaharakaWaterGunSkillBySlot('R');
        tests.Require(fan && bomb && single && fan->iSkillId == 56900u &&
            fan->iAttackClip == 4u && fan->iSpawnMs == 704u && fan->iCooldownMs == 3000u &&
            fan->iProjectileCount == 3u && fan->fSpreadDegrees == 30.f &&
            fan->fSpeedMps == 10.f && fan->fProjectileMaxDistanceM == 3.3f &&
            bomb->fSpeedMps == 10.f && bomb->fProjectileMaxDistanceM == 8.f && bomb->fMaxRangeM == 7.f &&
            single->fSpeedMps == 10.f && single->fProjectileMaxDistanceM == 3.f,
            "Watergun keeps stable input IDs and project cooldown while restoring source fan and projectile units");
        auto invalidRay = runtime.Begin_Transaction();
        tests.Require(!runtime.Stage_WaterGunPresentation(invalidRay,owner,56900u,
            room->m_GameplayCatalog,600u,status,3u) && invalidRay.Objects.empty() && invalidRay.Spawned.empty(),
            "An invalid fan ray is rejected before staging any replicated object");

        // Exercise the actual room authority, not a duplicate trajectory simulation.
        tests.Require(room->Is_Ready(), "Watergun trajectory fixture loads the published Maharaka room");
        if (room->Is_Ready())
        {
            S2C_WORLD_SEQUENCE_PLAY intro;
            intro.eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
            intro.strSequenceInstanceId = MAHARAKA_WATERPANG_INTRO_INSTANCE;
            intro.iStartTick = 1000u;
            room->m_MaharakaWaterpangIntro = intro;
            room->m_iServerTick = 1000u;
            owner.fPositionX = MAHARAKA_WATERPANG_CANNON_X + 4.f;
            owner.fPositionY = 22.4f;
            owner.fPositionZ = MAHARAKA_WATERPANG_CANNON_Z;
            room->m_Players.emplace(owner.iPlayerId, owner);
            auto& shooter = room->m_Players.at(owner.iPlayerId);
            C2S_USE_SKILL command;
            command.iClientSequence = 1u; command.iSkillId = 56900u;
            command.fAimX = shooter.fPositionX - 7.f; command.fAimZ = shooter.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter, command) &&
                room->m_MaharakaWaterGunShots.size() == 3u,
                "One accepted Q queues three independent shots under one cast and cooldown");
            if (room->m_MaharakaWaterGunShots.size() == 3u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                room->Update_MaharakaWaterGunShots(launchTick - 1u);
                tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                    "Q does not emit before its source action notify");
                room->Update_MaharakaWaterGunShots(launchTick);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                bool exactFan = spawned.size() == 3u && room->m_MaharakaWaterGunShots.size() == 3u;
                const std::array<float,3u> yawOffset{0.f,30.f,-30.f};
                std::set<COMBAT_OBJECT_ID> uniqueIds;
                for (std::size_t ray = 0u; exactFan && ray < spawned.size(); ++ray)
                {
                    const auto& message = spawned[ray];
                    const auto& shot = room->m_MaharakaWaterGunShots[ray];
                    uniqueIds.insert(message.iCombatObjectId);
                    exactFan = message.iSpawnTick == launchTick &&
                        std::abs(message.fYawDegrees - shooter.fYawDegrees - yawOffset[ray]) < .001f &&
                        std::abs(message.fPositionX - (shooter.fPositionX - .70f)) < .001f &&
                        std::abs(message.fPositionY - (shooter.fPositionY + .75f)) < .001f &&
                        std::abs(message.fPositionZ - (shooter.fPositionZ + .20f)) < .001f &&
                        std::abs(shot.fTravelM - (10.f / 30.f)) < .001f &&
                        std::abs(shot.fX - message.fPositionX - shot.fDirX * shot.fTravelM) < .001f &&
                        std::abs(shot.fZ - message.fPositionZ - shot.fDirZ * shot.fTravelM) < .001f;
                }
                tests.Require(exactFan && uniqueIds.size() == 3u,
                    "Q fan shares one muzzle but replicates three source yaws and independent authoritative rays");
                for (unsigned step = 1u; step < 10u; ++step)
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.empty() && despawned.size() == 3u,
                    "Q rays expire independently at 3.3m without invented impacts");
            }

            room->m_iServerTick = 2000u;
            command.iClientSequence = 2u; command.iSkillId = 56930u;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter, command) &&
                room->m_MaharakaWaterGunShots.size() == 1u,
                "R retains one basic projectile");
            if (room->m_MaharakaWaterGunShots.size() == 1u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                room->Update_MaharakaWaterGunShots(launchTick);
                tests.Require(std::abs(room->m_MaharakaWaterGunShots.front().fTravelM - 10.f/30.f) < .001f,
                    "R advances at the reflected 10m/s speed");
                for (unsigned step = 1u; step < 9u; ++step)
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && spawned.size() == 1u &&
                    events.empty() && despawned.size() == 1u,
                    "R stops at its independent 3m MaxDistance");
            }

            SERVER_PLAYER target;
            target.iPlayerId = 1003u; target.iNetEntityId = 1004u;
            target.fPositionX = shooter.fPositionX - 3.f;
            target.fPositionY = shooter.fPositionY; target.fPositionZ = shooter.fPositionZ;
            room->m_Players.emplace(target.iPlayerId,target);
            SERVER_PLAYER outside = target;
            outside.iPlayerId = 1005u; outside.iNetEntityId = 1006u; outside.fPositionZ += 2.f;
            room->m_Players.emplace(outside.iPlayerId,outside);
            room->m_iServerTick = 3000u;
            command.iClientSequence = 3u; command.iSkillId = 56910u;
            command.fAimX = target.fPositionX; command.fAimZ = target.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter,command) &&
                room->m_MaharakaWaterGunShots.size() == 1u,"W queues one aimed grenade");
            if (room->m_MaharakaWaterGunShots.size() == 1u)
            {
                const auto launchTick = room->m_MaharakaWaterGunShots.front().iSpawnTick;
                float peakY = shooter.fPositionY;
                for (unsigned step = 0u; step < 8u; ++step)
                {
                    room->Update_MaharakaWaterGunShots(launchTick + step);
                    const auto& live = room->m_CombatObjectRuntime.Get_LiveObjects();
                    if (!live.empty()) peakY = (std::max)(peakY,live.front().LiveState.CurrentPose.fPositionY);
                }
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(events.empty() && despawned.empty() &&
                    room->m_Players.at(target.iPlayerId).fKnockbackRemainingSeconds == 0.f &&
                    peakY > shooter.fPositionY + .6f && peakY <= shooter.fPositionY + .751f,
                    "W follows its bounded source-height arc and cannot hit during flight");
                room->Update_MaharakaWaterGunShots(launchTick + 8u);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.size() == 1u &&
                    despawned.size() == 1u && std::abs(events[0].fPositionX - target.fPositionX) < .001f &&
                    std::abs(events[0].fPositionY - target.fPositionY) < .001f &&
                    room->m_Players.at(target.iPlayerId).fKnockbackRemainingSeconds > 0.f &&
                    room->m_Players.at(target.iPlayerId).iCurrentHp == target.iCurrentHp &&
                    room->m_Players.at(outside.iPlayerId).fKnockbackRemainingSeconds == 0.f,
                    "W bursts once at authoritative ground arrival, preserves HP, and respects its hit radius");
            }

            room->m_Players.erase(outside.iPlayerId);
            auto& overlapping = room->m_Players.at(target.iPlayerId);
            overlapping = target;
            overlapping.fPositionX = shooter.fPositionX - 1.f;
            overlapping.fPositionZ = shooter.fPositionZ + .20f;
            room->m_iServerTick = 4000u;
            command.iClientSequence = 4u; command.iSkillId = 56900u;
            command.fAimX = shooter.fPositionX - 7.f; command.fAimZ = shooter.fPositionZ;
            tests.Require(room->Try_StartMaharakaWaterGunSkill(shooter,command),
                "Q can be cast again after its existing project cooldown");
            if (!room->m_MaharakaWaterGunShots.empty())
            {
                room->Update_MaharakaWaterGunShots(room->m_MaharakaWaterGunShots.front().iSpawnTick);
                room->m_CombatObjectRuntime.Drain_Lifecycle(spawned,events,despawned);
                tests.Require(room->m_MaharakaWaterGunShots.empty() && events.size() == 3u &&
                    despawned.size() == 3u && overlapping.iCurrentHp == target.iCurrentHp &&
                    std::abs(overlapping.fKnockbackSpeed - room->m_MaharakaAITuning.fKnockbackRangeM /
                        (float(room->m_MaharakaAITuning.iKnockbackMs) * .001f)) < .001f,
                    "Each nonpiercing fan ray consumes on its first body; overlapping rays replace rather than triple push velocity");
            }
        }

    }
        {
            auto room = std::make_unique<CGameRoom>(WORLD_ID::MAHARAKA);
            tests.Require(room->Is_Ready(), "Waterpang published room loads");
            room->m_iServerTick=100u;
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                0.f,0.f,0.f,0.f,0u,{}) && !room->m_MaharakaWaterpangIntro,
                "Invalid Waterpang packet cannot consume room reservation");
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro &&
                room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "Waterpang reserves exactly ten seconds on the Server clock");
            room->m_iServerTick=250u;
            tests.Require(room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{}) && room->m_MaharakaWaterpangIntro->iStartTick==400u,
                "A second arena entry cannot reset countdown");
            tests.Require(!room->Broadcast_WorldSequencePlay(MAHARAKA_WATERPANG_INTRO_INSTANCE,
                1.f,0.f,0.f,0.f,0u,{},WORLD_SEQUENCE_OPERATION::REPLAY),
                "Replay cannot restart an active Waterpang reservation");
            auto session=std::make_shared<CClientSession>(99001u,INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{},CClientSession::CLOSED_HANDLER{});
            C2S_ENTER_WORLD enter{}; enter.iProtocolVersion=NETWORK_PROTOCOL_VERSION;
            enter.eWorldId=WORLD_ID::MAHARAKA; enter.eCharacterClass=CHARACTER_CLASS_ID::ARTIST;
            enter.strNickName="WaterpangLateJoin";
            CGameRoom::STAGED_PLAYER_ENTRY admission{}; SESSION_DIAGNOSTIC_REASON reason{}; std::string admissionStatus;
            const bool admitted=room->Stage_PlayerEntry(session,enter,{},admission,reason,admissionStatus) &&
                room->Build_PlayerEntryFrames(admission,std::span<const CGameRoom::STAGED_PLAYER_ENTRY>{&admission,1u},admissionStatus);
            if (!admitted) std::cout << "Waterpang admission diagnostic: " << admissionStatus << '\n';
            bool sameReservation=false;
            for (const auto& frame:admission.Frames)
                if (frame.ePacketType==PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY)
                {
                    CPacketReader reader(frame.Payload); S2C_WORLD_SEQUENCE_PLAY play;
                    if (Read_Message(reader,play) && play.strSequenceInstanceId==MAHARAKA_WATERPANG_INTRO_INSTANCE)
                        sameReservation=play.iStartTick==400u && play.iServerTick==250u;
                }
            tests.Require(admitted && sameReservation,"Late Maharaka admission receives original start and current Server tick");
            tests.Require(room->Reset_ReplayableArenaWhenEmpty() && !room->m_MaharakaWaterpangIntro,
                "An empty Maharaka room releases Waterpang reservation");

            CServerNavigation navigation; SERVER_NAV_POINT point;
            tests.Require(navigation.Load("LV_OCN_EVENTIS_MHP") &&
                navigation.Sample_Position(73.041f,-979.223022f,point) && point.y>22.3f && point.y<22.5f &&
                navigation.Resolve_TraversalStep(73.041f,-979.223022f,73.1f,-979.4f,point,23.1289997f) && point.y>22.3f,
                "Waterpang landing and next walking step remain on mesh-baked stage floor");

            CServerTriggerSystem entry; entry.Set_WorldId(WORLD_ID::MAHARAKA);
            WORLD_BOOTSTRAP_PLACEMENT start{}; start.strPlacementId="waterpang.arena.start";
            start.eKind=WORLD_BOOTSTRAP_KIND::TRIGGER_BOX; start.isEnabled=true;
            start.fHalfExtentX=start.fHalfExtentY=start.fHalfExtentZ=1.f;
            WORLD_TRIGGER_ACTION action{}; action.eKind=WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
            action.strTargetId=MAHARAKA_WATERPANG_INTRO_INSTANCE; start.TriggerActions.push_back(action);
            std::string status; tests.Require(entry.Initialize({start},status),"Waterpang landing trigger initializes");
            std::map<PLAYER_ID,SERVER_PLAYER> players;
            auto& player=players[1u]; player.iPlayerId=1u; player.iCurrentHp=player.iMaximumHp=100u;
            player.TriggerMove.isActive=true;
            std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers; std::vector<SERVER_INTERACT_PROMPT_EDGE> edges;
            int fired=0; const auto activate=[&](WORLD_TRIGGER_ACTION_KIND,const std::string&){++fired;return true;};
            entry.Evaluate_Entries(players,1u,transfers,activate,edges);
            tests.Require(fired==0,"Flying through arena does not start countdown");
            player.TriggerMove.isActive=false;
            entry.Evaluate_Entries(players,2u,transfers,activate,edges);
            tests.Require(fired==1,"Landing inside arena starts countdown without an extra re-entry");
            entry.Evaluate_Entries(players,3u,transfers,activate,edges);
            tests.Require(fired==1,"Standing on arena does not repeatedly start countdown");
            CWorldBootstrap authored;
            tests.Require(authored.Load(WORLD_ID::MAHARAKA),"Published Waterpang G jumps load");
            for (const char* name:{"jump1","jump2","jump3"})
            {
                const auto box=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [&](const auto& row){return row.strPlacementId==name;});
                if (box==authored.Get_Placements().end()) { tests.Require(false,"Waterpang jump source missing"); continue; }
                const auto destination=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [&](const auto& row){return row.strPlacementId==std::string(name)+"_1";});
                const bool hasDestination=destination!=authored.Get_Placements().end() && box->TriggerActions.size()==1u;
                tests.Require(hasDestination,"Every published Waterpang jump has its saved landing marker and one action");
                if (!hasDestination) continue;
                const auto& move=box->TriggerActions.front();
                tests.Require(box->isEnabled && box->requiresInteract && !box->isTriggerOnce &&
                    !destination->isEnabled && destination->TriggerActions.empty() &&
                    move.eKind==WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER &&
                    move.fTargetX==destination->fPositionX && move.fTargetY==destination->fPositionY &&
                    move.fTargetZ==destination->fPositionZ,
                    "G jump consumes the current saved marker, never a stale copied destination");
                bool walkable=navigation.Sample_Position(move.fTargetX,move.fTargetZ,point,move.fTargetY) &&
                    point.y>22.3f && point.y<22.5f;
                for (int dx=-1;dx<=1;++dx) for (int dz=-1;dz<=1;++dz)
                    if (dx || dz) walkable=navigation.Resolve_TraversalStep(move.fTargetX,move.fTargetZ,
                        move.fTargetX+.25f*dx,move.fTargetZ+.25f*dz,point,move.fTargetY) &&
                        point.y>22.3f && point.y<22.5f && walkable;
                tests.Require(walkable,"Each saved landing has deck support and eight valid first walking steps");
                CServerTriggerSystem jump; jump.Set_WorldId(WORLD_ID::MAHARAKA);
                tests.Require(jump.Initialize({*box},status),"Waterpang authored jump initializes");
                player.fPositionX=box->fPositionX; player.fPositionY=box->fPositionY; player.fPositionZ=box->fPositionZ;
                jump.Evaluate_Entries(players,10u,transfers,activate,edges);
                tests.Require(!player.TriggerMove.isActive && !edges.empty(),"Waterpang jump offers G without automatic movement");
                const auto activated=jump.Activate_Here(1u,players,11u,transfers,activate);
                const auto target=player.TriggerMove;
                for (unsigned tick=0;tick<40;++tick) jump.Update_PlayerMotion(player,1.f/30.f);
                tests.Require(activated==1u && !player.TriggerMove.isActive &&
                    std::abs(player.fPositionX-target.fTargetX)<.001f &&
                    std::abs(player.fPositionY-target.fTargetY)<.001f &&
                    std::abs(player.fPositionZ-target.fTargetZ)<.001f,
                    "Waterpang G travels to the exact authored destination");
                const auto arena=std::find_if(authored.Get_Placements().begin(),authored.Get_Placements().end(),
                    [](const auto& row){return row.strPlacementId=="waterpang.arena.start";});
                tests.Require(arena!=authored.Get_Placements().end(),"Published arena start trigger exists");
                if (arena!=authored.Get_Placements().end())
                {
                    CServerTriggerSystem arrival; arrival.Set_WorldId(WORLD_ID::MAHARAKA);
                    tests.Require(arrival.Initialize({*arena},status),"Published arena start trigger initializes");
                    fired=0;
                    arrival.Evaluate_Entries(players,60u,transfers,activate,edges);
                    arrival.Evaluate_Entries(players,61u,transfers,activate,edges);
                    tests.Require(fired==1,"Every authored jump landing activates the arena countdown exactly once");
                }
            }
        }
		CWorldBootstrap bootstrap;
		tests.Require(bootstrap.Load(WORLD_ID::KAKULSAYDON_ARENA) && !bootstrap.Get_SequenceInstanceIds().empty(),
			"Viewer loads published Kouku sequence IDs with the world");
		CWorldBootstrap valtanOnly;
		tests.Require(valtanOnly.Load(WORLD_ID::VALTAN_ARENA) && bootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			bootstrap.Get_SequenceInstanceIds() == valtanOnly.Get_SequenceInstanceIds(),
			"Viewer switching to Valtan replaces previous IDs with its published sequences");
		CServerTriggerSystem triggers;
		triggers.Set_HonourTriggerOnce(true);
		WORLD_BOOTSTRAP_PLACEMENT box{};
		box.strPlacementId = "viewer.trigger"; box.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		box.fHalfExtentX = box.fHalfExtentY = box.fHalfExtentZ = 1.f;
		box.isTriggerOnce = true; box.requiresInteract = true;
		WORLD_TRIGGER_ACTION action{}; action.eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
		action.strTargetId = "viewer.sequence"; box.TriggerActions.push_back(action);
		std::string status;
		tests.Require(triggers.Initialize({ box }, status), "Viewer test initializes the real trigger system");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players[1u].iPlayerId = 1u; players[1u].iCurrentHp = players[1u].iMaximumHp = 100u;
		players[1u].fPositionX = 50.f;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		int fired = 0;
		const auto activate = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{ if (kind != WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE || id != "viewer.sequence") return false; ++fired; return true; };
		using R = DEBUG_WORLD_PLAYBACK_RESULT;
		tests.Require(triggers.Debug_Activate(2u, box.strPlacementId, false, players, 1u, transfers, activate) ==
#ifdef _DEBUG
			R::INVALID_PLAYER,
#else
			R::DISABLED,
#endif
			"Viewer rejects missing player without activating a trigger");
#ifdef _DEBUG
		tests.Require(triggers.Debug_Activate(1u, "missing", false, players, 1u, transfers, activate) == R::INVALID_TARGET && fired == 0,
			"Viewer rejects unknown targets without effects");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 1u, transfers, activate) == R::ACCEPTED && fired == 1,
			"Debug viewer uses the authored action outside the G-key box");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, false, players, 2u, transfers, activate) == R::ALREADY_USED && fired == 1,
			"Play preserves the one-shot latch");
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 3u, transfers, activate) == R::ACCEPTED && fired == 2,
			"Replay reuses the same authored action");
		players[1u].iCurrentHp = 0;
		tests.Require(triggers.Debug_Activate(1u, box.strPlacementId, true, players, 4u, transfers, activate) == R::INVALID_PLAYER && fired == 2,
			"Dead viewer cannot activate world actions");
		players[1u].iCurrentHp = 100;
		const auto reject = [](WORLD_TRIGGER_ACTION_KIND, const std::string&) { return false; };
		tests.Require(triggers.Initialize({ box }, status) &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 5u, transfers, reject) == R::ACTION_REJECTED &&
			triggers.Debug_Activate(1u, box.strPlacementId, false, players, 6u, transfers, activate) == R::ACCEPTED,
			"Failed action does not consume the one-shot trigger");
#endif
		{
			// Exercise the real broadcast boundary: stale bootstrap rows cannot revive
			// the old actor or move the party before the Client rejects the cue.
			auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
			auto& player = room->m_Players[1u];
			player.iPlayerId = 1u;
			player.iCurrentHp = player.iMaximumHp = 100u;
			bool allPreserve = room->Is_Ready();
			bool admissionMatches = room->Is_Ready();
			const WORLD_SEQUENCE_OPERATION operations[] = { WORLD_SEQUENCE_OPERATION::PLAY,
				WORLD_SEQUENCE_OPERATION::REPLAY, WORLD_SEQUENCE_OPERATION::STOP,
				WORLD_SEQUENCE_OPERATION::PLAY, WORLD_SEQUENCE_OPERATION::PLAY };
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				player.fPositionX = 12.f; player.fPositionY = 34.f; player.fPositionZ = 56.f;
				player.hasMoveGoal = true; player.TriggerMove.isActive = true;
				player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
				player.iMarioStage = 1u; player.ePreMarioForm = PLAYER_MADNESS_FORM::NORMAL;
				player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
				const bool accepted = room->Broadcast_WorldSequencePlay(
					scenario == 4u ? "world.sequence.instance.contract_ordinary" : "world.sequence.instance.original_kouku",
					1.f, 0.f, 0.f, 0.f, 0u, scenario == 3u ? "world.existing.target" : "", operations[scenario]);
				admissionMatches = admissionMatches && accepted == (scenario == 2u || scenario == 4u);
				allPreserve = allPreserve &&
					player.fPositionX == 12.f && player.fPositionY == 34.f && player.fPositionZ == 56.f &&
					player.hasMoveGoal && player.TriggerMove.isActive && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
					player.iMarioStage == 1u && player.eMadnessForm == PLAYER_MADNESS_FORM::CLOWN && player.iCurrentHp == 100u;
			}
			tests.Require(admissionMatches, "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted");
			tests.Require(allPreserve, "Legacy rejection and ordinary sequence cues preserve all player movement and form state");
		}
		{
			C2S_DEBUG_WORLD_PLAYBACK request{};
			request.iRequestSequence = 17u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
			request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
			request.strOccurrenceId = request.strTargetId + ".logic.51";
			request.iRunEpoch = 9u; request.iRoomPlayerSlot = 3u;
			request.fPositionX = -1.156042f; request.fPositionY = 1.3176255f; request.fPositionZ = 742.512031f;
			CPacketWriter writer;
			tests.Require(Write_Message(writer, request), "Arrival command encodes its run, occurrence, slot and destination");
			CPacketReader reader(writer.Get_Buffer()); C2S_DEBUG_WORLD_PLAYBACK decoded;
			tests.Require(Read_Message(reader, decoded) && reader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 9u && decoded.iRoomPlayerSlot == 3u && decoded.strOccurrenceId == request.strOccurrenceId &&
				decoded.fPositionX == request.fPositionX && decoded.fPositionY == request.fPositionY && decoded.fPositionZ == request.fPositionZ,
				"Arrival packet preserves exact slot coordinates and replay identity");
			auto bytes = writer.Get_Buffer(); bytes.pop_back();
			CPacketReader truncated(bytes); decoded.strOccurrenceId = "sentinel";
			tests.Require(!Read_Message(truncated, decoded) && decoded.strOccurrenceId == "sentinel",
				"Truncated arrival packet does not partially commit decoded intent");
			bool rejects = true;
			for (unsigned scenario = 0u; scenario < 5u; ++scenario)
			{
				auto bad = request;
				if (scenario == 0u) bad.iRoomPlayerSlot = 4u;
				if (scenario == 1u) bad.iRunEpoch = 0u;
				if (scenario == 2u) bad.fPositionX = std::numeric_limits<float>::quiet_NaN();
				if (scenario == 3u) bad.eWorldId = WORLD_ID::BERN;
				if (scenario == 4u) bad.fPositionZ = 100001.f;
				CPacketWriter invalid; rejects = rejects && !Write_Message(invalid, bad) && invalid.Get_Buffer().empty();
			}
			tests.Require(rejects, "Arrival rejects wrong world, invalid epoch, slot and coordinates before writing");
			S2C_DEBUG_WORLD_PLAYBACK_RESULT receipt{};
			receipt.iRequestSequence = request.iRequestSequence; receipt.eWorldId = request.eWorldId;
			receipt.eOperation = request.eOperation; receipt.strTargetId = request.strTargetId;
			receipt.eResult = R::SKIPPED_PLAYER;
			CPacketWriter replyWriter; const bool wroteReply = Write_Message(replyWriter, receipt);
			CPacketReader replyReader(replyWriter.Get_Buffer()); S2C_DEBUG_WORLD_PLAYBACK_RESULT reply;
			tests.Require(wroteReply && Read_Message(replyReader, reply) && replyReader.Get_RemainingSize() == 0u &&
				reply.iRequestSequence == 17u && reply.eOperation == request.eOperation && reply.eResult == R::SKIPPED_PLAYER,
				"Arrival skip reply uses the existing request-correlated result envelope");
			request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLAY_SEQUENCE;
			CPacketWriter legacyWriter; const bool wroteLegacy = Write_Message(legacyWriter, request);
			CPacketReader legacyReader(legacyWriter.Get_Buffer());
			tests.Require(wroteLegacy && Read_Message(legacyReader, decoded) && legacyReader.Get_RemainingSize() == 0u &&
				decoded.iRunEpoch == 0u && decoded.strOccurrenceId.empty(), "Ordinary world playback keeps its original payload shape");
		}
#ifdef _DEBUG
		{
			WSADATA winsock{};
			const bool socketReady = WSAStartup(MAKEWORD(2, 2), &winsock) == 0;
			tests.Require(socketReady, "Arrival fixture prepares unconnected session sockets without a listener");
			if (socketReady)
			{
				const std::array<std::array<float, 2>, 4> locations{{ {-3.913588f,739.883125f},
					{-3.290587f,742.070391f}, {-5.324063f,738.528281f}, {-1.156042f,742.512031f} }};
				for (unsigned count = 1u; count <= 4u; ++count)
				{
					auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
					std::vector<std::shared_ptr<CClientSession>> sessions;
					const auto join = [&](PLAYER_ID id)
					{
						const SESSION_ID sessionId = id + 1000u;
						auto connection = std::make_shared<CClientSession>(sessionId, ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
							CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
						sessions.push_back(connection); room->m_Sessions[sessionId] = connection;
						room->m_PlayerIdBySessionId[sessionId] = id;
						auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = sessionId;
						player.iNetEntityId = id + 100u; player.iCurrentHp = player.iMaximumHp = 100u;
						player.fPositionX = -100.f - static_cast<float>(id); player.fPositionY = 1.3f; player.fPositionZ = 740.f;
						player.hasMoveGoal = true; player.iCurrentSkillId = 34010u; player.eAction = PLAYER_ACTION_STATE::SKILL;
					};
					// Reverse insertion proves that stable PlayerId order, not joins or session order, chooses slots.
					for (unsigned n = count; n > 0u; --n) join(n * 10u);
					if (count == 4u) join(50u);
					bool nativeGround = room->Is_Ready();
					std::array<SERVER_NAV_POINT, 4> ground{};
					for (unsigned slot = 0u; slot < 4u; ++slot)
						nativeGround = nativeGround && room->m_ServerNavigation.Sample_Position(locations[slot][0], locations[slot][1], ground[slot]);
					tests.Require(nativeGround, "Arrival samples all four authored fireworks XZ on actual Server navigation");
					if (!nativeGround) continue;
					C2S_DEBUG_WORLD_PLAYBACK request{};
					request.iRequestSequence = 1u; request.iRunEpoch = 1u; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					request.eOperation = DEBUG_WORLD_PLAYBACK_OPERATION::PLACE_ROOM_PLAYER;
					request.strTargetId = "KAKULSAYDON_G1_PATTERN_4";
					const auto position = [&](unsigned slot)
					{
						request.iRoomPlayerSlot = static_cast<std::uint8_t>(slot);
						request.strOccurrenceId = request.strTargetId + ".logic." + std::to_string(slot + 1u);
						request.fPositionX = ground[slot].x; request.fPositionY = ground[slot].y; request.fPositionZ = ground[slot].z;
					};
					bool movedInOrder = true;
					for (unsigned slot = 0u; slot < 4u; ++slot)
					{
						position(slot); const auto verdict = room->Apply_DebugRoomPlayerArrival(1010u, request);
						movedInOrder = movedInOrder && verdict == (slot < count ? R::ACCEPTED : R::SKIPPED_PLAYER);
						if (slot < count)
						{
							const auto& player = room->m_Players.at((slot + 1u) * 10u);
							movedInOrder = movedInOrder && player.fPositionX == ground[slot].x && player.fPositionY == ground[slot].y &&
								player.fPositionZ == ground[slot].z && !player.hasMoveGoal && player.iCurrentSkillId == INVALID_SKILL_ID && player.iCurrentHp == 100u;
						}
					}
					tests.Require(movedInOrder, "One through four connected players arrive by PlayerId; missing slots are successful skips");
					if (count == 4u)
						tests.Require(room->m_RoomPlayerArrivalRuns.at(1010u).Players.size() == 4u && room->m_Players.at(50u).hasMoveGoal,
							"Arrival roster caps at four and preserves any later connected player");
					position(0u); auto& first = room->m_Players.at(10u); first.fPositionX -= 20.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ALREADY_USED && first.fPositionX == ground[0].x - 20.f,
						"Duplicate arrival occurrence never teleports an already consumed slot twice");
					request.iRunEpoch = 2u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACCEPTED && first.fPositionX == ground[0].x,
						"Explicit new playback epoch permits the same occurrence again");
					first.fPositionX -= 20.f; first.hasMoveGoal = true; first.iCurrentSkillId = 34010u;
					request.iRunEpoch = 1u;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::STALE_REQUEST && first.hasMoveGoal && first.iCurrentSkillId == 34010u,
						"An older playback cannot mutate the current run");
					request.iRunEpoch = 2u; request.strOccurrenceId += ".wrongheight"; request.fPositionY += 100.f;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::ACTION_REJECTED && first.fPositionX == ground[0].x - 20.f &&
						first.hasMoveGoal && first.iCurrentSkillId == 34010u && first.iCurrentHp == 100u,
						"Rejected destination preserves position, action, movement and health");
					request.eWorldId = WORLD_ID::BERN;
					tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::WRONG_WORLD && first.hasMoveGoal,
						"Arrival cannot cross the requesting room's world boundary");
					request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
					if (count > 1u)
					{
						// Epoch 2 already captured PlayerId 20; replacing its room binding cannot retarget that slot.
						room->m_PlayerIdBySessionId.erase(1020u); room->m_Players.erase(20u); join(21u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(21u).hasMoveGoal,
							"Departed roster member is skipped without teleporting its newly joined replacement");
					}
					else
					{
						join(20u); position(1u);
						tests.Require(room->Apply_DebugRoomPlayerArrival(1010u, request) == R::SKIPPED_PLAYER && room->m_Players.at(20u).hasMoveGoal,
							"Joining midway does not fill a slot absent from the playback's fixed roster");
					}
				}
				WSACleanup();
			}
		}
#endif
		std::cout << "World playback contract failures: " << tests.failures << '\n';
		return tests.failures == 0 ? 0 : 1;
	}
```
