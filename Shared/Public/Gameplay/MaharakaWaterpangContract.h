#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace LostArk::Shared
{
    // The existing stage instance identifies the entire admitted Waterpang intro.
    // S2C_WORLD_SEQUENCE_PLAY carries its future start on the 30 Hz Server clock.
    inline constexpr const char* MAHARAKA_WATERPANG_INTRO_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.intro15.stage";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TICK_HZ = 30u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COUNTDOWN_TICKS = 300u;
    // Navigation detail region that bakes the arena deck and its jump pier.
    inline constexpr const char* MAHARAKA_WATERPANG_REGION_ID = "WaterpangEntry";

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

    /* Big mokoko waterfall, arena action 4225612 (6 s clip att_battle_1_01, the
       Waterpang variant of 4225601; decal SkillDecal 1106 for 2.3 s). The source
       hits a 180 degree half disc 3.2 m ahead of the mokoko in two waves (inner
       1.2-2.5 m at 2.30 s, outer 2.5-5.9 m at 2.46 s) and launches 580-600 cm over
       1814-1834 ms at 200 cm height with FallDown. Project rule: the waves cover
       the whole deck (inner = within 2.5 m of that origin), and every hit body
       is launched away from the arena centre far enough to leave the deck; the
       launch always ends in the Waterpang fall (jump box revive). No damage. */
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

    /* Order, speeds and the 50/50 turn direction follow trigger chain 6000-6009;
       the chain's timers are condition-driven on the Server, so absolute times
       are this project's schedule (waterfalls at the chain's 120 s / 150 s). */
    inline constexpr std::array<MAHARAKA_WATERPANG_EVENT, 12u> MAHARAKA_WATERPANG_SCHEDULE{ {
        { 20u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 65u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 120u, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL, 0u },
        { 127u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 150u, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL, 0u },
        { 157u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 15u },
        { 202u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 20u },
        { 247u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON, 20u },
        { 300u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 15u },
        { 340u, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL, 0u },
        { 347u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 10u },
        { 377u, MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, 10u } } };
    // After the table a fast 10 s turn repeats every 40 s (trigger 1700 cadence).
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_START_SECONDS = 417u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_PERIOD_SECONDS = 40u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TAIL_FIRE_SECONDS = 10u;

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

    inline std::uint32_t Get_MaharakaWaterpangEventTicks(const MAHARAKA_WATERPANG_EVENT& event) noexcept
    {
        return MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind ?
            MAHARAKA_WATERPANG_WATERFALL_TICKS :
            MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + event.iFireSeconds * MAHARAKA_WATERPANG_TICK_HZ +
                MAHARAKA_WATERPANG_CANNON_END_TICKS;
    }

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
            event = { MAHARAKA_WATERPANG_TAIL_START_SECONDS + cycle * MAHARAKA_WATERPANG_TAIL_PERIOD_SECONDS,
                MAHARAKA_WATERPANG_EVENT_KIND::CANNON_FAST, MAHARAKA_WATERPANG_TAIL_FIRE_SECONDS };
            occurrence = static_cast<std::uint32_t>(MAHARAKA_WATERPANG_SCHEDULE.size()) + cycle;
            found = (now - tailStart) % period < Get_MaharakaWaterpangEventTicks(event);
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
}
