#include "Picking.h"
#include "GameInstance.h"

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
    m_hWnd = hWnd;

    m_vViewportSize = CGameInstance::Get().Get_ViewportSize();

    D3D11_TEXTURE2D_DESC            TextureDesc{};

    TextureDesc.Width = m_vViewportSize.x;
    TextureDesc.Height = m_vViewportSize.y;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_STAGING;
    TextureDesc.BindFlags = 0;
    TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    TextureDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
        return E_FAIL;

    m_pWorldPositions = make_shared<float4_t[]>(TextureDesc.Width * TextureDesc.Height);

    return S_OK;
}

void CPicking::Update()
{    
    /* 픽킹이 될 픽셀의 월드위치를 그려놓은 렌더타겟을 동적인 텍스쳐에 복사해준다. */
    CGameInstance::Get().Copy_RT_Resource(TEXT("Target_PickPos"), m_pTexture2D);

    /* 복사한 텍스쳐의 락, 언락을 호출해서 텍스쳐에 저장되어있는 월드위치를 미리 다 꺼내놓자. */
    D3D11_MAPPED_SUBRESOURCE            MappedSubResource{};

    if (FAILED(m_pContext->Map(m_pTexture2D.Get(), 0, D3D11_MAP_READ_WRITE, 0, &MappedSubResource)))
        return;

    memcpy(m_pWorldPositions.get(), MappedSubResource.pData, sizeof(float4_t) * m_vViewportSize.x * m_vViewportSize.y);

    m_pContext->Unmap(m_pTexture2D.Get(), 0);
}

bool_t CPicking::Picking(float4_t& vOut)
{
    ::POINT           ptMouse = {};
    GetCursorPos(&ptMouse);
    ScreenToClient(m_hWnd, &ptMouse);

    uint32_t        iIndex = ptMouse.y * static_cast<uint32_t>(m_vViewportSize.x) + ptMouse.x;

    if (0 != m_pWorldPositions[iIndex].w)
    {
        vOut = m_pWorldPositions[iIndex];
        return true;
    }
    else
        return false;    
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
