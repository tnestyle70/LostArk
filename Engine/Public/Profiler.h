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
#include <unordered_set>
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
    SceneColorCopies,
    SceneColorCopyBytes,
    ImGuiDrawLists,
    ImGuiVertices,
    ImGuiIndices,
    ImGuiDrawCommands,
    ImGuiDrawCalls,
    ImGuiCallbacks,
    ImGuiRenderWindows,
    ImGuiActiveWindows,
    ImGuiPlatformViewports,
    ImGuiVertexUploadBytes,
    ImGuiIndexUploadBytes,
    ImGuiConstantUploadBytes,
    ImGuiTextureUploadBytes,
    ImGuiBufferGrowths,
    ImGuiTextureCreates,
    ImGuiTextureUpdates,
    ImGuiDeviceObjectBuilds,
    ImGuiBufferMapFailures,
    PickingReadbacks,
    PickingReadbackBytes,
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

/* GPU samples belong to their original submitted frame. Pending is not a
   zero-duration result. Failed/unsupported GPU collection keeps CPU data. */
enum class EProfilerGpuFrameStatus : uint8_t
{
    Unsupported,
    Pending,
    Valid,
    Disjoint,
    Dropped,
    Error
};

/* Inclusive timestamp interval relative to the GPU frame begin. Nested scopes
   overlap; multiple copies with the same NameId remain separate samples. */
struct FProfilerGpuScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    double BeginMs = 0.0;
    double EndMs = 0.0;
    double DurationMs = 0.0;
};

/* Main-thread animation evaluation joined to successful model submissions in
   the same frame. NotSubmitted does not by itself mean outside the frustum. */
struct FProfilerAnimationStats final
{
    uint64_t UpdateCalls = 0;
    uint64_t UpdatedModels = 0;
    uint64_t SubmittedUpdatedModels = 0;
    uint64_t NotSubmittedUpdatedModels = 0;
    uint64_t DroppedSamples = 0;
    double CpuMs = 0.0;
    double NotSubmittedCpuMs = 0.0;
};

struct FProfilerModelAnimationToken final
{
    uint64_t BeginTick = 0;
    uint64_t FrameNumber = 0;
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    FProfilerAnimationStats Animation{};
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    EProfilerGpuFrameStatus GpuStatus = EProfilerGpuFrameStatus::Unsupported;
    bool GpuScopesSupported = false;
    uint32_t DroppedGpuScopes = 0;
    std::vector<FProfilerGpuScopeSample> GpuScopes;
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
    uint64_t DroppedGpuScopes = 0;
    uint64_t DroppedModelAnimationSamples = 0;
    bool GpuQueriesSupported = false;
    bool GpuScopesSupported = false;
    uint32_t MainThreadId = 0;
    uint64_t TicksPerSecond = 0;
};

struct FProfilerLiveStats final
{
    uint64_t TotalDroppedCpuScopes = 0;
    uint64_t TotalDroppedGpuFrames = 0;
    uint64_t TotalDroppedGpuScopes = 0;
    uint64_t TotalDroppedModelAnimationSamples = 0;
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    FProfilerAnimationStats Animation{};
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};

    EProfilerGpuFrameStatus LatestFrameGpuStatus = EProfilerGpuFrameStatus::Unsupported;
    uint64_t GpuFrameNumber = 0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    bool GpuScopesSupported = false;
    uint32_t DroppedGpuScopes = 0;
    std::vector<FProfilerGpuScopeSample> GpuScopes;
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

struct FProfilerGpuScopeAggregate final
{
    uint32_t NameId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double MaxFrameMs = 0.0;
    double P95FrameMs = 0.0;
};

class ENGINE_DLL CProfiler final
{
public:
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr uint32_t MAX_GPU_SCOPES_PER_FRAME = 128;
    static constexpr size_t MAX_ANIMATION_MODELS_PER_FRAME = 16384;
    static constexpr size_t MAX_HISTORY_FRAMES = 1200;
    static constexpr size_t MAX_LONG_OPERATIONS = 256;
    static constexpr double LONG_OPERATION_THRESHOLD_MS = 8.0;

public:
    CProfiler();
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
    uint32_t Register_ScopeName(std::string_view name) { return Intern_Name(name); }
    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    /* Immediate-context main thread only; disabled/unsupported/overflow scopes
       return UINT32_MAX. No query creation or GPU wait occurs on this path. */
    uint32_t Begin_GpuScope(std::string_view name);
    void End_GpuScope(uint32_t token) noexcept;

    FProfilerModelAnimationToken Begin_ModelAnimation() const noexcept;
    void End_ModelAnimation(const void* model, FProfilerModelAnimationToken token);
    void Record_ModelSubmitted(const void* model);

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
    /* Excludes incomplete GPU frames. Same-name calls are summed per frame;
       absent passes contribute zero to max/p95 and the valid-frame divisor. */
    void Get_GpuScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerGpuScopeAggregate>& outAggregates,
        size_t& outValidFrames,
        size_t& outPartialFrames) const;
    void Get_WindowFrameStats(
        size_t frameWindow,
        double& outCpuAvgMs,
        double& outCpuMaxMs,
        double& outGpuAvgMs,
        double& outGpuMaxMs,
        size_t& outFrames,
        size_t* outGpuValidFrames = nullptr) const;
    void Get_LongOperations(std::vector<FProfilerLongOperation>& outOperations) const;
    void Clear_LongOperations();

private:
    struct FGpuScopeQuery final
    {
        ComPtr<ID3D11Query> Begin;
        ComPtr<ID3D11Query> End;
        uint32_t Token = UINT32_MAX;
        uint32_t NameId = 0;
        uint32_t Depth = 0;
        bool Ended = false;
    };

    struct FGpuQuerySlot final
    {
        ComPtr<ID3D11Query> Disjoint;
        ComPtr<ID3D11Query> TimestampBegin;
        ComPtr<ID3D11Query> TimestampEnd;
        ComPtr<ID3D11Query> Pipeline;
        uint64_t FrameNumber = 0;
        uint64_t SubmittedPollFrame = 0;
        bool Pending = false;
        bool FrameEnded = false;
        uint32_t ScopeCount = 0;
        uint32_t OpenScopeCount = 0;
        std::array<uint32_t, MAX_GPU_SCOPES_PER_FRAME> OpenScopes{};
        std::array<FGpuScopeQuery, MAX_GPU_SCOPES_PER_FRAME> Scopes{};
    };

private:
    uint64_t Query_Tick() const noexcept;
    uint32_t Intern_Name(std::string_view name);
    bool Create_GpuQueries();
    void Begin_GpuFrame(uint64_t frameNumber);
    void End_GpuFrame(uint64_t frameNumber);
    void Resolve_GpuFrames(uint64_t currentPollFrame);
    void Commit_CurrentFrame();

private:
    // Distinguishes a new profiler constructed at a previously used address.
    const uint64_t m_InstanceId;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    LARGE_INTEGER m_Frequency{};
    std::atomic_bool m_Enabled = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_PollFrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    uint64_t m_PreviousFrameBeginTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_GpuScopeQueriesAvailable = false;
    uint32_t m_ActiveGpuSlot = UINT32_MAX;
    uint32_t m_NextGpuScopeToken = 0;
    std::atomic_bool m_FrameActive = false;
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
    uint64_t m_DroppedGpuScopes = 0;
    uint64_t m_DroppedModelAnimationSamples = 0;
    struct FModelAnimationWork final
    {
        uint64_t Calls = 0;
        double CpuMs = 0.0;
    };
    std::unordered_map<const void*, FModelAnimationWork> m_ModelAnimationWork;
    std::unordered_set<const void*> m_SubmittedModels;
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

class ENGINE_DLL CProfilerGpuScope final
{
public:
    CProfilerGpuScope(CProfiler* profiler, std::string_view name)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_GpuScope(name) : UINT32_MAX)
    {}
    ~CProfilerGpuScope()
    {
        if (m_pProfiler != nullptr && m_Token != UINT32_MAX)
            m_pProfiler->End_GpuScope(m_Token);
    }
    CProfilerGpuScope(const CProfilerGpuScope&) = delete;
    CProfilerGpuScope& operator=(const CProfilerGpuScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    uint32_t m_Token = UINT32_MAX;
};

class ENGINE_DLL CProfilerModelAnimationScope final
{
public:
    CProfilerModelAnimationScope(CProfiler* profiler, const void* model)
        : m_pProfiler(profiler), m_Model(model)
        , m_Token(profiler != nullptr ? profiler->Begin_ModelAnimation() : FProfilerModelAnimationToken{})
    {}
    ~CProfilerModelAnimationScope()
    {
        if (m_pProfiler != nullptr && m_Token.BeginTick != 0)
            m_pProfiler->End_ModelAnimation(m_Model, m_Token);
    }
    CProfilerModelAnimationScope(const CProfilerModelAnimationScope&) = delete;
    CProfilerModelAnimationScope& operator=(const CProfilerModelAnimationScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    const void* m_Model = nullptr;
    FProfilerModelAnimationToken m_Token{};
};

NS_END
