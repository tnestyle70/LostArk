#include "Effect_DocumentRenderer.h"

#include "DeferredMaterialRenderUtils.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <span>
#include <unordered_map>

namespace
{
	inline constexpr std::array<std::string_view, 7u>
		LINEARFLOW_SOURCE_TEXTURE_NAMES = {{
			"diff_tex", "diff_noise_tex", "a_mask_tex", "a_noise_01_tex",
			"b_mask_tex", "b_noise_01_tex", "dissolve_tex"
		}};
	inline constexpr std::array<std::string_view, 5u>
		BLACKLINE_SOURCE_TEXTURE_NAMES = {{
			"diffuse_tex", "flow_tex", "mask_a_tex", "mask_b_tex",
			"dissolve_tex"
		}};
	inline constexpr std::array<std::string_view, 3u>
		LOCAL_CRACK_SOURCE_TEXTURE_NAMES = {{
			"normal_tex", "refle_tex", "dissolve_tex"
		}};

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
			LINEARFLOW_SOURCE_TEXTURE_NAMES, strName);
	}

	int32_t BlacklineSourceTextureIndex(const std::string_view strName)
	{
		return NamedSourceTextureIndex(BLACKLINE_SOURCE_TEXTURE_NAMES, strName);
	}

	int32_t LocalCrackSourceTextureIndex(const std::string_view strName)
	{
		return NamedSourceTextureIndex(
			LOCAL_CRACK_SOURCE_TEXTURE_NAMES, strName);
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
		float4_t& vScalars0,
		float4_t& vScalars1,
		float4_t& vAuxiliary0,
		float4_t& vAuxiliary1)
	{
		auto S = [&Source](const std::string_view strName, const f32_t fFallback)
		{
			return SourceScalar(Source, strName, fFallback);
		};
		vScalars0 = { S("alpha_tex_strength", 1.f),
			S("alpha_out_falloff", 1.f), S("emissive_tex_strength", 1.f),
			S("emissive_tex_power", 1.f) };
		vScalars1 = { S("alpha_tex_r_texcoord", 1.f),
			S("alpha_tex_g_texcoord", 1.f), S("uv_noise_head_velue", 0.f),
			S("dissolve_hardness", 5.f) };
		vAuxiliary0 = { S("alpha_tex_positon_r", 0.f),
			S("alpha_tex_positon_g", 0.f),
			S("uvnoise_tex_01_r_texcoord", 1.f),
			S("uvnoise_tex_01_g_texcoord", 1.f) };
		vAuxiliary1 = { S("uvnoise_tex_02_r_texcoord", 1.f),
			S("uvnoise_tex_02_g_texcoord", 1.f),
			S("emissive_tex_backvelue", 0.f),
			S("fresnelalpha_power", 1.f) };
	}

	template <size_t Size>
	bool_t Has_NamedSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::array<std::string_view, Size>& Names)
	{
		uint32_t iMask = 0u;
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture : Source.Textures)
		{
			const int32_t iIndex = NamedSourceTextureIndex(Names, Texture.strName);
			if (iIndex >= 0 && !Texture.strAssetId.empty())
				iMask |= 1u << static_cast<uint32_t>(iIndex);
		}
		return iMask == (1u << Names.size()) - 1u;
	}

	bool_t Has_LinearFlowSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		return Has_NamedSourceTextureContract(
			Source, LINEARFLOW_SOURCE_TEXTURE_NAMES);
	}

	bool_t Has_BlacklineSourceTextureContract(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		return Has_NamedSourceTextureContract(
			Source, BLACKLINE_SOURCE_TEXTURE_NAMES);
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
		const std::array<ComPtr<ID3D11ShaderResourceView>, 5>& Textures,
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		if (eSlot < Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE ||
			eSlot > Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
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
				"effect.ue3.missiletrail-01.v1" ||
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
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE),
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
			if (A.strElementId != B.strElementId || A.eKind != B.eKind ||
				A.Material.strTemplateId != B.Material.strTemplateId ||
				!Client::Is_EffectSourceMaterialStagingSignatureEqual(
					A.Material.SourceMaterial, B.Material.SourceMaterial) ||
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

	float4x4_t Make_BillboardWorld(const float4x4_t& Source)
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
			XMMatrixScalingFromVector(Scale) * CameraWorld *
			XMMatrixTranslationFromVector(Translation));
		return Result;
	}

	float4x4_t Make_ParticleSpriteWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle)
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
			return Source;
		}
		Scale = XMVectorSet(
			std::abs(XMVectorGetX(Scale)),
			std::abs(XMVectorGetY(Scale)),
			std::abs(XMVectorGetZ(Scale)), 0.f);
		matrix_t CameraWorld = CameraWorldWithTranslation;
		CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		matrix_t Orientation = CameraWorld;
		const f32_t fRoll = XMConvertToRadians(
			Particle.fSpriteRotationDegrees);
		switch (Particle.eSpriteAlignment)
		{
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE:
		{
			const f32_t fX = XMVectorGetX(Scale);
			Scale = XMVectorSet(fX, fX, XMVectorGetZ(Scale), 0.f);
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
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(-XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Y:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(XM_PIDIV2);
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
		float4x4_t Result{};
		XMStoreFloat4x4(&Result,
			Pivot * XMMatrixScalingFromVector(Scale) * Orientation *
			XMMatrixTranslationFromVector(Translation));
		return Result;
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
			"effect.ue3.missiletrail-01.v1")
			return 13u;
		return UINT32_MAX;
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
		return 0u;
	}

	Client::EFFECT_SUBUV_FRAME_DESC Resolve_SubUVFrames(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle)
	{
		if (nullptr == Particle.pElement ||
			!Particle.pElement->SourceRecipe.bEnabled)
		{
			return {};
		}
		const Client::EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Particle.pElement->Material.SourceMaterial;
		if (!SourceMaterial.bEnabled ||
			SourceMaterial.strSubUVMode == "none")
		{
			return {};
		}
		const uint32_t iColumns = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(*Particle.pElement,
				"subimages_horizontal", 1.f)));
		const uint32_t iRows = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(*Particle.pElement,
				"subimages_vertical", 1.f)));
		const bool_t bAllowFlip = SourceLiteralBool(*Particle.pElement,
			"ballowimageflipping", false);
		const bool_t bSquareFlip = SourceLiteralBool(*Particle.pElement,
			"bsquareimageflipping", false);
		return Client::CEffectPlayback::Resolve_SourceSubUVFrame(
			iColumns, iRows, Particle.fSubImageIndex,
			bAllowFlip, bSquareFlip, Particle.fDistributionRandom,
			SourceMaterial.strSubUVMode.starts_with("psuvim_linear_blend"));
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

	unique_ptr<Engine::CShader> MeshShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl"),
		VTXMESH::Elements, VTXMESH::iNumElements);
	unique_ptr<Engine::CShader> AnimatedModelShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
		VTXANIMMESH::Elements, VTXANIMMESH::iNumElements);
	unique_ptr<Engine::CShader> RectShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl"),
		VTXTEX::Elements, VTXTEX::iNumElements);
	unique_ptr<Engine::CShader> ParticleShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl"),
		Engine::VTXEFFECT_PARTICLE::Elements,
		Engine::VTXEFFECT_PARTICLE::iNumElements);
	unique_ptr<Engine::CShader> TrailShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl"),
		Engine::VTXEFFECT_TRAIL::Elements,
		Engine::VTXEFFECT_TRAIL::iNumElements);
	unique_ptr<Engine::CShader> DecalShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl"),
		VTXTEX::Elements, VTXTEX::iNumElements);
	unique_ptr<Engine::CVIBuffer_Rect> Rect =
		Engine::CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	unique_ptr<Engine::CVIBuffer_ParticleRect> ParticleBuffer =
		Engine::CVIBuffer_ParticleRect::Create(m_pDevice, m_pContext, 2048u);
	unique_ptr<Engine::CVIBuffer_DynamicTrail> TrailBuffer =
		Engine::CVIBuffer_DynamicTrail::Create(m_pDevice, m_pContext, 256u);
	if (nullptr == MeshShader || nullptr == AnimatedModelShader ||
		nullptr == RectShader ||
		nullptr == ParticleShader || nullptr == TrailShader ||
		nullptr == DecalShader || nullptr == Rect ||
		nullptr == ParticleBuffer || nullptr == TrailBuffer ||
		FAILED(Create_FallbackTextures()))
	{
		m_strStatus = "Effect renderer core resource creation failed.";
		return E_FAIL;
	}

	m_pMeshShader = std::move(MeshShader);
	m_pAnimatedModelShader = std::move(AnimatedModelShader);
	m_pRectShader = std::move(RectShader);
	m_pParticleShader = std::move(ParticleShader);
	m_pTrailShader = std::move(TrailShader);
	m_pDecalShader = std::move(DecalShader);
	m_pRect = std::move(Rect);
	m_pParticleBuffer = std::move(ParticleBuffer);
	m_pTrailBuffer = std::move(TrailBuffer);
	m_strStatus = "Effect renderer ready.";
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Create_FallbackTextures()
{
	const auto CreateSolid = [this](
		const uint32_t iRGBA,
		ComPtr<ID3D11ShaderResourceView>& OutSRV) -> HRESULT
	{
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
		if (FAILED(m_pDevice->CreateTexture2D(&Desc, &Data, &Texture)))
			return E_FAIL;
		return m_pDevice->CreateShaderResourceView(
			Texture.Get(), nullptr, &OutSRV);
	};
	return FAILED(CreateSolid(0xffffffffu, m_pWhiteTexture)) ||
		FAILED(CreateSolid(0xff000000u, m_pBlackTexture)) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutSRV) const
{
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	return DirectX::CreateDDSTextureFromFile(
		m_pDevice.Get(), Path.c_str(), nullptr, &OutSRV);
}

HRESULT Client::CEffectDocumentRenderer::Load_SourceTexture(
	const EFFECT_NAMED_TEXTURE_DESC& Texture,
	ComPtr<ID3D11ShaderResourceView>& OutSRV) const
{
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(Texture.strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	const DirectX::DDS_LOADER_FLAGS Flags =
		EFFECT_TEXTURE_COLOR_SPACE::SRGB == Texture.eColorSpace ?
			DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_IGNORE_SRGB;
	return DirectX::CreateDDSTextureFromFileEx(
		m_pDevice.Get(), Path.c_str(), 0u, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0u, 0u, Flags, nullptr, &OutSRV);
}

HRESULT Client::CEffectDocumentRenderer::Stage_ElementResource(
	const EFFECT_ELEMENT_DESC& Element,
	ELEMENT_RESOURCE& OutResource,
	std::string& strOutError) const
{
	ELEMENT_RESOURCE Staged;
	if (EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
	EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
	{
		OutResource = std::move(Staged);
		return S_OK;
	}
	const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
		Element.Material.SourceMaterial;
	Staged.GroupedConstants =
		Build_EffectGroupedTranslucentConstants(SourceMaterial);
	Staged.iSourceMaterialProfile = SourceMaterialProfileIndex(SourceMaterial);
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
	else if (13u == Staged.iSourceMaterialProfile)
	{
		Build_MissileTrailConstants(SourceMaterial, Staged.vSourceScalars0,
			Staged.vSourceScalars1, Staged.vSourceVector0,
			Staged.vSourceVector1);
	}
	for (size_t iSemantic = 0u;
		iSemantic < Staged.DynamicParameterSemantics.size(); ++iSemantic)
	{
		Staged.DynamicParameterSemantics[iSemantic] =
			DynamicParameterSemanticIndex(
				SourceMaterial.DynamicParameterSemantics[iSemantic]);
	}
	const EFFECT_RESOURCE_BINDING_DESC* pModelBinding =
		Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL);
	if (nullptr != pModelBinding)
	{
		const std::filesystem::path ModelPath = CRuntimeAssetRoot::Resolve(
			std::filesystem::path(pModelBinding->strAssetId));
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::NONANIM,
			ModelPath.string().c_str(), XMMatrixIdentity());
		if (nullptr == Model)
		{
			strOutError = "CModel load failed: " + pModelBinding->strAssetId;
			return E_FAIL;
		}
		Staged.pModel = std::move(Model);
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
		if (FAILED(Load_Texture(Binding.strAssetId, Texture)))
		{
			strOutError = "DDS load failed: " + Binding.strAssetId;
			return E_FAIL;
		}
		Staged.Textures[Texture_Index(pInput->eRuntimeSlot)] = std::move(Texture);
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
			if (iIndex < 0 || TextureDesc.strAssetId.empty())
				continue;
			ComPtr<ID3D11ShaderResourceView> Texture;
			if (FAILED(Load_SourceTexture(TextureDesc, Texture)))
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
			(1u << LOCAL_CRACK_SOURCE_TEXTURE_NAMES.size()) - 1u)
	{
		strOutError =
			"Local-crack named normal/reflection/dissolve texture stage failed: " +
			Element.strElementId;
		return E_FAIL;
	}

	if (EFFECT_ELEMENT_KIND::MESH != Element.eKind &&
		nullptr == Staged.pModel &&
		nullptr == Find_Texture(Staged.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) &&
		Element.Material.strTemplateId != EFFECT_SOURCE_MATERIAL_TEMPLATE_ID)
	{
		strOutError = "Texture/Particle/Decal/Trail requires a Base texture: " +
			Element.strElementId;
		return E_FAIL;
	}
	Staged.bSourceMaterialFallbackBlocked =
		Is_SourceMaterialFallbackBlocked(Element, Staged.GroupedConstants) ||
		(8u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << BLACKLINE_SOURCE_TEXTURE_NAMES.size()) - 1u) ||
		(9u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << LOCAL_CRACK_SOURCE_TEXTURE_NAMES.size()) - 1u) ||
		(11u == Staged.iSourceMaterialProfile &&
			Staged.iSourceTextureMask !=
				(1u << LINEARFLOW_SOURCE_TEXTURE_NAMES.size()) - 1u);
	OutResource = std::move(Staged);
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Stage_ModelCueResource(
	const EFFECT_MODEL_CUE_DESC& Cue,
	MODEL_CUE_RESOURCE& OutResource,
	std::string& strOutError) const
{
	const std::filesystem::path ModelPath = CRuntimeAssetRoot::Resolve(
		std::filesystem::path(Cue.strModelAssetId));
	const matrix_t PreTransform =
		XMMatrixScaling(Cue.vAssetPreScale.x, Cue.vAssetPreScale.y,
			Cue.vAssetPreScale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.x),
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.y),
			XMConvertToRadians(Cue.vAssetPreRotationDegrees.z));
	unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
		m_pDevice, m_pContext, MODEL::ANIM,
		ModelPath.string().c_str(), PreTransform);
	if (nullptr == Model || !Model->Set_Animation(Cue.strClipName.c_str(), false))
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
		Cue.fDurationSeconds > fDurationTicks / fTicksPerSecond + 0.001f)
	{
		strOutError = "Animated Model Cue duration exceeds its source clip: " +
			Cue.strCueId;
		return E_FAIL;
	}
	Model->Set_AnimTrackPosition(iAnimation, 0.f);
	Model->Play_Animation(0.f);
	OutResource.pModel = std::move(Model);
	OutResource.iAnimationIndex = iAnimation;
	OutResource.fTicksPerSecond = fTicksPerSecond;
	return S_OK;
}

bool_t Client::CEffectDocumentRenderer::Stage_Document(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
		return false;
	if (Resource_SignatureMatches(m_Document, Document))
	{
		m_Document = Document;
		m_strStatus = "Effect Document values committed; GPU resources reused.";
		strOutError.clear();
		return true;
	}

	std::unordered_map<std::string, ELEMENT_RESOURCE> StagedResources;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		StagedModelCueResources;
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		MODEL_CUE_RESOURCE Resource;
		if (FAILED(Stage_ModelCueResource(Cue, Resource, strOutError)))
			return false;
		StagedModelCueResources.emplace(Cue.strCueId, std::move(Resource));
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		ELEMENT_RESOURCE Resource;
		if (FAILED(Stage_ElementResource(Element, Resource, strOutError)))
			return false;
		StagedResources.emplace(Element.strElementId, std::move(Resource));
	}
	m_Document = Document;
	m_Resources = std::move(StagedResources);
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_strStatus = "Effect Document resources committed.";
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentRenderer::Clear()
{
	m_Document = {};
	m_Resources.clear();
	m_ModelCueResources.clear();
	m_strStatus = "No Effect Document staged.";
}

const Client::CEffectDocumentRenderer::ELEMENT_RESOURCE*
Client::CEffectDocumentRenderer::Find_Resource(
	const std::string& strElementId) const
{
	const auto Iterator = m_Resources.find(strElementId);
	return Iterator == m_Resources.end() ? nullptr : &Iterator->second;
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

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale) const
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
	const f32_t fAlphaScale) const
{
	if (nullptr == pShader ||
		FAILED(pShader->Bind_Matrix("g_ViewMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(pShader->Bind_Matrix("g_ProjMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::PROJ))))
	{
		return E_FAIL;
	}
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
	const f32_t fAlphaScale) const
{
	if (nullptr == pShader)
		return E_INVALIDARG;
	const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
		Element.Material.SourceMaterial;
	const bool_t bSourceSubUV = Element.SourceRecipe.bEnabled &&
		SourceMaterial.bEnabled && SourceMaterial.strSubUVMode != "none";

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
	const uint32_t iDistortionOnBase =
		Element.Detail.Color.bDistortionOnBaseMaterial ? 1u : 0u;
	if ((pShader == m_pParticleShader || pShader == m_pMeshShader) &&
		(FAILED(pShader->Bind_RawValue("g_SourceMaterialProfile",
			&Resource.iSourceMaterialProfile,
			sizeof(Resource.iSourceMaterialProfile))) ||
		FAILED(pShader->Bind_RawValue("g_SourceScalars0",
			&Resource.vSourceScalars0,
			sizeof(Resource.vSourceScalars0))) ||
		FAILED(pShader->Bind_RawValue("g_SourceScalars1",
			&Resource.vSourceScalars1,
			sizeof(Resource.vSourceScalars1))) ||
		FAILED(pShader->Bind_RawValue("g_SourceVector0",
			&Resource.vSourceVector0,
			sizeof(Resource.vSourceVector0))) ||
		FAILED(pShader->Bind_RawValue("g_SourceVector1",
			&Resource.vSourceVector1,
			sizeof(Resource.vSourceVector1))) ||
		FAILED(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))) ||
		FAILED(pShader->Bind_RawValue("g_LinearFlowParameters",
			Resource.LinearFlowParameters.data(),
			sizeof(Resource.LinearFlowParameters))) ||
		FAILED(pShader->Bind_RawValue("g_LinearFlowMaskAColor",
			&Resource.vLinearFlowMaskAColor,
			sizeof(Resource.vLinearFlowMaskAColor))) ||
		FAILED(pShader->Bind_RawValue("g_LinearFlowMaskBColor",
			&Resource.vLinearFlowMaskBColor,
			sizeof(Resource.vLinearFlowMaskBColor))) ||
		FAILED(pShader->Bind_RawValue("g_BlacklineParameters",
			Resource.BlacklineParameters.data(),
			sizeof(Resource.BlacklineParameters))) ||
		FAILED(pShader->Bind_RawValue("g_BlacklineDiffuseColor",
			&Resource.vBlacklineDiffuseColor,
			sizeof(Resource.vBlacklineDiffuseColor))) ||
		FAILED(pShader->Bind_RawValue("g_BlacklineMaskColor",
			&Resource.vBlacklineMaskColor,
			sizeof(Resource.vBlacklineMaskColor))) ||
		FAILED(pShader->Bind_RawValue("g_LocalCrackParameters",
			Resource.LocalCrackParameters.data(),
			sizeof(Resource.LocalCrackParameters))) ||
		FAILED(pShader->Bind_RawValue("g_LocalCrackOutColor",
			&Resource.vLocalCrackOutColor,
			sizeof(Resource.vLocalCrackOutColor))) ||
		FAILED(pShader->Bind_RawValue("g_LocalCrackInColor",
			&Resource.vLocalCrackInColor,
			sizeof(Resource.vLocalCrackInColor))) ||
		FAILED(pShader->Bind_RawValue("g_LocalCrackReflectionColor",
			&Resource.vLocalCrackReflectionColor,
			sizeof(Resource.vLocalCrackReflectionColor))) ||
		FAILED(pShader->Bind_RawValue("g_SourceTextureClampUMask",
			&Resource.iSourceTextureClampUMask,
			sizeof(Resource.iSourceTextureClampUMask))) ||
		FAILED(pShader->Bind_RawValue("g_SourceTextureClampVMask",
			&Resource.iSourceTextureClampVMask,
			sizeof(Resource.iSourceTextureClampVMask))) ||
		FAILED(pShader->Bind_RawValue("g_GroupedUVScalePan",
			&Resource.GroupedConstants.vUVScalePan,
			sizeof(Resource.GroupedConstants.vUVScalePan))) ||
		FAILED(pShader->Bind_RawValue("g_GroupedAlphaEmissive",
			&Resource.GroupedConstants.vAlphaEmissive,
			sizeof(Resource.GroupedConstants.vAlphaEmissive))) ||
		FAILED(pShader->Bind_RawValue("g_GroupedNoiseDissolve",
			&Resource.GroupedConstants.vNoiseDissolve,
			sizeof(Resource.GroupedConstants.vNoiseDissolve))) ||
		FAILED(pShader->Bind_RawValue("g_GroupedTint",
			&Resource.GroupedConstants.vTint,
			sizeof(Resource.GroupedConstants.vTint))) ||
		FAILED(pShader->Bind_RawValue("g_GroupedMaterialFlags",
			&Resource.GroupedConstants.iFlags,
			sizeof(Resource.GroupedConstants.iFlags))) ||
		FAILED(pShader->Bind_RawValue("g_EffectLocalTime",
			&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		FAILED(pShader->Bind_RawValue("g_DynamicParameterSemantics",
			Resource.DynamicParameterSemantics.data(),
			sizeof(Resource.DynamicParameterSemantics)))))
	{
		return E_FAIL;
	}

	return FAILED(pShader->Bind_RawValue("g_UVScale", &UVScale, sizeof(UVScale))) ||
		FAILED(pShader->Bind_RawValue("g_UVOffset", &UVOffset, sizeof(UVOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset", &Color.vColorOffset, sizeof(Color.vColorOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMultiply", &ColorMultiply, sizeof(ColorMultiply))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClip", &Color.fColorClip, sizeof(Color.fColorClip))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity", &Color.fEmissiveIntensity, sizeof(Color.fEmissiveIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_DistortionIntensity", &Color.fDistortionIntensity, sizeof(Color.fDistortionIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_DistortionOnBaseMaterial", &iDistortionOnBase, sizeof(iDistortionOnBase))) ||
		FAILED(pShader->Bind_RawValue("g_RadialTime", &Color.fRadialTime, sizeof(Color.fRadialTime))) ||
		FAILED(pShader->Bind_RawValue("g_RadialIntensity", &Color.fRadialIntensity, sizeof(Color.fRadialIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &Dissolve, sizeof(Dissolve))) ||
		FAILED(pShader->Bind_RawValue("g_HasNoise", &iHasNoise, sizeof(iHasNoise))) ||
		FAILED(pShader->Bind_RawValue("g_HasMask", &iHasMask, sizeof(iHasMask))) ||
		FAILED(pShader->Bind_RawValue("g_HasEmissive", &iHasEmissive, sizeof(iHasEmissive))) ||
		FAILED(pShader->Bind_RawValue("g_HasDissolve", &iHasDissolve, sizeof(iHasDissolve))) ||
		FAILED(pShader->Bind_Texture("g_BaseTexture",
			nullptr != Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) ?
				Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) :
				m_pWhiteTexture)) ||
		FAILED(pShader->Bind_Texture("g_NoiseTexture", iHasNoise ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_MaskTexture", iHasMask ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) : m_pWhiteTexture)) ||
		FAILED(pShader->Bind_Texture("g_EmissiveTexture", iHasEmissive ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_DissolveTexture", iHasDissolve ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture0",
			Resource.SourceTextures[0] ? Resource.SourceTextures[0] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture1",
			Resource.SourceTextures[1] ? Resource.SourceTextures[1] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture2",
			Resource.SourceTextures[2] ? Resource.SourceTextures[2] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture3",
			Resource.SourceTextures[3] ? Resource.SourceTextures[3] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture4",
			Resource.SourceTextures[4] ? Resource.SourceTextures[4] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture5",
			Resource.SourceTextures[5] ? Resource.SourceTextures[5] : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_SourceTexture6",
			Resource.SourceTextures[6] ? Resource.SourceTextures[6] : m_pBlackTexture)) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Mesh(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride,
	const float4_t* pDynamicParameter)
{
	if (nullptr == Resource.pModel || nullptr == Element.pElement)
		return E_FAIL;
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return E_INVALIDARG;
	const float4x4_t& World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	const float4_t DynamicParameter = nullptr == pDynamicParameter ?
		float4_t{} : *pDynamicParameter;
	const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
	if (FAILED(m_pMeshShader->Bind_Matrix("g_WorldMatrix", &World)) ||
		FAILED(m_pMeshShader->Bind_RawValue("g_CameraPosition",
			&CameraPosition, sizeof(CameraPosition))) ||
		FAILED(m_pMeshShader->Bind_RawValue("g_EffectDynamicParameter",
			&DynamicParameter, sizeof(DynamicParameter))) ||
		FAILED(Bind_Common(m_pMeshShader, Element, Resource, fAlphaScale)))
	{
		return E_FAIL;
	}

	const ComPtr<ID3D11ShaderResourceView> BaseOverride =
		Element.pElement->Detail.Mesh.bUseModelMaterial ? nullptr :
		Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	const uint32_t iUseBaseOverride = nullptr != BaseOverride ? 1u : 0u;
	if (FAILED(m_pMeshShader->Bind_RawValue(
		"g_UseBaseOverride", &iUseBaseOverride, sizeof(iUseBaseOverride))))
	{
		return E_FAIL;
	}
	for (uint32_t iMesh = 0u; iMesh < Resource.pModel->Get_NumMeshes(); ++iMesh)
	{
		if ((iUseBaseOverride && FAILED(m_pMeshShader->Bind_Texture(
			"g_BaseTexture", BaseOverride))) ||
			(!iUseBaseOverride && FAILED(Resource.pModel->Bind_Material(
				m_pMeshShader, "g_BaseTexture", iMesh, aiTextureType_DIFFUSE))) ||
			FAILED(m_pMeshShader->Begin(iPass)) ||
			FAILED(Resource.pModel->Render(iMesh)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Rect(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride)
{
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	float4x4_t World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	if (Element.pElement->Detail.Sprite.bBillboard)
		World = Make_BillboardWorld(World);
	return UINT32_MAX == iPass ||
		FAILED(m_pRectShader->Bind_Matrix("g_WorldMatrix", &World)) ||
		FAILED(Bind_Common(m_pRectShader, Element, Resource, fAlphaScale)) ||
		FAILED(m_pRectShader->Begin(iPass)) ||
		FAILED(m_pRect->Render()) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Decal(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	float4x4_t InverseDecal{};
	const matrix_t Inverse = XMMatrixInverse(
		nullptr, XMLoadFloat4x4(&Element.World));
	XMStoreFloat4x4(&InverseDecal, Inverse);
	const float2_t Size = Element.pElement->Detail.Decal.vSize;
	const f32_t Depth = Element.pElement->Detail.Decal.fDepth;
	return UINT32_MAX == iPass ||
		FAILED(Bind_MaterialInputs(m_pDecalShader,
			*Element.pElement, Element.Color,
			Element.fLocalTimeSeconds, Element.fNormalizedLife, Resource)) ||
		FAILED(m_pDecalShader->Bind_Matrix("g_DecalWorldInverse", &InverseDecal)) ||
		FAILED(m_pDecalShader->Bind_Matrix("g_ViewMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pDecalShader->Bind_Matrix("g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pDecalShader->Bind_RawValue("g_DecalSize", &Size, sizeof(Size))) ||
		FAILED(m_pDecalShader->Bind_RawValue("g_DecalDepth", &Depth, sizeof(Depth))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pDecalShader, "g_DepthTexture")) ||
		FAILED(m_pDecalShader->Begin(iPass)) ||
		FAILED(m_pRect->Render()) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Element(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement)
		return E_INVALIDARG;
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
		return E_INVALIDARG;
	}
}

HRESULT Client::CEffectDocumentRenderer::Render_AfterImages(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_AFTERIMAGE> AfterImages)
{
	for (const EFFECT_EVALUATED_AFTERIMAGE& AfterImage : AfterImages)
	{
		if (nullptr == AfterImage.pElement)
			continue;
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(AfterImage.pElement->strElementId);
		if (nullptr == pResource)
			return E_FAIL;
		if (pResource->bSourceMaterialFallbackBlocked)
			continue;
		EFFECT_EVALUATED_ELEMENT Element;
		Element.pElement = AfterImage.pElement;
		Element.World = AfterImage.World;
		Element.Color = AfterImage.pElement->Detail.Color;
		Element.fLocalTimeSeconds = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			AfterImage.pElement->Detail.Timing.fStartDelaySeconds);
		Element.fNormalizedLife = 1.f - AfterImage.fAlpha;
		const HRESULT Result = EFFECT_ELEMENT_KIND::MESH == Element.pElement->eKind ?
			Render_Mesh(Element, *pResource, AfterImage.fAlpha, &AfterImage.World) :
			Render_Rect(Element, *pResource, AfterImage.fAlpha, &AfterImage.World);
		if (FAILED(Result))
			return Result;
	}
	return S_OK;
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
		return S_OK;
	const ELEMENT_RESOURCE* pResource = Find_Resource(pSource->strElementId);
	if (nullptr == pResource)
		return E_FAIL;
	if (pResource->bSourceMaterialFallbackBlocked)
		return S_FALSE;
	if (pSource->Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
		!pSource->Material.SourceMaterial.bEnabled)
	{
		// Version 10 and older source-material documents are intentionally
		// fail-closed.  They remain loadable for migration, but must not turn
		// into a white fallback particle after the v11 renderer is enabled.
		return S_FALSE;
	}
	if (nullptr != pResource->pModel)
	{
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
			if (FAILED(Render_Mesh(MeshParticle, *pResource, 1.f, nullptr,
				&Particle.vDynamicParameter)))
				return E_FAIL;
		}
		return S_OK;
	}
	std::vector<Engine::VTXEFFECT_PARTICLE> Instances;
	Instances.reserve(Particles.size());
	for (const EFFECT_EVALUATED_PARTICLE& Particle : Particles)
	{
		if (nullptr == Particle.pElement)
			continue;
		pSource = Particle.pElement;
		const float4x4_t World = Particle.pElement->Detail.Particle.bBillboard ?
			Make_ParticleSpriteWorld(Particle) : Particle.World;
		const EFFECT_SUBUV_FRAME_DESC SubUV = Resolve_SubUVFrames(Particle);
		Instances.push_back({ World, Particle.Color,
			Particle.vDynamicParameter,
			SubUV.Current,
			SubUV.Next,
			{ Particle.fNormalizedLife, SubUV.fBlend } });
	}
	if (nullptr == pSource || Instances.empty())
		return S_OK;

	const EFFECT_ELEMENT_DESC& Source = *pSource;
	if (FAILED(m_pParticleBuffer->Update_Instances(
			std::span<const Engine::VTXEFFECT_PARTICLE>(
				Instances.data(), Instances.size()))))
	{
		return E_FAIL;
	}
	const f32_t LocalTime = (std::max)(0.f,
		Frame.fSampleTimeSeconds - Source.Detail.Timing.fStartDelaySeconds);
	const f32_t Normalized = std::clamp(
		LocalTime / Source.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
	EFFECT_COLOR_DESC CommonColor =
		Evaluate_CommonColor(Source, Normalized);
	CommonColor.vColorMultiply = float4_t(1.f, 1.f, 1.f, 1.f);
	if (FAILED(Bind_Common(m_pParticleShader, Source, CommonColor,
		LocalTime, Normalized, *pResource)) ||
		FAILED(m_pParticleShader->Begin(
			Select_Pass(Source.Material.eRenderProfile))) ||
		FAILED(m_pParticleBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Trails(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_TRAIL> Trails)
{
	const vector_t CameraPosition = XMLoadFloat4(
		CGameInstance::Get().Get_CamPosition());
	for (const EFFECT_EVALUATED_TRAIL& Trail : Trails)
	{
		if (nullptr == Trail.pElement || Trail.Points.size() < 2u)
			continue;
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(Trail.pElement->strElementId);
		if (nullptr == pResource)
			return E_FAIL;
		if (pResource->bSourceMaterialFallbackBlocked)
			continue;

		std::vector<Engine::VTXEFFECT_TRAIL> Vertices;
		std::vector<uint32_t> Indices;
		Vertices.reserve(Trail.Points.size() * 2u);
		Indices.reserve((Trail.Points.size() - 1u) * 6u);
		for (size_t iPoint = 0u; iPoint < Trail.Points.size(); ++iPoint)
		{
			const vector_t Position = XMLoadFloat3(
				&Trail.Points[iPoint].vWorldPosition);
			const vector_t Previous = XMLoadFloat3(&Trail.Points[
				iPoint > 0u ? iPoint - 1u : iPoint].vWorldPosition);
			const vector_t Next = XMLoadFloat3(&Trail.Points[
				iPoint + 1u < Trail.Points.size() ? iPoint + 1u : iPoint].vWorldPosition);
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
			const f32_t Age = Trail.Points[iPoint].fNormalizedAge;
			const f32_t Width = Trail.pElement->Detail.Trail.fStartWidth +
				(Trail.pElement->Detail.Trail.fEndWidth -
					Trail.pElement->Detail.Trail.fStartWidth) * Age;
			const vector_t HalfSide = Side * (Width * 0.5f);
			const float4_t Color(1.f, 1.f, 1.f, 1.f - Age);
			Vertices.push_back({ To_Float3(Position - HalfSide),
				float2_t(static_cast<f32_t>(iPoint), 0.f), Color });
			Vertices.push_back({ To_Float3(Position + HalfSide),
				float2_t(static_cast<f32_t>(iPoint), 1.f), Color });
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
		if (FAILED(m_pTrailBuffer->Update_Geometry(
			std::span<const Engine::VTXEFFECT_TRAIL>(
				Vertices.data(), Vertices.size()),
			std::span<const uint32_t>(Indices.data(), Indices.size()))))
			return E_FAIL;
		const f32_t LocalTime = (std::max)(0.f,
			Frame.fSampleTimeSeconds -
			Trail.pElement->Detail.Timing.fStartDelaySeconds);
		const f32_t Normalized = std::clamp(LocalTime /
			Trail.pElement->Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
		float4x4_t Identity{};
		XMStoreFloat4x4(&Identity, XMMatrixIdentity());
		const EFFECT_COLOR_DESC CommonColor =
			Evaluate_CommonColor(*Trail.pElement, Normalized);
		if (FAILED(m_pTrailShader->Bind_Matrix("g_WorldMatrix", &Identity)) ||
			FAILED(Bind_Common(m_pTrailShader, *Trail.pElement,
				CommonColor, LocalTime, Normalized, *pResource)) ||
			FAILED(m_pTrailShader->Begin(
				Select_Pass(Trail.pElement->Material.eRenderProfile))) ||
			FAILED(m_pTrailBuffer->Render()))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_ModelCues(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	for (const EFFECT_MODEL_CUE_DESC& Cue : m_Document.ModelCues)
	{
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
			return E_FAIL;
		}
		Engine::CModel& Model = *Resource->second.pModel;
		if (!Model.Set_AnimTrackPosition(Resource->second.iAnimationIndex,
			fLocalTime * Resource->second.fTicksPerSecond))
		{
			return E_FAIL;
		}
		Model.Play_Animation(0.f);
		const EFFECT_TRANSFORM_DESC& Transform = Cue.LocalTransform;
		const matrix_t Local =
			XMMatrixScaling(Transform.vScale.x, Transform.vScale.y,
				Transform.vScale.z) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Transform.vRotationDegrees.x),
				XMConvertToRadians(Transform.vRotationDegrees.y),
				XMConvertToRadians(Transform.vRotationDegrees.z)) *
			XMMatrixTranslation(Transform.vPosition.x,
				Transform.vPosition.y, Transform.vPosition.z);
		float4x4_t World{};
		XMStoreFloat4x4(&World, Local * XMLoadFloat4x4(&Frame.RootWorld));
		if (FAILED(m_pAnimatedModelShader->Bind_Matrix(
			"g_WorldMatrix", &World)) ||
			FAILED(CGameInstance::Get().Bind_Transform(
				m_pAnimatedModelShader, "g_ViewMatrix", D3DTS::VIEW)) ||
			FAILED(CGameInstance::Get().Bind_Transform(
				m_pAnimatedModelShader, "g_ProjMatrix", D3DTS::PROJ)))
		{
			return E_FAIL;
		}
		for (uint32_t iMesh = 0u; iMesh < Model.Get_NumMeshes(); ++iMesh)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					Model, m_pAnimatedModelShader, iMesh)) ||
				FAILED(Model.Bind_BoneMatrices(m_pAnimatedModelShader,
					"g_BoneMatrices", iMesh)) ||
				FAILED(m_pAnimatedModelShader->Begin(0u)) ||
				FAILED(Model.Render(iMesh)))
			{
				return E_FAIL;
			}
		}
	}
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	if (FAILED(Render_ModelCues(Frame)))
		return E_FAIL;
	size_t iElement = 0u;
	size_t iParticle = 0u;
	size_t iTrail = 0u;
	size_t iAfterImage = 0u;
	for (const EFFECT_ELEMENT_DESC& DocumentElement : m_Document.Elements)
	{
		const std::string& strElementId = DocumentElement.strElementId;
		const size_t iAfterImageBegin = iAfterImage;
		while (iAfterImage < Frame.AfterImages.size() &&
			nullptr != Frame.AfterImages[iAfterImage].pElement &&
			Frame.AfterImages[iAfterImage].pElement->strElementId == strElementId)
		{
			++iAfterImage;
		}
		if (FAILED(Render_AfterImages(Frame,
			std::span<const EFFECT_EVALUATED_AFTERIMAGE>(Frame.AfterImages)
				.subspan(iAfterImageBegin,
					iAfterImage - iAfterImageBegin))))
			return E_FAIL;

		while (iElement < Frame.Elements.size() &&
			nullptr != Frame.Elements[iElement].pElement &&
			Frame.Elements[iElement].pElement->strElementId == strElementId)
		{
			const ELEMENT_RESOURCE* pResource = Find_Resource(strElementId);
			if (nullptr == pResource)
				return E_FAIL;
			if (pResource->bSourceMaterialFallbackBlocked)
			{
				++iElement;
				continue;
			}
			if (FAILED(Render_Element(Frame.Elements[iElement], *pResource)))
			{
				return E_FAIL;
			}
			++iElement;
		}

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
		if (FAILED(Render_Particles(Frame,
			std::span<const EFFECT_EVALUATED_PARTICLE>(Frame.Particles)
				.subspan(iParticleBegin,
					iParticle - iParticleBegin))) ||
			FAILED(Render_Trails(Frame,
				std::span<const EFFECT_EVALUATED_TRAIL>(Frame.Trails)
					.subspan(iTrailBegin, iTrail - iTrailBegin))))
		{
			return E_FAIL;
		}
	}
	m_strStatus = "Effect frame rendered.";
	return S_OK;
}
