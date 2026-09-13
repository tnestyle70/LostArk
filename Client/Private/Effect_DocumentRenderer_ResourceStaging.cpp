#include "Effect_DocumentRenderer_Internal.h"
#include "ActorCatalog.h"
#include "Model.h"
#include "BinaryAsset/ModelAssetData.h"
#include "RuntimeAssetRoot.h"
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
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"
#include "DirectXTK/DDSTextureLoader.h"

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
	if (!Element.Detail.Mesh.SourceMaterialSlots.empty())
	{
		const auto& Slots = Element.Detail.Mesh.SourceMaterialSlots;
		if (Slots.size() > 32u || !Is_EffectElementAuthoringExecutionTarget(Element))
		{
			strOutError = "Mesh source material slots are not executable.";
			return E_INVALIDARG;
		}
		PREWARM_ASSET_CACHE LocalAssets;
		PREWARM_ASSET_CACHE* pSlotAssets = nullptr != pSharedAssets ? pSharedAssets : &LocalAssets;
		EFFECT_ELEMENT_DESC Leaf = Element;
		Leaf.Detail.Mesh.SourceMaterialSlots.clear();
		std::unordered_set<uint32_t> ExpectedSlots;
		for (const auto& Slot : Slots)
		{
			if (!ExpectedSlots.insert(Slot.iSourceMaterialIndex).second)
			{
				strOutError = "Mesh source material slot is duplicated.";
				return E_INVALIDARG;
			}
			Leaf.Material = Slot.Material;
			auto pLeaf = std::make_shared<ELEMENT_RESOURCE>();
			if (FAILED(Stage_ElementResource(Leaf, *pLeaf, strOutError,
				pSlotAssets, fModelPreScale)))
				return E_FAIL;
			if (nullptr == pLeaf->pModel || pLeaf->bSourceMaterialFallbackBlocked ||
				pLeaf->bOccurrenceVisualSuppressed || !pLeaf->SourceMaterialSlots.empty())
			{
				strOutError = "Mesh source material slot preparation was incomplete.";
				return E_FAIL;
			}
			if (nullptr == Staged.pModel)
				Staged.pModel = pLeaf->pModel;
			if (Staged.pModel != pLeaf->pModel)
			{
				strOutError = "Mesh source material slots must share the same CModel.";
				return E_FAIL;
			}
			Staged.bSourceRequiresSceneColor |= pLeaf->bSourceRequiresSceneColor;
			Staged.bSourceRequiresSceneDepth |= pLeaf->bSourceRequiresSceneDepth;
			Staged.SourceMaterialSlots.push_back({ Slot.iSourceMaterialIndex, std::move(pLeaf) });
		}
		std::unordered_set<uint32_t> ActualSlots;
		for (uint32_t iMesh = 0u; iMesh < Staged.pModel->Get_NumMeshes(); ++iMesh)
		{
			uint32_t iMaterialIndex = 0u;
			if (!Staged.pModel->Try_GetSourceMaterialIndex(iMesh, iMaterialIndex))
			{
				strOutError = "CModel source material index is invalid.";
				return E_FAIL;
			}
			ActualSlots.insert(iMaterialIndex);
		}
		if (ActualSlots != ExpectedSlots)
		{
			strOutError = "Mesh source material slots do not exactly cover CModel material indices.";
			return E_FAIL;
		}
		// Publish only after every texture/program/model slot has succeeded.
		OutResource = std::move(Staged);
		return S_OK;
	}

	if (EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
		(EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind &&
		 nullptr == Find_LanceMasterVAProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId) &&
		 nullptr == Find_ArtistProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId) &&
		 nullptr == Find_WarlordNativeProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId) &&
		 nullptr == Find_DimensionMasterVProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId) &&
		 nullptr == Find_DimensionMasterALTVProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId)))
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
    if (Element.eKind == EFFECT_ELEMENT_KIND::MESH || Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
    {
        EFFECT_SHADER_FAMILY Family = EFFECT_SHADER_FAMILY::GENERIC;
        if (Find_LanceMasterVAProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::LANCE_MASTER;
        else if (Find_ArtistProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::ARTIST;
        else if (Find_WarlordNativeProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::WARLORD;
        else if (Find_DimensionMasterSDProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::DIMENSIONMASTER_SD;
        else if (Find_DimensionMasterWRProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::DIMENSIONMASTER_WR;
        else if (Find_DimensionMasterALTVProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::DIMENSIONMASTER_ALTV;
        else if (Find_DimensionMasterVProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::DIMENSIONMASTER_V;
        else if (Find_DimensionMasterQProgram(SourceMaterial.strRuntimeShaderProfileId)) Family = EFFECT_SHADER_FAMILY::DIMENSIONMASTER_Q;
        const EFFECT_SHADER_CARRIER Carrier = Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
            nullptr != Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) ?
            EFFECT_SHADER_CARRIER::MESH : EFFECT_SHADER_CARRIER::PARTICLE;
        Staged.iShaderProgramIndex = Find_EffectShaderProgramIndex(Carrier, Family, Staged.iSourceMaterialProfile);
        if (UINT32_MAX == Staged.iShaderProgramIndex)
        {
            strOutError = "Admitted source material has no compiled family/carrier program: " + Element.strElementId;
            return E_INVALIDARG;
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
	if (const auto* pNative = Find_LanceMasterVAProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!Build_LanceMasterVAParameters(SourceMaterial, Staged.LanceVASourceMaterialParameters))
		{
			strOutError = "Native Lance Master material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNative->bNeedsSceneColor;
		Staged.bSourceRequiresSceneDepth = pNative->bNeedsDepthSample || pNative->bNeedsSceneColor;
	}
	if (const auto* pNative = Find_ArtistProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!Build_ArtistParameters(SourceMaterial, Staged.ArtistSourceMaterialParameters))
		{
			strOutError = "Native Artist material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNative->bNeedsSceneColor;
		Staged.bSourceRequiresSceneDepth = pNative->bNeedsDepthSample || pNative->bNeedsSceneColor;
	}
	if (const auto* pNative = Find_WarlordNativeProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!Build_WarlordNativeParameters(SourceMaterial, Staged.VSourceMaterialParameters))
		{
			strOutError = "Native Warlord material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNative->bNeedsSceneColor;
		Staged.bSourceRequiresSceneDepth = pNative->bNeedsDepthSample || pNative->bNeedsSceneColor;
	}
	if (const auto* pNativeWR = Find_DimensionMasterWRProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		// W/R uses the existing V packet: at most 15 of its 32 float4 rows.
		if (!Build_DimensionMasterWRParameters(SourceMaterial, Staged.VSourceMaterialParameters))
		{
			strOutError = "Native W/R material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNativeWR->bNeedsSceneColor;
	}
	if (const auto* pNative = Find_DimensionMasterALTVProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!Build_DimensionMasterALTVParameters(SourceMaterial, Staged.ALTVSourceMaterialParameters))
		{
			strOutError = "Native ALT_V material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNative->bNeedsSceneColor;
	}
	if (const auto* pNativeSD = Find_DimensionMasterSDProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!Build_DimensionMasterSDParameters(SourceMaterial, Staged.VSourceMaterialParameters))
		{
			strOutError = "Native S material parameter names or values are invalid: " + Element.strElementId;
			return E_INVALIDARG;
		}
		Staged.bSourceRequiresSceneColor = pNativeSD->bNeedsSceneColor;
		Staged.bSourceRequiresSceneDepth = pNativeSD->bNeedsDepthSample;
		if (pNativeSD->bUsesOneLayerDistortion)
		{
			// UE3 BasePass switches OneLayer distortion to opaque blending:
			// its RT0 already contains the sampled scene. Retain the material's
			// read-only depth pass and leave the distortion accumulation MRT alone.
			D3D11_BLEND_DESC Blend{};
			Blend.IndependentBlendEnable = TRUE;
			for (auto& Target : Blend.RenderTarget)
			{
				Target.SrcBlend = Target.SrcBlendAlpha = D3D11_BLEND_ONE;
				Target.DestBlend = Target.DestBlendAlpha = D3D11_BLEND_ZERO;
				Target.BlendOp = Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			}
			Blend.RenderTarget[0u].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			Blend.RenderTarget[2u] = Blend.RenderTarget[0u];
			const HRESULT Result = m_pDevice->CreateBlendState(&Blend, &Staged.pNativeOneLayerBlend);
			if (FAILED(Result))
			{
				strOutError = "Native OneLayer blend-state creation failed: " + Element.strElementId;
				return Result;
			}
		}
	}
	if (Staged.iSourceMaterialProfile >= 52u && Staged.iSourceMaterialProfile <= 76u &&
		!Client::Build_DimensionMasterVParameters(SourceMaterial, Staged.VSourceMaterialParameters))
	{
		strOutError = "Native V material parameter names or values are invalid: " + Element.strElementId;
		return E_INVALIDARG;
	}
	if (Staged.iSourceMaterialProfile >= 44u && Staged.iSourceMaterialProfile <= 51u &&
		!Client::Build_DimensionMasterQParameters(SourceMaterial, Staged.QSourceMaterialParameters))
	{
		strOutError = "Native Q material parameter names or values are invalid: " + Element.strElementId;
		return E_INVALIDARG;
	}
	if (43u == Staged.iSourceMaterialProfile)
	{
		// Exact native uniform rotations use parameter * 0.25 radians.
		const float flowAngle = SourceScalar(SourceMaterial, "slice_flow_rot", 0.f) * 0.25f;
		const float sliceAngle = SourceScalar(SourceMaterial, "slice_rot", -3.140000104904175f) * 0.25f;
		Staged.vSourceScalars0 = { 0.f,
			SourceScalar(SourceMaterial, "opacity_radius", 2.f),
			SourceScalar(SourceMaterial, "flow_str", 0.f),
			SourceScalar(SourceMaterial, "depth", 0.30000001192092896f) };
		Staged.vSourceScalars1 = { SourceScalar(SourceMaterial, "slice_flow_tileu", 1.f),
			SourceScalar(SourceMaterial, "slice_flow_tilev", 1.f),
			SourceScalar(SourceMaterial, "slice_flow_offsetx", 0.f),
			SourceScalar(SourceMaterial, "slice_flow_offsety", 0.f) };
		Staged.vSourceVector0 = { std::cos(flowAngle), -std::sin(flowAngle),
			std::sin(flowAngle), std::cos(flowAngle) };
		Staged.vSourceVector1 = { std::cos(sliceAngle), -std::sin(sliceAngle),
			std::sin(sliceAngle), std::cos(sliceAngle) };
	}
	if (42u == Staged.iSourceMaterialProfile)
	{
		// Selected native PS CB0[5].xyz and CB0[3]/[4], packed by semantic name.
		Staged.vSourceScalars0 = { SourceScalar(SourceMaterial, "spec_str", 10.f),
			SourceScalar(SourceMaterial, "edge_line", 20.f),
			SourceScalar(SourceMaterial, "edge_str", 5.f), 0.f };
		Staged.vSourceVector0 = SourceVector(SourceMaterial, "color", { 1.f, 1.f, 1.f, 1.f });
		Staged.vSourceVector1 = SourceVector(SourceMaterial, "meshemitterdynamicparameter", { 1.f, 1.f, 1.f, 1.f });
		if (!std::isfinite(Staged.vSourceScalars0.x) || Staged.vSourceScalars0.x < 0.f ||
			!std::isfinite(Staged.vSourceScalars0.y) || Staged.vSourceScalars0.y < 0.f ||
			!std::isfinite(Staged.vSourceScalars0.z) || Staged.vSourceScalars0.z < 0.f)
		{
			strOutError = "CubeSample material has invalid spec/edge parameters: " + Element.strElementId;
			return E_INVALIDARG;
		}
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
					fModelPreScale, fModelPreScale, fModelPreScale), true);
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

	if (nullptr != Staged.pModel)
	{
		Staged.iSourceMeshHasUV1 =
			0u != (Staged.pModel->Get_GeometryEvidenceFlags() &
				Engine::MODEL_GEOMETRY_TEXCOORD1_PRESERVED_FROM_GLTF) ? 1u : 0u;
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
		HRESULT TextureResult;
		if (Element.Material.bColorTexturesSRGB)
		{
			EFFECT_NAMED_TEXTURE_DESC TextureDesc;
			TextureDesc.strName = Binding.strSlotId;
			TextureDesc.strAssetId = Binding.strAssetId;
			TextureDesc.eColorSpace =
				pInput->eSemantic == EFFECT_MATERIAL_INPUT_SEMANTIC::BASE ||
				pInput->eSemantic == EFFECT_MATERIAL_INPUT_SEMANTIC::BASE2 ||
				pInput->eSemantic == EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE ?
					EFFECT_TEXTURE_COLOR_SPACE::SRGB : EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
			TextureResult = Load_SourceTexture(TextureDesc, Texture, pSharedAssets);
		}
		else
		{
			TextureResult = Load_Texture(Binding.strAssetId, Texture, pSharedAssets);
		}
		if (FAILED(TextureResult))
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
	if (const auto* pLance = Find_LanceMasterVAProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pLance->TextureNames)) return E_FAIL;
	}
	else if (const auto* pNative = Find_ArtistProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNative->TextureNames)) return E_FAIL;
	}
	else	if (const auto* pNative = Find_WarlordNativeProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNative->TextureNames)) return E_FAIL;
	}
	else if (const auto* pNativeSD = Find_DimensionMasterSDProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNativeSD->TextureNames)) return E_FAIL;
	}
	else if (const auto* pNativeWR = Find_DimensionMasterWRProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNativeWR->TextureNames))
			return E_FAIL;
	}
	else if (const auto* pNativeALTV = Find_DimensionMasterALTVProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNativeALTV->TextureNames)) return E_FAIL;
	}
	else if (const auto* pNativeV = Client::Find_DimensionMasterVProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNativeV->TextureNames))
			return E_FAIL;
	}
	else if (const auto* pNativeQ = Client::Find_DimensionMasterQProgram(SourceMaterial.strRuntimeShaderProfileId))
	{
		if (!StageRequiredNamedTextureContract(pNativeQ->TextureNames))
			return E_FAIL;
	}
	else if (11u == Staged.iSourceMaterialProfile && SourceMaterial.Textures.empty())
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
	else if (43u == Staged.iSourceMaterialProfile)
	{
		if (!StageNamedSourceTexture(0u,
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "slice_flow_texture"), true))
			return E_FAIL;
	}
	else if (42u == Staged.iSourceMaterialProfile)
	{
		if (!StageNamedSourceTexture(0u,
			Client::Find_EffectUniqueNamedTexture(SourceMaterial, "spec_texture"), true))
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
		nullptr != Find_LanceMasterVAProgram(SourceMaterial.strRuntimeShaderProfileId) ||
		nullptr != Find_ArtistProgram(SourceMaterial.strRuntimeShaderProfileId) ||
		nullptr != Find_WarlordNativeProgram(SourceMaterial.strRuntimeShaderProfileId) ||
		nullptr != Find_DimensionMasterSDProgram(SourceMaterial.strRuntimeShaderProfileId) ||
		14u == Staged.iSourceMaterialProfile ||
		16u == Staged.iSourceMaterialProfile ||
		(Staged.iSourceMaterialProfile >= 19u &&
		 Staged.iSourceMaterialProfile <= 32u) ||
		33u == Staged.iSourceMaterialProfile ||
		34u == Staged.iSourceMaterialProfile ||
		35u == Staged.iSourceMaterialProfile ||
		(Staged.iSourceMaterialProfile >= 36u &&
		 Staged.iSourceMaterialProfile <= 76u) ||
		(Staged.iSourceMaterialProfile >= 80u && Staged.iSourceMaterialProfile <= 205u) ||
		(Staged.iSourceMaterialProfile >= 208u && Staged.iSourceMaterialProfile <= 263u || (Staged.iSourceMaterialProfile >= 277u && Staged.iSourceMaterialProfile <= 280u));
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
		((41u == Staged.iSourceMaterialProfile || 42u == Staged.iSourceMaterialProfile ||
		  43u == Staged.iSourceMaterialProfile) &&
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
	if (!Validate_DimensionMasterWaterDropletBurstExecution(
			Element, strOutError))
		return false;
	if (!Validate_DimensionMasterGlassMirrorMeshExecution(
			Element, strOutError))
		return false;
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
		std::to_string(Cue.vAssetPreRotationDegrees.z) + "\n" +
		Cue.strSuppressHorizontalRootMotionBone;
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
		{
			const std::scoped_lock Lock(g_EffectRenderCacheMutex);
			++g_EffectRenderPrewarmProbe.iModelDiskLoadCount;
		}
		Engine::MODEL_ASSET_LOAD_DESC ModelLoad;
		if (!CActorCatalog::Build_ModelLoadDescription(Cue.strModelAssetId, ModelLoad, strOutError))
			return E_FAIL;
		unique_ptr<Engine::CModel> Loaded = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::ANIM, ModelLoad, PreTransform);
		if (nullptr != Loaded)
		{
			// Set the suppression baseline on the unposed prototype. Clones retain
			// this setting, while their animation clocks remain independent.
			if (!Cue.strSuppressHorizontalRootMotionBone.empty() &&
				!Loaded->Enable_RootMotionSuppression(Cue.strSuppressHorizontalRootMotionBone.c_str(), 1))
			{
				strOutError = "Animated Model Cue root-motion bone is missing: " +
					Cue.strCueId + " / " + Cue.strSuppressHorizontalRootMotionBone;
				return E_FAIL;
			}
			Model = std::move(Loaded);
		}
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
		(!Cue.bLoop && !Cue.bHoldLastFrame &&
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

	std::shared_ptr<const ELEMENT_RESOURCE> MaterialResource;
	if (Cue.Material)
	{
		if (!Has_ArtistModelCueMaterialContract(Cue) && !Has_LanceMasterVAModelCueMaterialContract(Cue) &&
            !Has_DimensionMasterALTVModelCueMaterialContract(Cue))
		{
			strOutError = "Animated Model Cue native material is invalid: " + Cue.strCueId;
			return E_INVALIDARG;
		}
		for (uint32_t iMesh = 0u; iMesh < Model->Get_NumMeshes(); ++iMesh)
		{
			uint32_t iMaterial = 0u;
			if (!Model->Try_GetSourceMaterialIndex(iMesh, iMaterial) || iMaterial != 0u)
			{
				strOutError = "Recovered skeletal material requires the original single slot: " + Cue.strCueId;
				return E_FAIL;
			}
		}
		const auto& Source = Cue.Material->SourceMaterial;
		const auto* ArtistProgram = Find_ArtistProgram(Source.strRuntimeShaderProfileId);
		const auto* LanceProgram = Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
        const auto* ALTVProgram = Find_DimensionMasterALTVProgram(Source.strRuntimeShaderProfileId);
		auto StagedMaterial = std::make_shared<ELEMENT_RESOURCE>();
		const auto Names = ArtistProgram ? ArtistProgram->TextureNames : LanceProgram ? LanceProgram->TextureNames : ALTVProgram ? ALTVProgram->TextureNames : std::span<const std::string_view>{};
		const bool ParametersValid = ArtistProgram ? Build_ArtistParameters(Source, StagedMaterial->ArtistSourceMaterialParameters) :
			LanceProgram ? Build_LanceMasterVAParameters(Source, StagedMaterial->LanceVASourceMaterialParameters) :
            ALTVProgram && Build_DimensionMasterALTVParameters(Source, StagedMaterial->ALTVSourceMaterialParameters);
		if (!ParametersValid || Names.size() > StagedMaterial->SourceTextures.size())
		{
			strOutError = "Animated Model Cue native parameter stage failed: " + Cue.strCueId;
			return E_FAIL;
		}
		StagedMaterial->iSourceMaterialProfile = ArtistProgram ? ArtistProgram->iProfileIndex : LanceProgram ? LanceProgram->iProfileIndex : ALTVProgram->iProfileIndex;
		StagedMaterial->bSourceRequiresSceneDepth = ArtistProgram ? ArtistProgram->bNeedsDepthSample : LanceProgram ? LanceProgram->bNeedsDepthSample : ALTVProgram->bNeedsDepthSample;
		for (size_t iLane = 0u; iLane < Names.size(); ++iLane)
		{
			const auto* Texture = Find_EffectUniqueNamedTexture(Source, Names[iLane]);
			if (!Texture || FAILED(Load_SourceTexture(*Texture, StagedMaterial->SourceTextures[iLane], pSharedAssets)))
			{
				strOutError = "Animated Model Cue source DDS stage failed: " + Cue.strCueId +
					" / " + std::string(Names[iLane]);
				return E_FAIL;
			}
			const uint32_t iBit = 1u << static_cast<uint32_t>(iLane);
			StagedMaterial->iSourceTextureMask |= iBit;
			if (Texture->eAddressU == EFFECT_TEXTURE_ADDRESS_MODE::CLAMP)
				StagedMaterial->iSourceTextureClampUMask |= iBit;
			if (Texture->eAddressV == EFFECT_TEXTURE_ADDRESS_MODE::CLAMP)
				StagedMaterial->iSourceTextureClampVMask |= iBit;
		}
		MaterialResource = std::move(StagedMaterial);
	}
	OutResource.pModel = std::move(Model);
	OutResource.pMaterialResource = std::move(MaterialResource);
	OutResource.iAnimationIndex = iAnimation;
	OutResource.fTicksPerSecond = fTicksPerSecond;
	OutResource.fDurationSeconds = fDurationTicks / fTicksPerSecond;
	OutResource.bHasSampledPose = false;
	return S_OK;
}
