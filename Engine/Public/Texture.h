#pragma once

#include "Component.h"

/* 내가 지정해둔 텍스쳐여러장을 한번에 로드하여 보관한다. */
/* 쉐이더에 텍스쳐 리소스를 던져준다. */

NS_BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CTexture();

public:
	virtual HRESULT Initialize_Prototype(const tchar_t* pTextureFilePath, uint32_t iNumTextures);
	virtual HRESULT Initialize(void* pArg) override;
public:
	HRESULT Bind_ShaderResource(class shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iTextureIndex);
	HRESULT Bind_ShaderResources(class shared_ptr<class CShader> pShader, const char_t* pConstantName);

private:
	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures;
	uint32_t									m_iNumTextures = {};

private:
	vector<ID3D11ShaderResourceView*>			m_SRVs;

public:
	static unique_ptr<CTexture> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pTextureFilePath, uint32_t iNumTextures);	
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

