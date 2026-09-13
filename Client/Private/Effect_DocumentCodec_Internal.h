#pragma once

#include "Effect_DocumentCodec.h"
#include "DataJson.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <sstream>

// Shared codec declarations only; each non-template function has one CPP owner.
namespace Client::EffectDocumentCodecDetail
{

	inline constexpr const char_t* EFFECT_DOCUMENT_SCHEMA =
		"lostark.effect-authoring";
	inline constexpr size_t MAX_RESOURCE_ID_BYTES = 512u;
	inline constexpr size_t MAX_ELEMENTS = 2048u;
	inline constexpr size_t MAX_MODEL_CUES = 16u;
	inline constexpr uint64_t MAX_DOCUMENT_PARTICLES = 8192u;
	inline constexpr uint64_t MAX_DOCUMENT_TRAIL_POINTS = 2048u;
	inline constexpr uint64_t MAX_DOCUMENT_AFTERIMAGES = 256u;
	inline constexpr uint64_t MAX_PORTABLE_SOURCE_EVENTS_PER_STEP = 4096u;
	inline constexpr size_t MAX_SOURCE_MODULES_PER_ELEMENT = 256u;
	inline constexpr size_t MAX_SOURCE_LITERALS_PER_MODULE = 1024u;
	inline constexpr size_t MAX_SOURCE_DISTRIBUTIONS_PER_MODULE = 128u;
	inline constexpr size_t MAX_SOURCE_BURSTS_PER_ELEMENT = 1024u;
	inline constexpr size_t MAX_SOURCE_PRESENTATION_PARAMETERS = 256u;
	inline constexpr size_t MAX_SOURCE_COVERAGE_PROPERTIES_PER_MODULE = 2048u;
	inline constexpr size_t MAX_SOURCE_LOCAL_REFERENCE_BINDINGS_PER_ELEMENT = 2048u;
	inline constexpr size_t MAX_SOURCE_TYPED_FIELDS_PER_REFERENCE = 256u;
	inline constexpr size_t MAX_AUTHORED_MATERIAL_TEXTURE_LANES = 6u;
	inline constexpr size_t MAX_AUTHORED_MATERIAL_SCALARS = 52u;
	inline constexpr size_t MAX_AUTHORED_MATERIAL_VECTORS = 8u;
	inline constexpr size_t MAX_AUTHORED_MATERIAL_COLORS = 2u;
	inline constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_HISTORIES = 16u;
	inline constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_PER_HISTORY = 4096u;
	inline constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_TOTAL = 16384u;
	inline constexpr f32_t MAX_AUTHORED_RUNTIME_EDGE_TIME_SECONDS = 30.f;
	inline constexpr f32_t MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM = 1'000'000.f;
	inline constexpr const char_t* EFFECT_SOURCE_PRESENTATION_SCHEMA =
		"lostark.effect-source-presentation";
	inline constexpr std::string_view WARLORD_17090_EFFECT_ASSET_ID =
		"effect.warlord.skill.17090.unified";
	inline constexpr std::string_view WARLORD_CHAIN_SOURCE_MATERIAL_PATH =
		"fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma";
	inline constexpr std::string_view WARLORD_CHAIN_BASE_ALIAS_ASSET_ID =
		"Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.dds";
	inline constexpr std::string_view WARLORD_CHAIN_SOURCE_PROFILE_ID =
		"ue3.material.fx.m.mi.00.fx.m.fx.d.me.chain.01.ma.a8a92d2a6abc";
	inline constexpr std::string_view WARLORD_CHAIN_PARENT_MATERIAL_PATH =
		"fx_m_mi_00.fx_m.fx_d_me_chain_01_ma";
	inline constexpr std::string_view WARLORD_CHAIN_RUNTIME_PROFILE_ID =
		"effect.ue3.grouped-translucent.v1";
	inline constexpr std::string_view WARLORD_CHAIN_06_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_06.wmodel";
	inline constexpr std::string_view WARLORD_CHAIN_07_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel";

	inline constexpr const char_t* KIND_TOKENS[] =
	{
		"mesh", "sprite", "particle", "decal", "trail", "light",
		"screenPost"
	};
	inline constexpr const char_t* COMPOSITION_LAYER_TOKENS[] =
	{
		"normal", "worldMark", "sceneBackdrop"
	};
	inline constexpr const char_t* ATTACHMENT_ORIENTATION_TOKENS[] =
	{
		"bone", "owner_yaw", "camera_view"
	};
	inline constexpr const char_t* AUTHORED_RUNTIME_CARRIER_KIND_TOKENS[] =
	{
		"cascadeRibbonV1", "animationTrailBakedEdgeV1",
		"lightBakedEdgeAttachmentV1", "cascadeBeamV1"
	};
	inline constexpr const char_t* AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS[] =
	{
		"bounded"
	};
	inline constexpr const char_t* AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS[] =
	{
		"firstEdge"
	};
	inline constexpr const char_t* AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS[] =
	{
		"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS"
	};
	inline constexpr const char_t* RENDERER_TYPE_TOKENS[] =
	{
		"standaloneMesh", "legacyStandaloneSprite", "meshParticle",
		"spriteParticle", "decalParticle", "animTrail", "cascadeRibbon",
		"lightParticle", "screenPost"
	};
	inline constexpr const char_t* SOURCE_SPACE_TOKENS[] =
	{
		"clientMetersV1", "ue3CascadeV1", "screenSpaceV1"
	};
	inline constexpr const char_t* MODEL_CUE_ALPHA_MODE_TOKENS[] =
	{
		"OPAQUE", "MASKED", "TRANSLUCENT"
	};
	inline constexpr const char_t* SOURCE_COVERAGE_STATUS_TOKENS[] =
	{
		"source_decoded", "deterministic_conversion", "metadata_only",
		"unresolved"
	};
	inline constexpr const char_t* DISTRIBUTION_PARAMETER_BINDING_TOKENS[] =
	{
		"none", "actionCue"
	};
	inline constexpr const char_t* SLOT_TOKENS[] =
	{
		"meshModel", "base", "noise", "mask", "emissive", "dissolve",
		"base2", "mask2", "noise2"
	};
	inline constexpr const char_t* PARTICLE_SPAWN_SHAPE_TOKENS[] =
	{
		"point", "sphere", "ring", "box"
	};
	inline constexpr const char_t* PARTICLE_SPAWN_DISTRIBUTION_TOKENS[] =
	{
		"random", "even"
	};
	inline constexpr const char_t* PARTICLE_ORIENTATION_MODE_TOKENS[] =
	{
		"fixed", "groundRadialOutward", "groundRadialInward",
		"groundTangentClockwise", "groundTangentCounterClockwise"
	};
	inline constexpr const char_t* PARTICLE_VELOCITY_MODE_TOKENS[] =
	{
		"fixed", "outward", "inward", "cone"
	};
	inline constexpr const char_t* RING_FILL_DIRECTION_TOKENS[] =
	{
		"innerToOuter", "outerToInner"
	};
	inline constexpr const char_t* LINEAR_REVEAL_AXIS_TOKENS[] =
	{
		"u", "v"
	};
	inline constexpr const char_t* DECAL_RECEIVER_MODE_TOKENS[] =
	{
		"allOpaque", "upwardSurfaces"
	};
	inline constexpr const char_t* PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS[] =
	{
		"rootLocal", "elementLocal"
	};
	inline constexpr const char_t* PROFILE_TOKENS[] =
	{
		"opaque_back_depth_write",
		"alpha_two_sided_depth_read",
		"additive_two_sided_depth_read",
		"alpha_one_sided_depth_read",
		"additive_one_sided_depth_read"
	};
	inline constexpr const char_t* SOURCE_MATERIAL_STATUS_TOKENS[] =
	{
		"source_exact",
		"runtime_exact",
		"reconstructed_profile",
		"unsupported",
		"missing_resource"
	};
	inline constexpr const char_t* SOURCE_BLEND_CLASS_TOKENS[] =
	{
		"unknown", "additive", "translucent", "masked"
	};
	inline constexpr const char_t* TEXTURE_ADDRESS_MODE_TOKENS[] =
	{
		"wrap", "clamp"
	};
	inline constexpr const char_t* TEXTURE_COLOR_SPACE_TOKENS[] =
	{
		"linear", "srgb"
	};
	inline constexpr const char_t* MATERIAL_EXECUTION_BACKEND_TOKENS[] =
	{
		"generic", "runtimeMaterialV2", "artistVisualV4", "localDecal",
		"standardColorV1"
	};
	inline constexpr const char_t* STANDARD_COLOR_CHANNEL_TOKENS[] =
	{
		"invalid", "R", "G", "B", "A", "RGB"
	};
	inline constexpr const char_t* STANDARD_COLOR_EMISSIVE_MODE_TOKENS[] =
	{
		"none", "baseRadiance"
	};
	inline constexpr const char_t* STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS[] =
	{
		"invalid", "carrierAlpha"
	};
	inline constexpr const char_t* STANDARD_COLOR_DISSOLVE_MODE_TOKENS[] =
	{
		"none", "laneThreshold"
	};
	inline constexpr const char_t* STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS[] =
	{
		"invalid", "failClosed"
	};
	inline constexpr const char_t* MATERIAL_TEXTURE_FILTER_TOKENS[] =
	{
		"point", "linear", "anisotropic"
	};
	inline constexpr const char_t* MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[] =
	{
		"wrap", "mirror", "clamp", "border"
	};
	inline constexpr const char_t* MATERIAL_COMPARISON_FUNCTION_TOKENS[] =
	{
		"never", "less", "equal", "lessEqual", "greater", "notEqual",
		"greaterEqual", "always"
	};
	inline constexpr const char_t* SOURCE_LITERAL_KIND_TOKENS[] =
	{
		"boolean", "number", "string"
	};
	inline constexpr const char_t* SOURCE_TYPED_FIELD_KIND_TOKENS[] =
	{
		"boolean", "number", "string", "vector"
	};
	inline constexpr const char_t* DISTRIBUTION_INTERPOLATION_TOKENS[] =
	{
		"constant", "linear", "cubic"
	};
	inline constexpr const char_t* PRESENTATION_RUNTIME_STATUS_TOKENS[] =
	{
		"reconstructed_profile"
	};
	inline constexpr const char_t* LIGHT_PROFILE_TOKENS[] =
	{
		"light.point.reconstructed.v1"
	};
	inline constexpr const char_t* SCREEN_POST_PROFILE_TOKENS[] =
	{
		"screen.rgb-noise.reconstructed.v1",
		"screen.zoom-blur.reconstructed.v1",
		"screen.film-noise.reconstructed.v1",
		"screen.motion-blur.reconstructed.v1"
	};
	inline constexpr const char_t* SOURCE_PRESENTATION_STATUS_TOKENS[] =
	{
		"source_exact", "reconstructed", "unresolved"
	};
	inline constexpr const char_t* SOURCE_PRESENTATION_PARAMETER_KIND_TOKENS[] =
	{
		"number", "boolean", "vector", "string"
	};
	inline constexpr const char_t* SOURCE_PRESENTATION_PARAMETER_STATUS_TOKENS[] =
	{
		"source_explicit", "source_distribution",
		"unresolved_class_default"
	};
	uint64_t SourceScaledParticleCeiling(
		const Client::EFFECT_ELEMENT_DESC& Element);
	bool_t Is_StableId(const std::string& Value);
	bool_t Has_VisibleCharacter(const std::string& Value);
	bool_t Is_Finite(const float2_t& Value);
	bool_t Is_Finite(const float3_t& Value);
	bool_t Is_Finite(const float4_t& Value);
	const Client::DATA_JSON_VALUE* Find_Field(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		const Client::DATA_JSON_TYPE eType,
		std::string& strOutError);
	bool_t Read_Float(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError);
	bool_t Read_OptionalFloat(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError);
	bool_t Read_Int(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		int32_t& OutValue,
		std::string& strOutError);
	bool_t Read_UInt(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue,
		std::string& strOutError);
	bool_t Read_OptionalUInt(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue,
		std::string& strOutError);
	bool_t Read_Bool(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError);
	bool_t Read_OptionalBool(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError);
	bool_t Read_String(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		std::string& OutValue,
		std::string& strOutError);
	bool_t Read_Double(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f64_t& OutValue,
		std::string& strOutError);
	bool_t Read_StringArray(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		std::vector<std::string>& OutValues,
		std::string& strOutError);
	bool_t Read_Array(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOut,
		const size_t iCount,
		std::string& strOutError);
	bool_t Read_OptionalArray(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOut,
		const size_t iCount,
		std::string& strOutError);
	bool_t Validate_ExactFields(
		const Client::DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Allowed,
		const std::string_view Context,
		std::string& strOutError);
	bool_t Read_AuthoredRuntimeCarrier(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Out,
		std::string& strOutError);
	bool_t Read_AuthoredRuntimeExtensions(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_AUTHORED_RUNTIME_EXTENSIONS_DESC& Out,
		std::string& strOutError);
	bool_t Is_LowerHexSha256(const std::string_view Value);
	std::string Normalize_SourceModuleClass(const std::string_view Value);
	std::string Canonicalize_ExactSourceModuleClass(
		const std::string_view Value);
	bool_t Is_ParticleParameterDistribution(const std::string_view Value);
	bool_t Is_UnresolvedSourceToken(const std::string_view Value);
	Client::EFFECT_ELEMENT_KIND Kind_ForRenderer(
		const Client::EFFECT_RENDERER_TYPE eType);
	void Write_Float2(std::ostringstream& Output, const float2_t& Value);
	void Write_Float3(std::ostringstream& Output, const float3_t& Value);
	void Write_Float4(std::ostringstream& Output, const float4_t& Value);
	void Write_StringArray(
		std::ostringstream& Output,
		const std::vector<std::string>& Values);
	bool_t Read_Renderer(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_RENDERER_DESC& Out,
		std::string& strOutError);
	void Write_Renderer(
		std::ostringstream& Output,
		const Client::EFFECT_RENDERER_DESC& Renderer);
	bool_t Read_SourceMaterialProfile(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_MATERIAL_DESC& Out,
		std::string& strOutError);
	void Write_SourceMaterialProfile(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source);
	bool_t Read_MaterialExecution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_EXECUTION_DESC& Out,
		std::string& strOutError);
	void Write_MaterialExecution(
		std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution);
	bool_t Is_DefaultStandardColorV1(
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet);
	bool_t Validate_StandardColorV1Execution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError);
	bool_t Validate_MaterialExecution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError);
	bool_t Read_Distribution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DISTRIBUTION_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError);
	bool_t Read_SourceAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_ADMISSION_DESC& Out,
		std::string& strOutError);
	bool_t Validate_SourceTransformTrack(
        const Client::EFFECT_SOURCE_TRANSFORM_TRACK& Track, std::string& Error);
	bool_t Read_SourceTransformTrack(const Client::DATA_JSON_VALUE& Value,
        Client::EFFECT_SOURCE_TRANSFORM_TRACK& Out, std::string& Error);
	void Write_SourceTransformTrack(std::ostringstream& Output,
        const Client::EFFECT_SOURCE_TRANSFORM_TRACK& Track);
	bool_t Read_SourceRecipe(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_CASCADE_RECIPE_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError);
	void Write_SourceRecipe(
		std::ostringstream& Output,
		const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe,
		const bool_t bSourceContract);
	bool_t Read_PresentationDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError);
	void Write_PresentationDetail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail);
	bool_t Read_AuthoringOverrides(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ELEMENT_DESC& Out,
		std::string& strOutError);
	bool_t Read_SourcePresentation(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_PRESENTATION_DESC& Out,
		std::string& strOutError);
	void Write_SourcePresentation(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_PRESENTATION_DESC& Source);
	bool_t Read_ModelCueTransform(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MODEL_CUE_DESC& Out,
		std::string& strOutError);
	bool_t Read_ActionCueAttachment(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ACTION_CUE_ATTACHMENT_DESC& Out,
		std::string& strOutError);
	bool_t Read_TransformInheritance(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_TRANSFORM_INHERITANCE_DESC& Out,
		std::string& strOutError);
	bool_t Is_SafeModelCueAssetIdInternal(const std::string& strAssetId);
	bool_t Read_Material(const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_DESC& Out, const uint32_t iSourceVersion,
		const bool_t bSourceContract, std::string& strOutError);
	void Write_Material(std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_DESC& Material);
	bool_t Read_CommonDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError);
	bool_t Read_V5Detail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError);
	void Write_Detail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail);
	bool_t Validate_ExecutableSourceMaterialCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element,
		bool_t& bOutOwnsDrawableContract,
		std::string& strOutError);
	bool_t Apply_Warlord17090RetainedSourceProjection(
		Client::EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError);
	bool_t Validate_ElementMaterial(const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bSourceContract, std::string& strOutError);
	bool_t Validate_ElementSourceMaterialSlots(const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);
	bool_t ValidatePortableAuthoredParticleRuntimeCarrier(
		const EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);
	bool_t ValidatePortableAuthoredParticleEventRoutes(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Validate_AuthoredRuntimeExtensions(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);



	template<size_t COUNT>
	bool_t Read_UIntArray(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		std::array<uint32_t, COUNT>& OutValues,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pValue || pValue->Get_Array().size() != COUNT)
			return false;
		for (size_t iValue = 0u; iValue < COUNT; ++iValue)
		{
			const Client::DATA_JSON_VALUE& Item = pValue->Get_Array()[iValue];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() != std::floor(Item.Get_Number()) ||
				Item.Get_Number() < 0.0 ||
				Item.Get_Number() > static_cast<double>(UINT32_MAX))
			{
				return false;
			}
			OutValues[iValue] = static_cast<uint32_t>(Item.Get_Number());
		}
		return true;
	}

	template<typename ENUM>
	bool_t Parse_Token(
		const std::string& Value,
		const char_t* const* pTokens,
		const size_t iCount,
		ENUM& eOut)
	{
		for (size_t iToken = 0u; iToken < iCount; ++iToken)
		{
			if (Value == pTokens[iToken])
			{
				eOut = static_cast<ENUM>(iToken);
				return true;
			}
		}
		return false;
	}

	template<size_t COUNT>
	void Write_UIntArray(
		std::ostringstream& Output,
		const std::array<uint32_t, COUNT>& Values)
	{
		Output << '[';
		for (size_t iValue = 0u; iValue < COUNT; ++iValue)
		{
			if (0u != iValue)
				Output << ", ";
			Output << Values[iValue];
		}
		Output << ']';
	}
}
