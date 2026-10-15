#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace LostArk::Shared
{
    // 승인된 워터팡 입장 연출 전체를 식별하는 기존 stage instance ID다.
    // S2C_WORLD_SEQUENCE_PLAY의 미래 시작 tick을 30Hz Server 시계로 해석한다.
    inline constexpr const char* MAHARAKA_WATERPANG_INTRO_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.intro15.stage";
    // Server의 AI 생성과 Client의 Level 진입 preload가 같은 NPC 정의 목록을 소비한다.
    inline constexpr std::array<const char*, 8> MAHARAKA_WATERPANG_AI_NPCS = {
        "NPC_MHP_RESIDENT_8FC2DB56F0AA5175", "NPC_MHP_RESIDENT_7D37CE489AF57466",
        "NPC_MHP_RESIDENT_6C0BF3F0C656EBAB", "NPC_MHP_RESIDENT_48E52BAC20260E4C",
        "NPC_BEDA", "NPC_AYLARA", "NPC_FORMAN", "NPC_SCHMIDT" };
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_TICK_HZ = 30u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COUNTDOWN_TICKS = 300u;
    // WaterpangEntry는 경기장과 점프 부두를 포함하는 navigation detail region이다.
    // 섬 전체 grid까지 연결되므로 region 안이라는 조건만으로 경기장 위에 있는지 판정하지 않는다.
    // Server 위험물 판정은 아래 경기장 XZ 범위도 함께 검사한다.
    inline constexpr const char* MAHARAKA_WATERPANG_REGION_ID = "WaterpangEntry";
    // 경기장 XZ의 22m 사각 범위다. 최소값은 포함하고 최대값은 제외한다.
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MIN_X_M = 64.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MAX_X_M = 86.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MIN_Z_M = -995.f;
    inline constexpr float MAHARAKA_WATERPANG_ARENA_MAX_Z_M = -973.f;
    constexpr bool Is_MaharakaWaterpangArenaFootprint(const float x, const float z) noexcept
    {
        return x >= MAHARAKA_WATERPANG_ARENA_MIN_X_M && x < MAHARAKA_WATERPANG_ARENA_MAX_X_M &&
            z >= MAHARAKA_WATERPANG_ARENA_MIN_Z_M && z < MAHARAKA_WATERPANG_ARENA_MAX_Z_M;
    }

    // 배치 대응 기록은 57011 DeployData actor 3(NPC 570911 대포), actor 22(NPC 570941 모코코)다.
    // 좌표 상수는 runtime 월드 m, yaw는 degree로 소비한다.
    inline constexpr float MAHARAKA_WATERPANG_CANNON_X = 75.044f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_Z = -984.307f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_YAW_DEGREES = 223.59f;
    inline constexpr float MAHARAKA_WATERPANG_MOKOMOKO_X = 81.836f;
    inline constexpr float MAHARAKA_WATERPANG_MOKOMOKO_Z = -991.514f;

    // 회전 대포는 길이 15m·폭 0.7m의 중심 BOX로 양쪽 분사를 판정한다. 12tick=0.4초마다 재피격할 수 있다.
    // 밀침은 5.1m/545ms, 예고는 45tick=1.5초다. 한 회전은 일반 19.2초, 빠름 9.5초다.
    // 기존 원본 대응 기록: SkillEffect 422560330, action 4225613~16. 현재 상수의 단위와 원본 cm를 혼용하지 않는다.
    inline constexpr float MAHARAKA_WATERPANG_CANNON_HALF_LENGTH_M = 7.5f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_HALF_WIDTH_M = 0.35f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_HIT_INTERVAL_TICKS = 12u;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_PUSH_M = 5.1f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_PUSH_MS = 545u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS = 45u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_CANNON_END_TICKS = 30u;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_SECONDS_PER_TURN = 19.2f;
    inline constexpr float MAHARAKA_WATERPANG_CANNON_FAST_SECONDS_PER_TURN = 9.5f;

    // 경기장 deck의 Server 판정 반경은 7.5m, 높이 하한은 21.5m다.
    // 기존 navigation 측정 기록은 대포 중심 반경 약 7.45m, deck y=22.3~22.43m, 주변 물 y=20.48m다.
    inline constexpr float MAHARAKA_WATERPANG_DECK_RADIUS_M = 7.5f;
    inline constexpr float MAHARAKA_WATERPANG_DECK_MIN_Y_M = 21.5f;
    // 착지 방향이 부두 방향의 이 반각 안이면 Server가 방향을 돌려 부두를 피하도록 계산한다.
    inline constexpr float MAHARAKA_WATERPANG_PIER_HALF_ANGLE_DEGREES = 12.f;

    // jump1~3 트리거로 이동해 착지한 플레이어의 보호 시간이다. 30Hz에서 30tick=1초다.
    // 이미 진행 중인 위험물에 착지 직후 밀려나는 것을 방지하며 실제 무적 상태는 Server가 관리한다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_JUMP_INVULNERABLE_TICKS = 1u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr std::array<std::string_view, 3u> MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS{ "jump1", "jump2", "jump3" };

    constexpr bool Is_MaharakaWaterpangJumpTrigger(const std::string_view triggerPlacementId) noexcept
    {
        for (const std::string_view id : MAHARAKA_WATERPANG_JUMP_TRIGGER_IDS)
            if (id == triggerPlacementId)
                return true;
        return false;
    }

    // 큰 모코코 폭포의 원본 대응은 action 4225612, att_battle_1_01, SkillDecal 1106이다.
    // 현재 프로젝트는 대포 중심의 노란 원판만 판정하고 보라색 외곽 블록과 부두는 제외한다.
    // 모코코 전방 3.2m 기준 안쪽 2.5m는 69tick(2.30초), 나머지는 74tick에 판정한다.
    // Server는 피해 없이 바깥쪽으로 띄워 보내며 높이 2m, 1824ms와 deck 밖 착지 조건을 사용한다.
    // 폭포 판정과 Client 예고가 공유하는 노란 원판 반경이다.
    // 기존 배치 측정 기록: BG_OCN_ETC_FLOOR01_SM_LNH_OVR_8EB1D0D03456, scale 0.95/1.1875/0.95,
    // 윗면 반경 약 4.90m·최대 정점 반경 5.01m, 외곽 블록 중심 반경 6.15m. 이 정비에서 재측정한 값은 아니다.
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_HIT_RADIUS_M = 5.f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_TICKS = 180u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_HIT_TICK = 69u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_SECOND_HIT_TICK = 74u;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_ORIGIN_FORWARD_M = 3.2f;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_INNER_WAVE_M = 2.5f;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_LAUNCH_M = 5.9f;
    // Server의 폭포 착지 계산이 대포 중심에서 확보하려는 최소 거리다. deck 반경에 여유를 더한다.
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_EXIT_RADIUS_M = 9.f;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_WATERFALL_LAUNCH_MS = 1824u;
    inline constexpr float MAHARAKA_WATERPANG_WATERFALL_LAUNCH_HEIGHT_M = 2.f;

    enum class MAHARAKA_WATERPANG_EVENT_KIND : std::uint8_t
    {
        CANNON,      // 일반 대포 안내 GameMsg se_announce_52
        CANNON_FAST, // 빠른 대포 안내 se_announce_53
        WATERFALL    // 폭포 안내 se_announce_51
    };

    struct MAHARAKA_WATERPANG_EVENT final
    {
        std::uint32_t iStartSeconds;
        MAHARAKA_WATERPANG_EVENT_KIND eKind;
        std::uint32_t iFireSeconds; // 대포의 실제 분사 기간(초)만 사용한다.
    };

    // 예고 시작부터 종료까지의 tick 수다. 대포는 예고+분사+종료, 폭포는 고정 기간을 사용한다.
    constexpr std::uint32_t Get_MaharakaWaterpangEventTicks(const MAHARAKA_WATERPANG_EVENT& event) noexcept
    {
        return MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind ?
            MAHARAKA_WATERPANG_WATERFALL_TICKS :
            MAHARAKA_WATERPANG_CANNON_TELEGRAPH_TICKS + event.iFireSeconds * MAHARAKA_WATERPANG_TICK_HZ +
                MAHARAKA_WATERPANG_CANNON_END_TICKS;
    }

    // 프로젝트 일정은 대포와 폭포를 번갈아 배치한다. 각 이벤트 종료 뒤 이 간격을 더한다.
    // 다음 시작을 초 단위로 올림하므로 실제 빈 간격은 설정값보다 1초 미만 더 길 수 있다. 원본 조건 타이머의 재현값은 아니다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_EVENT_GAP_SECONDS = 8u;
    // 첫 위험물 전에는 countdown과 입장 연출이 진행된다. 시간 기준은 입장 sequence 시작이다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS = 20u;
    // 실제 경기는 첫 위험물부터 180초다. 입장 시작에서 보면 종료 시각은 200초다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_MATCH_SECONDS = 180u;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_MATCH_END_TICKS =
        (MAHARAKA_WATERPANG_FIRST_EVENT_SECONDS + MAHARAKA_WATERPANG_MATCH_SECONDS) * MAHARAKA_WATERPANG_TICK_HZ;
    // 외곽 붕괴 연출은 기존 SCENE03B Matinee42/Data157의 5초 track을 재사용하는 instance다.
    // 남은 60초는 countdown을 포함한 시간이 아니라 실제 경기 시간 기준이다.
    inline constexpr const char* MAHARAKA_WATERPANG_COLLAPSE_INSTANCE =
        "world.sequence.instance.maharaka.waterpang.source.collapse";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_START_TICKS =
        MAHARAKA_WATERPANG_MATCH_END_TICKS - 60u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_DURATION_MS = 5000u;
    // 붕괴 연출 시작 2초 뒤부터 Server가 외곽 바닥 지지를 제거한다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS =
        MAHARAKA_WATERPANG_COLLAPSE_START_TICKS + 2u * MAHARAKA_WATERPANG_TICK_HZ;
    inline constexpr float MAHARAKA_WATERPANG_COLLAPSED_LANDING_RADIUS_M = 4.4f;
    // 경과 tick의 문턱만 판정한다. 위치·현재 입장 여부는 검사하지 않는다.
    constexpr bool Has_MaharakaWaterpangCollapsedFloor(const std::int32_t elapsedTicks) noexcept
    {
        return elapsedTicks >= static_cast<std::int32_t>(MAHARAKA_WATERPANG_COLLAPSE_SUPPORT_TICKS);
    }
    // 붕괴 뒤 중심 기준 5m 초과~7.5m 이하의 고리를 검사한다. XZ 거리 제곱만 쓰며 Y는 호출자 책임이다.
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

    // 이전 이벤트 끝+간격을 tick으로 합산하고 다음 정수 초로 올림한다. 이 프로젝트의 작은 일정 상수를 전제로 한다.
    constexpr std::uint32_t Get_MaharakaWaterpangNextStartSeconds(
        const std::uint32_t startSeconds, const MAHARAKA_WATERPANG_EVENT& event) noexcept
    {
        const std::uint32_t idleUntilTicks = startSeconds * MAHARAKA_WATERPANG_TICK_HZ +
            Get_MaharakaWaterpangEventTicks(event) + MAHARAKA_WATERPANG_EVENT_GAP_SECONDS * MAHARAKA_WATERPANG_TICK_HZ;
        return (idleUntilTicks + MAHARAKA_WATERPANG_TICK_HZ - 1u) / MAHARAKA_WATERPANG_TICK_HZ;
    }

    // 기존 원본 대응은 trigger 6000~6009다. 일반 분사 15초 4회·20초 2회, 빠른 분사 15초·10초·10초다.
    // 현재 절대 시작 시각은 아래 프로젝트 일정에서 만들고 대포마다 폭포 한 번을 끼운다.
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
        std::uint32_t iTailStartSeconds = 0u; // 마지막 행과 대기 간격이 끝난 뒤 반복 구간이 시작하는 초
    };

    // 9개 대포 정의를 복사해 시작 초를 채우고 각각의 뒤에 폭포를 넣어 18행을 만든다. 런타임 진행 상태가 아닌 상수 계획이다.
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

    // 18행 뒤에는 빠른 대포 10초와 폭포를 같은 간격 규칙으로 반복한다.
    // 현재 경기 종료는 이 샘플 함수 밖의 Room이 결정하므로 반복 정의 자체는 종료 tick에서 멈추지 않는다.
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
        std::uint32_t iOccurrence = 0u;   // 처음에는 일정 행 번호, 이후에는 반복 구간의 연속 발생 번호
        std::uint32_t iElapsedTicks = 0u; // 이 이벤트의 예고 시작부터 지난 tick
        std::uint32_t iDurationTicks = 0u;
        bool bClockwise = true;
        bool bFiring = false;             // 대포의 실제 분사가 진행 중인지
        float fCannonYawDegrees = 0.f;    // 한쪽 분사 방향의 degree이며 반대쪽은 +180도
    };

    // 이벤트 내부 경과 tick으로 출력 sample 전체를 덮어쓴다. 기간 안인지의 검사는 호출자가 한다.
    // 회전 방향은 전달된 시작 tick과 발생 번호를 같은 식으로 혼합해 양쪽에서 재계산한다. 정규 일정은 경기 시작 tick, 강제 이벤트는 해당 시작 tick을 전달한다. 난수 상태나 별도 방향 패킷은 저장하지 않는다.
    inline bool Complete_MaharakaWaterpangSample(const MAHARAKA_WATERPANG_EVENT& event,
        const std::uint32_t occurrence, const std::uint32_t elapsedTicks, const std::uint32_t hashTick,
        MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        out = {};
        out.eKind = event.eKind;
        out.iOccurrence = occurrence;
        out.iElapsedTicks = elapsedTicks;
        out.iDurationTicks = Get_MaharakaWaterpangEventTicks(event);
        if (MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == event.eKind)
            return true;
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
        // 분사선은 대포 yaw에 90도를 더한 기준이다. 원본 Loop_800/FX_Turn WaterBomb의 축과 Client root 보정을 맞춘 값이다.
        // 실제 분사한 초/한 회전의 초에 360도를 곱하고 회전 방향 부호를 적용한다.
        const float turned = 360.f * fired / period;
        out.fCannonYawDegrees = MAHARAKA_WATERPANG_CANNON_YAW_DEGREES + 90.f + (out.bClockwise ? turned : -turned);
        return true;
    }

    // Server 충돌과 Client 연출이 같은 입장 경과 tick으로 현재 이벤트를 구한다.
    // 18행 선형 탐색 후 반복 구간에서는 몫으로 cycle, 나머지로 cycle 내부 시각을 찾는다.
    // 음수나 이벤트 사이 빈 구간이면 false이고 out은 보존한다. 경기 종료 여부는 호출자가 별도로 판단한다.
    inline bool Sample_MaharakaWaterpangEvent(const std::int32_t ticksSinceStart,
        const std::uint32_t matchStartTick, MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        if (ticksSinceStart < 0)
            return false;
        const auto now = static_cast<std::uint32_t>(ticksSinceStart);
        MAHARAKA_WATERPANG_EVENT event{};
        std::uint32_t occurrence = 0u;
        bool found = false;
        for (; occurrence < MAHARAKA_WATERPANG_SCHEDULE.size(); ++occurrence)
        {
            const auto& row = MAHARAKA_WATERPANG_SCHEDULE[occurrence];
            const std::uint32_t start = row.iStartSeconds * MAHARAKA_WATERPANG_TICK_HZ;
            if (now >= start && now - start < Get_MaharakaWaterpangEventTicks(row))
            {
                event = row;
                found = true;
                break;
            }
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
        if (!found)
            return false;
        return Complete_MaharakaWaterpangSample(event, occurrence,
            now - event.iStartSeconds * MAHARAKA_WATERPANG_TICK_HZ, matchStartTick, out);
    }

    // Debug Server가 강제 이벤트 ID와 시작 tick을 broadcast한다. 실행 중에는 Server와 Client 모두 정규 일정 대신 이것을 샘플링한다.
    // 끝나면 원래 경기 시계의 일정으로 복귀하며 정규 일정의 시계를 정지시키지 않는다.
    inline constexpr const char* MAHARAKA_WATERPANG_DEBUG_WATERFALL_INSTANCE =
        "maharaka.waterpang.debug.waterfall";
    inline constexpr const char* MAHARAKA_WATERPANG_DEBUG_CANNON_INSTANCE =
        "maharaka.waterpang.debug.cannon";
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_CANNON_FIRE_SECONDS = 15u;
    // 전달 여유를 위해 6tick 뒤에 시작한다. 모든 네트워크 지연을 흡수한다는 보장은 아니다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_LEAD_TICKS = 6u;
    // 강제 이벤트 번호의 상위 bit를 설정해 정상 경기 범위의 일정 번호와 구분한다.
    inline constexpr std::uint32_t MAHARAKA_WATERPANG_DEBUG_OCCURRENCE_BASE = 0x80000000u;

    // 알려진 강제 instance ID 두 개만 허용한다. 실패하면 출력 kind를 그대로 둔다.
    inline bool Find_MaharakaWaterpangDebugKind(const std::string_view instanceId,
        MAHARAKA_WATERPANG_EVENT_KIND& out) noexcept
    {
        if (instanceId == MAHARAKA_WATERPANG_DEBUG_WATERFALL_INSTANCE)
        {
            out = MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL;
            return true;
        }
        if (instanceId == MAHARAKA_WATERPANG_DEBUG_CANNON_INSTANCE)
        {
            out = MAHARAKA_WATERPANG_EVENT_KIND::CANNON;
            return true;
        }
        return false;
    }

    // caller가 확인한 kind로 상대 시작 0인 정의를 만든다. 이 함수 자체는 enum의 유효성을 검증하지 않는다.
    inline MAHARAKA_WATERPANG_EVENT Make_MaharakaWaterpangDebugEvent(const MAHARAKA_WATERPANG_EVENT_KIND kind) noexcept
    {
        return { 0u, kind, MAHARAKA_WATERPANG_EVENT_KIND::WATERFALL == kind ? 0u : MAHARAKA_WATERPANG_DEBUG_CANNON_FIRE_SECONDS };
    }

    // uint32 tick 차를 int32로 해석해 시작 전/종료 뒤를 거른다. 비교 간격이 signed 반범위 안이라는 전제를 사용한다.
    // false일 때 out은 보존하며 성공하면 Complete 함수가 출력 전체를 교체한다.
    inline bool Sample_MaharakaWaterpangDebugEvent(const MAHARAKA_WATERPANG_EVENT_KIND kind,
        const std::uint32_t startTick, const std::uint32_t nowTick, MAHARAKA_WATERPANG_EVENT_SAMPLE& out) noexcept
    {
        const auto elapsed = static_cast<std::int32_t>(nowTick - startTick);
        const MAHARAKA_WATERPANG_EVENT event = Make_MaharakaWaterpangDebugEvent(kind);
        if (elapsed < 0 || static_cast<std::uint32_t>(elapsed) >= Get_MaharakaWaterpangEventTicks(event))
            return false;
        return Complete_MaharakaWaterpangSample(event,
            MAHARAKA_WATERPANG_DEBUG_OCCURRENCE_BASE | (startTick & 0x7fffffffu),
            static_cast<std::uint32_t>(elapsed), startTick, out);
    }

    // Water Pro MK-1(Prop 15000)의 프로젝트 입력은 Q=56900, W=56910, E=56920, R=56930이다.
    // Q는 stable ID와 3초 cooldown을 유지하면서 MK2 action 57002/projectile 570020의 3방향 분사를 쓴다.
    // W/R의 원본 대응은 action 56912/56932, projectile 569120/569320이다.
    // 아래는 시간·형상·이동 정의이며 실제 승인/발사/밀침은 Room과 CombatObjectRuntime, clip·이펙트는 Client가 소비한다.
    // 원본 HP 피해·냉기 중첩은 현재 Waterpang 경로에 적용하지 않는다.
    enum class MAHARAKA_WATERGUN_KIND : std::uint8_t
    {
        MISSILE,    // 바라보는 방향으로 나가는 직선 투사체
        GRENADE,    // 지면 목표로 던지고 도착 시 터지는 투사체
        SPEED_BUFF  // 투사체 없이 자신에게 적용하는 이동 속도 효과
    };

    struct MAHARAKA_WATERGUN_SKILL final
    {
        std::uint32_t iSkillId;
        char cInputSlot;              // 입력 슬롯 Q/W/E/R
        MAHARAKA_WATERGUN_KIND eKind;
        std::uint32_t iCooldownMs;    // 재사용 대기 ms, 원본 대응 EFTable_Skill.Cooltime
        std::uint32_t iActionMs;      // 다음 물총 시전을 막는 action 기간 ms
        std::uint32_t iSpawnMs;       // 시작 후 발사까지의 ms, 원본 action Effect notify
        std::uint32_t iAttackClip;    // watergun_att_N의 N이며 0이면 해당 clip 없음
        float fMaxRangeM;             // 목표 지정 거리 m, 원본 대응 EFTable_Skill.MaxRange
        float fSpeedMps;              // 투사체 이동 속도 m/s
        float fLifeSeconds;           // 투사체 수명 초, 직선 도달 거리는 MaxDistance로도 제한한다.
        float fHitRadiusM;            // 투사체 피격 반경 m
        float fPushRangeM;            // 정의에 기록된 밀침 거리 m, 최종 밀침 정책은 소비자가 결정한다.
        std::uint32_t iPushMs;        // 정의에 기록된 밀침 기간 ms
        std::uint32_t iBuffMs;        // 이동 속도 효과 지속 ms, 원본 SkillBuff 569200
        float fBuffSpeedScale;        // 이동 속도 배율, 1.3은 30% 증가
        float fProjectileMaxDistanceM; // 투사체 최대 이동 거리 m, 속도·목표 지정 거리와 다른 제한이다.
        float fLaunchRightM;           // 발사자의 오른쪽 offset m, 부채꼴의 모든 발사는 같은 총구를 쓴다.
        std::uint32_t iProjectileCount;
        float fSpreadDegrees;          // 동시 발사의 중앙/양수/음수 yaw 간격 degree
    };

    inline constexpr std::array<MAHARAKA_WATERGUN_SKILL, 4u> MAHARAKA_WATERGUN_SKILLS{ {
        // Q의 stable ID와 cooldown은 유지하며 MK2 action 57002/projectile 570020을 사용한다.
        // Att4의 한 발사 시점에서 0/+30/-30도 세 방향이 나간다.
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

    // Room 충돌과 Client 복제가 공유하는 XZ 총구 위치·진행 방향 계산이다. Room 충돌의 Y와 표현용 높이는 소비자가 별도로 적용한다.
    // 부채꼴은 진행 방향만 돌린다. 총구 offset은 발사자 yaw로 계산하며 ray마다 돌리지 않는다.
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

    // 논리 archetype ID를 반환한다. Client가 자산을 연결하며 지원하지 않는 skill은 nullptr이다.
    inline constexpr const char* MaharakaWaterGunProjectileArchetype(const std::uint32_t skillId) noexcept
    {
        switch (skillId)
        {
        case 56900u:
            return "maharaka.watergun.burst";
        case 56910u:
            return "maharaka.watergun.bomb";
        case 56930u:
            return "maharaka.watergun.single";
        default:
            return nullptr;
        }
    }
    inline constexpr bool Is_MaharakaWaterGunProjectile(const std::string_view id) noexcept
    {
        return id == "maharaka.watergun.burst" || id == "maharaka.watergun.bomb" ||
            id == "maharaka.watergun.single";
    }
    inline constexpr std::uint32_t MAHARAKA_WATERGUN_SPEED_BUFF_ID = 569200u;

    // 투사체와 피격 몸체의 높이 차이에 사용하는 m 단위 상한이다.
    inline constexpr float MAHARAKA_WATERGUN_HIT_HEIGHT_M = 1.5f;

    // 정적 constexpr 배열의 원소를 차용해 반환한다. 배열은 프로그램 수명 동안 유지되고 일치 항목이 없으면 nullptr이다.
    inline const MAHARAKA_WATERGUN_SKILL* Find_MaharakaWaterGunSkill(const std::uint32_t skillId) noexcept
    {
        for (const MAHARAKA_WATERGUN_SKILL& skill : MAHARAKA_WATERGUN_SKILLS)
            if (skill.iSkillId == skillId)
                return &skill;
        return nullptr;
    }

    // 대문자 슬롯 문자와 정확히 비교한다. 반환 포인터의 소유권은 넘기지 않는다.
    inline const MAHARAKA_WATERGUN_SKILL* Find_MaharakaWaterGunSkillBySlot(const char inputSlot) noexcept
    {
        for (const MAHARAKA_WATERGUN_SKILL& skill : MAHARAKA_WATERGUN_SKILLS)
            if (skill.cInputSlot == inputSlot)
                return &skill;
        return nullptr;
    }

    // ms를 30Hz tick으로 올림한다. uint32 곱셈이므로 임의의 큰 입력을 위한 overflow 방어는 없고 현재 스킬 상수 범위에서 사용한다.
    inline constexpr std::uint32_t Get_MaharakaWaterGunTicks(const std::uint32_t milliseconds) noexcept
    {
        return (milliseconds * MAHARAKA_WATERPANG_TICK_HZ + 999u) / 1000u;
    }
}
