#pragma once

#include "Engine_Defines.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)

enum class EProfilerCounter : uint16_t
{
    DrawCalls,
    InstancedDrawCalls,
    Instances,
    Indices,
    RenderSubmissionsPriority,
    RenderSubmissionsShadow,
    RenderSubmissionsNonBlend,
    RenderSubmissionsBlend,
    MapPlacements,
    MapVisibleInstances,
    MapBatchCount,
    MapFallbackObjects,
    TextureRequests,
    TexturePathHits,
    TextureContentHits,
    TextureUniqueSrvs,
    TextureEstimatedGpuBytes,
    NavigationQueries,
    NavigationExpandedNodes,
    NavigationQueryMicroseconds,
    NavigationPathCells,
    Count
};

struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    /* Win32 thread id of the thread that ran the scope. Worker scopes (Loader,
       Effect preparation) are attributed to the frame in which they ended. */
    uint32_t ThreadId = 0;
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};
    std::vector<FProfilerScopeSample> CpuScopes;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
};

struct FProfilerCaptureSnapshot final
{
    std::vector<std::string> ScopeNames;
    std::vector<FProfilerFrame> Frames;
    uint64_t DroppedCpuScopes = 0;
    uint64_t DroppedGpuFrames = 0;
    uint32_t MainThreadId = 0;
    uint64_t TicksPerSecond = 0;
};

struct FProfilerLiveStats final
{
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};

    uint64_t GpuFrameNumber = 0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
};

/* One completed scope that took at least LONG_OPERATION_THRESHOLD_MS. It is
   kept in a small ring independent of frame history so a long JSON parse on
   the Loader thread stays visible after the frame it ended in scrolled out. */
struct FProfilerLongOperation final
{
    uint64_t Sequence = 0;
    uint64_t FrameNumber = 0;
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    double DurationMs = 0.0;
};

/* Sum over a window of history frames for one (scope name, thread) pair.
   Self time excludes scopes nested inside it on the same thread. */
struct FProfilerScopeAggregate final
{
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double SelfMs = 0.0;
    double MaxMs = 0.0;
};

class ENGINE_DLL CProfiler final
{
public:
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr size_t MAX_HISTORY_FRAMES = 1200;
    static constexpr size_t MAX_LONG_OPERATIONS = 256;
    static constexpr double LONG_OPERATION_THRESHOLD_MS = 8.0;

public:
    CProfiler() = default;
    ~CProfiler() = default;
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    void Begin_Frame();
    void End_Frame();

    void Set_Enabled(bool enabled) noexcept;
    bool Is_Enabled() const noexcept;

    void Reset_History();

    /* Thread-safe. Nesting is tracked per thread, so a scope may begin and end
       on any thread and outside the main-thread frame boundaries. The token is
       only meaningful on the thread that began the scope. */
    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot() const;
    bool Get_LiveStats(FProfilerLiveStats& outStats) const;

    uint32_t Get_MainThreadId() const noexcept { return m_MainThreadId; }
    double Ticks_ToMs(uint64_t ticks) const noexcept;
    size_t Get_HistoryFrameCount() const;
    void Get_ScopeNames(std::vector<std::string>& outNames) const;
    /* Aggregates the most recent frameWindow history frames, sorted by
       inclusive time descending. */
    void Get_ScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerScopeAggregate>& outAggregates) const;
    void Get_WindowFrameStats(
        size_t frameWindow,
        double& outCpuAvgMs,
        double& outCpuMaxMs,
        double& outGpuAvgMs,
        double& outGpuMaxMs,
        size_t& outFrames) const;
    void Get_LongOperations(std::vector<FProfilerLongOperation>& outOperations) const;
    void Clear_LongOperations();

private:
    struct FGpuQuerySlot final
    {
        ComPtr<ID3D11Query> Disjoint;
        ComPtr<ID3D11Query> TimestampBegin;
        ComPtr<ID3D11Query> TimestampEnd;
        ComPtr<ID3D11Query> Pipeline;
        uint64_t FrameNumber = 0;
        bool Pending = false;
    };

private:
    uint64_t Query_Tick() const noexcept;
    uint32_t Intern_Name(std::string_view name);
    bool Create_GpuQueries();
    void Begin_GpuFrame(uint64_t frameNumber);
    void End_GpuFrame(uint64_t frameNumber);
    void Resolve_GpuFrames(uint64_t currentFrame);
    void Commit_CurrentFrame();

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    LARGE_INTEGER m_Frequency{};
    std::atomic_bool m_Enabled = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_FrameActive = false;
    uint32_t m_MainThreadId = 0;
    mutable std::mutex m_Mutex;
    /* Completed scopes from every thread since the last End_Frame. */
    std::vector<FProfilerScopeSample> m_PendingScopes;
    std::deque<FProfilerLongOperation> m_LongOperations;
    uint64_t m_LongOperationSequence = 0;
    uint64_t m_SharedFrameNumber = 0;
    std::deque<FProfilerFrame> m_History;
    std::vector<std::string> m_ScopeNames;
    std::unordered_map<std::string, uint32_t> m_ScopeNameLookup;
    uint64_t m_DroppedCpuScopes = 0;
    uint64_t m_DroppedGpuFrames = 0;
};

class ENGINE_DLL CProfilerScope final
{
public:
    CProfilerScope(CProfiler* profiler, std::string_view name)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_Scope(name) : UINT32_MAX)
    {}

    ~CProfilerScope()
    {
        if (m_pProfiler != nullptr && m_Token != UINT32_MAX)
            m_pProfiler->End_Scope(m_Token);
    }

    CProfilerScope(const CProfilerScope&) = delete;
    CProfilerScope& operator=(const CProfilerScope&) = delete;

private:
    CProfiler* m_pProfiler = nullptr;
    uint32_t m_Token = UINT32_MAX;
};

NS_END
