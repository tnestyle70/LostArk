#pragma once

#include "Engine_Defines.h"

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class CMaterial
{
private:
	CMaterial(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CMaterial();

public:
	HRESULT Initialize(const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	HRESULT Initialize(const MODEL_MATERIAL_DATA& material);
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex);
	bool_t Has_Texture(aiTextureType eType, uint32_t iTextureIndex = 0) const;
	const string& Get_Name() const { return m_strName; }
	uint64_t Get_NameHash() const { return m_iNameHash; }
	/* Identity (isEnabled false) for every material without a WMA3 colour
	mask. The mask texture itself lives in the aiTextureType_BASE_COLOR slot. */
	const MODEL_COLOR_TINT& Get_ColorTint() const { return m_ColorTint; }

private:
	ComPtr<ID3D11Device>						m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>					m_pContext = { nullptr };
	string										m_strName;
	uint64_t									m_iNameHash = {};

	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures[AI_TEXTURE_TYPE_MAX];
	MODEL_COLOR_TINT							m_ColorTint;

public:
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material);
};

NS_END
