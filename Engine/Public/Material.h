#pragma once

#include "Engine_Defines.h"

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class CMaterial : public std::enable_shared_from_this<CMaterial>
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
	const MODEL_SURFACE_PARAMETERS& Get_Surface() const { return m_Surface; }
	HRESULT Bind_SurfaceTexture(shared_ptr<class CShader> pShader,
		const char_t* pConstantName, aiTextureType eType);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> pShader);
    HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader);
    static void Reset_SourceCharacterFrame(float presentationTime = 0.f);
    static uint32_t Get_SourceCharacterFrameCount();
    static HRESULT Bind_SourceCharacterLight(shared_ptr<class CShader> shader, uint32_t index);
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
	MODEL_SURFACE_PARAMETERS m_Surface;
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_CHARACTER_TEXTURE_COUNT> m_SourceCharacterTextures;
    HRESULT Bind_SourceCharacterInputs(shared_ptr<class CShader> shader, bool lightPass, uint32_t row);
	uint32_t m_iDiffuseMirrorU = 0u;
	/* Separate views keep the legacy A/B inputs unchanged when the source
	   material specifies a different colour-space interpretation. */
	ComPtr<ID3D11ShaderResourceView> m_SurfaceDiffuse;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceSpecular;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceReflection;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceNormal;
    ComPtr<ID3D11ShaderResourceView> m_SurfaceOverlayDiffuse;
    ComPtr<ID3D11ShaderResourceView> m_SurfaceOverlayNormal;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceDetailNormal;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceORM;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceEmissive;
    ComPtr<ID3D11ShaderResourceView> m_BakedAverage;
    ComPtr<ID3D11ShaderResourceView> m_BakedDirectional;
    ComPtr<ID3D11ShaderResourceView> m_EnvironmentCube;
    ComPtr<ID3D11ShaderResourceView> m_EnvironmentBRDF;

public:
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material);
};

NS_END
