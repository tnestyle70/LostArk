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
	/* Several windows can sit on top of the one drawing: every rect added here is honoured
	until Clear_ClipOutRect. Set_ClipOutRect is clear-then-add. */
	void Add_ClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_ClipOutRect() { m_isClipOutEnabled = false; m_ClipOutRects.clear(); }
	/* The inverse: a screen-pixel rect text must stay inside. Draw skips a string whose extent
	is not wholly within it -- a scrolling list whose rows slide under its own frame (the system
	option pane) keeps half-visible labels from bleeding over the panel above and below. */
	void Set_ClipInRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_ClipInRect() { m_isClipInEnabled = false; }

private:
	ComPtr<ID3D11Device>										m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>									m_pContext = { nullptr };
	map<const wstring_t, shared_ptr<class CCustomFont>>			m_Fonts;
	bool_t														m_isClipOutEnabled = false;
	vector<float4_t>											m_ClipOutRects;			/* x, y, width, height */
	bool_t														m_isClipInEnabled = false;
	float4_t													m_vClipInRect = {};		/* x, y, width, height */


private:
	shared_ptr<class CCustomFont> Find_Font(const wstring& strFontTag);

public:
	static unique_ptr<CFont_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END