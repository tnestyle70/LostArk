> 2026-10-14 재개: 사용자가 다른 세션 완료를 확인했다. 적용 대상 12개 경로를 최신 디스크와 대조했으며 기존 파일은 보관한 before와 byte가 같고 신규 파일은 없다. 아래 후보는 이 최신 상태 위에 반영하며, 다른 파일의 미커밋 변경은 보존한다. 빌드·검증의 완료 여부는 RESULT가 정본이다.

# G0. 보스 시뮬레이션 책임 분리

2026-10-14. 사용자가 구조 정리는 에이전트에게 맡기고 패턴은 직접 구현하기로 선택했다.
이 문서는 실제 적용 전에 준비한 전체 코드다. 적용·빌드·회귀 결과는 같은 이름의 RESULT에 기록한다.
기존 3,184행 파일의 함수 14개를 책임별 여섯 CPP로 배치하고, 999행 Update_WorldEntities를 조율 함수와 private helper로 분리한다.
구조 정리는 이 경계부터 진행하며 Server·Engine 전체 정비를 완료한 것으로 취급하지 않는다.

진짜 세이튼·조커 랜덤, 잡기 피해, 피자 bundle 넉백, 발탄 포탈 이펙트는 이번에 동작을 바꾸지 않는다.
Q/W/E/R/A/S와 빙고 Space 기믹은 사용자 요청으로 보류한다. 기존 원작 연결 자료가 확인될 때만 추출한다.

## 파일과 불변식

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp | 룸의 한 tick 순서만 조율한다. 진입 준비 → 부속 보스 → 개체별 갱신 → 보상·제거 → 기믹 확정 → 부속 보스 순서를 보존한다. |
| 추가 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_WorldEntitySimulation.cpp | NPC·에스더·쿠크·발탄·일반 몬스터의 개체별 분기와 순회 후 보상·제거를 맡는다. |
| 추가 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_ValtanSimulation.cpp | 발탄의 pinned catalog, Brain 결과와 stage 진입 transaction, 이동을 연결한다. |
| 추가 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_ValtanGhost.cpp | 유령 단계 재배치·포탈 scheduler와 부속 보스 수명을 묶는다. |
| 추가 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuTargeting.cpp | 쿠크 플레이어 표식, 랜덤 발사와 추적 투사체의 수명·타격을 묶는다. |
| 추가 | C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuMechanics.cpp | 공중 이동 확정, 시간 기믹, 소환·객체 접촉, 진짜·가짜 세이튼을 묶는다. |
| 수정 | C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h | CGameRoom의 private helper 여섯 개를 선언한다. 새 상태·public API·manager를 추가하지 않는다. |
| 수정 | C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj | 새 CPP 다섯 개 등록. 기존 항목·필터 순서 보존. |
| 수정 | C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj.filters | 새 CPP 다섯 개 등록. 기존 항목·필터 순서 보존. |

기존 state owner는 CGameRoom 하나이며 룸 worker가 순서대로 접근한다. 네트워크·JSON·publisher·프로토콜은 바뀌지 않는다.
기존 loop의 continue는 개체 helper의 true로, 전체 갱신 return은 false로 옮긴다.
내부 loop의 continue와 이동 lambda의 void return은 유지한다. entity vector의 생성·제거 경계, stage rollback,
부착 해제, 보상 후 despawn 순서를 바꾸지 않는다. 새 캐시·멤버 변수·병렬 실행은 없다.
기존 주석은 보존하고 짧은 한국어 책임·수명 주석과 Allman 중괄호·네 칸 들여쓰기를 적용한다.

## 파일별 설명과 전체 코드

### G0-1. Server/Private/GameRoom_BossSimulation.cpp

변경 종류: 수정. 적용 위치: 해당 파일 전체.

룸의 한 tick 순서만 조율한다. 진입 준비 → 부속 보스 → 개체별 갱신 → 보상·제거 → 기믹 확정 → 부속 보스 순서를 보존한다.

Update_WorldEntities는 updateTick을 한 번 계산하고 개체 갱신이 false면 즉시 끝낸다. vector를 순회한 뒤 생성·제거 경계를 확정한다. std::uint32_t와 tick wrap 계산에 cstdint·limits를 직접 포함한다.

```cpp
#include "GameRoom.h"

#include <cstdint>
#include <limits>

//룸 worker의 고정 tick 진입점. 순회 중 entity 저장소를 늘리거나 지우지 않고
//생성·제거를 앞뒤 경계에서 확정한다. 실패한 하위 갱신 이후 단계는 실행하지 않는다.
void LostArk::Server::CGameRoom::Update_WorldEntities(const float fixedDeltaSeconds)
{
    const std::uint32_t updateTick =
        (std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;

    //플레이어 진입·이전 패턴 잔여 객체를 먼저 확정한 뒤 본체 순회
    // Idempotent for direct simulation callers; normal ticks already prepared before players.
    Prepare_KoukuAuditionTick(updateTick);
    // Commit portal contact before child attacks mutate the contacting player.
    // This boundary is outside entity iteration because entry may spawn stage objects.
    Commit_KoukuMarioEntries();
    Update_KoukuPatternTails(updateTick);
    Update_KoukuGazeClones(updateTick);

    if (!Update_DependentBosses(updateTick))
    {
        Mark_RuntimeFailure("world-update.dependent-bosses-before-primary");
        return;
    }

    Update_PendingEstherSummons(fixedDeltaSeconds);
    Update_EstherZones(updateTick);

    //차용한 entity 참조는 이번 호출 동안만 사용. vector 순서와 조기 실패 유지
    for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
    {
        if (!Update_WorldEntity(entity, fixedDeltaSeconds, updateTick))
        {
            return;
        }
    }

    Tick_MvpLedgers();
    Grant_ValtanRaidClearRewards();

    if (!Despawn_FinishedWorldEntities(updateTick))
    {
        return;
    }

    //순회 종료 뒤 순간이동·분신 생성 확정. 생성으로 무효화될 참조를 보관하지 않는다.
    Commit_KoukuMechanicTriggers(updateTick);

    if (!Update_DependentBosses(updateTick))
    {
        Mark_RuntimeFailure("world-update.dependent-bosses-after-primary");
    }
}
```

### G0-2. Server/Private/GameRoom_WorldEntitySimulation.cpp

변경 종류: 추가. 적용 위치: 해당 파일 전체.

NPC·에스더·쿠크·발탄·일반 몬스터의 개체별 분기와 순회 후 보상·제거를 맡는다.

Update_WorldEntity는 true로 정상 처리·건너뜀을, false로 전체 tick 중단을 알린다. Grant_ValtanRaidClearRewards는 기존 보상 loop를 실행한다. Despawn_FinishedWorldEntities는 기존 제거 조건·collision 정리·broadcast 순서를 보존하며 실패를 호출자에게 반환한다. ClientSession은 에스더 종료 broadcast의 session 소비, WorldCollisionContract는 NPC blocking body 중심 높이에 필요하다.

```cpp
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
```

### G0-3. Server/Private/GameRoom_ValtanSimulation.cpp

변경 종류: 추가. 적용 위치: 해당 파일 전체.

발탄의 pinned catalog, Brain 결과와 stage 진입 transaction, 이동을 연결한다.

Update_ValtanWorldEntity는 기존 발탄 분기를 그대로 책임진다. Update_ValtanStageMotion은 현재 stage 이동의 제안 위치·navigation·충돌 반영을 처리한다. Release_WorldBossAttachments는 원래 지역 lambda의 보스 identity 검증과 부착 해제를 보존한다. GameRoom_Internal은 거리·각도·tick helper를 제공하며, occurrenceCatalog는 실행 중 고정된 revision의 차용 참조다. stage transaction은 rollback 순서가 하나의 계약이므로 임의로 더 쪼개지 않는다.

```cpp
#include "GameRoom.h"
#include "GameRoom_Internal.h"

#include "BossCombatRuntime.h"
#include "Gameplay/CombatCollisionContract.h"
#include "ValtanBrain.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using namespace GameRoomDetail;

//발탄 stage 전환 실패·종료에서 해당 보스가 소유한 부착만 해제
void LostArk::Server::CGameRoom::Release_WorldBossAttachments(const SERVER_WORLD_ENTITY& boss,
                                                              const std::uint32_t updateTick)
{
    if (WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind && LostArk::Shared::INVALID_NET_ENTITY_ID != boss.iNetEntityId)
    {
        (void)Release_PlayerAttachments(boss.iNetEntityId, 0.f, 0u, false, 0u, updateTick);
    }
}

//본체 순회가 호출하는 발탄 한 개체 갱신. Brain의 제안 상태를 stage transaction과 연결하며
//실패 시 원래 rollback·부착 보존 정책을 적용하고 false로 이후 개체 처리를 중단한다.
bool LostArk::Server::CGameRoom::Update_ValtanWorldEntity(SERVER_WORLD_ENTITY& entity,
                                                          const float fixedDeltaSeconds,
                                                          const std::uint32_t updateTick)
{
    //실행 시작 revision의 정의를 차용. 갱신된 Catalog로 실행 중 규칙을 바꾸지 않는다.
    const CGameplayCatalog* occurrenceCatalog = Resolve_ValtanGameplayCatalog(entity);
    if (nullptr == occurrenceCatalog)
    {
        m_strStatus = "Valtan occurrence pinned gameplay generation is missing";
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.valtan-pinned-generation");
        return false;
    }
    //유령 재등장 대기 중에는 Brain을 멈추고 소유한 숨김·무적 latch만 정리
    bool updateValtanBrain = true;
    const bool ghostRelocationOwnerAlive =
        entity.bGhostPhasePatternLoopActive && LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
        "BOSS_VALTAN" == entity.strArchetypeId && 3u == entity.iPhase && 0u != entity.iCurrentHp &&
        SERVER_ENTITY_ACTION::DEAD != entity.eAction && entity.strPatternId.empty();
    if ((entity.bGhostRepositionPending || entity.bGhostRelocationRetryPending) && !ghostRelocationOwnerAlive)
    {
        /* Death, reset and identity replacement are ordinary cancellation
		edges. Cleanup is ownership-aware and never makes the room fatal. */
        Clear_ValtanGhostRelocationState(entity);
    }
    if (entity.bGhostRelocationRetryPending)
    {
        updateValtanBrain = false;
        if (Has_ReachedServerTick(updateTick, entity.iGhostRelocationRetryTick) &&
            !Begin_ValtanGhostRelocation(entity, *occurrenceCatalog, updateTick))
        {
            /* A transient definition/flag conflict retains the visible pose and
			retries. Unknown state does not clear another mechanic's flag. */
            entity.bGhostRelocationRetryPending = true;
            entity.iGhostRelocationRetryTick = Add_ServerTicksSkippingReservedZero(updateTick, 1u);
            m_strStatus = "Valtan ghost relocation retry deferred";
        }
    }
    if (entity.bGhostRepositionPending)
    {
        const bool relocationLatchValid =
            ghostRelocationOwnerAlive && entity.bGhostRelocationOwnsInvulnerableFlag &&
            entity.bGhostRelocationOwnsHiddenFlag &&
            CBossCombatRuntime::Has_Flag(entity.BossCombat, SERVER_BOSS_COMBAT_FLAG::INVULNERABLE) &&
            CBossCombatRuntime::Has_Flag(entity.BossCombat, SERVER_BOSS_COMBAT_FLAG::GHOST_HIDDEN) &&
            0u != entity.iGhostReappearTick;
        if (!relocationLatchValid)
        {
            Clear_ValtanGhostRelocationState(entity);
            if (ghostRelocationOwnerAlive)
            {
                entity.bGhostRelocationRetryPending = true;
                entity.iGhostRelocationRetryTick = Add_ServerTicksSkippingReservedZero(updateTick, 1u);
                updateValtanBrain = false;
            }
            m_strStatus = "Valtan ghost relocation latch was safely cancelled";
        }
        else if (Has_ReachedServerTick(updateTick, entity.iGhostReappearTick))
        {
            Clear_ValtanGhostRelocationState(entity);
            updateValtanBrain = true;
        }
        else
        {
            /* The completion tick already published the hidden pose. Do not let
			the ordered selector start another pattern until the next tick clears
			the render/invulnerability edge. */
            updateValtanBrain = false;
        }
    }
    if (updateValtanBrain && !Prepare_ValtanPatternIdAuditionBeforeBrain(entity))
    {
        return true;
    }
    const std::uint32_t previousPatternSequence = entity.iPatternSequence;
    const std::uint32_t previousStageIndex = entity.iPatternStageIndex;
    const std::uint32_t previousActionStartTick = entity.iActionStartTick;
    const std::uint32_t previousAppliedPatternHitCount = entity.iAppliedPatternHitCount;
    const std::string previousPatternId = entity.strPatternId;
    const std::string previousStageId = entity.strPatternStageId;
    const std::string previousActionId = entity.strActionId;
    const LostArk::Shared::GameplayDataRevision previousDefinitionRevision = entity.PinnedDefinitionRevision;
    /* The Brain resolves the next stage into the boss value first. Keep the
	old value detached until every EXIT/ENTER action has passed preflight;
	on failure the stage graph rolls back while the typed mechanic ledger
	retains FAILED_REQUIRES_RESET. */
    //stage 전환이 실패하면 되돌릴 본체 값. mechanic 실패 기록은 별도로 보존
    const SERVER_WORLD_ENTITY bossBeforeBrain = entity;
    /* Where the body was before the brain moved it. The segment between
	the two is what actually touched a wall this tick, in any pattern and
	while idle, so a fast charge cannot step over a slab. */
    const float contactStartX = entity.fPositionX;
    const float contactStartY = entity.fPositionY;
    const float contactStartZ = entity.fPositionZ;
    if (updateValtanBrain)
    {
        updateValtanBrain = Prepare_ValtanFightPageBeforeBrain(entity, updateTick);
    }
    if (updateValtanBrain)
    {
        updateValtanBrain = Prepare_ValtanTimelineRowBeforeBrain(entity, updateTick);
    }
    if (updateValtanBrain)
    {
        std::vector<SERVER_PLAYER_CAPTURE_REQUEST> captureRequests;
        const BOSS_PATTERN_SEQUENCE_DEFINITION* patternFlowSequence = nullptr;
        patternFlowSequence = Resolve_ValtanPatternFlowSequence(entity);
        if (LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId)
        {
            patternFlowSequence = &entity.DependentPatternSequence;
        }
        else if (nullptr == patternFlowSequence && entity.bGhostPhasePatternLoopActive)
        {
            patternFlowSequence = &entity.GhostPhasePatternSequence;
        }
        /* Only a stele that is standing right now is cover. A slot that
		   is breaking or hidden stops answering the blow on the same
		   tick the Server retired it. */
        //이번 보스의 유효한 엄폐물만 모아 Brain의 공격 판정에 전달
        std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ> coverCircles;
        if (m_EncounterPropRuntime.Is_Initialized())
        {
            const float coverRadius = m_EncounterPropRuntime.Get_CoverRadiusMeters();
            for (const ENCOUNTER_PROP_SLOT_STATE& slot : m_EncounterPropRuntime.Get_SlotStates())
            {
                if (LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT != slot.eState)
                {
                    continue;
                }
                coverCircles.push_back({slot.fPositionX, slot.fPositionZ, coverRadius});
            }
        }
        /* Authored fixed combat objects become cover only until their first
		   timed damage pulse. Restrict the lookup to this boss occurrence so
		   another encounter entity cannot contribute foreign cover. */
        for (const SERVER_COMBAT_OBJECT& combatObject : m_CombatObjectRuntime.Get_LiveObjects())
        {
            if (combatObject.eSourceKind != SERVER_COMBAT_OBJECT_SOURCE_KIND::WORLD_ENTITY ||
                combatObject.iSourceNetEntityId != entity.iNetEntityId || !std::isfinite(combatObject.fCoverRadiusM) ||
                combatObject.fCoverRadiusM <= 0.f)
            {
                continue;
            }
            bool pendingTimedDamage = false;
            for (const SERVER_COMBAT_OBJECT_HIT_RUNTIME& hit : combatObject.Hits)
            {
                if (SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED == hit.eTrigger && 0u == hit.iAppliedTimedCount &&
                    (!combatObject.OwnerHitChain.strTriggerActionId.empty() ||
                     combatObject.fElapsedMilliseconds < static_cast<float>(hit.iAtMs)))
                {
                    pendingTimedDamage = true;
                    break;
                }
            }
            if (pendingTimedDamage)
            {
                coverCircles.push_back({combatObject.LiveState.CurrentPose.fPositionX,
                                        combatObject.LiveState.CurrentPose.fPositionZ,
                                        combatObject.fCoverRadiusM});
            }
        }
        //본체와 종속 유령은 기존 Brain을 선택하고 피해·잡기 요청은 룸이 적용
        CValtanBrain& brain = LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId
                                  ? m_ValtanBrain
                                  : *m_DependentValtanBrain;
        std::vector<SERVER_BOSS_PATTERN_HIT> ownerHits;
        brain.Update(entity,
                     m_Players,
                     *occurrenceCatalog,
                     m_ServerNavigation,
                     fixedDeltaSeconds,
                     updateTick,
                     coverCircles,
                     m_TickDamageEvents,
                     &m_GameplayCatalog.Active(),
                     m_GameplayCatalog.Get_ActiveGenerationEpoch(),
                     &captureRequests,
                     patternFlowSequence,
                     &ownerHits);
        if (!ownerHits.empty())
        {
            m_CombatObjectRuntime.Apply_OwnerHits(ownerHits,
                                                  m_Players,
                                                  m_WorldEntities,
                                                  *occurrenceCatalog,
                                                  updateTick,
                                                  m_TickDamageEvents);
        }
        for (const SERVER_PLAYER_CAPTURE_REQUEST& request : captureRequests)
        {
            (void)Capture_PlayerAttachment(request.iPlayerNetEntityId,
                                           entity.iNetEntityId,
                                           request.eAttachmentSlot,
                                           updateTick);
        }
        if (SERVER_ENTITY_ACTION::DEAD == entity.eAction || 0u == entity.iCurrentHp)
        {
            Clear_ValtanGhostRelocationState(entity);
            (void)Release_PlayerAttachments(entity.iNetEntityId, 0.f, 0u, false, 0u, updateTick);
        }
        const VALTAN_DECISION_TRACE* latestTrace = m_ValtanBrain.Get_LatestDecisionTrace();
        if (LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId && nullptr != latestTrace &&
            latestTrace->iTraceSequence != m_ValtanDecisionTraceRevision.iTraceSequence)
        {
            m_ValtanDecisionTraceRevision.iBossEntityId = entity.iNetEntityId;
            m_ValtanDecisionTraceRevision.strBossPlacementId = entity.strPlacementId;
            m_ValtanDecisionTraceRevision.iTraceSequence = latestTrace->iTraceSequence;
            m_ValtanDecisionTraceRevision.DefinitionRevision = occurrenceCatalog->Get_ActiveRevision();
        }
    }
    if (!previousPatternId.empty() && entity.strPatternId.empty() &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult)
    {
        const auto& objects = m_CombatObjectRuntime.Get_LiveObjects();
        const bool independentEffectStillRunning =
            std::any_of(objects.begin(),
                        objects.end(),
                        [&entity, &previousPatternId, previousPatternSequence](const SERVER_COMBAT_OBJECT& object)
                        {
                            return object.iSourceNetEntityId == entity.iNetEntityId &&
                                   object.LiveState.strOwnerPatternId == previousPatternId &&
                                   object.LiveState.iOwnerPatternSequence == previousPatternSequence &&
                                   object.fRemainingMilliseconds > 0.f;
                        });
        /* The object owns its tail clock and immutable birth pose. A completed
		foreground must not insert a pursuit hold before its successor. */
        if (independentEffectStillRunning && !entity.PendingPatternFollowup.Is_Pending() &&
            0u == entity.iPatternFollowupDepth)
        {
            entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
        }
    }
    //종료 영수증이 현재 실행과 일치할 때만 발악 반복을 이어 붙임
    bool finaleCycleRestarted = false;
    if (updateValtanBrain && LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
        !previousPatternId.empty() && entity.strPatternId.empty() && 0u != entity.iCurrentHp &&
        !entity.bMechanicLedgerRequiresReset &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult &&
        entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence)
    {
        const auto* definitions = occurrenceCatalog->Find_BossPatterns(entity.strEncounterId);
        const auto finale = nullptr == definitions ? nullptr : [&]() -> const BOSS_PATTERN_DEFINITION*
        {
            const auto found = std::find_if(definitions->begin(),
                                            definitions->end(),
                                            [&previousPatternId](const BOSS_PATTERN_DEFINITION& definition)
                                            {
                                                return definition.strPatternId == previousPatternId;
                                            });
            return found == definitions->end() ? nullptr : &*found;
        }();
        bool stopRequested = entity.bAutomaticPatternSequenceAuditionHold;
        stopRequested =
            stopRequested || (m_ValtanNextPattern && m_ValtanNextPattern->iBossEntityId == entity.iNetEntityId) ||
            (Is_ValtanPatternFlowRunning() && m_ValtanPatternFlowAudition.iBossEntityId == entity.iNetEntityId &&
             m_ValtanPatternFlowAudition.bStopAfterCurrent);
        if (!stopRequested && nullptr != finale && BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == finale->Finale.eKind)
        {
            entity.iRotationStepIndex = bossBeforeBrain.iRotationStepIndex;
            entity.bAutomaticPatternSequenceStepRunning = bossBeforeBrain.bAutomaticPatternSequenceStepRunning;
            entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
            if (!m_ValtanBrain.Restart_FinaleCycle(entity, m_Players, *finale, updateTick))
            {
                entity = bossBeforeBrain;
                m_strStatus = "Finale cycle identity space is exhausted";
                Mark_RuntimeFailure("world-update.finale-cycle-restart");
                return false;
            }
            for (auto& mechanic : entity.MechanicOccurrences)
            {
                if (mechanic.strPatternId != previousPatternId)
                {
                    continue;
                }
                mechanic.eState = SERVER_BOSS_MECHANIC_STATE::ACTIVE;
                mechanic.iPatternSequence = entity.iPatternSequence;
                mechanic.iFinishedTick = 0u;
            }
            finaleCycleRestarted = true;
        }
    }
    (void)finaleCycleRestarted;
    //stage identity 변화와 부활 대기의 시계 보정은 서로 다른 전이
    const bool stageIdentityChanged =
        previousPatternSequence != entity.iPatternSequence || previousStageIndex != entity.iPatternStageIndex ||
        previousPatternId != entity.strPatternId || previousActionId != entity.strActionId;
    const bool pauseClockOnlyChanged =
        !stageIdentityChanged && previousStageId == entity.strPatternStageId &&
        previousActionStartTick != entity.iActionStartTick &&
        (bossBeforeBrain.bAutomaticPatternSequencePausedForRevive || entity.bAutomaticPatternSequencePausedForRevive);
    const bool stageChanged = stageIdentityChanged || previousStageId != entity.strPatternStageId ||
                              (previousActionStartTick != entity.iActionStartTick && !pauseClockOnlyChanged);
    const auto hasGrabbedPlayerStageAction = [this, &entity](const std::string& patternId,
                                                             const std::string& actionId,
                                                             const LostArk::Shared::GameplayDataRevision& revision,
                                                             const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger)
    {
        if (patternId.empty() || actionId.empty())
        {
            return false;
        }
        const CGameplayCatalog* ownerCatalog = m_GameplayCatalog.Resolve(revision);
        const auto* definitions =
            nullptr == ownerCatalog ? nullptr : ownerCatalog->Find_BossPatterns(entity.strEncounterId);
        if (nullptr == definitions)
        {
            return false;
        }
        for (const auto& definition : *definitions)
        {
            if (definition.strPatternId != patternId)
            {
                continue;
            }
            for (const auto& stage : definition.Stages)
            {
                if (stage.strActionId != actionId)
                {
                    continue;
                }
                return std::any_of(stage.Actions.begin(),
                                   stage.Actions.end(),
                                   [trigger](const BOSS_PATTERN_STAGE_ACTION& action)
                                   {
                                       return action.eTrigger == trigger &&
                                              (BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS ==
                                                   action.eKind ||
                                               BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
                                               BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind);
                                   });
            }
        }
        return false;
    };
    /* Both sides use the same pinned catalogs and trigger boundary as the
	transaction. Failed EXIT release must preserve attachments even when
	FinishPattern has already cleared the proposed next identity. */
    const bool preserveGrabbedPlayersOnFailure =
        stageIdentityChanged && (hasGrabbedPlayerStageAction(previousPatternId,
                                                             previousActionId,
                                                             previousDefinitionRevision,
                                                             BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT) ||
                                 hasGrabbedPlayerStageAction(entity.strPatternId,
                                                             entity.strActionId,
                                                             entity.PinnedDefinitionRevision,
                                                             BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER));
    if (stageIdentityChanged && !Apply_BossPatternStageTransition(entity,
                                                                  previousPatternId,
                                                                  previousActionId,
                                                                  entity.strPatternId,
                                                                  entity.strActionId,
                                                                  previousDefinitionRevision,
                                                                  entity.PinnedDefinitionRevision,
                                                                  updateTick))
    {
        auto mechanicOccurrences = std::move(entity.MechanicOccurrences);
        auto pendingPatternIds = std::move(entity.PendingPatternIds);
        auto triggeredPatternIds = std::move(entity.TriggeredPatternIds);
        const bool mechanicResetRequired = entity.bMechanicLedgerRequiresReset;
        const std::uint32_t lastEvaluatedHealthBar = entity.iLastEvaluatedHealthBar;
        const bool restorePatternVerticalOffsetAfterRollback =
            bossBeforeBrain.bPatternVerticalOffsetApplied && std::isfinite(bossBeforeBrain.fPatternVerticalBaseY);
        const bool restoreStageVerticalOffsetAfterRollback = bossBeforeBrain.bPatternStageVerticalOffsetApplied &&
                                                             std::isfinite(bossBeforeBrain.fPatternStageVerticalBaseY);
        const bool restoreVerticalOffsetAfterRollback =
            restorePatternVerticalOffsetAfterRollback || restoreStageVerticalOffsetAfterRollback;
        const float restoredVerticalBaseY = restorePatternVerticalOffsetAfterRollback
                                                ? bossBeforeBrain.fPatternVerticalBaseY
                                                : bossBeforeBrain.fPatternStageVerticalBaseY;
        if (!preserveGrabbedPlayersOnFailure)
        {
            Release_WorldBossAttachments(entity, updateTick);
        }
        entity = bossBeforeBrain;
        /* Apply_BossPatternStageTransition already classified the occurrence as
		   aborted and restored its typed vertical offset. Do not let this
		   transaction rollback resurrect the preflight pose at Y+offset. */
        if (restoreVerticalOffsetAfterRollback)
        {
            entity.fPositionY = restoredVerticalBaseY;
            entity.bPatternVerticalOffsetApplied = false;
            entity.fPatternVerticalBaseY = 0.f;
            entity.bPatternStageVerticalOffsetApplied = false;
            entity.fPatternStageVerticalBaseY = 0.f;
        }
        entity.MechanicOccurrences = std::move(mechanicOccurrences);
        entity.PendingPatternIds = std::move(pendingPatternIds);
        entity.TriggeredPatternIds = std::move(triggeredPatternIds);
        entity.bMechanicLedgerRequiresReset = mechanicResetRequired;
        entity.iLastEvaluatedHealthBar = lastEvaluatedHealthBar;
        Mark_RuntimeFailure("world-update.pattern-stage-transition");
        return false;
    }
    //부활 패턴 완료 뒤 유령 반복을 활성화하고 다음 순번을 정리
    const bool completedRespawn =
        stageIdentityChanged && LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
        LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId &&
        "BOSS_VALTAN" == entity.strArchetypeId && "boss.valtan.center" == entity.strPlacementId &&
        "VALTAN_GHOST_RESPAWN_AUDITION" == previousPatternId && entity.strPatternId.empty() &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult &&
        entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence;
    if (completedRespawn && !Activate_ValtanGhostPhaseLoop(entity, *occurrenceCatalog))
    {
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.ghost-phase-loop-activation");
        return false;
    }
    const bool completedDirectGhostLoopStep =
        entity.bGhostPhasePatternLoopActive && !previousPatternId.empty() && entity.strPatternId.empty() &&
        !entity.PendingPatternFollowup.Is_Pending() &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult &&
        entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence &&
        std::find(entity.GhostPhasePatternSequence.PatternIds.begin(),
                  entity.GhostPhasePatternSequence.PatternIds.end(),
                  previousPatternId) != entity.GhostPhasePatternSequence.PatternIds.end();
    const bool completedGhostLoopOutcomeGroup =
        entity.bGhostPhasePatternLoopActive && !previousPatternId.empty() && entity.strPatternId.empty() &&
        !entity.PendingPatternFollowup.Is_Pending() && bossBeforeBrain.iPatternFollowupDepth > 0u &&
        0u != bossBeforeBrain.iPatternFollowupRootSequence && bossBeforeBrain.iRotationStepIndex > 0u &&
        bossBeforeBrain.iRotationStepIndex <= bossBeforeBrain.GhostPhasePatternSequence.PatternIds.size() &&
        !bossBeforeBrain.bAutomaticPatternSequenceStepRunning &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult &&
        entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence &&
        entity.PatternTerminalReceipt.iRootPatternSequence == bossBeforeBrain.iPatternFollowupRootSequence;
    const bool completedGhostLoopStep = completedDirectGhostLoopStep || completedGhostLoopOutcomeGroup;
    if (completedGhostLoopStep)
    {
        entity.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
        if (entity.iRotationStepIndex >= entity.GhostPhasePatternSequence.PatternIds.size())
        {
            entity.iRotationStepIndex = 0u;
        }
    }
    if (!previousPatternId.empty() && previousPatternId != entity.strPatternId)
    {
        /* A pattern owns every attachment it captured. Even a catalog-corrupt
		exit without its authored release action cannot leak that ownership
		into the next pattern. */
        Release_WorldBossAttachments(entity, updateTick);
    }
    if (!previousPatternId.empty() && SERVER_ENTITY_ACTION::IDLE != bossBeforeBrain.eAction &&
        SERVER_ENTITY_ACTION::IDLE == entity.eAction)
    {
        /* A mechanic-reset latch can abort motion without changing the stage
		identity. Attachment cleanup follows the action abort edge as well. */
        Release_WorldBossAttachments(entity, updateTick);
    }
    //stage 변경 여부에 따라 예약 소환·기둥·벽 진입을 원래 순서로 확정
    if (!stageChanged && updateValtanBrain && !Apply_BossPatternScheduledSpawnWave(entity, updateTick))
    {
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.pattern-scheduled-spawn-wave");
        return false;
    }
    if (stageChanged && !Apply_EncounterPropStageEntry(entity, updateTick))
    {
        CValtanBrain::Fail_ActiveMechanic(entity, SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, updateTick);
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.encounter-prop-stage-entry");
        return false;
    }
    if (stageChanged && !Apply_WorldDestructionStageEntry(entity, updateTick))
    {
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.world-destruction-stage-entry");
        return false;
    }
    /* A charge owns one swept wall transaction below. Letting the stationary
	   body-contact pass run first can break every overlapping wall box before
	   the first surface chooses its single impact/contact mutation. */
    if (!entity.bPatternChargeImpact && !entity.bPortalMotionActive && !bossBeforeBrain.bPortalMotionActive &&
        !Apply_WorldDestructionBodyContact(entity, contactStartX, contactStartY, contactStartZ, updateTick))
    {
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.world-destruction-body-contact");
        return false;
    }
    /* A damage pulse is evaluated by the Brain at the same fixed tick as
	   the axe proxy. Only stages compiled in the wall-contact allowlist set
	   bPatternWallContact, so roars, waves and magic never reach here. */
    if (entity.bPatternWallContact && entity.iAppliedPatternHitCount > previousAppliedPatternHitCount &&
        !Apply_WorldDestructionPatternHitContact(entity, updateTick))
    {
        Release_WorldBossAttachments(entity, updateTick);
        Mark_RuntimeFailure("world-update.world-destruction-pattern-hit");
        return false;
    }
    //돌진의 벽 접촉·지형 파괴·후속 stage 확정을 같은 순서로 완료
    if (!Update_ValtanStageMotion(entity, *occurrenceCatalog, fixedDeltaSeconds, updateTick))
    {
        return false;
    }
    if (finaleCycleRestarted && m_ValtanPatternIdAudition.iBossEntityId == entity.iNetEntityId &&
        VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == m_ValtanPatternIdAudition.ePhase &&
        m_ValtanPatternIdAudition.iExpectedPatternSequence == previousPatternSequence)
    {
        m_ValtanPatternIdAudition.iExpectedPatternSequence = entity.iPatternSequence;
        Queue_ValtanPatternIdAuditionLifecycle(LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
    }
    if (updateValtanBrain)
    {
        Restore_ValtanTimelineRowAfterBrain(entity, updateTick);
    }
    if (updateValtanBrain)
    {
        Refresh_ValtanPatternFlowState(entity);
    }
    /* Debug controllers consume the root-group receipt above. Product has no
	   controller, so retire the leaf-only bookkeeping at the same post-brain
	   seam; the terminal receipt keeps the immutable root identity. */
    if (entity.strPatternId.empty() && !entity.PendingPatternFollowup.Is_Pending() &&
        entity.iPatternFollowupDepth > 0u &&
        SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == entity.PatternTerminalReceipt.eResult &&
        entity.PatternTerminalReceipt.iPatternSequence == previousPatternSequence &&
        entity.PatternTerminalReceipt.iRootPatternSequence == entity.iPatternFollowupRootSequence)
    {
        entity.iPatternFollowupDepth = 0u;
        entity.iPatternFollowupRootSequence = 0u;
    }
    if (LostArk::Shared::INVALID_NET_ENTITY_ID == entity.iOwnerBossNetEntityId && entity.strPatternId.empty() &&
        occurrenceCatalog->Get_ActiveRevision() == m_GameplayCatalog.Get_ActiveRevision())
    {
        /* Publish the active identity only after the brain has observed it.
		If an old occurrence finished on this tick, retaining its pin for one
		more boundary makes the next active-catalog evaluation detectable. */
        entity.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision();
    }

    return true;
}

//Brain 갱신 뒤 예정 이동을 지형·벽 접촉에 반영. 기존 충돌 결과와 stage 전환 순서를 보존한다.
//catalog는 실행 revision에 고정된 정의의 차용이며 이 호출 밖으로 포인터를 저장하지 않는다.
bool LostArk::Server::CGameRoom::Update_ValtanStageMotion(SERVER_WORLD_ENTITY& entity,
                                                          const CGameplayCatalog& occurrenceCatalog,
                                                          const float fixedDeltaSeconds,
                                                          const std::uint32_t updateTick)
{
    //예정 이동을 바로 확정하지 않고 첫 벽 접촉과 stage 결과를 먼저 검사
    float proposedX = 0.f;
    float proposedZ = 0.f;
    if (!entity.bAutomaticPatternSequencePausedForRevive &&
        m_ValtanBrain.Try_BuildStageMotion(entity, fixedDeltaSeconds, proposedX, proposedZ))
    {
        const bool portalTargetRush =
            BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH == entity.ePatternStageMotionKind;
        const auto Take_MotionStep = [this, &entity, portalTargetRush](const float targetX, const float targetZ)
        {
            if (portalTargetRush)
            {
                entity.fPositionX = targetX;
                entity.fPositionZ = targetZ;
                return;
            }
            Resolve_NavigableStep(m_ServerNavigation,
                                  entity.fPositionX,
                                  entity.fPositionZ,
                                  targetX,
                                  targetZ,
                                  entity.fPositionX,
                                  entity.fPositionZ);
        };
        /* Dash Charge stops on the first authoritative collisionBox, including
		ordinary walls. Other authored charge-impact mechanics retain their
		exact receiver-only contract. */
        const bool dashStopsOnEveryWall = "VALTAN_DASH_CHARGE" == entity.strPatternId &&
                                          "CHARGE" == entity.strPatternStageId &&
                                          "valtan.attack.dash-charge.active" == entity.strActionId;
        SERVER_BOSS_WALL_HIT hit{};
        bool foundChargeWall = false;
        if (entity.bPatternChargeImpact && dashStopsOnEveryWall)
        {
            foundChargeWall = m_ServerCollisionSystem.Sweep_BossCircleAgainstWalls(entity.fPositionX,
                                                                                   entity.fPositionY,
                                                                                   entity.fPositionZ,
                                                                                   proposedX,
                                                                                   entity.fPositionY,
                                                                                   proposedZ,
                                                                                   entity.fCollisionRadius,
                                                                                   hit);
        }
        else if (entity.bPatternChargeImpact)
        {
            SERVER_BOSS_RECEIVER_HIT receiverHit{};
            foundChargeWall = m_ServerCollisionSystem.Sweep_BossCircleAgainstReceivers(entity.fPositionX,
                                                                                       entity.fPositionY,
                                                                                       entity.fPositionZ,
                                                                                       proposedX,
                                                                                       entity.fPositionY,
                                                                                       proposedZ,
                                                                                       entity.fCollisionRadius,
                                                                                       receiverHit);
            if (foundChargeWall)
            {
                hit.strCollisionPlacementId = receiverHit.strReceiverPlacementId;
                hit.strImpactReceiverPlacementId = receiverHit.strReceiverPlacementId;
                hit.fHitRatio = receiverHit.fHitRatio;
            }
        }
        if (foundChargeWall)
        {
            const float deltaX = proposedX - entity.fPositionX;
            const float deltaZ = proposedZ - entity.fPositionZ;
            const float distance = std::sqrt(deltaX * deltaX + deltaZ * deltaZ);
            const float marginRatio = distance > 0.000001f ? 0.001f / distance : 0.f;
            const float safeRatio = (std::max)(0.f, hit.fHitRatio - marginRatio);
            Take_MotionStep(entity.fPositionX + deltaX * safeRatio, entity.fPositionZ + deltaZ * safeRatio);
            //충돌 stage 전환 실패 시 위치·단계 복원에 사용할 값
            const SERVER_WORLD_ENTITY bossBeforeImpactTransition = entity;
            const std::string impactPreviousPatternId = entity.strPatternId;
            const std::string impactPreviousActionId = entity.strActionId;
            bool triggered = false;
            if (!hit.strImpactReceiverPlacementId.empty() &&
                !Apply_WorldDestructionImpact(entity, hit.strImpactReceiverPlacementId, updateTick, triggered))
            {
                Mark_RuntimeFailure("world-update.world-destruction-impact");
                return false;
            }
            if (dashStopsOnEveryWall && !triggered &&
                !Apply_WorldDestructionContacts(entity, {hit.strCollisionPlacementId}, updateTick))
            {
                Mark_RuntimeFailure("world-update.world-destruction-contact");
                return false;
            }
            if (!dashStopsOnEveryWall && !triggered)
            {
                entity.fPatternForcedMotionSpeed = 0.f;
            }
            /* A fresh receiver publishes WALL_CONTACT as part of its exact
			   mutation. Ordinary walls and already-consumed receiver surfaces
			   publish the same geometry outcome without manufacturing another
			   destruction transition. */
            const bool impactOutcomePublished =
                !dashStopsOnEveryWall || triggered ||
                CBossCombatRuntime::Publish_PatternOutcome(entity,
                                                           BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT,
                                                           updateTick);
            const CGameplayCatalog* impactCatalog = m_GameplayCatalog.Resolve(entity.PinnedDefinitionRevision);
            if ((dashStopsOnEveryWall || triggered) &&
                (!impactOutcomePublished || nullptr == impactCatalog ||
                 !m_ValtanBrain.Complete_ImpactStage(entity, *impactCatalog, updateTick) ||
                 !Apply_BossPatternStageTransition(entity,
                                                   impactPreviousPatternId,
                                                   impactPreviousActionId,
                                                   entity.strPatternId,
                                                   entity.strActionId,
                                                   bossBeforeImpactTransition.PinnedDefinitionRevision,
                                                   entity.PinnedDefinitionRevision,
                                                   updateTick) ||
                 !Apply_WorldDestructionStageEntry(entity, updateTick)))
            {
                auto mechanicOccurrences = std::move(entity.MechanicOccurrences);
                const bool mechanicResetRequired = entity.bMechanicLedgerRequiresReset;
                Release_WorldBossAttachments(entity, updateTick);
                entity = bossBeforeImpactTransition;
                entity.MechanicOccurrences = std::move(mechanicOccurrences);
                entity.bMechanicLedgerRequiresReset = mechanicResetRequired;
                m_strStatus = "Valtan wall-contact stage transition failed";
                Mark_RuntimeFailure("world-update.wall-contact-stage-transition");
                return false;
            }
            /* Complete_ImpactStage resolves the authored WALL_CONTACT branch;
			   GameRoom owns the shared wall-stop/destruction transaction order. */
        }
        else
        {
            Take_MotionStep(proposedX, proposedZ);
            const float yaw = entity.fYawDegrees * DEGREES_TO_RADIANS;
            const float probeDistance = (std::max)(0.1f, m_ServerNavigation.Get_CellSize());
            const bool blocked =
                !portalTargetRush &&
                (std::fabs(entity.fPositionX - proposedX) > 0.001f ||
                 std::fabs(entity.fPositionZ - proposedZ) > 0.001f ||
                 !m_ServerNavigation.Is_PointWalkableExact(entity.fPositionX + std::sin(yaw) * probeDistance,
                                                           entity.fPositionZ + std::cos(yaw) * probeDistance));
            if (blocked && BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == entity.ePatternPlayerResponse)
            {
                const SERVER_WORLD_ENTITY beforeBlocked = entity;
                if (m_ValtanBrain.Complete_NavigationBlockedStage(entity, m_Players, occurrenceCatalog, updateTick) &&
                    (!Apply_BossPatternStageTransition(entity,
                                                       beforeBlocked.strPatternId,
                                                       beforeBlocked.strActionId,
                                                       entity.strPatternId,
                                                       entity.strActionId,
                                                       beforeBlocked.PinnedDefinitionRevision,
                                                       entity.PinnedDefinitionRevision,
                                                       updateTick) ||
                     !Apply_EncounterPropStageEntry(entity, updateTick) ||
                     !Apply_WorldDestructionStageEntry(entity, updateTick)))
                {
                    entity = beforeBlocked;
                    entity.bMechanicLedgerRequiresReset = true;
                    m_strStatus = "Capture charge navigation outcome could not commit";
                    Mark_RuntimeFailure("world-update.capture-charge-navigation-outcome");
                    return false;
                }
            }
        }
    }

    return true;
}
```

### G0-4. Server/Private/GameRoom_ValtanGhost.cpp

변경 종류: 추가. 적용 위치: 해당 파일 전체.

유령 단계 재배치·포탈 scheduler와 부속 보스 수명을 묶는다.

Activate_ValtanGhostPhaseLoop는 루프 활성화를 검증한다. Begin_ValtanGhostRelocation은 재배치 상태를 준비한다. Update_ValtanGhostPortalScheduler는 예정 tick에 포탈 배우를 생성·정리한다. Update_DependentBosses는 owner에 매인 부속 보스를 갱신한다. 기존 네 함수의 본문·호출 순서를 유지한다. GameRoom_Internal의 좌표·tick helper와 BossCombatRuntime의 flag 처리를 사용한다.

```cpp
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
```

### G0-5. Server/Private/GameRoom_KoukuTargeting.cpp

변경 종류: 추가. 적용 위치: 해당 파일 전체.

쿠크 플레이어 표식, 랜덤 발사와 추적 투사체의 수명·타격을 묶는다.

Clear_KoukuPlayerTargets는 소유 표식을 정리한다. Update_KoukuRandomVolley는 기존 발사 주기를 소비한다. Update_KoukuPursuitProjectiles는 추적 이동·접촉을 갱신한다. Update_KoukuPlayerTargets는 유효 플레이어와 표식 실행 상태를 갱신한다. 기존 서버 소유 상태를 그대로 사용하며 새 random 정책은 넣지 않는다. GameRoom_Internal·BossCombatRuntime·KoukuSaydonBrain·KoukuSaydonLogicRuntime·PacketMessages는 실제 helper/타입/전송 소비를 직접 선언한다.

```cpp
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
```

### G0-6. Server/Private/GameRoom_KoukuMechanics.cpp

변경 종류: 추가. 적용 위치: 해당 파일 전체.

공중 이동 확정, 시간 기믹, 소환·객체 접촉, 진짜·가짜 세이튼을 묶는다.

Commit_KoukuAlbionAirborne은 착지/공중 이동 결과를 확정한다. Commit_KoukuMechanicTriggers는 이동·분신 생성처럼 entity 순회 뒤 수행해야 하는 작업을 처리한다. Update_KoukuSummonTriggers는 소환 trigger를 소비한다. Update_KoukuActorContacts는 객체 간 접촉을 갱신한다. Update_KoukuGazeClones는 분신 회전·수명을 관리한다. 진짜 세이튼 랜덤을 직접 구현할 다음 위치는 Commit_KoukuMechanicTriggers의 REAL_GAZE_TELEPORT 분기다.

```cpp
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
```

### G0-7. Server/Public/GameRoom.h

변경 종류: 수정. 적용 위치: 해당 파일 전체.

CGameRoom의 private helper 여섯 개를 선언한다. 새 상태·public API·manager를 추가하지 않는다.

private의 기존 Update_WorldEntities 선언 바로 아래에 Update_WorldEntity, Update_ValtanWorldEntity, Update_ValtanStageMotion, Release_WorldBossAttachments, Grant_ValtanRaidClearRewards, Despawn_FinishedWorldEntities를 추가한다. entity는 룸 worker가 이번 호출 동안 차용하는 참조이며 저장하지 않는다. fixedDeltaSeconds는 고정 tick의 초 단위, updateTick은 기존 0 예약 wrap 정책을 유지한다. bool은 데이터가 아닌 호출 흐름의 성공/실패 전달이다.

```cpp
#pragma once

// 수신 스레드에서 룸 큐로 넘기는 값 명령과 룸 간 이동 요청 형식을 사용한다.
#include "RoomCommand.h"
// 생성 준비 중 빌드 작업의 취소 여부를 차용한 atomic_bool로 확인한다.
#include <atomic>
// 룸이 소유할 플레이어 값 상태와 입장 후보·재화·피격 처리 형식을 사용한다.
#include "ServerPlayer.h"
// 룸의 보스·몬스터 등 월드 개체와 pattern occurrence 상태를 값으로 보관한다.
#include "ServerWorldEntity.h"
// 월드의 배치·진입 위치·trigger 설정을 읽는 카탈로그와 배치 형식을 사용한다.
#include "WorldBootstrap.h"
// 공유하는 불변 gameplay 세대와 스킬·보스·패턴 정의의 조회 형식을 제공한다.
#include "GameplayCatalog.h"
// 룸별 아이템 정의와 구매·장착·지급 검증 카탈로그를 소유한다.
#include "ItemCatalog.h"
// 장착 칭호의 유효 ID를 검사하는 룸별 카탈로그를 소유한다.
#include "HonorTitleCatalog.h"
// 승선·탈것 이동과 스킬에 필요한 정의 카탈로그를 소유한다.
#include "VehicleCatalog.h"
// 가이드 대사·trigger·행동 조합 정의를 소유하고 대사 예약에 GUIDE_TRIGGER를 받는다.
#include "GuideCatalog.h"
// 발탄 클리어 때 지급할 직업별 item ID 목록을 읽는 객체를 소유한다. 실제 지급은 룸이 맡는다.
#include "ValtanClearRewards.h"
// 플레이어의 스킬 시작과 진행을 처리하는 룸 소유 시스템을 선언한다.
#include "PlayerSkillSystem.h"
// 투사체 등 전투 개체 런타임과 stage 준비 transaction·고정 대상 형식을 사용한다.
#include "CombatObjectRuntime.h"
// 콜로세움 용병별 위협 평가 상태를 값 멤버로 보관한다.
#include "ColosseumThreatAssessment.h"
// 룸 navigation과 지면 점·지지면·탐색 계측 형식을 사용한다.
#include "ServerNavigation.h"
// 플레이어·보스 이동에 공유되는 룸 충돌 시스템을 소유한다.
#include "ServerCollisionSystem.h"
// 공간 trigger 처리와 상호작용 prompt·Mario 진입 판정 형식을 사용한다.
#include "ServerTriggerSystem.h"
// spawn 그룹의 정의·entry·anchor를 읽고 몬스터 생성 입력으로 받는다.
#include "SpawnGroupBootstrap.h"
// spawn 그룹의 웨이브 진행 상태를 룸 안에 소유한다.
#include "SpawnGroupRuntime.h"
// 일반 몬스터 판단을 실행하는 룸 소유 객체를 선언한다.
#include "MonsterBrain.h"
// 월드 NPC의 행동 진행을 처리하는 룸 소유 객체를 선언한다.
#include "NpcBehaviorRuntime.h"
// 발탄 본체와 종속 개체의 판단 객체를 소유하고 선택 기록을 조회한다.
#include "ValtanBrain.h"
// 쿠크 보스의 패턴 판단을 실행하는 룸 소유 객체를 선언한다.
#include "KoukuSaydonBrain.h"
// 쿠크 패턴의 판정 ledger·출력·대상 창과 카드미로·빙고 상태를 사용한다.
#include "KoukuSaydonLogicRuntime.h"
// 되돌릴 수 있는 기둥 상태를 일반 월드 파괴와 별도 런타임으로 소유한다.
#include "EncounterPropRuntime.h"
// 에스더 사용 시스템과 소환 예약이 차용하는 roster 항목 형식을 사용한다.
#include "EstherSkillSystem.h"
// 에스더 장판 예약이 차용하는 ZONE 형식을 사용한다.
#include "Gameplay/EstherStrikeContract.h"
// 워터팡 경기의 현재 event 표본 조회 형식을 선언한다.
#include "Gameplay/MaharakaWaterpangContract.h"
// 월드 파괴 정의와 pickup descriptor를 읽어 룸 안에 보관한다.
#include "WorldDestructionBootstrap.h"
// 파괴 상태·transaction·전이와 live event를 룸에서 관리한다.
#include "WorldDestructionRuntime.h"
// 입장 전 미리 조립한 송신 frame을 입장 후보가 값으로 소유한다.
#include "Network/PacketFrame.h"
// 입장 준비 실패를 연결 진단 reason으로 호출자에게 전달한다.
#include "Network/SessionDiagnostic.h"

// 큐 깊이·vector cursor·개수와 용량을 size_t로 표현한다.
#include <cstddef>
// wire ID·tick·revision과 누적 계측에 고정 폭 정수를 사용한다.
#include <cstdint>
// FIFO 입력·정리·월드 이동과 가이드 대사 예약 큐를 소유한다.
#include <deque>
// 플레이어 값과 ID/문자열 키별 runtime 상태를 정렬된 map에 보관한다.
#include <map>
// 불변 catalog와 입장 후보는 공유 소유하고 세션 목록은 weak_ptr로 차용하며 종속 brain은 단독 소유한다.
#include <memory>
// 수신 큐·정리 identity·공유 계측 조회를 룸 worker와 수신 경로 사이에서 동기화한다.
#include <mutex>
// 예약·최근 응답·실행 상태가 없는 경우를 기본값이 있는 실제 상태와 구분한다.
#include <optional>
// Mario 배치 선택과 장비 재련이 서로 다른 난수 엔진을 소유한다.
#include <random>
// 입장 batch 준비에서 앞서 준비한 후보 목록을 복사 없이 차용한다.
#include <span>
// stable ID·진단·원본 bytes와 대사 상태의 문자열을 소유한다.
#include <string>
// 실패 출처와 검증 메시지 입력을 호출 동안 복사 없이 받는다.
#include <string_view>
// session/player/entity ID와 요청 sequence를 키로 룸 내부 상태를 찾는다.
#include <unordered_map>
// 정리 대기 session과 가이드의 진입 공간 집합을 중복 없이 보관한다.
#include <unordered_set>
// 세대·플레이어 입장 frame·실행 개체·예약·관측 event의 가변 길이 목록을 소유한다.
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	// 룸은 승인한 catalog 객체를 제자리 수정하지 않고 shared_ptr<const>로 공유한다.
	// 진행 occurrence의 revision pin이 필요한 이전 세대를 보관하며 룸 실행 경로가 조회·교체·회수를 직렬로 호출한다.
	// 수치 변경도 객체를 교체한다. 같은 revision이라고 raw 포인터의 수명이 자동으로 유지되지는 않는다.
	class CGameplayCatalogGenerations final
	{
	public:
		// 일반 세대 보관 상한이다. 생성자가 용량을 예약하며 live pin 때문에 가득 차면 새 revision의 Stage를 거절한다.
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		// 룸 생성 시 일반 세대 vector 공간을 미리 확보한다. 메모리 할당 예외를 이 생성자가 처리하지는 않는다.
		CGameplayCatalogGenerations();

		// 초기 룸이 새 catalog를 로드해 Initialize한다. 명시적 로드 실패는 기존 세대를 유지하고 진단만 바꾼다.
		bool Load();

		// 유효한 초기 세대를 공유 소유하고 일반 보관 목록·stage·epoch를 초기화한다. 잘못된 입력은 false다.
		// 수치 예약을 정리하지 않으므로 실행 중 수치 transaction의 재설정 용도로 쓰지 않는다.
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);

		// CGameRoom이 live pin을 회수한 뒤 일반 정의 세대를 예약한다. revision·sequence·용량 실패는 기존 예약을 유지한다.
		// 같은 sequence/revision 재요청은 true이며 status가 그대로일 수 있다. 승인 전 active는 바꾸지 않는다.
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);

		// 예약 sequence를 확인해 active를 교체하고 epoch를 증가시킨다. false는 기존 예약을 남긴다.
		// 호출자는 전 방 준비를 끝내야 하며 noexcept이 진단 문자열 할당 실패의 복구를 보장하지는 않는다.
		bool Commit(std::uint32_t transactionSequence) noexcept;

		// CGameRoom의 수치 준비가 같은 active revision의 후보를 전달한다.
		// 과거 세대마다 choreography를 보존한 수치 복사본을 먼저 만들며 명시적 false에서는 active·보관 목록을 유지한다.
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);

		// 준비한 수치 vector와 active를 교체한다. 잘못된 sequence/빈 예약은 false이며 revision과 epoch는 유지한다.
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 일치한 수치 예약만 해제한다. sequence 0은 무조건 취소이며 active는 되돌리지 않는다.
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 일반 stage의 공유 소유권만 해제한다. sequence 0은 무조건 취소, 다른 비영 sequence는 무시한다.
		void Abort(std::uint32_t transactionSequence) noexcept;

		// Tick 끝과 일반 Stage 직전이 살아 있는 occurrence의 revision을 전달한다.
		// active·일반 stage·live pin을 제외한 보관 소유권만 놓는다. 외부 shared_ptr가 있으면 객체는 계속 유지된다.
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		// 보관 목록과 일반 stage에서 revision을 찾는다. 없거나 invalid이면 nullptr이며 수치 후보는 아직 노출하지 않는다.
		// 반환 포인터는 차용이다. 회수·재초기화·수치 Commit을 넘겨 보관하려면 별도 소유권 또는 revision 재조회가 필요하다.
		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;

		// 현재 active를 차용한다. 초기화 전에는 정적 빈 catalog를 반환하며 보관 소유권을 추가하지 않는다.
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;

		// 생성 준비·전역 세대 처리에 현재 객체의 공유 소유권을 돌려준다. 이후 active 교체와 반환 객체 수명은 분리된다.
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept
		{
			return m_pActiveGeneration;
		}

		// 현재 일반 보관 목록의 크기다. 별도 수치 준비 vector의 항목 수는 더하지 않는다.
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}

		// 초기화는 1, 일반 Commit은 1씩 증가시킨다. 수치만 교체할 때는 이 값이 바뀌지 않는다.
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		// 아래 Find 계열은 현재 Active의 조회를 그대로 전달한다. 과거 occurrence는 Resolve(pin)한 catalog를 직접 사용한다.
		// 반환 포인터·참조는 catalog 내부를 차용한다. ID가 없는 포인터 조회는 nullptr이며 값을 생성하지 않는다.
		// 스킬 입력은 stable SKILL_ID다.
		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;

		// 보스 생성·수치 갱신이 archetype ID로 현재 profile을 조회한다.
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;

		// archetype별 부위 목록을 차용한다. nullptr와 존재하지만 빈 목록을 구분할 수 있다.
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;

		// encounter별 현재 패턴 목록을 차용한다. 실행 중 pattern의 이전 세대를 자동 선택하지는 않는다.
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;

		// 전투 개체 생성·검증에 필요한 archetype 정의를 차용한다.
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;

		// 발탄 timeline 준비가 encounter별 timeline 정의를 차용한다.
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;

		// encounter와 stable command ID로 발탄 timeline의 한 행을 차용한다.
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;

		// encounter·gameplay phase·체력 줄 수를 catalog의 rotation 구간 판정에 전달한다.
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;

		// 입장 패턴 ID를 차용한다. 정의가 없으면 catalog가 제공하는 정적 빈 문자열 참조다.
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;

		// 생성·class 변경·수치 갱신이 class별 현재 profile을 차용한다.
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;

		// 피해 profile의 percent를 값으로 반환한다. 정의가 없을 때의 0도 catalog 반환값 그대로다.
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		// snapshot·새 occurrence가 쓰는 현재 choreography revision을 차용한다. 수치 Commit은 같은 revision을 유지한다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;

		// facade 자체 진단이 있으면 우선하고 없으면 active 진단을 차용한다. 외부 status 출력과 자동 동기화되지는 않는다.
		[[nodiscard]] const std::string& Get_Status() const noexcept;

		// 기존 catalog 입력 API에 Active 참조로 전달한다. 변환이 공유 소유권을 만들지는 않는다.
		operator const CGameplayCatalog&() const noexcept
		{
			return Active();
		}

	private:
		// 현재·일반 예약 객체와 그 sequence다. 일반 세대들은 m_Generations가 공유 소유한다.
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;

		// 수치 교체는 별도 후보 vector에 모아 둔다. Commit이 일반 보관 vector와 바꾸고 이전 소유권을 해제한다.
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_NumericStagedGenerations;
		std::shared_ptr<const CGameplayCatalog> m_pNumericStagedActive;
		std::uint32_t m_iNumericTransactionSequence = 0u;

		std::string m_strStatus;
	};

	// ServerApp이 직전에 마친 바깥 룸 loop의 스케줄 표본이다. 다음 Tick에서 모든 룸에 같은 값이 전달된다.
	// Sample은 Unix ms, lateness는 μs다. 늦어진 loop와 일정 재설정 횟수를 룸 자체 처리 시간과 구분한다.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	// 수신 큐와 Tick·snapshot 처리의 누적/최근 표본이다. 각 접미 Microseconds는 μs이며 개수는 별도 필드다.
	// Get_PerformanceMetrics는 mutex 아래 값을 복사하므로 반환 이후의 계측은 그 복사본에 반영되지 않는다.
	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;

		// 정상 완료와 명시적 조기 중단에서 기록한 Tick 호출의 수·최근/최대 시간이다. 서버 tick 번호와 다르다.
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;

		// Tick이 일반/정리 큐를 가져오던 깊이와 소비 후 잔량, 수신 중 최고 깊이를 구분한다.
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;

		// 일반 drain 제한·MOVE/AIM 합치기·용량 거절과 LEAVE 중복/취소를 각각 누적한다.
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;

		// snapshot 조립과 수신자 enqueue를 별도로 측정한다. enqueue 성공은 wire 전송 완료를 뜻하지 않는다.
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	// ServerApp의 수신 경로는 용량 초과·실패 룸·봉인·정리 대기를 구분해 연결 정책과 진단에 사용한다.
	// best-effort 유실과 중복 LEAVE 합침도 허용 결과이므로 bool 성공이 새 명령 보관을 뜻하지는 않는다.
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	// Enqueue와 ServerApp이 연결을 계속 처리할 수 있는 세 결과를 묶는다. 명령 실행 완료 여부는 판정하지 않는다.
	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	// Mark_RuntimeFailure가 최초 실패의 당시 서버 tick·출처·상세를 보관하고 조회자에게 복사한다.
	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	// ServerApp이 한 simulation의 수명을 소유한다. 정상 gameplay 변경은 룸 worker가 직렬로 실행한다.
	// 수신 경로는 Enqueue로 값 명령을 전달하며 command mutex가 모든 gameplay 멤버를 보호하는 것은 아니다.
	// 준비 생성과 worker 합류 뒤 종료 중 제어는 해당 worker와 겹치지 않는 호출자 수명 관리가 전제다.
	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		// ServerApp의 시작·개인 방·콜로세움 준비가 월드와 선택적 초기 세대를 전달한다.
		// 취소 atomic 포인터는 생성 호출 동안만 차용한다. 필수 로드 실패 객체는 Is_Ready가 false이고 Get_Status로 확인한다.
		// 실패 전 만든 멤버는 객체가 소유하며, ready 확정 뒤 가이드 부가 초기화 실패는 룸 전체 준비를 취소하지 않는다.
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {},
			const std::atomic_bool* pPreparationCancelled = nullptr);

		// Enqueue_Detailed의 결과를 허용 bool로 줄인다. MOVE/AIM 유실과 LEAVE 중복도 true일 수 있다.
		bool Enqueue(ROOM_COMMAND command);

		// 수신/제어 경로가 값을 넘긴다. session 식별자 검증 후 mutex 아래 ready·봉인·정리 대기·용량을 검사한다.
		// 허용한 명령은 큐가 소유한다. LEAVE는 별도 우선 큐에서 session별로 합치고 공유 일반 큐의 같은 session 명령을 지운다.
		// 이미 Tick 지역 큐로 가져온 명령까지 취소하는 것은 아니다. MOVE/AIM은 stream 경계 안에서 최신 값으로 합친다.
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);

		// ServerApp이 판정 enum에 현재 큐 깊이·봉인·최초 실패를 붙인다. 원래 enqueue 시점의 snapshot은 아니다.
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;

		// 진단/검증 경로가 mutex 아래 최초 실패를 복사한다. 기록이 없으면 false이며 outFailure는 보존한다.
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;

		// ServerApp 룸 worker가 delta 초와 스케줄 표본을 전달한다. 정리 전체/일반 최대 256개를 잠금 밖에서 처리한다.
		// 그 뒤 simulation·송신을 순서대로 진행하며 중간 실패는 이미 바뀐 상태를 되돌리지 않는다.
		// ready가 아니면 즉시 반환한다. 잘못된 delta도 앞선 명령 소비까지 취소하지는 않는다.
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});

		// Tick 직후 같은 룸 worker의 ServerApp이 최신 진단 한 줄을 가져가며 멤버를 비운다. 별도 mutex는 없다.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();

		// 모든 룸 Tick 뒤 ServerApp이 FIFO 요청을 이동해 받는다. 빈 큐는 false이며 출력은 유지한다.
		// 성공 시 Bern 선박 복귀 기억도 갱신하지만 실제 월드 전환 성공이 확정된 것은 아니다.
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);
		bool Configure_ColosseumMatch(std::uint64_t matchId, const std::vector<SESSION_ID>& sessions);
		void Remove_ColosseumExpectedSession(SESSION_ID sessionId);

		// 생성 시 결정한 simulation의 월드 ID다. shared/private 여부를 이 ID만으로 판별하지 않는다.
		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		// 준비 또는 룸 실행 실패 상태를 그대로 읽는다. 이 getter 자체는 mutex/atomic 동기화를 제공하지 않는다.
		[[nodiscard]] bool Is_Ready() const
		{
			return m_isReady;
		}

		// 현재 진단 문자열을 차용한다. 다른 제어/실행이 갱신할 수 있으므로 호출자는 룸 접근 순서를 지켜야 한다.
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}

		// ServerApp은 룸 worker에서 전 방 수치 후보를 준비하고 디스크 저장 성공 뒤 확정한다.
		// 종료 중에는 worker 합류 뒤 같은 제어를 수행한다. Stage는 snapshot·별도 catalog 수치 복사본을
		// 지역 값에서 완성하며 명시적 false는 이전 룸 예약 배열을 유지한다. 할당 예외는 여기서 잡지 않는다.
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);

		// 같은 sequence의 세대를 교체한 뒤 player·투사체·boss의 실제 수치와 비율을 조정한다.
		// 포인터 교체만 하는 함수가 아니며 noexcept이 이후 변경의 rollback을 보장하지는 않는다.
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// catalog는 다른 비영 sequence를 무시하지만 이 wrapper의 예약 entry/remap 배열은 항상 비운다.
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 전역 정의 교체 준비가 live pin 구성·미사용 세대 회수 뒤 후보를 예약한다.
		// 예약 실패여도 앞서 회수한 미사용 세대 소유권까지 복구하지는 않는다.
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);

		// ServerApp이 전 방 준비와 참여자 승인을 확인한 fixed-step 경계에서 확정한다.
		// 성공 시 비보스 identity는 즉시 갱신하고 보스는 다음 brain 평가가 갱신한다. 진행 occurrence pin은 유지한다.
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;

		// ServerApp의 준비/승인 실패가 일반 예약만 해제한다. sequence 0은 무조건 취소이며 active는 보존한다.
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;

		// ServerApp의 입장·전환 세대 일치 검사와 계약 검증이 active 공유 소유권을 받는다. 최초 초기화 전에는 비어 있을 수 있다.
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}

		// 현재 호출자는 세대 보존/회수 계약 검증이다. 반환 포인터는 차용이며 회수/수치 교체 뒤에는 다시 조회한다.
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}

		// ServerApp 제어 이벤트가 룸 worker에서 발탄 brain의 최신 선택 기록을 wire 값으로 복사한다.
		// 잘못된 world·보스 부재·새 기록 없음도 typed 응답을 만든 true다. false는 내부 기록 계약 오류이며 이전 응답은 보존한다.
		// 송신은 호출자 책임이다. 관측한 revision 값 자체가 세대 수명을 붙잡는 live pin은 아니다.
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;

		// 성능 로그/검증 경로가 mutex 아래 최근 계측값의 복사본을 받는다.
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// ServerApp 룸 worker가 비어 있는 Character Select 방의 회수 직전에 호출한다.
		// 큐·전환·session/player lookup이 비면 입력을 봉인한다. 이미 봉인된 방도 true이며 객체 삭제는 호출자 책임이다.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();

		// 일반 단독 전환에서 룸 worker가 대상 REGISTER/ENTER를 예약한 뒤 source Leave를 확정한다.
		// 다음 대상 Tick 전에 기존 player binding을 해제한다. 준비 안 됨/없는 session은 false다.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);

		// ServerApp의 월드 전환과 Transfer_PartyTo가 단독 guide owner의 admission 경로를 선택한다.
		// 룸 실행 스레드에서 ServerApp의 session-binding mutex를 보유한 호출 전제이며 내부 잠금은 없다.
		// Bern의 같은 실제 session·안내 actor·binding만 허용한다. 월드 이탈 대기는 binding이 없는 경우에 true다.
		[[nodiscard]] bool Has_PersonalGuideOwner(const std::shared_ptr<CClientSession>& ownerSession) const;
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		bool Transfer_ColosseumMatchTo(CGameRoom& target,
			const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status);
		void Notify_ColosseumTransferResult(bool committed);
		[[nodiscard]] bool Try_SealColosseumForRetirement();
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		// Tick 등 룸 실패 경로가 ready를 내리고 최초 tick·출처·현재 진단만 기록한다. 기존 변경 rollback이나 큐 비우기는 하지 않는다.
		void Mark_RuntimeFailure(std::string_view source);

		// 레이드 인원·빈 방 정리·Esther 갱신이 m_Players의 인간 제어 항목만 센다. HP가 0인 인간도 포함하고 AI는 제외한다.
		std::size_t Count_HumanPlayers() const;

		// 생성자가 필수 룸 준비 뒤 안내 catalog와 설치된 차원술사 skill binding을 확인한다.
		// 지원 world에서 읽되 actor는 Bern에만 만든다. 안내 실패는 로그로 남기며 이미 설정한 룸 ready를 내리지 않는다.
		void Initialize_Guide();

		// 초기 생성/부활이 지정한 player/entity ID와 world XYZ(m)로 지역 GUIDE_AI 값을 준비한다.
		// 정의·ID·snapshot 수용량·착지 검사 실패는 outPlayer를 보존한다. 성공도 여기서 룸 삽입이나 session 생성까지 하지 않는다.
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;

		// 생성·부활·회피·도착 경로가 중심과 3m 이내 고정 후보 중 첫 허용 위치를 찾는다. 성공할 때만 point를 바꾼다.
		// 입력 참조는 호출 중 차용한다. anchor가 있으면 navigation 연결이 필수이며 충돌·다른 살아 있는 player 겹침도 검사한다.
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
			const SERVER_PLAYER* anchor = nullptr) const;

		// GUIDE_CONTROL START가 기존 Bern 안내 actor에 최대 한 owner를 연결한다. 실제 session은 weak_ptr로 저장한다.
		// false는 target이 guide가 아님을 뜻한다. true에는 거리/상태 거절과 기존 owner 유지도 포함하므로 새 시작 성공값이 아니다.
		bool Start_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);

		// Tick의 명령 drain이 Bern START/STOP을 처리한다. 새 요청 sequence는 actor/binding 확인보다 먼저 기록한다.
		// STOP은 이 session의 안내 예약만 해제한다. 거절 요청에 별도 결과 packet을 보내는 함수는 아니다.
		void Handle_GuideControl(SESSION_ID sessionId, const LostArk::Shared::C2S_GUIDE_CONTROL& request);

		// 시작/해제와 owner 부재 대기 갱신이 단일 안내 actor의 소유 표시를 만든다. actor가 없으면 송신하지 않는다.
		void Broadcast_GuideOwnership();

		// 소유 표시/판단 trace를 방 안의 인간 session들에게 보낸다. 직렬화 실패는 송신 없이 끝낸다.
		// 개별 enqueue 실패는 해당 연결 종료를 요청하며, 이미 바뀐 안내 상태나 다른 수신자의 송신을 되돌리지 않는다.
		void Broadcast_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& message);

		// Tick이 player 이동 갱신 전에 초 단위 고정 dt로 호출한다. 현재 실행 입구는 catalog가 Loaded인 Bern뿐이다.
		// 대사 처리와 판단 주기를 나누고 이동·skill은 공통 player executor에 제출한다. 실패 시 전체 상태 rollback은 없다.
		void Update_Guides(float seconds);
		void Update_Colosseum(float seconds);
		void Handle_ColosseumRecruit(SESSION_ID sessionId, const LostArk::Shared::C2S_COLOSSEUM_RECRUIT& request);
		LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE Build_ColosseumState() const;
		void Broadcast_ColosseumState();

		// Update_Guides가 후보 world XZ(m)의 게시 패턴·Kouku ledger·빙고 망치 접촉 위험을 읽는다.
		// guide 높이/identity를 지역 probe에 복사하며 실제 player는 바꾸지 않는다. 반환은 피해량/확률이 아닌 근사 점수다.
		float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);

		// 확정된 owner 이동과 먼 거리 복구가 안내 접근 상태를 다시 준비한다. 인간 전용 기믹/월드 대기 owner는 건너뛴다.
		// Bern localMapTravel은 선박에 타지 않았을 때 연결된 착지를 먼저 검사한다. 실패는 Reason만 바꾸고 actor/실행 상태를 유지한다.
		// 그 밖에는 현재 위치에서 도보 접근하도록 reset한다. 실제 공간 진입 edge를 남기려고 InsideBoxes는 비우지 않는다.
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor, bool localMapTravel = false);

		// Handle_Chat이 인간 owner의 문자열을 전달한다. 앞뒤 ASCII 공백 제거 후 정확한 별칭만 찾으며 일반 채팅 송신은 호출자가 계속한다.
		// 월드/선박 대기 중에는 무시한다. Stop은 명령 cooldown을 우회해 combo 예약을 비우지만 진행 중 skill을 강제로 끝내지는 않는다.
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);

		// STOP·일반 Leave·owner/actor 소실이 안내 제어와 대사 예약을 해제한다. 배치 actor는 남기고 이동/skill과 비행 입력을 정리한다.
		// publish=false는 다른 outbound transaction 안에서 즉시 안내 송신을 피한다. owner runtime이 없으면 변경하지 않는다.
		void Remove_Guide(SESSION_ID ownerSessionId, bool publish = true);

		// Leave의 LEVEL_CHANGED가 Bern 안내를 현 위치에 대기시킨다. weak owner/actor identity는 보존하고 combo·대사·공간 접촉을 비운다.
		// 입장 transaction 중에도 호출하므로 여기서는 packet을 직접 보내지 않는다.
		void Suspend_PersonalGuide(SESSION_ID ownerSessionId);

		// 단독/파티 입장 commit 뒤 Bern 복귀를 알린다. 같은 실제 owner session과 새 player binding을 확인한 경우만 추적을 재개한다.
		// sourceWorld에 맞는 복귀 대사는 예약만 하고 다음 Update_Guides가 송신한다. 이 함수가 actor를 순간이동시키지는 않는다.
		void Resume_PersonalGuide(SESSION_ID ownerSessionId, LostArk::Shared::WORLD_ID sourceWorld);

		// 시작/복귀 시 owner가 이미 닿은 SPACE_ENTER trigger ID를 현재 접촉 기준으로 기록한다. 대사 예약이나 cooldown 소비는 하지 않는다.
		void Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor);

		// 예약/갱신이 미시작 공간 대사에서 더 이상 접촉하지 않는 source를 지운다. 남은 source의 최고 priority로 다시 정렬한다.
		// 첫 segment가 이미 송신된 대사는 접촉 이탈로 취소하지 않고 앞에 유지한다.
		void Prune_GuideSpacePrompts(SESSION_ID ownerSessionId);

		// 시작·복귀·도움·공간/보스 사건을 owner별 대사 큐에 예약한다. 같은 prompt는 진행 상태·source·priority에 따라 합치거나 유지/교체한다.
		// 최대 16개이며 가득 차면 새 예약을 버린다. 여기서는 송신하거나 trigger cooldown을 소비하지 않는다.
		void Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);

		// 입장 준비가 소유하는 임시 값이다. 룸 membership과 실제 reliable FIFO는 호출자가 별도로 commit한다.
		struct STAGED_PLAYER_ENTRY final
		{
			// Stage부터 packet/FIFO 준비까지 실제 연결 수명을 공유 소유한다. 룸의 m_Sessions 등록은 weak_ptr다.
			std::shared_ptr<CClientSession> pSession;

			// 목적 룸 profile·착지·이월 inventory로 만든 값이며 Stage만으로 m_Players에 들어가지는 않는다.
			SERVER_PLAYER Player;

			// 인코딩한 초기 메시지 payload를 소유한다. Build_PlayerEntryFrames가 완성한 목록만 교체한다.
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};

		// 단독 Join과 파티/콜로세움 전환이 목적 룸의 후보를 준비한다. precedingEntries는 같은 배치의 ID offset과 spawn 예약이다.
		// 실패는 출력 후보/룸 membership을 보존하고 outReason/status를 바꾼다. 성공도 송신·ID 증가·player binding까지 하지 않는다.
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {}, bool hasCarriedCharacterState = false);

		// 위 입장 경로가 승인/revision부터 월드 상태·기존/신규 player까지 FIFO 순서로 준비한다.
		// false는 entry.Frames를 보존한다. 성공한 payload를 실제 session 큐에 예약하는 책임은 호출자에게 있다.
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);

		// 단독 Join이 FIFO 예약 성공 뒤 membership/lookup/ID와 연결 binding을 확정한다.
		// 할당 예외의 중간 삽입 rollback은 보장하지 않는다. 일괄 전환은 이 함수 대신 준비한 map들을 swap한다.
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();

		// Tick REGISTER_SESSION이 유효한 실제 연결을 약하게 등록한다. player 생성/binding은 뒤의 Join 단계다.
		void Handle_Register(const std::shared_ptr<CClientSession>& session);

		// Tick ENTER_WORLD가 지역 player·초기 packet·reliable FIFO를 준비한 뒤 입장을 확정한다.
		// 명시적 준비 실패는 membership을 만들지 않고 연결 종료를 요청한다. true는 로컬 commit이며 Client 수신 완료는 아니다.
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {},
			LostArk::Shared::WORLD_ID sourceWorld = LostArk::Shared::WORLD_ID::BERN,
			bool hasCarriedCharacterState = false);

		// 정리 큐와 월드 전환이 session 예약부터 player/lookup·파티·빈 방 상태까지 순서대로 해제한다.
		// publishDeparture는 안내/대기열/명시적 despawn 통지 일부를 제어한다. 모든 하위 종료 함수의 송신을 막는 값은 아니다.
		// Leave는 초기화 실패를 로그로 남기며 이미 끝낸 퇴장을 되돌리지 않는다. 하위 초기화 실패는 룸을 실패 상태로 내릴 수 있다.
		// 이 함수 본문은 룸 mutex를 직접 잡지 않는다.
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);

		// 명령 handler가 session/player lookup 불일치를 공통 진단으로 종료 요청한다.
		// 살아 있는 연결이 없으면 무시하며 실제 룸 상태 제거는 후속 Leave에 맡긴다.
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		// 현재 스킬의 이동 취소 구간인지 확인해, 허용된 이동 명령이 후딜레이를 끝낼 수 있게 한다.
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		// Debug F1은 게이지·슬롯 선택을 생략하고 같은 Esther 시전·소환 경로를 쓴다. Release는 무시한다.
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		// session 소유 플레이어가 생존·대기·비탑승 등 시전 조건을 만족할 때만 포인터를 빌려준다.
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		// 조준 방향의 소환을 예약하고 시전자를 ESTHER_CAST로 묶는다. 게이지 차감 여부는 호출자가 결정한다.
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		// 비활성 squarehole.<id> 배치에서 착지점을 찾고 지면·높이·충돌을 검증한다. 착지할 수 없으면 false다.
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		// 노래 종료 후 목적지로 이동하며, 착지 검증에 실패하면 현재 위치를 유지한다.
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		// Debug에서 자기 HP를 0, 행동을 DEAD로 만들어 사망 화면을 확인한다. Release는 무시한다.
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		// Debug Character Select에서 기존 Server 월드 전환을 예약한다. Client Level 변경은 승인 흐름이 담당한다.
		void Handle_DebugEnterKoukuSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA& request);
		// Debug Kouku waypoint 이동은 playerSpawn·Server 지면·충돌을 검증한다. 시작점 ID는 별도 복귀 경로를 쓴다.
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		// Tick의 trigger callback이 Mario lane과 최종 출구를 검증한다. 기본 처리·내부 재시도·시작 완료를 enum으로 구분한다.
		// 이동은 player 복사본에서 성공한 뒤 반영하지만 진단/충돌 보조 상태까지 모두 되돌리는 함수는 아니다.
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		// Debug 빙고 칸과 완성 줄을 갱신한다. 별도 응답 없이 월드 snapshot으로 전달한다.
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		// Debug 빙고 폭탄 표식을 요청자에게 붙여 월드 snapshot으로 전달한다.
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		// Debug 빙고 망치는 행·열의 20개 시작점 중 하나에서 움직이고 월드 snapshot에 반영된다.
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		// Debug 일반 몬스터 버튼은 대응 SpawnGroup의 생존 몬스터를 지우고 원래 배치에서 재시작한다.
		// Release는 무시하며 별도 응답 없이 월드 snapshot으로 전달한다.
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		// Debug에서 요청자의 광대 변신과 관련 행동 상태를 정리해 반영한다. Release는 REJECTED_DISABLED로 응답한다.
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		// 요청자 탑승 여부를 판정해 응답한다. 실제 탈것 상태는 월드 snapshot으로 전달한다.
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		// 플레이어의 생존·행동·구속 상태 등이 탑승을 허용하는지 검사한다.
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		// 요청자의 칭호 변경을 판정해 응답한다. 장착된 칭호는 월드 snapshot으로 전달한다.
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		// 이동과 snapshot이 함께 쓰는 속도(m/s)다. 탈것·선박 가속 또는 기본 속도·stance·워터팡 가속을 반영한다.
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		// 매 tick의 snapshot 전에 월드·catalog·플레이어 상태상 탑승 불가인 플레이어를 하차시킨다.
		void Enforce_VehicleRidingState();
		// 승선은 BernSea의 가까운 탐색 반경에서 바다를 찾고 부두 위치를 보관한다. 찾지 못하면 상태를 유지하고 false다.
		// 하차·강제 하차는 저장한 부두 위치와 방향을 복원한다.
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		// 탑승한 탈것의 스킬·sequence·쿨타임·상태를 검사한다. 일반 시전 외에 비행 E 토글도 이 경로에서 처리한다.
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		// 고정 tick마다 탈것 스킬을 진행한다. 비행은 이륙·이동·착륙 상태를, 지상 스킬은 지면·충돌을 반영한다.
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		// 현재 HUD 슬롯의 스킬을 sequence·상태·쿨타임 검증 후 시작한다. DANCE 입력은 열린 pose 판정 창에도 기록한다.
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		// HUD 강제 모드를 판정해 요청자에게 응답한다. Release는 Kouku의 NONE/MARIO/MAZE만 허용한다.
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		// 매 tick encounter의 광기 상한, 광대 유지 시간, 플레이어별 HUD 모드와 슬롯을 갱신한다.
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		// stable ID·revision 기반 Play/Restart/Next와 아레나 preset 요청을 판정한다. 구 HP·timeline 명령은 거절한다.
		// Evaluate가 상태와 재시도 receipt를 처리하고 Handle은 session을 찾아 응답한다.
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		// 배치 ID·archetype이 일치하는 Kouku 보스 포인터를 빌려준다. 목록에 없으면 nullptr이며 생존 여부는 호출자가 검사한다.
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		// Logic 출력의 기믹을 보스 ID·패턴 sequence와 함께 보관해, 해당 실행에 속한 요청만 후속 처리한다.
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		// Logic의 포획·기믹·연출·후속 패턴 요청을 방 상태에 반영한다. true면 Brain의 패턴 완료까지 반영한 것이다.
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// 진입 root가 후속 패턴들이 끝날 때까지 포털 기준 객체를 보관한다.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// 나중에 방에 남은 사람이 아니라 실제 입장에 성공한 플레이어·session·entity를 고정한다.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// 본 패턴의 actor 진행이 끝나도 남은 occurrence의 시간과 소유 객체를 유지한다.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			// 전체 gameplay는 PinnedGameplayRevision을 따른다. 이 shared_ptr는 이번 실행의 Kouku 패턴·Logic 정의를 유지한다.
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// 자연스러운 패턴 완료 뒤에도 유지하며, reset·cancel 또는 HP 0에서 제거한다.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// 이미 실행 중인 선행 패턴을 인계받은 경우에는 대응하는 Client Play 요청이 없다.
			bool bAdoptedLivePredecessor = false;
			// 현재 Flow occurrence만 기존 Brain의 순서 실행에 이어 붙인다.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		// 전투 페이지 시작은 그 시점의 아레나 파괴 상태를 준비하고 실제 Brain을 재개해 이후 전투를 계속 진행한다.
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Debug 지급과 클리어 보상이 공유한다. catalog의 maxStack·인벤토리 슬롯 한도 안에서 가방 수량을 합친다.
		// 알 수 없는 ID나 새 슬롯 부족은 false로 거절하고 인벤토리를 유지한다.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// 요청자의 아이템을 Grant_Item으로 지급하고 성공하면 전체 인벤토리를 응답한다.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// 소유 수량·생존과 아이템별 사용 조건을 검증해 회복 또는 전투 효과를 적용한 뒤 수량을 차감한다.
		// 인벤토리를 응답하고 HP 등 월드 상태는 snapshot으로 전달한다.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		// 장착 슬롯·직업·가방 여유를 검사한다. 변경 성공 여부와 관계없이 전체 인벤토리를 응답한다.
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		// 장착 장비 또는 전체 가방의 수리비를 합산한다. 실링이 충분할 때만 차감·완전 수리하고 인벤토리를 응답한다.
		void Handle_RepairEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request);
		// 직업 변경 뒤 새 직업이 쓸 수 없는 장비를 가방으로 돌린다.
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		// 상점 NPC 거리·판매 목록·총액·가방 공간을 모두 검증한 뒤 구매한다. 거절 시 상태를 유지하고 인벤토리를 응답한다.
		void Handle_BuyItems(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_BUY_ITEMS& request);
		bool Apply_BuyItems(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_BUY_ITEMS& request) const;
		// Bern 신규 입장의 인벤토리 변경 전 한 번만 저장 캐릭터를 복원한다. 전체 요청을 검증해 거절 시 기존 상태를 유지한다.
		void Handle_RestoreCharacter(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request);
		bool Validate_RestoreCharacter(const SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const;
		void Handle_UpgradeEquipment(SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPGRADE_EQUIPMENT& request);
		void Handle_CaptureCharacter(SESSION_ID sessionId,
			const LostArk::Shared::C2S_CAPTURE_CHARACTER& request);
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// Character Select는 전체 시험 객체·SpawnGroup을 초기화하고 Kouku는 카드미로·시험 객체를 정리한다.
		// Valtan은 주 보스 소유 트리만 제거해 파동·NPC·파괴 상태·요청 receipt를 보존한다.
		// 실행 중 Kouku raid와 허용 외 월드·session 요청은 응답 없이 무시한다.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		// 기본은 비활성 배치에서 만든 시험 객체·종속 객체와 Esther 소환을 정리한다.
		// allArenaBosses는 모든 보스로 범위를 넓히며, preflightOnly는 복사본으로 전송 자료를 검증한다.
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern 안내 NPC 확인 요청의 거리·생존·행동을 재검사하고 기존 Server 월드 전환을 예약한다.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// Bern에서 거리·상태를 재검사해 최대 4명을 10초간 모은다. 승인 순서로 팀을 나눠 private 경기 입장을 함께 준비한다.
		// 전환 성공 때만 대기열을 소비하며 LEAVE는 요청자만 제외한다.
		void Handle_ColosseumQueueJoin(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request);
		void Handle_ColosseumQueueLeave(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request);
		void Send_ColosseumQueueState(
			SESSION_ID sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE state);
		void Try_FormColosseumMatch();
		void Broadcast_ColosseumQueueState();
		void Try_StartColosseumEntry();
		void Handle_ColosseumLoadReady(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_LOAD_READY&);
		void Handle_ColosseumReturn(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_RETURN&);
		void Update_ColosseumMatch(std::uint32_t tick);
		void Broadcast_ColosseumMatchState();
		void Score_ColosseumKills(std::uint32_t tick);
		// 상호작용 키 요청은 trigger ID만 받는다. 플레이어가 아직 상자 안에 있는지 재검사한 뒤 실행한다.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// 클리어한 Valtan/Kouku에서 거리·파티장 조건 없이 각자 기록된 진입 안내 NPC로 돌아간다. Lobby 입장은 기본 안내점을 쓴다.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		// 클리어 복귀 버튼과 관문 EXIT가 공유하는 Bern 전환 예약이다. 예약하지 못하면 false다.
		bool Stage_ReturnToBern(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t requestSequence);
		// 같은 방의 현재 NetEntityId를 플레이어로 찾아 초대한다. 다른 방·사라진 ID는 거절하며 닉네임은 표시용이다.
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		// 관문의 마지막 주 보스 사망을 기록하고 이동 투표를 모은다. 전원 동의하면 다음 관문 배치와 플레이어 위치로 전환한다.
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1부터 시작하며 0은 아직 열린 관문 없음
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // 0이 아닌 raid run ID로 투표 대상을 고정
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		bool Complete_GateProgressTransition(const GATE_PROGRESS_STATE& transition);
		void Update_ArenaAssembly();
		bool Complete_ArenaAssembly();
		bool Collect_KoukuEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants, std::uint32_t& raidEpoch) const;
		bool Collect_ValtanEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants) const;
		bool Start_ValtanEntry(const std::vector<LostArk::Shared::PLAYER_ID>& expectedParticipants);
		// 사망한 주 보스가 관문의 마지막 보스인지 확인해 클리어를 기록한다.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// Debug 또는 관문 진행으로 배치가 생성되면 현재 관문을 갱신한다.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result,
			LostArk::Shared::GATE_PROGRESS_KIND completedKind = LostArk::Shared::GATE_PROGRESS_KIND::END);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		// tick별 전투 기여를 보스에 누적하고 사망 시 방 기록에 합친다. 클리어는 방 기록을 전송한 뒤 비운다.
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId,
			const std::string& spawnPlacementOverrideId = {});
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		// 일반 연출은 현재 방에 재생 명령을 보낸다. 워터팡 시작은 보관해 늦은 입장에도 전달한다.
		// Kouku 입장 연출은 raid 준비로 연결하며, 거절된 요청은 false를 반환한다.
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		// 상호작용 상자의 제안·철회는 해당 플레이어 한 명에게만 보낸다.
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		// 잠든 SpawnGroup을 시작하고, 몬스터가 사라진 완료 그룹은 재시작한다. 실행 중인 그룹에는 파동을 겹치지 않는다.
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		// 자동 진입과 G 상호작용이 공유하는 trigger 대상 실행 경로다.
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		// Leave에서 이탈한 플레이어를 파티 명단에서도 제거한다.
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		// 요청자·sequence·지면 위치를 검증한 핑을 현재 방에 전송한다.
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		// 발신자도 같은 S2C_CHAT을 받아 말풍선을 그리도록 현재 방의 모든 session에 중계한다.
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// 인벤토리는 플레이어 개인 상태이므로 지정 session에만 응답한다.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const SERVER_PLAYER& player);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// 파괴 메시지에 Server 충돌·navigation 진단값을 담아, Client가 벽 표시만으로 통행 가능 여부를 추측하지 않게 한다.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		// 잡힌 대상과 피해 정의를 검증해 플레이어 복사본·피해 이벤트를 준비한다. 실제 반영은 호출자가 담당한다.
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		// stage 사전검증과 전투 transaction 반영 뒤, 플레이어·타깃 관련 동작을 실제 상태에 반영한다.
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		// 일반 벽과 외곽 벽 파괴를 한 transaction으로 반영하고 바닥은 유지한다. 임계 구간 시험에 필요한 아레나를 함께 준비한다.
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		// 보스 이동을 navigation이 허용한 마지막 표본까지 제한하고 벽 접촉은 충돌 경로에 맡긴다.
		// navigation 미로드 또는 시작점부터 바닥 밖이면 목표점을 그대로 돌려준다.
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		// 현재 패턴 stage의 기둥 생성·지정 슬롯 파괴를 반영한다. 정의 세대·슬롯 준비나 commit 실패는 false다.
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Initialize_WorldPickups();
		void Reset_WorldPickups(std::uint32_t serverTick);
		void Update_WorldPickups(std::uint32_t serverTick, bool allowCollection = true);
		void Apply_WorldPickupDestruction(const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::uint32_t serverTick);
		void Remove_RemainingWorldPickups(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		// 일반 보스 이동은 몸체가 스친 벽을 파괴한다. 돌진 충격은 별도 경로에서 receiver와 일반 접촉을 함께 처리한다.
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		// 망원경을 획득한 플레이어로 카드미로를 시작하고 표적을 만든다. 망치는 정해진 타격 tick에 한 번 판정한다.
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBouncingBallContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		// 워터팡 경기의 회전 물대포·폭포가 플레이어에 주는 영향을 갱신한다.
		void Update_MaharakaWaterpangHazards(SERVER_PLAYER& player, std::uint32_t updateTick);
        void Handle_MaharakaAITuning(SESSION_ID sessionId, const LostArk::Shared::C2S_MAHARAKA_AI_TUNING& request);
        bool Spawn_MaharakaWaterpangAI();
        void Update_MaharakaWaterpangMatch(std::uint32_t updateTick);
        void Clear_MaharakaWaterpangAI(std::uint32_t keepCount = 0u);
        bool Finish_MaharakaWaterpangMatch();
		// Debug 강제 이벤트를 먼저, 없으면 경기 일정을 조회한다. 둘 다 없으면 false다.
		bool Sample_MaharakaWaterpangNow(std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const;
		// Server의 물총 장착 조건을 판정하며 결과는 PLAYER_SNAPSHOT.isWaterpangArmed로 전달한다.
		bool Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const;
		// 물총 Q/W/E/R의 쿨타임을 적용하고 발사를 예약한다. 이동 자체를 잠그지는 않는다.
		bool Try_StartMaharakaWaterGunSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command);
		// 예약된 물줄기를 진행시키고 맞은 플레이어를 밀어낸다.
		void Update_MaharakaWaterGunShots(std::uint32_t updateTick);
		// 강제 이벤트 종료 뒤에도 잠시 유지해 마지막 밀림의 낙하를 처리한다.
		bool Is_MaharakaWaterpangDebugEventLive(std::uint32_t tick) const;
		// Debug F1 패턴 버튼으로 방 전체의 워터팡 이벤트를 시작한다.
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Start_MaharakaWaterpangDebugEvent(const std::string& instanceId);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		// 카드미로 시작 전 광대 상자를 생성하고 파괴 여부를 유지한다. Clear는 남은 상자와 기록을 정리한다.
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		// 빙고 표식 만료 시 보유자 위치에 폭탄을 설치한다. 보유자가 방을 떠나면 표식도 제거한다.
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		// 게이지를 사용하는 stance 유지 중이면 직업별 이동 배율을, 그 외에는 1을 반환한다.
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		// 이번 tick에 플레이어 이동을 막는 몬스터·보스 몸체를 충돌 시스템에 전달한다.
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		// 보스 패턴은 복제 ID로 잡기를 요청한다. 방이 행동 중단·고정 tick 위치 보정·해제를 함께 책임진다.
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		// 넉백 시간을 진행하며 바닥·몸체 충돌로 이동을 제한한다. 벽에 막히면 일찍 끝낸다.
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);
		//고정 tick 안에서만 호출하는 개체별 처리. false는 호출자의 남은 갱신을 중단한다.
		bool Update_WorldEntity(SERVER_WORLD_ENTITY& entity, float fixedDeltaSeconds, std::uint32_t updateTick);
		bool Update_ValtanWorldEntity(SERVER_WORLD_ENTITY& entity, float fixedDeltaSeconds, std::uint32_t updateTick);
		bool Update_ValtanStageMotion(SERVER_WORLD_ENTITY& entity, const CGameplayCatalog& occurrenceCatalog,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		//보상은 제거 전, 부착·공격 객체 정리는 제거 직전에 수행한다.
		void Release_WorldBossAttachments(const SERVER_WORLD_ENTITY& boss, std::uint32_t updateTick);
		void Grant_ValtanRaidClearRewards();
		bool Despawn_FinishedWorldEntities(std::uint32_t updateTick);


	private:
		// 합칠 기존 명령이 없고 일반 큐가 768개 이상이면 새 best-effort를 버려 reliable 여유를 남긴다. reliable 상한은 960개다.
		// Tick은 일반 명령 최대 256개를 소비한다. LEAVE는 이 상한을 공유하지 않는 별도 우선 큐에서 session별로 합친다.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		// 수신 큐·정리 identity·공유 계측과 봉인/최초 실패 기록 경계를 보호한다. gameplay 컨테이너 전체의 잠금은 아니다.
		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		// LEAVE가 큐에 있거나 Tick이 처리 중인 session을 붙잡아 중복 정리와 새 일반 명령을 막는다.
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		// 아래 값은 룸 worker의 직전 로그 비교 표본이다. pending 문자열은 최대 최신 한 줄만 보관한다.
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		// 회수 준비가 false로 봉인하면 이후 Enqueue는 거절한다. ready와 달리 생성 성공 여부가 아니다.
		bool m_acceptsCommands = true;
		// 룸 처리에서 준비한 이동 요청을 소유한다. ServerApp의 dequeue가 가져간 뒤 실제 admission을 수행한다.
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		// Bern에서 Maharaka로 요청을 꺼낼 때 session별 선박·부두 좌표를 기억해 복귀 준비가 사용한다.
		// world 좌표는 m, yaw는 degree다. 실제 전환 성공 전 기록이며 256개 이상이면 기존 기억 전체를 비운다.
		struct SHIP_RETURN_STATE final
		{
			LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
			float fDockX = 0.f;
			float fDockY = 0.f;
			float fDockZ = 0.f;
			float fDockYawDegrees = 0.f;
		};
		std::unordered_map<SESSION_ID, SHIP_RETURN_STATE> m_MaharakaShipReturnBySession;
		// Try_DequeueWorldTransfer가 대표자와 batch 멤버에 호출한다. 다른 목적지나 선박 정보 부재는 해당 session 기억을 지운다.
		void Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		// ServerApp이 소유하는 연결을 weak_ptr로 참조한다. 룸은 아래 player 값과 ID lookup을 별도로 소유한다.
		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;

		// 대사 한 번의 예약을 소유한다. prompt/trigger ID는 catalog 포인터가 아니며 실제 segment는 송신 직전에 ID로 조회한다.
		struct GUIDE_PROMPT_OCCURRENCE
		{
			std::string PromptId, TriggerId;

			// 송신 enqueue가 성공한 다음 읽을 segment index다. 0이면 공간 접촉 이탈로 취소할 수 있는 미시작 예약이다.
			std::size_t NextSegment = 0;
			int Priority = 0;
			bool IsSpaceEnter = false;

			// 같은 prompt를 실제로 발생시킨 공간 trigger ID들을 보관한다. 첫 segment 송신 성공 때 이 source들의 cooldown을 함께 기록한다.
			std::vector<std::string> SpaceTriggerIds;
			bool operator==(const GUIDE_PROMPT_OCCURRENCE&) const = default;
		};

		// 단일 Bern actor에 연결된 owner별 제어 상태다. 룸 실행이 직렬로 갱신하며 아래 큐/시계 자체에 별도 mutex는 없다.
		struct GUIDE_RUNTIME
		{
			// PlayerId는 안내 actor, AnchorId는 현재 같은 방 owner player다. owner 재입장 commit 뒤 AnchorId를 새 binding으로 바꾼다.
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;

			// ServerApp이 소유하는 실제 연결을 약하게 참조한다. worker는 사용 중 lock한 shared_ptr로 수명을 붙잡는다.
			std::weak_ptr<CClientSession> OwnerSession;

			// owner가 다른 world에 나간 동안에도 안내 소유 표시를 보낼 수 있도록 마지막 승인 entity ID를 보관한다.
			LostArk::Shared::NET_ENTITY_ID OwnerNetEntityId = 0;

			// 순서대로 월드 이탈 대기·실제 선박 승선 대기·도보 재접근 상태다. 월드/선박 대기는 일반 판단과 대사 진행을 건너뛴다.
			bool WaitingForOwner = false, WaitingForShip = false, ReturningOnFoot = false;

			// 현재 combo와 진행 중 skill 뒤에 적용할 대기 combo의 ID, 마지막 판단 이유다. Stop은 두 예약을 비운다.
			std::string ComboId, PendingComboId, Reason;

			// 승인된 공통 skill 실행만 다음 단계로 전진시킨다. 반복 combo는 이 index를 되돌려도 전체 제한 시간은 유지한다.
			std::size_t ComboStep = 0;

			// 모두 초 단위다. 판단 누적·combo 전체 시간·실행 거절 대기·원거리 복구 대기·행동 유지·대사 남은 시간을 나눈다.
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f,
				FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;

			// 가이드가 공통 이동/skill/탈것 executor로 보내는 내부 명령 순번이다. 사용자 GUIDE_CONTROL 순번과 별도로 유지한다.
			std::uint32_t Sequence = 0;

			// 정확히 일치한 도움 명령의 처리 tick이다. 실제 skill 승인이나 prompt 송신 성공 시각은 아니다.
			std::map<std::string, std::uint32_t> CommandTicks;

			// 판단/추적용 행동 코드다. SERVER_PLAYER의 실제 eAction과 별개이며 게시 trace가 이 값과 점수를 소비한다.
			std::uint8_t Action = 0;
			float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;

			// 진행 중 맨 앞 대사는 유지하고 미시작 대사는 priority 내림차순/동순위 입력 순으로 보관한다.
			std::deque<GUIDE_PROMPT_OCCURRENCE> PromptQueue;

			// 첫 segment 송신 성공 tick만 기록하는 trigger cooldown이다. 취소·한도 초과·송신 실패는 기록하지 않는다.
			std::map<std::string, std::uint32_t> TriggerTicks;

			// 직전 owner 공간 접촉을 보관해 바깥→안 edge를 만든다. 처음 시작/복귀 위치는 Seed가 기준값으로 채운다.
			std::unordered_set<std::string> InsideBoxes;

			// 직전 관측한 boss별 pattern sequence다. 한 판단에서 모든 trigger를 검사한 뒤 갱신한다.
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};

		// 초기화가 읽은 안내 설정/정의를 룸이 소유한다. 아래 상태는 정의와 별개인 process-session 실행 상태다.
		CGuideCatalog m_GuideCatalog;

		// Bern의 배치 안내 actor 하나에 최대 한 owner만 연결한다. 별도 companion clone이나 인간 party slot은 만들지 않는다.
		std::map<SESSION_ID, GUIDE_RUNTIME> m_PersonalGuides;

		// 대사 segment 송신 시도 전에 올리고 0을 건너뛴다. 송신 실패에도 이 순번은 소비될 수 있다.
		std::uint32_t m_iGuideEventSequence = 0;

		// Loaded인 Bern 갱신마다 누적하는 초 값이다. owner 부재 또는 월드 대기이고 ThinkSeconds에 도달하면 소유 표시를 보내고 0으로 만든다.
		float m_fGuideOwnershipElapsed = 0.f;

		// session별 마지막 사용자 GUIDE_CONTROL 순번이다. actor 검사 이전에 갱신하고 Leave가 지워 새 Level의 순번을 받는다.
		std::unordered_map<SESSION_ID, std::uint32_t> m_GuideControlSequences;

		// Bern 초기 배치 actor의 room-local ID다. 안내 종료 후에도 같은 actor를 계속 사용한다.
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;

		// 초기 actor 할당용 player ID 영역이다. 공통 NetEntityId는 별도 m_iNextNetEntityId에서 가져온다.
		LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		// 시작한 스킬의 버프를 시전자·같은 방 파티·지정 대상에게 적용한다.
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		// PLAYER_ID와 파티 ID는 방 안에서만 유효하며 0은 파티 없음이다. 초대·명단은 이 방이 소유한다.
		// 파티 전환은 전원 입장을 준비한 뒤 대상 방의 새 ID로 함께 구성하며, 방 사이의 일반 파티 ID는 유지하지 않는다.
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// 대상별 미응답 초대는 하나만 유지하고 새 초대로 교체한다.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// 플레이어별 최신 전환 실패를 보관한다. 송신 큐가 가득 차면 통지를 미루고 기존 파티 연결은 유지한다.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		// Bern에서 승인받은 session을 입장 순서대로 유지하는 콜로세움 대기열이다.
		struct COLOSSEUM_QUEUE_ENTRY
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iRequestSequence = 0u;
		};
		std::vector<COLOSSEUM_QUEUE_ENTRY> m_ColosseumQueue;
		bool m_bColosseumTransferPending = false;
		std::uint32_t m_iColosseumRetryTick = 0u;
		std::uint64_t m_iColosseumMatchId = 0u;
		LostArk::Shared::COLOSSEUM_MATCH_PHASE m_eColosseumPhase = LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING;
		std::uint8_t m_iColosseumWinnerTeam = 255u;
		std::uint32_t m_iColosseumRevision = 1u;
		std::array<std::uint32_t, 2> m_ColosseumTeamPartyIds{};
		enum class COLOSSEUM_TACTIC : std::uint8_t { WAIT, ENGAGE, REPOSITION, RETREAT, EVADE, RECOVER };
		struct COLOSSEUM_MERCENARY_RUNTIME final
		{
			float fThinkElapsed = 0.f;
			float fSenseElapsed = 0.f, fDecisionInterval = .2f, fThreatAgeSeconds = 0.f;
			std::uint32_t iSequence = 0u;
			// 경기가 소유하는 입장 시각이므로 플레이어 부활·전술 초기화 뒤에도 유지한다.
			std::optional<std::uint32_t> iLastAltVAdmissionTick;
			std::uint64_t iRandomState = 0u;
			std::vector<LostArk::Shared::SKILL_ID> AvailableSkills;
			std::array<LostArk::Shared::SKILL_ID, 3> RecentSkills{};
			std::size_t iRecentSkillCursor = 0u;
			LostArk::Shared::SKILL_ID iLastSkillId = LostArk::Shared::INVALID_SKILL_ID;
			LostArk::Shared::NET_ENTITY_ID iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iTargetSelectedTick = 0u, iTacticUntilTick = 0u;
			std::uint32_t iNextAttackTick = 0u, iNextEvadeTick = 0u, iRetreatAllowedTick = 0u;
			std::uint32_t iLastDamageTick = 0u, iObservedHp = 0u;
			COLOSSEUM_TACTIC eTactic = COLOSSEUM_TACTIC::ENGAGE;
			CColosseumThreatAssessment Threats;
			const char* pReason = "Waiting for recruitment";
		};
		std::map<LostArk::Shared::PLAYER_ID, COLOSSEUM_MERCENARY_RUNTIME> m_ColosseumMercenaries;
		std::unordered_map<SESSION_ID, std::uint32_t> m_ColosseumRecruitSequences;
		std::vector<SESSION_ID> m_ColosseumSessions;
		std::uint32_t m_iColosseumPhaseStart = 0u, m_iColosseumPhaseEnd = 0u;
		std::uint32_t m_iColosseumScores[2]{};
		std::uint32_t m_iColosseumKillSequence = 0u;
		std::vector<LostArk::Shared::COLOSSEUM_KILL_EVENT> m_ColosseumRecentKills;
		std::uint32_t m_iColosseumQueueDeadline = 0u, m_iColosseumQueueBroadcastTick = 0u;
		std::array<std::uint8_t, 2> m_ColosseumInitialHumans{};
		GATE_PROGRESS_STATE m_GateProgress;
		std::uint32_t m_iArenaAssemblyStartTick = 0u;
		std::uint32_t m_iArenaAssemblyRaidEpoch = 0u;
		bool m_bArenaAssemblyAttempted = false;
		std::vector<LostArk::Shared::PLAYER_ID> m_ArenaAssemblyParticipants;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		// 월드·공간 시스템과 세대 보관 멤버는 simulation별로 소유한다.
		// 불변 catalog 객체는 공유할 수 있지만 active·예약 상태는 방마다 따로 보관한다.
		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		// 수치 Stage 성공 시 entry snapshot과 원본→대체 catalog 소유권을 예약한다. Commit 또는 Abort가 소비/해제한다.
		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> m_StagedNumericEntries;
		std::vector<std::pair<std::shared_ptr<const CGameplayCatalog>, std::shared_ptr<const CGameplayCatalog>>>
			m_StagedNumericCatalogRemaps;
		// 아이템·탈것·칭호·보상 정의는 룸 생성 시 읽는다. gameplay 세대의 수치 교체와 별도 객체다.
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
        struct MAHARAKA_WATERPANG_AI final
        {
            std::uint32_t iSlot = 0u, iSequence = 0u, iNextThinkTick = 0u, iNextMoveTick = 0u, iNextShotTick = 0u, iSkillSlot = 0u;
        };
        std::map<LostArk::Shared::PLAYER_ID, MAHARAKA_WATERPANG_AI> m_MaharakaWaterpangAI;
        LostArk::Shared::PLAYER_ID m_iNextWaterpangAIPlayerId = 0x90000000u;
        std::uint32_t m_iWaterpangAIRetryTick = 0u;
        LostArk::Shared::MAHARAKA_AI_TUNING m_MaharakaAITuning;
        bool m_bMaharakaAITuningLoaded = false;
        std::string m_strMaharakaAISourceBytes;
		// 방 전체의 워터팡 시작 연출을 빈 방이 될 때까지 보관해 늦은 입장에도 전달한다.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		// 워터팡 발사부터 소멸까지의 물줄기 상태다. 이미 맞은 대상을 기억해 중복 피격을 막는다.
		struct MAHARAKA_WATERGUN_SHOT final
		{
			LostArk::Shared::PLAYER_ID iOwnerId = 0;
			std::uint32_t iSkillId = 0u;
			std::uint32_t iSpawnTick = 0u;
            std::uint32_t iProjectileIndex = 0u;
            LostArk::Shared::COMBAT_OBJECT_ID iVisualObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
            LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fAimDistanceM = 0.f;
			bool bLaunched = false;
			bool bSpent = false;
			float fX = 0.f, fY = 0.f, fZ = 0.f;
			float fDirX = 0.f, fDirZ = 1.f;
			float fTravelM = 0.f;
			float fReachM = 0.f;
			std::vector<LostArk::Shared::PLAYER_ID> Struck;
		};
		std::vector<MAHARAKA_WATERGUN_SHOT> m_MaharakaWaterGunShots;
		// Debug 폭포·물대포 재생은 다음 요청으로 교체하며 늦은 입장에도 전달하도록 보관한다.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangDebugEvent;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		std::mt19937 m_EquipmentUpgradeRandom{std::random_device{}()};
		// Mario 1~4단계별 터진 공 기록이며 각 bit는 bootstrap 슬롯을 가리킨다.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u, iActivationTick = 0u;
            BOSS_BINGO_BOARD_SETTINGS Settings;
            bool bActivated = false, bInitializeCells = false;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);
        void Activate_KoukuBingoBoard(std::uint32_t tick);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		struct WORLD_PICKUP_RUNTIME final
		{
			WORLD_PICKUP_DESCRIPTOR Descriptor;
			LostArk::Shared::WORLD_PICKUP_SNAPSHOT Snapshot;
		};
		std::vector<WORLD_PICKUP_RUNTIME> m_WorldPickups;
		std::uint32_t m_iWorldPickupEncounterEpoch = 0u;
		// 전투 중 기둥을 다시 세우고 부술 수 있도록 가역 상태를 소유한다.
		CEncounterPropRuntime m_EncounterPropRuntime;
		// 주 Valtan 사망 시 despawn 전에 클리어를 기록한다. 마지막 플레이어 이탈의 방 초기화에서 해제한다.
		bool m_bValtanRaidCleared = false;
		// 기둥 시험의 생성 후 일괄 파괴 예약 tick과 대기 플래그다. 제품의 stage별 슬롯 파괴와 별도로 유지한다.
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		// 룸이 실제 월드 개체 값을 소유한다. vector 변경을 넘겨 개별 원소 포인터를 계속 보관할 수는 없다.
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		// 유효한 simulation 단계 시작에서 비우고 그 Tick의 snapshot에 담을 피해 event를 소유한다.
		// snapshot 없이 조기 중단하면 전송을 보장하지 않으며 별도 재송신 원장은 아니다.
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		// 명령 처리 중 만든 회복/피해 표시를 위 목록의 clear와 분리해 보관한다.
		// simulation 시작 뒤 wire 개수 상한까지 옮기고 원본을 비운다. 상한 초과 항목의 전송은 보장하지 않는다.
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		// 최초 runtime 실패를 따로 보관한다. 일반 status는 이후 달라질 수 있으며 ready false는 정상 Tick을 중단한다.
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		// 유효한 simulation 단계가 계산하는 서버 tick이다. 0을 건너뛰며 Tick 호출 횟수 계측과 일치한다고 가정하지 않는다.
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		// 구 HP 임계 시험의 잔여 상태는 초기화·배타 검사에 남아 있다. 현재 HP 구명령은 거절한다.
		// preset sequence와 stable-ID 패턴 receipt를 나눠 서로 다른 요청의 재전송을 판정한다.
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			// 현재 시험이 끝나도 마지막 lifecycle을 보관해, QUEUED 요청의 같은 재시도에 완료 상태까지 응답한다.
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		// stable-ID Play/Restart의 원문과 결과를 보관한다. 완전히 같은 재시도에만 결과를 재사용한다.
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			// 동일 Start 재시도는 COMPLETED_HOLD 이후에도 마지막 lifecycle을 돌려주며 새 실행을 만들지 않는다.
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
```

## 프로젝트 등록

`Server/Default/Server.vcxproj`의 기존 `GameRoom_BossSimulation.cpp` ClCompile 바로 아래에 다음 블록을 넣는다.
새 파일을 프로젝트에서 실제 컴파일하기 위한 등록이다.

```xml
    <ClCompile Include="..\Private\GameRoom_WorldEntitySimulation.cpp" />
    <ClCompile Include="..\Private\GameRoom_ValtanSimulation.cpp" />
    <ClCompile Include="..\Private\GameRoom_ValtanGhost.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuTargeting.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuMechanics.cpp" />
```

`Server/Default/Server.vcxproj.filters`의 기존 `GameRoom_BossSimulation.cpp` ClCompile 닫는 태그 바로 아래에 다음 블록을 넣는다.
기존 Shared·Valtan·KoukuSaydon 필터를 사용하며 필터 정의나 기존 파일 배치를 바꾸지 않는다.

```xml
    <ClCompile Include="..\Private\GameRoom_WorldEntitySimulation.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_ValtanSimulation.cpp">
      <Filter>07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_ValtanGhost.cpp">
      <Filter>07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuTargeting.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuMechanics.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
```

## 적용과 검증

1. 기존 네 파일의 hash를 준비 시점과 대조한다. 변경됐으면 최신 파일에서 이번 필드만 병합한다.
2. 새 CPP 다섯 개, 기존 CPP·H, 프로젝트·필터를 반영한다. 기존 UTF-8/BOM·개행을 유지한다.
3. 원본 함수 토큰·주석, 갱신 순서와 실패 경로를 독립 검토한다. XML parse와 각 CPP의 중복 없는 등록을 확인한다.
4. 정상 Product Debug 빌드를 수행한다. Rebuild·Clean·tlog 제거를 하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory out/BossSimulationRefactor20261014/build
```

5. `Server/Default`를 작업 디렉터리로 기존 계약 검사를 실행하고 전후 로그를 구분한다.

```powershell
../Bin/Debug/Server.exe --kouku-product-contract-test
../Bin/Debug/Server.exe --kouku-object-overlap-contract-test
../Bin/Debug/Server.exe --valtan-lifecycle-contract-test
```

6. 변경 범위 `git diff --check`, 소스/계획서 일치와 실행 종료를 확인한다. Client/UI를 자동으로 실행하지 않는다.
7. 사용자 화면 확인은 Debug Lobby → Valtan/KoukuSaydon 승인 진입 → F1 게시 패턴 재생에서 수행한다.
   기존 패턴 동작·피해 결함의 해결과 이 구조 분리의 빌드·회귀 통과는 구분한다. 화면 검증 여부는 RESULT에 따로 남긴다.


## G0 후속 — 분리된 호출자와 추적 방향 주석 참조

파일 분리 뒤 예전 CPP를 가리키는 주석 세 줄만 교체한다. 아래 전체 코드를 적용 후보로 먼저 기록하며, 함수 선언·실행 토큰·상태·입력 단위와 기존의 나머지 주석을 보존한다. 적용 직전 각 파일의 hash가 준비 시점과 같은지 다시 확인한다.

### Server/Public/ValtanClearRewards.h

변경 종류: 기존 주석 한 줄 교체.

보상 목록을 차용하는 실제 함수와 구현 파일을 가리킨다. 반환 참조의 소유·수명 설명과 선언은 유지한다.

```cpp
#pragma once

#include "Network/PacketType.h"

#include <array>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// GameRoom이 발탄 클리어 때 읽을 직업별 보상 item ID 목록이다. Data/Valtan/Valtan.clearrewards.json의 게시 결과를 사용한다.
	// 실제 reader 입력은 DataFiles/Valtan/ClearRewards.bootstrap이며 이 클래스는 JSON을 직접 읽거나 아이템을 지급하지 않는다.
	// 목록 순서는 지급 순서로 유지한다. 주 발탄의 DEAD/중복 지급 방지와 실제 지급 성공 여부는 GameRoom이 담당한다.
	class CValtanClearRewards final
	{
	public:
		// GameRoom 초기화에서 호출한다. false 반환이면 이전 직업별 목록을 복원하고 상태 문자열에는 실패 이유를 남긴다.
		// item ID 형식만 검증하며 ItemCatalog에 실제 존재하는지 또는 지급 가능한지까지 여기서 확인하지 않는다.
		bool Load();

		// GameRoom_WorldEntitySimulation.cpp의 Grant_ValtanRaidClearRewards가 목록을 차용한다. 잘못된 직업은 정적인 빈 목록을 반환한다.
		// 유효 직업 목록의 참조는 이 객체가 소유하며 성공한 재로드/파괴 뒤에는 재사용하지 않는다.
		const std::vector<std::string>& Get_ItemIds(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;

		// 최근 로드 결과의 차용 참조다. 다음 Load가 내용을 갱신할 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	private:
		// 알려진 직업 enum을 배열 index로 사용하고 각 vector가 item ID 문자열을 소유한다. 현재 player의 인벤토리는 저장하지 않는다.
		using CLASS_ITEM_IDS = std::array<std::vector<std::string>,
			static_cast<std::size_t>(LostArk::Shared::CHARACTER_CLASS_ID::END)>;
		CLASS_ITEM_IDS m_ItemIds;
		std::string m_strStatus;
	};
}
```

### Server/Public/ValtanBrain.h

변경 종류: 기존 주석 한 줄 교체.

ValtanBrain::Update의 직접 호출자를 분리된 CGameRoom::Update_ValtanWorldEntity로 바로잡는다. 입력 단위와 갱신 계약은 유지한다.

```cpp
#pragma once

#include "GameplayCatalog.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/CombatObjectHitChain.h"
#include "ServerNavigation.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// 패턴 선택의 출처를 기록한다. 선택 결과와 별개로 강제 요청, 순차 선택, 가중치 선택을 구분한다.
	enum class VALTAN_DECISION_SOURCE : std::uint8_t
	{
		NONE,
		INTRO,
		FORCED_HEALTH_BAR,
		FORCED_AUDITION,
		ORDERED,
		WEIGHTED,
		GLOBAL,
		// 기존 Debug 호출부의 소스 호환성을 유지하는 별칭이다. 새 기록은 위의 명시적인 이름을 사용한다.
		FORCED_MECHANIC = FORCED_HEALTH_BAR,
		ROTATION_ORDERED = ORDERED,
		ROTATION_WEIGHTED = WEIGHTED,
		GLOBAL_WEIGHTED = GLOBAL
	};

	// 선택 성공뿐 아니라 대기, 후보 부재, 카탈로그 부재도 남겨 디버그 조회에서 원인을 구분한다.
	enum class VALTAN_DECISION_RESULT : std::uint8_t
	{
		SELECTED,
		WAITING_FOR_INTRO_RANGE,
		NO_ELIGIBLE_PATTERN,
		NO_VALID_TARGET,
		CATALOG_UNAVAILABLE,
		MECHANIC_RESET_REQUIRED
	};

	// 한 후보가 여러 제외 사유를 동시에 가질 수 있으므로 비트 마스크를 사용한다.
	// SOFT_REPEAT_BLOCKED는 영구 제외가 아니다. 다른 조건을 통과한 후보가 모두 반복 제한에 걸릴 수 있다.
	// 이때 선택기가 SOFT_REPEAT_RELAXED를 기록하고 원래 가중치로 추첨한다.
	enum VALTAN_DECISION_EXCLUSION : std::uint32_t
	{
		VALTAN_EXCLUDE_NONE = 0u,
		VALTAN_EXCLUDE_WRONG_SELECTION_KIND = 1u << 0u,
		VALTAN_EXCLUDE_INTRO_ROW = 1u << 1u,
		VALTAN_EXCLUDE_NOT_IN_SELECTION_SET = 1u << 2u,
		VALTAN_EXCLUDE_ARMOR_MISMATCH = 1u << 3u,
		VALTAN_EXCLUDE_PHASE_REQUIREMENT = 1u << 4u,
		VALTAN_EXCLUDE_PHASE_RANGE = 1u << 5u,
		VALTAN_EXCLUDE_HEALTH_BAR_RANGE = 1u << 6u,
		VALTAN_EXCLUDE_NO_TARGET = 1u << 7u,
		VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE = 1u << 8u,
		VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE = 1u << 9u,
		VALTAN_EXCLUDE_COOLDOWN = 1u << 10u,
		VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED = 1u << 11u,
		VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED = 1u << 12u,
		VALTAN_EXCLUDE_DISABLED = 1u << 13u,
		VALTAN_EXCLUDE_UNRESOLVED_DEFINITION = 1u << 14u,
		// 초기 디버그 관측 코드가 사용한 이름과 비트 값을 유지한다.
		VALTAN_EXCLUDE_NOT_NORMAL = VALTAN_EXCLUDE_WRONG_SELECTION_KIND,
		VALTAN_EXCLUDE_INTRO = VALTAN_EXCLUDE_INTRO_ROW,
		VALTAN_EXCLUDE_ROTATION = VALTAN_EXCLUDE_NOT_IN_SELECTION_SET,
		VALTAN_EXCLUDE_ARMOR = VALTAN_EXCLUDE_ARMOR_MISMATCH,
		VALTAN_EXCLUDE_HEALTH_BAR = VALTAN_EXCLUDE_HEALTH_BAR_RANGE,
		VALTAN_EXCLUDE_RANGE = VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE |
			VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE,
		VALTAN_EXCLUDE_MAXIMUM_CONSECUTIVE =
			VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED,
		VALTAN_EXCLUDE_MISSING_DEFINITION =
			VALTAN_EXCLUDE_UNRESOLVED_DEFINITION
	};

	// 한 번의 선택에서 후보 한 개가 통과하거나 탈락한 근거를 값으로 보관한다.
	// 정의의 주소를 저장하지 않으므로 이후 카탈로그 교체와 독립적으로 당시 결과를 조회할 수 있다.
	struct VALTAN_DECISION_CANDIDATE_TRACE final
	{
		std::string strPatternId;
		std::uint32_t iExclusionMask = VALTAN_EXCLUDE_NONE;
		// 저작 가중치와 조건 적용 후 실제 추첨 가중치를 나눠 제한이 미친 영향을 보여 준다.
		std::uint32_t iAuthoredWeight = 0u;
		std::uint32_t iEffectiveWeight = 0u;
		std::uint32_t iCooldownRemainingTicks = 0u;
		std::uint32_t iConsecutiveUses = 0u;
		std::uint32_t iMaximumConsecutiveUses = 0u;
		// 누적 가중치 구간은 [시작, 끝)이다. 추첨 번호가 이 구간에 들어오는 후보가 선택된다.
		std::uint64_t iWeightBeginInclusive = 0u;
		std::uint64_t iWeightEndExclusive = 0u;
		bool bSoftRepeatBlocked = false;
		bool bSoftRepeatRelaxed = false;
		bool bSelected = false;
	};

	// 선택 시점의 입력 상태, 난수와 후보 결과를 묶어 당시 HP, 대상과 선택 경로를 보존한다.
	// CValtanBrain의 제한된 순환 버퍼가 이 값을 소유한다.
	struct VALTAN_DECISION_TRACE final
	{
		// 기록 순번과 서버 틱은 다른 값이다. 패턴 실행 순번도 별도로 남겨 같은 틱의 판단과 실행을 연결한다.
		std::uint64_t iTraceSequence = 0u;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequenceBeforeDecision = 0u;
		std::uint32_t iExpectedPatternSequence = 0u;
		std::uint32_t iCurrentHp = 0u;
		std::uint32_t iMaximumHp = 0u;
		std::uint32_t iHealthBar = 0u;
		std::uint8_t iGameplayPhase = 1u;
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fTargetDistance = 0.f;
		bool bIntroPatternConsumed = false;
		std::uint32_t iRotationStepIndex = 0u;
		VALTAN_DECISION_SOURCE eSource = VALTAN_DECISION_SOURCE::NONE;
		VALTAN_DECISION_RESULT eResult =
			VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN;
		std::string strRotationId;
		// 아직 실행하지 못한 요청과 이번에 선택한 결과를 분리해 강제 패턴의 대기 사유를 추적한다.
		std::string strPendingPatternId;
		VALTAN_DECISION_SOURCE ePendingSource =
			VALTAN_DECISION_SOURCE::NONE;
		std::string strSelectedPatternId;
		// 원래 난수 입력, 혼합 결과, 전체 가중치와 최종 추첨 번호를 함께 보관한다.
		std::uint64_t iRawRandomInput = 0u;
		std::uint64_t iMixedRandomValue = 0u;
		std::uint64_t iTotalWeight = 0u;
		std::uint64_t iRandomTicket = 0u;
		bool bMaximumConsecutiveRelaxed = false;
		bool bCandidatesTruncated = false;
		// 후보 기록은 MAX_DECISION_CANDIDATE_COUNT로 제한한다. 잘린 기록은 위 플래그로 구분한다.
		std::vector<VALTAN_DECISION_CANDIDATE_TRACE> Candidates;
	};

	// Brain은 CAPTURE 공격의 명중과 생존 여부를 판단하고, GameRoom은 행동 중단과 부착 수명을 관리한다.
	// 같은 고정 틱 안에서 두 책임을 잇는 요청이다. 플레이어 포인터 대신 네트워크 ID를 넘긴다.
	// 소비 시 GameRoom이 현재 플레이어를 다시 찾고 부착을 적용한다.
	struct SERVER_PLAYER_CAPTURE_REQUEST final
	{
		LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	};

	enum class SERVER_BOSS_GRAB_CLASSIFICATION : std::uint8_t
	{
		NONE,
		PARTIAL,
		ALL
	};

	// 안내 NPC를 제외한 생존 플레이어와 이 보스의 현재 패턴에 유효하게 붙잡힌 플레이어를 집계한다.
	// 잘못된 부착이 하나라도 있으면 전원 포획으로 확정하지 않도록 별도 플래그를 둔다.
	struct SERVER_BOSS_GRAB_ROSTER final
	{
		std::size_t iAliveCount = 0u;
		std::size_t iGrabbedCount = 0u;
		bool bHasInvalidAttachment = false;
		SERVER_BOSS_GRAB_CLASSIFICATION eClassification =
			SERVER_BOSS_GRAB_CLASSIFICATION::NONE;
	};

	// Brain이 산출한 부채꼴 공격을 GameRoom의 오브젝트 피격 처리로 전달하는 값이다.
	// 공격 출처와 패턴 실행 순번을 함께 넘겨 해당 공격의 오브젝트 반응과 연결한다.
	struct SERVER_BOSS_PATTERN_HIT final
	{
		LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		std::string strPatternId, strActionId;
		LostArk::Shared::CombatObjectHitChain::CONE_HIT Cone;
	};

	// GameRoom의 보스 갱신에서 패턴 선택, 단계 진행과 플레이어 명중을 처리한다.
	// 보스와 플레이어의 실제 상태는 호출자가 소유하고, Brain은 선택 판단의 최근 기록만 보관한다.
	// 이동 후보의 충돌 검증, 단계 액션의 적용과 포획 부착은 GameRoom의 후속 처리로 연결된다.
	class CValtanBrain final
	{
	public:
		// GameRoom의 안내 NPC 위협 판단이 호출하는 읽기 전용 조회다. horizonMs는 앞으로 살펴볼 밀리초다.
		// 현재 보스 위치와 공격 도형, 단계별 명중 시점을 사용해 probe가 닿을 예정인 공격 수를 누적한다.
		// 미래 이동 전체를 시뮬레이션한 피해량이나 확률이 아니며 실제 피해는 Update의 명중 처리에서 정한다.
		[[nodiscard]] static float Predict_ContactRisk(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const SERVER_PLAYER& probe,
			std::uint32_t serverTick, std::uint32_t horizonMs);

		// 기록과 기믹 발생 내역의 보관량을 제한한다. 오래된 선택 기록은 새 기록이 들어오면 덮어쓴다.
		static constexpr std::size_t MAX_DECISION_TRACE_COUNT = 32u;
		static constexpr std::size_t MAX_DECISION_CANDIDATE_COUNT = 64u;
		static constexpr std::size_t MAX_MECHANIC_OCCURRENCE_COUNT = 64u;

		// Brain의 분기 선택과 GameRoom의 단계 액션 검증이 같은 포획 기준을 사용한다.
		// 생존, 보스 ID, 현재 패턴 순번, 손 부착 슬롯과 유한한 부착 좌표를 확인하며 입력은 변경하지 않는다.
		[[nodiscard]] static SERVER_BOSS_GRAB_ROSTER Classify_GrabbedPlayers(
			const SERVER_WORLD_ENTITY& boss,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);

		// 현재 HP를 전체 체력줄 수에 비례시켜 올림한다. HP가 남아 있으면 마지막 줄도 1로 표시한다.
		// 패턴의 체력줄 임계값 판단과 GameRoom의 상태 조회가 같은 환산을 사용한다. 유효한 HP 범위가 없으면 0이다.
		[[nodiscard]] static std::uint32_t Calculate_HealthBar(
			const SERVER_WORLD_ENTITY& boss);
		// GameRoom의 Debug 체력줄 지정에서 최대 HP * healthBar / 전체 체력줄 수를 내림해 HP를 구한다.
		// 유효한 요청의 계산값은 최소 1이며, 0 또는 저작 범위를 벗어난 체력줄과 유효하지 않은 최대값은 0이다.
		// HP가 정수이므로 모든 가능한 설정에서 Calculate_HealthBar의 정확한 역함수인 것은 아니다.
		[[nodiscard]] static std::uint32_t Resolve_HealthBarHp(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t healthBar);

		// 패턴 후보 검사가 현재 갑옷 상태와 저작 조건을 대조한다. 내구도가 남은 갑옷판이 있으면 ARMORED다.
		// 저작된 갑옷판이 없으면 갑옷이 벗겨진 상태로 취급하며, ANY 조건은 항상 통과한다.
		[[nodiscard]] static bool Is_ArmorRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_ARMOR_REQUIREMENT requirement);

		// 패턴 후보 검사가 현재 게임플레이 페이즈와 저작 조건을 대조한다.
		// 이 함수는 상태를 읽기만 한다. 체력줄 임계값에 따른 페이즈 갱신은 Update 쪽의 책임이다.
		[[nodiscard]] static bool Is_PhaseRequirementMet(
			const SERVER_WORLD_ENTITY& boss,
			BOSS_PATTERN_PHASE_REQUIREMENT requirement);

		// GameRoom의 단계 액션 검증이나 적용이 실패하면 현재 패턴 ID를 Fail_Mechanic에 전달한다.
		// 대상 부재로 재시도할 경우와 초기화가 필요한 실패는 전달된 failure에 따라 구분한다.
		static void Fail_ActiveMechanic(
			SERVER_WORLD_ENTITY& boss,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);
		// GameRoom은 특정 패턴 ID의 기믹 실패도 전달할 수 있다.
		// 현재 실행 중인 패턴과 일치하면 임시 높이 보정을 복원하고 중단 영수증을 남긴다.
		// NO_VALID_TARGET은 진행 또는 대기 중인 기믹을 재예약한다. 다른 실패는 초기화 필요 상태로 바꾼다.
		// 해당 기믹 발생 기록이 없으면 기믹 원장은 변경하지 않는다.
		static void Fail_Mechanic(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			SERVER_BOSS_MECHANIC_FAILURE failure,
			std::uint32_t serverTick);

		// 외부 디버그 조회가 순환 버퍼에서 현재 보관 중인 기록 수를 확인한다.
		[[nodiscard]] std::size_t Get_DecisionTraceCount() const noexcept
		{
			return m_iDecisionTraceCount;
		}
		// age 0은 보관된 기록 중 가장 오래된 항목이며 범위를 벗어나면 nullptr다.
		// 반환 포인터는 Brain 소유 기록을 차용한다. 새 기록이 쌓이면 내용이 덮어써질 수 있다.
		// 내부 동기화가 없으므로 기록 중인 Update와 동시에 조회하지 않도록 호출자가 접근을 조율한다.
		[[nodiscard]] const VALTAN_DECISION_TRACE* Get_DecisionTrace(
			std::size_t age) const noexcept;
		// 가장 최근 기록을 차용하며 아직 기록이 없으면 nullptr다. 수명과 접근 조건은 위 조회와 같다.
		[[nodiscard]] const VALTAN_DECISION_TRACE*
			Get_LatestDecisionTrace() const noexcept;

		// GameRoom_ValtanSimulation.cpp의 Update_ValtanWorldEntity가 고정 틱마다 호출한다. fixedDeltaSeconds는 초, serverTick은 서버 틱이다.
		// 호출자가 소유한 boss와 players에 패턴 진행, 명중과 행동 상태를 반영하며 피해 이벤트를 뒤에 추가한다.
		// coverCircles는 이번 틱의 엄폐물 XZ 원이다. 적용 대상 공격은 계산된 공격 원점과 플레이어 사이의 가림도 확인한다.
		// catalog는 실행 정의를 제공한다. activeThresholdCatalog는 교체된 체력줄 선택 정의를 제공할 수 있다.
		// 제품 시퀀스가 정의를 고정한 동안에는 그 실행 catalog를 계속 사용한다.
		// 선택적 출력 벡터는 GameRoom이 소유하고 같은 틱의 포획 부착과 오브젝트 피격 후속 처리에서 소비한다.
		// automaticSequenceOverride는 외부에서 지정한 시퀀스다. 포인터 입력과 출력은 이 호출 동안만 차용한다.
		// 보스가 아니면 즉시 반환한다. 사망 시에는 진행 중 패턴과 이동을 정리하므로 실패 시 항상 무변경인 API는 아니다.
		void Update(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const CServerNavigation& navigation,
			float fixedDeltaSeconds,
			std::uint32_t serverTick,
			const std::vector<LostArk::Shared::CombatCollision::CIRCLE_XZ>&
				coverCircles,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const CGameplayCatalog* activeThresholdCatalog = nullptr,
			std::uint16_t activeThresholdGenerationEpoch = 1u,
			std::vector<SERVER_PLAYER_CAPTURE_REQUEST>* outCaptureRequests =
				nullptr,
			const BOSS_PATTERN_SEQUENCE_DEFINITION*
				automaticSequenceOverride = nullptr,
			std::vector<SERVER_BOSS_PATTERN_HIT>* outOwnerHits = nullptr) const;
		// GameRoom이 내비게이션 검증 전에 다음 월드 XZ 위치를 요청한다. 좌표는 m, 시간은 초다.
		// 명시적인 단계 이동 또는 루트 모션 차분으로 후보만 계산하며 boss 자체는 움직이지 않는다.
		// 점프와 아레나 횡단처럼 별도 경로가 위치를 소유하는 단계는 여기서 중복 이동시키지 않는다.
		// false일 때 출력은 사용할 수 없다. 마지막 유한성 검사 전 출력에 값을 쓴 경로도 있다.
		bool Try_BuildStageMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		// 현재 계약 테스트가 돌진의 다음 월드 XZ 후보를 확인할 때 호출한다. 속도와 고정 틱 시간으로 계산한다.
		// boss는 변경하지 않으며 false인 경우 출력은 사용하지 않는다. 이 함수 자체는 충돌과 단계 종료를 판단하지 않는다.
		bool Try_BuildImpactMotion(
			const SERVER_WORLD_ENTITY& boss,
			float fixedDeltaSeconds,
			float& outProposedX,
			float& outProposedZ) const;
		// GameRoom이 실제 벽 접촉을 확인한 뒤 WALL_CONTACT 분기를 적용한다.
		// 대응 단계와 분기가 없으면 false다. 분기를 처리하면 돌진 속도를 0으로 만들고 true를 반환한다.
		// 잘못된 다음 정의를 중단 처리한 경우도 true이므로 패턴 성공 여부는 보스 상태와 종료 영수증으로 확인한다.
		bool Complete_ImpactStage(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;

		// GameRoom이 이동 불가를 확인한 뒤 호출한다. 포획 인원 조건에 맞는 분기를 우선 처리한다.
		// 포획 분기가 있으면 그 처리 결과를 바로 반환하고, 없으면 NAVIGATION_BLOCKED 분기를 처리한다.
		// true는 분기가 처리됐다는 뜻이다. 잘못된 다음 정의에 따른 중단도 포함하므로 패턴 성공 여부와 구분한다.
		bool Complete_NavigationBlockedStage(
			SERVER_WORLD_ENTITY& boss,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick) const;
		// GameRoom의 유령 포털 반복 진행에서 기존 고정 정의 revision으로 패턴을 다시 시작한다.
		// 해당 기믹인지, 단계와 생존 HP가 있는지, 틱과 패턴 순번이 유효한지 확인한 후 상태를 갱신한다.
		bool Restart_FinaleCycle(
			SERVER_WORLD_ENTITY& boss,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const BOSS_PATTERN_DEFINITION& pattern,
			std::uint32_t serverTick) const;
		// Brain의 단계 진입과 GameRoom의 단계 액션이 포털 이동 상태를 준비할 때 호출한다.
		// 이전 포털 상태를 먼저 초기화한다. 포털 단계에서는 이동 경로도 비운다.
		// 아레나 횡단은 시작 위치와 방향까지 즉시 설정하므로 단순한 읽기 전용 설정 조회가 아니다.
		static void Configure_PortalMotion(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_DEFINITION& stage);
		// GameRoom의 단계 시작에서 마지막 유효 대상을 향한 돌진 경로와 방향을 한 번 고정한다.
		// 이미 고정됐으면 true다. 아직 고정되지 않았고 입력이 유효하지 않으면 상태 변경 전 false를 반환한다.
		static bool Lock_PortalTargetRushAtStageStart(
			SERVER_WORLD_ENTITY& boss);

	private:
		// Update의 판단 결과를 다음 슬롯으로 이동하고 기록 순번을 부여한다. 포화 시 가장 오래된 기록을 교체한다.
		void Record_DecisionTrace(VALTAN_DECISION_TRACE&& trace) const;

		// const Update에서도 관측 기록은 쌓는다. 보스의 게임플레이 상태는 외부에 있고 이 버퍼만 Brain이 소유한다.
		// 쓰기 위치는 다음 교체 슬롯, 개수는 현재 유효 기록 수이며 다음 순번은 넘칠 때 1부터 다시 시작한다.
		mutable std::array<VALTAN_DECISION_TRACE, MAX_DECISION_TRACE_COUNT>
			m_DecisionTraces{};
		mutable std::size_t m_iDecisionTraceWriteIndex = 0u;
		mutable std::size_t m_iDecisionTraceCount = 0u;
		mutable std::uint64_t m_iNextDecisionTraceSequence = 1u;
	};
}
```

### Server/Private/KoukuSaydonBrain.cpp

변경 종류: 기존 주석 한 줄 교체.

root motion 설명에서 참조하는 실제 추적 방향 보정의 파일과 함수를 명시한다. 좌표 계산과 모든 분기는 유지한다.

```cpp
#include "KoukuSaydonBrain.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace
{
	// Server 시계는 초당 30 tick이다. 저작 시간 ms와 비교할 때 정수 곱을 사용하여 stage마다 float 반올림이 끼지 않게 한다.
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr std::uint64_t MILLISECONDS_PER_SECOND = 1000u;

	// 0은 미설정 tick이므로 UINT32_MAX 다음은 1이다. stage 전환 뒤 첫 평가 tick을 예약할 때 사용한다.
	std::uint32_t Next_ServerTick(const std::uint32_t tick) noexcept
	{
		return tick == (std::numeric_limits<std::uint32_t>::max)() ? 1u : tick + 1u;
	}

	// 일반 stage 시계는 첫 평가 tick 자체를 1 tick으로 센다. 시작 tick/입력 tick이 0이면 아직 평가하지 않은 것으로 본다.
	std::uint64_t Stage_ElapsedTicks(
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) noexcept
	{
		const std::uint32_t firstTick = boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		const std::uint64_t age = serverTick >= firstTick
			? static_cast<std::uint64_t>(serverTick - firstTick)
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - firstTick) +
				static_cast<std::uint64_t>(serverTick);
		return age + 1u;
	}

	// 경과 tick과 기간 ms를 64비트 곱으로 비교한다. 30Hz에서 기간에 도달하는 첫 tick부터 true다.
	bool Has_ElapsedMilliseconds(const std::uint64_t elapsedTicks, const std::uint32_t durationMs) noexcept
	{
		return elapsedTicks * MILLISECONDS_PER_SECOND >=
			static_cast<std::uint64_t>(durationMs) * SERVER_TICK_HZ;
	}

	// 지원하는 세 stage 종류를 복제되는 action 상태로 바꾼다. 다른 종류는 IDLE이며 정의 validator가 먼저 지원 종류를 제한한다.
	LostArk::Server::SERVER_ENTITY_ACTION To_ServerAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_KIND kind) noexcept
	{
		using namespace LostArk::Server;
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_KIND::WINDUP:
			return SERVER_ENTITY_ACTION::PATTERN_WINDUP;
		case BOSS_PATTERN_STAGE_KIND::ACTIVE:
			return SERVER_ENTITY_ACTION::PATTERN_ACTIVE;
		case BOSS_PATTERN_STAGE_KIND::RECOVERY:
			return SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
		default:
			return SERVER_ENTITY_ACTION::IDLE;
		}
	}

	// 일반 이동 옵션이 비어 있는 animation root motion만 승인한다. 키가 있다면 2~512개, 시작 0/끝 duration, 시간 엄격 증가를 요구한다.
	// 좌표는 source 상대 변위이며 finite/범위를 확인한다. navigation 경로의 실제 유효성은 여기서 검사하지 않는다.
	bool Is_AnimationRootMotion(
		const LostArk::Server::BOSS_PATTERN_STAGE_MOTION& motion, const std::uint32_t durationMs) noexcept
	{
		using namespace LostArk::Server;
		if (BOSS_PATTERN_STAGE_MOTION_KIND::NONE != motion.eKind || 0u != motion.iRetargetDelayMs ||
			0.f != motion.fSpeedMps || 0.f != motion.fDistance || 0u != motion.iCornerIndex ||
			0.f != motion.fHalfExtentsX || 0.f != motion.fHalfExtentsZ)
			return false;
		if (motion.RootMotion.empty())
			return true;
		const auto& samples = motion.RootMotion;
		if (samples.size() < 2u || samples.size() > 512u || samples.front().iTimeMs != 0u ||
			samples.back().iTimeMs != durationMs || samples.front().fForward != 0.f ||
			samples.front().fLateral != 0.f || samples.front().fUp != 0.f)
			return false;
		for (std::size_t index = 0u; index < samples.size(); ++index)
		{
			const auto& sample = samples[index];
			if (!std::isfinite(sample.fForward) || !std::isfinite(sample.fLateral) ||
				!std::isfinite(sample.fUp) || std::abs(sample.fForward) > 100000.f ||
				std::abs(sample.fLateral) > 100000.f || std::abs(sample.fUp) > 100000.f ||
				(index && sample.iTimeMs <= samples[index - 1u].iTimeMs))
				return false;
		}
		return true;
	}
}

// PartyWorld의 G1 주 보스 조회에 사용한다. owner가 없는 정확한 world/encounter/archetype/placement 조합만 true다.
bool LostArk::Server::CKoukuSaydonBrain::Is_GateOneBoss(
	const LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == boss.strArchetypeId &&
		KOUKUSAYDON_G1_BOSS_PLACEMENT_ID == boss.strPlacementId;
}

// GameRoom이 다룰 쿠크 아레나 주 보스를 분류한다. 호출자가 가진 entity의 필드만 읽으며 alive/실행 소유 여부는 별도다.
bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBoss(
	const LostArk::Shared::WORLD_ID worldId, const SERVER_WORLD_ENTITY& boss) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId &&
		WORLD_BOOTSTRAP_KIND::BOSS == boss.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == boss.iOwnerBossNetEntityId &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == boss.strEncounterId &&
		boss.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

// GateProgress의 활성화 후보를 분류한다. disabled placement를 찾는 조건이며 실제 spawn 성공이나 definition 전체 유효성은 뜻하지 않는다.
bool LostArk::Server::CKoukuSaydonBrain::Is_ArenaBossPlacement(
	const LostArk::Shared::WORLD_ID worldId, const WORLD_BOOTSTRAP_PLACEMENT& placement) noexcept
{
	return LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId && !placement.isEnabled &&
		WORLD_BOOTSTRAP_KIND::BOSS == placement.eKind &&
		KOUKUSAYDON_G1_ENCOUNTER_ID == placement.strEncounterId &&
		placement.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
}

// Catalog 검증과 GameRoom 승인/조회가 공용으로 사용한다. pattern은 읽기 전용이고 첫 실패의 status를 반환한다.
// 이름은 animation-only지만 현재는 LogicWindow/MechanicTrigger의 공격도 검증한다. 아래 제한은 stage 직접 공격과 지원하지 않는 조합에 대한 것이다.
// 외부 참조의 존재, navigation, Catalog 전체 무결성은 호출 계층의 추가 검사에 남는다.
bool LostArk::Server::CKoukuSaydonBrain::Validate_AnimationOnlyPattern(
	const BOSS_PATTERN_DEFINITION& pattern, std::string& status)
{
	if (KOUKUSAYDON_G1_ENCOUNTER_ID != pattern.strEncounterId || pattern.strPatternId.empty() ||
		BOSS_PATTERN_SELECTION::AUDITION_ONLY != pattern.eSelection ||
		BOSS_PATTERN_TARGET_POLICY::NONE != pattern.eTargetPolicy ||
		BOSS_PATTERN_AIM_POLICY::NONE != pattern.eAimPolicy ||
		BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind || !pattern.Motion.strAnchorId.empty() ||
		0.f != pattern.Motion.fLandingX || 0.f != pattern.Motion.fLandingY || 0.f != pattern.Motion.fLandingZ ||
		0.f != pattern.Motion.fApexHeight || pattern.Motion.bMoveToAnchorBeforeTakeoff ||
		BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind || !pattern.Finale.strGhostArchetypeId.empty() ||
		!pattern.Finale.GhostPatternIds.empty() || 0.f != pattern.fVerticalOffsetM ||
		pattern.bInvulnerableWhileRunning || pattern.Stages.empty() ||
		pattern.Stages.size() != pattern.iExpectedStageCount)
	{
		status = "KoukuSaydon pattern is not an animation-only audition definition";
		return false;
	}

	// root motion과 다른 보스 이동 작성 방식이 동시에 좌표를 소유하지 못하도록 충돌하는 조합을 거부한다.
	const bool hasRootMotion = std::any_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) {
		return !stage.Motion.RootMotion.empty();
	});
	if (hasRootMotion &&
		(pattern.BossMotion ||
		 std::any_of(
			 pattern.LogicWindows.begin(),
			 pattern.LogicWindows.end(),
			 [](const auto& window) {
				 return window.fBossChargeDistanceM > 0.f;
			 }) ||
		 std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(), [](const auto& trigger) {
			 return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT;
		 })))
	{
		status = "KoukuSaydon animation root motion conflicts with authored movement";
		return false;
	}
	// stage/action ID의 중복 검사용 지역 set이다. 실행 순서는 pattern.Stages vector가 그대로 보유한다.
	std::unordered_set<std::string> stageIds;
	std::unordered_set<std::string> actionIds;
	for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
	{
		const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern.Stages[index];
		// 현재 stage action은 비어 있거나 지정된 ENTER 대상 재선택 한 개만 허용한다. action을 여기서 실행하지 않는다.
		const bool supportedActions =
			stage.Actions.empty() || (stage.Actions.size() == 1u && !pattern.BossMotion && [&]() {
				const auto& action = stage.Actions.front();
				return action.eTrigger == BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER &&
					action.eKind == BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE &&
					(action.strTargetId == "boss.target.pattern" ||
					 action.strTargetId == "boss.target.nearest") &&
					action.iValue == 1u && action.iDurationMs == 0u &&
					action.eReleaseMode == BOSS_GRABBED_RELEASE_MODE::NONE && action.fReleaseSpeedMps == 0.f &&
					action.fReleaseYawOffsetDegrees == 0.f &&
					action.Volley.ePolicy == BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE;
			}());
		const bool supportedKind = BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::ACTIVE == stage.eStageKind ||
			BOSS_PATTERN_STAGE_KIND::RECOVERY == stage.eStageKind;
		// 분기가 있으면 다음 stage action으로 가는 TIMEOUT 한 개여야 한다. 마지막 stage는 next action이 비어 있어야 한다.
		const bool validTimeoutBranch = stage.Branches.empty() ||
			(1u == stage.Branches.size() &&
			 BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT == stage.Branches.front().eOutcome &&
			 stage.Branches.front().strNextPatternId.empty() &&
			 stage.Branches.front().strNextActionId ==
				 (index + 1u < pattern.Stages.size() ? pattern.Stages[index + 1u].strActionId : std::string{}));
		// 식별자/종류/기간 뒤에 stage 직접 피해·타격·반복·이동·분기 필드의 지원 범위를 차례로 검사한다.
		if (stage.strStageId.empty() || stage.strActionId.empty() ||
			!stageIds.insert(stage.strStageId).second || !actionIds.insert(stage.strActionId).second ||
			!supportedKind || 0u == stage.iDurationMs || !stage.strDamageProfileId.empty() ||
			BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT != stage.eHitAnchorKind ||
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE != stage.eHitActivationKind ||
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE != stage.ePlayerResponse ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE != stage.eAttachmentSlot ||
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL != stage.ePartDamagePolicy || stage.bHasCounterProxy ||
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE != stage.eCounterProxyKind ||
			0.f != stage.fCounterProxyForwardOffsetM || 0.f != stage.fCounterProxyRightOffsetM ||
			0.f != stage.fCounterProxyRadiusM || 0.f != stage.fCounterProxyArcDegrees ||
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE != stage.eBossResponseKind ||
			0u != stage.iBossResponseThreshold || 0.f != stage.fVerticalOffsetM ||
			0.f != stage.fHitOuterRadius || 0.f != stage.fHitInnerRadius || 0.f != stage.fHitAngleDegrees ||
			0.f != stage.fHitLength || 0.f != stage.fHitHalfWidth || 0u != stage.iHitCount ||
			0u != stage.iHitIntervalMs || 0u != stage.iHitDelayMs || !stage.HitOffsetsMs.empty() ||
			0.f != stage.fHitAnchorForwardOffsetM || 0.f != stage.fHitAnchorRightOffsetM ||
			0.f != stage.fHitAnchorYawOffsetDegrees || 0u != stage.iHitActivationStartMs ||
			0u != stage.iHitActivationLifetimeMs || 0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
			stage.bKnockdown || 0u != stage.iDownMs || stage.bWallContact || stage.bChargeImpact ||
			stage.bPiercesCover || !Is_AnimationRootMotion(stage.Motion, stage.iDurationMs) ||
			!stage.strPropBreakSetId.empty() || !stage.PropBreakSlotIds.empty() || !supportedActions ||
			!validTimeoutBranch)
		{
			status = "KoukuSaydon pattern contains unsupported combat, motion, action, or branch data";
			return false;
		}
	}
	// 고정 시계에서는 누적 경계를 30Hz tick으로 올림했을 때 각 stage가 적어도 한 tick 간격을 갖는지 검사한다.
	std::uint64_t patternDurationMs = 0u;
	for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
	{
		if (pattern.bFixedTimelineClock &&
			((patternDurationMs + stage.iDurationMs) * SERVER_TICK_HZ + 999u) / 1000u <=
				(patternDurationMs * SERVER_TICK_HZ + 999u) / 1000u)
		{
			status = "Parent stage boundaries must occupy distinct Server ticks";
			return false;
		}
		patternDurationMs += stage.iDurationMs;
	}
	// 독립 row 수명 검증은 timeline 기간까지 보지만, Brain의 stage 완료 시계는 Stages의 길이 합만 사용한다.
	patternDurationMs = (std::max)(patternDurationMs, std::uint64_t(pattern.iTimelineDurationMs));
	// BossMotion은 stage root와 별도인 절대 월드 위치 경로다. 범위와 키의 시작/종료 일치·시간 증가를 검사한다.
	if (pattern.BossMotion)
	{
		const auto& motion = *pattern.BossMotion;
		bool valid = !pattern.bResetBossToSpawn && !pattern.ResetBossYawDegrees &&
			motion.iStartMs < motion.iEndMs && motion.iEndMs <= patternDurationMs &&
			(!motion.Keys.empty() || motion.StartPosition[1] == motion.EndPosition[1]) &&
			std::isfinite(motion.fYawDegrees) && std::abs(motion.fYawDegrees) <= 360.f;
		for (std::size_t axis = 0u; axis < 3u; ++axis)
			valid = valid && std::isfinite(motion.StartPosition[axis]) &&
				std::isfinite(motion.EndPosition[axis]) && std::abs(motion.StartPosition[axis]) <= 100000.f &&
				std::abs(motion.EndPosition[axis]) <= 100000.f;
		if (!motion.Keys.empty())
		{
			valid = valid && motion.Keys.size() >= 2u && motion.Keys.size() <= 512u &&
				motion.Keys.front().iTimeMs == motion.iStartMs && motion.Keys.back().iTimeMs == motion.iEndMs &&
				motion.Keys.front().Position == motion.StartPosition &&
				motion.Keys.back().Position == motion.EndPosition;
			for (std::size_t i = 0u; i < motion.Keys.size(); ++i)
				valid = valid && (i == 0u || motion.Keys[i - 1u].iTimeMs < motion.Keys[i].iTimeMs) &&
					std::all_of(
							motion.Keys[i].Position.begin(), motion.Keys[i].Position.end(), [](float value) {
								return std::isfinite(value) && std::abs(value) <= 100000.f;
							});
		}
		if (!valid)
		{
			status = "KoukuSaydon Boss Motion interval, keys, base height or reset policy is invalid";
			return false;
		}
	}
	// trigger별 필요한 필드와 다른 종류에 남아 있으면 안 되는 필드를 구분한다. 이 반복은 실제 trigger 소비가 아닌 정의 검증이다.
	std::unordered_set<std::string> triggerIds;
	for (const auto& trigger : pattern.MechanicTriggers)
	{
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
		{
			std::unordered_set<std::string> directions(
				trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end());
			if (pattern.BossMotion || trigger.DirectionPatternIds.size() != 4u || directions.size() != 4u ||
				directions.count("") || directions.count(pattern.strPatternId) ||
				trigger.strCloneEndStageId.empty() ||
				(!trigger.strRealPatternId.empty() && !directions.count(trigger.strRealPatternId)) ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f ||
				trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() ||
				!trigger.ClockHours.empty() || !trigger.PatternSpawns.empty() || trigger.iCountPerPlayer ||
				trigger.iEffectLifetimeMs || trigger.iArenaRandomCount || trigger.bRandomPlayerOnly)
			{
				status = "Cross direction requires four distinct child Patterns and one clone end Stage";
				return false;
			}
		}
		else if (
			!trigger.DirectionPatternIds.empty() || !trigger.strCloneEndStageId.empty() ||
			!trigger.strRealPatternId.empty())
		{
			status = "Only cross direction owns directional Pattern references";
			return false;
		}

		// Albion 공중 phase의 높이 m/기간 ms와 선택 effect 조합을 제한한다. 선행 JUMP와 SLAM의 남은 root 하강 구간도 아래에서 확인한다.
		using Air = ALBION_AIRBORNE_PHASE;
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE)
		{
			const auto phase = trigger.eAirbornePhase;
			const bool jump = phase == Air::JUMP, appear = phase == Air::APPEAR_PLAYER;
			if (phase == Air::NONE || phase > Air::SLAM || pattern.BossMotion ||
				!std::isfinite(trigger.fAirborneHeightM) || trigger.fAirborneHeightM < 0.f ||
				trigger.fAirborneHeightM > 100000.f || ((jump || appear) != (trigger.fAirborneHeightM > 0.f)) ||
				(trigger.bCaptureAirborneTargetPosition && phase != Air::SELECT_PLAYER) ||
				trigger.strSelectedEffectVisualId.empty() != (trigger.iSelectedEffectLifetimeMs == 0u) ||
				(!trigger.strSelectedEffectVisualId.empty() && !trigger.bCaptureAirborneTargetPosition) ||
				std::uint64_t(trigger.iStartMs) + trigger.iSelectedEffectLifetimeMs > patternDurationMs ||
				(jump ? (trigger.iAirborneDurationMs > 600000u ||
						 std::uint64_t(trigger.iStartMs) + trigger.iAirborneDurationMs > patternDurationMs)
					  : trigger.iAirborneDurationMs != 0u) ||
				trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE ||
				!trigger.strClonePatternId.empty() || !trigger.ClockHours.empty() ||
				trigger.fFaceCenterYawOffsetDegrees != 0.f || trigger.iCountPerPlayer ||
				trigger.fPlayerEffectRadiusM != 0.f || trigger.iEffectLifetimeMs || trigger.iArenaRandomCount ||
				trigger.bRandomPlayerOnly || !trigger.PatternSpawns.empty() ||
				!std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) ||
				!std::isfinite(trigger.fTeleportZ) || std::abs(trigger.fTeleportX) > 100000.f ||
				std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f ||
				(phase != Air::CENTER &&
				 (trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f)))
			{
				status = "Albion airborne phase has invalid or unrelated values";
				return false;
			}
			if (phase != Air::JUMP && phase != Air::SELECT_PLAYER &&
				!std::any_of(
					pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(), [&](const auto& row) {
						return row.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
							row.eAirbornePhase == Air::JUMP && row.iStartMs <= trigger.iStartMs;
					}))
			{
				status = "Albion airborne phase requires its initial jump";
				return false;
			}
			if (phase == Air::SLAM)
			{
				std::uint64_t start = 0u;
				const BOSS_PATTERN_STAGE_DEFINITION* source = nullptr;
				for (const auto& stage : pattern.Stages)
				{
					if (trigger.iStartMs < start + stage.iDurationMs)
					{
						source = &stage;
						break;
					}
					start += stage.iDurationMs;
				}
				if (!source || source->Motion.RootMotion.empty())
				{
					status = "Albion slam requires source root motion";
					return false;
				}
				const double age = double(trigger.iStartMs - start);
				const float up = Sample_StageRootMotion(source->Motion.RootMotion, age).fUp;
				float minimum = up;
				for (const auto& sample : source->Motion.RootMotion)
					if (sample.iTimeMs >= age)
						minimum = (std::min)(minimum, sample.fUp);
				if (up - minimum <= .000001f)
				{
					status = "Albion slam has no remaining source descent";
					return false;
				}
			}
		}
		else if (
			trigger.eAirbornePhase != Air::NONE || trigger.fAirborneHeightM != 0.f ||
			trigger.iAirborneDurationMs != 0u || trigger.bCaptureAirborneTargetPosition ||
			!trigger.strSelectedEffectVisualId.empty() || trigger.iSelectedEffectLifetimeMs != 0u)
		{
			status = "Non-Albion trigger carries airborne values";
			return false;
		}
		// Showtime 폭탄은 마지막 회전 추적과 BOSS_CURRENT 부채꼴을 함께 사용한다. 고정/폭발 표시 ID와 수명도 검사한다.
		if (trigger.ShowtimeBomb)
		{
			const auto& bomb = *trigger.ShowtimeBomb;
			const auto& fan = bomb.FanRegion;
			if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
				trigger.fFollowSpeedScale != 0.f || trigger.iDurationMs <= 34u ||
				bomb.iBodyStartMs >= std::uint64_t(trigger.iStartMs) + trigger.iDurationMs ||
				bomb.strBodyVisualId.empty() || bomb.strExplosionVisualId.empty() ||
				bomb.strBodyVisualId == bomb.strExplosionVisualId || !bomb.iExplosionLifetimeMs ||
				bomb.iExplosionLifetimeMs > 600000u ||
				std::any_of(
					bomb.BodyPosition.begin(),
					bomb.BodyPosition.end(),
					[](float value) {
						return !std::isfinite(value) || std::abs(value) > 100000.f;
					}) ||
				fan.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || !fan.bSector || fan.bReverseSector ||
				fan.bCircle || fan.bCylinder || fan.bLinearMotion || fan.WorldTrack.bEnabled ||
				fan.fInnerRadiusM != 0.f || !std::isfinite(fan.fCenterX) || std::abs(fan.fCenterX) > 1000.f ||
				!std::isfinite(fan.fCenterY) || std::abs(fan.fCenterY) > 1000.f ||
				!std::isfinite(fan.fCenterZ) || std::abs(fan.fCenterZ) > 1000.f ||
				!std::isfinite(fan.fYawDegrees) || std::abs(fan.fYawDegrees) > 360.f ||
				!std::isfinite(fan.fRadiusM) || fan.fRadiusM <= 0.f || !std::isfinite(fan.fRadiusXM) ||
				fan.fRadiusXM <= 0.f || fan.fRadiusXM > 1000.f || !std::isfinite(fan.fRadiusZM) ||
				fan.fRadiusZM <= 0.f || fan.fRadiusZM > 1000.f || !std::isfinite(fan.fHalfAngleDegrees) ||
				fan.fHalfAngleDegrees <= 0.f || fan.fHalfAngleDegrees >= 180.f)
			{
				status = "Showtime bomb requires a fixed MAP visual and the final rotate-only boss fan";
				return false;
			}
		}
		// 공격 template은 사용할 trigger 종류와 visual 소유자가 맞아야 한다. 종류별 실제 수명 ms를 Shared validator에 넘긴다.
		// FixedHits의 수명은 Albion effect, 선택 대상 effect, 일반 fixed visual 순서로 선택한다.
		const bool hitShowtime = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
		const bool hitPursuit = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
		const bool hitAlbion = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
		const bool hitSelected = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
			trigger.eAirbornePhase == Air::SELECT_PLAYER && trigger.bCaptureAirborneTargetPosition &&
			!trigger.strSelectedEffectVisualId.empty();
		if ((!hitShowtime && !trigger.TrackingHits.empty()) ||
			(!hitShowtime && !hitAlbion && !hitSelected && !trigger.FixedHits.empty()) ||
			(!hitPursuit && !trigger.ProjectileHits.empty()) ||
			(!hitAlbion && !hitSelected && !trigger.FixedHits.empty() && trigger.strFixedVisualId.empty()) ||
			(!trigger.TrackingHits.empty() && trigger.strTrackingVisualId.empty()) ||
			!LostArk::Shared::Validate_AttackHitTemplates(
				trigger.FixedHits,
				hitAlbion		  ? trigger.iEffectLifetimeMs
					: hitSelected ? trigger.iSelectedEffectLifetimeMs
								  : trigger.iFixedLifetimeMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(trigger.TrackingHits, trigger.iDurationMs) ||
			!LostArk::Shared::Validate_AttackHitTemplates(
				trigger.ProjectileHits,
				trigger.iProjectileLifetimeMs ? trigger.iProjectileLifetimeMs : 600000u) ||
			std::any_of(trigger.RandomVolleys.begin(), trigger.RandomVolleys.end(), [](const auto& volley) {
				return !LostArk::Shared::Validate_AttackHitTemplates(volley.Hits, volley.iLifetimeMs);
			}))
		{
			status = "Dynamic attack templates have invalid owners, shapes or hit windows";
			return false;
		}
		// 추적 발사체의 표시 목록/카드 수와 이동·반지름·수명을 제한한다. 다른 trigger에는 발사체 전용 필드가 남지 않아야 한다.
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES)
		{
			const auto count = trigger.ProjectileVisualIds.size();
			if (!count || count > 4u || trigger.strContactVisualId.empty() ||
				(!trigger.ProjectileCardSymbols.empty() &&
				 (trigger.ProjectileCardSymbols.size() != count ||
				  std::any_of(
					  trigger.ProjectileCardSymbols.begin(),
					  trigger.ProjectileCardSymbols.end(),
					  [](const auto symbol) {
						  return symbol <= LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE ||
							  symbol >= LostArk::Shared::MECHANIC_CARD_SYMBOL::END;
					  }))) ||
				std::any_of(
					trigger.ProjectileVisualIds.begin(),
					trigger.ProjectileVisualIds.end(),
					[](const auto& id) {
						return id.empty();
					}) ||
				!std::isfinite(trigger.fProjectileSpeedMps) || trigger.fProjectileSpeedMps < .01f ||
				trigger.fProjectileSpeedMps > 100.f || !std::isfinite(trigger.fProjectileContactRadiusM) ||
				trigger.fProjectileContactRadiusM < .01f || trigger.fProjectileContactRadiusM > 10.f ||
				!std::isfinite(trigger.fProjectileSpawnRadiusM) || trigger.fProjectileSpawnRadiusM < 0.f ||
				trigger.fProjectileSpawnRadiusM > 100.f || !std::isfinite(trigger.fProjectileMaxDistanceM) ||
				trigger.fProjectileMaxDistanceM < 0.f || trigger.fProjectileMaxDistanceM > 1000.f ||
				trigger.iProjectileLifetimeMs > 600000u || trigger.iSpawnIntervalMs > 600000u ||
				!trigger.iProjectileCountPerWave || trigger.iProjectileCountPerWave > 16u ||
				(trigger.iProjectileLifetimeMs == 0u &&
				 (!trigger.bProjectileHoming || trigger.iSpawnIntervalMs != 0u ||
				  trigger.fProjectileMaxDistanceM != 0.f)))
			{
				status = "Pursuit projectile visual, motion or lifetime contract is invalid";
				return false;
			}
		}
		else if (
			!trigger.ProjectileVisualIds.empty() || !trigger.ProjectileCardSymbols.empty() ||
			!trigger.strContactVisualId.empty() || trigger.fProjectileSpeedMps != 0.f ||
			trigger.fProjectileMaxDistanceM != 0.f || trigger.fProjectileContactRadiusM != 0.f ||
			trigger.fProjectileSpawnRadiusM != 0.f || trigger.iProjectileLifetimeMs != 0u ||
			trigger.iProjectileCountPerWave != 0u || trigger.bProjectileHoming)
		{
			status = "Only pursuit projectiles own projectile values";
			return false;
		}
		// Showtime random volley가 있으면 anchor/크기/간격/수명과 각 visual을 검사한다. 없을 때는 관련 필드도 기본값이어야 한다.
		const bool hasRandomVolleys = !trigger.RandomVolleys.empty();
		if (hasRandomVolleys
				? (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
				   trigger.RandomVolleys.size() > 32u ||
				   (trigger.strRandomAnchorKind != "BOSS_SPAWN" && trigger.strRandomAnchorKind != "BOSS") ||
				   !std::isfinite(trigger.fRandomScaleMin) || !std::isfinite(trigger.fRandomScaleMax) ||
				   trigger.fRandomScaleMin < .01f || trigger.fRandomScaleMax > 10.f ||
				   trigger.fRandomScaleMax < trigger.fRandomScaleMin || trigger.iRandomSpawnIntervalMs == 0u ||
				   trigger.iRandomSpawnIntervalMs > 600000u || !std::isfinite(trigger.fRandomArenaRadiusM) ||
				   trigger.fRandomArenaRadiusM <= 0.f || trigger.fRandomArenaRadiusM > 1000.f ||
				   !std::isfinite(trigger.fRandomArenaHeightToleranceM) ||
				   trigger.fRandomArenaHeightToleranceM <= 0.f || trigger.fRandomArenaHeightToleranceM > 10.f ||
				   std::any_of(
					   trigger.RandomVolleys.begin(),
					   trigger.RandomVolleys.end(),
					   [](const auto& volley) {
						   return volley.strClientVisualId.empty() || volley.iLifetimeMs == 0u ||
							   volley.iLifetimeMs > 600000u;
					   }))
				: (trigger.iRandomSpawnIntervalMs != 0u || trigger.fRandomArenaRadiusM != 0.f ||
				   trigger.fRandomArenaHeightToleranceM != 0.f || trigger.strRandomAnchorKind != "BOSS_SPAWN" ||
				   trigger.fRandomScaleMin != 1.f || trigger.fRandomScaleMax != 1.f))
		{
			status = "Showtime random volley pool, cadence or arena contract is invalid";
			return false;
		}
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS &&
			((trigger.strFixedVisualId == trigger.strTrackingVisualId &&
			  (!hasRandomVolleys || !trigger.strFixedVisualId.empty())) ||
			 trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) ||
			 trigger.iFixedLifetimeMs > 600000u || trigger.iSpawnIntervalMs == 0u ||
			 trigger.iSpawnIntervalMs > 600000u || !std::isfinite(trigger.fFollowSpeedScale) ||
			 trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
		{
			status = "Showtime player-target visual or timing contract is invalid";
			return false;
		}
		if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
			 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET ||
			 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION) &&
			(trigger.iDurationMs > 600000u || !trigger.strFixedVisualId.empty() ||
			 !trigger.strTrackingVisualId.empty() || trigger.iFixedLifetimeMs != 0u ||
			 trigger.iSpawnIntervalMs != 0u ||
			 (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
			  trigger.fFollowSpeedScale != 0.f) ||
			 // scale 0은 회전만 허용하고, 양수 scale은 추적 대상 player 속도에 대한 이동 비율로 사용한다.
			 !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < 0.f ||
			 (trigger.fFollowSpeedScale != 0.f &&
			  (trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f)) ||
			 trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f ||
			 trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() ||
			 !trigger.ClockHours.empty() || trigger.fFaceCenterYawOffsetDegrees != 0.f ||
			 trigger.iCountPerPlayer != 0u || trigger.fPlayerEffectRadiusM != 0.f ||
			 trigger.iEffectLifetimeMs != 0u || trigger.iArenaRandomCount != 0u || trigger.bRandomPlayerOnly ||
			 !trigger.PatternSpawns.empty()))
		{
			status = "Boss tracking duration must not create visuals or carry another mechanic's values";
			return false;
		}
		// 소환 요청은 spawn ID 중복, 자기 패턴 참조, anchor별 좌표 범위부터 검사한다. 실제 child 존재와 owner 조합 검증은 Catalog/GameRoom이 이어서 수행한다.
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
		{
			std::unordered_set<std::string> spawnIds;
			if (trigger.PatternSpawns.empty() || trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
			{
				status = "Summon Pattern trigger requires one to sixteen spawns";
				return false;
			}
			for (const auto& spawn : trigger.PatternSpawns)
			{
				if (spawn.strSpawnId.empty() || !spawnIds.insert(spawn.strSpawnId).second ||
					spawn.strPatternId.empty() || spawn.strPatternId == pattern.strPatternId ||
					!std::isfinite(spawn.fYawOffsetDegrees) || std::abs(spawn.fYawOffsetDegrees) > 360.f ||
					(spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::BOSS &&
					 spawn.eAnchorKind != BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP) ||
					std::any_of(
						spawn.PositionOffset.begin(), spawn.PositionOffset.end(), [&](const float value) {
							return !std::isfinite(value) ||
								std::abs(value) >
								(spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP ? 100000.f : 1000.f);
						}))
				{
					status = "Summon Pattern spawn identity or transform is invalid";
					return false;
				}
			}
		}
		if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS) !=
				!trigger.PlayerEntryPositions.empty() ||
			trigger.PlayerEntryPositions.size() > 4u ||
			std::any_of(
				trigger.PlayerEntryPositions.begin(),
				trigger.PlayerEntryPositions.end(),
				[](const auto& position) {
					return std::any_of(position.begin(), position.end(), [](const float value) {
						return !std::isfinite(value) || std::abs(value) > 100000.f;
					});
				}))
		{
			status = "Card maze staging requires one to four finite destinations on its own trigger";
			return false;
		}
		// 모든 trigger의 ID 중복과 시작/종료 범위를 검사한다. teleport는 BossMotion과 병행할 수 없다.
		if (trigger.strTriggerId.empty() || !triggerIds.insert(trigger.strTriggerId).second ||
			trigger.iStartMs >= patternDurationMs || 0u == trigger.iDurationMs ||
			static_cast<std::uint64_t>(trigger.iStartMs) + trigger.iDurationMs > patternDurationMs ||
			((BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER == trigger.eKind ||
			  BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS == trigger.eKind) &&
			 (pattern.BossMotion || !std::isfinite(trigger.fTeleportX) || !std::isfinite(trigger.fTeleportY) ||
			  !std::isfinite(trigger.fTeleportZ) || std::abs(trigger.fTeleportX) > 100000.f ||
			  std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f)) ||
			(BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT == trigger.eKind &&
			 (pattern.BossMotion || trigger.strClonePatternId.empty() || trigger.ClockHours.size() != 3u)))
		{
			status = "KoukuSaydon mechanic trigger is invalid";
			return false;
		}
	}
	// LogicWindow의 kind별 입력/결과 허용 범위와 판정 시간을 검사한다. 실행 시 플레이어 판정 기록은 LogicRuntime이 소유한다.
	std::unordered_set<std::string> windowIds;
	for (const BOSS_PATTERN_LOGIC_WINDOW& window : pattern.LogicWindows)
	{
		const std::uint64_t endMs = static_cast<std::uint64_t>(window.iStartMs) + window.iDurationMs;
		const bool endTickKind = BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS == window.eKind;
		if (window.bGazeDuringWindow &&
			(window.eKind != BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS || !window.bInsideIsFail ||
			 !window.OnSuccess.empty() || !window.OnTimeout.empty()))
		{
			status = "Gaze during window requires facing FAIL and only Fail outcomes";
			return false;
		}
		bool valuesValid = true;
		if (((window.bRearmOnExit || window.bRepeatAfterKnockback) &&
			 window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA) ||
			(window.iRepeatIntervalMs && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
			 window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) ||
			(window.bRearmOnExit && window.bRepeatAfterKnockback) || window.iRepeatIntervalMs > 600000u ||
			(window.iRepeatIntervalMs && (window.bRearmOnExit || window.bRepeatAfterKnockback)))
		{
			status = "ENTER_AREA accepts one contact repeat policy";
			return false;
		}
		if (window.bRepeatAfterKnockback &&
			(window.OnSuccess.size() != 1u ||
			 window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			 window.OnSuccess.front().fPushRangeM <= 0.f || window.OnSuccess.front().iPushMs == 0u))
		{
			status = "Repeat after knockback requires one damage Success with positive knockback";
			return false;
		}
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT:
			valuesValid = !window.PatternIds.empty() && window.PatternIds.size() <= 16u &&
				window.iCompletionCount > 0u && window.iCompletionCount <= window.PatternIds.size() &&
				window.CardRegions.empty() && window.OnFail.empty() && window.OnTimeout.empty() &&
				window.OnSuccess.size() <= 1u &&
				(window.OnSuccess.empty() ||
				 window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN);
			break;
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
			valuesValid = window.CardRegions.size() == 8u ||
				(window.iSectorCount >= 2u && window.SectorSymbols.size() == window.iSectorCount &&
				 window.fOuterRadiusM > 0.f);
			break;
		case BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE:
			valuesValid =
				window.iThreshold <= 4u && !window.CardRegions.empty() && window.OnSuccess.empty() &&
				window.OnFail.empty() && window.OnTimeout.empty() && !window.bEndsPatternOnSuccess &&
				std::all_of(window.CardRegions.begin(), window.CardRegions.end(), [](const auto& region) {
					return region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && !region.WorldTrack.bEnabled;
				});
			break;
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
			valuesValid = !window.CardRegions.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
			valuesValid = !window.CardRegions.empty() && !window.strTargetWorldInstanceId.empty() &&
				std::isfinite(window.fTargetWorldX) && std::isfinite(window.fTargetWorldZ) &&
				std::abs(window.fTargetWorldX) <= 100000.f && std::abs(window.fTargetWorldZ) <= 100000.f &&
				window.fTargetRadiusM >= .01f && window.fTargetRadiusM <= 1000.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			valuesValid = !window.CardRegions.empty() && !window.ContactTargets.empty() &&
				window.ContactTargets.size() <= 64u && !window.OnSuccess.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
			break;
		case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
		case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
			valuesValid = window.CardRegions.empty() && window.OnSuccess.empty() && window.OnFail.empty() &&
				window.OnTimeout.empty() && !window.bEndsPatternOnSuccess;
			break;
		case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			valuesValid = window.CardRegions.empty() && window.OnFail.empty();
			break;
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
			valuesValid = window.fHalfAngleDegrees > 0.f;
			break;
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
			valuesValid = window.iPoseIndex < 4u;
			break;
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
			valuesValid = window.iThreshold > 0u;
			break;
		case BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES:
			valuesValid = window.iThreshold >= 1u && window.iThreshold <= 10u && window.CardRegions.empty() &&
				!window.bInsideIsFail && !window.bEndsPatternOnSuccess && window.OnTimeout.empty();
			break;
		default:
			valuesValid = false;
			break;
		}
		// ENTER_AREA 돌진은 BossMotion과 겹치지 않아야 한다. 서로 겹친 돌진창과 그 기간의 ENTER action도 배제한다.
		if (!std::isfinite(window.fBossChargeDistanceM) || window.fBossChargeDistanceM < 0.f ||
			window.fBossChargeDistanceM > 1000.f || !std::isfinite(window.fChargeYawOffsetDegrees) ||
			std::abs(window.fChargeYawOffsetDegrees) > 360.f ||
			(window.fChargeYawOffsetDegrees != 0.f && window.fBossChargeDistanceM <= 0.f) ||
			(window.fBossChargeDistanceM > 0.f &&
			 (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || pattern.BossMotion)))
			valuesValid = false;
		if (window.fBossChargeDistanceM > 0.f)
		{
			for (const auto& other : pattern.LogicWindows)
				if (&other != &window && other.fBossChargeDistanceM > 0.f && other.iStartMs < endMs &&
					window.iStartMs < std::uint64_t(other.iStartMs) + other.iDurationMs)
					valuesValid = false;
			std::uint64_t stageStartMs = 0u;
			for (const auto& stage : pattern.Stages)
			{
				if (!stage.Actions.empty() && window.iStartMs <= stageStartMs && stageStartMs < endMs)
					valuesValid = false;
				stageStartMs += stage.iDurationMs;
			}
		}
		// 플레이어 capture는 sole success 결과여야 한다. 이를 유지할 ATTACHMENT_HOLD 창이 실제 시간 구간을 덮는지 대조한다.
		const auto captureCount =
			std::count_if(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
				return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
			});
		if (captureCount > 1 || (captureCount == 1 && window.OnSuccess.size() != 1u) ||
			(captureCount == 1) != !window.strHoldLogicOccurrenceId.empty())
			valuesValid = false;
		if (!window.strHoldLogicOccurrenceId.empty())
		{
			const auto hold =
				std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
					return row.strWindowId == window.strHoldLogicOccurrenceId;
				});
			if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || hold == pattern.LogicWindows.end() ||
				hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD || hold->iStartMs > window.iStartMs ||
				std::uint64_t(hold->iStartMs) + hold->iDurationMs < endMs)
				valuesValid = false;
		}
		// success/fail/timeout 결과 배열을 같은 규칙으로 검사한다. 값 단위는 피해 percent/양, 이동 m, 지속 ms이고 종류별 조합 제한이 다르다.
		const auto resultsValid = [&window](const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results) {
			if (results.size() > 4u)
				return false;
			for (const BOSS_PATTERN_LOGIC_RESULT& result : results)
			{
				if (result.iPercent > 100u ||
					(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
					 result.iPercent == 0u))
					return false;
				if (!std::isfinite(result.fPushRangeM) || result.fPushRangeM < 0.f ||
					result.fPushRangeM > (result.bPushBallistic ? 100.f : 20.f) ||
					(result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_BOSS &&
					 result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD &&
					 result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD &&
					 result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					result.iPushMs > (result.bPushBallistic ? 5000u : 600000u) ||
					(result.bPushBallistic && result.iPushMs < 100u) || !std::isfinite(result.fPushHeightM) ||
					result.fPushHeightM < 0.f || result.fPushHeightM > 100.f ||
					(result.fPushHeightM > 0.f && !result.bPushBallistic) ||
					(result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT &&
					 ((result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f) ||
					  &results == &window.OnTimeout || window.CardRegions.empty() ||
					  (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
					   window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP))) ||
					((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) != (result.iPushMs > 0u)) ||
					((result.fPushRangeM > 0.f || result.fPushHeightM > 0.f) &&
					 (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
					  result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE)) ||
					((result.bForcePush || result.bPushCanLeaveArena) && result.fPushRangeM <= 0.f &&
					 result.fPushHeightM <= 0.f) ||
					!std::isfinite(result.fPushYawOffsetDegrees) ||
					std::abs(result.fPushYawOffsetDegrees) > 360.f ||
					(result.fPushYawOffsetDegrees != 0.f &&
					 (result.ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD ||
					  (result.fPushRangeM <= 0.f && result.fPushHeightM <= 0.f))))
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE
						? (result.iDamageAmount == 0u || result.iDamageAmount > 1000000000u ||
						   result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())
						: result.iDamageAmount != 0u)
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY &&
					(!result.iDurationMs || result.iDurationMs > 600000u || result.iPercent ||
					 !result.strPatternId.empty()))
					return false;
				const bool worldMotion =
					BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind;
				if ((worldMotion &&
					 (result.strTargetWorldInstanceId.empty() || result.strMotionInstanceId.empty() ||
					  result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())) ||
					(!worldMotion &&
					 (!result.strTargetWorldInstanceId.empty() || !result.strMotionInstanceId.empty())))
					return false;
				const bool contactMotion =
					result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
				const bool contactSignal =
					result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
				if (((contactMotion || contactSignal) &&
					 window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT) ||
					(window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && !contactMotion &&
					 !contactSignal) ||
					(contactMotion && result.ContactMotions.size() != window.ContactTargets.size()) ||
					(contactSignal && result.strTargetLogicOccurrenceId.empty()))
					return false;
				const bool capture = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
				if (capture
						? (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA ||
						   &results != &window.OnSuccess ||
						   result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
						   result.iPercent || result.iDurationMs ||
						   std::any_of(
							   result.GripLocalOffset.begin(),
							   result.GripLocalOffset.end(),
							   [](const float value) {
								   return !std::isfinite(value) || std::abs(value) > 10.f;
							   }))
						: (result.eAttachmentSlot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ||
						   result.GripLocalOffset != std::array<float, 3u>{}))
					return false;
				if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR &&
					(result.iDurationMs == 0u || result.iDurationMs > 600000u || result.iPercent != 0u ||
					 result.strFearPresentationId.empty()))
					return false;
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::NONE == result.eKind)
					return false;
				// 보스 단위 완료 결과만 후속 pattern을 넘긴다. 플레이어별 판정창은 여기서 보스 패턴 전환을 요청할 수 없다.
				if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind &&
					BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL != window.eKind &&
					BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT != window.eKind)
					return false;
			}
			return true;
		};
		// grip 위치를 담은 track은 단일 물체 잡기 결과를 가진 WORLD ENTER_AREA 창의 제약을 추가로 지켜야 한다.
		for (const auto& region : window.CardRegions)
		{
			const auto& track = region.WorldTrack;
			const bool hasGrip = std::any_of(track.Keys.begin(), track.Keys.end(), [](const auto& key) {
				return key.bHasGripPosition;
			});
			if (!hasGrip)
				continue;
			if (!track.bEnabled || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::WORLD ||
				window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || window.OnSuccess.size() != 1u ||
				window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT ||
				!window.OnFail.empty() || !window.OnTimeout.empty() || region.bCircle || region.bSector ||
				track.iStartMs != window.iStartMs || track.iDurationMs < window.iDurationMs ||
				track.iStartDelayMs || track.fPlaybackSpeed != 1.f || track.bSmoothStep ||
				track.fBaselineX != 0.f || track.fBaselineY != 0.f || track.fBaselineZ != 0.f ||
				track.fBaselineYawDegrees != 0.f || track.fBaselineScaleX != 1.f ||
				track.fBaselineScaleY != 1.f || track.fBaselineScaleZ != 1.f ||
				track.Keys.front().iTimeMs != 0u || track.Keys.back().iTimeMs != track.iDurationMs ||
				std::any_of(
					track.Keys.begin(),
					track.Keys.end(),
					[](const auto& key) {
						return !key.bHasGripPosition ||
							std::any_of(key.GripPosition.begin(), key.GripPosition.end(), [](float x) {
								return !std::isfinite(x) || std::abs(x) > 100000.f;
							});
					}))
			{
				status =
					"Physical hook grip requires a complete identity-baseline ENTER_AREA BOX track and sole world-hook result.";
				return false;
			}
		}
		// 앞선 종류별 검사 뒤 공통 window ID/기간/범위 및 세 결과 배열을 최종 확인한다.
		if (window.strWindowId.empty() || !windowIds.insert(window.strWindowId).second ||
			0u == window.iDurationMs || endMs > patternDurationMs || !valuesValid ||
			(window.bInsideIsFail && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
			 BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
			 BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS != window.eKind) ||
			std::any_of(
				window.CardRegions.begin(),
				window.CardRegions.end(),
				[&](const BOSS_LOGIC_REGION& region) {
					return region.WorldTrack.bEnabled &&
						(region.WorldTrack.Keys.empty() ||
						 (BOSS_PATTERN_LOGIC_KIND::ENTER_AREA != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window.eKind &&
						  BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT != window.eKind) ||
						 region.WorldTrack.iStartMs > window.iStartMs);
				}) ||
			!resultsValid(window.OnSuccess) || !resultsValid(window.OnFail) ||
			!resultsValid(window.OnTimeout) || (endTickKind && !window.OnTimeout.empty()) ||
			(BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind && !window.OnFail.empty()))
		{
			status =
				"KoukuSaydon pattern logic window is out of the pattern lifetime or carries invalid values: " +
				pattern.strPatternId + "/" + window.strWindowId;
			return false;
		}
	}
	// World cue는 combat body, placement override 등 선택한 기능의 필수 ID/수치와 금지 조합을 확인한다. 실제 world instance 조회는 별도다.
	for (const BOSS_PATTERN_WORLD_SEQUENCE& sequence : pattern.WorldSequences)
	{
		if (sequence.CombatBody)
		{
			const auto& body = *sequence.CombatBody;
			if (sequence.strOccurrenceId.empty() || sequence.bAnchorBossSpawn ||
				!sequence.SupportWindows.empty() || !body.iMaximumHp || body.iMaximumHp > 1000000000u ||
				!std::isfinite(body.fRadiusM) || body.fRadiusM <= .001f || body.fRadiusM > 1000.f ||
				!std::isfinite(body.fCenterX) || !std::isfinite(body.fCenterY) ||
				!std::isfinite(body.fCenterZ) || std::abs(body.fCenterX) > 100000.f ||
				std::abs(body.fCenterY) > 100000.f || std::abs(body.fCenterZ) > 100000.f)
			{
				status = "World combat body is outside the fixed owned cue contract";
				return false;
			}
		}
		if (sequence.Placement)
		{
			const auto& placement = *sequence.Placement;
			const float values[] = {
				placement.fPositionX,
				placement.fPositionY,
				placement.fPositionZ,
				placement.fRotationXDegrees,
				placement.fRotationYDegrees,
				placement.fRotationZDegrees,
				placement.fScaleX,
				placement.fScaleY,
				placement.fScaleZ};
			bool valid = !sequence.strOccurrenceId.empty() && sequence.SupportWindows.empty() &&
				!sequence.bAnchorBossSpawn && sequence.fPositionOffsetX == 0.f &&
				sequence.fPositionOffsetY == 0.f && sequence.fPositionOffsetZ == 0.f &&
				sequence.fAnchorPositionX == 0.f && sequence.fAnchorPositionY == 0.f &&
				sequence.fAnchorPositionZ == 0.f;
			for (std::size_t index = 0u; index < 9u; ++index)
				valid = valid && std::isfinite(values[index]) &&
					(index < 3u		  ? std::abs(values[index]) <= 100000.f
						 : index < 6u ? std::abs(values[index]) <= 36000.f
									  : values[index] >= .001f && values[index] <= 1000.f);
			if (!valid)
			{
				status = "KoukuSaydon absolute World placement or occurrence ownership is invalid";
				return false;
			}
		}
		if (sequence.strInstanceId.empty() || sequence.iStartMs > patternDurationMs ||
			sequence.iDurationMs > 600000u ||
			!(sequence.fPlaybackSpeed >= 0.05f && sequence.fPlaybackSpeed <= 16.f))
		{
			status = "KoukuSaydon pattern world sequence cue is invalid";
			return false;
		}
	}
	// scene profile cue는 비어 있지 않은 ID와 pattern timeline 안의 시작 시각을 요구한다. profile 존재를 여기서 조회하지 않는다.
	for (const BOSS_PATTERN_SCENE_PROFILE& profile : pattern.SceneProfiles)
	{
		if (profile.strProfileId.empty() || profile.iStartMs > patternDurationMs)
		{
			status = "KoukuSaydon pattern scene profile cue is invalid";
			return false;
		}
	}
	status.clear();
	return true;
}

// Catalog의 소환 참조 검사와 방향 분신 선택에서 사용한다. child 기본 계약을 확인한 뒤 owner와 함께 실행 가능한 제한을 적용한다.
// allowActorLocalAirborne은 지정된 JUMP/SLAM만 추가 허용하며 실제 actor 생성/수명 관리는 이 함수 밖에 있다.
bool LostArk::Server::CKoukuSaydonBrain::Validate_SummonedPattern(
	const BOSS_PATTERN_DEFINITION& owner,
	const BOSS_PATTERN_DEFINITION& child,
	std::string& status,
	const bool allowActorLocalAirborne)
{
	if (!Validate_AnimationOnlyPattern(child, status))
		return false;
	if (owner.strPatternId == child.strPatternId || owner.strEncounterId != child.strEncounterId ||
		owner.strGateId.empty() || owner.strGateId != child.strGateId ||
		child.eCategory != BOSS_PATTERN_CATEGORY::MECHANIC ||
		owner.strTargetBossPlacementId != child.strTargetBossPlacementId ||
		owner.AuditionBossArchetypeIds.empty() ||
		owner.AuditionBossArchetypeIds.size() != child.AuditionBossArchetypeIds.size() ||
		!std::all_of(
			owner.AuditionBossArchetypeIds.begin(),
			owner.AuditionBossArchetypeIds.end(),
			[&](const auto& archetype) {
				return std::find(
						   child.AuditionBossArchetypeIds.begin(),
						   child.AuditionBossArchetypeIds.end(),
						   archetype) != child.AuditionBossArchetypeIds.end();
			}) ||
		child.BossMotion || child.bResetBossToSpawn || child.ResetBossYawDegrees ||
		std::any_of(
			child.LogicWindows.begin(),
			child.LogicWindows.end(),
			[](const auto& window) {
				if (window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP &&
					window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
					return true;
				const auto unsupported = [](const auto& result) {
					return result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE &&
						result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE &&
						result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
				};
				return std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), unsupported) ||
					std::any_of(window.OnFail.begin(), window.OnFail.end(), unsupported) ||
					std::any_of(window.OnTimeout.begin(), window.OnTimeout.end(), unsupported);
			}) ||
		std::any_of(
			child.MechanicTriggers.begin(),
			child.MechanicTriggers.end(),
			[&](const auto& trigger) {
				if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
					trigger.iDurationMs > 0u && trigger.iDurationMs <= 34u && trigger.fFollowSpeedScale == 0.f)
					return false;
				return !allowActorLocalAirborne ||
					trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
					(trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::JUMP &&
					 trigger.eAirbornePhase != ALBION_AIRBORNE_PHASE::SLAM) ||
					trigger.bCaptureAirborneTargetPosition || !trigger.strSelectedEffectVisualId.empty();
			}) ||
		!child.WorldSequences.empty() || !child.SceneProfiles.empty() ||
		std::any_of(child.Stages.begin(), child.Stages.end(), [](const auto& stage) {
			return !stage.Actions.empty();
		}))
	{
		status =
			"Summoned Pattern requires another same-body, same-Gate leaf with only supported actor-local motion and no world, player or reset actions: " +
			child.strPatternId;
		return false;
	}
	status.clear();
	return true;
}

// Catalog의 참조 검사와 BossSimulation의 분신 준비가 호출한다. 네 child의 정의/종료 stage를 모두 확인한 뒤에만 출력을 교체한다.
// 평가 중인 Catalog 정의는 차용하며, 실패는 status만 바꾸고 selected/durations는 유지한다.
// 기분나빠 패턴 기준으로 4방향 이동에 대한 위치를 먼저 계산한 이후에, 중앙과 가까운 위치를 진짜 세이튼으로 설정을 한다. 
bool LostArk::Server::CKoukuSaydonBrain::Select_CrossDirection(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& parent,
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
	const CGameplayCatalog& catalog,
	std::size_t& selected,
	std::array<std::uint32_t, 4u>& cloneDurationsMs,
	std::string& status)
{
	//trigger의 kind가 cross direction clones가 아니거나, trigger
	if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES ||
		trigger.DirectionPatternIds.size() != 4u || trigger.strCloneEndStageId.empty())
	{
		status = "Cross direction requires four Patterns and a clone end Stage";
		return false;
	}
	//staged duration? 4개의 방향을 저장하는 거 아닌가? 왜 이름이 duration이지?
	std::array<std::uint32_t, 4u> stagedDurations{};

	std::size_t stagedSelection = 0u;
	//이게 이제 가장 중앙와 가장 가까운 값을 저장하는 곳이다. 
	//왜 double로 선언이 된 거지?
	double bestDistance = (std::numeric_limits<double>::max)();
	//왜 yawDegrees에 0.0174 이런 식의 소수점 값을 붙인 거지?
	const double yaw = boss.fYawDegrees * 0.01745329251994329577;

	std::unordered_set<std::string> identities;

	for (std::size_t index = 0u; index < 4u; ++index)
	{
		const auto* child = Find_AnimationOnlyPattern(catalog, trigger.DirectionPatternIds[index], status);

		if (!identities.insert(trigger.DirectionPatternIds[index]).second || !child ||
			!Validate_SummonedPattern(parent, *child, status))
			return false;

		double forward = 0., lateral = 0.;
		std::uint64_t duration = 0u;
		bool foundEnd = false;
		for (const auto& stage : child->Stages)
		{
			duration += stage.iDurationMs;
			if (!foundEnd)
			{
				const auto endpoint = Sample_StageRootMotion(stage.Motion.RootMotion, stage.iDurationMs);
				forward += endpoint.fForward;
				lateral += endpoint.fLateral;
				if (stage.strStageId == trigger.strCloneEndStageId)
				{
					foundEnd = true;
					stagedDurations[index] = static_cast<std::uint32_t>(duration);
				}
			}
		}
		if (!child->bFixedTimelineClock || !foundEnd || duration > trigger.iDurationMs ||
			stagedDurations[index] >= duration)
		{
			status =
				"Cross direction child requires a clone end Stage before its full ending, within the Logic duration";
			return false;
		}
		const double x = boss.fPositionX + lateral * std::cos(yaw) + forward * std::sin(yaw);
		const double z = boss.fPositionZ - lateral * std::sin(yaw) + forward * std::cos(yaw);
		const double dx = x - boss.fSpawnPositionX, dz = z - boss.fSpawnPositionZ;
		const double distance = dx * dx + dz * dz;
		if (!std::isfinite(distance) || forward * forward + lateral * lateral < .000001)
		{
			status = "Cross direction child has no finite directional movement at the clone end Stage";
			return false;
		}
		// 거리 비교는 월드 XZ의 제곱 거리다. clone end stage까지 합산한 변위를 owner의 현재 yaw와 위치로 옮긴 뒤 spawn XZ와 비교한다.
		// 현재 최솟값보다 0.00001을 초과하는 차이로 작을 때만 교체하므로 같은 거리와 작은 차이는 저작된 앞/뒤/좌/우 순서를 유지한다.
		if (distance < bestDistance - .00001)
		{
			bestDistance = distance;
			stagedSelection = index;
		}
	}
	// 명시 real ID가 있으면 거리 선택보다 우선한다. 이 경로도 앞서 네 후보 전체의 유효성을 확인한 뒤 적용한다.
	if (!trigger.strRealPatternId.empty())
	{
		const auto fixed = std::find(
			trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end(), trigger.strRealPatternId);
		if (fixed == trigger.DirectionPatternIds.end())
		{
			status = "Cross direction real Pattern is not a candidate";
			return false;
		}
		stagedSelection = static_cast<std::size_t>(fixed - trigger.DirectionPatternIds.begin());
	}
	selected = stagedSelection;
	cloneDurationsMs = stagedDurations;
	status.clear();
	return true;
}

// GameRoom의 Play All 승인에서 호출한다. 먼저 저작 sequence의 각 참조를 검증하고 이후 몸체/관문/placement로 걸러낸다.
// 따라서 최종 선택에서 제외될 패턴도 이 루프에 도달하면 존재/지원 검증은 통과해야 한다.
bool LostArk::Server::CKoukuSaydonBrain::Select_AnimationOnlySequence(
	const std::vector<BOSS_PATTERN_DEFINITION>& definitions,
	const BOSS_PATTERN_SEQUENCE_DEFINITION& sequence,
	const std::string_view bossArchetypeId,
	std::vector<std::string>& outPatternIds,
	std::vector<std::uint32_t>& outTransitionTicks,
	std::string& status,
	const std::string_view gateId,
	const std::string_view targetBossPlacementId)
{
	if (sequence.PatternIds.empty() || sequence.PatternIds.size() != sequence.iExpectedStepCount ||
		sequence.TransitionPursuitTicks.size() + 1u != sequence.PatternIds.size())
	{
		status = "KoukuSaydon Product pattern sequence is unavailable";
		return false;
	}
	std::vector<std::string> selectedPatterns;
	std::vector<std::uint32_t> selectedTransitions;
	std::size_t previousSelectedIndex = 0u;
	for (std::size_t index = 0u; index < sequence.PatternIds.size(); ++index)
	{
		const std::string& patternId = sequence.PatternIds[index];
		const auto pattern = std::find_if(
			definitions.begin(), definitions.end(), [&patternId](const BOSS_PATTERN_DEFINITION& candidate) {
				return candidate.strPatternId == patternId;
			});
		if (definitions.end() == pattern)
		{
			status = "KoukuSaydon Product sequence names an unknown pattern: " + patternId;
			return false;
		}
		if (!Validate_AnimationOnlyPattern(*pattern, status))
			return false;
		const bool admitted =
			pattern->AuditionBossArchetypeIds.empty() ? KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID == bossArchetypeId
													  : pattern->AuditionBossArchetypeIds.end() !=
				std::find(pattern->AuditionBossArchetypeIds.begin(),
						  pattern->AuditionBossArchetypeIds.end(),
						  bossArchetypeId);
		if (!admitted || (!gateId.empty() && !pattern->strGateId.empty() && pattern->strGateId != gateId) ||
			(!targetBossPlacementId.empty() && !pattern->strTargetBossPlacementId.empty() &&
			 pattern->strTargetBossPlacementId != targetBossPlacementId))
			continue;
		// 새로 남길 패턴 앞에 바로 이전에 남긴 패턴의 transition을 넣는다. 건너뛴 row들의 transition 합은 사용하지 않는다.
		if (!selectedPatterns.empty())
			selectedTransitions.push_back(sequence.TransitionPursuitTicks[previousSelectedIndex]);
		selectedPatterns.push_back(patternId);
		previousSelectedIndex = index;
	}
	if (selectedPatterns.empty())
	{
		status =
			"KoukuSaydon Play All has no Product pattern for the target boss: " + std::string{bossArchetypeId};
		return false;
	}
	// 성공 결과만 출력 vector로 이동한다. 앞선 모든 false 경로는 호출자의 기존 선택을 유지한다.
	outPatternIds = std::move(selectedPatterns);
	outTransitionTicks = std::move(selectedTransitions);
	status.clear();
	return true;
}

// GameRoom/GateProgress가 게시 원본 revision을 확인한다. 새 revision을 생성하거나 파일을 읽는 함수는 아니다.
std::uint32_t LostArk::Server::CKoukuSaydonBrain::Resolve_ProductSourceRevision(
	const CGameplayCatalog& catalog) noexcept
{
	return catalog.Find_KoukuSaydonProductSourceRevision(std::string{KOUKUSAYDON_G1_ENCOUNTER_ID});
}

// G1 encounter의 Catalog vector를 차용해 선형 검색한다. 성공 포인터의 수명은 호출자가 유지하는 Catalog 세대에 달려 있다.
const LostArk::Server::BOSS_PATTERN_DEFINITION* LostArk::Server::CKoukuSaydonBrain::Find_AnimationOnlyPattern(
	const CGameplayCatalog& catalog, const std::string_view patternId, std::string& status)
{
	const auto* patterns = catalog.Find_BossPatterns(std::string{KOUKUSAYDON_G1_ENCOUNTER_ID});
	if (nullptr == patterns)
	{
		status = "KoukuSaydon encounter patterns are missing";
		return nullptr;
	}
	const auto found =
		std::find_if(patterns->begin(), patterns->end(), [patternId](const BOSS_PATTERN_DEFINITION& pattern) {
			return pattern.strPatternId == patternId;
		});
	if (patterns->end() == found)
	{
		status = "KoukuSaydon pattern ID is unknown";
		return nullptr;
	}
	return Validate_AnimationOnlyPattern(*found, status) ? &*found : nullptr;
}

// Begin/Update가 공용으로 쓰는 stage 상태 교체다. root sample을 boss에 복사하고 원점/마지막 적용 tick을 초기화한다.
// 첫 stage는 진입 tick부터, 일반적인 다음 stage는 다음 tick부터 평가하도록 firstEvaluationTick을 정한다.
void LostArk::Server::CKoukuSaydonBrain::Enter_Stage(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_DEFINITION& stage,
	const std::uint32_t stageIndex,
	const std::uint32_t serverTick,
	const bool evaluatesOnEntryTick)
{
	boss.PatternStageRootMotion = stage.Motion.RootMotion;
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginY = 0.f;
	boss.fPatternStageRootGroundY = 0.f;
	boss.iPatternStageIndex = stageIndex;
	boss.strPatternStageId = stage.strStageId;
	boss.strActionId = stage.strActionId;
	boss.iPatternStageDurationMs = stage.iDurationMs;
	boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ? serverTick : Next_ServerTick(serverTick);
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = To_ServerAction(stage.eStageKind);
}

// GameRoom은 승인 사전 점검/예약 시작에서 boss 복사본으로 호출하고 별도 준비 후 live 상태에 반영한다.
// 명시적인 false 경로는 boss를 바꾸기 전에 끝난다. 성공 뒤 수행할 spawn reset/ENTER action/Logic 기록은 GameRoom 책임이다.
bool LostArk::Server::CKoukuSaydonBrain::Begin_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (!revision.Is_Valid() || 0u == serverTick || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction || !boss.strPatternId.empty() ||
		KOUKUSAYDON_G1_ENCOUNTER_ID != boss.strEncounterId || !Validate_AnimationOnlyPattern(pattern, status))
	{
		if (status.empty())
			status = "KoukuSaydon pattern cannot begin from the current boss state";
		return false;
	}
	// 새 occurrence의 접촉/방향/공중/소환 시작 기록을 비우고 revision과 sequence를 고정한다. sequence도 예약값 0을 건너뛴다.
	boss.Restore_KoukuPresentationAim();
	boss.PinnedDefinitionRevision = revision;
	boss.KoukuContactLedger.reset();
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.PatternTerminalReceipt = {};
	boss.bPatternRootGrounded = false;
	boss.AlbionAirborne = {};
	boss.KoukuSummonStartedTriggers.clear();
	boss.strPatternId = pattern.strPatternId;
	boss.iPatternStartTick = serverTick;
	boss.iPatternSequence =
		boss.iPatternSequence == (std::numeric_limits<std::uint32_t>::max)() ? 1u : boss.iPatternSequence + 1u;
	Enter_Stage(boss, pattern.Stages.front(), 0u, serverTick, true);
	Apply_BossMotion(boss, pattern, serverTick);
	status.clear();
	return true;
}

// Update/Abort/Complete가 실행을 종료할 때 공용으로 사용한다. 진행 중인 ID가 있을 때만 terminal receipt를 새로 기록한다.
// Brain의 진행 상태만 비우며 외부 소환/플레이어/월드 cue 정리는 GameRoom이 수행한다. pinned revision과 sequence는 남는다.
void LostArk::Server::CKoukuSaydonBrain::Finish_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick, const SERVER_BOSS_PATTERN_TERMINAL_RESULT result)
{
	(void)serverTick;
	if (!boss.strPatternId.empty())
	{
		boss.PatternTerminalReceipt.iPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.iRootPatternSequence = boss.iPatternSequence;
		boss.PatternTerminalReceipt.eResult = result;
	}
	boss.Restore_KoukuPresentationAim();
	boss.PatternStageRootMotion.clear();
	boss.KoukuContactLedger.reset();
	boss.bPatternRootGrounded = false;
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.AlbionAirborne = {};
	boss.bPatternStageRootOriginCaptured = false;
	boss.iPatternStageRootLastTick = 0u;
	boss.fPatternStageOriginX = boss.fPatternStageOriginY = boss.fPatternStageOriginZ = 0.f;
	boss.fPatternStageOriginYawDegrees = boss.fPatternStageRootGroundY = 0.f;
	boss.strPatternId.clear();
	boss.iPatternStartTick = 0u;
	boss.strPatternStageId.clear();
	boss.strActionId.clear();
	boss.strDamageProfileId.clear();
	boss.iPatternStageIndex = 0u;
	boss.iPatternStageDurationMs = 0u;
	boss.iPatternStageFirstEvaluationTick = 0u;
	boss.iActionStartTick = 0u == serverTick ? 1u : serverTick;
	boss.fActionElapsedSeconds = 0.f;
	boss.eAction = 0u == boss.iCurrentHp ? SERVER_ENTITY_ACTION::DEAD : SERVER_ENTITY_ACTION::IDLE;
}

// GameRoom이 이번 tick의 이동/Logic 처리를 마친 뒤 호출한다. 정의 불일치나 사망은 receipt를 남기고 실행을 중단한다.
// 여기서는 받은 Catalog에서 조회하며, 실행 revision에 맞는 Catalog를 선택하는 일은 GameRoom이 담당한다.
// 보충: 위 순서는 주 보스의 일반 tick 기준이다. 방향 child는 별도 복사본에서 root 이동과 actor contact 판정 뒤 Update를 호출한다.
LostArk::Server::KOUKUSAYDON_BRAIN_UPDATE_RESULT LostArk::Server::CKoukuSaydonBrain::Update(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::uint32_t serverTick,
	std::string& status) const
{
	// pattern ID가 없으면 HP 검사보다 먼저 IDLE을 반환한다. 실행 중인 경우에만 아래 사망/정의 오류 종료가 적용된다.
	if (boss.strPatternId.empty())
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::IDLE;
	}
	if (0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		status = "KoukuSaydon boss died during pattern playback";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD;
	}
	const BOSS_PATTERN_DEFINITION* pattern = Find_AnimationOnlyPattern(catalog, boss.strPatternId, status);
	if (nullptr == pattern || boss.iPatternStageIndex >= pattern->Stages.size() ||
		pattern->Stages[boss.iPatternStageIndex].strStageId != boss.strPatternStageId ||
		pattern->Stages[boss.iPatternStageIndex].strActionId != boss.strActionId)
	{
		Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
		if (status.empty())
			status = "KoukuSaydon running stage no longer matches its pinned definition";
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION;
	}
	Apply_BossMotion(boss, *pattern, serverTick);
	if (pattern->bFixedTimelineClock)
	{
		// 각 stage마다 올림한 기간을 누적하지 않고 pattern 공통 시계의 누적 ms 경계와 비교한다.
		// Logic/이펙트와 같은 시작 시계를 사용하며, 지연된 호출은 이미 지난 여러 stage 경계를 한 번에 넘길 수 있다.
		const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
			? serverTick - boss.iPatternStartTick
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
				serverTick;
		std::uint64_t startMs = 0u;
		std::uint32_t selected = 0u;
		while (selected < pattern->Stages.size() &&
			   ticks * MILLISECONDS_PER_SECOND >=
				   (startMs + pattern->Stages[selected].iDurationMs) * SERVER_TICK_HZ)
			startMs += pattern->Stages[selected++].iDurationMs;
		if (selected == pattern->Stages.size())
		{
			Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
			status.clear();
			return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
		}
		const bool changed = selected != boss.iPatternStageIndex;
		if (changed)
			Enter_Stage(boss, pattern->Stages[selected], selected, serverTick, false);
		const auto offset = static_cast<std::uint32_t>(
			(startMs * SERVER_TICK_HZ + MILLISECONDS_PER_SECOND - 1u) / MILLISECONDS_PER_SECOND);
		boss.iActionStartTick = boss.iPatternStartTick + offset;
		if (boss.iActionStartTick < boss.iPatternStartTick)
			++boss.iActionStartTick;
		if (boss.iActionStartTick == 0u)
			boss.iActionStartTick = 1u;
		boss.fActionElapsedSeconds =
			static_cast<float>((std::max)(0.0,
										  static_cast<double>(ticks) / SERVER_TICK_HZ -
											  static_cast<double>(startMs) / MILLISECONDS_PER_SECOND));
		status.clear();
		return changed ? KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED
					   : KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint64_t elapsedTicks = Stage_ElapsedTicks(boss, serverTick);
	boss.fActionElapsedSeconds = static_cast<float>(elapsedTicks) / static_cast<float>(SERVER_TICK_HZ);
	const BOSS_PATTERN_STAGE_DEFINITION& stage = pattern->Stages[boss.iPatternStageIndex];
	// 일반 시계는 이 호출에서 stage를 최대 한 번 전환한다. 지연분을 여러 stage로 소비하는 고정 시계와 구분한다.
	if (!Has_ElapsedMilliseconds(elapsedTicks, stage.iDurationMs))
	{
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint32_t nextStage = boss.iPatternStageIndex + 1u;
	if (nextStage < pattern->Stages.size())
	{
		Enter_Stage(boss, pattern->Stages[nextStage], nextStage, serverTick, false);
		status.clear();
		return KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED;
	}
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
	status.clear();
	return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
}

// GameRoom의 Logic 전처리와 Brain Update 양쪽에서 호출한다. 동일한 pattern 경과 시각의 절대 XYZ/yaw를 다시 써 누적 이동을 만들지 않는다.
// 이 함수 자체는 navigation/collision을 조회하지 않는다. GameRoom 승인은 key가 있으면 시작 지면만, 없으면 시작/끝과 XZ 경로를 확인한다.
void LostArk::Server::CKoukuSaydonBrain::Apply_BossMotion(
	SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, const std::uint32_t serverTick) noexcept
{
	if (!pattern.BossMotion || !boss.iPatternStartTick || boss.strPatternId != pattern.strPatternId)
		return;
	const auto& motion = *pattern.BossMotion;
	if (motion.iEndMs <= motion.iStartMs)
		return;
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
		? serverTick - boss.iPatternStartTick
		: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
			serverTick;
	const auto position = Sample_BossMotion(motion, static_cast<std::uint32_t>(ticks));
	boss.fPositionX = position[0];
	boss.fPositionY = position[1];
	boss.fPositionZ = position[2];
	boss.fYawDegrees = motion.fYawDegrees;
}

// BossMotion key가 있으면 endpoint clamp와 선형 보간을 사용한다. 없으면 시작/종료 XZ만 보간하고 Y는 시작 높이를 유지한다.
// 키 순서와 유효 범위는 사전 validator의 계약이며 함수 내부에서 전체 키를 재검증하지 않는다.
std::array<float, 3u> LostArk::Server::CKoukuSaydonBrain::Sample_BossMotion(
	const BOSS_PATTERN_BOSS_MOTION& motion, const std::uint32_t elapsedTicks) noexcept
{
	if (motion.iEndMs <= motion.iStartMs)
		return motion.StartPosition;
	const double timeMs = static_cast<double>(elapsedTicks) * 1000.0 / SERVER_TICK_HZ;
	if (!motion.Keys.empty())
	{
		if (timeMs <= motion.Keys.front().iTimeMs)
			return motion.Keys.front().Position;
		if (timeMs >= motion.Keys.back().iTimeMs)
			return motion.Keys.back().Position;
		const auto next =
			std::upper_bound(motion.Keys.begin(), motion.Keys.end(), timeMs, [](double time, const auto& key) {
				return time < key.iTimeMs;
			});
		const auto& previous = *(next - 1);
		const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
		std::array<float, 3u> position{};
		for (std::size_t axis = 0u; axis < position.size(); ++axis)
			position[axis] = static_cast<float>(
				previous.Position[axis] + (next->Position[axis] - previous.Position[axis]) * alpha);
		return position;
	}
	const double alpha =
		(std::clamp)((timeMs - motion.iStartMs) / static_cast<double>(motion.iEndMs - motion.iStartMs),
					 0.0,
					 1.0);
	return {
		static_cast<float>(motion.StartPosition[0] + (motion.EndPosition[0] - motion.StartPosition[0]) * alpha),
		motion.StartPosition[1],
		static_cast<float>(
			motion.StartPosition[2] + (motion.EndPosition[2] - motion.StartPosition[2]) * alpha)};
}

// stage 로컬 시간 ms의 상대 F/L/U 변위를 구한다. 정렬된 키 사이를 선형 보간하며 입력 시간 밖에서는 끝점을 쓴다.
LostArk::Server::ROOT_MOTION_SAMPLE LostArk::Server::CKoukuSaydonBrain::Sample_StageRootMotion(
	const std::vector<ROOT_MOTION_SAMPLE>& samples, const double timeMs) noexcept
{
	if (samples.empty() || !std::isfinite(timeMs))
		return {};
	if (timeMs <= samples.front().iTimeMs)
		return samples.front();
	if (timeMs >= samples.back().iTimeMs)
		return samples.back();
	const auto next = std::upper_bound(
		samples.begin(), samples.end(), timeMs, [](const double time, const ROOT_MOTION_SAMPLE& sample) {
			return time < sample.iTimeMs;
		});
	const auto& previous = *(next - 1);
	const double alpha = (timeMs - previous.iTimeMs) / (next->iTimeMs - previous.iTimeMs);
	return {
		static_cast<std::uint32_t>(timeMs),
		static_cast<float>(previous.fForward + (next->fForward - previous.fForward) * alpha),
		static_cast<float>(previous.fLateral + (next->fLateral - previous.fLateral) * alpha),
		static_cast<float>(previous.fUp + (next->fUp - previous.fUp) * alpha)};
}

// 30Hz pattern 경과 tick을 ms로 바꾼다. UINT32_MAX에서 1로 넘어가는 wrap을 포함하며 유효 시작 tick은 호출자가 보장한다.
double LostArk::Server::CKoukuSaydonBrain::Pattern_ElapsedMs(
	const SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) noexcept
{
	const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
		? serverTick - boss.iPatternStartTick
		: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
			serverTick;
	return double(ticks) * 1000.0 / SERVER_TICK_HZ;
}

// 고정 시계는 pattern 공통 시간에서 이전 stage들의 저작 ms를 뺀다. 일반 시계는 첫 평가 tick을 포함하는 stage 시계를 쓴다.
double LostArk::Server::CKoukuSaydonBrain::Stage_RootTimeMs(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick) noexcept
{
	if (!pattern.bFixedTimelineClock)
		return double(Stage_ElapsedTicks(boss, serverTick)) * 1000.0 / SERVER_TICK_HZ;
	std::uint64_t start = 0u;
	for (std::uint32_t index = 0u; index < boss.iPatternStageIndex && index < pattern.Stages.size(); ++index)
		start += pattern.Stages[index].iDurationMs;
	return (std::max)(0.0, Pattern_ElapsedMs(boss, serverTick) - double(start));
}

// BossSimulation의 공중 phase 전환과 아래 root 이동이 사용한다. 높이는 현재 지면 위 m이고 SLAM 진행률은 역행하지 않게 누적한다.
// state/outHeight는 참조 출력이므로 false라도 변경될 수 있다. root 이동 호출자는 지역 airborneState를 넘겨 최종 성공 때 반영한다.
bool LostArk::Server::CKoukuSaydonBrain::Sample_AlbionAirborneHeight(
	SERVER_ALBION_AIRBORNE_STATE& state,
	const double patternMs,
	const std::uint32_t stageIndex,
	const float sourceUp,
	float& outHeight) noexcept
{
	using Phase = ALBION_AIRBORNE_PHASE;
	switch (state.ePhase)
	{
	case Phase::JUMP:
		if (state.iDurationMs == 0u)
		{
			outHeight = state.fJumpHeightM;
			break;
		}
		outHeight = static_cast<float>(
			state.fStartHeightM +
			(state.fJumpHeightM - state.fStartHeightM) *
				(std::clamp)((patternMs - state.iStartMs) / state.iDurationMs, 0.0, 1.0));
		break;
	case Phase::APPEAR_PLAYER:
		outHeight = stageIndex == state.iSourceStageIndex ? state.fPhaseHeightM + sourceUp : 0.f;
		break;
	case Phase::DISAPPEAR:
	case Phase::CENTER:
		outHeight = state.fJumpHeightM;
		break;
	case Phase::SLAM:
		if (state.fSourceUpAtStart - state.fSourceUpMinimum <= .000001f)
			return false;
		if (stageIndex != state.iSourceStageIndex)
			state.fLandingProgress = 1.f;
		else
			state.fLandingProgress =
				(std::max)(state.fLandingProgress,
						   (std::clamp)((state.fSourceUpAtStart - sourceUp) /
											(state.fSourceUpAtStart - state.fSourceUpMinimum),
										0.f,
										1.f));
		outHeight = state.fStartHeightM * (1.f - state.fLandingProgress);
		break;
	default:
		return false;
	}
	return std::isfinite(outHeight) && std::abs(outHeight) <= 100000.f;
}

// GameRoom이 Logic 전에 호출한다. navigation/collision은 Room 소유의 차용 입력이고, 현재 root 기준 시각의 후보 XYZ를 검사한다.
// 최종 좌표는 끝에서 반영하지만 최초 origin capture는 뒤 검사가 실패해도 남을 수 있다. true에는 경계 위치 유지와 no-op도 포함된다.
// 보충: 주 보스는 일반 tick의 Logic 전에, 방향 child는 접촉 판정 전에 사용한다. Commit_KoukuAlbionAirborne의 SLAM 전환도 staged 상태로 호출한다.
bool LostArk::Server::CKoukuSaydonBrain::Apply_StageRootMotion(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const std::uint32_t serverTick,
	const CServerNavigation& navigation,
	const CServerCollisionSystem& collision,
	std::string& status)
{
	status.clear();
	const bool airborne = boss.AlbionAirborne.iPatternSequence == boss.iPatternSequence &&
		boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::NONE &&
		(boss.AlbionAirborne.ePhase != ALBION_AIRBORNE_PHASE::SLAM ||
		 boss.iPatternStageIndex == boss.AlbionAirborne.iSourceStageIndex);
	// 이동할 것이 없거나 이미 처리한 tick이면 true다. 이 조기 반환은 아래 serverTick/현재 stage 검사보다 먼저 실행된다.
	if ((!airborne && boss.PatternStageRootMotion.empty()) || boss.strPatternId.empty() ||
		boss.iPatternStageRootLastTick == serverTick)
		return true;
	if (serverTick == 0u || boss.strPatternId != pattern.strPatternId ||
		boss.iPatternStageIndex >= pattern.Stages.size())
	{
		status = "KoukuSaydon root motion lost its running stage";
		return false;
	}
	// GameRoom이 spawn reset과 stage ENTER action을 반영한 뒤 첫 root 이동에서 원점을 capture한다.
	SERVER_NAV_POINT originGround;
	if (!boss.bPatternStageRootOriginCaptured)
	{
		if (!std::isfinite(boss.fPositionX) || !std::isfinite(boss.fPositionY) ||
			!std::isfinite(boss.fPositionZ) || !std::isfinite(boss.fYawDegrees) ||
			!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, originGround))
		{
			status = "KoukuSaydon root motion origin is not navigable";
			return false;
		}
		boss.fPatternStageOriginX = boss.fPositionX;
		// 명시적으로 착지한 뒤 다음 stage를 capture할 때는 현재 지면을 Y 기준으로 써 이전 clip의 잔여 Up을 누적하지 않는다.
		// 착지 표시가 없는 경우에는 현재 boss Y를 기준으로 유지한다.
		boss.fPatternStageOriginY = boss.bPatternRootGrounded ? originGround.y : boss.fPositionY;
		boss.fPatternStageOriginZ = boss.fPositionZ;
		boss.fPatternStageOriginYawDegrees = boss.fYawDegrees;
		boss.fPatternStageRootGroundY = originGround.y;
		boss.bPatternStageRootOriginCaptured = true;
	}
	// 고정 origin과 현재 stage sample로 후보를 만든다. origin을 매 tick 갱신해 상대 변위를 다시 누적하지 않는다.
	const double elapsedMs = Stage_RootTimeMs(boss, pattern, serverTick);
	const auto sample = Sample_StageRootMotion(boss.PatternStageRootMotion, elapsedMs);
	constexpr double RADIANS_PER_DEGREE = 3.14159265358979323846 / 180.0;
	const double yaw = boss.fPatternStageOriginYawDegrees * RADIANS_PER_DEGREE;
	// 아래 변환은 lateral을 model X, forward를 model Z로 읽고 capture한 yaw로 월드 XZ에 회전한다.
	// 시각적 전방은 보스 몸체별 yaw 보정도 소비하므로 필드명 forward를 곧바로 화면 전방으로 해석하지 않는다.
	// 관련 추적 방향 보정은 GameRoom_KoukuTargeting.cpp의 Update_KoukuPlayerTargets에서 forwardYawOffset을 사용한다.
	SERVER_NAV_POINT destination{
		static_cast<float>(
			boss.fPatternStageOriginX + sample.fLateral * std::cos(yaw) + sample.fForward * std::sin(yaw)),
		boss.fPatternStageOriginY + sample.fUp,
		static_cast<float>(
			boss.fPatternStageOriginZ - sample.fLateral * std::sin(yaw) + sample.fForward * std::cos(yaw))};
	// SLAM은 현재 확정된 목표/중앙의 XZ를 유지한다. source의 가로 흔들림을 보스 월드 이동에 더하지 않는다.
	if (airborne && boss.AlbionAirborne.ePhase == ALBION_AIRBORNE_PHASE::SLAM)
	{
		destination.x = boss.fPositionX;
		destination.z = boss.fPositionZ;
	}
	if (!std::isfinite(destination.x) || !std::isfinite(destination.y) || !std::isfinite(destination.z))
	{
		status = "KoukuSaydon root motion destination is invalid";
		return false;
	}
	SERVER_NAV_POINT ground;
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(
			boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		// 현재 위치에서 요청 위치까지의 구간을 이분 탐색해 마지막으로 승인된 navigation 지점을 찾는다.
		// 최대 40회이며 남은 구간 길이가 1mm 이하가 되면 멈춘다. stage 시계와 capture 원점은 변경하지 않는다.
		// 다음 호출도 원점+해당 시각의 sample을 요청하므로 매 tick 이동량을 계속 누적하는 방식은 아니다.
		const double segmentX = double(destination.x) - double(boss.fPositionX);
		const double segmentZ = double(destination.z) - double(boss.fPositionZ);
		const double segmentLength = std::sqrt(segmentX * segmentX + segmentZ * segmentZ);
		constexpr double CLAMP_RESOLUTION_M = 0.001;
		double low = 0.0, high = 1.0;
		bool found = false;
		SERVER_NAV_POINT clamped{};
		for (unsigned iteration = 0u; iteration < 40u && (high - low) * segmentLength > CLAMP_RESOLUTION_M;
			 ++iteration)
		{
			const double mid = (low + high) * 0.5;
			const float candidateX = static_cast<float>(boss.fPositionX + segmentX * mid);
			const float candidateZ = static_cast<float>(boss.fPositionZ + segmentZ * mid);
			SERVER_NAV_POINT candidate{};
			if (navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, candidateX, candidateZ) &&
				navigation.Resolve_TraversalStep(
					boss.fPositionX, boss.fPositionZ, candidateX, candidateZ, candidate))
			{
				low = mid;
				clamped = candidate;
				clamped.x = candidateX;
				clamped.z = candidateZ;
				found = true;
			}
			else
				high = mid;
		}
		// 승인된 구간을 찾지 못했거나 이동 거리가 1mm 미만이면 XYZ는 그대로 두고 이번 tick을 처리한 것으로 기록한다.
		if (!found || low * segmentLength < CLAMP_RESOLUTION_M)
		{
			boss.iPatternStageRootLastTick = serverTick;
			return true; // 경계에서 위치를 유지해도 GameRoom은 이어서 stage 시계를 진행한다.
		}
		destination.x = clamped.x;
		destination.z = clamped.z;
		ground = clamped;
	}
	// navigation의 목적지 지면 높이 차를 더한다. source Up은 capture 시점의 stage 원점에 대한 상대 높이 m다.
	SERVER_NAV_POINT startGround;
	if (!navigation.Sample_Position(boss.fPositionX, boss.fPositionZ, startGround))
	{
		status = "KoukuSaydon root motion lost its current ground";
		return false;
	}
	const float targetGroundY = ground.y;
	destination.y += targetGroundY - boss.fPatternStageRootGroundY;
	if (boss.bPatternRootGrounded)
		destination.y = (std::max)(destination.y, targetGroundY);
	auto airborneState = boss.AlbionAirborne;
	// airborne 계산은 복사본에서 수행한다. 아래 검사 실패로 실제 boss의 착지 진행도까지 먼저 갱신하지 않도록 한다.
	if (airborne)
	{
		float height = 0.f;
		float airborneSourceUp = sample.fUp;
		if (airborneState.ePhase == ALBION_AIRBORNE_PHASE::SLAM &&
			boss.iPatternStageIndex == airborneState.iSourceStageIndex)
		{
			std::uint64_t stageStart = 0u;
			for (std::uint32_t index = 0u; index < boss.iPatternStageIndex; ++index)
				stageStart += pattern.Stages[index].iDurationMs;
			const double triggerAge = (std::max)(0.0, double(airborneState.iStartMs) - double(stageStart));
			// 지연된 호출도 SLAM 시작부터 현재 시각 사이에 지나온 root Up 최솟값을 고려한다. 과거 위치로 충돌을 재실행하지는 않는다.
			for (const auto& key : boss.PatternStageRootMotion)
				if (key.iTimeMs >= triggerAge && key.iTimeMs <= elapsedMs)
					airborneSourceUp = (std::min)(airborneSourceUp, key.fUp);
		}
		if (!Sample_AlbionAirborneHeight(
				airborneState,
				Pattern_ElapsedMs(boss, serverTick),
				boss.iPatternStageIndex,
				airborneSourceUp,
				height))
		{
			status = "Albion airborne height lost its validated phase";
			return false;
		}
		destination.y = targetGroundY + height;
	}
	const SERVER_NAV_POINT proposed = destination;
	bool blocked = false;
	if (!collision.Resolve_CircleMove(
			boss.fPositionX,
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
			blocked,
			boss.iNetEntityId,
			false))
	{
		status = "KoukuSaydon root motion collision resolution failed";
		return false;
	}
	// 몸체 sweep 후의 XZ도 navigation으로 다시 검사한다. 승인되지 않으면 현재 좌표를 유지하고 이 tick을 소비한다.
	if (!navigation.Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
		!navigation.Resolve_TraversalStep(
			boss.fPositionX, boss.fPositionZ, destination.x, destination.z, ground))
	{
		boss.iPatternStageRootLastTick = serverTick;
		return true;
	}
	if (blocked)
	{
		// 충돌로 잘린 XYZ의 경로 비율에서 예상 지면 높이를 계산하고 실제 resolved XZ의 지면과 차이를 보정한다.
		// 보간된 source Up은 유지하며 필요한 높이 보정도 별도 body sweep으로 확인한다.
		const double dx = proposed.x - boss.fPositionX, dy = proposed.y - boss.fPositionY,
					 dz = proposed.z - boss.fPositionZ;
		const double squaredDistance = dx * dx + dy * dy + dz * dz;
		const double ratio = squaredDistance > 1e-12
			? (std::clamp)(((destination.x - boss.fPositionX) * dx + (destination.y - boss.fPositionY) * dy +
							(destination.z - boss.fPositionZ) * dz) /
							   squaredDistance,
						   0.0,
						   1.0)
			: 0.0;
		const float terrainCorrection =
			ground.y - static_cast<float>(startGround.y + (targetGroundY - startGround.y) * ratio);
		if (std::abs(terrainCorrection) > .00001f)
		{
			bool correctionBlocked = false;
			SERVER_NAV_POINT corrected;
			if (!collision.Resolve_CircleMove(
					destination.x,
					destination.y,
					destination.z,
					destination.x,
					destination.y + terrainCorrection,
					destination.z,
					boss.fCollisionRadius,
					boss.fCollisionRadius,
					boss.fCollisionRadius,
					corrected.x,
					corrected.y,
					corrected.z,
					correctionBlocked,
					boss.iNetEntityId,
					false))
			{
				status = "KoukuSaydon root ground correction failed collision validation";
				return false;
			}
			if (correctionBlocked)
			{
				boss.iPatternStageRootLastTick = serverTick;
				return true;
			}
			destination = corrected;
		}
	}
	// navigation과 몸체 sweep의 검사 후에만 최종 XYZ를 함께 대입한다. 앞선 원점 capture와 no-op tick 기록은 별도 상태 변경이다.
	boss.fPositionX = destination.x;
	boss.fPositionY = destination.y;
	boss.fPositionZ = destination.z;
	if (airborne)
		boss.AlbionAirborne = airborneState;
	boss.iPatternStageRootLastTick = serverTick;
	return true;
}

// GameRoom의 취소/실행 정리에서 호출한다. 실행 ID가 있으면 ABORTED receipt를 쓰고 없어도 내부 진행 필드는 정리한다.
void LostArk::Server::CKoukuSaydonBrain::Abort_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) const
{
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::ABORTED);
}

// Logic 결과/연쇄 종료가 요청한 조기 완료다. 실행 중인 ID가 있을 때만 COMPLETED로 정리하며 나머지 stage는 진행하지 않는다.
void LostArk::Server::CKoukuSaydonBrain::Complete_Pattern(
	SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick) const
{
	if (boss.strPatternId.empty())
		return;
	Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
}
```

