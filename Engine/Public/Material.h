#pragma once

#include "Engine_Defines.h"
#include "Assimp/material.h"

#include "BinaryAsset/ModelAssetData.h"
#include <span>

NS_BEGIN(Engine)

// CModel이 공유하는 재질로, 텍스처와 표면 상수를 보관해 선택한 CShader의 입력에 바인딩한다.
// 텍스처 수명은 ComPtr로 유지하고 개별 외형 변경은 Clone_ForOverrides의 별도 재질 상태에 적용한다.
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
    HRESULT Bind_SourceLandscapeSurface(shared_ptr<class CShader> shader);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> pShader);
    bool_t Can_BatchStaticLightingWith(const CMaterial& other, bool_t staticCluster = false) const;
    bool_t Has_SameStaticLightingTextures(const CMaterial& other) const;
    bool_t Can_BakeStaticProxy() const;
    bool_t Validate_StaticProxySourceFiles() const;
    std::span<const filesystem::path> Get_StaticProxySourcePaths() const
    { return m_StaticProxySourcePaths; }
    HRESULT Bind_StaticLightingBank(const shared_ptr<class CShader>& shader,
        std::span<const CMaterial* const> materials, bool_t staticCluster = false) const;
    HRESULT Bind_StaticShadow(shared_ptr<class CShader> shader);
    HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader);
    // base program도 실행하는 forward pass에 조명 상수·텍스처를 바인딩한다.
    HRESULT Bind_SourceCharacterForwardLight(shared_ptr<class CShader> shader);
    static void Reset_SourceCharacterFrame(float presentationTime = 0.f);
    static uint32_t Get_SourceCharacterFrameCount();
    static HRESULT Bind_SourceCharacterLight(shared_ptr<class CShader> shader, uint32_t index);
	/* 복제 재질의 legacy·surface texture 슬롯을 override한다. 원본 texture는 바꾸지 않으며 null이면 원본 view로 복원한다. */
	void Set_TextureOverride(aiTextureType eType, uint32_t iTextureIndex,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_TextureOverrides();
	bool_t Has_TextureOverrides() const { return !m_TextureOverrides.empty(); }
	/* 염색 mask가 있는 재질만 복제본에 선택 색을 적용한다. 원본 색은 보존해 prototype을 공유하는 캐릭터의 선택을 분리한다. */
	void Set_DyeColorOverride(const float4_t& vDiffuse, const float4_t& vRegionA);
	void Clear_DyeColorOverride();
	/* 머리카락 두 번째 색의 혼합량·적용 범위를 색 자체와 별도로 조절한다. */
	void Set_DyeTwoTone(f32_t fStrength, f32_t fRange);
	/* 피부·눈처럼 영역 mask 없는 surface의 diffuse에 색을 곱한다. 기본 identity에서는 원래 값이 유지된다. */
	void Set_DiffuseTint(const float4_t& vTint) { m_vDiffuseTint = vTint; }
	const float4_t& Get_DiffuseTint() const { return m_vDiffuseTint; }
	/* LostArk 고유 이름·수식의 상수 packing은 Client가 맡고 Engine은 결과만 전달한다. program·mask는 유지하며 비유한 값·허용 범위 밖 값은 거부한다. */
	bool_t Set_SourceCharacterConstants(const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
	/* 얼굴 stamp의 texture register override를 복제본에 보관한다. null이면 원본으로 복원하고 재질이 사용하지 않는 register는 거부한다. */
	bool_t Set_SourceCharacterTextureOverride(
		uint32_t iRegister, ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_SourceCharacterOverrides();
    // 상수·override map은 instance별로 보관하고 GPU texture view는 공유한다.
    shared_ptr<CMaterial> Clone_ForOverrides() const
    { return shared_ptr<CMaterial>(new CMaterial(*this)); }
	bool_t Has_SourceCharacterProgram() const
	{
		return m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER;
	}
	bool_t Has_DyeColor() const { return m_ColorTint.isEnabled; }
	const string& Get_Name() const { return m_strName; }
	uint64_t Get_NameHash() const { return m_iNameHash; }
	/* WMA3 color mask가 없으면 isEnabled=false다. mask texture는 BASE_COLOR 슬롯에 있다. */
	const MODEL_COLOR_TINT& Get_ColorTint() const { return m_ColorTint; }

private:
	ComPtr<ID3D11Device>						m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>					m_pContext = { nullptr };
	string										m_strName;
	uint64_t									m_iNameHash = {};

    // cache는 weak 참조만 보관하고 GPU texture 수명은 재질이 유지한다.
    vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>> m_SharedTextureViews;
    // 파생 정적 재질 atlas의 cache key에 사용할 불변 원본 의존성이다.
    vector<filesystem::path> m_StaticProxySourcePaths;
    struct STATIC_PROXY_FILE_STAMP final
    {
        uintmax_t size = 0u;
        filesystem::file_time_type modified{};
        bool valid = false;
    };
    vector<STATIC_PROXY_FILE_STAMP> m_StaticProxySourceStamps;
	vector<ComPtr<ID3D11ShaderResourceView>>	m_Textures[AI_TEXTURE_TYPE_MAX];
	/* Sparse: only the slots the creation screen actually repainted. */
	unordered_map<uint32_t, ComPtr<ID3D11ShaderResourceView>> m_TextureOverrides;
	MODEL_COLOR_TINT							m_ColorTint;
	/* 선택 해제 시 정확히 복원할 원본 asset 값이다. */
	MODEL_COLOR_TINT							m_AuthoredColorTint;
	float4_t									m_vDiffuseTint = { 1.f, 1.f, 1.f, 1.f };
	MODEL_SURFACE_PARAMETERS m_Surface;
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_CHARACTER_TEXTURE_COUNT> m_SourceCharacterTextures;
    /* 편집한 register만 별도로 보관한다. 원본 view는 m_SourceCharacterTextures에 남겨 선택 해제 시 복원한다. */
    unordered_map<uint32_t, ComPtr<ID3D11ShaderResourceView>> m_SourceCharacterTextureOverrides;
    /* 선택 해제 시 program 전체를 복원할 원본 asset 상수다. */
    MODEL_SOURCE_CHARACTER_PARAMETERS m_AuthoredSourceCharacter;
    HRESULT Bind_SourceCharacterInputs(shared_ptr<class CShader> shader, bool lightPass, uint32_t row);
	uint32_t m_iDiffuseMirrorU = 0u;
	/* 원본 재질의 색 공간 해석이 다를 때 별도 view를 사용해 legacy A/B 입력을 유지한다. */
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
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_LANDSCAPE_LAYER_COUNT> m_SourceLandscapeDiffuse;
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_LANDSCAPE_LAYER_COUNT> m_SourceLandscapeNormal;
    std::array<ComPtr<ID3D11ShaderResourceView>, SOURCE_LANDSCAPE_WEIGHTMAP_COUNT> m_SourceLandscapeWeights;
    ComPtr<ID3D11ShaderResourceView> m_SourceLandscapeHeight;
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
    ComPtr<ID3D11ShaderResourceView> m_SourceIndirectBRDF;
    ComPtr<ID3D11ShaderResourceView> m_SourceIndirectCube;

public:
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath);
	static shared_ptr<CMaterial> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material);
};

NS_END
