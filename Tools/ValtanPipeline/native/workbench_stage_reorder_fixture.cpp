// Storage doubles test the extracted production selection operation and rollback.
// The Balance source topology validator and physical Save have separate checks.
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
using bool_t = bool;
namespace Client {
struct VALTAN_CLIP_OCCURRENCE_VIEW {
    std::string strClipOccurrenceId;
    uint32_t iSourceStartMs = 0, iPlayMs = 500;
    bool operator==(const VALTAN_CLIP_OCCURRENCE_VIEW&) const = default;
};
struct VALTAN_STAGE_VIEW {
    std::string strStageId, strActionId;
    uint32_t iDurationMs = 1000;
    std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;
    std::vector<std::string> ProductCues, SoundCues, ColliderCues, ShakeCues;
    bool operator==(const VALTAN_STAGE_VIEW&) const = default;
};
struct VALTAN_PATTERN_VIEW {
    std::string strPatternId = "fixture";
    bool bManualServerAudition = true;
    std::vector<VALTAN_STAGE_VIEW> Stages;
    bool operator==(const VALTAN_PATTERN_VIEW&) const = default;
};
struct CAnimation_Tool {
    bool blocked = false;
    bool Is_ValtanCompositionPatternTransactionActive() const { return blocked; }
};
struct CBalanceTool {
    VALTAN_PATTERN_VIEW pattern;
    bool blocked = false, publishRunning = false, dirty = false;
    unsigned generation = 0, moveCalls = 0, failMoveCall = 0, transactions = 0;
    bool Is_ValtanSaveJobBlockingAuthoring() const { return blocked; }
    bool Is_ServerRuntimeSetPublishRunning() const { return publishRunning; }
    bool Is_ValtanDraftDirty() const { return dirty; }
    bool Get_ValtanPatternDraft(const std::string& id, VALTAN_PATTERN_VIEW& output, std::string& status) const {
        if (id != pattern.strPatternId) { status = "Pattern absent"; return false; }
        output = pattern; return true;
    }
    bool Move_ValtanManualStage(const std::string& patternId, const std::string& id,
        const std::string& anchor, const bool beforeAnchor, std::string& status) {
        ++moveCalls;
        if (moveCalls == failMoveCall) { status = "Injected second-owner failure"; return false; }
        if (patternId != pattern.strPatternId || id == anchor) return false;
        auto source = std::find_if(pattern.Stages.begin(), pattern.Stages.end(), [&](const auto& row) { return row.strStageId == id; });
        if (source == pattern.Stages.end()) return false;
        auto moved = *source;
        pattern.Stages.erase(source);
        auto target = std::find_if(pattern.Stages.begin(), pattern.Stages.end(), [&](const auto& row) { return row.strStageId == anchor; });
        if (target == pattern.Stages.end()) return false;
        if (!beforeAnchor) ++target;
        pattern.Stages.insert(target, moved);
        dirty = true; ++generation; return true;
    }
    bool Apply_ValtanCompositionDraftTransaction(const std::function<bool(std::string&)>& operation, std::string& status) {
        ++transactions;
        auto baseline = pattern; auto oldDirty = dirty; auto oldGeneration = generation;
        if (operation(status)) return true;
        pattern = baseline; dirty = oldDirty; generation = oldGeneration; return false;
    }
};
class CValtanActionWorkbench {
public:
    enum class DETAIL_OWNER { NONE, GAMEPLAY_STAGE, ANIMATION, EFFECT, SOUND };
    struct TIMELINE_SELECTION {
        std::string strPatternId, strStageId, strStableId;
        DETAIL_OWNER eOwner = DETAIL_OWNER::NONE;
        bool operator==(const TIMELINE_SELECTION&) const = default;
    };
    CBalanceTool* m_pBalanceTool = nullptr;
    CAnimation_Tool* m_pAnimationTool = nullptr;
    int m_eAdmission = 1;
    std::vector<TIMELINE_SELECTION> m_TimelineSelection;
    std::string m_strSelectedPatternId = "fixture", m_strSelectedStageId, m_strSelectedStableId, m_strEffectEditIdentity = "old-effect";
    DETAIL_OWNER m_eDetailOwner = DETAIL_OWNER::NONE;
    std::vector<int> m_BossPatternOutcomeOverrides = { 1 };
    unsigned m_iBossPatternRouteGeneration = 3, effectiveInvalidations = 0, timelineInvalidations = 0, previewRefreshes = 0;
    bool m_bAuthoringDraftDirty = false;
    void Invalidate_EffectivePatternCache() { ++effectiveInvalidations; }
    void Invalidate_TimelineCache() { ++timelineInvalidations; }
    void Refresh_PatternLocalPreviewAfterMutation(const VALTAN_PATTERN_VIEW*, std::string&) { ++previewRefreshes; }
    bool_t Reorder_SelectedStages(const VALTAN_PATTERN_VIEW&, const int32_t, std::string&);
};
}
using namespace Client;
bool Can_MutateValtanView(int admission) { return admission != 0; }
// @PRODUCTION_STAGE_REORDER@

unsigned checks = 0;
void check(bool value, const std::string& text) {
    ++checks;
    if (!value) throw std::runtime_error(text);
}
using Owner = CValtanActionWorkbench::DETAIL_OWNER;
using Key = CValtanActionWorkbench::TIMELINE_SELECTION;
VALTAN_PATTERN_VIEW fixture() {
    VALTAN_PATTERN_VIEW pattern;
    for (const auto id : { "A", "B", "C", "D", "E", "F" }) {
        VALTAN_STAGE_VIEW row;
        row.strStageId = id; row.strActionId = std::string("action-") + id;
        row.iDurationMs = 500 + static_cast<uint32_t>(pattern.Stages.size()) * 733;
        row.ClipOccurrences = { { std::string(id) + "-clip", 43, 411 }, { std::string(id) + "-clip2", 77, 100 } };
        row.ProductCues = { std::string(id) + "-effect" }; row.SoundCues = { std::string(id) + "-sound" };
        row.ColliderCues = { std::string(id) + "-collider" }; row.ShakeCues = { std::string(id) + "-shake" };
        pattern.Stages.push_back(row);
    }
    return pattern;
}
std::string order(const VALTAN_PATTERN_VIEW& pattern) {
    std::string text; for (const auto& row : pattern.Stages) text += row.strStageId; return text;
}
Key stage(const std::string& id) { return { "fixture", id, id, Owner::GAMEPLAY_STAGE }; }
Key animation(const std::string& id) { return { "fixture", id, id + "-clip", Owner::ANIMATION }; }
struct TestCase {
    CBalanceTool balance;
    CAnimation_Tool animationTool;
    CValtanActionWorkbench ui;
    TestCase() { balance.pattern = fixture(); ui.m_pBalanceTool = &balance; ui.m_pAnimationTool = &animationTool; }
    bool move(int direction, std::string& status) { const auto snapshot = balance.pattern; return ui.Reorder_SelectedStages(snapshot, direction, status); }
};
int main() {
    try {
        std::string status;
        TestCase mixed;
        const auto baseline = mixed.balance.pattern;
        mixed.ui.m_TimelineSelection = { stage("A"), animation("A"), stage("B"), animation("B"), stage("C"), animation("C") };
        const auto mixedSelection = mixed.ui.m_TimelineSelection;
        check(mixed.move(1, status), status);
        check(order(mixed.balance.pattern) == "DABCEF", "Three selected Stage/Animation pairs move right by one neighbour");
        check(mixed.balance.moveCalls == 3, "Child selection does not move its owning Stage twice");
        check(mixed.balance.transactions == 1, "One transaction owns the three-Stage move");
        check(mixed.ui.m_TimelineSelection == mixedSelection, "Stable mixed selection survives move");
        for (const auto& row : baseline.Stages) {
            const auto found = std::find_if(mixed.balance.pattern.Stages.begin(), mixed.balance.pattern.Stages.end(), [&](const auto& item) { return item.strStageId == row.strStageId; });
            check(found != mixed.balance.pattern.Stages.end() && *found == row, "Every moved and neighbouring Stage retains clips, cues, durations and IDs");
        }
        check(mixed.move(-1, status) && mixed.balance.pattern == baseline, "Earlier reverses the adjacent move exactly");
        check(mixed.ui.previewRefreshes == 2, "Preview refreshes once for each committed move");
        check(mixed.ui.m_bAuthoringDraftDirty, "Success marks authoring dirty");

        TestCase clips;
        clips.ui.m_TimelineSelection = { animation("B"), animation("C"), { "fixture", "B", "B-clip2", Owner::ANIMATION } };
        check(clips.move(1, status) && order(clips.balance.pattern) == "ADBCEF", "Animation-only selection moves deduplicated owning Stages");
        check(clips.balance.moveCalls == 2, "Multiple clips in one Stage count once");

        TestCase sparse;
        sparse.ui.m_TimelineSelection = { stage("B"), animation("D") };
        check(sparse.move(1, status) && order(sparse.balance.pattern) == "ACBEDF", "Noncontiguous runs each cross one neighbour");
        check(sparse.move(-1, status) && order(sparse.balance.pattern) == "ABCDEF", "Noncontiguous runs retain their separation after reverse");

        for (const auto& edge : std::vector<std::pair<Key, int>> { { stage("A"), -1 }, { animation("F"), 1 } }) {
            TestCase value; value.ui.m_TimelineSelection = { edge.first };
            check(!value.move(edge.second, status) && value.balance.pattern == fixture(), "Edge operation preserves all Stage contents");
            check(!value.balance.dirty && value.balance.generation == 0 && value.ui.previewRefreshes == 0, "Edge does not dirty or refresh");
        }
        TestCase all;
        for (const auto& row : all.balance.pattern.Stages) all.ui.m_TimelineSelection.push_back(stage(row.strStageId));
        check(!all.move(1, status) && order(all.balance.pattern) == "ABCDEF", "All-Stage selection has no later neighbour");
        TestCase zero; zero.ui.m_TimelineSelection = { stage("B") };
        check(!zero.move(0, status) && zero.balance.generation == 0, "Zero direction leaves draft unchanged");

        for (const auto& bad : std::vector<Key> {
            { "fixture", "B", "absent-clip", Owner::ANIMATION },
            { "other-pattern", "B", "B", Owner::GAMEPLAY_STAGE },
            { "fixture", "missing-stage", "missing-stage", Owner::GAMEPLAY_STAGE },
            { "fixture", "B", "B-effect", Owner::EFFECT } }) {
            TestCase value; value.ui.m_TimelineSelection = { stage("C"), bad };
            const auto selected = value.ui.m_TimelineSelection;
            check(!value.move(1, status) && value.balance.pattern == fixture(), "Stale, foreign or unsupported box rejects before partial move");
            check(value.ui.m_TimelineSelection == selected && value.balance.generation == 0, "Rejected selection is preserved");
        }

        TestCase failure;
        failure.ui.m_TimelineSelection = { stage("B"), animation("C"), stage("D") };
        failure.balance.failMoveCall = 2;
        const auto beforeSelection = failure.ui.m_TimelineSelection;
        check(!failure.move(1, status) && failure.balance.moveCalls == 2, "Late owner failure occurs after first accepted move");
        check(failure.balance.pattern == fixture() && failure.balance.generation == 0 && !failure.balance.dirty, "Late failure rolls back complete draft and generation");
        check(failure.ui.m_TimelineSelection == beforeSelection && failure.ui.previewRefreshes == 0 && failure.ui.m_iBossPatternRouteGeneration == 3 && failure.ui.m_strEffectEditIdentity == "old-effect", "Late failure preserves selection and preview/cache metadata");

        TestCase focus;
        focus.ui.m_strSelectedStageId = "C"; focus.ui.m_strSelectedStableId = "C-clip"; focus.ui.m_eDetailOwner = Owner::ANIMATION;
        check(focus.move(-1, status) && order(focus.balance.pattern) == "ACBDEF", "Single focused Animation without vector selection moves owner Stage");
        TestCase focusedStage;
        focusedStage.ui.m_strSelectedStageId = "C"; focusedStage.ui.m_strSelectedStableId = "C"; focusedStage.ui.m_eDetailOwner = Owner::GAMEPLAY_STAGE;
        check(focusedStage.move(1, status) && order(focusedStage.balance.pattern) == "ABDCEF", "Single focused Stage without vector selection moves one neighbour");
        TestCase none;
        check(!none.move(1, status) && none.balance.generation == 0, "No selection leaves draft unchanged");
        TestCase save; save.ui.m_TimelineSelection = { stage("B") }; save.balance.blocked = true;
        check(!save.move(1, status) && save.balance.generation == 0, "Active save blocks mutation");
        TestCase published; published.ui.m_TimelineSelection = { stage("B") }; published.balance.publishRunning = true;
        check(!published.move(1, status) && published.balance.generation == 0, "Active publish blocks mutation");
        TestCase readonly; readonly.ui.m_TimelineSelection = { stage("B") }; readonly.ui.m_eAdmission = 0;
        check(!readonly.move(1, status) && readonly.balance.generation == 0, "Read-only admission preserves source");
        TestCase canonical; canonical.ui.m_TimelineSelection = { stage("B") }; canonical.balance.pattern.bManualServerAudition = false;
        check(!canonical.move(1, status) && canonical.balance.generation == 0, "Canonical gameplay topology remains protected");
        std::cout << checks << " stage reorder checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << "After " << checks << " checks: " << error.what() << '\n'; return 1;
    }
}
