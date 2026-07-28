#include "CustomFont.h"

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

    m_pBatch->Begin();

    

    /*  
    _In_ SpriteBatch* spriteBatch,
    _In_z_ wchar_t const* text,
    XMFLOAT2 const& position,
    FXMVECTOR color = Colors::White, float rotation = 0, XMFLOAT2 const& origin = Float2Zero, float scale = 1,
    SpriteEffects effects = SpriteEffects_None, float layerDepth = 0

    */

    m_pFont->DrawString(m_pBatch.get(), pText, vPosition, vColor, fRotation, vOrigin, fScale);

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
