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
       GADGET.loa actions 56902 / 56912 / 56932, SkillEffect 569020-569021,
       569120-569121 and 569320-569321, Projectile 569020 / 569120 / 569320. The
       four quick slots are the prop's own list: Q connected shot (56900), W water
       bomb (56910), E speed up (56920) and the prop's default attack (56930) on R.
       Times, ranges, speeds and cooldowns are the source values; a shot leaves
       the muzzle at the action's "Effect" notify. The source hit rows also carry
       HP damage and cold stacks (buff 569302, 8 stacks kill); neither is applied
       because the extracted data does not give the Waterpang stat adjustment they
       need, so a hit only pushes and staggers the body, as the source row's push
       fields say. Casting never locks movement (prop MoveSkillEnable = 1). */
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
        float fLifeSeconds;           // Projectile life (missile reach = speed * life)
        float fHitRadiusM;            // hit SkillEffect AreaRange
        float fPushRangeM;            // hit SkillEffect push range
        std::uint32_t iPushMs;        // hit SkillEffect push time
        std::uint32_t iBuffMs;        // SPEED_BUFF duration (SkillBuff 569200)
        float fBuffSpeedScale;        // SPEED_BUFF move speed factor (+30 %)
    };

    inline constexpr std::array<MAHARAKA_WATERGUN_SKILL, 4u> MAHARAKA_WATERGUN_SKILLS{ {
        { 56900u, 'Q', MAHARAKA_WATERGUN_KIND::MISSILE, 3000u, 1800u, 694u, 2u, 7.f, 4.2f, 1.5f, 1.f, 0.18f, 5u, 0u, 1.f },
        { 56910u, 'W', MAHARAKA_WATERGUN_KIND::GRENADE, 8000u, 1000u, 200u, 5u, 7.f, 8.f, 6.f, 1.05f, 0.2f, 200u, 0u, 1.f },
        { 56920u, 'E', MAHARAKA_WATERGUN_KIND::SPEED_BUFF, 7000u, 0u, 0u, 0u, 0.f, 0.f, 0.f, 0.f, 0.f, 0u, 5000u, 1.3f },
        { 56930u, 'R', MAHARAKA_WATERGUN_KIND::MISSILE, 0u, 1000u, 402u, 1u, 4.f, 3.f, 1.5f, 1.f, 0.18f, 5u, 0u, 1.f } } };

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
