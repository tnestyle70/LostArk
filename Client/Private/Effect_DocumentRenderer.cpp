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
#include <limits>
#include <map>
#include <mutex>
#include <span>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
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
			std::array<ID3D11SamplerState*, 5u> PreviousRaw{};
			m_pContext->PSGetSamplers(5u, m_iCount, PreviousRaw.data());
			for (size_t i = 0u; i < m_iCount; ++i)
				m_Previous[i].Attach(PreviousRaw[i]);

			std::array<ID3D11SamplerState*, 5u> Desired{};
			for (size_t i = 0u; i < m_iCount; ++i)
				Desired[i] = Samplers[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, Desired.data());

			std::array<ID3D11SamplerState*, 5u> Applied{};
			m_pContext->PSGetSamplers(5u, m_iCount, Applied.data());
			bool_t bMatches = true;
			for (size_t i = 0u; i < m_iCount; ++i)
			{
				bMatches = bMatches && Applied[i] == Desired[i];
				if (nullptr != Applied[i])
					Applied[i]->Release();
			}
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
			std::array<ID3D11SamplerState*, 5u> PreviousRaw{};
			for (size_t i = 0u; i < m_iCount; ++i)
				PreviousRaw[i] = m_Previous[i].Get();
			m_pContext->PSSetSamplers(5u, m_iCount, PreviousRaw.data());
			m_bApplied = false;
		}

		ID3D11DeviceContext* m_pContext = nullptr;
		std::array<ComPtr<ID3D11SamplerState>, 5u> m_Previous{};
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
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE),
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
				A.Renderer.eType != B.Renderer.eType ||
				A.Renderer.eSourceSpace != B.Renderer.eSourceSpace ||
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
			Client::Is_EffectSourceLinearBlendSubUVMode(
				SourceMaterial.strSubUVMode));
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
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pReconstructedRuntimePreparation;
	uint32_t iReconstructedNeutralBaseCount = 0u;
	uint32_t iReconstructedOneLayerCount = 0u;
	uint32_t iReconstructedMaterialEvaluatorCount = 0u;
	uint32_t iRuntimeMaterialV2Count = 0u;
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
	std::string& strOutError,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		pPreparation) const
{
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
	Staged->iResourceSignature = Build_ResourceSignature(Document);
	Staged->strEffectAssetId = strEffectAssetId;
	Staged->pCatalogDocumentIdentity =
		0u == iCatalogRevision ? nullptr : &Document;
	Staged->ResourceDocument = Document;
	Staged->pReconstructedRuntimePreparation = pPreparation;
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
	const auto StageReconstructedMaterialEvaluator = [&]
		(const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
			const EFFECT_ELEMENT_DESC& Element, ELEMENT_RESOURCE& Resource,
			bool_t& bOutStaged) -> bool_t
	{
		bOutStaged = false;
		const bool_t bActive003RuntimeV2 = Emitter.Row.iOrder == 3u;
		const bool_t bActive004RuntimeV2 = Emitter.Row.iOrder == 4u;
		const bool_t bActive023RuntimeV2 = Emitter.Row.iOrder == 23u;
		const bool_t bActive016RuntimeV2 = Emitter.Row.iOrder == 16u;
		const bool_t bActive011RuntimeV2 = Emitter.Row.iOrder == 11u;
		const bool_t bActive022RuntimeV2 = Emitter.Row.iOrder == 22u;
		const bool_t bActive030RuntimeV2 = Emitter.Row.iOrder == 30u;
		const bool_t bActive031RuntimeV2 = Emitter.Row.iOrder == 31u;
		const bool_t bActive009010RuntimeV2 =
			Emitter.Row.iOrder == 9u || Emitter.Row.iOrder == 10u;
		const bool_t bRuntimeMaterialV2 = bActive003RuntimeV2 ||
			bActive004RuntimeV2 || bActive011RuntimeV2 ||
			bActive022RuntimeV2 ||
			bActive023RuntimeV2 || bActive016RuntimeV2 ||
			bActive030RuntimeV2 || bActive031RuntimeV2 ||
			bActive009010RuntimeV2;
		const bool_t bMainComposite = Emitter.Row.iOrder == 4u ||
			(Emitter.Row.iOrder >= 7u && Emitter.Row.iOrder <= 18u);
		if ((!bMainComposite && !bActive003RuntimeV2 &&
			!bActive022RuntimeV2 && !bActive023RuntimeV2 &&
			!bActive030RuntimeV2 && !bActive031RuntimeV2) ||
			Emitter.Row.iOrder == 17u)
			return true;
		if (Element.Material.SourceMaterial.strRuntimeShaderProfileId !=
			"effect.ue3.reconstructed-standard.v1" ||
			nullptr == pPreparation || nullptr == pPreparation->Get_Program() ||
			nullptr == pPreparation->Get_RenderResourceAuthority() ||
			!Emitter.strMaterialOccurrenceId.has_value() ||
			Resource.iSourceMaterialProfile != 0u ||
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
		const bool_t bExactJoin = nullptr != Occurrence && nullptr != Recipe &&
			nullptr != Family && nullptr != TextureDecision &&
			iTextureDecisionCount == 1u &&
			Occurrence->Row.iOrder == Emitter.Row.iOrder &&
			Occurrence->strEmitterId == Emitter.Row.strId &&
			Occurrence->eRenderer == Emitter.eRenderer &&
			Occurrence->strFamilyId == Recipe->strFamilyId &&
			Occurrence->strEvaluatorRegistryId == Recipe->strEvaluatorRegistryId &&
			Occurrence->strEvaluatorRegistryId == Family->strEvaluatorRegistryId &&
			TextureDecision->iFeatureMask == Family->iFeatureMask &&
			Family->iFeatureMask != 0u && Family->iEvaluatorVersion == 1u &&
			!Family->strEvaluatorId.empty() && !Family->strEvaluatorSha256.empty() &&
			Family->bCpuNumericOracleVerified &&
			Family->bHlslNumericOracleVerified &&
			(bRuntimeMaterialV2 || Recipe->NumericBindingSamples.size() == 4u);
		if (!bExactJoin)
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
		if (!bRuntimeMaterialV2)
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
						"edbfb74716e58e4e6eb6f30bc89c6a74d4f907efcbc32ea3526c7b3e61248485" },
					{ 24u, "material-input-3c6503f3b8165e28", "02.map_e",
						"fx_tex_00.fx_a_ice_002",
						"Effect/Artist/Textures/fx_a_ice_002.dds",
						"d8b2c5cc3e3f1e0181f40487d0c3f4ff5ee7b21e826870802080d3f5ef5d7c93",
						"d9aa6264270d04f35f509e0c47355edd346be916c55f25142da29d5695f22282" },
					{ 25u, "material-input-323ed9dbf8e8f9ee", "12.map_f",
						"fx_tex_02.fx_d_atypical_098",
						"Effect/Artist/Textures/fx_d_atypical_098.dds",
						"075102273f9eb40add8154ac8d91d01de87499040a9c9cd58967f3345fa52781",
						"e8cc31aa7d30901d5cbb1c16a64e111e561adf72078b8f43492c5f43678f7870" },
					{ 26u, "material-input-2f830bc7586bd3e0", "01.specmap",
						"fx_tex_00.fx_a_noise_002",
						"Effect/Artist/Textures/fx_a_noise_002.dds",
						"6fa324a5425f137f778a27bf26aa93e347cb5db820055ad1d0666248cdfc0330",
						"a3cc2103ce1fed54f12887af31550cdd386afe914b0329d99a2417e4496c0769" },
					{ 27u, "material-input-e51237e20a813da8", "06.map",
						"fx_tex_00.fx_a_noise_002",
						"Effect/Artist/Textures/fx_a_noise_002.dds",
						"f55be7cbc10df7d36fa0211ab4490a8a0817401183efdb7995294433fe37545f",
						"bea6202f62730c2daab2d8d4d93fd7b8d58d0a43cdb9c2fd70103de8908107e0" }
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
		else if (bActive031RuntimeV2)
		{
			if (Emitter.Row.strRowSha256 !=
					"c8e93946ca1e43663944573f1d7e4e4522eda5757ca3d3dd4368e1b9e7b4c2c4" ||
				Occurrence->Row.strId != "source-active-031" ||
				Occurrence->Row.strRowSha256 !=
					"5beb0f7e5cd36360376c17297e253860c38888afcf55bcb5c0e5f29fbd68e686" ||
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
					"e8fe3b0a327f9699f587e9cbcb1b9a7b1f681c2af846f5f4af1c58a10b43a667" ||
				!TextureDecision->bSecondTextureOperationEnabled ||
				TextureDecision->Texture0Provider.strMaterialInputFieldId !=
					"material-input-281f2c37bdf51c10" ||
				TextureDecision->Texture0Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_d_noise_003.dds" ||
				TextureDecision->Texture1Provider.strMaterialInputFieldId !=
					"material-input-30557ad00debea91" ||
				TextureDecision->Texture1Provider.strRuntimeAssetId !=
					"Effect/Artist/Textures/fx_c_noise_002.dds")
			{
				strOutError = "Artist F active031 runtime-material identity changed.";
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
					strOutError = "Artist F active031 material input changed.";
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
				strOutError = "Artist F active031 provider alias changed.";
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
					strOutError = "Artist F active031 render input changed.";
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
			static constexpr std::array<std::string_view, 4u> DYNAMIC_ROWS = {{
				"35ed171fe67bfe0df6142ca75ca3d3f7e71c8ba51e9fd66fc5d964a72bc31edc",
				"b7b1bbd29bbc54d11f0956c418f71cb056483e80b00853cd3fa4712834f9db12",
				"c6217d69da19db35acdfe1de5fa43943a0afd9e62d8f4a165f9c0c180c8ec027",
				"66e0b3d912c8b3ded65b0d3be5e86caeb0fd5a53cea19c20c79c31d15b608429"
			}};
			bool_t bDynamicContract = iDynamicCount == 1u && DynamicModule != nullptr &&
				DynamicModule->Row.iOrder == 8u &&
				DynamicModule->Row.strRowSha256 ==
					"5f952db6da8ca1b5a8d90656a16bb7c5d9ce42969c80c713d793ef7ab434934c" &&
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
					"dfb01c49b44b01ba8a759d0d0b0c49dcc74b3795a58f56e8ed1b02cb2171883d" ||
				nullptr == ColorScaleModule || ColorScaleModule->Row.iOrder != 6u ||
				ColorScaleModule->Row.strRowSha256 !=
					"3873af1e16834e9ae65fb5e0b49b0c99b96b8a7003fa1562242a622925b0de13" ||
				!HasDistribution(ColorModule, "startcolor",
					"d49cd750c515a9af50d763db601679615df847f6f8e7f71c3819875388d8a729") ||
				!HasDistribution(ColorModule, "startalpha",
					"52db1d2aec382dc715ab48fc6dbd1f2d14ac558c227d41d67c63d0e9e20d9bed") ||
				!HasDistribution(ColorScaleModule, "colorscaleoverlife",
					"744ec56af03043ca2ea466d61aeb865b2dc03d2b1da057ac85d90bc35239520c") ||
				!HasDistribution(ColorScaleModule, "alphascaleoverlife",
					"05c9643f956604286a87007c5fcf0167a2e0a5509fef71dea8fc816a3644d1c0") ||
				Element.Material.SourceMaterial.strSubUVMode != "none")
			{
				strOutError = "Artist F active031 dynamic/color/UV contract changed.";
				return false;
			}

			if (!AssignFloat(*OrderedInputs[0u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].x) ||
				!AssignFloat(*OrderedInputs[2u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].y) ||
				!AssignFloat(*OrderedInputs[4u]->fValue,
					Resource.RuntimeMaterialV2ScalarBlocks[0u].z))
			{
				strOutError = "Artist F active031 scalar packing changed.";
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
					"86aae4afb560fbe064479f339b94e8e92b95b92f784d47f7e1325af56304b1a0",
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
					"df8512f2e414900b981a492537db001fc0dfba75bb1af26b4ba08e3ea06808f2",
					20u, 303u, 17u, 14u, 1u, 2u, 1u, 0x01u, 0u, 0x01u,
					0u, 0x0fu, { 0x0001849du, 0u }, { 0x00007b62u, 0u },
					{ 0x0fu, 0u, 0u }, { 0u, 0u, 0u },
					EFFECT_ELEMENT_KIND::TRAIL } :
				(bActive011RuntimeV2 ? RUNTIME_V2_PROFILE{
					"a025008232de2fd361af824619c27c4f9338b184264585365bd9a254e456f6af",
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
					"764cd92a5151d4d017f403108e7ec0eae3bcd54bacd555eeb44c159274891222",
					23u, 1007u, 55u, 47u, 1u, 7u, 9u, 0x1efu, 0x1ffu,
					0u, 0x07u, 0x08u, { 0xffffffffu, 0x0000007fu },
					{ 0u, 0x007fff80u }, { 0x07u, 0u, 0u }, { 0x08u, 0u, 0u },
					EFFECT_ELEMENT_KIND::PARTICLE } : RUNTIME_V2_PROFILE{
					"d6a0be5527d46ff1c189f09f11af1fcef5ef7c4f233f8c70e47478972df65c94",
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
					"d78aaec1f03d22ef2b3063ef1e3c2b34fa5a2d8a5721348fa54560ee72ab18f0",
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
				if (Recipe->InputIds[0u] != "material-input-2b161e06207b4a97" ||
					Recipe->InputIds[10u] != "material-input-88d1c7b61c5c81d7" ||
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
					"bf1216fa2596b5a731a6dc17bcef587b9f28c8fb40e398ef2465352be695a36a",
					"material-reconstructed-policy-3e961174dffa8d797bd8",
					"6bdfee584fbca204070fe582f1750d4cf9e61b0aecd71889e3c7e22d1d86067c",
					"Effect/Artist/Textures/fx_m_atypical_012.dds",
					"FROZEN_PROGRAM_TEXTURE_BINDING_ORDER" });
				RuntimeTextureProviders.push_back({ "MATERIAL_TEXTURE_BINDING", {},
					"material-input-cb33861f6d80fecb",
					"387c779bb8ebb30e60f3a50012317da9ddb824258f713bfcc3a52f713afb6984",
					"material-input-cb33861f6d80fecb::runtime-texture-binding",
					"48301de579de30aabef9b1509cd7462dfe9fbaa625e8cca57686ff5057cde74b",
					"material-reconstructed-policy-89ec510e668d6b156733",
					"0c6bd408baee598f6920790b616e6e5e1fdba7cbf63734cdeb63b1e422e3c53c",
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
		if (bActive003RuntimeV2 || bActive004RuntimeV2 ||
			bActive009010RuntimeV2 || bActive011RuntimeV2 ||
			bActive016RuntimeV2 || bActive022RuntimeV2)
		{
			for (size_t iLane = 0u; iLane < RuntimeTextureProviders.size(); ++iLane)
			{
				if (!StageTextureLane(iLane, RuntimeTextureProviders[iLane]))
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
		else if (!bActive023RuntimeV2 &&
			(!StageTextureLane(0u, TextureDecision->Texture0Provider) ||
				!StageTextureLane(1u, TextureDecision->Texture1Provider)))
		{
			strOutError =
				"Reconstructed common material evaluator texture stage failed: " +
				Element.strElementId;
			return false;
		}
		const uint32_t iExpectedTextureMask = bActive004RuntimeV2 ? 0x0fu :
			(bActive016RuntimeV2 ? 0x1fu :
				(bActive023RuntimeV2 ? 0u :
					(bActive011RuntimeV2 ? 0x0fu :
						(bActive022RuntimeV2 ? 0x01u : 0x03u))));
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
		else if (bActive031RuntimeV2)
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
		Resource.iReconstructedMaterialEvaluatorEnabled = 1u;
		Resource.iReconstructedMaterialFeatureMask = Family->iFeatureMask;
		Resource.bSourceMaterialFallbackBlocked = false;
		bOutStaged = true;
		return true;
	};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		f32_t fModelPreScale = 1.f;
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
		if (FAILED(Stage_ElementResource(
			Element, Resource, strOutError, pSharedAssets, fModelPreScale)))
		{
			return false;
		}
		bool_t bReconstructedMaterialEvaluatorStaged = false;
		if (nullptr != pProgramEmitter &&
			!StageReconstructedMaterialEvaluator(*pProgramEmitter, Element,
				Resource, bReconstructedMaterialEvaluatorStaged))
		{
			return false;
		}
		if (bReconstructedMaterialEvaluatorStaged)
			++Staged->iReconstructedMaterialEvaluatorCount;
		if (0u != Resource.iRuntimeMaterialV2Enabled)
			++Staged->iRuntimeMaterialV2Count;
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
	uint32_t iRequiredTrailPoints = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
		{
			uint32_t iElementRenderPoints = Element.Detail.Trail.iMaxPoints;
			if (Element.Detail.Trail.fDistanceTessellationStepWorldUnits > 0.f)
			{
				constexpr uint32_t MAX_SEGMENT_SUBDIVISIONS = 25u;
				if (Element.Detail.Trail.iMaxPoints >
					((std::numeric_limits<uint32_t>::max)() - 1u) /
					MAX_SEGMENT_SUBDIVISIONS + 1u)
				{
					strOutError = "Effect trail tessellation capacity overflowed.";
					return false;
				}
				iElementRenderPoints = 1u +
					(Element.Detail.Trail.iMaxPoints - 1u) *
					MAX_SEGMENT_SUBDIVISIONS;
			}
			iRequiredTrailPoints = (std::max)(
				iRequiredTrailPoints, iElementRenderPoints);
		}
	}
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
	if (bNeedsTrailBuffer)
	{
		const uint32_t iTrailBufferPoints =
			(std::max)(256u, iRequiredTrailPoints);
		TrailBuffer = Engine::CVIBuffer_DynamicTrail::Create(
			m_pDevice, m_pContext,
			iTrailBufferPoints);
		if (nullptr == TrailBuffer)
		{
			strOutError =
				"Per-instance Effect trail buffer creation failed for maxPoints=" +
				std::to_string(iTrailBufferPoints) + ".";
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
		OutPrepared->iReconstructedMaterialEvaluatorCount != 17u ||
		OutPrepared->iRuntimeMaterialV2Count != 10u)
	{
		const uint32_t iNeutralCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedNeutralBaseCount;
		const uint32_t iOneLayerCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedOneLayerCount;
		const uint32_t iEvaluatorCount = nullptr == OutPrepared ? 0u :
			OutPrepared->iReconstructedMaterialEvaluatorCount;
		const uint32_t iRuntimeV2Count = nullptr == OutPrepared ? 0u :
			OutPrepared->iRuntimeMaterialV2Count;
		OutPrepared.reset();
		strOutError =
			"Reconstructed source runtime prewarm lost its immutable authority "
			"or exact material evaluator denominator: neutral=" +
			std::to_string(iNeutralCount) + ", oneLayer=" +
			std::to_string(iOneLayerCount) + ", evaluator=" +
			std::to_string(iEvaluatorCount) + ", runtimeV2=" +
			std::to_string(iRuntimeV2Count) + ".";
		return false;
	}
	strOutError.clear();
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
	m_bReconstructedSourceRuntimeActive = false;
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
		m_bReconstructedSourceRuntimeActive = false;
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
	m_bReconstructedSourceRuntimeActive = false;
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
	if (!Ensure_MutableInstanceBuffers(Document, strOutError) ||
		!Clone_ModelCueResources(*pPrepared,
			StagedModelCueResources, strOutError))
	{
		return false;
	}
	m_Document = Document;
	m_pPreparedDocument = std::move(pPrepared);
	m_pReconstructedDiagnostic.reset();
	m_ModelCueResources = std::move(StagedModelCueResources);
	m_ReconstructedRuntimeBoundary = std::move(StagedBoundary);
	m_bReconstructedSourceRuntimeActive = true;
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
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
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
	m_bReconstructedSourceRuntimeActive = false;
	m_ModelCueResources.clear();
	m_LastRenderSubmissionStats = {};
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
				sizeof(Resource.vReconstructedParams1))))
		{
			return Fail_RenderOperation(
				"Material bind failed: reconstructed evaluator block.",
				hFirstBindFailure);
		}
	}
	const bool_t bRuntimeMaterialV2Shader = pShader == m_pMeshShader ||
		pShader == m_pParticleShader || pShader == m_pDecalShader ||
		pShader == m_pTrailShader;
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
		(BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
			&Resource.iSourceMaterialProfile,
			sizeof(Resource.iSourceMaterialProfile))) ||
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
	return bBindFailed ? Fail_RenderOperation(
		"Material bind failed: common constants/textures block.",
		hFirstBindFailure) : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Mesh(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const float4x4_t* pWorldOverride,
	const float4_t* pDynamicParameter)
{
	if (nullptr == Resource.pModel || nullptr == Element.pElement ||
		nullptr == m_pMeshShader)
		return Fail_RenderOperation(
			"Mesh resource/model/shader contract is missing.", E_FAIL, true);
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Mesh render-profile pass is invalid.", E_INVALIDARG, true);
	const float4x4_t& World = nullptr != pWorldOverride ?
		*pWorldOverride : Element.World;
	const bool_t bMainSourceReplay =
		0u != Resource.iRuntimeMaterialV2Enabled &&
		(3u == Resource.iRuntimeMaterialV2Opcode ||
			8u == Resource.iRuntimeMaterialV2Opcode);
	if (bMainSourceReplay && nullptr == pDynamicParameter)
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
	const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
	float4x4_t NormalMatrix = World;
	if (0u != Resource.iRuntimeMaterialV2Enabled)
	{
		const matrix_t LoadedWorld = XMLoadFloat4x4(&World);
		const vector_t Determinant = XMMatrixDeterminant(LoadedWorld);
		const f32_t fDeterminant = XMVectorGetX(Determinant);
		if (!std::isfinite(fDeterminant))
			return Fail_RenderOperation(
				"Mesh world determinant is non-finite.", E_INVALIDARG, true);
		// A zero SizeMultiplyLife sample is a valid no-pixel state at the
		// occurrence boundary.  It must not abort the whole effect frame.
		if (std::abs(fDeterminant) <=
			std::numeric_limits<f32_t>::epsilon())
			return S_FALSE;
		XMStoreFloat4x4(&NormalMatrix,
			XMMatrixTranspose(XMMatrixInverse(nullptr, LoadedWorld)));
	}
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
	hResult = Bind_Common(m_pMeshShader, Element, Resource, fAlphaScale);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Mesh common/material shader bind failed.", hResult);

	const ComPtr<ID3D11ShaderResourceView> BaseOverride =
		Element.pElement->Detail.Mesh.bUseModelMaterial ? nullptr :
		Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	const uint32_t iUseBaseOverride = nullptr != BaseOverride ? 1u : 0u;
	hResult = m_pMeshShader->Bind_RawValue(
		"g_UseBaseOverride", &iUseBaseOverride, sizeof(iUseBaseOverride));
	if (FAILED(hResult))
	{
		return Fail_RenderOperation(
			"Mesh shader bind failed: g_UseBaseOverride.", hResult);
	}
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
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (0u != Resource.iRuntimeMaterialV2Enabled)
		{
			const size_t iSamplerCount =
				static_cast<size_t>(Resource.iRuntimeMaterialV2TextureLaneCount);
			if (iSamplerCount == 0u ||
				iSamplerCount > Resource.RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					Resource.RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Mesh RuntimeMaterialV2 sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
		}
		hResult = Resource.pModel->Render(iMesh);
		if (FAILED(hResult))
			return Fail_RenderOperation("Mesh model draw failed.", hResult);
	}
	return S_OK;
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
	hResult = m_pRect->Render();
	return FAILED(hResult) ?
		Fail_RenderOperation("Sprite rectangle draw failed.", hResult) : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Decal(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement || nullptr == m_pDecalShader ||
		nullptr == m_pRect)
		return Fail_RenderOperation("Decal element/shader/buffer contract is missing.",
			E_INVALIDARG, true);
	const uint32_t iPass = Select_Pass(
		Element.pElement->Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation("Decal render-profile pass is invalid.",
			E_INVALIDARG, true);
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
	hResult = CGameInstance::Get().Bind_RT_SRV(
		TEXT("Target_Depth"), m_pDecalShader, "g_DepthTexture");
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Decal depth render-target bind failed.", hResult);
	hResult = m_pDecalShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation("Decal shader pass apply failed.", hResult);
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
				"Decal RuntimeMaterialV2 sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
	}
	hResult = m_pRect->Render();
	return FAILED(hResult) ?
		Fail_RenderOperation("Decal rectangle draw failed.", hResult) : S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Element(
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource)
{
	if (nullptr == Element.pElement)
		return Fail_RenderOperation(
			"Effect element descriptor is missing.", E_INVALIDARG, true);
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
	const ELEMENT_RESOURCE* pResource = Find_Resource(pSource->strElementId);
	if (nullptr == pResource)
		return Fail_RenderOperation(
			"Particle resource contract is missing.", E_FAIL, true);
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
			const HRESULT Result = Render_Mesh(MeshParticle, *pResource,
				1.f, nullptr, &Particle.vDynamicParameter);
			if (FAILED(Result))
				return Result;
			bSubmitted = bSubmitted || S_OK == Result;
		}
		return bSubmitted ? S_OK : S_FALSE;
	}
	std::vector<Engine::VTXEFFECT_PARTICLE> Instances;
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
		const EFFECT_SUBUV_FRAME_DESC SubUV = Resolve_SubUVFrames(Particle);
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
	const f32_t LocalTime = (std::max)(0.f,
		Frame.fSampleTimeSeconds - Source.Detail.Timing.fStartDelaySeconds);
	const f32_t Normalized = std::clamp(
		LocalTime / Source.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f);
	EFFECT_COLOR_DESC CommonColor =
		Evaluate_CommonColor(Source, Normalized);
	CommonColor.vColorMultiply = float4_t(1.f, 1.f, 1.f, 1.f);
	hResult = Bind_Common(m_pParticleShader, Source, CommonColor,
		LocalTime, Normalized, *pResource);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle common/material shader bind failed.", hResult);
	const uint32_t iPass = Select_Pass(Source.Material.eRenderProfile);
	if (UINT32_MAX == iPass)
		return Fail_RenderOperation(
			"Particle render-profile pass is invalid.", E_INVALIDARG, true);
	hResult = m_pParticleShader->Begin(iPass);
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle shader pass apply failed.", hResult);
	PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
	if (0u != pResource->iRuntimeMaterialV2Enabled &&
		0u != pResource->iRuntimeMaterialV2TextureLaneCount)
	{
		const size_t iSamplerCount = static_cast<size_t>(
			pResource->iRuntimeMaterialV2TextureLaneCount);
		if (iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
			!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
				pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
		{
			return Fail_RenderOperation(
				"Particle RuntimeMaterialV2 sampler apply failed.", E_FAIL,
				SamplerScope.Was_LastFailureContractInvalid());
		}
	}
	hResult = m_pParticleBuffer->Render();
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Particle instance-buffer draw failed.", hResult);
	return S_OK;
}

HRESULT Client::CEffectDocumentRenderer::Render_Trails(
	const EFFECT_EVALUATED_FRAME& Frame,
	const std::span<const EFFECT_EVALUATED_TRAIL> Trails)
{
	bool_t bSubmitted = false;
	const vector_t CameraPosition = XMLoadFloat4(
		CGameInstance::Get().Get_CamPosition());
	for (const EFFECT_EVALUATED_TRAIL& Trail : Trails)
	{
		if (nullptr == Trail.pElement || Trail.Points.size() < 2u)
			continue;
		const ELEMENT_RESOURCE* pResource =
			Find_Resource(Trail.pElement->strElementId);
		if (nullptr == pResource)
			return Fail_RenderOperation(
				"Trail resource contract is missing.", E_FAIL, true);
		if (pResource->bSourceMaterialFallbackBlocked)
			continue;
		const bool_t bTypedArtistRibbon =
			0u != pResource->iRuntimeMaterialV2Enabled &&
			9u == pResource->iRuntimeMaterialV2Opcode;
		if (0u != pResource->iRuntimeMaterialV2Enabled &&
			!bTypedArtistRibbon)
		{
			return Fail_RenderOperation(
				"Trail RuntimeMaterialV2 opcode is invalid.", E_INVALIDARG, true);
		}
		const f32_t fTilingDistance =
			Trail.pElement->Detail.Trail.fTilingDistanceWorldUnits;
		const f32_t fTessellationStep =
			Trail.pElement->Detail.Trail.fDistanceTessellationStepWorldUnits;
		constexpr f32_t ARTIST_RIBBON_TILING_DISTANCE = 6.f;
		constexpr f32_t ARTIST_RIBBON_TESSELLATION_STEP = 0.05f;
		constexpr uint32_t ARTIST_RIBBON_MAX_SUBDIVISIONS = 25u;
		if (bTypedArtistRibbon &&
			(!std::isfinite(fTilingDistance) ||
				!std::isfinite(fTessellationStep) ||
				std::abs(fTilingDistance - ARTIST_RIBBON_TILING_DISTANCE) > 1e-6f ||
				std::abs(fTessellationStep - ARTIST_RIBBON_TESSELLATION_STEP) > 1e-6f))
		{
			return Fail_RenderOperation(
				"Trail typed tiling/tessellation contract is invalid.",
				E_INVALIDARG, true);
		}

		std::vector<EFFECT_EVALUATED_TRAIL_POINT> TessellatedPoints;
		std::span<const EFFECT_EVALUATED_TRAIL_POINT> RenderPoints(
			Trail.Points.data(), Trail.Points.size());
		if (bTypedArtistRibbon)
		{
			const auto ValidatePoint = [](const EFFECT_EVALUATED_TRAIL_POINT& Point)
			{
				return Point.iSourceColorComponentMask == 0x08u &&
					Point.iDynamicParameterComponentMask == 0x0fu &&
					std::isfinite(Point.vWorldPosition.x) &&
					std::isfinite(Point.vWorldPosition.y) &&
					std::isfinite(Point.vWorldPosition.z) &&
					std::isfinite(Point.fNormalizedAge) &&
					Point.fNormalizedAge >= 0.f && Point.fNormalizedAge < 1.f &&
					std::isfinite(Point.fCumulativeDistance) &&
					Point.fCumulativeDistance >= 0.f &&
					std::isfinite(Point.vSourceColor.w) &&
					std::isfinite(Point.vDynamicParameter.x) &&
					std::isfinite(Point.vDynamicParameter.y) &&
					std::isfinite(Point.vDynamicParameter.z) &&
					std::isfinite(Point.vDynamicParameter.w);
			};
			if (!std::all_of(Trail.Points.begin(), Trail.Points.end(), ValidatePoint))
				return Fail_RenderOperation(
					"Trail typed point payload is invalid.", E_INVALIDARG, true);

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

		std::vector<Engine::VTXEFFECT_TRAIL> Vertices;
		std::vector<uint32_t> Indices;
		Vertices.reserve(RenderPoints.size() * 2u);
		Indices.reserve((RenderPoints.size() - 1u) * 6u);
		for (size_t iPoint = 0u; iPoint < RenderPoints.size(); ++iPoint)
		{
			const EFFECT_EVALUATED_TRAIL_POINT& Point = RenderPoints[iPoint];
			if (bTypedArtistRibbon)
			{
				if (Point.iSourceColorComponentMask != 0x08u ||
					Point.iDynamicParameterComponentMask != 0x0fu ||
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
			const f32_t Width = Trail.pElement->Detail.Trail.fStartWidth +
				(Trail.pElement->Detail.Trail.fEndWidth -
					Trail.pElement->Detail.Trail.fStartWidth) * Age;
			const vector_t HalfSide = Side * (Width * 0.5f);
			const f32_t U = bTypedArtistRibbon ?
				Point.fCumulativeDistance / fTilingDistance :
				static_cast<f32_t>(iPoint);
			const float4_t Color = bTypedArtistRibbon ?
				float4_t(1.f, 1.f, 1.f, Point.vSourceColor.w) :
				float4_t(1.f, 1.f, 1.f, 1.f - Age);
			Vertices.push_back({ To_Float3(Position - HalfSide),
				float2_t(U, 0.f), Color });
			Vertices.push_back({ To_Float3(Position + HalfSide),
				float2_t(U, 1.f), Color });
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
		PIXEL_SHADER_SAMPLER_SCOPE SamplerScope(m_pContext.Get());
		if (bTypedArtistRibbon)
		{
			const size_t iSamplerCount = static_cast<size_t>(
				pResource->iRuntimeMaterialV2TextureLaneCount);
			if (iSamplerCount != 2u ||
				iSamplerCount > pResource->RuntimeMaterialV2Samplers.size() ||
				!SamplerScope.Apply(std::span<const ComPtr<ID3D11SamplerState>>(
					pResource->RuntimeMaterialV2Samplers.data(), iSamplerCount)))
			{
				return Fail_RenderOperation(
					"Trail RuntimeMaterialV2 sampler apply failed.", E_FAIL,
					SamplerScope.Was_LastFailureContractInvalid());
			}
		}
		hResult = m_pTrailBuffer->Render();
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Trail geometry-buffer draw failed.", hResult);
		bSubmitted = true;
	}
	return bSubmitted ? S_OK : S_FALSE;
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
			return Fail_RenderOperation(
				"Animated model-cue resource contract is missing.", E_FAIL, true);
		}
		Engine::CModel& Model = *Resource->second.pModel;
		if (!Model.Set_AnimTrackPosition(Resource->second.iAnimationIndex,
			fLocalTime * Resource->second.fTicksPerSecond))
		{
			return Fail_RenderOperation(
				"Animated model-cue track position is invalid.", E_FAIL, true);
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
			hResult = m_pAnimatedModelShader->Begin(0u);
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

HRESULT Client::CEffectDocumentRenderer::Render(
	const EFFECT_EVALUATED_FRAME& Frame)
{
	m_LastRenderSubmissionStats = {};
	m_strRenderFailureDetail.clear();
	m_bLastRenderFailureObjectLocal = false;
	size_t iConfiguredGpuOccurrenceCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : m_Document.Elements)
	{
		if (!Element.bVisible)
			continue;
		const EFFECT_GPU_RENDER_FAMILY eFamily =
			Resolve_GpuRenderFamily(Element);
		if (EFFECT_GPU_RENDER_FAMILY::END != eFamily)
		{
			++iConfiguredGpuOccurrenceCount;
			++m_LastRenderSubmissionStats.Families[
				static_cast<size_t>(eFamily)].iConfigured;
		}
	}
	const auto FailFrame = [this](std::string strStatus,
		const HRESULT hResult = E_FAIL,
		const bool_t bObjectLocal = false) -> HRESULT
	{
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
	std::unordered_map<std::string,
		const EFFECT_EVALUATED_GPU_OCCURRENCE*> GpuOccurrenceEvaluations;
	GpuOccurrenceEvaluations.reserve(Frame.GpuOccurrences.size());
	for (const EFFECT_EVALUATED_GPU_OCCURRENCE& Occurrence :
		Frame.GpuOccurrences)
	{
		if (nullptr == Occurrence.pElement ||
			EFFECT_GPU_RENDER_FAMILY::END ==
				Resolve_GpuRenderFamily(*Occurrence.pElement) ||
			!GpuOccurrenceEvaluations.emplace(
				Occurrence.pElement->strElementId, &Occurrence).second)
		{
			return FailFrame(
				"Effect GPU occurrence evaluation is invalid or duplicated.",
				E_FAIL, true);
		}
	}
	if (GpuOccurrenceEvaluations.size() != iConfiguredGpuOccurrenceCount)
	{
		return FailFrame(
			"Effect GPU occurrence evaluation count does not match the document.",
			E_FAIL, true);
	}
	std::string GateStatus;
	if (!m_bReconstructedSourceRuntimeActive &&
		!m_ReconstructedRuntimeBoundary.Admit_Render(GateStatus))
	{
		return FailFrame(std::move(GateStatus), E_FAIL, true);
	}
	const HRESULT hModelCueResult = Render_ModelCues(Frame);
	if (FAILED(hModelCueResult))
	{
		return FailFrame(
			"Effect animated model-cue rendering failed.", hModelCueResult);
	}
	size_t iElement = 0u;
	size_t iParticle = 0u;
	size_t iTrail = 0u;
	size_t iAfterImage = 0u;
	for (const EFFECT_ELEMENT_DESC& DocumentElement : m_Document.Elements)
	{
		const std::string& strElementId = DocumentElement.strElementId;
		const EFFECT_GPU_RENDER_FAMILY eFamily = DocumentElement.bVisible ?
			Resolve_GpuRenderFamily(DocumentElement) :
			EFFECT_GPU_RENDER_FAMILY::END;
		EFFECT_GPU_RENDER_FAMILY_STATS* pFamilyStats =
			EFFECT_GPU_RENDER_FAMILY::END == eFamily ? nullptr :
			&m_LastRenderSubmissionStats.Families[
				static_cast<size_t>(eFamily)];
		const auto GpuOccurrenceIterator =
			GpuOccurrenceEvaluations.find(strElementId);
		const EFFECT_EVALUATED_GPU_OCCURRENCE* pGpuOccurrence =
			GpuOccurrenceEvaluations.end() == GpuOccurrenceIterator ?
			nullptr : GpuOccurrenceIterator->second;
		if ((nullptr != pFamilyStats) != (nullptr != pGpuOccurrence) ||
			(nullptr != pGpuOccurrence &&
			 eFamily != Resolve_GpuRenderFamily(*pGpuOccurrence->pElement)))
		{
			return FailFrame(
				"Effect GPU occurrence evaluation does not match the document.",
				E_FAIL, true);
		}
		if (nullptr != pFamilyStats)
			++pFamilyStats->iEvaluated;
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
		const HRESULT hAfterImageResult = Render_AfterImages(Frame,
			std::span<const EFFECT_EVALUATED_AFTERIMAGE>(Frame.AfterImages)
				.subspan(iAfterImageBegin,
					iAfterImage - iAfterImageBegin));
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
			const ELEMENT_RESOURCE* pResource = Find_Resource(strElementId);
			if (nullptr == pResource)
			{
				if (nullptr != pFamilyStats)
					++pFamilyStats->iFailed;
				return FailFrame(
					"Effect element resource is missing: " + strElementId,
					E_FAIL, true);
			}
			if (pResource->bSourceMaterialFallbackBlocked)
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
		const HRESULT hParticleResult = Render_Particles(Frame,
			std::span<const EFFECT_EVALUATED_PARTICLE>(Frame.Particles)
				.subspan(iParticleBegin,
					iParticle - iParticleBegin));
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
		const HRESULT hTrailResult = Render_Trails(Frame,
				std::span<const EFFECT_EVALUATED_TRAIL>(Frame.Trails)
					.subspan(iTrailBegin, iTrail - iTrailBegin));
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
		if (bOccurrenceActive)
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
			if (bOccurrenceSubmitted)
				++pFamilyStats->iSubmitted;
			else if (bOccurrenceSuppressed)
				++pFamilyStats->iSuppressed;
			else
			{
				++pFamilyStats->iFailed;
				return FailFrame(
					"Effect active GPU occurrence produced no disposition: " +
					strElementId, E_FAIL, true);
			}
		}
	}
	if (iElement != Frame.Elements.size() ||
		iParticle != Frame.Particles.size() ||
		iTrail != Frame.Trails.size() ||
		iAfterImage != Frame.AfterImages.size())
	{
		return FailFrame(
			"Effect frame contains unconsumed evaluated GPU rows.",
			E_FAIL, true);
	}
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
	m_strStatus = "Effect GPU occurrence disposition: evaluated " +
		std::to_string(Frame.GpuOccurrences.size()) + ", active " +
		std::to_string(iActive) + ", submitted " +
		std::to_string(iSubmitted) + ", suppressed " +
		std::to_string(iSuppressed) + ".";
	return 0u < iActive && 0u == iSubmitted ? S_FALSE : S_OK;
}
