#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
class CVIBuffer_ParticleRect;
class CVIBuffer_DynamicTrail;
NS_END

NS_BEGIN(Client)

struct EFFECT_RENDER_PREWARM_PROBE final
{
	uint64_t iCoreBuildCount = 0u;
	uint64_t iCatalogCommitCount = 0u;
	uint64_t iPreparedDocumentBuildCount = 0u;
	uint64_t iModelDiskLoadCount = 0u;
	uint64_t iTextureDiskLoadCount = 0u;
	uint64_t iVectorFieldDiskLoadCount = 0u;
	uint64_t iPreparedAttachCount = 0u;
	uint64_t iSynchronousDocumentStageCount = 0u;
	uint64_t iPreparedLookupMissCount = 0u;
	uint64_t iCatalogRevision = 0u;
	uint32_t iPreparedDocumentCount = 0u;
};

class CEffectDocumentRenderer final
{
public:
	struct PREPARED_DOCUMENT;

private:
	struct ELEMENT_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		std::array<ComPtr<ID3D11ShaderResourceView>, 5> Textures;
		std::array<ComPtr<ID3D11ShaderResourceView>, 7> SourceTextures;
		uint32_t iSourceTextureMask = 0u;
		uint32_t iSourceMaterialProfile = 0u;
		float4_t vSourceScalars0{};
		float4_t vSourceScalars1{};
		float4_t vSourceVector0{};
		float4_t vSourceVector1{};
		std::array<float4_t, 16u> LinearFlowParameters{};
		float4_t vLinearFlowMaskAColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vLinearFlowMaskBColor{ 1.f, 1.f, 1.f, 1.f };
		std::array<float4_t, 16u> BlacklineParameters{};
		float4_t vBlacklineDiffuseColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vBlacklineMaskColor{ 1.f, 1.f, 1.f, 1.f };
		std::array<float4_t, 5u> LocalCrackParameters{};
		float4_t vLocalCrackOutColor{ 0.1f, 0.1f, 0.1f, 1.f };
		float4_t vLocalCrackInColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vLocalCrackReflectionColor{ 1.f, 1.f, 1.f, 1.f };
		uint32_t iSourceTextureClampUMask = 0u;
		uint32_t iSourceTextureClampVMask = 0u;
		std::array<uint32_t, 4u> DynamicParameterSemantics{};
		EFFECT_GROUPED_TRANSLUCENT_CONSTANTS GroupedConstants;
		bool_t bSourceMaterialFallbackBlocked = false;
	};
	struct MODEL_CUE_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		uint32_t iAnimationIndex = 0u;
		f32_t fTicksPerSecond = 0.f;
	};

public:
	CEffectDocumentRenderer(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffectDocumentRenderer();

	static bool_t Prepare_Catalog(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint64_t iCatalogRevision,
		const std::vector<std::pair<std::string,
			std::shared_ptr<const EFFECT_DOCUMENT_DESC>>>& Documents,
		std::string& strOutError);
	static std::shared_ptr<const PREPARED_DOCUMENT> Find_Prepared(
		uint64_t iCatalogRevision,
		const std::string& strEffectAssetId,
		const EFFECT_DOCUMENT_DESC& Document);
	static std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		Get_PlaybackResources(
			const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared);
	static EFFECT_RENDER_PREWARM_PROBE Get_PrewarmProbe();
	static void Clear_Prepared_Catalog();

	HRESULT Initialize();
	bool_t Stage_Prepared(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::string& strOutError);
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Stage_ReconstructedRuntimeProgram(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	HRESULT Render(const EFFECT_EVALUATED_FRAME& Frame);
	void Clear();
	const std::string& Get_Status() const { return m_strStatus; }
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_ReconstructedRuntimeEntry() const
	{
		return m_ReconstructedRuntimeBoundary.Get_CatalogEntry();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_ReconstructedRuntimePreparation() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Preparation();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_ReconstructedRuntimeProgram() const
	{
		return m_ReconstructedRuntimeBoundary.Get_Program();
	}

private:
	struct PREWARM_ASSET_CACHE;

	HRESULT Stage_ElementResource(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& OutResource,
		std::string& strOutError,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr) const;
	HRESULT Stage_ModelCueResource(
		const EFFECT_MODEL_CUE_DESC& Cue,
		MODEL_CUE_RESOURCE& OutResource,
		std::string& strOutError,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr) const;
	bool_t Build_PreparedDocument(
		uint64_t iCatalogRevision,
		const std::string& strEffectAssetId,
		const EFFECT_DOCUMENT_DESC& Document,
		PREWARM_ASSET_CACHE* pSharedAssets,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError) const;
	bool_t Clone_ModelCueResources(
		const PREPARED_DOCUMENT& Prepared,
		std::unordered_map<std::string, MODEL_CUE_RESOURCE>& OutResources,
		std::string& strOutError) const;
	bool_t Ensure_MutableInstanceBuffers(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	HRESULT Load_Texture(
		const std::string& strAssetId,
		ComPtr<ID3D11ShaderResourceView>& OutSRV,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr) const;
	HRESULT Load_SourceTexture(
		const EFFECT_NAMED_TEXTURE_DESC& Texture,
		ComPtr<ID3D11ShaderResourceView>& OutSRV,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr) const;
	HRESULT Bind_Common(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Bind_Common(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Bind_MaterialInputs(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f) const;
	HRESULT Render_Element(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Mesh(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f,
		const float4x4_t* pWorldOverride = nullptr,
		const float4_t* pDynamicParameter = nullptr);
	HRESULT Render_Rect(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f,
		const float4x4_t* pWorldOverride = nullptr);
	HRESULT Render_Decal(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Particles(
		const EFFECT_EVALUATED_FRAME& Frame,
		std::span<const EFFECT_EVALUATED_PARTICLE> Particles);
	HRESULT Render_Trails(
		const EFFECT_EVALUATED_FRAME& Frame,
		std::span<const EFFECT_EVALUATED_TRAIL> Trails);
	HRESULT Render_AfterImages(
		const EFFECT_EVALUATED_FRAME& Frame,
		std::span<const EFFECT_EVALUATED_AFTERIMAGE> AfterImages);
	HRESULT Render_ModelCues(const EFFECT_EVALUATED_FRAME& Frame);
	uint32_t Select_Pass(EFFECT_RENDER_PROFILE eProfile) const;
	const ELEMENT_RESOURCE* Find_Resource(const std::string& strElementId) const;

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	EFFECT_DOCUMENT_DESC m_Document;
	std::shared_ptr<const PREPARED_DOCUMENT> m_pPreparedDocument;
	CEffectReconstructedRuntimeBoundary m_ReconstructedRuntimeBoundary;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> m_ModelCueResources;
	shared_ptr<Engine::CShader> m_pMeshShader;
	shared_ptr<Engine::CShader> m_pAnimatedModelShader;
	shared_ptr<Engine::CShader> m_pRectShader;
	shared_ptr<Engine::CShader> m_pParticleShader;
	shared_ptr<Engine::CShader> m_pTrailShader;
	shared_ptr<Engine::CShader> m_pDecalShader;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CVIBuffer_ParticleRect> m_pParticleBuffer;
	shared_ptr<Engine::CVIBuffer_DynamicTrail> m_pTrailBuffer;
	ComPtr<ID3D11ShaderResourceView> m_pWhiteTexture;
	ComPtr<ID3D11ShaderResourceView> m_pBlackTexture;
	std::string m_strStatus;
};

NS_END

