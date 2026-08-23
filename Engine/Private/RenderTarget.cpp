#include "RenderTarget.h"

#include "GameInstance.h"

CRenderTarget::CRenderTarget(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{

}

CRenderTarget::~CRenderTarget()
{
	//SaveDDSTextureToFile(m_pContext.Get(), m_pTexture2D.Get(), TEXT("../Bin/Diffuse.dds"));
}

ComPtr<ID3D11ShaderResourceView> CRenderTarget::Get_SRV() const
{
	return m_pSRV;
}

HRESULT CRenderTarget::Initialize(uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	D3D11_TEXTURE2D_DESC	TextureDesc{};
	
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = ePixelFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateRenderTargetView(m_pTexture2D.Get(), nullptr, &m_pRTV)))
		return E_FAIL;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture2D.Get(), nullptr, &m_pSRV)))
		return E_FAIL;

	m_vClearColor = vClearColor;

    return S_OK;
}

HRESULT CRenderTarget::Bind_SRV(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_Texture(pConstantName, m_pSRV);
}

void CRenderTarget::Clear()
{
	m_pContext->ClearRenderTargetView(m_pRTV.Get(), reinterpret_cast<const f32_t*>(&m_vClearColor));

}

HRESULT CRenderTarget::Copy_Resource(ComPtr<ID3D11Texture2D> pTexture2D)
{
	m_pContext->CopyResource(pTexture2D.Get(), m_pTexture2D.Get());

	return S_OK;
}

#ifdef _DEBUG

HRESULT CRenderTarget::Ready_DebugDesc(f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	m_WorldMatrix._11 = fSizeX;
	m_WorldMatrix._22 = fSizeY;
	m_WorldMatrix._41 = fX - vViewportSize.x * 0.5f;
	m_WorldMatrix._42 = -fY + vViewportSize.y * 0.5f;

	return S_OK;
}

HRESULT CRenderTarget::Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(pShader->Bind_Texture("g_Texture", m_pSRV)))
		return E_FAIL;

	if (FAILED(pShader->Begin(ETOUI(DEFERRED::DEBUG))))
		return E_FAIL;

	if (FAILED(pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

#endif

shared_ptr<CRenderTarget> CRenderTarget::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
    auto pInstance = shared_ptr<CRenderTarget>(new CRenderTarget(pDevice, pContext));

    if (FAILED(pInstance->Initialize(iWidth, iHeight, ePixelFormat, vClearColor)))
    {
        MSG_BOX("Failed to Created : CRenderTarget");
        return nullptr;
    }

    return pInstance;
}
