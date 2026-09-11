#include "Picking.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <cstring>

CPicking::CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{
}

CPicking::~CPicking()
{

}

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
    return m_pDevice->CreateTexture2D(&desc, nullptr, &m_pTexture2D);
}

bool_t CPicking::Read_Pixel(ID3D11Texture2D* pSource, const uint32_t x,
    const uint32_t y, float4_t& vOut, CProfiler* pProfiler)
{
    if (!pSource || !m_pTexture2D || GetCurrentThreadId() != m_iOwnerThreadId)
        return false;
    ComPtr<ID3D11Device> sourceDevice;
    pSource->GetDevice(&sourceDevice);
    if (sourceDevice.Get() != m_pDevice.Get()) return false;
    D3D11_TEXTURE2D_DESC sourceDesc{};
    pSource->GetDesc(&sourceDesc);
    if (sourceDesc.Format != DXGI_FORMAT_R32G32B32A32_FLOAT ||
        sourceDesc.SampleDesc.Count != 1u || sourceDesc.ArraySize != 1u ||
        x >= sourceDesc.Width || y >= sourceDesc.Height)
        return false;

    // Each request reads exactly its current cursor pixel from the current RT.
    // No previous-cursor or delayed-frame sample can become a gameplay command.
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
        // Exact requested position: this bounded on-demand Map may wait for the
        // rendering/copy command. Idle gameplay never submits a readback.
        CProfilerScope mapScope(pProfiler, "Picking.MapWait");
        if (FAILED(m_pContext->Map(m_pTexture2D.Get(), 0u, D3D11_MAP_READ, 0u, &mapped)))
            return false;
    }
    float4_t position{};
    {
        CProfilerScope readScope(pProfiler, "Picking.ReadPixel");
        const bool_t readable = mapped.pData && mapped.RowPitch >= sizeof(position);
        if (readable) std::memcpy(&position, mapped.pData, sizeof(position));
        m_pContext->Unmap(m_pTexture2D.Get(), 0u);
        if (!readable) return false;
    }
    if (position.w == 0.f) return false;
    vOut = position;
    // PBR target W may hold finite auxiliary normal bits; public picking is a position.
    vOut.w = 1.f;
    return true;
}

bool_t CPicking::Picking(float4_t& vOut)
{
    if (GetCurrentThreadId() != m_iOwnerThreadId) return false;
    CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope scope(pProfiler, "Picking.Readback");
    ::POINT mouse{};
    if (!m_hWnd || !GetCursorPos(&mouse) || !ScreenToClient(m_hWnd, &mouse) ||
        mouse.x < 0 || mouse.y < 0)
        return false;
    // Resolve the resource for every demand: resize/recreation cannot retain an
    // old texture or its dimensions, and no full-viewport CPU copy is needed.
    const auto sourceView = CGameInstance::Get().Get_RT_SRV(TEXT("Target_PickPos"));
    if (!sourceView) return false;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    sourceView->GetDesc(&viewDesc);
    if (viewDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
        viewDesc.Texture2D.MostDetailedMip != 0u)
        return false;
    ComPtr<ID3D11Resource> resource;
    sourceView->GetResource(&resource);
    ComPtr<ID3D11Texture2D> source;
    if (!resource || FAILED(resource.As(&source))) return false;
    return Read_Pixel(source.Get(), static_cast<uint32_t>(mouse.x),
        static_cast<uint32_t>(mouse.y), vOut, pProfiler);
}

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
