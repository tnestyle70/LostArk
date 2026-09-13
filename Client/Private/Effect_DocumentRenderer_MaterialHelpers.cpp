#include "Effect_DocumentRenderer_Internal.h"
#include "GameInstance.h"
#include "Model.h"
#include "Render_OutputContract.h"
#include "VIBuffer_ParticleRect.h"
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
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Shader.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_Rect.h"

namespace EffectDocumentRendererDetail
{

    bool Requires_StartingSceneCapture(const Client::EFFECT_DOCUMENT_DESC& Document)
    {
        return std::ranges::any_of(Document.Elements, [](const auto& Element) {
            return Element.bVisible && Element.Material.SourceMaterial.strRuntimeShaderProfileId ==
                "effect.ue3.altv-178-native.v1";
        }) || std::ranges::any_of(Document.ModelCues, [](const auto& Cue) {
            return Cue.bVisible && Client::Has_DimensionMasterALTVModelCueMaterialContract(Cue);
        });
    }

    bool Is_StartingSceneCaptureCameraEmitter(const Client::EFFECT_ELEMENT_DESC& Element)
    {
        return Element.Material.SourceMaterial.strRuntimeShaderProfileId == "effect.ue3.altv-178-native.v1" &&
            (Element.strElementId == "fx_pc_swp_04.par_m_swp_tw_s1_camera_01.particlespriteemitter_18" ||
             Element.strElementId == "fx_pc_swp_04.par_m_swp_tw_s1_camera_01.particlespriteemitter_31");
    }

    // Project framing adapter only: endpoint geometry and all authored transforms stay intact.
    // Return without touching World on invalid bounds/camera or at the exact authored endpoint.
    void Fit_StartingCaptureMeshToCamera(const float3_t& BoundsMin, const float3_t& BoundsMax,
        const float4x4_t& View, const float4x4_t& Projection, const f32_t fProgress,
        float4x4_t& World)
    {
        if (!std::isfinite(fProgress) || fProgress >= 1.f ||
            !std::isfinite(Projection._11) || Projection._11 <= 0.f ||
            !std::isfinite(Projection._22) || Projection._22 <= 0.f ||
            !std::isfinite(Projection._33) || Projection._33 == 0.f ||
            std::abs(Projection._34 - 1.f) > 1e-5f || std::abs(Projection._44) > 1e-5f)
            return;
        const f32_t fNear = -Projection._43 / Projection._33;
        if (!std::isfinite(fNear) || fNear <= 0.f ||
            !(BoundsMax.x > BoundsMin.x && BoundsMax.y > BoundsMin.y && BoundsMax.z > BoundsMin.z))
            return;
        const matrix_t ViewMatrix = XMLoadFloat4x4(&View);
        const f32_t fViewDeterminant = XMVectorGetX(XMMatrixDeterminant(ViewMatrix));
        if (!std::isfinite(fViewDeterminant) || std::abs(fViewDeterminant) < 1e-6f) return;
        const matrix_t WorldView = XMLoadFloat4x4(&World) * ViewMatrix;
        float3_t Minimum{ FLT_MAX, FLT_MAX, FLT_MAX }, Maximum{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (uint32_t i = 0u; i < 8u; ++i)
        {
            float3_t Corner{};
            XMStoreFloat3(&Corner, XMVector3TransformCoord(XMVectorSet(
                (i & 1u) ? BoundsMax.x : BoundsMin.x,
                (i & 2u) ? BoundsMax.y : BoundsMin.y,
                (i & 4u) ? BoundsMax.z : BoundsMin.z, 1.f), WorldView));
            if (!std::isfinite(Corner.x) || !std::isfinite(Corner.y) || !std::isfinite(Corner.z)) return;
            Minimum.x = (std::min)(Minimum.x, Corner.x); Maximum.x = (std::max)(Maximum.x, Corner.x);
            Minimum.y = (std::min)(Minimum.y, Corner.y); Maximum.y = (std::max)(Maximum.y, Corner.y);
            Minimum.z = (std::min)(Minimum.z, Corner.z); Maximum.z = (std::max)(Maximum.z, Corner.z);
        }
        const f32_t fWidth = Maximum.x - Minimum.x, fHeight = Maximum.y - Minimum.y;
        if (!(fWidth > 1e-6f && fHeight > 1e-6f) || Maximum.z <= fNear) return;
        const f32_t fFrontDepth = (std::max)(Minimum.z, fNear * 1.01f);
        const f32_t fScaleX = 2.f * fFrontDepth / (Projection._11 * fWidth);
        const f32_t fScaleY = 2.f * fFrontDepth / (Projection._22 * fHeight);
        const matrix_t Fitted = WorldView *
            XMMatrixTranslation(-(Minimum.x + Maximum.x) * .5f, -(Minimum.y + Maximum.y) * .5f,
                fFrontDepth - Minimum.z) * XMMatrixScaling(fScaleX, fScaleY, 1.f) *
            XMMatrixTranslation(-Projection._31 * fFrontDepth / Projection._11,
                -Projection._32 * fFrontDepth / Projection._22, 0.f) * XMMatrixInverse(nullptr, ViewMatrix);
        const f32_t fLinear = std::clamp(fProgress, 0.f, 1.f);
        const f32_t fBlend = fLinear * fLinear * (3.f - 2.f * fLinear);
        const matrix_t Authored = XMLoadFloat4x4(&World);
        float4x4_t Staged{};
        XMStoreFloat4x4(&Staged, matrix_t(
            XMVectorLerp(Fitted.r[0], Authored.r[0], fBlend),
            XMVectorLerp(Fitted.r[1], Authored.r[1], fBlend),
            XMVectorLerp(Fitted.r[2], Authored.r[2], fBlend),
            XMVectorLerp(Fitted.r[3], Authored.r[3], fBlend)));
        for (const auto& Row : Staged.m) for (const f32_t Value : Row)
            if (!std::isfinite(Value)) return;
        World = Staged;
    }

    void Apply_StartingCaptureCameraFraming(const Client::EFFECT_DOCUMENT_DESC& Document,
        const Client::EFFECT_EVALUATED_PARTICLE& Particle, const Engine::CModel& Model,
        const f32_t fRootTimeSeconds, float4x4_t& World)
    {
        if (!Particle.pElement || !Is_StartingSceneCaptureCameraEmitter(*Particle.pElement) ||
            !Model.Has_LocalBounds()) return;
        f32_t fBegin = FLT_MAX, fEnd = FLT_MAX;
        for (const auto& Element : Document.Elements)
            if (Element.bVisible && Is_StartingSceneCaptureCameraEmitter(Element))
                fBegin = (std::min)(fBegin, Element.Detail.Timing.fStartDelaySeconds);
        for (const auto& Cue : Document.ModelCues)
            if (Cue.bVisible && Cue.strCueId == "altv.source.notify036.cube")
                fEnd = Cue.fStartDelaySeconds;
        const auto* View = Engine::CGameInstance::Get().Get_Transform(Engine::D3DTS::VIEW);
        const auto* Projection = Engine::CGameInstance::Get().Get_Transform(Engine::D3DTS::PROJ);
        if (!View || !Projection || fBegin == FLT_MAX || fEnd == FLT_MAX || fEnd <= fBegin) return;
        Fit_StartingCaptureMeshToCamera(Model.Get_LocalBoundsMin(), Model.Get_LocalBoundsMax(),
            *View, *Projection, (fRootTimeSeconds - fBegin) / (fEnd - fBegin), World);
    }

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
		const D3D11_RENDER_TARGET_BLEND_DESC& Bloom = Blend.RenderTarget[2u];
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
			Bloom.BlendEnable == SceneColor.BlendEnable &&
			Bloom.SrcBlend == SceneColor.SrcBlend &&
			Bloom.DestBlend == SceneColor.DestBlend &&
			Bloom.BlendOp == SceneColor.BlendOp &&
			Bloom.SrcBlendAlpha == SceneColor.SrcBlendAlpha &&
			Bloom.DestBlendAlpha == SceneColor.DestBlendAlpha &&
			Bloom.BlendOpAlpha == SceneColor.BlendOpAlpha &&
			Bloom.RenderTargetWriteMask == SceneColor.RenderTargetWriteMask &&
			std::all_of(Blend.RenderTarget + 3u, Blend.RenderTarget + 8u,
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

	Client::EFFECT_GPU_RENDER_FAMILY Resolve_GpuRenderFamily(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		// Location providers simulate for dependent particles but never own a GPU draw.
		if (Client::Is_EffectSimulationOnlyParticle(Element))
			return Client::EFFECT_GPU_RENDER_FAMILY::END;
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

	bool_t Validate_DimensionMasterWaterDropletBurstExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		constexpr std::string_view ELEMENT_ID =
			"project-tuned.water-burst.2050230.01";
		constexpr std::string_view SOURCE_NODE =
			"authored-copy:source.68619daf746949ce5bee";
		constexpr std::string_view NOISE_ASSET =
			"Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_003.dds";
		constexpr std::string_view FLUID_ASSET =
			"Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_006.dds";
		static constexpr std::array<std::string_view, 16u> SCALAR_NAMES = {{
			"water.noise-tiling", "water.pan-x", "water.pan-y",
			"water.second-octave-scale", "water.flow-warp",
			"water.mask-threshold", "water.edge-softness", "water.rim-width",
			"water.coverage-power", "water.body-strength",
			"water.rim-strength", "water.distortion-strength",
			"water.alpha-gain", "water.fade-start-seconds",
			"water.fade-end-seconds", "water.card-feather"
		}};
		static constexpr std::array<std::array<f32_t, 2u>, 16u>
			SCALAR_BOUNDS = {{
			{{ 0.01f, 16.f }}, {{ -8.f, 8.f }}, {{ -8.f, 8.f }},
			{{ 0.5f, 8.f }}, {{ 0.f, 0.25f }}, {{ 0.f, 1.f }},
			{{ 0.1f, 8.f }}, {{ 0.001f, 0.5f }}, {{ 0.1f, 4.f }},
			{{ 0.f, 8.f }}, {{ 0.f, 8.f }}, {{ -0.025f, 0.025f }},
			{{ 0.f, 4.f }}, {{ 0.f, 5.f }}, {{ 0.001f, 5.f }},
			{{ 0.001f, 0.49f }}
		}};
		const auto InRange = [](const f32_t Value, const f32_t Minimum,
			const f32_t Maximum)
		{
			return std::isfinite(Value) && Value >= Minimum && Value <= Maximum;
		};
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
			Element.Material.Execution;
		if (Element.strElementId != ELEMENT_ID)
		{
			if (Execution.bEnabled && Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				Execution.iOpcode == DIMENSIONMASTER_WATER_DROPLET_BURST_OPCODE)
			{
				strOutError =
					"DimensionMaster water-droplet opcode escaped its exact occurrence allowlist: " +
					Element.strElementId;
				return false;
			}
			return true;
		}

		const auto MatchesSampler = [](
			const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler,
			const Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE eAddress)
		{
			return Sampler.eFilter ==
					Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR &&
				Sampler.eAddressU == eAddress && Sampler.eAddressV == eAddress &&
				Sampler.eAddressW == eAddress && Sampler.fMipLodBias == 0.f &&
				Sampler.iMaxAnisotropy == 1u && Sampler.eComparison ==
					Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER &&
				Sampler.vBorderColor.x == 0.f && Sampler.vBorderColor.y == 0.f &&
				Sampler.vBorderColor.z == 0.f && Sampler.vBorderColor.w == 0.f &&
				Sampler.fMinLod == 0.f && Sampler.fMaxLod ==
					(std::numeric_limits<f32_t>::max)();
		};
		const auto MatchesLane = [&](
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
			const uint32_t iIndex,
			const std::string_view strRole,
			const std::string_view strAsset,
			const std::string_view strChannel,
			const Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE eAddress)
		{
			return Lane.strLaneId == "lane." + std::to_string(iIndex) &&
				Lane.strRole == strRole && Lane.strAssetId == strAsset &&
				Lane.iTextureRegister == iIndex &&
				Lane.iSamplerRegister == 5u + iIndex &&
				Lane.strSourceChannel == strChannel && Lane.eColorSpace ==
					Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
				MatchesSampler(Lane.Sampler, eAddress);
		};

		const bool_t bCarrier =
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.strSourceNode == SOURCE_NODE &&
			!Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == Client::EFFECT_RENDERER_TYPE::END &&
			Element.Renderer.eSourceSpace == Client::EFFECT_SOURCE_SPACE::END &&
			Element.ResourceBindings.size() == 2u &&
			Element.ResourceBindings[0u].strSlotId == "base" &&
			Element.ResourceBindings[0u].strAssetId == NOISE_ASSET &&
			Element.ResourceBindings[1u].strSlotId == "mask" &&
			Element.ResourceBindings[1u].strAssetId == FLUID_ASSET;
		const bool_t bMaterial =
			Element.Material.strTemplateId == "effect.standard" &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_01.fx_mi.fx_e_pa_fd_07_1_ad" &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
			!Element.Material.SourceMaterial.bEnabled;
		const bool_t bPacket = Execution.bEnabled && !Execution.bFailClosed &&
			!Execution.bAuthoringApproximate && Execution.eFidelity ==
				Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX &&
			Execution.iVersion == 1u && Execution.eBackend ==
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Execution.iOpcode == DIMENSIONMASTER_WATER_DROPLET_BURST_OPCODE &&
			Execution.iPassIndex == 1u &&
			Execution.strRasterizerState == "RS_Cull_None" &&
			Execution.strDepthStencilState == "DSS_ReadOnly" &&
			Execution.strBlendState == "BS_EffectAlpha" &&
			Execution.iStencilReference == 0u &&
			Execution.iTextureLaneCount == 2u &&
			Execution.iTextureMask == 0x03u &&
			Execution.TextureLanes.size() == 2u &&
			MatchesLane(Execution.TextureLanes[0u], 0u, "flow_noise",
				NOISE_ASSET, "RG",
				Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP) &&
			MatchesLane(Execution.TextureLanes[1u], 1u, "droplet_mask",
				FLUID_ASSET, "RGBA",
				Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP);
		const bool_t bMasks = Execution.iDynamicConsumedMask == 0u &&
			Execution.iDynamicSuppressedMask == 0x0fu &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x08u &&
			Execution.iParticleColorSuppressedMask == 0x07u &&
			Execution.iScalarCount == SCALAR_NAMES.size() &&
			Execution.iVectorCount == 2u && Execution.iInputCount == 16u &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ 0xffffu, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0x0fu, 0x0fu, 0u } &&
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

		bool_t bScalars = Execution.Scalars.size() == SCALAR_NAMES.size();
		for (size_t i = 0u; bScalars && i < SCALAR_NAMES.size(); ++i)
		{
			bScalars = Execution.Scalars[i].strName == SCALAR_NAMES[i] &&
				Execution.Scalars[i].iPackedIndex == i &&
				InRange(Execution.Scalars[i].fValue,
					SCALAR_BOUNDS[i][0u], SCALAR_BOUNDS[i][1u]);
		}
		if (bScalars)
		{
			const f32_t fFadeStart = Execution.Scalars[13u].fValue;
			const f32_t fFadeEnd = Execution.Scalars[14u].fValue;
			bScalars = fFadeEnd > fFadeStart;
		}
		bool_t bVectors = Execution.Vectors.size() == 2u;
		if (bVectors)
		{
			static constexpr std::array<std::string_view, 2u> NAMES = {{
				"water.body-color", "water.rim-color"
			}};
			for (size_t i = 0u; bVectors && i < NAMES.size(); ++i)
			{
				const auto& Vector = Execution.Vectors[i];
				const f32_t fMaximumRgb = 0u == i ? 4.f : 8.f;
				bVectors = Vector.strName == NAMES[i] &&
					Vector.iPackedIndex == i &&
					InRange(Vector.vValue.x, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.y, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.z, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.w, 0.f, 1.f);
			}
		}

		const Client::EFFECT_DETAIL_DESC& Detail = Element.Detail;
		const Client::EFFECT_PARTICLE_DESC& Particle = Detail.Particle;
		const Client::EFFECT_LINEAR_LERP_DESC& LinearLerp = Detail.LinearLerp;
		const auto InRange2 = [&InRange](const float2_t& Value,
			const f32_t Minimum, const f32_t Maximum)
		{
			return InRange(Value.x, Minimum, Maximum) &&
				InRange(Value.y, Minimum, Maximum);
		};
		const auto InRange3 = [&InRange](const float3_t& Value,
			const f32_t Minimum, const f32_t Maximum)
		{
			return InRange(Value.x, Minimum, Maximum) &&
				InRange(Value.y, Minimum, Maximum) &&
				InRange(Value.z, Minimum, Maximum);
		};
		const bool_t bMotion =
			InRange3(Detail.Transform.vPosition, -5.f, 8.f) &&
			InRange3(Detail.Transform.vRotationDegrees, -360.f, 360.f) &&
			InRange3(Detail.Transform.vRevolutionDegreesPerSecond,
				-720.f, 720.f) &&
			InRange3(Detail.Transform.vScale, 0.01f, 10.f) &&
			InRange3(Detail.Transform.vVelocityPerSecond, -20.f, 20.f) &&
			InRange(Detail.Timing.fStartDelaySeconds, 0.f, 5.f) &&
			InRange(Detail.Timing.fLifeTimeSeconds, 0.05f, 5.f) &&
			InRange(Detail.Color.vColorMultiply.x, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.y, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.z, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.w, 0.f, 1.f) &&
			Detail.Color.vColorOffset.x == 0.f &&
			Detail.Color.vColorOffset.y == 0.f &&
			Detail.Color.vColorOffset.z == 0.f &&
			Detail.Color.vColorOffset.w == 0.f &&
			InRange(Detail.Color.fColorClip, 0.f, 1.f) &&
			InRange(Detail.Color.fEmissiveIntensity, 0.f, 8.f) &&
			Detail.Color.fDistortionIntensity == 0.f &&
			!Detail.Color.bDistortionOnBaseMaterial &&
			Particle.iMaxParticles >= 1u && Particle.iMaxParticles <= 64u &&
			Particle.iBurstCount >= 1u &&
			Particle.iBurstCount <= Particle.iMaxParticles &&
			Particle.fSpawnRatePerSecond == 0.f &&
			Particle.iRandomSeed == 2050230u &&
			InRange2(Particle.vLifeTimeSeconds, 0.05f, 3.f) &&
			Particle.vLifeTimeSeconds.x <= Particle.vLifeTimeSeconds.y &&
			InRange3(Particle.vInitialPositionMin, -5.f, 5.f) &&
			InRange3(Particle.vInitialPositionMax, -5.f, 5.f) &&
			Particle.vInitialPositionMin.x <= Particle.vInitialPositionMax.x &&
			Particle.vInitialPositionMin.y <= Particle.vInitialPositionMax.y &&
			Particle.vInitialPositionMin.z <= Particle.vInitialPositionMax.z &&
			InRange3(Particle.vInitialVelocityMin, -20.f, 20.f) &&
			InRange3(Particle.vInitialVelocityMax, -20.f, 20.f) &&
			Particle.vInitialVelocityMin.x <= Particle.vInitialVelocityMax.x &&
			Particle.vInitialVelocityMin.y <= Particle.vInitialVelocityMax.y &&
			Particle.vInitialVelocityMin.z <= Particle.vInitialVelocityMax.z &&
			InRange3(Particle.vAcceleration, -40.f, 40.f) &&
			InRange2(Particle.vStartSize, 0.01f, 4.f) &&
			InRange2(Particle.vEndSize, 0.01f, 4.f) &&
			!Particle.bLocalSpace && Particle.bBillboard &&
			!LinearLerp.bColorOffset &&
			LinearLerp.vEndColorOffset.x == 0.f &&
			LinearLerp.vEndColorOffset.y == 0.f &&
			LinearLerp.vEndColorOffset.z == 0.f &&
			LinearLerp.vEndColorOffset.w == 0.f &&
			!LinearLerp.bColorMultiply &&
			LinearLerp.vEndColorMultiply.x == 1.f &&
			LinearLerp.vEndColorMultiply.y == 1.f &&
			LinearLerp.vEndColorMultiply.z == 1.f &&
			LinearLerp.vEndColorMultiply.w == 1.f;

		if (!bCarrier || !bMaterial || !bPacket || !bMasks || !bScalars ||
			!bVectors || !bMotion)
		{
			strOutError =
				"DimensionMaster water-droplet bounded project-tuned contract changed: " +
				Element.strElementId;
			return false;
		}
		return true;
	}

	bool_t Validate_DimensionMasterGlassMirrorMeshExecution(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		constexpr std::string_view AUDITION_ELEMENT_ID =
			"project-tuned.glass-mirror-shards.2050230.01";
		constexpr std::string_view PRODUCT_ELEMENT_ID =
			"project-tuned.single-glass.2050230.01";
		constexpr std::string_view AUDITION_SOURCE_NODE =
			"authored-copy:geometry-oracle.fx_m_glass_01";
		constexpr std::string_view PRODUCT_SOURCE_NODE =
			"authored-copy:single-carrier.fm_a_broken_012";
		constexpr std::string_view AUDITION_MODEL_ASSET =
			"Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel";
		constexpr std::string_view PRODUCT_MODEL_ASSET =
			"Effect/DimensionMaster/Meshes/fm_a_broken_012.wmodel";
		constexpr std::string_view PATTERN_ASSET =
			"Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/"
			"fx_h_brokenglass_02_1.dds";
		static constexpr std::array<std::string_view, 8u> SCALAR_NAMES = {{
			"CoverageGain", "BodyOpacity", "FresnelPower", "EdgeGain",
			"CrackGain", "RefractionStrength", "DistortionClamp",
			"EmissionGain"
		}};
		static constexpr std::array<std::array<f32_t, 2u>, 8u>
			SCALAR_BOUNDS = {{
			{{ 0.f, 4.f }}, {{ 0.f, 1.f }}, {{ 0.25f, 16.f }},
			{{ 0.f, 8.f }}, {{ 0.f, 4.f }}, {{ -0.025f, 0.025f }},
			{{ 0.f, 0.025f }}, {{ 0.f, 8.f }}
		}};
		const auto InRange = [](const f32_t Value, const f32_t Minimum,
			const f32_t Maximum)
		{
			return std::isfinite(Value) && Value >= Minimum && Value <= Maximum;
		};
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
			Element.Material.Execution;
		const bool_t bAuditionOccurrence =
			Element.strElementId == AUDITION_ELEMENT_ID;
		const bool_t bProductOccurrence =
			Element.strElementId == PRODUCT_ELEMENT_ID;
		if (!bAuditionOccurrence && !bProductOccurrence)
		{
			if (Execution.bEnabled && Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				Execution.iOpcode == DIMENSIONMASTER_GLASS_MIRROR_MESH_OPCODE)
			{
				strOutError =
					"DimensionMaster glass-mirror opcode escaped its exact occurrence allowlist: " +
					Element.strElementId;
				return false;
			}
			return true;
		}

		const auto MatchesSampler = [](
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
				Sampler.fMinLod == 0.f && Sampler.fMaxLod ==
					(std::numeric_limits<f32_t>::max)();
		};
		const bool_t bCarrier =
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.strSourceNode == (bAuditionOccurrence ?
				AUDITION_SOURCE_NODE : PRODUCT_SOURCE_NODE) &&
			!Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == Client::EFFECT_RENDERER_TYPE::END &&
			Element.Renderer.eSourceSpace == Client::EFFECT_SOURCE_SPACE::END &&
			Element.ResourceBindings.size() == 2u &&
			Element.ResourceBindings[0u].strSlotId == "meshModel" &&
			Element.ResourceBindings[0u].strAssetId == (bAuditionOccurrence ?
				AUDITION_MODEL_ASSET : PRODUCT_MODEL_ASSET) &&
			Element.ResourceBindings[1u].strSlotId == "base" &&
			Element.ResourceBindings[1u].strAssetId == PATTERN_ASSET;
		const bool_t bMaterial =
			Element.Material.strTemplateId == "effect.standard" &&
			Element.Material.strSourceMaterialPath.empty() &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
			!Element.Material.SourceMaterial.bEnabled;
		const bool_t bPacket = Execution.bEnabled && !Execution.bFailClosed &&
			!Execution.bAuthoringApproximate && Execution.eFidelity ==
				Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX &&
			Execution.iVersion == 1u && Execution.eBackend ==
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Execution.iOpcode == DIMENSIONMASTER_GLASS_MIRROR_MESH_OPCODE &&
			Execution.iPassIndex == 1u &&
			Execution.strRasterizerState == "RS_Cull_None" &&
			Execution.strDepthStencilState == "DSS_ReadOnly" &&
			Execution.strBlendState == "BS_EffectAlpha" &&
			Execution.iStencilReference == 0u &&
			Execution.iTextureLaneCount == 1u &&
			Execution.iTextureMask == 0x01u &&
			Execution.TextureLanes.size() == 1u &&
			Execution.TextureLanes[0u].strLaneId == "lane.0" &&
			Execution.TextureLanes[0u].strRole == "broken_glass_pattern" &&
			Execution.TextureLanes[0u].strAssetId == PATTERN_ASSET &&
			Execution.TextureLanes[0u].iTextureRegister == 0u &&
			Execution.TextureLanes[0u].iSamplerRegister == 5u &&
			Execution.TextureLanes[0u].strSourceChannel == "RGBA" &&
			Execution.TextureLanes[0u].eColorSpace ==
				Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR &&
			MatchesSampler(Execution.TextureLanes[0u].Sampler);
		const bool_t bMasks = Execution.iDynamicConsumedMask == 0u &&
			Execution.iDynamicSuppressedMask == 0x0fu &&
			Execution.iParticleColorPolicy == 2u &&
			Execution.iParticleColorConsumedMask == 0x08u &&
			Execution.iParticleColorSuppressedMask == 0x07u &&
			Execution.iScalarCount == SCALAR_NAMES.size() &&
			Execution.iVectorCount == 2u && Execution.iInputCount == 8u &&
			Execution.InputConsumedMask ==
				std::array<uint32_t, 2u>{ 0xffu, 0u } &&
			Execution.InputSuppressedMask ==
				std::array<uint32_t, 2u>{ 0u, 0u } &&
			Execution.VectorComponentConsumedMask ==
				std::array<uint32_t, 3u>{ 0x0fu, 0x0fu, 0u } &&
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

		bool_t bScalars = Execution.Scalars.size() == SCALAR_NAMES.size();
		for (size_t i = 0u; bScalars && i < SCALAR_NAMES.size(); ++i)
		{
			bScalars = Execution.Scalars[i].strName == SCALAR_NAMES[i] &&
				Execution.Scalars[i].iPackedIndex == i &&
				InRange(Execution.Scalars[i].fValue,
					SCALAR_BOUNDS[i][0u], SCALAR_BOUNDS[i][1u]);
		}
		bool_t bVectors = Execution.Vectors.size() == 2u;
		if (bVectors)
		{
			static constexpr std::array<std::string_view, 2u> NAMES = {{
				"BodyTintLinear", "EdgeTintLinear"
			}};
			for (size_t i = 0u; bVectors && i < NAMES.size(); ++i)
			{
				const auto& Vector = Execution.Vectors[i];
				const f32_t fMaximumRgb = 0u == i ? 4.f : 8.f;
				bVectors = Vector.strName == NAMES[i] &&
					Vector.iPackedIndex == i &&
					InRange(Vector.vValue.x, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.y, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.z, 0.f, fMaximumRgb) &&
					InRange(Vector.vValue.w, 0.f, 1.f);
			}
		}

		const auto InRange2 = [&InRange](const float2_t& Value,
			const f32_t Minimum, const f32_t Maximum)
		{
			return InRange(Value.x, Minimum, Maximum) &&
				InRange(Value.y, Minimum, Maximum);
		};
		const auto InRange3 = [&InRange](const float3_t& Value,
			const f32_t Minimum, const f32_t Maximum)
		{
			return InRange(Value.x, Minimum, Maximum) &&
				InRange(Value.y, Minimum, Maximum) &&
				InRange(Value.z, Minimum, Maximum);
		};
		const Client::EFFECT_DETAIL_DESC& Detail = Element.Detail;
		const Client::EFFECT_PARTICLE_DESC& Particle = Detail.Particle;
		const Client::EFFECT_LINEAR_LERP_DESC& LinearLerp = Detail.LinearLerp;
		const bool_t bShapeMotion =
			InRange3(Detail.Transform.vPosition, -5.f, 5.f) &&
			InRange3(Detail.Transform.vRotationDegrees, -360.f, 360.f) &&
			InRange3(Detail.Transform.vRevolutionDegreesPerSecond,
				-720.f, 720.f) &&
			InRange3(Detail.Transform.vScale, 0.01f, 10.f) &&
			InRange3(Detail.Transform.vVelocityPerSecond, -10.f, 10.f) &&
			InRange(Detail.Timing.fStartDelaySeconds, 0.f, 5.f) &&
			InRange(Detail.Timing.fLifeTimeSeconds, 0.05f, 5.f) &&
			!Detail.Mesh.bUseModelMaterial &&
			InRange(Detail.Mesh.fModelPreScale, 0.001f, 0.1f) &&
			InRange3(Detail.Mesh.vSourceTypeDataRotationDegrees,
				-360.f, 360.f) &&
			InRange(Detail.Color.vColorMultiply.x, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.y, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.z, 0.f, 4.f) &&
			InRange(Detail.Color.vColorMultiply.w, 0.f, 1.f) &&
			Detail.Color.vColorOffset.x == 0.f &&
			Detail.Color.vColorOffset.y == 0.f &&
			Detail.Color.vColorOffset.z == 0.f &&
			Detail.Color.vColorOffset.w == 0.f &&
			InRange(Detail.Color.fColorClip, 0.f, 1.f) &&
			InRange(Detail.Color.fEmissiveIntensity, 0.f, 8.f) &&
			Detail.Color.fDistortionIntensity == 0.f &&
			!Detail.Color.bDistortionOnBaseMaterial &&
			Particle.iMaxParticles == 1u && Particle.iBurstCount == 1u &&
			Particle.fSpawnRatePerSecond == 0.f &&
			Particle.iRandomSeed == 2050231u &&
			InRange2(Particle.vLifeTimeSeconds, 0.05f, 5.f) &&
			Particle.vLifeTimeSeconds.x <= Particle.vLifeTimeSeconds.y &&
			InRange3(Particle.vInitialPositionMin, -2.f, 2.f) &&
			InRange3(Particle.vInitialPositionMax, -2.f, 2.f) &&
			Particle.vInitialPositionMin.x <= Particle.vInitialPositionMax.x &&
			Particle.vInitialPositionMin.y <= Particle.vInitialPositionMax.y &&
			Particle.vInitialPositionMin.z <= Particle.vInitialPositionMax.z &&
			InRange3(Particle.vInitialVelocityMin, -10.f, 10.f) &&
			InRange3(Particle.vInitialVelocityMax, -10.f, 10.f) &&
			Particle.vInitialVelocityMin.x <= Particle.vInitialVelocityMax.x &&
			Particle.vInitialVelocityMin.y <= Particle.vInitialVelocityMax.y &&
			Particle.vInitialVelocityMin.z <= Particle.vInitialVelocityMax.z &&
			InRange3(Particle.vAcceleration, -20.f, 20.f) &&
			InRange2(Particle.vStartSize, 0.01f, 4.f) &&
			InRange2(Particle.vEndSize, 0.01f, 4.f) &&
			Particle.bLocalSpace && !Particle.bBillboard &&
			Particle.fDrag == 0.f &&
			Particle.vRotationRangeDegrees.x == 0.f &&
			Particle.vRotationRangeDegrees.y == 0.f &&
			Particle.vSpinRangeDegreesPerSecond.x == 0.f &&
			Particle.vSpinRangeDegreesPerSecond.y == 0.f &&
			!Particle.bSubUVOverLife &&
			Particle.iDynamicParameterComponentMask == 0u &&
			Particle.SpawnShape.eKind ==
				Client::EFFECT_PARTICLE_SPAWN_SHAPE::POINT &&
			Particle.SpawnShape.eDistribution ==
				Client::EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM &&
			Particle.InitialOrientation.Is_Default() &&
			Particle.InitialVelocity.eMode ==
				Client::EFFECT_PARTICLE_VELOCITY_MODE::FIXED &&
			Particle.TargetAttractor.Is_Default() &&
			Particle.SourceScale.Is_Default() &&
			!LinearLerp.bColorOffset &&
			LinearLerp.vEndColorOffset.x == 0.f &&
			LinearLerp.vEndColorOffset.y == 0.f &&
			LinearLerp.vEndColorOffset.z == 0.f &&
			LinearLerp.vEndColorOffset.w == 0.f &&
			!LinearLerp.bColorMultiply &&
			LinearLerp.vEndColorMultiply.x == 1.f &&
			LinearLerp.vEndColorMultiply.y == 1.f &&
			LinearLerp.vEndColorMultiply.z == 1.f &&
			LinearLerp.vEndColorMultiply.w == 1.f;

		if (!bCarrier || !bMaterial || !bPacket || !bMasks || !bScalars ||
			!bVectors || !bShapeMotion)
		{
			strOutError =
				"DimensionMaster glass-mirror bounded project-tuned contract changed: " +
				Element.strElementId;
			return false;
		}
		return true;
	}

	bool_t Validate_DimensionMasterGlassMirrorDocumentOccurrence(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		constexpr std::string_view AUDITION_DOCUMENT_ID =
			"effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified";
		constexpr std::string_view PRODUCT_DOCUMENT_ID =
			"effect.dimensionmaster.skill.2050230.single-glass-canary";
		constexpr std::string_view AUDITION_ELEMENT_ID =
			"project-tuned.glass-mirror-shards.2050230.01";
		constexpr std::string_view PRODUCT_ELEMENT_ID =
			"project-tuned.single-glass.2050230.01";
		size_t iOccurrenceCount = 0u;
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution =
				Element.Material.Execution;
			if (!Execution.bEnabled || Execution.eBackend !=
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 ||
				Execution.iOpcode != DIMENSIONMASTER_GLASS_MIRROR_MESH_OPCODE)
			{
				continue;
			}
			++iOccurrenceCount;
			const bool_t bAuditionOccurrence =
				Document.strEffectAssetId == AUDITION_DOCUMENT_ID &&
				Element.strElementId == AUDITION_ELEMENT_ID;
			const bool_t bProductOccurrence =
				Document.strEffectAssetId == PRODUCT_DOCUMENT_ID &&
				Element.strElementId == PRODUCT_ELEMENT_ID;
			if (!bAuditionOccurrence && !bProductOccurrence)
			{
				strOutError =
					"DimensionMaster glass-mirror opcode escaped its exact document/element occurrence allowlist.";
				return false;
			}
		}
		if (iOccurrenceCount > 1u)
		{
			strOutError =
				"DimensionMaster glass-mirror document contains more than one admitted opcode occurrence.";
			return false;
		}
		return true;
	}

	bool_t Validate_DimensionMasterProjectTunedDocumentExecution(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		if (!Validate_DimensionMasterGlassMirrorDocumentOccurrence(
				Document, strOutError))
		{
			return false;
		}
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Validate_DimensionMasterWaterDropletBurstExecution(
					Element, strOutError) ||
				!Validate_DimensionMasterGlassMirrorMeshExecution(
					Element, strOutError))
			{
				return false;
			}
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
				A.strSuppressHorizontalRootMotionBone != B.strSuppressHorizontalRootMotionBone ||
				A.eAlphaMode != B.eAlphaMode ||
				A.Material.has_value() != B.Material.has_value() ||
				(A.Material && (A.Material->strTemplateId != B.Material->strTemplateId ||
				 A.Material->strSourceMaterialPath != B.Material->strSourceMaterialPath ||
				 A.Material->eRenderProfile != B.Material->eRenderProfile ||
				 !Same_MaterialExecutionResourceSignature(A.Material->Execution, B.Material->Execution) ||
				 !Client::Is_EffectSourceMaterialStagingSignatureEqual(
					 A.Material->SourceMaterial, B.Material->SourceMaterial))) ||
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
				A.SourceRecipe.bSimulationOnly != B.SourceRecipe.bSimulationOnly ||
				A.SourceRecipe.strRendererShape !=
					B.SourceRecipe.strRendererShape ||
				!Same_TypedDynamicParameterResourceSignature(
					A.SourceRecipe, B.SourceRecipe) ||
				A.Material.strTemplateId != B.Material.strTemplateId ||
				A.Material.bColorTexturesSRGB != B.Material.bColorTexturesSRGB ||
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
			if (A.Detail.Mesh.SourceMaterialSlots.size() != B.Detail.Mesh.SourceMaterialSlots.size())
				return false;
			for (size_t j = 0u; j < A.Detail.Mesh.SourceMaterialSlots.size(); ++j)
			{
				const auto& X = A.Detail.Mesh.SourceMaterialSlots[j];
				const auto& Y = B.Detail.Mesh.SourceMaterialSlots[j];
				if (X.iSourceMaterialIndex != Y.iSourceMaterialIndex ||
					X.Material.strTemplateId != Y.Material.strTemplateId ||
					X.Material.strSourceMaterialPath != Y.Material.strSourceMaterialPath ||
					X.Material.bColorTexturesSRGB != Y.Material.bColorTexturesSRGB ||
					X.Material.eRenderProfile != Y.Material.eRenderProfile ||
					!Same_MaterialExecutionResourceSignature(X.Material.Execution, Y.Material.Execution) ||
					!Client::Is_EffectSourceMaterialStagingSignatureEqual(
						X.Material.SourceMaterial, Y.Material.SourceMaterial))
					return false;
			}
			for (size_t j = 0u; j < A.ResourceBindings.size(); ++j)
			{
				if (A.ResourceBindings[j].strSlotId != B.ResourceBindings[j].strSlotId ||
					A.ResourceBindings[j].strAssetId != B.ResourceBindings[j].strAssetId)
					return false;
			}
		}
		return true;
	}
}
