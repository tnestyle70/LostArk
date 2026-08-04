#include "CustomFont.h"

#include "GameInstance.h"

CCustomFont::CCustomFont(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CCustomFont::~CCustomFont()
{
}

HRESULT CCustomFont::Initialize(const tchar_t* pFontFilePath)
{
    m_pBatch = make_unique<SpriteBatch>(m_pContext.Get());

    m_pFont = make_unique<SpriteFont>(m_pDevice.Get(), pFontFilePath);

	return S_OK;
}

void CCustomFont::Draw(const tchar_t* pText, const float2_t& vPosition, fvector_t vColor, f32_t fRotation, const float2_t& vOrigin, f32_t fScale)
{
    /* SpriteBatch derives its projection from whatever viewport is currently bound. Shadow/bloom
    passes earlier in the frame temporarily swap the viewport to a different size and restore it
    afterward, but re-asserting the real screen viewport here removes any dependency on that
    restore ordering still being correct by the time text is drawn. */
    const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
    const D3D11_VIEWPORT ViewportDesc{ 0.f, 0.f, vViewportSize.x, vViewportSize.y, 0.f, 1.f };
    m_pContext->RSSetViewports(1, &ViewportDesc);

    m_pBatch->Begin();

    

    /*  
    _In_ SpriteBatch* spriteBatch,
    _In_z_ wchar_t const* text,
    XMFLOAT2 const& position,
    FXMVECTOR color = Colors::White, float rotation = 0, XMFLOAT2 const& origin = Float2Zero, float scale = 1,
    SpriteEffects effects = SpriteEffects_None, float layerDepth = 0

    */

    /* vOrigin comes in as a 0~1 anchor fraction (0.5 = centered), but SpriteFont::DrawString
    wants an absolute pixel offset within the string's own measured extent -- scale it here so
    callers get real text-width-aware centering instead of a fixed sub-pixel nudge. */
    const XMVECTOR vMeasured = m_pFont->MeasureString(pText);
    const float2_t vPixelOrigin(XMVectorGetX(vMeasured) * vOrigin.x, XMVectorGetY(vMeasured) * vOrigin.y);

    m_pFont->DrawString(m_pBatch.get(), pText, vPosition, vColor, fRotation, vPixelOrigin, fScale);

    m_pBatch->End();

}

unique_ptr<CCustomFont> CCustomFont::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pFontFilePath)
{
    auto pInstance = unique_ptr<CCustomFont>(new CCustomFont(pDevice, pContext));

    if (FAILED(pInstance->Initialize(pFontFilePath)))
    {
        MSG_BOX("Failed to Created : CCustomFont");
        return nullptr;
    }

    return pInstance;
}
