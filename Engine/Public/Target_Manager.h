#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CTarget_Manager final 
{
private:
	CTarget_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext); 
public:
	~CTarget_Manager();

public:
	HRESULT Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	HRESULT Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag);
	HRESULT Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Begin_DepthOnly(ComPtr<ID3D11DepthStencilView> pDSV);
	HRESULT End_DepthOnly();
	HRESULT Bind_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName);
	ComPtr<ID3D11ShaderResourceView> Get_SRV(const wstring_t& strTargetTag) const;
	HRESULT Copy_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D);

#ifdef _DEBUG
public:
	HRESULT Ready_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	HRESULT Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);

#endif

private:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

	ComPtr<ID3D11RenderTargetView>	m_pBackBufferRTV = { nullptr };
	ComPtr<ID3D11DepthStencilView>  m_pOriginalDSV = { nullptr };

private:
	map<const wstring_t, shared_ptr<class CRenderTarget>>			m_RenderTargets;
	map<const wstring_t, list<shared_ptr<class CRenderTarget>>>		m_MRTs;
private:
	shared_ptr<class CRenderTarget> Find_RenderTarget(const wstring_t& strTargetTag) const;
	list<shared_ptr<class CRenderTarget>>* Find_MRT(const wstring_t& strMRTTag);

public:
	static unique_ptr<CTarget_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
