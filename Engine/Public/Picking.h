#pragma once

#include "Engine_Defines.h"

/* 동적인 텍스쳐를 보관한다. */
/* 렌더타겟을 복사해온다. */
/* 픽셀에 들어가 있는 정보를 꺼내온다. */

NS_BEGIN(Engine)

class CPicking final 
{
private:
	CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CPicking();

public:
	HRESULT Initialize(HWND hWnd);
	void Update();
	bool_t Picking(float4_t& vOut);

private:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D>				m_pTexture2D = { nullptr };
	HWND								m_hWnd = {};
	float2_t							m_vViewportSize = {};

	shared_ptr<float4_t[]>				m_pWorldPositions = { };

public:
	static unique_ptr<CPicking> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, HWND hWnd);
};

NS_END