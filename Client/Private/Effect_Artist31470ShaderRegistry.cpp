#include "Effect_Artist31470ShaderRegistry.h"

#include <array>

namespace
{
	using namespace Client;

	using Fidelity = EFFECT_ARTIST31470_SHADER_FIDELITY;
	using Backend = EFFECT_ARTIST31470_SHADER_BACKEND;
	using Output = EFFECT_ARTIST31470_SHADER_OUTPUT_ROLE;
	using Renderer = EFFECT_RUNTIME_RENDERER_KIND;
	using Row = EFFECT_ARTIST31470_SHADER_REGISTRY_ROW;

	constexpr std::string_view VF_LOCAL = "flocalvertexfactory";
	constexpr std::string_view VF_LOCAL_DECAL = "flocaldecalvertexfactory";
	constexpr std::string_view VF_PARTICLE = "fparticlevertexfactory";
	constexpr std::string_view VF_UNRESOLVED = "unresolved";
	constexpr std::string_view VF_NONE = "";

	constexpr Output NO_OUTPUT = Output::NONE;
	constexpr Output SCENE_COLOR = Output::SCENE_COLOR_RT0;

	// EXACT_CACHE_DXBC_SEMANTIC_REPLAY means that recovered shader equations are
	// replayed by bounded HLSL. It does not assert raw DXBC binding or bytecode equality.
	constexpr std::array<Row, EFFECT_ARTIST31470_SHADER_REGISTRY_ROW_COUNT>
		ARTIST31470_SHADER_REGISTRY =
	{{
		{ 0u, "source-active-000", "fx_pc_sdm_07.par_v_sdm_ink_spw_01.particlespriteemitter_21", Renderer::SPRITE_PARTICLE, "material-recipe-a63f54d1380f34cc", "material-family-2174c88b2cbf6c72", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 8u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, "45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4", "456fc57bd455014b93b97e18c9390a4f", "4181e26cdef6e7b9a6839e13d393305bec43460a91c9d5878a5ba975885a1700", 0x07u },
		{ 1u, "source-active-001", "fx_pc_sdm_07.par_v_sdm_ink_spw_01.particlespriteemitter_1", Renderer::SPRITE_PARTICLE, "material-recipe-680e4619051fb2fe", "material-family-d0f9d7bb8b80fee0", Fidelity::UNRESOLVED_FAIL_CLOSED, Backend::NONE, 0u, 1u, VF_PARTICLE, false, false, NO_OUTPUT, NO_OUTPUT },
		{ 2u, "source-active-002", "fx_pc_sdm_07.par_v_sdm_ink_spw_01.particlespriteemitter_6", Renderer::SPRITE_PARTICLE, "material-recipe-aa19ee0d487380ca", "material-family-ce6de128d3435d31", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 6u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 3u, "source-active-003", "fx_pc_sdm_07.par_v_sdm_ink_spw_01.particlespriteemitter_0", Renderer::CASCADE_RIBBON, "material-recipe-b508403ea55fedbc", "material-family-918f4ae1ed4d8a70", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 9u, 1u, VF_UNRESOLVED, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 4u, "source-active-004", "fx_pc_sdm_07.par_v_smd_onestroke_weapon_01.particlespriteemitter_6", Renderer::MESH_PARTICLE, "material-recipe-a4ee2b242b08bb39", "material-family-2c00ce5593538d7c", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 1u, 0u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 5u, "source-active-005", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_23", Renderer::SPRITE_PARTICLE, "material-recipe-533e2243c834e448", "material-family-b53107e635922285", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::RUNTIME_V2, 10u, 3u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, "5c01a1d9213bccf1ba8c9c02e1059f9dc834eeb5e60200f805b9a59f9026d573", "3e38be239225c7498a67fd9d3d400d24", "85ca55fab3b87dfb72a7fbbac841d52fff6eec2c526c4457608c7366af5673b5", 0u, "881a765fbf1c764fa5263eecacb0d5fc", "15b96080570506a854225adbc87b119123b6dea40901a8ce3dd8d3f9505de7aa" },
		{ 6u, "source-active-006", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_26", Renderer::SPRITE_PARTICLE, "material-recipe-533e2243c834e448", "material-family-b53107e635922285", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::RUNTIME_V2, 10u, 3u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, "5c01a1d9213bccf1ba8c9c02e1059f9dc834eeb5e60200f805b9a59f9026d573", "3e38be239225c7498a67fd9d3d400d24", "85ca55fab3b87dfb72a7fbbac841d52fff6eec2c526c4457608c7366af5673b5", 0u, "881a765fbf1c764fa5263eecacb0d5fc", "15b96080570506a854225adbc87b119123b6dea40901a8ce3dd8d3f9505de7aa" },
		{ 7u, "source-active-007", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_4", Renderer::MESH_PARTICLE, "material-recipe-96708f89604b9d71", "material-family-4e1ecdcff38a53d1", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 1u, 3u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x03u },
		{ 8u, "source-active-008", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_2", Renderer::MESH_PARTICLE, "material-recipe-96708f89604b9d71", "material-family-4e1ecdcff38a53d1", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 1u, 3u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x03u },
		{ 9u, "source-active-009", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_7", Renderer::MESH_PARTICLE, "material-recipe-03cc03b86c1a4c8f", "material-family-89af5c77d8e35f99", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::RUNTIME_V2, 3u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, "54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3", "70bf2a6e9bf4f0478cecbfc43c4e160f", "b16e274cfad5ba27b3be0f8c8bb4c1e663768ded79d4ddf119204fb8a1e9c6bb" },
		{ 10u, "source-active-010", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_13", Renderer::MESH_PARTICLE, "material-recipe-03cc03b86c1a4c8f", "material-family-89af5c77d8e35f99", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::RUNTIME_V2, 3u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, "54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3", "70bf2a6e9bf4f0478cecbfc43c4e160f", "b16e274cfad5ba27b3be0f8c8bb4c1e663768ded79d4ddf119204fb8a1e9c6bb" },
		{ 11u, "source-active-011", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_6", Renderer::MESH_PARTICLE, "material-recipe-daf220acad2b656e", "material-family-097bd8d9597721b5", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::RUNTIME_V2, 8u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, "468bfdf79d6dc23e741433c076e865a0dc985c19ebfc0e1519efd8ca20aad846", "39f7e63594b10f4a9237dc9eb19a1dfc", "7e8dbb706620c5ec6d991d99c70d6daa6b9df2258060796597c0678358b4f5e0" },
		{ 12u, "source-active-012", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_0", Renderer::MESH_PARTICLE, "material-recipe-4d8f47d8fa273079", "material-family-3919858495713fad", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 2u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x0fu },
		{ 13u, "source-active-013", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_28", Renderer::MESH_PARTICLE, "material-recipe-eca6507a183c9c39", "material-family-ce499c1d6d0fddcc", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::ARTIST_V4, 7u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, "60d8c69eaffb733d56f5284bf72a9d88904e48cc29fad4d6566c309f32eccbc2", "98a9cc70d9b42e45a66ea097407a0894", "4adf706f0819276f6a8577968a46605435bbd60c95c481fa80b3c0075481d9e8", 0x0fu },
		{ 14u, "source-active-014", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_30", Renderer::MESH_PARTICLE, "material-recipe-eca6507a183c9c39", "material-family-ce499c1d6d0fddcc", Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY, Backend::ARTIST_V4, 7u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, "60d8c69eaffb733d56f5284bf72a9d88904e48cc29fad4d6566c309f32eccbc2", "98a9cc70d9b42e45a66ea097407a0894", "4adf706f0819276f6a8577968a46605435bbd60c95c481fa80b3c0075481d9e8", 0x0fu },
		{ 15u, "source-active-015", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_3", Renderer::MESH_PARTICLE, "material-recipe-3d8700096e32aa5b", "material-family-0f9365f5179c729b", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 2u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x0fu },
		{ 16u, "source-active-016", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_15", Renderer::SPRITE_PARTICLE, "material-recipe-4070769760015ff3", "material-family-d0f9d7bb8b80fee0", Fidelity::UNRESOLVED_FAIL_CLOSED, Backend::NONE, 0u, 1u, VF_PARTICLE, false, false, NO_OUTPUT, NO_OUTPUT, "733e602dee649c6770d977bf315a489c1cb76ea9fabba625febb6870da34f33c", "13e34fcdce789f4eb7b751a705a350f9", "4f8d149d2e497bb481cb642e0d486d9560e14dca23359f568c4e6c5f272c3c5e", 0u, "fece8af6605ef5448261e47aa5699af9", "072befb401440d697aeb140384c30e8f2c4ea788789b30825112c0c1a7852dff" },
		{ 17u, "source-active-017", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_9", Renderer::MESH_PARTICLE, "material-recipe-76e78b77b8e6f2f1", "material-family-4e1ecdcff38a53d1", Fidelity::BOUNDED_EXPLICIT, Backend::FINITE_COMMON, 13u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 18u, "source-active-018", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_10", Renderer::MESH_PARTICLE, "material-recipe-77d6e867e3a6ff7e", "material-family-bcd3ca469af508c1", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 3u, 1u, VF_LOCAL, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x1fu },
		{ 19u, "source-active-019", "fx_pc_sdm_07.par_v_smd_onestroke_swing_01.particlespriteemitter_17", Renderer::SPRITE_PARTICLE, "material-recipe-aa19ee0d487380ca", "material-family-ce6de128d3435d31", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 6u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 20u, "source-active-020", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_43", Renderer::DECAL_PARTICLE, "material-recipe-a0a4fd34c2f220dc", "material-family-f1667adae7da4bdd", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 14u, 3u, VF_LOCAL_DECAL, false, true, SCENE_COLOR, NO_OUTPUT, "1f5c24844e2464446898632467277aa236fd04957def5b78c17cb0f63af80106", "ef68ae7aec8f94458ef2cbb3c6bafd2d", "d5a1d55021ff7e2a06e4de978e6850da56b9ff3ba6c7f68321f04852bd28ff1c", 0u, "5d79421dc8571c45aa49790f50274f51", "94072a22ef44ce0319bc9bd7915bded5f7ce94d9a5badde32def4fc01a4bff4d" },
		{ 21u, "source-active-021", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_6", Renderer::DECAL_PARTICLE, "material-recipe-a0a4fd34c2f220dc", "material-family-f1667adae7da4bdd", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 14u, 3u, VF_LOCAL_DECAL, false, true, SCENE_COLOR, NO_OUTPUT, "1f5c24844e2464446898632467277aa236fd04957def5b78c17cb0f63af80106", "ef68ae7aec8f94458ef2cbb3c6bafd2d", "d5a1d55021ff7e2a06e4de978e6850da56b9ff3ba6c7f68321f04852bd28ff1c", 0u, "5d79421dc8571c45aa49790f50274f51", "94072a22ef44ce0319bc9bd7915bded5f7ce94d9a5badde32def4fc01a4bff4d" },
		{ 22u, "source-active-022", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_15", Renderer::DECAL_PARTICLE, "material-recipe-99ec58031edc07b9", "material-family-472b1be487b92e70", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 7u, 3u, VF_UNRESOLVED, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 23u, "source-active-023", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_9", Renderer::SPRITE_PARTICLE, "material-recipe-ff6a10a52995006e", "material-family-9fce58ec032dee02", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 2u, 2u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 24u, "source-active-024", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_16", Renderer::SPRITE_PARTICLE, "material-recipe-6ac2c48a0ff3bfa0", "material-family-600687e6c2277444", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 11u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 25u, "source-active-025", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_7", Renderer::SPRITE_PARTICLE, "material-recipe-d75fcbbbae79bd5f", "material-family-61182c380df3a6cd", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 6u, 3u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x0fu },
		{ 26u, "source-active-026", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_17", Renderer::MESH_PARTICLE, "material-recipe-4b4c59364690a66d", "material-family-5fc89efe09353236", Fidelity::UNRESOLVED_FAIL_CLOSED, Backend::NONE, 0u, 0u, VF_LOCAL, false, false, NO_OUTPUT, NO_OUTPUT },
		{ 27u, "source-active-027", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_0", Renderer::SPRITE_PARTICLE, "material-recipe-2073fb45e643d1d5", "material-family-ee42f716afdf6145", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 13u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, "49cc0c15999c691875f7198b309c3f1f74ef57b0bbfd8f063d74d9c9648eda05", "67ed51e00d8a4247a540ccf8a05a6c8c", "7253dacbe9d1bf3b9410b0beeb341b8a4dfc47e8726b267c23c2eba8366189d3" },
		{ 28u, "source-active-028", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_10", Renderer::SPRITE_PARTICLE, "material-recipe-c9a33671bd6f57df", "material-family-fcc81a924169d053", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 12u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, "6d9b9545ede5f25f758e0514f12aa140a7b86412fc532751fde380921e3518e5" },
		{ 29u, "source-active-029", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_3", Renderer::SPRITE_PARTICLE, "material-recipe-e45b1f4c930b11df", "material-family-61182c380df3a6cd", Fidelity::BOUNDED_EXPLICIT, Backend::ARTIST_V4, 6u, 3u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT, {}, {}, {}, 0x0fu },
		{ 30u, "source-active-030", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_4", Renderer::SPRITE_PARTICLE, "material-recipe-9c82043000d98e73", "material-family-d0f9d7bb8b80fee0", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 5u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 31u, "source-active-031", "fx_pc_sdm_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_18", Renderer::SPRITE_PARTICLE, "material-recipe-aa19ee0d487380ca", "material-family-ce6de128d3435d31", Fidelity::BOUNDED_EXPLICIT, Backend::RUNTIME_V2, 6u, 1u, VF_PARTICLE, false, true, SCENE_COLOR, NO_OUTPUT },
		{ 32u, "source-active-032", "fx_post.fx_par.par_c_zoomblur_03.particlespriteemitter_0", Renderer::SCREEN_POST, "material-recipe-2348ec0702b07bc8", "material-family-b6660d647ef8c502", Fidelity::NON_CORE_FORBIDDEN, Backend::NONE, 0u, EFFECT_ARTIST31470_NO_PASS, VF_UNRESOLVED, false, false, NO_OUTPUT, NO_OUTPUT },
		{ 33u, "source-active-033", "fx_cm_01.distortion_onelayer.par_convatedisol_fsm_pushi.particlespriteemitter_21", Renderer::SPRITE_PARTICLE, "material-recipe-3992a9772b12abf2", "material-family-344b3aeb8eb1418d", Fidelity::UNRESOLVED_FAIL_CLOSED, Backend::NONE, 0u, 2u, VF_PARTICLE, false, false, NO_OUTPUT, NO_OUTPUT },
		{ 34u, "source-active-034", "fx_cm_02.light.par_mp_light_01.particlespriteemitter_2", Renderer::LIGHT_PARTICLE, "", "", Fidelity::NON_CORE_FORBIDDEN, Backend::NONE, 0u, EFFECT_ARTIST31470_NO_PASS, VF_NONE, false, false, NO_OUTPUT, NO_OUTPUT },
	}};

	// sourceEmitterPath is the exact raw UPK emitter identity. It is not the
	// same namespace as the runtime Effect element id: the latter is a bounded
	// authored identifier and occurrence 33 is intentionally truncated.
	constexpr std::array<std::string_view,
		EFFECT_ARTIST31470_SHADER_REGISTRY_ROW_COUNT>
		ARTIST31470_SOURCE_EMITTER_PATHS =
	{{
		"FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_21",
		"FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_1",
		"FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_6",
		"FX_PC_SDM_07.par_v_sdm_ink_spw_01.particlespriteemitter_0",
		"FX_PC_SDM_07.par_v_smd_onestroke_weapon_01.particlespriteemitter_6",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_23",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_26",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_4",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_2",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_7",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_13",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_6",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_0",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_28",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_30",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_3",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_15",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_9",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_10",
		"FX_PC_SDM_07.par_v_smd_onestroke_swing_01.particlespriteemitter_17",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_43",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_6",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_15",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_9",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_16",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_7",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_17",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_0",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_10",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_3",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_4",
		"FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_18",
		"FX_POST.fx_par.par_c_zoomblur_03.particlespriteemitter_0",
		"FX_CM_01.distortion_onelayer.par_convatedisol_fsm_pushinghit_01.particlespriteemitter_21",
		"FX_CM_02.light.par_mp_light_01.particlespriteemitter_2",
	}};

	constexpr bool Is_DrawFidelity(Fidelity eFidelity) noexcept
	{
		return eFidelity == Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY ||
			eFidelity == Fidelity::BOUNDED_EXPLICIT;
	}

	constexpr bool Has_ExpectedVertexFactory(const Row& Entry) noexcept
	{
		switch (Entry.eRenderer)
		{
		case Renderer::MESH_PARTICLE:
			return Entry.strSourceVertexFactoryCandidate == VF_LOCAL;
		case Renderer::SPRITE_PARTICLE:
			return Entry.strSourceVertexFactoryCandidate == VF_PARTICLE;
		case Renderer::DECAL_PARTICLE:
			return (Entry.iOrder == 20u || Entry.iOrder == 21u) ?
				Entry.strSourceVertexFactoryCandidate == VF_LOCAL_DECAL :
				Entry.strSourceVertexFactoryCandidate == VF_UNRESOLVED;
		case Renderer::CASCADE_RIBBON:
		case Renderer::SCREEN_POST:
			return Entry.strSourceVertexFactoryCandidate == VF_UNRESOLVED;
		case Renderer::LIGHT_PARTICLE:
			return Entry.strSourceVertexFactoryCandidate.empty();
		default:
			return false;
		}
	}

	constexpr uint32_t Expected_PassIndex(uint32_t iOrder) noexcept
	{
		switch (iOrder)
		{
		case 4u:
		case 26u:
			return 0u;
		case 0u:
		case 1u:
		case 2u:
		case 3u:
		case 9u:
		case 10u:
		case 11u:
		case 12u:
		case 13u:
		case 14u:
		case 15u:
		case 16u:
		case 17u:
		case 18u:
		case 19u:
		case 24u:
		case 27u:
		case 28u:
		case 30u:
		case 31u:
			return 1u;
		case 23u:
		case 33u:
			return 2u;
		case 5u:
		case 6u:
		case 7u:
		case 8u:
		case 20u:
		case 21u:
		case 22u:
		case 25u:
		case 29u:
			return 3u;
		case 32u:
		case 34u:
		default:
			return EFFECT_ARTIST31470_NO_PASS;
		}
	}

	constexpr uint32_t Expected_ArtistVisualV4TextureMask(
		uint32_t iOrder) noexcept
	{
		switch (iOrder)
		{
		case 0u:
			return 0x07u;
		case 7u:
		case 8u:
			return 0x03u;
		case 12u:
		case 13u:
		case 14u:
		case 15u:
		case 25u:
		case 29u:
			return 0x0fu;
		case 18u:
			return 0x1fu;
		default:
			return 0u;
		}
	}

	constexpr bool Has_ExpectedExactReplayIdentity(const Row& Entry) noexcept
	{
		if (Entry.eFidelity != Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY)
			return false;

		switch (Entry.iOrder)
		{
		case 5u:
		case 6u:
			return Entry.strEngineEqualityStaticSetSha256 ==
				"5c01a1d9213bccf1ba8c9c02e1059f9dc834eeb5e60200f805b9a59f9026d573" &&
				Entry.strRecoveredPixelShaderId ==
				"3e38be239225c7498a67fd9d3d400d24" &&
				Entry.strRecoveredPixelDxbcSha256 ==
				"85ca55fab3b87dfb72a7fbbac841d52fff6eec2c526c4457608c7366af5673b5" &&
				Entry.strRecoveredVertexShaderId ==
				"881a765fbf1c764fa5263eecacb0d5fc" &&
				Entry.strRecoveredVertexDxbcSha256 ==
				"15b96080570506a854225adbc87b119123b6dea40901a8ce3dd8d3f9505de7aa";
		case 9u:
		case 10u:
			return Entry.strEngineEqualityStaticSetSha256 ==
				"54a52a78d4c82bd42962193bcb3a64e28cce1275eb69c9affb1d3006478abcc3" &&
				Entry.strRecoveredPixelShaderId ==
				"70bf2a6e9bf4f0478cecbfc43c4e160f" &&
				Entry.strRecoveredPixelDxbcSha256 ==
				"b16e274cfad5ba27b3be0f8c8bb4c1e663768ded79d4ddf119204fb8a1e9c6bb";
		case 11u:
			return Entry.strEngineEqualityStaticSetSha256 ==
				"468bfdf79d6dc23e741433c076e865a0dc985c19ebfc0e1519efd8ca20aad846" &&
				Entry.strRecoveredPixelShaderId ==
				"39f7e63594b10f4a9237dc9eb19a1dfc" &&
				Entry.strRecoveredPixelDxbcSha256 ==
				"7e8dbb706620c5ec6d991d99c70d6daa6b9df2258060796597c0678358b4f5e0";
		case 13u:
		case 14u:
			return Entry.strEngineEqualityStaticSetSha256 ==
				"60d8c69eaffb733d56f5284bf72a9d88904e48cc29fad4d6566c309f32eccbc2" &&
				Entry.strRecoveredPixelShaderId ==
				"98a9cc70d9b42e45a66ea097407a0894" &&
				Entry.strRecoveredPixelDxbcSha256 ==
				"4adf706f0819276f6a8577968a46605435bbd60c95c481fa80b3c0075481d9e8";
		default:
			return false;
		}
	}

	constexpr bool Validate_Entry(const Row& Entry) noexcept
	{
		if (Entry.strOccurrenceId.empty() || Entry.strRuntimeElementId.empty() ||
			Entry.bNativeSelectionAdmitted ||
			Entry.bDrawAdmitted != Is_DrawFidelity(Entry.eFidelity) ||
			!Has_ExpectedVertexFactory(Entry) ||
			Entry.iCurrentPassIndex != Expected_PassIndex(Entry.iOrder) ||
			Entry.iExpectedArtistVisualV4TextureMask !=
				Expected_ArtistVisualV4TextureMask(Entry.iOrder) ||
			((Entry.eBackend == Backend::ARTIST_V4) !=
				(Entry.iExpectedArtistVisualV4TextureMask != 0u)))
		{
			return false;
		}

		if (Entry.eRenderer == Renderer::LIGHT_PARTICLE)
		{
			return Entry.strRecipeId.empty() && Entry.strFamilyId.empty() &&
				Entry.iCurrentPassIndex == EFFECT_ARTIST31470_NO_PASS &&
				!Entry.bDrawAdmitted && Entry.eBackend == Backend::NONE &&
				Entry.iOpcode == 0u && Entry.eSceneColorOutput == NO_OUTPUT &&
				Entry.eDistortionOutput == NO_OUTPUT;
		}

		if (Entry.strRecipeId.empty() || Entry.strFamilyId.empty())
			return false;

		const bool bActive000RecoveredEquation = Entry.iOrder == 0u;
		const bool bActive016RecoveredFailClosed = Entry.iOrder == 16u;
		const bool bActive020021RecoveredBounded =
			Entry.iOrder == 20u || Entry.iOrder == 21u;
		const bool bActive027RecoveredEquation = Entry.iOrder == 27u;
		const bool bActive028RecoveredEquation = Entry.iOrder == 28u;
		if (Entry.eFidelity == Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY)
		{
			if (!Has_ExpectedExactReplayIdentity(Entry))
				return false;
		}
		else if (bActive000RecoveredEquation)
		{
			if (Entry.strEngineEqualityStaticSetSha256 !=
					"45f9f31688792848e09af99f5a64410bef1bac4520e1f2c9707f5c31e5b7fdc4" ||
				Entry.strRecoveredPixelShaderId !=
					"456fc57bd455014b93b97e18c9390a4f" ||
				Entry.strRecoveredPixelDxbcSha256 !=
					"4181e26cdef6e7b9a6839e13d393305bec43460a91c9d5878a5ba975885a1700")
			{
				return false;
			}
			/* Exact PS/static-selector evidence is replayed by a bounded RT0-only
			   particle program. Native sampler/pass/fog/aux-MRT selection remains
			   unresolved, so native selection stays false. */
		}
		else if (bActive016RecoveredFailClosed)
		{
			/* The official same-cohort map uniquely recovers this BasePass PS and
			   its no-density SubUV/dynamic VF candidate.  Fidelity remains
			   unresolved because the PS consumes projected SceneDepth/fog and
			   auxiliary MRT carriers absent from the current particle ABI. */
			if (Entry.eFidelity != Fidelity::UNRESOLVED_FAIL_CLOSED ||
				Entry.eBackend != Backend::NONE || Entry.bDrawAdmitted ||
				Entry.strEngineEqualityStaticSetSha256 !=
					"733e602dee649c6770d977bf315a489c1cb76ea9fabba625febb6870da34f33c" ||
				Entry.strRecoveredPixelShaderId !=
					"13e34fcdce789f4eb7b751a705a350f9" ||
				Entry.strRecoveredPixelDxbcSha256 !=
					"4f8d149d2e497bb481cb642e0d486d9560e14dca23359f568c4e6c5f272c3c5e" ||
				Entry.strRecoveredVertexShaderId !=
					"fece8af6605ef5448261e47aa5699af9" ||
				Entry.strRecoveredVertexDxbcSha256 !=
					"072befb401440d697aeb140384c30e8f2c4ea788789b30825112c0c1a7852dff")
			{
				return false;
			}
		}
		else if (bActive020021RecoveredBounded)
		{
			/* The official same-cohort cache uniquely joins the MIC static set to
			   an FLocalDecal map and preserves this VS/PS bytecode identity.  V6
			   executes an explicitly bounded RT0 semantic replay through the typed
			   projection carrier; it does not bind the raw LocalDecal VF, native
			   projector/fog outputs, or auxiliary MRT2-5. */
			if (Entry.eFidelity != Fidelity::BOUNDED_EXPLICIT ||
				Entry.eBackend != Backend::RUNTIME_V2 || Entry.iOpcode != 14u ||
				Entry.bNativeSelectionAdmitted || !Entry.bDrawAdmitted ||
				Entry.strEngineEqualityStaticSetSha256 !=
					"1f5c24844e2464446898632467277aa236fd04957def5b78c17cb0f63af80106" ||
				Entry.strRecoveredPixelShaderId !=
					"ef68ae7aec8f94458ef2cbb3c6bafd2d" ||
				Entry.strRecoveredPixelDxbcSha256 !=
					"d5a1d55021ff7e2a06e4de978e6850da56b9ff3ba6c7f68321f04852bd28ff1c" ||
				Entry.strRecoveredVertexShaderId !=
					"5d79421dc8571c45aa49790f50274f51" ||
				Entry.strRecoveredVertexDxbcSha256 !=
					"94072a22ef44ce0319bc9bd7915bded5f7ce94d9a5badde32def4fc01a4bff4d")
			{
				return false;
			}
		}
		else if (bActive027RecoveredEquation)
		{
			if (Entry.strEngineEqualityStaticSetSha256 !=
					"49cc0c15999c691875f7198b309c3f1f74ef57b0bbfd8f063d74d9c9648eda05" ||
				Entry.strRecoveredPixelShaderId !=
					"67ed51e00d8a4247a540ccf8a05a6c8c" ||
				Entry.strRecoveredPixelDxbcSha256 !=
					"7253dacbe9d1bf3b9410b0beeb341b8a4dfc47e8726b267c23c2eba8366189d3")
			{
				return false;
			}
			/* The PS/static-set evidence is exact, but five possible source VFs
			   remain ambiguous. Keep this semantic replay bounded and native=false. */
		}
		else if (bActive028RecoveredEquation)
		{
			if (Entry.strEngineEqualityStaticSetSha256 !=
				"6d9b9545ede5f25f758e0514f12aa140a7b86412fc532751fde380921e3518e5" ||
			 !Entry.strRecoveredPixelShaderId.empty() ||
			 !Entry.strRecoveredPixelDxbcSha256.empty())
			{
			/* The recovered base-PS equation is bounded evidence; it does not
			   establish this occurrence's exact ShaderMap/DXBC join. Only the
			   engine-equality MIC static-set identity is admitted here. */
				return false;
			}
		}
		else if (!Entry.strEngineEqualityStaticSetSha256.empty() ||
			!Entry.strRecoveredPixelShaderId.empty() ||
			!Entry.strRecoveredPixelDxbcSha256.empty() ||
			!Entry.strRecoveredVertexShaderId.empty() ||
			!Entry.strRecoveredVertexDxbcSha256.empty())
		{
			return false;
		}

		if (Entry.bDrawAdmitted)
		{
			if (Entry.eBackend == Backend::NONE || Entry.iOpcode == 0u ||
				Entry.iCurrentPassIndex == EFFECT_ARTIST31470_NO_PASS)
			{
				return false;
			}

			return Entry.eSceneColorOutput == SCENE_COLOR &&
				Entry.eDistortionOutput == NO_OUTPUT;
		}

		return Entry.eSceneColorOutput == NO_OUTPUT &&
			Entry.eDistortionOutput == NO_OUTPUT;
	}

	constexpr bool Validate_RegistryTable() noexcept
	{
		size_t iExactCount = 0u;
		size_t iBoundedCount = 0u;
		size_t iUnresolvedCount = 0u;
		size_t iForbiddenCount = 0u;
		size_t iDrawCount = 0u;

		for (size_t i = 0u; i < ARTIST31470_SHADER_REGISTRY.size(); ++i)
		{
			const Row& Entry = ARTIST31470_SHADER_REGISTRY[i];
			if (Entry.iOrder != i || !Validate_Entry(Entry))
				return false;

			for (size_t j = i + 1u; j < ARTIST31470_SHADER_REGISTRY.size(); ++j)
			{
				if (Entry.strOccurrenceId ==
						ARTIST31470_SHADER_REGISTRY[j].strOccurrenceId ||
					Entry.strRuntimeElementId ==
						ARTIST31470_SHADER_REGISTRY[j].strRuntimeElementId ||
					ARTIST31470_SOURCE_EMITTER_PATHS[i] ==
						ARTIST31470_SOURCE_EMITTER_PATHS[j])
					return false;
			}
			if (ARTIST31470_SOURCE_EMITTER_PATHS[i].empty())
				return false;

			switch (Entry.eFidelity)
			{
			case Fidelity::EXACT_CACHE_DXBC_SEMANTIC_REPLAY:
				++iExactCount;
				break;
			case Fidelity::BOUNDED_EXPLICIT:
				++iBoundedCount;
				break;
			case Fidelity::UNRESOLVED_FAIL_CLOSED:
				++iUnresolvedCount;
				break;
			case Fidelity::NON_CORE_FORBIDDEN:
				++iForbiddenCount;
				break;
			default:
				return false;
			}

			if (Entry.bDrawAdmitted)
				++iDrawCount;
		}

		return iExactCount == 7u && iBoundedCount == 22u &&
			iUnresolvedCount == 4u && iForbiddenCount == 2u &&
			iDrawCount == 29u &&
			ARTIST31470_SHADER_REGISTRY[32u].eFidelity ==
				Fidelity::NON_CORE_FORBIDDEN &&
			ARTIST31470_SHADER_REGISTRY[34u].eFidelity ==
				Fidelity::NON_CORE_FORBIDDEN;
	}

	static_assert(Validate_RegistryTable());
}

std::span<const Client::EFFECT_ARTIST31470_SHADER_REGISTRY_ROW>
Client::Get_Artist31470ShaderRegistry() noexcept
{
	return ARTIST31470_SHADER_REGISTRY;
}

std::optional<Client::EFFECT_ARTIST31470_SHADER_REGISTRY_ROW>
Client::Find_Artist31470ShaderRegistry(
	uint32_t iOrder,
	std::string_view strOccurrenceId) noexcept
{
	if (iOrder >= ARTIST31470_SHADER_REGISTRY.size())
		return std::nullopt;

	const EFFECT_ARTIST31470_SHADER_REGISTRY_ROW& Entry =
		ARTIST31470_SHADER_REGISTRY[iOrder];
	if (Entry.iOrder != iOrder || Entry.strOccurrenceId != strOccurrenceId)
		return std::nullopt;

	return Entry;
}

bool_t Client::Validate_Artist31470ShaderRegistryOccurrence(
	uint32_t iOrder,
	std::string_view strOccurrenceId,
	std::string_view strRuntimeElementId,
	std::string_view strSourceEmitterPath,
	const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence) noexcept
{
	const std::optional<EFFECT_ARTIST31470_SHADER_REGISTRY_ROW> Entry =
		Find_Artist31470ShaderRegistry(iOrder, strOccurrenceId);
	return Entry.has_value() && Occurrence.Row.iOrder == iOrder &&
		Occurrence.Row.strId == strOccurrenceId &&
		Validate_Artist31470ShaderRegistryEmitterIdentity(iOrder,
			strOccurrenceId, strRuntimeElementId, strSourceEmitterPath) &&
		Occurrence.eRenderer == Entry->eRenderer &&
		Occurrence.strRecipeId == Entry->strRecipeId &&
		Occurrence.strFamilyId == Entry->strFamilyId;
}

bool_t Client::Validate_Artist31470ShaderRegistryEmitterIdentity(
	uint32_t iOrder,
	std::string_view strOccurrenceId,
	std::string_view strRuntimeElementId,
	std::string_view strSourceEmitterPath) noexcept
{
	const std::optional<EFFECT_ARTIST31470_SHADER_REGISTRY_ROW> Entry =
		Find_Artist31470ShaderRegistry(iOrder, strOccurrenceId);
	return Entry.has_value() && iOrder < ARTIST31470_SOURCE_EMITTER_PATHS.size() &&
		Entry->strRuntimeElementId == strRuntimeElementId &&
		ARTIST31470_SOURCE_EMITTER_PATHS[iOrder] == strSourceEmitterPath;
}

bool_t Client::Validate_Artist31470ShaderRegistry() noexcept
{
	return Validate_RegistryTable();
}
