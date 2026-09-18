#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"

#include <algorithm>
#include <limits>
#include <string_view>

namespace
{
    using namespace Client;

    struct STAGE_CLOCK final
    {
        const VALTAN_STAGE_VIEW* stage = nullptr;
        uint32_t startMs = 0u;
        uint32_t endMs = 0u;
    };

    bool BuildStageClocks(const VALTAN_PATTERN_VIEW& pattern,
        VALTAN_PATTERN_PREVIEW_PATH path, std::vector<STAGE_CLOCK>& clocks,
        std::string& status)
    {
        std::vector<const VALTAN_STAGE_VIEW*> stages;
        if (!CValtanPatternTree::Build_PreviewStagePath(pattern, path, stages, status))
            return false;
        uint64_t cursor = 0u;
        for (const auto* stage : stages)
        {
            if (nullptr == stage || 0u == stage->iDurationMs ||
                cursor + stage->iDurationMs > (std::numeric_limits<uint32_t>::max)())
            {
                status = "The current preview path has an unresolved or overflowing Stage clock.";
                return false;
            }
            clocks.push_back({stage, static_cast<uint32_t>(cursor),
                static_cast<uint32_t>(cursor + stage->iDurationMs)});
            cursor += stage->iDurationMs;
        }
        return !clocks.empty();
    }

    const STAGE_CLOCK* FindContainingStage(const std::vector<STAGE_CLOCK>& clocks,
        uint32_t startMs, uint32_t endMs, bool requireEnd)
    {
        const auto found = std::find_if(clocks.begin(), clocks.end(),
            [=](const STAGE_CLOCK& clock)
            {
                return startMs >= clock.startMs && startMs < clock.endMs &&
                    (!requireEnd || endMs <= clock.endMs);
            });
        return found == clocks.end() ? nullptr : &*found;
    }

    struct ACTION_BOUNDARY final
    {
        const STAGE_CLOCK* clock = nullptr;
        std::string trigger;
        uint32_t timeMs = 0u;
    };

    ACTION_BOUNDARY NearestBoundary(const std::vector<STAGE_CLOCK>& clocks,
        uint32_t requestedMs, const std::string& preferredTrigger,
        const std::string& preferredStage, bool preserveTrigger)
    {
        ACTION_BOUNDARY result;
        uint64_t bestDistance = (std::numeric_limits<uint64_t>::max)();
        int bestPreference = -1;
        for (const auto& clock : clocks)
        {
            for (const char* trigger : {"ENTER", "EXIT"})
            {
                if (preserveTrigger && preferredTrigger != trigger)
                    continue;
                const uint32_t boundary = std::string_view(trigger) == "ENTER" ?
                    clock.startMs : clock.endMs;
                const uint64_t distance = requestedMs > boundary ?
                    uint64_t(requestedMs) - boundary : uint64_t(boundary) - requestedMs;
                const int preference = (preferredTrigger == trigger ? 2 : 0) +
                    (preferredStage == clock.stage->strStageId ? 1 : 0);
                if (distance < bestDistance ||
                    (distance == bestDistance && preference > bestPreference))
                {
                    result = {&clock, trigger, boundary};
                    bestDistance = distance;
                    bestPreference = preference;
                }
            }
        }
        return result;
    }

    bool SameActionIdentity(const VALTAN_STAGE_ACTION_VIEW& left,
        const VALTAN_STAGE_ACTION_VIEW& right)
    {
        return left.strTrigger == right.strTrigger && left.strKind == right.strKind &&
            left.strTargetId == right.strTargetId;
    }

    template<typename T, typename Setter>
    bool MoveEnvironmentOccurrence(CBalanceTool& balance,
        const VALTAN_PATTERN_VIEW& pattern, const STAGE_CLOCK& source,
        const STAGE_CLOCK& target, const std::string& occurrenceId,
        uint32_t newStartMs, uint32_t newEndMs,
        const std::vector<T>& sourceRows, const std::vector<T>& targetRows,
        Setter setter, std::string& status)
    {
        auto sourceCandidate = sourceRows;
        const auto found = std::find_if(sourceCandidate.begin(), sourceCandidate.end(),
            [&](const T& row) { return row.strOccurrenceId == occurrenceId; });
        if (found == sourceCandidate.end())
        {
            status = "The selected environment occurrence no longer exists.";
            return false;
        }
        T changed = *found;
        changed.iStartMs = newStartMs - target.startMs;
        changed.iDurationMs = newEndMs - newStartMs;
        if (source.stage == target.stage)
        {
            *found = changed;
            return (balance.*setter)(pattern.strPatternId, source.stage->strStageId,
                sourceCandidate, status);
        }
        auto targetCandidate = targetRows;
        if (targetCandidate.end() != std::find_if(targetCandidate.begin(), targetCandidate.end(),
            [&](const T& row) { return row.strOccurrenceId == occurrenceId; }))
        {
            status = "The target Stage already owns this environment occurrence ID.";
            return false;
        }
        sourceCandidate.erase(found);
        targetCandidate.push_back(std::move(changed));
        return balance.Apply_ValtanCompositionDraftTransaction(
            [&](std::string& transactionStatus)
            {
                return (balance.*setter)(pattern.strPatternId, source.stage->strStageId,
                           sourceCandidate, transactionStatus) &&
                    (balance.*setter)(pattern.strPatternId, target.stage->strStageId,
                           targetCandidate, transactionStatus);
            }, status);
    }
}

bool_t Client::CValtanActionWorkbench::Apply_AuxiliaryTimelineTiming(
    const VALTAN_PATTERN_VIEW& pattern, const TIMELINE_ITEM& item,
    uint32_t newStartMs, uint32_t newEndMs, bool trim, std::string& status)
{
    if (nullptr == m_pBalanceTool || item.strPatternId != pattern.strPatternId ||
        newEndMs < newStartMs)
    {
        status = "The selected timeline owner or proposed timing is unavailable.";
        return false;
    }
    // Use the shared draft again at release: the displayed row may predate a
    // Details edit, and its cached absolute bounds are not the source clock.
    VALTAN_PATTERN_VIEW fresh;
    if (!m_pBalanceTool->Get_ValtanPatternDraft(pattern.strPatternId, fresh, status))
        return false;
    std::vector<STAGE_CLOCK> clocks;
    if (!BuildStageClocks(fresh, m_ePreviewPath, clocks, status))
        return false;
    const auto sourceIt = std::find_if(clocks.begin(), clocks.end(),
        [&](const STAGE_CLOCK& clock) { return clock.stage->strStageId == item.strStageId; });
    if (sourceIt == clocks.end())
    {
        status = "The selected Stage is no longer on the active preview path.";
        return false;
    }
    const STAGE_CLOCK& source = *sourceIt;

    if (item.eLane == TIMELINE_LANE::STAGE)
    {
        if (trim)
        {
            if (newStartMs != source.startMs || newEndMs <= newStartMs)
            {
                status = "A Stage trim must retain its start boundary and a positive duration.";
                return false;
            }
            return Apply_StageDurationDraft(fresh, source.stage->strStageId,
                newEndMs - newStartMs, status);
        }
        if (sourceIt == clocks.begin())
        {
            status = "The first Stage starts at zero; move later Stages to change the preceding gap.";
            return false;
        }
        const STAGE_CLOCK& previous = *(sourceIt - 1);
        const int64_t duration = int64_t(previous.stage->iDurationMs) +
            int64_t(newStartMs) - source.startMs;
        if (duration <= 0 || duration > 600000)
        {
            status = "The preceding Stage must retain a duration between 1 and 600000 ms.";
            return false;
        }
        if (!Apply_StageDurationDraft(fresh, previous.stage->strStageId,
            static_cast<uint32_t>(duration), status))
            return false;
        status = "Moved the Stage boundary by resizing " + previous.stage->strStageId +
            "; following Stage clocks move together.";
        return true;
    }

    if ((item.eLane == TIMELINE_LANE::LOGIC || item.eLane == TIMELINE_LANE::WORLD) &&
        item.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE)
    {
        const auto found = std::find_if(source.stage->Actions.begin(), source.stage->Actions.end(),
            [&](const VALTAN_STAGE_ACTION_VIEW& action)
            {
                return source.stage->strStageId + "/action/" + action.strKind + "/" +
                    action.strTargetId + "/" + action.strTrigger == item.strStableId;
            });
        if (found == source.stage->Actions.end())
        {
            status = "This Logic row is a branch or motion owner; edit its typed detail controls.";
            return false;
        }
        const VALTAN_STAGE_ACTION_VIEW original = *found;
        if (original.strKind == "SET_BOSS_FLAG" &&
            (original.strTargetId == "boss.flag.counterable" ||
             original.strTargetId == "boss.flag.groggy"))
        {
            status = "Counter and Groggy flags follow their typed Stage topology.";
            return false;
        }
        const bool isState = original.strKind == "SET_BOSS_FLAG" ||
            original.strKind == "SET_PLAYER_BIND" || original.strKind == "SET_PLAYER_SILENCE";
        std::vector<VALTAN_STAGE_ACTION_VIEW> originals{original};
        if (isState)
        {
            for (const auto& action : source.stage->Actions)
            {
                if (action.strKind == original.strKind && action.strTargetId == original.strTargetId &&
                    action.strTrigger != original.strTrigger &&
                    ((action.fValue == 0.f) != (original.fValue == 0.f)))
                    originals.push_back(action);
            }
            if (originals.size() > 2u)
            {
                status = "The selected state has an ambiguous closing action.";
                return false;
            }
        }
        const bool paired = originals.size() == 2u;
        if (isState && !paired)
        {
            for (const auto& clock : clocks)
            {
                if (clock.stage == source.stage)
                    continue;
                const auto release = std::find_if(clock.stage->Actions.begin(), clock.stage->Actions.end(),
                    [&](const auto& action)
                    {
                        return action.strKind == original.strKind && action.strTargetId == original.strTargetId &&
                            ((action.fValue == 0.f) != (original.fValue == 0.f));
                    });
                if (release != clock.stage->Actions.end())
                {
                    status = "This state is paired across Stages. Move its Stage boundaries to preserve both transitions.";
                    return false;
                }
            }
        }
        if (trim)
        {
            status = "Logic and World actions run at ENTER/EXIT boundaries. Move the row, or resize its Stage.";
            return false;
        }
        const ACTION_BOUNDARY boundary = NearestBoundary(clocks, newStartMs,
            original.strTrigger, source.stage->strStageId, paired);
        if (nullptr == boundary.clock)
            return false;
        std::vector<VALTAN_STAGE_ACTION_VIEW> changed = originals;
        if (!paired)
            changed.front().strTrigger = boundary.trigger;
        const auto& target = *boundary.clock;
        if (source.stage == target.stage && original.strTrigger == changed.front().strTrigger)
        {
            status = "The action remains at its nearest Stage boundary.";
            return true;
        }
        for (const auto& row : changed)
        {
            if (target.stage->Actions.end() != std::find_if(target.stage->Actions.begin(),
                target.stage->Actions.end(), [&](const auto& candidate)
                { return SameActionIdentity(candidate, row); }))
            {
                status = "The target boundary already owns this action identity; the existing action was preserved.";
                return false;
            }
        }
        if (!m_pBalanceTool->Apply_ValtanCompositionDraftTransaction(
            [&](std::string& transactionStatus)
            {
                for (const auto& row : originals)
                    if (!m_pBalanceTool->Remove_ValtanStageActionDraft(fresh.strPatternId,
                        source.stage->strStageId, row, transactionStatus)) return false;
                for (const auto& row : changed)
                    if (!m_pBalanceTool->Upsert_ValtanStageActionDraft(fresh.strPatternId,
                        target.stage->strStageId, row, transactionStatus)) return false;
                return true;
            }, status)) return false;
        status = "Action snapped to " + target.stage->strStageId + " / " + boundary.trigger +
            (paired ? "; its paired state release moved with it." : ".");
        return true;
    }

    const bool summon = item.eOwner == DETAIL_OWNER::COMBAT_OBJECT &&
        item.eLane == TIMELINE_LANE::SUMMON;
    if (newEndMs == newStartMs)
    {
        status = "This occurrence requires a positive duration.";
        return false;
    }
    const STAGE_CLOCK* target = trim ? &source :
        FindContainingStage(clocks, newStartMs, newEndMs, !summon);
    if (nullptr == target || newStartMs < target->startMs ||
        newStartMs >= target->endMs || (!summon && newEndMs > target->endMs))
    {
        status = "The occurrence must fit inside one Stage; resize it before moving across a boundary.";
        return false;
    }

    if (item.eOwner == DETAIL_OWNER::SCENE_PROFILE)
        return MoveEnvironmentOccurrence(*m_pBalanceTool, fresh, source, *target,
            item.strStableId, newStartMs, newEndMs, source.stage->SceneProfileOccurrences,
            target->stage->SceneProfileOccurrences,
            &CBalanceTool::Set_ValtanStageSceneProfileOccurrences, status);
    if (item.eOwner == DETAIL_OWNER::LIGHT)
        return MoveEnvironmentOccurrence(*m_pBalanceTool, fresh, source, *target,
            item.strStableId, newStartMs, newEndMs, source.stage->LightOccurrences,
            target->stage->LightOccurrences,
            &CBalanceTool::Set_ValtanStageLightOccurrences, status);

    if (item.eOwner == DETAIL_OWNER::CAMERA)
    {
        const auto found = std::find_if(source.stage->CameraInvocations.begin(),
            source.stage->CameraInvocations.end(), [&](const auto& row)
            { return row.strCameraInvocationId == item.strStableId; });
        if (found == source.stage->CameraInvocations.end())
        {
            status = "This Camera row is not a Stage camera invocation.";
            return false;
        }
        const auto original = *found;
        auto changed = original;
        changed.iStartOffsetMs = newStartMs - target->startMs;
        changed.iDurationMs = newEndMs - newStartMs;
        changed.strDurationPolicy = "EXPLICIT";
        if (target == &source)
            return m_pBalanceTool->Upsert_ValtanCameraInvocationDraft(fresh.strPatternId,
                source.stage->strStageId, changed, status);
        if (target->stage->CameraInvocations.end() != std::find_if(
            target->stage->CameraInvocations.begin(), target->stage->CameraInvocations.end(),
            [&](const auto& row) { return row.strCameraInvocationId == original.strCameraInvocationId; }))
        {
            status = "The target Stage already owns this Camera invocation ID.";
            return false;
        }
        return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction(
            [&](std::string& transactionStatus)
            {
                return m_pBalanceTool->Remove_ValtanCameraInvocationDraft(fresh.strPatternId,
                    source.stage->strStageId, original, transactionStatus) &&
                    m_pBalanceTool->Upsert_ValtanCameraInvocationDraft(fresh.strPatternId,
                        target->stage->strStageId, changed, transactionStatus);
            }, status);
    }

    if (summon)
    {
        const auto& objects = source.stage->CombatObjectEffects;
        const auto found = std::find_if(objects.begin(), objects.end(), [&](const auto& row)
            { return row.strCombatObjectArchetypeId == item.strStableId; });
        if (found == objects.end() || 1 != std::count_if(objects.begin(), objects.end(),
            [&](const auto& row) { return row.strCombatObjectArchetypeId == item.strStableId; }))
        {
            status = "The selected Summon identity is missing or ambiguous.";
            return false;
        }
        const auto original = *found;
        auto changed = original;
        changed.iFirstSpawnOffsetMs = newStartMs - target->startMs;
        if (trim)
            changed.iLifetimeMs = newEndMs - newStartMs;
        if (source.stage != target->stage && target->stage->CombatObjectEffects.end() !=
            std::find_if(target->stage->CombatObjectEffects.begin(), target->stage->CombatObjectEffects.end(),
                [&](const auto& row) { return row.strCombatObjectArchetypeId == original.strCombatObjectArchetypeId &&
                    row.strTrigger == original.strTrigger; }))
        {
            status = "The target Stage already owns this Summon archetype and trigger.";
            return false;
        }
        const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction(
            [&](std::string& transactionStatus)
            {
                if (source.stage != target->stage &&
                    !m_pBalanceTool->Remove_ValtanSummonDraft(fresh.strPatternId,
                        source.stage->strStageId, original, transactionStatus)) return false;
                return m_pBalanceTool->Upsert_ValtanSummonDraft(fresh.strPatternId,
                    target->stage->strStageId, changed, transactionStatus);
            }, status);
        if (applied && trim)
            status = "Updated the shared Summon resource lifetime; every use of this archetype consumes that lifetime.";
        return applied;
    }
    status = "This timeline row is owned by a separate typed editor.";
    return false;
}
