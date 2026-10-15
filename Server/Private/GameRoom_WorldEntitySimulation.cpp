#include "GameRoom.h"
#include "GameRoom_Internal.h"

#include "ClientSession.h"
#include "KoukuSaydonBrain.h"
#include "MonsterBrain.h"
#include "ServerCombatHitRuntime.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

//개체 하나의 갱신 분기. true는 이 개체 처리 완료·건너뜀, false는 룸 갱신 중단이다.
//m_WorldEntities의 소유권과 순서는 호출자가 유지하며 이 함수는 원소를 추가·삭제하지 않는다.
bool LostArk::Server::CGameRoom::Update_WorldEntity(SERVER_WORLD_ENTITY& entity,
                                                    const float fixedDeltaSeconds,
                                                    const std::uint32_t updateTick)
{
    CServerBuffRuntime::Expire(entity.ActiveBuffs, updateTick);
    if (entity.bKoukuGazeClone || entity.eKind == WORLD_BOOTSTRAP_KIND::WORLD_OBJECT)
    {
        return true;
    }
    // Maze targets share card-soldier models/profiles, but the maze owns their motion and contact.
    if (m_KoukuCardMaze.Is_Target(entity.iNetEntityId))
    {
        return true;
    }
    if (entity.isEstherSummon)
    {
        /* The clip carries its own entrance and exit; the room only clocks
		the strike so the sweep below despawns it the moment it ends. */
        entity.fActionElapsedSeconds += fixedDeltaSeconds;
        Apply_EstherStrikeHits(entity, updateTick);
        return true;
    }
    if (SERVER_DEPENDENT_BOSS_ROLE::PORTAL_RUNNER == entity.eDependentBossRole)
    {
        /* The dependent scheduler owns this actor's exact world transform and
		lifetime. It never enters target selection, navigation or boss combat. */
        return true;
    }
    if (entity.eKind == WORLD_BOOTSTRAP_KIND::NPC)
    {
        const WORLD_BOOTSTRAP_PLACEMENT* placement = Find_Placement(entity.strPlacementId);
        if (nullptr == placement)
        {
            m_strStatus = "NPC runtime placement disappeared: " + entity.strPlacementId;
            Mark_RuntimeFailure("world-update.npc-placement");
            return false;
        }
        if (!placement->bHasNpcBehavior)
        {
            return true;
        }
        const SERVER_WORLD_ENTITY* lookTarget = nullptr;
        if (!placement->NpcBehavior.strLookTargetPlacementId.empty())
        {
            const auto target =
                std::find_if(m_WorldEntities.begin(),
                             m_WorldEntities.end(),
                             [placement](const SERVER_WORLD_ENTITY& candidate)
                             {
                                 return candidate.strPlacementId == placement->NpcBehavior.strLookTargetPlacementId;
                             });
            if (target != m_WorldEntities.end())
            {
                lookTarget = &*target;
            }
        }
        if (!m_NpcBehaviorRuntime.Update(*placement,
                                         lookTarget,
                                         m_ServerNavigation,
                                         m_ServerCollisionSystem,
                                         fixedDeltaSeconds,
                                         updateTick,
                                         entity,
                                         m_strStatus))
        {
            Mark_RuntimeFailure("world-update.npc-behavior");
            return false;
        }
        if (!m_ServerCollisionSystem.Update_BlockingBody(entity.iNetEntityId,
                                                         entity.fPositionX,
                                                         entity.fPositionY +
                                                             LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y,
                                                         entity.fPositionZ))
        {
            m_strStatus = "NPC blocking body disappeared: " + entity.strPlacementId;
            Mark_RuntimeFailure("world-update.npc-blocking-body");
            return false;
        }
        return true;
    }
    if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS && m_ServerNavigation.Is_Loaded() &&
        CKoukuSaydonBrain::Is_ArenaBoss(m_eWorldId, entity))
    {
        /* Every arena boss, including the enabled starting Kouku, waits
		where it spawned until an audition names it. It never enters the Valtan brain, so the gate
		button that activated it cannot start a pattern by itself. While an
		audition owns another arena boss this one only keeps its pin. */
        const bool auditionOwnsAnotherBoss =
            KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE != m_KoukuSaydonPatternAudition.ePhase &&
            nullptr == Find_KoukuAuditionMember(entity.iNetEntityId);
        if (auditionOwnsAnotherBoss)
        {
            entity.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
            return true;
        }
        if (!Update_KoukuSaydonBoss(entity, updateTick))
        {
            Mark_RuntimeFailure("world-update.koukusaydon-arena-boss");
            return false;
        }
        (void)m_ServerCollisionSystem.Update_BlockingBody(entity.iNetEntityId,
                                                          entity.fPositionX,
                                                          entity.fPositionY + entity.fCollisionRadius,
                                                          entity.fPositionZ);
        return true;
    }
    if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS && m_ServerNavigation.Is_Loaded())
    {
        return Update_ValtanWorldEntity(entity, fixedDeltaSeconds, updateTick);
    }
    if (entity.eKind == WORLD_BOOTSTRAP_KIND::MONSTER && m_ServerNavigation.Is_Loaded())
    {
        if (CMonsterBrain::Advance_Knockback(entity, m_ServerNavigation, m_ServerCollisionSystem, fixedDeltaSeconds))
        {
            (void)m_ServerCollisionSystem.Update_BlockingBody(entity.iNetEntityId,
                                                              entity.fPositionX,
                                                              entity.fPositionY + entity.fCollisionRadius,
                                                              entity.fPositionZ);
            return true;
        }
        m_MonsterBrain.Update(entity,
                              m_Players,
                              m_GameplayCatalog,
                              m_ServerNavigation,
                              m_ServerCollisionSystem,
                              fixedDeltaSeconds,
                              updateTick,
                              m_TickDamageEvents);
        /* Later entities in this deterministic vector order collide against
		the position accepted earlier in the same fixed tick. */
        (void)m_ServerCollisionSystem.Update_BlockingBody(entity.iNetEntityId,
                                                          entity.fPositionX,
                                                          entity.fPositionY + entity.fCollisionRadius,
                                                          entity.fPositionZ);
    }

    return true;
}

//사망 보스를 제거하기 전에 기존 보상 latch와 MVP 원장을 한 번만 확정
void LostArk::Server::CGameRoom::Grant_ValtanRaidClearRewards()
{
    for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
    {
        // Only the Product arena's primary Valtan completes the raid. Dependent
        // ghost bosses share BOSS runtime state, and Character Select can spawn a
        // Valtan audition, but neither is a reward authority. The entity latch
        // prevents the retained DEAD presentation from granting again.
        if (LostArk::Shared::WORLD_ID::VALTAN_ARENA != m_eWorldId || WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind ||
            LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId ||
            "BOSS_VALTAN" != entity.strArchetypeId || "boss.valtan.center" != entity.strPlacementId ||
            SERVER_ENTITY_ACTION::DEAD != entity.eAction || entity.bLootGranted)
        {
            continue;
        }
        entity.bLootGranted = true;
        m_bValtanRaidCleared = true;
        m_GateMvpLedger.clear();
        Merge_MvpLedger(entity);
        Broadcast_RaidMvpResult(1u);
        for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
        {
            const auto playerIter = m_Players.find(playerId);
            if (playerIter == m_Players.end())
            {
                continue;
            }
            for (const std::string& itemId : m_ValtanClearRewards.Get_ItemIds(playerIter->second.eCharacterClass))
            {
                (void)Grant_Item(playerIter->second, itemId, 1u);
            }
            const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
            if (nullptr != session && !Send_InventorySnapshot(session, 0u, playerIter->second))
            {
                session->Request_Close();
            }
        }
    }
}

//종료 개체의 부착·공격 객체·복제를 정리한 뒤 vector에서 제거
//lifecycle 송신 실패는 현재 원소를 지우지 않고 기존 룸 실패 정책으로 중단한다.
bool LostArk::Server::CGameRoom::Despawn_FinishedWorldEntities(const std::uint32_t updateTick)
{
    for (auto iter = m_WorldEntities.begin(); iter != m_WorldEntities.end();)
    {
        const bool shouldDespawn =
            (WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind &&
             (0u == iter->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == iter->eAction)) ||
            (WORLD_BOOTSTRAP_KIND::MONSTER == iter->eKind && SERVER_ENTITY_ACTION::DEAD == iter->eAction &&
             iter->fActionElapsedSeconds * 1000.f >= static_cast<float>(iter->iDeadDespawnMs)) ||
            (iter->isEstherSummon && iter->fActionElapsedSeconds * 1000.f >= static_cast<float>(iter->iEstherStrikeMs));
        if (!shouldDespawn)
        {
            ++iter;
            continue;
        }
        if (WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind)
        {
            /* A gate boss's award ledger goes to the room before the clear is decided. */
            if (Gate_IndexOfPlacement(iter->strPlacementId) >= 0)
            {
                Merge_MvpLedger(*iter);
            }
            Notify_KoukuRaidBossDeath(*iter, updateTick);
            if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId && "ENCOUNTER_VALTAN" == iter->strEncounterId &&
                "BOSS_VALTAN" == iter->strArchetypeId)
            {
                GameRoomDetail::Clear_ValtanGhostRelocationState(*iter);
            }
            (void)Release_PlayerAttachments(iter->iNetEntityId, 0.f, 0u, false, 0u, updateTick);
        }
        m_CombatObjectRuntime.Cancel_Source(iter->iNetEntityId);
        if (!Broadcast_CombatObjectLifecycle())
        {
            Mark_RuntimeFailure("world-update.dead-entity-combat-object-lifecycle");
            return false;
        }
        Broadcast_WorldEntityDespawned(iter->iNetEntityId,
                                       WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind
                                           ? LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::DEAD
                                           : LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
        /* Gate progress: the last primary boss of a gate dying is that gate's clear. */
        if (WORLD_BOOTSTRAP_KIND::BOSS == iter->eKind)
        {
            Notify_GateBossDeath(*iter);
        }
        iter = m_WorldEntities.erase(iter);
    }
    return true;
}
