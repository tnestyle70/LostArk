#include "Profiler.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <map>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 4096;
    constexpr size_t MAX_OPEN_SCOPES_PER_THREAD = 64;

    uint64_t AllocateProfilerInstanceId()
    {
        static std::atomic_uint64_t nextId{1};
        const uint64_t id = nextId.fetch_add(1, std::memory_order_relaxed);
        if (id == 0) std::terminate(); // Never reuse a wrapped cache identity.
        return id;
    }

    struct FOpenScope final
    {
        uint32_t NameId;
        uint32_t Depth;
        uint64_t BeginTick;
    };

    /* Each thread owns its own nesting stack. A token is the index into this
       stack on the thread that began the scope.
       Kept trivially constructible (fixed array, no std::vector) on purpose: a thread_local
       with a dynamic initializer runs on every thread the process ever creates -- D3D driver,
       FMOD and PhysX workers included -- and the debug STL vector's heap-allocated container
       proxy then shows up in the CRT exit leak dump for every such thread still alive at
       shutdown. */
    struct FOpenScopeStack final
    {
        FOpenScope Scopes[MAX_OPEN_SCOPES_PER_THREAD];
        uint32_t Count;
    };
    thread_local FOpenScopeStack t_OpenScopes{};
}

CProfiler::CProfiler()
    : m_InstanceId(AllocateProfilerInstanceId())
{
}

HRESULT CProfiler::Initialize(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context)
{
    if (nullptr == device || nullptr == context ||
        !QueryPerformanceFrequency(&m_Frequency) ||
        0 == m_Frequency.QuadPart)
        return E_INVALIDARG;

    m_pDevice = std::move(device);
    m_pContext = std::move(context);
    m_MainThreadId = GetCurrentThreadId();
    m_GpuQueriesAvailable = Create_GpuQueries();
    return S_OK;
}

void CProfiler::Begin_Frame()
{
    // Counts real main-loop frame boundaries even while capture is paused.
    // Query latency must not be synthesized by advancing the capture number.
    ++m_PollFrameNumber;
    if (!m_Enabled.load(std::memory_order_relaxed))
        return;

    m_FrameActive = true;
    ++m_FrameNumber;
    {
        std::lock_guard lock(m_Mutex);
        m_SharedFrameNumber = m_FrameNumber;
    }
    m_CurrentFrame = {};
    m_CurrentFrame.FrameNumber = m_FrameNumber;
    m_CurrentFrame.GpuScopesSupported = m_GpuScopeQueriesAvailable;
    m_ModelAnimationWork.clear();
    m_SubmittedModels.clear();
    m_FrameBeginTick = Query_Tick();
    if (m_PreviousFrameBeginTick != 0 && m_FrameBeginTick >= m_PreviousFrameBeginTick)
        m_CurrentFrame.FrameIntervalMs = Ticks_ToMs(m_FrameBeginTick - m_PreviousFrameBeginTick);
    m_PreviousFrameBeginTick = m_FrameBeginTick;
    Begin_GpuFrame(m_FrameNumber);
}

void CProfiler::End_Frame()
{
    if (!m_FrameActive)
    {
        Resolve_GpuFrames(m_PollFrameNumber);
        return;
    }
    m_FrameActive = false;

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.CpuFrameMs =
        static_cast<double>(endTick - m_FrameBeginTick) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);

    for (const auto& [model, work] : m_ModelAnimationWork)
    {
        FProfilerAnimationStats& stats = m_CurrentFrame.Animation;
        ++stats.UpdatedModels;
        stats.UpdateCalls += work.Calls;
        stats.CpuMs += work.CpuMs;
        if (m_SubmittedModels.find(model) != m_SubmittedModels.end())
            ++stats.SubmittedUpdatedModels;
        else
        {
            ++stats.NotSubmittedUpdatedModels;
            stats.NotSubmittedCpuMs += work.CpuMs;
        }
    }

    for (size_t index = 0; index < m_AtomicCounters.size(); ++index)
    {
        m_CurrentFrame.Counters[index] =
            m_AtomicCounters[index].exchange(0, std::memory_order_relaxed);
    }

    {
        /* Every scope that ended since the previous frame, on any thread,
           belongs to this frame. */
        std::lock_guard lock(m_Mutex);
        m_CurrentFrame.CpuScopes = std::move(m_PendingScopes);
        m_PendingScopes.clear();
        if (m_History.size() < MAX_HISTORY_FRAMES)
            m_PendingScopes.reserve(128);
    }

    End_GpuFrame(m_FrameNumber);
    Commit_CurrentFrame();
    Resolve_GpuFrames(m_PollFrameNumber);
}

void CProfiler::Set_Enabled(bool enabled) noexcept
{
    const bool previous = m_Enabled.exchange(
        enabled, std::memory_order_relaxed);
    if (previous == enabled)
        return;
    m_PreviousFrameBeginTick = 0;

    for (std::atomic_uint64_t& counter : m_AtomicCounters)
        counter.store(0, std::memory_order_relaxed);
    if (!enabled)
    {
        std::lock_guard lock(m_Mutex);
        m_PendingScopes.clear();
    }
}

bool CProfiler::Is_Enabled() const noexcept
{
    return m_Enabled.load(std::memory_order_relaxed);
}

void CProfiler::Reset_History()
{
    std::lock_guard lock(m_Mutex);
    m_History.clear();
    m_PendingScopes.clear();
    m_LongOperations.clear();
    m_DroppedCpuScopes = 0;
    m_DroppedGpuFrames = 0;
    m_DroppedGpuScopes = 0;
    m_DroppedModelAnimationSamples = 0;
}

uint32_t CProfiler::Begin_Scope(std::string_view name)
{
    if (!m_Enabled.load(std::memory_order_relaxed))
        return UINT32_MAX;
    FOpenScopeStack& openScopes = t_OpenScopes;
    if (openScopes.Count >= MAX_OPEN_SCOPES_PER_THREAD)
    {
        std::lock_guard lock(m_Mutex);
        ++m_DroppedCpuScopes;
        return UINT32_MAX;
    }

    FOpenScope open{};
    open.NameId = Intern_Name(name);
    open.Depth = openScopes.Count;
    open.BeginTick = Query_Tick();
    openScopes.Scopes[openScopes.Count] = open;
    return openScopes.Count++;
}

void CProfiler::End_Scope(uint32_t token) noexcept
{
    FOpenScopeStack& openScopes = t_OpenScopes;
    if (token >= openScopes.Count)
        return;

    const uint64_t endTick = Query_Tick();
    const FOpenScope open = openScopes.Scopes[token];
    /* Unwinding to the token also closes any inner scope whose End_Scope was
       skipped, so a mismatched pair cannot corrupt later depths. */
    openScopes.Count = token;
    if (!m_Enabled.load(std::memory_order_relaxed))
        return;

    FProfilerScopeSample sample{};
    sample.NameId = open.NameId;
    sample.Depth = open.Depth;
    sample.ThreadId = GetCurrentThreadId();
    sample.BeginTick = open.BeginTick;
    sample.EndTick = endTick;
    const double durationMs = Ticks_ToMs(endTick - open.BeginTick);

    std::lock_guard lock(m_Mutex);
    if (m_PendingScopes.size() < MAX_SCOPES_PER_FRAME)
        m_PendingScopes.push_back(sample);
    else
        ++m_DroppedCpuScopes;
    if (durationMs >= LONG_OPERATION_THRESHOLD_MS)
    {
        FProfilerLongOperation operation{};
        operation.Sequence = ++m_LongOperationSequence;
        operation.FrameNumber = m_SharedFrameNumber;
        operation.NameId = sample.NameId;
        operation.ThreadId = sample.ThreadId;
        operation.DurationMs = durationMs;
        m_LongOperations.push_back(operation);
        while (m_LongOperations.size() > MAX_LONG_OPERATIONS)
            m_LongOperations.pop_front();
    }
}

uint32_t CProfiler::Begin_GpuScope(std::string_view name, bool collectPipeline)
{
    if (GetCurrentThreadId() != m_MainThreadId ||
        !m_Enabled.load(std::memory_order_relaxed) ||
        !m_FrameActive || !m_GpuScopeQueriesAvailable ||
        m_ActiveGpuSlot == UINT32_MAX)
        return UINT32_MAX;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    if (slot.ScopeCount >= MAX_GPU_SCOPES_PER_FRAME)
    {
        ++m_CurrentFrame.DroppedGpuScopes;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedGpuScopes;
        return UINT32_MAX;
    }
    const uint32_t index = slot.ScopeCount++;
    FGpuScopeQuery& scope = slot.Scopes[index];
    scope.NameId = Intern_Name(name);
    scope.Depth = slot.OpenScopeCount;
    scope.Ended = false;
    scope.PipelineIndex = UINT32_MAX;
    if (collectPipeline && m_GpuPipelineQueriesAvailable &&
        slot.PipelineScopeCount < MAX_GPU_PIPELINE_SCOPES_PER_FRAME)
    {
        scope.PipelineIndex = slot.PipelineScopeCount++;
        m_pContext->Begin(slot.PassPipelines[scope.PipelineIndex].Get());
    }
    if (m_NextGpuScopeToken == UINT32_MAX)
        m_NextGpuScopeToken = 0;
    scope.Token = m_NextGpuScopeToken++;
    slot.OpenScopes[slot.OpenScopeCount++] = index;
    m_pContext->End(scope.Begin.Get());
    return scope.Token;
}

void CProfiler::End_GpuScope(uint32_t token) noexcept
{
    if (GetCurrentThreadId() != m_MainThreadId ||
        token == UINT32_MAX || m_ActiveGpuSlot == UINT32_MAX)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    for (uint32_t position = slot.OpenScopeCount; position > 0; --position)
    {
        const uint32_t index = slot.OpenScopes[position - 1];
        if (slot.Scopes[index].Token != token)
            continue;
        while (slot.OpenScopeCount >= position)
        {
            FGpuScopeQuery& scope = slot.Scopes[slot.OpenScopes[--slot.OpenScopeCount]];
            m_pContext->End(scope.End.Get());
            if (scope.PipelineIndex != UINT32_MAX)
                m_pContext->End(slot.PassPipelines[scope.PipelineIndex].Get());
            scope.Ended = true;
        }
        return;
    }
}

FProfilerModelAnimationToken CProfiler::Begin_ModelAnimation() const noexcept
{
    if (GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Enabled.load(std::memory_order_relaxed))
        return {};
    return {Query_Tick(), m_FrameNumber};
}

void CProfiler::End_ModelAnimation(const void* model, FProfilerModelAnimationToken token)
{
    if (!model || token.BeginTick == 0 || GetCurrentThreadId() != m_MainThreadId)
        return;
    if (!m_FrameActive || token.FrameNumber != m_FrameNumber)
    {
        std::lock_guard lock(m_Mutex);
        ++m_DroppedModelAnimationSamples;
        return;
    }
    const uint64_t end = Query_Tick();
    if (end < token.BeginTick)
        return;
    auto found = m_ModelAnimationWork.find(model);
    if (found == m_ModelAnimationWork.end())
    {
        if (m_ModelAnimationWork.size() >= MAX_ANIMATION_MODELS_PER_FRAME)
        {
            ++m_CurrentFrame.Animation.DroppedSamples;
            std::lock_guard lock(m_Mutex);
            ++m_DroppedModelAnimationSamples;
            return;
        }
        found = m_ModelAnimationWork.emplace(model, FModelAnimationWork{}).first;
    }
    FModelAnimationWork& work = found->second;
    ++work.Calls;
    work.CpuMs += Ticks_ToMs(end - token.BeginTick);
}

void CProfiler::Record_ModelSubmitted(const void* model)
{
    if (!model || GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Enabled.load(std::memory_order_relaxed) ||
        m_SubmittedModels.find(model) != m_SubmittedModels.end())
        return;
    if (m_SubmittedModels.size() >= MAX_ANIMATION_MODELS_PER_FRAME)
    {
        ++m_CurrentFrame.Animation.DroppedSamples;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedModelAnimationSamples;
        return;
    }
    m_SubmittedModels.insert(model);
}

void CProfiler::Add_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].fetch_add(value, std::memory_order_relaxed);
}

void CProfiler::Set_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].store(value, std::memory_order_relaxed);
}

FProfilerCaptureSnapshot CProfiler::Snapshot() const
{
    std::lock_guard lock(m_Mutex);
    FProfilerCaptureSnapshot snapshot{};
    snapshot.ScopeNames = m_ScopeNames;
    snapshot.Frames.assign(m_History.begin(), m_History.end());
    snapshot.DroppedCpuScopes = m_DroppedCpuScopes;
    snapshot.DroppedGpuFrames = m_DroppedGpuFrames;
    snapshot.DroppedGpuScopes = m_DroppedGpuScopes;
    snapshot.DroppedModelAnimationSamples = m_DroppedModelAnimationSamples;
    snapshot.GpuQueriesSupported = m_GpuQueriesAvailable;
    snapshot.GpuScopesSupported = m_GpuScopeQueriesAvailable;
    snapshot.MainThreadId = m_MainThreadId;
    snapshot.TicksPerSecond = static_cast<uint64_t>(m_Frequency.QuadPart);
    return snapshot;
}

bool CProfiler::Get_LiveStats(FProfilerLiveStats& outStats) const
{
    std::lock_guard lock(m_Mutex);
    outStats = {};
    outStats.TotalDroppedCpuScopes = m_DroppedCpuScopes;
    outStats.TotalDroppedGpuFrames = m_DroppedGpuFrames;
    outStats.TotalDroppedGpuScopes = m_DroppedGpuScopes;
    outStats.TotalDroppedModelAnimationSamples = m_DroppedModelAnimationSamples;
    if (m_History.empty())
        return false;

    const FProfilerFrame& latest = m_History.back();
    outStats.FrameNumber = latest.FrameNumber;
    outStats.CpuFrameMs = latest.CpuFrameMs;
    outStats.FrameIntervalMs = latest.FrameIntervalMs;
    outStats.Animation = latest.Animation;
    outStats.Counters = latest.Counters;
    outStats.LatestFrameGpuStatus = latest.GpuStatus;
    outStats.GpuScopesSupported = m_GpuScopeQueriesAvailable;

    const auto gpuFrame = std::find_if(
        m_History.rbegin(), m_History.rend(),
        [](const FProfilerFrame& frame)
        { return frame.GpuValid; });
    if (gpuFrame != m_History.rend())
    {
        outStats.GpuFrameNumber = gpuFrame->FrameNumber;
        outStats.GpuFrameMs = gpuFrame->GpuFrameMs;
        outStats.GpuValid = true;
        outStats.GpuLatencyFrames = gpuFrame->GpuLatencyFrames;
        outStats.Pipeline = gpuFrame->Pipeline;
        outStats.GpuScopes = gpuFrame->GpuScopes;
        outStats.DroppedGpuScopes = gpuFrame->DroppedGpuScopes;
    }

    return true;
}

double CProfiler::Ticks_ToMs(uint64_t ticks) const noexcept
{
    if (0 == m_Frequency.QuadPart)
        return 0.0;
    return static_cast<double>(ticks) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);
}

size_t CProfiler::Get_HistoryFrameCount() const
{
    std::lock_guard lock(m_Mutex);
    return m_History.size();
}

void CProfiler::Get_ScopeNames(std::vector<std::string>& outNames) const
{
    std::lock_guard lock(m_Mutex);
    outNames = m_ScopeNames;
}

void CProfiler::Get_ScopeAggregates(
    size_t frameWindow,
    std::vector<FProfilerScopeAggregate>& outAggregates) const
{
    outAggregates.clear();
    std::lock_guard lock(m_Mutex);
    if (m_History.empty() || 0 == frameWindow)
        return;

    // End_Scope appends in completion order on each thread. Reduce completed
    // children when their parent arrives instead of sorting every raw frame
    // again on every panel refresh. Scope IDs are dense, interned indices.
    struct FThreadReduction final
    {
        uint32_t ThreadId = 0;
        std::vector<FProfilerScopeAggregate> ByName;
        std::vector<size_t> Completed;
        size_t CompletedCount = 0;
    };
    std::vector<FThreadReduction> threads;
    threads.reserve(4);
    size_t lastThread = 0;
    const auto threadFor = [&](uint32_t id) -> FThreadReduction&
    {
        if (lastThread < threads.size() && threads[lastThread].ThreadId == id)
            return threads[lastThread];
        for (size_t i = 0; i < threads.size(); ++i)
            if (threads[i].ThreadId == id)
            {
                lastThread = i;
                return threads[i];
            }
        lastThread = threads.size();
        threads.emplace_back();
        FThreadReduction& added = threads.back();
        added.ThreadId = id;
        added.ByName.resize(m_ScopeNames.size());
        return added;
    };

    const size_t frameCount = (std::min)(frameWindow, m_History.size());
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        for (FThreadReduction& thread : threads) thread.CompletedCount = 0;
        const auto& frameScopes = m_History[frameIndex].CpuScopes;
        const size_t scopeCount = frameScopes.size();
        const FProfilerScopeSample* scopes = frameScopes.data();
        for (size_t index = 0; index < scopeCount; ++index)
        {
            const FProfilerScopeSample& sample = scopes[index];
            if (sample.NameId >= m_ScopeNames.size()) continue;
            FThreadReduction& thread = threadFor(sample.ThreadId);
            // There is at most one push per input sample. Allocate the bounded
            // stack once and avoid Debug STL container churn for every scope.
            if (thread.Completed.size() < scopeCount) thread.Completed.resize(scopeCount);
            size_t* completed = thread.Completed.data();
            const double inclusiveMs = Ticks_ToMs(sample.EndTick >= sample.BeginTick ?
                sample.EndTick - sample.BeginTick : 0);
            double selfMs = inclusiveMs;
            while (thread.CompletedCount != 0)
            {
                const FProfilerScopeSample& child = scopes[completed[thread.CompletedCount - 1]];
                if (child.Depth <= sample.Depth) break;
                // An orphan from an earlier interval is not this scope's child.
                if (child.BeginTick >= sample.BeginTick && child.EndTick <= sample.EndTick)
                    selfMs -= Ticks_ToMs(child.EndTick >= child.BeginTick ?
                        child.EndTick - child.BeginTick : 0);
                --thread.CompletedCount;
            }
            completed[thread.CompletedCount++] = index;

            FProfilerScopeAggregate& aggregate = thread.ByName.data()[sample.NameId];
            aggregate.NameId = sample.NameId;
            aggregate.ThreadId = sample.ThreadId;
            ++aggregate.Calls;
            aggregate.InclusiveMs += inclusiveMs;
            aggregate.SelfMs += (std::max)(0.0, selfMs);
            aggregate.MaxMs = (std::max)(aggregate.MaxMs, inclusiveMs);
        }
    }
    for (const FThreadReduction& thread : threads)
        for (const FProfilerScopeAggregate& aggregate : thread.ByName)
            if (aggregate.Calls != 0) outAggregates.push_back(aggregate);
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerScopeAggregate& left, const FProfilerScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

void CProfiler::Get_GpuScopeAggregates(
    size_t frameWindow,
    std::vector<FProfilerGpuScopeAggregate>& outAggregates,
    size_t& outValidFrames,
    size_t& outPartialFrames) const
{
    std::lock_guard lock(m_Mutex);
    outAggregates.clear();
    outValidFrames = 0;
    outPartialFrames = 0;
    const size_t count = std::min(frameWindow, m_History.size());
    struct FAccumulated final
    {
        FProfilerGpuScopeAggregate Aggregate;
        std::vector<double> FrameTimes;
    };
    std::map<uint32_t, FAccumulated> accumulated;
    for (size_t offset = m_History.size() - count; offset < m_History.size(); ++offset)
    {
        const FProfilerFrame& frame = m_History[offset];
        if (!frame.GpuValid || !frame.GpuScopesSupported)
            continue;
        if (frame.DroppedGpuScopes != 0)
        {
            ++outPartialFrames;
            continue;
        }
        const size_t frameIndex = outValidFrames++;
        for (const FProfilerGpuScopeSample& sample : frame.GpuScopes)
        {
            FAccumulated& value = accumulated[sample.NameId];
            value.Aggregate.NameId = sample.NameId;
            ++value.Aggregate.Calls;
            value.Aggregate.InclusiveMs += sample.DurationMs;
            if (sample.PipelineValid)
            {
                ++value.Aggregate.PipelineSamples;
                value.Aggregate.PSInvocations += sample.PSInvocations;
                value.Aggregate.VSInvocations += sample.VSInvocations;
            }
            value.FrameTimes.resize(frameIndex + 1, 0.0);
            value.FrameTimes[frameIndex] += sample.DurationMs;
        }
    }
    for (auto& [name, value] : accumulated)
    {
        value.FrameTimes.resize(outValidFrames, 0.0);
        std::sort(value.FrameTimes.begin(), value.FrameTimes.end());
        value.Aggregate.MaxFrameMs = value.FrameTimes.back();
        const size_t percentile = static_cast<size_t>(
            std::ceil(static_cast<double>(outValidFrames) * 0.95)) - 1;
        value.Aggregate.P95FrameMs = value.FrameTimes[percentile];
        outAggregates.push_back(value.Aggregate);
    }
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerGpuScopeAggregate& left, const FProfilerGpuScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

void CProfiler::Get_WindowFrameStats(
    size_t frameWindow,
    double& outCpuAvgMs,
    double& outCpuMaxMs,
    double& outGpuAvgMs,
    double& outGpuMaxMs,
    size_t& outFrames,
    size_t* outGpuValidFrames) const
{
    outCpuAvgMs = 0.0;
    outCpuMaxMs = 0.0;
    outGpuAvgMs = 0.0;
    outGpuMaxMs = 0.0;
    outFrames = 0;
    if (outGpuValidFrames)
        *outGpuValidFrames = 0;
    std::lock_guard lock(m_Mutex);
    if (m_History.empty() || 0 == frameWindow)
        return;
    const size_t frameCount = (std::min)(frameWindow, m_History.size());
    size_t gpuFrames = 0;
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        const FProfilerFrame& frame = m_History[frameIndex];
        outCpuAvgMs += frame.CpuFrameMs;
        outCpuMaxMs = (std::max)(outCpuMaxMs, frame.CpuFrameMs);
        if (frame.GpuValid)
        {
            outGpuAvgMs += frame.GpuFrameMs;
            outGpuMaxMs = (std::max)(outGpuMaxMs, frame.GpuFrameMs);
            ++gpuFrames;
        }
    }
    outFrames = frameCount;
    if (outGpuValidFrames)
        *outGpuValidFrames = gpuFrames;
    outCpuAvgMs /= static_cast<double>(frameCount);
    if (0 != gpuFrames)
        outGpuAvgMs /= static_cast<double>(gpuFrames);
}

void CProfiler::Get_LongOperations(
    std::vector<FProfilerLongOperation>& outOperations) const
{
    std::lock_guard lock(m_Mutex);
    outOperations.assign(m_LongOperations.begin(), m_LongOperations.end());
}

void CProfiler::Clear_LongOperations()
{
    std::lock_guard lock(m_Mutex);
    m_LongOperations.clear();
}

uint64_t CProfiler::Query_Tick() const noexcept
{
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return static_cast<uint64_t>(value.QuadPart);
}

uint32_t CProfiler::Intern_Name(std::string_view name)
{
    struct FNameHash final
    {
        using is_transparent = void;
        size_t operator()(std::string_view value) const noexcept
        {
            return std::hash<std::string_view>{}(value);
        }
    };
    struct FThreadNameCache final
    {
        uint64_t InstanceId = 0;
        std::unordered_map<std::string, uint32_t, FNameHash, std::equal_to<>> Names;
    };
    // Function-local TLS allocates only on threads that actually register a scope.
    static thread_local FThreadNameCache threadCache;
    if (threadCache.InstanceId != m_InstanceId)
    {
        threadCache.Names.clear();
        threadCache.InstanceId = m_InstanceId;
    }
    const auto cached = threadCache.Names.find(name);
    if (cached != threadCache.Names.end()) return cached->second;

    uint32_t id;
    {
        const std::string key(name);
        std::lock_guard lock(m_Mutex);
        const auto found = m_ScopeNameLookup.find(key);
        if (found != m_ScopeNameLookup.end()) id = found->second;
        else
        {
            id = static_cast<uint32_t>(m_ScopeNames.size());
            m_ScopeNames.push_back(key);
            m_ScopeNameLookup.emplace(m_ScopeNames.back(), id);
        }
    }
    // Dynamic names cannot grow one thread's cache without bound. Global IDs
    // survive this eviction and Reset_History, so a later miss remains stable.
    if (threadCache.Names.size() >= 512u) threadCache.Names.clear();
    threadCache.Names.emplace(std::string(name), id);
    return id;
}

bool CProfiler::Create_GpuQueries()
{
    D3D11_QUERY_DESC desc{};
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Disjoint)))
            return false;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampBegin)) ||
            FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampEnd)))
            return false;
        desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Pipeline)))
            return false;
    }
    // Pass-query allocation may fail without disabling full-frame GPU timing.
    desc.Query = D3D11_QUERY_TIMESTAMP;
    m_GpuScopeQueriesAvailable = true;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        for (FGpuScopeQuery& scope : slot.Scopes)
        {
            if (FAILED(m_pDevice->CreateQuery(&desc, &scope.Begin)) ||
                FAILED(m_pDevice->CreateQuery(&desc, &scope.End)))
            {
                m_GpuScopeQueriesAvailable = false;
                break;
            }
        }
        if (!m_GpuScopeQueriesAvailable)
            break;
    }
    if (!m_GpuScopeQueriesAvailable)
    {
        for (FGpuQuerySlot& slot : m_GpuSlots)
            for (FGpuScopeQuery& scope : slot.Scopes)
            {
                scope.Begin.Reset();
                scope.End.Reset();
            }
    }
    // Only the selected leaf passes need pipeline counts, not all 128 scopes.
    desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
    m_GpuPipelineQueriesAvailable = m_GpuScopeQueriesAvailable;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!m_GpuPipelineQueriesAvailable) break;
        for (auto& query : slot.PassPipelines)
            if (FAILED(m_pDevice->CreateQuery(&desc, &query)))
            { m_GpuPipelineQueriesAvailable = false; break; }
    }
    if (!m_GpuPipelineQueriesAvailable)
        for (FGpuQuerySlot& slot : m_GpuSlots)
            for (auto& query : slot.PassPipelines) query.Reset();
    return true;
}

void CProfiler::Begin_GpuFrame(uint64_t frameNumber)
{
    m_ActiveGpuSlot = UINT32_MAX;
    if (!m_GpuQueriesAvailable)
        return;
    const uint32_t index = static_cast<uint32_t>(frameNumber % GPU_QUERY_RING_SIZE);
    FGpuQuerySlot& slot = m_GpuSlots[index];
    if (slot.Pending)
    {
        m_CurrentFrame.GpuStatus = EProfilerGpuFrameStatus::Dropped;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedGpuFrames;
        return;
    }
    slot.FrameNumber = frameNumber;
    slot.SubmittedPollFrame = m_PollFrameNumber;
    slot.Pending = true;
    slot.FrameEnded = false;
    slot.ScopeCount = 0;
    slot.PipelineScopeCount = 0;
    slot.OpenScopeCount = 0;
    m_ActiveGpuSlot = index;
    m_CurrentFrame.GpuStatus = EProfilerGpuFrameStatus::Pending;
    m_pContext->Begin(slot.Disjoint.Get());
    m_pContext->Begin(slot.Pipeline.Get());
    m_pContext->End(slot.TimestampBegin.Get());
}

void CProfiler::End_GpuFrame(uint64_t frameNumber)
{
    if (m_ActiveGpuSlot == UINT32_MAX)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    if (!slot.Pending || slot.FrameNumber != frameNumber)
        return;
    if (slot.OpenScopeCount != 0)
    {
        const uint32_t dropped = slot.OpenScopeCount;
        // Complete outstanding query commands, but omit truncated intervals.
        while (slot.OpenScopeCount != 0)
        {
            const auto& scope = slot.Scopes[slot.OpenScopes[--slot.OpenScopeCount]];
            m_pContext->End(scope.End.Get());
            if (scope.PipelineIndex != UINT32_MAX)
                m_pContext->End(slot.PassPipelines[scope.PipelineIndex].Get());
        }
        m_CurrentFrame.DroppedGpuScopes += dropped;
        std::lock_guard lock(m_Mutex);
        m_DroppedGpuScopes += dropped;
    }
    m_pContext->End(slot.TimestampEnd.Get());
    m_pContext->End(slot.Pipeline.Get());
    m_pContext->End(slot.Disjoint.Get());
    slot.FrameEnded = true;
    m_ActiveGpuSlot = UINT32_MAX;
}

void CProfiler::Resolve_GpuFrames(uint64_t currentPollFrame)
{
    if (!m_GpuQueriesAvailable || currentPollFrame <= GPU_READ_LATENCY)
        return;
    constexpr uint32_t flags = D3D11_ASYNC_GETDATA_DONOTFLUSH;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!slot.Pending || !slot.FrameEnded ||
            currentPollFrame < slot.SubmittedPollFrame + GPU_READ_LATENCY)
            continue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        uint64_t begin = 0;
        uint64_t end = 0;
        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline{};
        EProfilerGpuFrameStatus status = EProfilerGpuFrameStatus::Valid;
        bool ready = true;
        const auto read = [&](ID3D11Query* query, void* data, UINT size)
        {
            const HRESULT result = m_pContext->GetData(query, data, size, flags);
            if (FAILED(result))
                status = EProfilerGpuFrameStatus::Error;
            else if (result != S_OK)
                ready = false;
        };
        read(slot.Disjoint.Get(), &disjoint, sizeof(disjoint));
        read(slot.TimestampBegin.Get(), &begin, sizeof(begin));
        read(slot.TimestampEnd.Get(), &end, sizeof(end));
        read(slot.Pipeline.Get(), &pipeline, sizeof(pipeline));
        if (status != EProfilerGpuFrameStatus::Error && !ready)
            continue;
        if (status != EProfilerGpuFrameStatus::Error &&
            (disjoint.Disjoint || disjoint.Frequency == 0 || end < begin))
            status = EProfilerGpuFrameStatus::Disjoint;
        std::array<FProfilerGpuScopeSample, MAX_GPU_SCOPES_PER_FRAME> samples{};
        uint32_t sampleCount = 0;
        if (status == EProfilerGpuFrameStatus::Valid)
        {
            for (uint32_t index = 0; index < slot.ScopeCount; ++index)
            {
                const FGpuScopeQuery& scope = slot.Scopes[index];
                if (!scope.Ended)
                    continue;
                uint64_t scopeBegin = 0;
                uint64_t scopeEnd = 0;
                read(scope.Begin.Get(), &scopeBegin, sizeof(scopeBegin));
                read(scope.End.Get(), &scopeEnd, sizeof(scopeEnd));
                if (!ready || status == EProfilerGpuFrameStatus::Error)
                    break;
                if (scopeBegin < begin || scopeEnd < scopeBegin || scopeEnd > end)
                {
                    status = EProfilerGpuFrameStatus::Error;
                    break;
                }
                FProfilerGpuScopeSample& sample = samples[sampleCount++];
                sample.NameId = scope.NameId;
                sample.Depth = scope.Depth;
                const double scale = 1000.0 / static_cast<double>(disjoint.Frequency);
                sample.BeginMs = static_cast<double>(scopeBegin - begin) * scale;
                sample.EndMs = static_cast<double>(scopeEnd - begin) * scale;
                sample.DurationMs = static_cast<double>(scopeEnd - scopeBegin) * scale;
                if (scope.PipelineIndex != UINT32_MAX)
                {
                    D3D11_QUERY_DATA_PIPELINE_STATISTICS passPipeline{};
                    const HRESULT result = m_pContext->GetData(
                        slot.PassPipelines[scope.PipelineIndex].Get(),
                        &passPipeline, sizeof(passPipeline), flags);
                    if (result == S_FALSE) { ready = false; break; }
                    sample.PipelineValid = result == S_OK;
                    if (sample.PipelineValid)
                    {
                        sample.PSInvocations = passPipeline.PSInvocations;
                        sample.VSInvocations = passPipeline.VSInvocations;
                    }
                }
            }
            if (status != EProfilerGpuFrameStatus::Error && !ready)
                continue;
        }
        std::lock_guard lock(m_Mutex);
        const auto frame = std::find_if(m_History.rbegin(), m_History.rend(),
            [&slot](const FProfilerFrame& value)
            { return value.FrameNumber == slot.FrameNumber; });
        if (frame != m_History.rend())
        {
            frame->GpuStatus = status;
            frame->GpuLatencyFrames = static_cast<uint32_t>(currentPollFrame - slot.SubmittedPollFrame);
            frame->GpuValid = status == EProfilerGpuFrameStatus::Valid;
            if (frame->GpuValid)
            {
                frame->Pipeline = pipeline;
                frame->GpuFrameMs = static_cast<double>(end - begin) * 1000.0 /
                    static_cast<double>(disjoint.Frequency);
                frame->GpuScopes.assign(samples.begin(), samples.begin() + sampleCount);
            }
        }
        slot.Pending = false;
        slot.FrameEnded = false;
    }
}

void CProfiler::Commit_CurrentFrame()
{
    std::lock_guard lock(m_Mutex);
    // Preserve worker scopes that completed between End_Frame and this lock.
    // Otherwise recycle the evicted CPU buffer instead of allocating each frame.
    if (m_History.size() >= MAX_HISTORY_FRAMES && m_PendingScopes.empty())
    {
        m_PendingScopes.swap(m_History.front().CpuScopes);
        m_PendingScopes.clear();
    }
    m_History.push_back(std::move(m_CurrentFrame));
    while (m_History.size() > MAX_HISTORY_FRAMES)
        m_History.pop_front();
}
