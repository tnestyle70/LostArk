#pragma once

#include "Effect_AuthoringDocument.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class EFFECT_MATERIAL_INPUT_SEMANTIC : uint8_t
{
	BASE,
	NOISE,
	MASK,
	EMISSIVE,
	DISSOLVE,
	BASE2,
	MASK2,
	NOISE2,
	END
};

struct EFFECT_MATERIAL_INPUT_SLOT_DESC final
{
	std::string_view strSlotId;
	std::string_view strDisplayName;
	std::string_view strHlslBindingName;
	EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic =
		EFFECT_MATERIAL_INPUT_SEMANTIC::END;
	EFFECT_RESOURCE_FILE_KIND eAllowedResourceKind =
		EFFECT_RESOURCE_FILE_KIND::END;
	EFFECT_RESOURCE_SLOT eRuntimeSlot = EFFECT_RESOURCE_SLOT::END;
};

struct EFFECT_MATERIAL_TEMPLATE_DESC final
{
	std::string_view strTemplateId;
	std::string_view strShaderProfileId;
	const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInputs = nullptr;
	std::size_t iInputCount = 0u;
};

inline constexpr std::string_view EFFECT_MESH_SHAPE_SLOT_ID = "meshModel";
inline constexpr std::string_view EFFECT_STANDARD_MATERIAL_TEMPLATE_ID =
	"effect.standard";
inline constexpr std::string_view EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID =
	"effect.standard_color_v1";
inline constexpr std::string_view EFFECT_SOURCE_MATERIAL_TEMPLATE_ID =
	"effect.source_material";
inline constexpr f32_t EFFECT_MANUAL_MESH_DEFAULT_SCALE = 0.01f;

inline constexpr std::string_view EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID =
	"effect.ue3.missiletrail-01.v1";
inline constexpr std::string_view
	EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID =
	"effect.ue3.missiletrail-two-emissive.v1";
inline constexpr std::string_view EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID =
	"effect.ue3.watertrail-01.v1";

inline constexpr std::array<std::string_view, 22u>
	EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS = {{
		"effect.ue3.reconstructed-standard.v1",
		"effect.ue3.fallback-blocked.v1",
		"effect.ue3.circle.v1",
		"effect.ue3.dot.v1",
		"effect.ue3.ring.v1",
		"effect.ue3.aura.v1",
		"effect.ue3.one-layer-distortion.v1",
		"effect.ue3.grouped-translucent.v1",
		"effect.ue3.shine.v1",
		"effect.ue3.blackline-aura.v1",
		"effect.ue3.linearflow-02.v1",
		"effect.ue3.slice.v1",
		EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID,
		EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID,
		"effect.ue3.local-crack.v1",
		"effect.ue3.procedural-center-glow.v1",
		EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID,
		"effect.ue3.glasshole-02.v1",
		"effect.ue3.fluidninja-01.v1",
		"effect.ue3.customparticle-01.v1",
		"effect.ue3.crackholev2-01.v1",
		"effect.ue3.simple-01.v1"
	}};

// fx_mm_simple_01_ad is the corpus' most-used parent: 359 grouped occurrences
// across 74 documents and all four classes, every one of them a sprite binding
// only `base`. The material itself is small - one emissive sample, an optional
// UV-noise offset, a desaturation - so it needs a named lane set rather than the
// substring guessing the grouped profile falls back on.
inline constexpr std::array<std::string_view, 1u>
	EFFECT_SIMPLE01_SOURCE_TEXTURE_NAMES = {{
		"emissive_tex"
	}};
inline constexpr std::array<std::string_view, 3u>
	EFFECT_SIMPLE02_SOURCE_TEXTURE_NAMES = {{
		"emissive_tex", "uv_noise_tex", "emissive_tex_02"
	}};
inline constexpr std::array<std::string_view, 1u>
	EFFECT_MM_BASIC01_SOURCE_TEXTURE_NAMES = {{
		"emissive_tex"
	}};
inline constexpr std::array<std::string_view, 2u>
	EFFECT_FLOWTRAIL01_SOURCE_TEXTURE_NAMES = {{
		"diff_tex", "opacity_tex"
	}};

inline constexpr std::array<std::string_view, 7u>
	EFFECT_LINEARFLOW_SOURCE_TEXTURE_NAMES = {{
		"diff_tex", "diff_noise_tex", "a_mask_tex", "a_noise_01_tex",
		"b_mask_tex", "b_noise_01_tex", "dissolve_tex"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_BLACKLINE_SOURCE_TEXTURE_NAMES = {{
		"diffuse_tex", "flow_tex", "mask_a_tex", "mask_b_tex",
		"dissolve_tex"
	}};
inline constexpr std::array<std::string_view, 3u>
	EFFECT_LOCAL_CRACK_SOURCE_TEXTURE_NAMES = {{
		"normal_tex", "refle_tex", "dissolve_tex"
	}};
inline constexpr std::array<std::string_view, 2u>
	EFFECT_WATERTRAIL_SOURCE_TEXTURE_NAMES = {{
		"maintex", "uv_noise_tex"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_MAKEFLOW_MESH_SOURCE_TEXTURE_NAMES = {{
		"opacity_tex", "diff_tex1", "diff_tex2", "color_tex", "flowtex"
	}};
inline constexpr std::array<std::string_view, 4u>
	EFFECT_MAKEFLOW03_SPRITE_SOURCE_TEXTURE_NAMES = {{
		"diff_tex1", "diff_tex2", "color_tex", "mask_tex"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_MISSILETRAIL_SOURCE_TEXTURE_NAMES = {{
		"alpha_tex", "emissive_tex01", "emissive_tex02",
		"uv_dissolve_tex", "uv_noise_tex"
	}};
inline constexpr std::array<std::string_view, 6u>
	EFFECT_PARTICLE_MASTER_SOURCE_TEXTURE_NAMES = {{
		"21.map_c", "01.map_a", "11.map_b", "06.map", "02.map_e",
		"12.map_f"
	}};
inline constexpr std::array<std::string_view, 7u>
	EFFECT_SPRITEWAVE_SOURCE_TEXTURE_NAMES = {{
		"maintex", "uv_noise_tex", "dissolve_tex_01",
		"noisedissolve_tex", "dissolve_tex02", "emissivetex02",
		"uv_noise_tex_02"
	}};
inline constexpr std::array<std::string_view, 2u>
	EFFECT_PARTICLETRAIL_SINGLE_ALPHA_SOURCE_TEXTURE_NAMES = {{
		"tex_alpha_02", "tex_uvnoise_01"
	}};
inline constexpr std::array<std::string_view, 4u>
	EFFECT_ARTIST_SPLA01_SOURCE_TEXTURE_NAMES = {{
		"00_mapsource", "06.map", "02_specmap_b", "01_specmap_a"
	}};
inline constexpr std::array<std::string_view, 4u>
	EFFECT_ARTIST_SPLA05_SOURCE_TEXTURE_NAMES = {{
		"06.map_a", "06.map", "00.map_alpha", "01.specmap_a"
	}};
inline constexpr std::array<std::string_view, 3u>
	EFFECT_ARTIST_TWINKLE_SOURCE_TEXTURE_NAMES = {{
		"twinkle_tex", "twinkle_tex_01", "mask_tex"
	}};
inline constexpr std::array<std::string_view, 4u>
	EFFECT_ARTIST_FLUID01_SOURCE_TEXTURE_NAMES = {{
		"normal_tex", "alpha_tex", "emissive_tex", "subuv_alpha_tex"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_ARTIST_WORLDOFFSET01_SOURCE_TEXTURE_NAMES = {{
		"uv_noise_tex_02", "uv_noise_tex_01", "emissivee_wave_texture",
		"worldoffset_emissive_tex_01", "umodel_dependency"
	}};
inline constexpr std::array<std::string_view, 6u>
	EFFECT_ARTIST_MAKEFLOW01_SOURCE_TEXTURE_NAMES = {{
		"diff_tex2", "diff_tex1", "color_tex", "opacity_tex", "mask_tex",
		"umodel_dependency"
	}};
inline constexpr std::array<std::string_view, 1u>
	EFFECT_ARTIST_LENSFLARE01_SOURCE_TEXTURE_NAMES = {{
		"lensflaretexture"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_ARTIST_WORLDOFFSET02_SOURCE_TEXTURE_NAMES = {{
		"alpha_texture2", "alpha_texture1_mask", "uv_noise_texture",
		"emissive_tex_01", "emissive_tex_02"
	}};
inline constexpr std::array<std::string_view, 3u>
	EFFECT_ARTIST_MM_FLUID01_SOURCE_TEXTURE_NAMES = {{
		"transition texture", "emissive_tex", "uv_noise_01_tex"
	}};
inline constexpr std::array<std::string_view, 2u>
	EFFECT_ARTIST_MM_DISSOLVE01_SOURCE_TEXTURE_NAMES = {{
		"dissolve_noise_tex", "emissive_tex"
	}};
inline constexpr std::array<std::string_view, 4u>
	EFFECT_ARTIST_RING07_SOURCE_TEXTURE_NAMES = {{
		"00.map_b", "06.map", "31.map_e", "umodel_dependency"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_ARTIST_RINGMASTER01_SOURCE_TEXTURE_NAMES = {{
		"22.map_a", "06.map", "01.specmap", "02.map_e", "12.map_f"
	}};
inline constexpr std::array<std::string_view, 1u>
	EFFECT_ARTIST_LIGHTFLARE01_SOURCE_TEXTURE_NAMES = {{
		"lensflaretexture"
	}};
inline constexpr std::array<std::string_view, 3u>
	EFFECT_GLASSHOLE02_SOURCE_TEXTURE_NAMES = {{
		"aura_texture", "cracknormal_tex", "in_hole_texture"
	}};
inline constexpr std::array<std::string_view, 5u>
	EFFECT_FLUIDNINJA01_SOURCE_TEXTURE_NAMES = {{
		"diff_tex", "flow_1_tex", "flow_2_tex", "mask_tex",
		"opacity_tex"
	}};
inline constexpr std::array<std::string_view, 2u>
	EFFECT_CUSTOMPARTICLE01_SOURCE_TEXTURE_NAMES = {{
		"diff_tex", "a_noise_01_tex"
	}};
inline constexpr std::array<std::string_view, 6u>
	EFFECT_CRACKHOLEV2_SOURCE_TEXTURE_NAMES = {{
		"01.map_e", "06.map_f", "06.map", "mask_noisemap",
		"mask_tex_l", "mask_tex_r"
	}};

inline constexpr std::array<std::string_view, 23u>
	EFFECT_SOURCE_DYNAMIC_PARAMETER_SEMANTICS = {{
		"unbound",
		"opacity",
		"emissive",
		"dissolve",
		"uv_pan",
		"distortion",
		"radial_size",
		"mask_a_offset",
		"mask_b_offset",
		"mask_a_distort",
		"mask_b_distort",
		"mask_a_pan",
		"flow_strength",
		"mask_b_pan",
		"diffuse_pan",
		"missile_alpha_pan",
		"missile_noise_strength",
		"missile_noise_pan",
		"missile_dissolve",
		"water_alpha_pan",
		"water_noise_pan",
		"water_dissolve",
		"water_noise_strength"
	}};

enum class EFFECT_STRICT_TYPED_SOURCE_PROFILE : uint8_t
{
	NONE,
	MISSILETRAIL,
	WATERTRAIL,
	LINEARFLOW_02,
	MAKEFLOW_02,
	MAKEFLOW_03,
	MAKEFLOW_03_SPRITE,
	RING_01,
	PARTICLETRAIL_01,
	PARTICLE_MASTER_01,
	SPRITEWAVE_01,
	PARTICLETRAIL_SINGLE_ALPHA,
	ARTIST_SPLA01,
	ARTIST_SPLA05,
	ARTIST_TWINKLE,
	ARTIST_FLUID01,
	ARTIST_WORLDOFFSET01,
	ARTIST_MAKEFLOW01,
	ARTIST_LENSFLARE01,
	ARTIST_WORLDOFFSET02,
	ARTIST_MM_FLUID01,
	ARTIST_MM_DISSOLVE01,
	ARTIST_RING07,
	ARTIST_RINGMASTER01,
	ARTIST_LIGHTFLARE01,
	FLOWRIBBON01,
	SIMPLE01,
	SIMPLE02,
	GLASSHOLE02,
	FLUIDNINJA01,
	CUSTOMPARTICLE01,
	CRACKHOLEV2,
	MM_BASIC01,
	FLOWTRAIL01,
	MM_LIGHT01,
	END
};

/* This is deliberately an exact compiler-evidence identity join.  It does
   not mutate the authored runtime profile or its admission status; the
   renderer may use it only to select an effective in-memory evaluator for an
   existing grouped baseline. */
inline EFFECT_STRICT_TYPED_SOURCE_PROFILE Resolve_EffectStrictTypedSourceProfile(
	const std::string_view strSourceMaterialPath,
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	if (!Source.bEnabled ||
		Source.strRuntimeShaderProfileId !=
			"effect.ue3.grouped-translucent.v1")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_m_00.fx_mi.fx_m_pa_missiletrail_01_17_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.m.me.trail.02.tr.8742928bef93" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_m_me_trail_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MISSILETRAIL;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::WATERTRAIL;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_j_00.fx_mi.fx_j_me_linearflow_02_12_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.linearflow.02.tr.ac0bdcfd95f7" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_j_00.fx_m.fx_j_pa_linearflow_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::LINEARFLOW_02;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.02.fx.m.fx.k.me.makeflow.02.tr.5059859991f8" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_02;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.k.00.fx.m.fx.k.me.makeflow.03.tr.6e5c0dd36299" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_k_00.fx_m.fx_k_me_makeflow_03_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.makeflow.03.tr.fdf195f33a6c" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_k_00.fx_m.fx_k_pa_makeflow_03_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03_SPRITE;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_s_00.fx_mi.fx_s_pa_ring_01_1_ts_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.n.pa.ring.05.tr.f23a0bf40d4e" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_n_pa_ring_05_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::RING_01;
	}
	if ((strSourceMaterialPath ==
			"fx_m_mi_s_00.fx_mi.fx_s_pa_trail_03_01_tr" ||
		 strSourceMaterialPath ==
			"fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_6_tr") &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.trail.01.tr.2372f1d945ed" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_mi.fx_m_pa_trail_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_01;
	}
	if (((Source.strProfileId ==
			"ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_00.fx_m.fx_d_pa_master_01_tr") ||
		 (Source.strProfileId ==
			"ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.ad.2bf3a6febe9f" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_00.fx_m.fx_d_pa_master_01_ad")))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01;
	}
	if (((Source.strProfileId ==
			"ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr") ||
		 (Source.strProfileId ==
			"ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.ad.caabbddf8b55" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad")))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::SPRITEWAVE_01;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_o_00.fx_mi.fx_o_pa_trail_01_01_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.trail.01.tr.2372f1d945ed" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_mi.fx_m_pa_trail_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_SINGLE_ALPHA;
	}
	if (Source.strProfileId ==
			"ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.spla.01.tr.316c6e8d71a0" &&
		Source.strParentMaterialPath ==
			"bfx_m_mi_00.bfx_m.bfx_d_pa_spla_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA01;
	}
	if (Source.strProfileId ==
			"ue3.material.bfx.m.mi.00.bfx.m.bfx.d.pa.spla.05.tr.7274940fdb66" &&
		Source.strParentMaterialPath ==
			"bfx_m_mi_00.bfx_m.bfx_d_pa_spla_05_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA05;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.01.fx.m.fx.e.pa.twinkle.01.ad.72979792ae2b" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_01.fx_m.fx_e_pa_twinkle_01_ad")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_TWINKLE;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.05.fx.m.fx.e.pa.fluid.01.tr.fba820645bb0" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_05.fx_m.fx_e_pa_fluid_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_FLUID01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.m.pa.worldoffset.01.tr.692cba4c40ad" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_m_pa_worldoffset_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_WORLDOFFSET01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.makeflow.01.tr.13159d5d398d" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_k_00.fx_m.fx_k_pa_makeflow_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MAKEFLOW01;
	}
	/* Both package paths resolve to the same parent object
	   fx_c_pa_lensflare_01_ad: identical 46 expression slots, zero static
	   switches, the single lensflaretexture parameter bound to
	   fx_c_glow_006 and the same three scalar defaults.  Only the child
	   package differs, so the equation and lane layout are shared. */
	if ((Source.strProfileId ==
			"ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af" &&
		 Source.strParentMaterialPath ==
			"fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad") ||
		(Source.strProfileId ==
			"ue3.material.fx.m.fx.c.pa.lensflare.01.ad.ed326b13c7b3" &&
		 Source.strParentMaterialPath ==
			"fx_m.fx_c_pa_lensflare_01_ad"))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_LENSFLARE01;
	}
	/* fx_mm_basic_01 is one master material authored in an additive and a
	   translucent variant.  Both expose the same four texture parameters
	   (emissive_tex, alpha_tex, uv_noise_01_tex, uv_noise_02_tex) and the
	   same scalar surface, so they share one equation; the blend belongs
	   to the element render profile, not to the family. */
	if ((Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99" &&
		 Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_basic_01_ad") ||
		(Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.tr.ce17b96d1b77" &&
		 Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_basic_01_tr"))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_BASIC01;
	}
	/* fx_k_me_flowtrail_01_ts_tr keeps the diff, opacity and noise lanes in
	   three separate UV domains with their own tiling, centre and rotation.
	   The grouped path has one shared UV scale, so a trail authored with
	   diff v-tiling 0.2 against opacity v-tiling 1.0 cannot be expressed. */
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.02.fx.m.fx.k.me.flowtrail.01.ts.tr.bc0628267aaa" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWTRAIL01;
	}
	/* fx_mm_light_01 is the smallest master material in the corpus: one
	   emissive lane, no scalar and no vector parameter.  Its output is what
	   fx_mm_simple_01 already computes once every simple_01 input is neutral,
	   so the two share the evaluator and only the carrier blend differs
	   between the additive and translucent variants. */
	if ((Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.ad.f431613b2bdf" &&
		 Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_light_01_ad") ||
		(Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.light.01.tr.8a6435f0c4e0" &&
		 Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_light_01_tr"))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_LIGHT01;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_w_00.mi.fx_w_pa_worldoffset_02_14_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.mi.fx.m.pa.worldoffset.02.tr.6507c4b13a9b" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_WORLDOFFSET02;
	}
	if ((strSourceMaterialPath ==
			"fx_m_mi_o_00.fx_mi.fx_o_me_fd_01_3_ts_tr" ||
		 strSourceMaterialPath ==
			"fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr") &&
		Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.fluid.01.tr.99f00cf3e57f" &&
		Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MM_FLUID01;
	}
	if (strSourceMaterialPath ==
			"fx_m_mi_01.fx_mi.fx_e_pa_fd_04_1_tr" &&
		Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.dissolve.01.tr.a799c9636783" &&
		Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_dissolve_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MM_DISSOLVE01;
	}
	if (((Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.ad.82e9116584b2" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_d_pa_ring_07_ad") ||
		 (Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ring.07.tr.ac51b180263c" &&
		  Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_d_pa_ring_07_tr")))
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_RING07;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.03.fx.m.fx.d.pa.ringmaster.01.tr.5629c8a9bc12" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_03.fx_m.fx_d_pa_ringmaster_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_RINGMASTER01;
	}
	if (Source.strProfileId ==
			"ue3.material.bfx.m.mi.00.bfx.m.bfx.c.pa.lightflare.01.ddt.ad.33b058471d6a" &&
		Source.strParentMaterialPath ==
			"bfx_m_mi_00.bfx_m.bfx_c_pa_lightflare_01_ddt_ad")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_LIGHTFLARE01;
	}
	if ((strSourceMaterialPath ==
			"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_01_tr" ||
		 strSourceMaterialPath ==
			"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr") &&
		Source.strProfileId ==
			"ue3.material.fx.m.fx.k.flowrib.01.tr.0ca247309b89" &&
		Source.strParentMaterialPath == "fx_m.fx_k_flowrib_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2" &&
		Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_simple_01_ad")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.02.tr.68704f814ab0" &&
		Source.strParentMaterialPath ==
			"fx_mastermaterial.fx_mm.fx_mm_simple_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE02;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::GLASSHOLE02;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.fluidninja.01.tr.534340d78128" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_k_00.fx_m.fx_k_pa_fluidninja_01_tr")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLUIDNINJA01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.customparticle.01.ad.e6b959010967" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_j_00.fx_m.fx_j_pa_customparticle_01_ad")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::CUSTOMPARTICLE01;
	}
	if (Source.strProfileId ==
			"ue3.material.fx.m.mi.k.00.fx.m.fx.k.crackholev2.01.3aac97e0fcad" &&
		Source.strParentMaterialPath ==
			"fx_m_mi_k_00.fx_m.fx_k_crackholev2_01")
	{
		return EFFECT_STRICT_TYPED_SOURCE_PROFILE::CRACKHOLEV2;
	}
	return EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE;
}

inline bool_t Has_EffectFlowRibbon01TrailContract(
	const EFFECT_ELEMENT_DESC& Element)
{
	if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
		!Element.SourceRecipe.bEnabled ||
		Element.SourceRecipe.strRendererShape != "ribbon" ||
		Element.Material.eRenderProfile !=
			EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ||
		Resolve_EffectStrictTypedSourceProfile(
			Element.Material.strSourceMaterialPath,
			Element.Material.SourceMaterial) !=
			EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01)
	{
		return false;
	}

	const auto HasUniqueModule = [&Element](const std::string_view ClassName)
	{
		return std::ranges::count_if(Element.SourceRecipe.Modules,
			[ClassName](const EFFECT_SOURCE_MODULE_DESC& Module)
			{
				return Module.strClassName == ClassName;
			}) == 1;
	};
	if (!HasUniqueModule("particlemodulesize") ||
		!HasUniqueModule("particlemodulecolor") ||
		!HasUniqueModule("particlemodulecolorscaleoverlife") ||
		!HasUniqueModule("particlemoduleparameterdynamic") ||
		!HasUniqueModule("particlemoduletypedataribbon"))
	{
		return false;
	}

	const auto Dynamic = std::ranges::find_if(Element.SourceRecipe.Modules,
		[](const EFFECT_SOURCE_MODULE_DESC& Module)
		{
			return Module.strClassName == "particlemoduleparameterdynamic";
		});
	static constexpr std::array<std::string_view, 4u> DYNAMIC_NAMES = {{
		"x_tiling", "y_tiling", "dissolve", "disort"
	}};
	for (size_t iLane = 0u; iLane < DYNAMIC_NAMES.size(); ++iLane)
	{
		const std::string Property = "dynamicparams[" +
			std::to_string(iLane) + "].paramname";
		if (std::ranges::count_if(Dynamic->Literals,
			[&Property, iLane](const EFFECT_SOURCE_LITERAL_DESC& Literal)
			{
				return Literal.strPropertyPath == Property &&
					Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::STRING &&
					Literal.strString == DYNAMIC_NAMES[iLane];
			}) != 1)
		{
			return false;
		}
	}

	const auto BindingMatches = [&Element](
		const std::string_view strSlotId,
		const std::string_view strAssetId)
	{
		return std::ranges::count_if(Element.ResourceBindings,
			[strSlotId, strAssetId](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == strSlotId &&
					Binding.strAssetId == strAssetId;
			}) == 1;
	};
	const auto NamedTextureMatches = [&Element](
		const std::string_view strName,
		const std::string_view strAssetId)
	{
		return std::ranges::count_if(
			Element.Material.SourceMaterial.Textures,
			[strName, strAssetId](const EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == strName &&
					Texture.strAssetId == strAssetId;
			}) == 1;
	};

	if (Element.Material.strSourceMaterialPath ==
		"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_03_tr")
	{
		return Element.ResourceBindings.size() == 3u &&
			Element.Material.SourceMaterial.Textures.size() == 3u &&
			BindingMatches("base",
				"Effect/Artist/Textures/fx_i_atypical_03_ycl.dds") &&
			BindingMatches("noise",
				"Effect/Artist/Textures/fx_l_environment_001.dds") &&
			BindingMatches("mask",
				"Effect/Artist/Textures/fx_d_noise_002.dds") &&
			NamedTextureMatches("tex_main",
				"Effect/Artist/Textures/fx_i_atypical_03_ycl.dds") &&
			NamedTextureMatches("colormap",
				"Effect/Artist/Textures/fx_l_environment_001.dds") &&
			NamedTextureMatches("flowtex",
				"Effect/Artist/Textures/fx_d_noise_002.dds");
	}

	if (Element.Material.strSourceMaterialPath !=
		"fx_m_mi_k_00.fx_mi.fx_k_flowrib_01_01_tr" ||
		Element.ResourceBindings.size() != 2u ||
		Element.Material.SourceMaterial.Textures.size() != 2u ||
		!BindingMatches("noise",
			"Effect/Artist/Textures/fx_d_noise_002.dds") ||
		!NamedTextureMatches("flowtex",
			"Effect/Artist/Textures/fx_d_noise_002.dds"))
	{
		return false;
	}

	constexpr std::string_view COMPILER_MAIN_TEXTURE =
		"Effect/Artist/Textures/fx_k_auraline_02.dds";
	if (BindingMatches("base", COMPILER_MAIN_TEXTURE) &&
		NamedTextureMatches("tex_main", COMPILER_MAIN_TEXTURE))
	{
		return true;
	}

	/* Artist E intentionally keeps this texture swap pinned to one stable
	   compiler occurrence.  fx_m_trail_010 has opaque cooked alpha, so the
	   FlowRibbon evaluator must consume its RGB luminance and the source
	   DynamicParameter dissolve lane; no generic alpha fallback is admitted. */
	constexpr std::string_view ARTIST_E_WHITE_TRAIL_ELEMENT_ID =
		"authored.source-particle.c6d4247396f641316d28767c";
	constexpr std::string_view ARTIST_E_WHITE_TRAIL_SOURCE_NODE =
		"authored-source-cascade-ribbon:effect.artist.skill.31480.unified|"
		"source:effect.artist.skill.31480.imported|"
		"element:fx_pc_sdm_00.par_o_flyinheaven_01_01."
		"particlespriteemitter_17";
	constexpr std::string_view PROJECT_TUNED_MAIN_TEXTURE =
		"Effect/Artist/Textures/fx_m_trail_010.dds";
	if (Element.strElementId != ARTIST_E_WHITE_TRAIL_ELEMENT_ID ||
		Element.strSourceNode != ARTIST_E_WHITE_TRAIL_SOURCE_NODE ||
		!BindingMatches("base", PROJECT_TUNED_MAIN_TEXTURE) ||
		!NamedTextureMatches("tex_main", PROJECT_TUNED_MAIN_TEXTURE) ||
		Element.AuthoringOverrides.ResourceBindings.size() != 2u)
	{
		return false;
	}
	const auto OverrideMatches = [&Element](
		const std::string_view strSlotId)
	{
		return std::ranges::count_if(
			Element.AuthoringOverrides.ResourceBindings,
			[strSlotId](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
			{
				return Override.strSlotId == strSlotId &&
					Override.strAssetId ==
						"Effect/Artist/Textures/fx_m_trail_010.dds" &&
					Override.strCompilerAssetId ==
						"Effect/Artist/Textures/fx_k_auraline_02.dds";
			}) == 1;
	};
	return OverrideMatches("base") &&
		OverrideMatches("sourceMaterialTexture:tex_main");
}

inline constexpr std::array<std::string_view, 4u>
	EFFECT_SOURCE_SUBUV_MODES = {{
		"none",
		"psuvim_random",
		"psuvim_linear_blend",
		"psuvim_linear_blend_random_flip_square"
	}};

inline constexpr std::array<EFFECT_MATERIAL_INPUT_SLOT_DESC, 8u>
	EFFECT_STANDARD_MATERIAL_INPUTS = {{
		{ "base", "Base", "g_BaseTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::BASE_TEXTURE },
		{ "noise", "Noise", "g_NoiseTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::NOISE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::NOISE_TEXTURE },
		{ "mask", "Mask", "g_MaskTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::MASK,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::MASK_TEXTURE },
		{ "emissive", "Emissive", "g_EmissiveTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE },
		{ "dissolve", "Dissolve", "g_DissolveTexture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE },
		{ "base2", "Base 2", "g_Base2Texture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE2,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::BASE2_TEXTURE },
		{ "mask2", "Mask 2", "g_Mask2Texture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::MASK2,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::MASK2_TEXTURE },
		{ "noise2", "Noise 2", "g_Noise2Texture",
			EFFECT_MATERIAL_INPUT_SEMANTIC::NOISE2,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE,
			EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE }
	}};

inline constexpr EFFECT_MATERIAL_TEMPLATE_DESC
	EFFECT_STANDARD_MATERIAL_TEMPLATE = {
		EFFECT_STANDARD_MATERIAL_TEMPLATE_ID,
		"effect.standard.hlsl.v1",
		EFFECT_STANDARD_MATERIAL_INPUTS.data(),
		EFFECT_STANDARD_MATERIAL_INPUTS.size()
	};

inline constexpr EFFECT_MATERIAL_TEMPLATE_DESC
	EFFECT_STANDARD_COLOR_V1_TEMPLATE = {
		EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID,
		"effect.standard-color.hlsl.v1",
		nullptr,
		0u
	};

inline constexpr EFFECT_MATERIAL_TEMPLATE_DESC
	EFFECT_SOURCE_MATERIAL_TEMPLATE = {
		EFFECT_SOURCE_MATERIAL_TEMPLATE_ID,
		"effect.source-material.ue3-profile-runtime.v1",
		EFFECT_STANDARD_MATERIAL_INPUTS.data(),
		EFFECT_STANDARD_MATERIAL_INPUTS.size()
	};

inline const EFFECT_MATERIAL_TEMPLATE_DESC* Find_EffectMaterialTemplate(
	const std::string_view strTemplateId)
{
	if (strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID)
		return &EFFECT_STANDARD_MATERIAL_TEMPLATE;
	if (strTemplateId == EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID)
		return &EFFECT_STANDARD_COLOR_V1_TEMPLATE;
	if (strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID)
		return &EFFECT_SOURCE_MATERIAL_TEMPLATE;
	return nullptr;
}

template <std::size_t Size>
inline bool_t Contains_EffectMaterialToken(
	const std::array<std::string_view, Size>& Tokens,
	const std::string_view strValue)
{
	for (const std::string_view strToken : Tokens)
	{
		if (strToken == strValue)
			return true;
	}
	return false;
}

inline bool_t Is_SupportedEffectSourceRuntimeShaderProfile(
	const std::string_view strProfileId)
{
	return Contains_EffectMaterialToken(
		EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS, strProfileId);
}

inline bool_t Is_SupportedEffectSourceDynamicParameterSemantic(
	const std::string_view strSemantic)
{
	return Contains_EffectMaterialToken(
		EFFECT_SOURCE_DYNAMIC_PARAMETER_SEMANTICS, strSemantic);
}

inline bool_t Is_SupportedEffectSourceSubUVMode(
	const std::string_view strMode)
{
	return Contains_EffectMaterialToken(EFFECT_SOURCE_SUBUV_MODES, strMode);
}

inline bool_t Is_EffectSourceLinearBlendSubUVMode(
	const std::string_view strMode)
{
	return strMode == "psuvim_linear_blend" ||
		strMode == "psuvim_linear_blend_random_flip_square";
}

inline bool_t Is_EffectSourceMaterialStagingSignatureEqual(
	const EFFECT_SOURCE_MATERIAL_DESC& Left,
	const EFFECT_SOURCE_MATERIAL_DESC& Right)
{
	if (Left.bEnabled != Right.bEnabled ||
		Left.strProfileId != Right.strProfileId ||
		Left.strRuntimeShaderProfileId != Right.strRuntimeShaderProfileId ||
		Left.strParentMaterialPath != Right.strParentMaterialPath ||
		Left.eStatus != Right.eStatus ||
		Left.DynamicParameterSemantics != Right.DynamicParameterSemantics ||
		Left.strSubUVMode != Right.strSubUVMode ||
		Left.Textures.size() != Right.Textures.size() ||
		Left.Scalars.size() != Right.Scalars.size() ||
		Left.Vectors.size() != Right.Vectors.size() ||
		Left.StaticSwitches.size() != Right.StaticSwitches.size())
	{
		return false;
	}
	for (std::size_t i = 0u; i < Left.Textures.size(); ++i)
	{
		const EFFECT_NAMED_TEXTURE_DESC& A = Left.Textures[i];
		const EFFECT_NAMED_TEXTURE_DESC& B = Right.Textures[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.strSourceObjectPath != B.strSourceObjectPath ||
			A.strAssetId != B.strAssetId ||
			A.eAddressU != B.eAddressU || A.eAddressV != B.eAddressV ||
			A.eColorSpace != B.eColorSpace ||
			A.strSamplingEvidence != B.strSamplingEvidence)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.Scalars.size(); ++i)
	{
		const EFFECT_NAMED_FLOAT_DESC& A = Left.Scalars[i];
		const EFFECT_NAMED_FLOAT_DESC& B = Right.Scalars[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.fValue != B.fValue)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.Vectors.size(); ++i)
	{
		const EFFECT_NAMED_FLOAT4_DESC& A = Left.Vectors[i];
		const EFFECT_NAMED_FLOAT4_DESC& B = Right.Vectors[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.vValue.x != B.vValue.x || A.vValue.y != B.vValue.y ||
			A.vValue.z != B.vValue.z || A.vValue.w != B.vValue.w)
		{
			return false;
		}
	}
	for (std::size_t i = 0u; i < Left.StaticSwitches.size(); ++i)
	{
		const EFFECT_NAMED_BOOL_DESC& A = Left.StaticSwitches[i];
		const EFFECT_NAMED_BOOL_DESC& B = Right.StaticSwitches[i];
		if (A.strName != B.strName || A.strGroup != B.strGroup ||
			A.bValue != B.bValue)
		{
			return false;
		}
	}
	return true;
}

inline bool_t Contains_EffectMaterialTokenNoCase(
	const std::string_view strValue,
	const std::string_view strToken)
{
	if (strToken.empty())
		return true;
	if (strToken.size() > strValue.size())
		return false;
	const auto LowerAscii = [](const char Character)
	{
		return Character >= 'A' && Character <= 'Z' ?
			static_cast<char>(Character + ('a' - 'A')) : Character;
	};
	for (std::size_t iOffset = 0u;
		iOffset + strToken.size() <= strValue.size(); ++iOffset)
	{
		bool_t bMatches = true;
		for (std::size_t iCharacter = 0u;
			iCharacter < strToken.size(); ++iCharacter)
		{
			if (LowerAscii(strValue[iOffset + iCharacter]) !=
				LowerAscii(strToken[iCharacter]))
			{
				bMatches = false;
				break;
			}
		}
		if (bMatches)
			return true;
	}
	return false;
}

inline bool_t Is_UnsafeEffectBaseTextureAssetId(
	const std::string_view strAssetId)
{
	return strAssetId.empty() ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "blankwhite") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "normal") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "bump") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "_n.dds") ||
		Contains_EffectMaterialTokenNoCase(strAssetId, "_n_");
}

inline bool_t Is_EffectManualMeshAuthoringContractSatisfied(
	const bool_t bHasMeshModel,
	const bool_t bHasBaseTexture,
	const bool_t bUnsafeBaseTexture)
{
	return bHasMeshModel && bHasBaseTexture && !bUnsafeBaseTexture;
}

inline bool_t Is_EffectManualMeshCreateReady(
	const std::string_view strEffectName,
	const bool_t bHasMeshModel,
	const bool_t bHasBaseTexture,
	const bool_t bUnsafeBaseTexture)
{
	return !strEffectName.empty() &&
		Is_EffectManualMeshAuthoringContractSatisfied(
			bHasMeshModel, bHasBaseTexture, bUnsafeBaseTexture);
}

enum EFFECT_GROUPED_MATERIAL_FLAGS : uint32_t
{
	EFFECT_GROUPED_MATERIAL_HAS_ALPHA = 1u << 0u,
	EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE = 1u << 1u,
	EFFECT_GROUPED_MATERIAL_HAS_NOISE = 1u << 2u,
	EFFECT_GROUPED_MATERIAL_HAS_DISTORTION = 1u << 3u,
	EFFECT_GROUPED_MATERIAL_HAS_DISSOLVE = 1u << 4u
};

struct EFFECT_GROUPED_TRANSLUCENT_CONSTANTS final
{
	// xy: UV scale, zw: UV panning per second.
	float4_t vUVScalePan = { 1.f, 1.f, 0.f, 0.f };
	// x: alpha strength, y: alpha power,
	// z: emissive strength, w: emissive power.
	float4_t vAlphaEmissive = { 1.f, 1.f, 1.f, 1.f };
	// x: noise UV strength, y: distortion strength,
	// z: dissolve threshold, w: dissolve hardness.
	float4_t vNoiseDissolve = { 0.f, 0.f, 0.f, 1.f };
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	uint32_t iFlags = 0u;
};

inline bool_t EffectMaterialParameterContains(
	const std::string_view strName,
	const std::string_view strGroup,
	const std::string_view strToken)
{
	return Contains_EffectMaterialTokenNoCase(strName, strToken) ||
		Contains_EffectMaterialTokenNoCase(strGroup, strToken);
}

inline bool_t EffectMaterialParameterIsStrength(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".str") ||
		Contains_EffectMaterialTokenNoCase(strName, "_str") ||
		Contains_EffectMaterialTokenNoCase(strName, "strength") ||
		Contains_EffectMaterialTokenNoCase(strName, "intensity") ||
		Contains_EffectMaterialTokenNoCase(strName, "density");
}

inline bool_t EffectMaterialParameterIsPower(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, "pow") ||
		Contains_EffectMaterialTokenNoCase(strName, "hardness");
}

inline bool_t EffectMaterialParameterIsAxisX(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".x") ||
		Contains_EffectMaterialTokenNoCase(strName, "_x") ||
		Contains_EffectMaterialTokenNoCase(strName, "scale_r") ||
		Contains_EffectMaterialTokenNoCase(strName, "uvscale_r");
}

inline bool_t EffectMaterialParameterIsAxisY(
	const std::string_view strName)
{
	return Contains_EffectMaterialTokenNoCase(strName, ".y") ||
		Contains_EffectMaterialTokenNoCase(strName, "_y") ||
		Contains_EffectMaterialTokenNoCase(strName, "scale_g") ||
		Contains_EffectMaterialTokenNoCase(strName, "uvscale_g");
}

inline int32_t EffectMaterialUVPriority(
	const std::string_view strName,
	const std::string_view strGroup)
{
	if (EffectMaterialParameterContains(strName, strGroup, "alpha") ||
		EffectMaterialParameterContains(strName, strGroup, "map_a") ||
		EffectMaterialParameterContains(strName, strGroup, "main") ||
		EffectMaterialParameterContains(strName, strGroup, "diff"))
	{
		return 3;
	}
	if (EffectMaterialParameterContains(strName, strGroup, "emiss"))
		return 2;
	return 1;
}

inline EFFECT_GROUPED_TRANSLUCENT_CONSTANTS
Build_EffectGroupedTranslucentConstants(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	EFFECT_GROUPED_TRANSLUCENT_CONSTANTS Result;
	int32_t iScaleXPriority = -1;
	int32_t iScaleYPriority = -1;
	int32_t iPanXPriority = -1;
	int32_t iPanYPriority = -1;
	int32_t iAlphaStrengthPriority = -1;
	int32_t iAlphaPowerPriority = -1;
	int32_t iEmissiveStrengthPriority = -1;
	int32_t iEmissivePowerPriority = -1;
	int32_t iTintPriority = -1;

	for (const EFFECT_NAMED_FLOAT_DESC& Scalar : Source.Scalars)
	{
		const std::string_view strName = Scalar.strName;
		const std::string_view strGroup = Scalar.strGroup;
		const bool_t bAlpha =
			EffectMaterialParameterContains(strName, strGroup, "alpha") ||
			EffectMaterialParameterContains(strName, strGroup, "mask") ||
			EffectMaterialParameterContains(strName, strGroup, "opacity") ||
			EffectMaterialParameterContains(strName, strGroup, "density");
		const bool_t bEmissive =
			EffectMaterialParameterContains(strName, strGroup, "emiss");
		const bool_t bNoise =
			EffectMaterialParameterContains(strName, strGroup, "noise") ||
			EffectMaterialParameterContains(strName, strGroup, "flow");
		const bool_t bDistortion =
			EffectMaterialParameterContains(strName, strGroup, "distort");
		const bool_t bDissolve =
			EffectMaterialParameterContains(strName, strGroup, "dissol");

		if (bAlpha)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_ALPHA;
		if (bEmissive)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE;
		if (bNoise)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_NOISE;
		if (bDistortion)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_DISTORTION;
		if (bDissolve)
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_DISSOLVE;

		const bool_t bUVScale =
			EffectMaterialParameterContains(strName, strGroup, "uvscale") ||
			EffectMaterialParameterContains(strName, strGroup, "uv_scale") ||
			EffectMaterialParameterContains(strName, strGroup, "tile");
		const bool_t bUVPan =
			EffectMaterialParameterContains(strName, strGroup, "pan");
		if (bUVScale || bUVPan)
		{
			const int32_t iPriority =
				EffectMaterialUVPriority(strName, strGroup);
			const bool_t bAxisX = EffectMaterialParameterIsAxisX(strName);
			const bool_t bAxisY = EffectMaterialParameterIsAxisY(strName);
			if (bUVScale && (!bAxisY || bAxisX) &&
				iPriority > iScaleXPriority)
			{
				Result.vUVScalePan.x = Scalar.fValue;
				iScaleXPriority = iPriority;
			}
			if (bUVScale && (!bAxisX || bAxisY) &&
				iPriority > iScaleYPriority)
			{
				Result.vUVScalePan.y = Scalar.fValue;
				iScaleYPriority = iPriority;
			}
			if (bUVPan && (!bAxisY || bAxisX) &&
				iPriority > iPanXPriority)
			{
				Result.vUVScalePan.z = Scalar.fValue;
				iPanXPriority = iPriority;
			}
			if (bUVPan && (!bAxisX || bAxisY) &&
				iPriority > iPanYPriority)
			{
				Result.vUVScalePan.w = Scalar.fValue;
				iPanYPriority = iPriority;
			}
		}

		if (bAlpha)
		{
			const int32_t iPriority =
				EffectMaterialParameterContains(strName, strGroup, "opacity") ? 4 :
				(EffectMaterialParameterContains(strName, strGroup, "alpha") ? 3 :
				(EffectMaterialParameterContains(strName, strGroup, "mask") ? 2 : 1));
			if (EffectMaterialParameterIsStrength(strName) &&
				iPriority > iAlphaStrengthPriority)
			{
				Result.vAlphaEmissive.x = Scalar.fValue;
				iAlphaStrengthPriority = iPriority;
			}
			if (EffectMaterialParameterIsPower(strName) &&
				iPriority > iAlphaPowerPriority)
			{
				Result.vAlphaEmissive.y = Scalar.fValue;
				iAlphaPowerPriority = iPriority;
			}
		}
		if (bEmissive)
		{
			const int32_t iPriority =
				Contains_EffectMaterialTokenNoCase(strGroup, "emiss") ? 2 : 1;
			if (EffectMaterialParameterIsStrength(strName) &&
				iPriority > iEmissiveStrengthPriority)
			{
				Result.vAlphaEmissive.z = Scalar.fValue;
				iEmissiveStrengthPriority = iPriority;
			}
			if (EffectMaterialParameterIsPower(strName) &&
				iPriority > iEmissivePowerPriority)
			{
				Result.vAlphaEmissive.w = Scalar.fValue;
				iEmissivePowerPriority = iPriority;
			}
		}
		if ((bNoise || bDistortion) &&
			EffectMaterialParameterIsStrength(strName))
		{
			if (bNoise)
				Result.vNoiseDissolve.x = Scalar.fValue;
			if (bDistortion)
				Result.vNoiseDissolve.y = Scalar.fValue;
		}
		if (bDissolve)
		{
			if (EffectMaterialParameterIsPower(strName))
				Result.vNoiseDissolve.w = Scalar.fValue;
			else if (EffectMaterialParameterIsStrength(strName) ||
				EffectMaterialParameterContains(strName, strGroup, "threshold") ||
				EffectMaterialParameterContains(strName, strGroup, "amount"))
			{
				Result.vNoiseDissolve.z = Scalar.fValue;
			}
		}
	}

	for (const EFFECT_NAMED_FLOAT4_DESC& Vector : Source.Vectors)
	{
		const bool_t bEmissive =
			EffectMaterialParameterContains(
				Vector.strName, Vector.strGroup, "emiss");
		const bool_t bColor = bEmissive ||
			EffectMaterialParameterContains(
				Vector.strName, Vector.strGroup, "color");
		if (!bColor)
			continue;
		const int32_t iPriority = bEmissive ? 2 : 1;
		if (iPriority <= iTintPriority)
			continue;
		Result.vTint = Vector.vValue;
		iTintPriority = iPriority;
		if (bEmissive)
		{
			Result.iFlags |= EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE;
			if (Vector.vValue.w > 1.f && iEmissiveStrengthPriority < 0)
				Result.vAlphaEmissive.z = Vector.vValue.w;
		}
	}
	return Result;
}

inline bool_t Is_EffectGroupedTranslucentResourceContractSatisfied(
	const EFFECT_GROUPED_TRANSLUCENT_CONSTANTS& Constants,
	const bool_t bSafeBase,
	const bool_t bHasMask,
	const bool_t bHasEmissive,
	const bool_t bHasDissolve)
{
	if (0u != (Constants.iFlags & EFFECT_GROUPED_MATERIAL_HAS_ALPHA) &&
		!bSafeBase && !bHasMask && !bHasDissolve)
	{
		return false;
	}
	if (0u != (Constants.iFlags & EFFECT_GROUPED_MATERIAL_HAS_EMISSIVE) &&
		!bSafeBase && !bHasEmissive)
	{
		return false;
	}
	return bSafeBase || bHasMask || bHasEmissive;
}

inline bool_t Is_EffectGroupedTranslucentResourceContractSatisfied(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const bool_t bSafeBase,
	const bool_t bHasMask,
	const bool_t bHasEmissive,
	const bool_t bHasDissolve)
{
	return Is_EffectGroupedTranslucentResourceContractSatisfied(
		Build_EffectGroupedTranslucentConstants(Source), bSafeBase,
		bHasMask, bHasEmissive, bHasDissolve);
}

inline bool_t Is_EffectFiniteProfileResourceContractSatisfied(
	const std::string_view strRuntimeShaderProfileId,
	const bool_t bSafeBase,
	const bool_t bHasNoise,
	const bool_t bHasMask,
	const bool_t bHasEmissive,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	if (strRuntimeShaderProfileId == "effect.ue3.shine.v1")
		return bSafeBase && bHasMask;
	if (strRuntimeShaderProfileId == "effect.ue3.blackline-aura.v1")
		return bHasMask && bHasDissolve;
	if (strRuntimeShaderProfileId == "effect.ue3.local-crack.v1")
		return false;
	if (strRuntimeShaderProfileId == "effect.ue3.slice.v1")
		return bSafeBase;
	if (strRuntimeShaderProfileId == EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID)
		return bSafeBase && bHasNoise && bHasMask && bHasDissolve && bHasMesh;
	if (strRuntimeShaderProfileId ==
		EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID)
	{
		return bSafeBase && bHasNoise && bHasMask && bHasEmissive &&
			bHasDissolve && bHasMesh;
	}
	if (strRuntimeShaderProfileId == EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID)
		return bSafeBase && bHasNoise && bHasMesh;
	if (strRuntimeShaderProfileId ==
		"effect.ue3.procedural-center-glow.v1")
	{
		return true;
	}
	return true;
}

inline bool_t Is_EffectLocalCrackResourceContractSatisfied(
	const bool_t bHasNormal,
	const bool_t bHasReflection,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	return bHasNormal && bHasReflection && bHasDissolve && bHasMesh;
}

template <std::size_t Size>
inline bool_t Has_EffectNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const std::array<std::string_view, Size>& RequiredNames)
{
	for (const std::string_view strRequiredName : RequiredNames)
	{
		const auto Iterator = std::find_if(
			Source.Textures.begin(), Source.Textures.end(),
			[strRequiredName](const EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == strRequiredName &&
					!Texture.strAssetId.empty();
			});
		if (Iterator == Source.Textures.end())
			return false;
	}
	return true;
}

inline const EFFECT_NAMED_TEXTURE_DESC* Find_EffectUniqueNamedTexture(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const std::string_view strName)
{
	const EFFECT_NAMED_TEXTURE_DESC* pMatch = nullptr;
	for (const EFFECT_NAMED_TEXTURE_DESC& Texture : Source.Textures)
	{
		if (Texture.strName != strName)
			continue;
		if (nullptr != pMatch)
			return nullptr;
		pMatch = &Texture;
	}
	return nullptr != pMatch && !pMatch->strAssetId.empty() ? pMatch : nullptr;
}

inline bool_t Is_EffectNamedTextureLaneUnique(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const std::string_view strName)
{
	return std::ranges::count_if(Source.Textures,
		[strName](const EFFECT_NAMED_TEXTURE_DESC& Texture)
		{
			return Texture.strName == strName;
		}) <= 1;
}

template <std::size_t Size>
inline bool_t Has_EffectUniqueNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const std::array<std::string_view, Size>& RequiredNames)
{
	for (const std::string_view strName : RequiredNames)
	{
		if (!Is_EffectNamedTextureLaneUnique(Source, strName) ||
			nullptr == Find_EffectUniqueNamedTexture(Source, strName))
		{
			return false;
		}
	}
	return true;
}

inline bool_t Has_EffectParticleMasterNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	for (const std::string_view strName :
		EFFECT_PARTICLE_MASTER_SOURCE_TEXTURE_NAMES)
	{
		if (!Is_EffectNamedTextureLaneUnique(Source, strName))
			return false;
	}
	// The portable Artist-F oracle and the ParticleMaster source graph both
	// make map_c the coverage owner.  The exact non-expanding map_a/map_b
	// operator is not recovered, so this bounded approximation deliberately
	// ignores those optional alpha-group lanes.  Accepting either dense noise
	// lane as an alternate carrier turns the whole sprite card opaque.
	const bool_t bHasAlphaCarrier =
		nullptr != Find_EffectUniqueNamedTexture(Source, "21.map_c");
	const bool_t bHasEmission =
		nullptr != Find_EffectUniqueNamedTexture(Source, "02.map_e") ||
		nullptr != Find_EffectUniqueNamedTexture(Source, "12.map_f");
	return bHasAlphaCarrier && bHasEmission;
}

inline bool_t Has_EffectSimple01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_SIMPLE01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_tex");
}

/* Only the emissive lane is required.  alpha_tex and the two uv_noise
   lanes are child-optional in this family and the equation gates them on
   the staged texture mask instead of rejecting the occurrence. */
inline bool_t Has_EffectMmBasic01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_MM_BASIC01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "alpha_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_01_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_02_tex");
}

/* diff and opacity are the two lanes the equation cannot run without.  The
   noise lane is child-optional and gated on the staged texture mask. */
inline bool_t Has_EffectFlowTrail01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_FLOWTRAIL01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "noise_tex");
}

inline bool_t Has_EffectSimple02NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_SIMPLE02_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectMakeFlowMeshNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_MAKEFLOW_MESH_SOURCE_TEXTURE_NAMES);
}

inline const EFFECT_NAMED_TEXTURE_DESC* Resolve_EffectMakeFlow03SpriteFlowTexture(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	if (!Is_EffectNamedTextureLaneUnique(Source, "flowtex") ||
		!Is_EffectNamedTextureLaneUnique(Source, "umodel_dependency"))
	{
		return nullptr;
	}
	const EFFECT_NAMED_TEXTURE_DESC* pNamedFlow =
		Find_EffectUniqueNamedTexture(Source, "flowtex");
	const EFFECT_NAMED_TEXTURE_DESC* pDependencyFlow =
		Find_EffectUniqueNamedTexture(Source, "umodel_dependency");
	/* The parent graph owns one non-parameter flow texture.  A materialized
	   role may call it flowtex; legacy source evidence exposes the same single
	   dependency lane.  Exactly one representation is required so an unrelated
	   dependency can never be selected by ordering. */
	return (nullptr != pNamedFlow) != (nullptr != pDependencyFlow) ?
		(nullptr != pNamedFlow ? pNamedFlow : pDependencyFlow) : nullptr;
}

inline bool_t Has_EffectMakeFlow03SpriteNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_MAKEFLOW03_SPRITE_SOURCE_TEXTURE_NAMES) &&
		nullptr != Resolve_EffectMakeFlow03SpriteFlowTexture(Source);
}

inline const EFFECT_NAMED_TEXTURE_DESC* Resolve_EffectSpriteWaveCarrierTexture(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	if (const EFFECT_NAMED_TEXTURE_DESC* pMain =
		Find_EffectUniqueNamedTexture(Source, "maintex"))
	{
		return pMain;
	}
	// Some cooked instances expose the sampled carrier only through the
	// stable dependency lane.  This remains an approximate evaluator and must
	// never change admission/exactness.
	return Find_EffectUniqueNamedTexture(Source, "umodel_dependency");
}

inline bool_t Has_EffectSpriteWaveNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	for (const std::string_view strName :
		EFFECT_SPRITEWAVE_SOURCE_TEXTURE_NAMES)
	{
		if (!Is_EffectNamedTextureLaneUnique(Source, strName))
			return false;
	}
	if (!Is_EffectNamedTextureLaneUnique(Source, "umodel_dependency"))
		return false;
	return nullptr != Resolve_EffectSpriteWaveCarrierTexture(Source) &&
		(nullptr != Find_EffectUniqueNamedTexture(Source, "uv_noise_tex") ||
		 nullptr != Find_EffectUniqueNamedTexture(Source, "uv_noise_tex_02")) &&
		nullptr != Find_EffectUniqueNamedTexture(Source, "emissivetex02");
}

inline bool_t Has_EffectParticleTrailSingleAlphaNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	for (const std::string_view strName :
		EFFECT_PARTICLETRAIL_SINGLE_ALPHA_SOURCE_TEXTURE_NAMES)
	{
		if (!Is_EffectNamedTextureLaneUnique(Source, strName) ||
			nullptr == Find_EffectUniqueNamedTexture(Source, strName))
		{
			return false;
		}
	}
	return true;
}

inline bool_t Has_EffectArtistSpla01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_SPLA01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistSpla05NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_SPLA05_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistTwinkleNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_TWINKLE_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "add_emissive_tex");
}

inline bool_t Has_EffectArtistFluid01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_FLUID01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "specular_tex");
}

inline bool_t Has_EffectArtistWorldOffset01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_WORLDOFFSET01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistMakeFlow01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_MAKEFLOW01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistLensFlare01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_LENSFLARE01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectGlasshole02NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_GLASSHOLE02_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectFluidNinja01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_FLUIDNINJA01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectCustomParticle01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return nullptr != Find_EffectUniqueNamedTexture(Source, "diff_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "a_noise_01_tex");
}

inline bool_t Has_EffectCrackholeV2NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_CRACKHOLEV2_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistWorldOffset02NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_WORLDOFFSET02_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistMmFluid01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_MM_FLUID01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_02_tex");
}

inline bool_t Has_EffectArtistMmDissolve01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_MM_DISSOLVE01_SOURCE_TEXTURE_NAMES) &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_01_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "uv_noise_02_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "alpha_tex") &&
		Is_EffectNamedTextureLaneUnique(Source, "lamp_tex");
}

inline const EFFECT_NAMED_TEXTURE_DESC* Resolve_EffectArtistRing07NewAlphaTexture(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	if (!Is_EffectNamedTextureLaneUnique(Source, "04.map_anew") ||
		!Is_EffectNamedTextureLaneUnique(Source, "06.map_anew"))
	{
		return nullptr;
	}
	const EFFECT_NAMED_TEXTURE_DESC* pMap04 =
		Find_EffectUniqueNamedTexture(Source, "04.map_anew");
	const EFFECT_NAMED_TEXTURE_DESC* pMap06 =
		Find_EffectUniqueNamedTexture(Source, "06.map_anew");
	return (nullptr != pMap04) != (nullptr != pMap06) ?
		(nullptr != pMap04 ? pMap04 : pMap06) : nullptr;
}

inline bool_t Has_EffectArtistRing07NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_RING07_SOURCE_TEXTURE_NAMES) &&
		nullptr != Resolve_EffectArtistRing07NewAlphaTexture(Source);
}

inline bool_t Has_EffectArtistRingMaster01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_RINGMASTER01_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectArtistLightFlare01NamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_ARTIST_LIGHTFLARE01_SOURCE_TEXTURE_NAMES);
}

/* Compatibility overload for profiles that do not own an emissive lane.
   Exact missile-trail callers must use the seven-argument contract above. */
inline bool_t Is_EffectFiniteProfileResourceContractSatisfied(
	const std::string_view strRuntimeShaderProfileId,
	const bool_t bSafeBase,
	const bool_t bHasNoise,
	const bool_t bHasMask,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	return Is_EffectFiniteProfileResourceContractSatisfied(
		strRuntimeShaderProfileId, bSafeBase, bHasNoise, bHasMask, false,
		bHasDissolve, bHasMesh);
}

inline bool_t Has_EffectLinearFlowNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectNamedTextureContract(
		Source, EFFECT_LINEARFLOW_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectBlacklineNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectNamedTextureContract(
		Source, EFFECT_BLACKLINE_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectLocalCrackNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	for (const std::string_view strRequiredName :
		EFFECT_LOCAL_CRACK_SOURCE_TEXTURE_NAMES)
	{
		bool_t bFound = false;
		for (const EFFECT_NAMED_TEXTURE_DESC& Texture : Source.Textures)
		{
			if (Texture.strName == strRequiredName &&
				!Texture.strAssetId.empty() &&
				!Texture.strSamplingEvidence.empty() &&
				Texture.strSamplingEvidence != "legacy_default")
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return false;
	}
	return true;
}

inline bool_t Has_EffectWaterTrailNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectUniqueNamedTextureContract(
		Source, EFFECT_WATERTRAIL_SOURCE_TEXTURE_NAMES);
}

inline bool_t Has_EffectMissileTrailNamedTextureContract(
	const EFFECT_SOURCE_MATERIAL_DESC& Source)
{
	return Has_EffectNamedTextureContract(
		Source, EFFECT_MISSILETRAIL_SOURCE_TEXTURE_NAMES);
}

inline bool_t Is_EffectLegacyLocalCrackResourceContractSatisfied(
	const EFFECT_SOURCE_MATERIAL_DESC& Source,
	const bool_t bHasDissolve,
	const bool_t bHasMesh)
{
	return Source.Textures.empty() && bHasDissolve && bHasMesh;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const std::string_view strSlotId)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].strSlotId == strSlotId)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const std::string_view strTemplateId,
	const std::string_view strSlotId)
{
	const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
		Find_EffectMaterialTemplate(strTemplateId);
	return nullptr == pTemplate ? nullptr :
		Find_EffectMaterialInput(*pTemplate, strSlotId);
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const EFFECT_RESOURCE_SLOT eRuntimeSlot)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].eRuntimeSlot == eRuntimeSlot)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

inline const EFFECT_MATERIAL_INPUT_SLOT_DESC* Find_EffectMaterialInput(
	const EFFECT_MATERIAL_TEMPLATE_DESC& Template,
	const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
{
	for (std::size_t iInput = 0u; iInput < Template.iInputCount; ++iInput)
	{
		if (Template.pInputs[iInput].eSemantic == eSemantic)
			return &Template.pInputs[iInput];
	}
	return nullptr;
}

NS_END
