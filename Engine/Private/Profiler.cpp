#include "Profiler.h"

#include <algorithm>
#include <limits>
#include <map>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 4096;
    constexpr size_t MAX_OPEN_SCOPES_PER_THREAD = 64;

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
    m_CurrentFrame.GpuLatencyFrames = GPU_READ_LATENCY;
    m_FrameBeginTick = Query_Tick();
    Begin_GpuFrame(m_FrameNumber);
}

void CProfiler::End_Frame()
{
    if (!m_FrameActive)
        return;
    m_FrameActive = false;

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.CpuFrameMs =
        static_cast<double>(endTick - m_FrameBeginTick) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);

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
        m_PendingScopes.reserve(128);
    }

    End_GpuFrame(m_FrameNumber);
    Commit_CurrentFrame();
    Resolve_GpuFrames(m_FrameNumber);
}

void CProfiler::Set_Enabled(bool enabled) noexcept
{
    const bool previous = m_Enabled.exchange(
        enabled, std::memory_order_relaxed);
    if (previous == enabled)
        return;

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
    snapshot.MainThreadId = m_MainThreadId;
    snapshot.TicksPerSecond = static_cast<uint64_t>(m_Frequency.QuadPart);
    return snapshot;
}

bool CProfiler::Get_LiveStats(FProfilerLiveStats& outStats) const
{
    std::lock_guard lock(m_Mutex);
    outStats = {};
    if (m_History.empty())
        return false;

    const FProfilerFrame& latest = m_History.back();
    outStats.FrameNumber = latest.FrameNumber;
    outStats.CpuFrameMs = latest.CpuFrameMs;
    outStats.Counters = latest.Counters;

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

    const size_t frameCount = (std::min)(frameWindow, m_History.size());
    std::map<std::pair<uint32_t, uint32_t>, FProfilerScopeAggregate> aggregates;
    std::vector<size_t> order;
    std::vector<size_t> parentStack;
    std::vector<double> selfMs;
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        const FProfilerFrame& frame = m_History[frameIndex];
        const std::vector<FProfilerScopeSample>& scopes = frame.CpuScopes;
        if (scopes.empty())
            continue;

        /* Self time needs the nesting per thread: visit samples in begin
           order and subtract each child from the innermost open parent. */
        order.resize(scopes.size());
        for (size_t index = 0; index < scopes.size(); ++index)
            order[index] = index;
        std::sort(order.begin(), order.end(),
            [&scopes](const size_t left, const size_t right)
            {
                const FProfilerScopeSample& a = scopes[left];
                const FProfilerScopeSample& b = scopes[right];
                if (a.ThreadId != b.ThreadId)
                    return a.ThreadId < b.ThreadId;
                if (a.BeginTick != b.BeginTick)
                    return a.BeginTick < b.BeginTick;
                return a.EndTick > b.EndTick;
            });
        selfMs.assign(scopes.size(), 0.0);
        parentStack.clear();
        uint32_t currentThread = 0;
        for (const size_t index : order)
        {
            const FProfilerScopeSample& sample = scopes[index];
            if (sample.ThreadId != currentThread)
            {
                parentStack.clear();
                currentThread = sample.ThreadId;
            }
            while (!parentStack.empty() &&
                scopes[parentStack.back()].EndTick <= sample.BeginTick)
            {
                parentStack.pop_back();
            }
            const double inclusiveMs = Ticks_ToMs(
                sample.EndTick >= sample.BeginTick ?
                    sample.EndTick - sample.BeginTick : 0);
            selfMs[index] += inclusiveMs;
            if (!parentStack.empty())
                selfMs[parentStack.back()] -= inclusiveMs;
            parentStack.push_back(index);
        }
        for (size_t index = 0; index < scopes.size(); ++index)
        {
            const FProfilerScopeSample& sample = scopes[index];
            FProfilerScopeAggregate& aggregate =
                aggregates[{ sample.NameId, sample.ThreadId }];
            aggregate.NameId = sample.NameId;
            aggregate.ThreadId = sample.ThreadId;
            const double inclusiveMs = Ticks_ToMs(
                sample.EndTick >= sample.BeginTick ?
                    sample.EndTick - sample.BeginTick : 0);
            ++aggregate.Calls;
            aggregate.InclusiveMs += inclusiveMs;
            aggregate.SelfMs += (std::max)(0.0, selfMs[index]);
            aggregate.MaxMs = (std::max)(aggregate.MaxMs, inclusiveMs);
        }
    }

    outAggregates.reserve(aggregates.size());
    for (const auto& [key, aggregate] : aggregates)
        outAggregates.push_back(aggregate);
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerScopeAggregate& left, const FProfilerScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

void CProfiler::Get_WindowFrameStats(
    size_t frameWindow,
    double& outCpuAvgMs,
    double& outCpuMaxMs,
    double& outGpuAvgMs,
    double& outGpuMaxMs,
    size_t& outFrames) const
{
    outCpuAvgMs = 0.0;
    outCpuMaxMs = 0.0;
    outGpuAvgMs = 0.0;
    outGpuMaxMs = 0.0;
    outFrames = 0;
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
    const std::string key(name);
    std::lock_guard lock(m_Mutex);
    const auto found = m_ScopeNameLookup.find(key);
    if (found != m_ScopeNameLookup.end())
        return found->second;
    const uint32_t id = static_cast<uint32_t>(m_ScopeNames.size());
    m_ScopeNames.push_back(key);
    m_ScopeNameLookup.emplace(m_ScopeNames.back(), id);
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
    return true;
}

void CProfiler::Begin_GpuFrame(uint64_t frameNumber)
{
    if (!m_GpuQueriesAvailable)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[frameNumber % GPU_QUERY_RING_SIZE];
    if (slot.Pending)
    {
        ++m_DroppedGpuFrames;
        return;
    }
    slot.FrameNumber = frameNumber;
    slot.Pending = true;
    m_pContext->Begin(slot.Disjoint.Get());
    m_pContext->Begin(slot.Pipeline.Get());
    m_pContext->End(slot.TimestampBegin.Get());
}

void CProfiler::End_GpuFrame(uint64_t frameNumber)
{
    if (!m_GpuQueriesAvailable)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[frameNumber % GPU_QUERY_RING_SIZE];
    if (!slot.Pending || slot.FrameNumber != frameNumber)
        return;
    m_pContext->End(slot.TimestampEnd.Get());
    m_pContext->End(slot.Pipeline.Get());
    m_pContext->End(slot.Disjoint.Get());
}

void CProfiler::Resolve_GpuFrames(uint64_t currentFrame)
{
    if (!m_GpuQueriesAvailable || currentFrame <= GPU_READ_LATENCY)
        return;

    constexpr uint32_t flags = D3D11_ASYNC_GETDATA_DONOTFLUSH;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!slot.Pending ||
            currentFrame < slot.FrameNumber + GPU_READ_LATENCY)
            continue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        uint64_t begin = {};
        uint64_t end = {};
        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline{};
        if (S_OK != m_pContext->GetData(
            slot.Disjoint.Get(), &disjoint, sizeof(disjoint), flags) ||
            S_OK != m_pContext->GetData(
                slot.TimestampBegin.Get(), &begin, sizeof(begin), flags) ||
            S_OK != m_pContext->GetData(
                slot.TimestampEnd.Get(), &end, sizeof(end), flags) ||
            S_OK != m_pContext->GetData(
                slot.Pipeline.Get(), &pipeline, sizeof(pipeline), flags))
            continue;

        std::lock_guard lock(m_Mutex);
        const auto frame = std::find_if(
            m_History.begin(), m_History.end(),
            [&slot](const FProfilerFrame& value)
            { return value.FrameNumber == slot.FrameNumber; });
        if (frame != m_History.end())
        {
            frame->GpuLatencyFrames = static_cast<uint32_t>(
                currentFrame - slot.FrameNumber);
            frame->Pipeline = pipeline;
            if (!disjoint.Disjoint && 0 != disjoint.Frequency && end >= begin)
            {
                frame->GpuFrameMs =
                    static_cast<double>(end - begin) * 1000.0 /
                    static_cast<double>(disjoint.Frequency);
                frame->GpuValid = true;
            }
        }
        slot.Pending = false;
    }
}

void CProfiler::Commit_CurrentFrame()
{
    std::lock_guard lock(m_Mutex);
    m_History.push_back(m_CurrentFrame);
    while (m_History.size() > MAX_HISTORY_FRAMES)
        m_History.pop_front();
}
