#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_ComponentDocument.h"
#include "Effect_RuntimeAuthority.h"

#include <cstdint>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

NS_BEGIN(Client)

struct EFFECT_OCCURRENCE_TUNING_DOCUMENT;
struct EFFECT_VISUAL_PROGRAM;
struct EFFECT_VISUAL_PROGRAM_CORPUS;
struct EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION;

struct EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY final
{
	uint64_t iCatalogRevision = 0u;
	uint32_t iArtifactRevision = 0u;
	uint32_t iProgramVersion = 0u;
	uint32_t iInputArtifactCount = 0u;
	uint64_t iCandidateByteCount = 0u;
	std::string strEffectAssetId;
	std::string strCompilerRevision;
	std::string strCandidateBuilderCommitId;
	std::string strCandidateBuilderTreeId;
	std::string strCandidateBlobId;
	std::string strProgramId;
	std::string strProgramSha256;
	std::string strCandidateRawSha256;
	std::string strResourceBindingHash;
	std::string strInputArtifactsOrderedSha256;
	std::string strReconstructedRuntimeProgramSha256;
	std::string strPublishReceiptSha256;
	uint32_t iRenderResourceSidecarFormatVersion = 0u;
	uint64_t iRenderResourceSidecarByteCount = 0u;
	std::string strRenderResourceSidecarSchema;
	std::string strRenderResourceAuthorityId;
	std::string strRenderResourceSidecarDecisionProjectionSha256;
	std::string strRenderResourceSidecarReceiptSha256;
	std::string strRenderResourceSidecarRawSha256;
	std::string strRenderResourceAuthorityLinkSha256;
	std::string strRenderResourcePublishReceiptSha256;
	uint32_t iOccurrenceTuningEntryCount = 0u;
	std::string strOccurrenceTuningSourcePath;
	std::string strOccurrenceTuningSha256;
};

struct EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY final
{
	DXGI_FORMAT eFormat = DXGI_FORMAT_UNKNOWN;
	D3D11_SRV_DIMENSION eViewDimension = D3D11_SRV_DIMENSION_UNKNOWN;
	uint32_t iMostDetailedMip = 0u;
	uint32_t iMipLevels = 0u;
	std::string strFormatName;
	std::string strViewDimensionName;
	std::string strColorSpace;
};

struct EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE final
{
	uint32_t iOrder = 0u;
	uint64_t iByteCount = 0u;
	std::string strResourceAuthorityId;
	std::string strRuntimeAssetId;
	std::string strRawSha256;
	std::string strRowSha256;
	EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY ExpectedSrv;
};

struct EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING final
{
	uint32_t iOrder = 0u;
	uint64_t iActualDdsByteCount = 0u;
	std::string strBindingAuthorityId;
	std::string strCandidateBindingId;
	std::string strCandidateBindingRowSha256;
	std::string strRecipeId;
	std::string strMaterialInputFieldId;
	std::string strSamplerPolicyRowId;
	std::string strSamplerPolicyRowSha256;
	std::vector<std::string> MaterialOccurrenceIds;
	std::string strRuntimeAssetId;
	std::string strResourceAuthorityId;
	std::string strResourceAuthorityRowSha256;
	D3D11_SAMPLER_DESC SamplerDescriptor{};
	std::string strSamplerSrvColorSpace;
	EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY ActualDdsSrv;
	std::string strActualDdsRawSha256;
	std::string strRowSha256;
};

struct EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER final
{
	uint32_t iOrder = 0u;
	std::string strNeutralProviderId;
	std::array<float, 4u> RgbaF32{};
	std::string strEvaluatorSemantic;
	float fSecondaryMultiplyFactor = 0.f;
	float fSignedDistortionOffset = 0.f;
	std::string strRowSha256;
};

struct EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER final
{
	std::string strProviderKind;
	std::string strNeutralProviderId;
	std::string strMaterialInputFieldId;
	std::string strMaterialInputRowSha256;
	std::string strTextureBindingId;
	std::string strTextureBindingRowSha256;
	std::string strSamplerPolicyRowId;
	std::string strSamplerPolicyRowSha256;
	std::string strRuntimeAssetId;
	std::string strSelectionBasis;
};

struct EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING final
{
	uint32_t iOrder = 0u;
	uint32_t iFeatureMask = 0u;
	bool_t bSecondTextureOperationEnabled = false;
	bool_t bDistortionOperationEnabled = false;
	float fDistortionStrength = 0.f;
	std::string strRecipeTextureDecisionId;
	std::string strRecipeId;
	std::string strRecipeRowSha256;
	std::string strFamilyId;
	std::string strFamilyRowSha256;
	EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER Texture0Provider;
	EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER Texture1Provider;
	std::string strRowSha256;
};

struct EFFECT_RECONSTRUCTED_RENDERER_SLOT_BINDING final
{
	uint32_t iOrder = 0u;
	uint32_t iCandidateCount = 0u;
	std::string strRendererBindingDecisionId;
	std::string strTextureResourceId;
	std::string strRendererResourceRowSha256;
	std::string strMaterialOccurrenceId;
	std::string strMaterialOccurrenceRowSha256;
	std::string strRecipeId;
	std::string strSlotId;
	std::string strRuntimeAssetId;
	std::string strSelectedMaterialInputFieldId;
	std::string strSelectedMaterialInputRowSha256;
	std::string strSelectedNormalizedParameterName;
	std::string strSelectedTextureBindingId;
	std::string strSelectedTextureBindingRowSha256;
	std::string strSelectedSamplerPolicyRowId;
	std::string strSelectedSamplerPolicyRowSha256;
	std::string strDecisionBasis;
	std::string strRowSha256;
};

enum class EFFECT_RECONSTRUCTED_RENDER_STATE_KIND : uint8_t
{
	BLEND,
	RASTERIZER,
	DEPTH_STENCIL,
	END
};

struct EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR final
{
	uint32_t iOrder = 0u;
	EFFECT_RECONSTRUCTED_RENDER_STATE_KIND eKind =
		EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::END;
	std::string strRenderStateDecisionId;
	std::string strRenderBindingId;
	std::string strRenderBindingRowSha256;
	std::string strRecipeId;
	std::string strRecipeRowSha256;
	std::string strFieldName;
	std::string strImplementationStateName;
	std::string strRowSha256;
	D3D11_BLEND_DESC BlendDescriptor{};
	D3D11_RASTERIZER_DESC RasterizerDescriptor{};
	D3D11_DEPTH_STENCIL_DESC DepthStencilDescriptor{};
};

struct EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_IDENTITY final
{
	uint32_t iFormatVersion = 0u;
	uint32_t iProgramVersion = 0u;
	uint64_t iSidecarByteCount = 0u;
	std::string strSchema;
	std::string strAuthorityId;
	std::string strProgramId;
	std::string strProgramSha256;
	std::string strSidecarDecisionProjectionSha256;
	std::string strSidecarReceiptSha256;
	std::string strSidecarRawSha256;
	std::string strAuthorityLinkSha256;
	std::string strPublishReceiptSha256;
};

struct EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY final
{
	EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_IDENTITY Identity;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_TEXTURE_RESOURCE,
		std::less<>> TextureResourcesById;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING,
		std::less<>> TextureBindingsById;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_NEUTRAL_PROVIDER,
		std::less<>> NeutralProvidersById;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING,
		std::less<>> RecipeTextureBindingsById;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDERER_SLOT_BINDING,
		std::less<>> RendererSlotBindingsById;
	std::map<std::string, EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR,
		std::less<>> RenderStateDescriptorsById;
};

class CEffectCatalog;

class EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY final
{
public:
	const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& Get_Identity() const
	{
		return m_Identity;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return m_pProgram;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Get_RenderResourceAuthority() const
	{
		return m_pRenderResourceAuthority;
	}
	std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>
		Get_OccurrenceTuning() const
	{
		return m_pOccurrenceTuning;
	}

private:
	friend class CEffectCatalog;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY&) = delete;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY& operator=(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY&) = delete;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY(
		EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY Identity,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
			pRenderResourceAuthority,
		std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>
			pOccurrenceTuning)
		: m_Identity(std::move(Identity)), m_pProgram(std::move(pProgram)),
		  m_pRenderResourceAuthority(std::move(pRenderResourceAuthority)),
		  m_pOccurrenceTuning(std::move(pOccurrenceTuning))
	{
	}

private:
	EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY m_Identity;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> m_pProgram;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		m_pRenderResourceAuthority;
	std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>
		m_pOccurrenceTuning;
};

struct EFFECT_RECONSTRUCTED_ANCHOR_BINDING final
{
	std::string strOwnerEmitterId;
	EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST Request;
};

class EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION final
{
public:
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_CatalogEntry() const
	{
		return m_pCatalogEntry;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return nullptr == m_pCatalogEntry ? nullptr :
			m_pCatalogEntry->Get_Program();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Get_RenderResourceAuthority() const
	{
		return nullptr == m_pCatalogEntry ? nullptr :
			m_pCatalogEntry->Get_RenderResourceAuthority();
	}
	const std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING>&
		Get_AnchorRequests() const
	{
		return m_AnchorRequests;
	}

private:
	friend class CEffectCatalog;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION(
		const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION&) = delete;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION& operator=(
		const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION&) = delete;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION(
		std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pCatalogEntry,
		std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING> AnchorRequests)
		: m_pCatalogEntry(std::move(pCatalogEntry)),
		  m_AnchorRequests(std::move(AnchorRequests))
	{
	}

private:
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> m_pCatalogEntry;
	std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING> m_AnchorRequests;
};

enum class EFFECT_RECONSTRUCTED_RUNTIME_SEAM : uint8_t
{
	OBJECT,
	PLAYBACK,
	RENDERER,
	END
};

class CEffectReconstructedRuntimeBoundary final
{
public:
	static bool_t Prepare_Presentation(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutError);
	static bool_t Admit_ProductSpawn(
		const std::string& strEffectAssetId,
		std::string& strOutError);
	bool_t Stage(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM eSeam,
		std::string& strOutError);
	bool_t Admit_Execution(std::string& strOutError) const;
	bool_t Admit_Submit(std::string& strOutError) const;
	bool_t Admit_Render(std::string& strOutError) const;
	void Clear() { m_pPreparation.reset(); }
	bool_t Is_Staged() const { return nullptr != m_pPreparation; }
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_Preparation() const
	{
		return m_pPreparation;
	}
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_CatalogEntry() const
	{
		return nullptr == m_pPreparation ? nullptr :
			m_pPreparation->Get_CatalogEntry();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return nullptr == m_pPreparation ? nullptr :
			m_pPreparation->Get_Program();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Get_RenderResourceAuthority() const
	{
		return nullptr == m_pPreparation ? nullptr :
			m_pPreparation->Get_RenderResourceAuthority();
	}

private:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		m_pPreparation;
};

class CEffectCatalog final
{
public:
    struct RUNTIME_SNAPSHOT;

    static bool_t Load(std::string& strOutStatus);
    static std::shared_ptr<const RUNTIME_SNAPSHOT> Capture_Runtime();
    static bool_t Restore_Runtime(
        std::shared_ptr<const RUNTIME_SNAPSHOT> pSnapshot,
        std::string& strOutStatus);
    static std::shared_ptr<const EFFECT_DOCUMENT_DESC> Find(
        const std::string& strEffectAssetId);
    static std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Find_Assembly(
        const std::string& strEffectAssetId);
    static std::shared_ptr<const EFFECT_COMPONENT_DESC> Find_Component(
        const std::string& strComponentAssetId);
	static std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>
		Find_RuntimeAuthority(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Find_RuntimeProgramEntry(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Find_ReconstructedRuntimeProgram(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>
		Find_VisualProgramCorpus();
	static std::shared_ptr<const EFFECT_VISUAL_PROGRAM>
		Find_VisualProgram(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		Find_VisualProjection(const std::string& strEffectAssetId);
	static bool_t Prepare_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutError);
    static bool_t Contains(const std::string& strEffectAssetId);
	static bool_t Contains_RuntimeAuthority(
		const std::string& strEffectAssetId);
	static bool_t Contains_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId);
	static bool_t Is_ReconstructedRuntimeProgramAssetId(
		const std::string& strEffectAssetId);
    static std::vector<std::string> Get_EffectAssetIds();
    static std::vector<std::string> Get_ComponentAssetIds();
	static std::vector<std::string> Get_RuntimeAuthorityAssetIds();
	static std::vector<std::string> Get_ReconstructedRuntimeProgramAssetIds();
	static std::vector<std::string> Get_VisualProgramAssetIds();
    static uint64_t Get_RuntimeRevision();
    static const std::string& Get_Status();
    static void Clear();
};

NS_END

