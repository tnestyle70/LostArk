#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CRenderTarget final 
{
private:
	CRenderTarget(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CRenderTarget();

public:
	ComPtr<ID3D11RenderTargetView> Get_RTV() {
		return m_pRTV;
	}

public:
	HRESULT Initialize(uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	HRESULT Bind_SRV(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	void Clear();
	HRESULT Copy_Resource(ComPtr<ID3D11Texture2D> pTexture2D);

#ifdef _DEBUG
public:
	HRESULT Ready_DebugDesc(f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	HRESULT Render(shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);

#endif

private:
	ComPtr<ID3D11Device>						m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>					m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D>						m_pTexture2D = { nullptr };
	ComPtr<ID3D11RenderTargetView>				m_pRTV = { nullptr };
	ComPtr<ID3D11ShaderResourceView>			m_pSRV = { nullptr };

private:
	float4_t			m_vClearColor = {};

#ifdef _DEBUG
private:
	float4x4_t			m_WorldMatrix = {};
#endif

public:
	static shared_ptr<CRenderTarget> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
};

NS_END