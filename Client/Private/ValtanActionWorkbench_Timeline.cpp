// timeline 드래그의 Pattern 전체 시각을 소유 Stage의 local 시각으로 변환해 typed draft setter에 보낸다.
// 눈금·box를 그리는 본체 Render_Timeline과 달리 이 파일은 변경할 시간 구간의 검증과 적용을 담당한다.
#include "CompositionTimeline.h"
#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"

#include <algorithm>
#include <limits>
#include <string_view>

// Stage 누적 clock과 경계 탐색은 이 CPP의 적용 알고리즘에서만 쓰므로 익명 이름 공간에 둔다.
// 아래 차용 포인터는 호출 안에서 만든 fresh Pattern과 clocks vector가 살아 있는 동안만 사용한다.
namespace
{
    using namespace Client;

    // stage는 입력 Pattern 안의 주소이고 startMs/endMs는 선택 preview 경로 시작을 0으로 한 누적 ms다.
    // Stage-local clock과 Pattern-global clock을 같이 보관해 이동 때 올바른 원점을 사용할 수 있게 한다.
    struct STAGE_CLOCK final
    {
        const VALTAN_STAGE_VIEW* stage = nullptr;
        uint32_t startMs = 0u;
        uint32_t endMs = 0u;
    };

    // 선택 경로의 Stage 길이를 64bit로 누적하며 0 길이·null·32bit 결과 범위 초과를 거절한다.
    // 현재 호출자는 빈 vector를 넘기고 false이면 폐기한다. 함수 내부 실패가 이미 push한 앞쪽 clock을 되돌리지는 않는다.
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

    // 각 Stage의 ENTER/EXIT까지 ms 거리를 비교한다. 같은 거리면 기존 trigger와 Stage 선호 점수로 결정한다.
    // 모든 경계를 한 번씩 보므로 별도 검색 cache 없이 현재 preview 경로의 결과를 얻는다.
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

    // 환경 occurrence의 원본/목적지 행을 값으로 복사해 준비한다. 같은 Stage면 한 setter만 호출한다.
    // 다른 Stage로 옮길 때는 두 setter를 BalanceTool transaction으로 묶어 앞쪽 삭제만 남는 부분 적용을 막는다.
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

// Render_Timeline이 놓기·trim을 확정할 때 호출한다. frame 표시 행의 ID는 재조회 key로만 사용하고 최신 Balance draft를 읽는다.
// 입력 newStartMs/newEndMs는 Pattern 전체의 ms다. 대상 Stage를 정한 뒤 local offset·duration으로 바꿔 해당 소유 도구에 전달한다.
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
    // 드래그를 놓는 시점에는 공용 draft를 다시 읽는다. 표시 행은 Details 편집보다 오래됐을 수 있고
    // cache의 절대 시각은 현재 원본 clock을 대신할 수 없다.
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

bool_t Client::CValtanActionWorkbench::Trim_AnimationOccurrence(
    const VALTAN_PATTERN_VIEW& Pattern, const VALTAN_STAGE_VIEW& Stage,
    const std::string& occurrenceId, bool_t startEdge, int64_t wallDeltaMs,
    std::string& status)
{
    if (!m_pBalanceTool || !m_pAnimationTool) { status = "Animation owner is unavailable."; return false; }
    CBalanceTool::PATTERN_STAGE_EDIT draft;
    if (!m_pBalanceTool->Get_ValtanStageDraft(Pattern.strPatternId, Stage.strStageId, draft, status)) return false;
    const auto slot = std::find_if(draft.animationSlots.begin(), draft.animationSlots.end(),
        [&](const auto& value) { return value.clipOccurrenceId == occurrenceId; });
    if (slot == draft.animationSlots.end() || slot->repeatUntilStageEnd)
    { status = "Source edge trim requires one finite animation occurrence."; return false; }
    uint32_t nativeDuration = 0u, start = 0u, play = 0u;
    if (!m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(slot->clip, nativeDuration, status)) return false;
    if (!CompositionTimeline::TrimSequentialSourceWindow(slot->sourceStartMs, slot->playMs,
            nativeDuration, slot->playRate, startEdge, wallDeltaMs, start, play))
    { status = "The source window or play rate no longer admits an edge trim."; return false; }
    return Apply_AnimationOccurrenceTiming(Pattern, Stage, occurrenceId, start, play, status);
}
