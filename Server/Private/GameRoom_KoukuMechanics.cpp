#include "GameRoom.h"
#include "GameRoom_Internal.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerCombatHitRuntime.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace GameRoomDetail;

// 기믹 commit 또는 소환 배우의 시작 trigger가 호출한다. 플레이어 선택·도약·등장·착지를 같은 패턴 실행 상태로 진행한다.
// 보스 사본에서 navigation·몸체·root motion을 검증하고 성공 위치를 본체에 반영한다. 실패 이유는 m_strStatus에 남긴다.
bool LostArk::Server::CGameRoom::Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
                                                            const BOSS_PATTERN_DEFINITION& pattern,
                                                            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
                                                            const std::uint32_t serverTick)
{
    using Phase = ALBION_AIRBORNE_PHASE;
    using namespace LostArk::Shared;
    const auto reject = [&](const char* reason)
    {
        m_strStatus = std::string("Albion airborne preserved its pose: ") + reason;
        return false;
    };
    if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
        boss.strPatternId != pattern.strPatternId || boss.iPatternStageIndex >= pattern.Stages.size() || !serverTick)
    {
        return reject("owner or stage is unavailable");
    }
    // 현재 boss를 직접 이동하기 전에 후보 상태로 높이와 root 기준을 검증
    auto staged = std::make_unique<SERVER_WORLD_ENTITY>(boss);
    auto& state = staged->AlbionAirborne;
    if (state.iPatternSequence != boss.iPatternSequence)
    {
        state = {};
        state.iPatternSequence = boss.iPatternSequence;
    }
    const auto eligible = [](const SERVER_PLAYER& player)
    {
        return player.Is_Human() && player.isCombatReady && player.iCurrentHp && player.iMarioStage == 0u &&
               player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING &&
               player.eAction != PLAYER_ACTION_STATE::GRABBED;
    };
    const auto selectedPlayer = [&]() -> SERVER_PLAYER*
    {
        for (auto& [id, player] : m_Players)
        {
            if (player.iNetEntityId == state.iSelectedPlayer && eligible(player))
            {
                return &player;
            }
        }
        return nullptr;
    };
    if (trigger.eAirbornePhase == Phase::SELECT_PLAYER)
    {
        auto* selected = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "albion.airborne.player", serverTick);
        state.iSelectedPlayer = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
        state.bHasSelectedGround = false;
        if (trigger.bCaptureAirborneTargetPosition || !selected)
        {
            // An empty arena still completes its authored sequence at a fixed ground
            // point. Mario players belong to another area and never supply this anchor.
            const float x = selected ? selected->fPositionX : boss.fPositionX;
            const float z = selected ? selected->fPositionZ : boss.fPositionZ;
            if (!m_ServerNavigation.Is_Loaded() || !m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                !m_ServerNavigation.Sample_Position(x, z, state.SelectedGround) ||
                !std::isfinite(state.SelectedGround.y))
            {
                return reject("selected navigation ground is unavailable");
            }
            state.SelectedGround.x = x;
            state.SelectedGround.z = z;
            state.bHasSelectedGround = true;
            if (!trigger.strSelectedEffectVisualId.empty())
            {
                const auto* catalog = Resolve_KoukuProductCatalog();
                if (!catalog)
                {
                    return reject("selected Effect catalog is unavailable");
                }
                SERVER_COMBAT_OBJECT_LOCKED_TARGET target;
                target.iNetEntityId = state.iSelectedPlayer;
                target.fPositionX = state.SelectedGround.x;
                target.fPositionY = state.SelectedGround.y;
                target.fPositionZ = state.SelectedGround.z;
                BOSS_COMBAT_OBJECT_DEFINITION definition;
                definition.strEncounterId = boss.strEncounterId;
                definition.strOwnerPatternId = pattern.strPatternId;
                definition.strOwnerStageActionId = trigger.strTriggerId;
                definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
                definition.strClientVisualId = trigger.strSelectedEffectVisualId;
                definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
                definition.iLifeMs = trigger.iSelectedEffectLifetimeMs;
                definition.AttackTemplates = trigger.FixedHits;
                definition.PresentationPulses.push_back({"combatpresentation.kouku.showtime.started", 0u});
                auto transaction = m_CombatObjectRuntime.Begin_Transaction();
                std::string status;
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  boss,
                                                                  &target,
                                                                  definition,
                                                                  nullptr,
                                                                  *catalog,
                                                                  1u,
                                                                  serverTick,
                                                                  status))
                {
                    m_strStatus = "Selected airborne Effect preserved capture: " + status;
                    return false;
                }
                auto& object = transaction.Objects.back();
                object.LiveState.CurrentPose.fYawDegrees = 0.f;
                object.LiveState.CurrentPose.fDirectionX = 0.f;
                object.LiveState.CurrentPose.fDirectionZ = 1.f;
                object.LiveState.PreviousPose = object.LiveState.CurrentPose;
                transaction.Spawned.back().fYawDegrees = 0.f;
                if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
                {
                    return reject("selected Effect transaction changed");
                }
            }
        }
        boss.AlbionAirborne = state;
        boss.iTargetEntityId = boss.iPatternTargetEntityId = state.iSelectedPlayer;
        return true;
    }
    SERVER_NAV_POINT oldGround{}, newGround{};
    if (!m_ServerNavigation.Is_Loaded() ||
        !m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, oldGround))
    {
        return reject("current navigation ground is unavailable");
    }
    const double patternMs = CKoukuSaydonBrain::Pattern_ElapsedMs(boss, serverTick);
    const double stageMs = CKoukuSaydonBrain::Stage_RootTimeMs(boss, pattern, serverTick);
    const float sourceUp = CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion, stageMs).fUp;
    state.ePhase = trigger.eAirbornePhase;
    state.iStartMs = trigger.iStartMs;
    state.iDurationMs = trigger.iAirborneDurationMs;
    state.iSourceStageIndex = boss.iPatternStageIndex;
    state.fStartHeightM = boss.fPositionY - oldGround.y;
    state.fPhaseHeightM = trigger.fAirborneHeightM;
    state.fLandingProgress = 0.f;
    float x = boss.fPositionX, z = boss.fPositionZ;
    if (state.ePhase == Phase::JUMP)
    {
        if (!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM <= 0.f ||
            trigger.fAirborneHeightM > 100000.f || trigger.iAirborneDurationMs > 600000u)
        {
            return reject("jump height or duration is invalid");
        }
        state.fJumpHeightM = trigger.fAirborneHeightM;
        // Reuse the actual navigation/body sweep. The same source clock still owns XZ.
        staged->iPatternStageRootLastTick = 0u;
        std::string status;
        if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,
                                                      pattern,
                                                      serverTick,
                                                      m_ServerNavigation,
                                                      m_ServerCollisionSystem,
                                                      status))
        {
            m_strStatus = "Albion jump preserved its pose: " + status;
            return false;
        }
    }
    else
    {
        if (!std::isfinite(state.fJumpHeightM) || state.fJumpHeightM <= 0.f)
        {
            return reject("initial jump has not committed");
        }
        if (state.ePhase == Phase::APPEAR_PLAYER)
        {
            if (state.fPhaseHeightM <= 0.f || !std::isfinite(state.fPhaseHeightM))
            {
                return reject("appearance height is invalid");
            }
            if (state.bHasSelectedGround)
            {
                x = state.SelectedGround.x;
                z = state.SelectedGround.z;
            }
            else
            {
                auto* selected = selectedPlayer();
                if (!selected)
                {
                    selected =
                        Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "albion.airborne.player", serverTick);
                }
                state.iSelectedPlayer = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
                if (selected)
                {
                    x = selected->fPositionX;
                    z = selected->fPositionZ;
                }
                else
                {
                    state.SelectedGround = oldGround;
                    state.SelectedGround.x = x;
                    state.SelectedGround.z = z;
                    state.bHasSelectedGround = true;
                }
            }
        }
        else if (state.ePhase == Phase::CENTER)
        {
            x = trigger.fTeleportX;
            z = trigger.fTeleportZ;
        }
        else if (state.ePhase == Phase::SLAM)
        {
            std::uint64_t stageStart = 0u;
            for (std::uint32_t index = 0u; index < boss.iPatternStageIndex; ++index)
            {
                stageStart += pattern.Stages[index].iDurationMs;
            }
            const double triggerAge = (std::max)(0.0, double(trigger.iStartMs) - double(stageStart));
            state.fSourceUpAtStart =
                CKoukuSaydonBrain::Sample_StageRootMotion(boss.PatternStageRootMotion, triggerAge).fUp;
            state.fSourceUpMinimum = state.fSourceUpAtStart;
            for (const auto& sample : boss.PatternStageRootMotion)
            {
                if (sample.iTimeMs >= triggerAge)
                {
                    state.fSourceUpMinimum = (std::min)(state.fSourceUpMinimum, sample.fUp);
                }
            }
            if (state.fSourceUpAtStart - state.fSourceUpMinimum <= .000001f)
            {
                return reject("source animation has no remaining descent");
            }
        }
        else if (state.ePhase != Phase::DISAPPEAR)
        {
            return reject("unknown motion phase");
        }
        if (state.ePhase == Phase::SLAM)
        {
            staged->iPatternStageRootLastTick = 0u;
            std::string status;
            if (!CKoukuSaydonBrain::Apply_StageRootMotion(*staged,
                                                          pattern,
                                                          serverTick,
                                                          m_ServerNavigation,
                                                          m_ServerCollisionSystem,
                                                          status))
            {
                m_strStatus = "Albion slam preserved its pose: " + status;
                return false;
            }
        }
        else
        {
            if (!std::isfinite(x) || !std::isfinite(z) || !m_ServerNavigation.Is_PointWalkableExact(x, z) ||
                !m_ServerNavigation.Sample_Position(x, z, newGround))
            {
                return reject("destination navigation is unavailable");
            }
            if (state.ePhase == Phase::CENTER &&
                (!std::isfinite(trigger.fTeleportY) || std::abs(trigger.fTeleportY - newGround.y) > 1.f))
            {
                return reject("center reference height does not match navigation");
            }
            float height = 0.f;
            if (!CKoukuSaydonBrain::Sample_AlbionAirborneHeight(state,
                                                                patternMs,
                                                                boss.iPatternStageIndex,
                                                                sourceUp,
                                                                height))
            {
                return reject("phase height is invalid");
            }
            // A captured ground point owns the complete anchor, including terrain height.
            if (state.ePhase == Phase::APPEAR_PLAYER && state.bHasSelectedGround)
            {
                newGround.y = state.SelectedGround.y;
            }
            const float y = newGround.y + height;
            if (!std::isfinite(y) || !m_ServerCollisionSystem.Is_CirclePositionClear(x,
                                                                                     y,
                                                                                     z,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.fCollisionRadius,
                                                                                     boss.iNetEntityId))
            {
                return reject("destination body overlap is invalid");
            }
            // Rebase the existing stage origin, including terrain, so its next sample
            // cannot undo the teleport or introduce a second vertical displacement.
            if (staged->bPatternStageRootOriginCaptured)
            {
                staged->fPatternStageOriginX += x - boss.fPositionX;
                staged->fPatternStageOriginY += y - boss.fPositionY;
                staged->fPatternStageOriginZ += z - boss.fPositionZ;
                staged->fPatternStageRootGroundY += newGround.y - oldGround.y;
            }
            staged->fPositionX = x;
            staged->fPositionY = y;
            staged->fPositionZ = z;
            staged->iPatternStageRootLastTick = serverTick;
        }
    }
    // 검증된 후보를 적용하고 실제 룸 actor일 때만 blocking body 위치를 갱신
    staged->iTargetEntityId = staged->iPatternTargetEntityId = state.iSelectedPlayer;
    boss = std::move(*staged);
    if (std::any_of(m_WorldEntities.begin(),
                    m_WorldEntities.end(),
                    [&](const auto& body)
                    {
                        return &body == &boss;
                    }))
    {
        (void)m_ServerCollisionSystem.Update_BlockingBody(boss.iNetEntityId,
                                                          boss.fPositionX,
                                                          boss.fPositionY + boss.fCollisionRadius,
                                                          boss.fPositionZ);
    }
    return true;
}

// Update_WorldEntities의 entity 순회가 끝난 뒤 호출한다. Logic이 보관한 owner ID·sequence를 다시 확인해 기믹을 반영한다.
// 소환은 m_WorldEntities를 늘릴 수 있으므로 순회 중 실행하지 않는다. 실패한 요청은 기존 상태 보존 경계에서 건너뛴다.
// 본체와 detached tail의 권한을 구분하고, 모든 pending 요청 처리 뒤 큐를 비운다.
void LostArk::Server::CGameRoom::Commit_KoukuMechanicTriggers(const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    // Equal-clock player selection precedes appearance regardless of authored row order.
    std::stable_sort(m_PendingKoukuMechanicTriggers.begin(),
                     m_PendingKoukuMechanicTriggers.end(),
                     [](const auto& a, const auto& b)
                     {
                         if (a.iBossEntityId != b.iBossEntityId)
                         {
                             return a.iBossEntityId < b.iBossEntityId;
                         }
                         if (a.iPatternSequence != b.iPatternSequence)
                         {
                             return a.iPatternSequence < b.iPatternSequence;
                         }
                         if (a.Trigger.iStartMs != b.Trigger.iStartMs)
                         {
                             return a.Trigger.iStartMs < b.Trigger.iStartMs;
                         }
                         const auto priority = [](const auto& t)
                         {
                             return t.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
                                            t.eAirbornePhase == ALBION_AIRBORNE_PHASE::SELECT_PLAYER
                                        ? 0
                                        : 1;
                         };
                         return priority(a.Trigger) < priority(b.Trigger);
                     });
    // This executes outside the world iteration: vector growth cannot invalidate a boss reference.
    for (const auto& pending : m_PendingKoukuMechanicTriggers)
    {
        auto liveOwner = std::find_if(m_WorldEntities.begin(),
                                      m_WorldEntities.end(),
                                      [&pending](const SERVER_WORLD_ENTITY& entity)
                                      {
                                          return entity.iNetEntityId == pending.iBossEntityId;
                                      });
        auto* owner = Find_KoukuOccurrenceOwner(pending.iBossEntityId, pending.iPatternSequence);
        if (!owner || liveOwner == m_WorldEntities.end())
        {
            continue;
        }
        const bool detached = owner != &*liveOwner;
        const auto& trigger = pending.Trigger;
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
        {
            // A Trigger selects exactly once. Detached tails cannot retarget the
            // actor after another Pattern has taken ownership.
            if (detached || !owner->iCurrentHp || owner->eAction == SERVER_ENTITY_ACTION::DEAD)
            {
                continue;
            }
            const auto* selected =
                Select_BossRandomAliveTarget(*owner,
                                             trigger.strTriggerId,
                                             trigger.iStartMs == 0u ? "boss.target.pattern" : "boss.target.random.next",
                                             serverTick);
            owner->iTargetEntityId = owner->iPatternTargetEntityId =
                selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
            owner->bHasPatternTargetLastPosition = selected != nullptr;
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
            {
                owner->Restore_KoukuPresentationAim();
                const auto* product = Resolve_KoukuProductCatalog();
                std::string status;
                const auto* pattern =
                    product ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*product, owner->strPatternId, status)
                            : nullptr;
                if (auto* member = Find_KoukuAuditionMember(owner->iNetEntityId, owner->iPatternSequence))
                {
                    for (auto& window : member->LogicLedger.PlayerTargetWindows)
                    {
                        if (!pattern || window.iTriggerIndex >= pattern->MechanicTriggers.size() ||
                            pattern->MechanicTriggers[window.iTriggerIndex].strTriggerId != trigger.strTriggerId)
                        {
                            continue;
                        }
                        window.iSelectedTargetNetEntityId = selected ? selected->iNetEntityId : INVALID_NET_ENTITY_ID;
                        if (selected)
                        {
                            window.LastSelectedTargetPosition =
                                std::array<float, 3u>{selected->fPositionX, selected->fPositionY, selected->fPositionZ};
                        }
                    }
                }
            }
            if (!selected)
            {
                continue;
            }
            owner->fPatternTargetLastPositionX = selected->fPositionX;
            owner->fPatternTargetLastPositionY = selected->fPositionY;
            owner->fPatternTargetLastPositionZ = selected->fPositionZ;
            // Tracking is presentation-only until END pins the selected point.
            // The following Duration and pattern termination restore this body basis.
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
            {
                continue;
            }
            const float dx = selected->fPositionX - owner->fPositionX, dz = selected->fPositionZ - owner->fPositionZ;
            if (dx * dx + dz * dz > .000001f)
            {
                owner->fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
                if (owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON" ||
                    owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_KOUKU")
                {
                    owner->fYawDegrees -= 90.f;
                }
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD)
        {
            Begin_KoukuBingoDuration(*owner, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            if (!catalog || detached || owner->strArchetypeId != "BOSS_KOUKUSAYDON_BINGO_SAYDON" ||
                !owner->iMaximumHealthBars)
            {
                continue;
            }
            for (auto& [id, player] : m_Players)
            {
                if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD)
                {
                    continue;
                }
                if (Is_KoukuRaidRunning() &&
                    std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), id) ==
                        m_KoukuRaid.PlayerIds.end())
                {
                    continue;
                }
                // Only an unexpired earned line or Inanna zone satisfies this mechanic.
                // Generic immunity and item protection cannot turn a failed verdict into success.
                const auto protectedUntil = [&](const std::uint32_t until)
                {
                    return until && !CKoukuSaydonLogicRuntime::Has_ReachedTick(serverTick, until);
                };
                if (protectedUntil(player.iKoukuBingoLineProtectionEndTick) ||
                    protectedUntil(player.iEstherZoneProtectionEndTick))
                {
                    // Emit the same deduplicated combat-text pulse as a safe-zone verdict.
                    player.iInvulnerabilityZoneContactTick = serverTick;
                    player.iInvulnerabilityZonePulseTick = serverTick;
                    continue;
                }
                SERVER_WORLD_TO_PLAYER_HIT wipe{};
                wipe.iRawDamage = player.iCurrentHp;
                wipe.iServerTick = serverTick;
                wipe.bEncounterWipe = true;
                wipe.fSourceX = owner->fPositionX;
                wipe.fSourceZ = owner->fPositionZ;
                (void)CServerCombatHitRuntime::Apply_WorldToPlayer(player, wipe, *catalog, m_TickDamageEvents);
            }
            if (!m_KoukuBingoDuration.bLastLineCompletionSucceeded)
            {
                continue;
            }
            const auto damage = static_cast<std::uint32_t>(
                (std::min)(std::uint64_t(owner->iCurrentHp),
                           std::uint64_t(owner->iMaximumHp) * 13u / owner->iMaximumHealthBars));
            owner->iCurrentHp -= damage;
            if (damage)
            {
                DAMAGE_EVENT event;
                event.iTargetNetEntityId = owner->iNetEntityId;
                event.iAmount = damage;
                event.fPositionX = owner->fPositionX;
                event.fPositionY = owner->fPositionY;
                event.fPositionZ = owner->fPositionZ;
                event.isOutgoing = true;
                m_TickDamageEvents.push_back(event);
            }
            if (!owner->iCurrentHp)
            {
                owner->eAction = SERVER_ENTITY_ACTION::DEAD;
                Notify_KoukuRaidBossDeath(*owner, serverTick);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            std::string status;
            const auto* pattern =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            if (!pattern)
            {
                m_strStatus = "Albion airborne preserved its pose: pinned pattern is unavailable: " + status;
                continue;
            }
            (void)Commit_KoukuAlbionAirborne(*owner, *pattern, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS)
        {
            (void)Commit_KoukuMarioPhasePlayers(*owner, trigger, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED ||
            trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
        {
            SERVER_NAV_POINT oldGround{}, newGround{};
            const bool faceCenter = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER;
            const bool grounded = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED;
            // Match the existing FaceCenter/Albion center deck admission tolerance.
            constexpr float destinationHeightToleranceM = 1.f;
            const float referenceY = (faceCenter || grounded) ? trigger.fTeleportY : owner->fPositionY;
            if (!std::isfinite(referenceY) || !std::isfinite(trigger.fTeleportX) ||
                !std::isfinite(trigger.fTeleportZ) ||
                !m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ) ||
                !m_ServerNavigation.Sample_Position(owner->fPositionX, owner->fPositionZ, oldGround) ||
                !m_ServerNavigation.Sample_Position(trigger.fTeleportX, trigger.fTeleportZ, newGround) ||
                ((faceCenter || grounded) && std::abs(referenceY - newGround.y) > destinationHeightToleranceM))
            {
                m_strStatus = "Boss teleport preserved its pose: destination navigation or height is invalid";
                continue;
            }
            const float destinationY = grounded ? newGround.y : referenceY;
            if (!m_ServerCollisionSystem.Is_CirclePositionClear(trigger.fTeleportX,
                                                                destinationY + owner->fCollisionRadius,
                                                                trigger.fTeleportZ,
                                                                owner->fCollisionRadius,
                                                                owner->fCollisionRadius,
                                                                owner->fCollisionRadius,
                                                                owner->iNetEntityId))
            {
                m_strStatus = "Boss XZ teleport preserved its pose: destination navigation or body overlap is invalid";
                continue;
            }
            const float rootX = owner->fPatternStageOriginX + trigger.fTeleportX - owner->fPositionX;
            const float rootZ = owner->fPatternStageOriginZ + trigger.fTeleportZ - owner->fPositionZ;
            const float rootGroundY = owner->fPatternStageRootGroundY + newGround.y - oldGround.y;
            const float rootY = owner->fPatternStageOriginY + (grounded ? destinationY - owner->fPositionY : 0.f);
            if (owner->bPatternStageRootOriginCaptured && (!std::isfinite(rootX) || !std::isfinite(rootZ) ||
                                                           !std::isfinite(rootY) || !std::isfinite(rootGroundY)))
            {
                m_strStatus = "Boss XZ teleport preserved its pose: root origin is invalid";
                continue;
            }
            // XZ keeps root-up; grounded rebases Y by the same committed floor delta.
            // Both preserve stage/action clocks and continue the existing root curve.
            if (owner->bPatternStageRootOriginCaptured)
            {
                owner->fPatternStageOriginX = rootX;
                owner->fPatternStageOriginZ = rootZ;
                owner->fPatternStageRootGroundY = rootGroundY;
                if (grounded)
                {
                    owner->fPatternStageOriginY = rootY;
                }
            }
            if (grounded)
            {
                owner->bPatternRootGrounded = true;
            }
            owner->fPositionX = trigger.fTeleportX;
            owner->fPositionZ = trigger.fTeleportZ;
            if (grounded)
            {
                owner->fPositionY = destinationY;
            }
            if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER)
            {
                owner->fPositionY = trigger.fTeleportY;
                const bool bingo = owner->strArchetypeId == "BOSS_KOUKUSAYDON_BINGO_SAYDON";
                const float dx = owner->fSpawnPositionX - owner->fPositionX,
                            dz = owner->fSpawnPositionZ - owner->fPositionZ;
                if (dx * dx + dz * dz > .000001f)
                {
                    owner->fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES -
                                         (bingo || owner->strArchetypeId == "BOSS_KOUKUSAYDON_G1_SAYDON" ||
                                                  owner->strArchetypeId == "BOSS_KOUKUSAYDON_G3_SAYDON" ||
                                                  owner->strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON"
                                              ? 90.f
                                              : 0.f);
                }
                owner->fPatternStageOriginYawDegrees = owner->fYawDegrees;
            }

            if (!detached)
            {
                m_ServerCollisionSystem.Update_BlockingBody(owner->iNetEntityId,
                                                            owner->fPositionX,
                                                            owner->fPositionY + owner->fCollisionRadius,
                                                            owner->fPositionZ);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            std::string status;
            const auto* parent =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            std::size_t selected = 0u;
            std::array<std::uint32_t, 4u> cloneDurations{};
            if (!parent || !baseCatalog || owner->KoukuDirectionPlayback ||
                owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID || !m_ServerNavigation.Is_Loaded() ||
                !CKoukuSaydonBrain::Select_CrossDirection(*owner,
                                                          *parent,
                                                          trigger,
                                                          *catalog,
                                                          selected,
                                                          cloneDurations,
                                                          status))
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            auto real = std::make_shared<SERVER_WORLD_ENTITY>(*owner);
            m_KoukuSaydonBrain.Abort_Pattern(*real, serverTick);
            const auto* realPattern =
                CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[selected], status);
            if (!realPattern || !m_KoukuSaydonBrain.Begin_Pattern(*real,
                                                                  *realPattern,
                                                                  owner->PinnedDefinitionRevision,
                                                                  serverTick,
                                                                  status))
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            std::vector<SERVER_WORLD_ENTITY> clones;
            NET_ENTITY_ID nextId = m_iNextNetEntityId;
            for (std::size_t index = 0u; index < 4u; ++index)
            {
                if (index == selected)
                {
                    continue;
                }
                const auto* child =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.DirectionPatternIds[index], status);
                WORLD_BOOTSTRAP_PLACEMENT placement{};
                placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
                placement.strPlacementId = owner->strPlacementId + ".cross." + std::to_string(owner->iPatternSequence) +
                                           "." + std::to_string(index);
                placement.strArchetypeId = owner->strArchetypeId;
                placement.strEncounterId = owner->strEncounterId;
                placement.fPositionX = owner->fPositionX;
                placement.fPositionY = owner->fPositionY;
                placement.fPositionZ = owner->fPositionZ;
                placement.fYawDegrees = owner->fYawDegrees;
                SERVER_WORLD_ENTITY clone{};
                if (!child || nextId == INVALID_NET_ENTITY_ID ||
                    !Build_WorldEntity(placement,
                                       nextId,
                                       clone,
                                       baseCatalog,
                                       owner->iNetEntityId,
                                       owner->iPatternSequence))
                {
                    status = "a cross direction clone failed dependent-owner admission";
                    break;
                }
                clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
                clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
                clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
                clone.fYawDegrees = placement.fYawDegrees;
                clone.bKoukuGazeClone = clone.bKoukuSummonClone = true;
                clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
                clone.iKoukuCloneEndTick =
                    CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                        CKoukuSaydonLogicRuntime::Ticks_FromMs(cloneDurations[index]));
                if (!m_KoukuSaydonBrain
                         .Begin_Pattern(clone, *child, owner->PinnedDefinitionRevision, serverTick, status))
                {
                    break;
                }
                std::vector<std::uint8_t> payload;
                if (!Build_WorldEntitySpawnedPayload(clone, payload))
                {
                    status = "a cross direction clone failed spawn serialization";
                    break;
                }
                clones.push_back(std::move(clone));
                ++nextId;
            }
            if (clones.size() != 3u)
            {
                m_strStatus = "Cross direction preserved all actors: " + status;
                continue;
            }
            const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
            m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
            auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
            committedOwner.KoukuDirectionPlayback = std::move(real);
            committedOwner.bKoukuDirectionPlaybackComplete = false;
            committedOwner.iKoukuDirectionEndTick = CKoukuSaydonLogicRuntime::Add_Ticks(
                committedOwner.iPatternStartTick,
                CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
            for (auto& clone : clones)
            {
                m_WorldEntities.push_back(std::move(clone));
                Broadcast_WorldEntitySpawned(m_WorldEntities.back());
            }
            m_iNextNetEntityId = nextId;
            m_strStatus = "Cross direction selected real Pattern " + trigger.DirectionPatternIds[selected] +
                          "; three clones end at " + trigger.strCloneEndStageId;
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS)
        {
            if (!detached)
            {
                (void)Spawn_KoukuCardRainSoldiers(owner->iNetEntityId, serverTick, &trigger);
            }
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
        {
            const auto* catalog = Resolve_KoukuProductCatalog();
            const auto* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            std::string status;
            const auto* ownerPattern =
                catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner->strPatternId, status) : nullptr;
            if (!ownerPattern || !baseCatalog || owner->iOwnerBossNetEntityId != INVALID_NET_ENTITY_ID ||
                !m_ServerNavigation.Is_Loaded() || trigger.PatternSpawns.empty() ||
                trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
            {
                m_strStatus =
                    "Summon Patterns preserved all actors: owner, catalog or spawn list is unavailable: " + status;
                continue;
            }
            const auto authoredTrigger =
                std::find_if(ownerPattern->MechanicTriggers.begin(),
                             ownerPattern->MechanicTriggers.end(),
                             [&](const auto& row)
                             {
                                 return row.strTriggerId == trigger.strTriggerId &&
                                        row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS;
                             });
            if (authoredTrigger == ownerPattern->MechanicTriggers.end())
            {
                m_strStatus = "Summon Patterns preserved all actors: the owning occurrence is unavailable";
                continue;
            }
            const float yaw = owner->fYawDegrees * 0.01745329251994329577f;
            std::vector<SERVER_WORLD_ENTITY> clones;
            NET_ENTITY_ID nextId = m_iNextNetEntityId;
            bool admitted = true;
            for (const auto& spawn : trigger.PatternSpawns)
            {
                const auto* childPattern =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, spawn.strPatternId, status);
                if (!childPattern ||
                    !CKoukuSaydonBrain::Validate_SummonedPattern(*ownerPattern, *childPattern, status, true))
                {
                    admitted = false;
                    break;
                }
                std::uint64_t childDurationMs = 0u;
                for (const auto& stage : childPattern->Stages)
                {
                    childDurationMs += stage.iDurationMs;
                }
                // A fixed Parent clock shortens pending duration to its absolute deadline.
                // Admission compares against the original authored window, before tick rounding.
                if (childDurationMs > authoredTrigger->iDurationMs)
                {
                    status = "a summoned Pattern exceeds its occurrence lifetime";
                    admitted = false;
                    break;
                }
                WORLD_BOOTSTRAP_PLACEMENT placement{};
                placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
                placement.strPlacementId = owner->strPlacementId + ".summon." +
                                           std::to_string(owner->iPatternSequence) + "." + trigger.strTriggerId + "." +
                                           spawn.strSpawnId;
                placement.strArchetypeId = owner->strArchetypeId;
                placement.strEncounterId = owner->strEncounterId;
                const bool mapAnchor = spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP;
                placement.fPositionX = mapAnchor ? spawn.PositionOffset[0]
                                                 : owner->fPositionX + spawn.PositionOffset[0] * std::cos(yaw) +
                                                       spawn.PositionOffset[2] * std::sin(yaw);
                placement.fPositionY =
                    mapAnchor ? spawn.PositionOffset[1] : owner->fPositionY + spawn.PositionOffset[1];
                placement.fPositionZ = mapAnchor ? spawn.PositionOffset[2]
                                                 : owner->fPositionZ - spawn.PositionOffset[0] * std::sin(yaw) +
                                                       spawn.PositionOffset[2] * std::cos(yaw);
                placement.fYawDegrees =
                    std::remainder((mapAnchor ? 0.f : owner->fYawDegrees) + spawn.fYawOffsetDegrees, 360.f);
                SERVER_NAV_POINT mapGround{};
                if (mapAnchor &&
                    (!m_ServerNavigation.Sample_Position(placement.fPositionX, placement.fPositionZ, mapGround) ||
                     !std::isfinite(mapGround.y) || std::abs(mapGround.y - placement.fPositionY) > 1.f))
                {
                    status = "a MAP summon position does not match its navigation floor";
                    admitted = false;
                    break;
                }
                SERVER_WORLD_ENTITY clone{};
                if (nextId == INVALID_NET_ENTITY_ID || !std::isfinite(placement.fPositionX) ||
                    !std::isfinite(placement.fPositionY) || !std::isfinite(placement.fPositionZ) ||
                    !std::isfinite(placement.fYawDegrees) ||
                    !m_ServerNavigation.Is_PointWalkableExact(placement.fPositionX, placement.fPositionZ) ||
                    !Build_WorldEntity(placement,
                                       nextId,
                                       clone,
                                       baseCatalog,
                                       owner->iNetEntityId,
                                       owner->iPatternSequence))
                {
                    status = "a summoned actor failed navigation or dependent-owner admission";
                    admitted = false;
                    break;
                }
                clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
                clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
                clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
                clone.fYawDegrees = placement.fYawDegrees;
                // Reuse the existing dependent clone lifetime and primary-loop exclusion.
                clone.bKoukuGazeClone = true;
                clone.bKoukuSummonClone = true;
                clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
                clone.iKoukuCloneEndTick =
                    CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                        CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
                if (!m_KoukuSaydonBrain
                         .Begin_Pattern(clone, *childPattern, owner->PinnedDefinitionRevision, serverTick, status) ||
                    !CKoukuSaydonBrain::Apply_StageRootMotion(clone,
                                                              *childPattern,
                                                              serverTick,
                                                              m_ServerNavigation,
                                                              m_ServerCollisionSystem,
                                                              status) ||
                    !Update_KoukuSummonTriggers(clone, *childPattern, serverTick))
                {
                    if (status.empty())
                    {
                        status = m_strStatus;
                    }
                    admitted = false;
                    break;
                }
                std::vector<std::uint8_t> payload;
                if (!Build_WorldEntitySpawnedPayload(clone, payload))
                {
                    status = "a summoned actor failed spawn wire validation";
                    admitted = false;
                    break;
                }
                clones.push_back(std::move(clone));
                ++nextId;
            }
            if (!admitted || clones.size() != trigger.PatternSpawns.size())
            {
                m_strStatus = "Summon Patterns preserved all actors: " + status;
                continue;
            }
            m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
            for (auto& clone : clones)
            {
                m_WorldEntities.push_back(std::move(clone));
                Broadcast_WorldEntitySpawned(m_WorldEntities.back());
            }
            m_iNextNetEntityId = nextId;
            m_strStatus = "Summon Patterns started " + std::to_string(trigger.PatternSpawns.size()) +
                          " independently animated clones";
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE)
        {
            const CGameplayCatalog* catalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
            if (!catalog || !m_ServerNavigation.Is_Loaded() || trigger.iCountPerPlayer < 1u ||
                trigger.iCountPerPlayer > 8u || !std::isfinite(trigger.fPlayerEffectRadiusM) ||
                trigger.fPlayerEffectRadiusM < 0.f || trigger.fPlayerEffectRadiusM > 20.f ||
                ((trigger.iCountPerPlayer == 1u) != (trigger.fPlayerEffectRadiusM == 0.f)) ||
                (trigger.bRandomPlayerOnly && trigger.iCountPerPlayer != 1u) || trigger.iArenaRandomCount > 32u ||
                !std::isfinite(trigger.fArenaRandomRadiusM) || !std::isfinite(trigger.fArenaHeightToleranceM) ||
                !std::isfinite(trigger.fArenaMinimumSpacingM) ||
                (trigger.iArenaRandomCount == 0u
                     ? (trigger.fArenaRandomRadiusM != 0.f || trigger.fArenaHeightToleranceM != 0.f ||
                        trigger.fArenaMinimumSpacingM != 0.f)
                     : (trigger.fArenaRandomRadiusM <= 0.f || trigger.fArenaRandomRadiusM > 100.f ||
                        trigger.fArenaHeightToleranceM <= 0.f || trigger.fArenaHeightToleranceM > 10.f ||
                        trigger.fArenaMinimumSpacingM <= 0.f || trigger.fArenaMinimumSpacingM > 20.f)) ||
                trigger.iEffectLifetimeMs < 1u || trigger.iEffectLifetimeMs > 600000u)
            {
                m_strStatus = "Albion volley preserved existing objects: invalid catalog, navigation or layout";
                continue;
            }
            BOSS_COMBAT_OBJECT_DEFINITION definition{};
            definition.strEncounterId = owner->strEncounterId;
            definition.strOwnerPatternId = owner->strPatternId;
            definition.strOwnerStageActionId = trigger.strTriggerId;
            definition.strCombatObjectArchetypeId = "combatobject.kouku.albion.bluecircle";
            definition.strClientVisualId = "combatvisual.kouku.albion.bluecircle";
            definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
            definition.iLifeMs = trigger.iEffectLifetimeMs;
            // Hit templates share each authoritative circle instance and its warning clock.
            definition.AttackTemplates = trigger.FixedHits;
            definition.PresentationPulses.push_back({"combatpresentation.kouku.albion.started", 0u});
            BOSS_COMBAT_OBJECT_VOLLEY volley{};
            volley.ePolicy = BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER;
            volley.iCountPerResolvedTarget = trigger.iCountPerPlayer;
            volley.iMaximumTotalObjects = 64u;
            volley.fRadiusM = trigger.fPlayerEffectRadiusM;
            volley.iArenaRandomCount = trigger.iArenaRandomCount;
            volley.fArenaRandomRadiusM = trigger.fArenaRandomRadiusM;
            volley.fArenaHeightToleranceM = trigger.fArenaHeightToleranceM;
            if (trigger.iArenaRandomCount != 0u)
            {
                volley.eArenaAnchorPolicy = BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
            }
            if (trigger.iCountPerPlayer > 1u)
            {
                volley.eLayout = BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
                volley.fAngleStepDegrees = 360.f / static_cast<float>(trigger.iCountPerPlayer);
            }
            const auto eligiblePlayer = [](const SERVER_PLAYER& player)
            {
                return player.Is_Human() && player.isCombatReady && player.iCurrentHp != 0u &&
                       player.iMarioStage == 0u && player.eAction != PLAYER_ACTION_STATE::DEAD &&
                       player.eAction != PLAYER_ACTION_STATE::FALLING;
            };
            PLAYER_ID selectedPlayerId = INVALID_PLAYER_ID;
            if (trigger.bRandomPlayerOnly)
            {
                const std::uint64_t seed = Mix_DeterministicRandom(
                    Hash_StableId(trigger.strTriggerId) ^ (static_cast<std::uint64_t>(owner->iNetEntityId) << 32u) ^
                    owner->iPatternSequence);
                std::uint64_t selectedRank = (std::numeric_limits<std::uint64_t>::max)();
                for (const auto& [playerId, player] : m_Players)
                {
                    if (!eligiblePlayer(player))
                    {
                        continue;
                    }
                    const std::uint64_t rank = Mix_DeterministicRandom(seed ^ static_cast<std::uint64_t>(playerId));
                    if (selectedPlayerId == INVALID_PLAYER_ID || rank < selectedRank)
                    {
                        selectedPlayerId = playerId;
                        selectedRank = rank;
                    }
                }
            }
            BOSS_PATTERN_STAGE_ACTION arenaAction{};
            arenaAction.strTargetId = trigger.strTriggerId;
            arenaAction.Volley = volley;
            std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
            if (!Resolve_ArenaRandomVolleyOrigins(*owner,
                                                  arenaAction,
                                                  definition,
                                                  0u,
                                                  arenaOrigins,
                                                  trigger.fArenaMinimumSpacingM))
            {
                m_strStatus = "Albion volley preserved existing objects: " + m_strStatus;
                continue;
            }
            auto transaction = m_CombatObjectRuntime.Begin_Transaction();
            std::string failure;
            bool admitted = true;
            for (const auto& [playerId, player] : m_Players)
            {
                if (!eligiblePlayer(player) || (trigger.bRandomPlayerOnly && playerId != selectedPlayerId))
                {
                    continue;
                }
                SERVER_COMBAT_OBJECT_LOCKED_TARGET target{};
                target.iNetEntityId = player.iNetEntityId;
                target.fPositionX = player.fPositionX;
                target.fPositionY = player.fPositionY;
                target.fPositionZ = player.fPositionZ;
                target.bTrackUntilFirstPulse = false;
                if (transaction.Objects.size() + trigger.iCountPerPlayer + arenaOrigins.size() >
                    volley.iMaximumTotalObjects)
                {
                    failure = "the resolved volley exceeds the object limit";
                    admitted = false;
                    break;
                }
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  *owner,
                                                                  &target,
                                                                  definition,
                                                                  &volley,
                                                                  *catalog,
                                                                  trigger.iCountPerPlayer,
                                                                  serverTick,
                                                                  failure))
                {
                    admitted = false;
                    break;
                }
            }
            if (admitted && transaction.Objects.empty())
            {
                // Preserve the authored player volley at the boss ground when the
                // arena is empty; the resulting objects never follow later movement.
                SERVER_COMBAT_OBJECT_LOCKED_TARGET fallback{};
                fallback.fPositionX = owner->fPositionX;
                fallback.fPositionY = owner->fPositionY;
                fallback.fPositionZ = owner->fPositionZ;
                admitted = m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                        *owner,
                                                                        &fallback,
                                                                        definition,
                                                                        &volley,
                                                                        *catalog,
                                                                        trigger.iCountPerPlayer,
                                                                        serverTick,
                                                                        failure);
            }
            for (const auto& origin : arenaOrigins)
            {
                if (!admitted)
                {
                    break;
                }
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  *owner,
                                                                  &origin,
                                                                  definition,
                                                                  nullptr,
                                                                  *catalog,
                                                                  1u,
                                                                  serverTick,
                                                                  failure))
                {
                    admitted = false;
                    break;
                }
            }
            // Validate the actual staged radial poses, then keep spawn/reconnect snapshots on the same floor.
            for (std::size_t index = 0u; admitted && index < transaction.Objects.size(); ++index)
            {
                auto& object = transaction.Objects[index];
                auto& pose = object.LiveState.CurrentPose;
                SERVER_NAV_POINT ground{};
                if (!m_ServerNavigation.Is_PointWalkableExact(pose.fPositionX, pose.fPositionZ) ||
                    !m_ServerNavigation.Sample_Position(pose.fPositionX, pose.fPositionZ, ground) ||
                    !std::isfinite(ground.y))
                {
                    failure = "a staged point is outside navigable ground";
                    admitted = false;
                    break;
                }
                pose.fPositionY = ground.y;
                object.LiveState.PreviousPose = pose;
                transaction.Spawned[index].fPositionY = ground.y;
            }
            if (!admitted)
            {
                m_strStatus = "Albion volley preserved existing objects: " + failure;
                continue;
            }
            if (!transaction.Objects.empty() && !m_CombatObjectRuntime.Commit(std::move(transaction)))
            {
                m_strStatus = "Albion volley preserved existing objects: transaction revision changed";
            }
            continue;
        }
        const CGameplayCatalog* catalog = Resolve_KoukuProductCatalog();
        const CGameplayCatalog* baseCatalog = m_GameplayCatalog.Resolve(owner->PinnedDefinitionRevision);
        std::string status;
        const BOSS_PATTERN_DEFINITION* clonePattern =
            nullptr == catalog
                ? nullptr
                : CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, trigger.strClonePatternId, status);
        if (nullptr == clonePattern || nullptr == baseCatalog || trigger.ClockHours.size() != 3u ||
            std::find(clonePattern->AuditionBossArchetypeIds.begin(),
                      clonePattern->AuditionBossArchetypeIds.end(),
                      owner->strArchetypeId) == clonePattern->AuditionBossArchetypeIds.end())
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: clone pattern/body is unavailable: " + status;
            continue;
        }
        // 진짜 세이튼의 저작 위치를 원의 기준점으로 삼고 가짜 세 배우의 상대 시계 방향을 계산
        const float dx = trigger.fTeleportX - owner->fSpawnPositionX;
        const float dz = trigger.fTeleportZ - owner->fSpawnPositionZ;
        const float radius = std::sqrt(dx * dx + dz * dz);
        if (!std::isfinite(radius) || radius <= 0.01f ||
            (m_ServerNavigation.Is_Loaded() &&
             !m_ServerNavigation.Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ)))
        {
            m_strStatus = "KoukuSaydon teleport preserved the boss: target is not walkable";
            continue;
        }
        const float anchor = std::atan2(dx, dz);
        std::vector<SERVER_WORLD_ENTITY> clones;
        NET_ENTITY_ID nextId = m_iNextNetEntityId;
        bool admitted = true;
        for (const std::uint32_t hour : trigger.ClockHours)
        {
            const float angle = anchor + static_cast<float>(hour - 1u) * 0.52359877559829887f;
            WORLD_BOOTSTRAP_PLACEMENT placement{};
            placement.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
            placement.strPlacementId =
                owner->strPlacementId + ".gaze." + std::to_string(owner->iPatternSequence) + "." + std::to_string(hour);
            placement.strArchetypeId = owner->strArchetypeId;
            placement.strEncounterId = owner->strEncounterId;
            placement.fPositionX = owner->fSpawnPositionX + std::sin(angle) * radius;
            placement.fPositionY = trigger.fTeleportY;
            placement.fPositionZ = owner->fSpawnPositionZ + std::cos(angle) * radius;
            placement.fYawDegrees = std::atan2(owner->fSpawnPositionX - placement.fPositionX,
                                               owner->fSpawnPositionZ - placement.fPositionZ) *
                                        57.29577951308232f +
                                    trigger.fFaceCenterYawOffsetDegrees;
            SERVER_WORLD_ENTITY clone{};
            if (INVALID_NET_ENTITY_ID == nextId ||
                (m_ServerNavigation.Is_Loaded() &&
                 !m_ServerNavigation.Is_PointWalkableExact(placement.fPositionX, placement.fPositionZ)) ||
                !Build_WorldEntity(placement, nextId, clone, baseCatalog, owner->iNetEntityId, owner->iPatternSequence))
            {
                admitted = false;
                break;
            }
            // Static placement admission projects to cell centres; retain this admitted exact circle.
            clone.fPositionX = clone.fSpawnPositionX = placement.fPositionX;
            clone.fPositionY = clone.fSpawnPositionY = placement.fPositionY;
            clone.fPositionZ = clone.fSpawnPositionZ = placement.fPositionZ;
            clone.fYawDegrees = placement.fYawDegrees;
            clone.bKoukuGazeClone = true;
            clone.iKoukuCloneOwnerSequence = owner->iPatternSequence;
            clone.iKoukuCloneEndTick =
                CKoukuSaydonLogicRuntime::Add_Ticks(serverTick,
                                                    CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
            clone.PinnedDefinitionRevision = owner->PinnedDefinitionRevision;
            if (!m_KoukuSaydonBrain
                     .Begin_Pattern(clone, *clonePattern, owner->PinnedDefinitionRevision, serverTick, status))
            {
                admitted = false;
                break;
            }
            std::vector<std::uint8_t> payload;
            if (!Build_WorldEntitySpawnedPayload(clone, payload))
            {
                admitted = false;
                break;
            }
            clones.push_back(std::move(clone));
            ++nextId;
        }
        if (!admitted || clones.size() != 3u)
        {
            m_strStatus = "KoukuSaydon teleport preserved all actors: clone admission failed: " + status;
            continue;
        }
        const auto ownerIndex = static_cast<std::size_t>(std::distance(m_WorldEntities.begin(), liveOwner));
        m_WorldEntities.reserve(m_WorldEntities.size() + clones.size());
        auto& committedOwner = detached ? *owner : m_WorldEntities[ownerIndex];
        // reserve 뒤 다시 찾은 본체에 저작 위치를 적용하고 준비된 세 분신을 추가
        committedOwner.fPositionX = trigger.fTeleportX;
        committedOwner.fPositionY = trigger.fTeleportY;
        committedOwner.fPositionZ = trigger.fTeleportZ;
        committedOwner.fYawDegrees = std::atan2(-dx, -dz) * 57.29577951308232f + trigger.fFaceCenterYawOffsetDegrees;
        for (auto& clone : clones)
        {
            m_WorldEntities.push_back(std::move(clone));
            Broadcast_WorldEntitySpawned(m_WorldEntities.back());
        }
        m_iNextNetEntityId = nextId;
        m_strStatus = "KoukuSaydon real boss teleported; three inward-facing clones started";
    }
    m_PendingKoukuMechanicTriggers.clear();
    Commit_KoukuMarioEntries();
}

// 소환 생성·분신 갱신이 호출한다. 시작 시각에 도달한 미실행 trigger를 정렬하고 성공한 항목만 실행 목록에 기록한다.
// clone의 시작 목록이 중복 실행을 막으며, 실패는 false로 상위 소환/수명 처리에 전달한다.
bool LostArk::Server::CGameRoom::Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
                                                            const BOSS_PATTERN_DEFINITION& pattern,
                                                            const std::uint32_t serverTick)
{
    using Clock = CKoukuSaydonLogicRuntime;
    std::vector<std::size_t> due;
    for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
    {
        const auto& trigger = pattern.MechanicTriggers[index];
        // The actor ledger consumes this facing Trigger before its contact windows.
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET && trigger.iDurationMs > 0u &&
            trigger.iDurationMs <= 34u && trigger.fFollowSpeedScale == 0.f)
        {
            continue;
        }
        if (std::find(clone.KoukuSummonStartedTriggers.begin(), clone.KoukuSummonStartedTriggers.end(), index) ==
                clone.KoukuSummonStartedTriggers.end() &&
            Clock::Has_ReachedTick(serverTick,
                                   Clock::Add_Ticks(clone.iPatternStartTick, Clock::Ticks_FromMs(trigger.iStartMs))))
        {
            due.push_back(index);
        }
    }
    std::stable_sort(due.begin(),
                     due.end(),
                     [&](const auto a, const auto b)
                     {
                         const auto& left = pattern.MechanicTriggers[a];
                         const auto& right = pattern.MechanicTriggers[b];
                         if (left.iStartMs != right.iStartMs)
                         {
                             return left.iStartMs < right.iStartMs;
                         }
                         return left.eAirbornePhase == ALBION_AIRBORNE_PHASE::JUMP &&
                                right.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP;
                     });
    for (const auto index : due)
    {
        const auto& trigger = pattern.MechanicTriggers[index];
        if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
            (trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP &&
             trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
            !Commit_KoukuAlbionAirborne(clone, pattern, trigger, serverTick))
        {
            return false;
        }
        clone.KoukuSummonStartedTriggers.push_back(index);
    }
    return true;
}

// 소환 배우와 교차 방향 배우 갱신이 호출한다. 배우가 소유한 접촉 ledger를 준비해 실제 actor 위치로 Logic을 평가한다.
// 정의는 pinned catalog에서 차용하고 ledger는 actor에 남긴다. 일반 본체의 audition ledger와 합치지 않는다.
void LostArk::Server::CGameRoom::Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor,
                                                           const BOSS_PATTERN_DEFINITION& pattern,
                                                           const CGameplayCatalog& product,
                                                           const std::uint32_t serverTick)
{
    if (pattern.LogicWindows.empty() && pattern.MechanicTriggers.empty())
    {
        return;
    }
    const auto* base = m_GameplayCatalog.Resolve(actor.PinnedDefinitionRevision);
    if (!base)
    {
        return;
    }
    if (!actor.KoukuContactLedger)
    {
        actor.KoukuContactLedger = std::make_shared<KOUKUSAYDON_LOGIC_LEDGER>();
        CKoukuSaydonLogicRuntime::Build(pattern, actor, actor.iPatternStartTick, *actor.KoukuContactLedger);
    }
    // Root motion has already placed this particular body. Face its own target
    // before evaluating BOSS-anchored contacts; siblings keep their original yaw.
    Update_KoukuPlayerTargets(actor, pattern, *actor.KoukuContactLedger, *base, serverTick);
    KOUKUSAYDON_LOGIC_OUTPUT output;
    // Summoned children admit actor-local facing and contact damage; they cannot submit
    // room transitions, follow-up patterns, WORLD changes or player ownership.
    CKoukuSaydonLogicRuntime::Update(actor,
                                     pattern,
                                     *actor.KoukuContactLedger,
                                     m_Players,
                                     *base,
                                     product.Find_KoukuMadnessPolicy(actor.strEncounterId),
                                     serverTick,
                                     m_TickDamageEvents,
                                     output,
                                     &m_ServerNavigation,
                                     &m_ServerCollisionSystem);
}

// 월드 본체 순회 전에 호출한다. owner 실행과 수명이 유효한 분신의 애니메이션·root motion·접촉을 진행한다.
// 유효하지 않거나 재생이 실패한 분신은 기존 despawn 알림 뒤 현재 iterator로 제거해 다음 actor를 안전하게 계속한다.
void LostArk::Server::CGameRoom::Update_KoukuGazeClones(const std::uint32_t serverTick)
{
    for (auto clone = m_WorldEntities.begin(); clone != m_WorldEntities.end();)
    {
        if (!clone->bKoukuGazeClone)
        {
            ++clone;
            continue;
        }
        const auto* owner = Find_KoukuOccurrenceOwner(clone->iOwnerBossNetEntityId, clone->iKoukuCloneOwnerSequence);
        bool live = owner != nullptr && owner->iCurrentHp > 0u &&
                    !CKoukuSaydonLogicRuntime::Has_ReachedTick(serverTick, clone->iKoukuCloneEndTick);
        const auto* catalog = Resolve_KoukuProductCatalog();
        if (live && nullptr != catalog && !clone->strPatternId.empty())
        {
            std::string status;
            if (clone->bKoukuSummonClone)
            {
                const auto* pattern =
                    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
                live = pattern && CKoukuSaydonBrain::Apply_StageRootMotion(*clone,
                                                                           *pattern,
                                                                           serverTick,
                                                                           m_ServerNavigation,
                                                                           m_ServerCollisionSystem,
                                                                           status);
                if (live)
                {
                    Update_KoukuActorContacts(*clone, *pattern, *catalog, serverTick);
                }
            }
            if (live)
            {
                const auto result = m_KoukuSaydonBrain.Update(*clone, *catalog, serverTick, status);
                live = KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION != result &&
                       KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD != result;
                if (live && clone->bKoukuSummonClone && !clone->strPatternId.empty())
                {
                    const auto* pattern =
                        CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, clone->strPatternId, status);
                    live = pattern && Update_KoukuSummonTriggers(*clone, *pattern, serverTick);
                    if (!live && status.empty())
                    {
                        status = m_strStatus;
                    }
                }
            }
            if (!live)
            {
                m_strStatus = "KoukuSaydon clone playback failed: " + status;
            }
        }
        if (live)
        {
            ++clone;
            continue;
        }
        Broadcast_WorldEntityDespawned(clone->iNetEntityId);
        clone = m_WorldEntities.erase(clone);
    }
}
