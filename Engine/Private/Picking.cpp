// 한 픽셀 동기/비동기 GPU 읽기와 요청 ID 수명을 구현한다.
#include "Picking.h"
// 현재 Target_PickPos SRV와 Profiler를 매 요청에 조회한다.
#include "GameInstance.h"
// copy/Map/메모리 읽기 시간을 측정하고 GPU 읽기 횟수와 byte 수를 기록한다.
#include "Profiler.h"

// Map으로 받은 한 픽셀의 byte를 지역 float4에 복사한다.
#include <cstring>
// 비동기 Poll이 읽은 world 위치의 xyz 유한성을 검사한다.
#include <cmath>

CPicking::CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{
}

CPicking::~CPicking()
{

}

// immediate context와 소유 스레드를 기록하고 CPU 읽기용 1x1 RGBA32F staging texture 두 개를 만든다.
// 동기/비동기 경로가 각 texture를 사용한다. 부분 생성 실패는 Create의 후보 해제가 정리한다.
HRESULT CPicking::Initialize(HWND hWnd)
{
    if (!m_pDevice || !m_pContext || m_pContext->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE)
        return E_INVALIDARG;
    m_hWnd = hWnd;
    m_iOwnerThreadId = GetCurrentThreadId();
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 1u;
    desc.Height = 1u;
    desc.MipLevels = 1u;
    desc.ArraySize = 1u;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count = 1u;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    const HRESULT hr = m_pDevice->CreateTexture2D(&desc, nullptr, &m_pTexture2D);
    return FAILED(hr) ? hr : m_pDevice->CreateTexture2D(&desc, nullptr, &m_pAsyncTexture);
}

// 동기 Picking이 빌린 RT와 현재 커서 픽셀을 전달한다. 같은 device/형식/크기/스레드 조건을 먼저 검사한다.
bool_t CPicking::Read_Pixel(ID3D11Texture2D* pSource, const uint32_t x,
    const uint32_t y, float4_t& vOut, CProfiler* pProfiler)
{
    if (!pSource || !m_pTexture2D || GetCurrentThreadId() != m_iOwnerThreadId)
        return false;
    ComPtr<ID3D11Device> sourceDevice;
    pSource->GetDevice(&sourceDevice);
    if (sourceDevice.Get() != m_pDevice.Get())
        return false;
    D3D11_TEXTURE2D_DESC sourceDesc{};
    pSource->GetDesc(&sourceDesc);
    if (sourceDesc.Format != DXGI_FORMAT_R32G32B32A32_FLOAT ||
        sourceDesc.SampleDesc.Count != 1u || sourceDesc.ArraySize != 1u ||
        x >= sourceDesc.Width || y >= sourceDesc.Height)
        return false;

    // 화면 전체가 아니라 요청한 한 픽셀만 copy한다. source 좌표와 결과의 대응을 바꾸지 않는다.
    const D3D11_BOX box{ x, y, 0u, x + 1u, y + 1u, 1u };
    {
        CProfilerScope copyScope(pProfiler, "Picking.CopyPixel");
        CProfilerGpuScope gpuCopyScope(pProfiler, "Picking.CopyPixel");
        m_pContext->CopySubresourceRegion(m_pTexture2D.Get(), 0u, 0u, 0u, 0u,
            pSource, 0u, &box);
        if (pProfiler)
        {
            pProfiler->Add_Counter(EProfilerCounter::PickingReadbacks);
            pProfiler->Add_Counter(EProfilerCounter::PickingReadbackBytes, sizeof(float4_t));
        }
    }
    D3D11_MAPPED_SUBRESOURCE mapped{};
    {
        // 일반 Map은 GPU 완료를 기다릴 수 있다. 복사량이 1픽셀인 것과 최대 대기 시간이 보장되는 것은 다르다.
        CProfilerScope mapScope(pProfiler, "Picking.MapWait");
        if (FAILED(m_pContext->Map(m_pTexture2D.Get(), 0u, D3D11_MAP_READ, 0u, &mapped)))
            return false;
    }
    float4_t position{};
    {
        CProfilerScope readScope(pProfiler, "Picking.ReadPixel");
        const bool_t readable = mapped.pData && mapped.RowPitch >= sizeof(position);
        if (readable)
            std::memcpy(&position, mapped.pData, sizeof(position));
        m_pContext->Unmap(m_pTexture2D.Get(), 0u);
        if (!readable)
            return false;
    }
    // 동기 경로는 w=0만 빈 픽셀로 거부한다. xyz 유한성 검사는 아래 비동기 Poll과 같지 않다.
    if (position.w == 0.f)
        return false;
    vOut = position;
    // RT의 w가 보조 값을 담아도 외부에는 위치 벡터의 w=1로 전달한다.
    vOut.w = 1.f;
    return true;
}

// 요청 순간의 커서를 Client 좌표로 바꾸고 최신 RT를 다시 찾는다. resize 이전 texture를 계속 보관하지 않는다.
bool_t CPicking::Picking(float4_t& vOut)
{
    if (GetCurrentThreadId() != m_iOwnerThreadId)
        return false;
    CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope scope(pProfiler, "Picking.Readback");
    ::POINT mouse{};
    if (!m_hWnd || !GetCursorPos(&mouse) || !ScreenToClient(m_hWnd, &mouse) ||
        mouse.x < 0 || mouse.y < 0)
        return false;
    const auto sourceView = CGameInstance::Get().Get_RT_SRV(TEXT("Target_PickPos"));
    if (!sourceView)
        return false;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    sourceView->GetDesc(&viewDesc);
    if (viewDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
        viewDesc.Texture2D.MostDetailedMip != 0u)
        return false;
    ComPtr<ID3D11Resource> resource;
    sourceView->GetResource(&resource);
    ComPtr<ID3D11Texture2D> source;
    if (!resource || FAILED(resource.As(&source)))
        return false;
    return Read_Pixel(source.Get(), static_cast<uint32_t>(mouse.x),
        static_cast<uint32_t>(mouse.y), vOut, pProfiler);
}

// GameInstance용 후보를 초기화하고 HRESULT 실패는 nullptr로 전달한다. 별도 GPU 작업이나 커서 읽기는 하지 않는다.
unique_ptr<CPicking> CPicking::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, HWND hWnd)
{
    auto pInstance = unique_ptr<CPicking>(new CPicking(pDevice, pContext));

    if (FAILED(pInstance->Initialize(hWnd)))
    {
        MSG_BOX("Failed to Created : CPicking");
        return nullptr;
    }

    return pInstance;
}

// 커서 픽셀의 copy만 제출하고 즉시 ID를 돌려준다. GPU 완료 결과는 같은 스레드의 Poll이 소비한다.
uint64_t CPicking::Request_Picking()
{
    if (GetCurrentThreadId() != m_iOwnerThreadId || !m_pAsyncTexture)
        return 0u;
    // 새 입력은 이전 표본을 폐기한다. 이후 RT/좌표 검사가 실패해도 이전 요청을 되살리지 않는다.
    m_iPendingRequestId = 0u;
    ::POINT mouse{};
    if (!m_hWnd || !GetCursorPos(&mouse) || !ScreenToClient(m_hWnd, &mouse) ||
        mouse.x < 0 || mouse.y < 0)
        return 0u;
    const auto view = CGameInstance::Get().Get_RT_SRV(TEXT("Target_PickPos"));
    if (!view)
        return 0u;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    view->GetDesc(&viewDesc);
    if (viewDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
        viewDesc.Texture2D.MostDetailedMip != 0u)
        return 0u;
    ComPtr<ID3D11Resource> resource;
    view->GetResource(&resource);
    ComPtr<ID3D11Texture2D> source;
    if (!resource || FAILED(resource.As(&source)))
        return 0u;
    ComPtr<ID3D11Device> sourceDevice;
    source->GetDevice(&sourceDevice);
    if (sourceDevice.Get() != m_pDevice.Get())
        return 0u;
    D3D11_TEXTURE2D_DESC desc{};
    source->GetDesc(&desc);
    if (desc.Format != DXGI_FORMAT_R32G32B32A32_FLOAT || desc.SampleDesc.Count != 1u ||
        desc.ArraySize != 1u || uint32_t(mouse.x) >= desc.Width || uint32_t(mouse.y) >= desc.Height)
        return 0u;
    const D3D11_BOX box{uint32_t(mouse.x), uint32_t(mouse.y), 0u,
        uint32_t(mouse.x + 1), uint32_t(mouse.y + 1), 1u};
    auto* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope scope(profiler, "Picking.AsyncCopy");
    m_pContext->CopySubresourceRegion(m_pAsyncTexture.Get(), 0u, 0u, 0u, 0u, source.Get(), 0u, &box);
    if (profiler)
    {
        profiler->Add_Counter(EProfilerCounter::PickingReadbacks);
        profiler->Add_Counter(EProfilerCounter::PickingReadbackBytes, sizeof(float4_t));
    }
    // copy 제출 뒤에만 새 ID를 발급한다. 0은 실패/요청 없음으로 쓰므로 정수 순환 때 한 번 더 증가시킨다.
    if (++m_iNextRequestId == 0u)
        ++m_iNextRequestId;
    return m_iPendingRequestId = m_iNextRequestId;
}

// ID가 다르거나 스레드가 다르면 E_ABORT이며 현재 요청을 건드리지 않는다.
// GPU 처리 중인 S_FALSE만 pending을 유지한다. 실제 Map 결과가 나면 성공/실패와 무관하게 그 요청을 소비한다.
HRESULT CPicking::Poll_Picking(const uint64_t requestId, float4_t& vOut)
{
    if (GetCurrentThreadId() != m_iOwnerThreadId || !requestId || requestId != m_iPendingRequestId)
        return E_ABORT;
    D3D11_MAPPED_SUBRESOURCE mapped{};
    // DO_NOT_WAIT로 준비 상태만 확인한다. SUCCEEDED(S_FALSE)도 true이므로 호출자는 반드시 S_OK와 구분해야 한다.
    const HRESULT hr = m_pContext->Map(m_pAsyncTexture.Get(), 0u, D3D11_MAP_READ,
        D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped);
    if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
        return S_FALSE;
    m_iPendingRequestId = 0u;
    if (FAILED(hr))
        return hr;
    float4_t position{};
    const bool readable = mapped.pData && mapped.RowPitch >= sizeof(position);
    if (readable)
        std::memcpy(&position, mapped.pData, sizeof(position));
    m_pContext->Unmap(m_pAsyncTexture.Get(), 0u);
    if (!readable || position.w == 0.f || !std::isfinite(position.x) ||
        !std::isfinite(position.y) || !std::isfinite(position.z))
        return E_FAIL;
    vOut = position;
    vOut.w = 1.f;
    return S_OK;
}

// 현재 ID와 소유 스레드가 모두 맞을 때만 결과 수용을 취소한다. 오래된 ID의 취소는 새 요청에 영향을 주지 않는다.
void CPicking::Cancel_Picking(const uint64_t requestId)
{
    if (GetCurrentThreadId() == m_iOwnerThreadId && requestId == m_iPendingRequestId)
        m_iPendingRequestId = 0u;
}
