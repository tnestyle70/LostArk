#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CShader final : public CComponent
{
private:
	CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CShader();
public:
	virtual HRESULT Initialize_Prototype(const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength);
	HRESULT Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix);
	HRESULT Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices);
	HRESULT Bind_Texture(const char_t* pConstantName, ComPtr<ID3D11ShaderResourceView> pSRV);
	HRESULT Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs);
	HRESULT Begin(uint32_t iPassIndex);

private:
	ComPtr<ID3DX11Effect>				m_pEffect = { nullptr };
	vector<ComPtr<ID3D11InputLayout>>	m_InputLayouts;

private:
	uint32_t			m_iNumPasses = {};

public:
	static unique_ptr<CShader> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END