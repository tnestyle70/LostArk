#pragma once

#include "Engine_Defines.h"

// On-demand world-position readback from the currently rendered target.

NS_BEGIN(Engine)

class CPicking final 
{
private:
	CPicking(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CPicking();

public:
	HRESULT Initialize(HWND hWnd);
	bool_t Picking(float4_t& vOut);
	// Request captures the current pixel once; Poll never waits for the GPU.
	uint64_t Request_Picking();
	HRESULT Poll_Picking(uint64_t requestId, float4_t& vOut);
	void Cancel_Picking(uint64_t requestId);


private:
	bool_t Read_Pixel(ID3D11Texture2D* pSource, uint32_t x, uint32_t y,
		float4_t& vOut, class CProfiler* pProfiler);

private:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D>				m_pTexture2D = { nullptr };
	ComPtr<ID3D11Texture2D> m_pAsyncTexture;
	uint64_t m_iNextRequestId = 0u;
	uint64_t m_iPendingRequestId = 0u;
	HWND m_hWnd = {};

	DWORD m_iOwnerThreadId = 0u;

public:
	static unique_ptr<CPicking> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, HWND hWnd);
};

NS_END