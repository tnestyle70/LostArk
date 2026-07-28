#include "Font_Manager.h"

#include "CustomFont.h"

CFont_Manager::CFont_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CFont_Manager::~CFont_Manager()
{
}

HRESULT CFont_Manager::Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	auto		pFont = CCustomFont::Create(m_pDevice.Get(), m_pContext.Get(), pFontFilePath);
	if (nullptr == pFont)
		return E_FAIL;

	m_Fonts.emplace(strFontTag, move(pFont));

	return S_OK;
}

void CFont_Manager::Draw(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor, f32_t fRotation, const float2_t& vOrigin, f32_t fScale)
{
	auto		pFont = Find_Font(strFontTag);
	if (nullptr == pFont)
		return;

	pFont->Draw(pText, vPosition, vColor, fRotation, vOrigin, fScale);
}

shared_ptr<class CCustomFont> CFont_Manager::Find_Font(const wstring& strFontTag)
{
	auto	iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;	
}

unique_ptr<CFont_Manager> CFont_Manager::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	return unique_ptr<CFont_Manager>(new CFont_Manager(pDevice, pContext));
}
