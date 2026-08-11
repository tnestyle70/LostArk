#include "Effect_DocumentRenderer.h"

#include "DeferredMaterialRenderUtils.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

#include <d3d11sdklayers.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <span>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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
	uint64_t iResourceSignature = 0u;
	std::string strEffectAssetId;
	const EFFECT_DOCUMENT_DESC* pCatalogDocumentIdentity = nullptr;
	EFFECT_DOCUMENT_DESC ResourceDocument;
	std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
		pPlaybackResources;
	std::unordered_map<std::string, ELEMENT_RESOURCE> ElementResources;
	std::unordered_map<std::string, MODEL_CUE_RESOURCE>
		ModelCuePrototypes;
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
		ComPtr<ID3D11ShaderResourceView> pWhiteTexture;
		ComPtr<ID3D11ShaderResourceView> pBlackTexture;
	};

	struct PREPARED_KEY final
	{
		uint64_t iCatalogRevision = 0u;
		std::string strEffectAssetId;
		uint64_t iResourceSignature = 0u;

		bool operator<(const PREPARED_KEY& Right) const
		{
			return std::tie(iCatalogRevision, strEffectAssetId,
				iResourceSignature) <
				std::tie(Right.iCatalogRevision, Right.strEffectAssetId,
					Right.iResourceSignature);
		}
	};

	std::mutex g_EffectRenderCacheMutex;
	std::unordered_map<ID3D11Device*, std::shared_ptr<EFFECT_RENDERER_CORE>>
		g_EffectRendererCores;
	std::map<PREPARED_KEY,
		std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>>
		g_PreparedEffectDocuments;
	ID3D11Device* g_pPreparedDevice = nullptr;
	uint64_t g_iPreparedCatalogRevision = 0u;
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
		const std::string Canonical =
			Client::CEffectDocumentCodec::Serialize(Document);
		uint64_t Hash = 1469598103934665603ull;
		for (const unsigned char Byte : Canonical)
		{
			Hash ^= static_cast<uint64_t>(Byte);
			Hash *= 1099511628211ull;
		}
		return Hash;
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
		if (nullptr == Core->pMeshShader ||
			nullptr == Core->pAnimatedModelShader ||
			nullptr == Core->pRectShader ||
			nullptr == Core->pParticleShader ||
			nullptr == Core->pTrailShader ||
			nullptr == Core->pDecalShader || nullptr == Core->pRect ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xffffffffu, Core->pWhiteTexture)) ||
			FAILED(Create_SolidTexture(
				pDevice.Get(), 0xff000000u, Core->pBlackTexture)))
		{
			return nullptr;
		}
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
	PREWARM_ASSET_CACHE* pSharedAssets) const
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
		if (nullptr != pSharedAssets)
		{
			const auto Cached = pSharedAssets->NonAnimatedModels.find(
				pModelBinding->strAssetId);
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
				ModelPath.string().c_str(), XMMatrixIdentity());
			if (nullptr == Model)
			{
				strOutError = "CModel load failed: " +
					pModelBinding->strAssetId;
				return E_FAIL;
			}
			Staged.pModel = std::move(Model);
			if (nullptr != pSharedAssets)
				pSharedAssets->NonAnimatedModels.emplace(
					pModelBinding->strAssetId, Staged.pModel);
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
		Cue.fDurationSeconds > fDurationTicks / fTicksPerSecond + 0.001f)
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
	return S_OK;
}

bool_t Client::CEffectDocumentRenderer::Build_PreparedDocument(
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	const EFFECT_DOCUMENT_DESC& Document,
	PREWARM_ASSET_CACHE* pSharedAssets,
	std::shared_ptr<const PREPARED_DOCUMENT>& OutPrepared,
	std::string& strOutError) const
{
	if (strEffectAssetId.empty() ||
		strEffectAssetId != Document.strEffectAssetId ||
		!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
	{
		if (strOutError.empty())
			strOutError = "Prepared Effect identity is invalid.";
		return false;
	}
	auto Staged = std::make_shared<PREPARED_DOCUMENT>();
	Staged->iCatalogRevision = iCatalogRevision;
	Staged->iResourceSignature = Build_ResourceSignature(Document);
	Staged->strEffectAssetId = strEffectAssetId;
	Staged->pCatalogDocumentIdentity =
		0u == iCatalogRevision ? nullptr : &Document;
	Staged->ResourceDocument = Document;
	if (!CEffectPlayback::Prepare_DocumentResources(
		Document, Staged->pPlaybackResources, strOutError))
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
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		ELEMENT_RESOURCE Resource;
		if (FAILED(Stage_ElementResource(
			Element, Resource, strOutError, pSharedAssets)) ||
			!Staged->ElementResources.emplace(
				Element.strElementId, std::move(Resource)).second)
		{
			if (strOutError.empty())
				strOutError = "Prepared Effect has a duplicate Element.";
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
			Prototype.iAnimationIndex, 0.f) || !Model->Play_Animation(0.f))
		{
			strOutError = "Prepared animated Model Cue reset failed: " + CueId;
			return false;
		}
		MODEL_CUE_RESOURCE Resource;
		Resource.pModel = Model;
		Resource.iAnimationIndex = Prototype.iAnimationIndex;
		Resource.fTicksPerSecond = Prototype.fTicksPerSecond;
		Staged.emplace(CueId, std::move(Resource));
	}
	OutResources = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentRenderer::Ensure_MutableInstanceBuffers(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	const bool_t bNeedsParticleBuffer = std::any_of(
		Document.Elements.begin(), Document.Elements.end(),
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
		});
	const bool_t bNeedsTrailBuffer = std::any_of(
		Document.Elements.begin(), Document.Elements.end(),
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return EFFECT_ELEMENT_KIND::TRAIL == Element.eKind;
		});
	unique_ptr<Engine::CVIBuffer_ParticleRect> ParticleBuffer;
	unique_ptr<Engine::CVIBuffer_DynamicTrail> TrailBuffer;
	if (bNeedsParticleBuffer && nullptr == m_pParticleBuffer)
	{
		ParticleBuffer = Engine::CVIBuffer_ParticleRect::Create(
			m_pDevice, m_pContext, 2048u);
		if (nullptr == ParticleBuffer)
		{
			strOutError = "Per-instance Effect particle buffer creation failed.";
			return false;
		}
	}
	if (bNeedsTrailBuffer && nullptr == m_pTrailBuffer)
	{
		TrailBuffer = Engine::CVIBuffer_DynamicTrail::Create(
			m_pDevice, m_pContext, 256u);
		if (nullptr == TrailBuffer)
		{
			strOutError = "Per-instance Effect trail buffer creation failed.";
			return false;
		}
	}
	if (nullptr != ParticleBuffer)
		m_pParticleBuffer = std::move(ParticleBuffer);
	if (nullptr != TrailBuffer)
		m_pTrailBuffer = std::move(TrailBuffer);
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
	if (nullptr == pDevice || nullptr == pContext || 0u == iCatalogRevision ||
		Documents.empty() ||
		nullptr == Acquire_RendererCore(pDevice, pContext))
	{
		strOutError = "Effect product prewarm arguments or renderer core are invalid.";
		return false;
	}

	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Existing;
	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		if (g_pPreparedDevice == pDevice.Get() &&
			g_iPreparedCatalogRevision == iCatalogRevision)
		{
			Existing = g_PreparedEffectDocuments;
		}
	}
	std::map<PREPARED_KEY, std::shared_ptr<const PREPARED_DOCUMENT>> Staged;
	PREWARM_ASSET_CACHE SharedAssets;
	std::unordered_set<std::string> EffectIds;
	CEffectDocumentRenderer Loader(pDevice, pContext);
	for (const auto& [EffectId, Document] : Documents)
	{
		if (EffectId.empty() || nullptr == Document ||
			EffectId != Document->strEffectAssetId ||
			!EffectIds.insert(EffectId).second)
		{
			strOutError = "Effect product prewarm contains an invalid or duplicate target.";
			return false;
		}
		const PREPARED_KEY Key{
			iCatalogRevision, EffectId, Build_ResourceSignature(*Document) };
		const auto Reusable = Existing.find(Key);
		if (Reusable != Existing.end() && nullptr != Reusable->second &&
			Resource_SignatureMatches(
				Reusable->second->ResourceDocument, *Document))
		{
			Staged.emplace(Key, Reusable->second);
			continue;
		}
		std::shared_ptr<const PREPARED_DOCUMENT> Prepared;
		if (!Loader.Build_PreparedDocument(
			iCatalogRevision, EffectId, *Document, &SharedAssets,
			Prepared, strOutError))
		{
			return false;
		}
		Staged.emplace(Key, std::move(Prepared));
	}

	{
		const std::scoped_lock Lock(g_EffectRenderCacheMutex);
		g_PreparedEffectDocuments = std::move(Staged);
		g_pPreparedDevice = pDevice.Get();
		g_iPreparedCatalogRevision = iCatalogRevision;
		++g_EffectRenderPrewarmProbe.iCatalogCommitCount;
		g_EffectRenderPrewarmProbe.iCatalogRevision = iCatalogRevision;
		g_EffectRenderPrewarmProbe.iPreparedDocumentCount =
			static_cast<uint32_t>(g_PreparedEffectDocuments.size());
	}
	strOutError = "Prepared " + std::to_string(Documents.size()) +
		" admitted animation Effect targets for catalog revision " +
		std::to_string(iCatalogRevision) + ".";
	return true;
}

std::shared_ptr<const Client::CEffectDocumentRenderer::PREPARED_DOCUMENT>
Client::CEffectDocumentRenderer::Find_Prepared(
	const uint64_t iCatalogRevision,
	const std::string& strEffectAssetId,
	const EFFECT_DOCUMENT_DESC& Document)
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	const PREPARED_KEY LowerBound{
		iCatalogRevision, strEffectAssetId, 0u };
	const auto Iterator = g_PreparedEffectDocuments.lower_bound(LowerBound);
	if (g_iPreparedCatalogRevision != iCatalogRevision ||
		Iterator == g_PreparedEffectDocuments.end() ||
		Iterator->first.iCatalogRevision != iCatalogRevision ||
		Iterator->first.strEffectAssetId != strEffectAssetId ||
		nullptr == Iterator->second ||
		Iterator->second->pCatalogDocumentIdentity != &Document)
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

void Client::CEffectDocumentRenderer::Clear_Prepared_Catalog()
{
	const std::scoped_lock Lock(g_EffectRenderCacheMutex);
	g_PreparedEffectDocuments.clear();
	g_EffectRendererCores.clear();
	g_pPreparedDevice = nullptr;
	g_iPreparedCatalogRevision = 0u;
	g_EffectRenderPrewarmProbe.iCatalogRevision = 0u;
	g_EffectRenderPrewarmProbe.iPreparedDocumentCount = 0u;
}

bool_t Client::CEffectDocumentRenderer::Stage_Prepared(
	const EFFECT_DOCUMENT_DESC& Document,
	std::shared_ptr<const PREPARED_DOCUMENT> pPrepared,
	std::string& strOutError)
{
	const bool_t bCatalogPrepared = nullptr != pPrepared &&
		0u != pPrepared->iCatalogRevision;
	const bool_t bIdentityMatches = bCatalogPrepared ?
		pPrepared->pCatalogDocumentIdentity == &Document :
		(nullptr != pPrepared &&
			pPrepared->iResourceSignature == Build_ResourceSignature(Document) &&
			Resource_SignatureMatches(
				pPrepared->ResourceDocument, Document));
	if (nullptr == pPrepared || !bIdentityMatches ||
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
	if (!Ensure_MutableInstanceBuffers(Document, strOutError))
		return false;
	if (!Clone_ModelCueResources(
		*pPrepared, StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_strStatus = "Prepared Effect Document resources attached.";
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
		Resource_SignatureMatches(m_Document, Document))
	{
		m_Document = Document;
		m_pReconstructedDiagnostic.reset();
		m_ReconstructedRuntimeBoundary.Clear();
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
	return Stage_Prepared(Document, std::move(Prepared), strOutError);
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
	m_strStatus =
		"Reconstructed Effect program prepared; renderer execution remains blocked.";
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
			if (FAILED(m_pDevice->CreateSamplerState(
				&BindingIterator->second.SamplerDescriptor, &Sampler)))
			{
				strOutError =
					"Reconstructed diagnostic sampler-state creation failed.";
				return false;
			}
			D3D11_SAMPLER_DESC SamplerDescriptor{};
			Sampler->GetDesc(&SamplerDescriptor);
			if (!Same_SamplerDescriptor(SamplerDescriptor,
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
	m_strStatus =
		"Reconstructed diagnostic GPU resources committed; waiting for draw.";
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentRenderer::Clear()
{
	m_Document = {};
	m_pPreparedDocument.reset();
	m_pReconstructedDiagnostic.reset();
	m_ReconstructedRuntimeBoundary.Clear();
	m_ModelCueResources.clear();
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
	const uint32_t iReconstructedMaterialEvaluatorDisabled = 0u;
	if (FAILED(pShader->Bind_RawValue(
		"g_ReconstructedMaterialEvaluatorEnabled",
		&iReconstructedMaterialEvaluatorDisabled,
		sizeof(iReconstructedMaterialEvaluatorDisabled))))
	{
		return E_FAIL;
	}
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
		World = Make_BillboardWorld(World,
			Element.pElement->Detail.Sprite.fBillboardRollDegrees);
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

HRESULT Client::CEffectDocumentRenderer::Render_ReconstructedDiagnostic(
	const float4x4_t& RootWorld,
	const RECONSTRUCTED_DIAGNOSTIC_SOLO eSolo)
{
	if (nullptr == m_pReconstructedDiagnostic ||
		nullptr == m_pReconstructedDiagnostic->pFrame ||
		eSolo >= RECONSTRUCTED_DIAGNOSTIC_SOLO::END)
	{
		m_strStatus = "Reconstructed diagnostic draw was not staged.";
		return E_FAIL;
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
	if (PacketIterator == Packets.end() ||
		PacketIterator->Get_SelectionIndex() >=
			m_pReconstructedDiagnostic->Resources.size())
	{
		m_strStatus = "Reconstructed diagnostic Solo packet is unavailable.";
		return E_FAIL;
	}
	const EFFECT_RECONSTRUCTED_SELECTED_PACKET& Packet = *PacketIterator;
	const uint32_t iSelection = Packet.Get_SelectionIndex();
	const EFFECT_RECONSTRUCTED_SELECTED_EMITTER_SELECTION& Selection =
		Packet.Get_Preparation()->Get_Request().Emitters[iSelection];
	RECONSTRUCTED_DIAGNOSTIC_COMPOSITE::GPU_RESOURCE& Resource =
		m_pReconstructedDiagnostic->Resources[iSelection];
	const shared_ptr<Engine::CShader> Shader =
		eRequiredKind == EFFECT_RECONSTRUCTED_SELECTED_PACKET_KIND::MESH ?
			m_pMeshShader : m_pParticleShader;
	if (nullptr == Shader || nullptr == Resource.Textures[0u] ||
		nullptr == Resource.Textures[1u] || nullptr == Resource.Samplers[0u])
	{
		m_strStatus = "Reconstructed diagnostic GPU composite is incomplete.";
		return E_FAIL;
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
				m_strStatus =
					"Reconstructed diagnostic draw produced no shader invocations.";
				return E_FAIL;
			}
		}
		else if (FAILED(StatisticsResult))
		{
			m_strStatus =
				"Reconstructed diagnostic pipeline-statistics readback failed.";
			return E_FAIL;
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
	if (FAILED(Shader->Bind_Matrix("g_ViewMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(Shader->Bind_Matrix("g_ProjMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(Shader->Bind_RawValue("g_UVScale", &LegacyUvScale,
			sizeof(LegacyUvScale))) ||
		FAILED(Shader->Bind_RawValue("g_UVOffset", &LegacyUvOffset,
			sizeof(LegacyUvOffset))) ||
		FAILED(Shader->Bind_RawValue("g_EffectLocalTime", &fSampleTime,
			sizeof(fSampleTime))) ||
		FAILED(Shader->Bind_RawValue("g_SourceMaterialProfile",
			&iSourceMaterialProfile, sizeof(iSourceMaterialProfile))) ||
		FAILED(Shader->Bind_RawValue("g_SourceTextureClampUMask",
			&iSourceTextureClampMask, sizeof(iSourceTextureClampMask))) ||
		FAILED(Shader->Bind_RawValue("g_SourceTextureClampVMask",
			&iSourceTextureClampMask, sizeof(iSourceTextureClampMask))) ||
		FAILED(Shader->Bind_RawValue(
			Material.Shader.strFeatureMaskVariable.c_str(),
			&Material.iFeatureMask, sizeof(Material.iFeatureMask))) ||
		FAILED(Shader->Bind_RawValue(Material.Shader.strUvScaleVariable.c_str(),
			&UvScale, sizeof(UvScale))) ||
		FAILED(Shader->Bind_RawValue(
			Material.Shader.strPanRotationAuxVariable.c_str(),
			&PanRotationAux, sizeof(PanRotationAux))) ||
		FAILED(Shader->Bind_RawValue(Material.Shader.strColorVariable.c_str(),
			&MaterialColor, sizeof(MaterialColor))) ||
		FAILED(Shader->Bind_RawValue(Material.Shader.strParams0Variable.c_str(),
			&Params0, sizeof(Params0))) ||
		FAILED(Shader->Bind_RawValue(Material.Shader.strParams1Variable.c_str(),
			&Params1, sizeof(Params1))) ||
		FAILED(Shader->Bind_Texture(
			Material.TextureLanes[0u].strShaderVariableName.c_str(),
			Resource.Textures[0u])) ||
		FAILED(Shader->Bind_Texture(
			Material.TextureLanes[1u].strShaderVariableName.c_str(),
			Resource.Textures[1u])) ||
		FAILED(Shader->Bind_RawValue(
			Material.Shader.strEvaluatorEnabledVariable.c_str(),
			&iEnabled, sizeof(iEnabled))))
	{
		DrawResult = E_FAIL;
	}
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
		if (nullptr == Resource.pModel ||
			FAILED(Shader->Bind_Matrix("g_WorldMatrix", &World)) ||
			FAILED(Shader->Bind_RawValue("g_CameraPosition",
				&CameraPosition, sizeof(CameraPosition))) ||
			FAILED(Shader->Bind_RawValue("g_EffectDynamicParameter",
				&Dynamic, sizeof(Dynamic))) ||
			FAILED(Shader->Bind_RawValue("g_UseBaseOverride",
				&iUseBaseOverride, sizeof(iUseBaseOverride))))
		{
			DrawResult = E_FAIL;
		}
	}

	ComPtr<ID3D11InfoQueue> InfoQueue;
	const bool_t bHasInfoQueue = SUCCEEDED(m_pDevice.As(&InfoQueue));
	const uint64_t iMessageBegin = bHasInfoQueue ?
		InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter() : 0u;
	bool_t bQueryBegun = false;
	{
		CReconstructedPipelineStateGuard StateGuard(m_pContext.Get());
		if (SUCCEEDED(DrawResult) &&
			FAILED(Shader->Begin(Material.Shader.iPassIndex)))
		{
			DrawResult = E_FAIL;
		}
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
				DrawResult = E_FAIL;
			else
				ActualSampler->GetDesc(&ActualSamplerDescriptor);
			if (SUCCEEDED(DrawResult) && !Same_SamplerDescriptor(
				ActualSamplerDescriptor, Resource.SamplerDescriptors[0u]))
			{
				DrawResult = E_FAIL;
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
					DrawResult = E_FAIL;
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
					DrawResult = E_FAIL;
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
					DrawResult = E_FAIL;
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
				if (FAILED(Resource.pModel->Render(iMesh)))
					DrawResult = E_FAIL;
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
			if (nullptr == m_pParticleBuffer ||
				FAILED(m_pParticleBuffer->Update_Instances(
					std::span<const Engine::VTXEFFECT_PARTICLE>(&Instance, 1u))) ||
				FAILED(m_pParticleBuffer->Render()))
			{
				DrawResult = E_FAIL;
			}
		}
		if (bQueryBegun)
		{
			m_pContext->End(Resource.pPipelineStatisticsQuery.Get());
			Resource.bPipelineStatisticsPending = true;
		}
	}

	if (FAILED(Shader->Bind_RawValue(
		Material.Shader.strEvaluatorEnabledVariable.c_str(),
		&iDisabled, sizeof(iDisabled))))
	{
		DrawResult = E_FAIL;
	}
	if (SUCCEEDED(DrawResult) && bHasInfoQueue)
	{
		const uint64_t iMessageEnd =
			InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
		for (uint64_t iMessage = iMessageBegin; iMessage < iMessageEnd;
			++iMessage)
		{
			size_t iMessageSize = 0u;
			if (FAILED(InfoQueue->GetMessage(iMessage, nullptr, &iMessageSize)) ||
				0u == iMessageSize)
				continue;
			std::vector<uint8_t> MessageBytes(iMessageSize);
			D3D11_MESSAGE* pMessage =
				reinterpret_cast<D3D11_MESSAGE*>(MessageBytes.data());
			if (SUCCEEDED(InfoQueue->GetMessage(
				iMessage, pMessage, &iMessageSize)) &&
				(pMessage->Severity == D3D11_MESSAGE_SEVERITY_ERROR ||
				 pMessage->Severity == D3D11_MESSAGE_SEVERITY_CORRUPTION))
			{
				DrawResult = E_FAIL;
				break;
			}
		}
	}
	if (FAILED(DrawResult))
	{
		m_strStatus =
			"Reconstructed diagnostic GPU draw or state readback failed.";
		return E_FAIL;
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

HRESULT Client::CEffectDocumentRenderer::Render(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	std::string GateStatus;
	if (!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		m_strStatus = std::move(GateStatus);
		return E_FAIL;
	}
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
