#pragma once

#include "Engine_Defines.h"


NS_BEGIN(Engine)

class CCustomFont final 
{
private:
	CCustomFont(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCustomFont();

public:
	HRESULT Initialize(const tchar_t* pFontFilePath);
	void Draw(const tchar_t* pText, const float2_t& vPosition, fvector_t vColor = Colors::White, f32_t fRotation = 0.f, const float2_t& vOrigin = float2_t(0.f, 0.f), f32_t fScale = 1.f);
	float2_t Measure(const tchar_t* pText) const;

private:
	ComPtr<ID3D11Device>					m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>				m_pContext = { nullptr };
	unique_ptr<SpriteBatch>					m_pBatch = { nullptr };
	unique_ptr<SpriteFont>					m_pFont = { nullptr };

public:
	static unique_ptr<CCustomFont> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pFontFilePath);
};

NS_END