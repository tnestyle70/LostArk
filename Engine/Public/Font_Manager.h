#pragma once

#include "Engine_Defines.h"


NS_BEGIN(Engine)

class CFont_Manager final 
{
private:
	CFont_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CFont_Manager();

public:
	HRESULT Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath);
	void Draw(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor, f32_t fRotation, const float2_t& vOrigin, f32_t fScale);
	float2_t Measure(const wstring& strFontTag, const tchar_t* pText);

private:
	ComPtr<ID3D11Device>										m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>									m_pContext = { nullptr };
	map<const wstring_t, shared_ptr<class CCustomFont>>			m_Fonts;


private:
	shared_ptr<class CCustomFont> Find_Font(const wstring& strFontTag);

public:
	static unique_ptr<CFont_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END