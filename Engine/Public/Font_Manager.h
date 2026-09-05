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
	/* Screen-pixel rect no text may be drawn over: Draw skips a string whose extent overlaps it.
	Text has no depth and every text pass runs after the sprites, so the topmost UI window sets
	this for the other owners' text and clears it before drawing its own. */
	void Set_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_ClipOutRect() { m_isClipOutEnabled = false; }

private:
	ComPtr<ID3D11Device>										m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>									m_pContext = { nullptr };
	map<const wstring_t, shared_ptr<class CCustomFont>>			m_Fonts;
	bool_t														m_isClipOutEnabled = false;
	float4_t													m_vClipOutRect = {};	/* x, y, width, height */


private:
	shared_ptr<class CCustomFont> Find_Font(const wstring& strFontTag);

public:
	static unique_ptr<CFont_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END