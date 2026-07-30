#include "Profiler.h"

#include <algorithm>
#include <limits>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 4096;
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
    m_GpuQueriesAvailable = Create_GpuQueries();
    return S_OK;
}

void CProfiler::Begin_Frame()
{
    if (!m_Enabled.load(std::memory_order_relaxed))
        return;

    m_FrameActive = true;
    ++m_FrameNumber;
    m_CurrentFrame = {};
    m_CurrentFrame.FrameNumber = m_FrameNumber;
    m_CurrentFrame.GpuLatencyFrames = GPU_READ_LATENCY;
    m_CurrentFrame.CpuScopes.clear();
    m_CurrentFrame.CpuScopes.reserve(128);
    m_OpenScopes.clear();
    m_FrameBeginTick = Query_Tick();
    Begin_GpuFrame(m_FrameNumber);
}

void CProfiler::End_Frame()
{
    if (!m_FrameActive)
        return;
    m_FrameActive = false;

    while (!m_OpenScopes.empty())
    {
        const uint32_t sampleIndex = m_OpenScopes.back().SampleIndex;
        m_OpenScopes.pop_back();
        if (sampleIndex < m_CurrentFrame.CpuScopes.size())
            m_CurrentFrame.CpuScopes[sampleIndex].EndTick = Query_Tick();
    }

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.CpuFrameMs =
        static_cast<double>(endTick - m_FrameBeginTick) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);

    for (size_t index = 0; index < m_AtomicCounters.size(); ++index)
    {
        m_CurrentFrame.Counters[index] =
            m_AtomicCounters[index].exchange(0, std::memory_order_relaxed);
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
    if (!m_FrameActive)
        m_OpenScopes.clear();
}

bool CProfiler::Is_Enabled() const noexcept
{
    return m_Enabled.load(std::memory_order_relaxed);
}

void CProfiler::Reset_History()
{
    std::lock_guard lock(m_Mutex);
    m_History.clear();
    m_DroppedCpuScopes = 0;
    m_DroppedGpuFrames = 0;
}

uint32_t CProfiler::Begin_Scope(std::string_view name)
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return UINT32_MAX;
    if (m_CurrentFrame.CpuScopes.size() >= MAX_SCOPES_PER_FRAME)
    {
        ++m_DroppedCpuScopes;
        return UINT32_MAX;
    }

    FProfilerScopeSample sample{};
    sample.NameId = Intern_Name(name);
    sample.Depth = static_cast<uint32_t>(m_OpenScopes.size());
    sample.BeginTick = Query_Tick();
    sample.EndTick = sample.BeginTick;
    const uint32_t token = static_cast<uint32_t>(
        m_CurrentFrame.CpuScopes.size());
    m_CurrentFrame.CpuScopes.push_back(sample);
    m_OpenScopes.push_back({ token, sample.Depth });
    return token;
}

void CProfiler::End_Scope(uint32_t token) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) ||
        !m_FrameActive ||
        token >= m_CurrentFrame.CpuScopes.size())
        return;

    m_CurrentFrame.CpuScopes[token].EndTick = Query_Tick();
    const auto iter = std::find_if(
        m_OpenScopes.rbegin(), m_OpenScopes.rend(),
        [token](const FOpenScope& scope)
        { return scope.SampleIndex == token; });
    if (iter != m_OpenScopes.rend())
        m_OpenScopes.erase(std::next(iter).base());
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
