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
	HRESULT Bind_SourceSpecialSurface(shared_ptr<class CShader> shader);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> pShader);
    HRESULT Bind_StaticShadow(shared_ptr<class CShader> shader);
    HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader);
    static void Reset_SourceCharacterFrame(float presentationTime = 0.f);
    static uint32_t Get_SourceCharacterFrameCount();
    static HRESULT Bind_SourceCharacterLight(shared_ptr<class CShader> shader, uint32_t index);
	/* Swaps one of this material's texture slots for another view at runtime, for the
	character-creation choices that repaint a face rather than replace a mesh -- iris, lip,
	cheek, eye make and decals. Both the legacy A/B slots and the surface program's own slots
	honour it, so a material takes the override whichever path binds it. A null view clears
	the override and the authored texture comes back. Nothing is written to the loaded
	texture, so two characters sharing a material prototype do not share the choice: the
	override lives on the cloned material. */
	void Set_TextureOverride(aiTextureType eType, uint32_t iTextureIndex,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_TextureOverrides();
	/* The creation screen repaints a dyed material: the authored colour stays in the asset
	and the chosen one rides on the clone, so two characters sharing a prototype keep their
	own. Only a material that already dyes accepts one -- an undyed material has no mask to
	paint through, and forcing a colour on it would flatten its own texture. */
	void Set_DyeColorOverride(const float4_t& vDiffuse, const float4_t& vRegionA);
	void Clear_DyeColorOverride();
	/* Hair only: how much of the second colour blends in and how far up the strand it
	reaches. Kept apart from the colours so the two sliders can move without the screen
	having to know what the hairstyle was authored in. */
	void Set_DyeTwoTone(f32_t fStrength, f32_t fRange);
	/* A plain multiply on the sampled diffuse, for the surfaces the source game tints
	without a region mask -- skin (var_base_skincolor_ui) and eyes (var_eye_iriscolor_ui),
	both authored at identity so nothing changes until a colour is chosen. */
	void Set_DiffuseTint(const float4_t& vTint) { m_vDiffuseTint = vTint; }
	const float4_t& Get_DiffuseTint() const { return m_vDiffuseTint; }
	/* The creation screen's skin and make-up choices are named parameters of the retail head
	material, and the native program reads them already packed into its constant rows. The
	Client owns that packing (Client/Public/SourceCharacterMaterialParameters.h) because the
	names and the arithmetic between them are LostArk's; the Engine only carries the result.
	The program and its texture masks stay the material's own -- only the constants move, so a
	choice can never turn a face into a different surface. Non-finite or absurd values are
	refused rather than reaching the GPU. */
	bool_t Set_SourceCharacterConstants(const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
	/* One texture register of that program, for the lip, eye make-up, cheek and decal stamps
	the screen puts on a face. Like Set_TextureOverride this rides on the clone rather than the
	loaded texture, so two characters sharing a material prototype keep their own choice, and a
	null view restores what the asset loaded. A register the material's masks do not use is
	refused: binding one would be a silent no-op. */
	bool_t Set_SourceCharacterTextureOverride(
		uint32_t iRegister, ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_SourceCharacterOverrides();
	bool_t Has_SourceCharacterProgram() const
	{
		return m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
	}
	bool_t Has_DyeColor() const { return m_ColorTint.isEnabled; }
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

    // The cache holds weak references; material lifetime owns GPU texture data.
    vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>> m_SharedTextureViews;
	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures[AI_TEXTURE_TYPE_MAX];
	/* Sparse: only the slots the creation screen actually repainted. */
	unordered_map<uint32_t, ComPtr<ID3D11ShaderResourceView>> m_TextureOverrides;
	MODEL_COLOR_TINT							m_ColorTint;
	/* What the asset shipped, so clearing the choice restores it exactly. */
	MODEL_COLOR_TINT							m_AuthoredColorTint;
	float4_t									m_vDiffuseTint = { 1.f, 1.f, 1.f, 1.f };
	MODEL_SURFACE_PARAMETERS m_Surface;
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_CHARACTER_TEXTURE_COUNT> m_SourceCharacterTextures;
    /* Sparse: only the registers a creation choice repainted. The authored views stay in
    m_SourceCharacterTextures above, so clearing restores them exactly. */
    unordered_map<uint32_t, ComPtr<ID3D11ShaderResourceView>> m_SourceCharacterTextureOverrides;
    /* What the asset shipped, so clearing the choices puts the whole program back. */
    MODEL_SOURCE_CHARACTER_PARAMETERS m_AuthoredSourceCharacter;
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
    ComPtr<ID3D11ShaderResourceView> m_SourceFoliageMask;
    ComPtr<ID3D11ShaderResourceView> m_SourceSpecialMask;
    ComPtr<ID3D11ShaderResourceView> m_SourceBlendDiffuseG;
    ComPtr<ID3D11ShaderResourceView> m_SourceBlendDiffuseB;
    ComPtr<ID3D11ShaderResourceView> m_SourceBlendNormalG;
    ComPtr<ID3D11ShaderResourceView> m_SourceBlendNormalB;
	ComPtr<ID3D11ShaderResourceView> m_SurfaceEmissive;
    ComPtr<ID3D11ShaderResourceView> m_BakedAverage;
    ComPtr<ID3D11ShaderResourceView> m_BakedDirectional;
    ComPtr<ID3D11ShaderResourceView> m_StaticShadow;
    ComPtr<ID3D11ShaderResourceView> m_EnvironmentCube;
    ComPtr<ID3D11ShaderResourceView> m_EnvironmentBRDF;

public:
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material);
};

NS_END
