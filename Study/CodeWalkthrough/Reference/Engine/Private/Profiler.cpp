// 학습용 주석 사본 — 제품 빌드 입력이 아닙니다. 원본의 주석 외 C++ token을 그대로 보존했습니다.
// 원본: Engine/Private/Profiler.cpp; 사본의 줄 번호는 원본과 다릅니다. 검증: profiler-copy-manifest.json.

#include "Profiler.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <psapi.h>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 8192;
    // 자식이 먼저 끝나므로 상세 표본이 가득 차도 나중에 끝나는 메인 pass/root 자리부터 보존한다.
    constexpr size_t MAIN_PASS_SCOPE_RESERVE = 1024;
    constexpr size_t MAIN_ROOT_SCOPE_RESERVE = 128;
    constexpr size_t MAX_OPEN_SCOPES_PER_THREAD = 64;

    FProfilerDrawStats DrawDifference(const FProfilerDrawStats& end, const FProfilerDrawStats& begin)
    {
        return {end.DrawCalls - begin.DrawCalls, end.InstancedDrawCalls - begin.InstancedDrawCalls,
            end.Instances - begin.Instances, end.Indices - begin.Indices,
            end.MeshDrawCalls - begin.MeshDrawCalls, end.MeshInstances - begin.MeshInstances,
            end.MeshIndices - begin.MeshIndices};
    }

    void AddDrawStats(FProfilerDrawStats& total, const FProfilerDrawStats& value)
    {
        total.DrawCalls += value.DrawCalls;
        total.InstancedDrawCalls += value.InstancedDrawCalls;
        total.Instances += value.Instances;
        total.Indices += value.Indices;
        total.MeshDrawCalls += value.MeshDrawCalls;
        total.MeshInstances += value.MeshInstances;
        total.MeshIndices += value.MeshIndices;
    }

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
        uint64_t CaptureEpoch;
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

// G01 초기화: GameInstance가 장치와 immediate context를 넘긴다. 메인 thread ID와 QPC 주파수를 보관한다.
// GPU query를 여기서 미리 만들어 hot path에서 CreateQuery하지 않는다. GPU 지원 실패와 CPU 수집은 구분한다.
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
    m_pMemoryAdapter.Reset(); m_MemoryAdapterIdentityValid = false; m_MemoryAdapterLuid = {};
    m_MemorySample = {}; m_LastMemoryPollTick = 0;
    ComPtr<IDXGIDevice> dxgiDevice; ComPtr<IDXGIAdapter> adapter;
    if (SUCCEEDED(m_pDevice.As(&dxgiDevice)) && SUCCEEDED(dxgiDevice->GetAdapter(adapter.GetAddressOf())))
    {
        DXGI_ADAPTER_DESC desc{};
        if (SUCCEEDED(adapter->GetDesc(&desc)))
        { m_MemoryAdapterIdentityValid = true; m_MemoryAdapterLuid = desc.AdapterLuid; }
        (void)adapter.As(&m_pMemoryAdapter);
    }
    m_GpuQueriesAvailable = Create_GpuQueries();
    return S_OK;
}

// G02 호출자 Client 메인 루프. UI Capture/Reset/detail 요청을 여기서만 확정한다.
// QPC 시작점을 기록한 뒤 memory 저빈도 관측과 GPU query 시작을 수행한다.
// 첫 capture 프레임 interval은 0으로 두며, 정지 기간을 긴 프레임으로 오인하지 않는다.
void CProfiler::Begin_Frame()
{
    // Capture 정지 중에도 실제 루프 경계 횟수는 증가시킨다. GPU 회수 지연은 수집 프레임 번호와 분리한다.
    ++m_PollFrameNumber;
    if (m_ResetRequested.exchange(false, std::memory_order_relaxed))
        Reset_History();
    const bool enabled = m_Enabled.load(std::memory_order_relaxed);
    const bool previous = m_Collecting.exchange(enabled, std::memory_order_relaxed);
    if (previous != enabled)
    {
        m_CaptureEpoch.fetch_add(1, std::memory_order_relaxed);
        m_PreviousFrameBeginTick = 0;
        m_PreviousFrameEndTick = 0;
        for (auto& counter : m_AtomicCounters) counter.store(0, std::memory_order_relaxed);
        std::lock_guard lock(m_Mutex);
        m_PendingScopes.clear();
        m_PendingDroppedCpuScopes = 0;
    }
    if (!enabled) return;

    m_FrameActive = true;
    ++m_FrameNumber;
    {
        std::lock_guard lock(m_Mutex);
        m_SharedFrameNumber = m_FrameNumber;
    }
    m_CurrentFrame = {};
    m_CurrentFrame.FrameNumber = m_FrameNumber;
    const bool detail = m_RequestedDetailedScopes.load(std::memory_order_relaxed);
    m_DetailedScopes.store(detail, std::memory_order_relaxed);
    m_CurrentFrame.DetailedCpuScopes = detail;
    m_CurrentFrame.GpuScopesSupported = m_GpuScopeQueriesAvailable;
    m_ModelAnimationWork.clear();
    m_SubmittedModels.clear();
    m_SubmittedMeshKeys.fill(nullptr);
    m_SubmittedMeshCount = 0;
    m_FrameBeginTick = Query_Tick();
    m_CurrentFrame.FrameBeginTick = m_FrameBeginTick;
    if (m_PreviousFrameBeginTick != 0 && m_FrameBeginTick >= m_PreviousFrameBeginTick)
    {
        m_CurrentFrame.FrameIntervalMs = Ticks_ToMs(m_FrameBeginTick - m_PreviousFrameBeginTick);
        if (m_PreviousFrameEndTick >= m_PreviousFrameBeginTick &&
            m_FrameBeginTick >= m_PreviousFrameEndTick)
        {
            m_CurrentFrame.PreviousCpuFrameMs = Ticks_ToMs(m_PreviousFrameEndTick - m_PreviousFrameBeginTick);
            m_CurrentFrame.FrameGapMs = Ticks_ToMs(m_FrameBeginTick - m_PreviousFrameEndTick);
        }
    }
    m_PreviousFrameBeginTick = m_FrameBeginTick;
    Sample_Memory(m_FrameBeginTick);
    Begin_GpuFrame(m_FrameNumber);
}

// G03 Update/Render/Present가 반환한 뒤 호출된다. 먼저 CPU 끝 QPC를 찍는다.
// CpuFrameMs에는 프레임 안의 동기 대기가 들어가지만 이 시점 뒤의 집계/commit/회수 일부는 들어가지 않는다.
// 그 비용과 루프 밖 메시지 처리/FPS 제한은 다음 FrameInterval/FrameGap에 나타날 수 있다.
// 완료 CPU 표본을 가져오고 GPU 종료 명령 제출 -> history commit -> 오래된 GPU 결과 회수 순서다.
void CProfiler::End_Frame()
{
    if (!m_FrameActive)
    {
        Resolve_GpuFrames(m_PollFrameNumber);
        return;
    }
    m_FrameActive = false;

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.FrameEndTick = endTick;
    m_CurrentFrame.Memory = m_MemorySample;
    if (m_CurrentFrame.Memory.Sampled && endTick >= m_CurrentFrame.Memory.SampleTick)
        m_CurrentFrame.Memory.AgeMs = Ticks_ToMs(endTick - m_CurrentFrame.Memory.SampleTick);
    m_PreviousFrameEndTick = endTick;
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
        /* 어떤 스레드든 이전 End_Frame 뒤에 완료한 scope는 이번 프레임의 완료 표본으로 옮긴다. */
        std::lock_guard lock(m_Mutex);
        m_CurrentFrame.DroppedCpuScopes = m_PendingDroppedCpuScopes;
        m_PendingDroppedCpuScopes = 0;
        m_CurrentFrame.CpuScopes = std::move(m_PendingScopes);
        m_PendingScopes.clear();
        if (m_History.size() < MAX_HISTORY_FRAMES)
            m_PendingScopes.reserve(128);
    }

    End_GpuFrame(m_FrameNumber);
    Commit_CurrentFrame();
    Resolve_GpuFrames(m_PollFrameNumber);
}

// G04 초당 최대 1회 OS 관측. 프로세스 private commit/working set, 시스템 commit/available, DXGI budget을 구분한다.
// 실패는 Valid=false이며 0 사용량으로 해석하지 않는다. 여러 프레임이 같은 sample을 공유한다.
void CProfiler::Sample_Memory(uint64_t tick)
{
    // Begin_Frame calls this only with Capture ON. Pausing and resetting history
    // do not reset the wall-clock throttle. No GPU query, flush or readback.
    if (m_LastMemoryPollTick != 0 && tick >= m_LastMemoryPollTick &&
        tick - m_LastMemoryPollTick < static_cast<uint64_t>(m_Frequency.QuadPart)) return;
    m_LastMemoryPollTick = tick;
    CProfilerScope memoryCost(this, "Profiler.Memory.Sample");
    FProfilerMemoryStats sampled{};
    sampled.Sampled = true; sampled.SampleFrameNumber = m_FrameNumber; sampled.SampleTick = tick;
    sampled.ProcessId = GetCurrentProcessId();
    sampled.AdapterIdentityValid = m_MemoryAdapterIdentityValid;
    sampled.AdapterLuidLow = m_MemoryAdapterLuid.LowPart;
    sampled.AdapterLuidHigh = m_MemoryAdapterLuid.HighPart;
    PROCESS_MEMORY_COUNTERS_EX process{}; process.cb = sizeof(process);
    if (K32GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&process), sizeof(process)))
    {
        sampled.ProcessValid = true;
        sampled.PrivateCommitBytes = process.PrivateUsage;
        sampled.WorkingSetBytes = process.WorkingSetSize;
        sampled.PeakWorkingSetBytes = process.PeakWorkingSetSize;
        sampled.PeakPrivateCommitBytes = process.PeakPagefileUsage;
    }
    PERFORMANCE_INFORMATION system{}; system.cb = sizeof(system);
    if (K32GetPerformanceInfo(&system, sizeof(system)) && system.PageSize != 0)
    {
        sampled.SystemValid = true;
        sampled.SystemCommitBytes = static_cast<uint64_t>(system.CommitTotal) * system.PageSize;
        sampled.SystemCommitLimitBytes = static_cast<uint64_t>(system.CommitLimit) * system.PageSize;
        sampled.SystemAvailableBytes = static_cast<uint64_t>(system.PhysicalAvailable) * system.PageSize;
    }
    if (m_pMemoryAdapter)
    {
        const auto read = [&](DXGI_MEMORY_SEGMENT_GROUP group, FProfilerMemorySegment& segment)
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO info{};
            if (SUCCEEDED(m_pMemoryAdapter->QueryVideoMemoryInfo(0, group, &info)))
            { segment.Valid = true; segment.CurrentUsageBytes = info.CurrentUsage; segment.BudgetBytes = info.Budget; }
        };
        read(DXGI_MEMORY_SEGMENT_GROUP_LOCAL, sampled.Local);
        read(DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, sampled.NonLocal);
    }
    // A failed attempt replaces the previous values with unavailable rather
    // than silently presenting the last successful sample as a fresh success.
    m_MemorySample = sampled;
}

void CProfiler::Set_Enabled(bool enabled) noexcept
{
    m_Enabled.store(enabled, std::memory_order_relaxed);
}

bool CProfiler::Is_Enabled() const noexcept
{
    return m_Enabled.load(std::memory_order_relaxed);
}

// G05 프레임 중 호출은 요청만 저장한다. 프레임 밖에서 epoch/history/counter를 초기화한다.
// epoch 재확인 덕분에 Reset 전에 시작한 worker 표본이 새 history에 들어오지 않는다.
void CProfiler::Reset_History()
{
    if (m_FrameActive.load(std::memory_order_relaxed))
    {
        m_ResetRequested.store(true, std::memory_order_relaxed);
        return;
    }
    m_CaptureEpoch.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard lock(m_Mutex);
    m_History.clear();
    m_EvictedHistoryFrames = 0;
    m_MemorySample = {}; // No sample from before Reset is attributed to the new history.
    m_PendingScopes.clear();
    m_LongOperations.clear();
    m_DroppedCpuScopes = 0;
    m_PendingDroppedCpuScopes = 0;
    m_PreviousFrameBeginTick = 0;
    m_PreviousFrameEndTick = 0;
    m_DroppedGpuFrames = 0;
    m_DroppedGpuScopes = 0;
    m_DroppedModelAnimationSamples = 0;
}

// G06 CPU 구간 시작: Capture OFF는 즉시 무효 token, ON은 TLS 중첩 깊이와 QPC를 기록한다.
// 64개 중첩 제한을 넘으면 누락 수를 기록한다. 이름 intern은 반복 문자열 저장을 줄인다.
uint32_t CProfiler::Begin_Scope(std::string_view name)
{
    if (!m_Collecting.load(std::memory_order_relaxed))
        return UINT32_MAX;
    FOpenScopeStack& openScopes = t_OpenScopes;
    if (openScopes.Count >= MAX_OPEN_SCOPES_PER_THREAD)
    {
        std::lock_guard lock(m_Mutex);
        ++m_DroppedCpuScopes;
        ++m_PendingDroppedCpuScopes;
        return UINT32_MAX;
    }

    FOpenScope open{};
    open.NameId = Intern_Name(name);
    open.Depth = openScopes.Count;
    open.BeginTick = Query_Tick();
    open.CaptureEpoch = m_CaptureEpoch.load(std::memory_order_relaxed);
    openScopes.Scopes[openScopes.Count] = open;
    return openScopes.Count++;
}

// G07 CPU 구간 종료: QPC 차이를 계산하고 원래 ThreadId/Depth를 완료 큐에 넣는다.
// 최대 8192 raw 표본 중 main pass/root 공간을 남긴다. 모든 함수가 자동 계측되는 샘플링 profiler는 아니다.
// 8ms 이상 완료 구간은 별도 long-operation ring에 남긴다. 아직 끝나지 않은 deadlock을 감지하는 장치는 아니다.
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
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        open.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed))
        return;

    FProfilerScopeSample sample{};
    sample.NameId = open.NameId;
    sample.Depth = open.Depth;
    sample.ThreadId = GetCurrentThreadId();
    sample.BeginTick = open.BeginTick;
    sample.EndTick = endTick;
    const double durationMs = Ticks_ToMs(endTick - open.BeginTick);

    std::lock_guard lock(m_Mutex);
    // worker가 mutex를 기다리는 동안 Reset됐을 수 있으므로 잠금 획득 뒤 epoch를 다시 확인한다.
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        open.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed)) return;
    size_t sampleLimit = MAX_SCOPES_PER_FRAME - MAIN_PASS_SCOPE_RESERVE;
    if (sample.ThreadId == m_MainThreadId)
    {
        if (sample.Depth <= 2)
            sampleLimit = MAX_SCOPES_PER_FRAME;
        else if (sample.Depth == 3)
            sampleLimit = MAX_SCOPES_PER_FRAME - MAIN_ROOT_SCOPE_RESERVE;
    }
    if (m_PendingScopes.size() < sampleLimit)
    {
        // Cap vector growth as well as sample count; normal frames still grow
        // on demand and the history ring continues to recycle their buffers.
        if (m_PendingScopes.size() == m_PendingScopes.capacity())
            m_PendingScopes.reserve((std::min)(MAX_SCOPES_PER_FRAME,
                (std::max)(size_t{128}, m_PendingScopes.capacity() * 2)));
        m_PendingScopes.push_back(sample);
    }
    else
    {
        ++m_DroppedCpuScopes;
        ++m_PendingDroppedCpuScopes;
    }
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

// G08 GPU pass 시작: 메인 thread + Capture + active slot 조건을 확인한다.
// timestamp는 End(query)로 기록한다. 선택 pass는 Pipeline query도 Begin하고 시작 draw counter를 저장한다.
uint32_t CProfiler::Begin_GpuScope(std::string_view name, bool collectPipeline)
{
    if (GetCurrentThreadId() != m_MainThreadId ||
        !m_Collecting.load(std::memory_order_relaxed) ||
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
    scope.DrawBegin = Read_DrawCounters();
    scope.Draw = {};
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

// G09 GPU pass 종료: CPU draw counter 차이와 GPU 종료 timestamp 명령을 함께 남긴다.
// DrawCalls/Indices는 계측된 제출량이고 GPU timestamp duration과 서로 다른 단위다.
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
            scope.Draw = DrawDifference(Read_DrawCounters(), scope.DrawBegin);
            m_pContext->End(scope.End.Get());
            if (scope.PipelineIndex != UINT32_MAX)
                m_pContext->End(slot.PassPipelines[scope.PipelineIndex].Get());
            scope.Ended = true;
        }
        return;
    }
}

// G10 raw 상세 없이 자주 호출되는 고정 작업을 누적하기 위한 가벼운 경로.
// 같은 instance/epoch/frame/main-thread 조건이 맞을 때만 End_Work가 값을 반영한다.
FProfilerWorkToken CProfiler::Begin_Work(EProfilerWork work) const noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive.load(std::memory_order_relaxed) ||
        GetCurrentThreadId() != m_MainThreadId ||
        static_cast<size_t>(work) >= static_cast<size_t>(EProfilerWork::Count))
        return {};
    return {Query_Tick(), m_FrameNumber,
        m_CaptureEpoch.load(std::memory_order_relaxed), m_InstanceId};
}

void CProfiler::End_Work(EProfilerWork work, FProfilerWorkToken token) noexcept
{
    if (token.BeginTick == 0 || !m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive.load(std::memory_order_relaxed) ||
        GetCurrentThreadId() != m_MainThreadId || token.InstanceId != m_InstanceId ||
        token.FrameNumber != m_FrameNumber ||
        token.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed))
        return;
    const size_t index = static_cast<size_t>(work);
    if (index >= m_CurrentFrame.CpuWork.size()) return;
    const uint64_t endTick = Query_Tick();
    if (endTick < token.BeginTick) return;
    auto& stats = m_CurrentFrame.CpuWork[index];
    ++stats.Calls;
    stats.CpuMs += Ticks_ToMs(endTick - token.BeginTick);
}

const char* CProfiler::Get_WorkName(EProfilerWork work) noexcept
{
    static constexpr const char* names[] = {
        "Map.Batch.Render", "Map.Batch.Visibility", "Map.Batch.Material",
        "Map.Batch.Pass", "Map.Batch.Draw", "Map.Object.Render", "Map.Water.Render",
        "Npc.Update", "Npc.LateUpdate", "Npc.Render",
        "Ambient.Visibility", "Ambient.Advance", "Ambient.Submit"
    };
    static_assert(std::size(names) == static_cast<size_t>(EProfilerWork::Count));
    const size_t index = static_cast<size_t>(work);
    return index < std::size(names) ? names[index] : "<invalid>";
}

FProfilerModelAnimationToken CProfiler::Begin_ModelAnimation() const noexcept
{
    if (GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Collecting.load(std::memory_order_relaxed))
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
        !m_Collecting.load(std::memory_order_relaxed) ||
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

void CProfiler::Record_ViewportPresent(const FProfilerViewportPresent& sample)
{
    if (GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Collecting.load(std::memory_order_relaxed)) return;
    Add_Counter(EProfilerCounter::ImGuiPresentAttempts);
    if (sample.Result == DXGI_ERROR_WAS_STILL_DRAWING)
        Add_Counter(EProfilerCounter::ImGuiPresentBusy);
    else if (FAILED(sample.Result)) Add_Counter(EProfilerCounter::ImGuiPresentFailures);
    else if (sample.Result == DXGI_STATUS_OCCLUDED) Add_Counter(EProfilerCounter::ImGuiPresentOccluded);
    if (m_CurrentFrame.ViewportPresents.size() >= 32)
    {
        ++m_CurrentFrame.DroppedViewportPresents;
        return;
    }
    m_CurrentFrame.ViewportPresents.push_back(sample);
}

// G11 실제 CMesh 제출 경계의 집계. 포인터는 프레임 안 중복제거 키이며 영구 asset ID가 아니다.
// 상세 mesh trace는 opt-in이며 최대512개. 개별 draw GPU 시간을 재는 경로는 아니다.
void CProfiler::Record_MeshSubmitted(const void* mesh, uint32_t indexCount, uint32_t instances,
    std::string_view meshName, uint32_t vertexCount, uint32_t materialSlot)
{
    if (!mesh || instances == 0u || !m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive.load(std::memory_order_relaxed) || GetCurrentThreadId() != m_MainThreadId)
        return;
    Add_Counter(EProfilerCounter::MeshDrawCalls);
    Add_Counter(EProfilerCounter::MeshInstances, instances);
    Add_Counter(EProfilerCounter::MeshIndices, uint64_t(indexCount) * instances);

    if (m_DetailedScopes.load(std::memory_order_relaxed))
    {
        if (m_CurrentFrame.MeshDraws.size() < MAX_MESH_DRAWS_PER_FRAME)
        {
            // Allocate one bounded block, only in the explicitly enabled detail mode.
            if (m_CurrentFrame.MeshDraws.empty())
                m_CurrentFrame.MeshDraws.reserve(MAX_MESH_DRAWS_PER_FRAME);
            FProfilerMeshDrawSample sample{};
            sample.MeshNameId = Intern_Name(meshName.empty() ? std::string_view("(unnamed mesh)") : meshName);
            sample.VertexCount = vertexCount;
            sample.IndexCount = indexCount;
            sample.Instances = instances;
            sample.MaterialSlot = materialSlot;
            if (m_ActiveGpuSlot != UINT32_MAX && m_CurrentFrame.DroppedGpuScopes == 0)
            {
                const auto& gpu = m_GpuSlots[m_ActiveGpuSlot];
                if (gpu.OpenScopeCount)
                    sample.PassNameId = gpu.Scopes[gpu.OpenScopes[gpu.OpenScopeCount - 1]].NameId;
            }
            m_CurrentFrame.MeshDraws.push_back(sample);
        }
        else ++m_CurrentFrame.DroppedMeshDraws;
    }

    constexpr size_t mask = MAX_UNIQUE_MESHES_PER_FRAME * 2 - 1;
    static_assert((mask & (mask + 1)) == 0);
    const uintptr_t address = reinterpret_cast<uintptr_t>(mesh);
    size_t slot = static_cast<size_t>((address >> 4) ^ (address >> 19)) & mask;
    while (m_SubmittedMeshKeys[slot] != nullptr)
    {
        if (m_SubmittedMeshKeys[slot] == mesh) return;
        slot = (slot + 1) & mask;
    }
    if (m_SubmittedMeshCount == MAX_UNIQUE_MESHES_PER_FRAME)
    {
        // Submission totals remain exact; unique count is a lower bound.
        Add_Counter(EProfilerCounter::DroppedMeshSamples);
        return;
    }
    m_SubmittedMeshKeys[slot] = mesh;
    ++m_SubmittedMeshCount;
    Add_Counter(EProfilerCounter::UniqueMeshes);
}

FProfilerDrawStats CProfiler::Read_DrawCounters() const noexcept
{
    const auto read = [this](EProfilerCounter counter)
    { return m_AtomicCounters[static_cast<size_t>(counter)].load(std::memory_order_relaxed); };
    return {read(EProfilerCounter::DrawCalls), read(EProfilerCounter::InstancedDrawCalls),
        read(EProfilerCounter::Instances), read(EProfilerCounter::Indices),
        read(EProfilerCounter::MeshDrawCalls), read(EProfilerCounter::MeshInstances),
        read(EProfilerCounter::MeshIndices)};
}

void CProfiler::Add_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].fetch_add(value, std::memory_order_relaxed);
}

void CProfiler::Set_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].store(value, std::memory_order_relaxed);
}

// G12 저장 소비자에게 mutex 아래 불변 복사본을 만든다. 저장 worker가 live history를 참조하지 않게 한다.
// 복사 시 Pending인 GPU 결과는 나중에 완료돼도 이미 만든 snapshot에서 자동 갱신되지 않는다.
FProfilerCaptureSnapshot CProfiler::Snapshot(size_t frameWindow) const
{
    std::lock_guard lock(m_Mutex);
    FProfilerCaptureSnapshot snapshot{};
    snapshot.ScopeNames = m_ScopeNames;
    snapshot.CaptureWindow = Get_CaptureWindowLocked(frameWindow);
    const size_t count = (std::min)(frameWindow, m_History.size());
    snapshot.Frames.assign(m_History.end() - count, m_History.end());
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

FProfilerCaptureWindow CProfiler::Get_CaptureWindow(size_t frameWindow) const
{
    std::lock_guard lock(m_Mutex);
    return Get_CaptureWindowLocked(frameWindow);
}

// G13 저장 범위와 퇴출 범위를 같은 잠금에서 계산해 snapshot과 coverage가 일치하게 한다.
FProfilerCaptureWindow CProfiler::Get_CaptureWindowLocked(size_t frameWindow) const
{
    FProfilerCaptureWindow window{};
    window.RequestedFrames = static_cast<uint64_t>(frameWindow);
    window.RetainedFrames = static_cast<uint64_t>(m_History.size());
    const size_t count = (std::min)(frameWindow, m_History.size());
    const size_t excluded = m_History.size() - count;
    window.SavedFrames = static_cast<uint64_t>(count);
    window.ExcludedRetainedFrames = static_cast<uint64_t>(excluded);
    window.EvictedFramesSinceReset = m_EvictedHistoryFrames;
    if (!m_History.empty())
    {
        window.FirstRetainedFrameNumber = m_History.front().FrameNumber;
        window.LastRetainedFrameNumber = m_History.back().FrameNumber;
    }
    if (count != 0)
    {
        window.FirstSavedFrameNumber = m_History[excluded].FrameNumber;
        window.LastSavedFrameNumber = m_History.back().FrameNumber;
    }
    for (size_t index = 0; index < excluded; ++index)
    {
        const double interval = m_History[index].FrameIntervalMs;
        if (std::isfinite(interval))
            window.ExcludedMaxFrameIntervalMs = (std::max)(window.ExcludedMaxFrameIntervalMs, interval);
    }
    return window;
}

void CProfiler::Get_MemorySamples(size_t maxFrames, std::vector<FProfilerMemoryStats>& outSamples) const
{
    std::lock_guard lock(m_Mutex);
    outSamples.clear();
    const size_t count = (std::min)(maxFrames, m_History.size());
    const uint64_t now = Query_Tick();
    for (size_t i = 0; i < count; ++i)
    {
        const auto& memory = m_History[m_History.size() - 1u - i].Memory;
        if (!memory.Sampled) continue;
        if (!outSamples.empty())
        {
            const auto& previous = outSamples.back();
            if (previous.SampleTick == memory.SampleTick && previous.ProcessId == memory.ProcessId &&
                previous.SampleFrameNumber == memory.SampleFrameNumber) continue;
        }
        outSamples.push_back(memory);
        if (now >= memory.SampleTick) outSamples.back().AgeMs = Ticks_ToMs(now - memory.SampleTick);
    }
    std::reverse(outSamples.begin(), outSamples.end());
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
    outStats.DroppedCpuScopes = latest.DroppedCpuScopes;
    outStats.DetailedCpuScopes = latest.DetailedCpuScopes;
    outStats.CpuFrameMs = latest.CpuFrameMs;
    outStats.FrameIntervalMs = latest.FrameIntervalMs;
    outStats.FrameBeginTick = latest.FrameBeginTick;
    outStats.FrameEndTick = latest.FrameEndTick;
    outStats.Memory = latest.Memory;
    const uint64_t now = Query_Tick();
    if (outStats.Memory.Sampled && now >= outStats.Memory.SampleTick)
        outStats.Memory.AgeMs = Ticks_ToMs(now - outStats.Memory.SampleTick);
    outStats.PreviousCpuFrameMs = latest.PreviousCpuFrameMs;
    outStats.FrameGapMs = latest.FrameGapMs;
    outStats.Animation = latest.Animation;
    outStats.CpuWork = latest.CpuWork;
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

// G14 CPU 이름+스레드 집계: inclusive는 전체 구간, self는 같은 스레드의 계측된 자식을 차감한다.
// 다른 스레드의 work를 부모에서 빼지 않는다. frame 경계 침범/누락이 있으면 SelfComplete=false다.
// API 결과의 기본 정렬은 inclusive이며 ProfilerTool의 기본 병목 표는 main-thread self로 다시 본다.
void CProfiler::Get_ScopeAggregates(
    size_t frameWindow,
    std::vector<FProfilerScopeAggregate>& outAggregates) const
{
    outAggregates.clear();
    std::lock_guard lock(m_Mutex);
    if (m_History.empty() || 0 == frameWindow)
        return;

    // 스레드별 완료 순서는 자식 다음 부모다. 부모가 도착하면 완료된 자식을 차감한다.
    // 매 패널 갱신마다 모든 raw 표본을 정렬하지 않으며 이름은 조밀한 intern ID로 찾는다.
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
    bool selfComplete = true;
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        const FProfilerFrame& frame = m_History[frameIndex];
        const bool boundsKnown = frame.FrameBeginTick != 0 && frame.FrameEndTick >= frame.FrameBeginTick;
        selfComplete = selfComplete && frame.DroppedCpuScopes == 0 && boundsKnown;
        for (FThreadReduction& thread : threads) thread.CompletedCount = 0;
        const auto& frameScopes = frame.CpuScopes;
        const size_t scopeCount = frameScopes.size();
        const FProfilerScopeSample* scopes = frameScopes.data();
        for (size_t index = 0; index < scopeCount; ++index)
        {
            const FProfilerScopeSample& sample = scopes[index];
            // 완료 프레임에 귀속되므로 경계를 넘는 부모의 자식은 다른 프레임에 있을 수 있다.
            // 이 경우 프레임별 self는 확정하지 않고 inclusive만 보존한다.
            if (sample.BeginTick < frame.FrameBeginTick || sample.EndTick > frame.FrameEndTick ||
                sample.EndTick < sample.BeginTick) selfComplete = false;
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
        for (FProfilerScopeAggregate aggregate : thread.ByName)
            if (aggregate.Calls != 0)
            {
                // Even a name absent from an incomplete frame may have lost calls.
                aggregate.SelfComplete = selfComplete;
                outAggregates.push_back(aggregate);
            }
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerScopeAggregate& left, const FProfilerScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

// G15 GPU pass 평균/p95: GpuValid + scope 지원 + dropped scope 0인 동일 프레임들만 분모로 쓴다.
// 같은 pass 여러 호출은 프레임 단위로 합친다. 유효 프레임에서 호출 없는 pass는 0을 채운다.
// p95는 정렬한 N개 중 ceil(0.95*N)-1 인덱스다. Benchmark의 선형보간 p95와 방식이 다르다.
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
        std::vector<double> SelfFrameTimes;
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
            value.Aggregate.SelfMs += sample.SelfMs;
            AddDrawStats(value.Aggregate.Draw, sample.Draw);
            if (sample.PipelineValid)
            {
                ++value.Aggregate.PipelineSamples;
                value.Aggregate.PSInvocations += sample.PSInvocations;
                value.Aggregate.VSInvocations += sample.VSInvocations;
                value.Aggregate.IAVertices += sample.IAVertices;
                value.Aggregate.IAPrimitives += sample.IAPrimitives;
            }
            value.FrameTimes.resize(frameIndex + 1, 0.0);
            value.FrameTimes[frameIndex] += sample.DurationMs;
            value.SelfFrameTimes.resize(frameIndex + 1, 0.0);
            value.SelfFrameTimes[frameIndex] += sample.SelfMs;
        }
    }
    for (auto& [name, value] : accumulated)
    {
        value.FrameTimes.resize(outValidFrames, 0.0);
        value.SelfFrameTimes.resize(outValidFrames, 0.0);
        std::sort(value.FrameTimes.begin(), value.FrameTimes.end());
        std::sort(value.SelfFrameTimes.begin(), value.SelfFrameTimes.end());
        value.Aggregate.MaxFrameMs = value.FrameTimes.back();
        value.Aggregate.SelfMaxFrameMs = value.SelfFrameTimes.back();
        const size_t percentile = static_cast<size_t>(
            std::ceil(static_cast<double>(outValidFrames) * 0.95)) - 1;
        value.Aggregate.P95FrameMs = value.FrameTimes[percentile];
        value.Aggregate.SelfP95FrameMs = value.SelfFrameTimes[percentile];
        outAggregates.push_back(value.Aggregate);
    }
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerGpuScopeAggregate& left, const FProfilerGpuScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

// G16 CPU 평균은 선택된 CPU 프레임 수, GPU 평균은 그중 GPU Valid인 프레임 수로 나눈다.
// GPU 분모가 0이면 UI가 N/A로 표시해야 하며 outGpuAvgMs 초기값0을 측정0ms로 읽으면 안 된다.
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

// G17 QPC wall elapsed 계측. CPU 명령 실행뿐 아니라 sleep/락/드라이버 대기도 구간 안이면 포함된다.
uint64_t CProfiler::Query_Tick() const noexcept
{
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return static_cast<uint64_t>(value.QuadPart);
}

// G18 스레드별 이름 cache와 전역 ID표. raw 표본마다 긴 문자열을 복제하지 않기 위한 선택이다.
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

// G19 D3D11 TimestampDisjoint+Timestamp로 시간, PipelineStatistics로 IA/VS/PS 작업량을 잰다.
// PSInvocations는 실행된 shader invocation 수이며 shader 명령 수/ALU 점유율과 같지 않다.
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
    // pass query 생성이 실패해도 전체 프레임 GPU 시간 계측은 남길 수 있다.
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
    // 128개 모든 scope가 아니라 선택한 pass들에만 pipeline query를 사용한다.
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

// G20 frameNumber % 8 slot을 선택한다. 이전 query가 미완료면 기다리지 않고 새 GPU 프레임을 누락 처리한다.
// GPU 프레임 시작 timestamp가 GPU 명령열에 들어간다. 이 구간에는 CPU 명령 공급 공백도 반영될 수 있다.
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

// G21 프레임 종료 query를 제출한다. 완료를 기다리지 않으며 FrameEnded는 제출 끝이지 GPU 실행 완료가 아니다.
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
        // 남은 query는 종료하지만 정상 종료가 누락된 구간은 완전한 시간 표본에서 제외한다.
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

// G22 최소4 실제 프레임 뒤 GetData(DONOTFLUSH)로 회수한다. S_FALSE는 다음번에 다시 본다.
// Disjoint/주파수0/역전 timestamp는 무효. 유효 ms는 (end-begin)*1000/disjoint.Frequency다.
// 이전 frame ID로 history를 찾아 결과를 붙인다. CPU의 이번 프레임 GPU 시간으로 섞지 않는다.
// GPU Self는 직접 자식 timestamp 차감이다. 실행 busy cycle이나 shader ALU 전용 시간이 아니다.
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
                sample.SelfMs = sample.DurationMs;
                sample.Draw = scope.Draw;
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
                        sample.IAVertices = passPipeline.IAVertices;
                        sample.IAPrimitives = passPipeline.IAPrimitives;
                    }
                }
            }
            if (status != EProfilerGpuFrameStatus::Error && !ready)
                continue;
            // Begin 순 표본이다. immediate context 구간은 직렬 또는 중첩이므로 직접 자식만 한 번 뺀다.
            for (uint32_t parent = 0; parent < sampleCount; ++parent)
            {
                auto& sample = samples[parent];
                for (uint32_t child = parent + 1; child < sampleCount; ++child)
                {
                    if (samples[child].Depth <= sample.Depth) break;
                    if (samples[child].Depth == sample.Depth + 1)
                        sample.SelfMs -= samples[child].DurationMs;
                }
                sample.SelfMs = (std::max)(0.0, sample.SelfMs);
            }
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

// G23 최대1200개 프레임 ring. 메모리 무한 증가를 막고 퇴출 CPU vector 공간을 재사용한다.
// 퇴출 횟수는 export coverage에 남기며 오래된 hitch가 ring 밖으로 사라졌는지 판단하는 근거가 된다.
void CProfiler::Commit_CurrentFrame()
{
    std::lock_guard lock(m_Mutex);
    // End_Frame과 이 잠금 사이에 완료한 worker 표본을 보존한다. 비어 있을 때만 퇴출 vector를 재사용한다.
    if (m_History.size() >= MAX_HISTORY_FRAMES && m_PendingScopes.empty())
    {
        m_PendingScopes.swap(m_History.front().CpuScopes);
        m_PendingScopes.clear();
    }
    m_History.push_back(std::move(m_CurrentFrame));
    while (m_History.size() > MAX_HISTORY_FRAMES)
    {
        m_History.pop_front();
        ++m_EvictedHistoryFrames;
    }
}
