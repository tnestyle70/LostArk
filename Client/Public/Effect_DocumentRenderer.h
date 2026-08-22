#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_MaterialProgramRegistry.h"
#include "Effect_VisualProgramCorpus.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
class CVIBuffer_ParticleRect;
class CVIBuffer_DynamicTrail;
struct VTXEFFECT_PARTICLE;
struct VTXEFFECT_TRAIL;
NS_END

NS_BEGIN(Client)

class CValtanTranslatedCanaryRuntime;
struct VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET;

enum class RECONSTRUCTED_DIAGNOSTIC_SOLO : uint8_t
{
	MESH,
	SPRITE,
	END
};

struct EFFECT_RENDER_PREWARM_PROBE final
{
	uint64_t iCoreBuildCount = 0u;
	uint64_t iCatalogCommitCount = 0u;
	uint64_t iPreparedDocumentBuildCount = 0u;
	uint64_t iModelDiskLoadCount = 0u;
	uint64_t iTextureDiskLoadCount = 0u;
	uint64_t iVectorFieldDiskLoadCount = 0u;
	uint64_t iPreparedAttachCount = 0u;
	uint64_t iPreparedIdentityLookupCount = 0u;
	uint64_t iMutableInstanceBufferBuildCount = 0u;
	uint64_t iSynchronousDocumentStageCount = 0u;
	uint64_t iPreparedLookupMissCount = 0u;
	uint64_t iCatalogRevision = 0u;
	uint64_t iMaterialProgramRegistryGeneration = 0u;
	uint32_t iPreparedDocumentCount = 0u;
	uint32_t iMaterialProgramBindingCount = 0u;
	uint32_t iMaterialProgramResolvedElementCount = 0u;
};

struct EFFECT_DECAL_SHADER_PROJECTION_DESC final
{
	float2_t vSize = { 1.f, 1.f };
	f32_t fDepth = 1.f;
};

struct EFFECT_PARTICLE_SPRITE_SCALE_DESC final
{
	float3_t vScale = { 1.f, 1.f, 1.f };
};

struct EFFECT_RENDER_PREWARM_TARGET final
{
	std::string strEffectAssetId;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
};

enum class EFFECT_GPU_RENDER_FAMILY : uint8_t
{
	MESH,
	SPRITE,
	DECAL,
	RIBBON,
	END
};

enum class EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND : uint8_t
{
	ALL,
	FAMILY,
	OCCURRENCE,
	ELEMENT_SET,
	END
};

/* Authoring-only carrier for a sealed UE3 cooked pixel-shader variant.  The
   registration key is material identity, never a gameplay skill or occurrence
   id, so the same sealed variant can be auditioned by every matching Effect. */
enum class EFFECT_EXACT_PREVIEW_CARRIER : uint8_t
{
	SPRITE_PARTICLE,
	LOCAL_MESH,
	END
};

enum class EFFECT_EXACT_PREVIEW_OUTPUT_CONTRACT : uint8_t
{
	/* The admitted CustomParticle/Helix canary writes opacity into RGB while
	   returning zero alpha.  ONE/ONE is therefore part of the sealed preview
	   contract and must not inherit the family-lite SrcAlpha blend state. */
	ADDITIVE_ONE_ONE_RT0_ZERO_ALPHA,
	END
};

struct EFFECT_EXACT_PREVIEW_TEXTURE_BINDING_DESC final
{
	std::string strAssetId;
	EFFECT_TEXTURE_COLOR_SPACE eColorSpace =
		EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
	uint32_t iTextureRegister = UINT32_MAX;
	uint32_t iSamplerRegister = UINT32_MAX;
	EFFECT_MATERIAL_SAMPLER_DESC Sampler;
};

/* Non-owning install view produced by the cooked-shader sidecar loader.  The
   renderer validates and owns every byte/resource before this call returns. */
struct EFFECT_EXACT_PREVIEW_VARIANT_DESC final
{
	std::string strVariantKey;
	std::string strSourceMaterialPath;
	EFFECT_EXACT_PREVIEW_CARRIER eCarrier =
		EFFECT_EXACT_PREVIEW_CARRIER::END;
	EFFECT_EXACT_PREVIEW_OUTPUT_CONTRACT eOutputContract =
		EFFECT_EXACT_PREVIEW_OUTPUT_CONTRACT::END;
	std::span<const uint8_t> PixelShaderBytecode;
	uint64_t iExpectedPixelShaderByteCount = 0u;
	std::string strExpectedPixelShaderSha256;
	std::vector<float4_t> CBuffer0Rows;
	std::vector<EFFECT_EXACT_PREVIEW_TEXTURE_BINDING_DESC> TextureBindings;
	/* Optional row/component whose sealed scalar is multiplied by a draw-local
	   opacity scale. UINT32_MAX leaves the exact CB0 bytes untouched. */
	uint32_t iExternalOpacityRow = UINT32_MAX;
	uint32_t iExternalOpacityComponent = UINT32_MAX;
};

/* Object-local Tool preview control.  It never changes the immutable source
   document or evaluator scope; it only selects which already-evaluated Core
   renderer occurrences may reach their GPU draw carrier. */
struct EFFECT_PREVIEW_SUBMISSION_ISOLATION final
{
	EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND eKind =
		EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL;
	EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
	std::string strElementId;
	std::vector<std::string> ElementIds;
};

struct EFFECT_GPU_RENDER_FAMILY_STATS final
{
	uint64_t iConfigured = 0u;
	uint64_t iEvaluated = 0u;
	uint64_t iActive = 0u;
	uint64_t iCandidate = 0u;
	uint64_t iZeroCandidate = 0u;
	uint64_t iAttempted = 0u;
	uint64_t iSubmitted = 0u;
	uint64_t iSuppressed = 0u;
	uint64_t iFailed = 0u;
};

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
enum class EFFECT_GPU_RENDER_CARRIER : uint8_t
{
	MESH_CMODEL,
	SPRITE_RECT,
	SPRITE_INSTANCE,
	DECAL_RECT,
	RIBBON_DYNAMIC_TRAIL,
	END
};

/* Focused execution evidence for one stable renderer occurrence.  These rows
   are compiled only into the executable harness; production rendering keeps
   the same code path without retaining per-draw diagnostic strings. */
struct EFFECT_GPU_RENDER_OCCURRENCE_STATS final
{
	std::string strElementId;
	EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
	uint64_t iConfigured = 0u;
	uint64_t iEvaluated = 0u;
	uint64_t iActive = 0u;
	uint64_t iCandidateRowCount = 0u;
	uint64_t iAttempted = 0u;
	uint64_t iMaterialBindCount = 0u;
	uint64_t iTextureSrvBindCount = 0u;
	uint64_t iSamplerBindCount = 0u;
	uint64_t iShaderPassApplyCount = 0u;
	uint64_t iVIBufferBindCount = 0u;
	uint64_t iVIBufferDrawCount = 0u;
	uint64_t iGeometryUploadCount = 0u;
	uint64_t iIssuedDrawCallCount = 0u;
	uint64_t iDrawSelectionCount = 0u;
	uint64_t iSubmitted = 0u;
	uint64_t iSuppressed = 0u;
	uint64_t iFailed = 0u;
	uint32_t iSelectedPassIndex = UINT32_MAX;
	uint32_t iSourceMaterialProfile = UINT32_MAX;
	uint32_t iSourceTextureMask = 0u;
	EFFECT_GPU_RENDER_CARRIER eCarrier = EFFECT_GPU_RENDER_CARRIER::END;
	bool_t bSourceMaterialFallbackBlocked = false;
	bool_t bDrawSelectionDiverged = false;
	float3_t vSubmittedPositionMin{};
	float3_t vSubmittedPositionMax{};
	bool_t bHasSubmittedPosition = false;
	float4x4_t FirstSubmittedParticleWorld{};
	bool_t bHasFirstSubmittedParticleWorld = false;
	uint64_t iFinalTrailUploadedVertexCount = 0u;
	float3_t vFinalTrailPairCenterMin{};
	float3_t vFinalTrailPairCenterMax{};
	f32_t fFinalTrailPairWidthMin = 0.f;
	f32_t fFinalTrailPairWidthMax = 0.f;
	bool_t bHasFinalTrailPairCenter = false;
};
#endif

struct EFFECT_GPU_RENDER_SUBMISSION_STATS final
{
	std::array<EFFECT_GPU_RENDER_FAMILY_STATS,
		static_cast<size_t>(EFFECT_GPU_RENDER_FAMILY::END)> Families{};
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	std::vector<EFFECT_GPU_RENDER_OCCURRENCE_STATS> Occurrences;
#endif
	bool_t bCompleted = false;
	bool_t bCommitted = false;
};

class CEffectDocumentRenderer final
{
public:
	struct PREPARED_DOCUMENT;
	struct PRODUCT_PREWARM_SESSION;
	struct EXACT_PREVIEW_PROGRAM;
	struct EXACT_PREVIEW_ELEMENT_PACKET;
	struct GLASSHOLE02_TRANSLATED_CANARY_ELEMENT_PACKET;

	/* Tool-only translated-HLSL canary contract.  Product preparation never
	   enables this gate, and the stable occurrence must fail closed instead of
	   falling through to the family-lite particle shader. */
	static constexpr std::string_view
		GLASSHOLE02_TRANSLATED_CANARY_EFFECT_ASSET_ID =
		"effect.dimensionmaster.skill.2050120.clip3.unified";
	static constexpr std::string_view
		GLASSHOLE02_TRANSLATED_CANARY_OCCURRENCE_ID =
		"authored.source-particle.40e1b48e2f0f88dcfeff1549";
	static constexpr std::string_view
		GLASSHOLE02_TRANSLATED_CANARY_FAMILY_ID =
		"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2";
	static constexpr std::string_view
		GLASSHOLE02_TRANSLATED_CANARY_PROFILE_ID =
		"effect.ue3.glasshole-02.v1";
	static constexpr uint32_t
		GLASSHOLE02_TRANSLATED_CANARY_REQUIRED_SOURCE_MASK = 0x7fu;
	static constexpr bool_t
		GLASSHOLE02_TRANSLATED_CANARY_DEFAULT_ENABLED = false;
	static constexpr bool_t
		GLASSHOLE02_TRANSLATED_CANARY_PRODUCT_ENABLED = false;
	static constexpr bool_t
		GLASSHOLE02_TRANSLATED_CANARY_FAIL_CLOSED = true;

	/* One Tool-only gate owns the three source families selected for the first
	   Valtan FRONT_BACK_FRONT family audition.  Product preparation never
	   enables it; exact HLSL draw failures suppress only the admitted
	   occurrence instead of falling through to the family-lite shader. */
	static constexpr std::string_view
		VALTAN_TRANSLATED_CANARY_EFFECT_ASSET_ID =
		"effect.valtan.front-back-front.windup";
	static constexpr std::array<std::string_view, 9u>
		VALTAN_TRANSLATED_CANARY_OCCURRENCE_IDS = {{
		"par_n_rpbf_atk_01_02.em07",
		"par_n_rpbf_atk_01_02.em14",
		"par_n_rpbf_atk_04_11.em00",
		"par_n_rpbf_atk_04_11.em01",
		"par_n_rpbf_atk_04_12.em00",
		"par_n_rpbf_atk_04_12.em01",
		"par_n_rpbf_atk_04_12.em02",
		"par_n_rpbf_atk_04_13.em00",
		"par_n_rpbf_atk_04_13.em01",
	}};
	static constexpr bool_t
		VALTAN_TRANSLATED_CANARY_DEFAULT_ENABLED = false;
	static constexpr bool_t
		VALTAN_TRANSLATED_CANARY_PRODUCT_ENABLED = false;
	static constexpr bool_t
		VALTAN_TRANSLATED_CANARY_FAIL_CLOSED = true;

private:
	struct ELEMENT_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		/* One lane per EFFECT_RESOURCE_SLOT texture slot, indexed by
		   slot - BASE_TEXTURE. Grew from 5 to 8 with base2/mask2/noise2. */
		std::array<ComPtr<ID3D11ShaderResourceView>, 8> Textures;
		std::array<ComPtr<ID3D11ShaderResourceView>, 7> SourceTextures;
		uint32_t iSourceTextureMask = 0u;
		uint32_t iSourceMaterialProfile = 0u;
		float4_t vSourceScalars0{};
		float4_t vSourceScalars1{};
		float4_t vSourceVector0{};
		float4_t vSourceVector1{};
		std::array<float4_t, 8u> TypedTrailParameters{};
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
		uint32_t iReconstructedMaterialEvaluatorEnabled = 0u;
		uint32_t iReconstructedMaterialFeatureMask = 0u;
		uint32_t iRuntimeMaterialV2Enabled = 0u;
		uint32_t iRuntimeMaterialV2Opcode = 0u;
		uint32_t iRuntimeMaterialV2TextureLaneCount = 0u;
		uint32_t iRuntimeMaterialV2TextureMask = 0u;
		uint32_t iRuntimeMaterialV2DynamicConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2DynamicSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorPolicy = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2ScalarCount = 0u;
		uint32_t iRuntimeMaterialV2VectorCount = 0u;
		uint32_t iRuntimeMaterialV2InputCount = 0u;
		std::array<uint32_t, 2u> RuntimeMaterialV2InputConsumedMask{};
		std::array<uint32_t, 2u> RuntimeMaterialV2InputSuppressedMask{};
		uint32_t iRuntimeMaterialV2StaticInputCount = 0u;
		uint32_t iRuntimeMaterialV2StaticSelectedMask = 0u;
		uint32_t iRuntimeMaterialV2StaticConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2StaticSuppressedMask = 0u;
		uint32_t iRuntimeMaterialV2RenderInputCount = 0u;
		uint32_t iRuntimeMaterialV2RenderConsumedMask = 0u;
		uint32_t iRuntimeMaterialV2RenderSuppressedMask = 0u;
		uint32_t iStandardColorV1Enabled = 0u;
		std::array<uint32_t, 4u> StandardColorV1Header{};
		std::array<uint32_t, 4u> StandardColorV1BaseCoverage{};
		std::array<uint32_t, 4u> StandardColorV1Dissolve{};
		std::array<uint32_t, 4u> StandardColorV1Policies{};
		float4_t vStandardColorV1Scalars{};
		EFFECT_STANDARD_COLOR_V1_DESC StandardColorV1;
		/* Artist F V4 is a finite, occurrence-admitted visual program.  It
		   deliberately owns a separate opcode namespace from RuntimeMaterialV2;
		   SourceTextures remain the shared typed SRV carrier. */
		uint32_t iArtistVisualV4Opcode = 0u;
		uint32_t iArtistVisualV4TextureMask = 0u;
		std::array<float4_t, 8u> ArtistVisualV4Params{};
		std::array<float4_t, 2u> ArtistVisualV4Colors{};
		std::array<float4_t, 13u> RuntimeMaterialV2ScalarBlocks{};
		std::array<float4_t, 3u> RuntimeMaterialV2Vectors{};
		std::array<uint32_t, 3u>
			RuntimeMaterialV2VectorComponentConsumedMask{};
		std::array<uint32_t, 3u>
			RuntimeMaterialV2VectorComponentSuppressedMask{};
		std::array<ComPtr<ID3D11SamplerState>, 6u>
			RuntimeMaterialV2Samplers{};
		/* Import-time capture only.  The ordinary authored stage reconstructs
		   the same GPU resources from Material.Execution and never consults the
		   reconstructed source sidecars. */
		std::array<std::optional<EFFECT_MATERIAL_TEXTURE_LANE_DESC>, 6u>
			MaterialExecutionLanes{};
		float2_t vReconstructedUVScale{ 1.f, 1.f };
		float4_t vReconstructedPanRotationAux{};
		float4_t vReconstructedColor{ 1.f, 1.f, 1.f, 1.f };
		float4_t vReconstructedParams0{};
		float4_t vReconstructedParams1{};
		uint32_t iSourceTextureClampUMask = 0u;
		uint32_t iSourceTextureClampVMask = 0u;
		std::array<uint32_t, 4u> DynamicParameterSemantics{};
		EFFECT_GROUPED_TRANSLUCENT_CONSTANTS GroupedConstants;
		bool_t bSourceMaterialFallbackBlocked = false;
		bool_t bOccurrenceVisualSuppressed = false;
		std::shared_ptr<const EXACT_PREVIEW_ELEMENT_PACKET>
			pExactPreviewPacket;
		std::shared_ptr<const GLASSHOLE02_TRANSLATED_CANARY_ELEMENT_PACKET>
			pGlasshole02TranslatedCanaryPacket;
		std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>
			pValtanTranslatedCanaryPacket;
		std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
			pMaterialProgramBinding;
	};
	struct MODEL_CUE_RESOURCE final
	{
		shared_ptr<Engine::CModel> pModel;
		uint32_t iAnimationIndex = 0u;
		f32_t fTicksPerSecond = 0.f;
		f32_t fDurationSeconds = 0.f;
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
	static bool_t Prepare_VisualProgramCatalog(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint64_t iCatalogRevision,
		const std::vector<EFFECT_RENDER_PREWARM_TARGET>& Targets,
		std::string& strOutError);
	/* Main-thread-only Product step.  A successful call merges exactly one
	   target while preserving the other targets and the shared asset session. */
	static bool_t Prepare_VisualProgramTarget(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint64_t iCatalogRevision,
		const EFFECT_RENDER_PREWARM_TARGET& Target,
		std::string& strOutError);
	/* Main-thread-only authoring handoff.  The candidate is fully prepared
	   before the cache lock is reacquired, then every prepared entry for only
	   this Effect ID/revision is replaced in one commit.  Existing EffectObject
	   instances retain their shared prepared document; only subsequent lookups
	   can observe the candidate.  Failure leaves the prior cache untouched. */
	static bool_t Replace_VisualProgramTarget(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint64_t iCatalogRevision,
		const EFFECT_RENDER_PREWARM_TARGET& Target,
		std::string& strOutError);
	/* Main-thread authoring-preview registration. Empty Variants clears the
	   device-local registry. Product preparation never enables this path. */
	static bool_t Install_AuthoringExactPreviewVariants(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::span<const EFFECT_EXACT_PREVIEW_VARIANT_DESC> Variants,
		std::string& strOutError);
	static std::shared_ptr<const PREPARED_DOCUMENT> Find_Prepared(
		uint64_t iCatalogRevision,
		const std::string& strEffectAssetId,
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection = nullptr,
		std::shared_ptr<const CEffectMaterialProgramRegistry>
			pMaterialProgramRegistry = nullptr);
	static std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		Get_PlaybackResources(
			const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared);
	static EFFECT_RENDER_PREWARM_PROBE Get_PrewarmProbe();
	static bool_t Resolve_ParticleSpriteScale(
		const EFFECT_EVALUATED_PARTICLE& Particle,
		const float3_t& vDecomposedMagnitude,
		EFFECT_PARTICLE_SPRITE_SCALE_DESC& OutScale)
	{
		if (!std::isfinite(vDecomposedMagnitude.x) ||
			!std::isfinite(vDecomposedMagnitude.y) ||
			!std::isfinite(vDecomposedMagnitude.z) ||
			vDecomposedMagnitude.x < 0.f || vDecomposedMagnitude.y < 0.f ||
			vDecomposedMagnitude.z < 0.f)
		{
			return false;
		}
		EFFECT_PARTICLE_SPRITE_SCALE_DESC Staged;
		Staged.vScale = vDecomposedMagnitude;
		if (EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE ==
			Particle.eSpriteAlignment)
		{
			Staged.vScale.y = Staged.vScale.x;
		}
		else
		{
			const bool_t bFixedAxis =
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_X ==
					Particle.eSpriteAlignment ||
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_X ==
					Particle.eSpriteAlignment ||
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Y ==
					Particle.eSpriteAlignment ||
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Y ==
					Particle.eSpriteAlignment ||
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Z ==
					Particle.eSpriteAlignment ||
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Z ==
					Particle.eSpriteAlignment;
			/* Cascade fixed-axis sprites commonly author a single StartSize X
			   component and use it for both dimensions of the renderer plane.
			   Preserving literal Y=0 collapses the shared rect before the pixel
			   shader.  Expand only this source-evidenced one-component form;
			   rectangular and rotating presentations keep their authored axes. */
			if (bFixedAxis && Staged.vScale.x > 1.e-6f &&
				Staged.vScale.y <= 1.e-6f)
			{
				Staged.vScale.y = Staged.vScale.x;
			}
		}
		OutScale = Staged;
		return true;
	}
	static bool_t Resolve_DecalShaderProjection(
		const EFFECT_EVALUATED_ELEMENT& Element,
		EFFECT_DECAL_SHADER_PROJECTION_DESC& OutProjection)
	{
		if (nullptr == Element.pElement ||
			EFFECT_ELEMENT_KIND::DECAL != Element.pElement->eKind)
		{
			return false;
		}
		EFFECT_DECAL_SHADER_PROJECTION_DESC Staged{};
		if (Element.bWorldOwnsDecalProjectionVolume)
		{
			/* Reconstructed Cascade decals already bake source size and depth
			   into Element.World.  The shader must therefore receive identity
			   projection constants; leaving the value-initialized zeros here
			   rejects the otherwise valid draw, while reusing authored sizes
			   would apply the volume twice. */
			Staged.vSize = { 1.f, 1.f };
			Staged.fDepth = 1.f;
		}
		else
		{
			Staged.vSize = Element.pElement->Detail.Decal.vSize;
			Staged.fDepth = Element.pElement->Detail.Decal.fDepth;
		}
		if (!std::isfinite(Staged.vSize.x) ||
			!std::isfinite(Staged.vSize.y) ||
			!std::isfinite(Staged.fDepth) || Staged.vSize.x <= 0.f ||
			Staged.vSize.y <= 0.f || Staged.fDepth <= 0.f)
		{
			return false;
		}
		OutProjection = Staged;
		return true;
	}
	static HRESULT Validate_DecalProjectionWorld(
		const EFFECT_EVALUATED_ELEMENT& Element)
	{
		if (nullptr == Element.pElement ||
			EFFECT_ELEMENT_KIND::DECAL != Element.pElement->eKind)
		{
			return E_INVALIDARG;
		}
		if (!Element.bWorldOwnsDecalProjectionVolume)
			return S_OK;
		const f32_t* const pWorldValues = &Element.World._11;
		if (!std::all_of(pWorldValues, pWorldValues + 16u,
			[](const f32_t fValue) { return std::isfinite(fValue); }))
		{
			return E_INVALIDARG;
		}
		const f32_t fDeterminant = XMVectorGetX(XMMatrixDeterminant(
			XMLoadFloat4x4(&Element.World)));
		if (!std::isfinite(fDeterminant))
			return E_INVALIDARG;
		return std::abs(fDeterminant) <= 0.00000001f ? S_FALSE : S_OK;
	}
	static void Clear_Prepared_Catalog();
	static bool_t Prepare_ReconstructedSourceRuntime(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError);
	static bool_t Prepare_VisualProgramDocument(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError);
	static bool_t Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError);
	/* One-way authoring bridge.  It executes the existing immutable Track A
	   preparation once, then returns self-contained per-Element material
	   snapshots.  Runtime Stage_Document consumes only those snapshots. */
	static bool_t Bake_ReconstructedMaterialExecutionSnapshots(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>&
			OutSnapshots,
		std::string& strOutError);

	HRESULT Initialize();
	bool_t Stage_Prepared(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::string& strOutError);
	bool_t Stage_Document(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	/* Must be enabled before Stage_Document. Clear() returns it to false. */
	bool_t Set_AuthoringExactPreviewExecutionEnabled(
		bool_t bEnabled,
		std::string& strOutError);
	bool_t Is_AuthoringExactPreviewExecutionEnabled() const
	{
		return m_bAuthoringExactPreviewExecutionEnabled;
	}
	/* This translated-HLSL canary is intentionally independent from the raw
	   cooked preview registry.  Enable it before Stage_Document; Clear and all
	   Product/prepared paths return it to OFF. */
	bool_t Set_AuthoringGlasshole02TranslatedCanaryEnabled(
		bool_t bEnabled,
		std::string& strOutError);
	bool_t Is_AuthoringGlasshole02TranslatedCanaryEnabled() const
	{
		return m_bAuthoringGlasshole02TranslatedCanaryEnabled;
	}
	bool_t Set_AuthoringValtanTranslatedCanaryEnabled(
		bool_t bEnabled,
		std::string& strOutError);
	bool_t Is_AuthoringValtanTranslatedCanaryEnabled() const
	{
		return m_bAuthoringValtanTranslatedCanaryEnabled;
	}
	bool_t Stage_ReconstructedRuntimeProgram(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_PrevalidatedVisualProgramDocument(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntime(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError);
	bool_t Stage_ReconstructedDiagnostic(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
		std::string& strOutError);
	bool_t Has_NonBlendModelCues() const;
	HRESULT Render_NonBlendModelCues(const EFFECT_EVALUATED_FRAME& Frame);
	HRESULT Render(const EFFECT_EVALUATED_FRAME& Frame);
	HRESULT Render_ReconstructedDiagnostic(
		const float4x4_t& RootWorld,
		RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo);
	bool_t Set_PreviewSubmissionIsolation(
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
		std::string& strOutError);
	void Reset_PreviewSubmissionIsolation();
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION&
		Get_PreviewSubmissionIsolation() const
	{
		return m_PreviewSubmissionIsolation;
	}
	void Clear();
	const std::string& Get_Status() const { return m_strStatus; }
	bool_t Is_LastRenderFailureObjectLocal() const
	{
		return m_bLastRenderFailureObjectLocal;
	}
	const EFFECT_GPU_RENDER_SUBMISSION_STATS&
		Get_LastRenderSubmissionStats() const
	{
		return m_LastRenderSubmissionStats;
	}
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
	struct RECONSTRUCTED_DIAGNOSTIC_COMPOSITE;

	bool_t Stage_PreparedInternal(
		const EFFECT_DOCUMENT_DESC& Document,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::string& strOutError,
		bool_t bAllowPreserveGlasshole02TranslatedCanary);
	HRESULT Stage_ElementResource(
		const std::string& strEffectAssetId,
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& OutResource,
		std::string& strOutError,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr,
		f32_t fModelPreScale = 1.f) const;
	bool_t Stage_AuthoringExactPreviewPacket(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& InOutResource,
		std::string& strOutError) const;
	bool_t Stage_Glasshole02TranslatedCanaryPacket(
		const std::string& strEffectAssetId,
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& InOutResource,
		std::string& strOutError) const;
	bool_t Stage_ValtanTranslatedCanaryPacket(
		const std::string& strEffectAssetId,
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& InOutResource,
		std::string& strOutError) const;
	bool_t Stage_AuthoredMaterialExecution(
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& InOutResource,
		std::string& strOutError,
		PREWARM_ASSET_CACHE* pSharedAssets = nullptr) const;
	bool_t Capture_MaterialExecutionLane(
		ELEMENT_RESOURCE& InOutResource,
		size_t iLane,
		std::string strAssetId,
		std::string strRole,
		std::string strSourceChannel,
		EFFECT_TEXTURE_COLOR_SPACE eColorSpace,
		const ComPtr<ID3D11SamplerState>& pSampler,
		std::string& strOutError) const;
	bool_t Build_MaterialExecutionSnapshot(
		const EFFECT_ELEMENT_DESC& Element,
		const ELEMENT_RESOURCE& Resource,
		EFFECT_MATERIAL_EXECUTION_DESC& OutSnapshot,
		std::string& strOutError) const;
	bool_t Stage_VisualProgramAdapter(
		const EFFECT_VISUAL_PROGRAM_ROW& Row,
		const EFFECT_ELEMENT_DESC& Element,
		ELEMENT_RESOURCE& InOutResource,
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
		std::string& strOutError,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation = nullptr,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection = nullptr,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC>
			pImmutableDocument = nullptr,
		std::shared_ptr<const CEffectMaterialProgramRegistry>
			pMaterialProgramRegistry = nullptr) const;
	bool_t Clone_ModelCueResources(
		const PREPARED_DOCUMENT& Prepared,
		std::unordered_map<std::string, MODEL_CUE_RESOURCE>& OutResources,
		std::string& strOutError) const;
	bool_t Validate_PreparedInstanceBuffers(
		const EFFECT_DOCUMENT_DESC& Document,
		const PREPARED_DOCUMENT& Prepared,
		std::string& strOutError) const;
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
		f32_t fAlphaScale = 1.f);
	HRESULT Bind_Common(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f);
	HRESULT Bind_MaterialInputs(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_COLOR_DESC& Color,
		f32_t fLocalTimeSeconds,
		f32_t fNormalizedLife,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f);
	HRESULT Render_Element(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Mesh(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale = 1.f,
		const float4x4_t* pWorldOverride = nullptr,
		const float4_t* pDynamicParameter = nullptr);
	HRESULT Render_AuthoringExactPreviewMesh(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		f32_t fAlphaScale,
		const float4x4_t& World,
		const float4x4_t& NormalMatrix,
		const float4_t& DynamicParameter);
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
	HRESULT Render_AuthoringExactPreviewParticles(
		const EFFECT_ELEMENT_DESC& Source,
		const ELEMENT_RESOURCE& Resource,
		std::span<const Engine::VTXEFFECT_PARTICLE> Instances);
	HRESULT Render_Glasshole02TranslatedCanaryParticles(
		const EFFECT_ELEMENT_DESC& Source,
		const ELEMENT_RESOURCE& Resource,
		f32_t fLocalTimeSeconds,
		std::span<const Engine::VTXEFFECT_PARTICLE> Instances);
	HRESULT Render_ValtanTranslatedCanaryMesh(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource,
		const float4x4_t& World,
		const float4x4_t& NormalMatrix,
		const float4_t& DynamicParameter);
	HRESULT Render_ValtanTranslatedCanaryGround(
		const EFFECT_EVALUATED_ELEMENT& Element,
		const ELEMENT_RESOURCE& Resource);
	HRESULT Render_Trails(
		const EFFECT_EVALUATED_FRAME& Frame,
		std::span<const EFFECT_EVALUATED_TRAIL> Trails);
	HRESULT Render_AfterImages(
		const EFFECT_EVALUATED_FRAME& Frame,
		std::span<const EFFECT_EVALUATED_AFTERIMAGE> AfterImages);
	HRESULT Render_ModelCues(
		const EFFECT_EVALUATED_FRAME& Frame,
		bool_t bNonBlendCharacterSurfaceOnly);
	HRESULT Fail_RenderOperation(
		std::string strOperation,
		HRESULT hResult = E_FAIL,
		bool_t bObjectLocal = false);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	void Record_TestMaterialBinding();
	void Record_TestSamplerBinding();
	void Record_TestShaderPassApplication();
	void Record_TestGeometryUpload();
	void Record_TestTrailGeometryUpload(
		std::span<const Engine::VTXEFFECT_TRAIL> Vertices);
	void Record_TestVIBufferBinding();
	void Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER eCarrier,
		uint32_t iSelectedPassIndex);
	void Record_TestIssuedDraw(const float4x4_t& World);
	void Record_TestIssuedDraw(
		std::span<const Engine::VTXEFFECT_PARTICLE> Instances);
	void Record_TestIssuedDraw(std::span<const Engine::VTXEFFECT_TRAIL> Vertices);
#endif
	uint32_t Select_Pass(EFFECT_RENDER_PROFILE eProfile) const;
	const ELEMENT_RESOURCE* Find_Resource(const std::string& strElementId) const;
	bool_t Should_SubmitPreviewOccurrence(
		const EFFECT_ELEMENT_DESC& Element,
		EFFECT_GPU_RENDER_FAMILY eFamily) const;
	const EFFECT_DOCUMENT_DESC& Get_StagedDocument() const;

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	EFFECT_DOCUMENT_DESC m_Document;
	std::shared_ptr<const PREPARED_DOCUMENT> m_pPreparedDocument;
	std::unique_ptr<RECONSTRUCTED_DIAGNOSTIC_COMPOSITE>
		m_pReconstructedDiagnostic;
	CEffectReconstructedRuntimeBoundary m_ReconstructedRuntimeBoundary;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> m_ModelCueResources;
	shared_ptr<Engine::CShader> m_pMeshShader;
	shared_ptr<Engine::CShader> m_pAnimatedModelShader;
	shared_ptr<Engine::CShader> m_pRectShader;
	shared_ptr<Engine::CShader> m_pParticleShader;
	shared_ptr<Engine::CShader> m_pExactSpriteBridgeShader;
	shared_ptr<Engine::CShader> m_pExactLocalMeshBridgeShader;
	shared_ptr<Engine::CShader> m_pTrailShader;
	shared_ptr<Engine::CShader> m_pDecalShader;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CVIBuffer_ParticleRect> m_pParticleBuffer;
	shared_ptr<Engine::CVIBuffer_DynamicTrail> m_pTrailBuffer;
	/* Frame evaluation keeps its own vectors alive, but the renderer used to
	   allocate three additional transient vectors for every active particle or
	   trail occurrence.  One renderer instance is consumed serially, so retain
	   these capacities across frames without changing draw order or payloads. */
	std::vector<Engine::VTXEFFECT_PARTICLE> m_ParticleInstanceScratch;
	std::vector<EFFECT_EVALUATED_TRAIL_POINT> m_TrailPointScratch;
	std::vector<Engine::VTXEFFECT_TRAIL> m_TrailVertexScratch;
	std::vector<uint32_t> m_TrailIndexScratch;
	ComPtr<ID3D11ShaderResourceView> m_pWhiteTexture;
	ComPtr<ID3D11ShaderResourceView> m_pBlackTexture;
	ComPtr<ID3D11BlendState> m_pExactPreviewAdditiveOneOneBlendState;
	shared_ptr<Engine::CShader> m_pGlasshole02TranslatedCanaryShader;
	ComPtr<ID3D11BlendState> m_pGlasshole02TranslatedCanaryBlendState;
	std::unique_ptr<CValtanTranslatedCanaryRuntime>
		m_pValtanTranslatedCanaryRuntime;
	bool_t m_bAuthoringExactPreviewExecutionEnabled = false;
	bool_t m_bAuthoringGlasshole02TranslatedCanaryEnabled =
		GLASSHOLE02_TRANSLATED_CANARY_DEFAULT_ENABLED;
	bool_t m_bAuthoringValtanTranslatedCanaryEnabled =
		VALTAN_TRANSLATED_CANARY_DEFAULT_ENABLED;
	bool_t m_bReconstructedSourceRuntimeActive = false;
	bool_t m_bSourceVisualProgramActive = false;
	EFFECT_PREVIEW_SUBMISSION_ISOLATION m_PreviewSubmissionIsolation;
	EFFECT_GPU_RENDER_SUBMISSION_STATS m_LastRenderSubmissionStats;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	EFFECT_GPU_RENDER_OCCURRENCE_STATS* m_pActiveOccurrenceStats = nullptr;
#endif
	std::string m_strStatus;
	uint64_t m_iStatusEvaluated = (std::numeric_limits<uint64_t>::max)();
	uint64_t m_iStatusActive = (std::numeric_limits<uint64_t>::max)();
	uint64_t m_iStatusSubmitted = (std::numeric_limits<uint64_t>::max)();
	uint64_t m_iStatusSuppressed = (std::numeric_limits<uint64_t>::max)();
	std::string m_strRenderFailureDetail;
	bool_t m_bLastRenderFailureObjectLocal = false;
};

NS_END
