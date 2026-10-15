// Profiler.cpp만 포함한다. 별도 번역 단위가 아니므로 ClCompile 등록은 필요 없다.
// Capture history와 번호/수집 상태를 공유하지 않아 F7을 닫아도 HUD 계측은 계속된다.
// 완료된 staging 버퍼의 byte를 CPU 보관 배열로 복사한다.
#include <cstring>
// 주 스레드가 query ring과 최근 완료 결과를 소유한다. GPU가 끝내지 않은 slot은 재사용하지 않는다.
struct CProfiler::FGeometryState final
{
    static constexpr uint32_t RingSize = 8u, ReadLatency = 4u;
    static constexpr uint32_t MaxBuffers = 16u, MaxDraws = 16384u;
    static constexpr uint32_t MaxReadbackBytes = 4u * 1024u * 1024u;
    static constexpr size_t MaxHistory = 128u, MaxDetailHistory = 16u;
    struct Buffer final
    {
        // Source를 완료까지 보유해 재사용된 주소를 다른 frame 입력과 혼동하지 않는다.
        ComPtr<ID3D11Buffer> Source, Staging;
        uint32_t Bytes = 0u;
        std::vector<uint32_t> Words;
    };
    struct Draw final
    {
        uint32_t BufferOrdinal = 0u, Offset = 0u;
    };
    struct Slot final
    {
        ComPtr<ID3D11Query> Pipeline, Completion;
        std::array<Buffer, MaxBuffers> Buffers{};
        std::vector<Draw> Draws;
        FProfilerGeometryStats Stats{};
        uint32_t BufferCount = 0u, ReadbackBytes = 0u;
        bool Pending = false, PipelineBegun = false, DetailRequested = false;
        bool ReadbackValid = true;
    };
    struct Detail final
    {
        uint64_t FrameNumber = 0u;
        std::vector<FProfilerIndirectArgument> Arguments;
    };
    std::array<Slot, RingSize> Slots{};
    std::deque<FProfilerGeometryStats> History;
    std::deque<Detail> Details;
    uint64_t FrameNumber = 0u, DroppedFrames = 0u;
    uint32_t ActiveSlot = UINT32_MAX;
    bool FrameActive = false;
};

// unique_ptr가 해제할 내부 상태의 정의 뒤에서 소멸자를 완성한다.
CProfiler::~CProfiler() = default;

// Profiler 초기화가 capture와 독립인 HUD 계측 자원을 준비한다. 생성 실패는 계측 불가로만 남긴다.
void CProfiler::Initialize_Geometry()
{
    // 계측 자원 실패가 렌더러 초기화를 실패시키지 않는다. 조회는 unavailable을 보인다.
    try
    {
        auto state = std::make_unique<FGeometryState>();
        for (auto& slot : state->Slots)
        {
            slot.Draws.reserve(FGeometryState::MaxDraws);
            D3D11_QUERY_DESC query{D3D11_QUERY_PIPELINE_STATISTICS, 0u};
            (void)m_pDevice->CreateQuery(&query, slot.Pipeline.GetAddressOf());
            query.Query = D3D11_QUERY_EVENT;
            (void)m_pDevice->CreateQuery(&query, slot.Completion.GetAddressOf());
        }
        m_Geometry = std::move(state);
    }
    catch (...)
    {
        m_Geometry.reset();
    }
}

// Begin_Frame이 증가시킨 geometry 번호다. 아직 GPU가 완료한 번호와는 다르다.
uint64_t CProfiler::Get_GeometryFrameNumber() const noexcept
{
    return m_Geometry ? m_Geometry->FrameNumber : 0u;
}

// HUD가 가장 최근 완료 결과를 읽는다. 출력은 먼저 비우며 완료 기록이 없으면 false다.
bool CProfiler::Get_GeometryStats(FProfilerGeometryStats& output) const noexcept
{
    output = {};
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId)
    {
        return false;
    }
    const auto& state = *m_Geometry;
    output.CurrentFrameNumber = state.FrameNumber;
    output.DroppedFrames = state.DroppedFrames;
    output.Pending = std::any_of(state.Slots.begin(),
                                 state.Slots.end(),
                                 [](const auto& slot)
                                 {
                                     return slot.Pending;
                                 });
    if (state.History.empty())
    {
        return false;
    }
    const bool pending = output.Pending;
    output = state.History.back();
    output.CurrentFrameNumber = state.FrameNumber;
    output.LatencyFrames =
        static_cast<uint32_t>((std::min)(uint64_t(UINT32_MAX), state.FrameNumber - output.FrameNumber));
    output.DroppedFrames = state.DroppedFrames;
    output.Pending = pending;
    return true;
}

// 컷신 기록이 CPU 샘플과 같은 geometry frame을 찾아 결합한다. 보관 범위 밖이면 출력을 비우고 false다.
bool CProfiler::Get_GeometryStatsForFrame(uint64_t frameNumber, FProfilerGeometryStats& output) const noexcept
{
    output = {};
    if (!m_Geometry || !frameNumber || GetCurrentThreadId() != m_MainThreadId)
    {
        return false;
    }
    const auto& state = *m_Geometry;
    const auto found = std::find_if(state.History.begin(),
                                    state.History.end(),
                                    [frameNumber](const auto& frame)
                                    {
                                        return frame.FrameNumber == frameNumber;
                                    });
    if (found == state.History.end())
    {
        return false;
    }
    output = *found;
    output.CurrentFrameNumber = state.FrameNumber;
    output.LatencyFrames = static_cast<uint32_t>((std::min)(uint64_t(UINT32_MAX), state.FrameNumber - frameNumber));
    return true;
}

// 현재 진행 중인 slot에만 상세 args 보관을 예약한다. 이미 끝난 frame의 상세 기록은 만들지 않는다.
void CProfiler::Request_GeometryDetailForCurrentFrame() noexcept
{
    if (m_Geometry && GetCurrentThreadId() == m_MainThreadId && m_Geometry->FrameActive &&
        m_Geometry->ActiveSlot != UINT32_MAX)
    {
        m_Geometry->Slots[m_Geometry->ActiveSlot].DetailRequested = true;
    }
}

// 완료 시 보관한 상세 args를 복사한다. 별도 GPU readback은 시작하지 않으며 없으면 빈 출력을 반환한다.
bool CProfiler::Get_GeometryIndirectArguments(uint64_t frameNumber,
                                              std::vector<FProfilerIndirectArgument>& output) const
{
    output.clear();
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId)
    {
        return false;
    }
    const auto& details = m_Geometry->Details;
    const auto found = std::find_if(details.begin(),
                                    details.end(),
                                    [frameNumber](const auto& frame)
                                    {
                                        return frame.FrameNumber == frameNumber;
                                    });
    if (found == details.end())
    {
        return false;
    }
    output = found->Arguments;
    return true;
}

// 현재 frame 안의 args 버퍼 번호를 조회한다. 아직 기록되지 않은 버퍼는 UINT32_MAX다.
uint32_t CProfiler::Get_GeometryIndirectBufferOrdinal(ID3D11Buffer* arguments) const noexcept
{
    if (!m_Geometry || !arguments || GetCurrentThreadId() != m_MainThreadId || !m_Geometry->FrameActive ||
        m_Geometry->ActiveSlot == UINT32_MAX)
    {
        return UINT32_MAX;
    }
    const auto& slot = m_Geometry->Slots[m_Geometry->ActiveSlot];
    for (uint32_t index = 0u; index < slot.BufferCount; ++index)
    {
        if (slot.Buffers[index].Source.Get() == arguments)
        {
            return index;
        }
    }
    return UINT32_MAX;
}

// 이전 GPU 결과를 먼저 회수한 뒤 이번 frame의 빈 slot을 연다. ring이 밀리면 기다리지 않고 이번 계측만 생략한다.
void CProfiler::Begin_GeometryFrame() noexcept
{
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId)
    {
        return;
    }
    auto& state = *m_Geometry;
    ++state.FrameNumber;
    Resolve_GeometryFrames();
    state.ActiveSlot = UINT32_MAX;
    state.FrameActive = true;
    auto& slot = state.Slots[state.FrameNumber % FGeometryState::RingSize];
    // 읽지 못한 GPU 자원은 덮어쓰거나 기다리지 않는다. 이번 frame만 누락한다.
    if (slot.Pending)
    {
        ++state.DroppedFrames;
        return;
    }
    state.ActiveSlot = static_cast<uint32_t>(state.FrameNumber % FGeometryState::RingSize);
    for (auto& buffer : slot.Buffers)
    {
        buffer.Source.Reset();
    }
    slot.BufferCount = slot.ReadbackBytes = 0u;
    slot.Draws.clear();
    slot.Stats = {};
    slot.Stats.FrameNumber = state.FrameNumber;
    slot.ReadbackValid = true;
    slot.DetailRequested = false;
    slot.PipelineBegun = slot.Pipeline != nullptr;
    if (slot.PipelineBegun)
    {
        m_pContext->Begin(slot.Pipeline.Get());
    }
}

// Add_Counter(Indices)가 전달한 CPU 확정 제출량을 합산한다. capture를 꺼도 이 경로는 유지된다.
void CProfiler::Add_GeometryDirectIndices(uint64_t count) noexcept
{
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId || !m_Geometry->FrameActive ||
        m_Geometry->ActiveSlot == UINT32_MAX)
    {
        return;
    }
    auto& slot = m_Geometry->Slots[m_Geometry->ActiveSlot];
    if (count > UINT64_MAX - slot.Stats.DirectIndices)
    {
        slot.ReadbackValid = false;
        return;
    }
    slot.Stats.DirectIndices += count;
}

// CMesh가 제출한 args 위치와 버퍼 수명을 기록한다. 실제 instance 수는 GPU 완료 뒤에 읽는다.
void CProfiler::Record_GeometryIndirectDraw(ID3D11Buffer* arguments, uint32_t argsByteOffset) noexcept
{
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId || !m_Geometry->FrameActive ||
        m_Geometry->ActiveSlot == UINT32_MAX)
    {
        return;
    }
    auto& slot = m_Geometry->Slots[m_Geometry->ActiveSlot];
    ++slot.Stats.IndirectDrawCalls;
    if (!arguments || argsByteOffset % 4u || slot.Draws.size() == FGeometryState::MaxDraws)
    {
        slot.ReadbackValid = false;
        return;
    }
    uint32_t ordinal = Get_GeometryIndirectBufferOrdinal(arguments);
    if (ordinal == UINT32_MAX)
    {
        D3D11_BUFFER_DESC desc{};
        arguments->GetDesc(&desc);
        if (slot.BufferCount == FGeometryState::MaxBuffers || !desc.ByteWidth || desc.ByteWidth % 4u ||
            desc.ByteWidth > FGeometryState::MaxReadbackBytes - slot.ReadbackBytes ||
            !(desc.MiscFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS))
        {
            slot.ReadbackValid = false;
            return;
        }
        ordinal = slot.BufferCount++;
        auto& buffer = slot.Buffers[ordinal];
        buffer.Source = arguments;
        buffer.Bytes = desc.ByteWidth;
        slot.ReadbackBytes += desc.ByteWidth;
    }
    if (uint64_t(argsByteOffset) + 20u > slot.Buffers[ordinal].Bytes)
    {
        slot.ReadbackValid = false;
        return;
    }
    // Initialize에서 최대 개수를 reserve했으므로 hot path에 이름/노드 할당이 없다.
    slot.Draws.push_back({ordinal, argsByteOffset});
}

// frame의 query를 닫고 args 복사를 예약한다. 아직 결과를 기다리거나 CPU에 읽지 않는다.
void CProfiler::End_GeometryFrame() noexcept
{
    if (!m_Geometry || GetCurrentThreadId() != m_MainThreadId || !m_Geometry->FrameActive)
    {
        return;
    }
    auto& state = *m_Geometry;
    state.FrameActive = false;
    if (state.ActiveSlot == UINT32_MAX)
    {
        return;
    }
    auto& slot = state.Slots[state.ActiveSlot];
    state.ActiveSlot = UINT32_MAX;
    slot.Stats.CaptureFrameNumber = m_FrameActive ? m_FrameNumber : 0u;
    if (slot.PipelineBegun)
    {
        m_pContext->End(slot.Pipeline.Get());
    }
    try
    {
        // 한 buffer의 모든 제출 offset은 하나의 staging copy를 공유한다. 생산 경로는
        // 이 frame Dispatch 이후 args를 바꾸지 않으므로 실제 제출에 쓰인 값이 보존된다.
        for (uint32_t index = 0u; index < slot.BufferCount; ++index)
        {
            auto& buffer = slot.Buffers[index];
            D3D11_BUFFER_DESC current{};
            if (buffer.Staging)
            {
                buffer.Staging->GetDesc(&current);
            }
            if (!buffer.Staging || current.ByteWidth != buffer.Bytes)
            {
                D3D11_BUFFER_DESC desc{};
                desc.ByteWidth = buffer.Bytes;
                desc.Usage = D3D11_USAGE_STAGING;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                ComPtr<ID3D11Buffer> staged;
                if (FAILED(m_pDevice->CreateBuffer(&desc, nullptr, staged.GetAddressOf())))
                {
                    slot.ReadbackValid = false;
                    continue;
                }
                buffer.Staging = std::move(staged);
            }
            if (buffer.Words.size() != buffer.Bytes / sizeof(uint32_t))
            {
                std::vector<uint32_t>(buffer.Bytes / sizeof(uint32_t)).swap(buffer.Words);
            }
            m_pContext->CopyResource(buffer.Staging.Get(), buffer.Source.Get());
        }
        // 이전 frame보다 buffer 수가 줄었으면 미사용 고용량 staging도 반환한다.
        // 매 slot의 보존 메모리가 과거 서로 다른 크기의 합으로 커지지 않게 한다.
        for (uint32_t index = slot.BufferCount; index < FGeometryState::MaxBuffers; ++index)
        {
            auto& buffer = slot.Buffers[index];
            buffer.Staging.Reset();
            std::vector<uint32_t>().swap(buffer.Words);
            buffer.Bytes = 0u;
        }
    }
    catch (...)
    {
        slot.ReadbackValid = false;
    }
    if (slot.Completion)
    {
        m_pContext->End(slot.Completion.Get());
    }
    slot.Pending = true;
}

// 충분히 지난 slot만 비동기로 조회한다. VS 실행 수와 index 합의 유효성은 서로 독립적으로 남긴다.
void CProfiler::Resolve_GeometryFrames() noexcept
{
    auto& state = *m_Geometry;
    for (auto& slot : state.Slots)
    {
        if (!slot.Pending || state.FrameNumber < slot.Stats.FrameNumber + FGeometryState::ReadLatency)
        {
            continue;
        }
        bool ready = true, gpuError = false;
        if (slot.Completion)
        {
            BOOL complete = FALSE;
            const HRESULT result =
                m_pContext->GetData(slot.Completion.Get(), &complete, sizeof(complete), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (result == S_FALSE || (result == S_OK && !complete))
            {
                continue;
            }
            gpuError = FAILED(result);
        }
        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline{};
        bool pipelineValid = false;
        if (!gpuError && slot.PipelineBegun)
        {
            const HRESULT result =
                m_pContext->GetData(slot.Pipeline.Get(), &pipeline, sizeof(pipeline), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (result == S_FALSE)
            {
                continue;
            }
            pipelineValid = SUCCEEDED(result);
        }
        bool indicesValid = slot.ReadbackValid && !gpuError;
        if (indicesValid)
        {
            for (uint32_t index = 0u; index < slot.BufferCount; ++index)
            {
                auto& buffer = slot.Buffers[index];
                if (!buffer.Staging || buffer.Words.size() != buffer.Bytes / sizeof(uint32_t))
                {
                    indicesValid = false;
                    break;
                }
                D3D11_MAPPED_SUBRESOURCE mapped{};
                const HRESULT result =
                    m_pContext->Map(buffer.Staging.Get(), 0u, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped);
                if (result == DXGI_ERROR_WAS_STILL_DRAWING)
                {
                    ready = false;
                    break;
                }
                if (FAILED(result))
                {
                    indicesValid = false;
                    break;
                }
                std::memcpy(buffer.Words.data(), mapped.pData, buffer.Bytes);
                m_pContext->Unmap(buffer.Staging.Get(), 0u);
            }
        }
        if (!ready)
        {
            continue;
        }
        FGeometryState::Detail detail;
        detail.FrameNumber = slot.Stats.FrameNumber;
        uint64_t indirectIndices = 0u;
        try
        {
            if (slot.DetailRequested && indicesValid)
            {
                detail.Arguments.reserve(slot.Draws.size());
            }
            if (indicesValid)
            {
                for (const auto& draw : slot.Draws)
                {
                    const auto& words = slot.Buffers[draw.BufferOrdinal].Words;
                    const uint32_t first = draw.Offset / sizeof(uint32_t);
                    // DrawIndexedInstancedIndirect의 앞 두 값은 instance당 index 수와 실제 instance 수다.
                    const uint64_t indices = uint64_t(words[first]) * words[first + 1u];
                    if (indices > UINT64_MAX - indirectIndices)
                    {
                        indicesValid = false;
                        break;
                    }
                    indirectIndices += indices;
                    if (slot.DetailRequested)
                    {
                        detail.Arguments.push_back({draw.BufferOrdinal, draw.Offset, words[first], words[first + 1u], 1u});
                    }
                }
            }
            if (indirectIndices > UINT64_MAX - slot.Stats.DirectIndices)
            {
                indicesValid = false;
            }
            slot.Stats.VSInvocations = pipelineValid ? pipeline.VSInvocations : 0u;
            slot.Stats.VertexValid = pipelineValid;
            slot.Stats.IndexValid = indicesValid;
            slot.Stats.IndirectIndices = indicesValid ? indirectIndices : 0u;
            slot.Stats.TotalIndices = indicesValid ? slot.Stats.DirectIndices + indirectIndices : 0u;
            slot.Stats.CurrentFrameNumber = state.FrameNumber;
            slot.Stats.LatencyFrames =
                static_cast<uint32_t>((std::min)(uint64_t(UINT32_MAX), state.FrameNumber - slot.Stats.FrameNumber));
            slot.Stats.DroppedFrames = state.DroppedFrames;
            // 느린 slot이 늦게 도착해도 latest는 frame 번호가 가장 큰 결과다.
            const auto position = std::upper_bound(state.History.begin(),
                                                   state.History.end(),
                                                   slot.Stats.FrameNumber,
                                                   [](uint64_t frame, const auto& value)
                                                   {
                                                       return frame < value.FrameNumber;
                                                   });
            state.History.insert(position, slot.Stats);
            while (state.History.size() > FGeometryState::MaxHistory)
            {
                state.History.pop_front();
            }
            if (slot.DetailRequested && indicesValid)
            {
                auto& rows = detail.Arguments;
                std::sort(rows.begin(),
                          rows.end(),
                          [](const auto& a, const auto& b)
                          {
                              return a.BufferOrdinal != b.BufferOrdinal ? a.BufferOrdinal < b.BufferOrdinal
                                                                        : a.ArgsByteOffset < b.ArgsByteOffset;
                          });
                // 같은 args offset을 재사용한 draw는 한 행으로 합치되 제출 횟수를 보존한다.
                size_t count = 0u;
                for (const auto& row : rows)
                {
                    if (count && rows[count - 1u].BufferOrdinal == row.BufferOrdinal &&
                        rows[count - 1u].ArgsByteOffset == row.ArgsByteOffset)
                    {
                        rows[count - 1u].DrawCount += row.DrawCount;
                    }
                    else
                    {
                        rows[count++] = row;
                    }
                }
                rows.resize(count);
                state.Details.push_back(std::move(detail));
                while (state.Details.size() > FGeometryState::MaxDetailHistory)
                {
                    state.Details.pop_front();
                }
            }
        }
        catch (...)
        {
            ++state.DroppedFrames;
        }
        slot.Pending = false;
    }
}
