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
