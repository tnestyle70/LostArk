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
    FrameWaitMicroseconds,
    FrameCadenceMicroseconds,
    FrameUpdateMicroseconds,
    FrameRenderMicroseconds,
    FramePresentMicroseconds,
    PickingReadbackBytes,
    PickingReadbackMicroseconds,
    EffectActiveOccurrences,
    EffectActiveElements,
    EffectFixedSteps,
    EffectCatchupSteps,
    EffectParticles,
    EffectMeshParticles,
    EffectTrailPoints,
    EffectSpawned,
    EffectRejected,
    EffectSuppressed,
    EffectActualDraws,
    EffectDynamicUploadBytes,
    AnimationCharacters,
    AnimationChannels,
    AnimationBones,
    AnimationTimelineBuilds,
    AnimationCorrectionSeeks,
    AnimationPaletteUploadBytes,
    MapLightsSubmitted,
    MapLightsCulled,
    NetworkInboundFrames,
    NetworkEventsApplied,
    NetworkSnapshotsApplied,
    AudioFirstLoads,
    AudioFirstLoadMicroseconds,
    ProcessPrivateBytes,
    ProcessWorkingSetBytes,
    GpuLocalUsageBytes,
    GpuLocalBudgetBytes,
    Count
};

struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0; 
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    uint64_t CpuFrameBeginTick = 0;
    uint64_t CpuFrameEndTick = 0;
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
    uint64_t TickFrequency = 0;
    std::vector<std::string> ScopeNames;
    std::vector<FProfilerFrame> Frames;
    uint64_t DroppedCpuScopes = 0;
    uint64_t DroppedGpuFrames = 0;
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

class ENGINE_DLL CProfiler final
{
public:
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr size_t MAX_HISTORY_FRAMES = 7200;

public:
    CProfiler() = default;
    ~CProfiler() = default;
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    void Begin_Frame();
    void End_Frame();

    void Set_Enabled(bool enabled) noexcept;
    bool Is_Enabled() const noexcept;
    
    void Reset_History();

    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot() const;
    FProfilerCaptureSnapshot Snapshot_Recent(size_t maximumFrames) const;
    bool Get_LiveStats(FProfilerLiveStats& outStats) const;

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

    struct FOpenScope final
    {
        uint32_t SampleIndex = 0;
        uint32_t Depth = 0;
    };

private:
    uint64_t Query_Tick() const noexcept;
    uint32_t Intern_Name(std::string_view name);
    bool Create_GpuQueries();
    void Begin_GpuFrame(uint64_t frameNumber);
    void End_GpuFrame(uint64_t frameNumber);
    void Resolve_GpuFrames(uint64_t currentFrame);
    void Commit_CurrentFrame();
    void Sample_ProcessAndGpuMemory();

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    LARGE_INTEGER m_Frequency{};
    std::atomic_bool m_Enabled = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::vector<FOpenScope> m_OpenScopes;
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_FrameActive = false;
    mutable std::mutex m_Mutex;
    std::deque<FProfilerFrame> m_History;
    std::vector<std::string> m_ScopeNames;
    std::unordered_map<std::string, uint32_t> m_ScopeNameLookup;
    uint64_t m_DroppedCpuScopes = 0;
    uint64_t m_DroppedGpuFrames = 0;
    uint64_t m_CachedPrivateBytes = 0;
    uint64_t m_CachedWorkingSetBytes = 0;
    uint64_t m_CachedGpuLocalUsageBytes = 0;
    uint64_t m_CachedGpuLocalBudgetBytes = 0;
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
