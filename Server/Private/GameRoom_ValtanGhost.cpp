#include "GameRoom.h"
#include "BossCombatRuntime.h"
#include "GameRoom_Internal.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace GameRoomDetail;

// 월드 갱신이 부활 패턴을 마친 본체에 호출한다. 정의 검증을 끝낸 뒤 유령 체력과 반복 순서를 같은 entity에 적용한다.
// boss는 룸이 소유한 참조이며 새 보스를 만들지 않는다. 정의가 없으면 상태를 적용하기 전에 실패한다.
bool LostArk::Server::CGameRoom::Activate_ValtanGhostPhaseLoop(SERVER_WORLD_ENTITY& boss,
                                                               const CGameplayCatalog& catalog)
{
    using namespace LostArk::Shared;
    if (WORLD_ID::VALTAN_ARENA != m_eWorldId || WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
        INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId || "BOSS_VALTAN" != boss.strArchetypeId ||
        "boss.valtan.center" != boss.strPlacementId || 3u != boss.iPhase || 0u == boss.iCurrentHp ||
        SERVER_ENTITY_ACTION::DEAD == boss.eAction)
    {
        m_strStatus = "Valtan ghost phase activation owner is invalid";
        return false;
    }
    // Revival reuses the authored ghost health profile on the same primary
    // entity. This runs only after the canonical respawn pattern completes.
    const BOSS_RUNTIME_PROFILE* ghostProfile = catalog.Find_Boss("BOSS_VALTAN_GHOST");
    if (nullptr == ghostProfile || 0u == ghostProfile->iMaximumHp || 0u == ghostProfile->iMaximumHealthBars)
    {
        m_strStatus = "Valtan ghost revival health profile is unavailable";
        return false;
    }
    const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
    if (nullptr == patterns)
    {
        m_strStatus = "Valtan ghost phase loop definition is unavailable";
        return false;
    }
    const auto finale = std::find_if(patterns->begin(),
                                     patterns->end(),
                                     [](const BOSS_PATTERN_DEFINITION& definition)
                                     {
                                         return "VALTAN_GHOST_FINALE" == definition.strPatternId;
                                     });
    if (patterns->end() == finale || BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != finale->Finale.eKind ||
        (4u != finale->Finale.GhostPatternIds.size() && 6u != finale->Finale.GhostPatternIds.size()))
    {
        m_strStatus = "Valtan ghost phase loop definition is unavailable";
        return false;
    }
    BOSS_PATTERN_SEQUENCE_DEFINITION sequence{};
    sequence.strEncounterId = boss.strEncounterId;
    sequence.strSequenceId = "sequence.valtan.ghost-phase.primary-loop";
    sequence.eMode = BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
    sequence.iExpectedStepCount = static_cast<std::uint32_t>(finale->Finale.GhostPatternIds.size());
    sequence.PatternIds = finale->Finale.GhostPatternIds;
    const auto* automatic = catalog.Find_BossPatternSequence(boss.strEncounterId);
    if (nullptr != automatic && BOSS_PATTERN_SEQUENCE_MODE::HEALTH_BAR_ROTATIONS == automatic->eMode)
    {
        const auto* rotation =
            catalog.Find_BossPatternRotation(boss.strEncounterId, 3u, ghostProfile->iMaximumHealthBars);
        if (nullptr == rotation || BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP != rotation->eSelectionMode ||
            rotation->PatternIds.empty())
        {
            m_strStatus = "Valtan ghost revival has no ordered health rotation";
            return false;
        }
        sequence.PatternIds = rotation->PatternIds;
        sequence.iExpectedStepCount = static_cast<std::uint32_t>(sequence.PatternIds.size());
    }
    // 검증된 체력·순서를 반영하고 이전 유령 재배치 예약만 정리
    boss.iMaximumHp = ghostProfile->iMaximumHp;
    boss.iDamageReferenceHp = ghostProfile->iDamageReferenceHp;
    boss.iAttackPower = ghostProfile->iAttackPower;
    boss.fCollisionRadius = ghostProfile->fCollisionRadius;
    boss.fEngageDistance = ghostProfile->fEngageDistance;
    boss.fMoveSpeed = ghostProfile->fMoveSpeed;
    boss.iCurrentHp = ghostProfile->iMaximumHp;
    boss.iMaximumHealthBars = ghostProfile->iMaximumHealthBars;
    boss.iLastEvaluatedHealthBar = ghostProfile->iMaximumHealthBars;
    boss.GhostPhasePatternSequence = std::move(sequence);
    boss.bGhostPhasePatternLoopActive = true;
    boss.iGhostAuxiliaryOccurrenceSequence = 0u;
    boss.iGhostAuxiliaryNextSpawnTick = 0u;
    boss.strRotationId.clear();
    boss.iRotationStepIndex = 0u;
    boss.bAutomaticPatternSequenceStepRunning = false;
    boss.iAutomaticPatternSequenceInterStepPursuitTicks = 0u;
    boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
    boss.iGhostPortalLastSpawnTick = 0u;
    boss.iGhostPortalOccurrenceSequence = 0u;
    Clear_ValtanGhostRelocationState(boss);
    boss.iGhostRelocationSequence = 0u;
    return true;
}

// 본체 갱신이 패턴 사이에 호출한다. 결정적 후보와 navigation·몸체 여유를 검사한 뒤 숨김/재등장 예약을 준비한다.
// 목적지 확보 실패는 현재 위치의 유효성과 재시도 계약을 따르며, 다른 기믹이 소유한 숨김·무적 flag를 덮지 않는다.
bool LostArk::Server::CGameRoom::Begin_ValtanGhostRelocation(SERVER_WORLD_ENTITY& boss,
                                                             const CGameplayCatalog& catalog,
                                                             const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    if (!boss.bGhostPhasePatternLoopActive || boss.bGhostRepositionPending || WORLD_ID::VALTAN_ARENA != m_eWorldId ||
        WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind || INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId ||
        "BOSS_VALTAN" != boss.strArchetypeId || "boss.valtan.center" != boss.strPlacementId || 3u != boss.iPhase ||
        0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction || 0u == serverTick ||
        !boss.strPatternId.empty() ||
        (boss.bGhostRelocationRetryPending && !Has_ReachedServerTick(serverTick, boss.iGhostRelocationRetryTick)))
    {
        m_strStatus = "Valtan ghost relocation owner is invalid";
        return false;
    }
    const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
    const auto finale = nullptr == patterns ? nullptr : [patterns]() -> const BOSS_PATTERN_DEFINITION*
    {
        const auto found = std::find_if(patterns->begin(),
                                        patterns->end(),
                                        [](const BOSS_PATTERN_DEFINITION& definition)
                                        {
                                            return "VALTAN_GHOST_FINALE" == definition.strPatternId;
                                        });
        return found == patterns->end() ? nullptr : &*found;
    }();
    const BOSS_RUNTIME_PROFILE* profile = catalog.Find_Boss(boss.strArchetypeId);
    if (nullptr == finale || BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != finale->Finale.eKind ||
        (4u != finale->Finale.GhostPatternIds.size() && 6u != finale->Finale.GhostPatternIds.size()) ||
        nullptr == profile || !std::isfinite(finale->Finale.fSpawnHalfExtentsX) ||
        !std::isfinite(finale->Finale.fSpawnHalfExtentsZ) || finale->Finale.fSpawnHalfExtentsX <= 0.f ||
        finale->Finale.fSpawnHalfExtentsZ <= 0.f || !std::isfinite(profile->fCollisionRadius) ||
        profile->fCollisionRadius <= 0.f)
    {
        m_strStatus = "Valtan ghost relocation definition is unavailable";
        return false;
    }
    if (CBossCombatRuntime::Has_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) ||
        CBossCombatRuntime::Has_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE))
    {
        m_strStatus = "Valtan ghost relocation inherited an unclosed combat flag";
        return false;
    }

    const auto hasSpawnClearance = [this, &boss, profile](const SERVER_NAV_POINT& center)
    {
        /* A random centre is admitted only when the entire boss footprint remains
		on the same walkable deck. This is the same half-cell conservative sampling
		used by the former dependent-ghost path. */
        const float radius = profile->fCollisionRadius;
        const float spacing = (std::max)(0.05f, m_ServerNavigation.Get_CellSize() * 0.5f);
        const std::uint32_t segments = (std::max)(1u, static_cast<std::uint32_t>(std::ceil(2.f * radius / spacing)));
        for (std::uint32_t row = 0u; row <= segments; ++row)
        {
            for (std::uint32_t column = 0u; column <= segments; ++column)
            {
                const float x = center.x - radius + 2.f * radius * column / segments;
                const float z = center.z - radius + 2.f * radius * row / segments;
                SERVER_NAV_POINT ground{};
                if (!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                    !m_ServerNavigation.Sample_Position(x, z, ground) ||
                    std::fabs(ground.y - boss.fSpawnPositionY) > 1.5f)
                {
                    return false;
                }
            }
        }
        return true;
    };

    std::uint32_t relocationSequence = (std::numeric_limits<std::uint32_t>::max)() == boss.iGhostRelocationSequence
                                           ? 1u
                                           : boss.iGhostRelocationSequence + 1u;
    if (0u == relocationSequence)
    {
        relocationSequence = 1u;
    }
    const std::uint64_t seed = Mix_DeterministicRandom((static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
                                                       static_cast<std::uint64_t>(relocationSequence) ^
                                                       static_cast<std::uint64_t>(serverTick));
    SERVER_NAV_POINT spawn{};
    bool foundSpawn = false;
    for (std::uint32_t attempt = 0u; attempt < 128u && !foundSpawn; ++attempt)
    {
        const float x = boss.fSpawnPositionX +
                        (2.f * DeterministicUnitFloat(seed + attempt * 2u) - 1.f) * finale->Finale.fSpawnHalfExtentsX;
        const float z = boss.fSpawnPositionZ + (2.f * DeterministicUnitFloat(seed + attempt * 2u + 1u) - 1.f) *
                                                   finale->Finale.fSpawnHalfExtentsZ;
        foundSpawn = m_ServerNavigation.Is_PointWalkableExact(x, z) &&
                     m_ServerNavigation.Sample_Position(x, z, spawn) &&
                     std::fabs(spawn.y - boss.fSpawnPositionY) <= 1.5f && hasSpawnClearance(spawn);
    }
    if (!foundSpawn)
    {
        /* The arena can be changing on the same fixed tick as an attack finishes.
		A bounded random miss is therefore not room corruption. Preserve a valid
		current footprint, or move to the immutable encounter anchor only when that
		anchor validates, then retry with the next tick in the deterministic seed. */
        const SERVER_NAV_POINT currentPose{boss.fPositionX, boss.fPositionY, boss.fPositionZ};
        SERVER_NAV_POINT currentGround{};
        const bool currentPoseValid =
            std::isfinite(currentPose.x) && std::isfinite(currentPose.y) && std::isfinite(currentPose.z) &&
            m_ServerNavigation.Is_PointWalkableExact(currentPose.x, currentPose.z) &&
            m_ServerNavigation.Sample_Position(currentPose.x, currentPose.z, currentGround) &&
            std::fabs(currentPose.y - currentGround.y) <= 1.5f &&
            std::fabs(currentGround.y - boss.fSpawnPositionY) <= 1.5f && hasSpawnClearance(currentGround);
        SERVER_NAV_POINT fallback = currentPose;
        bool fallbackValid = currentPoseValid;
        bool fallbackRequiresCommit = false;
        if (!fallbackValid)
        {
            fallback = {boss.fSpawnPositionX, boss.fSpawnPositionY, boss.fSpawnPositionZ};
            fallbackValid = std::isfinite(fallback.x) && std::isfinite(fallback.y) && std::isfinite(fallback.z) &&
                            m_ServerNavigation.Is_PointWalkableExact(fallback.x, fallback.z) &&
                            m_ServerNavigation.Sample_Position(fallback.x, fallback.z, fallback) &&
                            std::fabs(fallback.y - boss.fSpawnPositionY) <= 1.5f && hasSpawnClearance(fallback);
            fallbackRequiresCommit = fallbackValid;
        }
        if (fallbackRequiresCommit)
        {
            boss.fPositionX = fallback.x;
            boss.fPositionY = fallback.y;
            boss.fPositionZ = fallback.z;
        }
        boss.bGhostRelocationRetryPending = true;
        boss.iGhostRelocationRetryTick = Add_ServerTicksSkippingReservedZero(serverTick, 1u);
        boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
        m_strStatus = fallbackValid ? "Valtan ghost relocation retry pending from a validated pose"
                                    : "Valtan ghost relocation retry pending while preserving its current pose";
        return true;
    }

    // 기존 전투 상태의 사본에서 숨김·무적 조합을 검증한 뒤 예약 상태와 함께 적용
    SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
    if (!CBossCombatRuntime::Set_Flag(stagedCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE, true) ||
        !CBossCombatRuntime::Set_Flag(stagedCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN, true))
    {
        m_strStatus = "Valtan ghost relocation flags could not be staged";
        return false;
    }

    /* Commit pose, replication flags, and the one-tick latch together. The
	immutable spawn pose remains the portal-triangle centre. */
    boss.fPositionX = spawn.x;
    boss.fPositionY = spawn.y;
    boss.fPositionZ = spawn.z;
    boss.BossCombat = std::move(stagedCombat);
    boss.bGhostRepositionPending = true;
    boss.iGhostReappearTick = Add_ServerTicksSkippingReservedZero(serverTick, 1u);
    boss.iGhostRelocationSequence = relocationSequence;
    boss.bGhostRelocationRetryPending = false;
    boss.iGhostRelocationRetryTick = 0u;
    boss.bGhostRelocationOwnsHiddenFlag = true;
    boss.bGhostRelocationOwnsInvulnerableFlag = true;
    boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
    return true;
}

// 종속 보스 갱신이 호출한다. 유령 본체의 시간표에 맞춰 삼각형 변을 달리는 세 배우와 stage action을 준비한다.
// 후보 spawn payload까지 확인한 뒤 룸 목록에 추가한다. 추가 중 vector 재할당이 가능하므로 owner는 ID로 다시 찾는다.
bool LostArk::Server::CGameRoom::Update_ValtanGhostPortalScheduler(SERVER_WORLD_ENTITY& boss,
                                                                   const CGameplayCatalog& catalog,
                                                                   const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    constexpr std::uint32_t PORTAL_RUNNER_START_DELAY_MS = 300u;
    constexpr float TRIANGLE_CIRCUMRADIUS_M = 13.5f;
    constexpr float TRIANGLE_EDGE_LENGTH_M = 23.3826859022f;
    constexpr float PORTAL_RUNNER_SPEED_MPS = 17.9866814632f;
    constexpr float TRIANGLE_START_ANGLE_DEGREES = 30.f;
    constexpr float TRIANGLE_ANGLE_STEP_DEGREES = 120.f;
    if (!boss.bGhostPhasePatternLoopActive)
    {
        return true;
    }
    if (WORLD_ID::VALTAN_ARENA != m_eWorldId || WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind ||
        INVALID_NET_ENTITY_ID != boss.iOwnerBossNetEntityId || "BOSS_VALTAN" != boss.strArchetypeId ||
        3u != boss.iPhase || 0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
    {
        return true;
    }
    const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
    if (nullptr == patterns)
    {
        m_strStatus = "Valtan ghost portal occurrence definition is unavailable";
        return false;
    }
    const auto finale = std::find_if(patterns->begin(),
                                     patterns->end(),
                                     [](const BOSS_PATTERN_DEFINITION& p)
                                     {
                                         return p.strPatternId == "VALTAN_GHOST_FINALE";
                                     });
    if (patterns->end() == finale || BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != finale->Finale.eKind)
    {
        m_strStatus = "Valtan ghost portal interval owner is unavailable";
        return false;
    }
    // Zero disables only scheduled triangles; the independent auxiliary lane remains active.
    if (0u == finale->Finale.iPortalSpawnIntervalMs)
    {
        return true;
    }
    if (0u != boss.iGhostPortalLastSpawnTick &&
        Elapsed_ServerTicksSkippingReservedZero(boss.iGhostPortalLastSpawnTick, serverTick) <
            DurationMillisecondsToServerTicks(finale->Finale.iPortalSpawnIntervalMs))
    {
        return true;
    }
    const bool runnerStillActive =
        std::any_of(m_WorldEntities.begin(),
                    m_WorldEntities.end(),
                    [&boss](const SERVER_WORLD_ENTITY& candidate)
                    {
                        return SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER == candidate.eDependentBossRole &&
                               candidate.iOwnerBossNetEntityId == boss.iNetEntityId;
                    });
    if (runnerStillActive)
    {
        return true;
    }
    const auto portal = std::find_if(patterns->begin(),
                                     patterns->end(),
                                     [](const BOSS_PATTERN_DEFINITION& definition)
                                     {
                                         return "VALTAN_GHOST_PORTAL_ONCE" == definition.strPatternId;
                                     });
    if (patterns->end() == portal || 1u != portal->Stages.size() ||
        "valtan.ghost.portal-once.active" != portal->Stages.front().strActionId ||
        1900u != portal->Stages.front().iDurationMs)
    {
        m_strStatus = "Valtan ghost portal occurrence definition is unavailable";
        return false;
    }
    std::uint32_t occurrenceSequence =
        (std::numeric_limits<std::uint32_t>::max)() == boss.iGhostPortalOccurrenceSequence
            ? 1u
            : boss.iGhostPortalOccurrenceSequence + 1u;
    if (0u == occurrenceSequence)
    {
        occurrenceSequence = 1u;
    }
    /* Validate and stage the authored world-space geometry before opening the
	combat-object transaction. Like VALTAN_WARP, neither the proxy nor its visible
	runner is a navigation-walking actor: an exact triangle edge may cross missing
	navigation. All three retain the immutable encounter-anchor Y. */
    std::array<SERVER_NAV_POINT, 3u> vertices{};
    for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
    {
        const float degrees = TRIANGLE_START_ANGLE_DEGREES + TRIANGLE_ANGLE_STEP_DEGREES * static_cast<float>(ordinal);
        const float radians = degrees * DEGREES_TO_RADIANS;
        const float x = boss.fSpawnPositionX + std::sin(radians) * TRIANGLE_CIRCUMRADIUS_M;
        const float z = boss.fSpawnPositionZ + std::cos(radians) * TRIANGLE_CIRCUMRADIUS_M;
        if (!std::isfinite(x) || !std::isfinite(boss.fSpawnPositionY) || !std::isfinite(z))
        {
            m_strStatus = "Valtan ghost portal triangle vertex is not finite";
            return false;
        }
        vertices[ordinal] = {x, boss.fSpawnPositionY, z};
    }
    for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
    {
        const SERVER_NAV_POINT& start = vertices[ordinal];
        const SERVER_NAV_POINT& end = vertices[(ordinal + 1u) % vertices.size()];
        const float routeLength = std::hypot(end.x - start.x, end.z - start.z);
        if (!std::isfinite(routeLength) || std::fabs(routeLength - TRIANGLE_EDGE_LENGTH_M) > 0.001f)
        {
            m_strStatus = "Valtan ghost portal triangle edge geometry is invalid";
            return false;
        }
    }
    // 세 runner와 다음 entity ID는 지역 후보로 유지해 부분 생성이 룸에 노출되지 않게 준비
    std::vector<SERVER_WORLD_ENTITY> stagedRunners;
    stagedRunners.reserve(vertices.size());
    NET_ENTITY_ID nextId = m_iNextNetEntityId;
    for (std::size_t ordinal = 0u; ordinal < vertices.size(); ++ordinal)
    {
        if (INVALID_NET_ENTITY_ID == nextId)
        {
            m_strStatus = "Portal runner entity ID space is exhausted";
            return false;
        }
        const SERVER_NAV_POINT& start = vertices[ordinal];
        const SERVER_NAV_POINT& end = vertices[(ordinal + 1u) % vertices.size()];
        const float yawDegrees = std::atan2(end.x - start.x, end.z - start.z) * RADIANS_TO_DEGREES;
        WORLD_BOOTSTRAP_PLACEMENT placement{};
        placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        placement.strPlacementId = boss.strPlacementId + ".portal-runner." + std::to_string(occurrenceSequence) + "." +
                                   std::to_string(ordinal);
        placement.strArchetypeId = "BOSS_VALTAN_GHOST";
        placement.strEncounterId = boss.strEncounterId;
        /* Build at the admitted encounter anchor, then commit the exact edge pose.
		This prevents generic boss construction from projecting a runner vertex
		back onto navigation. */
        placement.fPositionX = boss.fSpawnPositionX;
        placement.fPositionY = boss.fSpawnPositionY;
        placement.fPositionZ = boss.fSpawnPositionZ;
        placement.fYawDegrees = yawDegrees;
        SERVER_WORLD_ENTITY runner{};
        if (!Build_WorldEntity(placement, nextId, runner, &catalog, boss.iNetEntityId))
        {
            return false;
        }
        runner.eDependentBossRole = SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER;
        runner.fPositionX = runner.fSpawnPositionX = start.x;
        runner.fPositionY = runner.fSpawnPositionY = start.y;
        runner.fPositionZ = runner.fSpawnPositionZ = start.z;
        runner.fYawDegrees = yawDegrees;
        runner.eAction = SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
        runner.strPatternId = portal->strPatternId;
        runner.strPatternStageId = portal->Stages.front().strStageId;
        runner.strActionId = portal->Stages.front().strActionId;
        runner.iActionStartTick = serverTick;
        runner.iPatternSequence = occurrenceSequence;
        runner.iPatternStageIndex = 0u;
        runner.iPatternStageDurationMs = portal->Stages.front().iDurationMs;
        runner.iPatternStageFirstEvaluationTick = serverTick;
        runner.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
        runner.ProductSequencePinnedDefinitionRevision = runner.PinnedDefinitionRevision;
        runner.bIntroPatternConsumed = true;
        runner.iPhase = boss.iPhase;
        runner.bPortalMotionActive = true;
        runner.bPortalRushTargetLocked = true;
        runner.iPortalRushRetargetDelayMs = PORTAL_RUNNER_START_DELAY_MS;
        runner.fPortalRushSpeedMps = PORTAL_RUNNER_SPEED_MPS;
        runner.fPortalRushDistanceM = TRIANGLE_EDGE_LENGTH_M;
        runner.fPortalStartX = start.x;
        runner.fPortalStartZ = start.z;
        runner.fPortalEndX = end.x;
        runner.fPortalEndZ = end.z;
        runner.fPortalLastHitSampleX = start.x;
        runner.fPortalLastHitSampleZ = start.z;
        runner.ePatternStageMotionKind = BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
        std::vector<std::uint8_t> payload;
        if (!Build_WorldEntitySpawnedPayload(runner, payload))
        {
            m_strStatus = "Portal runner spawn wire admission failed";
            return false;
        }
        stagedRunners.push_back(std::move(runner));
        ++nextId;
    }
    const NET_ENTITY_ID ownerId = boss.iNetEntityId;
    SERVER_WORLD_ENTITY synthetic = boss;
    synthetic.strPatternId = portal->strPatternId;
    synthetic.strPatternStageId = portal->Stages.front().strStageId;
    synthetic.strActionId = portal->Stages.front().strActionId;
    synthetic.iPatternSequence = occurrenceSequence;
    synthetic.iPatternStageIndex = 0u;
    synthetic.iActionStartTick = serverTick;
    synthetic.PinnedDefinitionRevision = catalog.Get_ActiveRevision();
    synthetic.ProductSequencePinnedDefinitionRevision = catalog.Get_ActiveRevision();
    /* One atomic radial volley owns the three simultaneous edge damage proxies.
	The primary and auxiliary ghost action clocks remain independent. */
    synthetic.fPositionX = boss.fSpawnPositionX;
    synthetic.fPositionY = boss.fSpawnPositionY;
    synthetic.fPositionZ = boss.fSpawnPositionZ;
    synthetic.fYawDegrees = 0.f;
    /* Reserve before the central volley is committed. The owner is re-resolved
	by stable ID after these three preflighted appends, so vector relocation
	cannot turn one occurrence into a portal-only partial spawn. */
    m_WorldEntities.reserve(m_WorldEntities.size() + stagedRunners.size());
    if (!Apply_BossPatternStageActions(synthetic,
                                       portal->strPatternId,
                                       portal->Stages.front().strActionId,
                                       BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
                                       serverTick))
    {
        return false;
    }
    for (SERVER_WORLD_ENTITY& runner : stagedRunners)
    {
        m_WorldEntities.push_back(std::move(runner));
        Broadcast_WorldEntitySpawned(m_WorldEntities.back());
    }
    m_iNextNetEntityId = nextId;
    const auto committedOwner = std::find_if(m_WorldEntities.begin(),
                                             m_WorldEntities.end(),
                                             [ownerId](const SERVER_WORLD_ENTITY& candidate)
                                             {
                                                 return candidate.iNetEntityId == ownerId;
                                             });
    if (m_WorldEntities.end() == committedOwner)
    {
        m_strStatus = "Portal runner owner disappeared during commit";
        return false;
    }
    committedOwner->iGhostPortalOccurrenceSequence = occurrenceSequence;
    committedOwner->iGhostPortalLastSpawnTick = serverTick;
    return true;
}

// Update_WorldEntities가 본체 순회 전후에 호출한다. 기존 자식의 수명·경로를 진행하고 본체별 추가 소환을 처리한다.
// m_WorldEntities는 룸 스레드가 직렬 소유한다. 순회 중 무효화하지 않도록 소환 후보는 별도 vector에 모아 뒤에서 추가한다.
// 생성·lifecycle 전송 실패의 false는 호출자가 룸의 runtime failure로 처리한다.
bool LostArk::Server::CGameRoom::Update_DependentBosses(const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    constexpr std::uint32_t PORTAL_RUNNER_START_DELAY_MS = 300u;
    constexpr std::uint32_t PORTAL_RUNNER_TRAVEL_MS = 1300u;
    constexpr std::uint32_t PORTAL_RUNNER_DESPAWN_MS = 1600u;
    const auto finaleOf = [this](const SERVER_WORLD_ENTITY& owner) -> const BOSS_PATTERN_FINALE*
    {
        if (WORLD_BOOTSTRAP_KIND::BOSS != owner.eKind || INVALID_NET_ENTITY_ID != owner.iOwnerBossNetEntityId ||
            0u == owner.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == owner.eAction || owner.bMechanicLedgerRequiresReset)
        {
            return nullptr;
        }
        const bool phaseThreeFinaleController = owner.bGhostPhasePatternLoopActive && 3u == owner.iPhase &&
                                                "BOSS_VALTAN" == owner.strArchetypeId &&
                                                "boss.valtan.center" == owner.strPlacementId;
        if (!phaseThreeFinaleController && owner.strPatternId.empty())
        {
            return nullptr;
        }
        const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(owner);
        const auto* patterns = nullptr == catalog ? nullptr : catalog->Find_BossPatterns(owner.strEncounterId);
        if (nullptr == patterns)
        {
            return nullptr;
        }
        const std::string finalePatternId =
            phaseThreeFinaleController ? std::string("VALTAN_GHOST_FINALE") : owner.strPatternId;
        const auto pattern = std::find_if(patterns->begin(),
                                          patterns->end(),
                                          [&finalePatternId](const BOSS_PATTERN_DEFINITION& candidate)
                                          {
                                              return candidate.strPatternId == finalePatternId;
                                          });
        return pattern != patterns->end() && BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern->Finale.eKind
                   ? &pattern->Finale
                   : nullptr;
    };
    for (auto child = m_WorldEntities.begin(); child != m_WorldEntities.end();)
    {
        if (INVALID_NET_ENTITY_ID == child->iOwnerBossNetEntityId || child->bKoukuGazeClone)
        {
            ++child;
            continue;
        }
        auto owner = std::find_if(m_WorldEntities.begin(),
                                  m_WorldEntities.end(),
                                  [&child](const SERVER_WORLD_ENTITY& candidate)
                                  {
                                      return candidate.iNetEntityId == child->iOwnerBossNetEntityId;
                                  });
        const bool ownerLive = owner != m_WorldEntities.end() && nullptr != finaleOf(*owner) &&
                               owner->strEncounterId == child->strEncounterId;
        if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER == child->eDependentBossRole)
        {
            const std::uint64_t elapsedTicks =
                Elapsed_ServerTicksSkippingReservedZero(child->iActionStartTick, serverTick);
            /* Keep the exact 1600 ms endpoint alive for one authoritative snapshot.
			The Client's portal-route presentation hides the runner at that same
			visual boundary; the following fixed tick only retires wire identity. */
            if (ownerLive && elapsedTicks <= DurationMillisecondsToServerTicks(PORTAL_RUNNER_DESPAWN_MS))
            {
                const float elapsedMs = static_cast<float>(elapsedTicks) * 1000.f / static_cast<float>(SERVER_TICK_HZ);
                const float routeRatio = std::clamp((elapsedMs - static_cast<float>(PORTAL_RUNNER_START_DELAY_MS)) /
                                                        static_cast<float>(PORTAL_RUNNER_TRAVEL_MS),
                                                    0.f,
                                                    1.f);
                child->fPositionX = child->fPortalStartX + (child->fPortalEndX - child->fPortalStartX) * routeRatio;
                child->fPositionY = child->fSpawnPositionY;
                child->fPositionZ = child->fPortalStartZ + (child->fPortalEndZ - child->fPortalStartZ) * routeRatio;
                child->fActionElapsedSeconds = elapsedMs / 1000.f;
                ++child;
                continue;
            }
            m_CombatObjectRuntime.Cancel_Source(child->iNetEntityId);
            if (!Broadcast_CombatObjectLifecycle())
            {
                return false;
            }
            Broadcast_WorldEntityDespawned(child->iNetEntityId);
            child = m_WorldEntities.erase(child);
            continue;
        }
        if (SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY != child->eDependentBossRole)
        {
            m_strStatus = "Dependent boss runtime role is invalid";
            return false;
        }
        const bool finished = child->strPatternId.empty() &&
                              child->iRotationStepIndex >= child->DependentPatternSequence.PatternIds.size();
        if (ownerLive && child->bMechanicLedgerRequiresReset)
        {
            m_strStatus = "Dependent boss attack failed and requires an encounter reset";
            return false;
        }
        if (ownerLive && !finished && 0u != child->iCurrentHp && !child->bMechanicLedgerRequiresReset)
        {
            ++child;
            continue;
        }
        if (ownerLive)
        {
            /* The authored delay starts when the auxiliary disappears; its selected
			skill still owns its lifetime. Keep at least one separate fixed-tick edge. */
            const auto* finale = finaleOf(*owner);
            const std::uint32_t replacementDelayTicks =
                (std::max)(1u, DurationMillisecondsToServerTicks(finale->iAuxiliarySpawnIntervalMs));
            owner->iGhostAuxiliaryNextSpawnTick =
                Add_ServerTicksSkippingReservedZero(serverTick, replacementDelayTicks);
        }
        m_CombatObjectRuntime.Cancel_Source(child->iNetEntityId);
        if (!Broadcast_CombatObjectLifecycle())
        {
            return false;
        }
        Broadcast_WorldEntityDespawned(child->iNetEntityId);
        child = m_WorldEntities.erase(child);
    }
    /* Portal occurrences append three runners, so schedule them only from this
	iterator-free seam. Re-resolve the owner by stable ID after every call because
	the scheduler reserves and may relocate the world-entity vector. */
    // 스케줄러가 entity 목록을 늘릴 수 있으므로 본체 참조 대신 ID 목록을 먼저 수집
    std::vector<NET_ENTITY_ID> portalOwnerIds;
    for (const SERVER_WORLD_ENTITY& candidate : m_WorldEntities)
    {
        if (candidate.bGhostPhasePatternLoopActive && INVALID_NET_ENTITY_ID == candidate.iOwnerBossNetEntityId)
        {
            portalOwnerIds.push_back(candidate.iNetEntityId);
        }
    }
    for (const NET_ENTITY_ID ownerId : portalOwnerIds)
    {
        auto owner = std::find_if(m_WorldEntities.begin(),
                                  m_WorldEntities.end(),
                                  [ownerId](const SERVER_WORLD_ENTITY& candidate)
                                  {
                                      return candidate.iNetEntityId == ownerId;
                                  });
        if (m_WorldEntities.end() == owner)
        {
            continue;
        }
        const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(*owner);
        if (nullptr == catalog || !Update_ValtanGhostPortalScheduler(*owner, *catalog, serverTick))
        {
            return false;
        }
    }
    // owner 순회가 끝난 뒤 추가할 유령 후보와 아직 소비하지 않은 entity ID
    std::vector<SERVER_WORLD_ENTITY> stagedGhosts;
    NET_ENTITY_ID nextId = m_iNextNetEntityId;
    for (SERVER_WORLD_ENTITY& owner : m_WorldEntities)
    {
        const BOSS_PATTERN_FINALE* finale = finaleOf(owner);
        if (nullptr == finale || m_Players.empty())
        {
            continue;
        }
        if (0u != owner.iGhostAuxiliaryNextSpawnTick &&
            !Has_ReachedServerTick(serverTick, owner.iGhostAuxiliaryNextSpawnTick))
        {
            continue;
        }
        const auto activeCount =
            std::count_if(m_WorldEntities.begin(),
                          m_WorldEntities.end(),
                          [&owner](const SERVER_WORLD_ENTITY& child)
                          {
                              return SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY == child.eDependentBossRole &&
                                     child.iOwnerBossNetEntityId == owner.iNetEntityId;
                          });
        if (activeCount >= finale->iMaximumActiveGhosts)
        {
            continue;
        }
        if (INVALID_NET_ENTITY_ID == nextId)
        {
            m_strStatus = "Dependent boss entity ID space is exhausted";
            return false;
        }
        const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(owner);
        if (nullptr == catalog)
        {
            return false;
        }
        const BOSS_RUNTIME_PROFILE* ghostProfile = catalog->Find_Boss(finale->strGhostArchetypeId);
        if (nullptr == ghostProfile)
        {
            m_strStatus = "Ghost finale profile disappeared from its pinned generation";
            return false;
        }
        const std::uint32_t auxiliaryOccurrenceSequence =
            (std::numeric_limits<std::uint32_t>::max)() == owner.iGhostAuxiliaryOccurrenceSequence
                ? 1u
                : owner.iGhostAuxiliaryOccurrenceSequence + 1u;
        const std::uint64_t auxiliaryIdentity = (static_cast<std::uint64_t>(owner.iNetEntityId) << 32u) ^
                                                static_cast<std::uint64_t>(auxiliaryOccurrenceSequence);
        const std::uint64_t skillSeed =
            Mix_DeterministicRandom(auxiliaryIdentity ^ Hash_StableId("valtan.ghost-phase.auxiliary.skill"));
        const std::string& selectedPatternId =
            finale->GhostPatternIds[static_cast<std::size_t>(skillSeed % finale->GhostPatternIds.size())];
        const auto hasSpawnClearance = [this, &owner, ghostProfile](const SERVER_NAV_POINT& center)
        {
            /* Conservatively cover the entire body square, including the circle
			boundary. Half-cell spacing cannot skip a one-cell navigation hole. */
            const float radius = ghostProfile->fCollisionRadius;
            const float spacing = (std::max)(0.05f, m_ServerNavigation.Get_CellSize() * 0.5f);
            const std::uint32_t segments =
                (std::max)(1u, static_cast<std::uint32_t>(std::ceil(2.f * radius / spacing)));
            for (std::uint32_t row = 0u; row <= segments; ++row)
            {
                for (std::uint32_t column = 0u; column <= segments; ++column)
                {
                    const float x = center.x - radius + 2.f * radius * column / segments;
                    const float z = center.z - radius + 2.f * radius * row / segments;
                    SERVER_NAV_POINT ground{};
                    if (!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                        !m_ServerNavigation.Sample_Position(x, z, ground) ||
                        std::fabs(ground.y - owner.fSpawnPositionY) > 1.5f)
                    {
                        return false;
                    }
                }
            }
            return true;
        };
        SERVER_NAV_POINT spawn{};
        bool foundSpawn = false;
        const std::uint64_t seed =
            Mix_DeterministicRandom(auxiliaryIdentity ^ Hash_StableId("valtan.ghost-phase.auxiliary.spawn"));
        for (std::uint32_t attempt = 0u; attempt < 128u && !foundSpawn; ++attempt)
        {
            const float x = owner.fSpawnPositionX +
                            (2.f * DeterministicUnitFloat(seed + attempt * 2u) - 1.f) * finale->fSpawnHalfExtentsX;
            const float z = owner.fSpawnPositionZ +
                            (2.f * DeterministicUnitFloat(seed + attempt * 2u + 1u) - 1.f) * finale->fSpawnHalfExtentsZ;
            foundSpawn = m_ServerNavigation.Is_PointWalkableExact(x, z) &&
                         m_ServerNavigation.Sample_Position(x, z, spawn) &&
                         std::fabs(spawn.y - owner.fSpawnPositionY) <= 1.5f && hasSpawnClearance(spawn);
        }
        if (!foundSpawn)
        {
            /* Dynamic destruction can temporarily remove every candidate. Preserve
			the occurrence identity and entity ID, then retry the same deterministic
			candidate set on the next fixed tick without failing the room. */
            owner.iGhostAuxiliaryNextSpawnTick = Add_ServerTicksSkippingReservedZero(serverTick, 1u);
            m_strStatus = "Ghost finale auxiliary spawn retry deferred: no live same-deck footprint";
            continue;
        }
        WORLD_BOOTSTRAP_PLACEMENT placement{};
        placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        placement.strPlacementId = owner.strPlacementId + ".ghost." + std::to_string(nextId);
        placement.strArchetypeId = finale->strGhostArchetypeId;
        placement.strEncounterId = owner.strEncounterId;
        placement.fPositionX = spawn.x;
        placement.fPositionY = spawn.y;
        placement.fPositionZ = spawn.z;
        placement.fYawDegrees = owner.fYawDegrees;
        SERVER_WORLD_ENTITY child{};
        if (!Build_WorldEntity(placement, nextId, child, catalog, owner.iNetEntityId))
        {
            return false;
        }
        child.eDependentBossRole = SERVER_DEPENDENT_BOSS_ROLE::AUXILIARY;
        /* Build_WorldEntity projects static placements to a cell centre. This
		candidate already passed exact live clearance; retain that admitted pose. */
        child.fPositionX = child.fSpawnPositionX = spawn.x;
        child.fPositionY = child.fSpawnPositionY = spawn.y;
        child.fPositionZ = child.fSpawnPositionZ = spawn.z;
        child.PinnedDefinitionRevision = catalog->Get_ActiveRevision();
        child.ProductSequencePinnedDefinitionRevision = child.PinnedDefinitionRevision;
        child.bIntroPatternConsumed = true;
        child.iPhase = owner.iPhase;
        child.DependentPatternSequence.strEncounterId = owner.strEncounterId;
        child.DependentPatternSequence.strSequenceId = "sequence.valtan.ghost-phase.auxiliary";
        child.DependentPatternSequence.PatternIds = {selectedPatternId};
        child.DependentPatternSequence.iExpectedStepCount = 1u;
        std::vector<std::uint8_t> payload;
        if (!Build_WorldEntitySpawnedPayload(child, payload))
        {
            m_strStatus = "Dependent boss spawn wire admission failed";
            return false;
        }
        stagedGhosts.push_back(std::move(child));
        /* Commit the random occurrence only after every spawn precondition and
		wire admission has succeeded. */
        owner.iGhostAuxiliaryOccurrenceSequence = auxiliaryOccurrenceSequence;
        owner.iGhostAuxiliaryNextSpawnTick = 0u;
        ++nextId;
    }
    /* Appending only after iteration preserves every primary and child reference. */
    m_WorldEntities.reserve(m_WorldEntities.size() + stagedGhosts.size());
    for (SERVER_WORLD_ENTITY& child : stagedGhosts)
    {
        m_WorldEntities.push_back(std::move(child));
        Broadcast_WorldEntitySpawned(m_WorldEntities.back());
    }
    m_iNextNetEntityId = nextId;
    return true;
}
