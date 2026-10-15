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
    // 이펙트별 생략된 표시 시간의 합이다. 실제 경과 시간이나 CPU 절약 시간이 아니다.
    EffectAmbientDiscardedMicroseconds,
    EffectAmbientFixedSteps,
    EffectAmbientMaxFixedSteps,
    // chunk 개수·byte는 현재 보유량, draw·index 수는 프레임 제출량이다.
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
    // 준비 보유량은 Level snapshot, 선택은 batch별 현재 프레임의 마지막 판정, draw는 실제 제출량이다.
    MapLowGeometryLeaseActive,
    MapLowGeometryUnsupportedBatches,
    MapLowGeometrySupportedBatches,
    MapLowGeometryPreparedBatches,
    MapLowGeometryPreparationRejectedBatches,
    MapLowGeometryPreparationFailedBatches,
    MapLowGeometryPreparationReportFailures,
    MapLowGeometryVisitedBatches,
    MapLowGeometryBindingDisabledBatches,
    MapLowGeometryNoCameraBatches,
    MapLowGeometryMeshLodDisabledBatches,
    MapLowGeometryEnvironmentReplacedBatches,
    MapLowGeometrySourceMaterialsDisabledBatches,
    MapLowGeometryDebugViewBatches,
    MapLowGeometryMapPbrBatches,
    MapLowGeometryBindingCollectionBatches,
    MapLowGeometryFrustumOverrideBatches,
    MapLowGeometryInvalidCameraBatches,
    MapLowGeometryTestedInstances,
    MapLowGeometryVisibilityRejectedInstances,
    MapLowGeometryDistanceRejectedInstances,
    MapLowGeometryDepthRejectedInstances,
    MapLowGeometryPixelRejectedInstances,
    MapLowGeometryFrustumRejectedInstances,
    MapLowGeometryCandidateInstances,
    MapLowGeometryPreflightFailedBatches,
    MapLowGeometrySelectedInstances,
    MapLowGeometryDrawCalls,
    MapLowGeometrySubmittedIndices,
    MapLowGeometryLateFailures,
    MapLowGeometryFallbackDrawCalls,
    MapLowGeometryFallbackIndices,
    Count
};

// 상세 raw 기록을 꺼도 주 스레드의 고정 작업 분류는 누적한다. 부모·자식이 겹치는 inclusive 시간이다.
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

struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    /* scope를 실행한 Win32 thread ID다. worker scope는 종료된 프레임에 속한다. */
    uint32_t ThreadId = 0;
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

/* GPU sample은 원래 제출 프레임에 속한다. Pending은 0ms가 아니며 GPU 수집 실패·미지원이어도 CPU 데이터는 유지한다. */
enum class EProfilerGpuFrameStatus : uint8_t
{
    Unsupported,
    Pending,
    Valid,
    Disjoint,
    Dropped,
    Error
};

/* 계측한 VIBuffer·CMesh의 CPU 제출량이다. index에는 instance를 반영하고 MeshInstances는 일반 mesh draw도 1로 센다. 최종 가시 geometry·외부 UI draw·간접 draw 상계와 구분한다. */
struct FProfilerDrawStats final
{
    uint64_t DrawCalls = 0, InstancedDrawCalls = 0, Instances = 0, Indices = 0;
    uint64_t MeshDrawCalls = 0, MeshInstances = 0, MeshIndices = 0;
};

/* GPU 프레임 시작 기준 inclusive 구간이다. 중첩 scope는 겹치며 같은 NameId의 호출도 별도 sample로 보관한다. */
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

/* 주 스레드의 animation 계산과 같은 프레임의 성공한 모델 제출을 연결한다. NotSubmitted만으로 frustum 밖이라고 판단하지 않는다. */
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

// 보조 창 표시만 주 스레드에서 프레임별 상한 안에 수집한다.
struct FProfilerViewportPresent final
{
    uint32_t ViewportId = 0;
    float X = 0, Y = 0, Width = 0, Height = 0;
    double CpuMs = 0;
    uint32_t SyncInterval = 0, Flags = 0;
    int32_t Result = 0;
};

// 선택적 상세 제출 기록이다. 이름은 표시용이며 stable asset ID가 아니다. IndexCount는 instance당 값이고 draw별 GPU 시간은 없다.
struct FProfilerMeshDrawSample final
{
    uint32_t PassNameId = UINT32_MAX, MeshNameId = 0;
    uint32_t VertexCount = 0, IndexCount = 0, Instances = 0, MaterialSlot = 0;
};

// 낮은 빈도의 OS 관측값이며 할당자·자원별 사용량이 아니다. 각 유효 flag는 독립이고 unavailable을 측정된 0으로 표시하지 않는다.
struct FProfilerMemorySegment final
{
    bool Valid = false;
    uint64_t CurrentUsageBytes = 0, BudgetBytes = 0;
};
struct FProfilerMemoryStats final
{
    bool Sampled = false, ProcessValid = false, SystemValid = false;
    uint64_t SampleFrameNumber = 0, SampleTick = 0;
    // capture frame은 End 시각, LiveStats는 현재 QPC 시각을 기준으로 한다.
    double AgeMs = 0.0;
    uint32_t ProcessId = 0;
    uint64_t PrivateCommitBytes = 0, WorkingSetBytes = 0;
    // OS process-lifetime peaks, not capture-window peaks.
    uint64_t PeakWorkingSetBytes = 0, PeakPrivateCommitBytes = 0;
    uint64_t SystemCommitBytes = 0, SystemCommitLimitBytes = 0, SystemAvailableBytes = 0;
    bool AdapterIdentityValid = false;
    uint32_t AdapterLuidLow = 0, AdapterNodeIndex = 0;
    int32_t AdapterLuidHigh = 0;
    // Local 메모리가 항상 전용 VRAM을 뜻하지는 않는다. UMA도 고려해야 한다.
    FProfilerMemorySegment Local{}, NonLocal{};
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    uint64_t FrameBeginTick = 0, FrameEndTick = 0;
    FProfilerMemoryStats Memory{};
    std::vector<FProfilerMeshDrawSample> MeshDraws;
    uint64_t DroppedMeshDraws = 0;
    // Interval은 이전 Begin~현재 Begin, Gap은 이전 End~현재 Begin이다. 첫 프레임을 제외하면 Interval=이전 CPU 시간+Gap이다.
    double PreviousCpuFrameMs = 0.0, FrameGapMs = 0.0;
    // 이 프레임에 귀속된 누락 완료 수다. 명시적으로 끈 상세 scope는 제외한다.
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

// sample 복사 없이 최신 완료 frame의 export 범위를 설명한다. eviction은 Reset 이후 수이며 제외된 보유 frame은 별도로 저장할 수 있다.
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

// Capture ON/OFF와 독립인 완료 프레임 계측이다. Vertex는 전체 GPU VS 실행 수로
// TK/UI도 포함한다. Index는 계측된 Engine direct/indirect 제출의 index 참조 수다.
// 두 값은 unique vertex나 최종 보이는 픽셀 수가 아니며, 유효성은 각각 판단한다.
struct FProfilerGeometryStats final
{
    uint64_t FrameNumber = 0, CurrentFrameNumber = 0, CaptureFrameNumber = 0;
    uint64_t VSInvocations = 0, DirectIndices = 0, IndirectIndices = 0, TotalIndices = 0;
    uint64_t IndirectDrawCalls = 0, DroppedFrames = 0;
    uint32_t LatencyFrames = 0;
    bool VertexValid = false, IndexValid = false, Pending = false;
};

// BufferOrdinal은 이 geometry frame 안에서만 유효하다. 주소는 저장/출력하지 않는다.
// 같은 args offset을 여러 번 제출했으면 DrawCount가 그 횟수다.
struct FProfilerIndirectArgument final
{
    uint32_t BufferOrdinal = 0, ArgsByteOffset = 0;
    uint32_t IndexCount = 0, InstanceCount = 0, DrawCount = 0;
};

/* 임계 시간 이상인 완료 scope를 작은 별도 ring에 보관해 원래 frame이 history에서 사라져도 긴 작업을 조회한다. */
struct FProfilerLongOperation final
{
    uint64_t Sequence = 0;
    uint64_t FrameNumber = 0;
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    double DurationMs = 0.0;
};

/* 선택한 history 구간의 이름·thread 쌍별 합이다. self 시간은 같은 thread의 내부 scope를 뺀다. */
struct FProfilerScopeAggregate final
{
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double SelfMs = 0.0;
    // 선택 구간에 누락 scope가 하나라도 있으면 self 시간 귀속이 완전하지 않다.
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

// CPU 구간·GPU query·그리기 통계를 프레임 기록으로 모아 조회와 저장용 snapshot을 제공한다.
// CPU worker의 완료 구간은 잠금으로 모으고, immediate-context GPU 계측은 초기화한 주 스레드가 수행한다.
class ENGINE_DLL CProfiler final
{
public:
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
    ~CProfiler();
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    void Begin_Frame();
    void End_Frame();

    void Set_Enabled(bool enabled) noexcept;
    bool Is_Enabled() const noexcept;

    // Requested UI policy is applied at the next frame boundary.
    void Set_DetailedScopesEnabled(bool enabled) noexcept { m_RequestedDetailedScopes.store(enabled, std::memory_order_relaxed); }
    bool Is_DetailedScopesEnabled() const noexcept { return m_RequestedDetailedScopes.load(std::memory_order_relaxed); }
    bool Is_CollectingDetailedScopes() const noexcept { return m_DetailedScopes.load(std::memory_order_relaxed); }

    void Reset_History();

    /* 중첩을 thread별로 추적하며 main frame 밖에서도 호출할 수 있다. token의 시작·종료는 같은 thread에서 수행해야 한다. */
    uint32_t Register_ScopeName(std::string_view name) { return Intern_Name(name); }
    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    /* immediate-context 주 스레드 전용이다. 비활성·미지원·초과 시 UINT32_MAX이며 이 경로는 query 생성·GPU 대기를 하지 않는다. */
    uint32_t Begin_GpuScope(std::string_view name, bool collectPipeline = false);
    void End_GpuScope(uint32_t token) noexcept;

    // 주 스레드 전용이며 raw sample·이름·잠금·할당을 사용하지 않는다.
    FProfilerWorkToken Begin_Work(EProfilerWork work) const noexcept;
    void End_Work(EProfilerWork work, FProfilerWorkToken token) noexcept;
    static const char* Get_WorkName(EProfilerWork work) noexcept;

    FProfilerModelAnimationToken Begin_ModelAnimation() const noexcept;
    void End_ModelAnimation(const void* model, FProfilerModelAnimationToken token);
    void Record_ModelSubmitted(const void* model);
    // 주 스레드 CMesh draw에서만 기록한다. pointer는 export하지 않는 임시 중복 key이며 여러 복제본이 공유한 geometry는 한 번 센다.
    void Record_MeshSubmitted(const void* mesh, uint32_t indexCount, uint32_t instances = 1,
        std::string_view meshName = {}, uint32_t vertexCount = 0, uint32_t materialSlot = 0);
    void Record_ViewportPresent(const FProfilerViewportPresent& sample);

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot(size_t frameWindow = MAX_HISTORY_FRAMES) const;
    FProfilerCaptureWindow Get_CaptureWindow(size_t frameWindow) const;
    bool Get_LiveStats(FProfilerLiveStats& outStats) const;
    // 모두 immediate-context main thread용이다. 조회는 GPU 작업/대기를 만들지 않는다.
    uint64_t Get_GeometryFrameNumber() const noexcept;
    bool Get_GeometryStats(FProfilerGeometryStats& output) const noexcept;
    bool Get_GeometryStatsForFrame(uint64_t frameNumber, FProfilerGeometryStats& output) const noexcept;
    void Request_GeometryDetailForCurrentFrame() noexcept;
    bool Get_GeometryIndirectArguments(uint64_t frameNumber,
        std::vector<FProfilerIndirectArgument>& output) const;
    uint32_t Get_GeometryIndirectBufferOrdinal(ID3D11Buffer* arguments) const noexcept;
    // CMesh가 성공한 indirect 제출 뒤 호출한다. args는 End_Frame까지 불변이어야 한다.
    void Record_GeometryIndirectDraw(ID3D11Buffer* arguments, uint32_t argsByteOffset) noexcept;
    // 보유한 최대 maxFrames에서 중복 없는 메모리 조회 시도만 오래된 순서로 복사한다. 현재 tick으로 AgeMs를 구하며 OS를 재조회하지 않는다.
    void Get_MemorySamples(size_t maxFrames, std::vector<FProfilerMemoryStats>& outSamples) const;

    uint32_t Get_MainThreadId() const noexcept { return m_MainThreadId; }
    double Ticks_ToMs(uint64_t ticks) const noexcept;
    size_t Get_HistoryFrameCount() const;
    void Get_ScopeNames(std::vector<std::string>& outNames) const;
    /* 최근 frameWindow history를 집계해 inclusive 시간 내림차순으로 반환한다. */
    void Get_ScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerScopeAggregate>& outAggregates) const;
    /* 미완료 GPU frame은 제외한다. 같은 이름 호출은 프레임별로 합치고 없는 pass는 유효 frame의 0으로 max·p95에 포함한다. */
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
    void Initialize_Geometry();
    void Begin_GeometryFrame() noexcept;
    void End_GeometryFrame() noexcept;
    void Resolve_GeometryFrames() noexcept;
    void Add_GeometryDirectIndices(uint64_t count) noexcept;

private:
    // Distinguishes a new profiler constructed at a previously used address.
    const uint64_t m_InstanceId;
    struct FGeometryState;
    std::unique_ptr<FGeometryState> m_Geometry;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    ComPtr<IDXGIAdapter3> m_pMemoryAdapter;
    bool m_MemoryAdapterIdentityValid = false;
    LUID m_MemoryAdapterLuid{};
    uint64_t m_LastMemoryPollTick = 0;
    FProfilerMemoryStats m_MemorySample{};
    LARGE_INTEGER m_Frequency{};
    // UI 요청은 Begin_Frame에서 받아 capture가 프레임 중간에 시작되지 않게 한다.
    std::atomic_bool m_Enabled = false;
    std::atomic_bool m_Collecting = false;
    std::atomic_bool m_ResetRequested = false;
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
    mutable std::mutex m_Mutex;
    /* Completed scopes from every thread since the last End_Frame. */
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
    // 고정 table로 프레임마다 mesh별 node를 할당하지 않는다.
    std::array<const void*, MAX_UNIQUE_MESHES_PER_FRAME * 2> m_SubmittedMeshKeys{};
    size_t m_SubmittedMeshCount = 0;
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

// 빈번한 draw별 CPU 계측만 선택적으로 켠다. pass 시간·counter는 계속 제공한다.
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
