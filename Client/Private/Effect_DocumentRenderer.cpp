#include "Effect_DocumentRenderer.h"

#include "DeferredMaterialRenderUtils.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "Model.h"
#include "Profiler.h"
#include "RuntimeAssetRoot.h"
#include "Render_OutputContract.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

#include <d3d11sdklayers.h>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <map>
#include <mutex>
#include <span>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
	bool_t Is_ZeroFloatBits(const FLOAT fValue)
	{
		return std::bit_cast<uint32_t>(fValue) == 0u;
			}

	bool_t Is_DefaultStencilFace(
		const D3D11_DEPTH_STENCILOP_DESC& Face)
	{
		return Face.StencilFailOp == D3D11_STENCIL_OP_KEEP &&
			Face.StencilDepthFailOp == D3D11_STENCIL_OP_KEEP &&
			Face.StencilPassOp == D3D11_STENCIL_OP_KEEP &&
			Face.StencilFunc == D3D11_COMPARISON_ALWAYS;
	}

	bool_t Is_DefaultUnusedBlendTarget(
		const D3D11_RENDER_TARGET_BLEND_DESC& Target)
	{
		return !Target.BlendEnable && Target.SrcBlend == D3D11_BLEND_ONE &&
			Target.DestBlend == D3D11_BLEND_ZERO &&
			Target.BlendOp == D3D11_BLEND_OP_ADD &&
			Target.SrcBlendAlpha == D3D11_BLEND_ONE &&
			Target.DestBlendAlpha == D3D11_BLEND_ZERO &&
			Target.BlendOpAlpha == D3D11_BLEND_OP_ADD &&
			Target.RenderTargetWriteMask == D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	bool_t Is_CompiledMaterialAdapter(
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter)
	{
		using ADAPTER_ID =
			Client::EFFECT_COMPILED_MATERIAL_ADAPTER_ID;
		using CARRIER = Client::EFFECT_COMPILED_MATERIAL_CARRIER;
		using PROFILE = Client::EFFECT_RENDER_PROFILE;
		const auto MatchesIdentity = [&Adapter](
			const CARRIER eCarrier,
			const std::string_view strAdapterId,
			const std::string_view strShaderId,
			const std::string_view strVertexLayoutId,
			const PROFILE eProfile)
		{
			return Adapter.eCarrier == eCarrier &&
				Adapter.strAdapterId == strAdapterId &&
				Adapter.strShaderId == strShaderId &&
				Adapter.strVertexLayoutId == strVertexLayoutId &&
				Adapter.eRenderProfile == eProfile;
		};
		bool_t bIdentityValid = false;
		switch (Adapter.eAdapterId)
		{
		case ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ALPHA_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ALPHA_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::LOCAL_DECAL_PROJECTOR,
				Client::EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectDecal.hlsl", "VTXTEX",
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::LOCAL_DECAL_PROJECTOR,
				Client::EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectDecal.hlsl", "VTXTEX",
				PROFILE::ALPHA_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_SPRITE_PARTICLE_ALPHA_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_PROJECT_TUNED_SPRITE_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ALPHA_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_SPRITE_PARTICLE_ADDITIVE_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_PROJECT_TUNED_SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_SPRITE_PARTICLE_ALPHA_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_PROJECT_TUNED_SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_SPRITE_PARTICLE_ADDITIVE_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::SPRITE_PARTICLE,
				Client::EFFECT_PROJECT_TUNED_SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_MESH_PARTICLE_ALPHA_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_PROJECT_TUNED_MESH_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ALPHA_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_MESH_PARTICLE_ADDITIVE_TWO_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_PROJECT_TUNED_MESH_ADDITIVE_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_MESH_PARTICLE_ALPHA_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_PROJECT_TUNED_MESH_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ALPHA_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::PROJECT_TUNED_MESH_PARTICLE_ADDITIVE_ONE_SIDED_V1:
			bIdentityValid = MatchesIdentity(CARRIER::MESH_PARTICLE_CMODEL,
				Client::EFFECT_PROJECT_TUNED_MESH_ADDITIVE_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ);
			break;
		case ADAPTER_ID::END:
		default:
			return false;
		}
		if (!bIdentityValid || Adapter.strMrtId != "MRT_SceneHDR" ||
			Adapter.iSceneColorRenderTargetIndex != 0u ||
			Adapter.strSceneColorSemantic != "SV_TARGET0" ||
			Adapter.iDistortionRenderTargetIndex != 1u ||
			Adapter.strDistortionSemantic != "SV_TARGET1" ||
			!Adapter.bDistortionDeterministicZero ||
			Adapter.iStencilReference != 0u)
		{
			return false;
		}
		switch (Adapter.eRenderProfile)
		{
		case PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
			return Adapter.iPassIndex == 1u &&
				Adapter.strRasterizerState == "RS_Cull_None" &&
				Adapter.strDepthStencilState ==
					(Adapter.eCarrier == CARRIER::LOCAL_DECAL_PROJECTOR ?
						"DSS_ZNone" : "DSS_ReadOnly") &&
				Adapter.strBlendState == "BS_EffectAlpha";
		case PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
			return Adapter.iPassIndex == 3u &&
				Adapter.strRasterizerState == "RS_Default" &&
				Adapter.strDepthStencilState == "DSS_ReadOnly" &&
				Adapter.strBlendState == "BS_EffectAlpha";
		case PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
			return Adapter.iPassIndex == 2u &&
				Adapter.strRasterizerState == "RS_Cull_None" &&
				Adapter.strDepthStencilState == "DSS_ReadOnly" &&
				Adapter.strBlendState == "BS_EffectAdditive";
		case PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
			return Adapter.iPassIndex == 4u &&
				Adapter.strRasterizerState == "RS_Default" &&
				Adapter.strDepthStencilState == "DSS_ReadOnly" &&
				Adapter.strBlendState == "BS_EffectAdditive";
		case PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		case PROFILE::END:
		default:
			return false;
		}
	}

	enum class COMPILED_ADAPTER_ACTUAL_BLEND : uint8_t
	{
		ALPHA_BLEND,
		ADDITIVE_BLEND,
	};

	bool_t Resolve_ActualMaterialAdapterPipelineReceipt(
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex,
		D3D11_CULL_MODE& eOutCullMode,
		bool_t& bOutDepthWrite,
		COMPILED_ADAPTER_ACTUAL_BLEND& eOutBlend)
	{
		using CARRIER = Client::EFFECT_COMPILED_MATERIAL_CARRIER;
		using PROFILE = Client::EFFECT_RENDER_PROFILE;
		const bool_t bActualPassAllowed = [&]()
		{
			switch (Adapter.eCarrier)
			{
			case CARRIER::SPRITE_PARTICLE:
				return iActualPassIndex == Adapter.iPassIndex &&
					Adapter.eRenderProfile != PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
					Adapter.eRenderProfile != PROFILE::END;
			case CARRIER::MESH_PARTICLE_CMODEL:
				return (Adapter.eRenderProfile !=
						PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
					Adapter.eRenderProfile != PROFILE::END) &&
					(iActualPassIndex == Adapter.iPassIndex ||
					 (Adapter.iPassIndex == 3u && iActualPassIndex == 5u) ||
					 (Adapter.iPassIndex == 4u && iActualPassIndex == 6u));
			case CARRIER::LOCAL_DECAL_PROJECTOR:
				return (Adapter.eRenderProfile ==
						PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ||
					Adapter.eRenderProfile ==
						PROFILE::ALPHA_ONE_SIDED_DEPTH_READ) &&
					iActualPassIndex == Adapter.iPassIndex;
			case CARRIER::END:
			default:
				return false;
			}
		}();
		if (!bActualPassAllowed)
			return false;
		switch (iActualPassIndex)
		{
		case 1u:
			eOutCullMode = D3D11_CULL_NONE;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ALPHA_BLEND;
			return true;
		case 2u:
			eOutCullMode = D3D11_CULL_NONE;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ADDITIVE_BLEND;
			return true;
		case 3u:
			eOutCullMode = D3D11_CULL_BACK;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ALPHA_BLEND;
			return true;
		case 4u:
			eOutCullMode = D3D11_CULL_BACK;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ADDITIVE_BLEND;
			return true;
		case 5u:
			eOutCullMode = D3D11_CULL_FRONT;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ALPHA_BLEND;
			return true;
		case 6u:
			eOutCullMode = D3D11_CULL_FRONT;
			bOutDepthWrite = false;
			eOutBlend = COMPILED_ADAPTER_ACTUAL_BLEND::ADDITIVE_BLEND;
			return true;
		default:
			return false;
		}
	}

	bool_t Validate_ActualMaterialAdapterFixedFunctionState(
		ID3D11DeviceContext* pContext,
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex)
	{
		D3D11_CULL_MODE eExpectedCull = D3D11_CULL_NONE;
		bool_t bDepthWrite = false;
		COMPILED_ADAPTER_ACTUAL_BLEND eExpectedBlend =
			COMPILED_ADAPTER_ACTUAL_BLEND::ALPHA_BLEND;
		if (nullptr == pContext || !Is_CompiledMaterialAdapter(Adapter) ||
			!Resolve_ActualMaterialAdapterPipelineReceipt(
				Adapter, iActualPassIndex, eExpectedCull, bDepthWrite,
				eExpectedBlend))
		{
			return false;
		}

		ID3D11RasterizerState* pRasterizerRaw = nullptr;
		ID3D11DepthStencilState* pDepthStencilRaw = nullptr;
		ID3D11BlendState* pBlendRaw = nullptr;
		UINT iStencilReference = UINT32_MAX;
		std::array<FLOAT, 4u> BlendFactor{};
		UINT iSampleMask = 0u;
		pContext->RSGetState(&pRasterizerRaw);
		pContext->OMGetDepthStencilState(
			&pDepthStencilRaw, &iStencilReference);
		pContext->OMGetBlendState(
			&pBlendRaw, BlendFactor.data(), &iSampleMask);
		ComPtr<ID3D11RasterizerState> pRasterizer;
		ComPtr<ID3D11DepthStencilState> pDepthStencil;
		ComPtr<ID3D11BlendState> pBlend;
		pRasterizer.Attach(pRasterizerRaw);
		pDepthStencil.Attach(pDepthStencilRaw);
		pBlend.Attach(pBlendRaw);
		if (nullptr == pRasterizer || nullptr == pDepthStencil ||
			nullptr == pBlend || iStencilReference != Adapter.iStencilReference ||
			iSampleMask != 0xffffffffu ||
			!std::all_of(BlendFactor.begin(), BlendFactor.end(),
				Is_ZeroFloatBits))
		{
			return false;
		}

		D3D11_RASTERIZER_DESC Rasterizer{};
		D3D11_DEPTH_STENCIL_DESC DepthStencil{};
		D3D11_BLEND_DESC Blend{};
		pRasterizer->GetDesc(&Rasterizer);
		pDepthStencil->GetDesc(&DepthStencil);
		pBlend->GetDesc(&Blend);
		const D3D11_RENDER_TARGET_BLEND_DESC& SceneColor = Blend.RenderTarget[0u];
		const D3D11_RENDER_TARGET_BLEND_DESC& Distortion = Blend.RenderTarget[1u];
		const bool_t bSceneColorAlpha =
			eExpectedBlend == COMPILED_ADAPTER_ACTUAL_BLEND::ALPHA_BLEND;
		const bool_t bSceneColorAdditive =
			eExpectedBlend == COMPILED_ADAPTER_ACTUAL_BLEND::ADDITIVE_BLEND;
		const bool_t bDepthEnabled =
			Adapter.strDepthStencilState != "DSS_ZNone";
		// Effects11 normalizes the operationally ignored write mask to ALL for
		// DSS_ZNone.  Keep the enabled-depth receipt strict while matching the
		// actual disabled-depth state returned by D3D11/WARP.
		const D3D11_DEPTH_WRITE_MASK eExpectedDepthWriteMask =
			bDepthEnabled ?
				(bDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL :
					D3D11_DEPTH_WRITE_MASK_ZERO) :
				D3D11_DEPTH_WRITE_MASK_ALL;
		const bool_t bSceneColorAlphaValid =
			SceneColor.BlendEnable &&
			 SceneColor.SrcBlend == D3D11_BLEND_SRC_ALPHA &&
			 SceneColor.DestBlend == D3D11_BLEND_INV_SRC_ALPHA &&
			 SceneColor.BlendOp == D3D11_BLEND_OP_ADD &&
			 SceneColor.SrcBlendAlpha == D3D11_BLEND_ONE &&
			 SceneColor.DestBlendAlpha == D3D11_BLEND_INV_SRC_ALPHA &&
			 SceneColor.BlendOpAlpha == D3D11_BLEND_OP_ADD &&
			 SceneColor.RenderTargetWriteMask ==
				D3D11_COLOR_WRITE_ENABLE_ALL;
		const bool_t bSceneColorAdditiveValid =
			SceneColor.BlendEnable &&
			 SceneColor.SrcBlend == D3D11_BLEND_SRC_ALPHA &&
			 SceneColor.DestBlend == D3D11_BLEND_ONE &&
			 SceneColor.BlendOp == D3D11_BLEND_OP_ADD &&
			 SceneColor.SrcBlendAlpha == D3D11_BLEND_ONE &&
			 SceneColor.DestBlendAlpha == D3D11_BLEND_ONE &&
			 SceneColor.BlendOpAlpha == D3D11_BLEND_OP_ADD &&
			 SceneColor.RenderTargetWriteMask ==
				D3D11_COLOR_WRITE_ENABLE_ALL;
		return ((bSceneColorAlpha && bSceneColorAlphaValid) ||
			(bSceneColorAdditive && bSceneColorAdditiveValid)) &&
			Rasterizer.FillMode == D3D11_FILL_SOLID &&
			Rasterizer.CullMode == eExpectedCull &&
			!Rasterizer.FrontCounterClockwise && Rasterizer.DepthBias == 0 &&
			Is_ZeroFloatBits(Rasterizer.DepthBiasClamp) &&
			Is_ZeroFloatBits(Rasterizer.SlopeScaledDepthBias) &&
			Rasterizer.DepthClipEnable && !Rasterizer.ScissorEnable &&
			!Rasterizer.MultisampleEnable &&
			!Rasterizer.AntialiasedLineEnable &&
			DepthStencil.DepthEnable == bDepthEnabled &&
			DepthStencil.DepthWriteMask == eExpectedDepthWriteMask &&
			(!bDepthEnabled ||
			 DepthStencil.DepthFunc == D3D11_COMPARISON_LESS_EQUAL) &&
			!DepthStencil.StencilEnable &&
			DepthStencil.StencilReadMask == D3D11_DEFAULT_STENCIL_READ_MASK &&
			DepthStencil.StencilWriteMask == D3D11_DEFAULT_STENCIL_WRITE_MASK &&
			Is_DefaultStencilFace(DepthStencil.FrontFace) &&
			Is_DefaultStencilFace(DepthStencil.BackFace) &&
			!Blend.AlphaToCoverageEnable && Blend.IndependentBlendEnable &&
			Distortion.BlendEnable && Distortion.SrcBlend == D3D11_BLEND_ONE &&
			Distortion.DestBlend == D3D11_BLEND_ONE &&
			Distortion.BlendOp == D3D11_BLEND_OP_ADD &&
			Distortion.SrcBlendAlpha == D3D11_BLEND_ONE &&
			Distortion.DestBlendAlpha == D3D11_BLEND_ONE &&
			Distortion.BlendOpAlpha == D3D11_BLEND_OP_ADD &&
			Distortion.RenderTargetWriteMask ==
				(D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN) &&
			std::all_of(Blend.RenderTarget + 2u, Blend.RenderTarget + 8u,
				Is_DefaultUnusedBlendTarget);
	}

	bool_t Validate_ActualMaterialAdapterPipeline(
		ID3D11DeviceContext* pContext,
		const Client::EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const uint32_t iActualPassIndex)
	{
		return nullptr != pContext &&
			Engine::CRenderOutputContract::Get_Active() ==
				Engine::RENDER_OUTPUT_CONTRACT::
					SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION &&
			Engine::CRenderOutputContract::Matches_ActiveRenderTargets(pContext) &&
			Validate_ActualMaterialAdapterFixedFunctionState(
				pContext, Adapter, iActualPassIndex);
	}

	bool_t Validate_ActualLocalDecalSceneShaderResources(
		ID3D11DeviceContext* pContext)
	{
		/* The compiled six-lane LocalDecal pass consumes Common base/noise/
		   mask/emissive/dissolve/base2/mask2/noise2 plus SourceTexture0..5.
		   SourceTexture6 is unreachable for this opcode and is removed by FXC,
		   so the named Target_Depth/Target_Normal receipts are PS t14/t15
		   after Begin(3). */
		constexpr uint32_t LOCAL_DECAL_DEPTH_TEXTURE_SLOT = 14u;
		constexpr uint32_t LOCAL_DECAL_NORMAL_TEXTURE_SLOT = 15u;
		static_assert(LOCAL_DECAL_DEPTH_TEXTURE_SLOT <
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		static_assert(LOCAL_DECAL_NORMAL_TEXTURE_SLOT <
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		if (nullptr == pContext)
			return false;

		const ComPtr<ID3D11ShaderResourceView> pExpectedDepth =
			CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		const ComPtr<ID3D11ShaderResourceView> pExpectedNormal =
			CGameInstance::Get().Get_RT_SRV(TEXT("Target_Normal"));
		if (nullptr == pExpectedDepth || nullptr == pExpectedNormal)
			return false;

		ID3D11ShaderResourceView* pActualDepthRaw = nullptr;
		ID3D11ShaderResourceView* pActualNormalRaw = nullptr;
		pContext->PSGetShaderResources(
			LOCAL_DECAL_DEPTH_TEXTURE_SLOT, 1u, &pActualDepthRaw);
		pContext->PSGetShaderResources(
			LOCAL_DECAL_NORMAL_TEXTURE_SLOT, 1u, &pActualNormalRaw);
		ComPtr<ID3D11ShaderResourceView> pActualDepth;
		ComPtr<ID3D11ShaderResourceView> pActualNormal;
		pActualDepth.Attach(pActualDepthRaw);
		pActualNormal.Attach(pActualNormalRaw);
		return pActualDepth.Get() == pExpectedDepth.Get() &&
			pActualNormal.Get() == pExpectedNormal.Get();
	}

	struct ARTIST31470_EMITTER_ROW_PIN final
	{
		uint32_t iOrder;
		std::string_view strEmitterRowSha256;
	};

	constexpr std::array<ARTIST31470_EMITTER_ROW_PIN, 35u>
		ARTIST31470_EMITTER_ROW_PINS = {{
		{ 0u, "5b7356ded7c7acd6958a0f6142e7323e15826799d463985faac4736e30810190" },
		{ 1u, "e9723f41e11a388cc25c49d0354dfb23c42988ce4150991403e6a40e106cc1ab" },
		{ 2u, "8879fb2dd9d2e5e03b0e5308f705d5a63ba7e8b4d4d28f0d2c8714b552bd300c" },
		{ 3u, "3cf36d8cbb5d065246f285198974019bd79b9c08543fa7a4ab37f171d149e8fd" },
		{ 4u, "95df71aea59355b1ee5e33ed7c835419f7fb81d2a7a93100e26fd6879dad3c7b" },
		{ 5u, "ac42ab12d87d656a686ae54ca92e3eaff8f071cf403b5c7f63622ea94fbfc218" },
		{ 6u, "aad0501641477b7de98b44995741a037e3523b8d82d5d4693734e6268d1216d1" },
		{ 7u, "91ca71963c4dd676a46b6ce94a1402f630f8a91aadf216b66fa1be9fb90b105b" },
		{ 8u, "29a2bfe52b5f18982e8c6d7dd986c96673e6f832a0b41de84140b6a6b76b9405" },
		{ 9u, "8816671187dedf2f13752c188a8e694780ed2960c57cfc72bea5092283a7e488" },
		{ 10u, "60b7fd2bbc97289fe49bf9232061b89ecdfa56116f5deeae2d605bb86c5f8a9c" },
		{ 11u, "99416894217b2799e4fb05d6e2e7e0f2fd9e0aea4ea1d67952792d237d4bde4b" },
		{ 12u, "aaf9843fb105d48afd1b714ca15582385ea1989f7d8764714eadfedb0eaa4cd5" },
		{ 13u, "28df4bc241309a88baa970cde323fea0026ca0dbaeedebccbbca4241bbc5f065" },
		{ 14u, "f941c08118629b9627b9f51094d6101b772544359c7e4d2ed3d18d9391cb7d52" },
		{ 15u, "90e3bd852a65684faef50bed8955dcfbbf32014ecf7e90b78e00d2266ecedf4c" },
		{ 16u, "48df03db9a3c64fe48fc6833336c70d50dd8ed4db2c1c4908daa9f40b6af16a5" },
		{ 17u, "926cf4eeb702114359031ca06a4c82f668074be0e0a2345337a94ec7a5356953" },
		{ 18u, "890416d755e4ae6bb0f1e3c5ec6e044c3aaa5b6e843edf26fa3547b47557c6b5" },
		{ 19u, "c979185b5c078b5e986f1322434f90a6eeda0406da5db89961aa0519aa7a5e52" },
		{ 20u, "db45e06aa718a8161a62c046f54e60b0ec271f659d6a1241c1bbf3bacf02306a" },
		{ 21u, "5fa10f3eb73cf2d5e0064f9b1c590158274f506dd0d7dea016b6b10ed531419d" },
		{ 22u, "f272a1f511f153c22728399e8d3ab20aabea5ef86c8e9bd573ff4ec230379f00" },
		{ 23u, "3025afbdf23d0acd403e991bf6b8bb6b5d44f38bad530c4afef3b291d0c7fd2b" },
		{ 24u, "5b437795eb0654fb51188e620cc6e06771d394da29760aed71619bb918f6ea8d" },
		{ 25u, "e5ae5dae0370f233306afbb8a1af178b3ceb53ba398de50f8d42baa9678610b5" },
		{ 26u, "88c1911a8122c0e5c2b6b7a1d48fe7e91628c1a64b5d0f8124902557a8b4bc22" },
		{ 27u, "e4957941115ed44d62f51de033f084b95c670ae8728c2bd699955d7f806bc3c8" },
		{ 28u, "de05e5a4a9b549190f8368fbee6b8f043ffb7db68bf67040be043d0c3b36b939" },
		{ 29u, "e9a471c39bdde31b984c6911d1580deb26ad731557831d548d1ac1b67b842385" },
		{ 30u, "2d06df4f3fd03da20cc787dca4ce282e8d8ff438a3b941a8cbcac766d83e21f5" },
		{ 31u, "28af70cd32eca7130e179d3dc1e96954016f598b616c743502083e80efbcb21b" },
		{ 32u, "3d4873252aaa8fc8a98f2511c4047b5295199053b6f15dd3906fda554b4aa720" },
		{ 33u, "b3c7e77416e2861bdb05e3a1cede96da4bfb4f7fef47377e339785b2cb2e3c5c" },
		{ 34u, "b3af4b7b34f7f3760fa0b4e4da261fdbed12210d7e93287a433bc510cf7a9de4" },
	}};

	constexpr bool Validate_Artist31470EmitterRowPins() noexcept
	{
		for (size_t i = 0u; i < ARTIST31470_EMITTER_ROW_PINS.size(); ++i)
		{
			const auto& Row = ARTIST31470_EMITTER_ROW_PINS[i];
			if (Row.iOrder != i || Row.strEmitterRowSha256.size() != 64u)
				return false;
			for (const char_t Character : Row.strEmitterRowSha256)
			{
				if (!((Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f')))
					return false;
			}
			for (size_t j = 0u; j < i; ++j)
			{
				if (Row.strEmitterRowSha256 ==
					ARTIST31470_EMITTER_ROW_PINS[j].strEmitterRowSha256)
					return false;
			}
		}
		return true;
	}

	static_assert(Validate_Artist31470EmitterRowPins());

	class PIXEL_SHADER_SAMPLER_SCOPE final
	{
	public:
		explicit PIXEL_SHADER_SAMPLER_SCOPE(ID3D11DeviceContext* pContext)
			: m_pContext(pContext)
		{
		}

		~PIXEL_SHADER_SAMPLER_SCOPE()
		{
			Restore();
		}

		bool_t Apply(const std::span<const ComPtr<ID3D11SamplerState>> Samplers)
		{
			m_bLastFailureContractInvalid = false;
			if (nullptr == m_pContext)
				return false;
			if (m_bApplied || Samplers.empty() ||
				Samplers.size() > m_Previous.size() ||
				std::any_of(Samplers.begin(), Samplers.end(),
					[](const auto& Sampler) { return nullptr == Sampler; }))
			{
				m_bLastFailureContractInvalid = true;
				return false;
			}
			m_iCount = static_cast<uint32_t>(Samplers.size());
			std::array<ID3D11SamplerState*, 6u> PreviousRaw{};
			m_pContext->PSGetSamplers(5u, m_iCount, PreviousRaw.data());
			for (size_t i = 0u; i < m_iCount; ++i)
				m_Previous[i].Attach(PreviousRaw[i]);

			std::array<ID3D11SamplerState*, 6u> Desired{};
			for (size_t i = 0u; i < m_iCount; ++i)
				Desired[i] = Samplers[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, Desired.data());

			bool_t bMatches = true;
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			std::array<ID3D11SamplerState*, 6u> Applied{};
			m_pContext->PSGetSamplers(5u, m_iCount, Applied.data());
			for (size_t i = 0u; i < m_iCount; ++i)
			{
				bMatches = bMatches && Applied[i] == Desired[i];
				if (nullptr != Applied[i])
					Applied[i]->Release();
			}
#endif
			m_bApplied = true;
			if (!bMatches)
				Restore();
			return bMatches;
		}

		bool_t Was_LastFailureContractInvalid() const
		{
			return m_bLastFailureContractInvalid;
		}

	private:
		void Restore()
		{
			if (!m_bApplied || nullptr == m_pContext)
				return;
			std::array<ID3D11SamplerState*, 6u> PreviousRaw{};
			for (size_t i = 0u; i < m_iCount; ++i)
				PreviousRaw[i] = m_Previous[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, PreviousRaw.data());
			m_bApplied = false;
		}

		ID3D11DeviceContext* m_pContext = nullptr;
		std::array<ComPtr<ID3D11SamplerState>, 6u> m_Previous{};
		uint32_t m_iCount = 0u;
		bool_t m_bApplied = false;
		bool_t m_bLastFailureContractInvalid = false;
	};

	Client::EFFECT_GPU_RENDER_FAMILY Resolve_GpuRenderFamily(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		switch (Element.Renderer.eType)
		{
		case Client::EFFECT_RENDERER_TYPE::STANDALONE_MESH:
		case Client::EFFECT_RENDERER_TYPE::MESH_PARTICLE:
			return Client::EFFECT_GPU_RENDER_FAMILY::MESH;
		case Client::EFFECT_RENDERER_TYPE::LEGACY_STANDALONE_SPRITE:
		case Client::EFFECT_RENDERER_TYPE::SPRITE_PARTICLE:
			return Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
		case Client::EFFECT_RENDERER_TYPE::DECAL_PARTICLE:
			return Client::EFFECT_GPU_RENDER_FAMILY::DECAL;
		case Client::EFFECT_RENDERER_TYPE::ANIM_TRAIL:
		case Client::EFFECT_RENDERER_TYPE::CASCADE_RIBBON:
			return Client::EFFECT_GPU_RENDER_FAMILY::RIBBON;
		case Client::EFFECT_RENDERER_TYPE::LIGHT_PARTICLE:
		case Client::EFFECT_RENDERER_TYPE::SCREEN_POST:
			return Client::EFFECT_GPU_RENDER_FAMILY::END;
		case Client::EFFECT_RENDERER_TYPE::END:
			/* Authoring v3-v13 and compiled legacy assemblies predate the
			   native-v14 renderer descriptor, including valid source-recipe
			   documents.  Derive only the legacy family needed by the occurrence
			   denominator.  Native-v14 and reconstructed Artist documents are
			   validated separately and carry a non-END renderer here. */
			switch (Element.eKind)
			{
			case Client::EFFECT_ELEMENT_KIND::MESH:
				return Client::EFFECT_GPU_RENDER_FAMILY::MESH;
			case Client::EFFECT_ELEMENT_KIND::SPRITE:
				return Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
			case Client::EFFECT_ELEMENT_KIND::PARTICLE:
				return std::ranges::any_of(Element.ResourceBindings,
					[](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
					{
						return Binding.strSlotId ==
							Client::EFFECT_MESH_SHAPE_SLOT_ID;
					}) ? Client::EFFECT_GPU_RENDER_FAMILY::MESH :
					Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
			case Client::EFFECT_ELEMENT_KIND::DECAL:
				return Client::EFFECT_GPU_RENDER_FAMILY::DECAL;
			case Client::EFFECT_ELEMENT_KIND::TRAIL:
				return Client::EFFECT_GPU_RENDER_FAMILY::RIBBON;
			case Client::EFFECT_ELEMENT_KIND::LIGHT:
			case Client::EFFECT_ELEMENT_KIND::SCREEN_POST:
			case Client::EFFECT_ELEMENT_KIND::END:
			default:
				return Client::EFFECT_GPU_RENDER_FAMILY::END;
			}
		default:
			return Client::EFFECT_GPU_RENDER_FAMILY::END;
		}
	}

	bool_t Is_DimensionSummonCharacterSurfaceCue(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::EFFECT_MODEL_CUE_DESC& Cue)
	{
		return Document.strEffectAssetId ==
				"effect.dimensionmaster.skill.2050500.unified" &&
			Cue.strCueId == "dimension_summon" &&
			Cue.strModelAssetId ==
				"Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel" &&
			Cue.strClipName == "sk_swp_dms_00_sk_sk_dimensionprison" &&
			Cue.eAlphaMode ==
				Client::EFFECT_MODEL_CUE_ALPHA_MODE::MASKED_SURFACE;
	}

	template <size_t Size>
	int32_t NamedSourceTextureIndex(
		const std::array<std::string_view, Size>& Names,
		const std::string_view strName)
	{
		const auto Iterator = std::find(Names.begin(), Names.end(), strName);
		return Iterator == Names.end() ? -1 :
			static_cast<int32_t>(std::distance(Names.begin(), Iterator));
	}

	int32_t LinearFlowSourceTextureIndex(const std::string_view strName)
	{
		return NamedSourceTextureIndex(
			Client::EFFECT_LINEARFLOW_SOURCE_TEXTURE_NAMES, strName);
	}

	int32_t BlacklineSourceTextureIndex(const std::string_view strName)
	{
		return NamedSourceTextureIndex(
			Client::EFFECT_BLACKLINE_SOURCE_TEXTURE_NAMES, strName);
	}

	int32_t LocalCrackSourceTextureIndex(const std::string_view strName)
	{
		return NamedSourceTextureIndex(
			Client::EFFECT_LOCAL_CRACK_SOURCE_TEXTURE_NAMES, strName);
	}

	f32_t SourceScalar(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const f32_t fFallback)
	{
		const auto Iterator = std::find_if(
			Source.Scalars.begin(), Source.Scalars.end(),
			[strName](const Client::EFFECT_NAMED_FLOAT_DESC& Row)
			{
				return Row.strName == strName;
			});
		return Iterator == Source.Scalars.end() ? fFallback : Iterator->fValue;
	}

	f32_t SourceScalarAny(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::initializer_list<std::string_view> Names,
		const f32_t fFallback)
	{
		for (const std::string_view strName : Names)
		{
			const auto Iterator = std::find_if(
				Source.Scalars.begin(), Source.Scalars.end(),
				[strName](const Client::EFFECT_NAMED_FLOAT_DESC& Row)
				{
					return Row.strName == strName;
				});
			if (Iterator != Source.Scalars.end())
				return Iterator->fValue;
		}
		return fFallback;
	}

	bool_t SourceStaticSwitch(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const bool_t bFallback)
	{
		const auto Iterator = std::find_if(
			Source.StaticSwitches.begin(), Source.StaticSwitches.end(),
			[strName](const Client::EFFECT_NAMED_BOOL_DESC& Row)
			{
				return Row.strName == strName;
			});
		return Iterator == Source.StaticSwitches.end() ?
			bFallback : Iterator->bValue;
	}

	float4_t SourceVector(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const float4_t& vFallback)
	{
		const auto Iterator = std::find_if(
			Source.Vectors.begin(), Source.Vectors.end(),
			[strName](const Client::EFFECT_NAMED_FLOAT4_DESC& Row)
			{
				return Row.strName == strName;
			});
		return Iterator == Source.Vectors.end() ? vFallback : Iterator->vValue;
	}

	void Build_LinearFlowConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 16u>& Parameters,
		float4_t& vMaskAColor,
		float4_t& vMaskBColor,
		float4_t& vAuxiliary0,
		float4_t& vAuxiliary1)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("diff_tile_u", 1.f), S("diff_tile_v", 1.f),
			S("diff_panx_speed", 0.f), S("diff_pany_speed", 0.f) };
		Parameters[1] = { S("diff_offset_x", 0.f), S("diff_offset_y", 0.f),
			S("diff_rotator", 0.f), S("diff_noise_str", 0.f) };
		Parameters[2] = { S("diff_noise_tile_u", 1.f),
			S("diff_noise_tile_v", 1.f), 0.f, 0.f };
		Parameters[3] = { S("a_tile_u", 1.f), S("a_tile_v", 1.f),
			S("a_panx_speed", 0.f), S("a_pany_speed", 0.f) };
		Parameters[4] = { S("a_offset_x", 0.f), S("a_offset_y", 0.f),
			S("a_rotator", 0.f), S("a_sizecontrol", 1.f) };
		Parameters[5] = { S("a_noise_01_tile_u", 1.f),
			S("a_noise_01_tile_v", 1.f), S("a_noise_01_pan_x", 0.f),
			S("a_noise_01_pan_y", 0.f) };
		Parameters[6] = { S("a_noise_01_offset_x", 0.f),
			S("a_noise_01_offset_y", 0.f), S("a_noise_01_str", 0.f), 0.f };
		Parameters[7] = { S("b_tile_u", 1.f), S("b_tile_v", 1.f),
			S("b_panx_speed", 0.f), S("b_pany_speed", 0.f) };
		Parameters[8] = { S("b_offset_x", 0.f), S("b_offset_y", 0.f),
			S("b_rotator", 0.f), S("b_sizecontrol", 1.f) };
		Parameters[9] = { S("b_noise_01_tile_u", 1.f),
			S("b_noise_01_tile_v", 1.f), S("b_noise_01_pan_x", 0.f),
			S("b_noise_01_pan_y", 0.f) };
		Parameters[10] = { S("b_noise_01_offset_x", 0.f),
			S("b_noise_01_offset_y", 0.f), S("b_noise_01_str", 0.f), 0.f };
		Parameters[11] = { S("diff_str", 1.f), S("diff_pow", 1.f),
			S("a_mask_str", 1.f), S("a_mask_pow", 1.f) };
		Parameters[12] = { S("b_mask_str", 1.f), S("b_mask_pow", 1.f),
			S("opacity_str", 1.f), S("opacity_pow", 1.f) };
		Parameters[13] = { S("mask_density", 1.f), S("mask_radius", 1.f),
			S("mask_linearalpha", 0.5f), S("coresoft_str", 1.f) };
		Parameters[14] = { S("dissolve_tile_x", 1.f),
			S("dissolve_tile_y", 1.f), S("dissolve_pan_x", 0.f),
			S("dissolve_pan_y", 0.f) };
		Parameters[15] = { S("a_mask_desaturation", 0.f),
			S("b_mask_desaturation", 0.f), S("diff_tex_desaturation", 0.f),
			S("gra_pow", 1.f) };
		vAuxiliary0 = { S("dissolve_rot", 0.f),
			S("dissolve_hardness", 5.f), S("meshedgefade", 0.f),
			S("distortion", 0.f) };
		vAuxiliary1 = { S("depth", 0.f), 0.f, 0.f, 0.f };
		vMaskAColor = SourceVector(
			Source, "a_mask_color", { 1.f, 1.f, 1.f, 1.f });
		vMaskBColor = SourceVector(
			Source, "b_mask_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_BlacklineConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 16u>& Parameters,
		float4_t& vDiffuseColor,
		float4_t& vMaskColor)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("diff_tile_x", 1.f), S("diff_tile_y", 1.f),
			S("diff_offset_x", 0.f), S("diff_offset_y", 0.f) };
		Parameters[1] = { S("diff_dypan_x", 0.f), S("diff_dypan_y", 0.f),
			S("diff_rot", 0.f), S("diff_flow_str", 0.f) };
		Parameters[2] = { S("diff_str", 1.f), S("diff_pow", 1.f),
			S("desaturation", 0.f), S("depth", 0.f) };
		Parameters[3] = { S("mask_a_tile_x", 1.f),
			S("mask_a_tile_y", 1.f), S("mask_a_offset_x", 0.f),
			S("mask_a_offset_y", 0.f) };
		Parameters[4] = { S("maska_dypan_x", 0.f),
			S("maska_dypan_y", 0.f), S("mask_a_rot", 0.f),
			S("maska_flow_strength", 0.f) };
		Parameters[5] = { S("mask_a_str", 1.f), S("mask_a_pow", 1.f),
			S("mask_density", 1.f), S("mask_radius", 1.f) };
		Parameters[6] = { S("mask_b_tile_x", 1.f),
			S("mask_b_tile_y", 1.f), S("mask_b_offset_x", 0.f),
			S("mask_b_offset_y", 0.f) };
		Parameters[7] = { S("mask_b_pan_x", 0.f), S("mask_b_pan_y", 0.f),
			S("maskb_dypan_x", 0.f), S("maskb_dypan_y", 0.f) };
		Parameters[8] = { S("mask_b_rot", 0.f),
			S("maskb_flow_strength", 0.f), S("mask_b_str", 1.f),
			S("mask_b_pow", 1.f) };
		Parameters[9] = { S("flow01_tile_x", 1.f),
			S("flow01_tile_y", 1.f), S("flow01_pan_x", 0.f),
			S("flow01_pan_y", 0.f) };
		Parameters[10] = { S("flow01_str", 0.f), S("flow02_tile_x", 1.f),
			S("flow02_tile_y", 1.f), S("flow02_pan_x", 0.f) };
		Parameters[11] = { S("flow02_pan_y", 0.f), S("flow02_str", 0.f),
			S("dissolve_tile_x", 1.f), S("dissolve_tile_y", 1.f) };
		Parameters[12] = { S("dissolve_pan_x", 0.f),
			S("dissolve_pan_y", 0.f), S("dissolve_hardness", 5.f), 0.f };
		Parameters[13] = { S("emissive_str", 1.f), S("emissive_pow", 1.f),
			S("spheremask_str_min", 0.f), S("spheremask_str_max", 1.f) };
		Parameters[14] = { S("01.uv_xscale", 0.f),
			S("01.uv_yscale", 0.f), S("02.uv_xscale", 0.f),
			S("02.uv_yscale", 0.f) };
		vDiffuseColor = SourceVector(
			Source, "diff_color", { 1.f, 1.f, 1.f, 1.f });
		vMaskColor = SourceVector(
			Source, "mask_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_LocalCrackConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 5u>& Parameters,
		float4_t& vOutColor,
		float4_t& vInColor,
		float4_t& vReflectionColor)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("dissolve_tile_x", 1.f),
			S("dissolve_tile_y", 1.f), S("dissolve_pan_x", 0.f),
			S("dissolve_pan_y", 0.f) };
		Parameters[1] = { S("dissolve_hardness", 5.f),
			S("dissolve_tension", 0.f), S("normal_tileu", 1.f),
			S("normal_tilev", 1.f) };
		Parameters[2] = { S("distortion", 0.f), S("fresnel_pow", 1.f),
			S("depth", 0.f), S("refle_pow", 1.f) };
		Parameters[3] = { S("refle_desaturation", 0.f),
			S("refle_vector_divide", 1.f), S("refle_panspeed", 0.f),
			S("refle_offsetx", 0.5f) };
		Parameters[4] = { S("refle_offsety", 0.5f),
			S("refle_tileu", 1.f), S("refle_tilev", 1.f), 0.f };
		vOutColor = SourceVector(
			Source, "out_color", { 0.1f, 0.1f, 0.1f, 1.f });
		vInColor = SourceVector(
			Source, "in_color", { 1.f, 1.f, 1.f, 1.f });
		vReflectionColor = SourceVector(
			Source, "refle_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_SliceConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		float4_t& vScalars0,
		float4_t& vScalars1,
		float4_t& vAuxiliary)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		vScalars0 = { S("slice_rot", 0.f), S("opacity_radius", 2.f),
			S("flow_str", 0.f), S("distortion", 0.f) };
		vScalars1 = { S("slice_flow_tileu", 1.f),
			S("slice_flow_tilev", 1.f), S("slice_flow_offsetx", 0.f),
			S("slice_flow_offsety", 0.f) };
		vAuxiliary = { S("slice_flow_rot", 0.f), 0.f, 0.f, 0.f };
	}

	void Build_MissileTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](
			const std::initializer_list<std::string_view> Names,
			const f32_t fFallback)
		{
			return SourceScalarAny(Source, Names, fFallback);
		};
		Parameters[0] = { S({ "alpha_tex_strength" }, 1.f),
			S({ "alpha01_tex_power" }, 1.f),
			S({ "alpha_tex_dynamicpanspeed_x" }, 0.f),
			S({ "alpha_tex_dynamicpanspeed_y" }, 0.f) };
		Parameters[1] = {
			S({ "alpha_tex_texcoord_x", "alpha_tex_r_texcoord" }, 1.f),
			S({ "alpha_tex_texcoord_y", "alpha_tex_g_texcoord" }, 1.f),
			S({ "alpha_tex_positon_x", "alpha_tex_positon_r" }, 0.f),
			S({ "alpha_tex_positon_y", "alpha_tex_positon_g" }, 0.f) };
		Parameters[2] = {
			S({ "uvnoise_tex_01_texcoord_x", "uvnoise_tex_01_r_texcoord" }, 1.f),
			S({ "uvnoise_tex_01_texcoord_y", "uvnoise_tex_01_g_texcoord" }, 1.f),
			S({ "uvnoise_tex_dynamicpanspeed_x" }, 0.f),
			S({ "uvnoise_tex_dynamicpanspeed_y" }, 0.f) };
		Parameters[3] = {
			S({ "maintex_uv_noise_velue", "uv_noise_head_velue" }, 0.f),
			S({ "dissolve_hardness" }, 5.f),
			S({ "dissolve_tex_dynamicpanspeed_x" }, 0.f),
			S({ "dissolve_tex_dynamicpanspeed_y" }, 0.f) };
		Parameters[4] = {
			S({ "emissive_tex_core_x", "emissive_tex_core_r" }, 1.f),
			S({ "emissive_tex_core_y", "emissive_tex_core_g" }, 1.f),
			S({ "emissive_tex01tile_dynamicpanspeed_x" }, 0.f),
			S({ "emissive_tex01tile_dynamicpanspeed_y" }, 0.f) };
		Parameters[5] = {
			S({ "emissive_tex_core_x_02", "emissive_tex_core_r_02" }, 1.f),
			S({ "emissive_tex_core_y_02", "emissive_tex_core_g_02" }, 1.f),
			S({ "emissive_tex02tile_dynamicpanspeed_x" }, 0.f),
			S({ "emissive_tex02tile_dynamicpanspeed_y" }, 0.f) };
		Parameters[6] = {
			S({ "emissive_tex_core_positon_x", "emissive_tex_core_positon_r" }, 0.f),
			S({ "emissive_tex_core_positon_y", "emissive_tex_core_positon_g" }, 0.f),
			S({ "emissive_tex_backvelue" }, 0.f),
			S({ "fresnelalpha_power" }, 1.f) };
		Parameters[7] = { S({ "emissive_tex_strength" }, 1.f),
			S({ "emissive_tex_power" }, 1.f),
			S({ "alpha_disslove_tex_coord_x",
				"alpha_disslove_tex_coord_r" }, 1.f),
			S({ "alpha_disslove_tex_coord_y",
				"alpha_disslove_tex_coord_g" }, 1.f) };
	}

	void Build_WaterTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("maintex_texcoord_u", 1.f),
			S("maintex_texcoord_v", 1.f), S("main_panspeed_u", 0.f),
			S("main_panspeed_v", 0.f) };
		Parameters[1] = { S("maintex_move_u", 0.f),
			S("maintex_move_v", 0.f), S("maintex_pan_u_time", 0.f),
			S("maintex_pan_v_time", 0.f) };
		Parameters[2] = { S("main_tex_power", 1.f),
			S("main_tex_power_multiply", 1.f),
			S("maintex_desaturation", 0.f),
			S("main_tex_background_velue", 0.f) };
		Parameters[3] = { S("uv_noise_texcoord_u", 1.f),
			S("uv_noise_texcoord_v", 1.f), S("uv_noise_panspeed_u", 0.f),
			S("uv_noise_panspeed_v", 0.f) };
		Parameters[4] = { S("uv_noise_velue", 0.f),
			S("alpha_strength", 1.f), S("outalpha_falloff_velue", 1.f),
			S("camera_vector_fresnel_velue", 1.f) };
		Parameters[5] = { S("dissolve_texcoord_u", 1.f),
			S("dissolve_texcoord_v", 1.f), S("dissolve_pan_u_speed", 0.f),
			S("dissolve_pan_v_speed", 0.f) };
		Parameters[6] = { S("disslove_hardness", 1.f),
			SourceStaticSwitch(Source, "use_reflction", false) ? 1.f : 0.f,
			S("reflection_vector_panspeed", 0.f), S("disto_power", 0.f) };
		Parameters[7] = SourceVector(
			Source, "reflection_color", { 0.f, 0.f, 0.f, 0.f });
	}

	void Build_MakeFlowConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("opacity_tile_u", 1.f),
			S("opacity_tile_v", 1.f), S("opacity_rot", 0.f),
			S("opacity_distort_str", 0.f) };
		Parameters[1] = { S("diff1_tile_u", 1.f),
			S("diff1_tile_v", 1.f), S("diff1_pan_u", 0.f),
			S("diff1_pan_v", 0.f) };
		Parameters[2] = { S("diff2_tile_u", 1.f),
			S("diff2_tile_v", 1.f), S("diff2_pan_u", 0.f),
			S("diff2_pan_v", 0.f) };
		Parameters[3] = { S("flow_tile_u", 1.f), S("flow_tile_v", 1.f),
			S("flow_pan_u", 0.f), S("flow_pan_v", 0.f) };
		Parameters[4] = { S("opacity_str", 1.f), S("opacity_pow", 1.f),
			S("flow_bias", 0.f), S("distort_str", 0.f) };
		Parameters[5] = { S("diff_str", 1.f), S("diff_pow", 1.f),
			S("diff_des", 0.f), S("gra_pow", 1.f) };
		Parameters[6] = { S("color_str", 1.f), S("color_pow", 1.f),
			S("color_des", 0.f), S("cameravec_pow", 1.f) };
		Parameters[7] = SourceVector(
			Source, "diff_backcolor", { 0.f, 0.f, 0.f, 0.f });
	}

	void Build_MakeFlow03SpriteConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		Build_MakeFlowConstants(Source, Parameters);
		Parameters[7] = {
			SourceScalar(Source, "mask_tile_u", 1.f),
			SourceScalar(Source, "mask_tile_v", 1.f), 0.f, 0.f };
	}

	void Build_ParticleTrailConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("tex_alpha_01_r_tile", 1.f),
			S("tex_alpha_01_g_tile", 1.f),
			S("tex_alpha_02_r_tile", 1.f),
			S("tex_alpha_02_g_tile", 1.f) };
		Parameters[1] = { S("tex_alpha_move_u", 0.f),
			S("tex_alpha_move_v", 0.f), S("tex_alpha_01_rotator", 0.f),
			S("tex_noise_velue", 0.f) };
		Parameters[2] = { S("tex_alpha_multiply", 1.f),
			S("tex_alpha_power", 1.f), S("tex_alpha_backvelue", 0.f),
			S("disto_velue", 0.f) };
		Parameters[3] = { S("tex_flow_r_tile", 1.f),
			S("tex_flow_g_tile", 1.f), S("center_hole_size", 0.f),
			S("center_hole_power", 1.f) };
		Parameters[4] = { S("center_hole_hardness", 0.f), 0.f, 0.f, 0.f };
	}

	void Build_RingConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("uv_xscale", 1.f), S("uv_yscale", 1.f),
			S("worldtex_u", 1.f), S("worldtex_v", 1.f) };
		Parameters[1] = { S("uvdistort_mapuvscale", 1.f),
			S("uvdistort_timescale", 0.f), S("distortstr", 0.f),
			S("3_distort_str", 0.f) };
		Parameters[2] = { S("0_map_emit_str", 1.f),
			S("0_map_emit_power", 1.f), S("emissive_str", 1.f),
			S("power", 1.f) };
		Parameters[3] = { S("centerhole_power", 1.f), S("mask_str", 1.f),
			S("fresnel_power", 1.f), S("fresnel_str", 1.f) };
	}

	void Build_ParticleMasterConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vSourceColor)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("21.uvscale.x", 1.f), S("22.uvscale.y", 1.f),
			S("36.str", 1.f), S("37.power", 1.f) };
		Parameters[1] = { S("02.map_a_uvscale_r", 1.f),
			S("03.map_a_uvscale_g", 1.f), S("04.map_a_panning_x", 0.f),
			S("05.map_a_panning_y", 0.f) };
		Parameters[2] = { S("12.map_b_uvscale_r", 1.f),
			S("13.map_b_uvscale_g", 1.f), S("14.map_b_panning_x", 0.f),
			S("15.map_b_panning_y", 0.f) };
		Parameters[3] = { S("07.map_d_uvscale_r", 1.f),
			S("08.map_d_uvscale_g", 1.f), S("09.map_d_panning_x", 0.f),
			S("10.map_d_panning_y", 0.f) };
		Parameters[4] = { S("05.distort_str", 0.f),
			S("92.emissiion_power", 1.f), S("91.desaturation", 0.f),
			S("61.power", 1.f) };
		Parameters[5] = { S("03.map_e_uvscale_r", 1.f),
			S("04.map_e_uvscale_g", 1.f), S("05.map_e_panning_x", 0.f),
			S("06.map_e_panning_y", 0.f) };
		Parameters[6] = { S("13.map_f_uvscale_r", 1.f),
			S("14.map_f_uvscale_g", 1.f), S("15.map_f_panning_x", 0.f),
			S("16.map_f_panning_y", 0.f) };
		Parameters[7] = SourceVector(
			Source, "93.emissiion_color", { 1.f, 1.f, 1.f, 1.f });
		vSourceColor = SourceVector(
			Source, "62.color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_SpriteWaveConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEdgeColor)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		Parameters[0] = { S("maintex_tile_x", 1.f), S("maintex_tile_y", 1.f),
			S("maintex_panspeed_x", 0.f), S("maintex_panspeed_y", 0.f) };
		Parameters[1] = { S("maintex_move_x", 0.f), S("maintex_move_y", 0.f),
			S("maintex_rotator", 0.f), S("maintex_alpha_strength", 1.f) };
		Parameters[2] = { S("uv_noisetex_tile_x", 1.f),
			S("uv_noisetex_tile_y", 1.f), S("uv_noisetex_pan_x", 0.f),
			S("uv_noisetex_pan_y", 0.f) };
		Parameters[3] = { S("uv_noise_velue", 0.f),
			S("uv_noise_02_strength", 0.f), S("dynamic_uvnoise_x", 0.f),
			S("dynamic_uvnoise_y", 0.f) };
		Parameters[4] = { S("disslovetex_01_tile_x", 1.f),
			S("disslovetex_01_tile_y", 1.f),
			S("disslovetex_01_panspeed_x", 0.f),
			S("disslovetex_01_panspeed_y", 0.f) };
		Parameters[5] = { S("dissolve_hardness", 5.f),
			S("noisedissolvetex_strength", 0.f),
			S("spheremask_strength", 1.f),
			S("spheremask_strength_min", 0.f) };
		Parameters[6] = { S("emissivetex02_tile_x", 1.f),
			S("emissivetex02_tile_y", 1.f),
			S("emissivetex02_panspeed_x", 0.f),
			S("emissivetex02_panspeed_y", 0.f) };
		Parameters[7] = { S("emissive_core_strength", 1.f),
			S("emissive_core_power", 1.f), S("emissive_base", 1.f),
			S("spheremask_strength_max", 1.f) };
		vEdgeColor = SourceVector(
			Source, "edge_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_ArtistSpla01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("12.alpha_str", 1.f), S("11.alpha_power", 1.f),
			S("05.distort_str", 0.f), S("05_spacular_str", 1.f) };
		Parameters[1] = { S("07.map_d_uvscale_r", 1.f),
			S("08.map_d_uvscale_g", 1.f), S("09.map_d_panning_x", 0.f),
			S("10.map_d_panning_y", 0.f) };
		Parameters[2] = { S("04_spacular_power", 1.f),
			S("07_spacular_timescale", 0.f), S("06_spacular_uvscale", 1.f),
			S("depthbiasdalpha_bias", 0.f) };
		Parameters[3] = SourceVector(Source, "01.color", { 1.f, 1.f, 1.f, 1.f });
		Parameters[4] = SourceVector(
			Source, "03_spacular_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_ArtistSpla05Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("11.str", 1.f), S("12.power", 1.f),
			S("05.distort_str", 0.f), S("01.distortion_str", 0.f) };
		Parameters[1] = { S("07.map_a_uvscale_r", 1.f),
			S("08.map_a_uvscale_g", 1.f), S("09.map_a_panning_x", 0.f),
			S("10.map_a_panning_y", 0.f) };
		Parameters[2] = { S("07.map_d_uvscale_r", 1.f),
			S("08.map_d_uvscale_g", 1.f), S("09.map_d_panning_x", 0.f),
			S("10.map_d_panning_y", 0.f) };
		Parameters[3] = { S("04.uv.x", 1.f), S("05.uv.y", 1.f),
			S("20.min_alpha", 0.f), S("41.fresnal_power", 1.f) };
		Parameters[4] = { S("02.spacular_str", 1.f),
			S("03.spacular_power", 1.f), S("06_spacular_uvscale", 1.f),
			S("07_spacular_timescale", 0.f) };
		Parameters[5] = SourceVector(Source, "01.color", { 1.f, 1.f, 1.f, 1.f });
		Parameters[6] = SourceVector(
			Source, "04.spacular_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_ArtistTwinkleConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("twinkle_intensity", 1.f),
			S("add_emissive_intensity", 0.f), S("emissive_paning", 0.f),
			S("emissive_tiling", 1.f) };
		Parameters[1] = { S("twinkle_paning", 0.f),
			S("twinkle_tiling", 1.f), 0.f, 0.f };
		Parameters[2] = SourceVector(
			Source, "emissive_color&intensity", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_ArtistFluid01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("normal_intensity", 0.f),
			S("alpha_power", 1.f), S("alpha_intensity", 1.f),
			S("distiortion_intensity", 0.f) };
		Parameters[1] = { S("blood_coloruv_size", 1.f),
			S("camera&reflrectionblend", 0.f), S("specular_power", 1.f),
			S("specularuv_size", 1.f) };
		Parameters[2] = { S("emissive_desaturation", 0.f), 0.f, 0.f, 0.f };
		Parameters[3] = SourceVector(Source, "emissive_color", { 1.f, 1.f, 1.f, 1.f });
		Parameters[4] = SourceVector(
			Source, "specular_color&intensity", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_ArtistWorldOffset01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("alpha_radius", 0.5f),
			S("outline_hardness", 1.f), S("outline_strangth", 1.f),
			S("alpha_power_02", 1.f) };
		Parameters[1] = { S("uv_noise_velue_centeralpha", 0.f),
			S("uv_noise_velue_centeralpha_02", 0.f),
			S("uv_noise_velue_emissive_tex_01", 0.f),
			S("uv_noise_pola_tex_velue", 0.f) };
		Parameters[2] = { S("emissive_texcoord_u", 1.f),
			S("emissive_texcoord_v", 1.f),
			S("emissive_wave_tex_stragth", 1.f),
			S("worldoffset_emissive_texcoord(x,y)", 1.f) };
		Parameters[3] = { S("worldoffset_uv_noise_texcoord(x,y)_01", 1.f),
			S("worldoffset_uv_noise_texcoord(x,y)_02", 1.f),
			S("worldoffset_uv_noise_texcoord(x,y)_03", 1.f),
			S("emissive_panspeed", 0.f) };
		Parameters[4] = { S("emissive_power", 1.f), 0.f, 0.f, 0.f };
		Parameters[5] = SourceVector(Source, "hole_bright", { 0.f, 0.f, 0.f, 1.f });
	}

	void Build_ArtistLensFlare01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		Parameters[0] = { SourceScalar(Source, "desaturation", 0.f),
			SourceScalar(Source, "select texture(0 or 0.5)", 0.f),
			SourceScalar(Source, "depthbaisalpha", 0.f), 0.f };
	}

	void Build_Glasshole02Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vAuraColor,
		float4_t& vInHoleColor)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("alpha_tile_x", 1.f), S("alpha_tile_y", 1.f),
			S("alpha_offsetx", 0.5f), S("alpha_offsety", 0.5f) };
		Parameters[1] = { S("aura_str", 1.f), S("aura_pow", 1.f),
			S("curve_power", 1.f), S("twist_str", 0.f) };
		Parameters[2] = { S("main_ucoord", 1.f), S("main_v_coord", 0.f),
			S("main_tex_upanner", 0.f), S("main_v_panner", 0.f) };
		Parameters[3] = { S("uvnoise_utile", 1.f), S("uvnoise_vtile", 1.f),
			S("uvnoise_pan", 0.f), S("in_hole_crackuv", 0.f) };
		Parameters[4] = { S("in_hole_panx", 0.f), S("in_hole_pany", 0.f),
			S("in_hole_pow", 1.f), S("in_hole_str", 1.f) };
		Parameters[5] = { S("in_hole_desaturation", 0.f),
			S("distortionpower", 0.f), S("distortionscale", 0.f),
			S("scale", 1.f) };
		Parameters[6] = { S("cracknormal_tile_x", 1.f),
			S("cracknormal_tile_y", 1.f), S("cracknormal_str", 0.f),
			S("edge_crack_desaturation", 0.f) };
		Parameters[7] = { S("edge_line", 4.f), S("edge_size", 2.f),
			S("time", 0.f), S("in_hole_height", 0.f) };
		vAuraColor = SourceVector(
			Source, "aura_color", { 1.f, 1.f, 1.f, 1.f });
		vInHoleColor = SourceVector(
			Source, "in_hole_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_FluidNinja01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vColor1,
		float4_t& vColor2)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("diff_u", 1.f), S("diff_v", 1.f),
			S("diff_pow", 1.f), S("diff_str", 1.f) };
		Parameters[1] = { S("desaturation", 0.f), S("flow_1_str", 0.f),
			S("flow_2_str", 0.f), S("depth", 0.f) };
		Parameters[2] = { S("flow_1_tile_u", 1.f),
			S("flow_1_tile_v", 1.f), S("flow_1_pan_x", 0.f),
			S("flow_1_pan_y", 0.f) };
		Parameters[3] = { S("flow_1_offset_x", 0.f),
			S("flow_1_offset_y", 0.f), S("flow_1_sizecontrol", 1.f),
			S("mask_noisestr", 0.f) };
		Parameters[4] = { S("flow_2_tile_u", 1.f),
			S("flow_2_tile_v", 1.f), S("flow_2_pan_x", 0.f),
			S("flow_2_pan_y", 0.f) };
		Parameters[5] = { S("flow_2_offset_x", 0.f),
			S("flow_2_offset_y", 0.f), S("flow_2_sizecontrol", 1.f),
			S("mask_pow", 1.f) };
		Parameters[6] = { S("mask_u", 1.f), S("mask_v", 1.f),
			S("mask_str", 1.f), S("opacity_str", 1.f) };
		Parameters[7] = { S("opacity_u", 1.f), S("opacity_v", 1.f),
			S("opacity_pow", 1.f), S("gra_pow", 1.f) };
		vColor1 = SourceVector(Source, "color_1", { 1.f, 1.f, 1.f, 1.f });
		vColor2 = SourceVector(Source, "color_2", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_CustomParticle01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vDiffuseColor)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("diff_tile_u", 1.f), S("diff_tile_v", 1.f),
			S("diff_panx_speed", 0.f), S("diff_pany_speed", 0.f) };
		Parameters[1] = { S("diff_offset_x", 0.f), S("diff_offset_y", 0.f),
			S("diff_rotator", 0.f), S("desaturation", 0.f) };
		Parameters[2] = { S("diff_str", 1.f), S("diff_pow", 1.f),
			S("a_noise_01_str", 0.f), S("a_noise_01_tile_u", 1.f) };
		Parameters[3] = { S("a_noise_01_tile_v", 1.f),
			S("a_noise_01_offset_y", 0.f), S("cast_fov", 0.5f),
			S("cast_speed", 0.f) };
		Parameters[4] = { S("cast_center", 0.5f), S("cast_dirinout", 0.f),
			S("cast_particle", 0.f), S("particlesize", 1.f) };
		Parameters[5] = { S("step_min", 0.f), S("step_max", 1.f),
			S("flow_sizecontrol", 1.f), S("flow_str", 0.f) };
		Parameters[6] = { S("flow_tile_u", 1.f), S("flow_tile_v", 1.f),
			S("flow_panx_speed", 0.f), S("flow_pany_speed", 0.f) };
		Parameters[7] = { S("mask_value", 1.f), S("mask_pow", 1.f),
			S("mask_bias", 0.f), S("a_sizecontrol", 1.f) };
		vDiffuseColor = SourceVector(
			Source, "diff_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_CrackholeV2Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEmissionColor,
		float4_t& vBaseColor)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("02.map_e_uvscale_r", 1.f),
			S("03.map_e_uvscale_g", 1.f), S("04.map_e_panning_x", 0.f),
			S("05.map_e_panning_y", 0.f) };
		Parameters[1] = { S("07.map_f_uvscale_r", 1.f),
			S("08.map_f_uvscale_g", 1.f), S("09.map_f_panning_x", 0.f),
			S("10.map_f_panning_y", 0.f) };
		Parameters[2] = { S("04.str", 1.f), S("05.power", 1.f),
			S("11.radius", 1.f), S("12.hardness", 1.f) };
		Parameters[3] = { S("01.thickness", 1.f), S("02.depth", 0.f),
			S("03.innerthickness", 0.f), S("01_thickness", 0.f) };
		Parameters[4] = { S("05.distort_str", 0.f),
			S("07.map_d_uvscale_r", 1.f), S("08.map_d_uvscale_g", 1.f),
			S("09.map_d_panning_x", 0.f) };
		Parameters[5] = { S("10.map_d_panning_y", 0.f),
			S("mask_noise_str", 0.f), S("mask_noise_tile_x", 1.f),
			S("mask_noise_tile_y", 1.f) };
		Parameters[6] = { S("mask_offset_x", 0.f), S("mask_offset_y", 0.f),
			S("mask_pan_speed", 0.f), S("02.curvartuer", 1.f) };
		Parameters[7] = { S("21.uv_offset.x", 0.f),
			S("22.uv_offset.y", 0.f), S("01.line thickness", 1.f),
			S("02.line_power", 1.f) };
		vEmissionColor = SourceVector(
			Source, "19.emission_color", { 1.f, 1.f, 1.f, 1.f });
		vBaseColor = SourceVector(
			Source, "20.base_color", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_Simple01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		/* The grouped fallback funnels every name containing "pan" into one
		   float2, so uv_noise_panning wins over uv_panning and scrolls the
		   emissive sample the source keeps still. Keeping the two UV domains in
		   separate lanes is the whole reason this profile exists. */
		Parameters[0] = { S("uv_panning_x", 0.f), S("uv_panning_y", 0.f),
			S("uv_noise_panning_x", 0.f), S("uv_noise_panning_y", 0.f) };
		Parameters[1] = { S("uv_noise_tilling", 1.f),
			S("uv_noise_intensity", 0.f),
			S("emissive_tex_desturation", 0.f), 0.f };
		Parameters[2] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[3] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[4] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[5] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[6] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[7] = { 0.f, 0.f, 0.f, 0.f };
	}

	/* fx_mm_basic_01_ad / _tr.  The grouped path collapsed this master material
	   to one pan and one gray carrier, which loses the two independent uv_noise
	   domains and treats the dedicated alpha_tex as if it were artwork.  Lane
	   assignment here follows the parent parameter groups: emissive_tex and
	   alpha_tex are the "emissive" group, uv_noise_01/02 are the "uv_noise"
	   group with their own tiling and panning.

	   fresnel_power, edge_power, edge_intensity, depth_alpha_bias,
	   camera_distance and world_normal_intensity are deliberately not packed.
	   They need scene depth and world normal inputs that the RT0 base pass does
	   not carry, and inventing them would change coverage without evidence. */
	void Build_MmBasic01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("uv_panning_x", 0.f), S("uv_panning_y", 0.f),
			S("uv_scale", 1.f), S("emissive_power", 1.f) };
		Parameters[1] = { S("emissive_desaturation", 0.f),
			S("distortion_intensity", 0.f), 0.f, 0.f };
		Parameters[2] = { S("uv_noise_01_panning_x", 0.f),
			S("uv_noise_01_panning_y", 0.f),
			S("uv_noise_01_tiling_x", 1.f),
			S("uv_noise_01_tiling_y", 1.f) };
		Parameters[3] = { S("uv_noise_02_panning_x", 0.f),
			S("uv_noise_02_panning_y", 0.f),
			S("uv_noise_02_tiling_x", 1.f),
			S("uv_noise_02_tiling_y", 1.f) };
		Parameters[4] = { S("uv_noise_01_intensity", 0.f),
			S("uv_noise_02_intensity", 0.f), 0.f, 0.f };
		Parameters[5] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[6] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[7] = { 0.f, 0.f, 0.f, 0.f };
	}

	/* fx_k_me_flowtrail_01_ts_tr.  Three source groups, three UV domains:
	   diff owns radiance, opacity owns coverage and noise offsets both.  The
	   wave group (wave_str, wave_tile, wave_pan_speed, wave_noise_str) is not
	   packed: no child in the corpus overrides wave_tile or wave_pan_speed and
	   the parent expression graph is not in evidence, so its geometry would be
	   invented.  cameravec_pow needs a camera vector the RT0 base pass does not
	   carry.  Both stay in the NATIVE_PARITY backlog. */
	void Build_FlowTrail01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("diff_u_tile", 1.f), S("diff_v_tile", 1.f),
			S("diff_u_center", 0.f), S("diff_rotation", 0.f) };
		Parameters[1] = { S("diff_pow", 1.f), S("diff_str", 1.f),
			S("diff_desturation", 0.f), S("distortion_str", 0.f) };
		Parameters[2] = { S("opacity_u_tile", 1.f), S("opacity_v_tile", 1.f),
			S("opacity_u_center", 0.f), S("opacity_rotation", 0.f) };
		Parameters[3] = { S("opacity_str", 1.f), S("noise_str", 0.f),
			S("noise_u_tile", 1.f), S("noise_v_tile", 1.f) };
		Parameters[4] = { S("noise_u_pan", 0.f), S("noise_v_pan", 0.f),
			0.f, 0.f };
		Parameters[5] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[6] = { 0.f, 0.f, 0.f, 0.f };
		Parameters[7] = { 0.f, 0.f, 0.f, 0.f };
	}

	void Build_Simple02Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters,
		float4_t& vEmissiveColor)
	{
		Build_Simple01Constants(Source, Parameters);
		Parameters[1].w = SourceScalar(
			Source, "emissive_tex_02_intensity", 1.f);
		vEmissiveColor = SourceVector(
			Source, "emissive_color&intensity", { 1.f, 1.f, 1.f, 1.f });
	}

	void Build_MmFluid01SpriteConstants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("transition_tiling", 1.f),
			S("transition_panning_x", 0.f),
			S("transition_panning_y", 0.f),
			S("transition thickness", 0.2f) };
		Parameters[1] = { S("transition direction", 0.f),
			S("transition line thickness", 1.f),
			S("emissive_line_intensity", 1.f),
			S("emissive_intensity", 1.f) };
		Parameters[2] = { S("emissive_uv_scale_x", 1.f),
			S("emissive_uv_scale_y", 1.f),
			S("emissive_desaturation", 0.f),
			S("total_scale", 1.f) };
		Parameters[3] = { S("fresnel_power", 1.f),
			S("distortion_intensity", 0.f), 0.f, 0.f };
	}

	void Build_FlowRibbon01Constants(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strSourceMaterialPath,
		std::array<float4_t, 8u>& Parameters)
	{
		auto S = [&Source](const std::string_view Name, const f32_t Fallback)
		{
			return SourceScalar(Source, Name, Fallback);
		};
		Parameters[0] = { S("maintex_ucoord", 1.f),
			S("maintex_vcoord", 1.f), S("maintex_pos_v", 0.f),
			S("maintex_str", 1.f) };
		Parameters[1] = { S("flow_ucoord", 1.f),
			S("flow_vcoord", 1.f), S("colormap_coord_x", 1.f),
			S("colormap_coord_y", 1.f) };
		Parameters[2] = { S("colormap_des", 0.f),
			S("colormap_power", 1.f), S("colormap_str", 1.f),
			strSourceMaterialPath ==
				"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr" ? 3.f : 1.f };
		Parameters[3] = SourceVector(Source, "colormap_color",
			float4_t(1.f, 1.f, 1.f, 1.f));
		/* These two lanes are exact constant ParticleModuleParameterDynamic
		   distributions for every occurrence admitted by the FlowRibbon carrier.
		   The varying dissolve/distort lanes remain per-point payload. */
		Parameters[4] = { 0.02f, 1.f, 0.f, 0.f };
	}

	bool_t Has_LinearFlowSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		return Client::Has_EffectLinearFlowNamedTextureContract(Source);
	}

	bool_t Has_BlacklineSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		return Client::Has_EffectBlacklineNamedTextureContract(Source);
	}

	bool_t Has_LocalCrackSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		return Client::Has_EffectLocalCrackNamedTextureContract(Source);
	}

	size_t Texture_Index(const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		return static_cast<size_t>(eSlot) -
			static_cast<size_t>(Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	}

	ComPtr<ID3D11ShaderResourceView> Find_Texture(
		const std::array<ComPtr<ID3D11ShaderResourceView>, 8>& Textures,
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		if (eSlot < Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE ||
			eSlot > Client::EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE)
		{
			return nullptr;
		}
		return Textures[Texture_Index(eSlot)];
	}

	const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		std::string_view strSlotId = Client::EFFECT_MESH_SHAPE_SLOT_ID;
		if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL != eSlot)
		{
			const Client::EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
				Client::Find_EffectMaterialTemplate(
					Element.Material.strTemplateId);
			const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				nullptr == pTemplate ? nullptr :
				Client::Find_EffectMaterialInput(*pTemplate, eSlot);
			if (nullptr == pInput)
				return nullptr;
			strSlotId = pInput->strSlotId;
		}
		const auto Iterator = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[strSlotId](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == strSlotId;
			});
		return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
	}

	constexpr uint32_t ARTIST_D_BLACK_TIGER_STROKE_OPCODE = 18u;

	struct ARTIST_D_BLACK_TIGER_STROKE_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceElementId;
		std::string_view strDynamicModuleStableId;
		uint32_t iScalarCount;
	};

	constexpr std::array<ARTIST_D_BLACK_TIGER_STROKE_ROW, 12u>
		ARTIST_D_BLACK_TIGER_STROKE_ROWS = {{
		{ "authored.source-particle.763aea38ab1100ba9072dbfb",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.e6c3ffec9fbc27024e2ce78c",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_5.event_source-event-002",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.91392dd3a1710c9d411bfff6",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.382ed3229ddf083cfd22ee11",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_6.event_source-event-002",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.4f0381d175d441978f26ebfc",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_7",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.31fa700c084ab0b11447f7c7",
			"fx_pc_sdm_08.par_l_sdm_sk_01_3.particlespriteemitter_7.event_source-event-002",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.5571970d95f97aecb889fed7",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.87c8abd0423fcb7e9a725659",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.ac2d4d3e467dc4442cba60c3",
			"fx_pc_sdm_08.par_l_sdm_sk_05_3.particlespriteemitter_11",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
		{ "authored.source-particle.01c398219f73706b66509e77",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_5",
			"FX_PC_MSR_03:export:2587@ref:4", 28u },
		{ "authored.source-particle.93420edbc5815b8a01b38ef4",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_6",
			"FX_PC_SDM_08:export:1241@ref:3", 24u },
		{ "authored.source-particle.76d0b67fe194395ce21c51ab",
			"fx_pc_sdm_08.par_l_sdm_sk_06_3.particlespriteemitter_2",
			"FX_PC_SDM_08:export:1240@ref:3", 24u },
	}};

	constexpr std::array<f32_t, 28u> ARTIST_D_TIGER_CHILD5_SCALARS = {{
		0.f, 0.f, 1.f, 1.f, 1.f, 1.100000023841858f, 1.f, 0.f,
		1.f, 4.f, 0.10000000149011612f, 0.f, 0.f, 0.f,
		-0.10000000149011612f, 0.f, 2.f, 1.f, 1.f, 2.f, 3.f,
		-0.20000000298023224f, 0.f, 0.f, 0.5f, 0.5f, 0.f, 0.f,
	}};

	constexpr std::array<f32_t, 24u> ARTIST_D_TIGER_CHILD6_SCALARS = {{
		0.f, 0.f, 1.f, 1.f, 2.f, 1.2000000476837158f,
		0.10000000149011612f, 5.f, 25.f, 15.f, 0.f,
		-0.10000000149011612f, 0.f, 0.f, 1.f, 1.f, 1.f,
		-0.800000011920929f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f,
	}};

	const ARTIST_D_BLACK_TIGER_STROKE_ROW* Find_ArtistDBlackTigerStrokeRow(
		const std::string_view strElementId)
	{
		const auto Iterator = std::ranges::find_if(
			ARTIST_D_BLACK_TIGER_STROKE_ROWS,
			[strElementId](const ARTIST_D_BLACK_TIGER_STROKE_ROW& Row)
			{
				return Row.strElementId == strElementId;
			});
		return Iterator == ARTIST_D_BLACK_TIGER_STROKE_ROWS.end() ?
			nullptr : &*Iterator;
	}

	bool_t Is_ArtistDBlackTigerSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler)
	{
		return Sampler.eFilter ==
				Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR &&
			Sampler.eAddressU ==
				Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
			Sampler.eAddressV ==
				Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
			Sampler.eAddressW ==
				Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
			Sampler.fMipLodBias == 0.f && Sampler.iMaxAnisotropy == 1u &&
			Sampler.eComparison ==
				Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER &&
			Sampler.vBorderColor.x == 0.f && Sampler.vBorderColor.y == 0.f &&
			Sampler.vBorderColor.z == 0.f && Sampler.vBorderColor.w == 0.f &&
			Sampler.fMinLod == 0.f &&
			Sampler.fMaxLod == (std::numeric_limits<f32_t>::max)();
	}

	bool_t Is_ArtistDBlackTigerLane(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const uint32_t iIndex,
		const std::string_view strRole,
		const std::string_view strAssetId,
		const std::string_view strSourceChannel)
	{
		return Lane.strLaneId == "lane." + std::to_string(iIndex) &&
			Lane.strRole == strRole && Lane.strAssetId == strAssetId &&
			Lane.iTextureRegister == iIndex &&
			Lane.iSamplerRegister == 5u + iIndex &&
			Lane.strSourceChannel == strSourceChannel &&
			Lane.eColorSpace == Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
			Is_ArtistDBlackTigerSampler(Lane.Sampler);
	}

	template <size_t ScalarCount>
	bool_t Is_ArtistDBlackTigerScalars(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		const std::array<f32_t, ScalarCount>& Expected)
	{
		if (Execution.Scalars.size() != Expected.size())
			return false;
		for (size_t i = 0u; i < Expected.size(); ++i)
		{
			const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			if (Scalar.strName != "scalar." + std::to_string(i) ||
				Scalar.iPackedIndex != i || Scalar.fValue != Expected[i])
			{
				return false;
			}
		}
		return true;
	}

	bool_t Has_ArtistDBlackTigerDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strExpectedStableId)
	{
		size_t iMatchCount = 0u;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			++iMatchCount;
			if (Module.strStableId != strExpectedStableId ||
				Module.Distributions.size() != 4u)
			{
				return false;
			}
			for (size_t i = 0u; i < Module.Distributions.size(); ++i)
			{
				if (Module.Distributions[i].strPropertyPath !=
					"dynamicparams[" + std::to_string(i) + "].paramvalue")
				{
					return false;
				}
			}
		}
		return iMatchCount == 1u;
	}

	bool_t Validate_ArtistDBlackTigerStrokeExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
			Element.Material.Execution;
		const ARTIST_D_BLACK_TIGER_STROKE_ROW* pRow =
			Find_ArtistDBlackTigerStrokeRow(
				Client::Resolve_EffectPortableOriginElementId(Element));
		if (nullptr == pRow)
		{
			if (Execution.bEnabled &&
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				Execution.iOpcode == ARTIST_D_BLACK_TIGER_STROKE_OPCODE)
			{
				strOutError =
					"Artist D BLACK_TIGER_STROKE opcode escaped its exact occurrence allowlist: " +
					Element.strElementId;
				return false;
			}
			return true;
		}

		const bool_t bChild5 = pRow->iScalarCount == 28u;
		const std::string_view strMaterialPath = bChild5 ?
			"fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_5_ad" :
			"fx_m_mi_l_00.fx_mi.fx_l_pa_spritewave_01_6_ad";
		const std::string_view strBaseAsset = bChild5 ?
			"Effect/Artist/Textures/fx_m_trail_010.dds" :
			"Effect/Artist/Textures/fx_m_trail_004_cl.dds";
		const std::string_view strNoiseAsset = bChild5 ?
			"Effect/Artist/Textures/fx_c_noise_009.dds" :
			"Effect/Artist/Textures/fx_bg_dustpanner_01.dds";
		constexpr std::string_view strDissolveAsset =
			"Effect/Artist/Textures/fx_o_symbol_14.dds";
		const std::string strExpectedSourceNode =
			"authored-source-particle:effect.artist.skill.31490.unified|source:"
			"effect.artist.skill.31490.imported|element:" +
			std::string(pRow->strSourceElementId);

		const bool_t bCarrier = Element.bVisible &&
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Client::Is_EffectSourceIdentityOrPortableCopy(
				Element, pRow->strElementId, strExpectedSourceNode) &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "sprite" &&
			Element.ResourceBindings.size() == 3u &&
			Element.ResourceBindings[0u].strSlotId == "base" &&
			Element.ResourceBindings[0u].strAssetId == strBaseAsset &&
			Element.ResourceBindings[1u].strSlotId == "dissolve" &&
			Element.ResourceBindings[1u].strAssetId == strDissolveAsset &&
			Element.ResourceBindings[2u].strSlotId == "noise" &&
			Element.ResourceBindings[2u].strAssetId == strNoiseAsset &&
			nullptr == Find_Binding(
				Element, Client::EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			Has_ArtistDBlackTigerDynamicModule(
				Element, pRow->strDynamicModuleStableId);
		const bool_t bMaterial =
			Element.Material.strTemplateId == "effect.standard" &&
			Element.Material.strSourceMaterialPath == strMaterialPath &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ &&
			!Element.Material.SourceMaterial.bEnabled;
		const bool_t bPacketIdentity = Execution.bEnabled &&
			!Execution.bFailClosed && !Execution.bAuthoringApproximate &&
			Execution.iVersion == 1u &&
			Execution.eBackend ==
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Execution.iOpcode == ARTIST_D_BLACK_TIGER_STROKE_OPCODE &&
			Execution.iPassIndex == 2u &&
			Execution.strRasterizerState == "RS_Cull_None" &&
			Execution.strDepthStencilState == "DSS_ReadOnly" &&
			Execution.strBlendState == "BS_EffectAdditive" &&
			Execution.iStencilReference == 0u &&
			Execution.iTextureLaneCount == 3u &&
			Execution.iTextureMask == 0x07u &&
			Execution.TextureLanes.size() == 3u &&
			Is_ArtistDBlackTigerLane(Execution.TextureLanes[0u], 0u,
				"maintex", strBaseAsset, "RGB") &&
			Is_ArtistDBlackTigerLane(Execution.TextureLanes[1u], 1u,
				"uv_noise_tex", strNoiseAsset, "RG") &&
			Is_ArtistDBlackTigerLane(Execution.TextureLanes[2u], 2u,
				"dissolve_tex_01", strDissolveAsset, "R");
		const uint32_t iInputMask = bChild5 ? 0x0fffffffu : 0x00ffffffu;
		const bool_t bPacketMasks =
			Execution.iDynamicConsumedMask == 0x0fu &&
			Execution.iDynamicSuppressedMask == 0u &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x0fu &&
			Execution.iParticleColorSuppressedMask == 0u &&
			Execution.iScalarCount == pRow->iScalarCount &&
			Execution.iVectorCount == 1u &&
			Execution.iInputCount == pRow->iScalarCount &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ iInputMask, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0x07u, 0u, 0u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0x08u, 0u, 0u } &&
			Execution.iStaticInputCount == 0u &&
			Execution.iStaticSelectedMask == 0u &&
			Execution.iStaticConsumedMask == 0u &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 6u &&
			Execution.iRenderConsumedMask == 0x2fu &&
			Execution.iRenderSuppressedMask == 0x10u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty();
		const bool_t bScalars = bChild5 ?
			Is_ArtistDBlackTigerScalars(
				Execution, ARTIST_D_TIGER_CHILD5_SCALARS) :
			Is_ArtistDBlackTigerScalars(
				Execution, ARTIST_D_TIGER_CHILD6_SCALARS);
		const float4_t vExpectedEdge = bChild5 ?
			float4_t(1.f, 1.f, 1.f, 1.f) :
			float4_t(20.f, 20.f, 20.f, 1.f);
		const bool_t bVector = Execution.Vectors.size() == 1u &&
			Execution.Vectors[0u].strName == "vector.0" &&
			Execution.Vectors[0u].iPackedIndex == 0u &&
			Execution.Vectors[0u].vValue.x == vExpectedEdge.x &&
			Execution.Vectors[0u].vValue.y == vExpectedEdge.y &&
			Execution.Vectors[0u].vValue.z == vExpectedEdge.z &&
			Execution.Vectors[0u].vValue.w == vExpectedEdge.w;
		if (!bCarrier || !bMaterial || !bPacketIdentity || !bPacketMasks ||
			!bScalars || !bVector)
		{
			strOutError =
				"Artist D BLACK_TIGER_STROKE exact typed contract changed: " +
				Element.strElementId;
			return false;
		}
		return true;
	}

	constexpr uint32_t LANCE_DRAGON_MASKED_OPCODE = 19u;
	constexpr uint32_t PROJECT_BASE_COVERAGE_EMISSIVE_DISSOLVE_RECT_OPCODE = 21u;
	constexpr uint32_t WARLORD_WPO_SINWAVE_ELECTRIC_RT0_OPCODE = 22u;

	struct WARLORD_WPO_SINWAVE_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceNode;
	};

	constexpr std::array<WARLORD_WPO_SINWAVE_ROW, 2u>
		WARLORD_WPO_SINWAVE_ROWS = {{
		{ "authored.source-particle.8c0d6ab070c1a6c83479e590",
			"authored-source-particle:effect.warlord.skill.17140.unified|"
			"source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
			"par_s_wgl_guardianlightning_01.particlespriteemitter_13" },
		{ "authored.source-particle.59e6ffa8852fba74279b6ae9",
			"authored-source-particle:effect.warlord.skill.17140.unified|"
			"source:effect.warlord.skill.17140.imported|element:fx_pc_wgl_07."
			"par_s_wgl_guardianlightning_02.particlespriteemitter_52" },
	}};

	const WARLORD_WPO_SINWAVE_ROW* Find_WarlordWpoSinWaveRow(
		const std::string_view strElementId)
	{
		const auto Iterator = std::ranges::find_if(
			WARLORD_WPO_SINWAVE_ROWS,
			[strElementId](const WARLORD_WPO_SINWAVE_ROW& Row)
			{
				return Row.strElementId == strElementId;
			});
		return Iterator == WARLORD_WPO_SINWAVE_ROWS.end() ? nullptr : &*Iterator;
	}

	bool_t Has_WarlordWpoSinWaveDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		static constexpr std::array<std::string_view, 4u> PATHS = {{
			"dynamicparams[0].paramvalue", "dynamicparams[1].paramvalue",
			"dynamicparams[2].paramvalue", "dynamicparams[3].paramvalue"
		}};
		static constexpr std::array<std::array<f32_t, 4u>, 4u> VALUES = {{
			{{ 1.f, 1.f, 1.f, 1.f }}, {{ 0.f, 1.f, 1.f, 0.f }},
			{{ 0.5f, 0.5f, 0.5f, 0.5f }}, {{ 1.f, 1.f, 1.f, 1.f }}
		}};
		size_t iCount = 0u;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			++iCount;
			if (Module.strStableId != "FX_PC_WGL_07:export:1032@ref:4" ||
				Module.Distributions.size() != PATHS.size())
			{
				return false;
			}
			for (size_t i = 0u; i < PATHS.size(); ++i)
			{
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution =
					Module.Distributions[i];
				if (Distribution.strPropertyPath != PATHS[i] ||
					Distribution.LookupTable.size() != VALUES[i].size() ||
					!std::equal(Distribution.LookupTable.begin(),
						Distribution.LookupTable.end(), VALUES[i].begin()))
				{
					return false;
				}
			}
		}
		return iCount == 1u;
	}

	bool_t Validate_WarlordWpoSinWaveElectricExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
			Element.Material.Execution;
		const WARLORD_WPO_SINWAVE_ROW* pRow =
			Find_WarlordWpoSinWaveRow(
				Client::Resolve_EffectPortableOriginElementId(Element));
		if (nullptr == pRow)
		{
			if (Execution.bEnabled &&
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				Execution.iOpcode == WARLORD_WPO_SINWAVE_ELECTRIC_RT0_OPCODE)
			{
				strOutError =
					"Warlord WPO SinWave opcode escaped its two-occurrence allowlist: " +
					Element.strElementId;
				return false;
			}
			return true;
		}
		/* The Product Warlord F rows intentionally remain on their byte-frozen
		   grouped source profile.  Only a separately authored Tool candidate
		   enables opcode 22. */
		if (!Execution.bEnabled)
			return true;

		static constexpr std::array<std::string_view, 2u> LANE_ROLES = {{
			"alpha_mask_21_map_c", "emission_02_map_e"
		}};
		static constexpr std::array<std::string_view, 2u> LANE_ASSETS = {{
			"Effect/Warlord/Textures/FX_TEX_04/fx_i_thunder_02_ycl.dds",
			"Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_049.dds"
		}};
		static constexpr std::array<std::string_view, 2u> LANE_CHANNELS = {{
			"R", "RGB"
		}};
		static constexpr std::array<std::string_view, 10u> SCALAR_NAMES = {{
			"alpha_power", "alpha_strength", "alpha_uv_scale_x",
			"alpha_uv_scale_y", "emission_power", "emission_desaturation",
			"emission_uv_scale_x", "emission_uv_scale_y",
			"emission_pan_x", "emission_pan_y"
		}};
		static constexpr std::array<f32_t, 10u> SCALAR_VALUES = {{
			2.f, 2.f, 2.f, 1.5f, 2.f, 1.f, 4.f, 1.f, 0.f, 0.f
		}};
		const auto NearlyEqual = [](const f32_t fLeft, const f32_t fRight)
		{
			return std::abs(fLeft - fRight) <= 1.0e-6f *
				(std::max)({ 1.f, std::abs(fLeft), std::abs(fRight) });
		};

		bool_t bValid = Element.bVisible &&
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Client::Is_EffectSourceIdentityOrPortableCopy(
				Element, pRow->strElementId, pRow->strSourceNode) &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "mesh" &&
			Has_WarlordWpoSinWaveDynamicModule(Element) &&
			Element.ResourceBindings.size() == 3u &&
			Element.ResourceBindings[0u].strSlotId == "meshModel" &&
			Element.ResourceBindings[0u].strAssetId ==
				"Effect/Warlord/Meshes/FX_SM_00/"
				"fm_d_electric_05_vertexcolor.wmodel" &&
			Element.ResourceBindings[1u].strSlotId == "base" &&
			Element.ResourceBindings[1u].strAssetId == LANE_ASSETS[1u] &&
			Element.ResourceBindings[2u].strSlotId == "noise" &&
			Element.ResourceBindings[2u].strAssetId == LANE_ASSETS[0u] &&
			Element.Material.strTemplateId == "effect.standard" &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_d_00.fx_mi."
				"fx_d_me_worldpositionoffset_sinwave_01_04_ad" &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.Material.SourceMaterial.strProfileId ==
				"ue3.material.fx.m.mi.d.00.fx.m.fx.d.me."
				"worldpositionoffset.sinwave.01.ad.1feb93cbb95e" &&
			Element.Material.SourceMaterial.strParentMaterialPath ==
				"fx_m_mi_d_00.fx_m."
				"fx_d_me_worldpositionoffset_sinwave_01_ad" &&
			!Execution.bFailClosed && !Execution.bAuthoringApproximate &&
			Execution.iVersion == 1u &&
			Execution.eBackend ==
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Execution.iOpcode == WARLORD_WPO_SINWAVE_ELECTRIC_RT0_OPCODE &&
			Execution.iPassIndex == 4u &&
			Execution.strRasterizerState == "RS_Default" &&
			Execution.strDepthStencilState == "DSS_ReadOnly" &&
			Execution.strBlendState == "BS_EffectAdditive" &&
			Execution.iStencilReference == 0u &&
			Execution.iTextureLaneCount == 2u &&
			Execution.iTextureMask == 0x03u &&
			Execution.TextureLanes.size() == 2u &&
			Execution.iDynamicConsumedMask == 0x02u &&
			Execution.iDynamicSuppressedMask == 0x0du &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x0fu &&
			Execution.iParticleColorSuppressedMask == 0u &&
			Execution.iScalarCount == SCALAR_NAMES.size() &&
			Execution.Scalars.size() == SCALAR_NAMES.size() &&
			Execution.iVectorCount == 1u && Execution.Vectors.size() == 1u &&
			Execution.iInputCount == 10u &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ 0x03ffu, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0x07u, 0u, 0u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0x08u, 0u, 0u } &&
			Execution.iStaticInputCount == 0u &&
			Execution.iStaticSelectedMask == 0u &&
			Execution.iStaticConsumedMask == 0u &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 6u &&
			Execution.iRenderConsumedMask == 0x2fu &&
			Execution.iRenderSuppressedMask == 0x10u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty();
		for (size_t i = 0u; bValid && i < LANE_ROLES.size(); ++i)
		{
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[i];
			bValid = Lane.strLaneId == "lane." + std::to_string(i) &&
				Lane.strRole == LANE_ROLES[i] && Lane.strAssetId == LANE_ASSETS[i] &&
				Lane.iTextureRegister == i && Lane.iSamplerRegister == 5u + i &&
				Lane.strSourceChannel == LANE_CHANNELS[i] &&
				Lane.eColorSpace == Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
				Is_ArtistDBlackTigerSampler(Lane.Sampler);
		}
		for (size_t i = 0u; bValid && i < SCALAR_NAMES.size(); ++i)
		{
			const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			bValid = Scalar.strName == SCALAR_NAMES[i] &&
				Scalar.iPackedIndex == i &&
				NearlyEqual(Scalar.fValue, SCALAR_VALUES[i]);
		}
		if (bValid)
		{
			const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Color =
				Execution.Vectors[0u];
			bValid = Color.strName == "emission_color" &&
				Color.iPackedIndex == 0u && NearlyEqual(Color.vValue.x, 5.f) &&
				NearlyEqual(Color.vValue.y, 5.f) &&
				NearlyEqual(Color.vValue.z, 5.f) &&
				NearlyEqual(Color.vValue.w, 1.f);
		}
		if (!bValid)
		{
			strOutError =
				"Warlord WPO SinWave opcode 22 packet is not the admitted "
				"child/parent/carrier/two-lane Tool tuple: " +
				Element.strElementId;
			return false;
		}
		return true;
	}

	struct LANCE_DRAGON_MASKED_ROW final
	{
		std::string_view strElementId;
		std::string_view strSourceNode;
		std::string_view strMeshAssetId;
		std::string_view strSourceMaterialPath;
		std::string_view strDynamicModuleStableId;
		bool_t bBody;
	};

	constexpr std::array<LANCE_DRAGON_MASKED_ROW, 12u>
		LANCE_DRAGON_MASKED_ROWS = {{
		{ "authored.source-particle.2b0f00d91a20ba785ba034ec",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.71ac47f40d13b3a7ca6ed561",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.aa3beb2d7ebbe4922f6df595",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_3",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.237b5cd9d1fafb4b95b41212",
			"authored-source-particle:effect.lancemaster.skill.34630.clip1.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_start.particlespriteemitter_4",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.6542736b94e7b9cd8ed5f2fd",
			"authored-source-particle:effect.lancemaster.skill.34630.clip2.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14.event_source-event-033",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.85571ac576a68cc3ff037cae",
			"authored-source-particle:effect.lancemaster.skill.34630.clip2.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15.event_source-event-033",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.92af24faaaeb30c6ac77d37c",
			"authored-source-particle:effect.lancemaster.skill.34630.clip3.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_14.event_source-event-048",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.80d7156c29e9bf140631ad2e",
			"authored-source-particle:effect.lancemaster.skill.34630.clip3.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_loop.particlespriteemitter_15.event_source-event-048",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.538fe0779d0718d30b68ef11",
			"authored-source-particle:effect.lancemaster.skill.34630.clip4.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_0",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_FS_AV_08:export:509@ref:5", true },
		{ "authored.source-particle.50385d998091ed0e55a047f8",
			"authored-source-particle:effect.lancemaster.skill.34630.clip4.unified|source:effect.lance_master.skill.34630.imported|element:fx_pc_flm_09.par_s_flm_superlance_wp_end.particlespriteemitter_1",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
		{ "authored.source-particle.0a019ebaff2bb55941d23ab8",
			"authored-source-particle:effect.lancemaster.skill.34650.clip1.unified|source:effect.lance_master.skill.34650.imported|element:fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s.particlespriteemitter_14",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_01_msk",
			"FX_PC_DDK_03:export:4292@ref:5", true },
		{ "authored.source-particle.65b74589de96c3f44e625f24",
			"authored-source-particle:effect.lancemaster.skill.34650.clip1.unified|source:effect.lance_master.skill.34650.imported|element:fx_pc_flm_08.par_t_flm_dragoncleave_01_wpcast_01_s.particlespriteemitter_15",
			"Effect/LanceMaster/Meshes/fm_x_flm_gdr_01_dragon.wmodel",
			"fx_m_mi_t_00.fx_mi.fx_t_me_master_01_ph_02_msk",
			"FX_PC_DDK_03:export:4292@ref:5", false },
	}};

	constexpr std::array<std::string_view, 25u> LANCE_DRAGON_SCALAR_NAMES = {{
		"02.n.uvscale.x", "03.n.uvscale.y", "05.n.panning.x",
		"06.n.panning.y", "11.normalmap.str", "02.map_a_uvscale_r",
		"03.map_a_uvscale_g", "04.map_a_panning_x", "05.map_a_panning_y",
		"36.str", "37.power", "03.emap_uv.x.scale", "04.emap_uv.y.scale",
		"15.emissiion_power", "02.uvscale.x", "03.uvscale.y",
		"91.desaturation", "92.emissiion_power", "05.specmap_uvscale.x",
		"06.specmap_uvscale.y", "02.specmap_str", "07.desaturation",
		"08.specmap_power", "05.power", "06.str",
	}};

	constexpr std::array<f32_t, 25u> LANCE_DRAGON_SCALAR_VALUES = {{
		1.f, 1.f, 0.f, 0.f, 1.5f, 10.f, 10.f, 0.f, -0.125f, 10.f,
		1.f, 1.f, 1.f, 0.1f, 1.f, 1.f, 0.f, 1.f, 1.f, 1.f, 0.25f,
		0.f, 1.2f, 2.f, 1.f,
	}};

	const LANCE_DRAGON_MASKED_ROW* Find_LanceDragonMaskedRow(
		const std::string_view strElementId)
	{
		const auto Iterator = std::ranges::find_if(
			LANCE_DRAGON_MASKED_ROWS,
			[strElementId](const LANCE_DRAGON_MASKED_ROW& Row)
			{
				return Row.strElementId == strElementId;
			});
		return Iterator == LANCE_DRAGON_MASKED_ROWS.end() ? nullptr : &*Iterator;
	}

	bool_t Has_LanceDragonDynamicModule(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strExpectedStableId)
	{
		size_t iMatchCount = 0u;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			++iMatchCount;
			if (Module.strStableId != strExpectedStableId ||
				Module.Distributions.size() != 4u)
			{
				return false;
			}
			for (size_t i = 0u; i < Module.Distributions.size(); ++i)
			{
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution =
					Module.Distributions[i];
				if (Distribution.strPropertyPath !=
						"dynamicparams[" + std::to_string(i) + "].paramvalue" ||
					Distribution.LookupTable.size() != 4u ||
					!std::ranges::all_of(Distribution.LookupTable,
						[](const f32_t fValue) { return fValue == 1.f; }))
				{
					return false;
				}
			}
		}
		return iMatchCount == 1u;
	}

	bool_t Is_LanceDragonLane(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const uint32_t iIndex,
		const std::string_view strRole,
		const std::string_view strAssetId,
		const std::string_view strChannel)
	{
		return Lane.strLaneId == "lane." + std::to_string(iIndex) &&
			Lane.strRole == strRole && Lane.strAssetId == strAssetId &&
			Lane.iTextureRegister == iIndex &&
			Lane.iSamplerRegister == 5u + iIndex &&
			Lane.strSourceChannel == strChannel &&
			Lane.eColorSpace == Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
			Is_ArtistDBlackTigerSampler(Lane.Sampler);
	}

	bool_t Validate_LanceDragonMaskedExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const auto NearlyEqual = [](const f32_t fLeft, const f32_t fRight)
		{
			return std::abs(fLeft - fRight) <= 0.0001f;
		};
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
			Element.Material.Execution;
		const LANCE_DRAGON_MASKED_ROW* pRow =
			Find_LanceDragonMaskedRow(
				Client::Resolve_EffectPortableOriginElementId(Element));
		if (nullptr == pRow)
		{
			if (Execution.bEnabled &&
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				Execution.iOpcode == LANCE_DRAGON_MASKED_OPCODE)
			{
				strOutError =
					"Lance dragon opcode escaped its exact occurrence allowlist: " +
					Element.strElementId;
				return false;
			}
			return true;
		}

		constexpr std::string_view PARENT_MATERIAL =
			"fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk";
		constexpr std::string_view PROFILE_ID =
			"ue3.material.fx.m.mi.00.fx.m.fx.d.me.master.01.ph.msk.8230663740c0";
		const std::string_view strNormal = pRow->bBody ?
			"Effect/LanceMaster/Textures/sk_flm_gdr_01_n.dds" :
			"Effect/LanceMaster/Textures/sk_flm_gdr_02_n.dds";
		constexpr std::string_view strAlpha =
			"Effect/LanceMaster/Textures/fx_d_noise_043.dds";
		const std::string_view strEmission = pRow->bBody ?
			"Effect/LanceMaster/Textures/sk_flm_gdr_01_e.dds" :
			"Effect/LanceMaster/Textures/sk_flm_gdr_02_e.dds";
		const std::string_view strDiffuse = pRow->bBody ?
			"Effect/LanceMaster/Textures/sk_flm_gdr_01_d.dds" :
			"Effect/LanceMaster/Textures/sk_flm_gdr_02_d.dds";
		const std::string_view strSpecular = pRow->bBody ?
			"Effect/LanceMaster/Textures/fx_d_atypical_010.dds" :
			"Effect/LanceMaster/Textures/sk_flm_gdr_02_s.dds";

		const bool_t bCarrier = Element.bVisible &&
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Client::Is_EffectSourceIdentityOrPortableCopy(
				Element, pRow->strElementId, pRow->strSourceNode) &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "mesh" &&
			Element.ResourceBindings.size() == 6u &&
			Element.ResourceBindings[0u].strSlotId == "meshModel" &&
			Element.ResourceBindings[0u].strAssetId == pRow->strMeshAssetId &&
			Element.ResourceBindings[1u].strSlotId == "base" &&
			Element.ResourceBindings[1u].strAssetId == strDiffuse &&
			Element.ResourceBindings[2u].strSlotId == "dissolve" &&
			Element.ResourceBindings[2u].strAssetId == strAlpha &&
			Element.ResourceBindings[3u].strSlotId == "noise" &&
			Element.ResourceBindings[3u].strAssetId == strNormal &&
			Element.ResourceBindings[4u].strSlotId == "mask" &&
			Element.ResourceBindings[4u].strAssetId == strSpecular &&
			Element.ResourceBindings[5u].strSlotId == "emissive" &&
			Element.ResourceBindings[5u].strAssetId == strEmission &&
			Has_LanceDragonDynamicModule(
				Element, pRow->strDynamicModuleStableId);
		const bool_t bMaterial =
			Element.Material.strTemplateId == "effect.standard" &&
			Element.Material.strSourceMaterialPath ==
				pRow->strSourceMaterialPath &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.Material.SourceMaterial.strProfileId == PROFILE_ID &&
			Element.Material.SourceMaterial.strParentMaterialPath ==
				PARENT_MATERIAL;

		const bool_t bPacket = Execution.bEnabled && !Execution.bFailClosed &&
			!Execution.bAuthoringApproximate && Execution.iVersion == 1u &&
			Execution.eBackend ==
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Execution.iOpcode == LANCE_DRAGON_MASKED_OPCODE &&
			Execution.iPassIndex == 3u &&
			Execution.strRasterizerState == "RS_Default" &&
			Execution.strDepthStencilState == "DSS_ReadOnly" &&
			Execution.strBlendState == "BS_EffectAlpha" &&
			Execution.iStencilReference == 0u &&
			Execution.iTextureLaneCount == 5u &&
			Execution.iTextureMask == 0x1fu &&
			Execution.TextureLanes.size() == 5u &&
			Is_LanceDragonLane(Execution.TextureLanes[0u], 0u,
				"normal_map", strNormal, "RG") &&
			Is_LanceDragonLane(Execution.TextureLanes[1u], 1u,
				"alpha_map", strAlpha, "R") &&
			Is_LanceDragonLane(Execution.TextureLanes[2u], 2u,
				"emission_map", strEmission, "RGB") &&
			Is_LanceDragonLane(Execution.TextureLanes[3u], 3u,
				"diffuse_map", strDiffuse, "RGB") &&
			Is_LanceDragonLane(Execution.TextureLanes[4u], 4u,
				"specular_map", strSpecular, "RGB");
		const bool_t bMasks = Execution.iDynamicConsumedMask == 0x08u &&
			Execution.iDynamicSuppressedMask == 0x07u &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x0fu &&
			Execution.iParticleColorSuppressedMask == 0u &&
			Execution.iScalarCount == 25u && Execution.iVectorCount == 3u &&
			Execution.iInputCount == 25u &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ 0x01ffffffu, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0x07u, 0x07u, 0x07u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0x08u, 0x08u, 0x08u } &&
			Execution.iStaticInputCount == 23u &&
			Execution.iStaticSelectedMask == 0x0013b74fu &&
			Execution.iStaticConsumedMask == 0x007fffffu &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 6u &&
			Execution.iRenderConsumedMask == 0x2fu &&
			Execution.iRenderSuppressedMask == 0x10u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty();

		bool_t bScalars = Execution.Scalars.size() ==
			LANCE_DRAGON_SCALAR_VALUES.size();
		for (size_t i = 0u; bScalars && i < Execution.Scalars.size(); ++i)
		{
			const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			bScalars = Scalar.strName == LANCE_DRAGON_SCALAR_NAMES[i] &&
				Scalar.iPackedIndex == i &&
				NearlyEqual(Scalar.fValue, LANCE_DRAGON_SCALAR_VALUES[i]);
		}
		bool_t bVectors = Execution.Vectors.size() == 3u;
		if (bVectors)
		{
			const auto& Diffuse = Execution.Vectors[0u];
			const auto& Specular = Execution.Vectors[1u];
			const auto& Emission = Execution.Vectors[2u];
			bVectors = Diffuse.strName == "93.emissiion_color" &&
				Diffuse.iPackedIndex == 0u &&
				NearlyEqual(Diffuse.vValue.x, 1.f) &&
				NearlyEqual(Diffuse.vValue.y, 1.f) &&
				NearlyEqual(Diffuse.vValue.z, 1.f) &&
				NearlyEqual(Diffuse.vValue.w, 1.f) &&
				Specular.strName == "09.specmap_color" &&
				Specular.iPackedIndex == 1u &&
				NearlyEqual(Specular.vValue.x, 5.f) &&
				NearlyEqual(Specular.vValue.y, 2.5f) &&
				NearlyEqual(Specular.vValue.z, 0.75f) &&
				NearlyEqual(Specular.vValue.w, 1.f) &&
				Emission.strName == "19.emissiion_color" &&
				Emission.iPackedIndex == 2u &&
				NearlyEqual(Emission.vValue.x, 10.f) &&
				NearlyEqual(Emission.vValue.y, 0.f) &&
				NearlyEqual(Emission.vValue.z, 0.f) &&
				NearlyEqual(Emission.vValue.w, 1.f);
		}

		if (!bCarrier || !bMaterial || !bPacket || !bMasks ||
			!bScalars || !bVectors)
		{
			const std::string_view strChangedSection = !bCarrier ? "carrier" :
				!bMaterial ? "material" : !bPacket ? "packet" :
				!bMasks ? "masks" : !bScalars ? "scalars" : "vectors";
			strOutError =
				"Lance dragon exact typed contract changed (" +
				std::string(strChangedSection) + "): " +
				Element.strElementId;
			if (!bMaterial)
			{
				strOutError += " [template=" + Element.Material.strTemplateId +
					", source=" + Element.Material.strSourceMaterialPath +
					", renderProfile=" + std::to_string(static_cast<uint32_t>(
						Element.Material.eRenderProfile)) +
					", enabled=" + std::to_string(
						Element.Material.SourceMaterial.bEnabled ? 1u : 0u) +
					", profile=" + Element.Material.SourceMaterial.strProfileId +
					", parent=" +
					Element.Material.SourceMaterial.strParentMaterialPath + "]";
			}
			return false;
		}
		return true;
	}

	bool_t Is_SourceMaterialFallbackBlocked(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_GROUPED_TRANSLUCENT_CONSTANTS& GroupedConstants)
	{
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source =
			Element.Material.SourceMaterial;
		if (!Source.bEnabled)
			return false;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.fallback-blocked.v1")
		{
			return true;
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.grouped-translucent.v1")
		{
			const Client::EFFECT_RESOURCE_BINDING_DESC* pBase =
				Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
			const bool_t bSafeBase = nullptr != pBase &&
				!Client::Is_UnsafeEffectBaseTextureAssetId(pBase->strAssetId);
			return !Client::Is_EffectGroupedTranslucentResourceContractSatisfied(
				GroupedConstants,
				bSafeBase,
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE));
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.linearflow-02.v1")
		{
			return !Has_LinearFlowSourceTextureContract(Source);
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.blackline-aura.v1")
		{
			return !Has_BlacklineSourceTextureContract(Source);
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.local-crack.v1")
		{
			const bool_t bHasNamedContract =
				Has_LocalCrackSourceTextureContract(Source);
			return !Client::Is_EffectLocalCrackResourceContractSatisfied(
				bHasNamedContract, bHasNamedContract, bHasNamedContract,
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL));
		}
		if (Source.strRuntimeShaderProfileId == "effect.ue3.shine.v1" ||
			Source.strRuntimeShaderProfileId == "effect.ue3.slice.v1" ||
			Source.strRuntimeShaderProfileId ==
				Client::EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID ||
			Source.strRuntimeShaderProfileId ==
				Client::EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID ||
			Source.strRuntimeShaderProfileId ==
				Client::EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID ||
			Source.strRuntimeShaderProfileId ==
				"effect.ue3.procedural-center-glow.v1")
		{
			const Client::EFFECT_RESOURCE_BINDING_DESC* pBase =
				Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
			return !Client::Is_EffectFiniteProfileResourceContractSatisfied(
				Source.strRuntimeShaderProfileId,
				nullptr != pBase &&
					!Client::Is_UnsafeEffectBaseTextureAssetId(
						pBase->strAssetId),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE),
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL));
		}
		if (Source.strRuntimeShaderProfileId !=
			"effect.ue3.reconstructed-standard.v1")
		{
			return false;
		}
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBase =
			Find_Binding(Element, Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
		return nullptr == pBase ||
			Client::Is_UnsafeEffectBaseTextureAssetId(pBase->strAssetId);
	}

	bool_t Same_MaterialFloat4(
		const float4_t& Left,
		const float4_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y &&
			Left.z == Right.z && Left.w == Right.w;
	}

	bool_t Same_MaterialSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Left,
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Right)
	{
		return Left.eFilter == Right.eFilter &&
			Left.eAddressU == Right.eAddressU &&
			Left.eAddressV == Right.eAddressV &&
			Left.eAddressW == Right.eAddressW &&
			Left.fMipLodBias == Right.fMipLodBias &&
			Left.iMaxAnisotropy == Right.iMaxAnisotropy &&
			Left.eComparison == Right.eComparison &&
			Same_MaterialFloat4(Left.vBorderColor, Right.vBorderColor) &&
			Left.fMinLod == Right.fMinLod &&
			Left.fMaxLod == Right.fMaxLod;
	}

	bool_t Same_MaterialTextureLanes(
		const std::vector<Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iLane = 0u; iLane < Left.size(); ++iLane)
		{
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& A = Left[iLane];
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& B = Right[iLane];
			if (A.strLaneId != B.strLaneId || A.strRole != B.strRole ||
				A.strAssetId != B.strAssetId ||
				A.iTextureRegister != B.iTextureRegister ||
				A.iSamplerRegister != B.iSamplerRegister ||
				A.strSourceChannel != B.strSourceChannel ||
				A.eColorSpace != B.eColorSpace ||
				!Same_MaterialSampler(A.Sampler, B.Sampler))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Same_MaterialScalars(
		const std::vector<Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iScalar = 0u; iScalar < Left.size(); ++iScalar)
		{
			if (Left[iScalar].strName != Right[iScalar].strName ||
				Left[iScalar].iPackedIndex != Right[iScalar].iPackedIndex ||
				Left[iScalar].fValue != Right[iScalar].fValue)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Same_MaterialVectors(
		const std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Left,
		const std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iVector = 0u; iVector < Left.size(); ++iVector)
		{
			if (Left[iVector].strName != Right[iVector].strName ||
				Left[iVector].iPackedIndex != Right[iVector].iPackedIndex ||
				!Same_MaterialFloat4(
					Left[iVector].vValue, Right[iVector].vValue))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Same_StandardColorV1(
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Left,
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Right)
	{
		return Left.iPacketVersion == Right.iPacketVersion &&
			Left.strBaseRadianceLaneId == Right.strBaseRadianceLaneId &&
			Left.eBaseRadianceChannel == Right.eBaseRadianceChannel &&
			Left.strCoverageLaneId == Right.strCoverageLaneId &&
			Left.eCoverageChannel == Right.eCoverageChannel &&
			Left.eEmissiveMode == Right.eEmissiveMode &&
			Left.eLifetimeEnvelope == Right.eLifetimeEnvelope &&
			Left.eDissolveMode == Right.eDissolveMode &&
			Left.strDissolveLaneId == Right.strDissolveLaneId &&
			Left.eDissolveChannel == Right.eDissolveChannel &&
			Left.fDissolveSoftness == Right.fDissolveSoftness &&
			Left.eMissingLanePolicy == Right.eMissingLanePolicy;
	}

	uint32_t StandardColorChannelMask(
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel)
	{
		switch (eChannel)
		{
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::R:
			return 0x01u;
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::G:
			return 0x02u;
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::B:
			return 0x04u;
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::A:
			return 0x08u;
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::RGB:
			return 0x07u;
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID:
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::END:
		default:
			return 0u;
		}
	}

	uint32_t StandardColorSourceChannelMask(const std::string_view strChannel)
	{
		uint32_t iMask = 0u;
		for (const char_t Character : strChannel)
		{
			switch (Character)
			{
			case 'R': iMask |= 0x01u; break;
			case 'G': iMask |= 0x02u; break;
			case 'B': iMask |= 0x04u; break;
			case 'A': iMask |= 0x08u; break;
			default: return 0u;
			}
		}
		return iMask;
	}

	uint32_t StandardColorSrvChannelMask(const DXGI_FORMAT eFormat)
	{
		switch (eFormat)
		{
		case DXGI_FORMAT_A8_UNORM:
			return 0x08u;

		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 0x01u;

		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
			return 0x03u;

		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
			return 0x07u;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		/* BC1 sampling exposes an alpha component.  DDS occupancy admission
		   decides whether that component carries varying 1-bit coverage; the
		   runtime format mask must not reject an admitted BC1 alpha lane. */
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 0x0fu;

		default:
			return 0u;
		}
	}

	bool_t Is_StandardColorSrgbFormat(const DXGI_FORMAT eFormat)
	{
		return eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC1_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
	}

	bool_t Same_MaterialExecutionResourceSignature(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Left,
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Right)
	{
		return Left.bEnabled == Right.bEnabled &&
			Left.bFailClosed == Right.bFailClosed &&
			Left.bAuthoringApproximate == Right.bAuthoringApproximate &&
			Left.eFidelity == Right.eFidelity &&
			Left.iVersion == Right.iVersion &&
			Left.eBackend == Right.eBackend &&
			Left.iOpcode == Right.iOpcode &&
			Left.iPassIndex == Right.iPassIndex &&
			Left.strRasterizerState == Right.strRasterizerState &&
			Left.strDepthStencilState == Right.strDepthStencilState &&
			Left.strBlendState == Right.strBlendState &&
			Left.iStencilReference == Right.iStencilReference &&
			Left.iTextureLaneCount == Right.iTextureLaneCount &&
			Left.iTextureMask == Right.iTextureMask &&
			Same_StandardColorV1(
				Left.StandardColorV1, Right.StandardColorV1) &&
			Left.iDynamicConsumedMask == Right.iDynamicConsumedMask &&
			Left.iDynamicSuppressedMask == Right.iDynamicSuppressedMask &&
			Left.iParticleColorPolicy == Right.iParticleColorPolicy &&
			Left.iParticleColorConsumedMask ==
				Right.iParticleColorConsumedMask &&
			Left.iParticleColorSuppressedMask ==
				Right.iParticleColorSuppressedMask &&
			Left.iScalarCount == Right.iScalarCount &&
			Left.iVectorCount == Right.iVectorCount &&
			Left.iInputCount == Right.iInputCount &&
			Left.InputConsumedMask == Right.InputConsumedMask &&
			Left.InputSuppressedMask == Right.InputSuppressedMask &&
			Left.VectorComponentConsumedMask ==
				Right.VectorComponentConsumedMask &&
			Left.VectorComponentSuppressedMask ==
				Right.VectorComponentSuppressedMask &&
			Left.iStaticInputCount == Right.iStaticInputCount &&
			Left.iStaticSelectedMask == Right.iStaticSelectedMask &&
			Left.iStaticConsumedMask == Right.iStaticConsumedMask &&
			Left.iStaticSuppressedMask == Right.iStaticSuppressedMask &&
			Left.iRenderInputCount == Right.iRenderInputCount &&
			Left.iRenderConsumedMask == Right.iRenderConsumedMask &&
			Left.iRenderSuppressedMask == Right.iRenderSuppressedMask &&
			Same_MaterialTextureLanes(Left.TextureLanes, Right.TextureLanes) &&
			Same_MaterialScalars(Left.Scalars, Right.Scalars) &&
			Same_MaterialVectors(Left.Vectors, Right.Vectors) &&
			Same_MaterialVectors(
				Left.ArtistParameters, Right.ArtistParameters) &&
			Same_MaterialVectors(Left.Colors, Right.Colors);
	}

	bool_t Same_TypedDynamicParameterResourceSignature(
		const Client::EFFECT_CASCADE_RECIPE_DESC& Left,
		const Client::EFFECT_CASCADE_RECIPE_DESC& Right)
	{
		using DYNAMIC_STRING_LITERAL =
			std::pair<std::string_view, std::string_view>;
		const auto Collect = [](const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe)
		{
			std::vector<DYNAMIC_STRING_LITERAL> Result;
			for (const Client::EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
			{
				if (Module.strClassName != "particlemoduleparameterdynamic")
					continue;
				Result.emplace_back("#module", "");
				for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
					Module.Literals)
				{
					if (Literal.eKind !=
						Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
						continue;
					Result.emplace_back(
						Literal.strPropertyPath, Literal.strString);
				}
			}
			return Result;
		};
		return Collect(Left) == Collect(Right);
	}

	bool_t Resource_SignatureMatches(
		const Client::EFFECT_DOCUMENT_DESC& Left,
		const Client::EFFECT_DOCUMENT_DESC& Right)
	{
		if (Left.Elements.size() != Right.Elements.size() ||
			Left.ModelCues.size() != Right.ModelCues.size())
			return false;
		for (size_t i = 0u; i < Left.ModelCues.size(); ++i)
		{
			const Client::EFFECT_MODEL_CUE_DESC& A = Left.ModelCues[i];
			const Client::EFFECT_MODEL_CUE_DESC& B = Right.ModelCues[i];
			if (A.strCueId != B.strCueId ||
				A.strModelAssetId != B.strModelAssetId ||
				A.strClipName != B.strClipName ||
				A.eAlphaMode != B.eAlphaMode ||
				A.vAssetPreScale.x != B.vAssetPreScale.x ||
				A.vAssetPreScale.y != B.vAssetPreScale.y ||
				A.vAssetPreScale.z != B.vAssetPreScale.z ||
				A.vAssetPreRotationDegrees.x != B.vAssetPreRotationDegrees.x ||
				A.vAssetPreRotationDegrees.y != B.vAssetPreRotationDegrees.y ||
				A.vAssetPreRotationDegrees.z != B.vAssetPreRotationDegrees.z)
			{
				return false;
			}
		}
		for (size_t i = 0u; i < Left.Elements.size(); ++i)
		{
			const Client::EFFECT_ELEMENT_DESC& A = Left.Elements[i];
			const Client::EFFECT_ELEMENT_DESC& B = Right.Elements[i];
			if (A.strElementId != B.strElementId ||
				A.strSourceNode != B.strSourceNode || A.eKind != B.eKind ||
				A.Renderer.eType != B.Renderer.eType ||
				A.Renderer.eSourceSpace != B.Renderer.eSourceSpace ||
				A.SourceRecipe.bEnabled != B.SourceRecipe.bEnabled ||
				A.SourceRecipe.strRendererShape !=
					B.SourceRecipe.strRendererShape ||
				!Same_TypedDynamicParameterResourceSignature(
					A.SourceRecipe, B.SourceRecipe) ||
				A.Material.strTemplateId != B.Material.strTemplateId ||
				A.Material.strSourceMaterialPath !=
					B.Material.strSourceMaterialPath ||
				A.Material.eRenderProfile != B.Material.eRenderProfile ||
				!Same_MaterialExecutionResourceSignature(
					A.Material.Execution, B.Material.Execution) ||
				!Client::Is_EffectSourceMaterialStagingSignatureEqual(
					A.Material.SourceMaterial, B.Material.SourceMaterial) ||
				A.Detail.Mesh.fModelPreScale !=
					B.Detail.Mesh.fModelPreScale ||
				A.ResourceBindings.size() != B.ResourceBindings.size())
				return false;
			for (size_t j = 0u; j < A.ResourceBindings.size(); ++j)
			{
				if (A.ResourceBindings[j].strSlotId != B.ResourceBindings[j].strSlotId ||
					A.ResourceBindings[j].strAssetId != B.ResourceBindings[j].strAssetId)
					return false;
			}
		}
		return true;
	}

	float3_t To_Float3(const vector_t Value)
	{
		float3_t Result{};
		XMStoreFloat3(&Result, Value);
		return Result;
	}

	bool_t Normalize_Safe(const vector_t Value, vector_t& Out)
	{
		const f32_t LengthSquared = XMVectorGetX(XMVector3LengthSq(Value));
		if (!std::isfinite(LengthSquared) || LengthSquared <= 1.e-8f)
			return false;
		Out = XMVector3Normalize(Value);
		return true;
	}

	Client::EFFECT_COLOR_DESC Evaluate_CommonColor(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const f32_t fNormalizedLife)
	{
		Client::EFFECT_COLOR_DESC Color = Element.Detail.Color;
		const Client::EFFECT_LINEAR_LERP_DESC& Lerp =
			Element.Detail.LinearLerp;
		const f32_t T = std::clamp(fNormalizedLife, 0.f, 1.f);
		auto LerpValue = [T](const f32_t A, const f32_t B)
		{
			return A + (B - A) * T;
		};
		if (Lerp.bColorOffset)
		{
			Color.vColorOffset = {
				LerpValue(Color.vColorOffset.x, Lerp.vEndColorOffset.x),
				LerpValue(Color.vColorOffset.y, Lerp.vEndColorOffset.y),
				LerpValue(Color.vColorOffset.z, Lerp.vEndColorOffset.z),
				LerpValue(Color.vColorOffset.w, Lerp.vEndColorOffset.w) };
		}
		if (Lerp.bColorMultiply)
		{
			Color.vColorMultiply = {
				LerpValue(Color.vColorMultiply.x, Lerp.vEndColorMultiply.x),
				LerpValue(Color.vColorMultiply.y, Lerp.vEndColorMultiply.y),
				LerpValue(Color.vColorMultiply.z, Lerp.vEndColorMultiply.z),
				LerpValue(Color.vColorMultiply.w, Lerp.vEndColorMultiply.w) };
		}
		if (Lerp.bEmissiveIntensity)
		{
			Color.fEmissiveIntensity = LerpValue(
				Color.fEmissiveIntensity, Lerp.fEndEmissiveIntensity);
		}
		return Color;
	}

	float4x4_t Make_BillboardWorld(
		const float4x4_t& Source,
		const f32_t fRollDegrees)
	{
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMLoadFloat4x4(&Source)))
		{
			return Source;
		}
		matrix_t CameraWorld = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		float4x4_t Result{};
		XMStoreFloat4x4(&Result,
			XMMatrixScalingFromVector(Scale) *
			XMMatrixRotationZ(XMConvertToRadians(fRollDegrees)) * CameraWorld *
			XMMatrixTranslationFromVector(Translation));
		return Result;
	}

	bool_t Make_ParticleSpriteWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		float4x4_t& OutWorld)
	{
		float4x4_t Source = Particle.World;
		const matrix_t CameraWorldWithTranslation = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		XMStoreFloat4x4(&Source,
			XMLoadFloat4x4(&Source) * XMMatrixTranslationFromVector(
				XMVectorScale(CameraWorldWithTranslation.r[2],
					Particle.fCameraOffset)));
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMLoadFloat4x4(&Source)))
		{
			return false;
		}
		const float3_t DecomposedMagnitude = {
			std::abs(XMVectorGetX(Scale)),
			std::abs(XMVectorGetY(Scale)),
			std::abs(XMVectorGetZ(Scale))
		};
		Client::EFFECT_PARTICLE_SPRITE_SCALE_DESC ResolvedScale;
		if (!Client::CEffectDocumentRenderer::Resolve_ParticleSpriteScale(
			Particle, DecomposedMagnitude, ResolvedScale))
		{
			return false;
		}
		Scale = XMVectorSet(
			ResolvedScale.vScale.x, ResolvedScale.vScale.y,
			ResolvedScale.vScale.z, 0.f);
		matrix_t CameraWorld = CameraWorldWithTranslation;
		CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		matrix_t Orientation = CameraWorld;
		const f32_t fRoll = XMConvertToRadians(
			Particle.fSpriteRotationDegrees);
		switch (Particle.eSpriteAlignment)
		{
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE:
		{
			Orientation = XMMatrixRotationZ(fRoll) * CameraWorld;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_VELOCITY:
		{
			const vector_t Velocity = XMLoadFloat3(&Particle.vWorldVelocity);
			const f32_t fRight = XMVectorGetX(XMVector3Dot(
				Velocity, CameraWorld.r[0]));
			const f32_t fUp = XMVectorGetX(XMVector3Dot(
				Velocity, CameraWorld.r[1]));
			const f32_t fVelocityRoll =
				std::abs(fRight) + std::abs(fUp) > 1.e-6f ?
				std::atan2(fUp, fRight) : 0.f;
			Orientation = XMMatrixRotationZ(
				fVelocityRoll + fRoll) * CameraWorld;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_X:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_X:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(-XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Y:
			/* The shared rect's indexed front face has local normal -Z.  Rotate
			   that normal toward +Y so one-sided EPAL_Z particles face the
			   gameplay camera instead of being deterministically backface-culled. */
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Y:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(-XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Z:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(XM_PI);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Z:
			Orientation = XMMatrixRotationZ(fRoll);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Z:
		{
			const vector_t CameraPosition = XMLoadFloat4x4(
				CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW)).r[3];
			const vector_t Up = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			vector_t Facing = XMVectorSubtract(CameraPosition, Translation);
			Facing = XMVectorSubtract(Facing,
				XMVectorScale(Up, XMVectorGetX(XMVector3Dot(Facing, Up))));
			if (XMVectorGetX(XMVector3LengthSq(Facing)) > 1.e-8f)
			{
				Facing = XMVector3Normalize(Facing);
				const vector_t Right = XMVector3Normalize(
					XMVector3Cross(Up, Facing));
				Orientation = XMMatrixIdentity();
				Orientation.r[0] = Right;
				Orientation.r[1] = Up;
				Orientation.r[2] = Facing;
			}
			Orientation = XMMatrixRotationZ(fRoll) * Orientation;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_X:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Y:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::END:
		default:
			Orientation = XMMatrixRotationZ(fRoll) * CameraWorld;
			break;
		}
		const matrix_t Pivot = XMMatrixTranslation(
			0.5f - Particle.vSpritePivot.x,
			Particle.vSpritePivot.y - 0.5f, 0.f);
		XMStoreFloat4x4(&OutWorld,
			Pivot * XMMatrixScalingFromVector(Scale) * Orientation *
			XMMatrixTranslationFromVector(Translation));
		return true;
	}

	float4x4_t Apply_ParticleCameraOffset(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle)
	{
		if (std::abs(Particle.fCameraOffset) <= 1.e-6f)
			return Particle.World;
		const matrix_t CameraWorld = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		float4x4_t Result{};
		XMStoreFloat4x4(&Result,
			XMLoadFloat4x4(&Particle.World) * XMMatrixTranslationFromVector(
				XMVectorScale(CameraWorld.r[2], Particle.fCameraOffset)));
		return Result;
	}

	f32_t SourceLiteralNumber(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const f32_t fFallback)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
				Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind ==
						Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER)
				{
					return static_cast<f32_t>(Literal.fNumber);
				}
			}
		}
		return fFallback;
	}

	bool_t SourceLiteralBool(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const bool_t bFallback)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
				Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind ==
						Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
				{
					return Literal.bBoolean;
				}
			}
		}
		return bFallback;
	}

	std::string_view SourceLiteralString(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind == Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
					return Literal.strString;
			}
		}
		return {};
	}

	bool_t SourceMaterialIdentityMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strProfileId,
		const std::string_view strParentMaterialPath)
	{
		return Source.strProfileId == strProfileId &&
			Source.strParentMaterialPath == strParentMaterialPath;
	}

	uint32_t SourceMaterialProfileIndex(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		if (!Source.bEnabled ||
			Source.strRuntimeShaderProfileId ==
				"effect.ue3.reconstructed-standard.v1")
			return 0u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.circle.v1")
			return 1u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.dot.v1")
			return 2u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.ring.v1")
			return 3u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.aura.v1")
			return 4u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.one-layer-distortion.v1")
			return 5u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.grouped-translucent.v1")
			return 6u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.shine.v1")
			return 7u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.blackline-aura.v1")
			return 8u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.local-crack.v1")
		{
			if (Source.Textures.empty())
				return 0u;
			return Has_LocalCrackSourceTextureContract(Source) ? 9u :
				UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.procedural-center-glow.v1")
			return 10u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.linearflow-02.v1")
			return 11u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.slice.v1")
			return 12u;
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID)
		{
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.pa."
				"missiletrail.01.tr.9641f8d91e6a",
				"fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr") &&
				Source.StaticSwitches.empty() ? 13u : UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID)
		{
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50",
				"fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source) ?
				14u : UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID)
			return 15u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.glasshole-02.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",
				"fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr") &&
				Client::Has_EffectGlasshole02NamedTextureContract(Source) ?
				29u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.fluidninja-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.fluidninja.01.tr.534340d78128",
				"fx_m_mi_k_00.fx_m.fx_k_pa_fluidninja_01_tr") &&
				Client::Has_EffectFluidNinja01NamedTextureContract(Source) ?
				30u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.customparticle-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.customparticle.01.ad.e6b959010967",
				"fx_m_mi_j_00.fx_m.fx_j_pa_customparticle_01_ad") &&
				Client::Has_EffectCustomParticle01NamedTextureContract(Source) ?
				31u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.crackholev2-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.k.00.fx.m.fx.k.crackholev2.01.3aac97e0fcad",
				"fx_m_mi_k_00.fx_m.fx_k_crackholev2_01") &&
				Client::Has_EffectCrackholeV2NamedTextureContract(Source) ?
				32u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.simple-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",
				"fx_mastermaterial.fx_mm.fx_mm_simple_01_ad") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source) ?
				33u : UINT32_MAX;
		return UINT32_MAX;
	}

	bool_t BindingMatches(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_RESOURCE_SLOT eSlot,
		const std::string_view strAssetId)
	{
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBinding =
			Find_Binding(Element, eSlot);
		return nullptr != pBinding && pBinding->strAssetId == strAssetId;
	}

	bool_t NamedTextureMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const std::string_view strAssetId)
	{
		return std::ranges::count_if(Source.Textures,
			[strName, strAssetId](
				const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == strName &&
					Texture.strAssetId == strAssetId;
			}) == 1;
	}

	bool_t Is_DimensionMasterDBoundarySpriteWaveContract(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::Resolve_EffectPortableOriginElementId(Element) ==
				"authored.source-particle.03ae2d86558a1627f9d867e7" &&
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.SourceRecipe.strRendererShape == "sprite" &&
			Element.ResourceBindings.size() == 3u &&
			SourceMaterialIdentityMatches(Element.Material.SourceMaterial,
				"ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",
				"fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr") &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_spritewave_01_106_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_m_tilelinenoise_01.dds") &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_w_atypical_016_xcl.dds") &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_001.dds");
	}

	bool_t Is_DimensionMasterDBoundaryParticleMasterContract(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const std::string_view OriginElementId =
			Client::Resolve_EffectPortableOriginElementId(Element);
		const bool_t bExactDimensionMasterDOccurrence =
			OriginElementId ==
				"authored.source-particle.20e58ca3740942649576b818" ||
			OriginElementId ==
				"authored.source-particle.cd12d28ee975a182b849dae0" ||
			OriginElementId ==
				"authored.source-particle.1d400b300d15f98b78e45a92";
		if (!bExactDimensionMasterDOccurrence ||
			Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != "sprite" ||
			Element.ResourceBindings.size() != 4u ||
			!SourceMaterialIdentityMatches(Element.Material.SourceMaterial,
				"ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",
				"fx_m_mi_00.fx_m.fx_d_pa_master_01_tr") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_noise_03.dds") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_009.dds") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_environment_tile_02.dds"))
		{
			return false;
		}
		const bool_t bMaster04 = Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_master_01_04_dt_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_flowsmoke_01_cl.dds");
		const bool_t bMaster05 = Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_master_01_05_dt_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_021.dds");
		return bMaster04 || bMaster05;
	}

	bool_t Is_FamilyProfileCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile)
	{
		if (Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != "sprite")
		{
			return false;
		}
		if (29u == iProfile || 30u == iProfile)
		{
			return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		}
		if (31u == iProfile)
		{
			return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ;
		}
		if (33u == iProfile)
		{
			/* This family is authored both one- and two-sided across the corpus
			   (279 / 89 of 368). Sidedness is raster state, not a different
			   formula, so both carriers resolve to the same profile. */
			return Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::
						ADDITIVE_ONE_SIDED_DEPTH_READ ||
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::
						ADDITIVE_TWO_SIDED_DEPTH_READ;
		}
		return 32u == iProfile && Element.Material.eRenderProfile ==
			Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ;
	}

	bool_t Is_StrictParticleShapeCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strRequiredShape)
	{
		if (Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != strRequiredShape)
		{
			return false;
		}
		const Client::EFFECT_RESOURCE_BINDING_DESC* pMesh =
			Find_Binding(Element, Client::EFFECT_RESOURCE_SLOT::MESH_MODEL);
		return strRequiredShape == "mesh" ?
			(nullptr != pMesh && !pMesh->strAssetId.empty()) : nullptr == pMesh;
	}

	bool_t Is_StrictParticleBlendCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bAdditive)
	{
		if (!Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
			!Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh"))
		{
			return false;
		}
		if (bAdditive)
		{
			return Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ ||
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		}
		return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	}

	bool_t Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	}

	bool_t Is_MissileTrailFourLaneCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source =
			Element.Material.SourceMaterial;
		if (!Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") ||
			Element.Material.eRenderProfile !=
				Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
			Element.ResourceBindings.size() != 5u ||
			!SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.pa."
				"missiletrail.01.tr.9641f8d91e6a",
				"fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr") ||
			!Source.StaticSwitches.empty())
		{
			return false;
		}
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pMask = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pNoise = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pDissolve = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE);
		if (nullptr == pBase || pBase->strAssetId.empty() || nullptr == pMask ||
			pMask->strAssetId.empty() || nullptr == pNoise ||
			pNoise->strAssetId.empty() || nullptr == pDissolve ||
			pDissolve->strAssetId.empty() || nullptr != Find_Binding(
				Element, Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE))
		{
			return false;
		}
		/* Older sealed programs can lack named texture provenance. When names
		   are present, bind every executable lane back to the same typed
		   resource and require the one unsampled dependency receipt. Duplicate
		   aliases must not be admitted by a first-match lookup. */
		if (Source.Textures.empty())
			return true;
		static constexpr std::array<std::string_view, 3u> CONSUMED_NAMES = {{
			"alpha_tex", "uv_noise_tex", "uv_dissolve_tex"
		}};
		const size_t iDependencyCount = std::ranges::count_if(
			Source.Textures,
			[](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == "umodel_dependency" &&
					Texture.strAssetId.empty();
			});
		return Source.Textures.size() == 4u && iDependencyCount == 1u &&
			Client::Has_EffectUniqueNamedTextureContract(
				Source, CONSUMED_NAMES) &&
			NamedTextureMatches(Source, "alpha_tex", pMask->strAssetId) &&
			NamedTextureMatches(Source, "uv_noise_tex", pNoise->strAssetId) &&
			NamedTextureMatches(Source, "uv_dissolve_tex",
				pDissolve->strAssetId);
	}

	uint32_t EffectiveSourceMaterialProfileIndex(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source =
			Element.Material.SourceMaterial;
		const uint32_t iStoredProfile = SourceMaterialProfileIndex(Source);
		if (13u == iStoredProfile)
		{
			return Is_MissileTrailFourLaneCarrierContractSatisfied(Element) ?
				13u : UINT32_MAX;
		}
		if (14u == iStoredProfile)
		{
			return Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source) ?
				14u : UINT32_MAX;
		}
		if (iStoredProfile >= 29u && iStoredProfile <= 33u)
		{
			if (33u == iStoredProfile &&
				!Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite"))
			{
				return UINT32_MAX;
			}
			return Is_FamilyProfileCarrierContractSatisfied(
				Element, iStoredProfile) ? iStoredProfile : UINT32_MAX;
		}
		if (6u == iStoredProfile &&
			Client::Resolve_EffectStrictTypedSourceProfile(
				Element.Material.strSourceMaterialPath, Source) ==
				Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01)
		{
			return Client::Has_EffectFlowRibbon01TrailContract(Element) ?
				35u : UINT32_MAX;
		}
		if (6u != iStoredProfile ||
			Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			(Element.SourceRecipe.strRendererShape != "mesh" &&
			 Element.SourceRecipe.strRendererShape != "sprite"))
		{
			return iStoredProfile;
		}
		if (Is_DimensionMasterDBoundarySpriteWaveContract(Element) ||
			Is_DimensionMasterDBoundaryParticleMasterContract(Element))
		{
			/* These exact DimensionMaster-D cards lost only their named texture
			   lanes during Effect Tool compaction. Their opaque DDS alpha cannot
			   define coverage, so reuse the already bounded Slice evaluator used
			   by the W Voronoi card instead of exposing grouped profile 6. */
			return 12u;
		}

		switch (Client::Resolve_EffectStrictTypedSourceProfile(
			Element.Material.strSourceMaterialPath, Source))
		{
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MISSILETRAIL:
			if (BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL,
					"Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_trail_007.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_noise_003.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_noise_001.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_d_noise_030.dds") &&
				NamedTextureMatches(Source, "alpha_tex",
					"Effect/LanceMaster/Textures/fx_m_trail_007.dds") &&
				NamedTextureMatches(Source, "emissive_tex01",
					"Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds") &&
				NamedTextureMatches(Source, "emissive_tex02",
					"Effect/LanceMaster/Textures/fx_m_noise_003.dds") &&
				NamedTextureMatches(Source, "uv_dissolve_tex",
					"Effect/LanceMaster/Textures/fx_m_noise_001.dds") &&
				NamedTextureMatches(Source, "uv_noise_tex",
					"Effect/LanceMaster/Textures/fx_d_noise_030.dds"))
			{
				return 15u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::WATERTRAIL:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source))
			{
				return 14u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::LINEARFLOW_02:
			if (Element.SourceRecipe.strRendererShape == "mesh" &&
				Element.ResourceBindings.size() == 4u &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL,
					"Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_mirnoise_02.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_auraline_19_ycl.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds"))
			{
				return 11u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_02:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlowMeshNamedTextureContract(Source))
			{
				return 36u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlowMeshNamedTextureContract(Source))
			{
				return 16u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03_SPRITE:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlow03SpriteNamedTextureContract(Source))
			{
				return 37u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::RING_01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 1u &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_002.dds"))
			{
				return 17u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 3u)
			{
				const bool_t bTrail03 =
					Element.Material.strSourceMaterialPath ==
						"fx_m_mi_s_00.fx_mi.fx_s_pa_trail_03_01_tr" &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_007.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_007.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_004.dds");
				const bool_t bTrail01 =
					Element.Material.strSourceMaterialPath ==
						"fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_6_tr" &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_005.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_005.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds");
				if (bTrail03 || bTrail01)
					return 18u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01:
			if (Source.StaticSwitches.empty() &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element,
					Source.strParentMaterialPath.ends_with("_ad")) &&
				Client::Has_EffectParticleMasterNamedTextureContract(Source))
				return 19u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SPRITEWAVE_01:
			if (Source.StaticSwitches.empty() &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element,
					Source.strParentMaterialPath.ends_with("_ad")) &&
				Client::Has_EffectSpriteWaveNamedTextureContract(Source))
				return 20u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_SINGLE_ALPHA:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectParticleTrailSingleAlphaNamedTextureContract(Source))
			{
				return 21u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistSpla01NamedTextureContract(Source))
				return 22u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA05:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistSpla05NamedTextureContract(Source))
				return 23u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_TWINKLE:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistTwinkleNamedTextureContract(Source))
				return 24u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_FLUID01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistFluid01NamedTextureContract(Source))
				return 25u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_WORLDOFFSET01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistWorldOffset01NamedTextureContract(Source))
				return 26u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MAKEFLOW01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistMakeFlow01NamedTextureContract(Source))
				return 27u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_LENSFLARE01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistLensFlare01NamedTextureContract(Source))
				return 28u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MM_FLUID01:
			/* The source family spans mesh and sprite carriers with four role
			   sets. Admit only the exact two-lane sprite child used by the two
			   DimensionMaster F occurrences; the mesh variants remain on their
			   existing fail-closed/grouped boundary. */
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 2u &&
				Element.Material.strSourceMaterialPath ==
					"fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr" &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_cloud_035.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_o_glass_01.dds"))
			{
				return 34u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01:
			/* Trail carriers are resolved before the particle-only grouped branch. */
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE01:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element, true) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source))
			{
				return 33u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE02:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple02NamedTextureContract(Source))
			{
				return 38u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::GLASSHOLE02:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 29u) &&
				Client::Has_EffectGlasshole02NamedTextureContract(Source) ?
				29u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLUIDNINJA01:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 30u) &&
				Client::Has_EffectFluidNinja01NamedTextureContract(Source) ?
				30u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::CUSTOMPARTICLE01:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 31u) &&
				Client::Has_EffectCustomParticle01NamedTextureContract(Source) ?
				31u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::CRACKHOLEV2:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 32u) &&
				Client::Has_EffectCrackholeV2NamedTextureContract(Source) ?
				32u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_BASIC01:
			/* The corpus authors this family on both sprite and mesh particle
			   carriers and in additive and alpha blends.  Carrier geometry and
			   blend are render state; the equation is shared, so both admit. */
			if ((Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "sprite") ||
				Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "mesh")) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMmBasic01NamedTextureContract(Source))
			{
				return 39u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MESH_MASKED_CHAIN01:
			/* The parent owns no texture parameter, so the admission gate is the
			   mesh carrier plus the base binding the converter attached. */
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") &&
				Source.StaticSwitches.empty() &&
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE))
			{
				return 41u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_LIGHT01:
			/* Reuses the simple_01 evaluator and its lane staging.  With no
			   scalar authored, every simple_01 input falls back to neutral and
			   the equation reduces to the single emissive sample this family
			   owns.  Both blends admit because sidedness and blend are raster
			   state, not a different formula. */
			if (Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "sprite") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source))
			{
				return 33u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWTRAIL01:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectFlowTrail01NamedTextureContract(Source))
			{
				return 40u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE:
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::END:
		default:
			break;
		}
		return iStoredProfile;
	}

	uint32_t DynamicParameterSemanticIndex(const std::string& strSemantic)
	{
		if (strSemantic == "opacity") return 1u;
		if (strSemantic == "emissive") return 2u;
		if (strSemantic == "dissolve") return 3u;
		if (strSemantic == "uv_pan") return 4u;
		if (strSemantic == "distortion") return 5u;
		if (strSemantic == "radial_size") return 6u;
		if (strSemantic == "mask_a_offset") return 7u;
		if (strSemantic == "mask_b_offset") return 8u;
		if (strSemantic == "mask_a_distort") return 9u;
		if (strSemantic == "mask_b_distort") return 10u;
		if (strSemantic == "mask_a_pan") return 11u;
		if (strSemantic == "flow_strength") return 12u;
		if (strSemantic == "mask_b_pan") return 13u;
		if (strSemantic == "diffuse_pan") return 14u;
		if (strSemantic == "missile_alpha_pan") return 15u;
		if (strSemantic == "missile_noise_strength") return 16u;
		if (strSemantic == "missile_noise_pan") return 17u;
		if (strSemantic == "missile_dissolve") return 18u;
		if (strSemantic == "water_alpha_pan") return 19u;
		if (strSemantic == "water_noise_pan") return 20u;
		if (strSemantic == "water_dissolve") return 21u;
		if (strSemantic == "water_noise_strength") return 22u;
		return 0u;
	}

	uint32_t TypedDynamicParameterSemanticIndex(
		const uint32_t iProfile,
		const std::string_view strParameterName)
	{
		if (13u == iProfile || 15u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 15u;
			if (strParameterName == "uv_noise_velue" ||
				strParameterName == "uv_noise_value" ||
				strParameterName == "noise_velue") return 16u;
			if (strParameterName == "uv_noise_pan") return 17u;
			if (strParameterName == "alpha_dissolve" ||
				strParameterName == "dissolve") return 18u;
		}
		else if (14u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 19u;
			if (strParameterName == "uv_noise_pan") return 20u;
			if (strParameterName == "dissolve" ||
				strParameterName == "alpha_dissolve") return 21u;
			if (strParameterName == "noise_velue" ||
				strParameterName == "uv_noise_velue" ||
				strParameterName == "uv_noise_value") return 22u;
		}
		else if (11u == iProfile)
		{
			if (strParameterName == "mask_a_offset") return 7u;
			if (strParameterName == "mask_b_offset") return 8u;
			if (strParameterName == "mask_a_distort") return 9u;
			if (strParameterName == "mask_b_distort") return 10u;
		}
		else if (18u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 23u;
			if (strParameterName == "lerp") return 24u;
			if (strParameterName == "noise_pan") return 25u;
			if (strParameterName == "noise_velue") return 26u;
		}
		else if (16u == iProfile || 36u == iProfile)
		{
			if (strParameterName == "flow_str") return 27u;
			if (strParameterName == "opacity_pan_v") return 28u;
			if (strParameterName == "opacity_pan_u") return 29u;
			if (strParameterName == "param4") return 30u;
		}
		else if (19u == iProfile)
		{
			if (strParameterName == "alphadissolve[0-1]") return 31u;
			if (strParameterName == "pan[0-2]") return 32u;
			if (strParameterName == "edgestr[0-x]") return 33u;
			if (strParameterName == "disrotion[0-x]") return 34u;
		}
		else if (20u == iProfile)
		{
			if (strParameterName == "maintex_tile_pan") return 35u;
			if (strParameterName == "dissolve") return 36u;
			if (strParameterName == "uv_noisevelue") return 37u;
			if (strParameterName == "uv_sphery_uv_noisepan") return 38u;
		}
		else if (21u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 23u;
			if (strParameterName == "lerp") return 24u;
			if (strParameterName == "noise_pan") return 25u;
			if (strParameterName == "noise_velue") return 26u;
		}
		else if (23u == iProfile)
		{
			if (strParameterName == "_") return 47u;
			if (strParameterName == "dissolve[0-1]") return 39u;
			if (strParameterName == "uvdistort[0-1]") return 40u;
			if (strParameterName == "distortion[0-x]") return 41u;
		}
		else if (24u == iProfile)
		{
			if (strParameterName == "dissolve_density(0~1)") return 42u;
			if (strParameterName == "alpha_power(1~)") return 43u;
			if (strParameterName == "emissive_tiling(0.5~2)") return 44u;
			if (strParameterName == "lamp_time(0~1)") return 45u;
		}
		else if (26u == iProfile)
		{
			if (strParameterName == "dissolve") return 46u;
			if (strParameterName == "param2" ||
				strParameterName == "param3" ||
				strParameterName == "param4") return 47u;
		}
		else if (34u == iProfile)
		{
			if (strParameterName == "trasition_speed(0~1.5)") return 48u;
			if (strParameterName == "alpha_power(1~)") return 49u;
			if (strParameterName == "fresnel_alpha(1~)") return 50u;
			if (strParameterName == "param4") return 51u;
		}
		return 0u;
	}

	bool_t Try_ResolveTypedDynamicParameterSemantics(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile,
		std::array<uint32_t, 4u>& OutSemantics)
	{
		if (13u != iProfile && 15u != iProfile && 14u != iProfile &&
			16u != iProfile && 18u != iProfile && 19u != iProfile &&
			20u != iProfile && 21u != iProfile && 23u != iProfile &&
			24u != iProfile && 26u != iProfile && 34u != iProfile)
			return false;
		std::array<uint32_t, 4u> Staged{};
		std::array<bool_t, 4u> Seen{};
		uint32_t iDynamicModuleCount = 0u;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			++iDynamicModuleCount;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.eKind != Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
					continue;
				for (size_t iChannel = 0u; iChannel < Staged.size(); ++iChannel)
				{
					const std::string strExpected = "dynamicparams[" +
						std::to_string(iChannel) + "].paramname";
					if (Literal.strPropertyPath != strExpected)
						continue;
					if (Seen[iChannel])
						return false;
					Staged[iChannel] = TypedDynamicParameterSemanticIndex(
						iProfile, Literal.strString);
					Seen[iChannel] = true;
				}
			}
		}
		if (1u != iDynamicModuleCount ||
			!std::ranges::all_of(Seen, [](const bool_t bSeen) { return bSeen; }) ||
			!std::ranges::all_of(
				Staged, [](const uint32_t iSemantic) { return 0u != iSemantic; }))
		{
			return false;
		}
		OutSemantics = Staged;
		return true;
	}

	Client::EFFECT_SUBUV_FRAME_DESC Resolve_SubUVFrames(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle)
	{
		if (nullptr == Particle.pElement)
		{
			return {};
		}
		if (!Particle.pElement->SourceRecipe.bEnabled)
		{
			const Client::EFFECT_ELEMENT_DESC& Element = *Particle.pElement;
			const Client::EFFECT_UV_DESC& UV = Element.Detail.UV;
			if (!Element.Material.Execution.bEnabled ||
				UV.iTileColumns <= 0 || UV.iTileRows <= 0)
			{
				return {};
			}
			const uint32_t iColumns = static_cast<uint32_t>(UV.iTileColumns);
			const uint32_t iRows = static_cast<uint32_t>(UV.iTileRows);
			const uint64_t iFrameCount64 =
				static_cast<uint64_t>(iColumns) * iRows;
			if (iFrameCount64 <= 1u || iFrameCount64 > UINT32_MAX)
				return {};
			const uint32_t iFrameCount = static_cast<uint32_t>(iFrameCount64);

			/* Authored Track A copies intentionally do not retain the source
			   module graph.  Their typed material still requires the particle VF
			   to provide atlas coordinates, so derive a deterministic current/next
			   frame from the editable generic UV atlas and particle lifetime.  The
			   typed opcodes consume the current frame only; keeping blend at zero
			   also preserves active030's admitted packet contract. */
			const f32_t fLife = std::clamp(
				Particle.fNormalizedLife, 0.f, 1.f);
			f32_t fFrame = static_cast<f32_t>(UV.iTileIndex) +
				fLife * static_cast<f32_t>(
					UV.bLoop ? iFrameCount : iFrameCount - 1u);
			if (!UV.bLoop)
			{
				fFrame = (std::min)(fFrame,
					static_cast<f32_t>(iFrameCount - 1u));
			}
			return Client::CEffectPlayback::Resolve_SourceSubUVFrame(
				iColumns, iRows, fFrame, false, false,
				Particle.fDistributionRandom, false);
		}
		/* SubUV interpolation is a particle-VF property carried by Required,
		   not a material-backend property. Portable authored Track A recipes
		   intentionally clear native SourceMaterial after lowering to a typed
		   execution packet, so requiring that profile here discarded otherwise
		   complete 6x6/2x2 frame programs. */
		const std::string_view strSubUVMode = SourceLiteralString(
			*Particle.pElement, "interpolationmethod");
		const uint32_t iColumns = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(*Particle.pElement,
				"subimages_horizontal", 1.f)));
		const uint32_t iRows = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(*Particle.pElement,
				"subimages_vertical", 1.f)));
		const uint64_t iFrameCount =
			static_cast<uint64_t>(iColumns) * static_cast<uint64_t>(iRows);
		if (iFrameCount <= 1u || iFrameCount > UINT32_MAX ||
			strSubUVMode.empty() || strSubUVMode == "none" ||
			strSubUVMode == "psuvim_none")
		{
			return {};
		}
		/* The evaluated signed Size is the sole image-flip authority.  Build an
		   unmirrored atlas frame here; the shared Playback helper applies those
		   signs once after billboard geometry uses positive extents. */
		return Client::CEffectPlayback::Resolve_SourceSubUVFrame(
			iColumns, iRows, Particle.fSubImageIndex,
			false, false, Particle.fDistributionRandom,
			Client::Is_EffectSourceLinearBlendSubUVMode(
				strSubUVMode));
	}
}

struct Client::CEffectDocumentRenderer::PREWARM_ASSET_CACHE final
{
	std::unordered_map<std::string, std::shared_ptr<Engine::CModel>>
		NonAnimatedModels;
	std::unordered_map<std::string, std::shared_ptr<Engine::CModel>>
		AnimatedModelPrototypes;
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> Textures;
};

struct Client::CEffectDocumentRenderer::PREPARED_DOCUMENT final
{
	uint64_t iCatalogRevision = 0u;
	uint64_t iMaterialProgramRegistryGeneration = 0u;
	uint64_t iResourceSignature = 0u;
	std::string strEffectAssetId;
	const EFFECT_DOCUMENT_DESC* pCatalogDocumentIdentity = nullptr;
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument;
	EFFECT_DOCUMENT_DESC ResourceDocument;
	std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pReconstructedRuntimePreparation;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
	uint32_t iReconstructedNeutralBaseCount = 0u;
	uint32_t iReconstructedOneLayerCount = 0u;
	uint32_t iReconstructedMaterialEvaluatorCount = 0u;
	uint32_t iRuntimeMaterialV2Count = 0u;
	uint32_t iVisualProgramAdapterCount = 0u;
	uint32_t iArtistVisualV4Count = 0u;
	uint32_t iArtistVisualV4UnsupportedCount = 0u;
	uint32_t iLegacyOccurrenceVisualSuppressedCount = 0u;
	uint32_t iMaterialProgramResolvedElementCount = 0u;
	std::unordered_map<std::string, ELEMENT_RESOURCE> ElementResources;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		ModelCuePrototypes;
	std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pTrailBuffer;
};

const Client::EFFECT_DOCUMENT_DESC&
Client::CEffectDocumentRenderer::Get_StagedDocument() const
{
	return nullptr != m_pPreparedDocument &&
		nullptr != m_pPreparedDocument->pImmutableDocument ?
		*m_pPreparedDocument->pImmutableDocument : m_Document;
}

struct Client::CEffectDocumentRenderer::PRODUCT_PREWARM_SESSION final
{
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	uint64_t iCatalogRevision = 0u;
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry;
	PREWARM_ASSET_CACHE SharedAssets;
};

struct Client::CEffectDocumentRenderer::RECONSTRUCTED_DIAGNOSTIC_COMPOSITE final
{
	struct GPU_RESOURCE final
	{
		std::shared_ptr<Engine::CModel> pModel;
		std::array<ComPtr<ID3D11ShaderResourceView>, 2u> Textures;
		std::array<ComPtr<ID3D11SamplerState>, 2u> Samplers;
		ComPtr<ID3D11BlendState> pBlendState;
		ComPtr<ID3D11RasterizerState> pRasterizerState;
		ComPtr<ID3D11DepthStencilState> pDepthStencilState;
		D3D11_BLEND_DESC BlendDescriptor{};
		D3D11_RASTERIZER_DESC RasterizerDescriptor{};
		D3D11_DEPTH_STENCIL_DESC DepthStencilDescriptor{};
		std::array<D3D11_SAMPLER_DESC, 2u> SamplerDescriptors{};
		bool_t bHasBlendDescriptor = false;
		bool_t bHasRasterizerDescriptor = false;
		bool_t bHasDepthStencilDescriptor = false;
		ComPtr<ID3D11Query> pPipelineStatisticsQuery;
		bool_t bPipelineStatisticsPending = false;
		uint64_t iDrawCount = 0u;
		D3D11_QUERY_DATA_PIPELINE_STATISTICS PipelineStatistics{};
	};

	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame;
	std::array<GPU_RESOURCE, 2u> Resources;
};

namespace
{
	struct EFFECT_RENDERER_CORE final
	{
		shared_ptr<Engine::CShader> pMeshShader;
		shared_ptr<Engine::CShader> pAnimatedModelShader;
		shared_ptr<Engine::CShader> pRectShader;
		shared_ptr<Engine::CShader> pParticleShader;
		shared_ptr<Engine::CShader> pTrailShader;
		shared_ptr<Engine::CShader> pDecalShader;
		shared_ptr<Engine::CVIBuffer_Rect> pRect;
		shared_ptr<Engine::CVIBuffer_ParticleRect> pParticleBuffer;
		ComPtr<ID3D11ShaderResourceView> pWhiteTexture;
		ComPtr<ID3D11ShaderResourceView> pBlackTexture;
	};

	struct PREPARED_KEY final
	{
		uint64_t iCatalogRevision = 0u;
		std::string strEffectAssetId;
		uint64_t iResourceSignature = 0u;
		std::string strVisualProgramTokenSha256;

		bool operator<(const PREPARED_KEY& Right) const
		{
			return std::tie(iCatalogRevision, strEffectAssetId,
				iResourceSignature, strVisualProgramTokenSha256) <
				std::tie(Right.iCatalogRevision, Right.strEffectAssetId,
					Right.iResourceSignature,
					Right.strVisualProgramTokenSha256);
		}
	};

	std::mutex g_EffectRenderCacheMutex;
	std::unordered_map<ID3D11Device*, std::shared_ptr<EFFECT_RENDERER_CORE>>
		g_EffectRendererCores;
	std::map<PREPARED_KEY,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocuments;
	std::unordered_map<const Client::EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocumentsByIdentity;
	std::shared_ptr<Client::CEffectDocumentRenderer::PRODUCT_PREWARM_SESSION>
		g_pProductPrewarmSession;
	ID3D11Device* g_pPreparedDevice = nullptr;
	uint64_t g_iPreparedCatalogRevision = 0u;
	uint64_t g_iPreparedCatalogGeneration = 0u;
	Client::EFFECT_RENDER_PREWARM_PROBE g_EffectRenderPrewarmProbe;

	bool_t Read_ReconstructedAssetBytes(
		const std::string& strAssetId,
		const uint64_t iExpectedByteCount,
		const std::string& strExpectedSha256,
		std::filesystem::path& OutPath,
		std::vector<uint8_t>& OutBytes,
		std::string& strOutError)
	{
		const std::filesystem::path Path =
			Client::CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
		{
			strOutError = "Reconstructed diagnostic asset is missing: " +
				strAssetId;
			return false;
		}
		std::ifstream Input(Path, std::ios::binary | std::ios::ate);
		if (!Input)
		{
			strOutError = "Reconstructed diagnostic asset could not be opened: " +
				strAssetId;
			return false;
		}
		const std::streamoff Size = Input.tellg();
		if (Size < 0 || static_cast<uint64_t>(Size) != iExpectedByteCount)
		{
			strOutError = "Reconstructed diagnostic asset byte count changed: " +
				strAssetId;
			return false;
		}
		std::vector<uint8_t> Bytes(static_cast<size_t>(Size));
		Input.seekg(0, std::ios::beg);
		if (!Bytes.empty() && !Input.read(
			reinterpret_cast<char*>(Bytes.data()), Size))
		{
			strOutError = "Reconstructed diagnostic asset read failed: " +
				strAssetId;
			return false;
		}
		const std::string_view ByteView(
			reinterpret_cast<const char*>(Bytes.data()), Bytes.size());
		if (Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(ByteView) !=
			strExpectedSha256)
		{
			strOutError = "Reconstructed diagnostic asset SHA-256 changed: " +
				strAssetId;
			return false;
		}
		OutPath = Path;
		OutBytes = std::move(Bytes);
		return true;
	}

	std::string Sha256Hex(const std::array<uint8_t, 32u>& Bytes)
	{
		static constexpr char Hex[] = "0123456789abcdef";
		std::string Result(Bytes.size() * 2u, '0');
		for (size_t Index = 0u; Index < Bytes.size(); ++Index)
		{
			Result[Index * 2u] = Hex[Bytes[Index] >> 4u];
			Result[Index * 2u + 1u] = Hex[Bytes[Index] & 0x0fu];
		}
		return Result;
	}

	bool_t Same_SamplerDescriptor(
		const D3D11_SAMPLER_DESC& Left,
		const D3D11_SAMPLER_DESC& Right)
	{
		return Left.Filter == Right.Filter &&
			Left.AddressU == Right.AddressU &&
			Left.AddressV == Right.AddressV &&
			Left.AddressW == Right.AddressW &&
			Left.MipLODBias == Right.MipLODBias &&
			Left.MaxAnisotropy == Right.MaxAnisotropy &&
			Left.ComparisonFunc == Right.ComparisonFunc &&
			std::equal(std::begin(Left.BorderColor), std::end(Left.BorderColor),
				std::begin(Right.BorderColor)) &&
			Left.MinLOD == Right.MinLOD && Left.MaxLOD == Right.MaxLOD;
	}

	bool_t Is_AnisotropicSamplerFilter(const D3D11_FILTER Filter)
	{
		return Filter == D3D11_FILTER_ANISOTROPIC ||
			Filter == D3D11_FILTER_COMPARISON_ANISOTROPIC ||
			Filter == D3D11_FILTER_MINIMUM_ANISOTROPIC ||
			Filter == D3D11_FILTER_MAXIMUM_ANISOTROPIC;
	}

	bool_t Same_RuntimeSamplerReadbackDescriptor(
		const D3D11_SAMPLER_DESC& Actual,
		const D3D11_SAMPLER_DESC& Requested,
		const D3D11_SAMPLER_DESC& FrozenAuthority)
	{
		D3D11_SAMPLER_DESC CanonicalRequested = Requested;
		if (!Is_AnisotropicSamplerFilter(FrozenAuthority.Filter) &&
			0u == FrozenAuthority.MaxAnisotropy &&
			1u == Requested.MaxAnisotropy)
		{
			// D3D11 requires a legal creation value even though this field is
			// ignored for non-anisotropic filters.  GetDesc canonicalizes that
			// materialized value back to the frozen semantic zero.
			CanonicalRequested.MaxAnisotropy = 0u;
		}
		return Same_SamplerDescriptor(Actual, CanonicalRequested);
	}

	bool_t Materialize_RuntimeSamplerDescriptor(
		const D3D11_SAMPLER_DESC& Authority,
		D3D11_SAMPLER_DESC& OutRuntime)
	{
		OutRuntime = Authority;
		const bool_t bAnisotropic = Is_AnisotropicSamplerFilter(Authority.Filter);
		if (0u == OutRuntime.MaxAnisotropy)
		{
			// The frozen UE3 sampler evidence records zero when anisotropy is
			// disabled.  D3D11 still requires the otherwise ignored field to be
			// in [1, 16], so materialize the unique non-anisotropic identity.
			if (bAnisotropic)
				return false;
			OutRuntime.MaxAnisotropy = 1u;
		}
		return OutRuntime.MaxAnisotropy <= 16u;
	}

	bool_t Try_ToAuthoredFilter(
		const D3D11_FILTER Source,
		Client::EFFECT_MATERIAL_TEXTURE_FILTER& Out)
	{
		switch (Source)
		{
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT;
			return true;
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR;
			return true;
		case D3D11_FILTER_ANISOTROPIC:
			Out = Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC;
			return true;
		default:
			return false;
		}
	}

	bool_t Try_ToD3dFilter(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER Source,
		D3D11_FILTER& Out)
	{
		switch (Source)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT:
			Out = D3D11_FILTER_MIN_MAG_MIP_POINT;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR:
			Out = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC:
			Out = D3D11_FILTER_ANISOTROPIC;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::END:
		default:
			return false;
		}
	}

	bool_t Try_ToAuthoredAddress(
		const D3D11_TEXTURE_ADDRESS_MODE Source,
		Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE& Out)
	{
		switch (Source)
		{
		case D3D11_TEXTURE_ADDRESS_WRAP:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP;
			return true;
		case D3D11_TEXTURE_ADDRESS_MIRROR:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::MIRROR;
			return true;
		case D3D11_TEXTURE_ADDRESS_CLAMP:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
			return true;
		case D3D11_TEXTURE_ADDRESS_BORDER:
			Out = Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::BORDER;
			return true;
		default:
			return false;
		}
	}

	bool_t Try_ToD3dAddress(
		const Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE Source,
		D3D11_TEXTURE_ADDRESS_MODE& Out)
	{
		switch (Source)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP:
			Out = D3D11_TEXTURE_ADDRESS_WRAP;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::MIRROR:
			Out = D3D11_TEXTURE_ADDRESS_MIRROR;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP:
			Out = D3D11_TEXTURE_ADDRESS_CLAMP;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::BORDER:
			Out = D3D11_TEXTURE_ADDRESS_BORDER;
			return true;
		case Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END:
		default:
			return false;
		}
	}

	bool_t Try_ToAuthoredComparison(
		const D3D11_COMPARISON_FUNC Source,
		Client::EFFECT_MATERIAL_COMPARISON_FUNCTION& Out)
	{
		if (Source < D3D11_COMPARISON_NEVER ||
			Source > D3D11_COMPARISON_ALWAYS)
		{
			return false;
		}
		Out = static_cast<Client::EFFECT_MATERIAL_COMPARISON_FUNCTION>(
			static_cast<uint32_t>(Source) - 1u);
		return true;
	}

	bool_t Try_ToD3dComparison(
		const Client::EFFECT_MATERIAL_COMPARISON_FUNCTION Source,
		D3D11_COMPARISON_FUNC& Out)
	{
		if (Source >= Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::END)
			return false;
		Out = static_cast<D3D11_COMPARISON_FUNC>(
			static_cast<uint32_t>(Source) + 1u);
		return true;
	}

	bool_t Try_ToAuthoredSampler(
		const D3D11_SAMPLER_DESC& Source,
		Client::EFFECT_MATERIAL_SAMPLER_DESC& Out)
	{
		Client::EFFECT_MATERIAL_SAMPLER_DESC Staged;
		if (!Try_ToAuthoredFilter(Source.Filter, Staged.eFilter) ||
			!Try_ToAuthoredAddress(Source.AddressU, Staged.eAddressU) ||
			!Try_ToAuthoredAddress(Source.AddressV, Staged.eAddressV) ||
			!Try_ToAuthoredAddress(Source.AddressW, Staged.eAddressW) ||
			!Try_ToAuthoredComparison(Source.ComparisonFunc, Staged.eComparison) ||
			!std::isfinite(Source.MipLODBias) ||
			!std::isfinite(Source.MinLOD) || !std::isfinite(Source.MaxLOD) ||
			Source.MaxAnisotropy > 16u)
		{
			return false;
		}
		Staged.fMipLodBias = Source.MipLODBias;
		/* D3D readback may report zero for a non-anisotropic sampler even
		   though MaxAnisotropy is semantically unused.  The authored contract
		   has no zero state, so store its canonical inert value. */
		Staged.iMaxAnisotropy = (std::max)(1u, Source.MaxAnisotropy);
		Staged.vBorderColor = { Source.BorderColor[0u], Source.BorderColor[1u],
			Source.BorderColor[2u], Source.BorderColor[3u] };
		Staged.fMinLod = Source.MinLOD;
		Staged.fMaxLod = Source.MaxLOD;
		Out = Staged;
		return true;
	}

	bool_t Try_ToD3dSampler(
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Source,
		D3D11_SAMPLER_DESC& Out)
	{
		D3D11_SAMPLER_DESC Staged{};
		if (!Try_ToD3dFilter(Source.eFilter, Staged.Filter) ||
			!Try_ToD3dAddress(Source.eAddressU, Staged.AddressU) ||
			!Try_ToD3dAddress(Source.eAddressV, Staged.AddressV) ||
			!Try_ToD3dAddress(Source.eAddressW, Staged.AddressW) ||
			!Try_ToD3dComparison(Source.eComparison, Staged.ComparisonFunc) ||
			!std::isfinite(Source.fMipLodBias) ||
			!std::isfinite(Source.fMinLod) || !std::isfinite(Source.fMaxLod) ||
			Source.iMaxAnisotropy == 0u || Source.iMaxAnisotropy > 16u ||
			Source.fMinLod > Source.fMaxLod)
		{
			return false;
		}
		Staged.MipLODBias = Source.fMipLodBias;
		Staged.MaxAnisotropy = Source.iMaxAnisotropy;
		Staged.BorderColor[0u] = Source.vBorderColor.x;
		Staged.BorderColor[1u] = Source.vBorderColor.y;
		Staged.BorderColor[2u] = Source.vBorderColor.z;
		Staged.BorderColor[3u] = Source.vBorderColor.w;
		Staged.MinLOD = Source.fMinLod;
		Staged.MaxLOD = Source.fMaxLod;
		Out = Staged;
		return true;
	}

	bool_t Same_BlendDescriptor(
		const D3D11_BLEND_DESC& Left,
		const D3D11_BLEND_DESC& Right)
	{
		if (Left.AlphaToCoverageEnable != Right.AlphaToCoverageEnable ||
			Left.IndependentBlendEnable != Right.IndependentBlendEnable)
			return false;
		for (size_t Index = 0u; Index < 8u; ++Index)
		{
			const D3D11_RENDER_TARGET_BLEND_DESC& A = Left.RenderTarget[Index];
			const D3D11_RENDER_TARGET_BLEND_DESC& B = Right.RenderTarget[Index];
			if (A.BlendEnable != B.BlendEnable || A.SrcBlend != B.SrcBlend ||
				A.DestBlend != B.DestBlend || A.BlendOp != B.BlendOp ||
				A.SrcBlendAlpha != B.SrcBlendAlpha ||
				A.DestBlendAlpha != B.DestBlendAlpha ||
				A.BlendOpAlpha != B.BlendOpAlpha ||
				A.RenderTargetWriteMask != B.RenderTargetWriteMask)
				return false;
		}
		return true;
	}

	bool_t Same_RasterizerDescriptor(
		const D3D11_RASTERIZER_DESC& Left,
		const D3D11_RASTERIZER_DESC& Right)
	{
		return Left.FillMode == Right.FillMode &&
			Left.CullMode == Right.CullMode &&
			Left.FrontCounterClockwise == Right.FrontCounterClockwise &&
			Left.DepthBias == Right.DepthBias &&
			Left.DepthBiasClamp == Right.DepthBiasClamp &&
			Left.SlopeScaledDepthBias == Right.SlopeScaledDepthBias &&
			Left.DepthClipEnable == Right.DepthClipEnable &&
			Left.ScissorEnable == Right.ScissorEnable &&
			Left.MultisampleEnable == Right.MultisampleEnable &&
			Left.AntialiasedLineEnable == Right.AntialiasedLineEnable;
	}

	bool_t Same_DepthStencilOperation(
		const D3D11_DEPTH_STENCILOP_DESC& Left,
		const D3D11_DEPTH_STENCILOP_DESC& Right)
	{
		return Left.StencilFailOp == Right.StencilFailOp &&
			Left.StencilDepthFailOp == Right.StencilDepthFailOp &&
			Left.StencilPassOp == Right.StencilPassOp &&
			Left.StencilFunc == Right.StencilFunc;
	}

	bool_t Same_DepthStencilDescriptor(
		const D3D11_DEPTH_STENCIL_DESC& Left,
		const D3D11_DEPTH_STENCIL_DESC& Right)
	{
		return Left.DepthEnable == Right.DepthEnable &&
			Left.DepthWriteMask == Right.DepthWriteMask &&
			Left.DepthFunc == Right.DepthFunc &&
			Left.StencilEnable == Right.StencilEnable &&
			Left.StencilReadMask == Right.StencilReadMask &&
			Left.StencilWriteMask == Right.StencilWriteMask &&
			Same_DepthStencilOperation(Left.FrontFace, Right.FrontFace) &&
			Same_DepthStencilOperation(Left.BackFace, Right.BackFace);
	}

	class CReconstructedPipelineStateGuard final
	{
	public:
		explicit CReconstructedPipelineStateGuard(ID3D11DeviceContext* pContext)
			: m_pContext(pContext)
		{
			ID3D11SamplerState* pSampler = nullptr;
			ID3D11BlendState* pBlend = nullptr;
			ID3D11RasterizerState* pRasterizer = nullptr;
			ID3D11DepthStencilState* pDepthStencil = nullptr;
			m_pContext->PSGetSamplers(0u, 1u, &pSampler);
			m_pContext->OMGetBlendState(
				&pBlend, m_BlendFactor.data(), &m_iSampleMask);
			m_pContext->RSGetState(&pRasterizer);
			m_pContext->OMGetDepthStencilState(
				&pDepthStencil, &m_iStencilReference);
			m_pSampler.Attach(pSampler);
			m_pBlendState.Attach(pBlend);
			m_pRasterizerState.Attach(pRasterizer);
			m_pDepthStencilState.Attach(pDepthStencil);
		}

		~CReconstructedPipelineStateGuard()
		{
			ID3D11SamplerState* pSampler = m_pSampler.Get();
			m_pContext->PSSetSamplers(0u, 1u, &pSampler);
			m_pContext->OMSetBlendState(m_pBlendState.Get(),
				m_BlendFactor.data(), m_iSampleMask);
			m_pContext->RSSetState(m_pRasterizerState.Get());
			m_pContext->OMSetDepthStencilState(
				m_pDepthStencilState.Get(), m_iStencilReference);
		}

	private:
		ID3D11DeviceContext* m_pContext = nullptr;
		ComPtr<ID3D11SamplerState> m_pSampler;
		ComPtr<ID3D11BlendState> m_pBlendState;
		ComPtr<ID3D11RasterizerState> m_pRasterizerState;
		ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
		std::array<float, 4u> m_BlendFactor{};
		uint32_t m_iSampleMask = 0xffffffffu;
		uint32_t m_iStencilReference = 0u;
	};

	uint64_t Build_ResourceSignature(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		/* Product documents are immutable for one catalog revision.  Their
		   address and stable asset id already form the exact in-process cache
		   identity; serializing multi-megabyte JSON here repeated work performed
		   by the catalog parser on every prewarm target. */
		uint64_t Hash = static_cast<uint64_t>(
			reinterpret_cast<uintptr_t>(&Document));
		for (const unsigned char Byte : Document.strEffectAssetId)
		{
			Hash ^= static_cast<uint64_t>(Byte);
			Hash *= 1099511628211ull;
		}
		return Hash;
	}

	bool_t Needs_ParticleInstanceBuffer(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		return std::any_of(
			Document.Elements.begin(), Document.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				if (Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
					return true;
				/* An admitted source recipe preserves its source carrier kind while
				   producing runtime particles. */
				return Element.SourceRecipe.bEnabled &&
					((Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind &&
						Element.SourceRecipe.strRendererShape == "mesh") ||
					 (Client::EFFECT_ELEMENT_KIND::SPRITE == Element.eKind &&
						Element.SourceRecipe.strRendererShape == "sprite"));
			});
	}

	bool_t Try_ResolveTrailBufferPointCapacity(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		uint32_t& iOutPointCapacity,
		std::string& strOutError)
	{
		iOutPointCapacity = 0u;
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Client::EFFECT_ELEMENT_KIND::TRAIL != Element.eKind)
				continue;
			uint32_t iElementRenderPoints = Element.Detail.Trail.iMaxPoints;
			if (Element.Detail.Trail.fDistanceTessellationStepWorldUnits > 0.f)
			{
				constexpr uint32_t MAX_SEGMENT_SUBDIVISIONS = 25u;
				if (Element.Detail.Trail.iMaxPoints >
					((std::numeric_limits<uint32_t>::max)() - 1u) /
						MAX_SEGMENT_SUBDIVISIONS + 1u)
				{
					strOutError =
						"Effect trail tessellation capacity overflowed.";
					return false;
				}
				iElementRenderPoints = 1u +
					(Element.Detail.Trail.iMaxPoints - 1u) *
					MAX_SEGMENT_SUBDIVISIONS;
			}
			iOutPointCapacity = (std::max)(
				iOutPointCapacity, iElementRenderPoints);
		}
		if (0u != iOutPointCapacity)
			iOutPointCapacity = (std::max)(256u, iOutPointCapacity);
		return true;
	}

	HRESULT Create_SolidTexture(
		ID3D11Device* pDevice,
		const uint32_t iRGBA,
		ComPtr<ID3D11ShaderResourceView>& OutSRV)
	{
		if (nullptr == pDevice)
			return E_INVALIDARG;
		D3D11_TEXTURE2D_DESC Desc{};
		Desc.Width = 1u;
		Desc.Height = 1u;
		Desc.MipLevels = 1u;
		Desc.ArraySize = 1u;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1u;
		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = &iRGBA;
		Data.SysMemPitch = sizeof(iRGBA);
		ComPtr<ID3D11Texture2D> Texture;
		if (FAILED(pDevice->CreateTexture2D(&Desc, &Data, &Texture)))
			return E_FAIL;
		return pDevice->CreateShaderResourceView(
			Texture.Get(), nullptr, &OutSRV);
	}

	std::shared_ptr<EFFECT_RENDERER_CORE> Build_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (nullptr == pDevice || nullptr == pContext)
			return nullptr;
		auto Core = std::make_shared<EFFECT_RENDERER_CORE>();
		Core->pMeshShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements);
		Core->pAnimatedModelShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements, VTXANIMMESH::iNumElements);
		Core->pRectShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		Core->pParticleShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl"),
			Engine::VTXEFFECT_PARTICLE::Elements,
			Engine::VTXEFFECT_PARTICLE::iNumElements);
		Core->pTrailShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl"),
			Engine::VTXEFFECT_TRAIL::Elements,
			Engine::VTXEFFECT_TRAIL::iNumElements);
		Core->pDecalShader = Engine::CShader::Create(
			pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		Core->pRect = Engine::CVIBuffer_Rect::Create(pDevice, pContext);
		/* The immediate context consumes particle instances and draws them before
		   the next Effect is visited, so one device-local dynamic buffer is shared
		   safely by every renderer instance. This keeps action-edge attach free of
		   GPU buffer creation. */
		Core->pParticleBuffer = Engine::CVIBuffer_ParticleRect::Create(
			pDevice, pContext, 2048u);
		if (nullptr == Core->pMeshShader ||
			nullptr == Core->pAnimatedModelShader ||
			nullptr == Core->pRectShader ||
			nullptr == Core->pParticleShader ||
			nullptr == Core->pTrailShader ||
			nullptr == Core->pDecalShader || nullptr == Core->pRect ||
			nullptr == Core->pParticleBuffer ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xffffffffu, Core->pWhiteTexture)) ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xff000000u, Core->pBlackTexture)))
		{
			return nullptr;
		}
		++g_EffectRenderPrewarmProbe.iMutableInstanceBufferBuildCount;
		return Core;
	}

	std::shared_ptr<EFFECT_RENDERER_CORE> Acquire_RendererCore(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (nullptr == pDevice || nullptr == pContext)
			return nullptr;
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto Existing = g_EffectRendererCores.find(pDevice.Get());
		if (Existing != g_EffectRendererCores.end())
			return Existing->second;
		std::shared_ptr<EFFECT_RENDERER_CORE> Staged =
			Build_RendererCore(pDevice, pContext);
		if (nullptr == Staged)
			return nullptr;
		g_EffectRendererCores.emplace(pDevice.Get(), Staged);
		++g_EffectRenderPrewarmProbe.iCoreBuildCount;
		return Staged;
	}
}

Client::CEffectDocumentRenderer::CEffectDocumentRenderer(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffectDocumentRenderer::~CEffectDocumentRenderer() = default;

HRESULT Client::CEffectDocumentRenderer::Initialize()
{
	if (nullptr == m_pDevice || nullptr == m_pContext)
		return E_INVALIDARG;

	const std::shared_ptr<EFFECT_RENDERER_CORE> Core =
		Acquire_RendererCore(m_pDevice, m_pContext);
	if (nullptr == Core)
	{
		m_strStatus = "Effect renderer core resource creation failed.";
		return E_FAIL;
	}

	m_pMeshShader = Core->pMeshShader;
	m_pAnimatedModelShader = Core->pAnimatedModelShader;
	m_pRectShader = Core->pRectShader;
	m_pParticleShader = Core->pParticleShader;
	m_pTrailShader = Core->pTrailShader;
	m_pDecalShader = Core->pDecalShader;
	m_pRect = Core->pRect;
	m_pParticleBuffer = Core->pParticleBuffer;
	m_pWhiteTexture = Core->pWhiteTexture;
	m_pBlackTexture = Core->pBlackTexture;
	m_strStatus = "Effect renderer ready.";
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutSRV,
	PREWARM_ASSET_CACHE* pSharedAssets) const
{
	const std::string CacheKey = "default\n" + strAssetId;
	if (nullptr != pSharedAssets)
	{
		const auto Cached = pSharedAssets->Textures.find(CacheKey);
		if (Cached != pSharedAssets->Textures.end())
		{
			OutSRV = Cached->second;
			return S_OK;
		}
	}
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iTextureDiskLoadCount;
	}
	ComPtr<ID3D11ShaderResourceView> Staged;
	const HRESULT Result = DirectX::CreateDDSTextureFromFile(
		m_pDevice.Get(), Path.c_str(), nullptr, &Staged);
	if (FAILED(Result))
		return Result;
	if (nullptr != pSharedAssets)
		pSharedAssets->Textures.emplace(CacheKey, Staged);
	OutSRV = std::move(Staged);
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Load_SourceTexture(
	const EFFECT_NAMED_TEXTURE_DESC& Texture,
	ComPtr<ID3D11ShaderResourceView>& OutSRV,
	PREWARM_ASSET_CACHE* pSharedAssets) const
{
	const std::string CacheKey =
		(EFFECT_TEXTURE_COLOR_SPACE::SRGB == Texture.eColorSpace ?
			"source-srgb\n" : "source-linear\n") + Texture.strAssetId;
	if (nullptr != pSharedAssets)
	{
		const auto Cached = pSharedAssets->Textures.find(CacheKey);
		if (Cached != pSharedAssets->Textures.end())
		{
			OutSRV = Cached->second;
			return S_OK;
		}
	}
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(Texture.strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	const DirectX::DDS_LOADER_FLAGS Flags =
		EFFECT_TEXTURE_COLOR_SPACE::SRGB == Texture.eColorSpace ?
			DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iTextureDiskLoadCount;
	}
	ComPtr<ID3D11ShaderResourceView> Staged;
	const HRESULT Result = DirectX::CreateDDSTextureFromFileEx(
		m_pDevice.Get(), Path.c_str(), 0u, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0u, 0u, Flags, nullptr, &Staged);
	if (FAILED(Result))
		return Result;
	if (nullptr != pSharedAssets)
		pSharedAssets->Textures.emplace(CacheKey, Staged);
	OutSRV = std::move(Staged);
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Stage_ElementResource(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_RESOURCE& OutResource,
	std::string& strOutError,
	PREWARM_ASSET_CACHE* pSharedAssets,
	const f32_t fModelPreScale) const
{
	ELEMENT_RESOURCE Staged;
	if (EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
	EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
	{
		OutResource = std::move(Staged);
		return S_OK;
	}
	if (Element.Material.Execution.bFailClosed &&
		!Element.Material.Execution.bAuthoringApproximate)
	{
		Staged.bOccurrenceVisualSuppressed = true;
		OutResource = std::move(Staged);
		return S_OK;
	}
	const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
		Element.Material.SourceMaterial;
	Staged.GroupedConstants =
		Build_EffectGroupedTranslucentConstants(SourceMaterial);
	Staged.iSourceMaterialProfile = EffectiveSourceMaterialProfileIndex(Element);
	if (UINT32_MAX == Staged.iSourceMaterialProfile)
	{
		if (SourceMaterial.strRuntimeShaderProfileId ==
			"effect.ue3.fallback-blocked.v1")
		{
			Staged.iSourceMaterialProfile = 0u;
		}
		else
		{
			strOutError = "Source Material runtime profile is not executable: " +
				SourceMaterial.strRuntimeShaderProfileId;
			return E_FAIL;
		}
	}
	for (size_t iScalar = 0u;
		iScalar < SourceMaterial.Scalars.size() && iScalar < 8u; ++iScalar)
	{
		if (iScalar < 4u)
			(&Staged.vSourceScalars0.x)[iScalar] =
				SourceMaterial.Scalars[iScalar].fValue;
		else
			(&Staged.vSourceScalars1.x)[iScalar - 4u] =
				SourceMaterial.Scalars[iScalar].fValue;
	}
	if (!SourceMaterial.Vectors.empty())
		Staged.vSourceVector0 = SourceMaterial.Vectors[0].vValue;
	if (SourceMaterial.Vectors.size() > 1u)
		Staged.vSourceVector1 = SourceMaterial.Vectors[1].vValue;
	if (11u == Staged.iSourceMaterialProfile)
	{
		Build_LinearFlowConstants(
			SourceMaterial, Staged.LinearFlowParameters,
			Staged.vLinearFlowMaskAColor, Staged.vLinearFlowMaskBColor,
			Staged.vSourceScalars0, Staged.vSourceScalars1);
	}
	else if (8u == Staged.iSourceMaterialProfile)
	{
		Build_BlacklineConstants(
			SourceMaterial, Staged.BlacklineParameters,
			Staged.vBlacklineDiffuseColor, Staged.vBlacklineMaskColor);
	}
	else if (9u == Staged.iSourceMaterialProfile)
	{
		Build_LocalCrackConstants(
			SourceMaterial, Staged.LocalCrackParameters,
			Staged.vLocalCrackOutColor, Staged.vLocalCrackInColor,
			Staged.vLocalCrackReflectionColor);
	}
	else if (12u == Staged.iSourceMaterialProfile)
	{
		Build_SliceConstants(SourceMaterial, Staged.vSourceScalars0,
			Staged.vSourceScalars1, Staged.vSourceVector0);
	}
	else if (13u == Staged.iSourceMaterialProfile ||
		15u == Staged.iSourceMaterialProfile)
	{
		Build_MissileTrailConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (14u == Staged.iSourceMaterialProfile)
	{
		Build_WaterTrailConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (16u == Staged.iSourceMaterialProfile)
	{
		Build_MakeFlowConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (17u == Staged.iSourceMaterialProfile)
	{
		Build_RingConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (18u == Staged.iSourceMaterialProfile)
	{
		Build_ParticleTrailConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (19u == Staged.iSourceMaterialProfile)
	{
		Build_ParticleMasterConstants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0);
	}
	else if (20u == Staged.iSourceMaterialProfile)
	{
		Build_SpriteWaveConstants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0);
	}
	else if (21u == Staged.iSourceMaterialProfile)
	{
		Build_ParticleTrailConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (22u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistSpla01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (23u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistSpla05Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (24u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistTwinkleConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (25u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistFluid01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (26u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistWorldOffset01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (27u == Staged.iSourceMaterialProfile)
	{
		Build_MakeFlowConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (28u == Staged.iSourceMaterialProfile)
	{
		Build_ArtistLensFlare01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (29u == Staged.iSourceMaterialProfile)
	{
		Build_Glasshole02Constants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0, Staged.vSourceVector1);
	}
	else if (30u == Staged.iSourceMaterialProfile)
	{
		Build_FluidNinja01Constants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0, Staged.vSourceVector1);
	}
	else if (31u == Staged.iSourceMaterialProfile)
	{
		Build_CustomParticle01Constants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0);
	}
	else if (32u == Staged.iSourceMaterialProfile)
	{
		Build_CrackholeV2Constants(
			SourceMaterial, Staged.TypedTrailParameters,
			Staged.vSourceVector0, Staged.vSourceVector1);
	}
	else if (33u == Staged.iSourceMaterialProfile)
	{
		Build_Simple01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (34u == Staged.iSourceMaterialProfile)
	{
		Build_MmFluid01SpriteConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (35u == Staged.iSourceMaterialProfile)
	{
		Build_FlowRibbon01Constants(SourceMaterial,
			Element.Material.strSourceMaterialPath,
			Staged.TypedTrailParameters);
	}
	else if (36u == Staged.iSourceMaterialProfile)
	{
		Build_MakeFlowConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (37u == Staged.iSourceMaterialProfile)
	{
		Build_MakeFlow03SpriteConstants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (38u == Staged.iSourceMaterialProfile)
	{
		Build_Simple02Constants(SourceMaterial,
			Staged.TypedTrailParameters, Staged.vSourceVector0);
	}
	else if (39u == Staged.iSourceMaterialProfile)
	{
		Build_MmBasic01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	else if (40u == Staged.iSourceMaterialProfile)
	{
		Build_FlowTrail01Constants(
			SourceMaterial, Staged.TypedTrailParameters);
	}
	for (size_t iSemantic = 0u;
		iSemantic < Staged.DynamicParameterSemantics.size(); ++iSemantic)
	{
		Staged.DynamicParameterSemantics[iSemantic] =
			DynamicParameterSemanticIndex(
				SourceMaterial.DynamicParameterSemantics[iSemantic]);
	}
	std::array<uint32_t, 4u> TypedDynamicSemantics{};
	const bool_t bTypedDynamicSemanticsResolved =
		Try_ResolveTypedDynamicParameterSemantics(
			Element, Staged.iSourceMaterialProfile, TypedDynamicSemantics);
	if (bTypedDynamicSemanticsResolved)
	{
		Staged.DynamicParameterSemantics = TypedDynamicSemantics;
	}
	else if (13u == Staged.iSourceMaterialProfile)
	{
		strOutError =
			"MissileTrail source Material requires one exact typed dynamic "
			"parameter module: " + Element.strElementId;
		return E_FAIL;
	}
	const EFFECT_RESOURCE_BINDING_DESC* pModelBinding =
		Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL);
	if (nullptr != pModelBinding)
	{
		if (!std::isfinite(fModelPreScale) || fModelPreScale <= 0.f ||
			fModelPreScale > 100.f)
		{
			strOutError = "Effect model pre-scale is invalid: " +
				Element.strElementId;
			return E_FAIL;
		}
		const std::string ModelCacheKey = pModelBinding->strAssetId + "\n" +
			std::to_string(fModelPreScale);
		if (nullptr != pSharedAssets)
		{
			const auto Cached = pSharedAssets->NonAnimatedModels.find(
				ModelCacheKey);
			if (Cached != pSharedAssets->NonAnimatedModels.end())
				Staged.pModel = Cached->second;
		}
		if (nullptr == Staged.pModel)
		{
			const std::filesystem::path ModelPath = CRuntimeAssetRoot::Resolve(
				std::filesystem::path(pModelBinding->strAssetId));
			{
				const std::scoped_lock Lock(g_EffectRenderCacheMutex);
				++g_EffectRenderPrewarmProbe.iModelDiskLoadCount;
			}
			unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
				m_pDevice, m_pContext, MODEL::NONANIM,
				ModelPath.string().c_str(), XMMatrixScaling(
					fModelPreScale, fModelPreScale, fModelPreScale));
			if (nullptr == Model)
			{
				strOutError = "CModel load failed: " +
					pModelBinding->strAssetId;
				return E_FAIL;
			}
			Staged.pModel = std::move(Model);
			if (nullptr != pSharedAssets)
				pSharedAssets->NonAnimatedModels.emplace(
					ModelCacheKey, Staged.pModel);
		}
	}

	for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
	{
		if (Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID)
			continue;
		const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
			Find_EffectMaterialInput(
				Element.Material.strTemplateId, Binding.strSlotId);
		if (nullptr == pInput)
		{
			strOutError = "Material Template input is not registered: " +
				Binding.strSlotId;
			return E_FAIL;
		}
		ComPtr<ID3D11ShaderResourceView> Texture;
		if (FAILED(Load_Texture(
			Binding.strAssetId, Texture, pSharedAssets)))
		{
			strOutError = "DDS load failed: " + Binding.strAssetId;
			return E_FAIL;
		}
		Staged.Textures[Texture_Index(pInput->eRuntimeSlot)] = std::move(Texture);
	}
	const auto StageExactLinearSourceTextures = [this, &Staged, &strOutError,
		pSharedAssets, &Element](const std::span<const std::string_view> AssetIds)
	{
		if (AssetIds.empty() || AssetIds.size() > Staged.SourceTextures.size())
			return false;
		for (size_t iTexture = 0u; iTexture < AssetIds.size(); ++iTexture)
		{
			if (AssetIds[iTexture].empty() || Staged.SourceTextures[iTexture])
				return false;
			EFFECT_NAMED_TEXTURE_DESC TextureDesc;
			TextureDesc.strName = "typed-source-" + std::to_string(iTexture);
			TextureDesc.strAssetId = std::string(AssetIds[iTexture]);
			TextureDesc.eColorSpace = EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
			TextureDesc.eAddressU = EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
			TextureDesc.eAddressV = EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
			if (FAILED(Load_SourceTexture(TextureDesc,
				Staged.SourceTextures[iTexture], pSharedAssets)))
			{
				strOutError = "Typed source DDS stage failed: " +
					TextureDesc.strAssetId + " (" + Element.strElementId + ")";
				return false;
			}
			Staged.iSourceTextureMask |= 1u << static_cast<uint32_t>(iTexture);
		}
		return true;
	};
	const auto StageNamedSourceTexture = [this, &Staged, &strOutError,
		pSharedAssets, &Element](const size_t iLane,
		const EFFECT_NAMED_TEXTURE_DESC* pTexture, const bool_t bRequired)
	{
		if (iLane >= Staged.SourceTextures.size() || Staged.SourceTextures[iLane])
			return false;
		if (nullptr == pTexture)
		{
			if (bRequired)
			{
				strOutError = "Required typed source texture is absent: " +
					Element.strElementId;
				return false;
			}
			return true;
		}
		if (FAILED(Load_SourceTexture(
			*pTexture, Staged.SourceTextures[iLane], pSharedAssets)))
		{
			strOutError = "Typed named source DDS stage failed: " +
				pTexture->strAssetId + " (" + Element.strElementId + ")";
			return false;
		}
		const uint32_t iLaneBit = 1u << static_cast<uint32_t>(iLane);
		Staged.iSourceTextureMask |= iLaneBit;
		if (EFFECT_TEXTURE_ADDRESS_MODE::CLAMP == pTexture->eAddressU)
			Staged.iSourceTextureClampUMask |= iLaneBit;
		if (EFFECT_TEXTURE_ADDRESS_MODE::CLAMP == pTexture->eAddressV)
			Staged.iSourceTextureClampVMask |= iLaneBit;
		return true;
	};
	const auto StageRequiredNamedTextureContract = [&StageNamedSourceTexture,
		&SourceMaterial](const std::span<const std::string_view> Names)
	{
		for (size_t iLane = 0u; iLane < Names.size(); ++iLane)
		{
			if (!StageNamedSourceTexture(iLane,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, Names[iLane]), true))
			{
				return false;
			}
		}
		return true;
	};
	if (11u == Staged.iSourceMaterialProfile && SourceMaterial.Textures.empty())
	{
		static constexpr std::array<std::string_view, 7u> LINEARFLOW_TEXTURES = {{
			"Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_mirnoise_02.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_00/fx_bg_dustpanner_01.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_auraline_19_ycl.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_spatter_001_xyclamp.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds"
		}};
		if (!StageExactLinearSourceTextures(LINEARFLOW_TEXTURES))
			return E_FAIL;
	}
	else if (14u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_WATERTRAIL_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (16u == Staged.iSourceMaterialProfile ||
		36u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_MAKEFLOW_MESH_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (17u == Staged.iSourceMaterialProfile)
	{
		static constexpr std::array<std::string_view, 2u> RING_TEXTURES = {{
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_002.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds"
		}};
		if (!StageExactLinearSourceTextures(RING_TEXTURES))
			return E_FAIL;
	}
	else if (18u == Staged.iSourceMaterialProfile)
	{
		static constexpr std::array<std::string_view, 3u> TRAIL_03_TEXTURES = {{
			"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_007.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_001.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_004.dds"
		}};
		static constexpr std::array<std::string_view, 3u> TRAIL_01_TEXTURES = {{
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_005.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_05/fx_k_caustictile_01.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds"
		}};
		const std::span<const std::string_view> Textures =
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_s_00.fx_mi.fx_s_pa_trail_03_01_tr" ?
			std::span<const std::string_view>(TRAIL_03_TEXTURES) :
			std::span<const std::string_view>(TRAIL_01_TEXTURES);
		if (!StageExactLinearSourceTextures(Textures))
			return E_FAIL;
	}
	else if (19u == Staged.iSourceMaterialProfile)
	{
		for (size_t iLane = 0u;
			iLane < Client::EFFECT_PARTICLE_MASTER_SOURCE_TEXTURE_NAMES.size();
			++iLane)
		{
			if (!StageNamedSourceTexture(iLane,
				Client::Find_EffectUniqueNamedTexture(SourceMaterial,
					Client::EFFECT_PARTICLE_MASTER_SOURCE_TEXTURE_NAMES[iLane]),
				false))
			{
				return E_FAIL;
			}
		}
		const bool_t bHasAlpha = 0u != (Staged.iSourceTextureMask & 0x1u);
		const bool_t bHasEmission = 0u != (Staged.iSourceTextureMask & 0x30u);
		if (!bHasAlpha || !bHasEmission)
		{
			strOutError = "Particle-master typed source lanes are incomplete: " +
				Element.strElementId;
			return E_FAIL;
		}
	}
	else if (20u == Staged.iSourceMaterialProfile)
	{
		const EFFECT_NAMED_TEXTURE_DESC* pPrimaryNoise =
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "uv_noise_tex");
		if (nullptr == pPrimaryNoise)
		{
			pPrimaryNoise = Client::Find_EffectUniqueNamedTexture(
				SourceMaterial, "uv_noise_tex_02");
		}
		const EFFECT_NAMED_TEXTURE_DESC* pDissolve =
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "dissolve_tex_01");
		if (nullptr == pDissolve)
		{
			pDissolve = Client::Find_EffectUniqueNamedTexture(
				SourceMaterial, "dissolve_tex02");
		}
		const EFFECT_NAMED_TEXTURE_DESC* pNoiseDissolve =
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "noisedissolve_tex");
		if (nullptr == pNoiseDissolve)
		{
			pNoiseDissolve = Client::Find_EffectUniqueNamedTexture(
				SourceMaterial, "dissolve_tex02");
		}
		if (!StageNamedSourceTexture(0u,
				Client::Resolve_EffectSpriteWaveCarrierTexture(SourceMaterial), true) ||
			!StageNamedSourceTexture(1u, pPrimaryNoise, true) ||
			!StageNamedSourceTexture(2u, pDissolve, false) ||
			!StageNamedSourceTexture(3u, pNoiseDissolve, false) ||
			!StageNamedSourceTexture(4u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "emissivetex02"), true))
		{
			return E_FAIL;
		}
		const EFFECT_NAMED_TEXTURE_DESC* pSecondaryNoise =
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "uv_noise_tex_02");
		if (pSecondaryNoise != pPrimaryNoise &&
			!StageNamedSourceTexture(5u, pSecondaryNoise, false))
		{
			return E_FAIL;
		}
	}
	else if (21u == Staged.iSourceMaterialProfile)
	{
		for (size_t iLane = 0u; iLane <
			Client::EFFECT_PARTICLETRAIL_SINGLE_ALPHA_SOURCE_TEXTURE_NAMES.size();
			++iLane)
		{
			if (!StageNamedSourceTexture(iLane,
				Client::Find_EffectUniqueNamedTexture(SourceMaterial,
					Client::EFFECT_PARTICLETRAIL_SINGLE_ALPHA_SOURCE_TEXTURE_NAMES[iLane]),
				true))
			{
				return E_FAIL;
			}
		}
	}
	else if (22u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_ARTIST_SPLA01_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (23u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_ARTIST_SPLA05_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (24u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_ARTIST_TWINKLE_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(3u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "add_emissive_tex"), false))
		{
			return E_FAIL;
		}
	}
	else if (25u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_ARTIST_FLUID01_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(4u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "specular_tex"), false))
		{
			return E_FAIL;
		}
	}
	else if (26u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_ARTIST_WORLDOFFSET01_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (27u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_ARTIST_MAKEFLOW01_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (28u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_ARTIST_LENSFLARE01_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (29u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_GLASSHOLE02_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (30u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_FLUIDNINJA01_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (31u == Staged.iSourceMaterialProfile)
	{
		if (!StageNamedSourceTexture(0u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "diff_tex"), true) ||
			!StageNamedSourceTexture(1u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "a_noise_01_tex"), false))
		{
			return E_FAIL;
		}
	}
	else if (32u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_CRACKHOLEV2_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	else if (33u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_SIMPLE01_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(1u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "uv_noise_tex"), false))
			return E_FAIL;
	}
	else if (39u == Staged.iSourceMaterialProfile)
	{
		/* Lane order is the family contract, not the authored listing order:
		   0 emissive radiance, 1 dedicated coverage, 2 and 3 the two uv_noise
		   domains.  Only lane 0 is required. */
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_MM_BASIC01_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(1u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "alpha_tex"), false) ||
			!StageNamedSourceTexture(2u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "uv_noise_01_tex"), false) ||
			!StageNamedSourceTexture(3u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "uv_noise_02_tex"), false))
			return E_FAIL;
	}
	else if (40u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_FLOWTRAIL01_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(2u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "noise_tex"), false))
			return E_FAIL;
	}
	else if (41u == Staged.iSourceMaterialProfile)
	{
		/* This family has no named source texture: the chain artwork is the
		   element base binding, staged into lane 0 as linear. */
		const Client::EFFECT_RESOURCE_BINDING_DESC* pChainBase =
			Find_Binding(Element, Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
		if (nullptr == pChainBase || pChainBase->strAssetId.empty())
		{
			strOutError = "Masked chain element has no base binding: " +
				Element.strElementId;
			return E_FAIL;
		}
		const std::array<std::string_view, 1u> ChainBaseTexture = {{
			pChainBase->strAssetId
		}};
		if (!StageExactLinearSourceTextures(ChainBaseTexture))
			return E_FAIL;
	}
	else if (34u == Staged.iSourceMaterialProfile)
	{
		static constexpr std::array<std::string_view, 2u>
			MM_FLUID01_SPRITE_TEXTURES = {{
				"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_cloud_035.dds",
				"Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_o_glass_01.dds"
			}};
		if (!StageExactLinearSourceTextures(MM_FLUID01_SPRITE_TEXTURES))
			return E_FAIL;
	}
	else if (35u == Staged.iSourceMaterialProfile)
	{
		const bool_t bColorMapVariant =
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr";
		if (!StageNamedSourceTexture(0u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "tex_main"), true) ||
			(bColorMapVariant && !StageNamedSourceTexture(1u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "colormap"), true)) ||
			!StageNamedSourceTexture(bColorMapVariant ? 2u : 1u,
				Client::Find_EffectUniqueNamedTexture(
					SourceMaterial, "flowtex"), true))
		{
			return E_FAIL;
		}
	}
	else if (37u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
				Client::EFFECT_MAKEFLOW03_SPRITE_SOURCE_TEXTURE_NAMES) ||
			!StageNamedSourceTexture(4u,
				Client::Resolve_EffectMakeFlow03SpriteFlowTexture(
					SourceMaterial), true))
		{
			return E_FAIL;
		}
	}
	else if (38u == Staged.iSourceMaterialProfile)
	{
		if (!StageRequiredNamedTextureContract(
			Client::EFFECT_SIMPLE02_SOURCE_TEXTURE_NAMES))
			return E_FAIL;
	}
	if (8u == Staged.iSourceMaterialProfile ||
		9u == Staged.iSourceMaterialProfile ||
		11u == Staged.iSourceMaterialProfile)
	{
		for (const EFFECT_NAMED_TEXTURE_DESC& TextureDesc : SourceMaterial.Textures)
		{
			const int32_t iIndex = 8u == Staged.iSourceMaterialProfile ?
				BlacklineSourceTextureIndex(TextureDesc.strName) :
				(9u == Staged.iSourceMaterialProfile ?
					LocalCrackSourceTextureIndex(TextureDesc.strName) :
					LinearFlowSourceTextureIndex(TextureDesc.strName));
			/* The profile index tables are authored data, so a name the table
			   does not know about must not index past the SRV carrier. This
			   ran unchecked and aborted the tool mid-authoring. */
			if (iIndex < 0 || TextureDesc.strAssetId.empty() ||
				static_cast<size_t>(iIndex) >= Staged.SourceTextures.size())
			{
				continue;
			}
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (FAILED(Load_SourceTexture(
				TextureDesc, Texture, pSharedAssets)))
				continue;
			Staged.SourceTextures[static_cast<size_t>(iIndex)] =
				std::move(Texture);
			Staged.iSourceTextureMask |=
				1u << static_cast<uint32_t>(iIndex);
			if (EFFECT_TEXTURE_ADDRESS_MODE::CLAMP == TextureDesc.eAddressU)
				Staged.iSourceTextureClampUMask |=
					1u << static_cast<uint32_t>(iIndex);
			if (EFFECT_TEXTURE_ADDRESS_MODE::CLAMP == TextureDesc.eAddressV)
				Staged.iSourceTextureClampVMask |=
					1u << static_cast<uint32_t>(iIndex);
		}
	}
	if (9u == Staged.iSourceMaterialProfile &&
		Staged.iSourceTextureMask !=
			(1u << Client::EFFECT_LOCAL_CRACK_SOURCE_TEXTURE_NAMES.size()) - 1u)
	{
		strOutError =
			"Local-crack named normal/reflection/dissolve texture stage failed: " +
			Element.strElementId;
		return E_FAIL;
	}
	if (!Stage_AuthoredMaterialExecution(
		Element, Staged, strOutError, pSharedAssets))
	{
		return E_FAIL;
	}

	if (EFFECT_ELEMENT_KIND::MESH != Element.eKind &&
		nullptr == Staged.pModel &&
		nullptr == Find_Texture(Staged.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) &&
		Element.Material.strTemplateId != EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
		!Element.Material.Execution.bEnabled)
	{
		strOutError = "Texture/Particle/Decal/Trail requires a Base texture: " +
			Element.strElementId;
		return E_FAIL;
	}
	const bool_t bStrictTypedApproximateProfile =
		14u == Staged.iSourceMaterialProfile ||
		16u == Staged.iSourceMaterialProfile ||
		(Staged.iSourceMaterialProfile >= 19u &&
		 Staged.iSourceMaterialProfile <= 32u) ||
		33u == Staged.iSourceMaterialProfile ||
		34u == Staged.iSourceMaterialProfile ||
		35u == Staged.iSourceMaterialProfile ||
		(Staged.iSourceMaterialProfile >= 36u &&
		 Staged.iSourceMaterialProfile <= 41u);
	Staged.bSourceMaterialFallbackBlocked = !Element.Material.Execution.bEnabled &&
		((!bStrictTypedApproximateProfile &&
			Is_SourceMaterialFallbackBlocked(Element, Staged.GroupedConstants)) ||
		(8u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << Client::EFFECT_BLACKLINE_SOURCE_TEXTURE_NAMES.size()) - 1u) ||
		(9u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << Client::EFFECT_LOCAL_CRACK_SOURCE_TEXTURE_NAMES.size()) - 1u) ||
		(11u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << Client::EFFECT_LINEARFLOW_SOURCE_TEXTURE_NAMES.size()) - 1u) ||
		(14u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3u) != 0x3u) ||
		((16u == Staged.iSourceMaterialProfile ||
		  36u == Staged.iSourceMaterialProfile) &&
			(Staged.iSourceTextureMask & 0x1fu) != 0x1fu) ||
		(19u == Staged.iSourceMaterialProfile &&
			(0u == (Staged.iSourceTextureMask & 0x7u) ||
			 0u == (Staged.iSourceTextureMask & 0x30u))) ||
		(20u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x13u) != 0x13u) ||
		(21u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3u) != 0x3u) ||
		(22u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0xfu) != 0xfu) ||
		(23u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0xfu) != 0xfu) ||
		(24u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x7u) != 0x7u) ||
		(25u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0xfu) != 0xfu) ||
		(26u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1fu) != 0x1fu) ||
		(27u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3fu) != 0x3fu) ||
		(28u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1u) != 0x1u) ||
		(29u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x7u) != 0x7u) ||
		(30u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1fu) != 0x1fu) ||
		(31u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1u) != 0x1u) ||
		(32u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3fu) != 0x3fu) ||
		(33u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1u) != 0x1u) ||
		(34u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3u) != 0x3u) ||
		(35u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask &
				(Element.Material.strSourceMaterialPath ==
					"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr" ?
					0x7u : 0x3u)) !=
				(Element.Material.strSourceMaterialPath ==
					"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr" ?
					0x7u : 0x3u)) ||
		(37u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1fu) != 0x1fu) ||
		(38u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x7u) != 0x7u) ||
		(39u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1u) != 0x1u) ||
		(40u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x3u) != 0x3u) ||
		(41u == Staged.iSourceMaterialProfile &&
			(Staged.iSourceTextureMask & 0x1u) != 0x1u));
	OutResource = std::move(Staged);
	return S_OK;
}

bool_t Client::CEffectDocumentRenderer::Capture_MaterialExecutionLane(
	ELEMENT_RESOURCE& Resource,
	const size_t iLane,
	std::string strAssetId,
	std::string strRole,
	std::string strSourceChannel,
	const EFFECT_TEXTURE_COLOR_SPACE eColorSpace,
	const ComPtr<ID3D11SamplerState>& pSampler,
	std::string& strOutError) const
{
	if (iLane >= Resource.MaterialExecutionLanes.size() ||
		Resource.MaterialExecutionLanes[iLane].has_value() ||
		strAssetId.empty() || nullptr == pSampler ||
		eColorSpace == EFFECT_TEXTURE_COLOR_SPACE::END)
	{
		strOutError = "Material execution lane capture identity is invalid.";
		return false;
	}
	D3D11_SAMPLER_DESC D3dSampler{};
	pSampler->GetDesc(&D3dSampler);
	EFFECT_MATERIAL_TEXTURE_LANE_DESC Staged;
	Staged.strLaneId = "lane." + std::to_string(iLane);
	if (strRole.empty())
	{
		Staged.strRole = Staged.strLaneId;
	}
	else
	{
		Staged.strRole.reserve(std::min<size_t>(strRole.size(), 128u));
		bool_t bLastWasSeparator = false;
		for (const char_t Character : strRole)
		{
			const unsigned char Value = static_cast<unsigned char>(Character);
			if (0 != std::isalnum(Value) || Character == '_' || Character == '-')
			{
				Staged.strRole.push_back(Character);
				bLastWasSeparator = false;
			}
			else if (!bLastWasSeparator && !Staged.strRole.empty())
			{
				Staged.strRole.push_back('.');
				bLastWasSeparator = true;
			}
			if (Staged.strRole.size() == 128u)
				break;
		}
		while (!Staged.strRole.empty() && Staged.strRole.back() == '.')
			Staged.strRole.pop_back();
		if (Staged.strRole.empty())
			Staged.strRole = Staged.strLaneId;
	}
	Staged.strAssetId = std::move(strAssetId);
	Staged.iTextureRegister = static_cast<uint32_t>(iLane);
	Staged.iSamplerRegister = 5u + static_cast<uint32_t>(iLane);
	Staged.strSourceChannel = std::move(strSourceChannel);
	Staged.eColorSpace = eColorSpace;
	if (!Try_ToAuthoredSampler(D3dSampler, Staged.Sampler))
	{
		strOutError = "Material execution sampler cannot be authored: lane=" +
			std::to_string(iLane) + ".";
		return false;
	}
	Resource.MaterialExecutionLanes[iLane] = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_AuthoredMaterialExecution(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_RESOURCE& Resource,
	std::string& strOutError,
	PREWARM_ASSET_CACHE* pSharedAssets) const
{
	const EFFECT_MATERIAL_EXECUTION_DESC& Execution = Element.Material.Execution;
	if (!Validate_WarlordWpoSinWaveElectricExecution(Element, strOutError))
		return false;
	if (!Validate_LanceDragonMaskedExecution(Element, strOutError))
		return false;
	if (!Validate_ArtistDBlackTigerStrokeExecution(Element, strOutError))
		return false;
	if (!Execution.bEnabled)
		return true;
	if (Execution.iVersion != 1u ||
		Execution.eBackend == EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC ||
		Execution.eBackend == EFFECT_MATERIAL_EXECUTION_BACKEND::END ||
		Execution.iOpcode == 0u || Element.Material.SourceMaterial.bEnabled)
	{
		strOutError = "Authored material execution identity is invalid: " +
			Element.strElementId;
		return false;
	}
	if ((Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL &&
		 Element.eKind != EFFECT_ELEMENT_KIND::DECAL) ||
		(Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4 &&
		 Element.eKind != EFFECT_ELEMENT_KIND::MESH &&
		 Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE) ||
		(Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
		 ((Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		   Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END) ||
		  (Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		   Element.eKind != EFFECT_ELEMENT_KIND::DECAL &&
		   Element.eKind != EFFECT_ELEMENT_KIND::TRAIL))))
	{
		strOutError = "Authored material backend has no matching renderer carrier: " +
			Element.strElementId;
		return false;
	}
	if (Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		Execution.iOpcode ==
			PROJECT_BASE_COVERAGE_EMISSIVE_DISSOLVE_RECT_OPCODE)
	{
		static constexpr std::array<std::string_view, 4u> RESOURCE_SLOTS = {{
			"base", "mask", "emissive", "dissolve"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_ROLES = {{
			"base_radiance", "coverage", "emissive_radiance", "dissolve"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_CHANNELS = {{
			"RGBA", "R", "RGB", "R"
		}};
		bool_t bRectContractValid = Element.bVisible &&
			Element.eKind == EFFECT_ELEMENT_KIND::SPRITE &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			Element.Renderer.eSourceSpace == EFFECT_SOURCE_SPACE::END &&
			!Element.SourceRecipe.bEnabled &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.ResourceBindings.size() == RESOURCE_SLOTS.size() &&
			Execution.iTextureLaneCount == LANE_ROLES.size() &&
			Execution.iTextureMask == 0x0fu &&
			Execution.TextureLanes.size() == LANE_ROLES.size() &&
			Execution.iDynamicConsumedMask == 0u &&
			Execution.iDynamicSuppressedMask == 0u &&
			Execution.iParticleColorPolicy == 0u &&
			Execution.iParticleColorConsumedMask == 0u &&
			Execution.iParticleColorSuppressedMask == 0u &&
			Execution.iScalarCount == 0u && Execution.Scalars.empty() &&
			Execution.iVectorCount == 0u && Execution.Vectors.empty() &&
			Execution.iInputCount == 0u &&
			Execution.InputConsumedMask == std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.InputSuppressedMask == std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0u, 0u, 0u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0u, 0u, 0u } &&
			Execution.iStaticInputCount == 0u &&
			Execution.iStaticSelectedMask == 0u &&
			Execution.iStaticConsumedMask == 0u &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 0u &&
			Execution.iRenderConsumedMask == 0u &&
			Execution.iRenderSuppressedMask == 0u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty() &&
			Element.Detail.Color.fDistortionIntensity == 0.f &&
			!Element.Detail.Color.bDistortionOnBaseMaterial &&
			Element.Detail.Color.fRadialTime == 0.f &&
			Element.Detail.Color.fRadialIntensity == 0.f;
		for (size_t i = 0u; bRectContractValid && i < LANE_ROLES.size(); ++i)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding = Element.ResourceBindings[i];
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane = Execution.TextureLanes[i];
			bRectContractValid =
				Binding.strSlotId == RESOURCE_SLOTS[i] &&
				!Binding.strAssetId.empty() &&
				Lane.strLaneId == "lane." + std::to_string(i) &&
				Lane.strRole == LANE_ROLES[i] &&
				Lane.strAssetId == Binding.strAssetId &&
				Lane.iTextureRegister == i && Lane.iSamplerRegister == 5u + i &&
				Lane.strSourceChannel == LANE_CHANNELS[i] &&
				Lane.eColorSpace == EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
				Lane.Sampler.eFilter == EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR &&
				Lane.Sampler.eAddressU ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.eAddressV ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.eAddressW ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.fMipLodBias == 0.f &&
				Lane.Sampler.iMaxAnisotropy == 1u &&
				Lane.Sampler.eComparison ==
					EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER &&
				Lane.Sampler.vBorderColor.x == 0.f &&
				Lane.Sampler.vBorderColor.y == 0.f &&
				Lane.Sampler.vBorderColor.z == 0.f &&
				Lane.Sampler.vBorderColor.w == 0.f &&
				Lane.Sampler.fMinLod == 0.f &&
				Lane.Sampler.fMaxLod == (std::numeric_limits<f32_t>::max)();
		}
		if (!bRectContractValid)
		{
			strOutError =
				"Base/Coverage/Emissive/Dissolve rect opcode 21 packet is invalid: " +
				Element.strElementId;
			return false;
		}
	}
	if (Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		Execution.iOpcode == 17u)
	{
		static constexpr std::array<std::string_view, 2u> ELEMENT_IDS = {{
			"authored.source-particle.1ae3416ac205fee634b746a9",
			"authored.source-particle.ed33fb10661afb8854e76957"
		}};
		static constexpr std::array<std::string_view, 2u> SOURCE_NODES = {{
			"authored-source-particle:effect.dimensionmaster.skill.2050230."
				"unified|source:effect.dimensionmaster.skill.2050230.imported|"
				"element:fx_pc_swp_03.par_s_swp_chrono_atk_01."
				"particlespriteemitter_24",
			"authored-source-particle:effect.dimensionmaster.skill.2050230."
				"unified|source:effect.dimensionmaster.skill.2050230.imported|"
				"element:fx_pc_swp_03.par_s_swp_chrono_rewind_02."
				"particlespriteemitter_37"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_IDS = {{
			"lane.0", "lane.1", "lane.2", "lane.3"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_ROLES = {{
			"transition_texture", "emissive_tex",
			"uv_noise_01_tex", "uv_noise_02_tex"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_ASSETS = {{
			"Effect/DimensionMaster/Textures/FX_TEX_02/"
				"fx_d_cloud_035.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/"
				"fx_o_glass_01.dds",
			"Effect/DimensionMaster/Textures/FX_TEX_00/"
				"fx_bg_softriver_02_n.dds",
			"Effect/Warlord/Textures/FX_TEX_00/"
				"fx_bg_softriver_01_n.dds"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_CHANNELS = {{
			"RGB", "RGB", "RG", "RG"
		}};
		static constexpr std::array<std::string_view, 22u> SCALAR_NAMES = {{
			"transition_thickness", "transition_direction", "transition_tiling",
			"transition_panning_y", "transition_panning_x",
			"emissive_line_intensity", "transition_line_thickness",
			"uv_noise_01_tiling", "uv_noise_01_panning_y",
			"uv_noise_01_panning_x", "uv_noise_01_intensity",
			"uv_noise_02_tiling", "uv_noise_02_panning_y",
			"uv_noise_02_panning_x", "uv_noise_02_intensity",
			"emissive_intensity", "emissive_desaturation",
			"emissive_uv_scale_x", "emissive_uv_scale_y", "fresnel_power",
			"distortion_intensity", "total_scale"
		}};
		static constexpr std::array<f32_t, 22u> SCALAR_VALUES = {{
			0.3f, 0.1f, 4.f, 0.2f, 0.02f, 2.f, 2.f, 0.5f,
			0.1f, 0.2f, 0.f, 0.7f, 0.07f, 0.15f, 0.15f, 1.f,
			0.f, 2.f, 2.f, 1.f, 1.f, 1.f
		}};
		const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
		{
			return std::abs(Left - Right) <= 1.0e-6f *
				(std::max)({ 1.f, std::abs(Left), std::abs(Right) });
		};
		const bool_t bFirstIdentity =
			Client::Is_EffectSourceIdentityOrPortableCopy(
				Element, ELEMENT_IDS[0], SOURCE_NODES[0]);
		const bool_t bSecondIdentity =
			Client::Is_EffectSourceIdentityOrPortableCopy(
				Element, ELEMENT_IDS[1], SOURCE_NODES[1]);
		bool_t bFluid01ContractValid =
			(bFirstIdentity || bSecondIdentity) &&
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "sprite" &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr" &&
			Element.ResourceBindings.size() == 2u &&
			Element.ResourceBindings[0].strSlotId == "base" &&
			Element.ResourceBindings[0].strAssetId == LANE_ASSETS[0] &&
			Element.ResourceBindings[1].strSlotId == "emissive" &&
			Element.ResourceBindings[1].strAssetId == LANE_ASSETS[1] &&
			Execution.iTextureLaneCount == 4u && Execution.iTextureMask == 0xfu &&
			Execution.TextureLanes.size() == LANE_IDS.size() &&
			Execution.iDynamicConsumedMask == 0x7u &&
			Execution.iDynamicSuppressedMask == 0x8u &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0xfu &&
			Execution.iParticleColorSuppressedMask == 0u &&
			Execution.iScalarCount == SCALAR_NAMES.size() &&
			Execution.Scalars.size() == SCALAR_NAMES.size() &&
			Execution.iVectorCount == 0u && Execution.Vectors.empty() &&
			Execution.iInputCount == 22u &&
			Execution.InputConsumedMask == std::array<uint32_t, 2u>{
				0x003fffffu, 0u } &&
			Execution.InputSuppressedMask == std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0u, 0u, 0u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0u, 0u, 0u } &&
			Execution.iStaticInputCount == 0u &&
			Execution.iStaticSelectedMask == 0u &&
			Execution.iStaticConsumedMask == 0u &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 0u &&
			Execution.iRenderConsumedMask == 0u &&
			Execution.iRenderSuppressedMask == 0u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty();
		for (size_t i = 0u; bFluid01ContractValid && i < LANE_IDS.size(); ++i)
		{
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[i];
			bFluid01ContractValid =
				Lane.strLaneId == LANE_IDS[i] && Lane.strRole == LANE_ROLES[i] &&
				Lane.strAssetId == LANE_ASSETS[i] &&
				Lane.iTextureRegister == i && Lane.iSamplerRegister == 5u + i &&
				Lane.strSourceChannel == LANE_CHANNELS[i] &&
				Lane.eColorSpace == EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
				Lane.Sampler.eFilter == EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR &&
				Lane.Sampler.eAddressU ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.eAddressV ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.eAddressW ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.fMipLodBias == 0.f &&
				Lane.Sampler.iMaxAnisotropy == 1u &&
				Lane.Sampler.eComparison ==
					EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER &&
				Lane.Sampler.vBorderColor.x == 0.f &&
				Lane.Sampler.vBorderColor.y == 0.f &&
				Lane.Sampler.vBorderColor.z == 0.f &&
				Lane.Sampler.vBorderColor.w == 0.f &&
				Lane.Sampler.fMinLod == 0.f &&
				Lane.Sampler.fMaxLod == (std::numeric_limits<f32_t>::max)();
		}
		for (size_t i = 0u;
			bFluid01ContractValid && i < SCALAR_NAMES.size(); ++i)
		{
			const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			bFluid01ContractValid = Scalar.strName == SCALAR_NAMES[i] &&
				Scalar.iPackedIndex == i &&
				NearlyEqual(Scalar.fValue, SCALAR_VALUES[i]);
		}
		if (!bFluid01ContractValid)
		{
			strOutError = "Fluid01 W-FD-01-3 opcode 17 packet is not the admitted "
				"parent/child/carrier/role tuple: " + Element.strElementId;
			return false;
		}
	}
	if (Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		Execution.iOpcode == 20u)
	{
		static constexpr std::array<std::string_view, 4u> LANE_ROLES = {{
			"distortion_normal", "surface_normal", "alpha_aura",
			"reflection_fluid"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_ASSETS = {{
			"Effect/Artist/Textures/fx_d_normal_085.dds",
			"Effect/Artist/Textures/fx_d_normal_085.dds",
			"Effect/Artist/Textures/fx_k_auraline_14_ycl.dds",
			"Effect/Artist/Textures/fx_a_fluid_003.dds"
		}};
		static constexpr std::array<std::string_view, 4u> LANE_CHANNELS = {{
			"RG", "RG", "RGB", "RGB"
		}};
		static constexpr std::array<EFFECT_TEXTURE_COLOR_SPACE, 4u>
			LANE_COLOR_SPACES = {{
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
				EFFECT_TEXTURE_COLOR_SPACE::SRGB,
				EFFECT_TEXTURE_COLOR_SPACE::SRGB
			}};
		static constexpr std::array<std::string_view, 12u> SCALAR_NAMES = {{
			"normal_strength", "alpha_strength", "reflection_uv_scale",
			"distortion_strength", "normal_uv_scale_x", "normal_uv_scale_y",
			"alpha_uv_scale_x", "alpha_uv_scale_y", "normal_pan_x",
			"normal_pan_y", "alpha_pan_x", "alpha_pan_y"
		}};
		static constexpr std::array<f32_t, 12u> SCALAR_VALUES = {{
			0.5f, 2.f, 3.f, 50.f, 1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f
		}};
		const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
		{
			return std::abs(Left - Right) <= 1.0e-6f *
				(std::max)({ 1.f, std::abs(Left), std::abs(Right) });
		};
		bool_t bRibbonLiquidContractValid =
			Element.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "ribbon" &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_d_00.fx_mi.fx_d_pa_ribbonliquid_01_101_tr" &&
			Element.ResourceBindings.size() == 3u &&
			Element.ResourceBindings[0].strSlotId == "base" &&
			Element.ResourceBindings[0].strAssetId == LANE_ASSETS[0] &&
			Element.ResourceBindings[1].strSlotId == "noise" &&
			Element.ResourceBindings[1].strAssetId == LANE_ASSETS[3] &&
			Element.ResourceBindings[2].strSlotId == "emissive" &&
			Element.ResourceBindings[2].strAssetId == LANE_ASSETS[2] &&
			Execution.iTextureLaneCount == 4u &&
			Execution.iTextureMask == 0xfu &&
			Execution.TextureLanes.size() == LANE_ROLES.size() &&
			Execution.iDynamicConsumedMask == 0xfu &&
			Execution.iDynamicSuppressedMask == 0u &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x8u &&
			Execution.iParticleColorSuppressedMask == 0x7u &&
			Execution.iScalarCount == SCALAR_NAMES.size() &&
			Execution.Scalars.size() == SCALAR_NAMES.size() &&
			Execution.iVectorCount == 1u && Execution.Vectors.size() == 1u &&
			Execution.iInputCount == 17u &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ 0x1ff7fu, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0x80u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0xfu, 0u, 0u } &&
			Execution.VectorComponentSuppressedMask ==
				std::array<uint32_t, 3u>{ 0u, 0u, 0u } &&
			Execution.iStaticInputCount == 0u &&
			Execution.iStaticSelectedMask == 0u &&
			Execution.iStaticConsumedMask == 0u &&
			Execution.iStaticSuppressedMask == 0u &&
			Execution.iRenderInputCount == 6u &&
			Execution.iRenderConsumedMask == 0x2fu &&
			Execution.iRenderSuppressedMask == 0x10u &&
			Execution.ArtistParameters.empty() && Execution.Colors.empty();
		for (size_t i = 0u;
			bRibbonLiquidContractValid && i < LANE_ROLES.size(); ++i)
		{
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[i];
			bRibbonLiquidContractValid =
				Lane.strLaneId == "lane." + std::to_string(i) &&
				Lane.strRole == LANE_ROLES[i] && Lane.strAssetId == LANE_ASSETS[i] &&
				Lane.iTextureRegister == i && Lane.iSamplerRegister == 5u + i &&
				Lane.strSourceChannel == LANE_CHANNELS[i] &&
				Lane.eColorSpace == LANE_COLOR_SPACES[i] &&
				Lane.Sampler.eFilter == EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR &&
				Lane.Sampler.eAddressU ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.eAddressV == (i == 2u ?
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP :
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP) &&
				Lane.Sampler.eAddressW ==
					EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP &&
				Lane.Sampler.fMipLodBias == 0.f &&
				Lane.Sampler.iMaxAnisotropy == 1u &&
				Lane.Sampler.eComparison ==
					EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER &&
				Lane.Sampler.vBorderColor.x == 0.f &&
				Lane.Sampler.vBorderColor.y == 0.f &&
				Lane.Sampler.vBorderColor.z == 0.f &&
				Lane.Sampler.vBorderColor.w == 0.f &&
				Lane.Sampler.fMinLod == 0.f &&
				Lane.Sampler.fMaxLod ==
					(std::numeric_limits<f32_t>::max)();
		}
		for (size_t i = 0u;
			bRibbonLiquidContractValid && i < SCALAR_NAMES.size(); ++i)
		{
			const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar =
				Execution.Scalars[i];
			bRibbonLiquidContractValid =
				Scalar.strName == SCALAR_NAMES[i] && Scalar.iPackedIndex == i &&
				NearlyEqual(Scalar.fValue, SCALAR_VALUES[i]);
		}
		if (bRibbonLiquidContractValid)
		{
			const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Reflect =
				Execution.Vectors[0];
			bRibbonLiquidContractValid =
				Reflect.strName == "reflect_color_and_intensity" &&
				Reflect.iPackedIndex == 0u && NearlyEqual(Reflect.vValue.x, 1.f) &&
				NearlyEqual(Reflect.vValue.y, 1.f) &&
				NearlyEqual(Reflect.vValue.z, 3.f) &&
				NearlyEqual(Reflect.vValue.w, 50.f);
		}
		if (!bRibbonLiquidContractValid)
		{
			strOutError = "RibbonLiquid01 opcode 20 packet is not the admitted "
				"parent-default/carrier/role tuple: " + Element.strElementId;
			return false;
		}
	}
	const bool_t bStandardColorV1 = Execution.eBackend ==
		EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
	const bool_t bStandardColorMeshCarrier =
		Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Element.SourceRecipe.bEnabled &&
		Element.SourceRecipe.strRendererShape == "mesh" &&
		Element.ResourceBindings.size() == 1u &&
		Element.ResourceBindings[0u].strSlotId == "meshModel" &&
		!Element.ResourceBindings[0u].strAssetId.empty();
	const bool_t bStandardColorResourceContract =
		bStandardColorMeshCarrier ||
		(Element.ResourceBindings.empty() &&
		 ((Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		   Element.SourceRecipe.bEnabled &&
		   Element.SourceRecipe.strRendererShape == "sprite") ||
		  Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
		  Element.eKind == EFFECT_ELEMENT_KIND::TRAIL));
	if ((bStandardColorV1 &&
		 (Element.Material.strTemplateId != EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID ||
		  Execution.iOpcode != 1u ||
		  Execution.StandardColorV1.iPacketVersion != 1u ||
		  !bStandardColorResourceContract ||
		  Element.Material.SourceMaterial.bEnabled ||
		  Element.Material.eRenderProfile ==
			EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE)) ||
		(!bStandardColorV1 &&
		 Element.Material.strTemplateId == EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID))
	{
		strOutError = "StandardColorV1 admission identity is invalid: " +
			Element.strElementId;
		return false;
	}
	const uint32_t iSelectedPass = Select_Pass(Element.Material.eRenderProfile);
	if (iSelectedPass == UINT32_MAX || Execution.iPassIndex != iSelectedPass ||
		Execution.iStencilReference != 0u)
	{
		strOutError = "Authored material pass/stencil contract is invalid: " +
			Element.strElementId;
		return false;
	}
	std::string_view strExpectedRasterizer;
	std::string_view strExpectedDepth;
	std::string_view strExpectedBlend;
	switch (Element.Material.eRenderProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		strExpectedRasterizer = "RS_Default";
		strExpectedDepth = "DSS_Default";
		strExpectedBlend = "BS_EffectOpaque";
		break;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
		strExpectedRasterizer = "RS_Cull_None";
		strExpectedDepth = bStandardColorV1 &&
			Element.eKind == EFFECT_ELEMENT_KIND::DECAL ?
			"DSS_ZNone" : "DSS_ReadOnly";
		strExpectedBlend = "BS_EffectAlpha";
		break;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
		strExpectedRasterizer = "RS_Cull_None";
		strExpectedDepth = "DSS_ReadOnly";
		strExpectedBlend = "BS_EffectAdditive";
		break;
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
		strExpectedRasterizer = "RS_Default";
		strExpectedDepth = "DSS_ReadOnly";
		strExpectedBlend = "BS_EffectAlpha";
		break;
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
		strExpectedRasterizer = "RS_Default";
		strExpectedDepth = "DSS_ReadOnly";
		strExpectedBlend = "BS_EffectAdditive";
		break;
	case EFFECT_RENDER_PROFILE::END:
	default:
		strOutError = "Authored material render profile is invalid: " +
			Element.strElementId;
		return false;
	}
	if (Execution.strRasterizerState != strExpectedRasterizer ||
		Execution.strDepthStencilState != strExpectedDepth ||
		Execution.strBlendState != strExpectedBlend)
	{
		strOutError = "Authored material render-state snapshot does not match its "
			"shader pass: " + Element.strElementId;
		return false;
	}

	if (Execution.iTextureLaneCount > Resource.RuntimeMaterialV2Samplers.size() ||
		Execution.TextureLanes.size() != Execution.iTextureLaneCount ||
		Execution.iTextureMask != (Execution.iTextureLaneCount == 0u ? 0u :
			((1u << Execution.iTextureLaneCount) - 1u)))
	{
		strOutError = "Authored material texture-lane contract is invalid: " +
			Element.strElementId;
		return false;
	}
	Resource.SourceTextures.fill(nullptr);
	Resource.RuntimeMaterialV2Samplers.fill(nullptr);
	Resource.MaterialExecutionLanes.fill(std::nullopt);
	Resource.iStandardColorV1Enabled = 0u;
	Resource.StandardColorV1Header = {};
	Resource.StandardColorV1BaseCoverage = {};
	Resource.StandardColorV1Dissolve = {};
	Resource.StandardColorV1Policies = {};
	Resource.vStandardColorV1Scalars = {};
	Resource.StandardColorV1 = {};
	Resource.iSourceTextureMask = 0u;
	Resource.iSourceTextureClampUMask = 0u;
	Resource.iSourceTextureClampVMask = 0u;
	std::array<bool_t, 6u> LaneSeen{};
	for (const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane : Execution.TextureLanes)
	{
		const size_t iLane = static_cast<size_t>(Lane.iTextureRegister);
		if (iLane >= Execution.iTextureLaneCount || LaneSeen[iLane] ||
			Lane.iSamplerRegister != 5u + Lane.iTextureRegister ||
			Lane.strLaneId.empty() || Lane.strRole.empty() ||
			Lane.strAssetId.empty() ||
			((Execution.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL ||
			  bStandardColorV1) &&
			 Lane.strSourceChannel.empty()) ||
			(!Lane.strSourceChannel.empty() &&
			 (Lane.strSourceChannel.size() > 4u ||
			  !std::all_of(Lane.strSourceChannel.begin(),
				  Lane.strSourceChannel.end(), [](const char_t Character)
				  {
					  return std::string_view("RGBA").find(Character) !=
						  std::string_view::npos;
				  }))) ||
			Lane.eColorSpace == EFFECT_TEXTURE_COLOR_SPACE::END)
		{
			strOutError = "Authored material texture lane is invalid: " +
				Element.strElementId;
			return false;
		}
		LaneSeen[iLane] = true;
		EFFECT_NAMED_TEXTURE_DESC Texture;
		Texture.strName = Lane.strRole;
		Texture.strAssetId = Lane.strAssetId;
		Texture.eColorSpace = Lane.eColorSpace;
		Texture.eAddressU = EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
		Texture.eAddressV = EFFECT_TEXTURE_ADDRESS_MODE::WRAP;
		if (FAILED(Load_SourceTexture(
			Texture, Resource.SourceTextures[iLane], pSharedAssets)))
		{
			strOutError = "Authored material DDS stage failed: " +
				Lane.strAssetId;
			return false;
		}
		if (bStandardColorV1)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
			Resource.SourceTextures[iLane]->GetDesc(&SrvDesc);
			const uint32_t iDeclaredChannelMask =
				StandardColorSourceChannelMask(Lane.strSourceChannel);
			const uint32_t iAvailableChannelMask =
				StandardColorSrvChannelMask(SrvDesc.Format);
			const bool_t bExpectedSrgb = Lane.eColorSpace ==
				EFFECT_TEXTURE_COLOR_SPACE::SRGB;
			if (0u == iDeclaredChannelMask || 0u == iAvailableChannelMask ||
				(iDeclaredChannelMask & iAvailableChannelMask) !=
					iDeclaredChannelMask ||
				Is_StandardColorSrgbFormat(SrvDesc.Format) != bExpectedSrgb)
			{
				strOutError =
					"StandardColorV1 DDS channel/color-space contract changed: " +
					Lane.strLaneId;
				return false;
			}
		}
		D3D11_SAMPLER_DESC D3dSampler{};
		if (!Try_ToD3dSampler(Lane.Sampler, D3dSampler) ||
			FAILED(m_pDevice->CreateSamplerState(
				&D3dSampler, &Resource.RuntimeMaterialV2Samplers[iLane])))
		{
			strOutError = "Authored material sampler stage failed: " +
				Lane.strLaneId;
			return false;
		}
		D3D11_SAMPLER_DESC Readback{};
		Resource.RuntimeMaterialV2Samplers[iLane]->GetDesc(&Readback);
		EFFECT_MATERIAL_SAMPLER_DESC AuthoredReadback;
		if (!Try_ToAuthoredSampler(Readback, AuthoredReadback) ||
			AuthoredReadback.eFilter != Lane.Sampler.eFilter ||
			AuthoredReadback.eAddressU != Lane.Sampler.eAddressU ||
			AuthoredReadback.eAddressV != Lane.Sampler.eAddressV ||
			AuthoredReadback.eAddressW != Lane.Sampler.eAddressW ||
			AuthoredReadback.fMipLodBias != Lane.Sampler.fMipLodBias ||
			AuthoredReadback.iMaxAnisotropy != Lane.Sampler.iMaxAnisotropy ||
			AuthoredReadback.eComparison != Lane.Sampler.eComparison ||
			AuthoredReadback.vBorderColor.x != Lane.Sampler.vBorderColor.x ||
			AuthoredReadback.vBorderColor.y != Lane.Sampler.vBorderColor.y ||
			AuthoredReadback.vBorderColor.z != Lane.Sampler.vBorderColor.z ||
			AuthoredReadback.vBorderColor.w != Lane.Sampler.vBorderColor.w ||
			AuthoredReadback.fMinLod != Lane.Sampler.fMinLod ||
			AuthoredReadback.fMaxLod != Lane.Sampler.fMaxLod)
		{
			strOutError = "Authored material sampler readback changed: " +
				Lane.strLaneId;
			return false;
		}
		Resource.MaterialExecutionLanes[iLane] = Lane;
		Resource.iSourceTextureMask |= 1u << Lane.iTextureRegister;
		if (Lane.Sampler.eAddressU ==
			EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP)
		{
			Resource.iSourceTextureClampUMask |= 1u << Lane.iTextureRegister;
		}
		if (Lane.Sampler.eAddressV ==
			EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP)
		{
			Resource.iSourceTextureClampVMask |= 1u << Lane.iTextureRegister;
		}
	}

	const auto IsFinite4 = [](const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	};
	if (Execution.iScalarCount > Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u ||
		Execution.Scalars.size() != Execution.iScalarCount ||
		Execution.iVectorCount > Resource.RuntimeMaterialV2Vectors.size() ||
		Execution.Vectors.size() != Execution.iVectorCount)
	{
		strOutError = "Authored material packed parameter count is invalid: " +
			Element.strElementId;
		return false;
	}
	Resource.RuntimeMaterialV2ScalarBlocks.fill(float4_t{});
	std::array<bool_t, 52u> ScalarSeen{};
	for (const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar : Execution.Scalars)
	{
		if (Scalar.iPackedIndex >= Execution.iScalarCount ||
			ScalarSeen[Scalar.iPackedIndex] || !std::isfinite(Scalar.fValue))
		{
			strOutError = "Authored material scalar packing is invalid: " +
				Element.strElementId;
			return false;
		}
		ScalarSeen[Scalar.iPackedIndex] = true;
		f32_t* pBlock = &Resource.RuntimeMaterialV2ScalarBlocks[
			Scalar.iPackedIndex / 4u].x;
		pBlock[Scalar.iPackedIndex % 4u] = Scalar.fValue;
	}
	Resource.RuntimeMaterialV2Vectors.fill(float4_t{});
	std::array<bool_t, 3u> VectorSeen{};
	for (const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Vector : Execution.Vectors)
	{
		if (Vector.iPackedIndex >= Execution.iVectorCount ||
			VectorSeen[Vector.iPackedIndex] || !IsFinite4(Vector.vValue))
		{
			strOutError = "Authored material vector packing is invalid: " +
				Element.strElementId;
			return false;
		}
		VectorSeen[Vector.iPackedIndex] = true;
		Resource.RuntimeMaterialV2Vectors[Vector.iPackedIndex] = Vector.vValue;
	}

	Resource.iRuntimeMaterialV2DynamicConsumedMask = Execution.iDynamicConsumedMask;
	Resource.iRuntimeMaterialV2DynamicSuppressedMask = Execution.iDynamicSuppressedMask;
	Resource.iRuntimeMaterialV2ParticleColorPolicy = Execution.iParticleColorPolicy;
	Resource.iRuntimeMaterialV2ParticleColorConsumedMask =
		Execution.iParticleColorConsumedMask;
	Resource.iRuntimeMaterialV2ParticleColorSuppressedMask =
		Execution.iParticleColorSuppressedMask;
	Resource.iRuntimeMaterialV2ScalarCount = Execution.iScalarCount;
	Resource.iRuntimeMaterialV2VectorCount = Execution.iVectorCount;
	Resource.iRuntimeMaterialV2InputCount = Execution.iInputCount;
	Resource.RuntimeMaterialV2InputConsumedMask = Execution.InputConsumedMask;
	Resource.RuntimeMaterialV2InputSuppressedMask = Execution.InputSuppressedMask;
	Resource.RuntimeMaterialV2VectorComponentConsumedMask =
		Execution.VectorComponentConsumedMask;
	Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
		Execution.VectorComponentSuppressedMask;
	Resource.iRuntimeMaterialV2StaticInputCount = Execution.iStaticInputCount;
	Resource.iRuntimeMaterialV2StaticSelectedMask = Execution.iStaticSelectedMask;
	Resource.iRuntimeMaterialV2StaticConsumedMask = Execution.iStaticConsumedMask;
	Resource.iRuntimeMaterialV2StaticSuppressedMask = Execution.iStaticSuppressedMask;
	Resource.iRuntimeMaterialV2RenderInputCount = Execution.iRenderInputCount;
	Resource.iRuntimeMaterialV2RenderConsumedMask = Execution.iRenderConsumedMask;
	Resource.iRuntimeMaterialV2RenderSuppressedMask = Execution.iRenderSuppressedMask;
	Resource.iReconstructedMaterialEvaluatorEnabled = 0u;
	Resource.iReconstructedMaterialFeatureMask = 0u;
	Resource.bSourceMaterialFallbackBlocked = false;

	if (Execution.eBackend == EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
	{
		if (Execution.ArtistParameters.size() > Resource.ArtistVisualV4Params.size() ||
			Execution.Colors.size() > Resource.ArtistVisualV4Colors.size())
		{
			strOutError = "ArtistVisualV4 parameter count is invalid: " +
				Element.strElementId;
			return false;
		}
		Resource.ArtistVisualV4Params.fill(float4_t{});
		Resource.ArtistVisualV4Colors.fill(float4_t{});
		std::array<bool_t, 8u> ParamSeen{};
		for (const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Param :
			Execution.ArtistParameters)
		{
			if (Param.iPackedIndex >= Resource.ArtistVisualV4Params.size() ||
				ParamSeen[Param.iPackedIndex] || !IsFinite4(Param.vValue))
			{
				strOutError = "ArtistVisualV4 parameter packing is invalid: " +
					Element.strElementId;
				return false;
			}
			ParamSeen[Param.iPackedIndex] = true;
			Resource.ArtistVisualV4Params[Param.iPackedIndex] = Param.vValue;
		}
		std::array<bool_t, 2u> ColorSeen{};
		for (const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Color : Execution.Colors)
		{
			if (Color.iPackedIndex >= Resource.ArtistVisualV4Colors.size() ||
				ColorSeen[Color.iPackedIndex] || !IsFinite4(Color.vValue))
			{
				strOutError = "ArtistVisualV4 color packing is invalid: " +
					Element.strElementId;
				return false;
			}
			ColorSeen[Color.iPackedIndex] = true;
			Resource.ArtistVisualV4Colors[Color.iPackedIndex] = Color.vValue;
		}
		Resource.iArtistVisualV4Opcode = Execution.iOpcode;
		Resource.iArtistVisualV4TextureMask = Execution.iTextureMask;
		Resource.iRuntimeMaterialV2Enabled = 0u;
		Resource.iRuntimeMaterialV2Opcode = 0u;
		Resource.iRuntimeMaterialV2TextureLaneCount = 0u;
		Resource.iRuntimeMaterialV2TextureMask = 0u;
	}
	else if (bStandardColorV1)
	{
		const EFFECT_STANDARD_COLOR_V1_DESC& Packet =
			Execution.StandardColorV1;
		const auto FindLane = [&Execution](const std::string_view strLaneId,
			uint32_t& iOutLane)
		{
			for (uint32_t iLane = 0u;
				iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (Execution.TextureLanes[iLane].strLaneId == strLaneId)
				{
					iOutLane =
						Execution.TextureLanes[iLane].iTextureRegister;
					return true;
				}
			}
			return false;
		};
		uint32_t iBaseLane = UINT32_MAX;
		uint32_t iCoverageLane = UINT32_MAX;
		uint32_t iDissolveLane = UINT32_MAX;
		const uint32_t iBaseChannel = static_cast<uint32_t>(
			Packet.eBaseRadianceChannel);
		const uint32_t iCoverageChannel = static_cast<uint32_t>(
			Packet.eCoverageChannel);
		const bool_t bHasDissolve = Packet.eDissolveMode ==
			EFFECT_STANDARD_COLOR_DISSOLVE_MODE::LANE_THRESHOLD;
		if (!Execution.ArtistParameters.empty() || !Execution.Colors.empty() ||
			Execution.iScalarCount != 0u || Execution.iVectorCount != 0u ||
			!Execution.Scalars.empty() || !Execution.Vectors.empty() ||
			!FindLane(Packet.strBaseRadianceLaneId, iBaseLane) ||
			!FindLane(Packet.strCoverageLaneId, iCoverageLane) ||
			(bHasDissolve &&
			 !FindLane(Packet.strDissolveLaneId, iDissolveLane)) ||
			iBaseLane >= Execution.iTextureLaneCount ||
			iCoverageLane >= Execution.iTextureLaneCount ||
			(bHasDissolve && iDissolveLane >= Execution.iTextureLaneCount) ||
			0u == StandardColorChannelMask(Packet.eBaseRadianceChannel) ||
			0u == StandardColorChannelMask(Packet.eCoverageChannel) ||
			(bHasDissolve &&
			 0u == StandardColorChannelMask(Packet.eDissolveChannel)) ||
			Packet.eLifetimeEnvelope !=
				EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::CARRIER_ALPHA ||
			Packet.eMissingLanePolicy !=
				EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::FAIL_CLOSED ||
			Packet.eEmissiveMode >=
				EFFECT_STANDARD_COLOR_EMISSIVE_MODE::END ||
			Packet.eDissolveMode >= EFFECT_STANDARD_COLOR_DISSOLVE_MODE::END ||
			!std::isfinite(Packet.fDissolveSoftness) ||
			Packet.fDissolveSoftness < 0.f || Packet.fDissolveSoftness > 1.f)
		{
			strOutError = "StandardColorV1 typed packet cannot be staged: " +
				Element.strElementId;
			return false;
		}
		const uint32_t iRequiredMask = (1u << iBaseLane) |
			(1u << iCoverageLane) |
			(bHasDissolve ? (1u << iDissolveLane) : 0u);
		if (iRequiredMask != Execution.iTextureMask ||
			(!bHasDissolve &&
			 (!Packet.strDissolveLaneId.empty() ||
			  Packet.eDissolveChannel != EFFECT_STANDARD_COLOR_CHANNEL::INVALID ||
			  Packet.fDissolveSoftness != 0.f)))
		{
			strOutError = "StandardColorV1 required-lane closure changed: " +
				Element.strElementId;
			return false;
		}

		Resource.iArtistVisualV4Opcode = 0u;
		Resource.iArtistVisualV4TextureMask = 0u;
		Resource.iRuntimeMaterialV2Enabled = 0u;
		Resource.iRuntimeMaterialV2Opcode = 0u;
		Resource.iRuntimeMaterialV2TextureLaneCount = 0u;
		Resource.iRuntimeMaterialV2TextureMask = 0u;
		Resource.iStandardColorV1Enabled = 1u;
		Resource.StandardColorV1Header = {
			Packet.iPacketVersion, Execution.iOpcode,
			Execution.iTextureLaneCount, Execution.iTextureMask };
		Resource.StandardColorV1BaseCoverage = {
			iBaseLane, iBaseChannel, iCoverageLane, iCoverageChannel };
		Resource.StandardColorV1Dissolve = {
			static_cast<uint32_t>(Packet.eDissolveMode),
			bHasDissolve ? iDissolveLane : UINT32_MAX,
			static_cast<uint32_t>(Packet.eDissolveChannel),
			static_cast<uint32_t>(Packet.eMissingLanePolicy) };
		Resource.StandardColorV1Policies = {
			static_cast<uint32_t>(Packet.eEmissiveMode),
			static_cast<uint32_t>(Packet.eLifetimeEnvelope),
			iRequiredMask, Resource.iSourceTextureMask };
		Resource.vStandardColorV1Scalars = {
			Packet.fDissolveSoftness, 0.f, 0.f, 0.f };
		Resource.StandardColorV1 = Packet;
	}
	else
	{
		if (!Execution.ArtistParameters.empty() || !Execution.Colors.empty())
		{
			strOutError = "RuntimeMaterialV2 carries Artist-only parameters: " +
				Element.strElementId;
			return false;
		}
		Resource.iArtistVisualV4Opcode = 0u;
		Resource.iArtistVisualV4TextureMask = 0u;
		Resource.iRuntimeMaterialV2Enabled = 1u;
		Resource.iRuntimeMaterialV2Opcode = Execution.iOpcode;
		Resource.iRuntimeMaterialV2TextureLaneCount = Execution.iTextureLaneCount;
		Resource.iRuntimeMaterialV2TextureMask = Execution.iTextureMask;
	}
	return true;
}

bool_t Client::CEffectDocumentRenderer::Build_MaterialExecutionSnapshot(
	const EFFECT_ELEMENT_DESC& Element,
	const ELEMENT_RESOURCE& Resource,
	EFFECT_MATERIAL_EXECUTION_DESC& OutSnapshot,
	std::string& strOutError) const
{
	OutSnapshot = {};
	const bool_t bRuntime = 0u != Resource.iRuntimeMaterialV2Enabled;
	const bool_t bArtist = 0u != Resource.iArtistVisualV4Opcode;
	const bool_t bStandard = 0u != Resource.iStandardColorV1Enabled;
	const uint32_t iBackendCount = static_cast<uint32_t>(bRuntime) +
		static_cast<uint32_t>(bArtist) + static_cast<uint32_t>(bStandard);
	if (0u == iBackendCount)
		return false;
	if (1u != iBackendCount)
	{
		strOutError = "Prepared material selected multiple typed backends: " +
			Element.strElementId;
		return false;
	}
	EFFECT_MATERIAL_EXECUTION_DESC Staged;
	Staged.bEnabled = true;
	Staged.iVersion = 1u;
	Staged.eBackend = bStandard ?
		EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 :
		(bArtist ? EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4 :
		(Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
		 Resource.iRuntimeMaterialV2Opcode == 14u ?
			EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL :
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2));
	Staged.iOpcode = bStandard ? Resource.StandardColorV1Header[1u] :
		(bArtist ? Resource.iArtistVisualV4Opcode :
			Resource.iRuntimeMaterialV2Opcode);
	Staged.iPassIndex = Select_Pass(Element.Material.eRenderProfile);
	Staged.iStencilReference = 0u;
	switch (Element.Material.eRenderProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		Staged.strRasterizerState = "RS_Default";
		Staged.strDepthStencilState = "DSS_Default";
		Staged.strBlendState = "BS_EffectOpaque";
		break;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
		Staged.strRasterizerState = "RS_Cull_None";
		Staged.strDepthStencilState = bStandard &&
			Element.eKind == EFFECT_ELEMENT_KIND::DECAL ?
			"DSS_ZNone" : "DSS_ReadOnly";
		Staged.strBlendState = "BS_EffectAlpha";
		break;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
		Staged.strRasterizerState = "RS_Cull_None";
		Staged.strDepthStencilState = "DSS_ReadOnly";
		Staged.strBlendState = "BS_EffectAdditive";
		break;
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
		Staged.strRasterizerState = "RS_Default";
		Staged.strDepthStencilState = "DSS_ReadOnly";
		Staged.strBlendState = "BS_EffectAlpha";
		break;
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
		Staged.strRasterizerState = "RS_Default";
		Staged.strDepthStencilState = "DSS_ReadOnly";
		Staged.strBlendState = "BS_EffectAdditive";
		break;
	case EFFECT_RENDER_PROFILE::END:
	default:
		strOutError = "Prepared material has no authored render state: " +
			Element.strElementId;
		return false;
	}
	Staged.iTextureMask = bStandard ? Resource.StandardColorV1Header[3u] :
		(bArtist ? Resource.iArtistVisualV4TextureMask :
			Resource.iRuntimeMaterialV2TextureMask);
	Staged.iTextureLaneCount = std::popcount(Staged.iTextureMask);
	if ((bStandard &&
		 (Resource.StandardColorV1Header[0u] != 1u ||
		  Resource.StandardColorV1Header[1u] != 1u ||
		  Resource.StandardColorV1Header[2u] != Staged.iTextureLaneCount)) ||
		(!bArtist && !bStandard && Staged.iTextureLaneCount !=
			Resource.iRuntimeMaterialV2TextureLaneCount) ||
		Staged.iTextureLaneCount > Resource.MaterialExecutionLanes.size() ||
		Staged.iTextureMask != (Staged.iTextureLaneCount == 0u ? 0u :
			((1u << Staged.iTextureLaneCount) - 1u)) ||
		Resource.iSourceTextureMask != Staged.iTextureMask)
	{
		strOutError = "Prepared material texture mask cannot be authored: " +
			Element.strElementId;
		return false;
	}
	for (size_t iLane = 0u; iLane < Staged.iTextureLaneCount; ++iLane)
	{
		if (!Resource.MaterialExecutionLanes[iLane].has_value())
		{
			strOutError = "Prepared material lane metadata is missing: " +
				Element.strElementId + "/" + std::to_string(iLane);
			return false;
		}
		Staged.TextureLanes.push_back(*Resource.MaterialExecutionLanes[iLane]);
	}
	if (bStandard)
	{
		Staged.StandardColorV1 = Resource.StandardColorV1;
		const EFFECT_STANDARD_COLOR_V1_DESC& Packet = Staged.StandardColorV1;
		const bool_t bHasDissolve = Packet.eDissolveMode ==
			EFFECT_STANDARD_COLOR_DISSOLVE_MODE::LANE_THRESHOLD;
		const auto FindLaneRegister = [&Staged](
			const std::string_view strLaneId, uint32_t& iOutRegister)
		{
			const auto Iterator = std::find_if(Staged.TextureLanes.begin(),
				Staged.TextureLanes.end(), [strLaneId](const auto& Lane)
				{ return Lane.strLaneId == strLaneId; });
			if (Iterator == Staged.TextureLanes.end())
				return false;
			iOutRegister = Iterator->iTextureRegister;
			return true;
		};
		uint32_t iExpectedBaseLane = UINT32_MAX;
		uint32_t iExpectedCoverageLane = UINT32_MAX;
		uint32_t iExpectedDissolveLane = UINT32_MAX;
		if (!FindLaneRegister(
				Packet.strBaseRadianceLaneId, iExpectedBaseLane) ||
			!FindLaneRegister(Packet.strCoverageLaneId, iExpectedCoverageLane) ||
			(bHasDissolve && !FindLaneRegister(
				Packet.strDissolveLaneId, iExpectedDissolveLane)) ||
			Resource.StandardColorV1BaseCoverage[0u] != iExpectedBaseLane ||
			Resource.StandardColorV1BaseCoverage[1u] !=
				static_cast<uint32_t>(Packet.eBaseRadianceChannel) ||
			Resource.StandardColorV1BaseCoverage[2u] != iExpectedCoverageLane ||
			Resource.StandardColorV1BaseCoverage[3u] !=
				static_cast<uint32_t>(Packet.eCoverageChannel) ||
			Resource.StandardColorV1Dissolve[0u] !=
				static_cast<uint32_t>(Packet.eDissolveMode) ||
			Resource.StandardColorV1Dissolve[1u] != iExpectedDissolveLane ||
			Resource.StandardColorV1Dissolve[2u] !=
				static_cast<uint32_t>(Packet.eDissolveChannel) ||
			Resource.StandardColorV1Dissolve[3u] !=
				static_cast<uint32_t>(Packet.eMissingLanePolicy) ||
			Resource.StandardColorV1Policies[0u] !=
				static_cast<uint32_t>(Packet.eEmissiveMode) ||
			Resource.StandardColorV1Policies[1u] !=
				static_cast<uint32_t>(Packet.eLifetimeEnvelope) ||
			Resource.StandardColorV1Policies[2u] != Staged.iTextureMask ||
			Resource.StandardColorV1Policies[3u] != Staged.iTextureMask ||
			Resource.vStandardColorV1Scalars.x != Packet.fDissolveSoftness ||
			Resource.vStandardColorV1Scalars.y != 0.f ||
			Resource.vStandardColorV1Scalars.z != 0.f ||
			Resource.vStandardColorV1Scalars.w != 0.f)
		{
			strOutError = "Prepared StandardColorV1 GPU packet changed: " +
				Element.strElementId;
			return false;
		}
	}
	Staged.iDynamicConsumedMask = Resource.iRuntimeMaterialV2DynamicConsumedMask;
	Staged.iDynamicSuppressedMask = Resource.iRuntimeMaterialV2DynamicSuppressedMask;
	Staged.iParticleColorPolicy = Resource.iRuntimeMaterialV2ParticleColorPolicy;
	Staged.iParticleColorConsumedMask =
		Resource.iRuntimeMaterialV2ParticleColorConsumedMask;
	Staged.iParticleColorSuppressedMask =
		Resource.iRuntimeMaterialV2ParticleColorSuppressedMask;
	Staged.iScalarCount = Resource.iRuntimeMaterialV2ScalarCount;
	Staged.iVectorCount = Resource.iRuntimeMaterialV2VectorCount;
	Staged.iInputCount = Resource.iRuntimeMaterialV2InputCount;
	Staged.InputConsumedMask = Resource.RuntimeMaterialV2InputConsumedMask;
	Staged.InputSuppressedMask = Resource.RuntimeMaterialV2InputSuppressedMask;
	Staged.VectorComponentConsumedMask =
		Resource.RuntimeMaterialV2VectorComponentConsumedMask;
	Staged.VectorComponentSuppressedMask =
		Resource.RuntimeMaterialV2VectorComponentSuppressedMask;
	Staged.iStaticInputCount = Resource.iRuntimeMaterialV2StaticInputCount;
	Staged.iStaticSelectedMask = Resource.iRuntimeMaterialV2StaticSelectedMask;
	Staged.iStaticConsumedMask = Resource.iRuntimeMaterialV2StaticConsumedMask;
	Staged.iStaticSuppressedMask = Resource.iRuntimeMaterialV2StaticSuppressedMask;
	Staged.iRenderInputCount = Resource.iRuntimeMaterialV2RenderInputCount;
	Staged.iRenderConsumedMask = Resource.iRuntimeMaterialV2RenderConsumedMask;
	Staged.iRenderSuppressedMask = Resource.iRuntimeMaterialV2RenderSuppressedMask;
	if (Staged.iScalarCount > Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u ||
		Staged.iVectorCount > Resource.RuntimeMaterialV2Vectors.size())
	{
		strOutError = "Prepared material packed parameter count is invalid: " +
			Element.strElementId;
		return false;
	}
	for (uint32_t iScalar = 0u; iScalar < Staged.iScalarCount; ++iScalar)
	{
		const f32_t* pBlock = &Resource.RuntimeMaterialV2ScalarBlocks[
			iScalar / 4u].x;
		Staged.Scalars.push_back({ "scalar." + std::to_string(iScalar),
			iScalar, pBlock[iScalar % 4u] });
	}
	for (uint32_t iVector = 0u; iVector < Staged.iVectorCount; ++iVector)
	{
		Staged.Vectors.push_back({ "vector." + std::to_string(iVector),
			iVector, Resource.RuntimeMaterialV2Vectors[iVector] });
	}
	if (bArtist)
	{
		for (uint32_t iParam = 0u;
			iParam < Resource.ArtistVisualV4Params.size(); ++iParam)
		{
			Staged.ArtistParameters.push_back({
				"artist.param." + std::to_string(iParam), iParam,
				Resource.ArtistVisualV4Params[iParam] });
		}
		for (uint32_t iColor = 0u;
			iColor < Resource.ArtistVisualV4Colors.size(); ++iColor)
		{
			Staged.Colors.push_back({
				"artist.color." + std::to_string(iColor), iColor,
				Resource.ArtistVisualV4Colors[iColor] });
		}
	}
	OutSnapshot = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_VisualProgramAdapter(
	const EFFECT_VISUAL_PROGRAM_ROW& Row,
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_RESOURCE& Resource,
	std::string& strOutError,
	PREWARM_ASSET_CACHE* pSharedAssets) const
{
	if (Row.eDisposition !=
			EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
		Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE ||
		Row.strAdapterId != "local-decal-rt0-bounded-v1" ||
		Row.strPacketLayout != "LOCAL_DECAL_RT0_SIX_SRV_V1" ||
		!Row.TargetIdentity.has_value() ||
		Row.TargetIdentity->strTargetElementId != Element.strElementId ||
		Element.eKind != EFFECT_ELEMENT_KIND::DECAL ||
		Element.Renderer.eType != EFFECT_RENDERER_TYPE::DECAL_PARTICLE ||
		Element.Material.eRenderProfile !=
			EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
		!Row.LocalDecalPacket.has_value())
	{
		strOutError =
			"Visual-program LocalDecal selector/carrier identity is invalid: " +
			Element.strElementId;
		return false;
	}
	const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET& Packet =
		*Row.LocalDecalPacket;
	if (Packet.iPacketVersion != 1u ||
		Packet.strAdapterId != Row.strAdapterId ||
		!Packet.bBoundedSemanticReplay || Packet.bNativeExecution ||
		Packet.bNativeVertexFactoryAdmitted || Packet.bNativeMrtAdmitted ||
		Packet.strRuntimeCarrier != "EFFECT_TYPED_DECAL_PROJECTOR_RECT_V1" ||
		Packet.strRenderProfile != "ALPHA_ONE_SIDED_DEPTH_READ" ||
		Packet.iPassIndex != Select_Pass(Element.Material.eRenderProfile) ||
		Packet.strRasterizerState != "RS_Default" ||
		Packet.strDepthStencilState != "DSS_ReadOnly" ||
		Packet.strBlendState != "BS_EffectAlpha" ||
		Packet.iStencilReference != 0u || Packet.iOpcode != 14u ||
		Packet.iTextureLaneCount != 6u || Packet.iTextureMask != 0x3fu ||
		Packet.iDynamicConsumedMask != 0u ||
		Packet.iDynamicSuppressedMask != 0x0fu ||
		Packet.iParticleColorPolicy != 0u ||
		Packet.iParticleColorConsumedMask != 0u ||
		Packet.iParticleColorSuppressedMask != 0u ||
		Packet.InputConsumedMask != std::array<uint32_t, 2u>{ 0x820ec1ffu, 1u } ||
		Packet.InputSuppressedMask != std::array<uint32_t, 2u>{ 0x7df13e00u, 0u } ||
		Packet.VectorComponentConsumedMask !=
			std::array<uint32_t, 3u>{ 0x0fu, 0x0fu, 0u } ||
		Packet.VectorComponentSuppressedMask !=
			std::array<uint32_t, 3u>{ 0u, 0u, 0x0fu } ||
		Packet.iStaticSelectedMask != 0x3fffbu ||
		Packet.iStaticConsumedMask != 0x3ffffu ||
		Packet.iStaticSuppressedMask != 0u ||
		Packet.iRenderConsumedMask != 0x03u ||
		Packet.iRenderSuppressedMask != 0x3cu ||
		Packet.Inputs.size() != 33u || Packet.StaticBindings.size() != 18u ||
		Packet.RenderBindings.size() != 6u || Row.Resources.size() != 6u ||
		Resource.iSourceTextureMask != 0u || Packet.strPacketSha256.empty())
	{
		strOutError = "Visual-program LocalDecal packet contract is invalid: " +
			Element.strElementId;
		return false;
	}

	const auto AssignFloat = [](const double Source, f32_t& Target)
	{
		if (!std::isfinite(Source) ||
			Source < -(std::numeric_limits<f32_t>::max)() ||
			Source > (std::numeric_limits<f32_t>::max)())
			return false;
		Target = static_cast<f32_t>(Source);
		return std::isfinite(Target);
	};
	for (size_t i = 0u; i < Packet.PackedScalars.size(); ++i)
	{
		float4_t& Block = Resource.RuntimeMaterialV2ScalarBlocks[i / 4u];
		f32_t* Components = &Block.x;
		if (!AssignFloat(Packet.PackedScalars[i], Components[i % 4u]))
		{
			strOutError = "Visual-program LocalDecal scalar is invalid: " +
				Element.strElementId;
			return false;
		}
	}
	for (size_t i = 0u; i < Packet.PackedVectors.size(); ++i)
	{
		const std::array<double, 4u>& Source = Packet.PackedVectors[i];
		float4_t& Target = Resource.RuntimeMaterialV2Vectors[i];
		if (!AssignFloat(Source[0u], Target.x) ||
			!AssignFloat(Source[1u], Target.y) ||
			!AssignFloat(Source[2u], Target.z) ||
			!AssignFloat(Source[3u], Target.w))
		{
			strOutError = "Visual-program LocalDecal vector is invalid: " +
				Element.strElementId;
			return false;
		}
	}

	for (size_t iLane = 0u; iLane < Packet.Srvs.size(); ++iLane)
	{
		const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Lane = Packet.Srvs[iLane];
		const EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& ResourceRow =
			Row.Resources[iLane];
		const std::string ExpectedTextureRegister = "t" + std::to_string(iLane);
		const std::string ExpectedSamplerRegister = "s" +
			std::to_string(5u + iLane);
		const DXGI_FORMAT LinearFormat = Lane.strLinearFormat == "BC1_UNORM" ?
			DXGI_FORMAT_BC1_UNORM :
			(Lane.strLinearFormat == "BC3_UNORM" ?
				DXGI_FORMAT_BC3_UNORM : DXGI_FORMAT_UNKNOWN);
		const DXGI_FORMAT ExpectedFormat = Lane.bSrgb ?
			(LinearFormat == DXGI_FORMAT_BC1_UNORM ? DXGI_FORMAT_BC1_UNORM_SRGB :
				(LinearFormat == DXGI_FORMAT_BC3_UNORM ?
					DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_UNKNOWN)) : LinearFormat;
		if (Lane.strShaderRegister != ExpectedTextureRegister ||
			Lane.strRuntimeSamplerRegister != ExpectedSamplerRegister ||
			ResourceRow.strRole != Lane.strRole ||
			ResourceRow.strAssetId != Lane.strAssetId ||
			ResourceRow.strRawSha256 != Lane.strRawSha256 ||
			ResourceRow.iByteCount != Lane.iByteCount ||
			ResourceRow.strShaderRegister != Lane.strShaderRegister ||
			ResourceRow.strSourceChannel != Lane.strSourceChannel ||
			Lane.strSamplerPolicy != "LINEAR_CLAMP_UVW_BOUNDED_V1" ||
			Lane.strAssetId.empty() || Lane.strRawSha256.empty() ||
			Lane.iByteCount == 0u || Lane.iWidth == 0u || Lane.iHeight == 0u ||
			Lane.iMipCount == 0u || Lane.iArraySize == 0u ||
			ExpectedFormat == DXGI_FORMAT_UNKNOWN ||
			nullptr != Resource.SourceTextures[iLane])
		{
			strOutError = "Visual-program LocalDecal SRV descriptor is invalid: " +
				Element.strElementId;
			return false;
		}

		const std::string CacheKey = "visual-local-decal\n" + Lane.strAssetId +
			"\n" + Lane.strRawSha256 + (Lane.bSrgb ? "\nSRGB" : "\nLINEAR");
		ComPtr<ID3D11ShaderResourceView> Texture;
		if (nullptr != pSharedAssets)
		{
			const auto Cached = pSharedAssets->Textures.find(CacheKey);
			if (Cached != pSharedAssets->Textures.end())
				Texture = Cached->second;
		}
		if (nullptr == Texture)
		{
			std::filesystem::path TexturePath;
			std::vector<uint8_t> Bytes;
			if (!Read_ReconstructedAssetBytes(Lane.strAssetId, Lane.iByteCount,
				Lane.strRawSha256, TexturePath, Bytes, strOutError) ||
				FAILED(DirectX::CreateDDSTextureFromMemoryEx(m_pDevice.Get(),
					Bytes.data(), Bytes.size(), 0u, D3D11_USAGE_DEFAULT,
					D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
					Lane.bSrgb ? DirectX::DDS_LOADER_FORCE_SRGB :
						DirectX::DDS_LOADER_IGNORE_SRGB,
					nullptr, &Texture)))
			{
				if (strOutError.empty())
					strOutError = "Visual-program LocalDecal DDS creation failed: " +
						Lane.strAssetId;
				return false;
			}
			if (nullptr != pSharedAssets)
				pSharedAssets->Textures.emplace(CacheKey, Texture);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
		Texture->GetDesc(&SrvDesc);
		ComPtr<ID3D11Resource> D3dResource;
		Texture->GetResource(D3dResource.GetAddressOf());
		ComPtr<ID3D11Texture2D> Texture2D;
		D3D11_TEXTURE2D_DESC TextureDesc{};
		if (nullptr == D3dResource || FAILED(D3dResource.As(&Texture2D)) ||
			nullptr == Texture2D)
		{
			strOutError = "Visual-program LocalDecal DDS is not Texture2D: " +
				Lane.strAssetId;
			return false;
		}
		Texture2D->GetDesc(&TextureDesc);
		if (SrvDesc.Format != ExpectedFormat ||
			SrvDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
			SrvDesc.Texture2D.MostDetailedMip != 0u ||
			SrvDesc.Texture2D.MipLevels != Lane.iMipCount ||
			TextureDesc.Width != Lane.iWidth || TextureDesc.Height != Lane.iHeight ||
			TextureDesc.MipLevels != Lane.iMipCount ||
			TextureDesc.ArraySize != Lane.iArraySize ||
			TextureDesc.Format != ExpectedFormat)
		{
			strOutError = "Visual-program LocalDecal DDS descriptor changed: " +
				Lane.strAssetId;
			return false;
		}

		D3D11_SAMPLER_DESC SamplerDesc{};
		SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.MaxAnisotropy = 1u;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SamplerDesc.MinLOD = 0.f;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		ComPtr<ID3D11SamplerState> Sampler;
		if (FAILED(m_pDevice->CreateSamplerState(&SamplerDesc, &Sampler)))
		{
			strOutError = "Visual-program LocalDecal sampler creation failed.";
			return false;
		}
		Resource.SourceTextures[iLane] = std::move(Texture);
		Resource.RuntimeMaterialV2Samplers[iLane] = std::move(Sampler);
		if (!Capture_MaterialExecutionLane(Resource, iLane, Lane.strAssetId,
			Lane.strRole, Lane.strSourceChannel,
			Lane.bSrgb ? EFFECT_TEXTURE_COLOR_SPACE::SRGB :
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
			Resource.RuntimeMaterialV2Samplers[iLane], strOutError))
		{
			return false;
		}
		Resource.iSourceTextureMask |= 1u << static_cast<uint32_t>(iLane);
		Resource.iSourceTextureClampUMask |= 1u << static_cast<uint32_t>(iLane);
		Resource.iSourceTextureClampVMask |= 1u << static_cast<uint32_t>(iLane);
	}

	Resource.iRuntimeMaterialV2Enabled = 1u;
	Resource.iRuntimeMaterialV2Opcode = Packet.iOpcode;
	Resource.iRuntimeMaterialV2TextureLaneCount = Packet.iTextureLaneCount;
	Resource.iRuntimeMaterialV2TextureMask = Packet.iTextureMask;
	Resource.iRuntimeMaterialV2DynamicConsumedMask = Packet.iDynamicConsumedMask;
	Resource.iRuntimeMaterialV2DynamicSuppressedMask = Packet.iDynamicSuppressedMask;
	Resource.iRuntimeMaterialV2ParticleColorPolicy = Packet.iParticleColorPolicy;
	Resource.iRuntimeMaterialV2ParticleColorConsumedMask =
		Packet.iParticleColorConsumedMask;
	Resource.iRuntimeMaterialV2ParticleColorSuppressedMask =
		Packet.iParticleColorSuppressedMask;
	Resource.iRuntimeMaterialV2ScalarCount = 22u;
	Resource.iRuntimeMaterialV2VectorCount = 3u;
	Resource.iRuntimeMaterialV2InputCount = 33u;
	Resource.RuntimeMaterialV2InputConsumedMask = Packet.InputConsumedMask;
	Resource.RuntimeMaterialV2InputSuppressedMask = Packet.InputSuppressedMask;
	Resource.RuntimeMaterialV2VectorComponentConsumedMask =
		Packet.VectorComponentConsumedMask;
	Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
		Packet.VectorComponentSuppressedMask;
	Resource.iRuntimeMaterialV2StaticInputCount = 18u;
	Resource.iRuntimeMaterialV2StaticSelectedMask = Packet.iStaticSelectedMask;
	Resource.iRuntimeMaterialV2StaticConsumedMask = Packet.iStaticConsumedMask;
	Resource.iRuntimeMaterialV2StaticSuppressedMask = Packet.iStaticSuppressedMask;
	Resource.iRuntimeMaterialV2RenderInputCount = 6u;
	Resource.iRuntimeMaterialV2RenderConsumedMask = Packet.iRenderConsumedMask;
	Resource.iRuntimeMaterialV2RenderSuppressedMask = Packet.iRenderSuppressedMask;
	Resource.iReconstructedMaterialEvaluatorEnabled = 0u;
	Resource.iReconstructedMaterialFeatureMask = 0u;
	Resource.bSourceMaterialFallbackBlocked = false;
	Resource.bOccurrenceVisualSuppressed = false;
	strOutError.clear();
	return true;
}

HRESULT Client::CEffectDocumentRenderer::Stage_ModelCueResource(
	const EFFECT_MODEL_CUE_DESC& Cue,
	MODEL_CUE_RESOURCE& OutResource,
	std::string& strOutError,
	PREWARM_ASSET_CACHE* pSharedAssets) const
{
	const matrix_t PreTransform =
		XMMatrixScaling(Cue.vAssetPreScale.x, Cue.vAssetPreScale.y,
			Cue.vAssetPreScale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.x),
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.y),
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.z));
	const std::string CacheKey = Cue.strModelAssetId + "\n" +
		Cue.strClipName + "\n" + std::to_string(Cue.vAssetPreScale.x) +
		"\n" + std::to_string(Cue.vAssetPreScale.y) + "\n" +
		std::to_string(Cue.vAssetPreScale.z) + "\n" +
		std::to_string(Cue.vAssetPreRotationDegrees.x) + "\n" +
		std::to_string(Cue.vAssetPreRotationDegrees.y) + "\n" +
		std::to_string(Cue.vAssetPreRotationDegrees.z);
	shared_ptr<Engine::CModel> Model;
	if (nullptr != pSharedAssets)
	{
		const auto Cached =
			pSharedAssets->AnimatedModelPrototypes.find(CacheKey);
		if (Cached != pSharedAssets->AnimatedModelPrototypes.end())
			Model = Cached->second;
	}
	if (nullptr == Model)
	{
		const std::filesystem::path ModelPath = CRuntimeAssetRoot::Resolve(
			std::filesystem::path(Cue.strModelAssetId));
		{
			const std::scoped_lock Lock(g_EffectRenderCacheMutex);
			++g_EffectRenderPrewarmProbe.iModelDiskLoadCount;
		}
		unique_ptr<Engine::CModel> Loaded = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::ANIM,
			ModelPath.string().c_str(), PreTransform);
		if (nullptr != Loaded)
			Model = std::move(Loaded);
	}
	if (nullptr == Model ||
		!Model->Set_Animation(Cue.strClipName.c_str(), false))
	{
		strOutError = "Animated CModel or clip load failed: " +
			Cue.strModelAssetId + " / " + Cue.strClipName;
		return E_FAIL;
	}
	const uint32_t iAnimation = Model->Get_CurrentAnimIndex();
	const f32_t fTicksPerSecond =
		Model->Get_AnimationTickPerSecond(iAnimation);
	f32_t fPosition = 0.f;
	f32_t fDurationTicks = 0.f;
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
		!Model->Get_AnimationProgress(
			iAnimation, fPosition, fDurationTicks) ||
		!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f ||
		(!Cue.bHoldLastFrame &&
		 Cue.fDurationSeconds > fDurationTicks / fTicksPerSecond + 0.001f))
	{
		strOutError = "Animated Model Cue duration exceeds its source clip: " +
			Cue.strCueId;
		return E_FAIL;
	}
	Model->Set_AnimTrackPosition(iAnimation, 0.f);
	Model->Play_Animation(0.f);
	if (nullptr != pSharedAssets)
		pSharedAssets->AnimatedModelPrototypes.emplace(CacheKey, Model);
	OutResource.pModel = std::move(Model);
	OutResource.iAnimationIndex = iAnimation;
	OutResource.fTicksPerSecond = fTicksPerSecond;
	OutResource.fDurationSeconds = fDurationTicks / fTicksPerSecond;
	return S_OK;
}

bool_t Client::CEffectDocumentRenderer::Build_PreparedDocument(
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	const EFFECT_DOCUMENT_DESC& Document,
	PREWARM_ASSET_CACHE* pSharedAssets,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection,
	std::shared_ptr<const EFFECT_DOCUMENT_DESC> pImmutableDocument,
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry) const
{
	if ((0u == iCatalogRevision && nullptr != pImmutableDocument) ||
		(0u != iCatalogRevision &&
			(nullptr == pImmutableDocument ||
			 pImmutableDocument.get() != &Document ||
			 nullptr == pMaterialProgramRegistry ||
			 pMaterialProgramRegistry->Get_CatalogRevision() !=
				iCatalogRevision)) ||
		(0u == iCatalogRevision && nullptr != pMaterialProgramRegistry))
	{
		strOutError =
			"Prepared Effect catalog immutable document identity is invalid.";
		return false;
	}
	if (nullptr != pVisualProgramProjection &&
		(!pVisualProgramProjection->Is_Valid() ||
		 pVisualProgramProjection->Get_EffectAssetId() != strEffectAssetId ||
		 &pVisualProgramProjection->Get_Document() != &Document ||
		 pVisualProgramProjection->Get_ProjectedDocumentSha256().empty()))
	{
		strOutError = "Visual-program renderer projection identity is invalid.";
		return false;
	}
	if (nullptr != pPreparation && !Validate_Artist31470ShaderRegistry())
	{
		strOutError = "Artist F shader registry invariant validation failed.";
		return false;
	}
	const bool_t bDocumentValid = nullptr == pPreparation ?
		CEffectDocumentCodec::Validate_Drawable(Document, strOutError) :
		CEffectDocumentCodec::
			Validate_Artist31470ReconstructedRuntimeDrawable(
				Document, strOutError);
	if (strEffectAssetId.empty() ||
		strEffectAssetId != Document.strEffectAssetId ||
		!bDocumentValid)
	{
		if (strOutError.empty())
			strOutError = "Prepared Effect identity is invalid.";
		return false;
	}
	auto Staged = std::make_shared<PREPARED_DOCUMENT>();
	Staged->iCatalogRevision = iCatalogRevision;
	Staged->iMaterialProgramRegistryGeneration =
		nullptr == pMaterialProgramRegistry ? 0u :
			pMaterialProgramRegistry->Get_GenerationId();
	Staged->iResourceSignature = Build_ResourceSignature(Document);
	Staged->strEffectAssetId = strEffectAssetId;
	Staged->pCatalogDocumentIdentity =
		0u == iCatalogRevision ? nullptr : &Document;
	if (0u == iCatalogRevision)
		Staged->ResourceDocument = Document;
	else
		Staged->pImmutableDocument = std::move(pImmutableDocument);
	Staged->pReconstructedRuntimePreparation = pPreparation;
	Staged->pVisualProgramProjection = pVisualProgramProjection;
	Staged->pMaterialProgramRegistry = pMaterialProgramRegistry;
	uint32_t iTrailBufferPointCapacity = 0u;
	if (!Try_ResolveTrailBufferPointCapacity(
			Document, iTrailBufferPointCapacity, strOutError))
	{
		return false;
	}
	if (0u != iTrailBufferPointCapacity)
	{
		Staged->pTrailBuffer = Engine::CVIBuffer_DynamicTrail::Create(
			m_pDevice, m_pContext, iTrailBufferPointCapacity);
		if (nullptr == Staged->pTrailBuffer)
		{
			strOutError =
				"Prepared Effect trail buffer creation failed for maxPoints=" +
				std::to_string(iTrailBufferPointCapacity) + ".";
			return false;
		}
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iMutableInstanceBufferBuildCount;
	}
	if (!CEffectPlayback::Prepare_DocumentResources(
		Document, Staged->pPlaybackResources, strOutError,
		Staged->pImmutableDocument))
	{
		return false;
	}
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		MODEL_CUE_RESOURCE Resource;
		if (FAILED(Stage_ModelCueResource(
			Cue, Resource, strOutError, pSharedAssets)) ||
			!Staged->ModelCuePrototypes.emplace(
				Cue.strCueId, std::move(Resource)).second)
		{
			if (strOutError.empty())
				strOutError = "Prepared Effect has a duplicate Model Cue.";
			return false;
		}
	}
	const auto StageReconstructedMaterialEvaluator = [&]
		(const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
			const EFFECT_ELEMENT_DESC& Element, ELEMENT_RESOURCE& Resource,
			bool_t& bOutStaged) -> bool_t
	{
		bOutStaged = false;
		if (Emitter.Row.iOrder >= ARTIST31470_EMITTER_ROW_PINS.size() ||
			Emitter.Row.strRowSha256 !=
				ARTIST31470_EMITTER_ROW_PINS[Emitter.Row.iOrder].strEmitterRowSha256)
		{
			strOutError = "Artist F emitter row identity changed: " +
				Element.strElementId;
			return false;
		}
		if (!Emitter.strMaterialOccurrenceId.has_value())
			return true;
		const auto ShaderRegistry = Find_Artist31470ShaderRegistry(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
		if (!ShaderRegistry.has_value() ||
			!Validate_Artist31470ShaderRegistryEmitterIdentity(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
				Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
		{
			strOutError = "Artist F shader registry emitter identity changed: " +
				Element.strElementId;
			return false;
		}
		const std::string_view strStableOccurrenceId =
			ShaderRegistry->strOccurrenceId;
		const bool_t bActive003RuntimeV2 = Emitter.Row.iOrder == 3u;
		const bool_t bActive004RuntimeV2 = Emitter.Row.iOrder == 4u;
		const bool_t bActive005006RuntimeV2 =
			Emitter.Row.iOrder == 5u || Emitter.Row.iOrder == 6u;
		const bool_t bActive023RuntimeV2 = Emitter.Row.iOrder == 23u;
		const bool_t bActive024RuntimeV2 = Emitter.Row.iOrder == 24u;
		const bool_t bActive027RuntimeV2 = Emitter.Row.iOrder == 27u;
		const bool_t bActive028RuntimeV2 = Emitter.Row.iOrder == 28u;
		const bool_t bActive016RuntimeV2 = Emitter.Row.iOrder == 16u;
		const bool_t bActive011RuntimeV2 = Emitter.Row.iOrder == 11u;
		const bool_t bActive020021RuntimeV2 =
			Emitter.Row.iOrder == 20u || Emitter.Row.iOrder == 21u;
		const bool_t bActive022RuntimeV2 = Emitter.Row.iOrder == 22u;
		const bool_t bActive030RuntimeV2 = Emitter.Row.iOrder == 30u;
		const bool_t bActive002019031RuntimeV2 =
			Emitter.Row.iOrder == 2u || Emitter.Row.iOrder == 19u ||
			Emitter.Row.iOrder == 31u;
		const bool_t bActive009010RuntimeV2 =
			Emitter.Row.iOrder == 9u || Emitter.Row.iOrder == 10u;
		const bool_t bRuntimeMaterialV2 = bActive003RuntimeV2 ||
			bActive004RuntimeV2 || bActive011RuntimeV2 ||
			bActive020021RuntimeV2 || bActive022RuntimeV2 ||
			bActive023RuntimeV2 || bActive024RuntimeV2 ||
			bActive027RuntimeV2 || bActive028RuntimeV2 ||
			bActive016RuntimeV2 ||
			bActive030RuntimeV2 || bActive002019031RuntimeV2 ||
			bActive005006RuntimeV2 ||
			bActive009010RuntimeV2;
		const bool_t bArtistVisualV4Candidate = ShaderRegistry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4;
		const bool_t bArtistVisualV4 = bArtistVisualV4Candidate;
		if (ShaderRegistry->eBackend ==
				EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON ||
			ShaderRegistry->eBackend == EFFECT_ARTIST31470_SHADER_BACKEND::NONE)
		{
			if ((ShaderRegistry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON) !=
				(Emitter.Row.iOrder == 17u) ||
				ShaderRegistry->bNativeSelectionAdmitted)
			{
				strOutError = "Artist F finite/suppressed registry backend changed: " +
					Element.strElementId;
				return false;
			}
			Resource.bOccurrenceVisualSuppressed = !ShaderRegistry->bDrawAdmitted;
			return true;
		}
		if (bRuntimeMaterialV2 != (ShaderRegistry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2))
		{
			strOutError = "Artist F shader registry backend changed: " +
				Element.strElementId;
			return false;
		}
		const bool_t bStandardProfile =
			Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.reconstructed-standard.v1" &&
			Resource.iSourceMaterialProfile == 0u;
		const bool_t bDirectSmokeIdentity = bActive005006RuntimeV2 &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_m_00.fx_mi.fx_m_pa_sqc_01_01_tr";
		const bool_t bDirectSmokeCarrier = bDirectSmokeIdentity &&
			((!Element.Material.SourceMaterial.bEnabled &&
				Resource.iSourceMaterialProfile == 0u) ||
				(Element.Material.SourceMaterial.bEnabled &&
				 Element.Material.SourceMaterial.strProfileId ==
					"material-recipe-533e2243c834e448" &&
				 Element.Material.SourceMaterial.strParentMaterialPath ==
					Element.Material.strSourceMaterialPath &&
				 Element.Material.SourceMaterial.eStatus ==
					EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE &&
				 ((Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
					"effect.ue3.grouped-translucent.v1" &&
					Resource.iSourceMaterialProfile == 6u) || bStandardProfile)));
		const bool_t bOneLayerVisualV4Profile = Emitter.Row.iOrder == 33u &&
			Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.one-layer-distortion.v1" &&
			Resource.iSourceMaterialProfile == 5u;
		if ((!bStandardProfile && !bDirectSmokeCarrier &&
			!bOneLayerVisualV4Profile) ||
			nullptr == pPreparation || nullptr == pPreparation->Get_Program() ||
			nullptr == pPreparation->Get_RenderResourceAuthority() ||
			strStableOccurrenceId.empty() ||
			*Emitter.strMaterialOccurrenceId != strStableOccurrenceId ||
			Resource.iSourceTextureMask != 0u)
		{
			strOutError =
				"Reconstructed common material evaluator identity is invalid: " +
				Element.strElementId;
			return false;
		}
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program =
			*pPreparation->Get_Program();
		const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY& Authority =
			*pPreparation->Get_RenderResourceAuthority();
		const auto OccurrenceIt = std::find_if(Program.MaterialOccurrences.begin(),
			Program.MaterialOccurrences.end(), [&](const auto& Row)
			{
				return Row.Row.strId == *Emitter.strMaterialOccurrenceId;
			});
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE* Occurrence =
			OccurrenceIt == Program.MaterialOccurrences.end() ? nullptr :
				&*OccurrenceIt;
		const auto RecipeIt = nullptr == Occurrence ? Program.MaterialRecipes.end() :
			std::find_if(Program.MaterialRecipes.begin(), Program.MaterialRecipes.end(),
				[&](const auto& Row)
				{
					return Row.Row.strId == Occurrence->strRecipeId;
				});
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE* Recipe =
			RecipeIt == Program.MaterialRecipes.end() ? nullptr : &*RecipeIt;
		const auto FamilyIt = nullptr == Occurrence ? Program.MaterialFamilies.end() :
			std::find_if(Program.MaterialFamilies.begin(), Program.MaterialFamilies.end(),
				[&](const auto& Row)
				{
					return Row.Row.strId == Occurrence->strFamilyId;
				});
		const EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY* Family =
			FamilyIt == Program.MaterialFamilies.end() ? nullptr : &*FamilyIt;
		const EFFECT_RECONSTRUCTED_RENDER_RECIPE_TEXTURE_BINDING* TextureDecision =
			nullptr;
		uint32_t iTextureDecisionCount = 0u;
		if (nullptr != Recipe && nullptr != Family)
		{
			for (const auto& [strDecisionId, Decision] :
				Authority.RecipeTextureBindingsById)
			{
				(void)strDecisionId;
				if (Decision.strRecipeId == Recipe->Row.strId &&
					Decision.strRecipeRowSha256 == Recipe->Row.strRowSha256 &&
					Decision.strFamilyId == Family->Row.strId &&
					Decision.strFamilyRowSha256 == Family->Row.strRowSha256)
				{
					TextureDecision = &Decision;
					++iTextureDecisionCount;
				}
			}
		}
		const bool_t bStructuralJoin = nullptr != Occurrence && nullptr != Recipe &&
			nullptr != Family &&
			Occurrence->Row.strId == strStableOccurrenceId &&
			Occurrence->Row.iOrder == Emitter.Row.iOrder &&
			Occurrence->strEmitterId == Emitter.Row.strId &&
			Occurrence->eRenderer == Emitter.eRenderer &&
			Occurrence->strFamilyId == Recipe->strFamilyId &&
			Occurrence->strEvaluatorRegistryId == Recipe->strEvaluatorRegistryId &&
			Occurrence->strEvaluatorRegistryId == Family->strEvaluatorRegistryId;
		const bool_t bLegacyEvaluatorJoin = bStructuralJoin &&
			nullptr != TextureDecision && iTextureDecisionCount == 1u &&
			TextureDecision->iFeatureMask == Family->iFeatureMask &&
			Family->iFeatureMask != 0u && Family->iEvaluatorVersion == 1u &&
			!Family->strEvaluatorId.empty() && !Family->strEvaluatorSha256.empty() &&
			Family->bCpuNumericOracleVerified &&
			Family->bHlslNumericOracleVerified &&
			(bRuntimeMaterialV2 || Recipe->NumericBindingSamples.size() == 4u);
		const auto IsArtistVisualV4Identity = [&]()
		{
			if (!bStructuralJoin)
				return false;
			struct IDENTITY final
			{
				uint32_t iOrder;
				std::string_view strRecipeId;
				std::string_view strFamilyId;
			};
			static constexpr std::array<IDENTITY, 11u> IDENTITIES = {{
				{ 0u, "material-recipe-a63f54d1380f34cc", "material-family-2174c88b2cbf6c72" },
				{ 7u, "material-recipe-96708f89604b9d71", "material-family-4e1ecdcff38a53d1" },
				{ 8u, "material-recipe-96708f89604b9d71", "material-family-4e1ecdcff38a53d1" },
				{ 12u, "material-recipe-4d8f47d8fa273079", "material-family-3919858495713fad" },
				{ 13u, "material-recipe-eca6507a183c9c39", "material-family-ce499c1d6d0fddcc" },
				{ 14u, "material-recipe-eca6507a183c9c39", "material-family-ce499c1d6d0fddcc" },
				{ 15u, "material-recipe-3d8700096e32aa5b", "material-family-0f9365f5179c729b" },
				{ 18u, "material-recipe-77d6e867e3a6ff7e", "material-family-bcd3ca469af508c1" },
				{ 25u, "material-recipe-d75fcbbbae79bd5f", "material-family-61182c380df3a6cd" },
				{ 29u, "material-recipe-e45b1f4c930b11df", "material-family-61182c380df3a6cd" },
				{ 33u, "material-recipe-3992a9772b12abf2", "material-family-344b3aeb8eb1418d" }
			}};
			const auto Identity = std::find_if(IDENTITIES.begin(), IDENTITIES.end(),
				[&](const IDENTITY& Row) { return Row.iOrder == Occurrence->Row.iOrder; });
			return Identity != IDENTITIES.end() &&
				Recipe->Row.strId == Identity->strRecipeId &&
				Family->Row.strId == Identity->strFamilyId;
		};
		const bool_t bExactJoin = bArtistVisualV4 ?
			IsArtistVisualV4Identity() :
			((bActive005006RuntimeV2 || bActive020021RuntimeV2 ||
				bActive024RuntimeV2) ?
				bStructuralJoin : bLegacyEvaluatorJoin);
		if (!bExactJoin || !Validate_Artist31470ShaderRegistryOccurrence(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
			Emitter.strSourceElementId, Emitter.strSourceEmitterPath, *Occurrence))
		{
			strOutError =
				"Reconstructed common material evaluator Program/sidecar join failed: " +
				Element.strElementId;
			return false;
		}
		const auto AssignFloat = [](const double fSource, f32_t& fTarget)
		{
			if (!std::isfinite(fSource) ||
				fSource < -(std::numeric_limits<f32_t>::max)() ||
				fSource > (std::numeric_limits<f32_t>::max)())
			{
				return false;
			}
			fTarget = static_cast<f32_t>(fSource);
			return std::isfinite(fTarget);
		};
		const auto AssignFloat4 = [&AssignFloat](
			const std::array<double, 4u>& Source, float4_t& Target)
		{
			return AssignFloat(Source[0u], Target.x) &&
				AssignFloat(Source[1u], Target.y) &&
				AssignFloat(Source[2u], Target.z) &&
				AssignFloat(Source[3u], Target.w);
		};
		if (!bRuntimeMaterialV2 && !bArtistVisualV4)
		{
			const auto& FirstSample = Recipe->NumericBindingSamples.front();
			const bool_t bConstantSamplesAgree = std::all_of(
				Recipe->NumericBindingSamples.begin(),
				Recipe->NumericBindingSamples.end(), [&](const auto& Sample)
				{
					return Sample.vUvScale == FirstSample.vUvScale &&
						Sample.vPanRotationAux == FirstSample.vPanRotationAux &&
						Sample.vColor == FirstSample.vColor &&
						Sample.vParams0 == FirstSample.vParams0 &&
						Sample.vParams1 == FirstSample.vParams1;
				});
			if (!bConstantSamplesAgree ||
				!AssignFloat(FirstSample.vUvScale[0u],
					Resource.vReconstructedUVScale.x) ||
				!AssignFloat(FirstSample.vUvScale[1u],
					Resource.vReconstructedUVScale.y) ||
				!AssignFloat4(FirstSample.vPanRotationAux,
					Resource.vReconstructedPanRotationAux) ||
				!AssignFloat4(FirstSample.vColor, Resource.vReconstructedColor) ||
				!AssignFloat4(FirstSample.vParams0, Resource.vReconstructedParams0) ||
				!AssignFloat4(FirstSample.vParams1, Resource.vReconstructedParams1))
			{
				strOutError =
					"Reconstructed common material evaluator constants are invalid: " +
					Element.strElementId;
				return false;
			}
		}

		std::vector<EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER>
			RuntimeTextureProviders;
		struct ARTIST_VISUAL_V4_TEXTURE_LANE final
		{
			std::string_view strParameterName;
			std::string_view strExpectedSourceTextureId;
			std::string_view strExpectedRuntimeAssetId;
			std::string_view strExpectedFieldId;
			std::string_view strExpectedInputRowSha256;
			std::string_view strExpectedBindingRowSha256;
			size_t iShaderLane = static_cast<size_t>(-1);
			uint64_t iExpectedDdsByteCount = 0u;
			std::string_view strExpectedDdsRawSha256;
			std::string_view strExpectedSamplerPolicyId;
			std::string_view strExpectedSamplerPolicyRowSha256;
			uint32_t iExpectedWidth = 0u;
			uint32_t iExpectedHeight = 0u;
			DXGI_FORMAT eExpectedSrvFormat = DXGI_FORMAT_UNKNOWN;
			D3D11_TEXTURE_ADDRESS_MODE eExpectedAddressU =
				D3D11_TEXTURE_ADDRESS_WRAP;
			D3D11_TEXTURE_ADDRESS_MODE eExpectedAddressV =
				D3D11_TEXTURE_ADDRESS_WRAP;
			D3D11_TEXTURE_ADDRESS_MODE eExpectedAddressW =
				D3D11_TEXTURE_ADDRESS_WRAP;
		};
		std::vector<ARTIST_VISUAL_V4_TEXTURE_LANE> ArtistVisualV4TextureLanes;
		if (bArtistVisualV4)
		{
			const auto ValidateTypedScalars = [&](const std::initializer_list<
				std::pair<std::string_view, double>> Expected)
			{
				for (const auto& [strName, fExpected] : Expected)
				{
					const auto Matches = static_cast<size_t>(std::count_if(
						Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
						[&](const auto& Row)
						{
							return Row.strRecipeId == Recipe->Row.strId &&
								Row.strNormalizedParameterName == strName;
						}));
					const auto Input = std::find_if(Program.MaterialInputs.begin(),
						Program.MaterialInputs.end(), [&](const auto& Row)
						{
							return Row.strRecipeId == Recipe->Row.strId &&
								Row.strNormalizedParameterName == strName;
						});
					const double fTolerance = (std::max)(1e-6,
						std::abs(fExpected) * 1e-6);
					if (Matches != 1u || Input == Program.MaterialInputs.end() ||
						Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
						!Input->fValue.has_value() || !std::isfinite(*Input->fValue) ||
						std::abs(*Input->fValue - fExpected) > fTolerance ||
						Input->strFieldKind != "scalar" ||
						Input->strTypedValueSha256.empty() ||
						Input->strSourceFieldValueSha256.empty())
					{
						strOutError = "Artist F visual-program V4 typed scalar changed: " +
							Occurrence->Row.strId + "/" + std::string(strName);
						return false;
					}
				}
				return true;
			};
			const auto SetVisualV4Params = [&Resource](
				const uint32_t iOpcode, const uint32_t iTextureMask,
				const std::initializer_list<float4_t> Params,
				const float4_t& Color, const float4_t& SecondaryColor)
			{
				Resource.iArtistVisualV4Opcode = iOpcode;
				Resource.iArtistVisualV4TextureMask = iTextureMask;
				size_t iParam = 0u;
				for (const float4_t& Param : Params)
					Resource.ArtistVisualV4Params[iParam++] = Param;
				Resource.ArtistVisualV4Colors[0u] = Color;
				Resource.ArtistVisualV4Colors[1u] = SecondaryColor;
			};
			switch (Occurrence->Row.iOrder)
			{
			case 0u:
			{
				/* #0 is a bounded SpriteParticle replay of the recovered RT0 PS.
				   The native VF/pass sampler map, fog and auxiliary MRT carriers are
				   deliberately not inferred. */
				if (Emitter.Row.strRowSha256 !=
						"5b7356ded7c7acd6958a0f6142e7323e15826799d463985faac4736e30810190" ||
					Emitter.strSelectedLodPath !=
						"FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_21.particlelodlevel_14" ||
					Emitter.strSelectedLodNodeId != "FX_PC_SDM_07:export:190" ||
					Emitter.strSelectedLodRecordSha256 !=
						"2ee346ca8ce8210f358365f0a59effd194641f20479135401474b9e025af0793" ||
					Occurrence->Row.strRowSha256 !=
						"16182be8a097c1b64ec5b72c111a485bef17118df5f5849d915de1553ef60250" ||
					Occurrence->strSourceOccurrenceIdentitySha256 !=
						"5462e0a2016ac5334df6dbee204993f5725f71fc2c7bfce2907279a0d8a22b66" ||
					Occurrence->strSourceOccurrenceBindingSha256 !=
						"82a4492d47393030e5f49e474e57510c1c69e2f85bc976c5da868b3dd2dd35c6" ||
					Occurrence->strBindingSha256 !=
						"b6734ab0c54bc967f5ff198e81860601f2615b51db1fca51033734d51a76dbe8" ||
					Recipe->Row.strRowSha256 !=
						"bb21a716e1a0ef338ebe7e8946ea147f01ddcd3131f42d70cdb47a8a22bbe3b3" ||
					Recipe->strSourceMaterialPath !=
						"fx_m_mi_03.fx_mi.fx_m_pa_skull_02_23_tr" ||
					Recipe->strSourceRecipeCompositionSha256 !=
						"61f5a6c0e208dab7a0d2d27565da2d93c82814007d95dfda98c8853ba84e5669" ||
					Recipe->strBindingSha256 != Occurrence->strBindingSha256 ||
					Family->Row.strRowSha256 !=
						"03dc4e8e07250da3c69d1b3d4cd146123875cc9e4248203d18d76f5b94412e2d" ||
					Family->strFamilyIdentitySha256 !=
						"823da20bb99e7fe628f0ad3211021ef5d90e9948f0cd6e707536189ddd50f18a" ||
					Family->strEvaluatorRegistryId !=
						"handler-435ee51258a1c5b857866899" ||
					Family->strEvaluatorId != "reconstructed-evaluator-b1cbee588c4324e0" ||
					Family->iEvaluatorVersion != 1u ||
					Family->strEvaluatorSha256 !=
						"fe7082da4db76d5d1a86891d62bf9405d4496857484cd5d3f8beb278f5f4b3e4" ||
					Family->strSampleProjectionSha256 !=
						"9b879f154998a4f54f80f47484d57b38154fdbdec3f2c895016dc0c01666a830" ||
					Family->iFeatureMask != 895u || !Family->bCpuNumericOracleVerified ||
					!Family->bHlslNumericOracleVerified || Recipe->InputIds.size() != 34u ||
					Recipe->StaticBindingIds.size() != 4u ||
					Recipe->RenderBindingIds.size() != 6u ||
					ShaderRegistry->strEngineEqualityStaticSetSha256 !=
						"45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4" ||
					ShaderRegistry->strRecoveredPixelShaderId !=
						"456fc57bd455014b93b97e18c9390a4f" ||
					ShaderRegistry->strRecoveredPixelDxbcSha256 !=
						"4181e26cdef6e7b9a6839e13d393305bec43460a91c9d5878a5ba975885a1700" ||
					ShaderRegistry->bNativeSelectionAdmitted ||
					Element.Material.strSourceMaterialPath != Recipe->strSourceMaterialPath ||
					Element.Material.eRenderProfile !=
						EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
				{
					strOutError = "Artist F active000 bounded shader identity changed.";
					return false;
				}

				struct CACHE_SWITCH final
				{
					std::string_view strName;
					bool_t bValue;
					std::string_view strExpressionGuid;
				};
				static constexpr std::array<CACHE_SWITCH, 10u> CACHE_SWITCHES = {{
					{ "use_alpha_channal", false, "6ef284ae97c60740b71be3552863e998" },
					{ "use_b_channal", false, "61ac8ce80f371e43a9952b270addc393" },
					{ "use_centeralpha", true, "44333c2a3d8bbc4bb66499b98d493e2e" },
					{ "use_disslove", true, "7e72bebb18347b4f9eaaecd78456ef7a" },
					{ "use_disslove_dynamicparameter", true, "107a10747fae6245801e0e3375107192" },
					{ "use_g_channal", false, "5ef46e491242754c943b98fec63553fd" },
					{ "use_r_channal", true, "5bc0bb08eae6a04db8fb7762ca0d5289" },
					{ "use_rgb_channal", false, "a1525f89674e5943bb0ced9811152778" },
					{ "use_uv_noise_tex_03", false, "0f473e93b501d94ab3c922ec7fc30cdf" },
					{ "use_uvnoisetex_02", false, "d57168e0825fc347a8d1df7cc7469c38" }
				}};
				uint32_t iCacheSelectedMask = 0u;
				for (size_t i = 0u; i < CACHE_SWITCHES.size(); ++i)
				{
					if (CACHE_SWITCHES[i].strName.empty() ||
						CACHE_SWITCHES[i].strExpressionGuid.size() != 32u)
						return false;
					if (CACHE_SWITCHES[i].bValue)
						iCacheSelectedMask |= 1u << static_cast<uint32_t>(i);
				}
				if (iCacheSelectedMask != 0x05cu)
					return false;

				struct INPUT_CONTRACT final
				{
					std::string_view strId;
					std::string_view strName;
					std::string_view strRow;
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
					double fScalar;
					std::string_view strTexture;
				};
				static constexpr std::array<INPUT_CONTRACT, 34u> INPUTS = {{
					{ "material-input-159a7dd413037981", "alpha_background_velue", "00abb818b522434c713b89e89d1e097897d9393effc2bd389ab00e4fbd1575a4", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 50., {} },
					{ "material-input-aaf495ab8a7cfbc8", "alpha_hardness", "ea646634d75f23a3205d5a63b838e157b06fffcf4e34c1b64f29b8067b5ab191", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {} },
					{ "material-input-98da99090faf34f9", "alpha_position_x", "c14f83afee8243c8b488fd211494449c43f4913bd2aa4b619d7123bfb014f55c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0., {} },
					{ "material-input-23c88f028b1c4c19", "alpha_position_y", "fb05b4ed9c93ca0e47628b32c9d6d1ebd3113aadd9960d53704968dd857a9124", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.05000000074505806, {} },
					{ "material-input-f137fee587feffc5", "alpha_radius", "efca7ce6466f147681fedd7c513123bf003e88d95579f37e15c1f014d7efa98c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.25, {} },
					{ "material-input-204fbcca647804a0", "alpha_strength", "24cfe22bec7a352649370c7e2aa208f89b9d37845b656b352aaefeafdc3ed7a2", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {} },
					{ "material-input-8ce48b663a5fdcef", "dissolve_hardness", "d6dd2b155daa94e7c6c92a5590fb97cb578cc6abd663465a0fc8b562eaefb255", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {} },
					{ "material-input-78c7d4be5a5d06c1", "dissolve_pan_y", "fa4c7bc7a6d619c58700c2e6c7b98627327dc396eb782b18f8a1fbeff90be775", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {} },
					{ "material-input-9eedbe6983e36a61", "dissolve_texcoord_x", "76f50332ce5df7ed28994eb7208186c32d45022309d4437fcf17f03527e11d03", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.800000011920929, {} },
					{ "material-input-ca835034c4c0c2e9", "dissolve_texcoord_y", "6413229399f78fed9bba2bc1ad53d7268543e7b56af1f37b6eba8f71542ac75c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.800000011920929, {} },
					{ "material-input-1085a4637e48493f", "dissolve_velue", "cda9ea39fcd871017672a36374688ccd8e6289b5db27e851829bc54b1c5a3954", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, -1., {} },
					{ "material-input-539eef2b545722e8", "emissive_background_velue", "ed2330112528aeda61078746952435439b681f15fb6985cff866f4cea2c97a06", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 5., {} },
					{ "material-input-60787000a0cf590c", "emissive_power", "4d290fc89ef5ca6251f91e1ad3fd1a83df0a656ffe84cb580037128e930ef97a", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {} },
					{ "material-input-1991f371d22955e7", "emissive_strength", "9286c27cb72c70533d44395bd0684adf3503f6f69b4f1e7b0916d0089a466511", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {} },
					{ "material-input-18c428c1b260bf1d", "maintax_move_x", "e95f0ba2907e9a8bd85b21cc3d4a0b737f7a80335bcb42aa5795499c07454e23", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, -0.15000000596046448, {} },
					{ "material-input-dfdf0888c8a1f386", "maintax_move_y", "eff65fd8bb2369d81995ca1493f14d95abe2cbc8e38a36420a3d45b2f6e6db56", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, -0.15000000596046448, {} },
					{ "material-input-1ae3f777138d4d3d", "maintax_texcoord_x", "113e4060c2237371b02f92bf053f8c3ed2e2519f087b08e05c3d5da3f2a28103", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1.2999999523162842, {} },
					{ "material-input-59e2fb07453a1876", "maintax_texcoord_y", "8c09c19b3dc9aca2ea0906003daa367182b63df0e6d78ed30d95a3614c202c1e", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1.2999999523162842, {} },
					{ "material-input-8b5620f764876c24", "noise_pan_speed", "80b9f4be89a8c2d41a65dd7bcba769845fbb4f81760526e2408c9fd53e91666a", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {} },
					{ "material-input-c3eb2c8b1c5c6b1d", "uv_noise_velue_01", "2236d46b97a8c4ab5ab0e47e523482402da7c84a175688bc15bd6b89fafd2eba", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {} },
					{ "material-input-9a98a6b881b03764", "uv_noise_velue_02", "0181c2d17bc819d78e0bcbd02bd2b28d38195313125ff8e054f9897c23cbb97d", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 4., {} },
					{ "material-input-93725aab8c7579d2", "uv_noise_velue_all", "4c293ba1bd0d6275bc0a40fb0d6025361ada444eb67bd8165b0d12d9ee9b0036", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 4., {} },
					{ "material-input-a241794b40c6ce4d", "uvnoise_texcoord_02_x", "9d799e7dc4e2a2837c548b83f934a13eaf1b691309769fd7ec2f799cf27e5db1", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {} },
					{ "material-input-a8357737bd6ca2b3", "uvnoise_texcoord_02_y", "17d63be9ba97d8109889686e25f2d6bf55b9a4e21929a8bd0adefe06f892c6d0", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {} },
					{ "material-input-37d9f4c5c6cd2d7f", "uvnoise_texcoord_x", "67343dd40c20733e7e7ba3aa7ab464795dd3ca653deba038ef0867fe86e2e7ab", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {} },
					{ "material-input-e7c20cb3b2c10792", "uvnoise_texcoord_y", "d5fc8bd4e209ce2101ba03535f6f908e3ec829d091c8e0f2881d1cc27d2fd58c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {} },
					{ "material-input-4a154273a581f981", "uvnoisetex_move_x", "d90981f10b462eedbf7c24b8e3c2c7ae396bf0e7d6a394ffb9be326589e93879", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, -0.5, {} },
					{ "material-input-f0b98bc645b24021", "dissolve_tex", "21df01f9b3950533e084ef6656c58e7296e4316dbe0da18e458909fb5bf7c85c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_05.fx_m_noise_001" },
					{ "material-input-aa07db539b5504ef", "uv_noise_tex_01", "781595b18ba266a02cfb780d7c085a9de7d93199e790e7571e655837a78c5ea6", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_noise_030" },
					{ "material-input-338847b1c520f058", "mainalpha_tex", "2428ece20c175710e71b5f87ee7fa46178dfdf7907c339157d23cd4b7c10ddd4", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_03.fx_e_ring_001_cl" },
					{ "material-input-8117023dbeef8b0d", "uv_noise_tex_02", "cdde3c60804e53c0202abf70fa8a381a5b888f29beae6b490ff057aa6ad30999", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_noise_030" },
					{ "material-input-e73dc402634e3424", "uv_noise_tex_03", "231673479a3c728a430f24b7a1624ccc3dd0177fa728c9dd5d6c7ac98105b7ef", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_noise_030" },
					{ "material-input-544e90a8ca3f6dbc", "uvnoise_texcoord_03_x", "343b7cffb7cfbbcbfb9f13dc9fb9ccc601898020bf8b62df3c722610aa220815", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {} },
					{ "material-input-4fb175d9540382c5", "uvnoise_texcoord_03_y", "d321e235d4872a19defbb18375eea2064c51fa2cf30589b7f1f06a04d8e0ecfc", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1.5, {} }
				}};
				for (size_t i = 0u; i < INPUTS.size(); ++i)
				{
					const INPUT_CONTRACT& Expected = INPUTS[i];
					const auto It = std::find_if(Program.MaterialInputs.begin(),
						Program.MaterialInputs.end(), [&](const auto& Row)
						{ return Row.Row.strId == Expected.strId; });
					if (Recipe->InputIds[i] != Expected.strId ||
						It == Program.MaterialInputs.end() ||
						It->strRecipeId != Recipe->Row.strId ||
						It->strNormalizedParameterName != Expected.strName ||
						It->Row.strRowSha256 != Expected.strRow ||
						It->eVariant != Expected.eVariant ||
						It->strTypedValueSha256.empty() ||
						It->strSourceFieldValueSha256.empty() ||
						It->strSourceLineageSha256.empty() ||
						(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
							(!It->fValue.has_value() || *It->fValue != Expected.fScalar)) ||
						(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID &&
							It->strStringValue != Expected.strTexture))
					{
						strOutError = "Artist F active000 material input changed at index " +
							std::to_string(i) + ".";
						return false;
					}
				}

				struct STATIC_CONTRACT final
				{
					std::string_view strId, strName, strRow, strPolicy, strLineage;
				};
				static constexpr std::array<STATIC_CONTRACT, 4u> STATICS = {{
					{ "material-input-c2011bfa3eeeaf68", "use_centeralpha", "34e2705a451f7f950b85bfe25c8bd97d2877fba1b37565857a3c0be4ffcfe734", "material-reconstructed-policy-a2fccec99e03a62a95a1", "e78e1b7900791e4fb03027468d1d09f63f8ac6b48f08bdb7cb9707bde76653cd" },
					{ "material-input-38f18da88c00554a", "use_r_channal", "d70724a436797c2b155fcc727a79901dc23402a3fc885a9cb1a604723408c368", "material-reconstructed-policy-8e3813fac065fa188fe7", "22fc17316f2bb6b09754296549e381ec4caa6c59067388a0b2b64e4e0ce42be1" },
					{ "material-input-1647f7214ab68929", "use_r_channal", "2caab113fd2eaa58e6e8bd9a3b0fce7294998b367952c415a839d113b15b40de", "material-reconstructed-policy-dd7b235658c6da01ebd0", "2839bf75c898355b3c762a66555425ac2604968538f5513e57ac120909eda572" },
					{ "material-input-357cf3dc7e3f489e", "use_disslove_dynamicparameter", "6b4a859cb7d6c9fe448d018ccd5e8a11b66e129f75f1067a1072a1b81ad08584", "material-reconstructed-policy-1f58f5ba3a9b58049a64", "0f380bc03ba3dba05ffd4c12a039261fb8778e30b9f5e1e593330b84b579e10a" }
				}};
				for (size_t i = 0u; i < STATICS.size(); ++i)
				{
					const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
						Program.MaterialStaticBindings.end(), [&](const auto& Row)
						{ return Row.Row.strId == STATICS[i].strId; });
					if (Recipe->StaticBindingIds[i] != STATICS[i].strId ||
						It == Program.MaterialStaticBindings.end() ||
						It->strRecipeId != Recipe->Row.strId ||
						It->strNormalizedParameterName != STATICS[i].strName ||
						It->Row.strRowSha256 != STATICS[i].strRow ||
						It->strPolicyRowId != STATICS[i].strPolicy ||
						It->strSourceLineageSha256 != STATICS[i].strLineage ||
						!It->bSourceValue.value_or(false) ||
						!It->bSelectedValue.value_or(false))
					{
						strOutError = "Artist F active000 legacy static row changed.";
						return false;
					}
				}

				static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
					"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
					"opacitymaskclipvalue", "buseonelayerdistortion" }};
				static constexpr std::array<std::string_view, 6u> RENDER_ROWS = {{
					"5eb04cb66a03d78ebf2f81cb37afa3a6f558a517b1c3386587a2c4654c4dc454",
					"ddc286947927f2bd595cdac97060e48b49ae4f413b2ba4a03515f2ecd8d870d4",
					"ed6b6f11a859265f1ca4d2f82d83b7a15ff2787b9297bbb7c8a09f1a55951032",
					"a138a3dd925a16dd966cf20aa1030cd6357eab24a838013410256b5aa8c4b2e8",
					"7923c4d8afd6429138f426f35094892793a4f313927b71c2ecb0282f51db41e1",
					"03957d7d0f5111ad4d99b60c4fa790388680c2294c4405a68ea28c5948dd6c51" }};
				for (size_t i = 0u; i < RENDER_FIELDS.size(); ++i)
				{
					const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
						Program.MaterialRenderBindings.end(), [&](const auto& Row)
						{ return Row.Row.strId == Recipe->RenderBindingIds[i]; });
					if (It == Program.MaterialRenderBindings.end() ||
						It->strRecipeId != Recipe->Row.strId ||
						It->strFieldName != RENDER_FIELDS[i] ||
						It->Row.strRowSha256 != RENDER_ROWS[i] ||
						(i == 0u && It->strStringValue != "blend_translucent") ||
						(i == 1u && It->strStringValue != "mlm_unlit") ||
						(i == 2u && !It->bValue.value_or(false)))
					{
						strOutError = "Artist F active000 render-state row changed.";
						return false;
					}
				}

				const auto Dynamic = std::find_if(Program.Modules.begin(),
					Program.Modules.end(), [&](const auto& Row)
					{ return Row.strEmitterId == Emitter.Row.strId &&
						Row.strExactSourceClass == "particlemoduleparameterdynamic"; });
				const size_t iDynamicCount = static_cast<size_t>(std::count_if(
					Program.Modules.begin(), Program.Modules.end(), [&](const auto& Row)
					{ return Row.strEmitterId == Emitter.Row.strId &&
						Row.strExactSourceClass == "particlemoduleparameterdynamic"; }));
				if (iDynamicCount != 1u || Dynamic == Program.Modules.end() ||
					Dynamic->Row.iOrder != 4u || Dynamic->Row.strRowSha256 !=
						"4297ca6f1a3c2bea471e25483b787ff02318df859b5964c81758a8d8e0a4b1d6" ||
					Dynamic->strSourceObjectId != "FX_MN_CNBZ_00_N:export:8697" ||
					Dynamic->strSourceRecordSha256 !=
						"5cc8a4875c3a46bab605fac67e462c75662d54f207d8cba187ee5951bdb9ccfe" ||
					Dynamic->DistributionIds.size() != 4u ||
					Dynamic->LiteralIds.size() != 22u)
				{
					strOutError = "Artist F active000 dynamic module changed.";
					return false;
				}
				static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
					"disslove", "param2", "param3", "param4" }};
				static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
					"814c7439e2b11c8b44128e54bf1a6d062899585772697d92daa03b02bbb23439",
					"09f60253e457c28698a2df58da4710debe8e50071a149fe512f7097632534b5c",
					"b2979503e22f8b6e19c35b9a85a447a634a76fd9f44745907d59445d3a7cd69e",
					"6c01cebfdc62c3562f18a94570f304b1a28c771429c2006718ce09c0b6c2350a" }};
				const auto FindLiteral = [&](const std::string& strPath)
				{
					return std::find_if(Program.Literals.begin(), Program.Literals.end(),
						[&](const auto& Row) { return Row.strModuleId == Dynamic->Row.strId &&
							Row.strPropertyPath == strPath; });
				};
				for (size_t i = 0u; i < 4u; ++i)
				{
					const std::string Prefix = "dynamicparams[" + std::to_string(i) + "].";
					const auto Name = FindLiteral(Prefix + "paramname");
					const auto Method = FindLiteral(Prefix + "valuemethod");
					const auto Scale = FindLiteral(Prefix + "bscalevelocitybyparamvalue");
					const auto Spawn = FindLiteral(Prefix + "bspawntimeonly");
					const auto EmitterTime = FindLiteral(Prefix + "buseemittertime");
					const auto Dist = std::find_if(Program.Distributions.begin(),
						Program.Distributions.end(), [&](const auto& Row)
						{ return Row.Row.strId == Dynamic->DistributionIds[i]; });
					if (Name == Program.Literals.end() || Method == Program.Literals.end() ||
						Scale == Program.Literals.end() || Spawn == Program.Literals.end() ||
						EmitterTime == Program.Literals.end() ||
						Name->strEnumValue != DYNAMIC_NAMES[i] ||
						Method->strEnumValue != "edpv_userset" ||
						Scale->bValue.value_or(true) || Spawn->bValue.value_or(true) ||
						EmitterTime->bValue.value_or(true) ||
						Dist == Program.Distributions.end() || Dist->Row.iOrder != i ||
						Dist->Row.strRowSha256 != DYNAMIC_ROWS[i] ||
						Dist->strPropertyPath != Prefix + "paramvalue" ||
						Dist->eVariant != EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ||
						Dist->iComponentCount != 1u ||
						Dist->iOperation.value_or(0u) != 1u ||
						Dist->iLookupTableChunkSize.value_or(0u) != 1u ||
						Dist->iLookupTableNumElements.value_or(0u) != 1u)
					{
						strOutError = "Artist F active000 dynamic lane changed.";
						return false;
					}
				}
				const auto LodValidity = FindLiteral("lodvalidity");
				const auto UpdateFlags = FindLiteral("updateflags");
				if (LodValidity == Program.Literals.end() ||
					UpdateFlags == Program.Literals.end() ||
					LodValidity->fValue.value_or(-1.) != 1. ||
					UpdateFlags->fValue.value_or(-1.) != 15.)
				{
					strOutError = "Artist F active000 dynamic flags changed.";
					return false;
				}

				SetVisualV4Params(8u, 0x07u, {
					{ 50.f, 1.f, 0.f, 0.05000000074505806f },
					{ 0.25f, 1.f, 1.f, 0.10000000149011612f },
					{ 0.800000011920929f, 0.800000011920929f, -1.f, 5.f },
					{ 0.10000000149011612f, 1.f, -0.15000000596046448f,
						-0.15000000596046448f },
					{ 1.2999999523162842f, 1.2999999523162842f,
						0.10000000149011612f, 0.10000000149011612f },
					{ 4.f, 4.f, 2.f, 2.f },
					{ 2.f, 2.f, -0.5f, 1.f } },
					{ 0.f, 0.f, 0.f, 1.f }, { -0.2f, 1.f, 0.05f, 0.4f });
				for (size_t i = 0u; i < 7u; ++i)
					Resource.RuntimeMaterialV2ScalarBlocks[i] =
						Resource.ArtistVisualV4Params[i];
				Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x01u;
				Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0eu;
				Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
				Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
				Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
				Resource.iRuntimeMaterialV2ScalarCount = 27u;
				Resource.iRuntimeMaterialV2VectorCount = 0u;
				Resource.iRuntimeMaterialV2InputCount = 34u;
				Resource.RuntimeMaterialV2InputConsumedMask = { 0x3fffffffu, 0u };
				Resource.RuntimeMaterialV2InputSuppressedMask = { 0xc0000000u, 0x03u };
				Resource.iRuntimeMaterialV2StaticInputCount = 4u;
				Resource.iRuntimeMaterialV2StaticSelectedMask = 0x0fu;
				Resource.iRuntimeMaterialV2StaticConsumedMask = 0x0fu;
				Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
				Resource.iRuntimeMaterialV2RenderInputCount = 6u;
				Resource.iRuntimeMaterialV2RenderConsumedMask = 0x07u;
				Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x38u;
				ArtistVisualV4TextureLanes = {
					{ "uv_noise_tex_01", "fx_tex_02.fx_d_noise_030",
						"Effect/Artist/Textures/fx_d_noise_030.dds",
						"material-input-aa07db539b5504ef",
						"781595b18ba266a02cfb780d7c085a9de7d93199e790e7571e655837a78c5ea6",
						"eedf11f127656621c0f9922f56b932aabc2ef321bdc697dd487750b82886189e",
						0u, 32896u,
						"9a876ceb5d173869af5350d348462f54fc8a9b00134957bd3cd56805860d0581",
						"material-reconstructed-policy-2f786b657500e104d820",
						"2dbab5370c66169eb1013940f6544f979e8f72e9ee12ae977758c1e0e8046fb3",
						256u, 256u, DXGI_FORMAT_BC1_UNORM_SRGB,
						D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
						D3D11_TEXTURE_ADDRESS_WRAP },
					{ "mainalpha_tex", "fx_tex_03.fx_e_ring_001_cl",
						"Effect/Artist/Textures/FX_TEX_03/fx_e_ring_001_cl.dds",
						"material-input-338847b1c520f058",
						"2428ece20c175710e71b5f87ee7fa46178dfdf7907c339157d23cd4b7c10ddd4",
						"da83b07e78285c8113bd29bcab03cd77be7a490bcb4692530d5b8fd3af32af8c",
						1u, 65664u,
						"3c8987c8bc4bda1d3fd0f4840124e4fc1ba2eb3899307df8fe5852c4a760738e",
						"material-reconstructed-policy-73c145f5bcb8923c6655",
						"230064b5ea3ce2c2b8d94dcc4a5adb79cc3df299eb86ca1dfa2b2a149b1cdf78",
						256u, 256u, DXGI_FORMAT_BC3_UNORM_SRGB,
						D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP,
						D3D11_TEXTURE_ADDRESS_WRAP },
					{ "dissolve_tex", "fx_tex_05.fx_m_noise_001",
						"Effect/Artist/Textures/fx_m_noise_001.dds",
						"material-input-f0b98bc645b24021",
						"21df01f9b3950533e084ef6656c58e7296e4316dbe0da18e458909fb5bf7c85c",
						"7657e78caf559b2434fe273c4dbb672cc703b85aaee684d31a81f8d8d27a2031",
						2u, 131200u,
						"19843f9ee15e94e629926f45e1887ad6ca9815bfd785527ed8b4ce63692918b8",
						"material-reconstructed-policy-527d90a533e7f80af065",
						"c8ba52ad326e1a68838b39223a2e99dff808e3a01062ab40c9834b6445e2f86f",
						512u, 512u, DXGI_FORMAT_BC1_UNORM_SRGB,
						D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP,
						D3D11_TEXTURE_ADDRESS_WRAP }
				};
				break;
			}
			case 7u:
			case 8u:
				if (!ValidateTypedScalars({
					{ "alpha_disslove_tex_coord_r", 0.3 },
					{ "alpha_disslove_tex_coord_g", 1.0 },
					{ "alpha_tex_strength", 7.0 },
					{ "alpha_out_falloff", 10.0 },
					{ "dissolve_r", 0.0 },
					{ "dissolve_hardness", 1.0 },
					{ "emissive_tex_power", 1.5 },
					{ "emissive_tex_strength", 20.0 } }))
					return false;
				SetVisualV4Params(1u, 0x03u, {
					{ 0.3f, 1.f, 7.f, 10.f },
					{ 0.3f, 1.f, 0.f, 1.f },
					{ 0.f, 0.f, 0.f, 1.5f } },
					{ 1.f, 1.f, 1.f, 20.f }, {});
				ArtistVisualV4TextureLanes = {
					{ "alpha_tex", "fx_tex_05.fx_m_atypical_007", "Effect/Artist/Textures/fx_m_atypical_007.dds" },
					{ "uv_dissolve_tex", "fx_tex_05.fx_m_noise_001", "Effect/Artist/Textures/fx_m_noise_001.dds" } };
				break;
			case 12u:
				if (!ValidateTypedScalars({
					{ "diff1_tile_u", 2.5 }, { "diff1_tile_v", 0.2 },
					{ "diff2_tile_u", 2.0 }, { "diff2_tile_v", 0.5 },
					{ "flow_tile_u", 1.5 }, { "flow_tile_v", 0.5 },
					{ "opacity_tile_u", 1.0 }, { "opacity_tile_v", 1.2 },
					{ "opacity_str", 10.0 }, { "diff2_pan_v", -0.2 },
					{ "flow_bias", 0.2 }, { "opacity_distort_str", 1.0 },
					{ "diff_pow", 0.7 }, { "diff_str", 100.0 },
					{ "cameravec_pow", 2.0 } }))
					return false;
				SetVisualV4Params(2u, 0x0fu, {
					{ 2.5f, 0.2f, 2.f, 0.5f },
					{ 1.5f, 0.5f, 0.f, 0.f },
					{ 1.f, 1.2f, 0.f, 10.f },
					{ 0.f, -0.2f, 0.2f, 1.f },
					{ 0.7f, 100.f, 2.f, 0.f } },
					{ 1.f, 1.f, 1.f, 1.f }, {});
				ArtistVisualV4TextureLanes = {
					{ "diff_tex1", "fx_tex_05.fx_k_auraline_16", "Effect/Artist/Textures/fx_k_auraline_16.dds" },
					{ "diff_tex2", "fx_tex_05.fx_k_caustictile_01", "Effect/Artist/Textures/fx_k_caustictile_01.dds" },
					{ "flowtex", "fx_tex_02.fx_d_noise_002", "Effect/Artist/Textures/fx_d_noise_002.dds" },
					{ "opacity_tex", "fx_tex_01.fx_c_atypical_007_cl", "Effect/Artist/Textures/fx_c_atypical_007_cl.dds" } };
				break;
			case 13u:
			case 14u:
			{
				const bool_t bActive013 = Occurrence->Row.iOrder == 13u;
				const std::string_view strExpectedOccurrenceRowSha256 = bActive013 ?
					"7402d66262c83b2ee878f1a91fcdae14b8d71ab7f6c104736149eb6be195df89" :
					"bd8956383f67d2a1c2ccdf4324dd6a97d858f375561603cc2200da0fab092b55";
				const std::string_view strExpectedEmitterRowSha256 = bActive013 ?
					"28df4bc241309a88baa970cde323fea0026ca0dbaeedebccbbca4241bbc5f065" :
					"f941c08118629b9627b9f51094d6101b772544359c7e4d2ed3d18d9391cb7d52";
				const std::string_view strExpectedDynamicModuleRowSha256 = bActive013 ?
					"7f66db172116099310a268882ddd07ab52f4c6311edee0f34c4251a42ad8900c" :
					"67dddab7ab2bc87efca2e0a2982d4fe3fe8a84df1b6c33a6d0e681f9e050b264";
				if (Recipe->Row.strRowSha256 !=
						"d3abb4c84799e1f45464906d8bb793638ba805879e378e3887177c157d97651e" ||
					Recipe->strSourceMaterialPath !=
						"fx_m_mi_01.fx_mi.fx_o_me_flow_02_24_tr" ||
					Recipe->strSourceRecipeCompositionSha256 !=
						"db32af9cd1e35e5d07e8f379f3422d163e8dcc3b570cf32ff67822744fbc05fb" ||
					Recipe->strBindingSha256 !=
						"b0ce1911bdcabcef0b0625a3adf9e7b65a4ae2197e8c6d7cd26a1593561bb982" ||
					Recipe->InputIds.size() != 29u ||
					Recipe->StaticBindingIds.size() != 6u ||
					Recipe->RenderBindingIds.size() != 6u ||
					Family->Row.strRowSha256 !=
						"386fce4a0c98346fd7790fc381bbfc12105a8a77906e4a22d7d63311f80cc056" ||
					Family->strFamilyIdentitySha256 !=
						"7db75481767145e6e76bfa933b64a6545ac34cca5fb137f6a848daf3e09d5cb7" ||
					Occurrence->Row.strRowSha256 != strExpectedOccurrenceRowSha256 ||
					Emitter.Row.strRowSha256 != strExpectedEmitterRowSha256 ||
					Occurrence->strBindingSha256 != Recipe->strBindingSha256 ||
					Occurrence->strSourceOccurrenceIdentitySha256 != (bActive013 ?
						"631d02c069a70b424594844628537d0f196c6bcffc68b99bc04dd233b18e28c3" :
						"4aecaba083112be2d82b61c74d5833f12b512921c3123bc550ad519ae4e3dcfc") ||
					Occurrence->strSourceOccurrenceBindingSha256 != (bActive013 ?
						"173be48920edabe177e61548e7daf9064a0066cf8bc519e36454f553fde11238" :
						"1a728a211b91569bcadd90f79a694e43a9877133ee18063df396f189293a4973") ||
					ShaderRegistry->strEngineEqualityStaticSetSha256 !=
						"60d8c69eaffb733d56f5284bf72a9d88904e48cc29fad4d6566c309f32eccbc2" ||
					ShaderRegistry->strRecoveredPixelShaderId !=
						"98a9cc70d9b42e45a66ea097407a0894" ||
					ShaderRegistry->strRecoveredPixelDxbcSha256 !=
						"4adf706f0819276f6a8577968a46605435bbd60c95c481fa80b3c0075481d9e8" ||
					Element.Material.eRenderProfile !=
						EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ||
					!ValidateTypedScalars({
						{ "09.str", 2.0 },
						{ "10.power", 1.2000000476837158 },
						{ "02.map_e_uvscale_r", 0.5 },
						{ "03.map_e_uvscale_g", 1.0 },
						{ "04.map_e_panning_x", 0.4000000059604645 },
						{ "05.map_e_panning_y", -0.029999999329447746 },
						{ "12.desaturation", 1.0 },
						{ "15.emissiion_power", 1.5 },
						{ "18.headstr_power", 2.5 },
						{ "19.headstr", 60.0 },
						{ "05.uv.y.strech", -1.0 },
						{ "11.direct", 3.0 },
						{ "05.distort_str", 0.4000000059604645 },
						{ "07.map_d_uvscale_r", 1.0 },
						{ "08.map_d_uvscale_g", 3.0 },
						{ "10.map_d_panning_y", 1.0 } }))
				{
					strOutError = "Artist F flow-02 recovered identity changed: " +
						Occurrence->Row.strId;
					return false;
				}

				struct EXPECTED_MATERIAL_ROW final
				{
					std::string_view strId;
					std::string_view strName;
					std::string_view strRowSha256;
				};
				static constexpr std::array<EXPECTED_MATERIAL_ROW, 16u>
					EXPECTED_SCALARS = {{
					{ "material-input-b0bf8fcf91112d44", "09.str", "843e58850f0f28a674643ec8bf1b0b575fb72ae0512bc6c173d094baa66b5340" },
					{ "material-input-8d615a2e4a7ba103", "10.power", "91450ca5b22e5b645f4e8e2de4500f5f468870b4606787ddecb16430a818e291" },
					{ "material-input-6cce166be9ba82e1", "02.map_e_uvscale_r", "35ef649a030f81a0c9d53418c83c3439ce097a1f1b1e741cb21e3822c6aca1b0" },
					{ "material-input-574d354a0ede3496", "03.map_e_uvscale_g", "fb9cc22f10656a3d658c3d1eec89555190b4d7c6e92c756e53d670e77d3ec9eb" },
					{ "material-input-cb7154692f71c3d0", "04.map_e_panning_x", "e1b6d63ab2c6c828023f1fce6ed61b320d29bd699e891245a41939ca80f9a564" },
					{ "material-input-f114306d6040d34c", "05.map_e_panning_y", "e330514f4db87e9224efd0798cd2270bd4ee283855148010fa0f67fb1542b01a" },
					{ "material-input-37c8dd580755d3df", "12.desaturation", "cdcf3637600c143a792701d69cb7daf794dcf8151dfe3135457fa868eb9d09e6" },
					{ "material-input-14fee414dc9f260d", "15.emissiion_power", "b8249c816472820f6757490bcd06f740801b544ce15e6c4a721a7b905600ccae" },
					{ "material-input-e044e4b3cc601d1d", "18.headstr_power", "4b733bb25b1aa3974eedaf6dac2ecbcffac2208d1b5c85c64e8122fbad09268b" },
					{ "material-input-449c55e7fca0de99", "19.headstr", "4410f2e000daa76126e86f1fbad57a0080a0139f3c88d2dd7b273aa0f9a134a4" },
					{ "material-input-00978e11a446144d", "05.uv.y.strech", "3e1275ee2d5640186074b21a8297629a736c853cdfe6dcda9eec81dac71b6533" },
					{ "material-input-b3b87d88d46d0d56", "11.direct", "74cdf100ac2092910e1352c887e0506a9c43574c4c40f9dfe4ce9e8e79865ad1" },
					{ "material-input-ffcd62a09ab50d1c", "05.distort_str", "8083a2ab667febea80d1635352d2eace45361c2f438bbf568703c45e64919394" },
					{ "material-input-286f613498750f12", "07.map_d_uvscale_r", "41c101a5d4e9232ab92ab08208e8a9bf09a80b1419e0ca8671ff2ccce4bf64fd" },
					{ "material-input-c1ebc1087e0e0d0b", "08.map_d_uvscale_g", "412c8afa73d10f22daa998c9ae50fdc0e4dd5fac857db5269df044b89eddc2bb" },
					{ "material-input-9df28921287bcce4", "10.map_d_panning_y", "bbad18623da712dfb3c89c0d10c3ba2c293e82ba0b5648cc7a6acd767cc092d4" }
				}};
				for (const EXPECTED_MATERIAL_ROW& Expected : EXPECTED_SCALARS)
				{
					const auto Input = std::find_if(Program.MaterialInputs.begin(),
						Program.MaterialInputs.end(), [&](const auto& Row)
						{
							return Row.Row.strId == Expected.strId;
						});
					if (Input == Program.MaterialInputs.end() ||
						Input->strRecipeId != Recipe->Row.strId ||
						Input->strNormalizedParameterName != Expected.strName ||
						Input->Row.strRowSha256 != Expected.strRowSha256)
					{
						strOutError = "Artist F flow-02 scalar row changed: " +
							Occurrence->Row.strId + "/" + std::string(Expected.strName);
						return false;
					}
				}

				const auto ColorInput = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == "material-input-74e8e67100528942";
					});
				if (ColorInput == Program.MaterialInputs.end() ||
					ColorInput->strRecipeId != Recipe->Row.strId ||
					ColorInput->Row.strRowSha256 !=
						"d4903d709e6a3ed3e475829fc656b23fc909b4f6d21ceb87647eed88a74b166f" ||
					ColorInput->strNormalizedParameterName != "16.color" ||
					ColorInput->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
					ColorInput->vValue != std::array<double, 4u>{ 1.0, 1.0, 1.0, 10.0 })
				{
					strOutError = "Artist F flow-02 color row changed: " +
						Occurrence->Row.strId;
					return false;
				}

				static constexpr std::array<EXPECTED_MATERIAL_ROW, 6u>
					EXPECTED_STATICS = {{
					{ "material-input-9d1723f9494b12d8", "06.mapch.r", "387defb27ede153e72b5e3156e813b478694ccad02fd082061514d877d5aa7c6" },
					{ "material-input-533ef548561210a3", "07.mapch.g", "93c8dfa8be30b5a945b82dfcbad6d3b2d7faf58e95503549775743f372263c5a" },
					{ "material-input-85f381349ad53338", "08.mapch.b", "c0f918dbf42b100659cd1217b59b8cdd78886a86d66ed026e03819d781e84450" },
					{ "material-input-9fb043df2bf1121f", "00.useemissionmap", "e29ce43e78490665de2c5b6923f7fa6aec6646f292ed5514d697d0ae5a77be98" },
					{ "material-input-a678289e6db8a1db", "00.useemissionmap", "0813a28ee8401e5a04e963c45b5330435cd2fa624f33d208b66baea5f04ccc53" },
					{ "material-input-37f6a46992dcdb6f", "17.useheademission", "ba9298e00495944f2368e150310df5f183b2d2e3f358067d24b82d330538d224" }
				}};
				for (const EXPECTED_MATERIAL_ROW& Expected : EXPECTED_STATICS)
				{
					const auto Static = std::find_if(Program.MaterialStaticBindings.begin(),
						Program.MaterialStaticBindings.end(), [&](const auto& Row)
						{
							return Row.Row.strId == Expected.strId;
						});
					if (Static == Program.MaterialStaticBindings.end() ||
						Static->strRecipeId != Recipe->Row.strId ||
						Static->strNormalizedParameterName != Expected.strName ||
						Static->Row.strRowSha256 != Expected.strRowSha256 ||
						Static->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
						!Static->bSourceValue.value_or(false) ||
						!Static->bSelectedValue.value_or(false))
					{
						strOutError = "Artist F flow-02 static row changed: " +
							Occurrence->Row.strId + "/" + std::string(Expected.strName);
						return false;
					}
				}

				static constexpr std::array<EXPECTED_MATERIAL_ROW, 6u>
					EXPECTED_RENDERS = {{
					{ "material-recipe-eca6507a183c9c39::render:blendmode", "blendmode", "d30f8e4d2cae9a9abc89ffa6ef56c13a4df83437badd6f898daebf2fa31e2cab" },
					{ "material-recipe-eca6507a183c9c39::render:lightingmodel", "lightingmodel", "e467ab09b1c1b221ba3b5dfecbe99d3f727539c14184f33159f247b6d7cd879f" },
					{ "material-recipe-eca6507a183c9c39::render:twosided", "twosided", "d1fdda77f8546fb5cb66ad7f485ae7c2f67fcc7b3015122cf555239f5812a8eb" },
					{ "material-recipe-eca6507a183c9c39::render:bdisabledepthtest", "bdisabledepthtest", "40d9569370d171e18fc9eb0f9731a981467d52d305775bf0b95ac6c22621932b" },
					{ "material-recipe-eca6507a183c9c39::render:opacitymaskclipvalue", "opacitymaskclipvalue", "9d3c96ed523fe70bace44d209756d05541fe0974da0013d33a6a3008e289a12e" },
					{ "material-recipe-eca6507a183c9c39::render:buseonelayerdistortion", "buseonelayerdistortion", "21f36e6de49526929364eebd0a28e427da6457841dcbdb915147fd9b3d2a9a46" }
				}};
				for (const EXPECTED_MATERIAL_ROW& Expected : EXPECTED_RENDERS)
				{
					const auto Render = std::find_if(Program.MaterialRenderBindings.begin(),
						Program.MaterialRenderBindings.end(), [&](const auto& Row)
						{
							return Row.Row.strId == Expected.strId;
						});
					if (Render == Program.MaterialRenderBindings.end() ||
						Render->strRecipeId != Recipe->Row.strId ||
						Render->strFieldName != Expected.strName ||
						Render->Row.strRowSha256 != Expected.strRowSha256)
					{
						strOutError = "Artist F flow-02 render row changed: " +
							Occurrence->Row.strId + "/" + std::string(Expected.strName);
						return false;
					}
				}

				const size_t iDynamicModuleCount = static_cast<size_t>(std::count_if(
					Program.Modules.begin(), Program.Modules.end(), [&](const auto& Row)
					{
						return Row.strEmitterId == Emitter.Row.strId &&
							Row.strExactSourceClass == "particlemoduleparameterdynamic";
					}));
				const auto DynamicModule = std::find_if(Program.Modules.begin(),
					Program.Modules.end(), [&](const auto& Row)
					{
						return Row.strEmitterId == Emitter.Row.strId &&
							Row.strExactSourceClass == "particlemoduleparameterdynamic";
					});
				if (iDynamicModuleCount != 1u || DynamicModule == Program.Modules.end() ||
					DynamicModule->Row.iOrder != 5u ||
					DynamicModule->Row.strRowSha256 != strExpectedDynamicModuleRowSha256 ||
					DynamicModule->strSourceObjectId != "FX_MN_PPCD_00_T:export:1811" ||
					DynamicModule->strSourceRecordSha256 !=
						"3769a025fc762fd1dff91bb1dedef74a30ef6222330a31004d38f2a701f875f5" ||
					DynamicModule->DistributionIds.size() != 4u ||
					DynamicModule->LiteralIds.size() != 22u ||
					std::find(Emitter.ModuleIds.begin(), Emitter.ModuleIds.end(),
						DynamicModule->Row.strId) == Emitter.ModuleIds.end())
				{
					strOutError = "Artist F flow-02 dynamic module changed: " +
						Occurrence->Row.strId;
					return false;
				}

				const std::array<std::string_view, 4u> ExpectedDistributionRows =
					bActive013 ? std::array<std::string_view, 4u>{
						"7307421a2418ddc50b831c2b1c353ab9decaeaff3a412d8971bb2bfff847bf79",
						"8357afb7bd5d4a0220653eab35ee91be544b64402c1a84349763de1b23329308",
						"c801f75bf12c49d4118b267107958375ce17b476715c3a28382b5a2f02a3196a",
						"7e170d6172f33136f95f1560439323a08819359416458f114d81fb2c44b78107" } :
					std::array<std::string_view, 4u>{
						"1e5960d9648f602fab9d8a9f5e7fb5954e3c312d65301dbdfa3e04345d2d4223",
						"1b0a7d28a73ef1b00238e08603597253eaa26a6364730291160cf109845716cc",
						"eedc8b5094a0b54639ee4567083685564e875a80f049030933ef31c29dc3c2ab",
						"69fb75b61244f64e9947e4c892f8d0703f4562b8e36e9d2a25cc8cf83b8f473e" };
				const std::vector<double> Zero4{ 0.0, 0.0, 0.0, 0.0 };
				const std::vector<double> OneLookup{ 1.0, 1.0, 1.0, 1.0 };
				const std::vector<double> PanLookup{
					0.0, 2.0, 0.0, 0.20017966628074646, 0.3892975449562073,
					0.5674397945404053, 0.734692394733429, 0.8911415934562683,
					1.036873459815979, 1.171973705291748, 1.2965290546417236,
					1.4106252193450928, 1.514348030090332, 1.6077841520309448,
					1.6910194158554077, 1.7641396522521973, 1.8272314071655273,
					1.8803805112838745, 1.9236729145050049, 1.9571951627731323,
					1.9810333251953125, 1.9952727556228638, 2.0 };
				for (size_t iLane = 0u; iLane < 4u; ++iLane)
				{
					const auto Distribution = std::find_if(Program.Distributions.begin(),
						Program.Distributions.end(), [&](const auto& Row)
						{
							return Row.Row.strId == DynamicModule->DistributionIds[iLane];
						});
					const std::string strExpectedPath = "dynamicparams[" +
						std::to_string(iLane) + "].paramvalue";
					if (Distribution == Program.Distributions.end() ||
						Distribution->strModuleId != DynamicModule->Row.strId ||
						Distribution->Row.iOrder != iLane ||
						Distribution->Row.strRowSha256 != ExpectedDistributionRows[iLane] ||
						Distribution->strPropertyPath != strExpectedPath ||
						Distribution->eVariant != EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ||
						Distribution->iComponentCount != 1u ||
						Distribution->iOperation.value_or(0u) != 1u ||
						Distribution->iRandomLockAxes.value_or(1u) != 0u ||
						Distribution->iLookupTableChunkSize.value_or(0u) != 1u ||
						Distribution->iLookupTableNumElements.value_or(0u) != 1u ||
						Distribution->fLookupTableStartTime.value_or(-1.0) != 0.0 ||
						Distribution->fLookupTableTimeScale.value_or(-1.0) !=
							(iLane == 1u ? 20.0 : 0.0) ||
						Distribution->DefaultMinimum != Zero4 ||
						Distribution->DefaultMaximum != Zero4 ||
						Distribution->LookupTable != (iLane == 1u ? PanLookup : OneLookup))
					{
						strOutError = "Artist F flow-02 dynamic distribution changed: " +
							Occurrence->Row.strId + "/" + std::to_string(iLane);
						return false;
					}
				}

				const auto FindDynamicLiteral = [&](const std::string& strPath)
				{
					const auto Count = static_cast<size_t>(std::count_if(
						Program.Literals.begin(), Program.Literals.end(), [&](const auto& Row)
						{
							return Row.strModuleId == DynamicModule->Row.strId &&
								Row.strPropertyPath == strPath;
						}));
					const auto It = std::find_if(Program.Literals.begin(),
						Program.Literals.end(), [&](const auto& Row)
						{
							return Row.strModuleId == DynamicModule->Row.strId &&
								Row.strPropertyPath == strPath;
						});
					return Count == 1u ? It : Program.Literals.end();
				};
				static constexpr std::array<std::string_view, 4u> DynamicNames{{
					"distortion[0-x]", "pan[1-2]", "uv.y_controll[1-x]",
					"uvdistort[0-x]" }};
				for (size_t iLane = 0u; iLane < DynamicNames.size(); ++iLane)
				{
					for (const std::string_view strSuffix : {
						std::string_view("bscalevelocitybyparamvalue"),
						std::string_view("bspawntimeonly"),
						std::string_view("buseemittertime") })
					{
						const auto Literal = FindDynamicLiteral("dynamicparams[" +
							std::to_string(iLane) + "]." + std::string(strSuffix));
						if (Literal == Program.Literals.end() ||
							Literal->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN ||
							Literal->bValue.value_or(true))
						{
							strOutError = "Artist F flow-02 dynamic flag changed: " +
								Occurrence->Row.strId;
							return false;
						}
					}
					const auto Name = FindDynamicLiteral("dynamicparams[" +
						std::to_string(iLane) + "].paramname");
					const auto Method = FindDynamicLiteral("dynamicparams[" +
						std::to_string(iLane) + "].valuemethod");
					if (Name == Program.Literals.end() || Method == Program.Literals.end() ||
						Name->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING ||
						Name->strEnumValue != DynamicNames[iLane] ||
						Method->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING ||
						Method->strEnumValue != "edpv_userset")
					{
						strOutError = "Artist F flow-02 dynamic semantic changed: " +
							Occurrence->Row.strId;
						return false;
					}
				}
				const auto LodValidity = FindDynamicLiteral("lodvalidity");
				const auto UpdateFlags = FindDynamicLiteral("updateflags");
				if (LodValidity == Program.Literals.end() ||
					UpdateFlags == Program.Literals.end() ||
					LodValidity->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
					LodValidity->fValue.value_or(-1.0) != 3.0 ||
					UpdateFlags->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
					UpdateFlags->fValue.value_or(-1.0) != 15.0)
				{
					strOutError = "Artist F flow-02 dynamic module literals changed: " +
						Occurrence->Row.strId;
					return false;
				}

				SetVisualV4Params(7u, 0x0fu, {
					{ 0.5f, 1.f, 0.4000000059604645f, -0.029999999329447746f },
					{ 1.f, 3.f, 0.f, 1.f },
					{ -1.f, 0.4000000059604645f, 1.f, 1.5f },
					{ 2.5f, 60.f, 2.f, 1.2000000476837158f },
					{ 1.f, 3.f, 0.f, 0.f } },
					{ 1.f, 1.f, 1.f, 10.f }, {});
				/* The recovered flow equation consumes all four ParameterDynamic
				   lanes.  Preserve that authored contract when the prepared packet is
				   materialized into a standalone Effect document. */
				Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
				Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
				ArtistVisualV4TextureLanes = {
					{ "01.map_a", "fx_tex_02.fx_d_trail_002_cl",
						"Effect/Artist/Textures/fx_d_trail_002_cl.dds",
						"material-input-3e3a826a40153b8d",
						"c994d711b8b18b1c68d059fb7f86d6768ca00f7b5568c0e0d2e8b89d1c6b4345",
						"520a25671a7c799be5b2ee2a6543692774ab43f56b7e3215525db77c96938215",
						1u, 32896u,
						"79c6c94326a2995ecf224e667931207a912cf8fbbe21ab8de7fdf9f0f86935d2" },
					{ "01.map_e", "fx_tex_04.fx_i_noise_03",
						"Effect/Artist/Textures/fx_i_noise_03.dds",
						"material-input-35f5337597f009ac",
						"a34a5f386348419d5250f891d5c6e1fe18cecdd91fa54893a1ce9b1ca4a2f13f",
						"b6c5544732c75a3f36de57e31ea8500a1d5cd2962c4bd14008b270c21eb6feee",
						2u, 131200u,
						"a051e6562632c46f76aa100d40840cd33d260cac3f9be86b89b0c127ab15dd65" }
				};
				break;
			}
			case 15u:
				if (!ValidateTypedScalars({
					{ "diff1_tile_u", 1.2 }, { "diff1_tile_v", 1.0 },
					{ "diff2_tile_u", 0.5 }, { "diff2_tile_v", 0.5 },
					{ "flow_tile_u", 1.0 }, { "flow_tile_v", 2.0 },
					{ "flow_pan_v", -0.2 },
					{ "opacity_tile_u", 1.0 }, { "opacity_tile_v", 1.0 },
					{ "opacity_rot", 0.0 }, { "opacity_str", 1.0 },
					{ "diff2_pan_v", 0.0 }, { "flow_bias", 0.2 },
					{ "opacity_distort_str", 0.3 },
					{ "diff_pow", 1.0 }, { "diff_str", 30.0 },
					{ "cameravec_pow", 1.0 } }))
					return false;
				SetVisualV4Params(2u, 0x0fu, {
					{ 1.2f, 1.f, 0.5f, 0.5f },
					{ 1.f, 2.f, 0.f, -0.2f },
					{ 1.f, 1.f, 0.f, 1.f },
					{ 0.f, 0.f, 0.2f, 0.3f },
					{ 1.f, 30.f, 1.f, 0.f } },
					{ 1.f, 1.f, 1.f, 1.f }, {});
				ArtistVisualV4TextureLanes = {
					{ "diff_tex1", "fx_tex_05.fx_k_auraline_20", "Effect/Artist/Textures/fx_k_auraline_20.dds" },
					{ "diff_tex2", "fx_tex_05.fx_k_caustictile_01", "Effect/Artist/Textures/fx_k_caustictile_01.dds" },
					{ "flowtex", "fx_tex_02.fx_d_noise_002", "Effect/Artist/Textures/fx_d_noise_002.dds" },
					{ "opacity_tex", "fx_tex_05.fx_k_auraline_19", "Effect/Artist/Textures/fx_k_auraline_19.dds" } };
				break;
			case 18u:
				if (!ValidateTypedScalars({
					{ "alpha_tex_texcoord_x", 1.0 }, { "alpha_tex_texcoord_y", 1.0 },
					{ "alpha_tex_dynamicpanspeed_x", 0.0 },
					{ "alpha_tex_dynamicpanspeed_y", 0.3 },
					{ "alpha_disslove_tex_coord_x", 1.0 },
					{ "alpha_disslove_tex_coord_y", 1.0 },
					{ "dissolve_tex_dynamicpanspeed_x", 0.0 },
					{ "dissolve_tex_dynamicpanspeed_y", 0.2 },
					{ "emissive_tex01tile_dynamicpanspeed_x", 0.1 },
					{ "emissive_tex01tile_dynamicpanspeed_y", 0.2 },
					{ "emissive_tex02tile_dynamicpanspeed_x", 0.0 },
					{ "emissive_tex02tile_dynamicpanspeed_y", 0.0 },
					{ "uvnoise_tex_01_texcoord_x", 1.5 },
					{ "uvnoise_tex_01_texcoord_y", 2.5 },
					{ "uvnoise_tex_dynamicpanspeed_x", -0.4 },
					{ "uvnoise_tex_dynamicpanspeed_y", 0.0 },
					{ "alpha01_tex_power", 1.0 }, { "alpha_tex_strength", 4.0 },
					{ "dissolve_hardness", 15.0 }, { "maintex_uv_noise_velue", 0.5 },
					{ "emissive_tex_power", 2.0 },
					{ "emissive_tex_strength", 5000.0 },
					{ "fresnelalpha_power", 1.5 } }))
					return false;
				SetVisualV4Params(3u, 0x1fu, {
					{ 1.f, 1.f, 0.f, 0.3f },
					{ 1.f, 1.f, 0.f, 0.1f },
					{ 0.1f, 0.2f, 0.f, 0.f },
					{ 1.5f, 2.5f, -0.4f, 0.f },
					{ 1.f, 4.f, 0.f, 15.f },
					{ 0.5f, 2.f, 5000.f, 1.5f } },
					{ 1.f, 1.f, 1.f, 1.f }, {});
				ArtistVisualV4TextureLanes = {
					{ "alpha_tex", "fx_tex_05.fx_m_trail_007", "Effect/Artist/Textures/fx_m_trail_007.dds" },
					{ "uv_dissolve_tex", "fx_tex_05.fx_m_noise_001", "Effect/Artist/Textures/fx_m_noise_001.dds" },
					{ "emissive_tex01", "fx_tex_04.fx_h_atypical_01_1", "Effect/Artist/Textures/fx_h_atypical_01_1.dds" },
					{ "emissive_tex02", "fx_tex_05.fx_m_noise_003", "Effect/Artist/Textures/fx_m_noise_003.dds" },
					{ "uv_noise_tex", "fx_tex_02.fx_d_noise_030", "Effect/Artist/Textures/fx_d_noise_030.dds" } };
				break;
			case 25u:
			case 29u:
			{
				const bool_t bActive025 = Occurrence->Row.iOrder == 25u;
				const std::string_view strExpectedRecipeRowSha256 = bActive025 ?
					"4b0ad95831d69e16acc0c9703de1f7f0ce88b03c1f9220545b91a21c744c7b8b" :
					"f73a051c0fae2ff7fb28842afd48691aa95de38b64ae79f33b3bb355fc5a3a03";
				const std::string_view strExpectedOccurrenceRowSha256 = bActive025 ?
					"ea48d39423e4e1060591690cbdf707eed32b0125da871d65a718b15734748d9c" :
					"af7ae7047a2308318becae8f605bafd3a64428cceebfb46470b35db92795b674";
				const std::string_view strExpectedEmitterRowSha256 = bActive025 ?
					"e5ae5dae0370f233306afbb8a1af178b3ceb53ba398de50f8d42baa9678610b5" :
					"e9a471c39bdde31b984c6911d1580deb26ad731557831d548d1ac1b67b842385";
				if (Recipe->Row.strRowSha256 != strExpectedRecipeRowSha256 ||
					Occurrence->Row.strRowSha256 != strExpectedOccurrenceRowSha256 ||
					Emitter.Row.strRowSha256 != strExpectedEmitterRowSha256 ||
					Recipe->InputIds.size() != 27u ||
					Recipe->StaticBindingIds.size() != 3u ||
					Recipe->RenderBindingIds.size() != 6u ||
					Element.Material.eRenderProfile !=
						EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
					!ValidateTypedScalars({
						{ "07.map_a_uvscale_r", 1.0 },
						{ "08.map_a_uvscale_g", 1.0 },
						{ "09.map_a_panning_x", 0.0 },
						{ "10.map_a_panning_y", 0.30000001192092896 },
						{ "11.str", bActive025 ? 3.0 : 8.0 },
						{ "12.power", bActive025 ? 1.0 : 2.0 },
						{ "02.spacular_str", bActive025 ? 60.0 : 30.0 },
						{ "03.spacular_power", 2.0 },
						{ "06_spacular_uvscale", bActive025 ? 8.0 : 12.0 },
						{ "07_spacular_timescale", bActive025 ? 5.0 : 3.0 },
						{ "05.distort_str", bActive025 ?
							0.05000000074505806 : 0.07000000029802322 },
						{ "07.map_d_uvscale_r", 1.0 },
						{ "08.map_d_uvscale_g", 1.0 },
						{ "09.map_d_panning_x", 0.20000000298023224 },
						{ "10.map_d_panning_y", 0.5 },
						{ "01.distortion_str", bActive025 ? 20.0 : 10.0 },
						{ "05.uv.y", 1.0 }, { "04.uv.x", 1.0 },
						{ "20.min_alpha", 0.5 },
						{ "42.fresnal_str", 2.0 },
						{ "41.fresnal_power", 3.0 } }))
				{
					strOutError = "Artist F SPLA V4 structural/scalar contract changed: " +
						Occurrence->Row.strId;
					return false;
				}

				const auto ReadTypedVector = [&](const std::string_view strName,
					const std::array<double, 4u>& Expected, float4_t& Out)
				{
					const auto It = std::find_if(Program.MaterialInputs.begin(),
						Program.MaterialInputs.end(), [&](const auto& Row)
						{
							return Row.strRecipeId == Recipe->Row.strId &&
								Row.strNormalizedParameterName == strName;
						});
					const size_t iCount = static_cast<size_t>(std::count_if(
						Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
						[&](const auto& Row)
						{
							return Row.strRecipeId == Recipe->Row.strId &&
								Row.strNormalizedParameterName == strName;
						}));
					return iCount == 1u && It != Program.MaterialInputs.end() &&
						It->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 &&
						It->vValue == Expected && !It->strTypedValueSha256.empty() &&
						!It->strSourceFieldValueSha256.empty() &&
						AssignFloat4(It->vValue, Out);
				};
				float4_t BaseColor{};
				float4_t SpecularColor{};
				const std::array<double, 4u> ExpectedBaseColor = bActive025 ?
					std::array<double, 4u>{ 0.6744120121002197, 0.8700000047683716,
						0.9371190071105957, 1.0 } :
					std::array<double, 4u>{ 0.07182055711746216, 0.05495157092809677,
						0.03372928872704506, 1.0 };
				const std::array<double, 4u> ExpectedSpecularColor = bActive025 ?
					std::array<double, 4u>{ 9.0, 9.5, 10.0, 1.0 } :
					std::array<double, 4u>{ 6.0, 5.400000095367432, 4.0, 1.0 };
				if (!ReadTypedVector("01.color", ExpectedBaseColor, BaseColor) ||
					!ReadTypedVector("04.spacular_color", ExpectedSpecularColor,
						SpecularColor))
				{
					strOutError = "Artist F SPLA V4 typed color contract changed: " +
						Occurrence->Row.strId;
					return false;
				}
				SetVisualV4Params(6u, 0x0fu, {
					{ 1.f, 1.f, 0.2f, 0.5f },
					{ 1.f, 1.f, 0.f, 0.30000001192092896f },
					{ bActive025 ? 0.05000000074505806f : 0.07000000029802322f,
						bActive025 ? 3.f : 8.f, bActive025 ? 1.f : 2.f, 0.5f },
					{ bActive025 ? 60.f : 30.f, 2.f,
						bActive025 ? 8.f : 12.f, bActive025 ? 5.f : 3.f },
					{ 1.f, 0.f, 0.f, 0.f } }, BaseColor, SpecularColor);
				/* SPLA consumes the life-cut and warp lanes (Y/Z).  X and W belong
				   to source lanes that this bounded RT0 program intentionally does
				   not read. */
				Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x06u;
				Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x09u;
				if (bActive025)
				{
					ArtistVisualV4TextureLanes = {
						{ "06.map", "fx_tex_02.fx_d_noise_021",
							"Effect/Artist/Textures/fx_d_noise_021.dds",
							"material-input-a9c25d6f93e51403",
							"035cb4dd7bf88383bfd665a15ce78a090a4d6d45b97dea67863764510e0fc386",
							"5643d6cc434877aacc5438cd9b36359c6059fc0917815951b4702da566dfdb34" },
						{ "01.specmap_a", "fx_tex_03.fx_e_noise_008",
							"Effect/Artist/Textures/fx_e_noise_008.dds",
							"material-input-bdb70d9010d428cf",
							"71369771d97482c1a5682ce6120a1581048f830a2a783a2b9877c3bc1bee39c0",
							"eab261ceec7c38f6406f84cdbbce27c4d7131714d06f296c87e17dc044d58a07" },
						{ "00.map_alpha", "fx_tex_02.fx_d_fluid_013",
							"Effect/Artist/Textures/fx_d_fluid_013.dds",
							"material-input-cc8fba7dad283ae7",
							"53c680004d6c87e196a5b5fe2f6af4d24fa2eee5b7888469d6148afb920d3678",
							"c7d9a76514fa18b134ff3f4fef019e3a9369243ec0a8705111ef2e1c09f3cfe5" },
						{ "06.map_a", "fx_tex_01.fx_c_noise_009",
							"Effect/Artist/Textures/fx_c_noise_009.dds",
							"material-input-815bfc0630d9a68a",
							"e25aa0b5eaf6955915fddacc4946ffc987543fc8a8cd50fb6b2328d0f0aad7fc",
							"de1d19cce87bd1caf4c9f380ba11b37b919bcf24fc923abb2d6435d553f7ef8f" }
					};
				}
				else
				{
					ArtistVisualV4TextureLanes = {
						{ "06.map", "fx_tex_02.fx_d_noise_021",
							"Effect/Artist/Textures/fx_d_noise_021.dds",
							"material-input-6b91ebdb0fe18f90",
							"f5d1525f55caffeda0740eeb89a3c456db3dd281a38bc908a961f9cb53a78c28",
							"2a57a8c479bb1bbdccfdc71702fd87c8f25ed47c125d4d205747da40fe85f7f2" },
						{ "01.specmap_a", "fx_tex_03.fx_e_noise_008",
							"Effect/Artist/Textures/fx_e_noise_008.dds",
							"material-input-21152204a54f1241",
							"4e0e522369e73fa6e2d135c504b0163493e13f7b331c3724700c041c359defcd",
							"bf2ed46b6d118a166767d03603bbf5783aae9006cf37b649f1f624073d13636e" },
						{ "00.map_alpha", "fx_tex_02.fx_d_fluid_013",
							"Effect/Artist/Textures/fx_d_fluid_013.dds",
							"material-input-f025bcccbe3547b1",
							"eebc629313ff99cae34681c6c80d3e3535e615b3aed32c78246d75c3a17f98c5",
							"77bd67975bda73bd84c40c1ddf55f09661a9caa8df6ae850bd8bf06c88fe18b8" },
						{ "06.map_a", "fx_tex_01.fx_c_noise_009",
							"Effect/Artist/Textures/fx_c_noise_009.dds",
							"material-input-9b837252d3d0ae52",
							"b59a72c716b8030230df58f1814bc6ad58d9127297b5fd07b10d5a9a13f2088c",
							"9a1d184a24f166755514191a10e13264b453840b37c5b3ce97fa54f0555f3ac0" }
					};
				}
				break;
			}
			case 33u:
				if (!ValidateTypedScalars({
					{ "uv_noise_01_tiling_x", 3.0 },
					{ "uv_noise_01_tiling_y", 4.0 },
					{ "uv_noise_01_panning_y", 5.0 },
					{ "distortion_intensity", -40.0 } }))
					return false;
				SetVisualV4Params(4u, 0x03u, {
					{ 1.f, 1.f, 3.f, 4.f },
					{ 0.f, 0.f, 0.f, 5.f },
					{ -40.f, 1.f, 1.f, 128.f } }, {}, {});
				break;
			default:
				return false;
			}
		}
		if (bActive005006RuntimeV2)
		{
			struct SMOKE_OCCURRENCE_PROFILE final
			{
				uint32_t iOrder;
				std::string_view strOccurrenceId;
				std::string_view strOccurrenceRowSha256;
				std::string_view strOccurrenceIdentitySha256;
				std::string_view strOccurrenceBindingSha256;
				std::string_view strEmitterRowSha256;
				std::string_view strSelectedLodPath;
				std::string_view strSelectedLodNodeId;
				std::string_view strSelectedLodRowSha256;
				std::string_view strBurstProjectionSha256;
				double fDelay;
				uint32_t iBurstCount;
				std::array<std::string_view, 11u> ModuleRows;
				std::array<std::string_view, 4u> DynamicDistributionRows;
			};
			static constexpr std::array<std::string_view, 11u>
				EXPECTED_MODULE_CLASSES = {{
					"particlemodulerequired", "particlemodulelifetime",
					"particlemodulesize", "particlemodulesubuv",
					"particlemodulesizemultiplylife", "particlemodulecolor",
					"particlemodulecolorscaleoverlife",
					"particlemoduleparameterdynamic", "particlemodulerotation",
					"efparticlemodulelocationprimitivecylinderspin_seeded",
					"particlemodulespawn"
				}};
			static constexpr std::array<SMOKE_OCCURRENCE_PROFILE, 2u>
				SMOKE_PROFILES = {{
					{ 5u, "source-active-005",
						"732adc98287b37f7abc4aae31405d2cc9932e2400aec00f6550ebe7a36c04b36",
						"b4a40b5d84e986e09cc3c037a811acc9fd518632e4ba766a175159c5c251b4e3",
						"1fd7d23e06b00ea62129a286cd4ba3a75ffc0b4f48c045f4f7ba667c04e7b33d",
						"ac42ab12d87d656a686ae54ca92e3eaff8f071cf403b5c7f63622ea94fbfc218",
						"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_23.particlelodlevel_30",
						"FX_PC_SDM_07:export:519",
						"88153d4022633ab5949cfa29641dd051e915e306f6e98c1c06d4a1ba89a200b1",
						"8d90e075040205d3590967fa1a5798c52faf2f7dc3bca431ad19a068fe23efbd",
						0.05, 5u,
						{{ "bdf232b0f5b5960543ffb3c880de36f3cc38e14c213f7848397c8deb9c30cb8c",
							"f4020f8d642bf57cddcce6a606a627195d811c459eef9e25b22dac62bb439ceb",
							"a3422838657486eb4fcf871d044ef42dbe9852a11f74fcd62f52cd8e34cfb77f",
							"36790957f29e8c2f0698ec1678f5e2554a5b0aae8c7dcab9bf0c7a8127aaf1d1",
							"9170aee3b939b2090223e2ca0015e78d23c9a33d0db94cacf108c297c9546cef",
							"0be240014844cb10482293c4ecf6ff129bb2e5cd5418201d6bee8af266714001",
							"af52ad79435168e7553dc9bbd040dff94efed11195e962aa1a0cce3bd096ff50",
							"530fd84961c749273eaa16986ae209dbce30f4978c0bdc426479591235f9cf3a",
							"220daacc30193ba793560391d93c2def344a73ee73e4cbc6b268c7cf7e7b7eb9",
							"4d14e8eaece06389be6702efe5c6ac4bc4069914a339245a1eeb8971f5c6b237",
							"cb8a9f253ce6fa12758985e071fff52616fdcfc0a4902b1a2b765e5707aa678f" }},
						{{ "c67ff596baa2d4d67553e5c24640e6ce8614a89d597d5dbba49d4d81366dde51",
							"b88c09e06459c4c256c09e71feeb20478476be05674eb236dac41b39f57d1c80",
							"87b0089b8b6a3d17947a0ac57f9cfd6e7a1cf83b3a2d35e0cc07d240a02966c6",
							"13c751c1cd823f685ba6f98712bc5488b7877e101726d94b4bce8295b414c901" }} },
					{ 6u, "source-active-006",
						"a6c18145bfe4e4605da6ca762eb87786bcb91d51937969218b23a4d5e08df180",
						"135c50750688996234776edb4fa7b3b2286516c66686d0e5d8517370fb1213f0",
						"553b393c6d5a91af700b0c1881c06044f51d9d0e14923ee43e810e0508e644fb",
						"aad0501641477b7de98b44995741a037e3523b8d82d5d4693734e6268d1216d1",
						"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_26.particlelodlevel_36",
						"FX_PC_SDM_07:export:521",
						"4978055da7bfdf3e3fdde17a0aa293dd7aaf4dea6b3dcdf55e331505bc6ca831",
						"20e2c7a81fa589cb4af669793ac5f9ac56d6c8726a2a1e5c9b543f1f3d519772",
						0.1, 6u,
						{{ "cd65ac9802b919be97368a90f44ad32b7ba76c7a1ec88f8f31e60ea1181fa2b1",
							"357d2ca44431c481be6478f0467c46a95fed99f0affb6ac213efa645d614c3df",
							"bb0114214492f2d7e8401d0a26ab7ecddf575d7cce75545ceb21b44b4eff9991",
							"8ab40838bce99ee81e63bb9db1b3dfcfb1209a5f58a17b1b5485fa216ce11ee5",
							"4e328f37cc5de3cefa7507f2863f14b4e12c776dce44b250bc384b743f6139c5",
							"7e1ebf52734867854bbbe980f9e60de95afd6babf67666af1c7decb786632de6",
							"2cbdf37951f21c90757c589936aaa5948e7cd92b0a1ba650d54a3f82e6a58452",
							"dc74b0847621bae1cc661d0e7fd0d8c4d1964de2861c315987d7ac93fb3629c9",
							"1c17b877d4b85281c6375b959ae04efbecce66dc28e0ca3f1a6993e029a8b5b0",
							"b949513769d7ca9f328a777d387aa640da06c1293e5df3330f07e1c683fc47a2",
							"0b5287f917a7bcde36d162a3d02477cd3913362482422681cd0217849ae9d4b9" }},
						{{ "fcac1896024f78e87174b39643b5a8d8967cecae8bb56bdbf9b8e57291231d6c",
							"13c4dfead6ea25577832da7870446d6b0a836a7a95e12430603fc56453e6363c",
							"a1f67e0de9a95dded18b85186d89854f7473f0c040abebad886958828777ae1a",
							"94fff16acaea0630bd4942ae758af8c37ad697c9cea14d6c07c8104673fb3362" }} }
				}};
			const SMOKE_OCCURRENCE_PROFILE& Smoke =
				SMOKE_PROFILES[Emitter.Row.iOrder - 5u];
			/* Cache PS 3e38be239225c7498a67fd9d3d400d24 selects the
			   descriptor-local 2004-byte DXBC (SHA-256 85ca55fa...).  The adjacent
			   1936-byte descriptor is a different shader and is never concatenated.
			   Raw DXBC/native VF admission remains false in the stable registry. */
			if (Smoke.iOrder != Emitter.Row.iOrder ||
				Occurrence->Row.strId != Smoke.strOccurrenceId ||
				Occurrence->Row.strRowSha256 != Smoke.strOccurrenceRowSha256 ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					Smoke.strOccurrenceIdentitySha256 ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					Smoke.strOccurrenceBindingSha256 ||
				Emitter.Row.strRowSha256 != Smoke.strEmitterRowSha256 ||
				Emitter.strSelectedLodPath != Smoke.strSelectedLodPath ||
				Emitter.strSelectedLodNodeId != Smoke.strSelectedLodNodeId ||
				Emitter.strSelectedLodRecordSha256 != Smoke.strSelectedLodRowSha256 ||
				Emitter.strSourceActionCueProjectionSha256 !=
					"737d0046b1e1d2c91df2cbdfe3e78458d2fe3cb9ad40fbf7e656c6c29a204a1b" ||
				Recipe->Row.strId != "material-recipe-533e2243c834e448" ||
				Recipe->Row.strRowSha256 !=
					"d41f652df8d580d1ba4ecc9fedd42cbd010dc06bb39705e0b8f1c616fb785fbe" ||
				Recipe->strSourceMaterialPath !=
					"fx_m_mi_m_00.fx_mi.fx_m_pa_sqc_01_01_tr" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"c23d3840d278186f769bc328afc9d6dcd06ddf49ce3c7c7cd4392b8c93867710" ||
				Recipe->strBindingSha256 !=
					"f1e2572f75f7724f87e88833eede8f129b7f53081d59ef610f4211883f4931d2" ||
				Family->Row.strId != "material-family-b53107e635922285" ||
				Family->Row.strRowSha256 !=
					"7d3392049f6c02a6508f4bc606bc71048088131ccfc964a6b2a08e9aaa2cc4be" ||
				Family->strFamilyIdentitySha256 !=
					"f250e9a23df45f28ccfc29f8a12498edc3f1edd5e097d7a2e615a4c3a27e5df5" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-df3d5d2fe8c0cd39" ||
				Family->strEvaluatorSha256 !=
					"f5de0a3906e6ae942a967860666a23ec4780b98f58716e32c0b7df3f098023a8" ||
				Recipe->NumericBindingSamples.size() != 4u ||
				!std::all_of(Recipe->NumericBindingSamples.begin(),
					Recipe->NumericBindingSamples.end(), [](const auto& Sample)
					{
						return Sample.vUvScale == std::array<double, 2u>{ 6., 6. } &&
							Sample.vParams0 == std::array<double, 4u>{
								0.029999999329447746, 1., -1., 50. } &&
							Sample.vParams1 ==
								std::array<double, 4u>{ 0.5, 0.5, 0., 0. };
					}))
			{
				strOutError = "Artist F smoke-square cache/program identity changed: " +
					Occurrence->Row.strId;
				return false;
			}
			if (Emitter.ModuleIds.size() != EXPECTED_MODULE_CLASSES.size())
			{
				strOutError = "Artist F smoke-square module denominator changed.";
				return false;
			}
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			for (size_t iModule = 0u; iModule < Emitter.ModuleIds.size(); ++iModule)
			{
				const auto Module = std::find_if(Program.Modules.begin(),
					Program.Modules.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Emitter.ModuleIds[iModule];
					});
				if (Module == Program.Modules.end() || Module->Row.iOrder != iModule ||
					Module->strExactSourceClass != EXPECTED_MODULE_CLASSES[iModule] ||
					Module->Row.strRowSha256 != Smoke.ModuleRows[iModule])
				{
					strOutError = "Artist F smoke-square module identity changed.";
					return false;
				}
				if (iModule == 7u)
					DynamicModule = &*Module;
			}
			bool_t bDynamicExact = nullptr != DynamicModule &&
				DynamicModule->strSourceRecordSha256 ==
					"8796226e6e3151645b67547bcc53dd63cb76518ea69236578e3e64d6cf71a54f" &&
				DynamicModule->DistributionIds.size() == 4u;
			for (size_t iLane = 0u; bDynamicExact && iLane < 4u; ++iLane)
			{
				const auto Distribution = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == DynamicModule->DistributionIds[iLane];
					});
				bDynamicExact = Distribution != Program.Distributions.end() &&
					Distribution->strModuleId == DynamicModule->Row.strId &&
					Distribution->Row.iOrder == iLane &&
					Distribution->Row.strRowSha256 ==
						Smoke.DynamicDistributionRows[iLane] &&
					Distribution->eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
					Distribution->iComponentCount == 1u &&
					Distribution->iOperation.value_or(UINT32_MAX) == 1u &&
					Distribution->strPropertyPath == "dynamicparams[" +
						std::to_string(iLane) + "].paramvalue" &&
					Distribution->LookupTable == std::vector<double>{ 1., 1., 1., 1. } &&
					!Distribution->Samples.empty() &&
					std::all_of(Distribution->Samples.begin(), Distribution->Samples.end(),
						[](const auto& Sample)
						{
							return Sample.OutputValues ==
								std::vector<double>{ 1., 0., 0., 0. };
						});
			}
			const bool_t bLinearBlendSubUv =
				Element.Material.SourceMaterial.strSubUVMode ==
					"psuvim_linear_blend" ||
				SourceLiteralString(Element, "interpolationmethod") ==
					"psuvim_linear_blend";
			if (!bDynamicExact || !Element.SourceRecipe.bEnabled ||
				!bLinearBlendSubUv ||
				SourceLiteralNumber(Element, "subimages_horizontal", -1.f) != 6.f ||
				SourceLiteralNumber(Element, "subimages_vertical", -1.f) != 6.f ||
				SourceLiteralNumber(Element, "randomimagetime", -1.f) != 1.f ||
				!SourceLiteralBool(Element, "ballowimageflipping", false) ||
				!SourceLiteralBool(Element, "bsquareimageflipping", false) ||
				std::abs(Emitter.Timing.fEmitterDelaySeconds - Smoke.fDelay) > 1e-8 ||
				std::abs(Emitter.Timing.fEmitterDurationSeconds - 0.2) > 1e-8 ||
				Emitter.Timing.iEmitterLoopCount != 1u ||
				Emitter.Timing.Bursts.size() != 1u ||
				Emitter.Timing.Bursts[0u].fTimeSeconds != 0. ||
				Emitter.Timing.Bursts[0u].iCountMinimum != Smoke.iBurstCount ||
				Emitter.Timing.Bursts[0u].iCountMaximum != Smoke.iBurstCount ||
				Emitter.Timing.Bursts[0u].strSourceLiteralProjectionSha256 !=
					Smoke.strBurstProjectionSha256)
			{
				strOutError = "Artist F smoke-square SubUV/dynamic/timing contract changed.";
				return false;
			}

			struct EXPECTED_INPUT final
			{
				std::string_view strId;
				std::string_view strName;
				std::string_view strRowSha256;
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
				double fValue;
				std::string_view strTextureId;
			};
			static constexpr std::array<EXPECTED_INPUT, 6u> EXPECTED_INPUTS = {{
				{ "material-input-7e47cca6c521d831", "02.lod",
					"9102c93af2adb4cc85278a148161a4b648624e57a28aede0c1f362ae7cbf7267",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, -1., {} },
				{ "material-input-9cc55b7e91bf7dea", "02.uv.y",
					"3929a67a4c365b2832a074b8f620c457efc06c689f60935ff21b963b3fd77085",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 6., {} },
				{ "material-input-eae2bf7e4e89447f", "10.shadowstr",
					"d51c87ef875cba649f985dcc99fb2b0ebeed65803142700f61fceddde567c4ac",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64,
					0.029999999329447746, {} },
				{ "material-input-3084d12ada62be0f", "01.map",
					"b06d607464cc0368651d44e49d99184572892af29ea1a314a4b4f2450c5d2457",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0.,
					"fx_tex_05.fx_m_smokesq_01" },
				{ "material-input-1973a2afdf4d5446", "01.uv.x",
					"2326af1b8cc6f0b0b814b1aaf1c2e7cc96de38177fdee4dcb9b0afaa6b4482d6",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 6., {} },
				{ "material-input-a91c89f06d1dbd9d", "01.depthbiasdalpha_bias",
					"c723691af8de8814a636cda3eda6b9f4a52d04e6798f876b0f09fd33427c5868",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 50., {} }
			}};
			if (Recipe->InputIds.size() != EXPECTED_INPUTS.size())
				return false;
			for (size_t iInput = 0u; iInput < EXPECTED_INPUTS.size(); ++iInput)
			{
				const EXPECTED_INPUT& Expected = EXPECTED_INPUTS[iInput];
				const auto Input = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Expected.strId;
					});
				const bool_t bValueMatches = Input != Program.MaterialInputs.end() &&
					(Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ?
						Input->strStringValue == Expected.strTextureId :
						Input->fValue.has_value() && *Input->fValue == Expected.fValue);
				if (Recipe->InputIds[iInput] != Expected.strId ||
					Input == Program.MaterialInputs.end() ||
					Input->strRecipeId != Recipe->Row.strId ||
					Input->strNormalizedParameterName != Expected.strName ||
					Input->Row.strRowSha256 != Expected.strRowSha256 ||
					Input->eVariant != Expected.eVariant || !bValueMatches)
				{
					strOutError = "Artist F smoke-square typed input changed.";
					return false;
				}
			}
			static constexpr std::array<std::string_view, 2u> STATIC_IDS = {{
				"material-input-42b3438d5193f4fa",
				"material-input-4bb7a731b5332860" }};
			static constexpr std::array<std::string_view, 2u> STATIC_ROWS = {{
				"f2d7195a4560dfece784687bb95f4939bf9dd9cb522f5322b56ea60831ac5d14",
				"806ec3a4a2e29dfb0075f8744089977c62b3f72542f94aa6784b555453d37286" }};
			if (Recipe->StaticBindingIds.size() != STATIC_IDS.size())
				return false;
			for (size_t iStatic = 0u; iStatic < STATIC_IDS.size(); ++iStatic)
			{
				const auto Static = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == STATIC_IDS[iStatic];
					});
				if (Recipe->StaticBindingIds[iStatic] != STATIC_IDS[iStatic] ||
					Static == Program.MaterialStaticBindings.end() ||
					Static->Row.strRowSha256 != STATIC_ROWS[iStatic] ||
					Static->strNormalizedParameterName != "00.uselod" ||
					Static->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
					!Static->bSourceValue.value_or(false) ||
					!Static->bSelectedValue.value_or(false))
					return false;
			}
			static constexpr std::array<std::string_view, 6u> RENDER_ROWS = {{
				"3c380d0e66629e82f6ef5cff8c8733da6848c1d55662cdbc0bbd1dd8d6aa8e64",
				"5849563ba41f0c10dc894b6e1278e72a5404e2ffcd88a285df6c03d4c552afc1",
				"a7d9f8c449608c4273ade01e96719a4a8e3336c71922c7652ba844a9c9574341",
				"40cf64a189f5cdc754c1733e5a01871b9b0b0709cb39388b256e95b6f25faa71",
				"7f0314e5ec724ec9fe0a7f5483b574d8f05cd8255d2a3c6d94080fd578743a30",
				"f694c14e98b024b3470ea3710109a75303fc40023f99e0712e74ac24092a7bba" }};
			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion" }};
			if (Recipe->RenderBindingIds.size() != RENDER_ROWS.size())
				return false;
			for (size_t iRender = 0u; iRender < RENDER_ROWS.size(); ++iRender)
			{
				const auto Render = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[iRender];
					});
				if (Render == Program.MaterialRenderBindings.end() ||
					Render->Row.strRowSha256 != RENDER_ROWS[iRender] ||
					Render->strFieldName != RENDER_FIELDS[iRender] ||
					(iRender == 0u && Render->strStringValue != "blend_translucent") ||
					(iRender == 1u && Render->strStringValue != "mlm_unlit") ||
					(iRender == 2u &&
						(!Render->bValue.has_value() || *Render->bValue)))
					return false;
			}
			if (Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F smoke-square selected render pass changed.";
				return false;
			}
			const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING* SmokeBinding = nullptr;
			for (const auto& [strBindingId, Binding] : Authority.TextureBindingsById)
			{
				(void)strBindingId;
				if (Binding.strCandidateBindingId ==
					"material-input-3084d12ada62be0f::runtime-texture-binding")
				{
					if (nullptr != SmokeBinding)
						return false;
					SmokeBinding = &Binding;
				}
			}
			const auto SmokeResource = nullptr == SmokeBinding ?
				Authority.TextureResourcesById.end() :
				Authority.TextureResourcesById.find(
					SmokeBinding->strResourceAuthorityId);
			if (nullptr == SmokeBinding ||
				SmokeBinding->strActualDdsRawSha256 !=
					"746b2efc5845973bda6f61b7390e9dfb0e3508a28a046584ba0a468e8210bcdf" ||
				SmokeBinding->MaterialOccurrenceIds !=
					std::vector<std::string>{ "source-active-005", "source-active-006" } ||
				SmokeBinding->SamplerDescriptor.Filter !=
					D3D11_FILTER_MIN_MAG_MIP_LINEAR ||
				SmokeBinding->SamplerDescriptor.AddressU != D3D11_TEXTURE_ADDRESS_WRAP ||
				SmokeBinding->SamplerDescriptor.AddressV != D3D11_TEXTURE_ADDRESS_WRAP ||
				SmokeBinding->SamplerDescriptor.AddressW != D3D11_TEXTURE_ADDRESS_WRAP ||
				SmokeBinding->ActualDdsSrv.eFormat != DXGI_FORMAT_BC3_UNORM_SRGB ||
				SmokeBinding->ActualDdsSrv.strColorSpace != "SRGB" ||
				SmokeResource == Authority.TextureResourcesById.end())
			{
				strOutError = "Artist F smoke-square texture authority changed.";
				return false;
			}

			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x02u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0du;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 5u;
			Resource.iRuntimeMaterialV2VectorCount = 0u;
			Resource.iRuntimeMaterialV2InputCount = 6u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x0fu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x30u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 2u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x03u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x03u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x03u;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x3cu;
			Resource.RuntimeMaterialV2ScalarBlocks[0u] =
				{ -1.f, 6.f, 6.f, 0.029999999329447746f };
			Resource.RuntimeMaterialV2ScalarBlocks[1u] = { 0.5f, 1.f, 0.f, 0.f };
			RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
				"material-input-3084d12ada62be0f",
				"b06d607464cc0368651d44e49d99184572892af29ea1a314a4b4f2450c5d2457",
				"material-input-3084d12ada62be0f::runtime-texture-binding",
				"ab29be7ce30809fa87bb29f366873d446d97ddfb5814c5b6d9ba3ab63a5700c9",
				"material-reconstructed-policy-a8df058fb50ca74ca66f",
				"ea5ea59d0fa9c836406191918b2e0395a50837b92070e5357b9374ab68ca6391",
				"Effect/Artist/Textures/fx_m_smokesq_01.dds",
				"EXACT_CACHE_PS_TEXTURE_SLOT_T0" });
		}
		if (bActive004RuntimeV2)
		{
			static constexpr std::array<std::string_view, 4u>
				EXPECTED_TEXTURE_NAMES = {
					"diffuse_tex", "normal_tex", "spec_tex", "dissolve_texture"
				};
			static constexpr std::array<std::string_view, 6u>
				EXPECTED_SCALAR_NAMES = {
					"specural_power", "specural_intensity",
					"dissolve_line_thickness", "transition_area",
					"emissive_intensity", "fresnel_intensity"
				};
			if (Recipe->Row.strId != "material-recipe-a4ee2b242b08bb39" ||
				Family->Row.strId != "material-family-2c00ce5593538d7c" ||
				Occurrence->Row.strId != "source-active-004" ||
				Recipe->InputIds.size() != 11u ||
				Recipe->StaticBindingIds.size() != 0u ||
				Recipe->RenderBindingIds.size() != 6u)
			{
				strOutError =
					"Artist F active004 runtime-material identity changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 4u>
				EXPECTED_DYNAMIC_PARAMETER_NAMES = {
					"alpha", "bloodline", "finger_up", "distortion[0-x]"
				};
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			uint32_t iDynamicModuleCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto ModuleIt = std::find_if(Program.Modules.begin(),
					Program.Modules.end(), [&](const auto& Row)
					{
						return Row.Row.strId == ModuleId;
					});
				if (ModuleIt == Program.Modules.end())
				{
					strOutError =
						"Artist F active004 module identity changed.";
					return false;
				}
				if (ModuleIt->strExactSourceClass ==
					"particlemoduleparameterdynamic")
				{
					DynamicModule = &*ModuleIt;
					++iDynamicModuleCount;
				}
			}
			bool_t bExactDynamicContract = iDynamicModuleCount == 1u &&
				nullptr != DynamicModule && DynamicModule->Row.iOrder == 6u &&
				DynamicModule->DistributionIds.size() ==
					EXPECTED_DYNAMIC_PARAMETER_NAMES.size();
			for (size_t iDynamic = 0u;
				bExactDynamicContract &&
				iDynamic < EXPECTED_DYNAMIC_PARAMETER_NAMES.size(); ++iDynamic)
			{
				const std::string NamePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramname";
				const std::string ValuePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramvalue";
				const EFFECT_RUNTIME_PROGRAM_LITERAL* NameLiteral = nullptr;
				uint32_t iNameLiteralCount = 0u;
				for (const EFFECT_RUNTIME_PROGRAM_LITERAL& Literal : Program.Literals)
				{
					if (Literal.strModuleId == DynamicModule->Row.strId &&
						Literal.strPropertyPath == NamePath)
					{
						NameLiteral = &Literal;
						++iNameLiteralCount;
					}
				}
				const std::string& DistributionId =
					DynamicModule->DistributionIds[iDynamic];
				const auto DistributionIt = std::find_if(
					Program.Distributions.begin(), Program.Distributions.end(),
					[&](const auto& Row)
					{
						return Row.Row.strId == DistributionId;
					});
				bExactDynamicContract = nullptr != NameLiteral &&
					iNameLiteralCount == 1u &&
					NameLiteral->eVariant ==
						EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					NameLiteral->strEnumValue ==
						EXPECTED_DYNAMIC_PARAMETER_NAMES[iDynamic] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strModuleId == DynamicModule->Row.strId &&
					DistributionIt->Row.iOrder == iDynamic &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u;
			}
			if (!bExactDynamicContract)
			{
				strOutError =
					"Artist F active004 dynamic-parameter contract changed.";
				return false;
			}
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x01u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0eu;
			Resource.iRuntimeMaterialV2ScalarCount = 6u;
			Resource.iRuntimeMaterialV2VectorCount = 1u;
			Resource.iRuntimeMaterialV2InputCount = 11u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x7ffu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				{ 0x0fu, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				{ 0u, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 0u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x3fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0u;

			const auto FindRecipeInput = [&](const std::string_view Name)
				-> const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*
			{
				const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Found = nullptr;
				for (const std::string& InputId : Recipe->InputIds)
				{
					const auto It = std::find_if(Program.MaterialInputs.begin(),
						Program.MaterialInputs.end(), [&](const auto& Row)
						{
							return Row.Row.strId == InputId;
						});
					if (It == Program.MaterialInputs.end() ||
						It->strRecipeId != Recipe->Row.strId)
					{
						return nullptr;
					}
					if (It->strNormalizedParameterName == Name)
					{
						if (nullptr != Found)
							return nullptr;
						Found = &*It;
					}
				}
				return Found;
			};
			const auto ReadScalar = [&](const std::string_view Name,
				f32_t& OutValue)
			{
				const auto* Input = FindRecipeInput(Name);
				return nullptr != Input &&
					Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
					Input->fValue.has_value() && AssignFloat(*Input->fValue, OutValue);
			};
			const auto* DiffuseColor = FindRecipeInput("diffuse_color");
			if (!ReadScalar(EXPECTED_SCALAR_NAMES[0u],
					Resource.RuntimeMaterialV2ScalarBlocks[0u].x) ||
				!ReadScalar(EXPECTED_SCALAR_NAMES[1u],
					Resource.RuntimeMaterialV2ScalarBlocks[0u].y) ||
				!ReadScalar(EXPECTED_SCALAR_NAMES[2u],
					Resource.RuntimeMaterialV2ScalarBlocks[0u].z) ||
				!ReadScalar(EXPECTED_SCALAR_NAMES[3u],
					Resource.RuntimeMaterialV2ScalarBlocks[0u].w) ||
				!ReadScalar(EXPECTED_SCALAR_NAMES[4u],
					Resource.RuntimeMaterialV2ScalarBlocks[1u].x) ||
				!ReadScalar(EXPECTED_SCALAR_NAMES[5u],
					Resource.RuntimeMaterialV2ScalarBlocks[1u].y) ||
				nullptr == DiffuseColor ||
				DiffuseColor->eVariant !=
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
				!AssignFloat4(DiffuseColor->vValue,
					Resource.RuntimeMaterialV2Vectors[0u]))
			{
				strOutError =
					"Artist F active004 runtime-material constants changed.";
				return false;
			}

			const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* OpacityClip = nullptr;
			for (const std::string& BindingId : Recipe->RenderBindingIds)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == BindingId;
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId)
				{
					strOutError =
						"Artist F active004 render binding changed.";
					return false;
				}
				if (It->strFieldName == "opacitymaskclipvalue")
				{
					if (nullptr != OpacityClip)
						return false;
					OpacityClip = &*It;
				}
			}
			if (nullptr == OpacityClip ||
				OpacityClip->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
				!OpacityClip->fValue.has_value() ||
				!AssignFloat(*OpacityClip->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[1u].z) ||
				Resource.RuntimeMaterialV2ScalarBlocks[1u].z !=
					static_cast<f32_t>(0.3330000042915344))
			{
				strOutError =
					"Artist F active004 opacity-mask clip changed.";
				return false;
			}

			RuntimeTextureProviders.reserve(EXPECTED_TEXTURE_NAMES.size());
			for (const std::string_view Name : EXPECTED_TEXTURE_NAMES)
			{
				const auto* Input = FindRecipeInput(Name);
				const auto BindingIt = nullptr == Input ?
					Program.MaterialTextureBindings.end() :
					std::find_if(Program.MaterialTextureBindings.begin(),
						Program.MaterialTextureBindings.end(), [&](const auto& Row)
						{
							return Row.strMaterialInputFieldId == Input->Row.strId;
						});
				const auto PolicyIt = BindingIt == Program.MaterialTextureBindings.end() ?
					Program.MaterialPolicies.end() :
					std::find_if(Program.MaterialPolicies.begin(),
						Program.MaterialPolicies.end(), [&](const auto& Row)
						{
							return Row.Row.strId == BindingIt->strSamplerPolicyRowId;
						});
				if (nullptr == Input ||
					Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ||
					BindingIt == Program.MaterialTextureBindings.end() ||
					PolicyIt == Program.MaterialPolicies.end() ||
					!BindingIt->strRuntimeAssetId.has_value())
				{
					strOutError =
						"Artist F active004 texture binding changed.";
					return false;
				}
				EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER Provider;
				Provider.strProviderKind = "MATERIAL_TEXTURE_BINDING";
				Provider.strMaterialInputFieldId = Input->Row.strId;
				Provider.strMaterialInputRowSha256 = Input->Row.strRowSha256;
				Provider.strTextureBindingId = BindingIt->Row.strId;
				Provider.strTextureBindingRowSha256 = BindingIt->Row.strRowSha256;
				Provider.strSamplerPolicyRowId = PolicyIt->Row.strId;
				Provider.strSamplerPolicyRowSha256 = PolicyIt->Row.strRowSha256;
				Provider.strRuntimeAssetId = *BindingIt->strRuntimeAssetId;
				Provider.strSelectionBasis = "ACTIVE004_EXACT_RECIPE_INPUT_ORDER";
				RuntimeTextureProviders.push_back(std::move(Provider));
			}
		}
		else if (bActive024RuntimeV2)
		{
			/* PS 40282bcce2c90340afa03f7ffbade224 / DXBC
			   6b67e8a51c7afbca7e106ac7ed034b9afc81deae5e758cb63b7d02c7fc27f2c3.
			   This replays the recovered RT0 equation through the existing particle
			   ABI. Native VF/pass admission, selection/fog carriers and auxiliary
			   MRT2/3 outputs remain deliberately outside this bounded program. */
			if (Emitter.Row.strRowSha256 !=
					"5b437795eb0654fb51188e620cc6e06771d394da29760aed71619bb918f6ea8d" ||
				Emitter.strSelectedLodPath !=
					"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_16.particlelodlevel_22" ||
				Emitter.strSelectedLodNodeId != "FX_PC_SDM_07:export:263" ||
				Emitter.strSelectedLodRecordSha256 !=
					"4d30d6fe0b2ba98629dabacbf56c84b0ae794c36202195c237123defcb521f65" ||
				Occurrence->Row.strId != "source-active-024" ||
				Occurrence->Row.strRowSha256 !=
					"71e7105b6f92874f90e07c45972e34bd9cf69df3953e6cadfca33573ce129942" ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					"0a42bbc4ddaefb52de02dcbd9bf95adc4f0c6674016c298af63a6da3c323fd0a" ||
				Occurrence->strBindingSha256 !=
					"b192f539c94a9f09d48f72bf778fdbcf8c984385373a66296bf5545fe3ce9381" ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					"9f713d175bc82f8a786362d2193a269088066d3517ec90ba2f9b068f32b0a218" ||
				Recipe->Row.strId != "material-recipe-6ac2c48a0ff3bfa0" ||
				Recipe->Row.strRowSha256 !=
					"29647d4d82343a3a941b2cf5cebbb750073db77beda0e01fb26d78bae0fabd00" ||
				Recipe->strSourceMaterialPath !=
					"fx_m_mi_00.fx_mi.fx_d_pa_atta_02_01_ts_tr" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"84401f4884ccdbdde6093cb0b52a73e6f0f0d6927fa1ea8267b68f8494fa0e1e" ||
				Recipe->strBindingSha256 !=
					"b192f539c94a9f09d48f72bf778fdbcf8c984385373a66296bf5545fe3ce9381" ||
				Family->Row.strId != "material-family-600687e6c2277444" ||
				Family->Row.strRowSha256 !=
					"0331052694afe9885d273065f6c0a110f6216e5b852f114b0e680754098ad579" ||
				Family->strFamilyIdentitySha256 !=
					"f3fc5e78fd72621b74263edabfbf7dbc304b81281e48f97e86f948eb6027c93a" ||
				Family->strEvaluatorId != "reconstructed-evaluator-d963d018a13e8500" ||
				Family->strEvaluatorSha256 !=
					"30b65e50ea2456968cb33b6b5d764c36468d99d803c2a59d378288d60929967a" ||
				Family->iFeatureMask != 1023u || Recipe->InputIds.size() != 29u ||
				Recipe->StaticBindingIds.size() != 1u ||
				Recipe->RenderBindingIds.size() != 6u ||
				Element.Material.strSourceMaterialPath != Recipe->strSourceMaterialPath ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F active024 bounded shader identity changed.";
				return false;
			}

			struct ACTIVE024_INPUT final
			{
				size_t iRecipeIndex;
				std::string_view strId;
				std::string_view strName;
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
				double fValue;
				std::string_view strTexture;
				std::string_view strRowSha256;
			};
			static constexpr std::array<ACTIVE024_INPUT, 11u> LIVE_INPUTS = {{
				{ 0u, "material-input-5733883d3e2a44d3", "dynamic_parameter_explanation", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 5., {}, "da03ce5eb440e3ba1aebcc42c86ca531fce0cece6f4d463ed920f8b132cb234b" },
				{ 1u, "material-input-168968e3c81300df", "uv_noise_01_intensity", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {}, "a7fc67c55080aa94ca058029f953c1cf7400dc00d8c434b235e65428ca6d82da" },
				{ 2u, "material-input-22519d4affb4810c", "uv_noise_01_panning_x", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0., {}, "1cf181ad5f087c5cb74ae77f23cc887ad7a584bc09b68a625132b2dd239c55e3" },
				{ 3u, "material-input-3ea46d0023553983", "uv_noise_01_panning_y", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.20000000298023224, {}, "3eb391153f1d5e148f90f2f2551f7ed94839d609d123ac9d4e172a7654838f84" },
				{ 4u, "material-input-f3dc41471290e448", "uv_noise_01_tiling_x", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 3., {}, "1ca117717497b33f1c68e775e89a32e7e879b2736b551f4982d04c3bae264db2" },
				{ 5u, "material-input-0d73a91ea61f3df4", "uv_noise_01_tiling_y", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, "e558e81782706e99af29830b2e08dc1b135ea731f000a63ecc59137e8ce97a3a" },
				{ 6u, "material-input-adf8710a3f8cad4a", "alpha_tex", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_hit_006_cl", "2c1059a8c96db0e5c4e99196974a37d574643473a5da48da4167aff32a150ca2" },
				{ 7u, "material-input-787a89b9e8277bec", "emissive_tex", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_noise_014", "4900580c8de10f87cf71be052ee92e70e1ad5876d01686a881b4b2047bcb93c5" },
				{ 8u, "material-input-5d670256e1374c43", "uv_noise_01_tex", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., "fx_tex_02.fx_d_noise_014", "4c72500563b497454ea38cbb53c6bc428f1072f084b94a75f624c0d29a18d165" },
				{ 10u, "material-input-03ae15bad0a7e967", "emissive_power", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, "c2c7f888818f7786eadfe15e4c5bf04f8124c5f81affd1a0fe4842cee82d6a17" },
				{ 11u, "material-input-92116a2ea366e79a", "uv_scale", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, "564a40bc3ce4384df2c9cfbc4c1330f536a5aed3d8ec61861964e811f8a4bb27" }
			}};
			for (size_t i = 0u; i < LIVE_INPUTS.size(); ++i)
			{
				const ACTIVE024_INPUT& Expected = LIVE_INPUTS[i];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Expected.strId;
					});
				const bool_t bValueMatches = It != Program.MaterialInputs.end() &&
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ?
						It->strStringValue == Expected.strTexture :
						It->fValue.has_value() && *It->fValue == Expected.fValue);
				if (Recipe->InputIds[Expected.iRecipeIndex] != Expected.strId ||
					It == Program.MaterialInputs.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != Expected.strName ||
					It->eVariant != Expected.eVariant ||
					It->Row.strRowSha256 != Expected.strRowSha256 || !bValueMatches)
				{
					strOutError = "Artist F active024 live material input changed.";
					return false;
				}
			}

			const auto StaticIt = std::find_if(Program.MaterialStaticBindings.begin(),
				Program.MaterialStaticBindings.end(), [](const auto& Row)
				{
					return Row.Row.strId == "material-input-be0067a7ca36347a";
				});
			if (Recipe->StaticBindingIds[0u] != "material-input-be0067a7ca36347a" ||
				StaticIt == Program.MaterialStaticBindings.end() ||
				StaticIt->strNormalizedParameterName != "use_axisy" ||
				StaticIt->Row.strRowSha256 !=
					"5579cf5db32cfc1bc6696a285e57061e3bdd7698a75d515a0fe2a957e530afe9" ||
				!StaticIt->bSourceValue.value_or(false) ||
				!StaticIt->bSelectedValue.value_or(false))
				return false;

			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion" }};
			static constexpr std::array<std::string_view, 6u> RENDER_ROWS = {{
				"3df88518762f4ceb6ecf46cb52430506ab7c1f9ecf3a03545a37f36ad34ecf01",
				"5acf103ddfe84702bf399b30638a32eb4abea84b5c14898d6ed9a673263daa23",
				"1fe9baf9d16d7f85f5a75836ee3da1d8edabde77c856ed4b68509e6a3876d2a5",
				"fc6782d37af4874d629b9cddd8f6824b75543075d5185f8171523baf1c999589",
				"f76a224d6d0b3591e41d34c13329b24f1c2535eca05550e6499b6718cccd9d47",
				"c59111c9da893e7b13834949fafba642a46ecd9bf89fd836b01137eb444fd194" }};
			for (size_t i = 0u; i < RENDER_ROWS.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[i];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_FIELDS[i] ||
					It->Row.strRowSha256 != RENDER_ROWS[i])
					return false;
			}

			const EFFECT_RUNTIME_PROGRAM_MODULE* Dynamic = nullptr;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					if (nullptr != Dynamic)
						return false;
					Dynamic = &*It;
				}
			}
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"emissive_tiling(0.5~2)", "alpha_power(1~)",
				"panning_area(-1~1)", "fresnel_alpha(1~)" }};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
				"bfa41405a03eadfde61a8707f8ad1be78ce5a8edf841dbe5885a35123514b1a2",
				"eda300f125b6e54e2075480d7224b6fb984ddacab66320ea229a1a503940851f",
				"e0b593043ebc927d9f034ba487f1de0cea242bdc9ae7e1e439c878c3bd9207be",
				"95835b3bed1a0d0669522363b3592a4cf2abdc6696196aa903b47d7e619e8bf0" }};
			static const std::array<std::vector<double>, 4u> DYNAMIC_LOOKUPS = {{
				{ 1., 1., 1., 1. }, { 1., 2., 1., 2. },
				{ 1., 1., 1., 1. }, { 2.5, 4., 4., 2.5 } }};
			static constexpr std::array<double, 4u> DYNAMIC_TIME_SCALES = {{
				0., 1., 0., 1. }};
			bool_t bDynamicValid = nullptr != Dynamic && Dynamic->Row.iOrder == 4u &&
				Dynamic->Row.strRowSha256 ==
					"92b7fe472dae34cac57bc0e13feab767e2e00c785219ee8e052bc43014886b51" &&
				Dynamic->strSourceRecordSha256 ==
					"cc9a7bea309d81ae73fa303e93596ab0b8382178238b920f5620bd8d885d0fa9" &&
				Dynamic->DistributionIds.size() == DYNAMIC_ROWS.size();
			for (size_t i = 0u; bDynamicValid && i < DYNAMIC_ROWS.size(); ++i)
			{
				const std::string NamePath = "dynamicparams[" + std::to_string(i) +
					"].paramname";
				const std::string ValuePath = "dynamicparams[" + std::to_string(i) +
					"].paramvalue";
				const auto Name = std::find_if(Program.Literals.begin(), Program.Literals.end(),
					[&](const auto& Row)
					{
						return Row.strModuleId == Dynamic->Row.strId &&
							Row.strPropertyPath == NamePath;
					});
				const auto Distribution = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Dynamic->DistributionIds[i];
					});
				bDynamicValid = Name != Program.Literals.end() &&
					Name->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					Name->strEnumValue == DYNAMIC_NAMES[i] &&
					Distribution != Program.Distributions.end() &&
					Distribution->strModuleId == Dynamic->Row.strId &&
					Distribution->strPropertyPath == ValuePath &&
					Distribution->Row.iOrder == i &&
					Distribution->Row.strRowSha256 == DYNAMIC_ROWS[i] &&
					Distribution->eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
					Distribution->iComponentCount == 1u &&
					Distribution->iOperation.value_or(UINT32_MAX) == 1u &&
					Distribution->iLookupTableChunkSize == 1u &&
					Distribution->iLookupTableNumElements == 1u &&
					Distribution->fLookupTableTimeScale == DYNAMIC_TIME_SCALES[i] &&
					Distribution->fLookupTableStartTime == 0. &&
					Distribution->LookupTable == DYNAMIC_LOOKUPS[i];
			}
			if (!bDynamicValid)
			{
				strOutError = "Artist F active024 dynamic contract changed.";
				return false;
			}

			Resource.RuntimeMaterialV2ScalarBlocks[0u] = { 0.f, 0.2f, 3.f, 1.f };
			Resource.RuntimeMaterialV2ScalarBlocks[1u] = { 0.1f, 1.f, 5.f, 1.f };
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x03u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0cu;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 8u;
			Resource.iRuntimeMaterialV2VectorCount = 0u;
			Resource.iRuntimeMaterialV2InputCount = 29u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x00000dffu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x1ffff200u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 1u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x01u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x01u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x07u;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x38u;

			struct TEXTURE_SLOT final
			{
				size_t iInput;
				std::string_view strBindingId;
				std::string_view strBindingRow;
				std::string_view strPolicyId;
				std::string_view strPolicyRow;
				std::string_view strAssetId;
				std::string_view strSelection;
			};
			static constexpr std::array<TEXTURE_SLOT, 3u> TEXTURE_SLOTS = {{
				{ 8u, "material-input-5d670256e1374c43::runtime-texture-binding", "30d7dc4c28b3449b18eb1813baa3098129900c3df7eab0ed537d23dce91cf3b0", "material-reconstructed-policy-00b446a77e860816dc99", "71b5b76fe0cf00c70006452de3b0bccf1347e2e3e850d03b8773565a514d4a60", "Effect/Artist/Textures/fx_d_noise_014.dds", "RECOVERED_DXBC_T0_NOISE_RG" },
				{ 6u, "material-input-adf8710a3f8cad4a::runtime-texture-binding", "aac18acf46e3189ccc6be10e068261530a1514f40205ba559165d9513c912426", "material-reconstructed-policy-6d082a8fb7411643255d", "086a5c4f7a94c851de7c3d9e7b20e6db7091dc93835fc89ef376b6b856c655cf", "Effect/Artist/Textures/fx_d_hit_006_cl.dds", "RECOVERED_DXBC_T1_ALPHA_R" },
				{ 7u, "material-input-787a89b9e8277bec::runtime-texture-binding", "cc73e135a3a21560c0f77ad98e5dec2d87c2baccbc1291232f2395fd98506c6a", "material-reconstructed-policy-6ae5a4badcbb8d714068", "0635ea66f6c31696efe4da6d7dca80da8db85526e529876c00e6b8ed1b639fd7", "Effect/Artist/Textures/fx_d_noise_014.dds", "RECOVERED_DXBC_T2_EMISSIVE_RGB" }
			}};
			for (const TEXTURE_SLOT& Slot : TEXTURE_SLOTS)
			{
				const ACTIVE024_INPUT& Expected = LIVE_INPUTS[Slot.iInput];
				RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
					std::string(Expected.strId), std::string(Expected.strRowSha256),
					std::string(Slot.strBindingId), std::string(Slot.strBindingRow),
					std::string(Slot.strPolicyId), std::string(Slot.strPolicyRow),
					std::string(Slot.strAssetId), std::string(Slot.strSelection) });
			}
		}
		else if (bActive027RuntimeV2)
		{
			/* Exact recovered PS/static-set evidence with an unresolved native VF
			   choice. Replay only the finite RT0 equation; t6/t7 normal/spec,
			   selection/fog, auxiliary MRTs and external opacity stay suppressed. */
			if (Emitter.Row.strRowSha256 !=
					"e4957941115ed44d62f51de033f084b95c670ae8728c2bd699955d7f806bc3c8" ||
				Emitter.strSelectedLodPath !=
					"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_0.particlelodlevel_0" ||
				Emitter.strSelectedLodNodeId != "FX_PC_SDM_07:export:256" ||
				Emitter.strSelectedLodRecordSha256 !=
					"27f90b21a3aa23284e5f6739772c9cb087a7800c06cf6564883f2fe670751fe9" ||
				Occurrence->Row.strId != "source-active-027" ||
				Occurrence->Row.strRowSha256 !=
					"6eac180a4d907b9bd4510161d4e200f2c9bc81280ca618f086321fcaa461fe92" ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					"2ce71c7dfa66cd14a8296ba80152d2616bade8eb695e030d66409721eb483041" ||
				Occurrence->strBindingSha256 !=
					"938c3311cf5523b8fb16bf746e2542687bbf9eb51c9fbcc7af9e0265d86084b8" ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					"006ac38756e78e518359efe989903313c98caf7ad9bf927e47328a598c5e5ae7" ||
				Recipe->Row.strId != "material-recipe-2073fb45e643d1d5" ||
				Recipe->Row.strRowSha256 !=
					"f210da08033a522e3ab3e581a5535df5629a15411a6c46f5052b6c73d03202a1" ||
				Recipe->strSourceMaterialPath !=
					"fx_m_mi_05.fx_mi.fx_e_pa_fd_18_2_tr" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"ee53a30e9e2008365330761da31d10e6b49e1cbb29d3997fda355c415f072a1e" ||
				Recipe->strBindingSha256 != Occurrence->strBindingSha256 ||
				Family->Row.strId != "material-family-ee42f716afdf6145" ||
				Family->Row.strRowSha256 !=
					"78577bca3d6ff10f53428196606c79548b3b1d52cc0594e469f701a5ced8c568" ||
				Family->strFamilyIdentitySha256 !=
					"110d51c22adf7f38af65be9084f4ce8f7400ad922df9ca6dd73798fc7ddd9c9c" ||
				Family->strEvaluatorRegistryId !=
					"handler-4e2ab622309f436238122290" ||
				Family->strEvaluatorId != "reconstructed-evaluator-b64318cb50070e35" ||
				Family->strEvaluatorSha256 !=
					"15b89e56b711bc5a9627b03db91230b5d0000b3852a38ece158c8a68f4a34bf7" ||
				Family->strSampleProjectionSha256 !=
					"293c3f95a74d1ca903d4076536e07b6ba2e0c97547d5a7b84fcee0f4507cadc2" ||
				Family->iFeatureMask != 811u || Recipe->InputIds.size() != 14u ||
				Recipe->StaticBindingIds.size() != 3u ||
				Recipe->RenderBindingIds.size() != 6u ||
				ShaderRegistry->strEngineEqualityStaticSetSha256 !=
					"49cc0c15999c691875f7198b309c3f1f74ef57b0bbfd8f063d74d9c9648eda05" ||
				ShaderRegistry->strRecoveredPixelShaderId !=
					"67ed51e00d8a4247a540ccf8a05a6c8c" ||
				ShaderRegistry->strRecoveredPixelDxbcSha256 !=
					"7253dacbe9d1bf3b9410b0beeb341b8a4dfc47e8726b267c23c2eba8366189d3" ||
				ShaderRegistry->bNativeSelectionAdmitted ||
				Element.Material.strSourceMaterialPath != Recipe->strSourceMaterialPath ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F active027 bounded shader identity changed.";
				return false;
			}

			struct INPUT_CONTRACT final
			{
				std::string_view strId;
				std::string_view strName;
				std::string_view strRow;
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
				double fScalar;
				std::array<double, 4u> vVector;
				std::string_view strTexture;
			};
			static constexpr std::array<INPUT_CONTRACT, 14u> INPUTS = {{
				{ "material-input-f6e8ee4eda6c3c46", "specular_intensity", "2b60d20086689a71b140e09172c13c107bb97b8c45b504091b0bade66b368c95", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.20000000298023224, {}, {} },
				{ "material-input-919af5b5235fd7bb", "specular_power", "91af0daaf4a6636166055f8698985b2704abc715103908b1a6db4a51b63e332a", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 40., {}, {} },
				{ "material-input-ece8cca2188340b6", "alpha_tex_02_uvpos", "dc298f105274d9d973159eef1a838e1a45a86f795d5724977d9b4e27491d6a1b", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { 0., 0., 0., 1. }, {} },
				{ "material-input-72b9eeafb6f8eba5", "alpha_tex_01", "c2b9f2c9b6906ae325b67affe7607836576f5b8e48b8fa0bf5c9a164ae429815", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_03.fx_e_fluid_021" },
				{ "material-input-fb27adb50913c489", "emissive_tex", "b1383c558093e1e8b74f014b394d0a7b4371ec1e020eb7b8ada58151a2afa68b", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_00.fx_a_decal_013" },
				{ "material-input-e3c9e51c794ab716", "normal_tex", "2b407393e1dc90965cb5c3b677b34da6813205f2ccf988fca17e6bd106b4f491", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_03.fx_e_fluid_021_n" },
				{ "material-input-8955b5c15e5964b5", "noise_intensity", "2f70ccdbdb12b574f5d64f5b760b99453ed60401362c4aa7f84149331c585e3f", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.5, {}, {} },
				{ "material-input-e5118feaac6dff6e", "alpha_tex_02_mask", "95d9a9dfb431319b3da48e497263975d7afd206ff1296355220154f8a449312c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_03.fx_e_fluid_003" },
				{ "material-input-01e5e7ffe77b6d59", "alpha_tex_02", "e19e7d88a7254d0c49f01f2b908dbe3dea3199c618706b20e5cb5c41c20eaca0", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_03.fx_e_electric_002_cl" },
				{ "material-input-80d63639651d7b01", "alpha_tex_01_intensity", "1fbbf18d8c12f9b1c30dcd7451a51617aba06c1bfd1c0b9db007276e5a5af41b", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 5., {}, {} },
				{ "material-input-b4de1c39f04dda6a", "emissive_color&intensity", "36bdd4297a33f799774e55a0b031ea1a730482deb26dd026ba74e279e40db222", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { 15., 0.10000000149011612, 0., 1. }, {} },
				{ "material-input-f7152683c81ec450", "emissive_inner_color&intensity", "c03acb858ea1a281d7c931e337f65783d9aa2e9da118319c5bbae8eddd30d9db", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { 4., 0.5, 0.10000000149011612, 0.8500000238418579 }, {} },
				{ "material-input-5015ce54b2474df5", "emissive_color_total_intensity", "672d0e39980396a4bf303eeb2def6303c9cb0cb983ac502857cba47f35ee2227", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.10000000149011612, {}, {} },
				{ "material-input-034dae0788af0afa", "emissive_blackarea_intensity", "c959d0109727c605637a452b404268cb9103797df5233e5ddbb73460f599c2db", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0.009999999776482582, {}, {} }
			}};
			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 14u> InputRows{};
			for (size_t i = 0u; i < INPUTS.size(); ++i)
			{
				const INPUT_CONTRACT& Expected = INPUTS[i];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{ return Row.Row.strId == Expected.strId; });
				if (Recipe->InputIds[i] != Expected.strId ||
					It == Program.MaterialInputs.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != Expected.strName ||
					It->Row.strRowSha256 != Expected.strRow ||
					It->eVariant != Expected.eVariant ||
					It->strTypedValueSha256.empty() ||
					It->strSourceFieldValueSha256.empty() ||
					It->strSourceLineageSha256.empty() ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
						(!It->fValue.has_value() || *It->fValue != Expected.fScalar)) ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 &&
						It->vValue != Expected.vVector) ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID &&
						It->strStringValue != Expected.strTexture))
				{
					strOutError = "Artist F active027 material input changed.";
					return false;
				}
				InputRows[i] = &*It;
			}

			struct STATIC_CONTRACT final
			{
				std::string_view strId;
				bool_t bSelected;
				std::string_view strRow;
				std::string_view strPolicy;
				std::string_view strLineage;
			};
			static constexpr std::array<STATIC_CONTRACT, 3u> STATICS = {{
				{ "material-input-a3c10147a28c0876", false, "3141006b14aabedaecd15d229d929edca3182b3842c369a2ee52dbf9567dd75d", "material-reconstructed-policy-41fbfa7ccf93584b2762", "2c766cfdb6b9e1140bed1177b90ec4452de0078ac330e417d0e5c99feba1d042" },
				{ "material-input-f2f81ea1773b62f3", true, "d28707908ab019b35ef6947a5a6f8696756a8eb1d3ef75c596424948697c3ad7", "material-reconstructed-policy-83561d63e11032067af2", "4241cbad78bc730114ae510265dc19829aecbbff8042cb0fb3eefcbfa9288162" },
				{ "material-input-7d82d4624fed00f2", true, "78692106980b5de756ba76b7f639378c33bfd1e8ae5818ff63c9bb22367b30f1", "material-reconstructed-policy-360610823c30b0d0ef5a", "44464c56225958dab133017febc86848eca4ed7e97b1ba18c9bae1c60a5b2538" }
			}};
			/* Candidate rows do not expose expression GUIDs as runtime fields. The
			   exact GUID triplet (d8b709895d7d7246b2a035b2af9357b6,
			   e00140386ecd7d48b3e0c507eea1ce7d,
			   53bc7a5a99c01a4a9f408e95954537ca) is therefore admitted only through
			   the registry's engine-equality static-set SHA; this loop independently
			   pins the available field/row/policy/lineage/selection projection. */
			uint32_t iSelectedStaticMask = 0u;
			for (size_t i = 0u; i < STATICS.size(); ++i)
			{
				const STATIC_CONTRACT& Expected = STATICS[i];
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{ return Row.Row.strId == Expected.strId; });
				if (Recipe->StaticBindingIds[i] != Expected.strId ||
					It == Program.MaterialStaticBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != "use_meshtype" ||
					It->Row.strRowSha256 != Expected.strRow ||
					It->strPolicyRowId != Expected.strPolicy ||
					It->strSourceLineageSha256 != Expected.strLineage ||
					!It->bSourceValue.has_value() || !*It->bSourceValue ||
					!It->bSelectedValue.has_value() ||
					*It->bSelectedValue != Expected.bSelected)
				{
					strOutError = "Artist F active027 static-set row changed.";
					return false;
				}
				if (Expected.bSelected)
					iSelectedStaticMask |= 1u << static_cast<uint32_t>(i);
			}
			if (iSelectedStaticMask != 0x06u)
			{
				strOutError = "Artist F active027 selected static mask changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion" }};
			static constexpr std::array<std::string_view, 6u> RENDER_ROWS = {{
				"547922292a185d873c7622ef48e539bfd3d7a90b764a21555a86f6e68dd1bd0c",
				"18c1159e64d87138cd4b1b075a3f8f269e01c66ef7a908b74652b314ae801eb0",
				"73d582c643b6cfbe899bc649cb391ac6f2aaa6e4ff4862401d10e9368d854d01",
				"8649773878c4cae3c4701881a3c98046d65e652e790b8b3e61ddd41e21357381",
				"98ba4f42edc30df0e3166d3e879162244fc47e363015333bdc4808bb0f7da464",
				"f33a944d79161ebc169fac1c52cf8eaebf55cf5cdade5cb718203d171278caf3" }};
			for (size_t i = 0u; i < RENDER_FIELDS.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{ return Row.Row.strId == Recipe->RenderBindingIds[i]; });
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_FIELDS[i] ||
					It->Row.strRowSha256 != RENDER_ROWS[i])
				{
					strOutError = "Artist F active027 render-state row changed at index " +
						std::to_string(i) + ": id=" + Recipe->RenderBindingIds[i] +
						", field=" + (It == Program.MaterialRenderBindings.end() ?
							std::string{"<missing>"} : It->strFieldName) +
						", row=" + (It == Program.MaterialRenderBindings.end() ?
							std::string{"<missing>"} : It->Row.strRowSha256) + ".";
					return false;
				}
			}

			const EFFECT_RUNTIME_PROGRAM_MODULE* Dynamic = nullptr;
			uint32_t iDynamicCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
				{
					strOutError = "Artist F active027 module join changed.";
					return false;
				}
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					Dynamic = &*It;
					++iDynamicCount;
				}
			}
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"dissolve", "param2", "param3", "param4" }};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAME_ROWS = {{
				"010263638a3087ff0dbfd3544e95a9b90c3d0a285f2134b2332757eb6558fb92",
				"7cf2b4ce0c8c5a842d5d7395517127a97075a189d92fa23273e8c9edc70cb95b",
				"fc469c9914d13e5867d597a4c040e43cde871ae8690b909b021c3c391042ccb8",
				"4550e5382b741205717f51f8cd0bf9f639dc94e7f7a33072083eded7a6f90491" }};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
				"8e78c999078a72ad05aa230c164acb0d234d7b556360592870affd67bbe28c4d",
				"cb2e888296dd619a3bbb049e7dd82aa5525be520f3843f67e726383d5a97e19f",
				"2f5070396a3ab12fe634067efea9d83902490b35c843b64351eb49e9453dcb8f",
				"c63ef16fa5bcba2a6bf81c2cb9d769e73c6eae74f58d6e2c4fbe36dc6fb1471d" }};
			static constexpr std::array<double, 4u> DYNAMIC_TIME_SCALE = {{
				1.25, 1., 0., 0. }};
			static constexpr std::array<double, 4u> DYNAMIC_START = {{
				0.20000000298023224, 0., 0., 0. }};
			static constexpr std::array<std::array<double, 4u>, 4u> DYNAMIC_TABLES = {{
				{{ 0., 1., 1., 0. }}, {{ 0.5, 3., 0.5, 3. }},
				{{ 1., 1., 1., 1. }}, {{ 1., 1., 1., 1. }} }};
			bool_t bDynamicValid = iDynamicCount == 1u && nullptr != Dynamic &&
				Dynamic->Row.iOrder == 6u && Dynamic->Row.strRowSha256 ==
					"74f8f9b2b8d72b02847e6e2d41b8e77ddcf54d259f6c61dfff709f7fd7edd25e" &&
				Dynamic->strSourceRecordSha256 ==
					"1040d6f6874eb8bbf858483d3dec012eb8e196adb8feec4d93d8a7b8c3dc57d4" &&
				Dynamic->DistributionIds.size() == 4u;
			for (size_t i = 0u; bDynamicValid && i < 4u; ++i)
			{
				const std::string NamePath = "dynamicparams[" + std::to_string(i) +
					"].paramname";
				const std::string ValuePath = "dynamicparams[" + std::to_string(i) +
					"].paramvalue";
				const auto Name = std::find_if(Program.Literals.begin(), Program.Literals.end(),
					[&](const auto& Row) { return Row.strModuleId == Dynamic->Row.strId &&
						Row.strPropertyPath == NamePath; });
				const auto Dist = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{ return Row.Row.strId == Dynamic->DistributionIds[i]; });
				bDynamicValid = Name != Program.Literals.end() &&
					Name->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					Name->strEnumValue == DYNAMIC_NAMES[i] &&
					Name->Row.strRowSha256 == DYNAMIC_NAME_ROWS[i] &&
					Dist != Program.Distributions.end() &&
					Dist->strModuleId == Dynamic->Row.strId && Dist->Row.iOrder == i &&
					Dist->strPropertyPath == ValuePath &&
					Dist->Row.strRowSha256 == DYNAMIC_ROWS[i] &&
					Dist->eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
					Dist->iComponentCount == 1u &&
					Dist->iOperation.value_or(UINT32_MAX) == 1u &&
					Dist->iLookupTableChunkSize.value_or(0u) == 1u &&
					Dist->iLookupTableNumElements.value_or(0u) == 1u &&
					Dist->fLookupTableTimeScale.value_or(-1.) == DYNAMIC_TIME_SCALE[i] &&
					Dist->fLookupTableStartTime.value_or(-1.) == DYNAMIC_START[i] &&
					Dist->LookupTable == std::vector<double>(
						DYNAMIC_TABLES[i].begin(), DYNAMIC_TABLES[i].end());
			}
			if (!bDynamicValid)
			{
				strOutError = "Artist F active027 age-aligned dynamic.xyzw changed.";
				return false;
			}

			Resource.RuntimeMaterialV2ScalarBlocks[0u] = {
				0.5f, 5.f, 0.10000000149011612f, 0.009999999776482582f };
			if (!AssignFloat4(InputRows[2u]->vValue, Resource.RuntimeMaterialV2Vectors[0u]) ||
				!AssignFloat4(InputRows[10u]->vValue, Resource.RuntimeMaterialV2Vectors[1u]) ||
				!AssignFloat4(InputRows[11u]->vValue, Resource.RuntimeMaterialV2Vectors[2u]))
			{
				strOutError = "Artist F active027 bounded vector packing changed.";
				return false;
			}
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x01u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0eu;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 4u;
			Resource.iRuntimeMaterialV2VectorCount = 3u;
			Resource.iRuntimeMaterialV2InputCount = 14u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x3fdcu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x0023u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask = { 0x03u, 0x0fu, 0x0fu };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask = { 0x0cu, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 3u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x06u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x07u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x07u;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x38u;
		}
		else if (bActive028RuntimeV2)
		{
			/* Recovered base-PS equation, carried by the current particle ABI.
			   This is deliberately BOUNDED_EXPLICIT: native VF/pass selection is
			   not admitted, and selection/fog/aux-MRT/external-opacity carriers are
			   explicit omissions rather than guessed replacements. */
			if (Emitter.Row.strRowSha256 !=
					"de05e5a4a9b549190f8368fbee6b8f043ffb7db68bf67040be043d0c3b36b939" ||
				Emitter.strSelectedLodPath !=
					"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_10.particlelodlevel_0" ||
				Emitter.strSelectedLodNodeId != "FX_PC_SDM_07:export:258" ||
				Emitter.strSelectedLodRecordSha256 !=
					"e19be53c705529eda19d5d13559ba50ec4f4ca795302d9d1ade84049d2b175e7" ||
				Occurrence->Row.strId != "source-active-028" ||
				Occurrence->Row.strRowSha256 !=
					"620c66986c68893f1b8679adb8dc7590e632d353508f297875a6aa78578d6abb" ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					"1aa9266fa4c9bf56a495977cdf9a9d80e77b8528df298cefecc47769acdfdd87" ||
				Occurrence->strBindingSha256 !=
					"be1fbac51cbf129d1367d477ed75832c63184a649b2c8b89ac76cb90c391ef5e" ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					"dd50e244ff3972a28336e81db94bb08e64ed618667e000ce9f11bbcd922ba903" ||
				Recipe->Row.strId != "material-recipe-c9a33671bd6f57df" ||
				Recipe->Row.strRowSha256 !=
					"1730d7eaa739af650d36d8b919e85efd85c4ed9e44b4ad9bc2c044f7b90dfb40" ||
				Recipe->strSourceMaterialPath !=
					"fx_m_mi_k_00.fx_mi.fx_k_pa_flowmask_01_04_tr" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"e14079e3e243488b3a28bd041815a76ece979c5d6b85102e89b07752d3811e01" ||
				Recipe->strBindingSha256 != Occurrence->strBindingSha256 ||
				Family->Row.strId != "material-family-fcc81a924169d053" ||
				Family->Row.strRowSha256 !=
					"f1f755ddf40e9e464c4b1e89a235ab8614569bced6a0b558f9ed2693caf489ba" ||
				Family->strFamilyIdentitySha256 !=
					"1c828d78992f9038fb14fcb49f122a1aa53f06ec88bd4de4dd9a514249cb5158" ||
				Family->strEvaluatorRegistryId !=
					"handler-8b5b00e2122df0f229cbbde5" ||
				Family->strEvaluatorId != "reconstructed-evaluator-3d092db1c0886193" ||
				Family->strEvaluatorSha256 !=
					"7423b69e59c2df63c880f2b192d2b18ba4d3c33a410aa0ef69f96c3a7912f8bf" ||
				Family->strSampleProjectionSha256 !=
					"c793aa50fbe2f3607a11764351e092e3c18231d9c2bc4fc0590989c67a5bab19" ||
				Family->iFeatureMask != 815u || Recipe->InputIds.size() != 24u ||
				!Recipe->StaticBindingIds.empty() ||
				Recipe->RenderBindingIds.size() != 6u ||
				ShaderRegistry->strEngineEqualityStaticSetSha256 !=
					"6d9b9545ede5f25f758e0514f12aa140a7b86412fc532751fde380921e3518e5" ||
				!ShaderRegistry->strRecoveredPixelShaderId.empty() ||
				!ShaderRegistry->strRecoveredPixelDxbcSha256.empty() ||
				ShaderRegistry->bNativeSelectionAdmitted ||
				Element.Material.strSourceMaterialPath != Recipe->strSourceMaterialPath ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F active028 bounded shader identity changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 24u> INPUT_IDS = {{
				"material-input-165d0a9cb92bd1cf", "material-input-c94479ea753e90fe",
				"material-input-a73a3297af72016b", "material-input-d747761ccd0cc4f0",
				"material-input-cb9713ad93d79724", "material-input-5b7a1ad557a38243",
				"material-input-8775a6ba1343535d", "material-input-092a4f091213dd76",
				"material-input-b50b3f8103099beb", "material-input-6544f46ed7ba1e36",
				"material-input-b58bdeab951bd4ce", "material-input-ef68492277a56284",
				"material-input-acd370db8fbcf8e1", "material-input-7aed8cfe5ba9669b",
				"material-input-824d2830713d096e", "material-input-4569c3fd1757a400",
				"material-input-27540010adeccbdf", "material-input-3af4f370d01d7cee",
				"material-input-4abb1f5ae1e9135e", "material-input-244d796e05efb79c",
				"material-input-bbae2da0ef293fbf", "material-input-57c14a8e3e189456",
				"material-input-04bf2be0f133ea4f", "material-input-fddac7efa7f87ddf"
			}};
			static constexpr std::array<std::string_view, 24u> INPUT_NAMES = {{
				"diff1_tile_u", "diff1_tile_v", "diff2_tile_u", "diff2_tile_v",
				"diff_des", "diff_pow", "diff_str", "flow_tile_u", "flow_tile_v",
				"opacity_str", "opacity_tile_u", "opacity_tile_v", "diff_backcolor",
				"diff_tex1", "diff_tex2", "mask_tex", "opacity_tex", "distort_str",
				"flow_bias", "color_str", "color_pow", "color_tex", "opacity_pow",
				"opacity_distort_str"
			}};
			static constexpr std::array<std::string_view, 24u> INPUT_ROWS = {{
				"8c1bebc11ba93626110f0172d91cf380bc7dd85073a94df21fa90710375fdca0",
				"aeb5465c17411b7ebf6cc313ed145f8cff8685475e892f19e80f0466c158b22d",
				"42317882de13e43d64b034aabe03f10d3fd43909d139c657eadb0b99a4e3acba",
				"c8057f854072064bcf8e80b17760abfb7461feefb4f7e3c3ffe8a6419f1860c5",
				"3d030302d9fffd42e8b22ae71fa3cfb527ddaccfd28e1ae6490620a9513cd78f",
				"6503a9449303692bfb1fa013f8e10f0c4ae786ccda7d323e3ee0fff473a2c186",
				"4f565f8fb084fa50cffd4527649fc733d935f81d7bf9117b21c14e997328af87",
				"3860e52ef9ebea96aea21d331c58c75710d964981f646f314efcf72da32d102d",
				"1828507e557759cd58a536502418b395424e383ce4abe1afa8ce6b86f59d7acf",
				"3822ed6d0b0e5a1bc2d0c3a2b41f196fdf604e69855736f1aa86126902cfe771",
				"d0a0273a9e51b88ee09a71b45460ccfe702325824f7f432ea45b38e519ec374c",
				"9bb79e35f599e6e7ffb5cc3982eb865bc0af5014501116a3d15cfa16d5d375a6",
				"ae950723650268b6625f930551ffa4a20b39da036f68926286f29b7f0a5a1830",
				"d64f24d742c54358c4700fd823cd37bb53cc3ba0720822d8470f7700af2e32f2",
				"74a11be97cdacb814b099ce402d2548ee2cc02843a0a38b4645bac0b8b2325cf",
				"e0b8d5852d96627538489213137f1cdc7cfa7ee5e256f559f001fe46a71f600d",
				"62f5d1a9106bcfab9aa9624019b3736a2aa20c12f058c7f2f7cdd6893848220f",
				"8e18a94760794e27484029e3389f5c39c621163e093931833b2743a15c182882",
				"515a9c4fdcb8bc97a1a9924001bb26d671f3069cb66ec21d3be63bf2693928a3",
				"7bf459b4423c041483baa6cf239fadfe6539b27eceefdba46859d8996d05fa57",
				"1769e36668b10b6b8ca91fa392654775c54aaf7787be5669c86517fa4e0f88e0",
				"9bd39dc86fccf74c6aba36c5c05f23a4d1bd7c23b83341048c24ab0cab371ae9",
				"40b65ba5d65c49e31b7abaee768073f3a1fd7b0f42535f336bd8a6aacc7a5086",
				"0f4078fa7ba061f3b642550a3c0f1c79aacd5558a1758c9ad0c1f5e433f7ae56"
			}};
			static constexpr std::array<double, 24u> INPUT_SCALARS = {{
				1., 1., 1., 2., 0.800000011920929, 1.100000023841858,
				200., 1., 1., 20., 1., 1., 0., 0., 0., 0., 0., 0.5,
				0.20000000298023224, 2., 1., 0., 0.800000011920929, 1.
			}};
			static constexpr std::array<std::string_view, 5u> INPUT_TEXTURES = {{
				"fx_tex_05.fx_k_auraline_16", "fx_tex_05.fx_l_environment_001",
				"fx_tex_04.fx_j_smoke_01_cl", "fx_tex_05.fx_k_auraline_16",
				"fx_tex_05.fx_l_environment_001"
			}};
			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 24u> Inputs{};
			size_t iTextureValue = 0u;
			for (size_t i = 0u; i < INPUT_IDS.size(); ++i)
			{
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row) { return Row.Row.strId == INPUT_IDS[i]; }));
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == INPUT_IDS[i];
					});
				if (iMatches != 1u || It == Program.MaterialInputs.end() ||
					Recipe->InputIds[i] != INPUT_IDS[i] ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != INPUT_NAMES[i] ||
					It->Row.strRowSha256 != INPUT_ROWS[i] ||
					It->strTypedValueSha256.empty() ||
					It->strSourceFieldValueSha256.empty())
				{
					strOutError = "Artist F active028 material input identity changed.";
					return false;
				}
				Inputs[i] = &*It;
				if (i == 12u)
				{
					if (It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
						It->vValue != std::array<double, 4u>{ 0., 0., 0., 1. })
						return false;
				}
				else if ((i >= 13u && i <= 16u) || i == 21u)
				{
					if (It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ||
						It->strStringValue != INPUT_TEXTURES[iTextureValue++])
						return false;
				}
				else if (It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!It->fValue.has_value() || *It->fValue != INPUT_SCALARS[i])
				{
					return false;
				}
			}
			if (iTextureValue != INPUT_TEXTURES.size())
				return false;

			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion" }};
			static constexpr std::array<std::string_view, 6u> RENDER_ROWS = {{
				"9d7d85e05619b2e1868d75360456681de7150356369eb2b58aef8c414a8ee18b",
				"f4bc1e4b1eaa2d16dd439231e056cc7b3a94cc046689344f2624a3224a263c08",
				"44b44f3f1f3a6bad7fca62ec67a52d019195a813afcdfa32d2975b27e6741fb5",
				"c73a28d84f8d5a05ca143cc0cd5e182f6d318e6af9cbc31b198549f3ace1e850",
				"a67066c5352dc2999a10f56aa4b41150cadc268eef4c6e12d91daa4418d0f95d",
				"241d7e9f71ed834c23840b08438729766cfb12cd88baa45261accfd6e782fba6"
			}};
			for (size_t i = 0u; i < RENDER_FIELDS.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[i];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_FIELDS[i] ||
					It->Row.strRowSha256 != RENDER_ROWS[i])
				{
					strOutError = "Artist F active028 render-state input changed.";
					return false;
				}
			}

			const EFFECT_RUNTIME_PROGRAM_MODULE* Dynamic = nullptr;
			uint32_t iDynamicCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					Dynamic = &*It;
					++iDynamicCount;
				}
			}
			static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
				"d8c63ef2d5da3e8ed715acc6eb9c431bb8e44d7c4d67ef9f8fe22ebdc079cefb",
				"b615023c1fd21bf925c32f4bc483f2e5c41bc38b0783499cf173e76126c80c52",
				"64c74c02654d3c33c78d1be56757aa425f61524317cd5f6180bf73b040243138",
				"5c7db264a5961434b37c27a1316e5b938ee96a5f78254f1c4b299adf451fbbfe"
			}};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAME_ROWS = {{
				"f1037b0daff00bf6ebc2248f47812553a2dbf7f46db05d8ef2d30764ca32e2c3",
				"0e71a1f856d9239a1ff8ac92a43280f4d4f922dab7405ed696a3754def511adc",
				"d540f7d0f54d933492986bc31e2ef576ca91d0d5a8b05fb2d269db4c931f3da0",
				"4e1c5e25b98264e1ddece3febdcf3c0f20c5ecbdf487d4becf65385d50e2111a"
			}};
			bool_t bDynamicValid = iDynamicCount == 1u && nullptr != Dynamic &&
				Dynamic->Row.iOrder == 5u && Dynamic->Row.strRowSha256 ==
					"3264539fdd0f9acdfd312b1631aa54078199a54c20c9ec65415c452fe04ea9b0" &&
				Dynamic->strSourceRecordSha256 ==
					"3a47d5aeea39c5248c89be4615b1d87588205f88edf2f77f18ba35909332ac91" &&
				Dynamic->DistributionIds.size() == 4u;
			for (size_t i = 0u; bDynamicValid && i < DYNAMIC_ROWS.size(); ++i)
			{
				const std::string NamePath = "dynamicparams[" + std::to_string(i) +
					"].paramname";
				const std::string ValuePath = "dynamicparams[" + std::to_string(i) +
					"].paramvalue";
				const auto Name = std::find_if(Program.Literals.begin(),
					Program.Literals.end(), [&](const auto& Row)
					{
						return Row.strModuleId == Dynamic->Row.strId &&
							Row.strPropertyPath == NamePath;
					});
				const auto Distribution = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Dynamic->DistributionIds[i];
					});
				bDynamicValid = Name != Program.Literals.end() &&
					Name->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					Name->strEnumValue == "none" &&
					Name->Row.strRowSha256 == DYNAMIC_NAME_ROWS[i] &&
					Distribution != Program.Distributions.end() &&
					Distribution->strModuleId == Dynamic->Row.strId &&
					Distribution->strPropertyPath == ValuePath &&
					Distribution->Row.iOrder == i &&
					Distribution->Row.strRowSha256 == DYNAMIC_ROWS[i] &&
					Distribution->eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
					Distribution->iComponentCount == 1u &&
					Distribution->iOperation.value_or(UINT32_MAX) == 1u &&
					Distribution->iLookupTableChunkSize == 1u &&
					Distribution->iLookupTableNumElements == 1u &&
					Distribution->fLookupTableStartTime == 0.;
			}
			if (!bDynamicValid)
			{
				strOutError = "Artist F active028 age-aligned dynamic.xyzw changed.";
				return false;
			}

			size_t iScalar = 0u;
			size_t iVector = 0u;
			size_t iTexture = 0u;
			for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Input : Inputs)
			{
				if (Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64)
				{
					f32_t* Values = &Resource.RuntimeMaterialV2ScalarBlocks[iScalar / 4u].x;
					if (!AssignFloat(*Input->fValue, Values[iScalar % 4u]))
						return false;
					++iScalar;
				}
				else if (Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4)
				{
					if (!AssignFloat4(Input->vValue,
						Resource.RuntimeMaterialV2Vectors[iVector++]))
						return false;
				}
				else
				{
					++iTexture;
				}
			}
			if (iScalar != 18u || iVector != 1u || iTexture != 5u)
				return false;
			/* color_tex is a recipe alias for diff_tex2. Its input identity is
			   verified above, but the recovered base equation never reads it, so
			   bit 21 remains explicitly suppressed and no sixth SRV is staged. */
			Resource.RuntimeMaterialV2ScalarBlocks[4u].z = 1.f;
			/* The recovered neighbor taps encode the literal 0.0078125. The fixed
			   t0 stage below also pins the DDS hash and 128x128 dimensions, so this
			   is exact for that resource rather than a guessed resolution fallback. */
			Resource.RuntimeMaterialV2ScalarBlocks[4u].w = 1.f / 128.f;
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 18u;
			Resource.iRuntimeMaterialV2VectorCount = 1u;
			Resource.iRuntimeMaterialV2InputCount = 24u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x00c7ffffu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x00380000u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask = { 0x0fu, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask = { 0u, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 0u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x07u;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x38u;

			struct TEXTURE_SLOT final
			{
				size_t iInput;
				std::string_view strBindingRow;
				std::string_view strPolicyId;
				std::string_view strPolicyRow;
				std::string_view strAssetId;
				uint64_t iDdsBytes;
				std::string_view strDdsSha256;
				std::string_view strRole;
			};
			static constexpr std::array<TEXTURE_SLOT, 4u> TEXTURE_SLOTS = {{
				{ 15u, "4102d6c0f4837eaeddb4c8ac7c0cc6191f30d76ea3bf2938605ec04755724520", "material-reconstructed-policy-f9b2be2282d20e3823ff", "dde8f2ce15476fa3747cc9b3e32ed42cacae6eb664a9e33019ab748165dbf7d4", "Effect/Artist/Textures/fx_j_smoke_01_cl.dds", 32896u, "59ee3b1cd6119b3a3b5da94ccd2f1b4745bd236ad187d0e2862d416dfe11aa0d", "RECOVERED_BASE_PS_T1_MASK_G" },
				{ 16u, "62993bce51a1cd033081fd24e7e2d2d4c0e366ef47d7c473b95e9cc56540e360", "material-reconstructed-policy-855646ac4e09266526e4", "3a43fd3ca76d0560c06a1a82978b128753c06e3bacb8880f46892c3748c804ed", "Effect/Artist/Textures/fx_k_auraline_16.dds", 65664u, "9c945f925703e725d23ac226f7dcc734b478dd3904fe9f2a0aef38a51a258b7d", "RECOVERED_BASE_PS_T2_OPACITY_G" },
				{ 13u, "877b173d560bf6c9503ff0a4ba78c1a4f278c688e65211c3b94f144f31b0712e", "material-reconstructed-policy-3e54f5fafef85d526d36", "f839473ebe55880d780b9697d0546bd002cdd7cf1ca6e74e4598867f6165ad1b", "Effect/Artist/Textures/fx_k_auraline_16.dds", 65664u, "9c945f925703e725d23ac226f7dcc734b478dd3904fe9f2a0aef38a51a258b7d", "RECOVERED_BASE_PS_T3_DIFF1_RGB" },
				{ 14u, "cfb94cee95714010d238bb40414c9f097f52e8026ed739f0d590261eed8a6e0e", "material-reconstructed-policy-cc2ea5058f47933c9d5b", "ed320b5aad395e3b929f03e420611d446fedb018fda342fcd3ef3187a7e0e7a4", "Effect/Artist/Textures/fx_l_environment_001.dds", 32896u, "cd9285960a7fde29076a46a64911c23786d28eca4f64a6f9c7e6d468455b48d1", "RECOVERED_BASE_PS_T4_DIFF2_RGB" }
			}};
			for (const TEXTURE_SLOT& Slot : TEXTURE_SLOTS)
			{
				const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE& Input = *Inputs[Slot.iInput];
				const std::string BindingId = std::string(Input.Row.strId) +
					"::runtime-texture-binding";
				const auto Binding = std::find_if(Program.MaterialTextureBindings.begin(),
					Program.MaterialTextureBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == BindingId;
					});
				const auto Policy = std::find_if(Program.MaterialPolicies.begin(),
					Program.MaterialPolicies.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Slot.strPolicyId;
					});
				if (Binding == Program.MaterialTextureBindings.end() ||
					Policy == Program.MaterialPolicies.end() ||
					Binding->strRecipeId != Recipe->Row.strId ||
					Binding->strMaterialInputFieldId != Input.Row.strId ||
					Binding->Row.strRowSha256 != Slot.strBindingRow ||
					Binding->strSamplerPolicyRowId != Slot.strPolicyId ||
					Policy->Row.strRowSha256 != Slot.strPolicyRow ||
					!Binding->strRuntimeAssetId.has_value() ||
					*Binding->strRuntimeAssetId != Slot.strAssetId)
					return false;

				const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING* Sidecar = nullptr;
				uint32_t iSidecarCount = 0u;
				for (const auto& [strId, Candidate] : Authority.TextureBindingsById)
				{
					(void)strId;
					if (Candidate.strCandidateBindingId == Binding->Row.strId &&
						Candidate.strCandidateBindingRowSha256 ==
							Binding->Row.strRowSha256)
					{
						Sidecar = &Candidate;
						++iSidecarCount;
					}
				}
				if (iSidecarCount != 1u || nullptr == Sidecar ||
					Sidecar->strRuntimeAssetId != Slot.strAssetId ||
					Sidecar->iActualDdsByteCount != Slot.iDdsBytes ||
					Sidecar->strActualDdsRawSha256 != Slot.strDdsSha256)
				{
					strOutError = "Artist F active028 exact DDS resource changed.";
					return false;
				}
				RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
					std::string(Input.Row.strId), std::string(Input.Row.strRowSha256),
					BindingId, std::string(Slot.strBindingRow),
					std::string(Slot.strPolicyId), std::string(Slot.strPolicyRow),
					std::string(Slot.strAssetId), std::string(Slot.strRole) });
			}
		}
		else if (bActive009010RuntimeV2)
		{
			struct EXPECTED_SCALAR_INPUT final
			{
				size_t iRecipeIndex;
				std::string_view strId;
				std::string_view strName;
				double fValue;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_SCALAR_INPUT, 29u>
				EXPECTED_SCALARS = {{
					{ 0u, "material-input-d587e70846952155", "dissolve_pan_v_time", 1.0,
						"8c61ec8cc1ef301b15e4e235dc662b1639c388e3424e86ccea6f9c6e1a72d5a5" },
					{ 1u, "material-input-0a4ecd85a0a5354d", "dissolve_texcoord_u", 1.0,
						"6960f321118b6f5948d1b2b944ab6277874480e49b65b3fddd1c9258a9e1dd8f" },
					{ 2u, "material-input-66cd2977420655e9", "dissolve_texcoord_v", 0.10000000149011612,
						"59486187e4a7170f9e55a4a51f84c099925ce9ea722a2803fd788bc15dcf6188" },
					{ 3u, "material-input-1bde1901c8431cd9", "disto_power", 2.0,
						"81abc51a28f87b9577d946a91df639905e4b4d0cb0b2b8f83658c9ffd4e5a491" },
					{ 4u, "material-input-c0d57b402df2bb6b", "camera_vector_fresnel_velue", 1.2000000476837158,
						"49df01c0a3b3324a6fa57e4213a14a807a8169ee04361ca65151d665f38b0b4b" },
					{ 5u, "material-input-ad025e21b5551c06", "main_panspeed_u", 0.0,
						"3a5025095b990c6a9c4b20c725e6409138388c479f504738ae6953cb60b7cd91" },
					{ 6u, "material-input-511542f767f326b3", "main_panspeed_v", 0.0,
						"aadc5eabb7580998bb0e6c4e17a3a1acd1d5feb111c711071a01fe76a474ed65" },
					{ 7u, "material-input-e3ab3e55c3e88662", "main_tex_background_velue", 1.0,
						"140edabefb8ef757a8535db7d5e4f6a9b2f9763b3b6bc31cff13558798885214" },
					{ 8u, "material-input-008ec17dd080f3c0", "main_tex_power", 2.0,
						"b3a94356265846eb528733418922eb624d9ec8c9f2086be141161d447c213fb0" },
					{ 9u, "material-input-23ec8db0eb43b784", "main_tex_power_multiply", 0.009999999776482582,
						"2ab650e3f7bb38d4c1bc6ca4e01fae2d10fdff1612a096789ce561acfe6f0b9b" },
					{ 10u, "material-input-8c567b52d6217a7b", "maintex_move_u", 1.0,
						"578df353075e0d04e4e2e08b980409ee573bdd4be50f229121553f2ee3f901ee" },
					{ 11u, "material-input-eac478012b57679a", "maintex_move_v", 0.10000000149011612,
						"8317caea44f196132679f0942a346f1337042953973a468b7342dfa7857361ab" },
					{ 12u, "material-input-fea1be8d7d3b78d5", "maintex_pan_u_time", 0.0,
						"91d19d4f5f9813cbdfab2ad3803c0aeea1d247609b4b0821b28abc7191ffc0ed" },
					{ 13u, "material-input-70c5452e9ebf9980", "maintex_pan_v_time", 1.0,
						"8ae84aa46493efdce13ad7e2714d4de5f8ccd5ea4d519d80c7439f3dd0396013" },
					{ 14u, "material-input-2a983cbc77f16d22", "maintex_rotator", 0.0,
						"4a631d4377aec3d3c07c70d00c4a8b555bfec6682efaa90edaf48b55084115a7" },
					{ 15u, "material-input-07ff321fe3b8f0bd", "maintex_texcoord_u", 0.4000000059604645,
						"d748e7454748bb839aa2dc6678274cfd01558cd981cbfb701e24ddaf76c9d584" },
					{ 16u, "material-input-1936b9e732dfd3d9", "maintex_texcoord_v", 1.0,
						"e543177a843201e0ae7ced9aca1ef1e0b23e0e7e91819a1ccf32967b639496c0" },
					{ 17u, "material-input-64fab0943d4b012d", "uv_noise_pan_u", 0.0,
						"c107beef8f217dff98e3661665b3bc0fc51944d085f7f9bfef0647f67278a566" },
					{ 18u, "material-input-32e3f2d9949bd972", "uv_noise_pan_v", 1.0,
						"84bab665f513bdd711346c3cb94559d1ce7804a96183c2518788b83312072ff4" },
					{ 19u, "material-input-e7ff78cf0d839e38", "uv_noise_panspeed_u", 0.0,
						"7570c8cddb2b0167fbff9656f786b9c054926521e4d5db264635af3579690cfc" },
					{ 20u, "material-input-d85f9fa1f356ee0a", "uv_noise_panspeed_v", 0.0,
						"82a8b9e22f9b0b00692975f4316b47b3b3430a2e2d300fccc58654bb327edfd6" },
					{ 21u, "material-input-a7cbdcfc9506cfcb", "uv_noise_texcoord_u", 1.0,
						"faefc48e512e5ccd2c0ed5addc2f473574970ac17555fb5fd14bfad7e1112fc1" },
					{ 22u, "material-input-375eba8ae8b179cf", "uv_noise_texcoord_v", 1.0,
						"aa27298076bd6b8e8df28cd829d5c3e3468f25e82a4c4e2c7c611b8e8c6b18aa" },
					{ 23u, "material-input-398dc81056064cae", "uv_noise_velue", 0.0,
						"6b49a924b80002486435306ba5287e8b6342be4975a9e2637bc1e3a482f30154" },
					{ 26u, "material-input-d1db483f79dfe6fa", "outalpha_falloff_velue", 40.0,
						"5f99cad8f830ea195581ec74433fba7d97e74fd8083ddcb64dc51d8fc7d92e4a" },
					{ 27u, "material-input-8c5269165108f509", "disslove_hardness", 2.0,
						"02c8c097e285f7aa96b194b8982a4c2019e5628fc34a84b8e21dc37a7dafc80f" },
					{ 28u, "material-input-c9ff76add9f8edb3", "depthbias_velue", 1.0,
						"b772b9a89969e550c57f28471e9b4d57a466fca67d3a36e530449fb1d86240ed" },
					{ 30u, "material-input-6640406dd6ea5fae", "maintex_desaturation", 0.20000000298023224,
						"f67039edf3b258a9843a7b86d5a2717da95bee500086563d586ce749b9d66e5b" },
					{ 31u, "material-input-c8b4d8d5af173797", "alpha_strength", 1.0,
						"6103d97cc4e69ee98bc1df1d13facd203809a982acd501724154be3c34cd9804" }
				}};
			struct EXPECTED_TEXTURE_INPUT final
			{
				size_t iRecipeIndex;
				std::string_view strId;
				std::string_view strName;
				std::string_view strTextureId;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_TEXTURE_INPUT, 2u>
				EXPECTED_TEXTURES = {{
					{ 24u, "material-input-35e14c7b9b51af67", "maintex",
						"fx_tex_04.fx_h_wave_01",
						"4231425b0356dc5beba19ed99d62229c03ef2382187b653af176f003684ff988" },
					{ 25u, "material-input-f7f3ac4458ba6a16", "uv_noise_tex",
						"fx_tex_00.fx_a_noise_011",
						"331e7f28ffb33722e532720c9344d4a9dd1a06a945f844afed45d838d194d0cb" }
				}};
			struct EXPECTED_STATIC_INPUT final
			{
				std::string_view strId;
				std::string_view strName;
				bool_t bSelected;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_STATIC_INPUT, 14u>
				EXPECTED_STATICS = {{
					{ "material-input-8a5aa3538641c7ab", "usemesh", true, "5bb8f7a0417d9b88d9325909652e7158352b9bad147c844fac65e1140c225766" },
					{ "material-input-b906929226ec82e7", "usemesh", true, "efc62b6bc4da36a4052453caf87c68920f8b63bc33f6e177581cff04ce85be9f" },
					{ "material-input-7b2fed6c38dd5b7e", "usemesh", true, "6fe3aad3c52d3245bb57edb27072ce53a62514eb810f8850d1126cffcdea7a57" },
					{ "material-input-9ce189588ba0a559", "use_camera_vector_fresnel", true, "77a008ab570863c6275bacb36363843766e6313dc717e8aee888aa8a48cc2eae" },
					{ "material-input-bac22455d269b8a3", "use_distortion", true, "7fac984e6daa3357acc84f355a5d97568915372792c27a2f69c2b0354f7878df" },
					{ "material-input-97ce85896e54567f", "use_outfalloff", true, "8a8bb11ecc0448b4e6f9834c87817885235563f14122bef165669f85649a2e51" },
					{ "material-input-b37d26fc2a548ee7", "outalpha_falloff_v", true, "ae695ea56ee0550f53ff71be3e78aa42023ae0395f5944211664ea644082f0cb" },
					{ "material-input-210405d530895775", "outalpha_falloff_u", true, "eaa26ceb8e82a79a3ec645f50a4acdb144cdf6e4d0d62896575ee0de02494751" },
					{ "material-input-7e352b4b0a47e4a2", "usemesh", true, "8b77cdf9ef8e788e77f9688053e7624a062525dbe0f7ded9b7c45d72d00dbb80" },
					{ "material-input-772fb9cb6259f96f", "usemesh", true, "2e99eaf1ea4a12f919dedd187677a9e7212fd8a261d8df1c0e7578cf64c2c81e" },
					{ "material-input-134a7f9bf43cd082", "use_alpha_channal", false, "bae466ad58304e6d68e73d02244c7a87e638292ff0b165f16bfa25bc3e5438e4" },
					{ "material-input-c6ae83a89bb3119c", "use_reflction", false, "00a143bae5241b1d03bc4e81244a6a5da92a49a1dceab57367bc1177050e5bfe" },
					{ "material-input-efc101fd88c2dac4", "usemesh", true, "472571cced378e6b0b4ebb2e87ccbc9169c5bbf0787070c10b9be53b8dd7b0d2" },
					{ "material-input-44fd4d8c2f5a0f89", "uv_noise_on", true, "cfa3ac48d09ff54bd78b05ec9ad37fbe6a0ec218554993670be1bdd4941605c8" }
				}};
			static constexpr std::array<std::pair<std::string_view,
				std::string_view>, 6u> EXPECTED_RENDER_ROWS = {{
				{ "blendmode", "3c36148f3f585d7f5ec2c4f6463a17e7848cc205791164f3edabe4d29cb610ef" },
				{ "lightingmodel", "7165b3f2821500b3bb283017e97ca75bdef36c928d3120c91a05a438e1f4b246" },
				{ "twosided", "79aa2e86741a688c09e98699c6596eb1b8b7dc272edb479828473bd597e71589" },
				{ "bdisabledepthtest", "fd33c110a176592053c5c9b6ac823c357aa6f2cf99e689a844e0be32e81c54ea" },
				{ "opacitymaskclipvalue", "4d0f7c83e9e8f11130f82f7cf1683200d21693828e47d766aa89acba3e18a8f5" },
				{ "buseonelayerdistortion", "e1947e4365026a345e38a1e57462b8d2fb6aa8afb25456cba59bc45f69cbbaed" }
			}};
			static constexpr std::array<std::string_view, 2u>
				EXPECTED_OCCURRENCE_IDS = {{
					"source-active-009", "source-active-010"
				}};
			static constexpr std::array<std::string_view, 2u>
				EXPECTED_OCCURRENCE_ROWS = {{
					"d08da0a4103ad6794d2168263687b18a009ec2d12a5d1896f43be7136d58a658",
					"f616b5691f07e95036c8408d71ae9a9d8d2d6593778e040b85e438b17b79aaa4"
				}};

			if (Occurrence->Row.strId !=
					EXPECTED_OCCURRENCE_IDS[Emitter.Row.iOrder - 9u] ||
				Occurrence->Row.strRowSha256 !=
					EXPECTED_OCCURRENCE_ROWS[Emitter.Row.iOrder - 9u] ||
				Recipe->Row.strId != "material-recipe-03cc03b86c1a4c8f" ||
				Recipe->Row.strRowSha256 !=
					"b24ac0f12b03d6f92a6f8569c87eb7d766a4976d8d8073c8d6abb4bbfce8b45f" ||
				Family->Row.strId != "material-family-89af5c77d8e35f99" ||
				Family->Row.strRowSha256 !=
					"279b0cece1cb0b361318b4f94f31ba29b5f9a0eefaba2ccdc0a8e90e2421658b" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-f9236f78e622ee89" ||
				Family->strEvaluatorSha256 !=
					"5d2cd24c1076370ce8710dd70a7d7a0a8e1ab0959e2622e0c9ad7ff5033950af" ||
				Family->iFeatureMask != 1023u || Recipe->InputIds.size() != 32u ||
				Recipe->StaticBindingIds.size() != EXPECTED_STATICS.size() ||
				Recipe->RenderBindingIds.size() != EXPECTED_RENDER_ROWS.size() ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError =
					"Artist F active009/010 runtime-material identity changed.";
				return false;
			}

			const auto FindInputById = [&](const std::string_view Id)
				-> const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*
			{
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					});
				return It == Program.MaterialInputs.end() ? nullptr : &*It;
			};
			const auto StoreScalar = [&](const size_t iScalar,
				const double fValue)
			{
				if (iScalar >= Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u)
					return false;
				f32_t Converted = 0.f;
				if (!AssignFloat(fValue, Converted))
					return false;
				float4_t& Block = Resource.RuntimeMaterialV2ScalarBlocks[iScalar / 4u];
				switch (iScalar % 4u)
				{
				case 0u: Block.x = Converted; break;
				case 1u: Block.y = Converted; break;
				case 2u: Block.z = Converted; break;
				case 3u: Block.w = Converted; break;
				default: return false;
				}
				return true;
			};
			for (size_t iScalar = 0u; iScalar < EXPECTED_SCALARS.size(); ++iScalar)
			{
				const EXPECTED_SCALAR_INPUT& Expected = EXPECTED_SCALARS[iScalar];
				const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Input =
					FindInputById(Expected.strId);
				if (Recipe->InputIds[Expected.iRecipeIndex] != Expected.strId ||
					nullptr == Input || Input->strRecipeId != Recipe->Row.strId ||
					Input->strNormalizedParameterName != Expected.strName ||
					Input->Row.strRowSha256 != Expected.strRowSha256 ||
					Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!Input->fValue.has_value() || *Input->fValue != Expected.fValue ||
					!StoreScalar(iScalar, *Input->fValue))
				{
					strOutError = "Artist F active009/010 scalar input changed.";
					return false;
				}
			}
			const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* ReflectionColor =
				FindInputById("material-input-9e772bb243d4076a");
			if (Recipe->InputIds[29u] != "material-input-9e772bb243d4076a" ||
				nullptr == ReflectionColor ||
				ReflectionColor->strRecipeId != Recipe->Row.strId ||
				ReflectionColor->strNormalizedParameterName != "reflection_color" ||
				ReflectionColor->Row.strRowSha256 !=
					"52aaee6052a966def9d70e95108ad4c4fbf1c3dc541f2588dd69d4ed91416468" ||
				ReflectionColor->eVariant !=
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
				ReflectionColor->vValue != std::array<double, 4u>{ 4.0, 3.0, 2.0, 1.0 } ||
				!AssignFloat4(ReflectionColor->vValue,
					Resource.RuntimeMaterialV2Vectors[0u]))
			{
				strOutError = "Artist F active009/010 vector input changed.";
				return false;
			}

			RuntimeTextureProviders.reserve(EXPECTED_TEXTURES.size());
			for (size_t iTexture = 0u; iTexture < EXPECTED_TEXTURES.size(); ++iTexture)
			{
				const EXPECTED_TEXTURE_INPUT& Expected = EXPECTED_TEXTURES[iTexture];
				const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE* Input =
					FindInputById(Expected.strId);
				const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER& Provider =
					iTexture == 0u ? TextureDecision->Texture0Provider :
						TextureDecision->Texture1Provider;
				if (Recipe->InputIds[Expected.iRecipeIndex] != Expected.strId ||
					nullptr == Input || Input->strRecipeId != Recipe->Row.strId ||
					Input->strNormalizedParameterName != Expected.strName ||
					Input->Row.strRowSha256 != Expected.strRowSha256 ||
					Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ||
					Input->strStringValue != Expected.strTextureId ||
					Provider.strMaterialInputFieldId != Expected.strId ||
					Provider.strMaterialInputRowSha256 != Expected.strRowSha256)
				{
					strOutError = "Artist F active009/010 texture input changed.";
					return false;
				}
				RuntimeTextureProviders.push_back(Provider);
			}

			uint32_t iSelectedStaticMask = 0u;
			for (size_t iStatic = 0u; iStatic < EXPECTED_STATICS.size(); ++iStatic)
			{
				const EXPECTED_STATIC_INPUT& Expected = EXPECTED_STATICS[iStatic];
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Expected.strId;
					});
				if (Recipe->StaticBindingIds[iStatic] != Expected.strId ||
					It == Program.MaterialStaticBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != Expected.strName ||
					It->Row.strRowSha256 != Expected.strRowSha256 ||
					!It->bSourceValue.has_value() || !*It->bSourceValue ||
					!It->bSelectedValue.has_value() ||
					*It->bSelectedValue != Expected.bSelected)
				{
					strOutError = "Artist F active009/010 static input changed.";
					return false;
				}
				if (Expected.bSelected)
					iSelectedStaticMask |= 1u << static_cast<uint32_t>(iStatic);
			}
			if (iSelectedStaticMask != 0x33ffu)
			{
				strOutError = "Artist F active009/010 static selection changed.";
				return false;
			}

			for (size_t iRender = 0u; iRender < EXPECTED_RENDER_ROWS.size(); ++iRender)
			{
				const auto& [FieldName, RowSha256] = EXPECTED_RENDER_ROWS[iRender];
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[iRender];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != FieldName ||
					It->Row.strRowSha256 != RowSha256)
				{
					strOutError = "Artist F active009/010 render input changed.";
					return false;
				}
			}

			struct EXPECTED_DYNAMIC_CONTRACT final
			{
				std::string_view strModuleRowSha256;
				std::array<std::string_view, 4u> LiteralRows;
				std::array<std::string_view, 4u> DistributionRows;
			};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"alpha_pan", "uv_noise_pan", "dissolve", "noise_velue"
			}};
			static constexpr std::array<EXPECTED_DYNAMIC_CONTRACT, 2u>
				EXPECTED_DYNAMICS = {{
					{ "defa465a5cbf0658260d2c169c1392f0ec51207944cf455f0aa19f47ea4d89f4",
						{{ "e0214a7a33b92389ea50241295382e5b13391a04899f42054c72234285120b5e",
							"fc52c110c9640d3fa63cc75c04d99dd27275329f51995230a7a8c2f434939471",
							"753afbd0e733dfd1a30fb4ea3e6e1445e06dab5144d89f744b4af55df58bc3a1",
							"b9795394b265180e601a3b1b2b309475ac4627c992fd8934ccc2367773f5bf30" }},
						{{ "72b5fba05a0a69b7ceaa4ba83506ca2e0a52fe374c2d2bc9986a704ccd7ad0c4",
							"935a8a2005e3cb8995bdb10249421b5a91312aea69c769705e91fc0d84cfbd2b",
							"62a0d2f0dc503339a12a7a2426c8bad781130d8e827985100958bdb688a5e415",
							"189bacfebc3d312b98a6b0e02689d205e354f3aff18d503f23e4a7ea0d70e48b" }} },
					{ "1edefb2b1bc3b39fa22691dc44499eb55d50b9a682d5a3a4ddf3725856da0e7a",
						{{ "687acdaef6e84d6c154456234ab2c4fdb86c36daf7c87c00d26245ca83c60e01",
							"25347017608273098a3b72f8f51c41c6afa618d1ad71267b7eca22e5d80f87bc",
							"ba069d22438b4f690cd26ee9eccc5527ce8144ad46375108892a267d194af9ea",
							"72cefbb667c7d9b7f311189b2b6b8f3df01de039a5c231ada261c2d6a48dab04" }},
						{{ "de2904ce3fecbff953e452bac35ef13f1fcf6bcf98896ab8dcb5757f117798ef",
							"3dfc0c6da6a9adb6dc6f1313e13fee77441491ff68c534a688435fd40f2d3bf3",
							"a22d61eb156e26238861874fa97dfb93d20387834e5f5cb041a623837eb65923",
							"a2546b3cba57db7b9a30a4cb0919c74f080dcb262d4efc992561f74446cd2fd8" }} }
				}};
			const EXPECTED_DYNAMIC_CONTRACT& ExpectedDynamic =
				EXPECTED_DYNAMICS[Emitter.Row.iOrder - 9u];
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			uint32_t iDynamicModuleCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					DynamicModule = &*It;
					++iDynamicModuleCount;
				}
			}
			bool_t bDynamicContract = iDynamicModuleCount == 1u &&
				nullptr != DynamicModule && DynamicModule->Row.iOrder == 9u &&
				DynamicModule->Row.strRowSha256 == ExpectedDynamic.strModuleRowSha256 &&
				DynamicModule->DistributionIds.size() == DYNAMIC_NAMES.size();
			for (size_t iDynamic = 0u;
				bDynamicContract && iDynamic < DYNAMIC_NAMES.size(); ++iDynamic)
			{
				const std::string NamePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramname";
				const std::string ValuePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramvalue";
				const EFFECT_RUNTIME_PROGRAM_LITERAL* NameLiteral = nullptr;
				uint32_t iNameCount = 0u;
				for (const EFFECT_RUNTIME_PROGRAM_LITERAL& Literal : Program.Literals)
				{
					if (Literal.strModuleId == DynamicModule->Row.strId &&
						Literal.strPropertyPath == NamePath)
					{
						NameLiteral = &Literal;
						++iNameCount;
					}
				}
				const auto DistributionIt = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == DynamicModule->DistributionIds[iDynamic];
					});
				bDynamicContract = nullptr != NameLiteral && iNameCount == 1u &&
					NameLiteral->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					NameLiteral->strEnumValue == DYNAMIC_NAMES[iDynamic] &&
					NameLiteral->Row.strRowSha256 == ExpectedDynamic.LiteralRows[iDynamic] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strModuleId == DynamicModule->Row.strId &&
					DistributionIt->Row.iOrder == iDynamic &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u &&
					DistributionIt->Row.strRowSha256 ==
						ExpectedDynamic.DistributionRows[iDynamic];
			}
			if (!bDynamicContract)
			{
				strOutError = "Artist F active009/010 dynamic input changed.";
				return false;
			}

			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 29u;
			Resource.iRuntimeMaterialV2VectorCount = 1u;
			Resource.iRuntimeMaterialV2InputCount = 32u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0xcffffff7u, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x30000008u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				{ 0u, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				{ 0x0fu, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 14u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x33ffu;
			// The recovered ShaderMap identity consumes the complete active static
			// set.  A changed static row selects a different native shader map even
			// when that row is optimized out of this RT0 instruction slice.
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x3fffu;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
		}
		else if (bActive016RuntimeV2)
		{
			static constexpr std::array<size_t, 3u> VECTOR_INPUT_INDICES =
				{ 21u, 22u, 49u };
			static constexpr std::array<size_t, 8u> TEXTURE_INPUT_INDICES =
				{ 23u, 24u, 25u, 26u, 27u, 30u, 33u, 45u };
			static constexpr std::array<std::string_view, 3u> VECTOR_NAMES = {{
				"93.emissiion_color", "09.specmap_color", "62.color"
			}};
			static constexpr std::array<std::array<double, 4u>, 3u>
				VECTOR_VALUES = {{
					{{ 1.0, 1.0, 1.0, 3.0 }},
					{{ 1.0, 1.0, 1.0, 33.0 }},
					{{ 3.0, 1.0, 0.10000000149011612, 1.0 }}
				}};
			struct EXPECTED_TEXTURE_INPUT final
			{
				size_t iRecipeIndex;
				std::string_view strId;
				std::string_view strName;
				std::string_view strTextureId;
				std::string_view strRuntimeAssetId;
				std::string_view strRowSha256;
				std::string_view strBindingRowSha256;
			};
			static constexpr std::array<EXPECTED_TEXTURE_INPUT, 5u>
				EXPECTED_TEXTURES = {{
					{ 23u, "material-input-75efc29fef0cb95a", "21.map_c",
						"fx_tex_02.fx_d_fluid_007",
						"Effect/Artist/Textures/fx_d_fluid_007.dds",
						"76876fb6937a94af65387ea890ccf6229230d8d0f323f26ab72ace2ac37d1743",
						"86013c8e56b97a891dcd9e3d2281f08b3648fd395ae74c3518c7fb7507b2eeec" },
					{ 24u, "material-input-3c6503f3b8165e28", "02.map_e",
						"fx_tex_00.fx_a_ice_002",
						"Effect/Artist/Textures/fx_a_ice_002.dds",
						"d8b2c5cc3e3f1e0181f40487d0c3f4ff5ee7b21e826870802080d3f5ef5d7c93",
						"218c7ad8c45794c82bd2bb7f2f894b27056ca0fe12087d6b71d38aae677104b7" },
					{ 25u, "material-input-323ed9dbf8e8f9ee", "12.map_f",
						"fx_tex_02.fx_d_atypical_098",
						"Effect/Artist/Textures/fx_d_atypical_098.dds",
						"075102273f9eb40add8154ac8d91d01de87499040a9c9cd58967f3345fa52781",
						"edd5245447a501017a477bfa1e41a4de8a16dcaefc03641714308a1e3c31efad" },
					{ 26u, "material-input-2f830bc7586bd3e0", "01.specmap",
						"fx_tex_00.fx_a_noise_002",
						"Effect/Artist/Textures/fx_a_noise_002.dds",
						"6fa324a5425f137f778a27bf26aa93e347cb5db820055ad1d0666248cdfc0330",
						"2590416a7a0810806dde53ffb1f1f7fb8136ce2496c7dba58c22d8463280ff6a" },
					{ 27u, "material-input-e51237e20a813da8", "06.map",
						"fx_tex_00.fx_a_noise_002",
						"Effect/Artist/Textures/fx_a_noise_002.dds",
						"f55be7cbc10df7d36fa0211ab4490a8a0817401183efdb7995294433fe37545f",
						"af3c4955e8974073390c8cbf5b26a1f975a3328975b9ca9423542af5da9d4527" }
				}};
			if (Occurrence->Row.strId != "source-active-016" ||
				Occurrence->Row.strRowSha256 !=
					"18d01e703923a9328b35387d292a4378b632ebdb3f29a6d36371594f7f39f04e" ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					"95a019f8099d1b96f3c3fe139c33f6c7a44b990aca676c3532aa84743629ffd9" ||
				Occurrence->strBindingSha256 !=
					"a6c59a0325db51237620cdb4907d0028712164132d35baac0ad7cdda95554b48" ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					"beb50ac16bb083d085fd93973946addbea4696881d092aece2a3e45eeeaf7373" ||
				Recipe->Row.strId != "material-recipe-4070769760015ff3" ||
				Recipe->Row.strRowSha256 !=
					"f71db77d9000174409b7e4b9b95a0b106ff3b2e8d7bcbafe369531cde5a951a6" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"5ce4078e505acc23cce6f3b5c3184b2bc825ac1c180f872b0ab272e9a392ad46" ||
				Recipe->strBindingSha256 !=
					"a6c59a0325db51237620cdb4907d0028712164132d35baac0ad7cdda95554b48" ||
				Family->Row.strId != "material-family-d0f9d7bb8b80fee0" ||
				Family->Row.strRowSha256 !=
					"f4f53506aad5cd082f6622730a3186a30d5de348f14ad9808414bc351f0b4085" ||
				Family->strFamilyIdentitySha256 !=
					"165b79044e71cb8d3c58216288d5c8af2c834b79391a8acee9e47630e6eae219" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-f79a91bbcae88cbc" ||
				Family->strEvaluatorSha256 !=
					"ac5dc8467944d9ca8aba49989d220d3613193f15fa5525d47b476ed2d15786f7" ||
				Family->strSampleProjectionSha256 !=
					"993fc439d78e0647f94954257f05a001e27c3877fdf48fbdb3183eacfad4274c" ||
				Family->iFeatureMask != 1023u || Recipe->InputIds.size() != 55u ||
				Recipe->StaticBindingIds.size() != 7u ||
				Recipe->RenderBindingIds.size() != 6u ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F active016 runtime-material identity changed.";
				return false;
			}

			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 55u>
				OrderedInputs{};
			for (size_t iInput = 0u; iInput < Recipe->InputIds.size(); ++iInput)
			{
				const std::string& InputId = Recipe->InputIds[iInput];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == InputId;
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row) { return Row.Row.strId == InputId; }));
				if (It == Program.MaterialInputs.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId)
				{
					strOutError = "Artist F active016 material input join changed.";
					return false;
				}
				OrderedInputs[iInput] = &*It;
			}

			const auto StoreScalar = [&](const size_t iScalar, const double fValue)
			{
				if (iScalar >= Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u)
					return false;
				f32_t Converted = 0.f;
				if (!AssignFloat(fValue, Converted))
					return false;
				float4_t& Block = Resource.RuntimeMaterialV2ScalarBlocks[iScalar / 4u];
				switch (iScalar % 4u)
				{
				case 0u: Block.x = Converted; break;
				case 1u: Block.y = Converted; break;
				case 2u: Block.z = Converted; break;
				case 3u: Block.w = Converted; break;
				default: return false;
				}
				return true;
			};
			size_t iScalar = 0u;
			size_t iVector = 0u;
			size_t iTexture = 0u;
			for (size_t iInput = 0u; iInput < OrderedInputs.size(); ++iInput)
			{
				const auto* Input = OrderedInputs[iInput];
				const bool_t bVector = std::find(VECTOR_INPUT_INDICES.begin(),
					VECTOR_INPUT_INDICES.end(), iInput) != VECTOR_INPUT_INDICES.end();
				const bool_t bTexture = std::find(TEXTURE_INPUT_INDICES.begin(),
					TEXTURE_INPUT_INDICES.end(), iInput) != TEXTURE_INPUT_INDICES.end();
				if (bVector)
				{
					if (iVector >= VECTOR_NAMES.size() ||
						Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
						Input->strNormalizedParameterName != VECTOR_NAMES[iVector] ||
						Input->vValue != VECTOR_VALUES[iVector] ||
						!AssignFloat4(Input->vValue,
							Resource.RuntimeMaterialV2Vectors[iVector]))
					{
						strOutError = "Artist F active016 vector input changed.";
						return false;
					}
					++iVector;
				}
				else if (bTexture)
				{
					if (Input->eVariant !=
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
					{
						strOutError = "Artist F active016 texture input type changed.";
						return false;
					}
					++iTexture;
				}
				else
				{
					if (Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
						!Input->fValue.has_value() ||
						!StoreScalar(iScalar, *Input->fValue))
					{
						strOutError = "Artist F active016 scalar input changed.";
						return false;
					}
					++iScalar;
				}
			}
			if (iScalar != 44u || iVector != 3u || iTexture != 8u)
			{
				strOutError = "Artist F active016 material input denominator changed.";
				return false;
			}

			RuntimeTextureProviders.reserve(EXPECTED_TEXTURES.size());
			for (const EXPECTED_TEXTURE_INPUT& Expected : EXPECTED_TEXTURES)
			{
				const auto* Input = OrderedInputs[Expected.iRecipeIndex];
				const auto BindingIt = std::find_if(
					Program.MaterialTextureBindings.begin(),
					Program.MaterialTextureBindings.end(), [&](const auto& Row)
					{
						return Row.strMaterialInputFieldId == Input->Row.strId;
					});
				const auto PolicyIt = BindingIt == Program.MaterialTextureBindings.end() ?
					Program.MaterialPolicies.end() :
					std::find_if(Program.MaterialPolicies.begin(),
						Program.MaterialPolicies.end(), [&](const auto& Row)
						{
							return Row.Row.strId == BindingIt->strSamplerPolicyRowId;
						});
				if (Input->Row.strId != Expected.strId ||
					Input->strNormalizedParameterName != Expected.strName ||
					Input->strStringValue != Expected.strTextureId ||
					Input->Row.strRowSha256 != Expected.strRowSha256 ||
					BindingIt == Program.MaterialTextureBindings.end() ||
					BindingIt->Row.strRowSha256 != Expected.strBindingRowSha256 ||
					PolicyIt == Program.MaterialPolicies.end() ||
					!BindingIt->strRuntimeAssetId.has_value() ||
					*BindingIt->strRuntimeAssetId != Expected.strRuntimeAssetId)
				{
					strOutError = "Artist F active016 selected texture input changed.";
					return false;
				}
				EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER Provider;
				Provider.strProviderKind = "MATERIAL_TEXTURE_BINDING";
				Provider.strMaterialInputFieldId = Input->Row.strId;
				Provider.strMaterialInputRowSha256 = Input->Row.strRowSha256;
				Provider.strTextureBindingId = BindingIt->Row.strId;
				Provider.strTextureBindingRowSha256 = BindingIt->Row.strRowSha256;
				Provider.strSamplerPolicyRowId = PolicyIt->Row.strId;
				Provider.strSamplerPolicyRowSha256 = PolicyIt->Row.strRowSha256;
				Provider.strRuntimeAssetId = *BindingIt->strRuntimeAssetId;
				Provider.strSelectionBasis = "ACTIVE016_BOUNDED_RECIPE_INPUT_ORDER";
				RuntimeTextureProviders.push_back(std::move(Provider));
			}

			static constexpr std::array<std::string_view, 7u> STATIC_NAMES = {{
				"31.mapch.r", "32.mapch.g", "00.use_mapa",
				"01.use_emissionmap", "00.use_mapa", "33.mapch.b", "34.mapch.a"
			}};
			static constexpr std::array<bool_t, 7u> STATIC_SELECTED =
				{ true, true, false, true, true, true, true };
			uint32_t iSelectedStaticMask = 0u;
			for (size_t iStatic = 0u; iStatic < Recipe->StaticBindingIds.size(); ++iStatic)
			{
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->StaticBindingIds[iStatic];
					});
				if (It == Program.MaterialStaticBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != STATIC_NAMES[iStatic] ||
					!It->bSourceValue.has_value() || !*It->bSourceValue ||
					!It->bSelectedValue.has_value() ||
					*It->bSelectedValue != STATIC_SELECTED[iStatic])
				{
					strOutError = "Artist F active016 static input changed.";
					return false;
				}
				if (*It->bSelectedValue)
					iSelectedStaticMask |= 1u << static_cast<uint32_t>(iStatic);
			}
			if (iSelectedStaticMask != 0x7bu)
			{
				strOutError = "Artist F active016 static selection changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion"
			}};
			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 6u> RenderRows{};
			for (size_t iRender = 0u; iRender < Recipe->RenderBindingIds.size(); ++iRender)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[iRender];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_FIELDS[iRender])
				{
					strOutError = "Artist F active016 render input changed.";
					return false;
				}
				RenderRows[iRender] = &*It;
			}
			if (RenderRows[0u]->eVariant !=
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::ENUM_STRING ||
				RenderRows[0u]->strStringValue != "blend_translucent" ||
				RenderRows[1u]->eVariant !=
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::ENUM_STRING ||
				RenderRows[1u]->strStringValue != "mlm_unlit" ||
				RenderRows[2u]->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
				!RenderRows[2u]->bValue.has_value() || !*RenderRows[2u]->bValue ||
				RenderRows[3u]->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
				!RenderRows[3u]->bValue.has_value() || *RenderRows[3u]->bValue ||
				RenderRows[4u]->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
				!RenderRows[4u]->fValue.has_value() ||
				*RenderRows[4u]->fValue != 0.33329999446868896 ||
				RenderRows[5u]->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
				!RenderRows[5u]->bValue.has_value() || *RenderRows[5u]->bValue)
			{
				strOutError = "Artist F active016 render value changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"alphadissolve[0-1]", "pan[0-2]", "edgestr[0-x]", "disrotion[0-x]"
			}};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_LITERAL_ROWS = {{
				"54a1df8f43619184e2b68d80a8aea65e4f2e5f148739b10316e35901c539b7fe",
				"45ef7cf65231c31be9a0b74bd980da918518ca9af8a3a1d10f6ad964b4ec7120",
				"eca036cc067e6fb0e4d59f8b5950fb6e8c252be560e23388a797673d2786a5d2",
				"659a098904811d5f4ab43b0a9466e813ae80d23aeb457b35971a6657392670de"
			}};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_DISTRIBUTION_ROWS = {{
				"abbf3d633533b584449d7e03a39a5d0f580a7dfccebdc7f4f9bb1f080d75d4fa",
				"dd94cbdd44ec80231adcc6fa66acca8594cb397b740b5a62dea9424ee448f44e",
				"9d94685f3948a918f648e3e7063b5e0dcd89317f15541c3c0a71aaea5b669322",
				"e0cfcdf512f614a0711ad9b69c3e5abf0561314975a3ae70803dbb1aa3db3e35"
			}};
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			uint32_t iDynamicModuleCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					DynamicModule = &*It;
					++iDynamicModuleCount;
				}
			}
			bool_t bDynamicContract = iDynamicModuleCount == 1u &&
				nullptr != DynamicModule && DynamicModule->Row.iOrder == 5u &&
				DynamicModule->Row.strRowSha256 ==
					"533bd9e4738c3f647d8b7c6cf2098f92ed9abe2c94c065fa4a95432f18217690" &&
				DynamicModule->DistributionIds.size() == DYNAMIC_NAMES.size();
			for (size_t iDynamic = 0u;
				bDynamicContract && iDynamic < DYNAMIC_NAMES.size(); ++iDynamic)
			{
				const std::string NamePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramname";
				const std::string ValuePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramvalue";
				const EFFECT_RUNTIME_PROGRAM_LITERAL* NameLiteral = nullptr;
				uint32_t iNameCount = 0u;
				for (const auto& Literal : Program.Literals)
				{
					if (Literal.strModuleId == DynamicModule->Row.strId &&
						Literal.strPropertyPath == NamePath)
					{
						NameLiteral = &Literal;
						++iNameCount;
					}
				}
				const auto DistributionIt = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == DynamicModule->DistributionIds[iDynamic];
					});
				bDynamicContract = nullptr != NameLiteral && iNameCount == 1u &&
					NameLiteral->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					NameLiteral->strEnumValue == DYNAMIC_NAMES[iDynamic] &&
					NameLiteral->Row.strRowSha256 == DYNAMIC_LITERAL_ROWS[iDynamic] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strModuleId == DynamicModule->Row.strId &&
					DistributionIt->Row.iOrder == iDynamic &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u &&
					DistributionIt->Row.strRowSha256 ==
						DYNAMIC_DISTRIBUTION_ROWS[iDynamic];
			}
			if (!bDynamicContract || !Element.SourceRecipe.bEnabled ||
				Element.Material.SourceMaterial.strSubUVMode != "psuvim_random" ||
				SourceLiteralNumber(Element, "subimages_horizontal", -1.f) != 2.f ||
				SourceLiteralNumber(Element, "subimages_vertical", -1.f) != 2.f ||
				SourceLiteralNumber(Element, "randomimagetime", -1.f) != 1.f ||
				!SourceLiteralBool(Element, "ballowimageflipping", false))
			{
				strOutError = "Artist F active016 dynamic/SubUV contract changed.";
				return false;
			}

			// The cooked ColorOverLife RGB payload for active016 is a null
			// RawDistribution that the currently admitted Program materializes as
			// an empty operation-1 zero descriptor.  The recovered material graph
			// does not prove that ParticleColor.rgb was connected, so opcode 4
			// suppresses only those three unresolved channels.  Pin the exact
			// descriptor here: if a future artifact recovers a real RGB payload,
			// staging must stop instead of silently continuing to ignore it.
			static constexpr std::string_view COLOR_MODULE_ID =
				"fx_pc_sdm_07.par_v_smd_onestroke_swing_01::action-31470/"
				"stage-000/notify-018::FX_PC_SDM_07.par_v_smd_onestroke_swing_01."
				"particlespriteemitter_15::module:002";
			static constexpr std::string_view COLOR_DISTRIBUTION_ID =
				"fx_pc_sdm_07.par_v_smd_onestroke_swing_01::action-31470/"
				"stage-000/notify-018::FX_PC_SDM_07.par_v_smd_onestroke_swing_01."
				"particlespriteemitter_15::module:002::distribution:coloroverlife";
			const auto ColorModuleIt = std::find_if(Program.Modules.begin(),
				Program.Modules.end(), [&](const auto& Row)
				{
					return Row.Row.strId == COLOR_MODULE_ID;
				});
			const auto ColorDistributionIt = std::find_if(
				Program.Distributions.begin(), Program.Distributions.end(),
				[&](const auto& Row)
				{
					return Row.Row.strId == COLOR_DISTRIBUTION_ID;
				});
			const auto AllZero = [](const std::vector<double>& Values)
			{
				return std::all_of(Values.begin(), Values.end(),
					[](const double Value) { return Value == 0.0; });
			};
			bool_t bColorPolicyContract =
				ColorModuleIt != Program.Modules.end() &&
				ColorModuleIt->Row.iOrder == 2u &&
				ColorModuleIt->Row.strRowSha256 ==
					"a0c57cef9c30e9fb25476702897aa2f03065652a251683573f49b21a30aa6826" &&
				ColorModuleIt->strExactSourceClass == "particlemodulecoloroverlife" &&
				ColorModuleIt->DistributionIds.size() == 2u &&
				ColorModuleIt->DistributionIds[0u] == COLOR_DISTRIBUTION_ID &&
				ColorDistributionIt != Program.Distributions.end() &&
				ColorDistributionIt->strModuleId == COLOR_MODULE_ID &&
				ColorDistributionIt->Row.iOrder == 0u &&
				ColorDistributionIt->Row.strRowSha256 ==
					"c7f7677dc678d2673ab60741c0c922910ba8e1f30aca4cb2c9a87496194c63d5" &&
				ColorDistributionIt->strPropertyPath == "coloroverlife" &&
				ColorDistributionIt->iComponentCount == 3u &&
				ColorDistributionIt->eVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
				ColorDistributionIt->iOperation == 1u &&
				ColorDistributionIt->iRandomLockAxes == 0u &&
				ColorDistributionIt->iLookupTableChunkSize == 0u &&
				ColorDistributionIt->iLookupTableNumElements == 0u &&
				ColorDistributionIt->fLookupTableTimeScale == 0.0 &&
				ColorDistributionIt->fLookupTableStartTime == 0.0 &&
				ColorDistributionIt->DefaultMinimum.size() == 4u &&
				ColorDistributionIt->DefaultMaximum.size() == 4u &&
				AllZero(ColorDistributionIt->DefaultMinimum) &&
				AllZero(ColorDistributionIt->DefaultMaximum) &&
				ColorDistributionIt->LookupTable.empty() &&
				ColorDistributionIt->CurveKeys.empty() &&
				ColorDistributionIt->ConstantValues.empty() &&
				ColorDistributionIt->PreservedBlockers.empty() &&
				ColorDistributionIt->Samples.size() == 3u;
			if (ColorDistributionIt != Program.Distributions.end())
			{
				for (const auto& Sample : ColorDistributionIt->Samples)
				{
					bColorPolicyContract = bColorPolicyContract &&
						Sample.OutputValues.size() == 4u &&
						AllZero(Sample.OutputValues);
				}
			}
			if (!bColorPolicyContract)
			{
				strOutError =
					"Artist F active016 blocked ColorOverLife RGB policy changed.";
				return false;
			}

			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
			// BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1: A is consumed by the
			// explicit AlphaOverLife envelope; unresolved RGB is suppressed.
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 1u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x08u;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0x07u;
			Resource.iRuntimeMaterialV2ScalarCount = 44u;
			Resource.iRuntimeMaterialV2VectorCount = 3u;
			Resource.iRuntimeMaterialV2InputCount = 55u;
			Resource.RuntimeMaterialV2InputConsumedMask =
				{ 0x3fff7fffu, 0x00001800u };
			Resource.RuntimeMaterialV2InputSuppressedMask =
				{ 0xc0008000u, 0x007fe7ffu };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				{ 0x07u, 0x07u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				{ 0x08u, 0x08u, 0x0fu };
			Resource.iRuntimeMaterialV2StaticInputCount = 7u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x7bu;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x6bu;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0x14u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
		}
		else if (bActive030RuntimeV2)
		{
			static constexpr std::array<size_t, 3u> VECTOR_INPUT_INDICES =
				{ 18u, 38u, 48u };
			static constexpr std::array<size_t, 8u> TEXTURE_INPUT_INDICES =
				{ 19u, 20u, 21u, 22u, 25u, 28u, 40u, 44u };
			if (Occurrence->Row.strId != "source-active-030" ||
				Occurrence->Row.strRowSha256 !=
					"7228b4e6707e28c5b3edeb6f9a2c5b8a9d0b17d157beee005633225dcd2d327a" ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					"61cdb2597af295a48a8e267ae51d342aa8aac9f01c82a947f5ce9c80207fe2e1" ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					"ff0b6fdea6fa09468f74ca7c064549bef0d262a33888d7d0a08f73d188e83bb6" ||
				Recipe->Row.strId != "material-recipe-9c82043000d98e73" ||
				Recipe->Row.strRowSha256 !=
					"de782afd3ae8a7975d777be248f08531c0a230ad0cd043397485ce54934bf0e0" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"c0703e1b66140414e212b9eabf9f5c11005d08e8d483c4dead6097703c0ef650" ||
				Recipe->strBindingSha256 !=
					"6bccb5f904eb0558aca1db79ae485053bf027dd25ae35644fe294e30d150953e" ||
				Family->Row.strId != "material-family-d0f9d7bb8b80fee0" ||
				Family->Row.strRowSha256 !=
					"f4f53506aad5cd082f6622730a3186a30d5de348f14ad9808414bc351f0b4085" ||
				Family->strFamilyIdentitySha256 !=
					"165b79044e71cb8d3c58216288d5c8af2c834b79391a8acee9e47630e6eae219" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-f79a91bbcae88cbc" ||
				Family->strEvaluatorSha256 !=
					"ac5dc8467944d9ca8aba49989d220d3613193f15fa5525d47b476ed2d15786f7" ||
				Family->strSampleProjectionSha256 !=
					"993fc439d78e0647f94954257f05a001e27c3877fdf48fbdb3183eacfad4274c" ||
				Family->iFeatureMask != 1023u || Recipe->InputIds.size() != 54u ||
				Recipe->StaticBindingIds.size() != 7u ||
				Recipe->RenderBindingIds.size() != 6u ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ||
				TextureDecision->Texture0Provider.strMaterialInputFieldId !=
					"material-input-67c56cbf42c09219" ||
				TextureDecision->Texture0Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_d_fluid_007.dds" ||
				TextureDecision->Texture1Provider.strMaterialInputFieldId !=
					"material-input-2b80e2c1df640774" ||
				TextureDecision->Texture1Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_a_ice_002.dds")
			{
				strOutError = "Artist F active030 runtime-material identity changed.";
				return false;
			}

			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 54u>
				OrderedInputs{};
			for (size_t iInput = 0u; iInput < Recipe->InputIds.size(); ++iInput)
			{
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->InputIds[iInput];
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row)
					{
						return Row.Row.strId == Recipe->InputIds[iInput];
					}));
				if (It == Program.MaterialInputs.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId)
				{
					strOutError = "Artist F active030 material input join changed.";
					return false;
				}
				OrderedInputs[iInput] = &*It;
			}
			struct EXPECTED_SCALAR final
			{
				size_t iIndex;
				std::string_view strId;
				std::string_view strName;
				double fValue;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_SCALAR, 11u> EXPECTED_SCALARS = {{
				{ 0u, "material-input-ffb4e70a6fa763ca", "36.str", 1.0,
					"f6d059f590cc0479bb1470593b74f98dd9ead504a5ebb26a9973cd7d30d76d91" },
				{ 1u, "material-input-27ffd294f1016a5c", "37.power", 1.0,
					"5b72946bea4e644dc300a3ad920a7bf82a237fc57d7821f7e4232b6e8cf09178" },
				{ 2u, "material-input-a5795205decb0671", "03.map_e_uvscale_r", 2.0,
					"6fbe2bb6615e66d0091145fad867d1f9a929443a303385e8b759a842cb831414" },
				{ 3u, "material-input-c6cf066f364ca877", "04.map_e_uvscale_g", 2.0,
					"6b5c76f97ee769c6a97c2f661012b500e04a4380092a7cb72a176eb4c53c312b" },
				{ 4u, "material-input-29ed0ad8db16f14b", "05.map_e_panning_x",
					-0.019999999552965164,
					"12f90a09b949ca1141962e5fbe4157e926292af46833a29647999944c49d138f" },
				{ 5u, "material-input-b9dc4787ee45f914", "06.map_e_panning_y",
					0.10000000149011612,
					"1d29be1bce9e9c76106f198512d3deabcd3a2dea257c827b658658e02c66748b" },
				{ 8u, "material-input-652f69dfdd80429d", "15.map_f_panning_x",
					0.029999999329447746,
					"d9630127e1e4844d9881b921d8a6d82cc0e8f927c5969c2bc6fcaeade26f9dfe" },
				{ 10u, "material-input-308631746965ed38", "91.desaturation",
					0.8500000238418579,
					"9777e7a01800a92c638ddf2ef3c94aa9384e118ff0e60e8001c35c8e7d220c33" },
				{ 12u, "material-input-57da03040ceebfa6", "01.depthbiasdalpha_bias",
					50.0,
					"3bb6b10dc8d986f33500a935e52e206f35ac19c0e5fad58b3835131c10ce880c" },
				{ 13u, "material-input-f9cc6206108ca7e1", "05.distort_str",
					0.05000000074505806,
					"9af851ed3525ea6799bd2aacbbdeb40327f7b7ad025df7a3e29bd1c4a99a8cf6" },
				{ 32u, "material-input-6833e3787df74240", "33.fresnal_str", 1.0,
					"be8a8c57de7d797103538764ecca917eaa0e84221f712876b22cc9d2bd19adef" }
			}};
			for (const EXPECTED_SCALAR& Expected : EXPECTED_SCALARS)
			{
				const auto* Input = OrderedInputs[Expected.iIndex];
				if (Input->Row.strId != Expected.strId ||
					Input->strNormalizedParameterName != Expected.strName ||
					Input->Row.strRowSha256 != Expected.strRowSha256 ||
					Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!Input->fValue.has_value() || *Input->fValue != Expected.fValue)
				{
					strOutError = "Artist F active030 consumed scalar changed at recipe " +
						std::to_string(Expected.iIndex) + ": " + Input->Row.strId +
						" / " + Input->strNormalizedParameterName + ".";
					return false;
				}
			}
			const auto* ColorInput = OrderedInputs[18u];
			if (ColorInput->Row.strId != "material-input-91991a9c7864cb33" ||
				ColorInput->strNormalizedParameterName != "93.emissiion_color" ||
				ColorInput->Row.strRowSha256 !=
					"6b71f012c233f8c7f319f534ff8db3da28607aa11a78c0af52c09a72e15a06bc" ||
				ColorInput->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
				ColorInput->vValue != std::array<double, 4u>{ 1.0, 1.0, 1.0, 3.0 })
			{
				strOutError = "Artist F active030 consumed vector changed.";
				return false;
			}
			static constexpr std::array<std::array<std::string_view, 4u>, 2u>
				EXPECTED_TEXTURE_INPUTS = {{
					{{ "material-input-67c56cbf42c09219", "21.map_c",
						"fx_tex_02.fx_d_fluid_007",
						"69616d1b79e7f09d65266d79d7a5989eb5afea73feeac23a0e5be5c60aba19a2" }},
					{{ "material-input-2b80e2c1df640774", "02.map_e",
						"fx_tex_00.fx_a_ice_002",
						"257a86ff0383d3ad31927a18266be25f3c5d1def8b268b09512ff505611f7fc2" }}
				}};
			for (size_t iTexture = 0u; iTexture < EXPECTED_TEXTURE_INPUTS.size();
				++iTexture)
			{
				const auto* Input = OrderedInputs[19u + iTexture];
				const auto& Expected = EXPECTED_TEXTURE_INPUTS[iTexture];
				if (Input->Row.strId != Expected[0u] ||
					Input->strNormalizedParameterName != Expected[1u] ||
					Input->strStringValue != Expected[2u] ||
					Input->Row.strRowSha256 != Expected[3u] ||
					Input->eVariant !=
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
				{
					strOutError = "Artist F active030 selected texture changed.";
					return false;
				}
			}

			const auto StoreScalar = [&](const size_t iScalar, const double fValue)
			{
				if (iScalar >= Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u)
					return false;
				f32_t Converted = 0.f;
				if (!AssignFloat(fValue, Converted))
					return false;
				float4_t& Block = Resource.RuntimeMaterialV2ScalarBlocks[iScalar / 4u];
				switch (iScalar % 4u)
				{
				case 0u: Block.x = Converted; break;
				case 1u: Block.y = Converted; break;
				case 2u: Block.z = Converted; break;
				case 3u: Block.w = Converted; break;
				default: return false;
				}
				return true;
			};
			size_t iScalar = 0u;
			size_t iVector = 0u;
			size_t iTexture = 0u;
			for (size_t iInput = 0u; iInput < OrderedInputs.size(); ++iInput)
			{
				const auto* Input = OrderedInputs[iInput];
				const bool_t bVector = std::find(VECTOR_INPUT_INDICES.begin(),
					VECTOR_INPUT_INDICES.end(), iInput) != VECTOR_INPUT_INDICES.end();
				const bool_t bTexture = std::find(TEXTURE_INPUT_INDICES.begin(),
					TEXTURE_INPUT_INDICES.end(), iInput) != TEXTURE_INPUT_INDICES.end();
				if (bVector)
				{
					if (Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 ||
						iVector >= Resource.RuntimeMaterialV2Vectors.size() ||
						!AssignFloat4(Input->vValue,
							Resource.RuntimeMaterialV2Vectors[iVector++]))
					{
						strOutError = "Artist F active030 vector packing changed.";
						return false;
					}
				}
				else if (bTexture)
				{
					if (Input->eVariant !=
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
					{
						strOutError = "Artist F active030 texture packing changed.";
						return false;
					}
					++iTexture;
				}
				else if (Input->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!Input->fValue.has_value() ||
					!StoreScalar(iScalar++, *Input->fValue))
				{
					strOutError = "Artist F active030 scalar packing changed.";
					return false;
				}
			}
			if (iScalar != 43u || iVector != 3u || iTexture != 8u)
			{
				strOutError = "Artist F active030 material denominator changed.";
				return false;
			}

			static constexpr std::array<std::string_view, 7u> STATIC_NAMES = {{
				"31.mapch.r", "32.mapch.g", "00.use_mapa",
				"01.use_emissionmap", "00.use_mapa", "33.mapch.b", "34.mapch.a"
			}};
			static constexpr std::array<bool_t, 7u> STATIC_SELECTED =
				{ true, true, false, true, true, true, true };
			uint32_t iStaticMask = 0u;
			for (size_t i = 0u; i < Recipe->StaticBindingIds.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->StaticBindingIds[i];
					});
				if (It == Program.MaterialStaticBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != STATIC_NAMES[i] ||
					!It->bSelectedValue.has_value() ||
					*It->bSelectedValue != STATIC_SELECTED[i])
				{
					strOutError = "Artist F active030 static input changed.";
					return false;
				}
				if (*It->bSelectedValue)
					iStaticMask |= 1u << static_cast<uint32_t>(i);
			}
			if (iStaticMask != 0x7bu)
			{
				strOutError = "Artist F active030 static selection changed.";
				return false;
			}
			static constexpr std::array<std::string_view, 6u> RENDER_FIELDS = {{
				"blendmode", "lightingmodel", "twosided", "bdisabledepthtest",
				"opacitymaskclipvalue", "buseonelayerdistortion"
			}};
			for (size_t i = 0u; i < Recipe->RenderBindingIds.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[i];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_FIELDS[i])
				{
					strOutError = "Artist F active030 render input changed.";
					return false;
				}
			}

			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"alphadissolve[0-1]", "pan[0-2]", "edgestr[0-x]", "disrotion[0-x]"
			}};
			static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
				"2d89ae435e8345367b2e9d83c30fb6d3be1bb272d9c5e3fa5bd431c6280bf896",
				"bad3b38c4721a18570f39124896d32b0076454bf42b9fd5788918a8042b18b7b",
				"e24c8db3e1f515a67c81ad891aa6709a1125bb59ed67068d3038fec479d9a6aa",
				"ed45eca2d0d8f8586769c5690590528831642560a21ec9b349e6c6545b0d6880"
			}};
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			uint32_t iDynamicCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					DynamicModule = &*It;
					++iDynamicCount;
				}
			}
			bool_t bDynamicContract = iDynamicCount == 1u && DynamicModule != nullptr &&
				DynamicModule->Row.iOrder == 5u &&
				DynamicModule->Row.strRowSha256 ==
					"2d770361ca1ddef3837e4d428103a8926309510158cf5fababc5ed47de92e1ab" &&
				DynamicModule->DistributionIds.size() == 4u;
			for (size_t i = 0u; bDynamicContract && i < 4u; ++i)
			{
				const std::string NamePath = "dynamicparams[" + std::to_string(i) +
					"].paramname";
				const std::string ValuePath = "dynamicparams[" + std::to_string(i) +
					"].paramvalue";
				const auto LiteralIt = std::find_if(Program.Literals.begin(),
					Program.Literals.end(), [&](const auto& Row)
					{
						return Row.strModuleId == DynamicModule->Row.strId &&
							Row.strPropertyPath == NamePath;
					});
				const auto DistributionIt = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == DynamicModule->DistributionIds[i];
					});
				bDynamicContract = LiteralIt != Program.Literals.end() &&
					LiteralIt->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					LiteralIt->strEnumValue == DYNAMIC_NAMES[i] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u &&
					DistributionIt->Row.strRowSha256 == DYNAMIC_ROWS[i];
			}
			if (!bDynamicContract || !Element.SourceRecipe.bEnabled ||
				Element.Material.SourceMaterial.strSubUVMode != "psuvim_random" ||
				SourceLiteralNumber(Element, "subimages_horizontal", -1.f) != 2.f ||
				SourceLiteralNumber(Element, "subimages_vertical", -1.f) != 2.f ||
				SourceLiteralNumber(Element, "randomimagetime", -1.f) != 1.f ||
				!SourceLiteralBool(Element, "ballowimageflipping", false))
			{
				strOutError = "Artist F active030 dynamic/SubUV contract changed.";
				return false;
			}

			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 2u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 43u;
			Resource.iRuntimeMaterialV2VectorCount = 3u;
			Resource.iRuntimeMaterialV2InputCount = 54u;
			Resource.RuntimeMaterialV2InputConsumedMask =
				{ 0x001c353fu, 0x00000001u };
			Resource.RuntimeMaterialV2InputSuppressedMask =
				{ 0xffe3cac0u, 0x003ffffeu };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				{ 0x0fu, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				{ 0u, 0x0fu, 0x0fu };
			Resource.iRuntimeMaterialV2StaticInputCount = 7u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x7bu;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x6bu;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0x14u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
		}
		else if (bActive002019031RuntimeV2)
		{
			struct WIND_SPARKLE_OCCURRENCE_PROFILE final
			{
				uint32_t iOrder;
				std::string_view strEmitterRowSha256;
				std::string_view strOccurrenceId;
				std::string_view strOccurrenceRowSha256;
				std::string_view strOccurrenceIdentitySha256;
				std::string_view strOccurrenceBindingSha256;
				std::string_view strSourceOccurrenceBindingSha256;
				std::string_view strDynamicModuleRowSha256;
				std::array<std::string_view, 4u> DynamicDistributionRows;
				std::string_view strColorModuleRowSha256;
				std::array<std::string_view, 2u> ColorDistributionRows;
				std::string_view strColorScaleModuleRowSha256;
				std::array<std::string_view, 2u> ColorScaleDistributionRows;
			};
			static constexpr std::array<WIND_SPARKLE_OCCURRENCE_PROFILE, 3u>
				WIND_SPARKLE_PROFILES = {{
				{ 2u,
					"8879fb2dd9d2e5e03b0e5308f705d5a63ba7e8b4d4d28f0d2c8714b552bd300c",
					"source-active-002",
					"270c397fbe537acb9821ff34713fb1b4a9bb6622a34ada38e60e2a185c20abc3",
					"193067720d758f8736d08c362296c47f0c598d23c89cac633f7be9c9c9075595",
					"49e2413841a658d01538e587e8a24054e8ab66bf488d85d8b4e3ddae43024e10",
					"f9bde7204a0cbdf922f77000685b53a00620ebf1ddd8faf45b71efc567c96864",
					"332095fe403a365e0f4913fad9d81783798f8f423f0ffeb7f18fa1bbd7a26c84",
					{{ "aa3862acb08b8324e25e11344b1bc1c25eeab1151a1574f2bb6865feeae46940",
						"d01345633faa024b67357a24ddee77866f422717a2211f8a4404679712e8b024",
						"7ac613c13c7c78a496bc676f508eb598fa9f3383bab2c2cca3df8dc40bfd759d",
						"3be08049d2a8d0831a0608730ae44cf319009d8b28fcaa5efbbcc9a8b7e989a9" }},
					"f42ae2557b4acbf8e6ce737e21f68367b5a319c01592a1e2edca085403094ee1",
					{{ "d1571a23d1ad321511600ee481d89318b9eba2d07e9efa27ed975532ebc32439",
						"2ba69d229bab6ac5732a98ba9abdc4cdc85f5dccb4241c37dbc4a371592dfe1d" }},
					"e5c21b8441b6abc92435726799faf3a1422e131f4b5b09d48915f354853848a3",
					{{ "021331801a6bbbff0fe769223cb86b1231194e816966d0db5ed10d126fa15beb",
						"82b9c426b267c57ff7159da67ae1123f34b0e452326e6afba77599b66cf58dbd" }} },
				{ 19u,
					"c979185b5c078b5e986f1322434f90a6eeda0406da5db89961aa0519aa7a5e52",
					"source-active-019",
					"b9d41d8c26c80439fa510e6575b963527fa675c64a1c6d51afe42eab8cf91391",
					"37a159f542ad21d9c4ecafb10fbd8f91fba64e84fc1d80bbc7a38c2c74dfc97e",
					"49e2413841a658d01538e587e8a24054e8ab66bf488d85d8b4e3ddae43024e10",
					"ec2d6ac68a506ca5078657053971ce338435b67b84232e8ae12211e005c0f75e",
					"ff0358e36b255730c7cfab90f95d2f2c1c245bb1e5e675176aeb7c695f7ea117",
					{{ "d09ab834295e1b67ec9f84680dddbd9b991552b0e337d4454680b75101ca7e72",
						"aeddf4c8ea665d2ada4866bdec635a00aa806c2d9c010a5f7e9b544c9f8ee138",
						"9c41e38637347cee8c57636722e205e5938b8b338650b667660b45e85c78e477",
						"7d2106cb907f8a56a8f59ed0287d643db7a006bd1a02753961d2a5ec492bb4a6" }},
					"7d511262660a7490f6b720c7795ba1e13064fb818f25a19ff0e29068f2371f24",
					{{ "c35538b7a9b4d3b33f07c8f0cd4d24fa61027dc90eecfca5c4792c373618c553",
						"5a0e135ce6b836a38da81e4f6c296c31ef14ef75e1bdca6c13d2696aa4e9c543" }},
					"a7a36fd92e0a3a8fcbf173bbc4044867206da9a37ec01d74498abd6b26ec8b8a",
					{{ "26ee87c57a001edc7a28f54004740625f1b458f1795022f9840f418296ee4ef7",
						"58cd79df1956db2682eeacf5f4a68a4a6944d2d4416deb54d44572cfb35fc8bc" }} },
				{ 31u,
					"28af70cd32eca7130e179d3dc1e96954016f598b616c743502083e80efbcb21b",
					"source-active-031",
					"5beb0f7e5cd36360376c17297e253860c38888afcf55bcb5c0e5f29fbd68e686",
					"288f511fb34e5a2ff1697c7d809455585aeae5d7f5bae0e665e7b7d3d740df91",
					"49e2413841a658d01538e587e8a24054e8ab66bf488d85d8b4e3ddae43024e10",
					"27b1731bbdcc921f2aa6300f6abe2ce0a0f1c66f686b450fcd1c7ae27dc44687",
					"5f952db6da8ca1b5a8d90656a16bb7c5d9ce42969c80c713d793ef7ab434934c",
					{{ "35ed171fe67bfe0df6142ca75ca3d3f7e71c8ba51e9fd66fc5d964a72bc31edc",
						"b7b1bbd29bbc54d11f0956c418f71cb056483e80b00853cd3fa4712834f9db12",
						"c6217d69da19db35acdfe1de5fa43943a0afd9e62d8f4a165f9c0c180c8ec027",
						"66e0b3d912c8b3ded65b0d3be5e86caeb0fd5a53cea19c20c79c31d15b608429" }},
					"dfb01c49b44b01ba8a759d0d0b0c49dcc74b3795a58f56e8ed1b02cb2171883d",
					{{ "d49cd750c515a9af50d763db601679615df847f6f8e7f71c3819875388d8a729",
						"52db1d2aec382dc715ab48fc6dbd1f2d14ac558c227d41d67c63d0e9e20d9bed" }},
					"3873af1e16834e9ae65fb5e0b49b0c99b96b8a7003fa1562242a622925b0de13",
					{{ "744ec56af03043ca2ea466d61aeb865b2dc03d2b1da057ac85d90bc35239520c",
						"05c9643f956604286a87007c5fcf0167a2e0a5509fef71dea8fc816a3644d1c0" }} }
			}};
			const auto ProfileIt = std::find_if(WIND_SPARKLE_PROFILES.begin(),
				WIND_SPARKLE_PROFILES.end(), [&](const auto& Row)
				{
					return Row.iOrder == Emitter.Row.iOrder;
				});
			if (ProfileIt == WIND_SPARKLE_PROFILES.end())
			{
				strOutError = "Artist F wind-sparkle occurrence profile is missing.";
				return false;
			}
			const WIND_SPARKLE_OCCURRENCE_PROFILE& WindProfile = *ProfileIt;
			if (Emitter.Row.strRowSha256 != WindProfile.strEmitterRowSha256 ||
				Occurrence->Row.strId != WindProfile.strOccurrenceId ||
				Occurrence->Row.strRowSha256 != WindProfile.strOccurrenceRowSha256 ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					WindProfile.strOccurrenceIdentitySha256 ||
				Occurrence->strBindingSha256 !=
					WindProfile.strOccurrenceBindingSha256 ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					WindProfile.strSourceOccurrenceBindingSha256 ||
				Recipe->Row.strId != "material-recipe-aa19ee0d487380ca" ||
				Recipe->Row.strRowSha256 !=
					"e6351d85b678cd70bbd3e31da996648deba5a71d4334def48e4eb228c9d8fe25" ||
				Family->Row.strId != "material-family-ce6de128d3435d31" ||
				Family->Row.strRowSha256 !=
					"186ab66d08a1a310fe151ea9e9a4e2b6efc9340337b40b01ef08364f974e073c" ||
				Family->iFeatureMask != 101u || Recipe->InputIds.size() != 6u ||
				!Recipe->StaticBindingIds.empty() ||
				Recipe->RenderBindingIds.size() != 6u ||
				TextureDecision->strRecipeTextureDecisionId !=
					"recipe-texture-binding-19" ||
				TextureDecision->strRowSha256 !=
					"885360b2e71bf9b3a5216f9dd4b89d99113fe8d26160ceb873e575489b2b516b" ||
				!TextureDecision->bSecondTextureOperationEnabled ||
				TextureDecision->Texture0Provider.strMaterialInputFieldId !=
					"material-input-281f2c37bdf51c10" ||
				TextureDecision->Texture0Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_d_noise_003.dds" ||
				TextureDecision->Texture1Provider.strMaterialInputFieldId !=
					"material-input-30557ad00debea91" ||
				TextureDecision->Texture1Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_c_noise_002.dds" ||
				Element.Material.strSourceMaterialPath !=
					"fx_m_mi_01.fx_m.fx_f_pa_wind_05_tr" ||
				!Element.Material.SourceMaterial.bEnabled ||
				Element.Material.SourceMaterial.strProfileId != Recipe->Row.strId ||
				Element.Material.SourceMaterial.strParentMaterialPath !=
					Element.Material.strSourceMaterialPath ||
				Element.Material.SourceMaterial.eStatus !=
					EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F wind-sparkle runtime-material identity changed.";
				return false;
			}

			struct EXPECTED_INPUT final
			{
				std::string_view strId;
				std::string_view strName;
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
				double fValue;
				std::string_view strTexture;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_INPUT, 6u> EXPECTED_INPUTS = {{
				{ "material-input-6f32bc4cad5eae49", "sparkle_intensity",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 30.0, {},
					"e8560ca9ea515529695fd662c9f4307b6df0623e383b8feedb1ff239161dbe68" },
				{ "material-input-281f2c37bdf51c10", "sparkle_tex",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0.0,
					"fx_tex_02.fx_d_noise_003",
					"8ddea0cdab437debfdd1412d303ddf42b7e06acf2a12fed750e0996132d71c04" },
				{ "material-input-a7145136127bafec", "sparkle_tiling",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1.5, {},
					"58c63f0991cdb478bdf3241f0a9202b505c6b5d587863959fb6796f5cc7dad39" },
				{ "material-input-2c489800fead4548", "sparkle_tex",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0.0,
					"fx_tex_02.fx_d_noise_003",
					"05852fdef444234c472b21a1f92bd086f210f16eb1487ead54c5525ac9e2ce37" },
				{ "material-input-f1b4a409fb9391c0", "sparkle_paning",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64,
					0.10000000149011612, {},
					"3f03cedf782e74aabb068761b391ea79d99198d967e4fb5aa2177f7583b31aae" },
				{ "material-input-30557ad00debea91", "edgedeco texture01",
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0.0,
					"fx_tex_01.fx_c_noise_002",
					"ece520d813e7226bf8a696b80583e148842f330f556786099e4b9b8561da91c7" }
			}};
			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, 6u>
				OrderedInputs{};
			for (size_t i = 0u; i < EXPECTED_INPUTS.size(); ++i)
			{
				const EXPECTED_INPUT& Expected = EXPECTED_INPUTS[i];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Expected.strId;
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row) { return Row.Row.strId == Expected.strId; }));
				if (Recipe->InputIds[i] != Expected.strId ||
					It == Program.MaterialInputs.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != Expected.strName ||
					It->eVariant != Expected.eVariant ||
					It->Row.strRowSha256 != Expected.strRowSha256 ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
					 (!It->fValue.has_value() || *It->fValue != Expected.fValue)) ||
					(Expected.eVariant ==
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID &&
					 It->strStringValue != Expected.strTexture))
				{
					strOutError = "Artist F wind-sparkle material input changed.";
					return false;
				}
				OrderedInputs[i] = &*It;
			}
			if (TextureDecision->Texture0Provider.strMaterialInputRowSha256 !=
					EXPECTED_INPUTS[1u].strRowSha256 ||
				TextureDecision->Texture1Provider.strMaterialInputRowSha256 !=
					EXPECTED_INPUTS[5u].strRowSha256 ||
				TextureDecision->Texture1Provider.strMaterialInputFieldId ==
					EXPECTED_INPUTS[3u].strId)
			{
				strOutError = "Artist F wind-sparkle provider alias changed.";
				return false;
			}

			static constexpr std::array<std::pair<std::string_view,
				std::string_view>, 6u> EXPECTED_RENDER_ROWS = {{
				{ "blendmode", "4a2d00b7271abbd4442758de477e10e745cda7a038cb4f5b6e3bb9222c361248" },
				{ "lightingmodel", "d9c733a6cb88e19e5e997a6d5971772434cfdd0ed53255932248ed66219c0879" },
				{ "twosided", "5c616e30ec4baf3bf721062a89aa082e6c02e24fc72c9b4528ac80b37ebca7bb" },
				{ "bdisabledepthtest", "466cab2de2b9864a15aa7c57996d182a44aa82200696e28ff59a271a91c24705" },
				{ "opacitymaskclipvalue", "7c97e20e0bd665dbc47db85c1a9870a85a159c7d3150553e0a9bb7a108213854" },
				{ "buseonelayerdistortion", "2fcd311a979d68ca8219c2fe9b8188d8340395bffe6520a4160891562378a689" }
			}};
			for (size_t i = 0u; i < EXPECTED_RENDER_ROWS.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Recipe->RenderBindingIds[i];
					});
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != EXPECTED_RENDER_ROWS[i].first ||
					It->Row.strRowSha256 != EXPECTED_RENDER_ROWS[i].second)
				{
					strOutError = "Artist F wind-sparkle render input changed.";
					return false;
				}
			}

			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			const EFFECT_RUNTIME_PROGRAM_MODULE* ColorModule = nullptr;
			const EFFECT_RUNTIME_PROGRAM_MODULE* ColorScaleModule = nullptr;
			uint32_t iDynamicCount = 0u;
			uint32_t iColorCount = 0u;
			uint32_t iColorScaleCount = 0u;
			uint32_t iSubUvCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(), Program.Modules.end(),
					[&](const auto& Row) { return Row.Row.strId == ModuleId; });
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					DynamicModule = &*It;
					++iDynamicCount;
				}
				else if (It->strExactSourceClass == "particlemodulecolor")
				{
					ColorModule = &*It;
					++iColorCount;
				}
				else if (It->strExactSourceClass ==
					"particlemodulecolorscaleoverlife")
				{
					ColorScaleModule = &*It;
					++iColorScaleCount;
				}
				else if (It->strExactSourceClass == "particlemodulesubuv")
				{
					++iSubUvCount;
				}
			}
			static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
				"transition_time (0~1)", "alpha_power(1~)",
				"uv_noise_inetnsity(0~2)", "smokepanningspeed"
			}};
			bool_t bDynamicContract = iDynamicCount == 1u && DynamicModule != nullptr &&
				DynamicModule->Row.iOrder == 8u &&
				DynamicModule->Row.strRowSha256 ==
					WindProfile.strDynamicModuleRowSha256 &&
				DynamicModule->DistributionIds.size() == 4u;
			for (size_t i = 0u; bDynamicContract && i < 4u; ++i)
			{
				const std::string NamePath = "dynamicparams[" + std::to_string(i) +
					"].paramname";
				const std::string ValuePath = "dynamicparams[" + std::to_string(i) +
					"].paramvalue";
				const auto LiteralIt = std::find_if(Program.Literals.begin(),
					Program.Literals.end(), [&](const auto& Row)
					{
						return Row.strModuleId == DynamicModule->Row.strId &&
							Row.strPropertyPath == NamePath;
					});
				const auto DistributionIt = std::find_if(Program.Distributions.begin(),
					Program.Distributions.end(), [&](const auto& Row)
					{
						return Row.Row.strId == DynamicModule->DistributionIds[i];
					});
				bDynamicContract = LiteralIt != Program.Literals.end() &&
					LiteralIt->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					LiteralIt->strEnumValue == DYNAMIC_NAMES[i] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u &&
					DistributionIt->Row.strRowSha256 ==
						WindProfile.DynamicDistributionRows[i];
			}
			const auto HasDistribution = [&](const EFFECT_RUNTIME_PROGRAM_MODULE* Module,
				const std::string_view strProperty, const std::string_view strRowSha)
			{
				return nullptr != Module && std::any_of(Module->DistributionIds.begin(),
					Module->DistributionIds.end(), [&](const std::string& Id)
					{
						const auto It = std::find_if(Program.Distributions.begin(),
							Program.Distributions.end(), [&](const auto& Row)
							{
								return Row.Row.strId == Id;
							});
						return It != Program.Distributions.end() &&
							It->strPropertyPath == strProperty &&
							It->Row.strRowSha256 == strRowSha;
					});
			};
			if (!bDynamicContract || iColorCount != 1u || iColorScaleCount != 1u ||
				iSubUvCount != 0u || nullptr == ColorModule ||
				ColorModule->Row.iOrder != 5u ||
				ColorModule->Row.strRowSha256 !=
					WindProfile.strColorModuleRowSha256 ||
				nullptr == ColorScaleModule || ColorScaleModule->Row.iOrder != 6u ||
				ColorScaleModule->Row.strRowSha256 !=
					WindProfile.strColorScaleModuleRowSha256 ||
				!HasDistribution(ColorModule, "startcolor",
					WindProfile.ColorDistributionRows[0u]) ||
				!HasDistribution(ColorModule, "startalpha",
					WindProfile.ColorDistributionRows[1u]) ||
				!HasDistribution(ColorScaleModule, "colorscaleoverlife",
					WindProfile.ColorScaleDistributionRows[0u]) ||
				!HasDistribution(ColorScaleModule, "alphascaleoverlife",
					WindProfile.ColorScaleDistributionRows[1u]) ||
				Element.Material.SourceMaterial.strSubUVMode != "none")
			{
				strOutError =
					"Artist F wind-sparkle dynamic/color/SubUV contract changed.";
				return false;
			}

			if (!AssignFloat(*OrderedInputs[0u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].x) ||
				!AssignFloat(*OrderedInputs[2u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].y) ||
				!AssignFloat(*OrderedInputs[4u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].z))
			{
				strOutError = "Artist F wind-sparkle scalar packing changed.";
				return false;
			}
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 3u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 3u;
			Resource.iRuntimeMaterialV2VectorCount = 0u;
			Resource.iRuntimeMaterialV2InputCount = 6u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x37u, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x08u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask = { 0u, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask = { 0u, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 0u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0u;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0u;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
		}
		else if (bActive020021RuntimeV2)
		{
			/* The recovered LocalDecal cache program is preserved as evidence, but
			   this packet is an RT0 semantic replay on the existing typed projector.
			   Raw FLocalDecal VF/pass admission and auxiliary MRT2-5 remain false. */
			struct LOCAL_DECAL_OCCURRENCE final
			{
				uint32_t iOrder;
				std::string_view strOccurrenceId;
				std::string_view strOccurrenceRowSha256;
				std::string_view strOccurrenceIdentitySha256;
				std::string_view strSourceOccurrenceBindingSha256;
				std::string_view strSelectedLodPath;
				std::string_view strSelectedLodNodeId;
				std::string_view strSelectedLodRecordSha256;
			};
			static constexpr std::array<LOCAL_DECAL_OCCURRENCE, 2u>
				OCCURRENCES = {{
				{ 20u, "source-active-020",
					"30143cce534eef424d4f81479dab7e105a0fda43daf45a68c225d134a35c8de6",
					"5ccef74976494ebff9e93de159111f50866fbb056a148aee97e1c10934bcbb22",
					"84018bdf9c371827ac2c961e17695acfcf717aa9402754f93dd3f410eebab287",
					"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_43.particlelodlevel_2",
					"FX_PC_SDM_07:export:273",
					"9222d5e3982d6ec4e9e0ba9a73cd5ef6bbfe8aa5ccca6759ad42d5b41e47a3b4" },
				{ 21u, "source-active-021",
					"9be6e11b349509b005e6fde88800ecf5ddcf554a48b6da289619c5781e01fbdf",
					"b4f98fc99562c9a253c278fd3f0b9240f1631076f1df28421bbe429c7e37079b",
					"ed09a09912f154b74fc0ded834d33246a075e3f7e6d99b4055aaa9ab9cbacf8b",
					"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_6.particlelodlevel_10",
					"FX_PC_SDM_07:export:274",
					"2bdf32242995bab243ca1bf9b87b4a922e9624705e93ba8eacf15f2b60e2986a" }
			}};
			const auto ExpectedOccurrence = std::find_if(OCCURRENCES.begin(),
				OCCURRENCES.end(), [&](const LOCAL_DECAL_OCCURRENCE& Row)
				{ return Row.iOrder == Emitter.Row.iOrder; });
			if (ExpectedOccurrence == OCCURRENCES.end() ||
				Occurrence->Row.strId != ExpectedOccurrence->strOccurrenceId ||
				Occurrence->Row.strRowSha256 !=
					ExpectedOccurrence->strOccurrenceRowSha256 ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					ExpectedOccurrence->strOccurrenceIdentitySha256 ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					ExpectedOccurrence->strSourceOccurrenceBindingSha256 ||
				Emitter.strSelectedLodPath != ExpectedOccurrence->strSelectedLodPath ||
				Emitter.strSelectedLodNodeId != ExpectedOccurrence->strSelectedLodNodeId ||
				Emitter.strSelectedLodRecordSha256 !=
					ExpectedOccurrence->strSelectedLodRecordSha256 ||
				Recipe->Row.strId != "material-recipe-a0a4fd34c2f220dc" ||
				Recipe->Row.strRowSha256 !=
					"3914fa0fcfd1b7ab46cd7699974dc8d10f105eb87b8c0b14b09cdd2d6bc5833b" ||
				Recipe->strSourceMaterialPath !=
					"fx_m_mi_w_00.mi.fx_w_de_master_01_77_tr" ||
				Recipe->strSourceRecipeCompositionSha256 !=
					"a090c5fca9d8694ab42c600031d156307378b2a7e81ef67254a8868a4f7e4283" ||
				Recipe->strBindingSha256 !=
					"e506813b11f6ff9114f92e570c5849fe6ba8b1f3899f97b1ba283b708520c817" ||
				Occurrence->strBindingSha256 != Recipe->strBindingSha256 ||
				Family->Row.strId != "material-family-f1667adae7da4bdd" ||
				Family->Row.strRowSha256 !=
					"211b9300e10273f2321f6988ac14f8c74d0c2f4f7582d77a959803d313c4d628" ||
				Family->strFamilyIdentitySha256 !=
					"a9c9dc5d1bb54808a0c67f1ddcb750050234f5a7204512da9161571921d4ab54" ||
				Family->strEvaluatorRegistryId !=
					"handler-975a0a15187dec7f4e29565b" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-83456bfcd511b48b" ||
				Family->strEvaluatorSha256 !=
					"1c2020c8db49a87262434f59b18432d4f0f8767f7cddff5476e5122904e5a5e7" ||
				Family->strSampleProjectionSha256 !=
					"9b217834ccdd6ef075b5bf6f42bcc8d1cfd63a1f72827285802200ed0428e3d8" ||
				Family->iFeatureMask != 639u ||
				Recipe->InputIds.size() != 33u ||
				Recipe->StaticBindingIds.size() != 18u ||
				Recipe->RenderBindingIds.size() != 6u ||
				Element.eKind != EFFECT_ELEMENT_KIND::DECAL ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ)
			{
				strOutError = "Artist F LocalDecal occurrence/material identity changed.";
				return false;
			}

			struct INPUT_CONTRACT final
			{
				std::string_view strId;
				std::string_view strName;
				std::string_view strRowSha256;
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT eVariant;
				double fScalar;
				std::array<double, 4u> vVector;
				std::string_view strTextureId;
			};
			static constexpr std::array<INPUT_CONTRACT, 33u> INPUTS = {{
				{ "material-input-10981a097e141c45", "09.str", "1c19f8334fdf0248eb9a6fd8706ce86df904ac99a4f4481968770f5737ea116d", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 5., {}, {} },
				{ "material-input-aea70d7fc10cf484", "10.power", "920b4357ad2adf9dfaaeb36b6fa97c4292c9a09fe391b3b7bb2855f4d8c983d2", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-b18befc81ff93873", "31.dissolvemapscale", "697330c668ddd1caa12c1754f8aaf45c59ce8beefc621d6c78747dc7ce6d98de", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-84aade01c3f54e0d", "02.diffmap_a.uvscale", "173354247014e0a5ed3b014af095c431b2f3de587fe4f0895e8b87185b922667", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-e6014f8b1afc92d0", "81.diffusedesat", "628e48e22e16c8a8d4791493d65027062cc22247f0f36db8e766cf7fdc5b4959", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-511f3e13a9a8d375", "03.emap_uv.x.scale", "2fa07a8d109eca35ce709de97d34bab5d9dd44f06b1961c621ce372971bee8c6", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-a9558fd3fc5cc5aa", "04.emap_uv.y.scale", "02d2f9a13aaf732cbecbe7a178107e0cd1409c264752c0e3d7a20a068cb88d84", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-921ed67e92936281", "15.emissiion_power", "0d4a2f8f4d3023a83cf1284183b0a008da7d54be3c57bce325c8ecb065f1a4ef", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, .800000011920929, {}, {} },
				{ "material-input-6f6142b697bcfe6c", "21.phasepantimescale", "36427eeab337b08a74aafc30d9d2285eb6379c3821ed0ce44eca8ac7150f4d36", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, .019999999552965164, {}, {} },
				{ "material-input-867ac55d884ab650", "02.specmap_str", "a7a15619d9110d30801b54f67c51d051999cc1e58c4a18bc76b0687a322ce3f7", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 0., {}, {} },
				{ "material-input-d9e67c06d3d273ac", "05.specmap_uvscale.x", "f9c17173afb1e03ca9ab10765b647d3f53853e5641e31457874ab8984a3e0a87", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-97bbf83624cb870e", "06.specmap_uvscale.y", "b1c004d42e05266b81aa925fc3fc60031b939fa2d8006ab2db4ac09bf9dc2a82", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, .699999988079071, {}, {} },
				{ "material-input-b6d8b36505e81421", "07.desaturation", "65fb3d7cdfaf2b7bdd5b2daae09860556284280aa607bbc007639895610c2d8e", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, .8999999761581421, {}, {} },
				{ "material-input-0ee2db8b2c948f6b", "08.specmap_power", "489e8643ce3cd8ae2add3a10eb5dca50de92416f834dc32f2ee863b2ac9ca462", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 9., {}, {} },
				{ "material-input-bc1ff6137ba91b24", "82.diffusecolor", "bc5f49366c3daf29c4760e9f402bef378846db01d806aea1dc34e515e876d9cc", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { .20000000298023224, .20000000298023224, .20000000298023224, .30000001192092896 }, {} },
				{ "material-input-a4f17cb5ff24a2b7", "19.emissiion_color", "69c604d20d69fe84708ced29137c5e117d6230c000c8d0cce800f54a32ebd2b4", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { .5, .5, .5, 1. }, {} },
				{ "material-input-5fe13e69e19fc907", "09.specmap_color", "c6b335b167501674c13e58ca48128c29746cee69914ae6b2c326bbdb4d958856", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4, 0., { 1., 1., 1., .800000011920929 }, {} },
				{ "material-input-2801b1f3bd6d41ce", "01.diffmap_a", "2efe05f369c9e3f14b98260519b4a3e320b616de062652ba31bccc2b32f1fd21", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_00.fx_b_fluid_007" },
				{ "material-input-7910dd7ab378bbec", "01.emismap", "660f31b59352a5c9809a4761a81443508bf8e27fd27d137f40cc3d89ecab9669", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_00.fx_bg_dustpanner_01" },
				{ "material-input-ea6ccea9bb8eb73b", "06.normalmap", "2a4b992b53f3ee16b4a08a09a4572f4e42ab324683df6bd43b77c8422c5fb6bd", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_02.fx_d_normal_078" },
				{ "material-input-6d770743fc5578fc", "01.specmap", "f4839e6ab22eec1b6fae2b3ed8307f35b8ec777e14bdd1fb1a43fc543f796f3c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_02.fx_d_environ_018" },
				{ "material-input-bc1a5991f0567938", "02.bumpheight", "927a638a030bdebb44e3bef1a3647fe457fcb0ffb59a5523297f2061a40ad153", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1.5, {}, {} },
				{ "material-input-add35c19fa3753e8", "01.heightmap", "aa842d0777735a88b95a0ab97608b55876b3ebf367473a33c6f173c0ef3effbb", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_01.fx_c_decal_002_2" },
				{ "material-input-bb86a1dadcb5ed56", "01.lightdirection", "d6934b566f58a04376a728845439c93a03bd78f997bf5c9b81c5b56e38490554", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-481752c8cce400b5", "21.maskmap", "e096e88fe33b3ac49f25ffd55ee0285696dc9179ed4015484918a44ca948eead", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_01.fx_c_decal_002_2" },
				{ "material-input-f3eab86bb09ad91a", "22.maskmap.uvscale", "cd090333f69a2d80bbd0e68a17ed7e68182856c3a24d748045d142a20bcc6986", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-bd8b4383de296075", "11.diffmap_b", "9621095d2423dbf0de389dc2c5622b87c7a8c85ec8ac30655aabe6dc02e67f23", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_03.fx_e_cloud_008" },
				{ "material-input-74e91630bb0e2b50", "12.diffmap_a.uvscale", "64e1a31210df2dec9bf30622880b93246d7f669a65a05b78bfa0896700f9d906", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-06ec5ae9996cf133", "01.alphamaskmap", "16e23ad7cf38c2a1ba15e2d0994a44ade3de84029b42ca2ca2ee035eea74418c", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID, 0., {}, "fx_tex_00.fx_a_hit_004" },
				{ "material-input-edc4fcfce0a32737", "25.maskmap.str", "7a6c1e8e23ad3d18aa0a9e9cbcda8a5010767b6f48679355d33da952d6c04a59", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {}, {} },
				{ "material-input-cbfc5329067ff679", "24.maskmap.power", "968ebe223e1e4482eae4e36b8791b05e4a561a7202d2ca13474f84f9f90cc759", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 2., {}, {} },
				{ "material-input-eb565cf75e183ffb", "08.normalmap_intensity", "4d8e0c6b26b13a3d6045f30c6da334afe947ce15ae109ef8dcd7a100deab394e", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} },
				{ "material-input-38c52a2136f900f5", "07.normalmap_uvscale", "a86b8d0ca0a057cb1157e231900abefd7eddb5f7bfcc8c23127fb8503044f849", EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64, 1., {}, {} }
			}};
			std::array<const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE*, INPUTS.size()>
				OrderedInputs{};
			for (size_t i = 0u; i < INPUTS.size(); ++i)
			{
				const INPUT_CONTRACT& Expected = INPUTS[i];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{ return Row.Row.strId == Expected.strId; });
				const auto EqualsVector = [&](const std::array<double, 4u>& Value)
				{
					return Value == Expected.vVector;
				};
				if (Recipe->InputIds[i] != Expected.strId ||
					It == Program.MaterialInputs.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strNormalizedParameterName != Expected.strName ||
					It->Row.strRowSha256 != Expected.strRowSha256 ||
					It->eVariant != Expected.eVariant ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
						(!It->fValue.has_value() || *It->fValue != Expected.fScalar)) ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4 &&
						!EqualsVector(It->vValue)) ||
					(Expected.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID &&
						It->strStringValue != Expected.strTextureId) ||
					It->strTypedValueSha256.empty() ||
					It->strSourceFieldValueSha256.empty() ||
					It->strSourceLineageSha256.empty())
				{
					strOutError = "Artist F LocalDecal material input changed at index " +
						std::to_string(i) + ".";
					return false;
				}
				OrderedInputs[i] = &*It;
			}

			uint32_t iSelectedStaticMask = 0u;
			for (size_t i = 0u; i < Recipe->StaticBindingIds.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{ return Row.Row.strId == Recipe->StaticBindingIds[i]; });
				if (It == Program.MaterialStaticBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
					!It->bSourceValue.has_value() || !*It->bSourceValue ||
					!It->bSelectedValue.has_value() ||
					It->strPolicyRowId.empty() || It->strSourceLineageSha256.empty())
				{
					strOutError = "Artist F LocalDecal static-set projection changed.";
					return false;
				}
				if (*It->bSelectedValue)
					iSelectedStaticMask |= 1u << static_cast<uint32_t>(i);
			}
			static constexpr std::array<std::pair<std::string_view,
				std::string_view>, 6u> RENDER_ROWS = {{
				{ "blendmode", "741deaf3a19084ebf737f7c728fe0530465efdc478bf52d0f88c7c8f95aaf872" },
				{ "lightingmodel", "68b5ec6a19147d6e75666db84429196ac24cbafd02c2ad0c6e611e0fc15740a9" },
				{ "twosided", "9d34b1e88255a9f6f21c12b9616fdbbe56a4b89880231199c02a1ebe5fa6abd3" },
				{ "bdisabledepthtest", "322498f2791bca6ac25b8d8c79b61850c61ee3781137c1f4160e26aaeb8199f2" },
				{ "opacitymaskclipvalue", "7382bd0bb542fcb39d0bcf296d8e5518725f17fd8e525d9fdb95f0d62c52e53c" },
				{ "buseonelayerdistortion", "49cbf2168a51065cc360c2a5af22b0bf50c753941039210a9a4789abad405744" }
			}};
			for (size_t i = 0u; i < RENDER_ROWS.size(); ++i)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{ return Row.Row.strId == Recipe->RenderBindingIds[i]; });
				if (It == Program.MaterialRenderBindings.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != RENDER_ROWS[i].first ||
					It->Row.strRowSha256 != RENDER_ROWS[i].second)
				{
					strOutError = "Artist F LocalDecal render-state projection changed.";
					return false;
				}
			}
			if (iSelectedStaticMask != 0x3fffbu)
			{
				strOutError = "Artist F LocalDecal selected static mask changed.";
				return false;
			}

			const auto StoreLocalDecalScalar = [&](const size_t iScalar,
				const double fValue)
			{
				if (iScalar >= Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u)
					return false;
				f32_t Converted = 0.f;
				if (!AssignFloat(fValue, Converted))
					return false;
				float4_t& Block =
					Resource.RuntimeMaterialV2ScalarBlocks[iScalar / 4u];
				switch (iScalar % 4u)
				{
				case 0u: Block.x = Converted; break;
				case 1u: Block.y = Converted; break;
				case 2u: Block.z = Converted; break;
				case 3u: Block.w = Converted; break;
				default: return false;
				}
				return true;
			};
			size_t iScalar = 0u;
			size_t iVector = 0u;
			for (const auto* Input : OrderedInputs)
			{
				if (Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64)
				{
					if (iScalar >= 22u || !Input->fValue.has_value() ||
						!StoreLocalDecalScalar(iScalar, *Input->fValue))
					{
						return false;
					}
					++iScalar;
				}
				else if (Input->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4)
				{
					if (iVector >= Resource.RuntimeMaterialV2Vectors.size() ||
						!AssignFloat4(Input->vValue,
							Resource.RuntimeMaterialV2Vectors[iVector]))
					{
						return false;
					}
					++iVector;
				}
			}
			if (iScalar != 22u || iVector != 3u)
				return false;
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0fu;
			Resource.iRuntimeMaterialV2ParticleColorPolicy = 0u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask = 0u;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2ScalarCount = 22u;
			Resource.iRuntimeMaterialV2VectorCount = 3u;
			Resource.iRuntimeMaterialV2InputCount = 33u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x820ec1ffu, 0x1u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x7df13e00u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask = { 0x0fu, 0x0fu, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask = { 0u, 0u, 0x0fu };
			Resource.iRuntimeMaterialV2StaticInputCount = 18u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = iSelectedStaticMask;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x3ffffu;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x03u;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x3cu;
		}
		else if (bActive003RuntimeV2 || bActive011RuntimeV2 ||
			bActive022RuntimeV2)
		{
			struct RUNTIME_V2_PROFILE final
			{
				std::string_view strEmitterRowSha256;
				std::string_view strOccurrenceId;
				std::string_view strOccurrenceRowSha256;
				std::string_view strOccurrenceIdentitySha256;
				std::string_view strOccurrenceBindingSha256;
				std::string_view strSourceOccurrenceBindingSha256;
				std::string_view strRecipeId;
				std::string_view strRecipeRowSha256;
				std::string_view strRecipeCompositionSha256;
				std::string_view strFamilyId;
				std::string_view strFamilyRowSha256;
				std::string_view strFamilyIdentitySha256;
				std::string_view strEvaluatorRegistryId;
				std::string_view strEvaluatorId;
				std::string_view strEvaluatorSha256;
				std::string_view strSampleProjectionSha256;
				std::string_view strTextureDecisionId;
				std::string_view strTextureDecisionRowSha256;
				uint32_t iTextureDecisionOrder;
				uint32_t iFeatureMask;
				uint32_t iInputCount;
				uint32_t iScalarCount;
				uint32_t iVectorCount;
				uint32_t iTextureCount;
				uint32_t iStaticInputCount;
				uint32_t iStaticSelectedMask;
				uint32_t iStaticConsumedMask;
				uint32_t iStaticSuppressedMask;
				uint32_t iDynamicConsumedMask;
				uint32_t iDynamicSuppressedMask;
				std::array<uint32_t, 2u> InputConsumedMask;
				std::array<uint32_t, 2u> InputSuppressedMask;
				std::array<uint32_t, 3u> VectorConsumedMask;
				std::array<uint32_t, 3u> VectorSuppressedMask;
				EFFECT_ELEMENT_KIND eElementKind;
			};

			const RUNTIME_V2_PROFILE Profile = bActive003RuntimeV2 ?
				RUNTIME_V2_PROFILE{
					"3cf36d8cbb5d065246f285198974019bd79b9c08543fa7a4ab37f171d149e8fd",
					"source-active-003",
					"2bfd8fc8c45c160af1463273fa1762a9898a826f1747fa738422d466a3c5847b",
					"53aa5fa45c19cb855ba20300cee3487e873f8f71bd073317f5a5c376164c5227",
					"3ba0b8db3439e2b3c5cc2f893ed14d5f62541048a6b43aa3e5c8ea8608dc36c9",
					"d5682f821777984425a245fce49c898b748b796249db5b4c0dabb33fad552a39",
					"material-recipe-b508403ea55fedbc",
					"22c1c50d02782d4a8b1dfd2025492a6d65511db9b7daf48c4227f6540297a647",
					"449ef156aaaef5f6e61c2847d87acdbcfec4501383994058ac2b3cd10dc9875b",
					"material-family-918f4ae1ed4d8a70",
					"d688cc650f2d605434ce60c2fb3a07fe15e83bf2508121df408a5133d7c5cf8d",
					"8761298193b6fb25b11aa9d879142ebec1cc9751061879651b5a70744fe4504f",
					"handler-787ff99444497496a9b0cc39",
					"reconstructed-evaluator-d80c59d539697424",
					"e1cdf7f3333e840d4e02294af0618fc652f1e0f93e88a9852fcb33523252f8db",
					"54d9766386d4e9fe0a86e756719b2bd178eb7a9f0c5aa6a925655642ad56f4f4",
					"recipe-texture-binding-20",
					"07f45d86c50b8801d75d0aeeaf7cbe55291c890fffdc77c152a3062ab8f59f01",
					20u, 303u, 17u, 14u, 1u, 2u, 1u, 0x01u, 0x01u, 0u,
					0u, 0x0fu, { 0x00010481u, 0u }, { 0x0000fb7eu, 0u },
					{ 0x0fu, 0u, 0u }, { 0u, 0u, 0u },
					EFFECT_ELEMENT_KIND::TRAIL } :
				(bActive011RuntimeV2 ? RUNTIME_V2_PROFILE{
					"99416894217b2799e4fb05d6e2e7e0f2fd9e0aea4ea1d67952792d237d4bde4b",
					"source-active-011",
					"a896dbee74f6efaca7bcdc0743c0a45c5b774d9beda62c2ca92a39fa1e28c480",
					"7abd07208ddb595b2ca42c3163aeb31eeb93eb67aa770af500fafac78d7b0f10",
					"ef5b7ffc5f0215db38b8ccc207fda999bd03aee78ea9c8670819a5e6756412a8",
					"e46cc028ac3f6f9efec5517a7eef581304a6193aead8cad9ffb14e949ce5744a",
					"material-recipe-daf220acad2b656e",
					"8ec0ce103cbdb430761c8b0a754216bf0099746a7270cb0ebf259860742b4e98",
					"82b40e9f08349825c23cf69ea438d655208a4a9eb3244e123536307c144d420c",
					"material-family-097bd8d9597721b5",
					"9f61d7daa576164fe750ae77fc016a83cd0a022f8eb9e34ebd819ab789b02998",
					"5d4d3e2794d71d05711e1f8063f91b1481de57a0a690724e8e1b1bbcd21b9ddf",
					"handler-bfc182ce8f82a998d4a7aff4",
					"reconstructed-evaluator-c93ae4110be795bf",
					"0a31970a37d9bd7f0d050b5780ec622ed0693152d7c1ca8a4f2b0e21ad5e43d0",
					"3b4945ddf3fc004fa76eafbec7db5b9b2b9c273ed5e8f71e78d02a8f60769d49",
					"recipe-texture-binding-23",
					"64a91e38ad91498f2affcc572969adeb61b85f9d0ad3500db0ae1215f72f9e57",
					23u, 1007u, 55u, 47u, 1u, 7u, 9u, 0x1efu, 0x1ffu,
					0u, 0x07u, 0x08u, { 0xffffffffu, 0x0000007fu },
					{ 0u, 0x007fff80u }, { 0x07u, 0u, 0u }, { 0x08u, 0u, 0u },
					EFFECT_ELEMENT_KIND::PARTICLE } : RUNTIME_V2_PROFILE{
					"f272a1f511f153c22728399e8d3ab20aabea5ef86c8e9bd573ff4ec230379f00",
					"source-active-022",
					"15a835ef02c26ce2a1f5df44873ccc2b5c0a27c6964c835b46b3407b142f3533",
					"372e4a3da2464f7676ec2f8b55c7b147af0c58827d2ea93dec0777d2385eb2e1",
					"8f9b0f5d331117603b29d210927d8571e566f3754592c52ff21ea1542027a43d",
					"ab2e718e172e866f2c41af886c6c4a22e6ca9ebf5e560038ddc9982e8d9edd69",
					"material-recipe-99ec58031edc07b9",
					"8cd1bbc2e3b1f8719d102660048742e09134ff9e2cfcb3c6670a151930d9fbc7",
					"c8bd5e74186625f2d3ca9820032e131bfeed30291c814877295cfc029dcee37f",
					"material-family-472b1be487b92e70",
					"20d677ad6f34ec894e028a93e3e42f56a5200912822f4f90e6231d073de2d7bf",
					"556cb5a7ab50b6cdae8297669f8324603c487700520b382d3775f7c030634991",
					"handler-fc026c025fc8843a2b7f6249",
					"reconstructed-evaluator-354b34c7676181d5",
					"e5e735912abb6e97589ca071718d1136d3460668126a4b0c806dd21ba02c064a",
					"9b879f154998a4f54f80f47484d57b38154fdbdec3f2c895016dc0c01666a830",
					"recipe-texture-binding-14",
					"a8ffc952e337c88b71db1aa6c3f09e637cf468fb007ea29fef71608b67ab15fe",
					14u, 895u, 29u, 21u, 2u, 6u, 0u, 0u, 0u, 0u,
					0u, 0x0fu, { 0x00001f87u, 0u }, { 0x1fffe078u, 0u },
					{ 0x0fu, 0u, 0u }, { 0u, 0x0fu, 0u },
					EFFECT_ELEMENT_KIND::DECAL });

			if (Emitter.Row.strRowSha256 != Profile.strEmitterRowSha256 ||
				Element.eKind != Profile.eElementKind ||
				Occurrence->Row.strId != Profile.strOccurrenceId ||
				Occurrence->Row.strRowSha256 != Profile.strOccurrenceRowSha256 ||
				Occurrence->strSourceOccurrenceIdentitySha256 !=
					Profile.strOccurrenceIdentitySha256 ||
				Occurrence->strBindingSha256 != Profile.strOccurrenceBindingSha256 ||
				Occurrence->strSourceOccurrenceBindingSha256 !=
					Profile.strSourceOccurrenceBindingSha256 ||
				Recipe->Row.strId != Profile.strRecipeId ||
				Recipe->Row.strRowSha256 != Profile.strRecipeRowSha256 ||
				Recipe->strSourceRecipeCompositionSha256 !=
					Profile.strRecipeCompositionSha256 ||
				Recipe->strBindingSha256 != Profile.strOccurrenceBindingSha256 ||
				Family->Row.strId != Profile.strFamilyId ||
				Family->Row.strRowSha256 != Profile.strFamilyRowSha256 ||
				Family->strFamilyIdentitySha256 != Profile.strFamilyIdentitySha256 ||
				Family->strEvaluatorRegistryId != Profile.strEvaluatorRegistryId ||
				Family->strEvaluatorId != Profile.strEvaluatorId ||
				Family->iEvaluatorVersion != 1u ||
				Family->strEvaluatorSha256 != Profile.strEvaluatorSha256 ||
				Family->strSampleProjectionSha256 !=
					Profile.strSampleProjectionSha256 ||
				Family->iFeatureMask != Profile.iFeatureMask ||
				Recipe->InputIds.size() != Profile.iInputCount ||
				Recipe->StaticBindingIds.size() != Profile.iStaticInputCount ||
				Recipe->RenderBindingIds.size() != 6u ||
				TextureDecision->strRecipeTextureDecisionId !=
					Profile.strTextureDecisionId ||
				TextureDecision->strRowSha256 !=
					Profile.strTextureDecisionRowSha256 ||
				TextureDecision->iOrder != Profile.iTextureDecisionOrder ||
				!TextureDecision->bSecondTextureOperationEnabled ||
				!TextureDecision->bDistortionOperationEnabled ||
				(bActive011RuntimeV2 && Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ) ||
				TextureDecision->fDistortionStrength !=
					(bActive022RuntimeV2 ? 1.f : 0.f))
			{
				strOutError = "Artist F runtime-material v2 identity changed: " +
					Element.strElementId;
				return false;
			}

			uint32_t iStaticSelectedMask = 0u;
			for (size_t i = 0u; i < Recipe->StaticBindingIds.size(); ++i)
			{
				const std::string& Id = Recipe->StaticBindingIds[i];
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					}));
				if (It == Program.MaterialStaticBindings.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId ||
					It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN ||
					!It->bSelectedValue.has_value())
				{
					strOutError = "Artist F runtime-material static join changed.";
					return false;
				}
				if (*It->bSelectedValue)
					iStaticSelectedMask |= 1u << static_cast<uint32_t>(i);
			}
			static constexpr std::array<std::string_view, 6u>
				EXPECTED_RENDER_FIELDS = {{
					"blendmode", "lightingmodel", "twosided",
					"bdisabledepthtest", "opacitymaskclipvalue",
					"buseonelayerdistortion"
				}};
			for (size_t i = 0u; i < EXPECTED_RENDER_FIELDS.size(); ++i)
			{
				const std::string& Id = Recipe->RenderBindingIds[i];
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					}));
				if (It == Program.MaterialRenderBindings.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId ||
					It->strFieldName != EXPECTED_RENDER_FIELDS[i])
				{
					strOutError = "Artist F runtime-material render join changed.";
					return false;
				}
			}
			if (iStaticSelectedMask != Profile.iStaticSelectedMask)
			{
				strOutError = "Artist F runtime-material static selection changed.";
				return false;
			}

			size_t iScalar = 0u;
			size_t iVector = 0u;
			size_t iTexture = 0u;
			for (const std::string& InputId : Recipe->InputIds)
			{
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == InputId;
					});
				const size_t iMatches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row) { return Row.Row.strId == InputId; }));
				if (It == Program.MaterialInputs.end() || iMatches != 1u ||
					It->strRecipeId != Recipe->Row.strId)
				{
					strOutError = "Artist F runtime-material input join changed.";
					return false;
				}
				if (It->eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64)
				{
					if (!It->fValue.has_value() || iScalar >=
						Resource.RuntimeMaterialV2ScalarBlocks.size() * 4u)
					{
						return false;
					}
					f32_t* Values = &Resource.RuntimeMaterialV2ScalarBlocks[
						iScalar / 4u].x;
					if (!AssignFloat(*It->fValue, Values[iScalar % 4u]))
						return false;
					++iScalar;
				}
				else if (It->eVariant ==
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4)
				{
					if (iVector >= Resource.RuntimeMaterialV2Vectors.size() ||
						!AssignFloat4(It->vValue,
							Resource.RuntimeMaterialV2Vectors[iVector]))
					{
						return false;
					}
					++iVector;
				}
				else if (It->eVariant ==
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
				{
					if (It->strStringValue.empty())
						return false;
					++iTexture;
				}
				else
				{
					strOutError = "Artist F runtime-material input variant changed.";
					return false;
				}
			}
			if (iScalar != Profile.iScalarCount || iVector != Profile.iVectorCount ||
				iTexture != Profile.iTextureCount)
			{
				strOutError = "Artist F runtime-material packet denominator changed.";
				return false;
			}

			if (bActive003RuntimeV2)
			{
				const auto GraPower = std::find_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[](const auto& Row)
					{
						return Row.Row.strId ==
							"material-input-10b2901eed57c6c5";
					});
				const auto RedChannelSelection = std::find_if(
					Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [](const auto& Row)
					{
						return Row.Row.strId ==
							"material-input-95265aaecafc99ca";
					});
				if (Recipe->InputIds[0u] != "material-input-2b161e06207b4a97" ||
					Recipe->InputIds[10u] != "material-input-88d1c7b61c5c81d7" ||
					Recipe->InputIds[16u] != "material-input-10b2901eed57c6c5" ||
					Recipe->StaticBindingIds[0u] !=
						"material-input-95265aaecafc99ca" ||
					GraPower == Program.MaterialInputs.end() ||
					GraPower->strRecipeId != Recipe->Row.strId ||
					GraPower->Row.strRowSha256 !=
						"d1a96f83c76c730fd5b9cc13586ff05fe28a2de593fbe61f24dd84677b2253d1" ||
					GraPower->strNormalizedParameterName != "gra_pow" ||
					GraPower->eVariant !=
						EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!GraPower->fValue.has_value() || *GraPower->fValue != 1.5 ||
					GraPower->strTypedValueSha256 !=
						"9f29a130438b81170b92a42650f9a94291ecad60bd47af2a3886e75f7f728725" ||
					GraPower->strSourceFieldValueSha256 !=
						"9f29a130438b81170b92a42650f9a94291ecad60bd47af2a3886e75f7f728725" ||
					GraPower->strSourceLineageSha256 !=
						"eb4a900484c882205f0bed3b4f745af6d36fa0779790766e942a4ca7fd695d52" ||
					RedChannelSelection ==
						Program.MaterialStaticBindings.end() ||
					RedChannelSelection->strRecipeId != Recipe->Row.strId ||
					RedChannelSelection->Row.strRowSha256 !=
						"f76062d563878b420f1f22ba7fb05206984238cc4e93ccc9b554e4ac9df3aafb" ||
					RedChannelSelection->strNormalizedParameterName !=
						"use_gra_r_channel" ||
					RedChannelSelection->strBindingOrigin != "SELF_DEFAULT" ||
					RedChannelSelection->strSelectionRole !=
						"PARENT_DEFAULT_NOT_INSTANCE_SELECTION" ||
					RedChannelSelection->strPolicyRowId !=
						"material-reconstructed-policy-46757ecd896a5efa9c25" ||
					!RedChannelSelection->bSourceValue.has_value() ||
					!*RedChannelSelection->bSourceValue ||
					!RedChannelSelection->bSelectedValue.has_value() ||
					!*RedChannelSelection->bSelectedValue ||
					RedChannelSelection->strSourceFieldValueSha256 !=
						"b5bea41b6c623f7c09f1bf24dcae58ebab3c0cdd90ad966bc43a45b44867e12b" ||
					RedChannelSelection->strSourceLineageSha256 !=
						"c20897fedcf1a44b1d50b6cdf86e6e5e0ae8f78940de2bdb51bfaf6f610221bc" ||
					TextureDecision->Texture0Provider.strMaterialInputFieldId !=
						Recipe->InputIds[0u] ||
					TextureDecision->Texture1Provider.strMaterialInputFieldId !=
						Recipe->InputIds[10u] ||
					TextureDecision->Texture0Provider.strRuntimeAssetId !=
						"Effect/Artist/Textures/fx_i_atypical_03_ycl.dds" ||
					TextureDecision->Texture1Provider.strRuntimeAssetId !=
						"Effect/Artist/Textures/fx_b_atypical_004.dds")
				{
					return false;
				}
				RuntimeTextureProviders = { TextureDecision->Texture0Provider,
					TextureDecision->Texture1Provider };
			}
			else if (bActive011RuntimeV2)
			{
				if (Recipe->InputIds[35u] != "material-input-89f2e8474576e742" ||
					Recipe->InputIds[36u] != "material-input-cb33861f6d80fecb" ||
					Recipe->InputIds[37u] != "material-input-e8cdf5bc22fb2889" ||
					Recipe->InputIds[38u] != "material-input-d1afbde85befe0d0" ||
					TextureDecision->Texture0Provider.strMaterialInputFieldId !=
						Recipe->InputIds[37u] ||
					TextureDecision->Texture1Provider.strMaterialInputFieldId !=
						Recipe->InputIds[38u])
				{
					return false;
				}
				RuntimeTextureProviders = { TextureDecision->Texture0Provider,
					TextureDecision->Texture1Provider };
				RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
					"material-input-89f2e8474576e742",
					"3b42f39a17a018096a46bd9937024b84b1a87951958dccc3c71c6c83d841bafa",
					"material-input-89f2e8474576e742::runtime-texture-binding",
					"0119f6ffe687959d2b9c7ed12a08671be6cc93405ce54927a299ae225cd7d5f2",
					"material-reconstructed-policy-3e961174dffa8d797bd8",
					"8a1cff2bb9a7168e86220751f0b5c88b9fd9ecc0d868b2b3a7b87d3c7cde0ec3",
					"Effect/Artist/Textures/fx_m_atypical_012.dds",
					"FROZEN_PROGRAM_TEXTURE_BINDING_ORDER" });
				RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
					"material-input-cb33861f6d80fecb",
					"387c779bb8ebb30e60f3a50012317da9ddb824258f713bfcc3a52f713afb6984",
					"material-input-cb33861f6d80fecb::runtime-texture-binding",
					"83425ef8b18024cd1b6707d7ba06997acce2b5fbba0db226cd7cc50a3158e4bc",
					"material-reconstructed-policy-89ec510e668d6b156733",
					"1a4881cd64ed3f43f735020f28c63c9b3101195c13b50bb7cecc75623b63a383",
					"Effect/Artist/Textures/fx_m_noise_001.dds",
					"FROZEN_PROGRAM_TEXTURE_BINDING_ORDER" });
			}
			else
			{
				if (Recipe->InputIds[2u] != "material-input-baf9bb89db9f89ee" ||
					TextureDecision->Texture0Provider.strMaterialInputFieldId !=
						Recipe->InputIds[2u] ||
					TextureDecision->Texture0Provider.strRuntimeAssetId !=
						"Effect/Artist/Textures/FX_TEX_00/fx_a_decal_014.dds" ||
					TextureDecision->Texture1Provider.strProviderKind !=
						"NEUTRAL_CONSTANT" ||
					TextureDecision->Texture1Provider.strNeutralProviderId !=
						"RECONSTRUCTED_SIGNED_DISTORTION_ZERO_RGBA")
				{
					return false;
				}
				RuntimeTextureProviders = { TextureDecision->Texture0Provider };
			}

			Resource.iRuntimeMaterialV2DynamicConsumedMask =
				Profile.iDynamicConsumedMask;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask =
				Profile.iDynamicSuppressedMask;
			Resource.iRuntimeMaterialV2ParticleColorPolicy =
				bActive011RuntimeV2 ? 2u : 1u;
			Resource.iRuntimeMaterialV2ParticleColorConsumedMask =
				bActive011RuntimeV2 ? 0x0fu : 0x08u;
			Resource.iRuntimeMaterialV2ParticleColorSuppressedMask =
				bActive011RuntimeV2 ? 0u : 0x07u;
			Resource.iRuntimeMaterialV2ScalarCount = Profile.iScalarCount;
			Resource.iRuntimeMaterialV2VectorCount = Profile.iVectorCount;
			Resource.iRuntimeMaterialV2InputCount = Profile.iInputCount;
			Resource.RuntimeMaterialV2InputConsumedMask = Profile.InputConsumedMask;
			Resource.RuntimeMaterialV2InputSuppressedMask = Profile.InputSuppressedMask;
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				Profile.VectorConsumedMask;
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				Profile.VectorSuppressedMask;
			Resource.iRuntimeMaterialV2StaticInputCount = Profile.iStaticInputCount;
			Resource.iRuntimeMaterialV2StaticSelectedMask = iStaticSelectedMask;
			Resource.iRuntimeMaterialV2StaticConsumedMask =
				Profile.iStaticConsumedMask;
			Resource.iRuntimeMaterialV2StaticSuppressedMask =
				Profile.iStaticSuppressedMask;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
		}
		else if (bActive023RuntimeV2)
		{
			struct EXPECTED_SCALAR_INPUT final
			{
				std::string_view strId;
				std::string_view strName;
				double fValue;
				std::string_view strRowSha256;
			};
			static constexpr std::array<EXPECTED_SCALAR_INPUT, 5u>
				EXPECTED_INPUTS = {{
					{ "material-input-b0a5cc86ab65946d",
						"depthbiasdalpha_bias", 2.5,
						"7e79388f395001be2a25183b42b7bb5f1f482343142f18ca95ec1fc134d733da" },
					{ "material-input-f24590727ef2c78a",
						"03.spherepower", 4.0,
						"dae7f01be477e20ae3f0f32affa7625081279862fae066f3ce477a4145f36f50" },
					{ "material-input-2e8f4b435b5ab1bf",
						"04.sphere_str", 1.0,
						"6035b184242423ebdefee6bea164dea79083ad411c6ac0a1d59e0480afa5292c" },
					{ "material-input-f994a7616d1d1529",
						"01.radius", 0.5,
						"1dc1ffaf3fbbbe6489fdd9ba30cdf7e2109a549589cdb8d43446256dec6f19d8" },
					{ "material-input-21bbaca5f20a3d18",
						"31.fresnal_power", 1.0,
						"cf0b61200ebb4f549460f5e324e1e59d32d90308ca2ebb133810b4c08dd38d19" }
				}};
			static constexpr std::array<std::pair<std::string_view,
				std::string_view>, 6u> EXPECTED_STATIC_ROWS = {{
				{ "material-input-82f275a36b41d455",
					"365b737d4e7bcc8e4fdd46c73ab16dffb36cb0f3fd57f5dd84840dcf6bcc8300" },
				{ "material-input-6a2eb3626231ee86",
					"eee1406e524947fe6dc882b1568c01d01e724db6c7f2f2a5cd7261ece9b730be" },
				{ "material-input-a344fc1ecf985023",
					"d93561941329ef8db1fe48f9ace01d1d8ab78acc75aa8b5d1d72a5b2a3180646" },
				{ "material-input-768ded685b7c432e",
					"7bec945eb7c310c1ccdcbcbf4fe75aaeb2f4784166a6803d9829683bfd0bae24" },
				{ "material-input-cea8c01760241cd1",
					"3f03dbe1068581df068e8a094491a47b15db75db49b7f1e165d74290b966f2ec" },
				{ "material-input-1c557ab83db7f1f8",
					"219d5ba25819936b7ed2397465cc2ece9a787f1334eb748618da0373c7bb6ffa" }
			}};
			static constexpr std::array<std::pair<std::string_view,
				std::string_view>, 6u> EXPECTED_RENDER_ROWS = {{
				{ "blendmode", "a6ce79af339e2eaf7adb8bab6d0421bbfa04ebb6ed345e86a70196e81b44d56e" },
				{ "lightingmodel", "05185bfa10915b77b00e25a01c287e90a57c86a7a1684589f60671fc8c2d07e9" },
				{ "twosided", "32d976605bb789f5617e19601fcf9e7a4692955e298b2fddfb343aede935c7ff" },
				{ "bdisabledepthtest", "1772023fac57b56d03d8de41b74eaf2d244c987fbe51ec893220f17ae7bcdd01" },
				{ "opacitymaskclipvalue", "b0171683b8c26dd908272689d3c91cbe3ff272f2f64763e3470de1170ae90bb8" },
				{ "buseonelayerdistortion", "94f8609a54c16dbad2f5fa818c125d0fe02cfb41490ea61a693286ecd3ad9903" }
			}};

			if (Occurrence->Row.strId != "source-active-023" ||
				Occurrence->Row.strRowSha256 !=
					"3225d6ea915f2fce746e59d53688640ba0d2962bd392ff7a7812cfbdbb3a6570" ||
				Recipe->Row.strId != "material-recipe-ff6a10a52995006e" ||
				Recipe->Row.strRowSha256 !=
					"738c0368d7a0f56419b2c4f8ac6cc15f0a623f9121aa37aa3a0e52fc61403092" ||
				Family->Row.strId != "material-family-9fce58ec032dee02" ||
				Family->Row.strRowSha256 !=
					"93d9806fb74bddba7ce8e467f01238fea09e5431417b61cafdc81909c88138c0" ||
				Family->strEvaluatorId !=
					"reconstructed-evaluator-0d2b5c33172e3c39" ||
				Family->strEvaluatorSha256 !=
					"1c44ce0e1d92c87e70793a70e078c2ee5c3ce26d803c5062bfe94fdd401bb963" ||
				Family->iFeatureMask != 680u ||
				Recipe->InputIds.size() != EXPECTED_INPUTS.size() ||
				Recipe->StaticBindingIds.size() != EXPECTED_STATIC_ROWS.size() ||
				Recipe->RenderBindingIds.size() != EXPECTED_RENDER_ROWS.size() ||
				Element.Material.eRenderProfile !=
					EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ)
			{
				strOutError =
					"Artist F active023 runtime-material identity changed.";
				return false;
			}

			std::array<f32_t, EXPECTED_INPUTS.size()> ScalarValues{};
			for (size_t iInput = 0u; iInput < EXPECTED_INPUTS.size(); ++iInput)
			{
				const EXPECTED_SCALAR_INPUT& Expected = EXPECTED_INPUTS[iInput];
				const auto It = std::find_if(Program.MaterialInputs.begin(),
					Program.MaterialInputs.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Expected.strId;
					});
				if (It == Program.MaterialInputs.end() ||
					std::find(Recipe->InputIds.begin(), Recipe->InputIds.end(),
						std::string(Expected.strId)) == Recipe->InputIds.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->Row.strRowSha256 != Expected.strRowSha256 ||
					It->strNormalizedParameterName != Expected.strName ||
					It->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 ||
					!It->fValue.has_value() || *It->fValue != Expected.fValue ||
					!AssignFloat(*It->fValue, ScalarValues[iInput]))
				{
					strOutError =
						"Artist F active023 scalar input changed.";
					return false;
				}
			}

			for (const auto& [Id, RowSha256] : EXPECTED_STATIC_ROWS)
			{
				const auto It = std::find_if(Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&](const auto& Row)
					{
						return Row.Row.strId == Id;
					});
				if (It == Program.MaterialStaticBindings.end() ||
					std::find(Recipe->StaticBindingIds.begin(),
						Recipe->StaticBindingIds.end(), std::string(Id)) ==
						Recipe->StaticBindingIds.end() ||
					It->strRecipeId != Recipe->Row.strId ||
					It->Row.strRowSha256 != RowSha256 ||
					It->strNormalizedParameterName != "01.checkisvertexcolor" ||
					!It->bSourceValue.has_value() || !*It->bSourceValue ||
					!It->bSelectedValue.has_value() || !*It->bSelectedValue)
				{
					strOutError =
						"Artist F active023 static input changed.";
					return false;
				}
			}

			for (const auto& [FieldName, RowSha256] : EXPECTED_RENDER_ROWS)
			{
				const auto It = std::find_if(Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&](const auto& Row)
					{
						return Row.strRecipeId == Recipe->Row.strId &&
							Row.strFieldName == FieldName;
					});
				if (It == Program.MaterialRenderBindings.end() ||
					std::find(Recipe->RenderBindingIds.begin(),
						Recipe->RenderBindingIds.end(), It->Row.strId) ==
						Recipe->RenderBindingIds.end() ||
					It->Row.strRowSha256 != RowSha256)
				{
					strOutError =
						"Artist F active023 render input changed.";
					return false;
				}
			}

			static constexpr std::array<std::string_view, 4u>
				EXPECTED_DYNAMIC_NAMES = {{
					"y.pan[1-2]", "uvdistort_str[0-x]",
					"uv_sphery[1-x]", "twirl_curvature"
				}};
			static constexpr std::array<std::string_view, 4u>
				EXPECTED_DYNAMIC_NAME_ROWS = {{
					"0912cf0d50a48395c3192a039edc0a4ac8ccc78ebcde722fae410b78dda5bf47",
					"3e815852997cc466ca3f5bc6b289e98ae2cf23b3bdc49c56b4ccb6879a669fda",
					"e97a32075476e5f3a9a387bcaa9a62885d3d67767ad23833dec8d76066120637",
					"b710f9ac345802bb060fd5a8de036a347f2e682ef33d4de72ebf03725ad40b03"
				}};
			static constexpr std::array<std::string_view, 4u>
				EXPECTED_DYNAMIC_DISTRIBUTION_ROWS = {{
					"38f7d9e6d466f29281b7c6cddb5bc990ad885cccb2f6f731c684726a3fba4d8b",
					"f1fb61044aab0f24dea4caa058bc72985a47eb66666f93ad11f5e1fe5859b4d3",
					"5f693031e77e592773a89a5a3f6a05f70ed3d08e04ce75d460012c68b54daeaa",
					"64b61defb564706ec9c6d9b2d8194b1faab3db17c5848a1b7bb165a546ba8013"
				}};
			const EFFECT_RUNTIME_PROGRAM_MODULE* DynamicModule = nullptr;
			uint32_t iDynamicModuleCount = 0u;
			for (const std::string& ModuleId : Emitter.ModuleIds)
			{
				const auto It = std::find_if(Program.Modules.begin(),
					Program.Modules.end(), [&](const auto& Row)
					{
						return Row.Row.strId == ModuleId;
					});
				if (It == Program.Modules.end())
					return false;
				if (It->strExactSourceClass == "particlemoduleparameterdynamic")
				{
					DynamicModule = &*It;
					++iDynamicModuleCount;
				}
			}
			bool_t bDynamicContract = iDynamicModuleCount == 1u &&
				nullptr != DynamicModule && DynamicModule->Row.iOrder == 4u &&
				DynamicModule->Row.strRowSha256 ==
					"19fafd910690ffdec2057ad32c79fd92a05fdda4309f42d577d43e855941716b" &&
				DynamicModule->DistributionIds.size() ==
					EXPECTED_DYNAMIC_NAMES.size();
			for (size_t iDynamic = 0u;
				bDynamicContract && iDynamic < EXPECTED_DYNAMIC_NAMES.size();
				++iDynamic)
			{
				const std::string NamePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramname";
				const std::string ValuePath = "dynamicparams[" +
					std::to_string(iDynamic) + "].paramvalue";
				const EFFECT_RUNTIME_PROGRAM_LITERAL* NameLiteral = nullptr;
				uint32_t iNameCount = 0u;
				for (const EFFECT_RUNTIME_PROGRAM_LITERAL& Literal : Program.Literals)
				{
					if (Literal.strModuleId == DynamicModule->Row.strId &&
						Literal.strPropertyPath == NamePath)
					{
						NameLiteral = &Literal;
						++iNameCount;
					}
				}
				const auto DistributionIt = std::find_if(
					Program.Distributions.begin(), Program.Distributions.end(),
					[&](const auto& Row)
					{
						return Row.Row.strId ==
							DynamicModule->DistributionIds[iDynamic];
					});
				bDynamicContract = nullptr != NameLiteral && iNameCount == 1u &&
					NameLiteral->eVariant ==
						EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					NameLiteral->strEnumValue == EXPECTED_DYNAMIC_NAMES[iDynamic] &&
					NameLiteral->Row.strRowSha256 ==
						EXPECTED_DYNAMIC_NAME_ROWS[iDynamic] &&
					DistributionIt != Program.Distributions.end() &&
					DistributionIt->strModuleId == DynamicModule->Row.strId &&
					DistributionIt->Row.iOrder == iDynamic &&
					DistributionIt->strPropertyPath == ValuePath &&
					DistributionIt->iComponentCount == 1u &&
					DistributionIt->Row.strRowSha256 ==
						EXPECTED_DYNAMIC_DISTRIBUTION_ROWS[iDynamic];
			}
			if (!bDynamicContract)
			{
				strOutError =
					"Artist F active023 dynamic input changed.";
				return false;
			}

			// Bounded reconstructed visual policy: the cooked parent graph is
			// incomplete.  Only the named spherical carrier is consumed; pan,
			// distortion and twirl remain explicit suppressions rather than
			// invented equations.
			Resource.iRuntimeMaterialV2DynamicConsumedMask = 0x04u;
			Resource.iRuntimeMaterialV2DynamicSuppressedMask = 0x0bu;
			Resource.iRuntimeMaterialV2ScalarCount = 5u;
			Resource.iRuntimeMaterialV2VectorCount = 0u;
			Resource.iRuntimeMaterialV2InputCount = 5u;
			Resource.RuntimeMaterialV2InputConsumedMask = { 0x1eu, 0u };
			Resource.RuntimeMaterialV2InputSuppressedMask = { 0x01u, 0u };
			Resource.RuntimeMaterialV2VectorComponentConsumedMask =
				{ 0u, 0u, 0u };
			Resource.RuntimeMaterialV2VectorComponentSuppressedMask =
				{ 0u, 0u, 0u };
			Resource.iRuntimeMaterialV2StaticInputCount = 6u;
			Resource.iRuntimeMaterialV2StaticSelectedMask = 0x3fu;
			Resource.iRuntimeMaterialV2StaticConsumedMask = 0x3fu;
			Resource.iRuntimeMaterialV2StaticSuppressedMask = 0u;
			Resource.iRuntimeMaterialV2RenderInputCount = 6u;
			Resource.iRuntimeMaterialV2RenderConsumedMask = 0x2fu;
			Resource.iRuntimeMaterialV2RenderSuppressedMask = 0x10u;
			Resource.RuntimeMaterialV2ScalarBlocks[0u] = {
				ScalarValues[3u], ScalarValues[1u], ScalarValues[2u],
				ScalarValues[4u] };
			Resource.RuntimeMaterialV2ScalarBlocks[1u].x = ScalarValues[0u];
		}

		const auto StageTextureLane = [&](const size_t iLane,
			const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_PROVIDER& Provider) -> bool_t
		{
			if (iLane >= Resource.SourceTextures.size() ||
				iLane >= Resource.RuntimeMaterialV2Samplers.size() ||
				Provider.strProviderKind != "MATERIAL_TEXTURE_BINDING")
			{
				return false;
			}
			const auto InputIt = std::find_if(Program.MaterialInputs.begin(),
				Program.MaterialInputs.end(), [&](const auto& Row)
				{
					return Row.Row.strId == Provider.strMaterialInputFieldId;
				});
			const auto BindingIt = std::find_if(
				Program.MaterialTextureBindings.begin(),
				Program.MaterialTextureBindings.end(), [&](const auto& Row)
				{
					return Row.Row.strId == Provider.strTextureBindingId;
				});
			const auto PolicyIt = std::find_if(Program.MaterialPolicies.begin(),
				Program.MaterialPolicies.end(), [&](const auto& Row)
				{
					return Row.Row.strId == Provider.strSamplerPolicyRowId;
				});
			if (InputIt == Program.MaterialInputs.end() ||
				BindingIt == Program.MaterialTextureBindings.end() ||
				PolicyIt == Program.MaterialPolicies.end() ||
				InputIt->strRecipeId != Recipe->Row.strId ||
				BindingIt->strRecipeId != Recipe->Row.strId ||
				BindingIt->strMaterialInputFieldId != InputIt->Row.strId ||
				BindingIt->strSamplerPolicyRowId != PolicyIt->Row.strId ||
				InputIt->Row.strRowSha256 != Provider.strMaterialInputRowSha256 ||
				BindingIt->Row.strRowSha256 != Provider.strTextureBindingRowSha256 ||
				PolicyIt->Row.strRowSha256 != Provider.strSamplerPolicyRowSha256 ||
				!BindingIt->strRuntimeAssetId.has_value() ||
				*BindingIt->strRuntimeAssetId != Provider.strRuntimeAssetId ||
				!PolicyIt->SamplerDescriptor.has_value())
			{
				return false;
			}
			const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING* SidecarBinding =
				nullptr;
			uint32_t iSidecarBindingCount = 0u;
			for (const auto& [strBindingId, Binding] : Authority.TextureBindingsById)
			{
				(void)strBindingId;
				if (Binding.strCandidateBindingId == BindingIt->Row.strId &&
					Binding.strCandidateBindingRowSha256 ==
						BindingIt->Row.strRowSha256)
				{
					SidecarBinding = &Binding;
					++iSidecarBindingCount;
				}
			}
			const auto ResourceIt = nullptr == SidecarBinding ?
				Authority.TextureResourcesById.end() :
				Authority.TextureResourcesById.find(
					SidecarBinding->strResourceAuthorityId);
			const EFFECT_RUNTIME_PROGRAM_SAMPLER_DESCRIPTOR& ProgramSampler =
				*PolicyIt->SamplerDescriptor;
			const auto SameProgramSampler = [&ProgramSampler](
				const D3D11_SAMPLER_DESC& D3d)
			{
				if (ProgramSampler.iFilterD3d11 !=
					static_cast<uint32_t>(D3d.Filter) ||
					ProgramSampler.iAddressUD3d11 !=
						static_cast<uint32_t>(D3d.AddressU) ||
					ProgramSampler.iAddressVD3d11 !=
						static_cast<uint32_t>(D3d.AddressV) ||
					ProgramSampler.iAddressWD3d11 !=
						static_cast<uint32_t>(D3d.AddressW) ||
					static_cast<f32_t>(ProgramSampler.fMipLodBias) != D3d.MipLODBias ||
					ProgramSampler.iMaxAnisotropy != D3d.MaxAnisotropy ||
					ProgramSampler.iComparisonFuncD3d11 !=
						static_cast<uint32_t>(D3d.ComparisonFunc) ||
					static_cast<f32_t>(ProgramSampler.fMinLod) != D3d.MinLOD ||
					static_cast<f32_t>(ProgramSampler.fMaxLod) != D3d.MaxLOD)
				{
					return false;
				}
				for (size_t i = 0u; i < ProgramSampler.vBorderColor.size(); ++i)
				{
					if (static_cast<f32_t>(ProgramSampler.vBorderColor[i]) !=
						D3d.BorderColor[i])
					{
						return false;
					}
				}
				return true;
			};
			if (nullptr == SidecarBinding || iSidecarBindingCount != 1u ||
				ResourceIt == Authority.TextureResourcesById.end() ||
				SidecarBinding->strRecipeId != Recipe->Row.strId ||
				SidecarBinding->strMaterialInputFieldId != InputIt->Row.strId ||
				SidecarBinding->strSamplerPolicyRowId != PolicyIt->Row.strId ||
				SidecarBinding->strRuntimeAssetId != Provider.strRuntimeAssetId ||
				SidecarBinding->strActualDdsRawSha256 !=
					ResourceIt->second.strRawSha256 ||
				SidecarBinding->strRuntimeAssetId !=
					ResourceIt->second.strRuntimeAssetId ||
				SidecarBinding->iActualDdsByteCount != ResourceIt->second.iByteCount ||
				!SameProgramSampler(SidecarBinding->SamplerDescriptor) ||
				ProgramSampler.strSrvColorSpace !=
					SidecarBinding->ActualDdsSrv.strColorSpace ||
				SidecarBinding->SamplerDescriptor.Filter !=
					D3D11_FILTER_MIN_MAG_MIP_LINEAR)
			{
				return false;
			}
			const auto IsSupportedAddress = [](const D3D11_TEXTURE_ADDRESS_MODE Mode)
			{
				return Mode == D3D11_TEXTURE_ADDRESS_WRAP ||
					Mode == D3D11_TEXTURE_ADDRESS_CLAMP;
			};
			if (!IsSupportedAddress(SidecarBinding->SamplerDescriptor.AddressU) ||
				!IsSupportedAddress(SidecarBinding->SamplerDescriptor.AddressV) ||
				!IsSupportedAddress(SidecarBinding->SamplerDescriptor.AddressW))
			{
				return false;
			}

			const std::string CacheKey = "reconstructed-evaluator\n" +
				SidecarBinding->strRuntimeAssetId + "\n" +
				SidecarBinding->strActualDdsRawSha256 + "\n" +
				SidecarBinding->ActualDdsSrv.strColorSpace;
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (nullptr != pSharedAssets)
			{
				const auto Cached = pSharedAssets->Textures.find(CacheKey);
				if (Cached != pSharedAssets->Textures.end())
					Texture = Cached->second;
			}
			if (nullptr == Texture)
			{
				std::filesystem::path TexturePath;
				std::vector<uint8_t> TextureBytes;
				if (!Read_ReconstructedAssetBytes(
					SidecarBinding->strRuntimeAssetId,
					SidecarBinding->iActualDdsByteCount,
					SidecarBinding->strActualDdsRawSha256, TexturePath,
					TextureBytes, strOutError))
				{
					return false;
				}
				const DirectX::DDS_LOADER_FLAGS Flags =
					SidecarBinding->ActualDdsSrv.strColorSpace == "SRGB" ?
						DirectX::DDS_LOADER_FORCE_SRGB :
						DirectX::DDS_LOADER_IGNORE_SRGB;
				if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
					m_pDevice.Get(), TextureBytes.data(), TextureBytes.size(), 0u,
					D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
					Flags, nullptr, &Texture)))
				{
					strOutError =
						"Artist F runtime-material DDS upload failed.";
					return false;
				}
				if (nullptr != pSharedAssets)
					pSharedAssets->Textures.emplace(CacheKey, Texture);
			}
			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDescriptor{};
			Texture->GetDesc(&SrvDescriptor);
			const EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& ExpectedSrv =
				SidecarBinding->ActualDdsSrv;
			if (SrvDescriptor.Format != ExpectedSrv.eFormat ||
				SrvDescriptor.ViewDimension != ExpectedSrv.eViewDimension ||
				SrvDescriptor.Texture2D.MostDetailedMip !=
					ExpectedSrv.iMostDetailedMip ||
				SrvDescriptor.Texture2D.MipLevels != ExpectedSrv.iMipLevels)
			{
				strOutError =
					"Artist F runtime-material DDS SRV descriptor changed.";
				return false;
			}
			Resource.SourceTextures[iLane] = std::move(Texture);
			D3D11_SAMPLER_DESC RuntimeSamplerAuthority{};
			if (!Materialize_RuntimeSamplerDescriptor(
				SidecarBinding->SamplerDescriptor, RuntimeSamplerAuthority))
			{
				strOutError =
					"Artist F runtime-material sampler authority is invalid.";
				return false;
			}
			ComPtr<ID3D11SamplerState> RuntimeSampler;
			if (FAILED(m_pDevice->CreateSamplerState(
				&RuntimeSamplerAuthority, &RuntimeSampler)))
			{
				strOutError =
					"Artist F runtime-material sampler creation failed.";
				return false;
			}
			D3D11_SAMPLER_DESC RuntimeSamplerDescriptor{};
			RuntimeSampler->GetDesc(&RuntimeSamplerDescriptor);
			if (!Same_RuntimeSamplerReadbackDescriptor(RuntimeSamplerDescriptor,
				RuntimeSamplerAuthority, SidecarBinding->SamplerDescriptor))
			{
				strOutError =
					"Artist F runtime-material sampler readback changed.";
				return false;
			}
			Resource.RuntimeMaterialV2Samplers[iLane] =
				std::move(RuntimeSampler);
			if (!Capture_MaterialExecutionLane(Resource, iLane,
				SidecarBinding->strRuntimeAssetId,
				InputIt->strNormalizedParameterName, {},
				SidecarBinding->ActualDdsSrv.strColorSpace == "SRGB" ?
					EFFECT_TEXTURE_COLOR_SPACE::SRGB :
					EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
				Resource.RuntimeMaterialV2Samplers[iLane], strOutError))
			{
				return false;
			}
			Resource.iSourceTextureMask |= 1u << static_cast<uint32_t>(iLane);
			if (SidecarBinding->SamplerDescriptor.AddressU ==
				D3D11_TEXTURE_ADDRESS_CLAMP)
			{
				Resource.iSourceTextureClampUMask |=
					1u << static_cast<uint32_t>(iLane);
			}
			if (SidecarBinding->SamplerDescriptor.AddressV ==
				D3D11_TEXTURE_ADDRESS_CLAMP)
			{
				Resource.iSourceTextureClampVMask |=
					1u << static_cast<uint32_t>(iLane);
			}
			return true;
		};
		const auto StageVisualV4TextureLane = [&](const size_t iLane,
			const ARTIST_VISUAL_V4_TEXTURE_LANE& Lane) -> bool_t
		{
			const auto FailLane = [&](const std::string_view strCategory) -> bool_t
			{
				strOutError = "Artist F visual-program V4 texture lane stage failed: "
					"order=" + std::to_string(Occurrence->Row.iOrder) +
					", occurrence=" + Occurrence->Row.strId +
					", shaderLane=" + std::to_string(iLane) +
					", parameter=" + std::string(Lane.strParameterName) +
					", category=" + std::string(strCategory);
				return false;
			};
			if (iLane >= Resource.SourceTextures.size() ||
				iLane >= Resource.RuntimeMaterialV2Samplers.size())
			{
				return FailLane("LANE_RANGE");
			}
			const size_t iInputCount = static_cast<size_t>(std::count_if(
				Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
				[&](const auto& Row)
				{
					return Row.strRecipeId == Recipe->Row.strId &&
						Row.strNormalizedParameterName == Lane.strParameterName &&
						Row.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID;
				}));
			const auto InputIt = std::find_if(Program.MaterialInputs.begin(),
				Program.MaterialInputs.end(), [&](const auto& Row)
				{
					return Row.strRecipeId == Recipe->Row.strId &&
						Row.strNormalizedParameterName == Lane.strParameterName;
				});
			if (iInputCount != 1u || InputIt == Program.MaterialInputs.end())
				return FailLane("INPUT_CARDINALITY");
			if (InputIt->eVariant != EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ||
				InputIt->strStringValue != Lane.strExpectedSourceTextureId)
				return FailLane("INPUT_TEXTURE_IDENTITY");
			if (!Lane.strExpectedFieldId.empty() &&
				InputIt->Row.strId != Lane.strExpectedFieldId)
				return FailLane("INPUT_FIELD_ID");
			if (!Lane.strExpectedInputRowSha256.empty() &&
				InputIt->Row.strRowSha256 != Lane.strExpectedInputRowSha256)
				return FailLane("INPUT_ROW_SHA256");
			if (InputIt->strTypedValueSha256.empty() ||
				InputIt->strSourceFieldValueSha256.empty())
				return FailLane("INPUT_PROVENANCE");
			const size_t iBindingCount = static_cast<size_t>(std::count_if(
				Program.MaterialTextureBindings.begin(),
				Program.MaterialTextureBindings.end(), [&](const auto& Row)
				{
					return Row.strRecipeId == Recipe->Row.strId &&
						Row.strMaterialInputFieldId == InputIt->Row.strId;
				}));
			const auto BindingIt = std::find_if(
				Program.MaterialTextureBindings.begin(),
				Program.MaterialTextureBindings.end(), [&](const auto& Row)
				{
					return Row.strRecipeId == Recipe->Row.strId &&
						Row.strMaterialInputFieldId == InputIt->Row.strId;
				});
			const auto PolicyIt = BindingIt == Program.MaterialTextureBindings.end() ?
				Program.MaterialPolicies.end() :
				std::find_if(Program.MaterialPolicies.begin(),
					Program.MaterialPolicies.end(), [&](const auto& Row)
					{
						return Row.Row.strId == BindingIt->strSamplerPolicyRowId;
					});
			if (iBindingCount != 1u ||
				BindingIt == Program.MaterialTextureBindings.end())
				return FailLane("BINDING_CARDINALITY");
			if (PolicyIt == Program.MaterialPolicies.end())
				return FailLane("SAMPLER_POLICY_MISSING");
			if (!BindingIt->strRuntimeAssetId.has_value() ||
				*BindingIt->strRuntimeAssetId != Lane.strExpectedRuntimeAssetId)
				return FailLane("BINDING_RUNTIME_ASSET_ID");
			if (!Lane.strExpectedBindingRowSha256.empty() &&
				BindingIt->Row.strRowSha256 != Lane.strExpectedBindingRowSha256)
				return FailLane("BINDING_ROW_SHA256");
			if (!Lane.strExpectedSamplerPolicyId.empty() &&
				(BindingIt->strSamplerPolicyRowId !=
					Lane.strExpectedSamplerPolicyId ||
				 PolicyIt->Row.strId != Lane.strExpectedSamplerPolicyId))
				return FailLane("SAMPLER_POLICY_ID");
			if (!Lane.strExpectedSamplerPolicyRowSha256.empty() &&
				PolicyIt->Row.strRowSha256 !=
					Lane.strExpectedSamplerPolicyRowSha256)
				return FailLane("SAMPLER_POLICY_ROW_SHA256");
			if (!PolicyIt->SamplerDescriptor.has_value())
				return FailLane("SAMPLER_DESCRIPTOR_MISSING");

			const EFFECT_RECONSTRUCTED_RENDER_TEXTURE_BINDING* SidecarBinding = nullptr;
			uint32_t iSidecarBindingCount = 0u;
			for (const auto& [strBindingId, Binding] : Authority.TextureBindingsById)
			{
				(void)strBindingId;
				if (Binding.strCandidateBindingId == BindingIt->Row.strId &&
					Binding.strCandidateBindingRowSha256 == BindingIt->Row.strRowSha256)
				{
					SidecarBinding = &Binding;
					++iSidecarBindingCount;
				}
			}
			const auto ResourceIt = nullptr == SidecarBinding ?
				Authority.TextureResourcesById.end() :
				Authority.TextureResourcesById.find(
					SidecarBinding->strResourceAuthorityId);
			if (nullptr == SidecarBinding || iSidecarBindingCount != 1u)
				return FailLane("AUTHORITY_BINDING_CARDINALITY");
			if (ResourceIt == Authority.TextureResourcesById.end())
				return FailLane("RESOURCE_AUTHORITY_MISSING");
			if (SidecarBinding->strRecipeId != Recipe->Row.strId ||
				SidecarBinding->strMaterialInputFieldId != InputIt->Row.strId ||
				SidecarBinding->strSamplerPolicyRowId != PolicyIt->Row.strId ||
				SidecarBinding->strSamplerPolicyRowSha256 !=
					PolicyIt->Row.strRowSha256)
				return FailLane("AUTHORITY_PROGRAM_JOIN");
			if (SidecarBinding->strRuntimeAssetId != Lane.strExpectedRuntimeAssetId)
				return FailLane("AUTHORITY_RUNTIME_ASSET_ID");
			if (SidecarBinding->strActualDdsRawSha256 !=
					ResourceIt->second.strRawSha256 ||
				SidecarBinding->iActualDdsByteCount != ResourceIt->second.iByteCount)
				return FailLane("AUTHORITY_RESOURCE_IDENTITY");
			if (0u != Lane.iExpectedDdsByteCount &&
				SidecarBinding->iActualDdsByteCount != Lane.iExpectedDdsByteCount)
				return FailLane("DDS_BYTE_COUNT");
			if (!Lane.strExpectedDdsRawSha256.empty() &&
				SidecarBinding->strActualDdsRawSha256 !=
					Lane.strExpectedDdsRawSha256)
				return FailLane("DDS_SHA256");
			if (!Lane.strExpectedSamplerPolicyId.empty() &&
				(SidecarBinding->SamplerDescriptor.Filter !=
					D3D11_FILTER_MIN_MAG_MIP_LINEAR ||
				 SidecarBinding->SamplerDescriptor.AddressU !=
					Lane.eExpectedAddressU ||
				 SidecarBinding->SamplerDescriptor.AddressV !=
					Lane.eExpectedAddressV ||
				 SidecarBinding->SamplerDescriptor.AddressW !=
					Lane.eExpectedAddressW))
				return FailLane("SAMPLER_DESCRIPTOR");
			if (!Lane.strExpectedSamplerPolicyId.empty() &&
				SidecarBinding->ActualDdsSrv.eFormat !=
					Lane.eExpectedSrvFormat)
				return FailLane("AUTHORITY_SRV_FORMAT");

			const std::string CacheKey = "artist-visual-v4\n" +
				SidecarBinding->strRuntimeAssetId + "\n" +
				SidecarBinding->strActualDdsRawSha256 + "\n" +
				SidecarBinding->ActualDdsSrv.strColorSpace;
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (nullptr != pSharedAssets)
			{
				const auto Cached = pSharedAssets->Textures.find(CacheKey);
				if (Cached != pSharedAssets->Textures.end())
					Texture = Cached->second;
			}
			if (nullptr == Texture)
			{
				std::filesystem::path TexturePath;
				std::vector<uint8_t> TextureBytes;
				if (!Read_ReconstructedAssetBytes(
					SidecarBinding->strRuntimeAssetId,
					SidecarBinding->iActualDdsByteCount,
					SidecarBinding->strActualDdsRawSha256,
					TexturePath, TextureBytes, strOutError))
				{
					const std::string AssetError = std::move(strOutError);
					FailLane("DDS_FILE_IDENTITY");
					strOutError += ": " + AssetError;
					return false;
				}
				const DirectX::DDS_LOADER_FLAGS Flags =
					SidecarBinding->ActualDdsSrv.strColorSpace == "SRGB" ?
						DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB;
				if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
					m_pDevice.Get(), TextureBytes.data(), TextureBytes.size(), 0u,
					D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
					Flags, nullptr, &Texture)))
				{
					return FailLane("DDS_UPLOAD");
				}
				if (nullptr != pSharedAssets)
					pSharedAssets->Textures.emplace(CacheKey, Texture);
			}
			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDescriptor{};
			Texture->GetDesc(&SrvDescriptor);
			const EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& ExpectedSrv =
				SidecarBinding->ActualDdsSrv;
			if (SrvDescriptor.Format != ExpectedSrv.eFormat ||
				SrvDescriptor.ViewDimension != ExpectedSrv.eViewDimension ||
				SrvDescriptor.Texture2D.MostDetailedMip != ExpectedSrv.iMostDetailedMip ||
				SrvDescriptor.Texture2D.MipLevels != ExpectedSrv.iMipLevels)
			{
				return FailLane("DDS_SRV_DESCRIPTOR");
			}
			if (Lane.iExpectedWidth != 0u || Lane.iExpectedHeight != 0u)
			{
				ComPtr<ID3D11Resource> D3dResource;
				Texture->GetResource(D3dResource.GetAddressOf());
				ComPtr<ID3D11Texture2D> Texture2D;
				D3D11_TEXTURE2D_DESC TextureDescriptor{};
				if (nullptr == D3dResource || FAILED(D3dResource.As(&Texture2D)) ||
					nullptr == Texture2D)
				{
					return FailLane("DDS_TEXTURE_RESOURCE");
				}
				Texture2D->GetDesc(&TextureDescriptor);
				if (TextureDescriptor.Width != Lane.iExpectedWidth ||
					TextureDescriptor.Height != Lane.iExpectedHeight ||
					TextureDescriptor.MipLevels != 1u ||
					TextureDescriptor.ArraySize != 1u ||
					TextureDescriptor.Format != Lane.eExpectedSrvFormat)
				{
					return FailLane("DDS_TEXTURE_DESCRIPTOR");
				}
			}
			Resource.SourceTextures[iLane] = std::move(Texture);
			D3D11_SAMPLER_DESC RuntimeSamplerAuthority{};
			if (!Materialize_RuntimeSamplerDescriptor(
				SidecarBinding->SamplerDescriptor, RuntimeSamplerAuthority))
			{
				return FailLane("SAMPLER_MATERIALIZATION");
			}
			ComPtr<ID3D11SamplerState> RuntimeSampler;
			if (FAILED(m_pDevice->CreateSamplerState(
				&RuntimeSamplerAuthority, &RuntimeSampler)))
			{
				return FailLane("SAMPLER_CREATION");
			}
			D3D11_SAMPLER_DESC RuntimeSamplerDescriptor{};
			RuntimeSampler->GetDesc(&RuntimeSamplerDescriptor);
			if (!Same_RuntimeSamplerReadbackDescriptor(RuntimeSamplerDescriptor,
				RuntimeSamplerAuthority, SidecarBinding->SamplerDescriptor))
			{
				return FailLane("SAMPLER_READBACK");
			}
			Resource.RuntimeMaterialV2Samplers[iLane] =
				std::move(RuntimeSampler);
			if (!Capture_MaterialExecutionLane(Resource, iLane,
				SidecarBinding->strRuntimeAssetId,
				std::string(Lane.strParameterName), {},
				SidecarBinding->ActualDdsSrv.strColorSpace == "SRGB" ?
					EFFECT_TEXTURE_COLOR_SPACE::SRGB :
					EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
				Resource.RuntimeMaterialV2Samplers[iLane], strOutError))
			{
				return FailLane("AUTHORED_LANE_CAPTURE");
			}
			Resource.iSourceTextureMask |= 1u << static_cast<uint32_t>(iLane);
			if (SidecarBinding->SamplerDescriptor.AddressU ==
				D3D11_TEXTURE_ADDRESS_CLAMP)
			{
				Resource.iSourceTextureClampUMask |=
					1u << static_cast<uint32_t>(iLane);
			}
			if (SidecarBinding->SamplerDescriptor.AddressV ==
				D3D11_TEXTURE_ADDRESS_CLAMP)
			{
				Resource.iSourceTextureClampVMask |=
					1u << static_cast<uint32_t>(iLane);
			}
			return true;
		};
		const auto StageBoundedFixedTextureLane = [&] (const size_t iLane,
			const std::string& strRuntimeAssetId, const uint64_t iByteCount,
			const std::string& strRawSha256, const uint32_t iWidth,
			const uint32_t iHeight,
			const D3D11_TEXTURE_ADDRESS_MODE eAddress,
			const DXGI_FORMAT eLinearFormat = DXGI_FORMAT_BC1_UNORM,
			const bool_t bSrgb = false,
			const std::string_view strRole = {},
			const std::string_view strSourceChannel = {}) -> bool_t
		{
			if (iLane >= Resource.SourceTextures.size() ||
				iLane >= Resource.RuntimeMaterialV2Samplers.size() ||
				nullptr != Resource.SourceTextures[iLane])
			{
				strOutError = "Artist F fixed DDS lane identity is invalid: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId + ".";
				return false;
			}
			const DXGI_FORMAT eExpectedFormat = bSrgb ?
				(eLinearFormat == DXGI_FORMAT_BC1_UNORM ? DXGI_FORMAT_BC1_UNORM_SRGB :
					(eLinearFormat == DXGI_FORMAT_BC3_UNORM ? DXGI_FORMAT_BC3_UNORM_SRGB :
						DXGI_FORMAT_UNKNOWN)) : eLinearFormat;
			if (eExpectedFormat == DXGI_FORMAT_UNKNOWN)
			{
				strOutError = "Artist F fixed DDS lane has no supported SRV format: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId + ".";
				return false;
			}
			const std::string CacheKey = "artist-bounded-fixed\n" +
				strRuntimeAssetId + "\n" + strRawSha256 +
				(bSrgb ? "\nSRGB\n" : "\nLINEAR\n") +
				std::to_string(static_cast<uint32_t>(eAddress));
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (nullptr != pSharedAssets)
			{
				const auto Cached = pSharedAssets->Textures.find(CacheKey);
				if (Cached != pSharedAssets->Textures.end())
					Texture = Cached->second;
			}
			if (nullptr == Texture)
			{
				std::filesystem::path TexturePath;
				std::vector<uint8_t> TextureBytes;
				if (!Read_ReconstructedAssetBytes(strRuntimeAssetId, iByteCount,
					strRawSha256, TexturePath, TextureBytes, strOutError) ||
					FAILED(DirectX::CreateDDSTextureFromMemoryEx(
						m_pDevice.Get(), TextureBytes.data(), TextureBytes.size(), 0u,
						D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
						bSrgb ? DirectX::DDS_LOADER_FORCE_SRGB :
							DirectX::DDS_LOADER_IGNORE_SRGB, nullptr, &Texture)))
				{
					if (strOutError.empty())
						strOutError = "Artist F flow-02 fixed DDS creation failed.";
					return false;
				}
				if (nullptr != pSharedAssets)
					pSharedAssets->Textures.emplace(CacheKey, Texture);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDescriptor{};
			Texture->GetDesc(&SrvDescriptor);
			ComPtr<ID3D11Resource> D3dResource;
			Texture->GetResource(D3dResource.GetAddressOf());
			ComPtr<ID3D11Texture2D> Texture2D;
			D3D11_TEXTURE2D_DESC TextureDescriptor{};
			if (nullptr == D3dResource || FAILED(D3dResource.As(&Texture2D)) ||
				nullptr == Texture2D)
			{
				strOutError = "Artist F fixed DDS is not a Texture2D: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId + ".";
				return false;
			}
			Texture2D->GetDesc(&TextureDescriptor);
			if (SrvDescriptor.Format != eExpectedFormat ||
				SrvDescriptor.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
				SrvDescriptor.Texture2D.MostDetailedMip != 0u ||
				SrvDescriptor.Texture2D.MipLevels != 1u ||
				TextureDescriptor.Width != iWidth ||
				TextureDescriptor.Height != iHeight ||
				TextureDescriptor.MipLevels != 1u ||
				TextureDescriptor.ArraySize != 1u ||
				TextureDescriptor.Format != eExpectedFormat)
			{
				strOutError = "Artist F fixed DDS descriptor changed: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId +
					", expectedFormat=" + std::to_string(
						static_cast<uint32_t>(eExpectedFormat)) +
					", actualSrvFormat=" + std::to_string(
						static_cast<uint32_t>(SrvDescriptor.Format)) +
					", actualTextureFormat=" + std::to_string(
						static_cast<uint32_t>(TextureDescriptor.Format)) + ".";
				return false;
			}

			/* Native mode_default sampler evidence is unresolved. The semantic
			   replay therefore materializes a named linear address policy; this is
			   bounded and never upgrades native VF/pass admission. */
			D3D11_SAMPLER_DESC SamplerDescriptor{};
			SamplerDescriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			SamplerDescriptor.AddressU = eAddress;
			SamplerDescriptor.AddressV = eAddress;
			SamplerDescriptor.AddressW = eAddress;
			SamplerDescriptor.MaxAnisotropy = 1u;
			SamplerDescriptor.ComparisonFunc = D3D11_COMPARISON_NEVER;
			SamplerDescriptor.MinLOD = 0.f;
			SamplerDescriptor.MaxLOD = D3D11_FLOAT32_MAX;
			ComPtr<ID3D11SamplerState> Sampler;
			if (FAILED(m_pDevice->CreateSamplerState(&SamplerDescriptor, &Sampler)))
			{
				strOutError = "Artist F fixed DDS sampler creation failed: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId + ".";
				return false;
			}
			D3D11_SAMPLER_DESC SamplerReadback{};
			Sampler->GetDesc(&SamplerReadback);
			if (SamplerReadback.Filter != SamplerDescriptor.Filter ||
				SamplerReadback.AddressU != SamplerDescriptor.AddressU ||
				SamplerReadback.AddressV != SamplerDescriptor.AddressV ||
				SamplerReadback.AddressW != SamplerDescriptor.AddressW ||
				SamplerReadback.ComparisonFunc != SamplerDescriptor.ComparisonFunc ||
				SamplerReadback.MinLOD != SamplerDescriptor.MinLOD ||
				SamplerReadback.MaxLOD != SamplerDescriptor.MaxLOD)
			{
				strOutError = "Artist F fixed DDS sampler readback changed: lane=" +
					std::to_string(iLane) + ", asset=" + strRuntimeAssetId + ".";
				return false;
			}

			Resource.SourceTextures[iLane] = std::move(Texture);
			Resource.RuntimeMaterialV2Samplers[iLane] = std::move(Sampler);
			if (!Capture_MaterialExecutionLane(Resource, iLane,
				strRuntimeAssetId, std::string(strRole),
				std::string(strSourceChannel),
				bSrgb ? EFFECT_TEXTURE_COLOR_SPACE::SRGB :
					EFFECT_TEXTURE_COLOR_SPACE::LINEAR,
				Resource.RuntimeMaterialV2Samplers[iLane], strOutError))
			{
				return false;
			}
			Resource.iSourceTextureMask |= 1u << static_cast<uint32_t>(iLane);
			if (eAddress == D3D11_TEXTURE_ADDRESS_CLAMP)
			{
				Resource.iSourceTextureClampUMask |=
					1u << static_cast<uint32_t>(iLane);
				Resource.iSourceTextureClampVMask |=
					1u << static_cast<uint32_t>(iLane);
			}
			return true;
		};
		if (bArtistVisualV4 && Occurrence->Row.iOrder != 33u)
		{
			for (size_t iLane = 0u;
				iLane < ArtistVisualV4TextureLanes.size(); ++iLane)
			{
				const size_t iShaderLane = ArtistVisualV4TextureLanes[iLane].iShaderLane ==
					static_cast<size_t>(-1) ? iLane :
					ArtistVisualV4TextureLanes[iLane].iShaderLane;
				if (!StageVisualV4TextureLane(
					iShaderLane, ArtistVisualV4TextureLanes[iLane]))
				{
					if (strOutError.empty())
					{
						strOutError = "Artist F visual-program V4 texture lane stage failed: "
							"order=" + std::to_string(Occurrence->Row.iOrder) +
							", occurrence=" + Occurrence->Row.strId +
							", shaderLane=" + std::to_string(iShaderLane) +
							", parameter=" + std::string(
								ArtistVisualV4TextureLanes[iLane].strParameterName) +
							", category=UNKNOWN";
					}
					return false;
				}
			}
			if ((Occurrence->Row.iOrder == 13u ||
				Occurrence->Row.iOrder == 14u) &&
				(!StageBoundedFixedTextureLane(0u,
					"Effect/Artist/Textures/fx_d_atypical_076_cl.dds", 384u,
					"0e52495629185392bd73b308bf38a6c128bf1de6c3c1cf43471d41bf928ac223",
					128u, 4u, D3D11_TEXTURE_ADDRESS_CLAMP) ||
				 !StageBoundedFixedTextureLane(3u,
					"Effect/Artist/Textures/fx_d_fluid_032_1_cl.dds", 65664u,
					"6f99596c477d4eb5fb19f48075700dc6c8d64445baebd319ddc49c746c76e18f",
					512u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP)))
			{
				if (strOutError.empty())
					strOutError = "Artist F flow-02 fixed texture stage failed: " +
						Occurrence->Row.strId;
				return false;
			}
			if (Resource.iSourceTextureMask !=
				Resource.iArtistVisualV4TextureMask)
			{
				strOutError = "Artist F visual-program V4 texture mask changed: " +
					Occurrence->Row.strId;
				return false;
			}
		}
		else if (bArtistVisualV4 && Occurrence->Row.iOrder == 33u)
		{
			struct EXPECTED_ONE_LAYER_TEXTURE final
			{
				std::string_view strName;
				std::string_view strSourceTextureId;
				std::string_view strRuntimeAssetId;
			};
			static constexpr std::array<EXPECTED_ONE_LAYER_TEXTURE, 2u>
				EXPECTED_ONE_LAYER_TEXTURES = {{
				{ "emissive texture", "fx_tex_03.fx_e_atypical_012",
					"Effect/Artist/Textures/fx_e_atypical_012.dds" },
				{ "distortion texture", "fx_tex_02.fx_d_noise_009",
					"Effect/Artist/Textures/fx_d_noise_009.dds" }
			}};
			for (size_t iLane = 0u; iLane < EXPECTED_ONE_LAYER_TEXTURES.size();
				++iLane)
			{
				const auto& Expected = EXPECTED_ONE_LAYER_TEXTURES[iLane];
				const size_t iInputCount = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&](const auto& Row)
					{
						return Row.strRecipeId == Recipe->Row.strId &&
							Row.strNormalizedParameterName == Expected.strName &&
							Row.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID &&
							Row.strStringValue == Expected.strSourceTextureId;
					}));
				const size_t iSourceTextureCount = static_cast<size_t>(std::count_if(
					Element.Material.SourceMaterial.Textures.begin(),
					Element.Material.SourceMaterial.Textures.end(),
					[&](const EFFECT_NAMED_TEXTURE_DESC& Texture)
					{
						return Texture.strName == Expected.strName &&
							Texture.strAssetId == Expected.strRuntimeAssetId &&
							Texture.strSamplingEvidence ==
								"reconstructed-cross-export-byte-identical-v1";
					}));
				const auto TextureIt = std::find_if(
					Element.Material.SourceMaterial.Textures.begin(),
					Element.Material.SourceMaterial.Textures.end(),
					[&](const EFFECT_NAMED_TEXTURE_DESC& Texture)
					{
						return Texture.strName == Expected.strName &&
							Texture.strAssetId == Expected.strRuntimeAssetId &&
							Texture.strSamplingEvidence ==
								"reconstructed-cross-export-byte-identical-v1";
					});
				if (iInputCount != 1u || iSourceTextureCount != 1u ||
					TextureIt == Element.Material.SourceMaterial.Textures.end() ||
					FAILED(Load_SourceTexture(
						*TextureIt, Resource.SourceTextures[iLane], pSharedAssets)))
				{
					strOutError =
						"Artist F visual-program V4 one-layer texture stage failed: " +
						Occurrence->Row.strId;
					return false;
				}
				D3D11_SAMPLER_DESC SamplerDesc{};
				SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				SamplerDesc.AddressU = TextureIt->eAddressU ==
					EFFECT_TEXTURE_ADDRESS_MODE::CLAMP ?
					D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
				SamplerDesc.AddressV = TextureIt->eAddressV ==
					EFFECT_TEXTURE_ADDRESS_MODE::CLAMP ?
					D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
				SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				SamplerDesc.MaxAnisotropy = 1u;
				SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				SamplerDesc.MinLOD = 0.f;
				SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
				if (FAILED(m_pDevice->CreateSamplerState(&SamplerDesc,
					&Resource.RuntimeMaterialV2Samplers[iLane])) ||
					!Capture_MaterialExecutionLane(Resource, iLane,
						TextureIt->strAssetId, TextureIt->strName, {},
						TextureIt->eColorSpace,
						Resource.RuntimeMaterialV2Samplers[iLane], strOutError))
				{
					if (strOutError.empty())
						strOutError =
							"Artist F visual-program V4 one-layer sampler stage failed: " +
							Occurrence->Row.strId;
					return false;
				}
				Resource.iSourceTextureMask |= 1u << static_cast<uint32_t>(iLane);
				if (TextureIt->eAddressU == EFFECT_TEXTURE_ADDRESS_MODE::CLAMP)
					Resource.iSourceTextureClampUMask |=
						1u << static_cast<uint32_t>(iLane);
				if (TextureIt->eAddressV == EFFECT_TEXTURE_ADDRESS_MODE::CLAMP)
					Resource.iSourceTextureClampVMask |=
						1u << static_cast<uint32_t>(iLane);
			}
			if (Resource.iSourceTextureMask !=
				Resource.iArtistVisualV4TextureMask)
			{
				strOutError =
					"Artist F visual-program V4 one-layer texture mask changed.";
				return false;
			}
		}
		if (bActive027RuntimeV2 &&
			(!StageBoundedFixedTextureLane(0u,
				"Effect/Artist/Textures/fx_e_noise_008.dds", 65664u,
				"138ec46d262c75b1b4d2619212a0d9b76e263cf6b97dc2122fcb261a32279f9d",
				256u, 256u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC3_UNORM, true) ||
			 !StageBoundedFixedTextureLane(1u,
				"Effect/Artist/Textures/fx_e_fluid_021.dds", 32896u,
				"1cf86038645760963d6a6db584283795f09fc5078b94ffe8e204ca44ed8bdc75",
				256u, 256u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC1_UNORM, true) ||
			 !StageBoundedFixedTextureLane(2u,
				"Effect/Artist/Textures/fx_e_atypical_024_1_xcl.dds", 8320u,
				"8181d6ba0970e2193cd216c5e60b00d563eb38608ba5b664a51571bdbc93126f",
				128u, 128u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC1_UNORM, true) ||
			 !StageBoundedFixedTextureLane(3u,
				"Effect/Artist/Textures/fx_e_electric_002_cl.dds", 65664u,
				"880b9628bd861d807ea5990773ccbfc2f6f900ee890e370e7ab8574ba9340ca8",
				256u, 256u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC3_UNORM, true) ||
			 !StageBoundedFixedTextureLane(4u,
				"Effect/Artist/Textures/fx_e_fluid_003.dds", 65664u,
				"3339f6fd17e588fed6c11082a8a93a9b6dc55c49cb20f2a956dd6258f675a83f",
				256u, 256u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC3_UNORM, true) ||
			 !StageBoundedFixedTextureLane(5u,
				"Effect/Artist/Textures/fx_a_decal_013.dds", 65664u,
				"c37194e45c9dea1b1f897c150ae0fae113431c90cd8161494f716bc705ac368e",
				256u, 256u, D3D11_TEXTURE_ADDRESS_WRAP, DXGI_FORMAT_BC3_UNORM, true)))
		{
			if (strOutError.empty())
				strOutError = "Artist F active027 fixed source DDS stage failed.";
			return false;
		}
		if (bActive020021RuntimeV2 &&
			(!StageBoundedFixedTextureLane(0u,
				"Effect/Artist/Textures/fx_c_decal_002_2.dds", 32896u,
				"0ac0be83ed4e5e0d9c2637ba6daecaec90bca48629aa90533a65923544c8ba83",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC1_UNORM, false, "height", "B") ||
			 !StageBoundedFixedTextureLane(1u,
				"Effect/Artist/Textures/fx_b_fluid_007.dds", 65664u,
				"de26fb772dd39808b4309f58e821c75f971f7d67b4ccc23476e4e1e34044656c",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC3_UNORM, true, "diffuse", "RGBA") ||
			 !StageBoundedFixedTextureLane(2u,
				"Effect/Artist/Textures/fx_c_decal_002_1.dds", 32896u,
				"03ee85efabffc4eff947077095e781ab115bc78efd6440aba4ce2e1138b25ad9",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC1_UNORM, false, "dissolve", "G") ||
			 !StageBoundedFixedTextureLane(3u,
				"Effect/Artist/Textures/fx_d_normal_078.dds", 65664u,
				"c5dd4c2f0907987a0130e2eda4ce449251ca6b8c08d133c3d9a435963f6d9ac6",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC5_UNORM, false, "normal", "RG") ||
			 !StageBoundedFixedTextureLane(4u,
				"Effect/Artist/Textures/fx_d_environ_018.dds", 32896u,
				"7debac0649fcaad39d88dd9733a71b5cd727f699d2127cef5f1f8ce133e5629b",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC1_UNORM, true, "specular", "RGB") ||
			 !StageBoundedFixedTextureLane(5u,
				"Effect/Artist/Textures/fx_bg_dustpanner_01.dds", 32896u,
				"2eed480e6ca2baff90c08cc81b4cc6847ce61f79a2c14559d7a2984d6aa444c4",
				256u, 256u, D3D11_TEXTURE_ADDRESS_CLAMP,
				DXGI_FORMAT_BC1_UNORM, true, "emissive", "R")))
		{
			if (strOutError.empty())
				strOutError = "Artist F LocalDecal six-lane DDS stage failed.";
			return false;
		}
		if (bActive028RuntimeV2 && !StageBoundedFixedTextureLane(0u,
			"Effect/Artist/Textures/fx_d_noise_002.dds", 8320u,
			"6125c3c1bcea0455d3f3c9bf0c8092331cd789f9e6686d00eac45f136fe79393",
			128u, 128u, D3D11_TEXTURE_ADDRESS_WRAP))
		{
			if (strOutError.empty())
				strOutError = "Artist F active028 fixed flow DDS stage failed.";
			return false;
		}
		if (bActive003RuntimeV2 || bActive004RuntimeV2 ||
			bActive005006RuntimeV2 ||
			bActive009010RuntimeV2 || bActive011RuntimeV2 ||
			bActive016RuntimeV2 || bActive022RuntimeV2 ||
			bActive024RuntimeV2 || bActive027RuntimeV2 ||
			bActive028RuntimeV2)
		{
			for (size_t iLane = 0u; iLane < RuntimeTextureProviders.size(); ++iLane)
			{
				const size_t iShaderLane = iLane +
					(bActive028RuntimeV2 ? 1u : 0u);
				if (!StageTextureLane(iShaderLane, RuntimeTextureProviders[iLane]))
				{
					if (strOutError.empty())
					{
						strOutError = "Artist F runtime-material v2 texture lane " +
							std::to_string(iLane) + " stage failed: " +
							Element.strElementId;
					}
					return false;
				}
			}
		}
		else if (!bArtistVisualV4 && !bActive020021RuntimeV2 &&
			!bActive023RuntimeV2 &&
			(!StageTextureLane(0u, TextureDecision->Texture0Provider) ||
				!StageTextureLane(1u, TextureDecision->Texture1Provider)))
		{
			strOutError =
				"Reconstructed common material evaluator texture stage failed: " +
				Element.strElementId;
			return false;
		}
		const uint32_t iExpectedTextureMask = bArtistVisualV4 ?
			Resource.iArtistVisualV4TextureMask :
			(bActive020021RuntimeV2 ? 0x3fu : (bActive004RuntimeV2 ? 0x0fu :
			(bActive016RuntimeV2 ? 0x1fu :
				(bActive023RuntimeV2 ? 0u :
					(bActive011RuntimeV2 ? 0x0fu :
					(bActive027RuntimeV2 ? 0x3fu :
					(bActive028RuntimeV2 ? 0x1fu :
					(bActive024RuntimeV2 ? 0x07u :
						(bActive022RuntimeV2 || bActive005006RuntimeV2 ?
							0x01u : 0x03u)))))))));
		if (Resource.iSourceTextureMask != iExpectedTextureMask)
		{
			if (strOutError.empty())
				strOutError =
					"Reconstructed common material evaluator texture stage failed: " +
					Element.strElementId;
			return false;
		}
		if (bActive004RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 1u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 4u;
			Resource.iRuntimeMaterialV2TextureMask = 0x0fu;
		}
		else if (bActive023RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 2u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 0u;
			Resource.iRuntimeMaterialV2TextureMask = 0u;
		}
		else if (bActive009010RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 3u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 2u;
			Resource.iRuntimeMaterialV2TextureMask = 0x03u;
		}
		else if (bActive016RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 4u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 5u;
			Resource.iRuntimeMaterialV2TextureMask = 0x1fu;
		}
		else if (bActive030RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 5u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 2u;
			Resource.iRuntimeMaterialV2TextureMask = 0x03u;
		}
		else if (bActive002019031RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 6u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 2u;
			Resource.iRuntimeMaterialV2TextureMask = 0x03u;
		}
		else if (bActive022RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 7u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 1u;
			Resource.iRuntimeMaterialV2TextureMask = 0x01u;
		}
		else if (bActive020021RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 14u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 6u;
			Resource.iRuntimeMaterialV2TextureMask = 0x3fu;
		}
		else if (bActive011RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 8u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 4u;
			Resource.iRuntimeMaterialV2TextureMask = 0x0fu;
		}
		else if (bActive003RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 9u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 2u;
			Resource.iRuntimeMaterialV2TextureMask = 0x03u;
		}
		else if (bActive005006RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 10u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 1u;
			Resource.iRuntimeMaterialV2TextureMask = 0x01u;
		}
		else if (bActive024RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 11u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 3u;
			Resource.iRuntimeMaterialV2TextureMask = 0x07u;
		}
		else if (bActive028RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 12u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 5u;
			Resource.iRuntimeMaterialV2TextureMask = 0x1fu;
		}
		else if (bActive027RuntimeV2)
		{
			Resource.iRuntimeMaterialV2Enabled = 1u;
			Resource.iRuntimeMaterialV2Opcode = 13u;
			Resource.iRuntimeMaterialV2TextureLaneCount = 6u;
			Resource.iRuntimeMaterialV2TextureMask = 0x3fu;
		}
		const bool_t bRuntimeV2Registry = ShaderRegistry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2;
		const bool_t bArtistV4Registry = ShaderRegistry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4;
		const auto NONE_OUTPUT = EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE::NONE;
		const auto SCENE_COLOR_OUTPUT =
			EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE::SCENE_COLOR_RT0;
		Resource.bOccurrenceVisualSuppressed = !ShaderRegistry->bDrawAdmitted;
		if ((0u != Resource.iRuntimeMaterialV2Enabled) != bRuntimeV2Registry ||
			Resource.iRuntimeMaterialV2Opcode !=
				(bRuntimeV2Registry ? ShaderRegistry->iOpcode : 0u) ||
			Resource.iArtistVisualV4Opcode !=
				(bArtistV4Registry ? ShaderRegistry->iOpcode : 0u) ||
			Resource.iArtistVisualV4TextureMask !=
				(bArtistV4Registry ?
					ShaderRegistry->iExpectedArtistVisualV4TextureMask : 0u) ||
			Resource.bOccurrenceVisualSuppressed !=
				!ShaderRegistry->bDrawAdmitted ||
			ShaderRegistry->bNativeSelectionAdmitted ||
			(ShaderRegistry->bDrawAdmitted &&
				(ShaderRegistry->eSceneColorOutput != SCENE_COLOR_OUTPUT ||
					ShaderRegistry->eDistortionOutput != NONE_OUTPUT)) ||
			(!ShaderRegistry->bDrawAdmitted &&
				(ShaderRegistry->eSceneColorOutput != NONE_OUTPUT ||
					ShaderRegistry->eDistortionOutput != NONE_OUTPUT)))
		{
			strOutError = "Artist F stable occurrence/visual-program opcode mapping changed: " +
				Occurrence->Row.strId;
			return false;
		}
		Resource.iReconstructedMaterialEvaluatorEnabled = 1u;
		Resource.iReconstructedMaterialFeatureMask = Family->iFeatureMask;
		Resource.bSourceMaterialFallbackBlocked = false;
		bOutStaged = true;
		return true;
	};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
			pMaterialProgramBinding = nullptr == pMaterialProgramRegistry ?
				nullptr : pMaterialProgramRegistry->Resolve(
					strEffectAssetId, Element.strElementId);
		EFFECT_ELEMENT_DESC RegistryMaterializedElement;
		const EFFECT_ELEMENT_DESC* pStagedElement = &Element;
		if (nullptr != pMaterialProgramBinding)
		{
			if (pMaterialProgramBinding->iCatalogRevision != iCatalogRevision ||
				pMaterialProgramBinding->iRegistryGenerationId !=
					pMaterialProgramRegistry->Get_GenerationId() ||
				pMaterialProgramBinding->eInlineMirrorPolicy !=
					EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
				pMaterialProgramBinding->strEffectAssetId != strEffectAssetId ||
				pMaterialProgramBinding->strElementId != Element.strElementId ||
				!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
					pMaterialProgramBinding->Execution,
					Element.Material.Execution))
			{
				strOutError =
					"Bound material-program packet differs from inline golden mirror: " +
					Element.strElementId;
				return false;
			}
			RegistryMaterializedElement = Element;
			RegistryMaterializedElement.Material.Execution =
				pMaterialProgramBinding->Execution;
			pStagedElement = &RegistryMaterializedElement;
		}
		f32_t fModelPreScale = Element.Detail.Mesh.fModelPreScale;
		const EFFECT_RUNTIME_PROGRAM_EMITTER* pProgramEmitter = nullptr;
		if (nullptr != pPreparation && nullptr != pPreparation->Get_Program())
		{
			const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program =
				*pPreparation->Get_Program();
			const auto Emitter = std::find_if(Program.Emitters.begin(),
				Program.Emitters.end(), [&Element](const auto& Row)
				{
					return Row.strSourceElementId == Element.strElementId;
				});
			const auto RendererMatches = [&Element](
				const EFFECT_RUNTIME_RENDERER_KIND eRenderer)
			{
				switch (eRenderer)
				{
				case EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
					return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::MESH_PARTICLE;
				case EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
					return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::SPRITE_PARTICLE;
				case EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
					return Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::DECAL_PARTICLE;
				case EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
					return Element.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::CASCADE_RIBBON;
				case EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
					return Element.eKind == EFFECT_ELEMENT_KIND::LIGHT &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::LIGHT_PARTICLE;
				case EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
					return Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST &&
						Element.Renderer.eType ==
							EFFECT_RENDERER_TYPE::SCREEN_POST;
				default:
					return false;
				}
			};
			const EFFECT_SOURCE_SPACE eExpectedSourceSpace =
				Emitter != Program.Emitters.end() &&
				Emitter->eRenderer == EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST ?
					EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
					EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
			if (Emitter == Program.Emitters.end() ||
				!RendererMatches(Emitter->eRenderer) ||
				Element.Renderer.eSourceSpace != eExpectedSourceSpace)
			{
				strOutError =
					"Reconstructed renderer identity does not match its exact "
					"Program emitter: " + Element.strElementId;
				return false;
			}
			pProgramEmitter = &*Emitter;
			if (Emitter != Program.Emitters.end() &&
				Emitter->strGeometryUseId.has_value())
			{
				const auto Use = std::find_if(Program.GeometryUses.begin(),
					Program.GeometryUses.end(), [&Emitter](const auto& Row)
					{
						return Row.Row.strId == *Emitter->strGeometryUseId;
					});
				if (Use == Program.GeometryUses.end())
				{
					strOutError = "Reconstructed geometry use is missing: " +
						*Emitter->strGeometryUseId;
					return false;
				}
				const EFFECT_RESOURCE_BINDING_DESC* pModelBinding =
					Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL);
				if (nullptr == pModelBinding ||
					pModelBinding->strAssetId != Use->strAssetId)
				{
					strOutError =
						"Reconstructed geometry use/model binding mismatch: " +
						Element.strElementId;
					return false;
				}
				const auto Carrier = std::find_if(Program.GeometryCarriers.begin(),
					Program.GeometryCarriers.end(), [&Use](const auto& Row)
					{
						return Row.Row.strId == Use->strCarrierId;
					});
				if (Carrier == Program.GeometryCarriers.end())
				{
					strOutError = "Reconstructed geometry carrier is missing: " +
						Use->strCarrierId;
					return false;
				}
				fModelPreScale = static_cast<f32_t>(Carrier->fGeometryPreScale);
			}
		}
		ELEMENT_RESOURCE Resource;
		const bool_t bOrdinaryFailClosed = nullptr == pPreparation &&
			nullptr == pVisualProgramProjection &&
			Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate;
		if (bOrdinaryFailClosed)
		{
			// An ordinary fail-closed carrier remains in the playback document,
			// but cannot submit a GPU occurrence.  Avoid requiring resources for
			// that dormant carrier; clearing fail-closed changes the resource
			// signature and forces a full rebuild above.
			Resource.bOccurrenceVisualSuppressed = true;
		}
		else if (FAILED(Stage_ElementResource(
			*pStagedElement, Resource, strOutError,
			pSharedAssets, fModelPreScale)))
		{
			return false;
		}
		if (nullptr != pMaterialProgramBinding)
		{
			const EFFECT_MATERIAL_EXECUTION_DESC& Execution =
				pMaterialProgramBinding->Execution;
			const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
				pMaterialProgramBinding->Adapter;
			const bool_t bSpriteCarrier =
				Adapter.eCarrier ==
					EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE &&
				Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				Element.SourceRecipe.bEnabled &&
				Element.SourceRecipe.strRendererShape == "sprite" &&
				nullptr == Find_Binding(
					Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
				nullptr == Resource.pModel;
			const bool_t bMeshCarrier =
				Adapter.eCarrier ==
					EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL &&
				Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				Element.SourceRecipe.bEnabled &&
				Element.SourceRecipe.strRendererShape == "mesh" &&
				nullptr != Find_Binding(
					Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
				nullptr != Resource.pModel;
			const bool_t bDecalCarrier =
				Adapter.eCarrier ==
					EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR &&
				Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
				nullptr == Find_Binding(
					Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
				nullptr == Resource.pModel;
			const bool_t bStandardColorStaged = Execution.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
				Resource.iStandardColorV1Enabled == 1u &&
				Resource.StandardColorV1Header[0u] == 1u &&
				Resource.StandardColorV1Header[1u] == Execution.iOpcode &&
				Resource.StandardColorV1Header[2u] ==
					Execution.iTextureLaneCount &&
				Resource.StandardColorV1Header[3u] == Execution.iTextureMask &&
				0u == Resource.iRuntimeMaterialV2Enabled;
			const bool_t bRuntimeMaterialStaged = Execution.eBackend !=
				EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
				0u != Resource.iRuntimeMaterialV2Enabled &&
				Resource.iRuntimeMaterialV2Opcode == Execution.iOpcode &&
				Resource.iRuntimeMaterialV2TextureLaneCount ==
					Execution.iTextureLaneCount &&
				Resource.iRuntimeMaterialV2TextureMask == Execution.iTextureMask &&
				0u == Resource.iStandardColorV1Enabled;
			if (!Is_CompiledMaterialAdapter(Adapter) ||
				(!bSpriteCarrier && !bMeshCarrier && !bDecalCarrier) ||
				Element.Material.eRenderProfile != Adapter.eRenderProfile ||
				Select_Pass(Element.Material.eRenderProfile) !=
					Adapter.iPassIndex ||
				Execution.iPassIndex != Adapter.iPassIndex ||
				Execution.strRasterizerState != Adapter.strRasterizerState ||
				Execution.strDepthStencilState != Adapter.strDepthStencilState ||
				Execution.strBlendState != Adapter.strBlendState ||
				Execution.iStencilReference != 0u ||
				(!bStandardColorStaged && !bRuntimeMaterialStaged))
			{
				strOutError =
					"Bound material-program compiled adapter/carrier changed: " +
					Element.strElementId;
				return false;
			}
			EFFECT_MATERIAL_EXECUTION_DESC PreparedSnapshot;
			if (!Build_MaterialExecutionSnapshot(
					*pStagedElement, Resource, PreparedSnapshot, strOutError) ||
				!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
					pMaterialProgramBinding->Execution, PreparedSnapshot))
			{
				if (strOutError.empty())
				{
					strOutError =
						"Bound material-program prepared snapshot differs from registry: " +
						Element.strElementId;
				}
				return false;
			}
			Resource.pMaterialProgramBinding = pMaterialProgramBinding;
			++Staged->iMaterialProgramResolvedElementCount;
		}
		const EFFECT_VISUAL_PROGRAM_ROW* pVisualProgramRow =
			nullptr == pVisualProgramProjection ? nullptr :
				pVisualProgramProjection->Find_RowByTargetElementId(
					Element.strElementId);
		const bool_t bVisualProgramAdapter = nullptr != pVisualProgramRow &&
			pVisualProgramRow->LocalDecalPacket.has_value();
		bool_t bReconstructedMaterialEvaluatorStaged = false;
		if (nullptr != pProgramEmitter && !bVisualProgramAdapter &&
			!StageReconstructedMaterialEvaluator(*pProgramEmitter, Element,
				Resource, bReconstructedMaterialEvaluatorStaged))
		{
			if (strOutError.empty())
			{
				strOutError =
					"Artist F reconstructed material evaluator failed without a "
					"contract diagnostic at order " +
					std::to_string(pProgramEmitter->Row.iOrder) + ".";
			}
			return false;
		}
		if (bVisualProgramAdapter &&
			!Stage_VisualProgramAdapter(*pVisualProgramRow, Element, Resource,
				strOutError, pSharedAssets))
		{
			return false;
		}
		if (bVisualProgramAdapter)
			++Staged->iVisualProgramAdapterCount;
		if (nullptr != pPreparation && nullptr != pProgramEmitter)
		{
			const auto Registry = Find_Artist31470ShaderRegistry(
				pProgramEmitter->Row.iOrder, pProgramEmitter->strEvidenceId);
			if (!Registry.has_value() ||
				!Validate_Artist31470ShaderRegistryEmitterIdentity(
					pProgramEmitter->Row.iOrder, pProgramEmitter->strEvidenceId,
					Element.strElementId, pProgramEmitter->strSourceEmitterPath) ||
				Registry->eRenderer != pProgramEmitter->eRenderer ||
				Registry->bNativeSelectionAdmitted)
			{
				strOutError = "Artist F shader registry emitter stage changed: " +
					Element.strElementId;
				return false;
			}
			const uint32_t iSelectedPass = Select_Pass(
				Element.Material.eRenderProfile);
			const bool_t bMaterialRenderer = Registry->eRenderer !=
					EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE &&
				Registry->eRenderer != EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST;
			if ((bMaterialRenderer &&
					Registry->iCurrentPassIndex != iSelectedPass) ||
				(!bMaterialRenderer && Registry->iCurrentPassIndex !=
					EFFECT_ARTIST31470_NO_PASS))
			{
				strOutError = "Artist F shader registry pass changed: " +
					Element.strElementId;
				return false;
			}
			if (pProgramEmitter->Row.iOrder == 34u)
			{
				if (pProgramEmitter->strMaterialOccurrenceId.has_value() ||
					Registry->eFidelity !=
						EFFECT_ARTIST31470_SHADER_FIDELITY::NON_CORE_FORBIDDEN ||
					Registry->eBackend != EFFECT_ARTIST31470_SHADER_BACKEND::NONE ||
					Registry->iOpcode != 0u || Registry->bDrawAdmitted ||
					Registry->iCurrentPassIndex != EFFECT_ARTIST31470_NO_PASS)
				{
					strOutError = "Artist F light registry boundary changed.";
					return false;
				}
				Resource.bOccurrenceVisualSuppressed = true;
			}
			else if (pProgramEmitter->Row.iOrder == 32u &&
				(Registry->eFidelity !=
					EFFECT_ARTIST31470_SHADER_FIDELITY::NON_CORE_FORBIDDEN ||
					Registry->eBackend != EFFECT_ARTIST31470_SHADER_BACKEND::NONE ||
					Registry->iOpcode != 0u || Registry->bDrawAdmitted ||
					Registry->iCurrentPassIndex != EFFECT_ARTIST31470_NO_PASS))
			{
				strOutError = "Artist F screen-post registry boundary changed.";
				return false;
			}
			else if (!pProgramEmitter->strMaterialOccurrenceId.has_value())
			{
				strOutError = "Artist F material occurrence registry join is missing: " +
					Element.strElementId;
				return false;
			}
			else if (Registry->eBackend == EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
				!Registry->bDrawAdmitted)
			{
				Resource.bOccurrenceVisualSuppressed = true;
				if (Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED)
					++Staged->iLegacyOccurrenceVisualSuppressedCount;
			}
		}
		if (bReconstructedMaterialEvaluatorStaged)
			++Staged->iReconstructedMaterialEvaluatorCount;
		if (0u != Resource.iRuntimeMaterialV2Enabled)
			++Staged->iRuntimeMaterialV2Count;
		if (0u != Resource.iArtistVisualV4Opcode)
		{
			++Staged->iArtistVisualV4Count;
			if (5u == Resource.iArtistVisualV4Opcode)
			{
				++Staged->iArtistVisualV4UnsupportedCount;
				Resource.bOccurrenceVisualSuppressed = true;
			}
		}
		if (nullptr != pPreparation &&
			Resource.bSourceMaterialFallbackBlocked &&
			Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.reconstructed-standard.v1" &&
			nullptr == Find_Binding(Element, EFFECT_RESOURCE_SLOT::BASE_TEXTURE))
		{
			// This is an explicit nonProduct preview-only neutral provider for the
			// remaining cooked occurrence whose source-default texture was
			// stripped. active023 is owned by its textureless runtime-v2 procedural
			// packet and must never re-enter this white-neutral visibility fallback. The
			// ordinary/Product renderer remains fail-closed.
			Resource.bSourceMaterialFallbackBlocked = false;
			++Staged->iReconstructedNeutralBaseCount;
		}
		if (nullptr != pPreparation &&
			Resource.iSourceMaterialProfile == 5u)
		{
			if (nullptr == Find_Texture(
				Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) ||
				nullptr == Find_Texture(
					Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) ||
				Resource.bSourceMaterialFallbackBlocked)
			{
				strOutError =
					"Reconstructed one-layer profile lost its exact base/noise resources: " +
					Element.strElementId;
				return false;
			}
			++Staged->iReconstructedOneLayerCount;
		}
		if (!Staged->ElementResources.emplace(
			Element.strElementId, std::move(Resource)).second)
		{
			if (strOutError.empty())
				strOutError = "Prepared Effect has a duplicate Element.";
			return false;
		}
	}
	if (nullptr != pVisualProgramProjection &&
		pVisualProgramProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		const size_t iTypedAdapterCount = static_cast<size_t>(std::count_if(
			pVisualProgramProjection->Get_AdmittedRows().begin(),
			pVisualProgramProjection->Get_AdmittedRows().end(),
			[](const EFFECT_VISUAL_PROGRAM_ROW& Row)
			{
				return Row.LocalDecalPacket.has_value();
			}));
		if (0u == iTypedAdapterCount ||
			Staged->iVisualProgramAdapterCount != iTypedAdapterCount)
		{
			strOutError =
				"Visual-program adapter denominator did not map to prepared elements.";
			return false;
		}
	}
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedDocumentBuildCount;
	}
	OutPrepared = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Clone_ModelCueResources(
	const PREPARED_DOCUMENT& Prepared,
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>& OutResources,
	std::string& strOutError) const
{
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> Staged;
	for (const auto& [CueId, Prototype] : Prepared.ModelCuePrototypes)
	{
		if (nullptr == Prototype.pModel)
		{
			strOutError = "Prepared animated Model Cue prototype is missing: " +
				CueId;
			return false;
		}
		const std::shared_ptr<CPrototype> CloneBase =
			Prototype.pModel->Clone(nullptr);
		const std::shared_ptr<Engine::CModel> Model =
			std::dynamic_pointer_cast<Engine::CModel>(CloneBase);
		if (nullptr == Model)
		{
			strOutError = "Prepared animated Model Cue clone failed: " + CueId;
			return false;
		}
		Model->Set_Animation(Prototype.iAnimationIndex, false);
		if (!Model->Set_AnimTrackPosition(
			Prototype.iAnimationIndex, 0.f))
		{
			strOutError = "Prepared animated Model Cue reset failed: " + CueId;
			return false;
		}
		// Play_Animation reports whether a non-looping clip finished.  At the
		// reset position a valid clip therefore returns false; that is not a
		// staging failure.
		Model->Play_Animation(0.f);
		MODEL_CUE_RESOURCE Resource;
		Resource.pModel = Model;
		Resource.iAnimationIndex = Prototype.iAnimationIndex;
		Resource.fTicksPerSecond = Prototype.fTicksPerSecond;
		Resource.fDurationSeconds = Prototype.fDurationSeconds;
		Staged.emplace(CueId, std::move(Resource));
	}
	OutResources = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Validate_PreparedInstanceBuffers(
	const EFFECT_DOCUMENT_DESC& Document,
	const PREPARED_DOCUMENT& Prepared,
	std::string& strOutError) const
{
	uint32_t iRequiredTrailPoints = 0u;
	if (!Try_ResolveTrailBufferPointCapacity(
			Document, iRequiredTrailPoints, strOutError))
	{
		return false;
	}
	if ((Needs_ParticleInstanceBuffer(Document) &&
			nullptr == m_pParticleBuffer) ||
		(0u != iRequiredTrailPoints && nullptr == Prepared.pTrailBuffer) ||
		(0u == iRequiredTrailPoints && nullptr != Prepared.pTrailBuffer))
	{
		strOutError =
			"Prepared Effect mutable buffer contract does not match the Document.";
		return false;
	}
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_Catalog(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const std::vector<std::pair<std::string,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC>>>& Documents,
	std::string& strOutError)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry> Registry =
		CEffectCatalog::Acquire_MaterialProgramRegistry();
	if (nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision)
	{
		strOutError =
			"Effect catalog preparation has no matching immutable "
			"material-program generation.";
		return false;
	}
	std::vector<EFFECT_RENDER_PREWARM_TARGET> Targets;
	Targets.reserve(Documents.size());
	for (const auto& [EffectId, Document] : Documents)
		Targets.push_back({ EffectId, Document, nullptr, Registry });
	return Prepare_VisualProgramCatalog(std::move(pDevice),
		std::move(pContext), iCatalogRevision, Targets, strOutError);
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramCatalog(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const std::vector<EFFECT_RENDER_PREWARM_TARGET>& Targets,
	std::string& strOutError)
{
	const std::shared_ptr<const CEffectMaterialProgramRegistry> Registry =
		Targets.empty() ? nullptr : Targets.front().pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		Targets.empty() || nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		Registry->Get_BindingCount() > UINT32_MAX ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Effect product prewarm arguments or renderer core are invalid.";
		return false;
	}

	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Existing;
	std::shared_ptr<PRODUCT_PREWARM_SESSION> ExistingSession;
	uint64_t iStagedFromGeneration = 0u;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		iStagedFromGeneration = g_iPreparedCatalogGeneration;
		if (g_pPreparedDevice == pDevice.Get() &&
			g_iPreparedCatalogRevision == iCatalogRevision)
		{
			Existing = g_PreparedEffectDocuments;
			if (nullptr != g_pProductPrewarmSession &&
				g_pProductPrewarmSession->pDevice == pDevice.Get() &&
				g_pProductPrewarmSession->pContext == pContext.Get() &&
				g_pProductPrewarmSession->iCatalogRevision == iCatalogRevision &&
				g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
					Registry.get())
			{
				ExistingSession = g_pProductPrewarmSession;
			}
		}
	}
	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Staged;
	std::unordered_map<const EFFECT_DOCUMENT_DESC*,
		std::shared_ptr<const PREPARED_DOCUMENT>> StagedByIdentity;
	PREWARM_ASSET_CACHE SharedAssets = nullptr == ExistingSession ?
		PREWARM_ASSET_CACHE{} : ExistingSession->SharedAssets;
	std::unordered_set<std::string> EffectIds;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	for (const EFFECT_RENDER_PREWARM_TARGET& Target : Targets)
	{
		const std::string& EffectId = Target.strEffectAssetId;
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
			Target.pDocument;
		const std::shared_ptr<const
			EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& Projection =
			Target.pVisualProgramProjection;
		if (EffectId.empty() || nullptr == Document ||
			Target.pMaterialProgramRegistry.get() != Registry.get() ||
			EffectId != Document->strEffectAssetId ||
			(nullptr != Projection &&
			 (!Projection->Is_Valid() ||
			  Projection->Get_EffectAssetId() != EffectId ||
			  Projection->Get_DocumentShared().get() != Document.get())) ||
			!EffectIds.insert(EffectId).second)
		{
			strOutError = "Effect product prewarm contains an invalid or duplicate target.";
			return false;
		}
		const PREPARED_KEY Key{
			iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
			nullptr == Projection ? std::string{} :
				Projection->Get_AdmissionTokenSha256() };
		const auto Reusable = Existing.find(Key);
		if (Reusable != Existing.end() && nullptr != Reusable->second &&
			Reusable->second->pCatalogDocumentIdentity == Document.get() &&
			Reusable->second->pVisualProgramProjection.get() == Projection.get() &&
			Reusable->second->pMaterialProgramRegistry.get() == Registry.get() &&
			Reusable->second->pImmutableDocument.get() == Document.get())
		{
			Staged.emplace(Key, Reusable->second);
			if (!StagedByIdentity.emplace(
					Document.get(), Reusable->second).second)
			{
				strOutError =
					"Effect product prewarm reused one document identity twice.";
				return false;
			}
			continue;
		}
		std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
		if (!Loader.Build_PreparedDocument(
			iCatalogRevision, EffectId, *Document, &SharedAssets,
			Prepared, strOutError, nullptr, Projection, Document, Registry))
		{
			return false;
		}
		if (!StagedByIdentity.emplace(Document.get(), Prepared).second)
		{
			strOutError =
				"Effect product prewarm produced one document identity twice.";
			return false;
		}
		Staged.emplace(Key, std::move(Prepared));
	}

	auto StagedSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	StagedSession->pDevice = pDevice.Get();
	StagedSession->pContext = pContext.Get();
	StagedSession->iCatalogRevision = iCatalogRevision;
	StagedSession->pMaterialProgramRegistry = Registry;
	StagedSession->SharedAssets = std::move(SharedAssets);
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_iPreparedCatalogGeneration != iStagedFromGeneration)
		{
			strOutError =
				"Effect product prewarm cache changed during batch staging.";
			return false;
		}
		g_PreparedEffectDocuments = std::move(Staged);
		g_PreparedEffectDocumentsByIdentity = std::move(StagedByIdentity);
		g_pProductPrewarmSession = std::move(StagedSession);
		g_pPreparedDevice = pDevice.Get();
		g_iPreparedCatalogRevision = iCatalogRevision;
		++g_iPreparedCatalogGeneration;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
			Registry->Get_GenerationId();
		g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
			static_cast<uint32_t>(Registry->Get_BindingCount());
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
		g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
		for (const auto& [Key, Prepared] : g_PreparedEffectDocuments)
		{
			(void)Key;
			if (nullptr != Prepared)
			{
				g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount +=
					Prepared->iMaterialProgramResolvedElementCount;
			}
		}
	}
	strOutError = "Prepared " + std::to_string(Targets.size()) +
		" admitted animation Effect targets for catalog revision " +
		std::to_string(iCatalogRevision) + ".";
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const EFFECT_RENDER_PREWARM_TARGET& Target,
	std::string& strOutError)
{
	const std::string& EffectId = Target.strEffectAssetId;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
		Target.pDocument;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Projection = Target.pVisualProgramProjection;
	const std::shared_ptr<const CEffectMaterialProgramRegistry>& Registry =
		Target.pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		EffectId.empty() || nullptr == Document ||
		nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		Registry->Get_BindingCount() > UINT32_MAX ||
		EffectId != Document->strEffectAssetId ||
		(nullptr != Projection &&
			(!Projection->Is_Valid() ||
			 Projection->Get_EffectAssetId() != EffectId ||
			 Projection->Get_DocumentShared().get() != Document.get())) ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError =
			"Effect incremental Product prewarm target is invalid.";
		return false;
	}

	const PREPARED_KEY Key{
		iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
		nullptr == Projection ? std::string{} :
			Projection->Get_AdmissionTokenSha256() };
	PREWARM_ASSET_CACHE StagedSharedAssets;
	uint64_t iStagedFromGeneration = 0u;
	ID3D11Device* pStagedFromDevice = nullptr;
	uint64_t iStagedFromRevision = 0u;
	bool_t bMergeExisting = false;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		iStagedFromGeneration = g_iPreparedCatalogGeneration;
		pStagedFromDevice = g_pPreparedDevice;
		iStagedFromRevision = g_iPreparedCatalogRevision;
		bMergeExisting = g_pPreparedDevice == pDevice.Get() &&
			g_iPreparedCatalogRevision == iCatalogRevision;
		if (bMergeExisting && nullptr != g_pProductPrewarmSession &&
			(g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			 g_pProductPrewarmSession->pContext != pContext.Get() ||
			 g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			 g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get()))
		{
			strOutError =
				"Effect incremental Product prewarm session identity changed.";
			return false;
		}
		if (bMergeExisting)
		{
			const auto Existing = g_PreparedEffectDocuments.find(Key);
			if (Existing != g_PreparedEffectDocuments.end())
			{
				if (nullptr == Existing->second ||
					Existing->second->pCatalogDocumentIdentity != Document.get() ||
					Existing->second->pVisualProgramProjection.get() !=
						Projection.get() ||
					Existing->second->pMaterialProgramRegistry.get() !=
						Registry.get() ||
					Existing->second->pImmutableDocument.get() != Document.get())
				{
					strOutError =
						"Effect incremental Product prewarm duplicate identity diverged.";
					return false;
				}
				strOutError = "Product Effect target is already prepared.";
				return true;
			}
			const auto SameId = std::find_if(
				g_PreparedEffectDocuments.begin(),
				g_PreparedEffectDocuments.end(),
				[&EffectId, iCatalogRevision](const auto& Entry)
				{
					return Entry.first.iCatalogRevision == iCatalogRevision &&
						Entry.first.strEffectAssetId == EffectId;
				});
			if (SameId != g_PreparedEffectDocuments.end())
			{
				strOutError =
					"Effect incremental Product prewarm found a stale target identity.";
				return false;
			}
			if (nullptr != g_pProductPrewarmSession)
				StagedSharedAssets = g_pProductPrewarmSession->SharedAssets;
		}
	}

	CEffectDocumentRenderer Loader(pDevice, pContext);
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Loader.Build_PreparedDocument(
		iCatalogRevision, EffectId, *Document, &StagedSharedAssets,
		Prepared, strOutError, nullptr, Projection, Document, Registry))
	{
		return false;
	}
	auto StagedSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	StagedSession->pDevice = pDevice.Get();
	StagedSession->pContext = pContext.Get();
	StagedSession->iCatalogRevision = iCatalogRevision;
	StagedSession->pMaterialProgramRegistry = Registry;
	StagedSession->SharedAssets = std::move(StagedSharedAssets);

	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_iPreparedCatalogGeneration != iStagedFromGeneration ||
			g_pPreparedDevice != pStagedFromDevice ||
			g_iPreparedCatalogRevision != iStagedFromRevision)
		{
			strOutError =
				"Effect incremental Product prewarm cache changed during staging.";
			return false;
		}
		std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>>
			StagedDocuments = bMergeExisting ?
				g_PreparedEffectDocuments :
				std::map<PREPARED_KEY,
					std::shared_ptr<const PREPARED_DOCUMENT>>{};
		std::unordered_map<const EFFECT_DOCUMENT_DESC*,
			std::shared_ptr<const PREPARED_DOCUMENT>> StagedByIdentity =
			bMergeExisting ? g_PreparedEffectDocumentsByIdentity :
				std::unordered_map<const EFFECT_DOCUMENT_DESC*,
					std::shared_ptr<const PREPARED_DOCUMENT>>{};
		if (!StagedDocuments.emplace(Key, Prepared).second ||
			!StagedByIdentity.emplace(Document.get(), Prepared).second)
		{
			strOutError =
				"Effect incremental Product prewarm merge identity is duplicate.";
			return false;
		}
		g_PreparedEffectDocuments = std::move(StagedDocuments);
		g_PreparedEffectDocumentsByIdentity = std::move(StagedByIdentity);
		g_pProductPrewarmSession = std::move(StagedSession);
		g_pPreparedDevice = pDevice.Get();
		g_iPreparedCatalogRevision = iCatalogRevision;
		++g_iPreparedCatalogGeneration;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
			Registry->Get_GenerationId();
		g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
			static_cast<uint32_t>(Registry->Get_BindingCount());
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
		g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
		for (const auto& [PreparedKey, PreparedEntry] :
			g_PreparedEffectDocuments)
		{
			(void)PreparedKey;
			if (nullptr != PreparedEntry)
			{
				g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount +=
					PreparedEntry->iMaterialProgramResolvedElementCount;
			}
		}
	}
	strOutError = "Incrementally prepared Product Effect target " + EffectId +
		" for catalog revision " + std::to_string(iCatalogRevision) + ".";
	return true;
}

bool_t Client::CEffectDocumentRenderer::Replace_VisualProgramTarget(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint64_t iCatalogRevision,
	const EFFECT_RENDER_PREWARM_TARGET& Target,
	std::string& strOutError)
{
	const std::string& EffectId = Target.strEffectAssetId;
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC>& Document =
		Target.pDocument;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		Projection = Target.pVisualProgramProjection;
	const std::shared_ptr<const CEffectMaterialProgramRegistry>& Registry =
		Target.pMaterialProgramRegistry;
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		EffectId.empty() || nullptr == Document ||
		nullptr == Registry ||
		Registry->Get_CatalogRevision() != iCatalogRevision ||
		EffectId != Document->strEffectAssetId ||
		(nullptr != Projection &&
			(!Projection->Is_Valid() ||
			 Projection->Get_EffectAssetId() != EffectId ||
			 Projection->Get_DocumentShared().get() != Document.get())) ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Effect Product replacement target is invalid.";
		return false;
	}

	const PREPARED_KEY CandidateKey{
		iCatalogRevision, EffectId, Build_ResourceSignature(*Document),
		nullptr == Projection ? std::string{} :
			Projection->Get_AdmissionTokenSha256() };
	PREWARM_ASSET_CACHE StagedSharedAssets;
	uint64_t iStagedFromGeneration = 0u;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		iStagedFromGeneration = g_iPreparedCatalogGeneration;
		if (g_pPreparedDevice != pDevice.Get() ||
			g_iPreparedCatalogRevision != iCatalogRevision ||
			nullptr == g_pProductPrewarmSession ||
			g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			g_pProductPrewarmSession->pContext != pContext.Get() ||
			g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get())
		{
			strOutError =
				"Effect Product replacement has no matching prepared session.";
			return false;
		}
		StagedSharedAssets = g_pProductPrewarmSession->SharedAssets;
	}

	CEffectDocumentRenderer Loader(pDevice, pContext);
	std::shared_ptr<const PREPARED_DOCUMENT> Candidate;
	if (!Loader.Build_PreparedDocument(
		iCatalogRevision, EffectId, *Document, &StagedSharedAssets,
		Candidate, strOutError, nullptr, Projection, Document, Registry))
	{
		return false;
	}
	auto StagedSession = std::make_shared<PRODUCT_PREWARM_SESSION>();
	StagedSession->pDevice = pDevice.Get();
	StagedSession->pContext = pContext.Get();
	StagedSession->iCatalogRevision = iCatalogRevision;
	StagedSession->pMaterialProgramRegistry = Registry;
	StagedSession->SharedAssets = std::move(StagedSharedAssets);

	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_iPreparedCatalogGeneration != iStagedFromGeneration ||
			g_pPreparedDevice != pDevice.Get() ||
			g_iPreparedCatalogRevision != iCatalogRevision ||
			nullptr == g_pProductPrewarmSession ||
			g_pProductPrewarmSession->pDevice != pDevice.Get() ||
			g_pProductPrewarmSession->pContext != pContext.Get() ||
			g_pProductPrewarmSession->iCatalogRevision != iCatalogRevision ||
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() !=
				Registry.get())
		{
			strOutError =
				"Effect Product prepared cache changed during replacement staging.";
			return false;
		}

		auto StagedDocuments = g_PreparedEffectDocuments;
		auto StagedByIdentity = g_PreparedEffectDocumentsByIdentity;
		for (auto Iterator = StagedDocuments.begin();
			Iterator != StagedDocuments.end();)
		{
			if (Iterator->first.iCatalogRevision == iCatalogRevision &&
				Iterator->first.strEffectAssetId == EffectId)
			{
				Iterator = StagedDocuments.erase(Iterator);
			}
			else
			{
				++Iterator;
			}
		}
		for (auto Iterator = StagedByIdentity.begin();
			Iterator != StagedByIdentity.end();)
		{
			const std::shared_ptr<const PREPARED_DOCUMENT>& Prepared =
				Iterator->second;
			if (nullptr != Prepared &&
				Prepared->iCatalogRevision == iCatalogRevision &&
				Prepared->strEffectAssetId == EffectId)
			{
				Iterator = StagedByIdentity.erase(Iterator);
			}
			else
			{
				++Iterator;
			}
		}
		if (!StagedDocuments.emplace(CandidateKey, Candidate).second ||
			!StagedByIdentity.emplace(Document.get(), Candidate).second)
		{
			strOutError =
				"Effect Product replacement candidate identity is duplicate.";
			return false;
		}

		g_PreparedEffectDocuments = std::move(StagedDocuments);
		g_PreparedEffectDocumentsByIdentity = std::move(StagedByIdentity);
		g_pProductPrewarmSession = std::move(StagedSession);
		++g_iPreparedCatalogGeneration;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration =
			Registry->Get_GenerationId();
		g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount =
			static_cast<uint32_t>(Registry->Get_BindingCount());
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
		g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
		for (const auto& [PreparedKey, PreparedEntry] :
			g_PreparedEffectDocuments)
		{
			(void)PreparedKey;
			if (nullptr != PreparedEntry)
			{
				g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount +=
					PreparedEntry->iMaterialProgramResolvedElementCount;
			}
		}
	}
	strOutError = "Replaced prepared Product Effect target " + EffectId +
		" for catalog revision " + std::to_string(iCatalogRevision) + ".";
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_ReconstructedSourceRuntime(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() ||
		Document.strEffectAssetId !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError =
			"Reconstructed source runtime prewarm identity is invalid.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary Boundary;
	if (!Boundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u, Document.strEffectAssetId,
		Document, &SharedAssets, OutPrepared, strOutError, pPreparation))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		OutPrepared->iReconstructedNeutralBaseCount != 0u ||
		OutPrepared->iReconstructedOneLayerCount != 1u ||
		OutPrepared->iReconstructedMaterialEvaluatorCount != 28u ||
		OutPrepared->iRuntimeMaterialV2Count != 18u ||
		OutPrepared->iArtistVisualV4Count != 10u ||
		OutPrepared->iArtistVisualV4UnsupportedCount != 0u ||
		OutPrepared->iLegacyOccurrenceVisualSuppressedCount != 4u)
	{
		const uint32_t iNeutralCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedNeutralBaseCount;
		const uint32_t iOneLayerCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedOneLayerCount;
		const uint32_t iEvaluatorCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedMaterialEvaluatorCount;
		const uint32_t iRuntimeV2Count = nullptr == OutPrepared ? 0u :
			OutPrepared->iRuntimeMaterialV2Count;
		const uint32_t iArtistVisualV4Count = nullptr == OutPrepared ? 0u :
			OutPrepared->iArtistVisualV4Count;
		const uint32_t iArtistVisualV4UnsupportedCount =
			nullptr == OutPrepared ? 0u :
			OutPrepared->iArtistVisualV4UnsupportedCount;
		const uint32_t iLegacyOccurrenceVisualSuppressedCount =
			nullptr == OutPrepared ? 0u :
			OutPrepared->iLegacyOccurrenceVisualSuppressedCount;
		OutPrepared.reset();
		strOutError =
			"Reconstructed source runtime prewarm lost its immutable authority "
			"or exact material evaluator denominator: neutral=" +
			std::to_string(iNeutralCount) + ", oneLayer=" +
			std::to_string(iOneLayerCount) + ", evaluator=" +
			std::to_string(iEvaluatorCount) + ", runtimeV2=" +
			std::to_string(iRuntimeV2Count) + ", artistVisualV4=" +
			std::to_string(iArtistVisualV4Count) + ", artistVisualV4Unsupported=" +
			std::to_string(iArtistVisualV4UnsupportedCount) +
			", legacyVisualSuppressed=" +
			std::to_string(iLegacyOccurrenceVisualSuppressedCount) + ".";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Bake_ReconstructedMaterialExecutionSnapshots(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>&
			OutSnapshots,
		std::string& strOutError)
{
	OutSnapshots.clear();
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Prepare_ReconstructedSourceRuntime(pDevice, pContext, pPreparation,
		SourceDocument, Prepared, strOutError) || nullptr == Prepared)
	{
		return false;
	}

	CEffectDocumentRenderer Compiler(std::move(pDevice), std::move(pContext));
	std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC> Staged;
	Staged.reserve(SourceDocument.Elements.size());
	uint32_t iRuntimeMaterialV2Count = 0u;
	uint32_t iArtistVisualV4Count = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		const auto ResourceIt = Prepared->ElementResources.find(
			Element.strElementId);
		if (Element.strElementId.empty() ||
			ResourceIt == Prepared->ElementResources.end())
		{
			strOutError =
				"Prepared reconstructed material has no source Element resource: " +
				Element.strElementId;
			return false;
		}

		EFFECT_MATERIAL_EXECUTION_DESC Snapshot;
		std::string strElementError;
		const bool_t bTyped = Compiler.Build_MaterialExecutionSnapshot(
			Element, ResourceIt->second, Snapshot, strElementError);
		if (!bTyped && !strElementError.empty())
		{
			strOutError = std::move(strElementError);
			return false;
		}
		if (!bTyped)
			Snapshot = {};
		else if (Snapshot.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
		{
			++iArtistVisualV4Count;
		}
		else if (Snapshot.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 ||
			 Snapshot.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL)
		{
			++iRuntimeMaterialV2Count;
		}
		else
		{
			strOutError =
				"Prepared reconstructed material selected an unsupported authored "
				"backend: " + Element.strElementId;
			return false;
		}
		if (!Staged.emplace(Element.strElementId, std::move(Snapshot)).second)
		{
			strOutError =
				"Reconstructed material bake contains a duplicate Element ID: " +
				Element.strElementId;
			return false;
		}
	}
	if (Staged.size() != SourceDocument.Elements.size() ||
		iRuntimeMaterialV2Count != 18u || iArtistVisualV4Count != 10u)
	{
		strOutError =
			"Reconstructed material bake lost its typed denominator: elements=" +
			std::to_string(Staged.size()) + ", runtimeMaterialV2=" +
			std::to_string(iRuntimeMaterialV2Count) + ", artistVisualV4=" +
			std::to_string(iArtistVisualV4Count) + ".";
		return false;
	}
	OutSnapshots = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Prepare_VisualProgramDocument(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pProjection ||
		!pProjection->Is_Valid() ||
		pProjection->Get_EffectAssetId() !=
			pProjection->Get_Document().strEffectAssetId ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Visual-program renderer prewarm identity is invalid.";
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u,
		pProjection->Get_EffectAssetId(), pProjection->Get_Document(),
		&SharedAssets, OutPrepared, strOutError, nullptr, pProjection))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pVisualProgramProjection.get() != pProjection.get())
	{
		OutPrepared.reset();
		strOutError = "Visual-program renderer prewarm lost its immutable token.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Prepare_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPreparation ||
		nullptr == pPreparation->Get_Program() || nullptr == pProjection ||
		!pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		pProjection->Get_EffectAssetId() !=
			pPreparation->Get_Program()->strRuntimeCatalogAssetId ||
		pProjection->Get_Document().strEffectAssetId !=
			pProjection->Get_EffectAssetId() ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError =
			"Reconstructed visual-adapter renderer prewarm identity is invalid.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary Boundary;
	if (!Boundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(0u,
		pProjection->Get_EffectAssetId(), pProjection->Get_Document(),
		&SharedAssets, OutPrepared, strOutError, pPreparation, pProjection))
	{
		return false;
	}
	if (nullptr == OutPrepared ||
		OutPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		OutPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		0u == OutPrepared->iVisualProgramAdapterCount)
	{
		OutPrepared.reset();
		strOutError =
			"Reconstructed visual-adapter prewarm lost its immutable authorities.";
		return false;
	}
	strOutError.clear();
	return true;
}

std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
Client::CEffectDocumentRenderer::Find_Prepared(
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProgramProjection,
	std::shared_ptr<const CEffectMaterialProgramRegistry>
		pMaterialProgramRegistry)
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	++g_EffectRenderPrewarmProbe.iPreparedIdentityLookupCount;
	const auto Iterator = g_PreparedEffectDocumentsByIdentity.find(&Document);
	if (g_iPreparedCatalogRevision != iCatalogRevision ||
		Iterator == g_PreparedEffectDocumentsByIdentity.end() ||
		nullptr == Iterator->second ||
		Iterator->second->iCatalogRevision != iCatalogRevision ||
		Iterator->second->strEffectAssetId != strEffectAssetId ||
		Iterator->second->pCatalogDocumentIdentity != &Document ||
		Iterator->second->pImmutableDocument.get() != &Document ||
		Iterator->second->pMaterialProgramRegistry.get() !=
			pMaterialProgramRegistry.get() ||
		(nullptr != pMaterialProgramRegistry &&
			(pMaterialProgramRegistry->Get_CatalogRevision() != iCatalogRevision ||
			 Iterator->second->iMaterialProgramRegistryGeneration !=
				pMaterialProgramRegistry->Get_GenerationId())) ||
		Iterator->second->pVisualProgramProjection.get() !=
			pVisualProgramProjection.get())
	{
		++g_EffectRenderPrewarmProbe.iPreparedLookupMissCount;
		return nullptr;
	}
	return Iterator->second;
}

std::shared_ptr<const Client::CEffectPlayback::PREPARED_RESOURCES>
Client::CEffectDocumentRenderer::Get_PlaybackResources(
	const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared)
{
	return nullptr == pPrepared ? nullptr : pPrepared->pPlaybackResources;
}

Client::EFFECT_RENDER_PREWARM_PROBE
Client::CEffectDocumentRenderer::Get_PrewarmProbe()
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	EFFECT_RENDER_PREWARM_PROBE Probe = g_EffectRenderPrewarmProbe;
	Probe.iVectorFieldDiskLoadCount =
		CEffectPlayback::Get_VectorFieldDiskLoadCount();
	return Probe;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
bool_t Client::CEffectDocumentRenderer::
	Prepare_UnboundMaterialProgramDocumentForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_DOCUMENT_DESC> pDocument,
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pVisualProgramProjection,
		std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
		std::string& strOutError)
{
	OutPrepared.reset();
	if (nullptr == pDevice || nullptr == pContext || nullptr == pDocument ||
		(nullptr != pVisualProgramProjection &&
			(!pVisualProgramProjection->Is_Valid() ||
			 pVisualProgramProjection->Get_EffectAssetId() !=
				pDocument->strEffectAssetId ||
			 pVisualProgramProjection->Get_DocumentShared().get() !=
				pDocument.get())) ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Binding0 comparison preparation identity is invalid.";
		return false;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	if (!Loader.Build_PreparedDocument(
			0u, pDocument->strEffectAssetId, *pDocument, &SharedAssets,
			OutPrepared, strOutError, nullptr, pVisualProgramProjection) ||
		nullptr == OutPrepared || 0u != OutPrepared->iCatalogRevision ||
		0u != OutPrepared->iMaterialProgramRegistryGeneration ||
		nullptr != OutPrepared->pMaterialProgramRegistry ||
		0u != OutPrepared->iMaterialProgramResolvedElementCount)
	{
		OutPrepared.reset();
		if (strOutError.empty())
			strOutError = "Binding0 comparison preparation admitted a registry Binding.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Validate_MaterialProgramPreparedComparisonForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pUnboundPrepared,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pBoundPrepared,
		const std::string_view strElementId,
		std::string& strOutError)
{
	if (nullptr == pDevice || nullptr == pContext ||
		nullptr == pUnboundPrepared || nullptr == pBoundPrepared ||
		strElementId.empty() ||
		pUnboundPrepared->strEffectAssetId != pBoundPrepared->strEffectAssetId ||
		pUnboundPrepared->iResourceSignature != pBoundPrepared->iResourceSignature ||
		nullptr != pUnboundPrepared->pMaterialProgramRegistry ||
		0u != pUnboundPrepared->iMaterialProgramResolvedElementCount ||
		nullptr == pBoundPrepared->pMaterialProgramRegistry ||
		pBoundPrepared->iMaterialProgramResolvedElementCount == 0u)
	{
		strOutError = "Binding0/Binding1 prepared comparison identity is invalid.";
		return false;
	}
	const EFFECT_DOCUMENT_DESC& UnboundDocument =
		nullptr == pUnboundPrepared->pImmutableDocument ?
			pUnboundPrepared->ResourceDocument :
			*pUnboundPrepared->pImmutableDocument;
	const EFFECT_DOCUMENT_DESC& BoundDocument =
		nullptr == pBoundPrepared->pImmutableDocument ?
			pBoundPrepared->ResourceDocument : *pBoundPrepared->pImmutableDocument;
	const auto FindElement = [strElementId](const EFFECT_DOCUMENT_DESC& Document)
	{
		return std::find_if(Document.Elements.begin(), Document.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
	};
	const auto UnboundElement = FindElement(UnboundDocument);
	const auto BoundElement = FindElement(BoundDocument);
	const auto UnboundResource = pUnboundPrepared->ElementResources.find(
		std::string(strElementId));
	const auto BoundResource = pBoundPrepared->ElementResources.find(
		std::string(strElementId));
	if (UnboundElement == UnboundDocument.Elements.end() ||
		BoundElement == BoundDocument.Elements.end() ||
		UnboundResource == pUnboundPrepared->ElementResources.end() ||
		BoundResource == pBoundPrepared->ElementResources.end() ||
		nullptr != UnboundResource->second.pMaterialProgramBinding ||
		nullptr == BoundResource->second.pMaterialProgramBinding)
	{
		strOutError = "Binding0/Binding1 occurrence resource closure is invalid.";
		return false;
	}
	CEffectDocumentRenderer Compiler(std::move(pDevice), std::move(pContext));
	EFFECT_MATERIAL_EXECUTION_DESC UnboundSnapshot;
	EFFECT_MATERIAL_EXECUTION_DESC BoundSnapshot;
	if (!Compiler.Build_MaterialExecutionSnapshot(
			*UnboundElement, UnboundResource->second, UnboundSnapshot,
			strOutError) ||
		!Compiler.Build_MaterialExecutionSnapshot(
			*BoundElement, BoundResource->second, BoundSnapshot, strOutError) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			UnboundElement->Material.Execution,
			BoundElement->Material.Execution) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			UnboundSnapshot, BoundSnapshot) ||
		!CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
			BoundResource->second.pMaterialProgramBinding->Execution,
			BoundSnapshot))
	{
		if (strOutError.empty())
		{
			strOutError =
				"Binding0/Binding1 execution packet or prepared snapshot diverged.";
		}
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Probe_ModelCueCloneAnimationForTests(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const std::shared_ptr<const PREPARED_DOCUMENT>& pPrepared,
		const std::string_view strCueId,
		const std::string_view strWitnessBoneName,
		EFFECT_MODEL_CUE_ANIMATION_PROBE& OutProbe,
		std::string& strOutError)
{
	OutProbe = {};
	if (nullptr == pDevice || nullptr == pContext || nullptr == pPrepared ||
		strCueId.empty() || strWitnessBoneName.empty())
	{
		strOutError = "Model Cue clone animation probe identity is invalid.";
		return false;
	}
	const EFFECT_DOCUMENT_DESC& Document =
		nullptr == pPrepared->pImmutableDocument ?
			pPrepared->ResourceDocument : *pPrepared->pImmutableDocument;
	const auto Cue = std::find_if(Document.ModelCues.begin(),
		Document.ModelCues.end(), [strCueId](const EFFECT_MODEL_CUE_DESC& Candidate)
		{
			return Candidate.strCueId == strCueId;
		});
	const std::string CueId(strCueId);
	const auto Prototype = pPrepared->ModelCuePrototypes.find(CueId);
	if (Cue == Document.ModelCues.end() || !Cue->bHoldLastFrame ||
		Prototype == pPrepared->ModelCuePrototypes.end() ||
		nullptr == Prototype->second.pModel)
	{
		strOutError =
			"Hold-last Model Cue prototype is unavailable for the animation probe: " +
			CueId;
		return false;
	}

	CEffectDocumentRenderer Renderer(std::move(pDevice), std::move(pContext));
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> Clones;
	if (!Renderer.Clone_ModelCueResources(*pPrepared, Clones, strOutError))
		return false;
	const auto Clone = Clones.find(CueId);
	if (Clone == Clones.end() || nullptr == Clone->second.pModel)
	{
		strOutError = "Model Cue clone is unavailable for the animation probe: " +
			CueId;
		return false;
	}

	const MODEL_CUE_RESOURCE& PrototypeResource = Prototype->second;
	const MODEL_CUE_RESOURCE& CloneResource = Clone->second;
	OutProbe.fPrototypeDurationSeconds = PrototypeResource.fDurationSeconds;
	OutProbe.fCloneDurationSeconds = CloneResource.fDurationSeconds;
	if (!std::isfinite(PrototypeResource.fDurationSeconds) ||
		PrototypeResource.fDurationSeconds <= 0.f ||
		!std::isfinite(CloneResource.fDurationSeconds) ||
		CloneResource.fDurationSeconds <= 0.f ||
		std::fabs(PrototypeResource.fDurationSeconds -
			CloneResource.fDurationSeconds) > 0.00001f ||
		!std::isfinite(CloneResource.fTicksPerSecond) ||
		CloneResource.fTicksPerSecond <= 0.f)
	{
		strOutError = "Model Cue clone did not preserve finite clip timing: " +
			CueId;
		return false;
	}

	Engine::CModel& Model = *CloneResource.pModel;
	const std::string WitnessBoneName(strWitnessBoneName);
	if (!Model.Has_Bone(WitnessBoneName.c_str()))
	{
		strOutError = "Model Cue animation witness bone is unavailable: " +
			WitnessBoneName;
		return false;
	}
	if (!Model.Set_AnimTrackPosition(CloneResource.iAnimationIndex, 0.f))
	{
		strOutError = "Model Cue animation probe could not reset its track: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	float4x4_t StartBone{};
	XMStoreFloat4x4(&StartBone, Model.Get_BoneMatrix(WitnessBoneName.c_str()));

	const f32_t fMidAnimationSeconds = 0.5f * (std::min)(
		Cue->fDurationSeconds, CloneResource.fDurationSeconds);
	const f32_t fExpectedMidTrackTicks =
		fMidAnimationSeconds * CloneResource.fTicksPerSecond;
	if (!std::isfinite(fExpectedMidTrackTicks) ||
		fExpectedMidTrackTicks <= 0.f ||
		!Model.Set_AnimTrackPosition(
			CloneResource.iAnimationIndex, fExpectedMidTrackTicks))
	{
		strOutError = "Model Cue animation probe could not advance its track: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	f32_t fMidTrackPosition = 0.f;
	f32_t fTrackDuration = 0.f;
	if (!Model.Get_AnimationProgress(CloneResource.iAnimationIndex,
			fMidTrackPosition, fTrackDuration))
	{
		strOutError = "Model Cue animation probe could not read its track: " +
			CueId;
		return false;
	}
	if (!std::isfinite(fMidTrackPosition) ||
		!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
	{
		strOutError = "Model Cue animation probe read non-finite track timing: " +
			CueId;
		return false;
	}
	OutProbe.fMidTrackPositionTicks = fMidTrackPosition;
	OutProbe.fTrackDurationTicks = fTrackDuration;
	float4x4_t MidBone{};
	XMStoreFloat4x4(&MidBone, Model.Get_BoneMatrix(WitnessBoneName.c_str()));
	for (size_t iRow = 0u; iRow < 4u; ++iRow)
	{
		for (size_t iColumn = 0u; iColumn < 4u; ++iColumn)
		{
			if (!std::isfinite(StartBone.m[iRow][iColumn]) ||
				!std::isfinite(MidBone.m[iRow][iColumn]))
			{
				strOutError =
					"Model Cue animation witness matrix is non-finite: " +
					WitnessBoneName;
				return false;
			}
			OutProbe.fWitnessBoneMaximumDelta = (std::max)(
				OutProbe.fWitnessBoneMaximumDelta,
				std::fabs(StartBone.m[iRow][iColumn] -
					MidBone.m[iRow][iColumn]));
		}
	}

	const f32_t fExpectedTailTrackTicks = (std::min)(
		Cue->fDurationSeconds, CloneResource.fDurationSeconds) *
		CloneResource.fTicksPerSecond;
	if (!Model.Set_AnimTrackPosition(
			CloneResource.iAnimationIndex, fExpectedTailTrackTicks))
	{
		strOutError = "Model Cue animation probe could not clamp its tail: " +
			CueId;
		return false;
	}
	Model.Play_Animation(0.f);
	if (!Model.Get_AnimationProgress(CloneResource.iAnimationIndex,
			OutProbe.fTailTrackPositionTicks, fTrackDuration) ||
		!std::isfinite(OutProbe.fTailTrackPositionTicks) ||
		!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f ||
		std::fabs(OutProbe.fMidTrackPositionTicks -
			fExpectedMidTrackTicks) > 0.001f ||
		std::fabs(OutProbe.fTailTrackPositionTicks -
			fExpectedTailTrackTicks) > 0.001f ||
		std::fabs(OutProbe.fTrackDurationTicks - fTrackDuration) > 0.001f ||
		OutProbe.fWitnessBoneMaximumDelta <= 0.0001f)
	{
		strOutError =
			"Model Cue clone animation did not advance or clamp as authored: " +
			CueId;
		return false;
	}
	strOutError.clear();
	return true;
}

#endif

void Client::CEffectDocumentRenderer::Clear_Prepared_Catalog()
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	g_PreparedEffectDocuments.clear();
	g_PreparedEffectDocumentsByIdentity.clear();
	g_pProductPrewarmSession.reset();
	g_EffectRendererCores.clear();
	g_pPreparedDevice = nullptr;
	g_iPreparedCatalogRevision = 0u;
	++g_iPreparedCatalogGeneration;
	g_EffectRenderPrewarmProbe.iCatalogRevision = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramRegistryGeneration = 0u;
	g_EffectRenderPrewarmProbe.iPreparedDocumentCount = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramBindingCount = 0u;
	g_EffectRenderPrewarmProbe.iMaterialProgramResolvedElementCount = 0u;
}

bool_t Client::CEffectDocumentRenderer::Stage_Prepared(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	return Stage_PreparedInternal(Document, std::move(pPrepared), strOutError);
}

bool_t Client::CEffectDocumentRenderer::Stage_PreparedInternal(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	const bool_t bCatalogPrepared = nullptr != pPrepared &&
		0u != pPrepared->iCatalogRevision;
	bool_t bCatalogIdentityCurrent = true;
	if (bCatalogPrepared)
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto Current = g_PreparedEffectDocumentsByIdentity.find(&Document);
		bCatalogIdentityCurrent =
			g_iPreparedCatalogRevision == pPrepared->iCatalogRevision &&
			nullptr != g_pProductPrewarmSession &&
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
				pPrepared->pMaterialProgramRegistry.get() &&
			nullptr != pPrepared->pMaterialProgramRegistry &&
			pPrepared->pMaterialProgramRegistry->Get_CatalogRevision() ==
				pPrepared->iCatalogRevision &&
			pPrepared->pMaterialProgramRegistry->Get_GenerationId() ==
				pPrepared->iMaterialProgramRegistryGeneration &&
			Current != g_PreparedEffectDocumentsByIdentity.end() &&
			Current->second.get() == pPrepared.get();
	}
	const bool_t bIdentityMatches = bCatalogPrepared ?
		(pPrepared->pCatalogDocumentIdentity == &Document &&
		 pPrepared->pImmutableDocument.get() == &Document) :
		(nullptr != pPrepared &&
			pPrepared->iResourceSignature == Build_ResourceSignature(Document) &&
			Resource_SignatureMatches(
				pPrepared->ResourceDocument, Document));
	if (nullptr == pPrepared || !bCatalogIdentityCurrent || !bIdentityMatches ||
		pPrepared->strEffectAssetId != Document.strEffectAssetId ||
		(!bCatalogPrepared &&
			!CEffectDocumentCodec::Validate_Drawable(Document, strOutError)))
	{
		if (strOutError.empty())
			strOutError = "Prepared Effect resources do not match the Document.";
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			Document, *pPrepared, strOutError))
		return false;
	if (!Clone_ModelCueResources(
		*pPrepared, StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = bCatalogPrepared ? EFFECT_DOCUMENT_DESC{} : Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "Prepared Effect Document resources attached.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_PrevalidatedVisualProgramDocument(
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		nullptr == pPrepared ||
		pPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		pPrepared->strEffectAssetId != pProjection->Get_EffectAssetId())
	{
		strOutError =
			"Visual-program renderer resources or immutable token do not match.";
		return false;
	}
	const bool_t bCatalogPrepared = nullptr != pPrepared &&
		0u != pPrepared->iCatalogRevision;
	bool_t bCatalogIdentityCurrent = true;
	if (bCatalogPrepared)
	{
		const EFFECT_DOCUMENT_DESC& Document = pProjection->Get_Document();
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		const auto Current = g_PreparedEffectDocumentsByIdentity.find(&Document);
		bCatalogIdentityCurrent =
			g_iPreparedCatalogRevision == pPrepared->iCatalogRevision &&
			nullptr != g_pProductPrewarmSession &&
			g_pProductPrewarmSession->pMaterialProgramRegistry.get() ==
				pPrepared->pMaterialProgramRegistry.get() &&
			nullptr != pPrepared->pMaterialProgramRegistry &&
			pPrepared->pMaterialProgramRegistry->Get_CatalogRevision() ==
				pPrepared->iCatalogRevision &&
			pPrepared->pMaterialProgramRegistry->Get_GenerationId() ==
				pPrepared->iMaterialProgramRegistryGeneration &&
			Current != g_PreparedEffectDocumentsByIdentity.end() &&
			Current->second.get() == pPrepared.get();
	}
	const bool_t bDocumentIdentityMatches = bCatalogPrepared ?
		(nullptr != pPrepared->pImmutableDocument &&
		 pPrepared->pImmutableDocument.get() ==
			&pProjection->Get_Document() &&
		 pPrepared->pCatalogDocumentIdentity ==
			&pProjection->Get_Document()) :
		(nullptr != pPrepared &&
		 pPrepared->iResourceSignature ==
			Build_ResourceSignature(pProjection->Get_Document()) &&
		 Resource_SignatureMatches(
			pPrepared->ResourceDocument, pProjection->Get_Document()));
	if (!bCatalogIdentityCurrent || !bDocumentIdentityMatches)
	{
		strOutError =
			"Visual-program renderer resources or immutable token do not match.";
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE> StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			pProjection->Get_Document(), *pPrepared, strOutError) ||
		!Clone_ModelCueResources(
			*pPrepared, StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = bCatalogPrepared ? EFFECT_DOCUMENT_DESC{} :
		pProjection->Get_Document();
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	/* Source-module execution is enabled only by an admitted overlay program.
	   Adapter packets (for example LocalDecal) reuse the base playback document
	   and admit renderer material/projector state only. */
	m_bSourceVisualProgramActive =
		pProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "Admitted source visual-program renderer attached.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iSynchronousDocumentStageCount;
	}
	if (!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
		return false;
	if (nullptr != m_pPreparedDocument &&
		0u == m_pPreparedDocument->iCatalogRevision &&
		Resource_SignatureMatches(m_Document, Document))
	{
		m_Document = Document;
		m_pReconstructedDiagnostic.reset();
		m_ReconstructedRuntimeBoundary.Clear();
		m_bReconstructedSourceRuntimeActive = false;
		m_bSourceVisualProgramActive = false;
		Reset_PreviewSubmissionIsolation();
		m_strRenderFailureDetail.clear();
		m_bLastRenderFailureObjectLocal = false;
		m_strStatus = "Effect Document values committed; GPU resources reused.";
		strOutError.clear();
		return true;
	}
	PREWARM_ASSET_CACHE SharedAssets;
	std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
	if (!Build_PreparedDocument(0u, Document.strEffectAssetId, Document,
		&SharedAssets, Prepared, strOutError))
	{
		return false;
	}
	return Stage_PreparedInternal(Document, std::move(Prepared), strOutError);
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedRuntimeProgram(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(std::move(pPreparation),
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
		return false;
	m_Document = {};
	m_pPreparedDocument.reset();
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources.clear();
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed Effect program prepared; renderer execution remains blocked.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedSourceRuntime(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> pPreparation,
	std::string& strOutError)
{
	if (nullptr == pPrepared || nullptr == pPreparation ||
		pPrepared->pReconstructedRuntimePreparation.get() != pPreparation.get() ||
		pPrepared->strEffectAssetId != Document.strEffectAssetId ||
		pPrepared->iResourceSignature != Build_ResourceSignature(Document) ||
		!Resource_SignatureMatches(pPrepared->ResourceDocument, Document))
	{
		strOutError =
			"Reconstructed source runtime resources or authority do not match.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			Document, *pPrepared, strOutError) ||
		!Clone_ModelCueResources(*pPrepared,
			StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = true;
	m_bSourceVisualProgramActive = true;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed Artist source runtime resources attached; Product remains blocked.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::
	Stage_ReconstructedSourceRuntimeWithVisualProgramAdapter(
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection,
		std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		std::string& strOutError)
{
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		nullptr == pPrepared || nullptr == pPreparation ||
		pPrepared->pVisualProgramProjection.get() != pProjection.get() ||
		pPrepared->pReconstructedRuntimePreparation.get() !=
			pPreparation.get() ||
		pPrepared->strEffectAssetId != pProjection->Get_EffectAssetId() ||
		pPrepared->iResourceSignature !=
			Build_ResourceSignature(pProjection->Get_Document()) ||
		!Resource_SignatureMatches(pPrepared->ResourceDocument,
			pProjection->Get_Document()) ||
		0u == pPrepared->iVisualProgramAdapterCount)
	{
		strOutError =
			"Reconstructed visual-adapter renderer resources do not match.";
		return false;
	}
	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	const std::shared_ptr<Engine::CVIBuffer_DynamicTrail> pStagedTrailBuffer =
		pPrepared->pTrailBuffer;
	if (!Validate_PreparedInstanceBuffers(
			pProjection->Get_Document(), *pPrepared, strOutError) ||
		!Clone_ModelCueResources(*pPrepared,
			StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = pProjection->Get_Document();
	m_pPreparedDocument = std::move(pPrepared);
	m_pTrailBuffer = pStagedTrailBuffer;
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = true;
	/* Adapter packets add renderer state only.  The reconstructed preparation
	   continues to own source-module execution and its 35-row target closure. */
	m_bSourceVisualProgramActive = true;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed source runtime attached with immutable visual adapters.";
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		++g_EffectRenderPrewarmProbe.iPreparedAttachCount;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Stage_ReconstructedDiagnostic(
	std::shared_ptr<const EFFECT_RECONSTRUCTED_SELECTED_FRAME> pFrame,
	std::string& strOutError)
{
	if (nullptr == pFrame || nullptr == pFrame->Get_Preparation() ||
		nullptr == pFrame->Get_Preparation()->Get_RuntimePreparation() ||
		nullptr == pFrame->Get_Preparation()->Get_Program() ||
		nullptr == pFrame->Get_Preparation()->Get_RenderResourceAuthority() ||
		pFrame->Get_Packets().size() != 2u)
	{
		strOutError =
			"Reconstructed diagnostic frame is incomplete or not exact-two.";
		return false;
	}
	if (nullptr == m_pMeshShader || nullptr == m_pParticleShader ||
		nullptr == m_pRect)
	{
		if (FAILED(Initialize()))
		{
			strOutError = "Reconstructed diagnostic renderer core is unavailable.";
			return false;
		}
	}

	CEffectReconstructedRuntimeBoundary StagedBoundary;
	if (!StagedBoundary.Stage(
		pFrame->Get_Preparation()->Get_RuntimePreparation(),
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM::RENDERER, strOutError))
	{
		return false;
	}

	auto Staged = std::make_unique<RECONSTRUCTED_DIAGNOSTIC_COMPOSITE>();
	Staged->pFrame = pFrame;
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		pFrame->Get_Preparation()->Get_Program();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY>
		Authority = pFrame->Get_Preparation()->Get_RenderResourceAuthority();
	std::array<bool_t, 2u> SelectionSeen{};

	const auto StageState = [this, &Program, &Authority, &strOutError](
		const EFFECT_RECONSTRUCTED_SELECTED_STATE_BINDING& Binding,
		const EFFECT_RECONSTRUCTED_RENDER_STATE_KIND eKind,
		RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource) -> bool_t
	{
		if (!Binding.SidecarDecision.has_value())
		{
			if (!Binding.ProgramPolicy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic render state has no authority.";
				return false;
			}
			const auto BindingIterator = std::find_if(
				Program->MaterialRenderBindings.begin(),
				Program->MaterialRenderBindings.end(),
				[&Binding](const auto& Row)
				{
					return Row.Row.strId == Binding.ProgramBinding.strId &&
						Row.Row.strRowSha256 ==
							Binding.ProgramBinding.strRowSha256;
				});
			const auto PolicyIterator = std::find_if(
				Program->MaterialPolicies.begin(),
				Program->MaterialPolicies.end(),
				[&Binding](const auto& Row)
				{
					return Row.Row.strId == Binding.ProgramPolicy->strId &&
						Row.Row.strRowSha256 ==
							Binding.ProgramPolicy->strRowSha256;
				});
			if (BindingIterator == Program->MaterialRenderBindings.end() ||
				PolicyIterator == Program->MaterialPolicies.end() ||
				BindingIterator->strPolicyRowId != PolicyIterator->Row.strId ||
				PolicyIterator->eDomain !=
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE ||
				!PolicyIterator->D3dDescriptorOracle.has_value() ||
				PolicyIterator->D3dDescriptorOracle->strPolicyRowId !=
					PolicyIterator->Row.strId ||
				PolicyIterator->D3dDescriptorOracle->strDecision != "PASS")
			{
				strOutError =
					"Reconstructed diagnostic render policy changed.";
				return false;
			}
			const EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR& Descriptor =
				PolicyIterator->D3dDescriptorOracle->Actual;
			if (eKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER &&
				PolicyIterator->D3dDescriptorOracle->eKind ==
					EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER &&
				Descriptor.Rasterizer.has_value())
			{
				const EFFECT_RUNTIME_PROGRAM_D3D_RASTERIZER& Source =
					*Descriptor.Rasterizer;
				D3D11_RASTERIZER_DESC StagedDescriptor{};
				StagedDescriptor.FillMode =
					static_cast<D3D11_FILL_MODE>(Source.iFillMode);
				StagedDescriptor.CullMode =
					static_cast<D3D11_CULL_MODE>(Source.iCullMode);
				StagedDescriptor.FrontCounterClockwise =
					Source.bFrontCounterClockwise;
				StagedDescriptor.DepthBias = Source.iDepthBias;
				StagedDescriptor.DepthBiasClamp =
					static_cast<f32_t>(Source.fDepthBiasClamp);
				StagedDescriptor.SlopeScaledDepthBias =
					static_cast<f32_t>(Source.fSlopeScaledDepthBias);
				StagedDescriptor.DepthClipEnable = Source.bDepthClipEnable;
				StagedDescriptor.ScissorEnable = Source.bScissorEnable;
				StagedDescriptor.MultisampleEnable = Source.bMultisampleEnable;
				StagedDescriptor.AntialiasedLineEnable =
					Source.bAntialiasedLineEnable;
				if (FAILED(m_pDevice->CreateRasterizerState(
					&StagedDescriptor, &Resource.pRasterizerState)))
				{
					strOutError =
						"Reconstructed diagnostic policy rasterizer-state creation failed.";
					return false;
				}
				Resource.RasterizerDescriptor = StagedDescriptor;
				Resource.bHasRasterizerDescriptor = true;
				return true;
			}
			if (eKind == EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL &&
				PolicyIterator->D3dDescriptorOracle->eKind ==
					EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL &&
				Descriptor.DepthStencil.has_value())
			{
				const auto ConvertFace = [](const
					EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE& Source)
				{
					D3D11_DEPTH_STENCILOP_DESC Result{};
					Result.StencilFailOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilFailOp);
					Result.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilDepthFailOp);
					Result.StencilPassOp = static_cast<D3D11_STENCIL_OP>(
						Source.iStencilPassOp);
					Result.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(
						Source.iStencilFunc);
					return Result;
				};
				const EFFECT_RUNTIME_PROGRAM_D3D_DEPTH_STENCIL& Source =
					*Descriptor.DepthStencil;
				D3D11_DEPTH_STENCIL_DESC StagedDescriptor{};
				StagedDescriptor.DepthEnable = Source.bDepthEnable;
				StagedDescriptor.DepthWriteMask =
					static_cast<D3D11_DEPTH_WRITE_MASK>(Source.iDepthWriteMask);
				StagedDescriptor.DepthFunc =
					static_cast<D3D11_COMPARISON_FUNC>(Source.iDepthFunc);
				StagedDescriptor.StencilEnable = Source.bStencilEnable;
				StagedDescriptor.StencilReadMask =
					static_cast<uint8_t>(Source.iStencilReadMask);
				StagedDescriptor.StencilWriteMask =
					static_cast<uint8_t>(Source.iStencilWriteMask);
				StagedDescriptor.FrontFace = ConvertFace(Source.FrontFace);
				StagedDescriptor.BackFace = ConvertFace(Source.BackFace);
				if (FAILED(m_pDevice->CreateDepthStencilState(
					&StagedDescriptor, &Resource.pDepthStencilState)))
				{
					strOutError =
						"Reconstructed diagnostic policy depth-state creation failed.";
					return false;
				}
				Resource.DepthStencilDescriptor = StagedDescriptor;
				Resource.bHasDepthStencilDescriptor = true;
				return true;
			}
			strOutError =
				"Reconstructed diagnostic render policy kind changed.";
			return false;
		}
		const auto Iterator = Authority->RenderStateDescriptorsById.find(
			Binding.SidecarDecision->strId);
		if (Iterator == Authority->RenderStateDescriptorsById.end() ||
			Iterator->second.strRowSha256 !=
				Binding.SidecarDecision->strRowSha256 ||
			Iterator->second.eKind != eKind ||
			Iterator->second.strRenderBindingId !=
				Binding.ProgramBinding.strId ||
			Iterator->second.strRenderBindingRowSha256 !=
				Binding.ProgramBinding.strRowSha256)
		{
			strOutError =
				"Reconstructed diagnostic render-state authority changed.";
			return false;
		}
		const EFFECT_RECONSTRUCTED_RENDER_STATE_DESCRIPTOR& Descriptor =
			Iterator->second;
		switch (eKind)
		{
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND:
			if (FAILED(m_pDevice->CreateBlendState(
				&Descriptor.BlendDescriptor, &Resource.pBlendState)))
			{
				strOutError =
					"Reconstructed diagnostic blend-state creation failed.";
				return false;
			}
			Resource.BlendDescriptor = Descriptor.BlendDescriptor;
			Resource.bHasBlendDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER:
			if (FAILED(m_pDevice->CreateRasterizerState(
				&Descriptor.RasterizerDescriptor,
				&Resource.pRasterizerState)))
			{
				strOutError =
					"Reconstructed diagnostic rasterizer-state creation failed.";
				return false;
			}
			Resource.RasterizerDescriptor = Descriptor.RasterizerDescriptor;
			Resource.bHasRasterizerDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL:
			if (FAILED(m_pDevice->CreateDepthStencilState(
				&Descriptor.DepthStencilDescriptor,
				&Resource.pDepthStencilState)))
			{
				strOutError =
					"Reconstructed diagnostic depth-state creation failed.";
				return false;
			}
			Resource.DepthStencilDescriptor = Descriptor.DepthStencilDescriptor;
			Resource.bHasDepthStencilDescriptor = true;
			return true;
		case EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::END:
		default:
			strOutError = "Reconstructed diagnostic render-state kind is invalid.";
			return false;
		}
	};

	for (const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet :
		pFrame->Get_Packets())
	{
		const uint32_t iSelection = Packet.Get_SelectionIndex();
		if (iSelection >= SelectionSeen.size() || SelectionSeen[iSelection])
		{
			strOutError =
				"Reconstructed diagnostic packet selection is duplicated.";
			return false;
		}
		SelectionSeen[iSelection] = true;
		const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection =
			pFrame->Get_Preparation()->Get_Request().Emitters[iSelection];
		if (Selection.eKind != Packet.Get_Kind())
		{
			strOutError = "Reconstructed diagnostic packet kind changed.";
			return false;
		}
		RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource =
			Staged->Resources[iSelection];

		for (size_t iLane = 0u; iLane < Selection.Material.TextureLanes.size();
			++iLane)
		{
			const EFFECT_RECONSTRUCTED_SELECTED_TEXTURE_LANE& Lane =
				Selection.Material.TextureLanes[iLane];
			const auto BindingIterator = Authority->TextureBindingsById.find(
				Lane.SidecarTextureBinding.strId);
			const auto TextureIterator = Authority->TextureResourcesById.find(
				Lane.SidecarTextureResource.strId);
			if (BindingIterator == Authority->TextureBindingsById.end() ||
				TextureIterator == Authority->TextureResourcesById.end() ||
				BindingIterator->second.strRowSha256 !=
					Lane.SidecarTextureBinding.strRowSha256 ||
				TextureIterator->second.strRowSha256 !=
					Lane.SidecarTextureResource.strRowSha256 ||
				BindingIterator->second.strResourceAuthorityId !=
					Lane.SidecarTextureResource.strId ||
				BindingIterator->second.strRuntimeAssetId != Lane.strRuntimeAssetId ||
				TextureIterator->second.strRuntimeAssetId != Lane.strRuntimeAssetId ||
				BindingIterator->second.strActualDdsRawSha256 != Lane.strRawSha256 ||
				TextureIterator->second.strRawSha256 != Lane.strRawSha256)
			{
				strOutError =
					"Reconstructed diagnostic texture authority changed.";
				return false;
			}

			std::filesystem::path TexturePath;
			std::vector<uint8_t> TextureBytes;
			if (!Read_ReconstructedAssetBytes(Lane.strRuntimeAssetId,
				BindingIterator->second.iActualDdsByteCount,
				Lane.strRawSha256, TexturePath, TextureBytes, strOutError))
			{
				return false;
			}
			const DirectX::DDS_LOADER_FLAGS Flags =
				BindingIterator->second.ActualDdsSrv.strColorSpace == "SRGB" ?
					DirectX::DDS_LOADER_FORCE_SRGB :
					DirectX::DDS_LOADER_IGNORE_SRGB;
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (FAILED(DirectX::CreateDDSTextureFromMemoryEx(
				m_pDevice.Get(), TextureBytes.data(), TextureBytes.size(), 0u,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0u, 0u,
				Flags, nullptr, &Texture)))
			{
				strOutError = "Reconstructed diagnostic DDS upload failed: " +
					Lane.strRuntimeAssetId;
				return false;
			}
			D3D11_SHADER_RESOURCE_VIEW_DESC SrvDescriptor{};
			Texture->GetDesc(&SrvDescriptor);
			const EFFECT_RECONSTRUCTED_DDS_SRV_IDENTITY& ExpectedSrv =
				BindingIterator->second.ActualDdsSrv;
			if (SrvDescriptor.Format != ExpectedSrv.eFormat ||
				SrvDescriptor.ViewDimension != ExpectedSrv.eViewDimension ||
				SrvDescriptor.Texture2D.MostDetailedMip !=
					ExpectedSrv.iMostDetailedMip ||
				SrvDescriptor.Texture2D.MipLevels != ExpectedSrv.iMipLevels)
			{
				strOutError =
					"Reconstructed diagnostic DDS SRV descriptor changed.";
				return false;
			}
			ComPtr<ID3D11SamplerState> Sampler;
			D3D11_SAMPLER_DESC RuntimeSamplerAuthority{};
			if (!Materialize_RuntimeSamplerDescriptor(
				BindingIterator->second.SamplerDescriptor,
				RuntimeSamplerAuthority))
			{
				strOutError =
					"Reconstructed diagnostic sampler descriptor is invalid.";
				return false;
			}
			if (FAILED(m_pDevice->CreateSamplerState(
				&RuntimeSamplerAuthority, &Sampler)))
			{
				strOutError =
					"Reconstructed diagnostic sampler-state creation failed.";
				return false;
			}
			D3D11_SAMPLER_DESC SamplerDescriptor{};
			Sampler->GetDesc(&SamplerDescriptor);
			if (!Same_RuntimeSamplerReadbackDescriptor(SamplerDescriptor,
				RuntimeSamplerAuthority,
				BindingIterator->second.SamplerDescriptor))
			{
				strOutError =
					"Reconstructed diagnostic sampler descriptor changed.";
				return false;
			}
			Resource.Textures[iLane] = std::move(Texture);
			Resource.Samplers[iLane] = std::move(Sampler);
			Resource.SamplerDescriptors[iLane] = SamplerDescriptor;
		}
		if (!Same_SamplerDescriptor(Resource.SamplerDescriptors[0u],
			Resource.SamplerDescriptors[1u]))
		{
			strOutError =
				"Reconstructed diagnostic texture lanes require different samplers.";
			return false;
		}

		if (!StageState(Selection.Material.BlendState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::BLEND, Resource) ||
			!StageState(Selection.Material.RasterizerState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::RASTERIZER, Resource) ||
			!StageState(Selection.Material.DepthStencilState,
				EFFECT_RECONSTRUCTED_RENDER_STATE_KIND::DEPTH_STENCIL, Resource))
		{
			return false;
		}

		if (EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ==
			Selection.eKind)
		{
			if (!Selection.Geometry.has_value() ||
				!Packet.Get_Values().vMeshDimensionlessScaleXzy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic Mesh geometry is unavailable.";
				return false;
			}
			const EFFECT_RECONSTRUCTED_SELECTED_GEOMETRY_BINDING& Geometry =
				*Selection.Geometry;
			std::filesystem::path ModelPath;
			std::vector<uint8_t> ModelBytes;
			if (!Read_ReconstructedAssetBytes(Geometry.strRuntimeAssetId,
				Geometry.iCandidateResourceByteSize,
				Geometry.strCandidateResourceSha256,
				ModelPath, ModelBytes, strOutError))
			{
				return false;
			}
			const f32_t fPreScale = static_cast<f32_t>(Geometry.fGeometryPreScale);
			const std::string ModelPathString = ModelPath.string();
			unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
				m_pDevice, m_pContext, MODEL::NONANIM, ModelPathString.c_str(),
				XMMatrixScaling(fPreScale, fPreScale, fPreScale));
			if (nullptr == Model || Model->Get_NumMeshes() != Geometry.iSubmeshCount ||
				Sha256Hex(Model->Get_GeometryPayloadSha256()) !=
					Geometry.strPayloadSha256 ||
				Sha256Hex(Model->Get_GeometryMetadataIdentitySha256()) !=
					Geometry.strMetadataIdentitySha256)
			{
				strOutError =
					"Reconstructed diagnostic CModel identity changed.";
				return false;
			}
			Resource.pModel = std::move(Model);
		}
		else if (EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE ==
			Selection.eKind)
		{
			if (!Selection.SpriteSink.has_value() ||
				!Packet.Get_Values().vSpriteSignedWorldSizeXzy.has_value())
			{
				strOutError =
					"Reconstructed diagnostic Sprite sink is unavailable.";
				return false;
			}
		}
		else
		{
			strOutError = "Reconstructed diagnostic packet kind is unsupported.";
			return false;
		}

		D3D11_QUERY_DESC QueryDescriptor{};
		QueryDescriptor.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		if (FAILED(m_pDevice->CreateQuery(
			&QueryDescriptor, &Resource.pPipelineStatisticsQuery)))
		{
			strOutError =
				"Reconstructed diagnostic pipeline-statistics query failed.";
			return false;
		}
	}

	unique_ptr<Engine::CVIBuffer_ParticleRect> StagedParticleBuffer;
	if (nullptr == m_pParticleBuffer)
	{
		StagedParticleBuffer = Engine::CVIBuffer_ParticleRect::Create(
			m_pDevice, m_pContext, 1u);
		if (nullptr == StagedParticleBuffer)
		{
			strOutError =
				"Reconstructed diagnostic particle instance buffer failed.";
			return false;
		}
	}
	if (nullptr != StagedParticleBuffer)
		m_pParticleBuffer = std::move(StagedParticleBuffer);
	m_Document = {};
	m_pPreparedDocument.reset();
	m_ModelCueResources.clear();
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_pReconstructedDiagnostic = std::move(Staged);
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus =
		"Reconstructed diagnostic GPU resources committed; waiting for draw.";
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentRenderer::Set_PreviewSubmissionIsolation(
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
	std::string& strOutError)
{
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Program =
		m_ReconstructedRuntimeBoundary.Get_Program();
	std::string DocumentError;
	const bool_t bReconstructedArtist =
		m_bReconstructedSourceRuntimeActive && nullptr != Program;
	const bool_t bGenericVisual = m_bSourceVisualProgramActive &&
		nullptr != m_pPreparedDocument &&
		nullptr != m_pPreparedDocument->pVisualProgramProjection;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	/* The focused GPU harness stages the ordinary product document directly so
	   Binding 0 and Binding 1 exercise the same renderer path.  Only the
	   test-only CEffectObject entry point can reach this allowance; the product
	   entry point still requires an admitted source visual program. */
	const bool_t bStagedOrdinaryTest = nullptr != m_pPreparedDocument &&
		!m_bReconstructedSourceRuntimeActive && !m_bSourceVisualProgramActive;
#else
	constexpr bool_t bStagedOrdinaryTest = false;
#endif
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	if ((!bReconstructedArtist && !bGenericVisual && !bStagedOrdinaryTest) ||
		(bReconstructedArtist &&
		 (Program->iSkillId != 31470u || Program->Admission.bRuntimeExecution ||
		  Program->Admission.bProduct ||
		  !CEffectDocumentCodec::Validate_Artist31470ReconstructedRuntimeDrawable(
			  Document, DocumentError))))
	{
		strOutError =
			"Preview submission isolation requires an admitted source visual program.";
		if (!DocumentError.empty())
			strOutError += " " + DocumentError;
		return false;
	}

	std::array<uint32_t,
		static_cast<size_t>(EFFECT_GPU_RENDER_FAMILY::END)> VisibleFamilies{};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_GPU_RENDER_FAMILY eFamily =
			Resolve_GpuRenderFamily(Element);
		if (Element.bVisible)
		{
			if (EFFECT_GPU_RENDER_FAMILY::END == eFamily)
			{
				if (bReconstructedArtist)
				{
					strOutError =
						"Artist Core preview contains a visible Light/Post row.";
					return false;
				}
				continue;
			}
			++VisibleFamilies[static_cast<size_t>(eFamily)];
		}
	}
	if (bReconstructedArtist &&
		(VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::MESH)] != 13u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::SPRITE)] != 16u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::DECAL)] != 3u ||
		 VisibleFamilies[static_cast<size_t>(
			EFFECT_GPU_RENDER_FAMILY::RIBBON)] != 1u))
	{
		strOutError =
			"Preview submission isolation requires the exact visible Core33 scope.";
		return false;
	}

	EFFECT_PREVIEW_SUBMISSION_ISOLATION Staged = Isolation;
	switch (Isolation.eKind)
	{
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL:
		if (Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END ||
			!Isolation.strElementId.empty() || !Isolation.ElementIds.empty())
		{
			strOutError =
				"ALL preview isolation must not carry a family or occurrence ID.";
			return false;
		}
		break;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY:
		switch (Isolation.eFamily)
		{
		case EFFECT_GPU_RENDER_FAMILY::MESH:
		case EFFECT_GPU_RENDER_FAMILY::SPRITE:
		case EFFECT_GPU_RENDER_FAMILY::DECAL:
		case EFFECT_GPU_RENDER_FAMILY::RIBBON:
			break;
		case EFFECT_GPU_RENDER_FAMILY::END:
		default:
			strOutError =
				"FAMILY preview isolation requires one explicit Core family.";
			return false;
		}
		if (!Isolation.strElementId.empty() ||
			!Isolation.ElementIds.empty() ||
			0u == VisibleFamilies[static_cast<size_t>(Isolation.eFamily)])
		{
			strOutError =
				"FAMILY preview isolation carries an occurrence ID or empty family.";
			return false;
		}
		break;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE:
	{
		if (Isolation.strElementId.empty() || !Isolation.ElementIds.empty())
		{
			strOutError =
				"OCCURRENCE preview isolation requires one stable Element ID.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* pSelected = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId != Isolation.strElementId)
				continue;
			if (nullptr != pSelected)
			{
				strOutError =
					"OCCURRENCE preview isolation ID is duplicated.";
				return false;
			}
			pSelected = &Element;
		}
		const EFFECT_GPU_RENDER_FAMILY eSelectedFamily =
			nullptr == pSelected ? EFFECT_GPU_RENDER_FAMILY::END :
				Resolve_GpuRenderFamily(*pSelected);
		if (nullptr == pSelected || !pSelected->bVisible ||
			EFFECT_GPU_RENDER_FAMILY::END == eSelectedFamily ||
			(Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END &&
			 Isolation.eFamily != eSelectedFamily))
		{
			strOutError =
				"OCCURRENCE preview isolation must name one visible Core GPU row.";
			return false;
		}
		Staged.eFamily = eSelectedFamily;
		break;
	}
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET:
	{
		if (Isolation.eFamily != EFFECT_GPU_RENDER_FAMILY::END ||
			!Isolation.strElementId.empty() || Isolation.ElementIds.empty())
		{
			strOutError =
				"ELEMENT_SET preview isolation requires only stable Element IDs.";
			return false;
		}
		std::unordered_set<std::string> Seen;
		for (const std::string& ElementId : Isolation.ElementIds)
		{
			if (ElementId.empty() || !Seen.emplace(ElementId).second)
			{
				strOutError =
					"ELEMENT_SET preview isolation ID is empty or duplicated.";
				return false;
			}
			const auto Element = std::find_if(Document.Elements.begin(),
				Document.Elements.end(), [&ElementId](const auto& Candidate)
				{
					return Candidate.strElementId == ElementId;
				});
			if (Element == Document.Elements.end() || !Element->bVisible)
			{
				strOutError =
					"ELEMENT_SET preview isolation names a missing or hidden row.";
				return false;
			}
		}
		std::sort(Staged.ElementIds.begin(), Staged.ElementIds.end());
		break;
	}
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::END:
	default:
		strOutError = "Preview submission isolation kind is invalid.";
		return false;
	}

	m_PreviewSubmissionIsolation = std::move(Staged);
	m_strStatus = "Source visual-program preview submission isolation staged.";
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentRenderer::Reset_PreviewSubmissionIsolation()
{
	m_PreviewSubmissionIsolation = {};
}

bool_t Client::CEffectDocumentRenderer::Should_SubmitPreviewOccurrence(
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_GPU_RENDER_FAMILY eFamily) const
{
	switch (m_PreviewSubmissionIsolation.eKind)
	{
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL:
		return true;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY:
		return eFamily == m_PreviewSubmissionIsolation.eFamily;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE:
		return Element.strElementId ==
			m_PreviewSubmissionIsolation.strElementId;
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET:
		return std::binary_search(m_PreviewSubmissionIsolation.ElementIds.begin(),
			m_PreviewSubmissionIsolation.ElementIds.end(), Element.strElementId);
	case EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::END:
	default:
		return false;
	}
}

void Client::CEffectDocumentRenderer::Clear()
{
	m_Document = {};
	m_pPreparedDocument.reset();
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_bReconstructedSourceRuntimeActive = false;
	m_bSourceVisualProgramActive = false;
	Reset_PreviewSubmissionIsolation();
	m_ModelCueResources.clear();
	m_LastRenderSubmissionStats = {};
	m_bWorldMarkSubmissionPending = false;
	m_iWorldMarkSubmissionSerial = 0u;
	m_iStatusEvaluated = (std::numeric_limits<uint64_t>::max)();
	m_iStatusActive = (std::numeric_limits<uint64_t>::max)();
	m_iStatusSubmitted = (std::numeric_limits<uint64_t>::max)();
	m_iStatusSuppressed = (std::numeric_limits<uint64_t>::max)();
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	m_strStatus = "No Effect Document staged.";
}

const Client::CEffectDocumentRenderer::ELEMENT_RESOURCE*
Client::CEffectDocumentRenderer::Find_Resource(
	const std::string& strElementId) const
{
	if (nullptr == m_pPreparedDocument)
		return nullptr;
	const auto Iterator =
		m_pPreparedDocument->ElementResources.find(strElementId);
	return Iterator == m_pPreparedDocument->ElementResources.end() ?
		nullptr : &Iterator->second;
}

uint32_t Client::CEffectDocumentRenderer::Select_Pass(
	const EFFECT_RENDER_PROFILE eProfile) const
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE: return 0u;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ: return 1u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ: return 2u;
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ: return 3u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ: return 4u;
	case EFFECT_RENDER_PROFILE::END:
	default: return UINT32_MAX;
	}
}

HRESULT Client::CEffectDocumentRenderer::Fail_RenderOperation(
	std::string strOperation,
	const HRESULT hResult,
	const bool_t bObjectLocal)
{
	if (m_strRenderFailureDetail.empty())
		m_strRenderFailureDetail = std::move(strOperation);
	m_bLastRenderFailureObjectLocal =
		m_bLastRenderFailureObjectLocal || bObjectLocal;
	return FAILED(hResult) ? hResult : E_FAIL;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
void Client::CEffectDocumentRenderer::Record_TestMaterialBinding()
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iMaterialBindCount;
	/* Bind_MaterialInputs commits five standard SRVs and seven source SRVs.
	   Missing optional lanes are deliberately bound to typed white/black
	   resources, so every successful call has the same complete shader seam. */
	m_pActiveOccurrenceStats->iTextureSrvBindCount += 12u;
}

void Client::CEffectDocumentRenderer::Record_TestSamplerBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iSamplerBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestShaderPassApplication()
{
	if (nullptr != m_pActiveOccurrenceStats)
	{
		++m_pActiveOccurrenceStats->iShaderPassApplyCount;
		/* CShader::Begin applies the selected Effect pass, including its compiled
		   LinearSampler state. RuntimeMaterialV2 may then replace that PS sampler;
		   its successful override is recorded independently below. */
		++m_pActiveOccurrenceStats->iSamplerBindCount;
	}
}

void Client::CEffectDocumentRenderer::Record_TestGeometryUpload()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iGeometryUploadCount;
}

void Client::CEffectDocumentRenderer::Record_TestTrailGeometryUpload(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iGeometryUploadCount;
	m_pActiveOccurrenceStats->iFinalTrailUploadedVertexCount += Vertices.size();
	for (size_t iVertex = 0u; iVertex + 1u < Vertices.size(); iVertex += 2u)
	{
		const float3_t& Left = Vertices[iVertex].vPosition;
		const float3_t& Right = Vertices[iVertex + 1u].vPosition;
		const float3_t Center = {
			(Left.x + Right.x) * 0.5f,
			(Left.y + Right.y) * 0.5f,
			(Left.z + Right.z) * 0.5f };
		const f32_t DeltaX = Right.x - Left.x;
		const f32_t DeltaY = Right.y - Left.y;
		const f32_t DeltaZ = Right.z - Left.z;
		const f32_t Width = std::sqrt(
			DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ);
		if (!m_pActiveOccurrenceStats->bHasFinalTrailPairCenter)
		{
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin = Center;
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax = Center;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = Width;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = Width;
			m_pActiveOccurrenceStats->bHasFinalTrailPairCenter = true;
			continue;
		}
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z, Center.z);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z, Center.z);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = (std::min)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin, Width);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = (std::max)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax, Width);
	}
}

void Client::CEffectDocumentRenderer::Record_TestVIBufferBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iVIBufferBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestDrawSelection(
	const EFFECT_GPU_RENDER_CARRIER eCarrier,
	const uint32_t iSelectedPassIndex)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (0u == m_pActiveOccurrenceStats->iDrawSelectionCount)
	{
		m_pActiveOccurrenceStats->eCarrier = eCarrier;
		m_pActiveOccurrenceStats->iSelectedPassIndex = iSelectedPassIndex;
	}
	else if (m_pActiveOccurrenceStats->eCarrier != eCarrier ||
		m_pActiveOccurrenceStats->iSelectedPassIndex != iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	if (EFFECT_GPU_RENDER_CARRIER::END == eCarrier ||
		UINT32_MAX == iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	++m_pActiveOccurrenceStats->iDrawSelectionCount;
}

void Client::CEffectDocumentRenderer::
	Record_TestCompiledAdapterPipelineValidation()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iCompiledAdapterPipelineValidationCount;
}

namespace
{
	void Extend_SubmittedPosition(
		Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS& Stats,
		const float3_t& Position)
	{
		if (!Stats.bHasSubmittedPosition)
		{
			Stats.vSubmittedPositionMin = Position;
			Stats.vSubmittedPositionMax = Position;
			Stats.bHasSubmittedPosition = true;
			return;
		}
		Stats.vSubmittedPositionMin.x =
			(std::min)(Stats.vSubmittedPositionMin.x, Position.x);
		Stats.vSubmittedPositionMin.y =
			(std::min)(Stats.vSubmittedPositionMin.y, Position.y);
		Stats.vSubmittedPositionMin.z =
			(std::min)(Stats.vSubmittedPositionMin.z, Position.z);
		Stats.vSubmittedPositionMax.x =
			(std::max)(Stats.vSubmittedPositionMax.x, Position.x);
		Stats.vSubmittedPositionMax.y =
			(std::max)(Stats.vSubmittedPositionMax.y, Position.y);
		Stats.vSubmittedPositionMax.z =
			(std::max)(Stats.vSubmittedPositionMax.z, Position.z);
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const float4x4_t& World)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
	{
		m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = World;
		m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
	}
	Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
		{ World._41, World._42, World._43 });
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_PARTICLE> Instances)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_PARTICLE& Instance : Instances)
	{
		if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
		{
			m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = Instance.World;
			m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
		}
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
			{ Instance.World._41, Instance.World._42, Instance.World._43 });
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_TRAIL& Vertex : Vertices)
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats, Vertex.vPosition);
}
#endif

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale)
{
	return Bind_Common(pShader, *Element.pElement, Element.Color,
		Element.fLocalTimeSeconds, Element.fNormalizedLife,
		Resource, fAlphaScale);
}

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale)
{
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Common shader bind failed: shader is null.", E_POINTER);
	HRESULT hResult = pShader->Bind_Matrix("g_ViewMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ViewMatrix.", hResult);
	hResult = pShader->Bind_Matrix("g_ProjMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ProjMatrix.", hResult);
	return Bind_MaterialInputs(pShader, Element, Color,
		fLocalTimeSeconds, fNormalizedLife, Resource, fAlphaScale);
}

HRESULT Client::CEffectDocumentRenderer::Bind_MaterialInputs(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale)
{
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Material bind failed: shader is null.", E_INVALIDARG);
	HRESULT hFirstBindFailure = S_OK;
	const auto BindFailed = [&hFirstBindFailure](const HRESULT hResult)
	{
		if (SUCCEEDED(hResult))
			return false;
		if (SUCCEEDED(hFirstBindFailure))
			hFirstBindFailure = hResult;
		return true;
	};
	if (pShader == m_pMeshShader || pShader == m_pParticleShader)
	{
		if (BindFailed(pShader->Bind_RawValue(
			"g_ReconstructedMaterialEvaluatorEnabled",
			&Resource.iReconstructedMaterialEvaluatorEnabled,
			sizeof(Resource.iReconstructedMaterialEvaluatorEnabled))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_ReconstructedMaterialFeatureMask",
				&Resource.iReconstructedMaterialFeatureMask,
				sizeof(Resource.iReconstructedMaterialFeatureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedUVScale",
				&Resource.vReconstructedUVScale,
				sizeof(Resource.vReconstructedUVScale))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedPanRotationAux",
				&Resource.vReconstructedPanRotationAux,
				sizeof(Resource.vReconstructedPanRotationAux))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedColor",
				&Resource.vReconstructedColor,
				sizeof(Resource.vReconstructedColor))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams0",
				&Resource.vReconstructedParams0,
				sizeof(Resource.vReconstructedParams0))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams1",
				&Resource.vReconstructedParams1,
				sizeof(Resource.vReconstructedParams1))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Opcode",
				&Resource.iArtistVisualV4Opcode,
				sizeof(Resource.iArtistVisualV4Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4TextureMask",
				&Resource.iArtistVisualV4TextureMask,
				sizeof(Resource.iArtistVisualV4TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Params",
				Resource.ArtistVisualV4Params.data(),
				sizeof(Resource.ArtistVisualV4Params))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Colors",
				Resource.ArtistVisualV4Colors.data(),
				sizeof(Resource.ArtistVisualV4Colors))))
		{
			return Fail_RenderOperation(
				"Material bind failed: reconstructed evaluator block.",
				hFirstBindFailure);
		}
	}
	const bool_t bRuntimeMaterialV2Shader = pShader == m_pMeshShader ||
		pShader == m_pParticleShader || pShader == m_pDecalShader ||
		pShader == m_pTrailShader || pShader == m_pRectShader;
	if (bRuntimeMaterialV2Shader)
	{
		if (
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Enabled",
				&Resource.iRuntimeMaterialV2Enabled,
				sizeof(Resource.iRuntimeMaterialV2Enabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Opcode",
				&Resource.iRuntimeMaterialV2Opcode,
				sizeof(Resource.iRuntimeMaterialV2Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureLaneCount",
				&Resource.iRuntimeMaterialV2TextureLaneCount,
				sizeof(Resource.iRuntimeMaterialV2TextureLaneCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureMask",
				&Resource.iRuntimeMaterialV2TextureMask,
				sizeof(Resource.iRuntimeMaterialV2TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicConsumedMask",
				&Resource.iRuntimeMaterialV2DynamicConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicSuppressedMask",
				&Resource.iRuntimeMaterialV2DynamicSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ParticleColorPolicy",
				&Resource.iRuntimeMaterialV2ParticleColorPolicy,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorPolicy))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorConsumedMask",
				&Resource.iRuntimeMaterialV2ParticleColorConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorSuppressedMask",
				&Resource.iRuntimeMaterialV2ParticleColorSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarCount",
				&Resource.iRuntimeMaterialV2ScalarCount,
				sizeof(Resource.iRuntimeMaterialV2ScalarCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2VectorCount",
				&Resource.iRuntimeMaterialV2VectorCount,
				sizeof(Resource.iRuntimeMaterialV2VectorCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputCount",
				&Resource.iRuntimeMaterialV2InputCount,
				sizeof(Resource.iRuntimeMaterialV2InputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputConsumedMask",
				Resource.RuntimeMaterialV2InputConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputSuppressedMask",
				Resource.RuntimeMaterialV2InputSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticInputCount",
				&Resource.iRuntimeMaterialV2StaticInputCount,
				sizeof(Resource.iRuntimeMaterialV2StaticInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSelectedMask",
				&Resource.iRuntimeMaterialV2StaticSelectedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSelectedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticConsumedMask",
				&Resource.iRuntimeMaterialV2StaticConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSuppressedMask",
				&Resource.iRuntimeMaterialV2StaticSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderInputCount",
				&Resource.iRuntimeMaterialV2RenderInputCount,
				sizeof(Resource.iRuntimeMaterialV2RenderInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderConsumedMask",
				&Resource.iRuntimeMaterialV2RenderConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderSuppressedMask",
				&Resource.iRuntimeMaterialV2RenderSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarBlocks",
				Resource.RuntimeMaterialV2ScalarBlocks.data(),
				sizeof(Resource.RuntimeMaterialV2ScalarBlocks))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentConsumedMask",
				Resource.RuntimeMaterialV2VectorComponentConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentSuppressedMask",
				Resource.RuntimeMaterialV2VectorComponentSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2NormalizedLife",
				&fNormalizedLife, sizeof(fNormalizedLife))) ||
			BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
				&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))))
		{
			return Fail_RenderOperation(
				"Material bind failed: RuntimeMaterialV2 scalar block.",
				hFirstBindFailure);
		}
	}
	if (bRuntimeMaterialV2Shader &&
		BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Vectors",
			Resource.RuntimeMaterialV2Vectors.data(),
			sizeof(Resource.RuntimeMaterialV2Vectors))))
	{
		return Fail_RenderOperation(
			"Material bind failed: RuntimeMaterialV2 vector block.",
			hFirstBindFailure);
	}
	const bool_t bStandardColorV1Shader = pShader == m_pMeshShader ||
		pShader == m_pParticleShader ||
		pShader == m_pDecalShader || pShader == m_pTrailShader;
	if (bStandardColorV1Shader &&
		(BindFailed(pShader->Bind_RawValue("g_StandardColorV1Enabled",
			&Resource.iStandardColorV1Enabled,
			sizeof(Resource.iStandardColorV1Enabled))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Header",
			Resource.StandardColorV1Header.data(),
			sizeof(Resource.StandardColorV1Header))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1BaseCoverage",
			Resource.StandardColorV1BaseCoverage.data(),
			sizeof(Resource.StandardColorV1BaseCoverage))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Dissolve",
			Resource.StandardColorV1Dissolve.data(),
			sizeof(Resource.StandardColorV1Dissolve))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Policies",
			Resource.StandardColorV1Policies.data(),
			sizeof(Resource.StandardColorV1Policies))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Scalars",
			&Resource.vStandardColorV1Scalars,
			sizeof(Resource.vStandardColorV1Scalars)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: StandardColorV1 packet.",
			hFirstBindFailure);
	}
	const std::string_view strSourceSubUVMode = SourceLiteralString(
		Element, "interpolationmethod");
	const uint32_t iSourceSubUVColumns = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_horizontal", 1.f)));
	const uint32_t iSourceSubUVRows = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_vertical", 1.f)));
	const uint64_t iSourceSubUVFrameCount =
		static_cast<uint64_t>(iSourceSubUVColumns) * iSourceSubUVRows;
	const bool_t bSourceSubUV = Element.SourceRecipe.bEnabled &&
		iSourceSubUVFrameCount > 1u && iSourceSubUVFrameCount <= UINT32_MAX &&
		!strSourceSubUVMode.empty() && strSourceSubUVMode != "none" &&
		strSourceSubUVMode != "psuvim_none";

	float2_t UVOffset(
		Element.Detail.UV.vStart.x + Element.Detail.UV.vSpeed.x * fLocalTimeSeconds,
		Element.Detail.UV.vStart.y + Element.Detail.UV.vSpeed.y * fLocalTimeSeconds);
	if (Element.Detail.UV.bWave)
	{
		const f32_t Wave = std::sin(
			XM_2PI * Element.Detail.UV.fWaveFrequency * fLocalTimeSeconds);
		UVOffset.x += Element.Detail.UV.vWaveAmplitude.x * Wave;
		UVOffset.y += Element.Detail.UV.vWaveAmplitude.y * Wave;
	}
	int32_t iTileIndex = Element.Detail.UV.iTileIndex;
	if (!bSourceSubUV && Element.Detail.UV.bSequence)
	{
		const int32_t iTileCount = Element.Detail.UV.iTileColumns *
			Element.Detail.UV.iTileRows;
		const int32_t iFrame = static_cast<int32_t>(
			fLocalTimeSeconds / Element.Detail.UV.fSequenceTerm) +
			Element.Detail.UV.iTileIndex;
		iTileIndex = Element.Detail.UV.bLoop ?
			((iFrame % iTileCount) + iTileCount) % iTileCount :
			std::clamp(iFrame, 0, iTileCount - 1);
	}
	const float2_t UVScale = bSourceSubUV ? float2_t(1.f, 1.f) : float2_t(
		1.f / Element.Detail.UV.iTileColumns,
		1.f / Element.Detail.UV.iTileRows);
	if (!bSourceSubUV)
	{
		UVOffset.x += static_cast<f32_t>(
			iTileIndex % Element.Detail.UV.iTileColumns) * UVScale.x;
		UVOffset.y += static_cast<f32_t>(
			iTileIndex / Element.Detail.UV.iTileColumns) * UVScale.y;
	}
	const f32_t Dissolve = Element.Detail.Timing.fDissolveStartNormalized >= 1.f ?
		0.f : std::clamp(
			(fNormalizedLife - Element.Detail.Timing.fDissolveStartNormalized) /
			(1.f - Element.Detail.Timing.fDissolveStartNormalized), 0.f, 1.f);

	const float4_t AuthoredColorMultiply =
		Evaluate_CommonColor(Element, fNormalizedLife).vColorMultiply;
	float4_t ColorMultiply = Color.vColorMultiply;
	ColorMultiply.w *= fAlphaScale;
	const uint32_t iHasNoise = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) ? 1u : 0u;
	const uint32_t iHasEmissive = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasDissolve = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasBase2 = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask2 = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasNoise2 = nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) ? 1u : 0u;
	const uint32_t iDistortionOnBase =
		Element.Detail.Color.bDistortionOnBaseMaterial ? 1u : 0u;
	if ((pShader == m_pParticleShader || pShader == m_pMeshShader ||
		 pShader == m_pTrailShader) &&
		(BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
			&Resource.iSourceMaterialProfile,
			sizeof(Resource.iSourceMaterialProfile))) ||
		BindFailed(pShader->Bind_RawValue("g_AuthoredColorMultiply",
			&AuthoredColorMultiply, sizeof(AuthoredColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars0",
			&Resource.vSourceScalars0,
			sizeof(Resource.vSourceScalars0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars1",
			&Resource.vSourceScalars1,
			sizeof(Resource.vSourceScalars1))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector0",
			&Resource.vSourceVector0,
			sizeof(Resource.vSourceVector0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector1",
			&Resource.vSourceVector1,
			sizeof(Resource.vSourceVector1))) ||
		BindFailed(pShader->Bind_RawValue("g_TypedTrailParameters",
			Resource.TypedTrailParameters.data(),
			sizeof(Resource.TypedTrailParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowParameters",
			Resource.LinearFlowParameters.data(),
			sizeof(Resource.LinearFlowParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskAColor",
			&Resource.vLinearFlowMaskAColor,
			sizeof(Resource.vLinearFlowMaskAColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskBColor",
			&Resource.vLinearFlowMaskBColor,
			sizeof(Resource.vLinearFlowMaskBColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineParameters",
			Resource.BlacklineParameters.data(),
			sizeof(Resource.BlacklineParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineDiffuseColor",
			&Resource.vBlacklineDiffuseColor,
			sizeof(Resource.vBlacklineDiffuseColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineMaskColor",
			&Resource.vBlacklineMaskColor,
			sizeof(Resource.vBlacklineMaskColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackParameters",
			Resource.LocalCrackParameters.data(),
			sizeof(Resource.LocalCrackParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackOutColor",
			&Resource.vLocalCrackOutColor,
			sizeof(Resource.vLocalCrackOutColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackInColor",
			&Resource.vLocalCrackInColor,
			sizeof(Resource.vLocalCrackInColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackReflectionColor",
			&Resource.vLocalCrackReflectionColor,
			sizeof(Resource.vLocalCrackReflectionColor))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampUMask",
			&Resource.iSourceTextureClampUMask,
			sizeof(Resource.iSourceTextureClampUMask))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampVMask",
			&Resource.iSourceTextureClampVMask,
			sizeof(Resource.iSourceTextureClampVMask))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedUVScalePan",
			&Resource.GroupedConstants.vUVScalePan,
			sizeof(Resource.GroupedConstants.vUVScalePan))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedAlphaEmissive",
			&Resource.GroupedConstants.vAlphaEmissive,
			sizeof(Resource.GroupedConstants.vAlphaEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedNoiseDissolve",
			&Resource.GroupedConstants.vNoiseDissolve,
			sizeof(Resource.GroupedConstants.vNoiseDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedTint",
			&Resource.GroupedConstants.vTint,
			sizeof(Resource.GroupedConstants.vTint))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedMaterialFlags",
			&Resource.GroupedConstants.iFlags,
			sizeof(Resource.GroupedConstants.iFlags))) ||
		BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
			&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		BindFailed(pShader->Bind_RawValue("g_DynamicParameterSemantics",
			Resource.DynamicParameterSemantics.data(),
			sizeof(Resource.DynamicParameterSemantics)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: source-profile block.",
			hFirstBindFailure);
	}
	if (pShader == m_pRectShader && 0u != Resource.iRuntimeMaterialV2Enabled &&
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))))
	{
		return Fail_RenderOperation(
			"Material bind failed: typed sprite rect source texture mask.",
			hFirstBindFailure);
	}
	if (pShader == m_pMeshShader)
	{
		uint32_t iRingFillEnabled = 0u;
		uint32_t iRingFillDirection = 0u;
		uint32_t iRingFillInvert = 0u;
		f32_t fRingFillProgress = 1.f;
		f32_t fRingFillFeather = 0.f;
		const EFFECT_MESH_RING_FILL_DESC& RingFill =
			Element.Detail.Mesh.RingFill;
		const bool_t bGenericManualMeshParticle =
			RingFill.bEnabled &&
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			nullptr != Find_Binding(
				Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			nullptr != Resource.pModel &&
			!Element.SourceRecipe.bEnabled &&
			Element.Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualMeshParticle)
		{
			iRingFillEnabled = 1u;
			iRingFillDirection = static_cast<uint32_t>(RingFill.eDirection);
			iRingFillInvert = RingFill.bInvert ? 1u : 0u;
			fRingFillProgress = RingFill.fProgress;
			const EFFECT_LINEAR_LERP_DESC& LinearLerp =
				Element.Detail.LinearLerp;
			if (LinearLerp.bRingFillProgress)
			{
				/* Ring Fill is an Element timeline, not an implicit particle fade.
				   Fixed bursts are born on the first simulation step, so particle
				   age cannot reach one at the authored completion-wave boundary. */
				const f32_t fLifeT = Element.Detail.Timing.fLifeTimeSeconds > 0.f ?
					std::clamp(fLocalTimeSeconds /
						Element.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f) :
					std::clamp(fNormalizedLife, 0.f, 1.f);
				fRingFillProgress +=
					(LinearLerp.fEndRingFillProgress - fRingFillProgress) *
					fLifeT;
			}
			fRingFillProgress = std::clamp(fRingFillProgress, 0.f, 1.f);
			fRingFillFeather = std::clamp(RingFill.fFeather, 0.f, 0.5f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_RingFillEnabled",
				&iRingFillEnabled, sizeof(iRingFillEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillDirection",
				&iRingFillDirection, sizeof(iRingFillDirection))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillInvert",
				&iRingFillInvert, sizeof(iRingFillInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillProgress",
				&fRingFillProgress, sizeof(fRingFillProgress))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillFeather",
				&fRingFillFeather, sizeof(fRingFillFeather))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic mesh Ring Fill block.",
				hFirstBindFailure);
		}
	}
	if (pShader == m_pParticleShader || pShader == m_pRectShader)
	{
		uint32_t iLinearRevealEnabled = 0u;
		uint32_t iLinearRevealAxis = 1u;
		uint32_t iLinearRevealInvert = 1u;
		f32_t fLinearRevealStartSeconds = 0.f;
		f32_t fLinearRevealDurationSeconds = 0.55f;
		f32_t fLinearRevealEdgeWidth = 0.045f;
		f32_t fLinearRevealSoftness = 0.03f;
		float4_t vLinearRevealEdgeColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fLinearRevealEdgeEmissive = 7.f;
		const EFFECT_LINEAR_REVEAL_DESC& LinearReveal =
			Element.Detail.Sprite.LinearReveal;
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		const bool_t bExpectedSpriteCarrier =
			(pShader == m_pParticleShader &&
			 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			 nullptr == Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			 nullptr == Resource.pModel) ||
			(pShader == m_pRectShader &&
			 Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		const bool_t bGenericManualSprite = LinearReveal.bEnabled &&
			bExpectedSpriteCarrier && bDirectHandAuthored &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			!Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualSprite)
		{
			iLinearRevealEnabled = 1u;
			iLinearRevealAxis = static_cast<uint32_t>(LinearReveal.eAxis);
			iLinearRevealInvert = LinearReveal.bInvert ? 1u : 0u;
			fLinearRevealStartSeconds = std::clamp(
				LinearReveal.fStartSeconds, 0.f, 30.f);
			fLinearRevealDurationSeconds = std::clamp(
				LinearReveal.fDurationSeconds, 0.0001f, 30.f);
			fLinearRevealEdgeWidth = std::clamp(
				LinearReveal.fEdgeWidth, 0.f, 0.5f);
			fLinearRevealSoftness = std::clamp(
				LinearReveal.fSoftness, 0.f,
				(std::max)(0.f, 0.5f - fLinearRevealEdgeWidth));
			vLinearRevealEdgeColor =
			{
				std::clamp(LinearReveal.vEdgeColor.x, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.y, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.z, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.w, 0.f, 1.f)
			};
			fLinearRevealEdgeEmissive = std::clamp(
				LinearReveal.fEdgeEmissive, 0.f, 100.f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_LinearRevealEnabled",
				&iLinearRevealEnabled, sizeof(iLinearRevealEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealAxis",
				&iLinearRevealAxis, sizeof(iLinearRevealAxis))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealInvert",
				&iLinearRevealInvert, sizeof(iLinearRevealInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealStartSeconds",
				&fLinearRevealStartSeconds,
				sizeof(fLinearRevealStartSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealDurationSeconds",
				&fLinearRevealDurationSeconds,
				sizeof(fLinearRevealDurationSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeWidth",
				&fLinearRevealEdgeWidth, sizeof(fLinearRevealEdgeWidth))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealSoftness",
				&fLinearRevealSoftness, sizeof(fLinearRevealSoftness))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeColor",
				&vLinearRevealEdgeColor, sizeof(vLinearRevealEdgeColor))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeEmissive",
				&fLinearRevealEdgeEmissive,
				sizeof(fLinearRevealEdgeEmissive))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic sprite Linear Reveal block.",
				hFirstBindFailure);
		}
	}

	const bool_t bBindFailed =
		BindFailed(pShader->Bind_RawValue("g_UVScale", &UVScale, sizeof(UVScale))) ||
		BindFailed(pShader->Bind_RawValue("g_UVOffset", &UVOffset, sizeof(UVOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorOffset", &Color.vColorOffset, sizeof(Color.vColorOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorMultiply", &ColorMultiply, sizeof(ColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorClip", &Color.fColorClip, sizeof(Color.fColorClip))) ||
		BindFailed(pShader->Bind_RawValue("g_EmissiveIntensity", &Color.fEmissiveIntensity, sizeof(Color.fEmissiveIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DistortionIntensity", &Color.fDistortionIntensity, sizeof(Color.fDistortionIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DistortionOnBaseMaterial", &iDistortionOnBase, sizeof(iDistortionOnBase))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialTime", &Color.fRadialTime, sizeof(Color.fRadialTime))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialIntensity", &Color.fRadialIntensity, sizeof(Color.fRadialIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DissolveAmount", &Dissolve, sizeof(Dissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise", &iHasNoise, sizeof(iHasNoise))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask", &iHasMask, sizeof(iHasMask))) ||
		BindFailed(pShader->Bind_RawValue("g_HasEmissive", &iHasEmissive, sizeof(iHasEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_HasDissolve", &iHasDissolve, sizeof(iHasDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasBase2", &iHasBase2, sizeof(iHasBase2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask2", &iHasMask2, sizeof(iHasMask2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise2", &iHasNoise2, sizeof(iHasNoise2))) ||
		BindFailed(pShader->Bind_Texture("g_Base2Texture", iHasBase2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Mask2Texture", iHasMask2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Noise2Texture", iHasNoise2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_BaseTexture",
			nullptr != Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) ?
				Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) :
				m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_NoiseTexture", iHasNoise ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_MaskTexture", iHasMask ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_EmissiveTexture", iHasEmissive ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_DissolveTexture", iHasDissolve ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture0",
			Resource.SourceTextures[0] ? Resource.SourceTextures[0] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture1",
			Resource.SourceTextures[1] ? Resource.SourceTextures[1] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture2",
			Resource.SourceTextures[2] ? Resource.SourceTextures[2] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture3",
			Resource.SourceTextures[3] ? Resource.SourceTextures[3] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture4",
			Resource.SourceTextures[4] ? Resource.SourceTextures[4] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture5",
			Resource.SourceTextures[5] ? Resource.SourceTextures[5] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture6",
			Resource.SourceTextures[6] ? Resource.SourceTextures[6] : m_pBlackTexture));
	if (bBindFailed)
	{
		return Fail_RenderOperation(
			"Material bind failed: common constants/textures block.",
			hFirstBindFailure);
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestMaterialBinding();
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Mesh(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride,
	const float4_t* pDynamicParameter,
	const EFFECT_SUBUV_FRAME_DESC* pSubUVOverride)
{
	if (nullptr == Resource.pModel || nullptr == Element.pElement ||
		nullptr == m_pMeshShader)
		return Fail_RenderOperation(
			"Mesh resource/model/shader contract is missing.", E_FAIL, true);
	uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Mesh render-profile pass is invalid.", E_INVALIDARG, true);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = Resource.pMaterialProgramBinding;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Resource.iStandardColorV1Enabled == 1u &&
			Resource.StandardColorV1Header[0u] == 1u &&
			Resource.StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			Resource.StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != Resource.iRuntimeMaterialV2Enabled &&
			Resource.iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Element.pElement->strElementId).get() !=
				pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL ||
			Element.pElement->eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			!Element.pElement->SourceRecipe.bEnabled ||
			Element.pElement->SourceRecipe.strRendererShape != "mesh" ||
			nullptr == Find_Binding(
				*Element.pElement, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			Element.pElement->Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			iPass != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
					SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound Mesh material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	const float4x4_t& World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	const bool_t bMainSourceReplay =
		0u != Resource.iRuntimeMaterialV2Enabled &&
		(3u == Resource.iRuntimeMaterialV2Opcode ||
			8u == Resource.iRuntimeMaterialV2Opcode);
	const bool_t bLanceDragonMaskedReplay =
		0u != Resource.iRuntimeMaterialV2Enabled &&
		LANCE_DRAGON_MASKED_OPCODE == Resource.iRuntimeMaterialV2Opcode;
	const bool_t bFlow02RecoveredEquation =
		7u == Resource.iArtistVisualV4Opcode;
	if ((bMainSourceReplay || bLanceDragonMaskedReplay ||
		bFlow02RecoveredEquation) &&
		nullptr == pDynamicParameter)
	{
		// These source occurrences carry ParameterDynamic.  UE3's missing
		// payload fallback is ones, not this renderer's generic zero value;
		// reject the missing carrier instead of silently selecting a default.
		return Fail_RenderOperation(
			"Mesh source-replay dynamic payload is missing.", E_INVALIDARG, true);
	}
	const float4_t DynamicParameter = nullptr == pDynamicParameter ?
		float4_t{} : *pDynamicParameter;
	if (0u != Resource.iRuntimeMaterialV2Enabled &&
		1u == Resource.iRuntimeMaterialV2Opcode)
	{
		if (!std::isfinite(DynamicParameter.x))
			return Fail_RenderOperation(
				"Mesh RuntimeMaterialV2 dynamic alpha is non-finite.",
				E_INVALIDARG, true);
		// active004's exact named dynamic `alpha` lane owns visibility.  At
		// zero the dissolve equation is guaranteed to clip every pixel, so
		// report a typed suppression instead of laundering a zero-pixel draw
		// into a submitted occurrence.
		if (DynamicParameter.x <= 0.f)
			return S_FALSE;
	}
	if (bMainSourceReplay)
	{
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Mesh source-replay carrier is non-finite.",
				E_INVALIDARG, true);
		}
		// Particle alpha is multiplicative in both recovered opacity programs.
		// Dynamic X is a UV offset and Dynamic Z is a dissolve threshold, so
		// neither lane is a valid CPU zero-pixel predicate.
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	if (bLanceDragonMaskedReplay)
	{
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Lance dragon typed mesh carrier is non-finite.",
				E_INVALIDARG, true);
		}
		/* Dynamic W owns dissolve while ParticleColor alpha owns the lifetime
		   envelope.  Only the latter has a texture-independent zero predicate. */
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	if (bFlow02RecoveredEquation)
	{
		constexpr f32_t fDynamicTolerance = 1e-5f;
		if (!std::isfinite(DynamicParameter.x) ||
			!std::isfinite(DynamicParameter.y) ||
			!std::isfinite(DynamicParameter.z) ||
			!std::isfinite(DynamicParameter.w) ||
			std::abs(DynamicParameter.x - 1.f) > fDynamicTolerance ||
			DynamicParameter.y < -fDynamicTolerance ||
			DynamicParameter.y > 2.f + fDynamicTolerance ||
			std::abs(DynamicParameter.z - 1.f) > fDynamicTolerance ||
			std::abs(DynamicParameter.w - 1.f) > fDynamicTolerance ||
			!std::isfinite(Element.Color.vColorMultiply.x) ||
			!std::isfinite(Element.Color.vColorMultiply.y) ||
			!std::isfinite(Element.Color.vColorMultiply.z) ||
			!std::isfinite(Element.Color.vColorMultiply.w))
		{
			return Fail_RenderOperation(
				"Mesh flow-02 recovered dynamic carrier changed.",
				E_INVALIDARG, true);
		}
		if (Element.Color.vColorMultiply.w <= 0.f)
			return S_FALSE;
	}
	const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
	float4x4_t NormalMatrix = World;
	const matrix_t LoadedWorld = XMLoadFloat4x4(&World);
	const vector_t Determinant = XMMatrixDeterminant(LoadedWorld);
	const f32_t fDeterminant = XMVectorGetX(Determinant);
	if (!std::isfinite(fDeterminant))
		return Fail_RenderOperation(
			"Mesh world determinant is non-finite.", E_INVALIDARG, true);
	// A zero SizeMultiplyLife sample is a valid no-pixel state at the
	// occurrence boundary.  It must not abort the whole effect frame.
	if (std::abs(fDeterminant) <= std::numeric_limits<f32_t>::epsilon())
		return S_FALSE;
	if (nullptr != pMaterialProgramBinding && fDeterminant < 0.f)
	{
		/* Registry pass 3/4 is the nominal one-sided policy. A negative world
		   determinant reverses winding, so the actual CModel draw must use the
		   shader's matching front-cull receipt. Two-sided pass 1/2 is invariant. */
		if (iPass == 3u)
			iPass = 5u;
		else if (iPass == 4u)
			iPass = 6u;
	}
	XMStoreFloat4x4(&NormalMatrix,
		XMMatrixTranspose(XMMatrixInverse(nullptr, LoadedWorld)));
	HRESULT hResult = m_pMeshShader->Bind_Matrix("g_WorldMatrix", &World);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_WorldMatrix.", hResult);
	hResult = m_pMeshShader->Bind_Matrix("g_NormalMatrix", &NormalMatrix);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_NormalMatrix.", hResult);
	hResult = m_pMeshShader->Bind_RawValue("g_CameraPosition",
		&CameraPosition, sizeof(CameraPosition));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_CameraPosition.", hResult);
	hResult = m_pMeshShader->Bind_RawValue("g_EffectDynamicParameter",
		&DynamicParameter, sizeof(DynamicParameter));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_EffectDynamicParameter.", hResult);
	const bool_t bStandardColorSubUV =
		0u != Resource.iStandardColorV1Enabled && nullptr != pSubUVOverride;
	const uint32_t iStandardColorSubUVEnabled =
		bStandardColorSubUV ? 1u : 0u;
	const EFFECT_SUBUV_FRAME_DESC IdentitySubUV{};
	const EFFECT_SUBUV_FRAME_DESC& StandardColorSubUV =
		bStandardColorSubUV ? *pSubUVOverride : IdentitySubUV;
	const auto IsFiniteFloat4 = [](const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	};
	if (!IsFiniteFloat4(StandardColorSubUV.Current) ||
		!IsFiniteFloat4(StandardColorSubUV.Next) ||
		!std::isfinite(StandardColorSubUV.fBlend) ||
		StandardColorSubUV.fBlend < 0.f ||
		StandardColorSubUV.fBlend > 1.f)
	{
		return Fail_RenderOperation(
			"Mesh StandardColorV1 SubUV carrier is invalid.",
			E_INVALIDARG, true);
	}
	hResult = m_pMeshShader->Bind_RawValue(
		"g_StandardColorV1MeshSubUVEnabled",
		&iStandardColorSubUVEnabled,
		sizeof(iStandardColorSubUVEnabled));
	if (SUCCEEDED(hResult))
		hResult = m_pMeshShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVCurrent",
			&StandardColorSubUV.Current,
			sizeof(StandardColorSubUV.Current));
	if (SUCCEEDED(hResult))
		hResult = m_pMeshShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVNext",
			&StandardColorSubUV.Next,
			sizeof(StandardColorSubUV.Next));
	if (SUCCEEDED(hResult))
		hResult = m_pMeshShader->Bind_RawValue(
			"g_StandardColorV1MeshSubUVBlend",
			&StandardColorSubUV.fBlend,
			sizeof(StandardColorSubUV.fBlend));
	if (FAILED(hResult))
	{
		return Fail_RenderOperation(
			"Mesh shader bind failed: StandardColorV1 SubUV packet.",
			hResult, true);
	}
	hResult = Bind_Common(m_pMeshShader, Element, Resource, fAlphaScale);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh common/material shader bind failed.", hResult);

	const ComPtr<ID3D11ShaderResourceView> BaseOverride =
		0u != Resource.iStandardColorV1Enabled ? Resource.SourceTextures[0u] :
		(Element.pElement->Detail.Mesh.bUseModelMaterial ? nullptr :
		 Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE));
	const uint32_t iUseBaseOverride = nullptr != BaseOverride ? 1u : 0u;
	hResult = m_pMeshShader->Bind_RawValue(
		"g_UseBaseOverride", &iUseBaseOverride, sizeof(iUseBaseOverride));
	if (FAILED(hResult))
	{
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_UseBaseOverride.", hResult);
	}
	bool_t bSubmitted = false;
	for (uint32_t iMesh = 0u; iMesh < Resource.pModel->Get_NumMeshes(); ++iMesh)
	{
		if (iUseBaseOverride)
		{
			hResult = m_pMeshShader->Bind_Texture("g_BaseTexture", BaseOverride);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Mesh base-override texture bind failed.", hResult);
		}
		else
		{
			hResult = Resource.pModel->Bind_Material(m_pMeshShader,
				"g_BaseTexture", iMesh, aiTextureType_DIFFUSE);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Mesh model diffuse-material bind failed.", hResult);
		}
		hResult = m_pMeshShader->Begin(iPass);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Mesh shader pass apply failed.", hResult);
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (nullptr != pMaterialProgramBinding &&
			!Validate_ActualMaterialAdapterPipeline(
				m_pContext.Get(), pMaterialProgramBinding->Adapter,
				iPass))
		{
			return Fail_RenderOperation(
				"Bound Mesh material adapter actual pass/state/MRT changed.",
				E_FAIL, true);
		}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (nullptr != pMaterialProgramBinding)
			Record_TestCompiledAdapterPipelineValidation();
		Record_TestShaderPassApplication();
#endif
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (0u != Resource.iRuntimeMaterialV2Enabled ||
			0u != Resource.iArtistVisualV4Opcode ||
			0u != Resource.iStandardColorV1Enabled)
		{
			const size_t iSamplerCount = static_cast<size_t>(
				0u != Resource.iStandardColorV1Enabled ?
					Resource.StandardColorV1Header[2u] :
				(0u != Resource.iRuntimeMaterialV2Enabled ?
					Resource.iRuntimeMaterialV2TextureLaneCount :
					std::popcount(Resource.iArtistVisualV4TextureMask)));
			if (iSamplerCount == 0u ||
				iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Mesh typed material sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			Record_TestSamplerBinding();
#endif
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestDrawSelection(
			EFFECT_GPU_RENDER_CARRIER::MESH_CMODEL, iPass);
#endif
		hResult = Resource.pModel->Render(iMesh);
		if (S_OK != hResult)
			return Fail_RenderOperation("Mesh model draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestVIBufferBinding();
		Record_TestIssuedDraw(World);
#endif
		bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CEffectDocumentRenderer::Render_Rect(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride)
{
	if (nullptr == Element.pElement || nullptr == m_pRectShader ||
		nullptr == m_pRect)
		return Fail_RenderOperation(
			"Sprite element/shader/buffer contract is missing.",
			E_INVALIDARG, true);
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	float4x4_t World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	if (Element.pElement->Detail.Sprite.bBillboard)
		World = Make_BillboardWorld(World,
			Element.pElement->Detail.Sprite.fBillboardRollDegrees);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Sprite render-profile pass is invalid.", E_INVALIDARG, true);
	HRESULT hResult = m_pRectShader->Bind_Matrix("g_WorldMatrix", &World);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Sprite shader bind failed: g_WorldMatrix.", hResult);
	hResult = Bind_Common(m_pRectShader, Element, Resource, fAlphaScale);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Sprite common/material shader bind failed.", hResult);
	hResult = m_pRectShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation("Sprite shader pass apply failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if (0u != Resource.iRuntimeMaterialV2Enabled &&
		0u != Resource.iRuntimeMaterialV2TextureLaneCount)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			Resource.iRuntimeMaterialV2TextureLaneCount);
		if (iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Sprite Rect typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
	hResult = m_pRect->Bind_Resources();
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Sprite rectangle buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::SPRITE_RECT, iPass);
#endif
	hResult = m_pRect->Render();
	if (S_OK != hResult)
		return Fail_RenderOperation("Sprite rectangle draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestIssuedDraw(World);
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Decal(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement || nullptr == m_pDecalShader ||
		nullptr == m_pRect)
		return Fail_RenderOperation("Decal element/shader/buffer contract is missing.",
			E_INVALIDARG, true);
	uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation("Decal render-profile pass is invalid.",
			E_INVALIDARG, true);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = Resource.pMaterialProgramBinding;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Resource.iStandardColorV1Enabled == 1u &&
			Resource.StandardColorV1Header[0u] == 1u &&
			Resource.StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			Resource.StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != Resource.iRuntimeMaterialV2Enabled &&
			Resource.iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Element.pElement->strElementId).get() !=
				pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR ||
			Element.pElement->eKind != EFFECT_ELEMENT_KIND::DECAL ||
			nullptr != Find_Binding(
				*Element.pElement, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			nullptr != Resource.pModel ||
			Element.pElement->Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			iPass != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
					SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound LocalDecal material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	const matrix_t World = XMLoadFloat4x4(&Element.World);
	const HRESULT WorldStatus = Validate_DecalProjectionWorld(Element);
	if (S_OK != WorldStatus)
		return FAILED(WorldStatus) ? Fail_RenderOperation(
			"Decal projection world is invalid.", WorldStatus, true) : WorldStatus;
	float4x4_t InverseDecal{};
	const matrix_t Inverse = XMMatrixInverse(nullptr, World);
	XMStoreFloat4x4(&InverseDecal, Inverse);
	EFFECT_DECAL_SHADER_PROJECTION_DESC Projection{};
	if (!Resolve_DecalShaderProjection(Element, Projection))
		return Fail_RenderOperation("Decal shader projection is invalid.",
			E_INVALIDARG, true);
	HRESULT hResult = Bind_MaterialInputs(m_pDecalShader,
		*Element.pElement, Element.Color,
		Element.fLocalTimeSeconds, Element.fNormalizedLife, Resource);
	if (FAILED(hResult))
		return Fail_RenderOperation("Decal material shader bind failed.", hResult);
	hResult = m_pDecalShader->Bind_Matrix(
		"g_DecalWorldInverse", &InverseDecal);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalWorldInverse.", hResult);
	hResult = m_pDecalShader->Bind_Matrix("g_ViewMatrixInverse",
		CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_ViewMatrixInverse.", hResult);
	hResult = m_pDecalShader->Bind_Matrix("g_ProjMatrixInverse",
		CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_ProjMatrixInverse.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalSize", &Projection.vSize, sizeof(Projection.vSize));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalSize.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalDepth", &Projection.fDepth, sizeof(Projection.fDepth));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalDepth.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalEdgeFade", &Projection.fEdgeFade,
		sizeof(Projection.fEdgeFade));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalEdgeFade.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalUp", &Projection.vUp, sizeof(Projection.vUp));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalUp.", hResult);
	hResult = m_pDecalShader->Bind_RawValue(
		"g_DecalNormalCutoff", &Projection.fNormalCutoff,
		sizeof(Projection.fNormalCutoff));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal shader bind failed: g_DecalNormalCutoff.", hResult);
	hResult = CGameInstance::Get().Bind_RT_SRV(
		TEXT("Target_Depth"), m_pDecalShader, "g_DepthTexture");
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal depth render-target bind failed.", hResult);
	hResult = CGameInstance::Get().Bind_RT_SRV(
		TEXT("Target_Normal"), m_pDecalShader, "g_NormalTexture");
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal normal render-target bind failed.", hResult);
	hResult = m_pDecalShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation("Decal shader pass apply failed.", hResult);
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	if (nullptr != pMaterialProgramBinding &&
		!Validate_ActualMaterialAdapterPipeline(
			m_pContext.Get(), pMaterialProgramBinding->Adapter, iPass))
	{
		return Fail_RenderOperation(
			"Bound LocalDecal material adapter actual pass/state/MRT changed.",
			E_FAIL, true);
	}
	if (nullptr != pMaterialProgramBinding &&
		pMaterialProgramBinding->Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL &&
		!Validate_ActualLocalDecalSceneShaderResources(m_pContext.Get()))
	{
		return Fail_RenderOperation(
			"Bound LocalDecal material adapter actual depth/normal SRV changed.",
			E_FAIL, true);
	}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	if (nullptr != pMaterialProgramBinding)
		Record_TestCompiledAdapterPipelineValidation();
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if ((0u != Resource.iRuntimeMaterialV2Enabled &&
		 0u != Resource.iRuntimeMaterialV2TextureLaneCount) ||
		0u != Resource.iStandardColorV1Enabled)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			0u != Resource.iStandardColorV1Enabled ?
				Resource.StandardColorV1Header[2u] :
				Resource.iRuntimeMaterialV2TextureLaneCount);
		if (iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Decal typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
	hResult = m_pRect->Bind_Resources();
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Decal rectangle buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::DECAL_RECT, iPass);
#endif
	hResult = m_pRect->Render();
	if (S_OK != hResult)
		return Fail_RenderOperation("Decal rectangle draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestIssuedDraw(Element.World);
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Element(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement)
		return Fail_RenderOperation(
			"Effect element descriptor is missing.", E_INVALIDARG, true);
	if (Resource.bOccurrenceVisualSuppressed)
		return S_FALSE;
	switch (Element.pElement->eKind)
	{
	case EFFECT_ELEMENT_KIND::MESH:
		return Render_Mesh(Element, Resource);
	case EFFECT_ELEMENT_KIND::SPRITE:
		return Render_Rect(Element, Resource);
	case EFFECT_ELEMENT_KIND::DECAL:
		return Render_Decal(Element, Resource);
	case EFFECT_ELEMENT_KIND::PARTICLE:
	case EFFECT_ELEMENT_KIND::TRAIL:
	case EFFECT_ELEMENT_KIND::LIGHT:
	case EFFECT_ELEMENT_KIND::SCREEN_POST:
		return S_FALSE;
	case EFFECT_ELEMENT_KIND::END:
	default:
		return Fail_RenderOperation(
			"Effect element kind is invalid for rendering.", E_INVALIDARG, true);
	}
}

HRESULT Client::CEffectDocumentRenderer::Render_AfterImages(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_AFTERIMAGE> AfterImages)
{
	bool_t bSubmitted = false;
	for (const EFFECT_EVALUATED_AFTERIMAGE& AfterImage : AfterImages)
	{
		if (nullptr == AfterImage.pElement)
			return Fail_RenderOperation(
				"Afterimage element contract is missing.", E_INVALIDARG, true);
		if (!std::isfinite(AfterImage.fAlpha) || AfterImage.fAlpha < 0.f ||
			AfterImage.fAlpha > 1.f)
		{
			return Fail_RenderOperation(
				"Afterimage alpha contract is invalid.", E_INVALIDARG, true);
		}
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(AfterImage.pElement->strElementId);
		if (nullptr == pResource)
			return Fail_RenderOperation(
				"Afterimage resource contract is missing.", E_FAIL, true);
		if (pResource->bSourceMaterialFallbackBlocked ||
			pResource->bOccurrenceVisualSuppressed)
			continue;
		EFFECT_EVALUATED_ELEMENT Element;
		Element.pElement = AfterImage.pElement;
		Element.World = AfterImage.World;
		Element.Color = AfterImage.pElement->Detail.Color;
		Element.fLocalTimeSeconds = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			AfterImage.pElement->Detail.Timing.fStartDelaySeconds);
		Element.fNormalizedLife = 1.f - AfterImage.fAlpha;
		HRESULT Result = E_INVALIDARG;
		switch (Element.pElement->eKind)
		{
		case EFFECT_ELEMENT_KIND::MESH:
			Result = Render_Mesh(
				Element, *pResource, AfterImage.fAlpha, &AfterImage.World);
			break;
		case EFFECT_ELEMENT_KIND::SPRITE:
			Result = Render_Rect(
				Element, *pResource, AfterImage.fAlpha, &AfterImage.World);
			break;
		default:
			return Fail_RenderOperation(
				"Afterimage renderer kind is not Mesh or Sprite.",
				E_INVALIDARG, true);
		}
		if (FAILED(Result))
			return Result;
		if (S_OK == Result)
			bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}

HRESULT Client::CEffectDocumentRenderer::Render_Particles(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_PARTICLE> Particles)
{
	const EFFECT_ELEMENT_DESC* pSource = nullptr;
	for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
	{
		if (nullptr != Particle.pElement)
		{
			pSource = Particle.pElement;
			break;
		}
	}
	if (nullptr == pSource)
		return S_FALSE;
	Engine::CProfilerScope particleRenderProfile(
		CGameInstance::Get().Get_Profiler(), "Effect.Particle.Render");
	const ELEMENT_RESOURCE* pResource = Find_Resource(pSource->strElementId);
	if (nullptr == pResource)
		return Fail_RenderOperation(
			"Particle resource contract is missing.", E_FAIL, true);
	if (pResource->bSourceMaterialFallbackBlocked ||
		pResource->bOccurrenceVisualSuppressed)
		return S_FALSE;
	const bool_t bTypedDirectSmoke =
		0u != pResource->iRuntimeMaterialV2Enabled &&
		10u == pResource->iRuntimeMaterialV2Opcode &&
		pResource->iRuntimeMaterialV2TextureLaneCount == 1u &&
		pResource->iRuntimeMaterialV2TextureMask == 0x01u;
	if (pSource->Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
		!pSource->Material.SourceMaterial.bEnabled && !bTypedDirectSmoke)
	{
		// Version 10 and older source-material documents are intentionally
		// fail-closed.  They remain loadable for migration, but must not turn
		// into a white fallback particle after the v11 renderer is enabled.
		return S_FALSE;
	}
	if (nullptr != pResource->pModel)
	{
		bool_t bSubmitted = false;
		for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
		{
			if (nullptr == Particle.pElement)
				continue;
			EFFECT_EVALUATED_ELEMENT MeshParticle;
			MeshParticle.pElement = Particle.pElement;
			MeshParticle.World = Apply_ParticleCameraOffset(Particle);
			MeshParticle.Color = Particle.pElement->Detail.Color;
			MeshParticle.Color.vColorMultiply = Particle.Color;
			MeshParticle.fLocalTimeSeconds = (std::max)(0.f,
				Frame.fSampleTimeSeconds -
				Particle.pElement->Detail.Timing.fStartDelaySeconds);
			MeshParticle.fNormalizedLife = Particle.fNormalizedLife;
			const EFFECT_SUBUV_FRAME_DESC SourceSubUV =
				Resolve_SubUVFrames(Particle);
			EFFECT_SUBUV_FRAME_DESC SubUV;
			if (!CEffectPlayback::Resolve_ParticleSpriteSubUV(
					Particle, SourceSubUV, SubUV))
			{
				return Fail_RenderOperation(
					"Mesh particle source SubUV contract is invalid.",
					E_INVALIDARG, true);
			}
			const HRESULT Result = Render_Mesh(MeshParticle, *pResource,
				1.f, nullptr, &Particle.vDynamicParameter, &SubUV);
			if (FAILED(Result))
				return Result;
			bSubmitted = bSubmitted || S_OK == Result;
		}
		return bSubmitted ? S_OK : S_FALSE;
	}
	std::vector<Engine::VTXEFFECT_PARTICLE>& Instances =
		m_ParticleInstanceScratch;
	Instances.clear();
	if (Instances.capacity() < Particles.size())
		Instances.reserve(Particles.size());
	for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
	{
		if (nullptr == Particle.pElement)
			continue;
		if (0u != pResource->iRuntimeMaterialV2Enabled &&
			4u == pResource->iRuntimeMaterialV2Opcode)
		{
			const float4_t& Color = Particle.Color;
			const float4_t& Dynamic = Particle.vDynamicParameter;
			if (!std::isfinite(Color.x) || !std::isfinite(Color.y) ||
				!std::isfinite(Color.z) || !std::isfinite(Color.w) ||
				!std::isfinite(Dynamic.x) || !std::isfinite(Dynamic.y) ||
				!std::isfinite(Dynamic.z) || !std::isfinite(Dynamic.w))
			{
				return Fail_RenderOperation(
					"Particle RuntimeMaterialV2 color/dynamic input is non-finite.",
					E_INVALIDARG, true);
			}
			// active016's bounded opacity is the product of the exact named
			// alpha-dissolve lane and the particle alpha carrier.  Suppress the
			// deterministic zero envelope before issuing a zero-pixel draw.
			if (Color.w <= 0.f || Dynamic.x <= 0.f)
				continue;
		}
		pSource = Particle.pElement;
		float4x4_t World = Particle.World;
		if (Particle.pElement->Detail.Particle.bBillboard &&
			!Make_ParticleSpriteWorld(Particle, World))
		{
			return Fail_RenderOperation(
				"Particle billboard world reconstruction failed.",
				E_INVALIDARG, true);
		}
		const EFFECT_SUBUV_FRAME_DESC SourceSubUV =
			Resolve_SubUVFrames(Particle);
		EFFECT_SUBUV_FRAME_DESC SubUV;
		if (!CEffectPlayback::Resolve_ParticleSpriteSubUV(
			Particle, SourceSubUV, SubUV))
		{
			return Fail_RenderOperation(
				"Particle source image-flip contract is invalid.",
				E_INVALIDARG, true);
		}
		Instances.push_back({ World, Particle.Color,
			Particle.vDynamicParameter,
			SubUV.Current,
			SubUV.Next,
			{ Particle.fNormalizedLife, SubUV.fBlend } });
	}
	if (nullptr == pSource || Instances.empty())
		return S_FALSE;

	const EFFECT_ELEMENT_DESC& Source = *pSource;
	HRESULT hResult = m_pParticleBuffer->Update_Instances(
		std::span<const Engine::VTXEFFECT_PARTICLE>(
			Instances.data(), Instances.size()));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle instance-buffer update failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestGeometryUpload();
#endif
	const f32_t LocalTime = (std::max)(0.f,
		Frame.fSampleTimeSeconds - Source.Detail.Timing.fStartDelaySeconds);
	const f32_t Normalized = std::clamp(
		LocalTime / Source.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
	EFFECT_COLOR_DESC CommonColor =
		Evaluate_CommonColor(Source, Normalized);
	CommonColor.vColorMultiply = float4_t(1.f, 1.f, 1.f, 1.f);
	const std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
		pMaterialProgramBinding = pResource->pMaterialProgramBinding;
	uint32_t iPass = UINT32_MAX;
	if (nullptr != pMaterialProgramBinding)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter =
			pMaterialProgramBinding->Adapter;
		const EFFECT_MATERIAL_EXECUTION_DESC& BoundExecution =
			pMaterialProgramBinding->Execution;
		const bool_t bBoundStandardColor = BoundExecution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			pResource->iStandardColorV1Enabled == 1u &&
			pResource->StandardColorV1Header[0u] == 1u &&
			pResource->StandardColorV1Header[1u] == BoundExecution.iOpcode &&
			pResource->StandardColorV1Header[2u] ==
				BoundExecution.iTextureLaneCount;
		const bool_t bBoundRuntimeMaterial = BoundExecution.eBackend !=
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			0u != pResource->iRuntimeMaterialV2Enabled &&
			pResource->iRuntimeMaterialV2Opcode == BoundExecution.iOpcode;
		if (nullptr == m_pPreparedDocument ||
			nullptr == m_pPreparedDocument->pMaterialProgramRegistry ||
			m_pPreparedDocument->pMaterialProgramRegistry->Get_CatalogRevision() !=
				pMaterialProgramBinding->iCatalogRevision ||
			m_pPreparedDocument->iMaterialProgramRegistryGeneration !=
				pMaterialProgramBinding->iRegistryGenerationId ||
			m_pPreparedDocument->pMaterialProgramRegistry->Resolve(
					Get_StagedDocument().strEffectAssetId,
					Source.strElementId).get() != pMaterialProgramBinding.get() ||
			pMaterialProgramBinding->eInlineMirrorPolicy !=
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED ||
			!Is_CompiledMaterialAdapter(Adapter) ||
			Adapter.eCarrier !=
				EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE ||
			Source.eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			!Source.SourceRecipe.bEnabled ||
			Source.SourceRecipe.strRendererShape != "sprite" ||
			nullptr != Find_Binding(Source, EFFECT_RESOURCE_SLOT::MESH_MODEL) ||
			nullptr != pResource->pModel || nullptr == m_pParticleShader ||
			nullptr == m_pParticleBuffer ||
			Source.Material.eRenderProfile != Adapter.eRenderProfile ||
			(!bBoundStandardColor && !bBoundRuntimeMaterial) ||
			Select_Pass(Source.Material.eRenderProfile) != Adapter.iPassIndex ||
			Engine::CRenderOutputContract::Get_Active() !=
				Engine::RENDER_OUTPUT_CONTRACT::
				SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION)
		{
			return Fail_RenderOperation(
				"Bound Sprite material adapter draw contract changed.",
				E_FAIL, true);
		}
		iPass = Adapter.iPassIndex;
	}
	hResult = Bind_Common(m_pParticleShader, Source, CommonColor,
		LocalTime, Normalized, *pResource);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle common/material shader bind failed.", hResult);
	if (nullptr == pMaterialProgramBinding)
		iPass = Select_Pass(Source.Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Particle render-profile pass is invalid.", E_INVALIDARG, true);
	hResult = m_pParticleShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle shader pass apply failed.", hResult);
	if (nullptr != pMaterialProgramBinding)
	{
#if defined(_DEBUG) || \
	defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (!Validate_ActualMaterialAdapterPipeline(
				m_pContext.Get(), pMaterialProgramBinding->Adapter, iPass))
		{
			return Fail_RenderOperation(
				"Bound Sprite material adapter actual pass/state/MRT changed.",
				E_FAIL, true);
		}
#endif
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestCompiledAdapterPipelineValidation();
#endif
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestShaderPassApplication();
#endif
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if ((0u != pResource->iRuntimeMaterialV2Enabled &&
		0u != pResource->iRuntimeMaterialV2TextureLaneCount) ||
		0u != pResource->iArtistVisualV4Opcode ||
		0u != pResource->iStandardColorV1Enabled)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			0u != pResource->iStandardColorV1Enabled ?
				pResource->StandardColorV1Header[2u] :
			(0u != pResource->iRuntimeMaterialV2Enabled ?
				pResource->iRuntimeMaterialV2TextureLaneCount :
				std::popcount(pResource->iArtistVisualV4TextureMask)));
		if (iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Particle typed material sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestSamplerBinding();
#endif
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestDrawSelection(
		EFFECT_GPU_RENDER_CARRIER::SPRITE_INSTANCE, iPass);
#endif
	hResult = m_pParticleBuffer->Render();
	if (S_OK != hResult)
		return Fail_RenderOperation(
			"Particle instance-buffer draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestVIBufferBinding();
	Record_TestIssuedDraw(std::span<const Engine::VTXEFFECT_PARTICLE>(
		Instances.data(), Instances.size()));
#endif
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Trails(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_TRAIL> Trails)
{
	if (Trails.empty())
		return S_FALSE;
	Engine::CProfilerScope trailRenderProfile(
		CGameInstance::Get().Get_Profiler(), "Effect.Trail.Render");
	bool_t bSubmitted = false;
	const vector_t CameraPosition = XMLoadFloat4(
		CGameInstance::Get().Get_CamPosition());
	for (const EFFECT_EVALUATED_TRAIL& Trail : Trails)
	{
		const bool_t bBakedEdgeHistory = Trail.EdgePairs.size() >= 2u;
		if (nullptr == Trail.pElement ||
			(!bBakedEdgeHistory && Trail.Points.size() < 2u))
			continue;
		if (bBakedEdgeHistory && !Trail.Points.empty())
			return Fail_RenderOperation(
				"Trail carries both centerline and baked-edge geometry.",
				E_INVALIDARG, true);
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(Trail.pElement->strElementId);
		if (nullptr == pResource)
			return Fail_RenderOperation(
				"Trail resource contract is missing.", E_FAIL, true);
		if (pResource->bSourceMaterialFallbackBlocked ||
			pResource->bOccurrenceVisualSuppressed)
			continue;
		const bool_t bRuntimeMaterialV2Ribbon =
			0u != pResource->iRuntimeMaterialV2Enabled &&
			(9u == pResource->iRuntimeMaterialV2Opcode ||
			 20u == pResource->iRuntimeMaterialV2Opcode);
		const bool_t bRibbonLiquid01ParentDefault =
			bRuntimeMaterialV2Ribbon &&
			20u == pResource->iRuntimeMaterialV2Opcode;
		const bool_t bStandardColorV1 =
			0u != pResource->iStandardColorV1Enabled;
		const bool_t bTypedArtistRibbon =
			bRuntimeMaterialV2Ribbon && !bBakedEdgeHistory;
		const bool_t bFlowRibbon01 =
			35u == pResource->iSourceMaterialProfile && !bBakedEdgeHistory;
		const bool_t bTypedSourceRibbon =
			bTypedArtistRibbon || bFlowRibbon01;
		if (35u == pResource->iSourceMaterialProfile && bBakedEdgeHistory)
		{
			return Fail_RenderOperation(
				"FlowRibbon01 cannot consume baked-edge AnimationTrail geometry.",
				E_INVALIDARG, true);
		}
		if (0u != pResource->iRuntimeMaterialV2Enabled &&
			!bRuntimeMaterialV2Ribbon)
		{
			return Fail_RenderOperation(
				"Trail RuntimeMaterialV2 opcode is invalid.", E_INVALIDARG, true);
		}
		const f32_t fTilingDistance =
			Trail.pElement->Detail.Trail.fTilingDistanceWorldUnits;
		const f32_t fTessellationStep =
			Trail.pElement->Detail.Trail.fDistanceTessellationStepWorldUnits;
		constexpr f32_t ARTIST_RIBBON_TILING_DISTANCE = 6.f;
		constexpr f32_t RIBBON_LIQUID_TILING_DISTANCE = 3.f;
		constexpr f32_t ARTIST_RIBBON_TESSELLATION_STEP = 0.05f;
		constexpr uint32_t ARTIST_RIBBON_MAX_SUBDIVISIONS = 25u;
		const f32_t fExpectedTypedTilingDistance =
			bRibbonLiquid01ParentDefault ? RIBBON_LIQUID_TILING_DISTANCE :
			ARTIST_RIBBON_TILING_DISTANCE;
		if (bTypedArtistRibbon &&
			(!std::isfinite(fTilingDistance) ||
				!std::isfinite(fTessellationStep) ||
				std::abs(fTilingDistance - fExpectedTypedTilingDistance) > 1e-6f ||
				std::abs(fTessellationStep - ARTIST_RIBBON_TESSELLATION_STEP) > 1e-6f))
		{
			return Fail_RenderOperation(
				"Trail typed tiling/tessellation contract is invalid.",
				E_INVALIDARG, true);
		}
		if (bFlowRibbon01 &&
			(!std::isfinite(fTilingDistance) || fTilingDistance <= 0.f ||
			 !std::isfinite(fTessellationStep) || fTessellationStep <= 0.f))
		{
			return Fail_RenderOperation(
				"FlowRibbon01 tiling/tessellation contract is invalid.",
				E_INVALIDARG, true);
		}

		std::vector<EFFECT_EVALUATED_TRAIL_POINT>& TessellatedPoints =
			m_TrailPointScratch;
		TessellatedPoints.clear();
		std::span<const EFFECT_EVALUATED_TRAIL_POINT> RenderPoints(
			Trail.Points.data(), Trail.Points.size());
		if (bTypedSourceRibbon)
		{
			const auto ValidatePoint = [pResource, &Trail, bFlowRibbon01,
				bRibbonLiquid01ParentDefault](
				const EFFECT_EVALUATED_TRAIL_POINT& Point)
			{
				const uint32_t iColorMask =
					Point.iSourceColorComponentMask & 0x0fu;
				const uint32_t iDynamicMask =
					Point.iDynamicParameterComponentMask & 0x0fu;
				const bool_t bArtistCarrierContract =
					Trail.pElement->SourceRecipe.bEnabled &&
					!bRibbonLiquid01ParentDefault ?
					(iColorMask == 0x08u && iDynamicMask == 0x0fu) :
					((iColorMask &
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask) ==
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask &&
					 (iDynamicMask &
						pResource->iRuntimeMaterialV2DynamicConsumedMask) ==
							pResource->iRuntimeMaterialV2DynamicConsumedMask);
				const bool_t bFlowCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.y) <= 1e-4f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.z) <= 1e-4f &&
					std::abs(Point.vDynamicParameter.x - 0.02f) <= 1e-5f &&
					std::abs(Point.vDynamicParameter.y - 1.f) <= 1e-5f &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f;
				return (bFlowRibbon01 ? bFlowCarrierContract :
					bArtistCarrierContract) &&
					std::isfinite(Point.vWorldPosition.x) &&
					std::isfinite(Point.vWorldPosition.y) &&
					std::isfinite(Point.vWorldPosition.z) &&
					std::isfinite(Point.fNormalizedAge) &&
					Point.fNormalizedAge >= 0.f && Point.fNormalizedAge < 1.f &&
					std::isfinite(Point.fCumulativeDistance) &&
					Point.fCumulativeDistance >= 0.f &&
					std::isfinite(Point.vSourceColor.x) &&
					std::isfinite(Point.vSourceColor.y) &&
					std::isfinite(Point.vSourceColor.z) &&
					std::isfinite(Point.vSourceColor.w) &&
					std::isfinite(Point.vDynamicParameter.x) &&
					std::isfinite(Point.vDynamicParameter.y) &&
					std::isfinite(Point.vDynamicParameter.z) &&
					std::isfinite(Point.vDynamicParameter.w);
			};
			if (!std::all_of(Trail.Points.begin(), Trail.Points.end(), ValidatePoint))
				return Fail_RenderOperation(
					"Trail typed point payload is invalid.", E_INVALIDARG, true);
		}

		const bool_t bDistanceTessellated = bTypedSourceRibbon ||
			(std::isfinite(fTilingDistance) && fTilingDistance > 0.f &&
			 std::isfinite(fTessellationStep) && fTessellationStep > 0.f);
		if (bDistanceTessellated)
		{
			TessellatedPoints.reserve(1u +
				(Trail.Points.size() - 1u) * ARTIST_RIBBON_MAX_SUBDIVISIONS);
			TessellatedPoints.push_back(Trail.Points.front());
			for (size_t iPoint = 1u; iPoint < Trail.Points.size(); ++iPoint)
			{
				const EFFECT_EVALUATED_TRAIL_POINT& PreviousPoint =
					Trail.Points[iPoint - 1u];
				const EFFECT_EVALUATED_TRAIL_POINT& NextPoint =
					Trail.Points[iPoint];
				if (NextPoint.fCumulativeDistance + 1e-6f <
					PreviousPoint.fCumulativeDistance)
				{
					return Fail_RenderOperation(
						"Trail cumulative distance is not monotonic.",
						E_INVALIDARG, true);
				}
				const vector_t PreviousPosition =
					XMLoadFloat3(&PreviousPoint.vWorldPosition);
				const vector_t NextPosition = XMLoadFloat3(&NextPoint.vWorldPosition);
				const f32_t fSegmentDistance = XMVectorGetX(
					XMVector3Length(NextPosition - PreviousPosition));
				if (!std::isfinite(fSegmentDistance))
					return Fail_RenderOperation(
						"Trail segment distance is non-finite.",
						E_INVALIDARG, true);
				const f32_t fSubdivisionCount = std::clamp(
					static_cast<f32_t>(std::ceil(
						fSegmentDistance / fTessellationStep)),
					1.f, static_cast<f32_t>(ARTIST_RIBBON_MAX_SUBDIVISIONS));
				const uint32_t iSubdivisions =
					static_cast<uint32_t>(fSubdivisionCount);
				for (uint32_t iSubdivision = 1u;
					iSubdivision <= iSubdivisions; ++iSubdivision)
				{
					const f32_t fRatio = static_cast<f32_t>(iSubdivision) /
						static_cast<f32_t>(iSubdivisions);
					EFFECT_EVALUATED_TRAIL_POINT Point;
					XMStoreFloat3(&Point.vWorldPosition,
						XMVectorLerp(PreviousPosition, NextPosition, fRatio));
					Point.fNormalizedAge = PreviousPoint.fNormalizedAge +
						(NextPoint.fNormalizedAge - PreviousPoint.fNormalizedAge) * fRatio;
					Point.fCumulativeDistance = PreviousPoint.fCumulativeDistance +
						(NextPoint.fCumulativeDistance -
							PreviousPoint.fCumulativeDistance) * fRatio;
					Point.fSourceWidth = PreviousPoint.fSourceWidth +
						(NextPoint.fSourceWidth - PreviousPoint.fSourceWidth) * fRatio;
					XMStoreFloat4(&Point.vSourceColor,
						XMVectorLerp(XMLoadFloat4(&PreviousPoint.vSourceColor),
							XMLoadFloat4(&NextPoint.vSourceColor), fRatio));
					XMStoreFloat4(&Point.vDynamicParameter,
						XMVectorLerp(XMLoadFloat4(&PreviousPoint.vDynamicParameter),
							XMLoadFloat4(&NextPoint.vDynamicParameter), fRatio));
					Point.iSourceColorComponentMask =
						PreviousPoint.iSourceColorComponentMask &
						NextPoint.iSourceColorComponentMask;
					Point.iDynamicParameterComponentMask =
						PreviousPoint.iDynamicParameterComponentMask &
						NextPoint.iDynamicParameterComponentMask;
					TessellatedPoints.push_back(Point);
				}
			}
			RenderPoints = std::span<const EFFECT_EVALUATED_TRAIL_POINT>(
				TessellatedPoints.data(), TessellatedPoints.size());
		}

		std::vector<Engine::VTXEFFECT_TRAIL>& Vertices = m_TrailVertexScratch;
		std::vector<uint32_t>& Indices = m_TrailIndexScratch;
		Vertices.clear();
		Indices.clear();
		const size_t iGeometryPointCount = bBakedEdgeHistory ?
			Trail.EdgePairs.size() : RenderPoints.size();
		if (Vertices.capacity() < iGeometryPointCount * 2u)
			Vertices.reserve(iGeometryPointCount * 2u);
		if (Indices.capacity() < (iGeometryPointCount - 1u) * 6u)
			Indices.reserve((iGeometryPointCount - 1u) * 6u);
		if (bBakedEdgeHistory)
		{
			for (size_t iPair = 0u; iPair < Trail.EdgePairs.size(); ++iPair)
			{
				const EFFECT_EVALUATED_TRAIL_EDGE_PAIR& Pair =
					Trail.EdgePairs[iPair];
				const EFFECT_EVALUATED_TRAIL_POINT& Point = Pair.Payload;
				const auto IsFinite3 = [](const float3_t& Value)
				{
					return std::isfinite(Value.x) && std::isfinite(Value.y) &&
						std::isfinite(Value.z);
				};
				if (!IsFinite3(Pair.vFirstEdgeWorld) ||
					!IsFinite3(Pair.vControlPointWorld) ||
					!IsFinite3(Pair.vSecondEdgeWorld) ||
					!std::isfinite(Point.fNormalizedAge) ||
					Point.fNormalizedAge < 0.f || Point.fNormalizedAge >= 1.f ||
					!std::isfinite(Point.fCumulativeDistance) ||
					Point.fCumulativeDistance < 0.f)
				{
					return Fail_RenderOperation(
						"Baked-edge AnimationTrail geometry is invalid.",
						E_INVALIDARG, true);
				}
				if (bRuntimeMaterialV2Ribbon)
				{
					const uint32_t iColorMask =
						Point.iSourceColorComponentMask & 0x0fu;
					const uint32_t iDynamicMask =
						Point.iDynamicParameterComponentMask & 0x0fu;
					if ((iColorMask &
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask) !=
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask ||
						(iDynamicMask &
							pResource->iRuntimeMaterialV2DynamicConsumedMask) !=
						pResource->iRuntimeMaterialV2DynamicConsumedMask)
					{
						return Fail_RenderOperation(
							"Baked-edge AnimationTrail material carrier is incomplete.",
							E_INVALIDARG, true);
					}
				}
				const f32_t U = fTilingDistance > 0.f ?
					Point.fCumulativeDistance / fTilingDistance :
					static_cast<f32_t>(iPair);
				const float4_t Color = bRuntimeMaterialV2Ribbon ?
					float4_t(1.f, 1.f, 1.f, Point.vSourceColor.w) :
					float4_t(1.f, 1.f, 1.f, 1.f - Point.fNormalizedAge);
				Vertices.push_back({ Pair.vFirstEdgeWorld, float2_t(U, 0.f),
					Color, Point.vDynamicParameter });
				Vertices.push_back({ Pair.vSecondEdgeWorld, float2_t(U, 1.f),
					Color, Point.vDynamicParameter });
			}
		}
		else for (size_t iPoint = 0u; iPoint < RenderPoints.size(); ++iPoint)
		{
			const EFFECT_EVALUATED_TRAIL_POINT& Point = RenderPoints[iPoint];
			if (bTypedSourceRibbon)
			{
				const uint32_t iColorMask =
					Point.iSourceColorComponentMask & 0x0fu;
				const uint32_t iDynamicMask =
					Point.iDynamicParameterComponentMask & 0x0fu;
				const bool_t bArtistCarrierContract =
					Trail.pElement->SourceRecipe.bEnabled &&
					!bRibbonLiquid01ParentDefault ?
					(iColorMask == 0x08u && iDynamicMask == 0x0fu) :
					((iColorMask &
						pResource->iRuntimeMaterialV2ParticleColorConsumedMask) ==
							pResource->iRuntimeMaterialV2ParticleColorConsumedMask &&
					 (iDynamicMask &
						pResource->iRuntimeMaterialV2DynamicConsumedMask) ==
							pResource->iRuntimeMaterialV2DynamicConsumedMask);
				const bool_t bFlowCarrierContract =
					iColorMask == 0x0fu && iDynamicMask == 0x0fu &&
					std::isfinite(Point.fSourceWidth) && Point.fSourceWidth >= 0.f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.y) <= 1e-4f &&
					std::abs(Point.vSourceColor.x - Point.vSourceColor.z) <= 1e-4f &&
					std::abs(Point.vDynamicParameter.x - 0.02f) <= 1e-5f &&
					std::abs(Point.vDynamicParameter.y - 1.f) <= 1e-5f;
				if (!(bFlowRibbon01 ? bFlowCarrierContract :
						bArtistCarrierContract) ||
					!std::isfinite(Point.fCumulativeDistance) ||
					Point.fCumulativeDistance < 0.f ||
					!std::isfinite(Point.vSourceColor.w) ||
					!std::isfinite(Point.vDynamicParameter.x) ||
					!std::isfinite(Point.vDynamicParameter.y) ||
					!std::isfinite(Point.vDynamicParameter.z) ||
					!std::isfinite(Point.vDynamicParameter.w))
				{
					return Fail_RenderOperation(
						"Trail tessellated point payload is invalid.",
						E_INVALIDARG, true);
				}
			}
			const vector_t Position = XMLoadFloat3(
				&Point.vWorldPosition);
			const vector_t Previous = XMLoadFloat3(&RenderPoints[
				iPoint > 0u ? iPoint - 1u : iPoint].vWorldPosition);
			const vector_t Next = XMLoadFloat3(&RenderPoints[
				iPoint + 1u < RenderPoints.size() ? iPoint + 1u : iPoint].vWorldPosition);
			vector_t Tangent;
			if (!Normalize_Safe(Next - Previous, Tangent))
				continue;
			vector_t Side;
			if (Trail.pElement->Detail.Trail.bFaceCamera)
			{
				const vector_t View = CameraPosition - Position;
				if (!Normalize_Safe(XMVector3Cross(View, Tangent), Side))
					continue;
			}
			else if (!Normalize_Safe(XMVector3Cross(
				XMVectorSet(0.f, 1.f, 0.f, 0.f), Tangent), Side) &&
				!Normalize_Safe(XMVector3Cross(
					XMVectorSet(1.f, 0.f, 0.f, 0.f), Tangent), Side))
			{
				continue;
			}
			const f32_t Age = Point.fNormalizedAge;
			const f32_t Width = bFlowRibbon01 ? Point.fSourceWidth :
				Trail.pElement->Detail.Trail.fStartWidth +
				(Trail.pElement->Detail.Trail.fEndWidth -
					Trail.pElement->Detail.Trail.fStartWidth) * Age;
			const vector_t HalfSide = Side * (Width * 0.5f);
			const f32_t U = fTilingDistance > 0.f ?
				Point.fCumulativeDistance / fTilingDistance :
				static_cast<f32_t>(iPoint);
			const float4_t Color = bFlowRibbon01 ?
				float4_t(Point.vSourceColor.x, Point.vDynamicParameter.z,
					Point.vDynamicParameter.w, Point.vSourceColor.w) :
				(bTypedArtistRibbon ?
					float4_t(1.f, 1.f, 1.f, Point.vSourceColor.w) :
					float4_t(1.f, 1.f, 1.f, 1.f - Age));
			Vertices.push_back({ To_Float3(Position - HalfSide),
				float2_t(U, 0.f), Color, Point.vDynamicParameter });
			Vertices.push_back({ To_Float3(Position + HalfSide),
				float2_t(U, 1.f), Color, Point.vDynamicParameter });
		}
		if (Vertices.size() < 4u)
			continue;
		const uint32_t iPairs = static_cast<uint32_t>(Vertices.size() / 2u);
		for (uint32_t iPair = 0u; iPair + 1u < iPairs; ++iPair)
		{
			const uint32_t Base = iPair * 2u;
			Indices.insert(Indices.end(),
				{ Base, Base + 1u, Base + 2u,
				  Base + 1u, Base + 3u, Base + 2u });
		}
		HRESULT hResult = m_pTrailBuffer->Update_Geometry(
			std::span<const Engine::VTXEFFECT_TRAIL>(
				Vertices.data(), Vertices.size()),
			std::span<const uint32_t>(Indices.data(), Indices.size()));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail geometry-buffer update failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestTrailGeometryUpload(
			std::span<const Engine::VTXEFFECT_TRAIL>(
				Vertices.data(), Vertices.size()));
#endif
		const f32_t LocalTime = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			Trail.pElement->Detail.Timing.fStartDelaySeconds);
		const f32_t Normalized = std::clamp(LocalTime /
			Trail.pElement->Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());
		const EFFECT_COLOR_DESC CommonColor =
			Evaluate_CommonColor(*Trail.pElement, Normalized);
		hResult = m_pTrailShader->Bind_Matrix("g_WorldMatrix", &Identity);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail shader bind failed: g_WorldMatrix.", hResult);
		hResult = Bind_Common(m_pTrailShader, *Trail.pElement,
			CommonColor, LocalTime, Normalized, *pResource);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail common/material shader bind failed.", hResult);
		const uint32_t iPass = Select_Pass(
			Trail.pElement->Material.eRenderProfile);
		if (UINT32_MAX == iPass)
			return Fail_RenderOperation(
				"Trail render-profile pass is invalid.", E_INVALIDARG, true);
		hResult = m_pTrailShader->Begin(iPass);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail shader pass apply failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestShaderPassApplication();
#endif
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (bTypedArtistRibbon || bStandardColorV1)
		{
			const size_t iSamplerCount = static_cast<size_t>(
				bStandardColorV1 ? pResource->StandardColorV1Header[2u] :
					pResource->iRuntimeMaterialV2TextureLaneCount);
			const size_t iExpectedRuntimeSamplerCount =
				bRibbonLiquid01ParentDefault ? 4u : 2u;
			if ((!bStandardColorV1 &&
				 iSamplerCount != iExpectedRuntimeSamplerCount) ||
				iSamplerCount == 0u ||
				iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Trail typed material sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			Record_TestSamplerBinding();
#endif
		}
		hResult = m_pTrailBuffer->Bind_Resources();
		if (S_OK != hResult)
			return Fail_RenderOperation(
				"Trail geometry-buffer bind failed.", hResult, true);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestVIBufferBinding();
		Record_TestDrawSelection(
			EFFECT_GPU_RENDER_CARRIER::RIBBON_DYNAMIC_TRAIL, iPass);
#endif
		hResult = m_pTrailBuffer->Render();
		if (S_OK != hResult)
			return Fail_RenderOperation(
				"Trail geometry-buffer draw failed.", hResult);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		Record_TestIssuedDraw(std::span<const Engine::VTXEFFECT_TRAIL>(
			Vertices.data(), Vertices.size()));
#endif
		bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
}

bool_t Client::CEffectDocumentRenderer::Has_NonBlendModelCues() const
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	return std::ranges::any_of(Document.ModelCues,
		[&Document](const EFFECT_MODEL_CUE_DESC& Cue)
		{
			return Is_DimensionSummonCharacterSurfaceCue(Document, Cue);
		});
}

bool_t Client::CEffectDocumentRenderer::Has_WorldMarkElements() const
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	return std::ranges::any_of(Document.Elements,
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.bVisible && Element.eCompositionLayer ==
				EFFECT_COMPOSITION_LAYER::WORLD_MARK;
		});
}

HRESULT Client::CEffectDocumentRenderer::Render_NonBlendModelCues(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	if (!Has_NonBlendModelCues())
		return S_FALSE;
	std::string GateStatus;
	if (!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		m_bLastRenderFailureObjectLocal = true;
		return E_FAIL;
	}
	const HRESULT hResult = Render_ModelCues(Frame, true);
	if (FAILED(hResult))
	{
		m_strStatus = "Effect non-blend animated model-cue rendering failed.";
		if (!m_strRenderFailureDetail.empty())
			m_strStatus += " Operation: " + m_strRenderFailureDetail;
	}
	else
	{
		m_strStatus =
			"Effect non-blend animated model-cue rendering completed.";
	}
	return hResult;
}

HRESULT Client::CEffectDocumentRenderer::Render_ModelCues(
	const EFFECT_EVALUATED_FRAME& Frame,
	const bool_t bNonBlendCharacterSurfaceOnly)
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		const bool_t bCharacterSurface =
			Is_DimensionSummonCharacterSurfaceCue(Document, Cue);
		if (bCharacterSurface != bNonBlendCharacterSurfaceOnly)
			continue;
		const f32_t fLocalTime =
			Frame.fSampleTimeSeconds - Cue.fStartDelaySeconds;
		if (!Cue.bVisible || fLocalTime < 0.f ||
			fLocalTime > Cue.fDurationSeconds)
		{
			continue;
		}
		auto Resource = m_ModelCueResources.find(Cue.strCueId);
		if (Resource == m_ModelCueResources.end() ||
			nullptr == Resource->second.pModel)
		{
			return Fail_RenderOperation(
				"Animated model-cue resource contract is missing.", E_FAIL, true);
		}
		Engine::CModel& Model = *Resource->second.pModel;
		const f32_t fAnimationTime = Cue.bHoldLastFrame ?
			(std::min)(fLocalTime, Resource->second.fDurationSeconds) :
			fLocalTime;
		if (!Model.Set_AnimTrackPosition(Resource->second.iAnimationIndex,
			fAnimationTime * Resource->second.fTicksPerSecond))
		{
			return Fail_RenderOperation(
				"Animated model-cue track position is invalid.", E_FAIL, true);
		}
		Model.Play_Animation(0.f);
		const EFFECT_TRANSFORM_DESC& Transform = Cue.LocalTransform;
		const float3_t Position = {
			Transform.vPosition.x + Transform.vVelocityPerSecond.x * fLocalTime,
			Transform.vPosition.y + Transform.vVelocityPerSecond.y * fLocalTime,
			Transform.vPosition.z + Transform.vVelocityPerSecond.z * fLocalTime };
		const float3_t Rotation = {
			Transform.vRotationDegrees.x +
				Transform.vRevolutionDegreesPerSecond.x * fLocalTime,
			Transform.vRotationDegrees.y +
				Transform.vRevolutionDegreesPerSecond.y * fLocalTime,
			Transform.vRotationDegrees.z +
				Transform.vRevolutionDegreesPerSecond.z * fLocalTime };
		const matrix_t Local =
			XMMatrixScaling(Transform.vScale.x, Transform.vScale.y,
				Transform.vScale.z) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Rotation.x),
				XMConvertToRadians(Rotation.y),
				XMConvertToRadians(Rotation.z)) *
			XMMatrixTranslation(Position.x, Position.y, Position.z);
		float4x4_t World{};
		XMStoreFloat4x4(&World, Local * XMLoadFloat4x4(&Frame.RootWorld));
		HRESULT hResult = m_pAnimatedModelShader->Bind_Matrix(
			"g_WorldMatrix", &World);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_WorldMatrix.", hResult);
		hResult = CGameInstance::Get().Bind_Transform(
			m_pAnimatedModelShader, "g_ViewMatrix", D3DTS::VIEW);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_ViewMatrix.", hResult);
		hResult = CGameInstance::Get().Bind_Transform(
			m_pAnimatedModelShader, "g_ProjMatrix", D3DTS::PROJ);
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: g_ProjMatrix.", hResult);
		hResult = m_pAnimatedModelShader->Bind_RawValue(
			"g_EffectModelCueColorMultiply", &Cue.vColorMultiply,
			sizeof(Cue.vColorMultiply));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: color multiply.", hResult);
		hResult = m_pAnimatedModelShader->Bind_RawValue(
			"g_EffectModelCueOpacity", &Cue.fOpacity, sizeof(Cue.fOpacity));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Animated model-cue bind failed: opacity.", hResult);
		for (uint32_t iMesh = 0u; iMesh < Model.Get_NumMeshes(); ++iMesh)
		{
			hResult = Bind_DeferredMaterialInputs(
				Model, m_pAnimatedModelShader, iMesh);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue material bind failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			hResult = Model.Bind_BoneMatrices(
				m_pAnimatedModelShader, "g_BoneMatrices", iMesh);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue bone bind failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			uint32_t iPass =
				Cue.eAlphaMode == EFFECT_MODEL_CUE_ALPHA_MODE::OPAQUE_SURFACE ?
					2u : 0u;
			if (Cue.eAlphaMode ==
				EFFECT_MODEL_CUE_ALPHA_MODE::TRANSLUCENT_SURFACE)
			{
				iPass = 4u;
			}
			hResult = m_pAnimatedModelShader->Begin(iPass);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue shader apply failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
			hResult = Model.Render(iMesh);
			if (FAILED(hResult))
				return Fail_RenderOperation(
					"Animated model-cue draw failed at mesh " +
					std::to_string(iMesh) + ".", hResult);
		}
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_ReconstructedDiagnostic(
	const float4x4_t& RootWorld,
	const RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo)
{
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	const auto FailDiagnostic = [this](std::string strDetail,
		const HRESULT hResult, const bool_t bObjectLocal)
	{
		const HRESULT hFailure = Fail_RenderOperation(
			std::move(strDetail), hResult, bObjectLocal);
		m_strStatus = "Reconstructed diagnostic failed: " +
			m_strRenderFailureDetail;
		return hFailure;
	};
	if (nullptr == m_pReconstructedDiagnostic ||
		nullptr == m_pReconstructedDiagnostic->pFrame ||
		eSolo >= RECONSTRUCTED_DIAGNOSTIC_SOLO::END)
	{
		return FailDiagnostic(
			"Reconstructed diagnostic draw was not staged.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND eRequiredKind =
		eSolo == RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH ?
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH :
			EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE;
	const auto& Packets = m_pReconstructedDiagnostic->pFrame->Get_Packets();
	const auto PacketIterator = std::find_if(Packets.begin(), Packets.end(),
		[eRequiredKind](const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet)
		{
			return Packet.Get_Kind() == eRequiredKind;
		});
	if (PacketIterator == Packets.end())
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Solo packet is unavailable.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet = *PacketIterator;
	const uint32_t iSelection = Packet.Get_SelectionIndex();
	if (iSelection >= m_pReconstructedDiagnostic->Resources.size() ||
		nullptr == Packet.Get_Preparation() ||
		iSelection >= Packet.Get_Preparation()->Get_Request().Emitters.size())
	{
		return FailDiagnostic(
			"Reconstructed diagnostic selection join is invalid.", E_FAIL, true);
	}
	const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection =
		Packet.Get_Preparation()->Get_Request().Emitters[iSelection];
	RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource =
		m_pReconstructedDiagnostic->Resources[iSelection];
	const shared_ptr<Engine::CShader> Shader =
		eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ?
			m_pMeshShader : m_pParticleShader;
	if (nullptr == m_pDevice || nullptr == m_pContext || nullptr == Shader ||
		nullptr == Resource.Textures[0u] ||
		nullptr == Resource.Textures[1u] || nullptr == Resource.Samplers[0u] ||
		nullptr == Resource.pPipelineStatisticsQuery)
	{
		return FailDiagnostic(
			"Reconstructed diagnostic GPU composite is incomplete.",
			E_FAIL, true);
	}
	if (eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH &&
		(nullptr == Resource.pModel ||
			!Packet.Get_Values().vMeshDimensionlessScaleXzy.has_value()))
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Mesh packet contract is incomplete.",
			E_FAIL, true);
	}
	if (eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::SPRITE &&
		(nullptr == m_pParticleBuffer || !Packet.Get_SpriteSink().has_value() ||
			!Packet.Get_Values().vSpriteSignedWorldSizeXzy.has_value()))
	{
		return FailDiagnostic(
			"Reconstructed diagnostic Sprite packet contract is incomplete.",
			E_FAIL, true);
	}

	if (Resource.bPipelineStatisticsPending)
	{
		D3D11_QUERY_DATA_PIPELINE_STATISTICS Statistics{};
		const HRESULT StatisticsResult = m_pContext->GetData(
			Resource.pPipelineStatisticsQuery.Get(), &Statistics,
			sizeof(Statistics), D3D11_ASYNC_GETDATA_DONOTFLUSH);
		if (S_OK == StatisticsResult)
		{
			Resource.bPipelineStatisticsPending = false;
			Resource.PipelineStatistics = Statistics;
			if (0u == Statistics.VSInvocations || 0u == Statistics.PSInvocations)
			{
				return FailDiagnostic(
					"Reconstructed diagnostic draw produced no shader invocations.",
					E_FAIL, true);
			}
		}
		else if (FAILED(StatisticsResult))
		{
			return FailDiagnostic(
				"Reconstructed diagnostic pipeline-statistics readback failed.",
				StatisticsResult, false);
		}
	}

	const auto ToFloat4 = [](const std::array<double, 4u>& Value)
	{
		return float4_t(static_cast<f32_t>(Value[0u]),
			static_cast<f32_t>(Value[1u]), static_cast<f32_t>(Value[2u]),
			static_cast<f32_t>(Value[3u]));
	};
	const EFFECT_RECONSTRUCTED_SELECTED_MATERIAL_BINDING& Material =
		Selection.Material;
	const float2_t UvScale{
		static_cast<f32_t>(Material.Constants.vUvScale[0u]),
		static_cast<f32_t>(Material.Constants.vUvScale[1u]) };
	const float4_t PanRotationAux = ToFloat4(
		Material.Constants.vPanRotationAux);
	const float4_t MaterialColor = ToFloat4(Material.Constants.vColor);
	const float4_t Params0 = ToFloat4(Material.Constants.vParams0);
	const float4_t Params1 = ToFloat4(Material.Constants.vParams1);
	const float2_t LegacyUvScale{ 1.f, 1.f };
	const float2_t LegacyUvOffset{};
	const f32_t fSampleTime = static_cast<f32_t>(
		m_pReconstructedDiagnostic->pFrame->Get_SampleTimeSeconds());
	const uint32_t iEnabled = 1u;
	const uint32_t iDisabled = 0u;
	const uint32_t iSourceMaterialProfile = 0u;
	const uint32_t iSourceTextureClampMask = 0u;
	const uint32_t iUseBaseOverride = 1u;

	HRESULT DrawResult = S_OK;
	std::string strDrawFailureDetail;
	bool_t bDrawFailureObjectLocal = false;
	const auto RecordDrawFailure = [&](const HRESULT hResult,
		std::string strDetail, const bool_t bObjectLocal,
		const bool_t bOverrideObjectLocalFailure = false)
	{
		if (SUCCEEDED(hResult))
			return;
		if (FAILED(DrawResult) &&
			!(bOverrideObjectLocalFailure && bDrawFailureObjectLocal))
		{
			return;
		}
		DrawResult = hResult;
		strDrawFailureDetail = std::move(strDetail);
		bDrawFailureObjectLocal = bObjectLocal;
	};
	const auto TryGlobalOperation = [&](const auto& Operation,
		const char* const pFailureDetail)
	{
		if (FAILED(DrawResult))
			return;
		RecordDrawFailure(Operation(), pFailureDetail, false);
	};
	const auto RecordLocalContractFailure = [&](const char* const pFailureDetail)
	{
		RecordDrawFailure(E_FAIL, pFailureDetail, true);
	};

	TryGlobalOperation([&]() { return Shader->Bind_Matrix("g_ViewMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)); },
		"Reconstructed diagnostic shader bind failed: g_ViewMatrix.");
	TryGlobalOperation([&]() { return Shader->Bind_Matrix("g_ProjMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)); },
		"Reconstructed diagnostic shader bind failed: g_ProjMatrix.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_UVScale", &LegacyUvScale, sizeof(LegacyUvScale)); },
		"Reconstructed diagnostic shader bind failed: g_UVScale.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_UVOffset", &LegacyUvOffset, sizeof(LegacyUvOffset)); },
		"Reconstructed diagnostic shader bind failed: g_UVOffset.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_EffectLocalTime", &fSampleTime, sizeof(fSampleTime)); },
		"Reconstructed diagnostic shader bind failed: g_EffectLocalTime.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceMaterialProfile", &iSourceMaterialProfile,
		sizeof(iSourceMaterialProfile)); },
		"Reconstructed diagnostic shader bind failed: g_SourceMaterialProfile.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceTextureClampUMask", &iSourceTextureClampMask,
		sizeof(iSourceTextureClampMask)); },
		"Reconstructed diagnostic shader bind failed: g_SourceTextureClampUMask.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		"g_SourceTextureClampVMask", &iSourceTextureClampMask,
		sizeof(iSourceTextureClampMask)); },
		"Reconstructed diagnostic shader bind failed: g_SourceTextureClampVMask.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strFeatureMaskVariable.c_str(),
		&Material.iFeatureMask, sizeof(Material.iFeatureMask)); },
		"Reconstructed diagnostic feature-mask bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strUvScaleVariable.c_str(), &UvScale,
		sizeof(UvScale)); },
		"Reconstructed diagnostic UV-scale bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strPanRotationAuxVariable.c_str(),
		&PanRotationAux, sizeof(PanRotationAux)); },
		"Reconstructed diagnostic pan/rotation bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strColorVariable.c_str(), &MaterialColor,
		sizeof(MaterialColor)); },
		"Reconstructed diagnostic color bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strParams0Variable.c_str(), &Params0,
		sizeof(Params0)); },
		"Reconstructed diagnostic params0 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strParams1Variable.c_str(), &Params1,
		sizeof(Params1)); },
		"Reconstructed diagnostic params1 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_Texture(
		Material.TextureLanes[0u].strShaderVariableName.c_str(),
		Resource.Textures[0u]); },
		"Reconstructed diagnostic texture lane 0 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_Texture(
		Material.TextureLanes[1u].strShaderVariableName.c_str(),
		Resource.Textures[1u]); },
		"Reconstructed diagnostic texture lane 1 bind failed.");
	TryGlobalOperation([&]() { return Shader->Bind_RawValue(
		Material.Shader.strEvaluatorEnabledVariable.c_str(),
		&iEnabled, sizeof(iEnabled)); },
		"Reconstructed diagnostic evaluator-enable bind failed.");
	if (SUCCEEDED(DrawResult) &&
		eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH)
	{
		const auto& Scale =
			*Packet.Get_Values().vMeshDimensionlessScaleXzy;
		const auto& Position = Packet.Get_Values().vLocalPosition;
		float4x4_t World{};
		XMStoreFloat4x4(&World,
			XMMatrixScaling(static_cast<f32_t>(Scale[0u]),
				static_cast<f32_t>(Scale[1u]),
				static_cast<f32_t>(Scale[2u])) *
			XMMatrixRotationY(XMConvertToRadians(static_cast<f32_t>(
				Packet.Get_Values().fRotationDegrees))) *
			XMMatrixTranslation(static_cast<f32_t>(Position[0u]),
				static_cast<f32_t>(Position[1u]),
				static_cast<f32_t>(Position[2u])) *
			XMLoadFloat4x4(&RootWorld));
		const float4_t CameraPosition =
			*CGameInstance::Get().Get_CamPosition();
		const float4_t Dynamic = ToFloat4(
			Packet.Get_Values().vDynamicParameter);
		TryGlobalOperation([&]() { return Shader->Bind_Matrix(
			"g_WorldMatrix", &World); },
			"Reconstructed diagnostic Mesh world bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition)); },
			"Reconstructed diagnostic Mesh camera bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_EffectDynamicParameter", &Dynamic, sizeof(Dynamic)); },
			"Reconstructed diagnostic Mesh dynamic bind failed.");
		TryGlobalOperation([&]() { return Shader->Bind_RawValue(
			"g_UseBaseOverride", &iUseBaseOverride,
			sizeof(iUseBaseOverride)); },
			"Reconstructed diagnostic Mesh base-override bind failed.");
	}

	ComPtr<ID3D11InfoQueue> InfoQueue;
	const bool_t bHasInfoQueue = SUCCEEDED(m_pDevice.As(&InfoQueue));
	const uint64_t iMessageBegin = bHasInfoQueue ?
		InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter() : 0u;
	if ((Resource.bHasBlendDescriptor && nullptr == Resource.pBlendState) ||
		(Resource.bHasRasterizerDescriptor &&
			nullptr == Resource.pRasterizerState) ||
		(Resource.bHasDepthStencilDescriptor &&
			nullptr == Resource.pDepthStencilState))
	{
		RecordLocalContractFailure(
			"Reconstructed diagnostic pipeline-state contract is incomplete.");
	}
	bool_t bQueryBegun = false;
	{
		CReconstructedPipelineStateGuard StateGuard(m_pContext.Get());
		TryGlobalOperation([&]() {
			return Shader->Begin(Material.Shader.iPassIndex);
		}, "Reconstructed diagnostic shader pass apply failed.");
		if (SUCCEEDED(DrawResult))
		{
			ID3D11SamplerState* pSampler = Resource.Samplers[0u].Get();
			m_pContext->PSSetSamplers(0u, 1u, &pSampler);
			if (Resource.bHasBlendDescriptor)
			{
				const float BlendFactor[4u]{};
				m_pContext->OMSetBlendState(
					Resource.pBlendState.Get(), BlendFactor, 0xffffffffu);
			}
			if (Resource.bHasRasterizerDescriptor)
				m_pContext->RSSetState(Resource.pRasterizerState.Get());
			if (Resource.bHasDepthStencilDescriptor)
				m_pContext->OMSetDepthStencilState(
					Resource.pDepthStencilState.Get(), 0u);

			ComPtr<ID3D11SamplerState> ActualSampler;
			m_pContext->PSGetSamplers(0u, 1u, &ActualSampler);
			D3D11_SAMPLER_DESC ActualSamplerDescriptor{};
			if (nullptr == ActualSampler)
				RecordLocalContractFailure(
					"Reconstructed diagnostic sampler readback is null.");
			else
				ActualSampler->GetDesc(&ActualSamplerDescriptor);
			if (SUCCEEDED(DrawResult) && !Same_SamplerDescriptor(
				ActualSamplerDescriptor, Resource.SamplerDescriptors[0u]))
			{
				RecordLocalContractFailure(
					"Reconstructed diagnostic sampler descriptor changed.");
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasBlendDescriptor)
			{
				ComPtr<ID3D11BlendState> Actual;
				float Factor[4u]{};
				uint32_t iMask = 0u;
				m_pContext->OMGetBlendState(&Actual, Factor, &iMask);
				D3D11_BLEND_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || iMask != 0xffffffffu ||
					!Same_BlendDescriptor(
						Descriptor, Resource.BlendDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic blend-state readback changed.");
				}
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasRasterizerDescriptor)
			{
				ComPtr<ID3D11RasterizerState> Actual;
				m_pContext->RSGetState(&Actual);
				D3D11_RASTERIZER_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || !Same_RasterizerDescriptor(
					Descriptor, Resource.RasterizerDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic rasterizer-state readback changed.");
				}
			}
			if (SUCCEEDED(DrawResult) && Resource.bHasDepthStencilDescriptor)
			{
				ComPtr<ID3D11DepthStencilState> Actual;
				uint32_t iStencilReference = 0u;
				m_pContext->OMGetDepthStencilState(
					&Actual, &iStencilReference);
				D3D11_DEPTH_STENCIL_DESC Descriptor{};
				if (nullptr != Actual)
					Actual->GetDesc(&Descriptor);
				if (nullptr == Actual || 0u != iStencilReference ||
					!Same_DepthStencilDescriptor(
						Descriptor, Resource.DepthStencilDescriptor))
				{
					RecordLocalContractFailure(
						"Reconstructed diagnostic depth-state readback changed.");
				}
			}
		}

		if (SUCCEEDED(DrawResult) &&
			!Resource.bPipelineStatisticsPending)
		{
			m_pContext->Begin(Resource.pPipelineStatisticsQuery.Get());
			bQueryBegun = true;
		}
		if (SUCCEEDED(DrawResult) &&
			eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH)
		{
			for (uint32_t iMesh = 0u;
				SUCCEEDED(DrawResult) && iMesh < Resource.pModel->Get_NumMeshes();
				++iMesh)
			{
				const HRESULT hModelResult = Resource.pModel->Render(iMesh);
				if (FAILED(hModelResult))
				{
					RecordDrawFailure(hModelResult,
						"Reconstructed diagnostic Mesh draw failed at submesh " +
						std::to_string(iMesh) + ".", false);
				}
			}
		}
		else if (SUCCEEDED(DrawResult))
		{
			const auto& Size =
				*Packet.Get_Values().vSpriteSignedWorldSizeXzy;
			const auto& Position = Packet.Get_Values().vLocalPosition;
			const auto& Velocity = Packet.Get_Values().vVelocityPerSecond;
			const EFFECT_RECONSTRUCTED_SELECTED_SPRITE_SINK& Sink =
				*Packet.Get_SpriteSink();
			const matrix_t Root = XMLoadFloat4x4(&RootWorld);
			const vector_t WorldPosition = XMVector3TransformCoord(
				XMVectorSet(static_cast<f32_t>(Position[0u]),
					static_cast<f32_t>(Position[1u]),
					static_cast<f32_t>(Position[2u]), 1.f), Root);
			const vector_t WorldVelocity = XMVector3TransformNormal(
				XMVectorSet(static_cast<f32_t>(Velocity[0u]),
					static_cast<f32_t>(Velocity[1u]),
					static_cast<f32_t>(Velocity[2u]), 0.f), Root);
			matrix_t CameraWorld = XMLoadFloat4x4(
				CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			const f32_t fRight = XMVectorGetX(XMVector3Dot(
				WorldVelocity, CameraWorld.r[0u]));
			const f32_t fUp = XMVectorGetX(XMVector3Dot(
				WorldVelocity, CameraWorld.r[1u]));
			const f32_t fVelocityRoll =
				std::abs(fRight) + std::abs(fUp) > 1.e-6f ?
					std::atan2(fUp, fRight) : 0.f;
			const f32_t fRoll = fVelocityRoll + XMConvertToRadians(
				static_cast<f32_t>(Packet.Get_Values().fRotationDegrees +
					Sink.fBillboardRollDegrees));
			const matrix_t Pivot = XMMatrixTranslation(
				0.5f - static_cast<f32_t>(Sink.vPivotCenter[0u]),
				static_cast<f32_t>(Sink.vPivotCenter[1u]) - 0.5f, 0.f);
			float4x4_t World{};
			XMStoreFloat4x4(&World,
				Pivot * XMMatrixScaling(static_cast<f32_t>(Size[0u]),
					static_cast<f32_t>(Size[2u]), 1.f) *
				XMMatrixRotationZ(fRoll) * CameraWorld *
				XMMatrixTranslationFromVector(WorldPosition));
			const float4_t Color = ToFloat4(Packet.Get_Values().vColor);
			const float4_t Dynamic = ToFloat4(
				Packet.Get_Values().vDynamicParameter);
			const f32_t fNormalizedLife = static_cast<f32_t>(std::clamp(
				Packet.Get_Timing().fAgeSeconds /
					Packet.Get_Timing().fLifetimeSeconds, 0.0, 1.0));
			const Engine::VTXEFFECT_PARTICLE Instance{
				World, Color, Dynamic,
				{ 1.f, 1.f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.f },
				{ fNormalizedLife, 0.f } };
			const HRESULT hUpdateResult = m_pParticleBuffer->Update_Instances(
				std::span<const Engine::VTXEFFECT_PARTICLE>(&Instance, 1u));
			RecordDrawFailure(hUpdateResult,
				"Reconstructed diagnostic Sprite instance update failed.", false);
			if (SUCCEEDED(DrawResult))
			{
				const HRESULT hSpriteDrawResult = m_pParticleBuffer->Render();
				RecordDrawFailure(hSpriteDrawResult,
					"Reconstructed diagnostic Sprite draw failed.", false);
			}
		}
		if (bQueryBegun)
		{
			m_pContext->End(Resource.pPipelineStatisticsQuery.Get());
			Resource.bPipelineStatisticsPending = true;
		}
	}

	const HRESULT hEvaluatorResetResult = Shader->Bind_RawValue(
		Material.Shader.strEvaluatorEnabledVariable.c_str(),
		&iDisabled, sizeof(iDisabled));
	if (FAILED(hEvaluatorResetResult))
	{
		RecordDrawFailure(hEvaluatorResetResult,
			"Reconstructed diagnostic evaluator reset failed.", false, true);
	}
	if (SUCCEEDED(DrawResult) && bHasInfoQueue)
	{
		const uint64_t iMessageEnd =
			InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
		for (uint64_t iMessage = iMessageBegin; iMessage < iMessageEnd;
			++iMessage)
		{
			size_t iMessageSize = 0u;
			const HRESULT hMessageSizeResult =
				InfoQueue->GetMessage(iMessage, nullptr, &iMessageSize);
			if (FAILED(hMessageSizeResult))
			{
				RecordDrawFailure(hMessageSizeResult,
					"Reconstructed diagnostic D3D info-queue size read failed.",
					false);
				break;
			}
			if (0u == iMessageSize)
				continue;
			std::vector<uint8_t> MessageBytes(iMessageSize);
			D3D11_MESSAGE* pMessage =
				reinterpret_cast<D3D11_MESSAGE*>(MessageBytes.data());
			const HRESULT hMessageResult = InfoQueue->GetMessage(
				iMessage, pMessage, &iMessageSize);
			if (FAILED(hMessageResult))
			{
				RecordDrawFailure(hMessageResult,
					"Reconstructed diagnostic D3D info-queue read failed.", false);
				break;
			}
			if (pMessage->Severity == D3D11_MESSAGE_SEVERITY_ERROR ||
				pMessage->Severity == D3D11_MESSAGE_SEVERITY_CORRUPTION)
			{
				RecordDrawFailure(E_FAIL,
					"Reconstructed diagnostic D3D debug layer reported an error.",
					false);
				break;
			}
		}
	}
	if (FAILED(DrawResult))
	{
		if (strDrawFailureDetail.empty())
			strDrawFailureDetail =
				"Reconstructed diagnostic GPU draw or state readback failed.";
		return FailDiagnostic(std::move(strDrawFailureDetail), DrawResult,
			bDrawFailureObjectLocal);
	}
	++Resource.iDrawCount;
	const char* pKind =
		eSolo == RECONSTRUCTED_DIAGNOSTIC_SOLO::MESH ? "Mesh" : "Sprite";
	m_strStatus = std::string("Reconstructed ") + pKind +
		" Solo draw submitted; draw count " +
		std::to_string(Resource.iDrawCount) + ", VS/PS invocations " +
		std::to_string(Resource.PipelineStatistics.VSInvocations) + "/" +
		std::to_string(Resource.PipelineStatistics.PSInvocations) + ".";
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_WorldMarks(
	const EFFECT_EVALUATED_FRAME& Frame,
	const uint64_t iSubmissionSerial)
{
	if (!Has_WorldMarkElements())
	{
		m_bWorldMarkSubmissionPending = false;
		return S_FALSE;
	}
	m_bWorldMarkSubmissionPending = false;
	const HRESULT hResult = Render_CompositionPhase(Frame,
		EFFECT_COMPOSITION_LAYER::WORLD_MARK, true, false,
		iSubmissionSerial);
	if (SUCCEEDED(hResult))
	{
		m_bWorldMarkSubmissionPending = true;
		m_iWorldMarkSubmissionSerial = iSubmissionSerial;
	}
	return hResult;
}

HRESULT Client::CEffectDocumentRenderer::Render(
	const EFFECT_EVALUATED_FRAME& Frame,
	const uint64_t iSubmissionSerial)
{
	const bool_t bHasWorldMarks = Has_WorldMarkElements();
	if (bHasWorldMarks &&
		(!m_bWorldMarkSubmissionPending ||
		 m_iWorldMarkSubmissionSerial != iSubmissionSerial))
	{
		const HRESULT hWorldMarkResult =
			Render_WorldMarks(Frame, iSubmissionSerial);
		if (FAILED(hWorldMarkResult))
			return hWorldMarkResult;
	}
	if (!bHasWorldMarks)
		m_bWorldMarkSubmissionPending = false;
	const HRESULT hResult = Render_CompositionPhase(Frame,
		EFFECT_COMPOSITION_LAYER::NORMAL, !bHasWorldMarks, true,
		iSubmissionSerial);
	m_bWorldMarkSubmissionPending = false;
	return hResult;
}

HRESULT Client::CEffectDocumentRenderer::Render_CompositionPhase(
	const EFFECT_EVALUATED_FRAME& Frame,
	const EFFECT_COMPOSITION_LAYER ePhase,
	const bool_t bBeginSubmission,
	const bool_t bFinalizeSubmission,
	const uint64_t iSubmissionSerial)
{
	const EFFECT_DOCUMENT_DESC& Document = Get_StagedDocument();
	if (ePhase >= EFFECT_COMPOSITION_LAYER::END ||
		bFinalizeSubmission !=
			(ePhase == EFFECT_COMPOSITION_LAYER::NORMAL) ||
		(!bBeginSubmission &&
		 (!m_bWorldMarkSubmissionPending ||
		  m_iWorldMarkSubmissionSerial != iSubmissionSerial)))
	{
		m_bWorldMarkSubmissionPending = false;
		m_strStatus = "Effect composition phase submission is inconsistent.";
		m_bLastRenderFailureObjectLocal = true;
		return E_INVALIDARG;
	}
	if (bBeginSubmission)
		m_LastRenderSubmissionStats = {};
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	m_pActiveOccurrenceStats = nullptr;
	if (bBeginSubmission)
	{
		m_iTestIssuedDrawOrdinal = 0u;
		m_LastRenderSubmissionStats.Occurrences.reserve(
			Document.Elements.size());
	}
#endif
	if (bBeginSubmission)
	{
		m_strRenderFailureDetail.clear();
		m_bLastRenderFailureObjectLocal = false;
	}
	size_t iConfiguredGpuOccurrenceCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible)
			continue;
		const EFFECT_GPU_RENDER_FAMILY eFamily =
			Resolve_GpuRenderFamily(Element);
		if (EFFECT_GPU_RENDER_FAMILY::END != eFamily)
		{
			++iConfiguredGpuOccurrenceCount;
			if (bBeginSubmission)
			{
				++m_LastRenderSubmissionStats.Families[
					static_cast<size_t>(eFamily)].iConfigured;
			}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			if (bBeginSubmission)
			{
				EFFECT_GPU_RENDER_OCCURRENCE_STATS Occurrence;
				Occurrence.strElementId = Element.strElementId;
				Occurrence.eFamily = eFamily;
				Occurrence.eCompositionLayer = Element.eCompositionLayer;
				Occurrence.iConfigured = 1u;
				if (const ELEMENT_RESOURCE* pResource =
					Find_Resource(Element.strElementId))
				{
					Occurrence.iSourceMaterialProfile =
						pResource->iSourceMaterialProfile;
					Occurrence.iSourceTextureMask =
						pResource->iSourceTextureMask;
					Occurrence.bSourceMaterialFallbackBlocked =
						pResource->bSourceMaterialFallbackBlocked;
				}
				m_LastRenderSubmissionStats.Occurrences.emplace_back(
					std::move(Occurrence));
			}
#endif
		}
	}
	const auto FailFrame = [this](std::string strStatus,
		const HRESULT hResult = E_FAIL,
		const bool_t bObjectLocal = false) -> HRESULT
	{
		m_bWorldMarkSubmissionPending = false;
		m_LastRenderSubmissionStats.bCompleted = true;
		m_LastRenderSubmissionStats.bCommitted = false;
		m_bLastRenderFailureObjectLocal =
			m_bLastRenderFailureObjectLocal || bObjectLocal;
		if (!m_strRenderFailureDetail.empty())
			strStatus += " Operation: " + m_strRenderFailureDetail;
		m_strStatus = std::move(strStatus);
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	for (const EFFECT_EVALUATED_ELEMENT& Element : Frame.Elements)
	{
		if (nullptr == Element.pElement)
		{
			return FailFrame(
				"Effect frame contains an element row without a descriptor.",
				E_INVALIDARG, true);
		}
	}
	for (const EFFECT_EVALUATED_AFTERIMAGE& AfterImage : Frame.AfterImages)
	{
		if (nullptr == AfterImage.pElement ||
			!std::isfinite(AfterImage.fAlpha) || AfterImage.fAlpha < 0.f ||
			AfterImage.fAlpha > 1.f)
		{
			return FailFrame(
				"Effect frame contains an invalid afterimage row.",
				E_INVALIDARG, true);
		}
	}
	if (Frame.GpuOccurrences.size() != iConfiguredGpuOccurrenceCount)
	{
		return FailFrame(
			"Effect GPU occurrence evaluation count does not match the document.",
			E_FAIL, true);
	}
	std::string GateStatus;
	if (!m_bSourceVisualProgramActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		return FailFrame(std::move(GateStatus), E_FAIL, true);
	}
	const HRESULT hModelCueResult =
		ePhase == EFFECT_COMPOSITION_LAYER::NORMAL ?
			Render_ModelCues(Frame, false) : S_FALSE;
	if (FAILED(hModelCueResult))
	{
		return FailFrame(
			"Effect animated model-cue rendering failed.", hModelCueResult);
	}
	size_t iElement = 0u;
	size_t iParticle = 0u;
	size_t iTrail = 0u;
	size_t iAfterImage = 0u;
	size_t iGpuOccurrence = 0u;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	struct ACTIVE_OCCURRENCE_SCOPE final
	{
		EFFECT_GPU_RENDER_OCCURRENCE_STATS*& pSlot;
		explicit ACTIVE_OCCURRENCE_SCOPE(
			EFFECT_GPU_RENDER_OCCURRENCE_STATS*& pActiveSlot)
			: pSlot(pActiveSlot)
		{
		}
		~ACTIVE_OCCURRENCE_SCOPE() { pSlot = nullptr; }
	};
	size_t iOccurrenceStats = 0u;
#endif
	for (const EFFECT_ELEMENT_DESC& DocumentElement : Document.Elements)
	{
		const std::string& strElementId = DocumentElement.strElementId;
		const EFFECT_GPU_RENDER_FAMILY eFamily = DocumentElement.bVisible ?
			Resolve_GpuRenderFamily(DocumentElement) :
			EFFECT_GPU_RENDER_FAMILY::END;
		const bool_t bHasGpuFamily =
			EFFECT_GPU_RENDER_FAMILY::END != eFamily;
		const bool_t bOwnsPhase =
			DocumentElement.eCompositionLayer == ePhase;
		EFFECT_GPU_RENDER_FAMILY_STATS* pFamilyStats =
			!bHasGpuFamily || !bOwnsPhase ? nullptr :
				&m_LastRenderSubmissionStats.Families[
					static_cast<size_t>(eFamily)];
		const bool_t bSubmitPreviewOccurrence =
			bOwnsPhase &&
			Should_SubmitPreviewOccurrence(DocumentElement, eFamily);
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		EFFECT_GPU_RENDER_OCCURRENCE_STATS* pOccurrenceStats = nullptr;
		if (bHasGpuFamily)
		{
			if (iOccurrenceStats >=
				m_LastRenderSubmissionStats.Occurrences.size())
			{
				return FailFrame(
					"Effect GPU occurrence probe denominator overflow.",
					E_FAIL, true);
			}
			EFFECT_GPU_RENDER_OCCURRENCE_STATS* pDocumentOccurrenceStats =
				&m_LastRenderSubmissionStats.Occurrences[iOccurrenceStats++];
			if (pDocumentOccurrenceStats->strElementId != strElementId ||
				pDocumentOccurrenceStats->eFamily != eFamily)
			{
				return FailFrame(
					"Effect GPU occurrence probe order diverged.", E_FAIL, true);
			}
			if (bOwnsPhase)
				pOccurrenceStats = pDocumentOccurrenceStats;
		}
		m_pActiveOccurrenceStats = pOccurrenceStats;
		ACTIVE_OCCURRENCE_SCOPE ActiveOccurrenceScope(
			m_pActiveOccurrenceStats);
#endif
		const EFFECT_EVALUATED_GPU_OCCURRENCE* pGpuOccurrence = nullptr;
		if (bHasGpuFamily)
		{
			if (iGpuOccurrence >= Frame.GpuOccurrences.size())
			{
				return FailFrame(
					"Effect GPU occurrence evaluation order overflowed.",
					E_FAIL, true);
			}
			pGpuOccurrence = &Frame.GpuOccurrences[iGpuOccurrence++];
		}
		if (bHasGpuFamily != (nullptr != pGpuOccurrence) ||
			(nullptr != pGpuOccurrence &&
			 (nullptr == pGpuOccurrence->pElement ||
			  pGpuOccurrence->pElement->strElementId != strElementId ||
			  eFamily != Resolve_GpuRenderFamily(*pGpuOccurrence->pElement))))
		{
			return FailFrame(
				"Effect GPU occurrence evaluation does not match the document.",
				E_FAIL, true);
		}
		if (nullptr != pFamilyStats)
			++pFamilyStats->iEvaluated;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
		if (nullptr != pOccurrenceStats)
			++pOccurrenceStats->iEvaluated;
#endif
		bool_t bOccurrenceActive =
			nullptr != pGpuOccurrence && pGpuOccurrence->bActive;
		bool_t bOccurrenceSubmitted = false;
		bool_t bOccurrenceSuppressed = false;
		const size_t iAfterImageBegin = iAfterImage;
		while (iAfterImage < Frame.AfterImages.size() &&
			nullptr != Frame.AfterImages[iAfterImage].pElement &&
			Frame.AfterImages[iAfterImage].pElement->strElementId == strElementId)
		{
			++iAfterImage;
		}
		const HRESULT hAfterImageResult = bSubmitPreviewOccurrence ?
			Render_AfterImages(Frame,
				std::span<const EFFECT_EVALUATED_AFTERIMAGE>(Frame.AfterImages)
					.subspan(iAfterImageBegin,
						iAfterImage - iAfterImageBegin)) : S_FALSE;
		if (iAfterImageBegin != iAfterImage)
			bOccurrenceActive = true;
		if (FAILED(hAfterImageResult))
		{
			if (nullptr != pFamilyStats)
				++pFamilyStats->iFailed;
			return FailFrame(
				"Effect afterimage rendering failed: " + strElementId,
				hAfterImageResult);
		}
		bOccurrenceSubmitted = bOccurrenceSubmitted ||
			S_OK == hAfterImageResult;
		bOccurrenceSuppressed = bOccurrenceSuppressed ||
			S_FALSE == hAfterImageResult && iAfterImageBegin != iAfterImage;

		const size_t iElementBegin = iElement;
		while (iElement < Frame.Elements.size() &&
			nullptr != Frame.Elements[iElement].pElement &&
			Frame.Elements[iElement].pElement->strElementId == strElementId)
		{
			if (!bSubmitPreviewOccurrence)
			{
				++iElement;
				continue;
			}
			const ELEMENT_RESOURCE* pResource = Find_Resource(strElementId);
			if (nullptr == pResource)
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect element resource is missing: " + strElementId,
					E_FAIL, true);
			}
			if (pResource->bSourceMaterialFallbackBlocked ||
				pResource->bOccurrenceVisualSuppressed)
			{
				bOccurrenceSuppressed = true;
				++iElement;
				continue;
			}
			const HRESULT hElementResult =
				Render_Element(Frame.Elements[iElement], *pResource);
			if (FAILED(hElementResult))
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect element rendering failed: " + strElementId,
					hElementResult);
			}
			bOccurrenceSubmitted = bOccurrenceSubmitted ||
				S_OK == hElementResult;
			bOccurrenceSuppressed = bOccurrenceSuppressed ||
				S_FALSE == hElementResult;
			++iElement;
		}
		bOccurrenceActive = bOccurrenceActive || iElementBegin != iElement;

		const size_t iParticleBegin = iParticle;
		while (iParticle < Frame.Particles.size() &&
			nullptr != Frame.Particles[iParticle].pElement &&
			Frame.Particles[iParticle].pElement->strElementId == strElementId)
		{
			++iParticle;
		}
		const size_t iTrailBegin = iTrail;
		while (iTrail < Frame.Trails.size() &&
			nullptr != Frame.Trails[iTrail].pElement &&
			Frame.Trails[iTrail].pElement->strElementId == strElementId)
		{
			++iTrail;
		}
		const HRESULT hParticleResult = bSubmitPreviewOccurrence ?
			Render_Particles(Frame,
				std::span<const EFFECT_EVALUATED_PARTICLE>(Frame.Particles)
					.subspan(iParticleBegin,
						iParticle - iParticleBegin)) : S_FALSE;
		if (iParticleBegin != iParticle)
			bOccurrenceActive = true;
		if (FAILED(hParticleResult))
		{
			if (nullptr != pFamilyStats)
				++pFamilyStats->iFailed;
			return FailFrame(
				"Effect particle rendering failed: " + strElementId,
				hParticleResult);
		}
		bOccurrenceSubmitted = bOccurrenceSubmitted ||
			S_OK == hParticleResult && iParticleBegin != iParticle;
		bOccurrenceSuppressed = bOccurrenceSuppressed ||
			S_FALSE == hParticleResult && iParticleBegin != iParticle;
		const HRESULT hTrailResult = bSubmitPreviewOccurrence ?
			Render_Trails(Frame,
				std::span<const EFFECT_EVALUATED_TRAIL>(Frame.Trails)
					.subspan(iTrailBegin, iTrail - iTrailBegin)) : S_FALSE;
		if (iTrailBegin != iTrail)
			bOccurrenceActive = true;
		if (FAILED(hTrailResult))
		{
			if (nullptr != pFamilyStats)
				++pFamilyStats->iFailed;
			return FailFrame(
				"Effect trail rendering failed: " + strElementId,
				hTrailResult);
		}
		bOccurrenceSubmitted = bOccurrenceSubmitted ||
			S_OK == hTrailResult && iTrailBegin != iTrail;
		bOccurrenceSuppressed = bOccurrenceSuppressed ||
			S_FALSE == hTrailResult && iTrailBegin != iTrail;
		const size_t iCandidateRowCount =
			(iAfterImage - iAfterImageBegin) +
			(iElement - iElementBegin) +
			(iParticle - iParticleBegin) +
			(iTrail - iTrailBegin);
		if (nullptr != pGpuOccurrence &&
			pGpuOccurrence->iCandidateRowCount != iCandidateRowCount)
		{
			if (nullptr != pFamilyStats)
				++pFamilyStats->iFailed;
			return FailFrame(
				"Effect GPU occurrence candidate-row denominator mismatch: " +
				strElementId, E_FAIL, true);
		}
		if (0u < iCandidateRowCount && !bOccurrenceActive)
		{
			if (nullptr != pFamilyStats)
				++pFamilyStats->iFailed;
			return FailFrame(
				"Effect inactive GPU occurrence produced candidate rows: " +
				strElementId, E_FAIL, true);
		}
		if (bOccurrenceActive && 0u == iCandidateRowCount)
			bOccurrenceSuppressed = true;
		if (!bSubmitPreviewOccurrence && bOccurrenceActive)
			bOccurrenceSuppressed = true;
		if (bOccurrenceActive && bOwnsPhase)
		{
			if (nullptr == pFamilyStats)
			{
				return FailFrame(
					"Effect active GPU occurrence has no typed family: " +
					strElementId, E_FAIL, true);
			}
			++pFamilyStats->iActive;
			if (0u < iCandidateRowCount)
				++pFamilyStats->iCandidate;
			else
				++pFamilyStats->iZeroCandidate;
			++pFamilyStats->iAttempted;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
			++pOccurrenceStats->iActive;
			pOccurrenceStats->iCandidateRowCount += iCandidateRowCount;
			++pOccurrenceStats->iAttempted;
#endif
			if (bOccurrenceSubmitted)
			{
				++pFamilyStats->iSubmitted;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
				++pOccurrenceStats->iSubmitted;
				if (0u == pOccurrenceStats->iMaterialBindCount ||
					0u == pOccurrenceStats->iTextureSrvBindCount ||
					0u == pOccurrenceStats->iSamplerBindCount ||
					0u == pOccurrenceStats->iShaderPassApplyCount ||
					0u == pOccurrenceStats->iVIBufferBindCount ||
					0u == pOccurrenceStats->iVIBufferDrawCount ||
					0u == pOccurrenceStats->iIssuedDrawCallCount ||
					0u == pOccurrenceStats->iDrawSelectionCount ||
					pOccurrenceStats->iVIBufferBindCount !=
						pOccurrenceStats->iVIBufferDrawCount ||
					pOccurrenceStats->iVIBufferDrawCount !=
						pOccurrenceStats->iIssuedDrawCallCount ||
					pOccurrenceStats->iDrawSelectionCount !=
						pOccurrenceStats->iIssuedDrawCallCount ||
					pOccurrenceStats->bDrawSelectionDiverged ||
					EFFECT_GPU_RENDER_CARRIER::END ==
						pOccurrenceStats->eCarrier ||
					UINT32_MAX == pOccurrenceStats->iSelectedPassIndex ||
					!pOccurrenceStats->bHasSubmittedPosition)
				{
					++pOccurrenceStats->iFailed;
					++pFamilyStats->iFailed;
					return FailFrame(
						"Effect submitted occurrence bypassed material/pass/VF/draw evidence: " +
						strElementId, E_FAIL, true);
				}
#endif
			}
			else if (bOccurrenceSuppressed)
			{
				++pFamilyStats->iSuppressed;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
				++pOccurrenceStats->iSuppressed;
#endif
			}
			else
			{
				++pFamilyStats->iFailed;
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
				++pOccurrenceStats->iFailed;
#endif
				return FailFrame(
					"Effect active GPU occurrence produced no disposition: " +
					strElementId, E_FAIL, true);
			}
		}
	}
	if (iElement != Frame.Elements.size() ||
		iParticle != Frame.Particles.size() ||
		iTrail != Frame.Trails.size() ||
		iAfterImage != Frame.AfterImages.size() ||
		iGpuOccurrence != Frame.GpuOccurrences.size())
	{
		return FailFrame(
			"Effect frame contains unconsumed evaluated GPU rows.",
			E_FAIL, true);
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	if (iOccurrenceStats != m_LastRenderSubmissionStats.Occurrences.size())
	{
		return FailFrame(
			"Effect GPU occurrence probe denominator underflow.", E_FAIL, true);
	}
#endif
	if (!bFinalizeSubmission)
		return S_OK;
	uint64_t iActive = 0u;
	uint64_t iSubmitted = 0u;
	uint64_t iSuppressed = 0u;
	for (const EFFECT_GPU_RENDER_FAMILY_STATS& Stats :
		m_LastRenderSubmissionStats.Families)
	{
		if (Stats.iEvaluated != Stats.iConfigured ||
			Stats.iCandidate + Stats.iZeroCandidate != Stats.iActive ||
			Stats.iAttempted != Stats.iActive ||
			Stats.iSubmitted + Stats.iSuppressed != Stats.iAttempted ||
			0u != Stats.iFailed)
		{
			return FailFrame(
				"Effect GPU family occurrence denominator mismatch.",
				E_FAIL, true);
		}
		iActive += Stats.iActive;
		iSubmitted += Stats.iSubmitted;
		iSuppressed += Stats.iSuppressed;
	}
	m_LastRenderSubmissionStats.bCompleted = true;
	m_LastRenderSubmissionStats.bCommitted = true;
	const uint64_t iEvaluated = Frame.GpuOccurrences.size();
	if (m_iStatusEvaluated != iEvaluated || m_iStatusActive != iActive ||
		m_iStatusSubmitted != iSubmitted ||
		m_iStatusSuppressed != iSuppressed)
	{
		m_strStatus = "Effect GPU occurrence disposition: evaluated " +
			std::to_string(iEvaluated) + ", active " +
			std::to_string(iActive) + ", submitted " +
			std::to_string(iSubmitted) + ", suppressed " +
			std::to_string(iSuppressed) + ".";
		m_iStatusEvaluated = iEvaluated;
		m_iStatusActive = iActive;
		m_iStatusSubmitted = iSubmitted;
		m_iStatusSuppressed = iSuppressed;
	}
	return 0u < iActive && 0u == iSubmitted ? S_FALSE : S_OK;
}
