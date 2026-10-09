// 학습용 주석 사본 — 제품 빌드 입력이 아닙니다. 원본의 주석 외 C++ token을 그대로 보존했습니다.
// 원본: Engine/Public/Profiler.h; 사본의 줄 번호는 원본과 다릅니다. 검증: profiler-copy-manifest.json.

#pragma once

#include "Engine_Defines.h"
#include <dxgi1_4.h>
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

// G01 카운터: ms가 아니라 생산자가 실제 증가시킨 횟수/바이트 등이다.
// draw, instances, indices와 live gauge는 단위가 다르며 키 존재만으로 계측 생산자가 있음을 증명하지 않는다.
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
    IndirectDrawCalls,
    IndirectIndexUpperBound,
    ShadowCacheHits,
    ShadowCacheMisses,
    ShadowStaticCasters,
    ShadowDynamicCasters,
    MapCullingCandidates,
    MapCullingVisible,
    MapLod0Draws,
    MapLod1Draws,
    MapLod2Draws,
    MapLodSourceIndices,
    MapLodSubmittedIndices,
    LightRecords,
    LightDrawCalls,
    LightUploadBytes,
    MapLodAvailableDraws,
    LightCullingCandidates,
    LightCullingRejected,
    EffectBoundsCandidates,
    EffectBoundsCulled,
    EffectMarkerSamples,
    EffectMarkerHistoryRequests,
    EffectAmbientSuspended,
    EffectAmbientAdvanced,
    ImGuiPresentAttempts,
    ImGuiPresentBusy,
    ImGuiPresentFailures,
    ImGuiPresentOccluded,
    MapBatchVisibilityCacheHits,
    MapBatchVisibilityRebuilds,
    MapBatchEmptyRenders,
    MapBatchVisibleRenders,
    MapBatchBoundsRejected,
    MapBatchUploadBytes,
    NpcAuthoredHiddenUpdates,
    AmbientUnboundedUpdates,
    NpcCullingCandidates,
    NpcCulled,
    NpcDeferredPoseEvaluations,
    MeshDrawCalls,
    MeshInstances,
    MeshIndices,
    UniqueMeshes,
    DroppedMeshSamples,
    EffectAmbientClampedUpdates,
    // Sum of omitted visual time across effects, not wall time or CPU savings.
    EffectAmbientDiscardedMicroseconds,
    EffectAmbientFixedSteps,
    EffectAmbientMaxFixedSteps,
    // Chunk count/bytes are live gauges; draw/index counters are frame submissions.
    MapChunkCount,
    MapChunkNearDraws,
    MapChunkFarDraws,
    MapChunkSourceDraws,
    MapChunkSubmittedIndices,
    MapChunkOriginalIndices,
    MapChunkGpuBytes,
    MapChunkInvalidations,
    MapChunkEnabledCount,
    MapChunkHlodEnabledCount,
    MapIdenticalInstanceSourceDraws,
    MapIdenticalInstanceDraws,
    MapLightingBankSourceDraws,
    MapLightingBankDraws,
    MapOcclusionCandidates,
    MapOcclusionTested,
    MapOcclusionRejectedBatches,
    MapOcclusionSourceDraws,
    MapOcclusionRejectedIndices,
    MapOcclusionOccluders,
    MapOcclusionRasterizedTriangles,
    MapDistanceTestedInstances,
    MapDistanceRejectedInstances,
    MapDistanceRejectedIndices,
    MapOcclusionCacheHits,
    MapVisibilityPreparedBatches,
    MapVisibilityCpuJobs,
    MapVisibilityCallerJobs,
    MapVisibilityWorkerJobs,
    MapVisibilityAssistants,
    AnimationSampleRequests,
    AnimationSampleReuseHits,
    EffectParticleRootInverseRequests,
    EffectParticleRootInverseReuseHits,
    EffectParticleCallerJobs,
    EffectParticleWorkerJobs,
    EffectParticleAssistants,
    Count
};

// Fixed main-thread work categories are accumulated even with raw detail disabled.
// Times are inclusive: a parent category can overlap its instrumented children.
// G02 고정 CPU 작업 원장: raw 상세 계측을 꺼도 enum별 호출수와 QPC elapsed를 누적한다.
// 부모/자식 work는 중첩될 수 있으므로 전체 CPU 시간으로 합산하지 않는다.
enum class EProfilerWork : uint8_t
{
    MapBatchRender,
    MapBatchVisibility,
    MapBatchMaterial,
    MapBatchPass,
    MapBatchDraw,
    MapObjectRender,
    MapWaterRender,
    NpcUpdate,
    NpcLateUpdate,
    NpcRender,
    AmbientVisibility,
    AmbientAdvance,
    AmbientSubmit,
    Count
};

struct FProfilerWorkStats final
{
    uint64_t Calls = 0;
    double CpuMs = 0.0;
};

struct FProfilerWorkToken final
{
    uint64_t BeginTick = 0;
    uint64_t FrameNumber = 0;
    uint64_t CaptureEpoch = 0;
    uint64_t InstanceId = 0;
};

// G03 CPU raw 표본 계약: 이름 ID + 스레드 ID + 깊이 + QPC 시작/끝.
// 경과 ms = (EndTick - BeginTick) * 1000 / TicksPerSecond. 순수 CPU 실행시간이나 코어 사용률은 아니다.
struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    /* 실행한 Win32 thread ID. Loader/Effect worker도 자신이 완료한 프레임에 귀속된다. */
    uint32_t ThreadId = 0;
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

/* GPU 표본은 제출한 원래 프레임에 붙인다. Pending은 0ms가 아니다. GPU 실패에도 CPU 자료는 남긴다. */
enum class EProfilerGpuFrameStatus : uint8_t
{
    Unsupported,
    Pending,
    Valid,
    Disjoint,
    Dropped,
    Error
};

/* CPU-issued submissions through instrumented Engine VIBuffer/CMesh paths.
   Indices includes instances; Instances counts instanced submissions only.
   MeshInstances includes one for a non-instanced CMesh draw. These are not
   final visible geometry, and do not include DirectXTK/ImGui internal draws.
   Existing indirect upper-bound counters remain separate, never exact indices. */
struct FProfilerDrawStats final
{
    uint64_t DrawCalls = 0, InstancedDrawCalls = 0, Instances = 0, Indices = 0;
    uint64_t MeshDrawCalls = 0, MeshInstances = 0, MeshIndices = 0;
};

/* GPU 프레임 시작 기준 timestamp 구간. 부모와 자식은 중첩되며 같은 이름의 여러 호출도 별도 표본이다. */
// G04 GPU raw 표본 계약: Begin/EndMs는 CPU QPC가 아닌 GPU frame timestamp 기준이다.
// DurationMs는 자식 포함, SelfMs는 계측된 직접 자식 제외. 두 시계의 절대 원점을 맞춘 타임라인이 아니다.
struct FProfilerGpuScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    double BeginMs = 0.0;
    double EndMs = 0.0;
    double DurationMs = 0.0;
    double SelfMs = 0.0;
    FProfilerDrawStats Draw{};
    bool PipelineValid = false;
    uint64_t PSInvocations = 0, VSInvocations = 0;
    uint64_t IAVertices = 0, IAPrimitives = 0;
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

// Secondary window presentation only. Main-thread capture; bounded per frame.
struct FProfilerViewportPresent final
{
    uint32_t ViewportId = 0;
    float X = 0, Y = 0, Width = 0, Height = 0;
    double CpuMs = 0;
    uint32_t SyncInterval = 0, Flags = 0;
    int32_t Result = 0;
};

// Optional detailed submission trace. Names are display labels, not stable
// asset IDs. IndexCount is per instance; there is no per-draw GPU timing.
struct FProfilerMeshDrawSample final
{
    uint32_t PassNameId = UINT32_MAX, MeshNameId = 0;
    uint32_t VertexCount = 0, IndexCount = 0, Instances = 0, MaterialSlot = 0;
};

// Low-rate OS observations, not allocator/resource attribution. Each validity
// flag is independent; unavailable must never be displayed as measured zero.
struct FProfilerMemorySegment final
{
    bool Valid = false;
    uint64_t CurrentUsageBytes = 0, BudgetBytes = 0;
};
struct FProfilerMemoryStats final
{
    bool Sampled = false, ProcessValid = false, SystemValid = false;
    uint64_t SampleFrameNumber = 0, SampleTick = 0;
    // Captured frames use frame End; LiveStats uses the current QPC tick.
    double AgeMs = 0.0;
    uint32_t ProcessId = 0;
    uint64_t PrivateCommitBytes = 0, WorkingSetBytes = 0;
    // OS process-lifetime peaks, not capture-window peaks.
    uint64_t PeakWorkingSetBytes = 0, PeakPrivateCommitBytes = 0;
    uint64_t SystemCommitBytes = 0, SystemCommitLimitBytes = 0, SystemAvailableBytes = 0;
    bool AdapterIdentityValid = false;
    uint32_t AdapterLuidLow = 0, AdapterNodeIndex = 0;
    int32_t AdapterLuidHigh = 0;
    // Local does not necessarily mean dedicated VRAM (for example on UMA).
    FProfilerMemorySegment Local{}, NonLocal{};
};

// G05 한 프레임 봉투: CPU는 End_Frame 때 확정되지만 GPU는 나중에 원래 FrameNumber를 찾아 채운다.
// CpuFrameMs와 GpuFrameMs는 병렬로 진행 가능한 elapsed이므로 더하지 않는다.
// FrameIntervalMs는 이전 시작과 현재 시작 사이이다. 이번 CPU 프레임에 무조건 더할 gap이 아니다.
struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    uint64_t FrameBeginTick = 0, FrameEndTick = 0;
    FProfilerMemoryStats Memory{};
    std::vector<FProfilerMeshDrawSample> MeshDraws;
    uint64_t DroppedMeshDraws = 0;
    // Interval은 이전 Begin부터 이번 Begin, Gap은 이전 End부터 이번 Begin이다.
    // 첫 표본을 제외하면 Interval = PreviousCpuFrameMs + FrameGapMs다.
    double PreviousCpuFrameMs = 0.0, FrameGapMs = 0.0;
    // Dropped completions attributed to this frame; excludes deliberately disabled detail scopes.
    uint64_t DroppedCpuScopes = 0;
    bool DetailedCpuScopes = false;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    FProfilerAnimationStats Animation{};
    std::array<FProfilerWorkStats, static_cast<size_t>(EProfilerWork::Count)> CpuWork{};
    std::vector<FProfilerViewportPresent> ViewportPresents;
    uint32_t DroppedViewportPresents = 0;
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

// Describes one newest-completed-frame export without copying any frame samples.
// Evicted frames are counted since Reset; excluded retained frames can still be saved.
// G06 저장 범위 증거: 보관된 1200개 중 저장에서 제외한 수와 이미 ring에서 퇴출된 수는 다르다.
// Excluded는 더 넓게 저장하면 얻을 수 있지만 Evicted는 이미 복구할 수 없다.
struct FProfilerCaptureWindow final
{
    uint64_t RequestedFrames = 0;
    uint64_t RetainedFrames = 0;
    uint64_t SavedFrames = 0;
    uint64_t ExcludedRetainedFrames = 0;
    uint64_t EvictedFramesSinceReset = 0;
    uint64_t FirstRetainedFrameNumber = 0;
    uint64_t LastRetainedFrameNumber = 0;
    uint64_t FirstSavedFrameNumber = 0;
    uint64_t LastSavedFrameNumber = 0;
    double ExcludedMaxFrameIntervalMs = 0.0;
};

struct FProfilerCaptureSnapshot final
{
    std::vector<std::string> ScopeNames;
    std::vector<FProfilerFrame> Frames;
    FProfilerCaptureWindow CaptureWindow{};
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
    uint64_t DroppedCpuScopes = 0;
    bool DetailedCpuScopes = false;
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    uint64_t FrameBeginTick = 0, FrameEndTick = 0;
    FProfilerMemoryStats Memory{};
    double PreviousCpuFrameMs = 0.0, FrameGapMs = 0.0;
    FProfilerAnimationStats Animation{};
    std::array<FProfilerWorkStats, static_cast<size_t>(EProfilerWork::Count)> CpuWork{};
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
    // 선택 구간에서 scope 하나라도 누락되면 self의 완전성을 보장하지 않는다.
    bool SelfComplete = true;
    double MaxMs = 0.0;
};

struct FProfilerGpuScopeAggregate final
{
    uint32_t NameId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double SelfMs = 0.0;
    double MaxFrameMs = 0.0;
    double P95FrameMs = 0.0;
    double SelfMaxFrameMs = 0.0, SelfP95FrameMs = 0.0;
    FProfilerDrawStats Draw{};
    uint64_t PipelineSamples = 0, PSInvocations = 0, VSInvocations = 0;
    uint64_t IAVertices = 0, IAPrimitives = 0;
};

// G07 실제 계측 소유자. Client/Default/Client.cpp의 루프가 Begin_Frame -> Update -> Render -> End_Frame을 호출한다.
// Renderer는 pass마다 CPU/GPU RAII를 배치한다. ProfilerTool은 표시/수집 요청/저장 소비자다.
// 기본 Capture는 OFF. F7 창 열기만으로 수집이 시작되지는 않는다.
class ENGINE_DLL CProfiler final
{
public:
    // 8개 query slot을 순환하며 최소 4개의 실제 루프 경계 뒤에 읽는다.
    // slot이 아직 Pending이면 덮어쓰거나 기다리지 않고 이번 GPU 표본을 Dropped 처리한다.
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr uint32_t MAX_GPU_SCOPES_PER_FRAME = 128;
    static constexpr uint32_t MAX_GPU_PIPELINE_SCOPES_PER_FRAME = 8;
    static constexpr size_t MAX_ANIMATION_MODELS_PER_FRAME = 16384;
    static constexpr size_t MAX_UNIQUE_MESHES_PER_FRAME = 16384;
    static constexpr size_t MAX_MESH_DRAWS_PER_FRAME = 512;
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

    // UI에서 바꾼 상세 계측 정책은 다음 프레임 경계부터 적용한다.
    void Set_DetailedScopesEnabled(bool enabled) noexcept { m_RequestedDetailedScopes.store(enabled, std::memory_order_relaxed); }
    bool Is_DetailedScopesEnabled() const noexcept { return m_RequestedDetailedScopes.load(std::memory_order_relaxed); }
    bool Is_CollectingDetailedScopes() const noexcept { return m_DetailedScopes.load(std::memory_order_relaxed); }

    void Reset_History();

    /* Thread-safe. Nesting is tracked per thread, so a scope may begin and end
       on any thread and outside the main-thread frame boundaries. The token is
       only meaningful on the thread that began the scope. */
    uint32_t Register_ScopeName(std::string_view name) { return Intern_Name(name); }
    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    /* Immediate-context main thread only; disabled/unsupported/overflow scopes
       return UINT32_MAX. No query creation or GPU wait occurs on this path. */
    uint32_t Begin_GpuScope(std::string_view name, bool collectPipeline = false);
    void End_GpuScope(uint32_t token) noexcept;

    // 메인 스레드 전용 고정 enum 집계. raw 이름/표본/잠금/동적 할당을 만들지 않는다.
    FProfilerWorkToken Begin_Work(EProfilerWork work) const noexcept;
    void End_Work(EProfilerWork work, FProfilerWorkToken token) noexcept;
    static const char* Get_WorkName(EProfilerWork work) noexcept;

    FProfilerModelAnimationToken Begin_ModelAnimation() const noexcept;
    void End_ModelAnimation(const void* model, FProfilerModelAnimationToken token);
    void Record_ModelSubmitted(const void* model);
    // Main-thread CMesh draw boundary only. Pointer is an ephemeral dedup key,
    // never exported. Geometry shared by many CModel clones counts once.
    void Record_MeshSubmitted(const void* mesh, uint32_t indexCount, uint32_t instances = 1,
        std::string_view meshName = {}, uint32_t vertexCount = 0, uint32_t materialSlot = 0);
    void Record_ViewportPresent(const FProfilerViewportPresent& sample);

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot(size_t frameWindow = MAX_HISTORY_FRAMES) const;
    FProfilerCaptureWindow Get_CaptureWindow(size_t frameWindow) const;
    bool Get_LiveStats(FProfilerLiveStats& outStats) const;
    // Copies only distinct memory poll attempts from up to maxFrames retained
    // frames, oldest first. AgeMs uses the current tick; no OS query is issued.
    void Get_MemorySamples(size_t maxFrames, std::vector<FProfilerMemoryStats>& outSamples) const;

    uint32_t Get_MainThreadId() const noexcept { return m_MainThreadId; }
    double Ticks_ToMs(uint64_t ticks) const noexcept;
    size_t Get_HistoryFrameCount() const;
    void Get_ScopeNames(std::vector<std::string>& outNames) const;
    /* Aggregates the most recent frameWindow history frames, sorted by
       inclusive time descending. */
    void Get_ScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerScopeAggregate>& outAggregates) const;
    /* 불완전 GPU 프레임은 제외한다. 같은 이름 호출은 프레임 안에서 합친다.
       유효한 프레임에 호출 자체가 없으면 그 pass는 0이며 분모에도 포함한다. */
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
        uint32_t PipelineIndex = UINT32_MAX;
        uint32_t NameId = 0;
        uint32_t Depth = 0;
        bool Ended = false;
        FProfilerDrawStats DrawBegin{}, Draw{};
    };

    // G08 GPU 비동기 상태: FrameNumber는 원래 제출 프레임, SubmittedPollFrame은 실제 루프 경계다.
    // Pending/FrameEnded로 재사용 가능 여부를 판단하고 CPU를 GPU 완료에 동기화시키지 않는다.
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
        uint32_t PipelineScopeCount = 0;
        std::array<ComPtr<ID3D11Query>, MAX_GPU_PIPELINE_SCOPES_PER_FRAME> PassPipelines{};
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
    // The caller holds m_Mutex, keeping coverage and copied frames consistent.
    FProfilerCaptureWindow Get_CaptureWindowLocked(size_t frameWindow) const;
    FProfilerDrawStats Read_DrawCounters() const noexcept;
    void Sample_Memory(uint64_t tick);

private:
    // Distinguishes a new profiler constructed at a previously used address.
    const uint64_t m_InstanceId;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    ComPtr<IDXGIAdapter3> m_pMemoryAdapter;
    bool m_MemoryAdapterIdentityValid = false;
    LUID m_MemoryAdapterLuid{};
    uint64_t m_LastMemoryPollTick = 0;
    FProfilerMemoryStats m_MemorySample{};
    // QPC의 초당 tick 수. GPU는 각 disjoint query의 Frequency를 따로 사용한다.
    LARGE_INTEGER m_Frequency{};
    // UI 요청은 Begin_Frame에서 확정한다. 프레임 도중 일부만 Capture하는 상태를 피한다.
    std::atomic_bool m_Enabled = false;
    std::atomic_bool m_Collecting = false;
    std::atomic_bool m_ResetRequested = false;
    // Reset/수집 전환의 세대 번호. 오래된 worker scope가 새 capture에 섞이지 않게 거른다.
    std::atomic_uint64_t m_CaptureEpoch = 0;
    std::atomic_bool m_RequestedDetailedScopes = false;
    std::atomic_bool m_DetailedScopes = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_PollFrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    uint64_t m_PreviousFrameBeginTick = 0;
    uint64_t m_PreviousFrameEndTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_GpuScopeQueriesAvailable = false;
    bool m_GpuPipelineQueriesAvailable = false;
    uint32_t m_ActiveGpuSlot = UINT32_MAX;
    uint32_t m_NextGpuScopeToken = 0;
    std::atomic_bool m_FrameActive = false;
    uint32_t m_MainThreadId = 0;
    // 완료 CPU 표본, 이름 등록, history snapshot/집계를 직렬화한다.
    // 프로파일러 자체도 잠금/복사/집계 비용이 있어 상세 계측을 동일 조건에서 비교해야 한다.
    mutable std::mutex m_Mutex;
    /* 지난 End_Frame 이후 모든 스레드에서 완료한 scope. 다음 프레임 commit에서 이동한다. */
    std::vector<FProfilerScopeSample> m_PendingScopes;
    std::deque<FProfilerLongOperation> m_LongOperations;
    uint64_t m_LongOperationSequence = 0;
    uint64_t m_SharedFrameNumber = 0;
    std::deque<FProfilerFrame> m_History;
    uint64_t m_EvictedHistoryFrames = 0;
    std::vector<std::string> m_ScopeNames;
    std::unordered_map<std::string, uint32_t> m_ScopeNameLookup;
    uint64_t m_DroppedCpuScopes = 0;
    uint64_t m_PendingDroppedCpuScopes = 0;
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
    // Fixed table avoids allocating a node for each mesh on every frame.
    std::array<const void*, MAX_UNIQUE_MESHES_PER_FRAME * 2> m_SubmittedMeshKeys{};
    size_t m_SubmittedMeshCount = 0;
};

// G09 CPU RAII: 생성자에서 Begin_Scope, 블록 종료/조기 return 때 소멸자가 End_Scope를 호출한다.
// token은 시작한 스레드의 TLS stack 인덱스이므로 같은 스레드에서 닫아야 한다.
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

// High-frequency per-draw CPU work is opt-in; pass timing and counters stay available.
class CProfilerDetailScope final
{
public:
    CProfilerDetailScope(CProfiler* profiler, std::string_view name)
        : m_Scope(profiler && profiler->Is_CollectingDetailedScopes() ? profiler : nullptr, name) {}
    CProfilerDetailScope(const CProfilerDetailScope&) = delete;
    CProfilerDetailScope& operator=(const CProfilerDetailScope&) = delete;
private:
    CProfilerScope m_Scope;
};

class CProfilerWorkScope final
{
public:
    CProfilerWorkScope(CProfiler* profiler, EProfilerWork work) noexcept
        : m_pProfiler(profiler), m_Work(work)
        , m_Token(profiler ? profiler->Begin_Work(work) : FProfilerWorkToken{}) {}
    ~CProfilerWorkScope()
    {
        if (m_pProfiler && m_Token.BeginTick != 0)
            m_pProfiler->End_Work(m_Work, m_Token);
    }
    CProfilerWorkScope(const CProfilerWorkScope&) = delete;
    CProfilerWorkScope& operator=(const CProfilerWorkScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    EProfilerWork m_Work = EProfilerWork::Count;
    FProfilerWorkToken m_Token{};
};

// G10 GPU RAII: 실제 GPU를 지금 기다리는 객체가 아니라 immediate context에 timestamp query를 넣는다.
// Renderer 메인 스레드 전용이며 결과는 Resolve_GpuFrames가 나중에 회수한다.
class ENGINE_DLL CProfilerGpuScope final
{
public:
    CProfilerGpuScope(CProfiler* profiler, std::string_view name, bool collectPipeline = false)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_GpuScope(name, collectPipeline) : UINT32_MAX)
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
