#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CMaterial
{
private:
	CMaterial(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CMaterial();

public:
	HRESULT Initialize(const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex);

private:
	ComPtr<ID3D11Device>						m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>					m_pContext = { nullptr };

	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures[AI_TEXTURE_TYPE_MAX];

public:
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
};

NS_END