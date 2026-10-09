#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "ProfilerCaptureIO.h"

#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

NS_BEGIN(Client)

// Pure presentation helpers. CPU QPC and GPU timestamp origins stay separate.
struct FProfilerTimelineView final
{
    double BeginMs = 0.0, SpanMs = 16.6667;
    void Zoom(double factor, double anchor = 0.5)
    {
        if (!std::isfinite(factor) || factor <= 0.0 || !std::isfinite(anchor) ||
            !std::isfinite(BeginMs) || !std::isfinite(SpanMs) || SpanMs <= 0.0) return;
        anchor = (std::clamp)(anchor, 0.0, 1.0);
        const double fixed = BeginMs + SpanMs * anchor;
        SpanMs = (std::clamp)(SpanMs * factor, 0.001, 3600000.0);
        BeginMs = fixed - SpanMs * anchor;
    }
    void Pan(double fraction)
    { if (std::isfinite(fraction) && std::isfinite(BeginMs) && std::isfinite(SpanMs) && SpanMs > 0.0)
        BeginMs = (std::clamp)(BeginMs + fraction * SpanMs, -3600000.0, 3600000.0); }
};

struct FProfilerTimelineClip final
{
    bool Visible = false, LeftClipped = false, RightClipped = false;
    double BeginFraction = 0.0, EndFraction = 0.0;
};

inline double ProfilerRelativeMs(uint64_t tick, uint64_t origin, uint64_t frequency)
{
    if (!frequency) return 0.0;
    return tick >= origin ? double(tick - origin) * 1000.0 / double(frequency) :
        -double(origin - tick) * 1000.0 / double(frequency);
}

inline FProfilerTimelineClip ProfilerClipTimeline(double begin, double end, const FProfilerTimelineView& view)
{
    FProfilerTimelineClip result;
    if (!std::isfinite(begin) || !std::isfinite(end) || end < begin || !std::isfinite(view.BeginMs) ||
        !std::isfinite(view.SpanMs) || view.SpanMs <= 0.0 || end < view.BeginMs || begin > view.BeginMs + view.SpanMs) return result;
    result.Visible = true;
    result.LeftClipped = begin < view.BeginMs; result.RightClipped = end > view.BeginMs + view.SpanMs;
    result.BeginFraction = (std::clamp)((begin - view.BeginMs) / view.SpanMs, 0.0, 1.0);
    result.EndFraction = (std::clamp)((end - view.BeginMs) / view.SpanMs, 0.0, 1.0);
    return result;
}

struct FProfilerTimelineEvent final
{
    uint32_t NameId = 0, ThreadId = 0, Depth = 0;
    bool Gpu = false, SelfKnown = false, CrossFrame = false, PipelineValid = false;
    double BeginMs = 0.0, EndMs = 0.0, SelfMs = 0.0;
    Engine::FProfilerDrawStats Draw{};
    uint64_t PSInvocations = 0, VSInvocations = 0, IAVertices = 0;
};

inline std::vector<FProfilerTimelineEvent> ProfilerBuildTimelineEvents(
    const Engine::FProfilerFrame& frame, uint64_t frequency)
{
    std::vector<FProfilerTimelineEvent> result;
    result.reserve(frame.CpuScopes.size() + frame.GpuScopes.size());
    std::map<uint32_t, std::vector<size_t>> threads;
    std::map<uint32_t, bool> crossFrameThreads;
    bool cpuComplete = frequency != 0 && frame.DroppedCpuScopes == 0;
    if (frequency) for (const auto& scope : frame.CpuScopes)
    {
        if (scope.EndTick < scope.BeginTick) { cpuComplete = false; continue; }
        FProfilerTimelineEvent event;
        event.NameId = scope.NameId; event.ThreadId = scope.ThreadId; event.Depth = scope.Depth;
        event.BeginMs = ProfilerRelativeMs(scope.BeginTick, frame.FrameBeginTick, frequency);
        event.EndMs = ProfilerRelativeMs(scope.EndTick, frame.FrameBeginTick, frequency);
        event.SelfMs = event.EndMs - event.BeginMs;
        event.CrossFrame = scope.BeginTick < frame.FrameBeginTick || scope.EndTick > frame.FrameEndTick;
        // Earlier completed children may be stored in another frame. Inclusive
        // duration remains observed, but this snapshot cannot establish self.
        if (event.CrossFrame) crossFrameThreads[scope.ThreadId] = true;
        threads[scope.ThreadId].push_back(result.size()); result.push_back(event);
    }
    for (auto& [thread, indices] : threads)
    {
        std::stable_sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
        {
            if (result[a].BeginMs != result[b].BeginMs) return result[a].BeginMs < result[b].BeginMs;
            if (result[a].EndMs != result[b].EndMs) return result[a].EndMs > result[b].EndMs;
            return result[a].Depth < result[b].Depth;
        });
        std::vector<size_t> stack;
        for (const auto index : indices)
        {
            auto& event = result[index];
            while (!stack.empty() && result[stack.back()].Depth >= event.Depth)
            {
                if (event.BeginMs < result[stack.back()].EndMs) cpuComplete = false;
                stack.pop_back();
            }
            if (stack.empty()) { if (event.Depth) cpuComplete = false; }
            else
            {
                auto& parent = result[stack.back()];
                if (event.Depth != parent.Depth + 1 || event.BeginMs < parent.BeginMs || event.EndMs > parent.EndMs) cpuComplete = false;
                else parent.SelfMs -= event.EndMs - event.BeginMs;
            }
            stack.push_back(index);
        }
    }
    for (auto& event : result) { event.SelfKnown = cpuComplete && !crossFrameThreads[event.ThreadId]; event.SelfMs = (std::max)(0.0, event.SelfMs); }
    if (frame.GpuValid) for (const auto& scope : frame.GpuScopes)
    {
        FProfilerTimelineEvent event;
        event.NameId = scope.NameId; event.Depth = scope.Depth; event.Gpu = true;
        event.BeginMs = scope.BeginMs; event.EndMs = scope.EndMs; event.SelfMs = scope.SelfMs;
        event.SelfKnown = frame.GpuScopesSupported && frame.DroppedGpuScopes == 0;
        event.Draw = scope.Draw; event.PipelineValid = scope.PipelineValid;
        event.PSInvocations = scope.PSInvocations; event.VSInvocations = scope.VSInvocations; event.IAVertices = scope.IAVertices;
        result.push_back(event);
    }
    return result;
}

inline int ProfilerLatestObservableFrame(const Engine::FProfilerCaptureSnapshot& snapshot, bool preferGpu = true)
{
    if (preferGpu) for (size_t i = snapshot.Frames.size(); i > 0; --i)
        if (snapshot.Frames[i - 1].GpuValid) return static_cast<int>(i - 1);
    return static_cast<int>(snapshot.Frames.size()) - 1;
}

inline double ProfilerMainCoveredMs(const Engine::FProfilerFrame& frame, uint32_t mainThread, uint64_t frequency)
{
    if (!frequency || frame.FrameEndTick < frame.FrameBeginTick) return 0.0;
    std::vector<std::pair<uint64_t, uint64_t>> ranges;
    for (const auto& scope : frame.CpuScopes)
        if (scope.ThreadId == mainThread)
        {
            const auto begin = (std::max)(scope.BeginTick, frame.FrameBeginTick);
            const auto end = (std::min)(scope.EndTick, frame.FrameEndTick);
            if (end > begin) ranges.emplace_back(begin, end);
        }
    std::sort(ranges.begin(), ranges.end());
    uint64_t covered = 0, end = frame.FrameBeginTick;
    for (const auto& range : ranges)
    {
        if (range.second > end) covered += range.second - (std::max)(end, range.first);
        end = (std::max)(end, range.second);
    }
    return double(covered) * 1000.0 / double(frequency);
}

struct FProfilerCandidateEvidence final
{
    bool CpuOverBudget = false, GpuOverBudget = false, GapLarge = false;
    bool AnimationNotSubmitted = false, ManySubmissions = false, MissingCoverage = false;
    uint64_t DrawCalls = 0;
    double TargetMs = 0.0;
};

inline FProfilerCandidateEvidence ProfilerAssessCandidates(const Engine::FProfilerFrame& frame,
    double targetMs, uint64_t drawThreshold)
{
    FProfilerCandidateEvidence result;
    result.TargetMs = std::isfinite(targetMs) && targetMs > 0.0 ? targetMs : 16.6667;
    result.CpuOverBudget = frame.CpuFrameMs > result.TargetMs;
    result.GpuOverBudget = frame.GpuValid && frame.GpuFrameMs > result.TargetMs;
    result.GapLarge = frame.FrameIntervalMs > 0.0 && frame.FrameGapMs > result.TargetMs * 0.25;
    result.AnimationNotSubmitted = frame.Animation.DroppedSamples == 0 &&
        frame.Animation.NotSubmittedUpdatedModels > 0 && frame.Animation.NotSubmittedCpuMs > 0.0;
    result.DrawCalls = frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::DrawCalls)] +
        frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::ImGuiDrawCalls)];
    result.ManySubmissions = result.DrawCalls >= (std::max)(uint64_t(1), drawThreshold);
    result.MissingCoverage = !frame.DetailedCpuScopes || frame.DroppedCpuScopes != 0 || frame.DroppedGpuScopes != 0 ||
        !frame.GpuValid || !frame.GpuScopesSupported || frame.DroppedMeshDraws != 0 || frame.Animation.DroppedSamples != 0;
    return result;
}

enum class EProfilerMemoryMetric { PrivateCommit, WorkingSet, PeakPrivateCommit, PeakWorkingSet,
    SystemCommit, SystemLimit, SystemAvailable, LocalUsage, LocalBudget, NonLocalUsage, NonLocalBudget };
struct FProfilerMemoryChartValue final { bool Available = false; double MiB = 0.0; };
inline FProfilerMemoryChartValue ProfilerMemoryValue(const Engine::FProfilerMemoryStats& sample, EProfilerMemoryMetric metric)
{
    if (!sample.Sampled) return {};
    bool valid = false; uint64_t bytes = 0;
    switch (metric)
    {
    case EProfilerMemoryMetric::PrivateCommit: valid = sample.ProcessValid; bytes = sample.PrivateCommitBytes; break;
    case EProfilerMemoryMetric::WorkingSet: valid = sample.ProcessValid; bytes = sample.WorkingSetBytes; break;
    case EProfilerMemoryMetric::PeakPrivateCommit: valid = sample.ProcessValid; bytes = sample.PeakPrivateCommitBytes; break;
    case EProfilerMemoryMetric::PeakWorkingSet: valid = sample.ProcessValid; bytes = sample.PeakWorkingSetBytes; break;
    case EProfilerMemoryMetric::SystemCommit: valid = sample.SystemValid; bytes = sample.SystemCommitBytes; break;
    case EProfilerMemoryMetric::SystemLimit: valid = sample.SystemValid; bytes = sample.SystemCommitLimitBytes; break;
    case EProfilerMemoryMetric::SystemAvailable: valid = sample.SystemValid; bytes = sample.SystemAvailableBytes; break;
    case EProfilerMemoryMetric::LocalUsage: valid = sample.Local.Valid; bytes = sample.Local.CurrentUsageBytes; break;
    case EProfilerMemoryMetric::LocalBudget: valid = sample.Local.Valid; bytes = sample.Local.BudgetBytes; break;
    case EProfilerMemoryMetric::NonLocalUsage: valid = sample.NonLocal.Valid; bytes = sample.NonLocal.CurrentUsageBytes; break;
    case EProfilerMemoryMetric::NonLocalBudget: valid = sample.NonLocal.Valid; bytes = sample.NonLocal.BudgetBytes; break;
    }
    return {valid, valid ? double(bytes) / (1024.0 * 1024.0) : 0.0};
}

/* Profiler panel presented by Debug F1 and the common F7 route. It only reads Engine::CProfiler aggregates and never
   owns timing data: the Engine profiler stays the single owner of scopes,
   counters and GPU queries. */
class CProfilerTool final
{
public:
    explicit CProfilerTool(ID3D11Device* pDevice = nullptr);
    void Begin_Capture(Engine::CProfiler& Profiler);
	void Open() { m_bOpen = true; }
	[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
	void Render(Engine::CProfiler* pProfiler);
	void Request_Save(Engine::CProfiler& Profiler);
	void Update_SaveState();
	[[nodiscard]] bool_t Is_Saving() const noexcept { return m_Exporter.IsSaving(); }
	[[nodiscard]] const std::string& Get_CaptureStatus() const noexcept { return m_strCaptureStatus; }

private:
	void Refresh(Engine::CProfiler& Profiler);
    size_t Save_FrameWindow() const noexcept;
	const char_t* Scope_Name(uint32_t iNameId) const;
	std::string Thread_Label(uint32_t iThreadId) const;
	void Render_Bottlenecks(bool_t bImGuiOnly = false);
	void Render_ImGui();
	void Rebuild_CpuRows(bool_t bImGuiOnly);
	void Render_Gpu();
	void Render_FrameOverview();
    void Render_Culling();
	void Render_LongOperations();
	void Render_Counters() const;
	bool_t Refresh_CaptureFiles();
	void Render_CaptureFiles();
    FProfilerCaptureContext Sample_Context() const;
    void Render_FrameChanges(Engine::CProfiler& Profiler);
    void Render_Comparison(Engine::CProfiler& Profiler);
    void Render_Timeline(Engine::CProfiler& Profiler);
    void Render_TimelineAxis(bool Gpu, const Engine::FProfilerFrame& Frame);
    void Render_Candidates(Engine::CProfiler& Profiler);
    bool Refresh_RecentFrames(Engine::CProfiler& Profiler, bool Force = false);
    void Freeze_Timeline();
    const Engine::FProfilerCaptureSnapshot& Timeline_Snapshot() const;
    void Render_Memory(Engine::CProfiler& Profiler);

private:
	bool_t m_bOpen = true;
    FProfilerCaptureContext m_CaptureContext;
    bool_t m_bSaveWindowOnly = false;
    int32_t m_iSaveFrameInput = 120;
    std::string m_strSavingCoverage;
	bool_t m_bShowUnobserved = true;
	bool_t m_bCatalogRegistered = false;
	int32_t m_iWindowFrameInput = 120;
	float m_fRefreshIntervalSeconds = 0.5f;
	double m_fLastRefreshTime = -1.0;
	uint32_t m_iMainThreadId = 0u;
	size_t m_iHistoryFrames = 0u;
    Engine::FProfilerCaptureWindow m_SaveWindowCoverage{};
	double m_fWindowCpuAvgMs = 0.0;
	double m_fWindowCpuMaxMs = 0.0;
	double m_fWindowGpuAvgMs = 0.0;
	double m_fWindowGpuMaxMs = 0.0;
	size_t m_iWindowFrames = 0u;
	Engine::FProfilerLiveStats m_Live{};
	Engine::FProfilerFrame m_LatestFrame{};
	double m_fUnattributedCpuMs = 0.0;
	bool_t m_bCpuSortSelf = true;
	bool_t m_bMainThreadOnly = true;
	bool_t m_bGpuSortSelf = true;
	bool_t m_bLiveValid = false;
	std::vector<std::string> m_ScopeNames;
	std::vector<Engine::FProfilerScopeAggregate> m_Aggregates;
	struct FVisibleCpuRow final
	{
		const char* Name = nullptr;
		const Engine::FProfilerScopeAggregate* Aggregate = nullptr;
	};
	std::vector<FVisibleCpuRow> m_VisibleCpuRows;
	std::string m_strCpuRowFilter;
	bool_t m_bCpuRowsDirty = true;
	bool_t m_bCpuRowsImGuiOnly = false;
	bool_t m_bCpuRowsShowUnobserved = true;
	std::vector<Engine::FProfilerGpuScopeAggregate> m_GpuAggregates;
	size_t m_iGpuValidFrames = 0u;
	size_t m_iGpuFrameValidFrames = 0u;
	size_t m_iGpuPartialFrames = 0u;
	std::vector<Engine::FProfilerLongOperation> m_LongOperations;
	std::array<char_t, 96> m_Filter = {};
	std::array<char_t, 241> m_CaptureName = {};
	std::vector<FProfilerCaptureFile> m_CaptureFiles;
	std::string m_strSelectedCaptureId;
	std::string m_strCaptureFilesStatus;
	bool_t m_bCaptureFilesLoaded = false;
	std::string m_strCaptureStatus;
	CProfilerCaptureExporter m_Exporter;
    FProfilerComparisonCapture m_ComparisonFrames;
    std::array<std::map<std::string, FProfilerComparisonMean>, 2> m_FrameComparisonMeans;
    std::array<FProfilerComparisonCapture, 2> m_Baselines;
    std::array<std::map<std::string, FProfilerComparisonMean>, 2> m_BaselineMeans;
    std::array<std::array<char, 241>, 2> m_BaselineLabels{};
    std::string m_ComparisonStatus;
    bool m_bComparisonRefresh = true, m_bFollowComparisonFrames = true;
    int m_iComparisonFrame = 0;
    int m_iComparisonTabRequest = 0;
    int m_iComparedFrame = -1;
    Engine::FProfilerCaptureSnapshot m_RecentFrameSnapshot, m_TimelineSnapshot;
    std::vector<FProfilerTimelineEvent> m_TimelineEvents;
    FProfilerTimelineView m_CpuTimelineView, m_GpuTimelineView;
    bool m_bTimelineFollow = true, m_bTimelineRefresh = true;
    bool m_bTimelinePreferGpu = true;
    int m_iTimelineFrame = -1, m_iTimelineBuiltFrame = -1, m_iTimelineSelectedEvent = -1;
    float m_fCandidateTargetFps = 60.f;
    int m_iCandidateDrawThreshold = 1000;
    std::vector<Engine::FProfilerMemoryStats> m_MemorySamples;
    double m_fLastMemoryRefresh = -1.0;
};

NS_END
