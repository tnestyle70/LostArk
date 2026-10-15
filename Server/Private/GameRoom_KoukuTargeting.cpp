#include "GameRoom.h"
#include "GameRoom_Internal.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerCombatHitRuntime.h"
#include "Gameplay/KoukuTargetTracking.h"
#include "Gameplay/WorldCollisionContract.h"

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

// 패턴 종료·tail 정리가 호출한다. 이 ledger의 owner/sequence로 만든 추적 오브젝트만 취소하고 창 상태를 닫는다.
// boss와 ledger는 호출 중 차용하며 combat object의 실제 수명은 룸의 m_CombatObjectRuntime이 소유한다.
void LostArk::Server::CGameRoom::Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger)
{
    for (auto& window : ledger.PlayerTargetWindows)
    {
        for (const auto& [playerId, objectId] : window.TrackingObjects)
        {
            m_CombatObjectRuntime.Cancel_OwnedVisualObject(objectId, boss.iNetEntityId, ledger.iPatternSequence);
        }
        window.TrackingObjects.clear();
        if (window.iBombBodyObjectId)
        {
            m_CombatObjectRuntime.Cancel_OwnedVisualObject(window.iBombBodyObjectId,
                                                           boss.iNetEntityId,
                                                           ledger.iPatternSequence);
        }
        window.iBombBodyObjectId = 0u;
        window.bClosed = true;
    }
}

// 플레이어 대상 창 갱신이 호출한다. 서버 tick에 도달한 무작위 일제 사격의 원점·크기를 준비하고 transaction으로 생성한다.
// 빈 방은 시간표만 진행한다. 후보 생성 실패와 commit 실패의 기존 기한/오브젝트 처리 순서를 유지한다.
void LostArk::Server::CGameRoom::Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
                                                          const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
                                                          KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
                                                          const CGameplayCatalog& catalog,
                                                          const std::uint32_t serverTick,
                                                          const bool hasAlivePlayers)
{
    using Clock = CKoukuSaydonLogicRuntime;
    if (trigger.RandomVolleys.empty() || !Clock::Has_ReachedTick(serverTick, window.iNextRandomTick))
    {
        return;
    }
    if (trigger.iRandomSpawnIntervalMs == 0u || trigger.iRandomSpawnIntervalMs > 600000u)
    {
        return;
    }
    const auto advanceDeadline = [&]
    {
        do
        {
            window.iNextRandomTick =
                Clock::Add_Ticks(window.iNextRandomTick, Clock::Ticks_FromMs(trigger.iRandomSpawnIntervalMs));
        } while (Clock::Has_ReachedTick(serverTick, window.iNextRandomTick));
    };
    // Empty rooms consume schedule time, but keep the next authored volley and
    // never accumulate a burst to replay when a player becomes eligible again.
    if (!hasAlivePlayers)
    {
        advanceDeadline();
        return;
    }
    const auto& selected = trigger.RandomVolleys[window.iRandomWaveOrdinal % trigger.RandomVolleys.size()];
    BOSS_COMBAT_OBJECT_DEFINITION definition;
    definition.strEncounterId = boss.strEncounterId;
    definition.strOwnerPatternId = boss.strPatternId;
    definition.strOwnerStageActionId = trigger.strTriggerId;
    definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
    definition.strClientVisualId = selected.strClientVisualId;
    definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
    definition.iLifeMs = selected.iLifetimeMs;
    definition.AttackTemplates = selected.Hits;
    std::uint32_t scaleSeed =
        boss.iPatternSequence * 747796405u ^ boss.iNetEntityId ^ (window.iRandomWaveOrdinal * 2891336453u);
    for (const unsigned char ch : trigger.strTriggerId)
    {
        scaleSeed = (scaleSeed ^ ch) * 16777619u;
    }
    scaleSeed ^= scaleSeed >> 16u;
    scaleSeed *= 2246822519u;
    scaleSeed ^= scaleSeed >> 13u;
    const float uniformScale = trigger.fRandomScaleMin + (trigger.fRandomScaleMax - trigger.fRandomScaleMin) *
                                                             float(scaleSeed & 0x00ffffffu) / 16777215.f;
    for (auto& hit : definition.AttackTemplates)
    {
        hit.fRadiusM *= uniformScale;
        hit.fInnerRadiusM *= uniformScale;
        hit.fLengthM *= uniformScale;
        hit.fHalfWidthM *= uniformScale;
        hit.fOffsetForwardM *= uniformScale;
        hit.fOffsetRightM *= uniformScale;
    }
    definition.PresentationPulses.push_back({"combatpresentation.kouku.showtime.started", 0u});
    BOSS_PATTERN_STAGE_ACTION randomAction;
    randomAction.strTargetId = trigger.strTriggerId + ".random";
    randomAction.Volley.iArenaRandomCount = 1u;
    randomAction.Volley.eArenaAnchorPolicy = BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
    randomAction.Volley.fArenaRandomRadiusM = trigger.fRandomArenaRadiusM;
    randomAction.Volley.fArenaHeightToleranceM = trigger.fRandomArenaHeightToleranceM;
    std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> origins;
    // The existing sampler admits an exact walkable centre on the arena deck.
    // One centre has no intra-wave spacing constraint; no player overlap is a hit here.
    SERVER_NAV_POINT currentAnchor{};
    const SERVER_NAV_POINT* anchorOverride = nullptr;
    if (trigger.strRandomAnchorKind == "BOSS")
    {
        if (!m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, currentAnchor))
        {
            return;
        }
        anchorOverride = &currentAnchor;
    }
    if (!Resolve_ArenaRandomVolleyOrigins(boss,
                                          randomAction,
                                          definition,
                                          window.iRandomWaveOrdinal,
                                          origins,
                                          1.f,
                                          anchorOverride))
    {
        return;
    }
    auto transaction = m_CombatObjectRuntime.Begin_Transaction();
    std::string status;
    if (origins.size() != 1u || !m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                              boss,
                                                                              &origins.front(),
                                                                              definition,
                                                                              nullptr,
                                                                              catalog,
                                                                              1u,
                                                                              serverTick,
                                                                              status))
    {
        m_strStatus = "Showtime random volley preserved its interval: " + status;
        return;
    }
    auto& pose = transaction.Objects.back().LiveState.CurrentPose;
    pose.fYawDegrees = 0.f;
    pose.fDirectionX = 0.f;
    pose.fDirectionZ = 1.f;
    transaction.Objects.back().LiveState.PreviousPose = pose;
    transaction.Spawned.back().fYawDegrees = 0.f;
    transaction.Objects.back().fUniformScale = uniformScale;
    transaction.Spawned.back().fUniformScale = uniformScale;
    if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
    {
        m_strStatus = "Showtime random volley preserved its interval: transaction changed";
        return;
    }
    ++window.iRandomWaveOrdinal;
    advanceDeadline();
}

// 대상 창 갱신이 호출한다. 기존 주사위 구속의 해제 대상 또는 유효 플레이어를 골라 추적 투사체 묶음을 준비한다.
// ledger의 다음 발사 tick·wave ordinal은 실제 생성 흐름이 소비하며, Client 표시만으로 표적이나 피해를 결정하지 않는다.
void LostArk::Server::CGameRoom::Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss,
                                                                const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
                                                                KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
                                                                const CGameplayCatalog& catalog,
                                                                const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    using Clock = CKoukuSaydonLogicRuntime;
    if ((trigger.iSpawnIntervalMs == 0u && window.iRandomWaveOrdinal != 0u) ||
        !Clock::Has_ReachedTick(serverTick, window.iNextFixedTick))
    {
        return;
    }
    if (trigger.ProjectileVisualIds.empty() || trigger.ProjectileVisualIds.size() > 4u ||
        !trigger.iProjectileCountPerWave || trigger.iProjectileCountPerWave > 16u)
    {
        return;
    }
    SERVER_PLAYER* target = nullptr;
    NET_ENTITY_ID requiredTarget = boss.iPatternTargetEntityId;
    bool diceBinding = false;
    if (const auto* member = Find_KoukuAuditionMember(boss.iNetEntityId, boss.iPatternSequence))
    {
        for (const auto& binding : member->LogicLedger.Windows)
        {
            if (binding.bOpened && !binding.bClosed && binding.iFreePlayerNetEntityId != INVALID_NET_ENTITY_ID)
            {
                requiredTarget = binding.iFreePlayerNetEntityId;
                diceBinding = true;
                break;
            }
        }
    }
    for (auto& [id, player] : m_Players)
    {
        if (player.Is_Human() && player.iNetEntityId == requiredTarget && (!diceBinding || !player.bPatternBound) &&
            player.isCombatReady && player.iCurrentHp && !player.iMarioStage &&
            player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING &&
            player.eAction != PLAYER_ACTION_STATE::GRABBED)
        {
            target = &player;
            break;
        }
    }
    if (!target && !diceBinding)
    {
        target = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "pursuit.target", serverTick);
    }
    if (!target)
    {
        return;
    }
    const auto sample = [&](std::uint32_t ordinal)
    {
        std::uint32_t value = boss.iPatternSequence * 747796405u ^ boss.iNetEntityId ^
                              (window.iRandomWaveOrdinal * 2891336453u) ^ (ordinal * 277803737u);
        for (const unsigned char ch : trigger.strTriggerId)
        {
            value = (value ^ ch) * 16777619u;
        }
        value ^= value >> 16u;
        value *= 2246822519u;
        value ^= value >> 13u;
        return float(value & 0x00ffffffu) / 16777216.f;
    };
    auto transaction = m_CombatObjectRuntime.Begin_Transaction();
    std::string status;
    for (std::uint32_t ordinal = 0u; ordinal < trigger.iProjectileCountPerWave; ++ordinal)
    {
        BOSS_COMBAT_OBJECT_DEFINITION definition;
        definition.strEncounterId = boss.strEncounterId;
        definition.strOwnerPatternId = boss.strPatternId;
        definition.strOwnerStageActionId = trigger.strTriggerId;
        definition.strCombatObjectArchetypeId = "combatobject.kouku.pursuit";
        definition.strClientVisualId =
            trigger.ProjectileVisualIds[(window.iRandomWaveOrdinal * trigger.iProjectileCountPerWave + ordinal) %
                                        trigger.ProjectileVisualIds.size()];
        definition.iLifeMs = trigger.iProjectileLifetimeMs ? trigger.iProjectileLifetimeMs : 600000u;
        definition.AttackTemplates = trigger.ProjectileHits;
        definition.PresentationPulses.push_back({"combatpresentation.kouku.pursuit.started", 0u});
        if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                          boss,
                                                          nullptr,
                                                          definition,
                                                          nullptr,
                                                          catalog,
                                                          1u,
                                                          serverTick,
                                                          status))
        {
            m_strStatus = "Pursuit volley preserved existing objects: " + status;
            return;
        }
        auto& object = transaction.Objects.back();
        object.iLockedTargetNetEntityId = target->iNetEntityId;
        object.bPersistentLifetime = trigger.iProjectileLifetimeMs == 0u;
        // The staging lifetime bounds authoring; a full-lifetime contact hit follows
        // the persistent card itself. Explicitly shorter authored hit windows stay finite.
        if (object.bPersistentLifetime)
        {
            for (auto& hit : object.Hits)
            {
                if (hit.eTrigger == SERVER_COMBAT_OBJECT_HIT_TRIGGER::CONTACT && hit.iEndMs == definition.iLifeMs)
                {
                    hit.iEndMs = 0u;
                }
            }
        }
        object.bRoomOwnedTracking = true;
        object.bHoming = trigger.bProjectileHoming;
        object.bExpireOnDistanceEnd = trigger.fProjectileMaxDistanceM > 0.f;
        object.fRemainingDistanceM = trigger.fProjectileSpeedMps * definition.iLifeMs / 1000.f;
        if (object.bExpireOnDistanceEnd)
        {
            object.fRemainingDistanceM = (std::min)(object.fRemainingDistanceM, trigger.fProjectileMaxDistanceM);
        }
        object.fSpeedMps = trigger.fProjectileSpeedMps;
        object.fContactPresentationRadiusM = trigger.fProjectileContactRadiusM;
        object.strContactPresentationId = trigger.strContactVisualId;
        if (trigger.ProjectileCardSymbols.size() == trigger.ProjectileVisualIds.size())
        {
            object.eDamageImmuneCardSymbol =
                trigger.ProjectileCardSymbols[(window.iRandomWaveOrdinal * trigger.iProjectileCountPerWave + ordinal) %
                                              trigger.ProjectileVisualIds.size()];
        }
        /* Saydon faces model +X: slot zero is one quarter turn from the yaw basis,
		180 degrees opposite the old back-facing spawn. Keep this in step with the Tool preview
		copy in Client/Private/KoukuSaydonPresentationPlayer_LogicPreview.cpp. */
        constexpr float PURSUIT_BODY_FRONT_YAW_DEGREES = 90.f;
        const float angle = trigger.bProjectileHoming ? (boss.fYawDegrees + PURSUIT_BODY_FRONT_YAW_DEGREES +
                                                         360.f * ordinal / trigger.iProjectileCountPerWave)
                                                      : sample(ordinal) * 360.f;
        const float radians = angle * .0174532925f;
        auto& pose = object.LiveState.CurrentPose;
        pose.fDirectionX = std::sin(radians);
        pose.fDirectionZ = std::cos(radians);
        pose.fYawDegrees = angle;
        pose.fPositionX += pose.fDirectionX * trigger.fProjectileSpawnRadiusM;
        pose.fPositionZ += pose.fDirectionZ * trigger.fProjectileSpawnRadiusM;
        object.LiveState.PreviousPose = pose;
        auto& message = transaction.Spawned.back();
        message.fPositionX = pose.fPositionX;
        message.fPositionZ = pose.fPositionZ;
        message.fYawDegrees = pose.fYawDegrees;
    }
    if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
    {
        m_strStatus = "Pursuit volley preserved existing objects: transaction changed";
        return;
    }
    ++window.iRandomWaveOrdinal;
    if (trigger.iSpawnIntervalMs)
    {
        do
        {
            window.iNextFixedTick =
                Clock::Add_Ticks(window.iNextFixedTick, Clock::Ticks_FromMs(trigger.iSpawnIntervalMs));
        } while (Clock::Has_ReachedTick(serverTick, window.iNextFixedTick));
    }
}

// 본체·분신·끝난 패턴 tail이 각자 보유한 ledger를 전달한다. 창별 시간, 표적, 표시 회전, 이동, 폭탄과 투사체를 갱신한다.
// 룸 스레드가 boss/ledger/player를 직렬 수정한다. 참조는 이 호출에서만 사용하며 각 창의 실패는 기존 정리·재시도 경계를 따른다.
void LostArk::Server::CGameRoom::Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
                                                           const BOSS_PATTERN_DEFINITION& pattern,
                                                           KOUKUSAYDON_LOGIC_LEDGER& ledger,
                                                           const CGameplayCatalog& catalog,
                                                           const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    using Clock = CKoukuSaydonLogicRuntime;
    if (ledger.strPatternId != pattern.strPatternId || ledger.iPatternSequence != boss.iPatternSequence)
    {
        return;
    }
    // Only the authored one-second player pursuit ends its Pattern on contact.
    // Every other tracking window preserves its attack, landing and flow clocks.
    const bool completeOnPlayerContact = pattern.strPatternId == "KOUKUSAYDON_G1_PATTERN_104";
    const auto eligible = [](const SERVER_PLAYER& player)
    {
        return player.Is_Human() && player.isCombatReady && player.iCurrentHp != 0u && player.iMarioStage == 0u &&
               player.eAction != PLAYER_ACTION_STATE::DEAD && player.eAction != PLAYER_ACTION_STATE::FALLING;
    };
    for (auto& window : ledger.PlayerTargetWindows)
    {
        if (window.bClosed || window.iLastUpdateTick == serverTick ||
            window.iTriggerIndex >= pattern.MechanicTriggers.size())
        {
            continue;
        }
        if (window.iLastUpdateTick != 0u && !Clock::Has_ReachedTick(serverTick, window.iLastUpdateTick))
        {
            continue;
        }
        const auto& trigger = pattern.MechanicTriggers[window.iTriggerIndex];
        const auto spawnBombVisual = [&](const std::string& visualId,
                                         const std::uint32_t lifeMs,
                                         const std::uint32_t birthTick,
                                         COMBAT_OBJECT_ID& objectId)
        {
            const auto& bomb = *trigger.ShowtimeBomb;
            BOSS_COMBAT_OBJECT_DEFINITION definition;
            definition.strEncounterId = boss.strEncounterId;
            definition.strOwnerPatternId = pattern.strPatternId;
            definition.strOwnerStageActionId = trigger.strTriggerId;
            definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
            definition.strClientVisualId = visualId;
            definition.iLifeMs = lifeMs;
            definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
            definition.PresentationPulses.push_back({"combatpresentation.kouku.showtime.started", 0u});
            SERVER_COMBAT_OBJECT_LOCKED_TARGET origin;
            origin.fPositionX = bomb.BodyPosition[0];
            origin.fPositionY = bomb.BodyPosition[1];
            origin.fPositionZ = bomb.BodyPosition[2];
            auto transaction = m_CombatObjectRuntime.Begin_Transaction();
            std::string failure;
            if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                              boss,
                                                              &origin,
                                                              definition,
                                                              nullptr,
                                                              catalog,
                                                              1u,
                                                              serverTick,
                                                              failure))
            {
                m_strStatus = "Showtime bomb preserved its verdict: " + failure;
                return false;
            }
            auto& object = transaction.Objects.back();
            const auto stagedId = object.iCombatObjectId;
            // A late evaluation still presents the original authored birth clock. The
            // room owns the exact body deadline; float lifetime includes a spare tick.
            object.iSpawnTick = birthTick;
            object.LiveState.CurrentPose.fYawDegrees = 0.f;
            object.LiveState.CurrentPose.fDirectionX = 0.f;
            object.LiveState.CurrentPose.fDirectionZ = 1.f;
            object.LiveState.PreviousPose = object.LiveState.CurrentPose;
            transaction.Spawned.back().iSpawnTick = birthTick;
            transaction.Spawned.back().fYawDegrees = 0.f;
            if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
            {
                m_strStatus = "Showtime bomb preserved its verdict: visual transaction changed";
                return false;
            }
            objectId = stagedId;
            return true;
        };
        // The falling body and fuse precede the tracking window and share one owner.
        if (trigger.ShowtimeBomb && !window.iBombBodyObjectId &&
            Clock::Has_ReachedTick(serverTick, window.iBombStartTick) &&
            !Clock::Has_ReachedTick(serverTick, window.iEndTick))
        {
            const auto remaining = Elapsed_ServerTicksSkippingReservedZero(serverTick, window.iEndTick);
            const auto life = static_cast<std::uint32_t>((remaining * 1000u + 29u) / 30u + 34u);
            (void)spawnBombVisual(trigger.ShowtimeBomb->strBodyVisualId,
                                  life,
                                  window.iBombStartTick,
                                  window.iBombBodyObjectId);
        }
        if (!Clock::Has_ReachedTick(serverTick, window.iStartTick))
        {
            continue;
        }
        const std::uint32_t previousUpdateTick = window.iLastUpdateTick;
        window.iLastUpdateTick = serverTick;
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
        {
            // Keep the selected identity. A departed/dead target leaves its last
            // valid position; it never causes a different player to be selected.
            const auto selected = std::find_if(m_Players.begin(),
                                               m_Players.end(),
                                               [&](const auto& entry)
                                               {
                                                   const auto& player = entry.second;
                                                   return player.iNetEntityId == window.iSelectedTargetNetEntityId &&
                                                          eligible(player) && std::isfinite(player.fPositionX) &&
                                                          std::isfinite(player.fPositionY) &&
                                                          std::isfinite(player.fPositionZ);
                                               });
            if (selected != m_Players.end())
            {
                window.LastSelectedTargetPosition = std::array<float, 3u>{selected->second.fPositionX,
                                                                          selected->second.fPositionY,
                                                                          selected->second.fPositionZ};
            }
            if (!Clock::Has_ReachedTick(serverTick, window.iEndTick))
            {
                continue;
            }
            if (window.LastSelectedTargetPosition)
            {
                const auto& point = *window.LastSelectedTargetPosition;
                boss.iTargetEntityId = boss.iPatternTargetEntityId = window.iSelectedTargetNetEntityId;
                boss.bHasPatternTargetLastPosition = true;
                boss.fPatternTargetLastPositionX = point[0];
                boss.fPatternTargetLastPositionY = point[1];
                boss.fPatternTargetLastPositionZ = point[2];
                if (!boss.Has_KoukuPresentationAim())
                {
                    boss.KoukuPresentationAimRestoreYawDegrees = boss.fYawDegrees;
                    boss.iKoukuPresentationAimSequence = boss.iPatternSequence;
                }
                const float dx = point[0] - boss.fPositionX, dz = point[2] - boss.fPositionZ;
                if (dx * dx + dz * dz > .000001f)
                {
                    boss.fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
                    if (boss.strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON" ||
                        boss.strArchetypeId == "BOSS_KOUKUSAYDON_G2_KOUKU")
                    {
                        boss.fYawDegrees -= 90.f;
                    }
                }
            }
            window.bClosed = true;
            continue;
        }
        if (Clock::Has_ReachedTick(serverTick, window.iEndTick))
        {
            if (trigger.ShowtimeBomb)
            {
                const auto& bomb = *trigger.ShowtimeBomb;
                // Tracking owns [start,end): do not rotate toward a changed target at expiry.
                if (!window.BombInsideAtEnd)
                {
                    window.BombInsideAtEnd =
                        Clock::Is_PointInsideRegion(bomb.FanRegion, boss, bomb.BodyPosition[0], bomb.BodyPosition[2]);
                }
                if (!*window.BombInsideAtEnd)
                {
                    COMBAT_OBJECT_ID explosion = 0u;
                    if (!spawnBombVisual(bomb.strExplosionVisualId, bomb.iExplosionLifetimeMs, serverTick, explosion))
                    {
                        continue;
                    }
                    for (auto& [playerId, player] : m_Players)
                    {
                        if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD ||
                            (Is_KoukuRaidRunning() &&
                             std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), playerId) ==
                                 m_KoukuRaid.PlayerIds.end()))
                        {
                            continue;
                        }
                        SERVER_WORLD_TO_PLAYER_HIT wipe{};
                        wipe.bEncounterWipe = true;
                        wipe.bIgnoreDefense = true;
                        wipe.bIgnoreCounter = true;
                        wipe.iRawDamage = player.iCurrentHp;
                        wipe.iServerTick = serverTick;
                        wipe.fSourceX = bomb.BodyPosition[0];
                        wipe.fSourceZ = bomb.BodyPosition[2];
                        (void)CServerCombatHitRuntime::Apply_WorldToPlayer(player, wipe, catalog, m_TickDamageEvents);
                    }
                }
                if (window.iBombBodyObjectId)
                {
                    m_CombatObjectRuntime.Cancel_OwnedVisualObject(window.iBombBodyObjectId,
                                                                   boss.iNetEntityId,
                                                                   ledger.iPatternSequence);
                }
                window.iBombBodyObjectId = 0u;
                m_strStatus = *window.BombInsideAtEnd ? "Showtime bomb defused inside the final fan"
                                                      : "Showtime bomb missed the final fan; raid wiped";
            }
            for (const auto& [playerId, objectId] : window.TrackingObjects)
            {
                m_CombatObjectRuntime.Cancel_OwnedVisualObject(objectId, boss.iNetEntityId, ledger.iPatternSequence);
            }
            window.TrackingObjects.clear();
            window.bClosed = true;
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES)
        {
            Update_KoukuPursuitProjectiles(boss, trigger, window, catalog, serverTick);
            continue;
        }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
            trigger.strFixedVisualId.empty() && trigger.strTrackingVisualId.empty() && !trigger.RandomVolleys.empty())
        {
            const bool hasAlivePlayers = std::any_of(m_Players.begin(),
                                                     m_Players.end(),
                                                     [&](const auto& entry)
                                                     {
                                                         return eligible(entry.second);
                                                     });
            Update_KoukuRandomVolley(boss, trigger, window, catalog, serverTick, hasAlivePlayers);
            continue;
        }
        const bool rotateOnly = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
        // RPCT_05 and RPCT_06 face model +X in every tracking mode. The same
        // basis must aim the stationary Showtime body and moving pursuit.
        const bool saydon = boss.strArchetypeId == "BOSS_KOUKUSAYDON_G1_SAYDON" ||
                            boss.strArchetypeId == "BOSS_KOUKUSAYDON_G3_SAYDON" ||
                            boss.strArchetypeId == "BOSS_KOUKUSAYDON_BINGO_SAYDON" ||
                            boss.strArchetypeId == "BOSS_KOUKUSAYDON_G2_BIG_SAYDON";
        const float forwardYawOffset = saydon ? 90.f : 0.f;
        if (!rotateOnly && (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
                            trigger.iSpawnIntervalMs == 0u || trigger.strFixedVisualId == trigger.strTrackingVisualId ||
                            trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) ||
                            !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f ||
                            trigger.fFollowSpeedScale > 10.f))
        {
            m_strStatus = "Showtime target window lost its validated definition";
            continue;
        }
        // Fixed drops capture each player without turning the authored boss motion.
        // Tracking visuals and rotate-only windows keep the existing facing policy.
        const bool tracksFacing =
            rotateOnly || trigger.strFixedVisualId.empty() || !trigger.strTrackingVisualId.empty();
        const auto findFacingTarget = [&](const NET_ENTITY_ID id) -> SERVER_PLAYER*
        {
            for (auto& [playerId, player] : m_Players)
            {
                if (player.iNetEntityId == id && eligible(player) && player.eAction != PLAYER_ACTION_STATE::GRABBED)
                {
                    return &player;
                }
            }
            return nullptr;
        };
        auto* facingTarget = tracksFacing ? findFacingTarget(boss.iPatternTargetEntityId) : nullptr;
        const bool instantFacing = rotateOnly && trigger.iDurationMs <= 34u && trigger.fFollowSpeedScale == 0.f;
        if (instantFacing)
        {
            facingTarget = nullptr;
            float nearestSquared = (std::numeric_limits<float>::max)();
            for (auto& [id, player] : m_Players)
            {
                if (!eligible(player) || player.eAction == PLAYER_ACTION_STATE::GRABBED ||
                    (Is_KoukuRaidRunning() &&
                     std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), id) ==
                         m_KoukuRaid.PlayerIds.end()))
                {
                    continue;
                }
                const float dx = player.fPositionX - boss.fPositionX, dz = player.fPositionZ - boss.fPositionZ;
                const float distanceSquared = dx * dx + dz * dz;
                if (std::isfinite(distanceSquared) &&
                    (distanceSquared < nearestSquared || (distanceSquared == nearestSquared && facingTarget &&
                                                          player.iNetEntityId < facingTarget->iNetEntityId)))
                {
                    nearestSquared = distanceSquared;
                    facingTarget = &player;
                }
            }
        }
        if (!instantFacing && tracksFacing && !facingTarget)
        {
            facingTarget = findFacingTarget(boss.iTargetEntityId);
        }
        if (!instantFacing && tracksFacing && !facingTarget)
        {
            facingTarget = Select_BossRandomAliveTarget(boss, trigger.strTriggerId, "boss.target.pattern", serverTick);
        }
        if (facingTarget)
        {
            boss.iTargetEntityId = boss.iPatternTargetEntityId = facingTarget->iNetEntityId;
            boss.bHasPatternTargetLastPosition = true;
            boss.fPatternTargetLastPositionX = facingTarget->fPositionX;
            boss.fPatternTargetLastPositionY = facingTarget->fPositionY;
            boss.fPatternTargetLastPositionZ = facingTarget->fPositionZ;
            const float dx = facingTarget->fPositionX - boss.fPositionX,
                        dz = facingTarget->fPositionZ - boss.fPositionZ;
            if (std::isfinite(dx) && std::isfinite(dz) && dx * dx + dz * dz > .000001f)
            {
                float targetYaw = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
                targetYaw -= forwardYawOffset;
                // Both modes consume the same elapsed fixed ticks. Rotate-only windows
                // use the shared tenfold remaining-arc response; moving pursuit has
                // an independent 180-degree/second turn speed.
                const auto totalTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, window.iEndTick);
                const auto previousTicks =
                    previousUpdateTick == 0u
                        ? 0u
                        : Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, previousUpdateTick) + 1u;
                const auto currentTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, serverTick) + 1u;
                if (!rotateOnly || instantFacing)
                {
                    boss.fYawDegrees = targetYaw;
                }
                else if (std::isfinite(boss.fYawDegrees) && totalTicks > previousTicks && currentTicks > previousTicks)
                {
                    const double turn = std::remainder(double(targetYaw) - boss.fYawDegrees, 360.0);
                    // Moving pursuit has a turn-speed limit independent of the window lifetime.
                    // Rotate-only response is identical for grouped and individual fixed ticks.
                    const double yawStep =
                        trigger.fFollowSpeedScale > 0.f
                            ? (std::clamp)(turn,
                                           -6.0 * (currentTicks - previousTicks),
                                           6.0 * (currentTicks - previousTicks))
                            : turn * KoukuTargetTracking::RotateOnlyFraction(currentTicks - previousTicks,
                                                                             totalTicks - previousTicks);
                    boss.fYawDegrees = static_cast<float>(std::remainder(double(boss.fYawDegrees) + yawStep, 360.0));
                }
            }
        }
        else if (tracksFacing)
        {
            boss.iTargetEntityId = boss.iPatternTargetEntityId = INVALID_NET_ENTITY_ID;
            boss.bHasPatternTargetLastPosition = false;
        }
        // An instantaneous authored Trigger samples once, including an empty roster.
        if (instantFacing)
        {
            window.bClosed = true;
        }
        if (rotateOnly)
        {
            if (facingTarget && trigger.fFollowSpeedScale > 0.f &&
                std::hypot(facingTarget->fPositionX - boss.fPositionX, facingTarget->fPositionZ - boss.fPositionZ) <=
                    boss.fCollisionRadius + WorldCollision::PLAYER_HALF_EXTENT_X + .5f)
            {
                if (completeOnPlayerContact)
                {
                    window.bClosed = true;
                    ledger.bTrackingTargetReached = true;
                }
                // Attacks stop translation while touching, then resume following if
                // the player moves away before this authored window ends.
                continue;
            }
            /* A positive authored scale walks the body along the yaw this tick just wrote,
			   at the tracked player's own move speed, so BOSS-anchored Effects travel and
			   turn with it. Zero preserves the original rotate-only window exactly. */
            SERVER_NAV_POINT startGround{};
            const float followStep = facingTarget && trigger.fFollowSpeedScale > 0.f
                                         ? Resolve_PlayerMoveSpeed(*facingTarget) * trigger.fFollowSpeedScale / 30.f
                                         : 0.f;
            if (followStep <= 0.f || !std::isfinite(followStep) || !std::isfinite(boss.fYawDegrees))
            {
                continue;
            }
            if (!m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, startGround))
            {
                m_strStatus = "Boss tracking follow lost its current ground";
                continue;
            }
            // Terrain owns the base; the source clip owns the height above it.
            const float followHeight = boss.fPositionY - startGround.y;
            if (!std::isfinite(followHeight))
            {
                continue;
            }
            const float followRadians = (boss.fYawDegrees + forwardYawOffset) * .0174532925f;
            SERVER_NAV_POINT destination{boss.fPositionX + std::sin(followRadians) * followStep,
                                         boss.fPositionY,
                                         boss.fPositionZ + std::cos(followRadians) * followStep};
            if (!m_ServerNavigation.Resolve_TraversalStep(boss.fPositionX,
                                                          boss.fPositionZ,
                                                          destination.x,
                                                          destination.z,
                                                          destination))
            {
                m_strStatus = "Boss tracking follow stopped at the navigation boundary";
                continue;
            }
            destination.y += followHeight;
            bool followBlocked = false;
            SERVER_NAV_POINT followGround{};
            if (!m_ServerCollisionSystem.Resolve_CircleMove(boss.fPositionX,
                                                            boss.fPositionY,
                                                            boss.fPositionZ,
                                                            destination.x,
                                                            destination.y,
                                                            destination.z,
                                                            boss.fCollisionRadius,
                                                            boss.fCollisionRadius,
                                                            boss.fCollisionRadius,
                                                            destination.x,
                                                            destination.y,
                                                            destination.z,
                                                            followBlocked,
                                                            boss.iNetEntityId,
                                                            false) ||
                !m_ServerNavigation.Resolve_TraversalStep(boss.fPositionX,
                                                          boss.fPositionZ,
                                                          destination.x,
                                                          destination.z,
                                                          followGround))
            {
                m_strStatus = "Boss tracking follow preserved its pose at the collision boundary";
                continue;
            }
            const float followY = followGround.y + followHeight;
            if (!std::isfinite(destination.x) || !std::isfinite(followY) || !std::isfinite(destination.z))
            {
                continue;
            }
            /* The stage clock still owns its own baked root motion from a captured origin.
			   Move that origin and its ground base by the same delta, which keeps
			   originY - rootGroundY invariant so the next sample continues from here. */
            if (boss.bPatternStageRootOriginCaptured)
            {
                boss.fPatternStageOriginX += destination.x - boss.fPositionX;
                boss.fPatternStageOriginY += followY - boss.fPositionY;
                boss.fPatternStageOriginZ += destination.z - boss.fPositionZ;
                boss.fPatternStageRootGroundY += followGround.y - startGround.y;
            }
            boss.fPositionX = destination.x;
            boss.fPositionY = followY;
            boss.fPositionZ = destination.z;
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
            if (followBlocked)
            {
                m_strStatus = "Boss tracking follow stopped at the collision boundary";
            }
            continue;
        }
        for (auto it = window.TrackingObjects.begin(); it != window.TrackingObjects.end();)
        {
            const auto player = m_Players.find(it->first);
            const auto& live = m_CombatObjectRuntime.Get_LiveObjects();
            const auto object = std::find_if(live.begin(),
                                             live.end(),
                                             [&](const auto& row)
                                             {
                                                 return row.iCombatObjectId == it->second;
                                             });
            if (player == m_Players.end() || !eligible(player->second) || object == live.end() ||
                object->iLockedTargetNetEntityId != player->second.iNetEntityId)
            {
                m_CombatObjectRuntime.Cancel_OwnedVisualObject(it->second, boss.iNetEntityId, ledger.iPatternSequence);
                it = window.TrackingObjects.erase(it);
                continue;
            }
            const auto& pose = object->LiveState.CurrentPose;
            const float dx = player->second.fPositionX - pose.fPositionX,
                        dz = player->second.fPositionZ - pose.fPositionZ;
            const float distance = std::hypot(dx, dz);
            const float step = Resolve_PlayerMoveSpeed(player->second) * trigger.fFollowSpeedScale / 30.f;
            if (!std::isfinite(distance) || !std::isfinite(step) || step < 0.f)
            {
                m_strStatus = "Showtime tracking preserved its pose: non-finite target or speed";
                ++it;
                continue;
            }
            const float ratio = distance > 0.f ? (std::min)(1.f, step / distance) : 0.f;
            const float x = pose.fPositionX + dx * ratio, z = pose.fPositionZ + dz * ratio;
            SERVER_NAV_POINT ground{};
            if (m_ServerNavigation.Sample_Position(x, z, ground) && std::isfinite(ground.y))
            {
                m_CombatObjectRuntime
                    .Set_OwnedVisualPosition(it->second, boss.iNetEntityId, ledger.iPatternSequence, x, ground.y, z);
            }
            else
            {
                m_strStatus = "Showtime tracking preserved its pose: ground is unavailable";
            }
            ++it;
        }
        const bool fixedDue =
            !trigger.strFixedVisualId.empty() && Clock::Has_ReachedTick(serverTick, window.iNextFixedTick);
        std::size_t playerCount = 0u;
        for (const auto& [playerId, player] : m_Players)
        {
            if (eligible(player))
            {
                ++playerCount;
            }
        }
        if (playerCount > 4u)
        {
            m_strStatus = "Showtime target window exceeds four living players";
            continue;
        }
        Update_KoukuRandomVolley(boss, trigger, window, catalog, serverTick, playerCount != 0u);
        auto transaction = m_CombatObjectRuntime.Begin_Transaction();
        std::vector<std::pair<PLAYER_ID, COMBAT_OBJECT_ID>> createdTrackers;
        bool admitted = true;
        std::string failure;
        for (const auto& [playerId, player] : m_Players)
        {
            if (!eligible(player))
            {
                continue;
            }
            const bool newTracker = !trigger.strTrackingVisualId.empty() && !window.TrackingObjects.contains(playerId);
            if (!fixedDue && !newTracker)
            {
                continue;
            }
            for (const bool tracking : {false, true})
            {
                if ((tracking && !newTracker) || (!tracking && !fixedDue))
                {
                    continue;
                }
                // Each living player's tracker begins at the authored arena centre,
                // while its exact locked entity remains the independent pursuit target.
                const float originX = tracking ? boss.fSpawnPositionX : player.fPositionX;
                const float originZ = tracking ? boss.fSpawnPositionZ : player.fPositionZ;
                SERVER_NAV_POINT ground{};
                if (!m_ServerNavigation.Sample_Position(originX, originZ, ground) || !std::isfinite(ground.y))
                {
                    admitted = false;
                    failure = "the selected player or arena-centre ground is unavailable";
                    break;
                }
                SERVER_COMBAT_OBJECT_LOCKED_TARGET target;
                target.iNetEntityId = player.iNetEntityId;
                target.fPositionX = originX;
                target.fPositionY = ground.y;
                target.fPositionZ = originZ;
                BOSS_COMBAT_OBJECT_DEFINITION definition;
                definition.strEncounterId = boss.strEncounterId;
                definition.strOwnerPatternId = pattern.strPatternId;
                definition.strOwnerStageActionId = trigger.strTriggerId;
                definition.strCombatObjectArchetypeId =
                    tracking ? "combatobject.kouku.showtime.tracking" : "combatobject.kouku.showtime.fixed";
                definition.strClientVisualId = tracking ? trigger.strTrackingVisualId : trigger.strFixedVisualId;
                definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
                // Exact window cleanup owns tracking; its float lifetime has one spare tick.
                const auto remainingTicks = static_cast<std::uint32_t>(window.iEndTick - serverTick);
                definition.iLifeMs = tracking
                                         ? static_cast<std::uint32_t>(
                                               (static_cast<std::uint64_t>(remainingTicks) * 1000u + 29u) / 30u + 34u)
                                         : trigger.iFixedLifetimeMs;
                definition.AttackTemplates = tracking ? trigger.TrackingHits : trigger.FixedHits;
                if (tracking)
                {
                    std::erase_if(definition.AttackTemplates,
                                  [&](const auto& hit)
                                  {
                                      return hit.iAtMs >= definition.iLifeMs;
                                  });
                    for (auto& hit : definition.AttackTemplates)
                    {
                        if (hit.strTrigger == "CONTACT")
                        {
                            hit.iEndMs = (std::min)(hit.iEndMs, definition.iLifeMs);
                        }
                    }
                }
                definition.PresentationPulses.push_back({"combatpresentation.kouku.showtime.started", 0u});
                if (!m_CombatObjectRuntime.Stage_BossCombatObject(transaction,
                                                                  boss,
                                                                  &target,
                                                                  definition,
                                                                  nullptr,
                                                                  catalog,
                                                                  1u,
                                                                  serverTick,
                                                                  failure))
                {
                    admitted = false;
                    break;
                }
                // Group TRS is in map axes; the boss's current facing must not rotate it.
                auto& object = transaction.Objects.back();
                object.bRoomOwnedTracking = true;
                object.LiveState.CurrentPose.fYawDegrees = 0.f;
                object.LiveState.CurrentPose.fDirectionX = 0.f;
                object.LiveState.CurrentPose.fDirectionZ = 1.f;
                object.LiveState.PreviousPose = object.LiveState.CurrentPose;
                transaction.Spawned.back().fYawDegrees = 0.f;
                if (tracking)
                {
                    createdTrackers.emplace_back(playerId, object.iCombatObjectId);
                }
            }
            if (!admitted)
            {
                break;
            }
        }
        if (!admitted)
        {
            m_strStatus = "Showtime targets preserved existing objects: " + failure;
            continue;
        }
        if (!transaction.Objects.empty())
        {
            if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
            {
                m_strStatus = "Showtime targets preserved existing objects: transaction changed";
                continue;
            }
            for (const auto& [playerId, objectId] : createdTrackers)
            {
                window.TrackingObjects.emplace(playerId, objectId);
            }
        }
        if (fixedDue)
        {
            // Only consume an admitted interval; never replay missed ticks as a burst.
            do
            {
                window.iNextFixedTick =
                    Clock::Add_Ticks(window.iNextFixedTick, Clock::Ticks_FromMs(trigger.iSpawnIntervalMs));
            } while (Clock::Has_ReachedTick(serverTick, window.iNextFixedTick));
        }
    }
}
