#include "Effect_DocumentCodec.h"

#include "DataJson.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_RuntimeAuthority.h"
#include "Generated/Effect_SourceContractRegistry.generated.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr const char_t* EFFECT_DOCUMENT_SCHEMA =
		"lostark.effect-authoring";
	constexpr size_t MAX_RESOURCE_ID_BYTES = 512u;
	constexpr size_t MAX_ELEMENTS = 2048u;
	constexpr size_t MAX_MODEL_CUES = 8u;
	constexpr uint64_t MAX_DOCUMENT_PARTICLES = 8192u;
	constexpr uint64_t MAX_DOCUMENT_TRAIL_POINTS = 2048u;
	constexpr uint64_t MAX_DOCUMENT_AFTERIMAGES = 256u;
	constexpr uint64_t MAX_PORTABLE_SOURCE_EVENTS_PER_STEP = 4096u;
	constexpr size_t MAX_SOURCE_MODULES_PER_ELEMENT = 256u;
	constexpr size_t MAX_SOURCE_LITERALS_PER_MODULE = 1024u;
	constexpr size_t MAX_SOURCE_DISTRIBUTIONS_PER_MODULE = 128u;
	constexpr size_t MAX_SOURCE_BURSTS_PER_ELEMENT = 1024u;
	constexpr size_t MAX_SOURCE_PRESENTATION_PARAMETERS = 256u;
	constexpr size_t MAX_SOURCE_COVERAGE_PROPERTIES_PER_MODULE = 2048u;
	constexpr size_t MAX_SOURCE_LOCAL_REFERENCE_BINDINGS_PER_ELEMENT = 2048u;
	constexpr size_t MAX_SOURCE_TYPED_FIELDS_PER_REFERENCE = 256u;
	constexpr size_t MAX_AUTHORED_MATERIAL_TEXTURE_LANES = 6u;
	constexpr size_t MAX_AUTHORED_MATERIAL_SCALARS = 52u;
	constexpr size_t MAX_AUTHORED_MATERIAL_VECTORS = 8u;
	constexpr size_t MAX_AUTHORED_MATERIAL_COLORS = 2u;
	constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_HISTORIES = 16u;
	constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_PER_HISTORY = 4096u;
	constexpr size_t MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_TOTAL = 16384u;
	constexpr f32_t MAX_AUTHORED_RUNTIME_EDGE_TIME_SECONDS = 30.f;
	constexpr f32_t MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM = 1'000'000.f;

	uint64_t SourceScaledParticleCeiling(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const uint64_t iMaximum = Element.Detail.Particle.iMaxParticles;
		if (!Element.SourceRecipe.bEnabled)
			return iMaximum;
		const double fScaled = std::round(static_cast<double>(iMaximum) *
			static_cast<double>(Element.Detail.Particle.SourceScale.fCount));
		return static_cast<uint64_t>((std::max)(0.0, fScaled));
	}
	constexpr const char_t* EFFECT_SOURCE_PRESENTATION_SCHEMA =
		"lostark.effect-source-presentation";
	constexpr std::string_view WARLORD_17090_EFFECT_ASSET_ID =
		"effect.warlord.skill.17090.unified";
	constexpr std::string_view WARLORD_CHAIN_SOURCE_MATERIAL_PATH =
		"fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma";
	constexpr std::string_view WARLORD_CHAIN_BASE_ALIAS_ASSET_ID =
		"Effect/Warlord/Textures/FX_TEX_02/fx_d_atypical_028.dds";
	constexpr std::string_view WARLORD_CHAIN_SOURCE_PROFILE_ID =
		"ue3.material.fx.m.mi.00.fx.m.fx.d.me.chain.01.ma.a8a92d2a6abc";
	constexpr std::string_view WARLORD_CHAIN_PARENT_MATERIAL_PATH =
		"fx_m_mi_00.fx_m.fx_d_me_chain_01_ma";
	constexpr std::string_view WARLORD_CHAIN_RUNTIME_PROFILE_ID =
		"effect.ue3.grouped-translucent.v1";
	constexpr std::string_view WARLORD_CHAIN_06_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_06.wmodel";
	constexpr std::string_view WARLORD_CHAIN_07_MODEL_ASSET_ID =
		"Effect/Warlord/Meshes/FX_SM_01/fm_d_berchain_07.wmodel";

	bool_t ValidatePortableAuthoredParticleRuntimeCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);
	bool_t ValidatePortableAuthoredParticleEventRoutes(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Validate_AuthoredRuntimeExtensions(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);

	constexpr const char_t* KIND_TOKENS[] =
	{
		"mesh", "sprite", "particle", "decal", "trail", "light",
		"screenPost"
	};
	constexpr const char_t* COMPOSITION_LAYER_TOKENS[] =
	{
		"normal", "worldMark"
	};
	constexpr const char_t* ATTACHMENT_ORIENTATION_TOKENS[] =
	{
		"bone", "owner_yaw"
	};
	constexpr const char_t* AUTHORED_RUNTIME_CARRIER_KIND_TOKENS[] =
	{
		"cascadeRibbonV1", "animationTrailBakedEdgeV1",
		"lightBakedEdgeAttachmentV1"
	};
	constexpr const char_t* AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS[] =
	{
		"bounded"
	};
	constexpr const char_t* AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS[] =
	{
		"firstEdge"
	};
	constexpr const char_t* AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS[] =
	{
		"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS"
	};
	constexpr const char_t* RENDERER_TYPE_TOKENS[] =
	{
		"standaloneMesh", "legacyStandaloneSprite", "meshParticle",
		"spriteParticle", "decalParticle", "animTrail", "cascadeRibbon",
		"lightParticle", "screenPost"
	};
	constexpr const char_t* SOURCE_SPACE_TOKENS[] =
	{
		"clientMetersV1", "ue3CascadeV1", "screenSpaceV1"
	};
	constexpr const char_t* MODEL_CUE_ALPHA_MODE_TOKENS[] =
	{
		"OPAQUE", "MASKED", "TRANSLUCENT"
	};
	constexpr const char_t* SOURCE_COVERAGE_STATUS_TOKENS[] =
	{
		"source_decoded", "deterministic_conversion", "metadata_only",
		"unresolved"
	};
	constexpr const char_t* DISTRIBUTION_PARAMETER_BINDING_TOKENS[] =
	{
		"none", "actionCue"
	};
	constexpr const char_t* SLOT_TOKENS[] =
	{
		"meshModel", "base", "noise", "mask", "emissive", "dissolve",
		"base2", "mask2", "noise2"
	};
	constexpr const char_t* PARTICLE_SPAWN_SHAPE_TOKENS[] =
	{
		"point", "sphere", "ring", "box"
	};
	constexpr const char_t* PARTICLE_SPAWN_DISTRIBUTION_TOKENS[] =
	{
		"random", "even"
	};
	constexpr const char_t* PARTICLE_ORIENTATION_MODE_TOKENS[] =
	{
		"fixed", "groundRadialOutward", "groundRadialInward",
		"groundTangentClockwise", "groundTangentCounterClockwise"
	};
	constexpr const char_t* PARTICLE_VELOCITY_MODE_TOKENS[] =
	{
		"fixed", "outward", "inward", "cone"
	};
	constexpr const char_t* RING_FILL_DIRECTION_TOKENS[] =
	{
		"innerToOuter", "outerToInner"
	};
	constexpr const char_t* LINEAR_REVEAL_AXIS_TOKENS[] =
	{
		"u", "v"
	};
	constexpr const char_t* DECAL_RECEIVER_MODE_TOKENS[] =
	{
		"allOpaque", "upwardSurfaces"
	};
	constexpr const char_t* PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS[] =
	{
		"rootLocal", "elementLocal"
	};
	constexpr const char_t* PROFILE_TOKENS[] =
	{
		"opaque_back_depth_write",
		"alpha_two_sided_depth_read",
		"additive_two_sided_depth_read",
		"alpha_one_sided_depth_read",
		"additive_one_sided_depth_read"
	};
	constexpr const char_t* SOURCE_MATERIAL_STATUS_TOKENS[] =
	{
		"source_exact",
		"runtime_exact",
		"reconstructed_profile",
		"unsupported",
		"missing_resource"
	};
	constexpr const char_t* SOURCE_BLEND_CLASS_TOKENS[] =
	{
		"unknown", "additive", "translucent", "masked"
	};
	constexpr const char_t* TEXTURE_ADDRESS_MODE_TOKENS[] =
	{
		"wrap", "clamp"
	};
	constexpr const char_t* TEXTURE_COLOR_SPACE_TOKENS[] =
	{
		"linear", "srgb"
	};
	constexpr const char_t* MATERIAL_EXECUTION_BACKEND_TOKENS[] =
	{
		"generic", "runtimeMaterialV2", "artistVisualV4", "localDecal",
		"standardColorV1"
	};
	constexpr const char_t* STANDARD_COLOR_CHANNEL_TOKENS[] =
	{
		"invalid", "R", "G", "B", "A", "RGB"
	};
	constexpr const char_t* STANDARD_COLOR_EMISSIVE_MODE_TOKENS[] =
	{
		"none", "baseRadiance"
	};
	constexpr const char_t* STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS[] =
	{
		"invalid", "carrierAlpha"
	};
	constexpr const char_t* STANDARD_COLOR_DISSOLVE_MODE_TOKENS[] =
	{
		"none", "laneThreshold"
	};
	constexpr const char_t* STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS[] =
	{
		"invalid", "failClosed"
	};
	constexpr const char_t* MATERIAL_TEXTURE_FILTER_TOKENS[] =
	{
		"point", "linear", "anisotropic"
	};
	constexpr const char_t* MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[] =
	{
		"wrap", "mirror", "clamp", "border"
	};
	constexpr const char_t* MATERIAL_COMPARISON_FUNCTION_TOKENS[] =
	{
		"never", "less", "equal", "lessEqual", "greater", "notEqual",
		"greaterEqual", "always"
	};
	constexpr const char_t* SOURCE_LITERAL_KIND_TOKENS[] =
	{
		"boolean", "number", "string"
	};
	constexpr const char_t* SOURCE_TYPED_FIELD_KIND_TOKENS[] =
	{
		"boolean", "number", "string", "vector"
	};
	constexpr const char_t* DISTRIBUTION_INTERPOLATION_TOKENS[] =
	{
		"constant", "linear", "cubic"
	};
	constexpr const char_t* PRESENTATION_RUNTIME_STATUS_TOKENS[] =
	{
		"reconstructed_profile"
	};
	constexpr const char_t* LIGHT_PROFILE_TOKENS[] =
	{
		"light.point.reconstructed.v1"
	};
	constexpr const char_t* SCREEN_POST_PROFILE_TOKENS[] =
	{
		"screen.rgb-noise.reconstructed.v1",
		"screen.zoom-blur.reconstructed.v1",
		"screen.film-noise.reconstructed.v1"
	};
	constexpr const char_t* SOURCE_PRESENTATION_STATUS_TOKENS[] =
	{
		"source_exact", "reconstructed", "unresolved"
	};
	constexpr const char_t* SOURCE_PRESENTATION_PARAMETER_KIND_TOKENS[] =
	{
		"number", "boolean", "vector", "string"
	};
	constexpr const char_t* SOURCE_PRESENTATION_PARAMETER_STATUS_TOKENS[] =
	{
		"source_explicit", "source_distribution",
		"unresolved_class_default"
	};

	bool_t Is_StableId(const std::string& Value)
	{
		if (Value.empty() || Value.size() > 128u)
			return false;
		return std::all_of(Value.begin(), Value.end(),
			[](const char_t Character)
			{
				const unsigned char Value =
					static_cast<unsigned char>(Character);
				return 0 != std::isalnum(Value) || Character == '_' ||
					Character == '.' || Character == '-';
			});
	}

	bool_t Has_VisibleCharacter(const std::string& Value)
	{
		return std::any_of(Value.begin(), Value.end(),
			[](const char_t Character)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(Character));
			});
	}

	bool_t Is_Finite(const float2_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y);
	}

	bool_t Is_Finite(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	bool_t Is_Finite(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	Client::EFFECT_RESOURCE_FILE_KIND FileKindForSlot(
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
			return Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (eSlot >= Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
			eSlot <= Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
		{
			return Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
		}
		return Client::EFFECT_RESOURCE_FILE_KIND::END;
	}

	const Client::DATA_JSON_VALUE* Find_Field(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		const Client::DATA_JSON_TYPE eType,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pName);
		if (nullptr == pValue || pValue->Get_Type() != eType)
		{
			strOutError = std::string("Missing or invalid field: ") + pName;
			return nullptr;
		}
		return pValue;
	}

	bool_t Read_Float(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()))
			return false;
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return std::isfinite(OutValue);
	}

	bool_t Read_OptionalFloat(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError)
	{
		if (nullptr == Object.Find(pName))
			return true;
		return Read_Float(Object, pName, OutValue, strOutError);
	}

	bool_t Read_Int(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		int32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < static_cast<double>(INT32_MIN) ||
			pValue->Get_Number() > static_cast<double>(INT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<int32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_UInt(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() != std::floor(pValue->Get_Number()) ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_OptionalUInt(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		uint32_t& OutValue,
		std::string& strOutError)
	{
		if (nullptr == Object.Find(pName))
			return true;
		return Read_UInt(Object, pName, OutValue, strOutError);
	}

	bool_t Read_Bool(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::BOOLEAN, strOutError);
		if (nullptr == pValue)
			return false;
		OutValue = pValue->Get_Boolean();
		return true;
	}

	bool_t Read_OptionalBool(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError)
	{
		if (nullptr == Object.Find(pName))
			return true;
		return Read_Bool(Object, pName, OutValue, strOutError);
	}

	bool_t Read_String(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		std::string& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pValue)
			return false;
		OutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_Double(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f64_t& OutValue,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()))
			return false;
		OutValue = pValue->Get_Number();
		return true;
	}

	bool_t Read_StringArray(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		std::vector<std::string>& OutValues,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pValue)
			return false;
		OutValues.clear();
		OutValues.reserve(pValue->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Item : pValue->Get_Array())
		{
			if (!Item.Is_String())
			{
				strOutError = std::string("Effect source string array is invalid: ") +
					pName;
				return false;
			}
			OutValues.push_back(Item.Get_String());
		}
		return true;
	}

	bool_t Read_Array(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOut,
		const size_t iCount,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pValue = Find_Field(
			Object, pName, Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pValue || pValue->Get_Array().size() != iCount)
			return false;
		for (size_t iValue = 0u; iValue < iCount; ++iValue)
		{
			const Client::DATA_JSON_VALUE& Item = pValue->Get_Array()[iValue];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
				return false;
			pOut[iValue] = static_cast<f32_t>(Item.Get_Number());
			if (!std::isfinite(pOut[iValue]))
				return false;
		}
		return true;
	}

	bool_t Read_OptionalArray(
		const Client::DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOut,
		const size_t iCount,
		std::string& strOutError)
	{
		if (nullptr == Object.Find(pName))
			return true;
		return Read_Array(Object, pName, pOut, iCount, strOutError);
	}

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

	bool_t Validate_ExactFields(
		const Client::DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Allowed,
		const std::string_view Context,
		std::string& strOutError)
	{
		if (!Object.Is_Object())
		{
			strOutError = std::string(Context) + " must be an object.";
			return false;
		}
		for (const auto& [Name, Value] : Object.Get_Object())
		{
			UNREFERENCED_PARAMETER(Value);
			if (std::find(Allowed.begin(), Allowed.end(), Name) == Allowed.end())
			{
				strOutError = std::string(Context) +
					" contains an unknown native-v14 field: " + Name;
				return false;
			}
		}
		return true;
	}

	bool_t Read_AuthoredRuntimeCarrier(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pKind = Value.Find("kind");
		const Client::DATA_JSON_VALUE* pAdmission = Value.Find("admission");
		Client::EFFECT_AUTHORED_RUNTIME_CARRIER_DESC Staged;
		if (!Value.Is_Object() || nullptr == pKind || !pKind->Is_String() ||
			nullptr == pAdmission || !pAdmission->Is_String() ||
			!Read_UInt(Value, "formatVersion", Staged.iFormatVersion,
				strOutError) ||
			!Parse_Token(pKind->Get_String(),
				AUTHORED_RUNTIME_CARRIER_KIND_TOKENS,
				std::size(AUTHORED_RUNTIME_CARRIER_KIND_TOKENS),
				Staged.eKind) ||
			!Parse_Token(pAdmission->Get_String(),
				AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS,
				std::size(AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS),
				Staged.eAdmission) ||
			Staged.iFormatVersion !=
				Client::EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION)
		{
			if (strOutError.empty())
				strOutError = "Effect authored runtimeCarrier identity is invalid.";
			return false;
		}

		switch (Staged.eKind)
		{
		case Client::EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
		{
			const Client::DATA_JSON_VALUE* pTypeDataModuleStableId =
				Value.Find("typeDataModuleStableId");
			if (!Validate_ExactFields(Value,
					{ "formatVersion", "kind", "admission",
						"typeDataModuleStableId" },
					"Effect authored Cascade runtimeCarrier", strOutError) ||
				nullptr == pTypeDataModuleStableId ||
				!pTypeDataModuleStableId->Is_String())
			{
				if (strOutError.empty())
					strOutError =
						"Effect authored Cascade runtimeCarrier TypeData join is invalid.";
				return false;
			}
			Staged.strTypeDataModuleStableId =
				pTypeDataModuleStableId->Get_String();
			break;
		}
		case Client::EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
			ANIMATION_TRAIL_BAKED_EDGE_V1:
		{
			const Client::DATA_JSON_VALUE* pHistoryId = Value.Find("historyId");
			if (!Validate_ExactFields(Value,
					{ "formatVersion", "kind", "admission", "historyId" },
					"Effect authored Animation Trail runtimeCarrier",
					strOutError) ||
				nullptr == pHistoryId || !pHistoryId->Is_String())
			{
				if (strOutError.empty())
					strOutError =
						"Effect authored Animation Trail runtimeCarrier history join is invalid.";
				return false;
			}
			Staged.strHistoryId = pHistoryId->Get_String();
			break;
		}
		case Client::EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
			LIGHT_BAKED_EDGE_ATTACHMENT_V1:
		{
			const Client::DATA_JSON_VALUE* pHistoryId = Value.Find("historyId");
			const Client::DATA_JSON_VALUE* pEdgeLane = Value.Find("edgeLane");
			if (!Validate_ExactFields(Value,
					{ "formatVersion", "kind", "admission", "historyId",
						"edgeLane" },
					"Effect authored Light runtimeCarrier", strOutError) ||
				nullptr == pHistoryId || !pHistoryId->Is_String() ||
				nullptr == pEdgeLane || !pEdgeLane->Is_String() ||
				!Parse_Token(pEdgeLane->Get_String(),
					AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS,
					std::size(AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS),
					Staged.eEdgeLane))
			{
				if (strOutError.empty())
					strOutError =
						"Effect authored Light runtimeCarrier history/lane join is invalid.";
				return false;
			}
			Staged.strHistoryId = pHistoryId->Get_String();
			break;
		}
		default:
			strOutError = "Effect authored runtimeCarrier kind is unsupported.";
			return false;
		}

		Out = std::move(Staged);
		return true;
	}

	bool_t Read_AuthoredRuntimeExtensions(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_AUTHORED_RUNTIME_EXTENSIONS_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pHistories =
			Value.Find("bakedEdgeHistories");
		Client::EFFECT_AUTHORED_RUNTIME_EXTENSIONS_DESC Staged;
		if (!Validate_ExactFields(Value,
				{ "formatVersion", "bakedEdgeHistories" },
				"Effect authored runtimeExtensions", strOutError) ||
			!Read_UInt(Value, "formatVersion", Staged.iFormatVersion,
				strOutError) ||
			Staged.iFormatVersion !=
				Client::EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
			nullptr == pHistories || !pHistories->Is_Array() ||
			pHistories->Get_Array().size() >
				MAX_AUTHORED_RUNTIME_EDGE_HISTORIES)
		{
			if (strOutError.empty())
				strOutError = "Effect authored runtimeExtensions are invalid.";
			return false;
		}

		size_t iTotalSampleCount = 0u;
		std::string strPreviousHistoryId;
		Staged.BakedEdgeHistories.reserve(pHistories->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& HistoryValue :
			pHistories->Get_Array())
		{
			const Client::DATA_JSON_VALUE* pHistoryId =
				HistoryValue.Find("historyId");
			const Client::DATA_JSON_VALUE* pCoordinateBasis =
				HistoryValue.Find("coordinateBasis");
			const Client::DATA_JSON_VALUE* pSamples =
				HistoryValue.Find("samples");
			Client::EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC History;
			if (!Validate_ExactFields(HistoryValue,
					{ "historyId", "coordinateBasis", "sourceEndTimeSeconds",
						"playbackClampSeconds", "samples" },
					"Effect authored baked-edge history", strOutError) ||
				nullptr == pHistoryId || !pHistoryId->Is_String() ||
				nullptr == pCoordinateBasis || !pCoordinateBasis->Is_String() ||
				nullptr == pSamples || !pSamples->Is_Array() ||
				pSamples->Get_Array().size() < 2u ||
				pSamples->Get_Array().size() >
					MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_PER_HISTORY ||
				!Parse_Token(pCoordinateBasis->Get_String(),
					AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS,
					std::size(AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS),
					History.eCoordinateBasis) ||
				!Read_Float(HistoryValue, "sourceEndTimeSeconds",
					History.fSourceEndTimeSeconds, strOutError) ||
				!Read_Float(HistoryValue, "playbackClampSeconds",
					History.fPlaybackClampSeconds, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect authored baked-edge history is invalid.";
				return false;
			}
			History.strHistoryId = pHistoryId->Get_String();
			if (!Is_StableId(History.strHistoryId) ||
				(!strPreviousHistoryId.empty() &&
				 History.strHistoryId <= strPreviousHistoryId) ||
				History.fSourceEndTimeSeconds <= 0.f ||
				History.fSourceEndTimeSeconds >
					MAX_AUTHORED_RUNTIME_EDGE_TIME_SECONDS ||
				History.fPlaybackClampSeconds <= 0.f ||
				History.fPlaybackClampSeconds >
					History.fSourceEndTimeSeconds)
			{
				strOutError =
					"Effect authored baked-edge history identity/timing is invalid.";
				return false;
			}
			strPreviousHistoryId = History.strHistoryId;
			iTotalSampleCount += pSamples->Get_Array().size();
			if (iTotalSampleCount > MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_TOTAL)
			{
				strOutError =
					"Effect authored baked-edge history sample budget is exceeded.";
				return false;
			}

			f32_t fPreviousTime = -1.f;
			History.Samples.reserve(pSamples->Get_Array().size());
			for (const Client::DATA_JSON_VALUE& SampleValue :
				pSamples->Get_Array())
			{
				Client::EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC Sample;
				if (!Validate_ExactFields(SampleValue,
						{ "relativeTimeSeconds", "firstEdgeUE3Cm",
							"controlPointUE3Cm", "secondEdgeUE3Cm" },
						"Effect authored baked-edge sample", strOutError) ||
					!Read_Float(SampleValue, "relativeTimeSeconds",
						Sample.fRelativeTimeSeconds, strOutError) ||
					!Read_Array(SampleValue, "firstEdgeUE3Cm",
						&Sample.vFirstEdgeUE3Cm.x, 3u, strOutError) ||
					!Read_Array(SampleValue, "controlPointUE3Cm",
						&Sample.vControlPointUE3Cm.x, 3u, strOutError) ||
					!Read_Array(SampleValue, "secondEdgeUE3Cm",
						&Sample.vSecondEdgeUE3Cm.x, 3u, strOutError) ||
					Sample.fRelativeTimeSeconds <= fPreviousTime ||
					Sample.fRelativeTimeSeconds < 0.f ||
					Sample.fRelativeTimeSeconds >
						History.fSourceEndTimeSeconds + 5.0e-5f)
				{
					if (strOutError.empty())
						strOutError =
							"Effect authored baked-edge sample is invalid.";
					return false;
				}
				const auto CoordinatesBounded = [](const float3_t& Coordinates)
				{
					return Is_Finite(Coordinates) &&
						std::abs(Coordinates.x) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.y) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.z) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM;
				};
				if (!CoordinatesBounded(Sample.vFirstEdgeUE3Cm) ||
					!CoordinatesBounded(Sample.vControlPointUE3Cm) ||
					!CoordinatesBounded(Sample.vSecondEdgeUE3Cm))
				{
					strOutError =
						"Effect authored baked-edge coordinates are invalid.";
					return false;
				}
				fPreviousTime = Sample.fRelativeTimeSeconds;
				History.Samples.push_back(std::move(Sample));
			}
			if (std::abs(History.Samples.front().fRelativeTimeSeconds) >
					1.0e-6f ||
				std::abs(History.Samples.back().fRelativeTimeSeconds -
					History.fSourceEndTimeSeconds) > 5.0e-5f)
			{
				strOutError =
					"Effect authored baked-edge history does not close at its declared source interval.";
				return false;
			}
			Staged.BakedEdgeHistories.push_back(std::move(History));
		}

		Out = std::move(Staged);
		return true;
	}

	bool_t Is_LowerHexSha256(const std::string_view Value)
	{
		return 64u == Value.size() && std::all_of(Value.begin(), Value.end(),
			[](const char_t Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	std::string Normalize_SourceModuleClass(const std::string_view Value)
	{
		std::string Result(Value);
		std::transform(Result.begin(), Result.end(), Result.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char_t>(std::tolower(Character));
			});
		constexpr std::string_view EffectPrefix = "efparticlemodule";
		if (Result.starts_with(EffectPrefix))
			Result = "particlemodule" + Result.substr(EffectPrefix.size());
		constexpr std::string_view SeededSuffix = "_seeded";
		if (Result.ends_with(SeededSuffix))
			Result.resize(Result.size() - SeededSuffix.size());
		return Result;
	}

	std::string Canonicalize_ExactSourceModuleClass(
		const std::string_view Value)
	{
		std::string Result(Value);
		std::transform(Result.begin(), Result.end(), Result.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char_t>(std::tolower(Character));
			});
		return Result;
	}

	bool_t Is_ParticleParameterDistribution(const std::string_view Value)
	{
		std::string Normalized(Value);
		std::transform(Normalized.begin(), Normalized.end(), Normalized.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char_t>(std::tolower(Character));
			});
		return Normalized == "distributionfloatparticleparameter" ||
			Normalized == "distributionvectorparticleparameter";
	}

	bool_t Is_UnresolvedSourceToken(const std::string_view Value)
	{
		std::string Normalized(Value);
		std::transform(Normalized.begin(), Normalized.end(), Normalized.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char_t>(std::tolower(Character));
			});
		return Normalized.starts_with("unresolved");
	}

	Client::EFFECT_ELEMENT_KIND Kind_ForRenderer(
		const Client::EFFECT_RENDERER_TYPE eType)
	{
		using namespace Client;
		switch (eType)
		{
		case EFFECT_RENDERER_TYPE::STANDALONE_MESH: return EFFECT_ELEMENT_KIND::MESH;
		case EFFECT_RENDERER_TYPE::LEGACY_STANDALONE_SPRITE: return EFFECT_ELEMENT_KIND::SPRITE;
		case EFFECT_RENDERER_TYPE::MESH_PARTICLE:
		case EFFECT_RENDERER_TYPE::SPRITE_PARTICLE: return EFFECT_ELEMENT_KIND::PARTICLE;
		case EFFECT_RENDERER_TYPE::DECAL_PARTICLE: return EFFECT_ELEMENT_KIND::DECAL;
		case EFFECT_RENDERER_TYPE::ANIM_TRAIL:
		case EFFECT_RENDERER_TYPE::CASCADE_RIBBON: return EFFECT_ELEMENT_KIND::TRAIL;
		case EFFECT_RENDERER_TYPE::LIGHT_PARTICLE: return EFFECT_ELEMENT_KIND::LIGHT;
		case EFFECT_RENDERER_TYPE::SCREEN_POST: return EFFECT_ELEMENT_KIND::SCREEN_POST;
		default: return EFFECT_ELEMENT_KIND::END;
		}
	}

	void Write_Float2(std::ostringstream& Output, const float2_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ']';
	}

	void Write_Float3(std::ostringstream& Output, const float3_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ", " << Value.z << ']';
	}

	void Write_Float4(std::ostringstream& Output, const float4_t& Value)
	{
		Output << '[' << Value.x << ", " << Value.y << ", "
			<< Value.z << ", " << Value.w << ']';
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

	void Write_StringArray(
		std::ostringstream& Output,
		const std::vector<std::string>& Values)
	{
		Output << '[';
		for (size_t iValue = 0u; iValue < Values.size(); ++iValue)
		{
			if (iValue > 0u)
				Output << ", ";
			Output << '"' << Client::CDataJson::Escape(Values[iValue]) << '"';
		}
		Output << ']';
	}

	bool_t Read_Renderer(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_RENDERER_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value, { "type", "sourceSpace" },
			"Effect source renderer", strOutError))
			return false;
		const Client::DATA_JSON_VALUE* pType = Find_Field(
			Value, "type", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pSpace = Find_Field(
			Value, "sourceSpace", Client::DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pType || nullptr == pSpace ||
			!Parse_Token(pType->Get_String(), RENDERER_TYPE_TOKENS,
				std::size(RENDERER_TYPE_TOKENS), Out.eType) ||
			!Parse_Token(pSpace->Get_String(), SOURCE_SPACE_TOKENS,
				std::size(SOURCE_SPACE_TOKENS), Out.eSourceSpace))
		{
			strOutError = "Effect source renderer is invalid.";
			return false;
		}
		return true;
	}

	void Write_Renderer(
		std::ostringstream& Output,
		const Client::EFFECT_RENDERER_DESC& Renderer)
	{
		Output << "      \"renderer\": { \"type\": \""
			<< RENDERER_TYPE_TOKENS[static_cast<size_t>(Renderer.eType)]
			<< "\", \"sourceSpace\": \""
			<< SOURCE_SPACE_TOKENS[static_cast<size_t>(Renderer.eSourceSpace)]
			<< "\" },\n";
	}

	bool_t Read_SourceMaterialProfile(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_MATERIAL_DESC& Out,
		std::string& strOutError)
	{
		if (!Value.Is_Object() ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError))
		{
			return false;
		}
		/* A typed semantic replay may deliberately disable native source
		   execution while retaining the recovered parent/profile as immutable
		   evidence for its exact occurrence allowlist.  Minimal disabled
		   profiles remain valid; a profileId opts into full evidence parsing. */
		if (!Out.bEnabled && nullptr == Value.Find("profileId"))
			return true;
		if (const Client::DATA_JSON_VALUE* pSourceBlendClass =
			Value.Find("sourceBlendClass"))
		{
			if (!pSourceBlendClass->Is_String() ||
				!Parse_Token(pSourceBlendClass->Get_String(),
					SOURCE_BLEND_CLASS_TOKENS,
					std::size(SOURCE_BLEND_CLASS_TOKENS),
					Out.eSourceBlendClass))
			{
				strOutError =
					"Effect source Material blend class is invalid.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pTextures = Value.Find("textures");
		if (nullptr != pTextures && !pTextures->Is_Array())
		{
			strOutError = "Effect source Material textures must be an array.";
			return false;
		}
		const Client::DATA_JSON_VALUE* pScalars = Find_Field(
			Value, "scalars", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pVectors = Find_Field(
			Value, "vectors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pSwitches = Find_Field(
			Value, "staticSwitches", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pDynamicSemantics = Find_Field(
			Value, "dynamicParameterSemantics",
			Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pStatus = Find_Field(
			Value, "semanticStatus", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		if (nullptr == pScalars || nullptr == pVectors ||
			nullptr == pSwitches || nullptr == pDynamicSemantics ||
			nullptr == pStatus ||
			!Read_String(Value, "profileId", Out.strProfileId,
				strOutError) ||
			!Read_String(Value, "runtimeShaderProfileId",
				Out.strRuntimeShaderProfileId, strOutError) ||
			!Read_String(Value, "parentMaterialPath",
				Out.strParentMaterialPath, strOutError) ||
			!Read_String(Value, "subUVMode", Out.strSubUVMode,
				strOutError) ||
			!Parse_Token(pStatus->Get_String(),
				SOURCE_MATERIAL_STATUS_TOKENS,
				std::size(SOURCE_MATERIAL_STATUS_TOKENS), Out.eStatus) ||
			pDynamicSemantics->Get_Array().size() !=
				Out.DynamicParameterSemantics.size())
		{
			return false;
		}
		for (size_t iSemantic = 0u;
			iSemantic < Out.DynamicParameterSemantics.size(); ++iSemantic)
		{
			const Client::DATA_JSON_VALUE& Semantic =
				pDynamicSemantics->Get_Array()[iSemantic];
			if (!Semantic.Is_String())
				return false;
			Out.DynamicParameterSemantics[iSemantic] =
				Semantic.Get_String();
		}
		if (nullptr != pTextures)
		{
			for (const Client::DATA_JSON_VALUE& Item : pTextures->Get_Array())
			{
				Client::EFFECT_NAMED_TEXTURE_DESC Texture;
				if (!Item.Is_Object() ||
					!Read_String(Item, "name", Texture.strName, strOutError) ||
					!Read_String(Item, "sourceObjectPath",
						Texture.strSourceObjectPath, strOutError) ||
					!Read_String(Item, "assetId", Texture.strAssetId,
						strOutError))
				{
					return false;
				}
				if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
				{
					if (!pGroup->Is_String())
						return false;
					Texture.strGroup = pGroup->Get_String();
				}
				if (const Client::DATA_JSON_VALUE* pAddressU = Item.Find("addressU"))
				{
					if (!pAddressU->Is_String() ||
						!Parse_Token(pAddressU->Get_String(),
							TEXTURE_ADDRESS_MODE_TOKENS,
							std::size(TEXTURE_ADDRESS_MODE_TOKENS),
							Texture.eAddressU))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pAddressV = Item.Find("addressV"))
				{
					if (!pAddressV->Is_String() ||
						!Parse_Token(pAddressV->Get_String(),
							TEXTURE_ADDRESS_MODE_TOKENS,
							std::size(TEXTURE_ADDRESS_MODE_TOKENS),
							Texture.eAddressV))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pColorSpace =
					Item.Find("colorSpace"))
				{
					if (!pColorSpace->Is_String() ||
						!Parse_Token(pColorSpace->Get_String(),
							TEXTURE_COLOR_SPACE_TOKENS,
							std::size(TEXTURE_COLOR_SPACE_TOKENS),
							Texture.eColorSpace))
						return false;
				}
				if (const Client::DATA_JSON_VALUE* pEvidence =
					Item.Find("samplingEvidence"))
				{
					if (!pEvidence->Is_String())
						return false;
					Texture.strSamplingEvidence = pEvidence->Get_String();
				}
				Out.Textures.push_back(std::move(Texture));
			}
		}
		for (const Client::DATA_JSON_VALUE& Item : pScalars->Get_Array())
		{
			Client::EFFECT_NAMED_FLOAT_DESC Scalar;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Scalar.strName, strOutError) ||
				!Read_Float(Item, "value", Scalar.fValue, strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Scalar.strGroup = pGroup->Get_String();
			}
			Out.Scalars.push_back(std::move(Scalar));
		}
		for (const Client::DATA_JSON_VALUE& Item : pVectors->Get_Array())
		{
			Client::EFFECT_NAMED_FLOAT4_DESC Vector;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Vector.strName, strOutError) ||
				!Read_Array(Item, "value", &Vector.vValue.x, 4u,
					strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Vector.strGroup = pGroup->Get_String();
			}
			Out.Vectors.push_back(std::move(Vector));
		}
		for (const Client::DATA_JSON_VALUE& Item : pSwitches->Get_Array())
		{
			Client::EFFECT_NAMED_BOOL_DESC Switch;
			if (!Item.Is_Object() ||
				!Read_String(Item, "name", Switch.strName, strOutError) ||
				!Read_Bool(Item, "value", Switch.bValue, strOutError))
			{
				return false;
			}
			if (const Client::DATA_JSON_VALUE* pGroup = Item.Find("group"))
			{
				if (!pGroup->Is_String())
					return false;
				Switch.strGroup = pGroup->Get_String();
			}
			Out.StaticSwitches.push_back(std::move(Switch));
		}
		return true;
	}

	const char_t* SourceMaterialStatusToken(
		const Client::EFFECT_SOURCE_MATERIAL_STATUS eStatus)
	{
		const size_t iIndex = static_cast<size_t>(eStatus);
		return iIndex < std::size(SOURCE_MATERIAL_STATUS_TOKENS) ?
			SOURCE_MATERIAL_STATUS_TOKENS[iIndex] : "unsupported";
	}

	void Write_SourceMaterialProfile(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		Output << "{ \"enabled\": "
			<< (Source.bEnabled ? "true" : "false");
		/* Preserve optional disabled source evidence across Tool round trips.
		   Empty disabled profiles keep the compact legacy representation. */
		if (!Source.bEnabled && Source.strProfileId.empty())
		{
			Output << " }";
			return;
		}
		Output << ", \"profileId\": \""
			<< Client::CDataJson::Escape(Source.strProfileId)
			<< "\", \"runtimeShaderProfileId\": \""
			<< Client::CDataJson::Escape(Source.strRuntimeShaderProfileId)
			<< "\", \"parentMaterialPath\": \""
			<< Client::CDataJson::Escape(Source.strParentMaterialPath)
			<< "\", \"semanticStatus\": \""
			<< SourceMaterialStatusToken(Source.eStatus) << '"';
		if (Source.eSourceBlendClass !=
			Client::EFFECT_SOURCE_BLEND_CLASS::UNKNOWN)
		{
			Output << ", \"sourceBlendClass\": \""
				<< SOURCE_BLEND_CLASS_TOKENS[static_cast<size_t>(
					Source.eSourceBlendClass)] << '"';
		}
		Output << ", \"textures\": [";
		for (size_t i = 0u; i < Source.Textures.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Textures[i].strName)
				<< '"';
			if (!Source.Textures[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Textures[i].strGroup)
					<< '"';
			}
			Output << ", \"sourceObjectPath\": \""
				<< Client::CDataJson::Escape(
					Source.Textures[i].strSourceObjectPath)
				<< "\", \"assetId\": \""
				<< Client::CDataJson::Escape(Source.Textures[i].strAssetId)
				<< "\", \"addressU\": \""
				<< TEXTURE_ADDRESS_MODE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eAddressU)]
				<< "\", \"addressV\": \""
				<< TEXTURE_ADDRESS_MODE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eAddressV)]
				<< "\", \"colorSpace\": \""
				<< TEXTURE_COLOR_SPACE_TOKENS[
					static_cast<size_t>(Source.Textures[i].eColorSpace)]
				<< "\", \"samplingEvidence\": \""
				<< Client::CDataJson::Escape(
					Source.Textures[i].strSamplingEvidence)
				<< "\" }";
		}
		Output << "], \"scalars\": [";
		for (size_t i = 0u; i < Source.Scalars.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Scalars[i].strName)
				<< '"';
			if (!Source.Scalars[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Scalars[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": " << Source.Scalars[i].fValue << " }";
		}
		Output << "], \"vectors\": [";
		for (size_t i = 0u; i < Source.Vectors.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Source.Vectors[i].strName)
				<< '"';
			if (!Source.Vectors[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(Source.Vectors[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": ";
			Write_Float4(Output, Source.Vectors[i].vValue);
			Output << " }";
		}
		Output << "], \"staticSwitches\": [";
		for (size_t i = 0u; i < Source.StaticSwitches.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(
					Source.StaticSwitches[i].strName)
				<< '"';
			if (!Source.StaticSwitches[i].strGroup.empty())
			{
				Output << ", \"group\": \""
					<< Client::CDataJson::Escape(
						Source.StaticSwitches[i].strGroup)
					<< '"';
			}
			Output << ", \"value\": "
				<< (Source.StaticSwitches[i].bValue ? "true" : "false")
				<< " }";
		}
		Output << "], \"dynamicParameterSemantics\": [";
		for (size_t i = 0u; i < Source.DynamicParameterSemantics.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << '"' << Client::CDataJson::Escape(
				Source.DynamicParameterSemantics[i]) << '"';
		}
		Output << "], \"subUVMode\": \""
			<< Client::CDataJson::Escape(Source.strSubUVMode) << "\" }";
	}

	bool_t Read_MaterialSampler(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_SAMPLER_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "filter", "addressU", "addressV", "addressW", "mipLodBias",
				"maxAnisotropy", "comparison", "borderColor", "minLod",
				"maxLod" }, "Effect authored Material sampler", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pFilter = Find_Field(
			Value, "filter", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressU = Find_Field(
			Value, "addressU", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressV = Find_Field(
			Value, "addressV", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pAddressW = Find_Field(
			Value, "addressW", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pComparison = Find_Field(
			Value, "comparison", Client::DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pFilter || nullptr == pAddressU || nullptr == pAddressV ||
			nullptr == pAddressW || nullptr == pComparison ||
			!Parse_Token(pFilter->Get_String(), MATERIAL_TEXTURE_FILTER_TOKENS,
				std::size(MATERIAL_TEXTURE_FILTER_TOKENS), Out.eFilter) ||
			!Parse_Token(pAddressU->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressU) ||
			!Parse_Token(pAddressV->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressV) ||
			!Parse_Token(pAddressW->Get_String(),
				MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS,
				std::size(MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS), Out.eAddressW) ||
			!Parse_Token(pComparison->Get_String(),
				MATERIAL_COMPARISON_FUNCTION_TOKENS,
				std::size(MATERIAL_COMPARISON_FUNCTION_TOKENS), Out.eComparison) ||
			!Read_Float(Value, "mipLodBias", Out.fMipLodBias, strOutError) ||
			!Read_UInt(Value, "maxAnisotropy", Out.iMaxAnisotropy,
				strOutError) ||
			!Read_Array(Value, "borderColor", &Out.vBorderColor.x, 4u,
				strOutError) ||
			!Read_Float(Value, "minLod", Out.fMinLod, strOutError) ||
			!Read_Float(Value, "maxLod", Out.fMaxLod, strOutError))
		{
			return false;
		}
		return true;
	}

	void Write_MaterialSampler(
		std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler)
	{
		Output << "{ \"filter\": \""
			<< MATERIAL_TEXTURE_FILTER_TOKENS[
				static_cast<size_t>(Sampler.eFilter)]
			<< "\", \"addressU\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressU)]
			<< "\", \"addressV\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressV)]
			<< "\", \"addressW\": \""
			<< MATERIAL_TEXTURE_ADDRESS_MODE_TOKENS[
				static_cast<size_t>(Sampler.eAddressW)]
			<< "\", \"mipLodBias\": " << Sampler.fMipLodBias
			<< ", \"maxAnisotropy\": " << Sampler.iMaxAnisotropy
			<< ", \"comparison\": \""
			<< MATERIAL_COMPARISON_FUNCTION_TOKENS[
				static_cast<size_t>(Sampler.eComparison)]
			<< "\", \"borderColor\": ";
		Write_Float4(Output, Sampler.vBorderColor);
		Output << ", \"minLod\": " << Sampler.fMinLod
			<< ", \"maxLod\": " << Sampler.fMaxLod << " }";
	}

	bool_t Read_StandardColorV1(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_STANDARD_COLOR_V1_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "packetVersion", "baseRadianceLaneId", "baseRadianceChannel",
				"coverageLaneId", "coverageChannel", "emissiveMode",
				"lifetimeEnvelope", "dissolveMode", "dissolveLaneId",
				"dissolveChannel", "dissolveSoftness", "missingLanePolicy" },
			"Effect StandardColorV1 packet", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBaseChannel = Find_Field(
			Value, "baseRadianceChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pCoverageChannel = Find_Field(
			Value, "coverageChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pEmissiveMode = Find_Field(
			Value, "emissiveMode", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pLifetimeEnvelope = Find_Field(
			Value, "lifetimeEnvelope", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pDissolveMode = Find_Field(
			Value, "dissolveMode", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pDissolveChannel = Find_Field(
			Value, "dissolveChannel", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		const Client::DATA_JSON_VALUE* pMissingLanePolicy = Find_Field(
			Value, "missingLanePolicy", Client::DATA_JSON_TYPE::STRING,
			strOutError);
		return nullptr != pBaseChannel && nullptr != pCoverageChannel &&
			nullptr != pEmissiveMode && nullptr != pLifetimeEnvelope &&
			nullptr != pDissolveMode && nullptr != pDissolveChannel &&
			nullptr != pMissingLanePolicy &&
			Read_UInt(Value, "packetVersion", Out.iPacketVersion, strOutError) &&
			Read_String(Value, "baseRadianceLaneId",
				Out.strBaseRadianceLaneId, strOutError) &&
			Parse_Token(pBaseChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eBaseRadianceChannel) &&
			Read_String(Value, "coverageLaneId", Out.strCoverageLaneId,
				strOutError) &&
			Parse_Token(pCoverageChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eCoverageChannel) &&
			Parse_Token(pEmissiveMode->Get_String(),
				STANDARD_COLOR_EMISSIVE_MODE_TOKENS,
				std::size(STANDARD_COLOR_EMISSIVE_MODE_TOKENS),
				Out.eEmissiveMode) &&
			Parse_Token(pLifetimeEnvelope->Get_String(),
				STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS,
				std::size(STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS),
				Out.eLifetimeEnvelope) &&
			Parse_Token(pDissolveMode->Get_String(),
				STANDARD_COLOR_DISSOLVE_MODE_TOKENS,
				std::size(STANDARD_COLOR_DISSOLVE_MODE_TOKENS),
				Out.eDissolveMode) &&
			Read_String(Value, "dissolveLaneId", Out.strDissolveLaneId,
				strOutError) &&
			Parse_Token(pDissolveChannel->Get_String(),
				STANDARD_COLOR_CHANNEL_TOKENS,
				std::size(STANDARD_COLOR_CHANNEL_TOKENS),
				Out.eDissolveChannel) &&
			Read_Float(Value, "dissolveSoftness", Out.fDissolveSoftness,
				strOutError) &&
			Parse_Token(pMissingLanePolicy->Get_String(),
				STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS,
				std::size(STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS),
				Out.eMissingLanePolicy);
	}

	void Write_StandardColorV1(
		std::ostringstream& Output,
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet)
	{
		Output << "{ \"packetVersion\": " << Packet.iPacketVersion
			<< ", \"baseRadianceLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strBaseRadianceLaneId)
			<< "\", \"baseRadianceChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eBaseRadianceChannel)]
			<< "\", \"coverageLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strCoverageLaneId)
			<< "\", \"coverageChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eCoverageChannel)]
			<< "\", \"emissiveMode\": \""
			<< STANDARD_COLOR_EMISSIVE_MODE_TOKENS[
				static_cast<size_t>(Packet.eEmissiveMode)]
			<< "\", \"lifetimeEnvelope\": \""
			<< STANDARD_COLOR_LIFETIME_ENVELOPE_TOKENS[
				static_cast<size_t>(Packet.eLifetimeEnvelope)]
			<< "\", \"dissolveMode\": \""
			<< STANDARD_COLOR_DISSOLVE_MODE_TOKENS[
				static_cast<size_t>(Packet.eDissolveMode)]
			<< "\", \"dissolveLaneId\": \""
			<< Client::CDataJson::Escape(Packet.strDissolveLaneId)
			<< "\", \"dissolveChannel\": \""
			<< STANDARD_COLOR_CHANNEL_TOKENS[
				static_cast<size_t>(Packet.eDissolveChannel)]
			<< "\", \"dissolveSoftness\": " << Packet.fDissolveSoftness
			<< ", \"missingLanePolicy\": \""
			<< STANDARD_COLOR_MISSING_LANE_POLICY_TOKENS[
				static_cast<size_t>(Packet.eMissingLanePolicy)] << "\" }";
	}

	bool_t Read_MaterialExecution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MATERIAL_EXECUTION_DESC& Out,
		std::string& strOutError)
	{
		if (!Value.Is_Object() ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError))
		{
			return false;
		}
		if (!Out.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pFailClosed =
				Value.Find("failClosed");
			if (nullptr != pFailClosed)
			{
				if (!pFailClosed->Is_Boolean() ||
					!pFailClosed->Get_Boolean())
				{
					strOutError =
						"Disabled authored Material failClosed must be true when present.";
					return false;
				}
				Out.bFailClosed = true;
			}
			const Client::DATA_JSON_VALUE* pAuthoringApproximate =
				Value.Find("authoringApproximate");
			if (nullptr != pAuthoringApproximate)
			{
				if (!pAuthoringApproximate->Is_Boolean() ||
					!pAuthoringApproximate->Get_Boolean())
				{
					strOutError =
						"Disabled authored Material authoringApproximate must be true when present.";
					return false;
				}
				if (!Out.bFailClosed)
				{
					strOutError =
						"Authoring-approximate authored Material must stay fail-closed.";
					return false;
				}
				Out.bAuthoringApproximate = true;
			}
			const size_t iExpectedFieldCount = 1u +
				(Out.bFailClosed ? 1u : 0u) +
				(Out.bAuthoringApproximate ? 1u : 0u);
			if (iExpectedFieldCount != Value.Get_Object().size())
			{
				strOutError =
					"Disabled authored Material execution carries hidden state.";
				return false;
			}
			return true;
		}
		if (!Validate_ExactFields(Value,
			{ "enabled", "fidelity", "version", "backend", "opcode", "passIndex",
				"renderState", "textureLaneCount", "textureMask",
				"textureLanes", "dynamicConsumedMask",
				"dynamicSuppressedMask", "particleColorPolicy",
				"particleColorConsumedMask", "particleColorSuppressedMask",
				"scalarCount", "vectorCount", "inputCount",
				"inputConsumedMask", "inputSuppressedMask",
				"vectorComponentConsumedMask",
				"vectorComponentSuppressedMask", "staticInputCount",
				"staticSelectedMask", "staticConsumedMask",
				"staticSuppressedMask", "renderInputCount",
				"renderConsumedMask", "renderSuppressedMask", "scalars",
				"vectors", "artistParameters", "colors", "standardColor" },
			"Effect authored Material execution",
			strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBackend = Find_Field(
			Value, "backend", Client::DATA_JSON_TYPE::STRING, strOutError);
		Out.eFidelity =
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::SOURCE_EXACT;
		const Client::DATA_JSON_VALUE* pFidelity = Value.Find("fidelity");
		const Client::DATA_JSON_VALUE* pRenderState = Find_Field(
			Value, "renderState", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTextureLanes = Find_Field(
			Value, "textureLanes", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pScalars = Find_Field(
			Value, "scalars", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pVectors = Find_Field(
			Value, "vectors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pArtistParameters = Find_Field(
			Value, "artistParameters", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pColors = Find_Field(
			Value, "colors", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pStandardColor =
			Value.Find("standardColor");
		if (nullptr != pFidelity &&
			(!pFidelity->Is_String() ||
			 pFidelity->Get_String() != "PROJECT_TUNED_APPROX"))
		{
			strOutError = "Effect material execution fidelity is unsupported.";
			return false;
		}
		if (nullptr != pFidelity)
		{
			Out.eFidelity = Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::
				PROJECT_TUNED_APPROX;
		}
		if (nullptr == pBackend || nullptr == pRenderState ||
			nullptr == pTextureLanes || nullptr == pScalars ||
			nullptr == pVectors || nullptr == pArtistParameters ||
			nullptr == pColors ||
			!Parse_Token(pBackend->Get_String(),
				MATERIAL_EXECUTION_BACKEND_TOKENS,
				std::size(MATERIAL_EXECUTION_BACKEND_TOKENS), Out.eBackend) ||
			!Validate_ExactFields(*pRenderState,
				{ "rasterizer", "depthStencil", "blend", "stencilReference" },
				"Effect authored Material render state", strOutError) ||
			!Read_UInt(Value, "version", Out.iVersion, strOutError) ||
			!Read_UInt(Value, "opcode", Out.iOpcode, strOutError) ||
			!Read_UInt(Value, "passIndex", Out.iPassIndex, strOutError) ||
			!Read_String(*pRenderState, "rasterizer", Out.strRasterizerState,
				strOutError) ||
			!Read_String(*pRenderState, "depthStencil",
				Out.strDepthStencilState, strOutError) ||
			!Read_String(*pRenderState, "blend", Out.strBlendState,
				strOutError) ||
			!Read_UInt(*pRenderState, "stencilReference",
				Out.iStencilReference, strOutError) ||
			!Read_UInt(Value, "textureLaneCount", Out.iTextureLaneCount,
				strOutError) ||
			!Read_UInt(Value, "textureMask", Out.iTextureMask, strOutError) ||
			!Read_UInt(Value, "dynamicConsumedMask",
				Out.iDynamicConsumedMask, strOutError) ||
			!Read_UInt(Value, "dynamicSuppressedMask",
				Out.iDynamicSuppressedMask, strOutError) ||
			!Read_UInt(Value, "particleColorPolicy", Out.iParticleColorPolicy,
				strOutError) ||
			!Read_UInt(Value, "particleColorConsumedMask",
				Out.iParticleColorConsumedMask, strOutError) ||
			!Read_UInt(Value, "particleColorSuppressedMask",
				Out.iParticleColorSuppressedMask, strOutError) ||
			!Read_UInt(Value, "scalarCount", Out.iScalarCount, strOutError) ||
			!Read_UInt(Value, "vectorCount", Out.iVectorCount, strOutError) ||
			!Read_UInt(Value, "inputCount", Out.iInputCount, strOutError) ||
			!Read_UIntArray(Value, "inputConsumedMask", Out.InputConsumedMask,
				strOutError) ||
			!Read_UIntArray(Value, "inputSuppressedMask", Out.InputSuppressedMask,
				strOutError) ||
			!Read_UIntArray(Value, "vectorComponentConsumedMask",
				Out.VectorComponentConsumedMask, strOutError) ||
			!Read_UIntArray(Value, "vectorComponentSuppressedMask",
				Out.VectorComponentSuppressedMask, strOutError) ||
			!Read_UInt(Value, "staticInputCount", Out.iStaticInputCount,
				strOutError) ||
			!Read_UInt(Value, "staticSelectedMask", Out.iStaticSelectedMask,
				strOutError) ||
			!Read_UInt(Value, "staticConsumedMask", Out.iStaticConsumedMask,
				strOutError) ||
			!Read_UInt(Value, "staticSuppressedMask", Out.iStaticSuppressedMask,
				strOutError) ||
			!Read_UInt(Value, "renderInputCount", Out.iRenderInputCount,
				strOutError) ||
			!Read_UInt(Value, "renderConsumedMask", Out.iRenderConsumedMask,
				strOutError) ||
			!Read_UInt(Value, "renderSuppressedMask", Out.iRenderSuppressedMask,
				strOutError))
		{
			return false;
		}
		const bool_t bProjectTunedApprox = Out.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX;
		const bool_t bProjectTunedOpcode = Out.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			(Out.iOpcode == 1001u || Out.iOpcode == 1002u ||
			 Out.iOpcode == 1003u || Out.iOpcode == 1004u);
		if (bProjectTunedApprox != bProjectTunedOpcode)
		{
			strOutError =
				"Effect material execution fidelity/opcode contract changed.";
			return false;
		}
		if (nullptr != pStandardColor &&
			(!pStandardColor->Is_Object() ||
			 !Read_StandardColorV1(
				 *pStandardColor, Out.StandardColorV1, strOutError)))
		{
			return false;
		}
		const bool_t bStandardColorBackend = Out.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		if (bStandardColorBackend != (nullptr != pStandardColor))
		{
			strOutError =
				"Effect StandardColorV1 packet presence does not match its backend.";
			return false;
		}

		Out.TextureLanes.reserve(pTextureLanes->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& LaneValue :
			pTextureLanes->Get_Array())
		{
			if (!Validate_ExactFields(LaneValue,
				{ "laneId", "role", "assetId", "textureRegister",
					"samplerRegister", "sourceChannel", "colorSpace", "sampler" },
				"Effect authored Material texture lane", strOutError))
			{
				return false;
			}
			const Client::DATA_JSON_VALUE* pColorSpace = Find_Field(
				LaneValue, "colorSpace", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pSampler = Find_Field(
				LaneValue, "sampler", Client::DATA_JSON_TYPE::OBJECT,
				strOutError);
			Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC Lane;
			if (nullptr == pColorSpace || nullptr == pSampler ||
				!Read_String(LaneValue, "laneId", Lane.strLaneId, strOutError) ||
				!Read_String(LaneValue, "role", Lane.strRole, strOutError) ||
				!Read_String(LaneValue, "assetId", Lane.strAssetId,
					strOutError) ||
				!Read_UInt(LaneValue, "textureRegister", Lane.iTextureRegister,
					strOutError) ||
				!Read_UInt(LaneValue, "samplerRegister", Lane.iSamplerRegister,
					strOutError) ||
				!Read_String(LaneValue, "sourceChannel", Lane.strSourceChannel,
					strOutError) ||
				!Parse_Token(pColorSpace->Get_String(), TEXTURE_COLOR_SPACE_TOKENS,
					std::size(TEXTURE_COLOR_SPACE_TOKENS), Lane.eColorSpace) ||
				!Read_MaterialSampler(*pSampler, Lane.Sampler, strOutError))
			{
				return false;
			}
			Out.TextureLanes.push_back(std::move(Lane));
		}

		Out.Scalars.reserve(pScalars->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ParameterValue :
			pScalars->Get_Array())
		{
			if (!Validate_ExactFields(ParameterValue,
				{ "name", "packedIndex", "value" },
				"Effect authored Material scalar", strOutError))
			{
				return false;
			}
			Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC Parameter;
			if (!Read_String(ParameterValue, "name", Parameter.strName,
					strOutError) ||
				!Read_UInt(ParameterValue, "packedIndex", Parameter.iPackedIndex,
					strOutError) ||
				!Read_Float(ParameterValue, "value", Parameter.fValue,
					strOutError))
			{
				return false;
			}
			Out.Scalars.push_back(std::move(Parameter));
		}

		const auto ReadVectorParameters = [&strOutError](
			const Client::DATA_JSON_VALUE& Parameters,
			std::vector<Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& OutValues,
			const std::string_view strContext) -> bool_t
		{
			OutValues.reserve(Parameters.Get_Array().size());
			for (const Client::DATA_JSON_VALUE& ParameterValue :
				Parameters.Get_Array())
			{
				if (!Validate_ExactFields(ParameterValue,
					{ "name", "packedIndex", "value" }, strContext,
					strOutError))
				{
					return false;
				}
				Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC Parameter;
				if (!Read_String(ParameterValue, "name", Parameter.strName,
						strOutError) ||
					!Read_UInt(ParameterValue, "packedIndex",
						Parameter.iPackedIndex, strOutError) ||
					!Read_Array(ParameterValue, "value", &Parameter.vValue.x, 4u,
						strOutError))
				{
					return false;
				}
				OutValues.push_back(std::move(Parameter));
			}
			return true;
		};
		return ReadVectorParameters(*pVectors, Out.Vectors,
			"Effect authored Material vector") &&
			ReadVectorParameters(*pArtistParameters, Out.ArtistParameters,
				"Effect authored Artist Visual parameter") &&
			ReadVectorParameters(*pColors, Out.Colors,
				"Effect authored Material color");
	}

	void Write_MaterialExecution(
		std::ostringstream& Output,
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution)
	{
		Output << "{ \"enabled\": "
			<< (Execution.bEnabled ? "true" : "false");
		if (!Execution.bEnabled)
		{
			if (Execution.bFailClosed)
				Output << ", \"failClosed\": true";
			if (Execution.bAuthoringApproximate)
				Output << ", \"authoringApproximate\": true";
			Output << " }";
			return;
		}
		if (Execution.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX)
		{
			Output << ", \"fidelity\": \"PROJECT_TUNED_APPROX\"";
		}
		Output << ", \"version\": " << Execution.iVersion
			<< ", \"backend\": \""
			<< MATERIAL_EXECUTION_BACKEND_TOKENS[
				static_cast<size_t>(Execution.eBackend)]
			<< "\", \"opcode\": " << Execution.iOpcode
			<< ", \"passIndex\": " << Execution.iPassIndex
			<< ", \"renderState\": { \"rasterizer\": \""
			<< Client::CDataJson::Escape(Execution.strRasterizerState)
			<< "\", \"depthStencil\": \""
			<< Client::CDataJson::Escape(Execution.strDepthStencilState)
			<< "\", \"blend\": \""
			<< Client::CDataJson::Escape(Execution.strBlendState)
			<< "\", \"stencilReference\": " << Execution.iStencilReference
			<< " }, \"textureLaneCount\": " << Execution.iTextureLaneCount
			<< ", \"textureMask\": " << Execution.iTextureMask
			<< ", \"textureLanes\": [";
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			if (0u != iLane)
				Output << ", ";
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[iLane];
			Output << "{ \"laneId\": \""
				<< Client::CDataJson::Escape(Lane.strLaneId)
				<< "\", \"role\": \""
				<< Client::CDataJson::Escape(Lane.strRole)
				<< "\", \"assetId\": \""
				<< Client::CDataJson::Escape(Lane.strAssetId)
				<< "\", \"textureRegister\": " << Lane.iTextureRegister
				<< ", \"samplerRegister\": " << Lane.iSamplerRegister
				<< ", \"sourceChannel\": \""
				<< Client::CDataJson::Escape(Lane.strSourceChannel)
				<< "\", \"colorSpace\": \""
				<< TEXTURE_COLOR_SPACE_TOKENS[
					static_cast<size_t>(Lane.eColorSpace)]
				<< "\", \"sampler\": ";
			Write_MaterialSampler(Output, Lane.Sampler);
			Output << " }";
		}
		Output << "]";
		if (Execution.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1)
		{
			Output << ", \"standardColor\": ";
			Write_StandardColorV1(Output, Execution.StandardColorV1);
		}
		Output << ", \"dynamicConsumedMask\": " << Execution.iDynamicConsumedMask
			<< ", \"dynamicSuppressedMask\": "
			<< Execution.iDynamicSuppressedMask
			<< ", \"particleColorPolicy\": "
			<< Execution.iParticleColorPolicy
			<< ", \"particleColorConsumedMask\": "
			<< Execution.iParticleColorConsumedMask
			<< ", \"particleColorSuppressedMask\": "
			<< Execution.iParticleColorSuppressedMask
			<< ", \"scalarCount\": " << Execution.iScalarCount
			<< ", \"vectorCount\": " << Execution.iVectorCount
			<< ", \"inputCount\": " << Execution.iInputCount
			<< ", \"inputConsumedMask\": ";
		Write_UIntArray(Output, Execution.InputConsumedMask);
		Output << ", \"inputSuppressedMask\": ";
		Write_UIntArray(Output, Execution.InputSuppressedMask);
		Output << ", \"vectorComponentConsumedMask\": ";
		Write_UIntArray(Output, Execution.VectorComponentConsumedMask);
		Output << ", \"vectorComponentSuppressedMask\": ";
		Write_UIntArray(Output, Execution.VectorComponentSuppressedMask);
		Output << ", \"staticInputCount\": " << Execution.iStaticInputCount
			<< ", \"staticSelectedMask\": "
			<< Execution.iStaticSelectedMask
			<< ", \"staticConsumedMask\": "
			<< Execution.iStaticConsumedMask
			<< ", \"staticSuppressedMask\": "
			<< Execution.iStaticSuppressedMask
			<< ", \"renderInputCount\": " << Execution.iRenderInputCount
			<< ", \"renderConsumedMask\": "
			<< Execution.iRenderConsumedMask
			<< ", \"renderSuppressedMask\": "
			<< Execution.iRenderSuppressedMask
			<< ", \"scalars\": [";
		for (size_t i = 0u; i < Execution.Scalars.size(); ++i)
		{
			if (0u != i)
				Output << ", ";
			Output << "{ \"name\": \""
				<< Client::CDataJson::Escape(Execution.Scalars[i].strName)
				<< "\", \"packedIndex\": "
				<< Execution.Scalars[i].iPackedIndex
				<< ", \"value\": " << Execution.Scalars[i].fValue << " }";
		}
		const auto WriteVectorParameters = [&Output](const auto& Parameters)
		{
			for (size_t i = 0u; i < Parameters.size(); ++i)
			{
				if (0u != i)
					Output << ", ";
				Output << "{ \"name\": \""
					<< Client::CDataJson::Escape(Parameters[i].strName)
					<< "\", \"packedIndex\": "
					<< Parameters[i].iPackedIndex << ", \"value\": ";
				Write_Float4(Output, Parameters[i].vValue);
				Output << " }";
			}
		};
		Output << "], \"vectors\": [";
		WriteVectorParameters(Execution.Vectors);
		Output << "], \"artistParameters\": [";
		WriteVectorParameters(Execution.ArtistParameters);
		Output << "], \"colors\": [";
		WriteVectorParameters(Execution.Colors);
		Output << "] }";
	}

	bool_t Is_DefaultStandardColorV1(
		const Client::EFFECT_STANDARD_COLOR_V1_DESC& Packet)
	{
		return 0u == Packet.iPacketVersion &&
			Packet.strBaseRadianceLaneId.empty() &&
			Packet.eBaseRadianceChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			Packet.strCoverageLaneId.empty() &&
			Packet.eCoverageChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			Packet.eEmissiveMode ==
				Client::EFFECT_STANDARD_COLOR_EMISSIVE_MODE::NONE &&
			Packet.eLifetimeEnvelope ==
				Client::EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::INVALID &&
			Packet.eDissolveMode ==
				Client::EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE &&
			Packet.strDissolveLaneId.empty() &&
			Packet.eDissolveChannel ==
				Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID &&
			0.f == Packet.fDissolveSoftness &&
			Packet.eMissingLanePolicy ==
				Client::EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::INVALID;
	}

	std::string_view StandardColorChannelCharacters(
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel)
	{
		switch (eChannel)
		{
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::R:
			return "R";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::G:
			return "G";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::B:
			return "B";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::A:
			return "A";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::RGB:
			return "RGB";
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::INVALID:
		case Client::EFFECT_STANDARD_COLOR_CHANNEL::END:
		default:
			return {};
		}
	}

	bool_t Is_CanonicalStandardColorSourceChannel(
		const std::string_view strChannel)
	{
		if (strChannel.empty() || strChannel.size() > 4u)
			return false;
		size_t iPrevious = 0u;
		bool_t bFirst = true;
		for (const char_t Character : strChannel)
		{
			const size_t iPosition = std::string_view("RGBA").find(Character);
			if (iPosition == std::string_view::npos ||
				(!bFirst && iPosition <= iPrevious))
			{
				return false;
			}
			iPrevious = iPosition;
			bFirst = false;
		}
		return true;
	}

	bool_t StandardColorLaneContainsChannel(
		const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane,
		const Client::EFFECT_STANDARD_COLOR_CHANNEL eChannel)
	{
		const std::string_view Required =
			StandardColorChannelCharacters(eChannel);
		return !Required.empty() && std::all_of(
			Required.begin(), Required.end(), [&Lane](const char_t Character)
			{
				return Lane.strSourceChannel.find(Character) != std::string::npos;
			});
	}

	bool_t Validate_StandardColorV1Execution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError)
	{
		using namespace Client;
		const bool_t bStandard = Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		if (!bStandard)
		{
			if (!Is_DefaultStandardColorV1(Execution.StandardColorV1))
			{
				strOutError =
					"Non-StandardColor backend carries a StandardColorV1 packet.";
				return false;
			}
			return true;
		}

		const EFFECT_STANDARD_COLOR_V1_DESC& Packet =
			Execution.StandardColorV1;
		const auto AllZero = [](const auto& Values)
		{
			return std::all_of(Values.begin(), Values.end(),
				[](const uint32_t Value) { return 0u == Value; });
		};
		if (1u != Execution.iVersion || 1u != Execution.iOpcode ||
			1u != Packet.iPacketVersion || 0u == Execution.iTextureLaneCount ||
			0u != Execution.iDynamicConsumedMask ||
			0u != Execution.iDynamicSuppressedMask ||
			0u != Execution.iParticleColorPolicy ||
			0u != Execution.iParticleColorConsumedMask ||
			0u != Execution.iParticleColorSuppressedMask ||
			0u != Execution.iScalarCount || 0u != Execution.iVectorCount ||
			0u != Execution.iInputCount ||
			!AllZero(Execution.InputConsumedMask) ||
			!AllZero(Execution.InputSuppressedMask) ||
			!AllZero(Execution.VectorComponentConsumedMask) ||
			!AllZero(Execution.VectorComponentSuppressedMask) ||
			0u != Execution.iStaticInputCount ||
			0u != Execution.iStaticSelectedMask ||
			0u != Execution.iStaticConsumedMask ||
			0u != Execution.iStaticSuppressedMask ||
			0u != Execution.iRenderInputCount ||
			0u != Execution.iRenderConsumedMask ||
			0u != Execution.iRenderSuppressedMask ||
			!Execution.Scalars.empty() || !Execution.Vectors.empty() ||
			!Execution.ArtistParameters.empty() || !Execution.Colors.empty() ||
			Packet.eBaseRadianceChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
			(Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::R &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::G &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::B &&
			 Packet.eBaseRadianceChannel != EFFECT_STANDARD_COLOR_CHANNEL::RGB) ||
			Packet.eCoverageChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
			Packet.eCoverageChannel > EFFECT_STANDARD_COLOR_CHANNEL::A ||
			Packet.eEmissiveMode >= EFFECT_STANDARD_COLOR_EMISSIVE_MODE::END ||
			Packet.eLifetimeEnvelope !=
				EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::CARRIER_ALPHA ||
			Packet.eDissolveMode >= EFFECT_STANDARD_COLOR_DISSOLVE_MODE::END ||
			Packet.eMissingLanePolicy !=
				EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::FAIL_CLOSED ||
			!std::isfinite(Packet.fDissolveSoftness) ||
			Packet.fDissolveSoftness < 0.f || Packet.fDissolveSoftness > 1.f)
		{
			strOutError = "StandardColorV1 packet identity or hidden state is invalid.";
			return false;
		}

		const auto FindLane = [&Execution](const std::string_view strLaneId,
			size_t& iOutIndex) -> const EFFECT_MATERIAL_TEXTURE_LANE_DESC*
		{
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (Execution.TextureLanes[iLane].strLaneId == strLaneId)
				{
					iOutIndex =
						Execution.TextureLanes[iLane].iTextureRegister;
					return &Execution.TextureLanes[iLane];
				}
			}
			return nullptr;
		};
		size_t iBaseLane = 0u;
		size_t iCoverageLane = 0u;
		const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pBaseLane = FindLane(
			Packet.strBaseRadianceLaneId, iBaseLane);
		const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pCoverageLane = FindLane(
			Packet.strCoverageLaneId, iCoverageLane);
		if (nullptr == pBaseLane || nullptr == pCoverageLane ||
			!Is_CanonicalStandardColorSourceChannel(pBaseLane->strSourceChannel) ||
			!Is_CanonicalStandardColorSourceChannel(
				pCoverageLane->strSourceChannel) ||
			!StandardColorLaneContainsChannel(
				*pBaseLane, Packet.eBaseRadianceChannel) ||
			!StandardColorLaneContainsChannel(
				*pCoverageLane, Packet.eCoverageChannel) ||
			(Packet.eCoverageChannel != EFFECT_STANDARD_COLOR_CHANNEL::A &&
			 pCoverageLane->eColorSpace != EFFECT_TEXTURE_COLOR_SPACE::LINEAR))
		{
			strOutError = "StandardColorV1 base-radiance or coverage lane is invalid.";
			return false;
		}

		uint32_t iRequiredMask = (1u << iBaseLane) | (1u << iCoverageLane);
		if (Packet.eDissolveMode == EFFECT_STANDARD_COLOR_DISSOLVE_MODE::NONE)
		{
			if (!Packet.strDissolveLaneId.empty() ||
				Packet.eDissolveChannel != EFFECT_STANDARD_COLOR_CHANNEL::INVALID ||
				0.f != Packet.fDissolveSoftness)
			{
				strOutError = "Disabled StandardColorV1 dissolve carries hidden state.";
				return false;
			}
		}
		else
		{
			size_t iDissolveLane = 0u;
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pDissolveLane = FindLane(
				Packet.strDissolveLaneId, iDissolveLane);
			if (nullptr == pDissolveLane ||
				Packet.eDissolveChannel < EFFECT_STANDARD_COLOR_CHANNEL::R ||
				Packet.eDissolveChannel > EFFECT_STANDARD_COLOR_CHANNEL::A ||
				!Is_CanonicalStandardColorSourceChannel(
					pDissolveLane->strSourceChannel) ||
				!StandardColorLaneContainsChannel(
					*pDissolveLane, Packet.eDissolveChannel) ||
				(Packet.eDissolveChannel != EFFECT_STANDARD_COLOR_CHANNEL::A &&
				 pDissolveLane->eColorSpace != EFFECT_TEXTURE_COLOR_SPACE::LINEAR))
			{
				strOutError = "StandardColorV1 dissolve lane is invalid.";
				return false;
			}
			iRequiredMask |= 1u << iDissolveLane;
		}
		if (iRequiredMask != Execution.iTextureMask)
		{
			strOutError =
				"StandardColorV1 texture mask contains an unreferenced or missing lane.";
			return false;
		}
		return true;
	}

	bool_t Validate_MaterialExecution(
		const Client::EFFECT_MATERIAL_EXECUTION_DESC& Execution,
		std::string& strOutError)
	{
		const auto AllZero = [](const auto& Values)
		{
			return std::all_of(Values.begin(), Values.end(),
				[](const uint32_t Value) { return 0u == Value; });
		};
		if (!Execution.bEnabled)
		{
			if (Execution.bAuthoringApproximate && !Execution.bFailClosed)
			{
				strOutError =
					"Authoring-approximate authored Material must stay fail-closed.";
				return false;
			}
			if (Execution.eFidelity !=
					Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::SOURCE_EXACT ||
				1u != Execution.iVersion ||
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC !=
					Execution.eBackend ||
				0u != Execution.iOpcode || 0u != Execution.iPassIndex ||
				!Execution.strRasterizerState.empty() ||
				!Execution.strDepthStencilState.empty() ||
				!Execution.strBlendState.empty() ||
				0u != Execution.iStencilReference ||
				0u != Execution.iTextureLaneCount ||
				0u != Execution.iTextureMask || !Execution.TextureLanes.empty() ||
				!Is_DefaultStandardColorV1(Execution.StandardColorV1) ||
				0u != Execution.iDynamicConsumedMask ||
				0u != Execution.iDynamicSuppressedMask ||
				0u != Execution.iParticleColorPolicy ||
				0u != Execution.iParticleColorConsumedMask ||
				0u != Execution.iParticleColorSuppressedMask ||
				0u != Execution.iScalarCount || 0u != Execution.iVectorCount ||
				0u != Execution.iInputCount ||
				!AllZero(Execution.InputConsumedMask) ||
				!AllZero(Execution.InputSuppressedMask) ||
				!AllZero(Execution.VectorComponentConsumedMask) ||
				!AllZero(Execution.VectorComponentSuppressedMask) ||
				0u != Execution.iStaticInputCount ||
				0u != Execution.iStaticSelectedMask ||
				0u != Execution.iStaticConsumedMask ||
				0u != Execution.iStaticSuppressedMask ||
				0u != Execution.iRenderInputCount ||
				0u != Execution.iRenderConsumedMask ||
				0u != Execution.iRenderSuppressedMask ||
				!Execution.Scalars.empty() || !Execution.Vectors.empty() ||
				!Execution.ArtistParameters.empty() || !Execution.Colors.empty())
			{
				strOutError =
					"Disabled authored Material execution carries hidden state.";
				return false;
			}
			return true;
		}
		if (Execution.bFailClosed)
		{
			strOutError =
				"Enabled authored Material execution cannot also be fail-closed.";
			return false;
		}
		if (Execution.bAuthoringApproximate)
		{
			strOutError =
				"Enabled authored Material execution cannot be authoring-approximate.";
			return false;
		}
		const bool_t bProjectTunedApprox = Execution.eFidelity ==
			Client::EFFECT_MATERIAL_EXECUTION_FIDELITY::PROJECT_TUNED_APPROX;
		const bool_t bProjectTunedOpcode = Execution.eBackend ==
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			(Execution.iOpcode == 1001u || Execution.iOpcode == 1002u ||
			 Execution.iOpcode == 1003u || Execution.iOpcode == 1004u);
		if (bProjectTunedApprox != bProjectTunedOpcode)
		{
			strOutError =
				"Enabled material execution fidelity/opcode contract changed.";
			return false;
		}

		if (1u != Execution.iVersion ||
			Execution.eBackend <=
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC ||
			Execution.eBackend >=
				Client::EFFECT_MATERIAL_EXECUTION_BACKEND::END ||
			Execution.iOpcode > 65535u || Execution.iPassIndex > 63u ||
			!Is_StableId(Execution.strRasterizerState) ||
			!Is_StableId(Execution.strDepthStencilState) ||
			!Is_StableId(Execution.strBlendState) ||
			Execution.iStencilReference > 255u ||
			Execution.iTextureLaneCount > MAX_AUTHORED_MATERIAL_TEXTURE_LANES ||
			Execution.iTextureLaneCount != Execution.TextureLanes.size())
		{
			strOutError =
				"Authored Material execution identity, pass, or lane count is invalid.";
			return false;
		}
		const uint32_t iExpectedTextureMask =
			0u == Execution.iTextureLaneCount ? 0u :
				((1u << Execution.iTextureLaneCount) - 1u);
		if (Execution.iTextureMask != iExpectedTextureMask ||
			(Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL ==
				Execution.eBackend && 6u != Execution.iTextureLaneCount))
		{
			strOutError =
				"Authored Material texture mask is not the bounded contiguous lane contract.";
			return false;
		}

		std::unordered_set<std::string> LaneIds;
		std::unordered_set<uint32_t> TextureRegisters;
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				Execution.TextureLanes[iLane];
			Client::EFFECT_RESOURCE_FILE_KIND eActualKind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			const bool_t bChannelValid = Lane.strSourceChannel.empty() ||
				(Lane.strSourceChannel.size() <= 4u &&
					std::all_of(Lane.strSourceChannel.begin(),
						Lane.strSourceChannel.end(), [](const char_t Character)
						{
							return std::string_view("RGBA").find(Character) !=
								std::string_view::npos;
						}));
			const bool_t bChannelRequired =
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL ||
				Execution.eBackend ==
					Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
			const Client::EFFECT_MATERIAL_SAMPLER_DESC& Sampler = Lane.Sampler;
			const bool_t bLaneIdStable = Is_StableId(Lane.strLaneId);
			const bool_t bRoleStable = Is_StableId(Lane.strRole);
			const bool_t bLaneIdUnique = LaneIds.insert(Lane.strLaneId).second;
			const bool_t bSafeAsset =
				Client::CEffectDocumentCodec::Is_SafeResourceAssetId(
					Lane.strAssetId, &eActualKind);
			const bool_t bTextureRegisterUnique =
				TextureRegisters.insert(Lane.iTextureRegister).second;
			if (!bLaneIdStable || !bRoleStable || !bLaneIdUnique || !bSafeAsset ||
				eActualKind != Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE ||
				Lane.iTextureRegister != iLane ||
				!bTextureRegisterUnique ||
				Lane.iSamplerRegister != 5u + iLane || !bChannelValid ||
				(bChannelRequired && Lane.strSourceChannel.empty()) ||
				Lane.eColorSpace >= Client::EFFECT_TEXTURE_COLOR_SPACE::END ||
				Sampler.eFilter >= Client::EFFECT_MATERIAL_TEXTURE_FILTER::END ||
				Sampler.eAddressU >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eAddressV >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eAddressW >=
					Client::EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::END ||
				Sampler.eComparison >=
					Client::EFFECT_MATERIAL_COMPARISON_FUNCTION::END ||
				!std::isfinite(Sampler.fMipLodBias) ||
				std::abs(Sampler.fMipLodBias) > 16.f ||
				Sampler.iMaxAnisotropy < 1u || Sampler.iMaxAnisotropy > 16u ||
				!Is_Finite(Sampler.vBorderColor) ||
				!std::isfinite(Sampler.fMinLod) ||
				!std::isfinite(Sampler.fMaxLod) ||
				Sampler.fMinLod > Sampler.fMaxLod)
			{
				std::ostringstream Detail;
				Detail << "Authored Material texture lane or sampler is invalid: "
					<< Lane.strLaneId << " (role=" << Lane.strRole
					<< ", asset=" << Lane.strAssetId
					<< ", stable=" << bLaneIdStable << "/" << bRoleStable
					<< ", unique=" << bLaneIdUnique << "/"
					<< bTextureRegisterUnique << ", safe=" << bSafeAsset
					<< ", register=" << Lane.iTextureRegister << "/"
					<< Lane.iSamplerRegister << ", channel="
					<< Lane.strSourceChannel << ", colorSpace="
					<< static_cast<uint32_t>(Lane.eColorSpace)
					<< ", filter=" << static_cast<uint32_t>(Sampler.eFilter)
					<< ", address=" << static_cast<uint32_t>(Sampler.eAddressU)
					<< "/" << static_cast<uint32_t>(Sampler.eAddressV)
					<< "/" << static_cast<uint32_t>(Sampler.eAddressW)
					<< ", comparison="
					<< static_cast<uint32_t>(Sampler.eComparison)
					<< ", mipBias=" << Sampler.fMipLodBias
					<< ", anisotropy=" << Sampler.iMaxAnisotropy
					<< ", lod=" << Sampler.fMinLod << "/"
					<< Sampler.fMaxLod << ").";
				strOutError = Detail.str();
				return false;
			}
		}

		const auto MaskWithinCount = [](const uint32_t Mask,
			const uint32_t Count)
		{
			if (Count >= 32u)
				return true;
			const uint32_t Allowed = 0u == Count ? 0u : (1u << Count) - 1u;
			return 0u == (Mask & ~Allowed);
		};
		const auto MasksDisjointWithinCount = [&MaskWithinCount](
			const uint32_t Consumed, const uint32_t Suppressed,
			const uint32_t Count)
		{
			return 0u == (Consumed & Suppressed) &&
				MaskWithinCount(Consumed, Count) &&
				MaskWithinCount(Suppressed, Count);
		};
		if (Execution.iDynamicConsumedMask > 0x0fu ||
			Execution.iDynamicSuppressedMask > 0x0fu ||
			0u != (Execution.iDynamicConsumedMask &
				Execution.iDynamicSuppressedMask) ||
			Execution.iParticleColorPolicy > 3u ||
			Execution.iParticleColorConsumedMask > 0x0fu ||
			Execution.iParticleColorSuppressedMask > 0x0fu ||
			0u != (Execution.iParticleColorConsumedMask &
				Execution.iParticleColorSuppressedMask) ||
			Execution.iScalarCount > MAX_AUTHORED_MATERIAL_SCALARS ||
			Execution.iVectorCount > 3u || Execution.iInputCount > 64u ||
			Execution.iStaticInputCount > 32u ||
			Execution.iRenderInputCount > 32u ||
			Execution.Scalars.size() != Execution.iScalarCount ||
			Execution.Vectors.size() != Execution.iVectorCount ||
			!MasksDisjointWithinCount(Execution.InputConsumedMask[0u],
				Execution.InputSuppressedMask[0u],
				std::min(Execution.iInputCount, 32u)) ||
			!MasksDisjointWithinCount(Execution.InputConsumedMask[1u],
				Execution.InputSuppressedMask[1u],
				Execution.iInputCount > 32u ? Execution.iInputCount - 32u : 0u) ||
			!MasksDisjointWithinCount(Execution.iStaticConsumedMask,
				Execution.iStaticSuppressedMask, Execution.iStaticInputCount) ||
			!MaskWithinCount(Execution.iStaticSelectedMask,
				Execution.iStaticInputCount) ||
			0u != (Execution.iStaticSelectedMask &
				~(Execution.iStaticConsumedMask |
				  Execution.iStaticSuppressedMask)) ||
			!MasksDisjointWithinCount(Execution.iRenderConsumedMask,
				Execution.iRenderSuppressedMask, Execution.iRenderInputCount))
		{
			std::ostringstream Detail;
			Detail << "Authored Material packed counts or masks are invalid"
				<< " (dynamic=" << Execution.iDynamicConsumedMask << "/"
				<< Execution.iDynamicSuppressedMask << ", particleColor="
				<< Execution.iParticleColorPolicy << "/"
				<< Execution.iParticleColorConsumedMask << "/"
				<< Execution.iParticleColorSuppressedMask << ", scalar="
				<< Execution.iScalarCount << "/" << Execution.Scalars.size()
				<< ", vector=" << Execution.iVectorCount << "/"
				<< Execution.Vectors.size() << ", input="
				<< Execution.iInputCount << "/" << std::hex
				<< Execution.InputConsumedMask[0u] << "/"
				<< Execution.InputSuppressedMask[0u] << "/"
				<< Execution.InputConsumedMask[1u] << "/"
				<< Execution.InputSuppressedMask[1u] << std::dec
				<< ", static=" << Execution.iStaticInputCount << "/"
				<< Execution.iStaticSelectedMask << "/"
				<< Execution.iStaticConsumedMask << "/"
				<< Execution.iStaticSuppressedMask << ", render="
				<< Execution.iRenderInputCount << "/"
				<< Execution.iRenderConsumedMask << "/"
				<< Execution.iRenderSuppressedMask << ").";
			strOutError = Detail.str();
			return false;
		}
		for (size_t iVector = 0u;
			iVector < Execution.VectorComponentConsumedMask.size(); ++iVector)
		{
			const uint32_t iComponentCount =
				iVector < Execution.iVectorCount ? 4u : 0u;
			if (!MasksDisjointWithinCount(
					Execution.VectorComponentConsumedMask[iVector],
					Execution.VectorComponentSuppressedMask[iVector],
					iComponentCount))
			{
				strOutError =
					"Authored Material vector component masks are invalid.";
				return false;
			}
		}

		const auto ValidateParameterNames = [](const auto& Parameters,
			const uint32_t iPackedCount)
		{
			std::unordered_set<std::string> Names;
			std::unordered_set<uint32_t> Indices;
			for (const auto& Parameter : Parameters)
			{
				if (!Is_StableId(Parameter.strName) ||
					Parameter.iPackedIndex >= iPackedCount ||
					!Names.insert(Parameter.strName).second ||
					!Indices.insert(Parameter.iPackedIndex).second)
				{
					return false;
				}
			}
			return true;
		};
		if (!ValidateParameterNames(Execution.Scalars, Execution.iScalarCount) ||
			!ValidateParameterNames(Execution.Vectors, Execution.iVectorCount) ||
			!std::all_of(Execution.Scalars.begin(), Execution.Scalars.end(),
				[](const Client::EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Parameter)
				{
					return std::isfinite(Parameter.fValue);
				}) ||
			!std::all_of(Execution.Vectors.begin(), Execution.Vectors.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}))
		{
			strOutError = "Authored Material packed scalar or vector is invalid.";
			return false;
		}

		const bool_t bArtist =
			Client::EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4 ==
				Execution.eBackend;
		if ((!bArtist && (!Execution.ArtistParameters.empty() ||
				!Execution.Colors.empty())) ||
			Execution.ArtistParameters.size() > MAX_AUTHORED_MATERIAL_VECTORS ||
			Execution.Colors.size() > MAX_AUTHORED_MATERIAL_COLORS ||
			!ValidateParameterNames(Execution.ArtistParameters,
				static_cast<uint32_t>(MAX_AUTHORED_MATERIAL_VECTORS)) ||
			!ValidateParameterNames(Execution.Colors,
				static_cast<uint32_t>(MAX_AUTHORED_MATERIAL_COLORS)) ||
			!std::all_of(Execution.ArtistParameters.begin(),
				Execution.ArtistParameters.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}) ||
			!std::all_of(Execution.Colors.begin(), Execution.Colors.end(),
				[](const Client::EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Parameter)
				{
					return Is_Finite(Parameter.vValue);
				}))
		{
			strOutError = "Authored Artist Visual parameter or color is invalid.";
			return false;
		}
		return Validate_StandardColorV1Execution(Execution, strOutError);
	}

	bool_t Read_SourceAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_ADMISSION_DESC& Out,
		std::string& strOutError);

	bool_t Read_Distribution(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DISTRIBUTION_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError)
	{
		if (!bSourceContract)
		{
			constexpr const char_t* SourceOnlyFields[] = {
				"referenceId", "occurrenceId", "payloadStatus", "fidelity",
				"executionAdmission", "parameterBinding", "parameterName"
			};
			for (const char_t* pField : SourceOnlyFields)
			{
				if (nullptr != Value.Find(pField))
				{
					strOutError =
						"Legacy Effect distribution contains native-v14 evidence.";
					return false;
				}
			}
		}
		if (bSourceContract && !Validate_ExactFields(Value,
			{ "propertyPath", "referenceId", "occurrenceId", "payloadStatus",
				"fidelity", "executionAdmission", "sourceClass", "sourceObjectPath",
				"parameterBinding", "parameterName", "componentCount",
				"operation", "randomLockAxes", "lookupTableChunkSize",
				"lookupTableNumElements", "lookupTableTimeScale",
				"lookupTableStartTime", "defaultMinimum", "defaultMaximum",
				"lookupTable", "keys" },
			"Effect source distribution", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pExecutionAdmission = bSourceContract ?
			Find_Field(Value, "executionAdmission",
				Client::DATA_JSON_TYPE::OBJECT, strOutError) : nullptr;
		if (bSourceContract &&
			(nullptr == pExecutionAdmission ||
			 !Read_String(Value, "referenceId", Out.strReferenceId,
				strOutError) ||
			 !Read_String(Value, "occurrenceId", Out.strOccurrenceId,
				strOutError) ||
			 !Read_String(Value, "payloadStatus", Out.strPayloadStatus,
				strOutError) ||
			 !Read_String(Value, "fidelity", Out.strFidelity, strOutError) ||
			 !Read_SourceAdmission(*pExecutionAdmission,
				Out.ExecutionAdmission, strOutError)))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pLookupTable = Find_Field(
			Value, "lookupTable", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pKeys = Find_Field(
			Value, "keys", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pLookupTable || nullptr == pKeys ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Read_String(Value, "sourceClass", Out.strSourceClass,
				strOutError) ||
			!Read_String(Value, "sourceObjectPath", Out.strSourceObjectPath,
				strOutError) ||
			!Read_UInt(Value, "componentCount", Out.iComponentCount,
				strOutError) ||
			!Read_UInt(Value, "operation", Out.iOperation, strOutError) ||
			!Read_UInt(Value, "lookupTableChunkSize",
				Out.iLookupTableChunkSize, strOutError) ||
			!Read_UInt(Value, "lookupTableNumElements",
				Out.iLookupTableNumElements, strOutError) ||
			!Read_Float(Value, "lookupTableTimeScale",
				Out.fLookupTableTimeScale, strOutError) ||
			!Read_Float(Value, "lookupTableStartTime",
				Out.fLookupTableStartTime, strOutError) ||
			!Read_Array(Value, "defaultMinimum", &Out.vDefaultMinimum.x,
				4u, strOutError) ||
			!Read_Array(Value, "defaultMaximum", &Out.vDefaultMaximum.x,
				4u, strOutError))
		{
			return false;
		}
		if (bSourceContract)
		{
			const Client::DATA_JSON_VALUE* pBinding = Value.Find(
				"parameterBinding");
			const Client::DATA_JSON_VALUE* pName = Value.Find("parameterName");
			const bool_t bParticleParameter =
				Is_ParticleParameterDistribution(Out.strSourceClass);
			if (bParticleParameter)
			{
				if (nullptr == pBinding || !pBinding->Is_String() ||
					nullptr == pName || !pName->Is_String() ||
					!Parse_Token(pBinding->Get_String(),
						DISTRIBUTION_PARAMETER_BINDING_TOKENS,
						std::size(DISTRIBUTION_PARAMETER_BINDING_TOKENS),
						Out.eParameterBinding))
				{
					strOutError =
						"Effect ParticleParameter binding is missing or invalid.";
					return false;
				}
				Out.strParameterName = pName->Get_String();
				if ((Client::EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ==
						Out.eParameterBinding && !Out.strParameterName.empty()) ||
					(Client::EFFECT_DISTRIBUTION_PARAMETER_BINDING::ACTION_CUE ==
						Out.eParameterBinding && Out.strParameterName.empty()))
				{
					strOutError =
						"Effect ParticleParameter name and binding disagree.";
					return false;
				}
			}
			else if (nullptr != pBinding || nullptr != pName)
			{
				strOutError =
					"Non-ParticleParameter distribution carries source binding fields.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pRandomLockAxes =
			Value.Find("randomLockAxes");
		if (bSourceContract && nullptr == pRandomLockAxes)
		{
			strOutError = "Missing or invalid field: randomLockAxes";
			return false;
		}
		if (nullptr != pRandomLockAxes)
		{
			if (!pRandomLockAxes->Is_Number() ||
				!std::isfinite(pRandomLockAxes->Get_Number()) ||
				pRandomLockAxes->Get_Number() !=
					std::floor(pRandomLockAxes->Get_Number()) ||
				pRandomLockAxes->Get_Number() < 0.0 ||
				pRandomLockAxes->Get_Number() > 4.0)
			{
				strOutError = "Effect distribution randomLockAxes is invalid.";
				return false;
			}
			Out.iRandomLockAxes = static_cast<uint32_t>(
				pRandomLockAxes->Get_Number());
		}
		Out.LookupTable.reserve(pLookupTable->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& Item : pLookupTable->Get_Array())
		{
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
			{
				strOutError = "Effect distribution lookup table is invalid.";
				return false;
			}
			Out.LookupTable.push_back(static_cast<f32_t>(Item.Get_Number()));
		}
		Out.Keys.reserve(pKeys->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& KeyValue : pKeys->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(KeyValue,
				{ "time", "minimum", "maximum", "arriveTangentMinimum",
					"leaveTangentMinimum", "arriveTangentMaximum",
					"leaveTangentMaximum", "interpolation" },
				"Effect source distribution key", strOutError))
			{
				return false;
			}
			const Client::DATA_JSON_VALUE* pInterpolation =
				KeyValue.Is_Object() ? KeyValue.Find("interpolation") : nullptr;
			Client::EFFECT_DISTRIBUTION_KEY_DESC Key;
			if (nullptr == pInterpolation || !pInterpolation->Is_String() ||
				!Parse_Token(pInterpolation->Get_String(),
					DISTRIBUTION_INTERPOLATION_TOKENS,
					std::size(DISTRIBUTION_INTERPOLATION_TOKENS),
					Key.eInterpolation) ||
				!Read_Float(KeyValue, "time", Key.fTime, strOutError) ||
				!Read_Array(KeyValue, "minimum", &Key.vMinimum.x, 4u,
					strOutError) ||
				!Read_Array(KeyValue, "maximum", &Key.vMaximum.x, 4u,
					strOutError) ||
				!Read_Array(KeyValue, "arriveTangentMinimum",
					&Key.vArriveTangentMinimum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "leaveTangentMinimum",
					&Key.vLeaveTangentMinimum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "arriveTangentMaximum",
					&Key.vArriveTangentMaximum.x, 4u, strOutError) ||
				!Read_Array(KeyValue, "leaveTangentMaximum",
					&Key.vLeaveTangentMaximum.x, 4u, strOutError))
			{
				return false;
			}
			Out.Keys.push_back(std::move(Key));
		}
		return true;
	}

	bool_t Read_SourceCompilerEvidence(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "artifactFileSha256", "artifactSelfSha256", "evidenceId",
				"sourceEvidenceStatus", "sourceCueId", "sourceOccurrenceId",
				"sourceSystemId", "sourceEmitterPath", "sourceEmitterNodeId",
				"lodSelectionPolicy", "selectedLodPath", "selectedLodNodeId",
				"selectedLodArrayIndex", "selectedLodLevelProvenance",
				"selectedLodEnabledProvenance", "nonSelectedLodCount",
				"moduleReferenceOrder", "cueLocalTransform",
				"parameterOverrides", "compositionOrder",
				"localReferenceClosureFileSha256",
				"localReferenceClosureSelfSha256", "geometryParityFileSha256",
				"geometryParitySelfSha256" },
			"Effect source compiler evidence", strOutError))
			return false;

		const Client::DATA_JSON_VALUE* pModules = Find_Field(
			Value, "moduleReferenceOrder", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pCueTransform = Find_Field(
			Value, "cueLocalTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		const Client::DATA_JSON_VALUE* pParameters = Find_Field(
			Value, "parameterOverrides", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		if (nullptr == pModules || nullptr == pCueTransform ||
			nullptr == pParameters ||
			!Read_String(Value, "artifactFileSha256",
				Out.strArtifactFileSha256, strOutError) ||
			!Read_String(Value, "artifactSelfSha256",
				Out.strArtifactSelfSha256, strOutError) ||
			!Read_String(Value, "evidenceId", Out.strEvidenceId, strOutError) ||
			!Read_String(Value, "sourceEvidenceStatus",
				Out.strSourceEvidenceStatus, strOutError) ||
			!Read_String(Value, "sourceCueId", Out.strSourceCueId, strOutError) ||
			!Read_String(Value, "sourceOccurrenceId",
				Out.strSourceOccurrenceId, strOutError) ||
			!Read_String(Value, "sourceSystemId", Out.strSourceSystemId,
				strOutError) ||
			!Read_String(Value, "sourceEmitterPath", Out.strSourceEmitterPath,
				strOutError) ||
			!Read_String(Value, "sourceEmitterNodeId",
				Out.strSourceEmitterNodeId, strOutError) ||
			!Read_String(Value, "lodSelectionPolicy", Out.strLodSelectionPolicy,
				strOutError) ||
			!Read_String(Value, "selectedLodPath", Out.strSelectedLodPath,
				strOutError) ||
			!Read_String(Value, "selectedLodNodeId", Out.strSelectedLodNodeId,
				strOutError) ||
			!Read_UInt(Value, "selectedLodArrayIndex",
				Out.iSelectedLodArrayIndex, strOutError) ||
			!Read_String(Value, "selectedLodLevelProvenance",
				Out.strSelectedLodLevelProvenance, strOutError) ||
			!Read_String(Value, "selectedLodEnabledProvenance",
				Out.strSelectedLodEnabledProvenance, strOutError) ||
			!Read_UInt(Value, "nonSelectedLodCount", Out.iNonSelectedLodCount,
				strOutError) ||
			!Read_StringArray(Value, "compositionOrder", Out.CompositionOrder,
				strOutError) ||
			!Read_String(Value, "localReferenceClosureFileSha256",
				Out.strLocalReferenceClosureFileSha256, strOutError) ||
			!Read_String(Value, "localReferenceClosureSelfSha256",
				Out.strLocalReferenceClosureSelfSha256, strOutError) ||
			!Read_String(Value, "geometryParityFileSha256",
				Out.strGeometryParityFileSha256, strOutError) ||
			!Read_String(Value, "geometryParitySelfSha256",
				Out.strGeometryParitySelfSha256, strOutError) ||
			!Validate_ExactFields(*pCueTransform,
				{ "sourcePositionUeUnits", "position", "rotationDegrees", "scale" },
				"Effect source cue-local transform", strOutError) ||
			!Read_Array(*pCueTransform, "sourcePositionUeUnits",
				&Out.vCueSourcePositionUeUnits.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "position",
				&Out.CueLocalTransform.vPosition.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "rotationDegrees",
				&Out.CueLocalTransform.vRotationDegrees.x, 3u, strOutError) ||
			!Read_Array(*pCueTransform, "scale",
				&Out.CueLocalTransform.vScale.x, 3u, strOutError))
			return false;

		Out.ModuleReferenceOrder.reserve(pModules->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ModuleValue : pModules->Get_Array())
		{
			if (!Validate_ExactFields(ModuleValue,
				{ "order", "sourceReferenceIndex", "role", "sourceObjectId",
					"sourceRecordSha256" }, "Effect source module reference",
				strOutError))
				return false;
			Client::EFFECT_SOURCE_MODULE_REFERENCE_DESC Module;
			if (!Read_UInt(ModuleValue, "order", Module.iOrder, strOutError) ||
				!Read_UInt(ModuleValue, "sourceReferenceIndex",
					Module.iSourceReferenceIndex, strOutError) ||
				!Read_String(ModuleValue, "role", Module.strRole, strOutError) ||
				!Read_String(ModuleValue, "sourceObjectId",
					Module.strSourceObjectId, strOutError) ||
				!Read_String(ModuleValue, "sourceRecordSha256",
					Module.strSourceRecordSha256, strOutError))
				return false;
			Out.ModuleReferenceOrder.push_back(std::move(Module));
		}

		Out.ParameterOverrides.reserve(pParameters->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ParameterValue :
			pParameters->Get_Array())
		{
			if (!Validate_ExactFields(ParameterValue,
				{ "sourceIndex", "name", "sourceTypeCode",
					"sourceRecordByteOffset", "type", "scalarValue", "vectorValue",
					"sourceValueByteOffset" }, "Effect source parameter override",
				strOutError))
				return false;
			Client::EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC Parameter;
			if (!Read_UInt(ParameterValue, "sourceIndex", Parameter.iSourceIndex,
					strOutError) ||
				!Read_String(ParameterValue, "name", Parameter.strName,
					strOutError) ||
				!Read_UInt(ParameterValue, "sourceTypeCode",
					Parameter.iSourceTypeCode, strOutError) ||
				!Read_UInt(ParameterValue, "sourceRecordByteOffset",
					Parameter.iSourceRecordByteOffset, strOutError) ||
				!Read_String(ParameterValue, "type", Parameter.strType,
					strOutError) ||
				!Read_UInt(ParameterValue, "sourceValueByteOffset",
					Parameter.iSourceValueByteOffset, strOutError))
				return false;
			if (Parameter.strType == "scalar")
			{
				if (nullptr != ParameterValue.Find("vectorValue") ||
					!Read_Double(ParameterValue, "scalarValue",
						Parameter.fScalarValue, strOutError))
					return false;
			}
			else if (Parameter.strType == "vector")
			{
				if (nullptr != ParameterValue.Find("scalarValue") ||
					!Read_Array(ParameterValue, "vectorValue",
						&Parameter.vVectorValue.x, 3u, strOutError))
					return false;
			}
			else
			{
				strOutError = "Effect source parameter override type is invalid.";
				return false;
			}
			Out.ParameterOverrides.push_back(std::move(Parameter));
		}
		return true;
	}

	bool_t Read_SourceAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_ADMISSION_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value, { "allowed", "blockers" },
			"Effect source execution admission", strOutError) &&
			Read_Bool(Value, "allowed", Out.bAllowed, strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}

	bool_t Read_SourceTypedField(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_TYPED_FIELD_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value, { "propertyPath", "kind", "value" },
			"Effect source typed evidence field", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pKind = Find_Field(
			Value, "kind", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pValue = Value.Is_Object() ?
			Value.Find("value") : nullptr;
		if (nullptr == pKind || nullptr == pValue ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Parse_Token(pKind->Get_String(), SOURCE_TYPED_FIELD_KIND_TOKENS,
				std::size(SOURCE_TYPED_FIELD_KIND_TOKENS), Out.eKind))
		{
			if (strOutError.empty())
				strOutError = "Effect source typed evidence kind is invalid.";
			return false;
		}
		switch (Out.eKind)
		{
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN:
			if (!pValue->Is_Boolean())
			{
				strOutError = "Effect source typed Boolean evidence is invalid.";
				return false;
			}
			Out.bBoolean = pValue->Get_Boolean();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER:
			if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()))
			{
				strOutError = "Effect source typed number evidence is invalid.";
				return false;
			}
			Out.fNumber = pValue->Get_Number();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::STRING:
			if (!pValue->Is_String())
			{
				strOutError = "Effect source typed string evidence is invalid.";
				return false;
			}
			Out.strString = pValue->Get_String();
			return true;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR:
			return Read_Array(Value, "value", &Out.vVector.x, 4u, strOutError);
		default:
			strOutError = "Effect source typed evidence kind is invalid.";
			return false;
		}
	}

	bool_t Read_SourceLocalReferenceBinding(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value,
			{ "referenceKind", "referenceId", "definitionId", "occurrenceId",
				"moduleStableId", "propertyPath", "provenance", "exactPayload",
				"currentDefaultEvidence", "executionAdmission" },
			"Effect source local-reference binding", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pExactPayload = Find_Field(
			Value, "exactPayload", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pCurrentDefaultEvidence = Find_Field(
			Value, "currentDefaultEvidence", Client::DATA_JSON_TYPE::ARRAY,
			strOutError);
		const Client::DATA_JSON_VALUE* pExecutionAdmission = Find_Field(
			Value, "executionAdmission", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		if (nullptr == pExactPayload || nullptr == pCurrentDefaultEvidence ||
			nullptr == pExecutionAdmission ||
			!Read_String(Value, "referenceKind", Out.strReferenceKind,
				strOutError) ||
			!Read_String(Value, "referenceId", Out.strReferenceId, strOutError) ||
			!Read_String(Value, "definitionId", Out.strDefinitionId, strOutError) ||
			!Read_String(Value, "occurrenceId", Out.strOccurrenceId, strOutError) ||
			!Read_String(Value, "moduleStableId", Out.strModuleStableId,
				strOutError) ||
			!Read_String(Value, "propertyPath", Out.strPropertyPath,
				strOutError) ||
			!Read_String(Value, "provenance", Out.strProvenance, strOutError) ||
			!Read_SourceAdmission(*pExecutionAdmission,
				Out.ExecutionAdmission, strOutError))
		{
			return false;
		}
		Out.ExactPayload.reserve(pExactPayload->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& FieldValue :
			pExactPayload->Get_Array())
		{
			Client::EFFECT_SOURCE_TYPED_FIELD_DESC Field;
			if (!Read_SourceTypedField(FieldValue, Field, strOutError))
				return false;
			Out.ExactPayload.push_back(std::move(Field));
		}
		Out.CurrentDefaultEvidence.reserve(
			pCurrentDefaultEvidence->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& FieldValue :
			pCurrentDefaultEvidence->Get_Array())
		{
			Client::EFFECT_SOURCE_TYPED_FIELD_DESC Field;
			if (!Read_SourceTypedField(FieldValue, Field, strOutError))
				return false;
			Out.CurrentDefaultEvidence.push_back(std::move(Field));
		}
		return true;
	}

	bool_t Read_SourceMaterialAdmission(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_MATERIAL_ADMISSION_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value,
			{ "status", "sourceMaterialPaths", "materialRecipeId",
				"renderStateRecipeId", "blockers" },
			"Effect source material admission", strOutError) &&
			Read_String(Value, "status", Out.strStatus, strOutError) &&
			Read_StringArray(Value, "sourceMaterialPaths",
				Out.SourceMaterialPaths, strOutError) &&
			Read_String(Value, "materialRecipeId", Out.strMaterialRecipeId,
				strOutError) &&
			Read_String(Value, "renderStateRecipeId", Out.strRenderStateRecipeId,
				strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}

	bool_t Read_SourceGeometryBinding(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Out,
		std::string& strOutError)
	{
		return Validate_ExactFields(Value,
			{ "enabled", "assetId", "receiptFileSha256", "receiptSelfSha256",
				"carrierGeometryPreScale", "particleScaleSemantics", "status",
				"blockers" }, "Effect source geometry binding", strOutError) &&
			Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_String(Value, "assetId", Out.strAssetId, strOutError) &&
			Read_String(Value, "receiptFileSha256", Out.strReceiptFileSha256,
				strOutError) &&
			Read_String(Value, "receiptSelfSha256", Out.strReceiptSelfSha256,
				strOutError) &&
			Read_Float(Value, "carrierGeometryPreScale",
				Out.fCarrierGeometryPreScale, strOutError) &&
			Read_String(Value, "particleScaleSemantics",
				Out.strParticleScaleSemantics, strOutError) &&
			Read_String(Value, "status", Out.strStatus, strOutError) &&
			Read_StringArray(Value, "blockers", Out.Blockers, strOutError);
	}

	bool_t Read_SourceRecipe(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_CASCADE_RECIPE_DESC& Out,
		const bool_t bSourceContract,
		std::string& strOutError)
	{
		if (!bSourceContract)
		{
			constexpr const char_t* SourceOnlyFields[] = {
				"sourceContractProfileId", "sourceContractSha256",
				"sourceGraphSha256", "sourceClosureSha256",
				"sourceMaterialClosureSha256", "sourcePeakActiveParticles",
				"localReferenceBindings", "moduleCoverage", "compilerEvidence",
				"compiledExecutionAdmission", "materialAdmission",
				"geometryBinding"
			};
			for (const char_t* pField : SourceOnlyFields)
			{
				if (nullptr != Value.Find(pField))
				{
					strOutError =
						"Legacy Effect sourceRecipe contains native-v14 evidence.";
					return false;
				}
			}
		}
		if (bSourceContract && !Validate_ExactFields(Value,
			{ "enabled", "rendererShape", "sourceContractProfileId",
				"sourceContractSha256", "sourceGraphSha256",
				"sourceClosureSha256", "sourceMaterialClosureSha256",
				"sourcePeakActiveParticles", "emitterDelaySeconds",
				"emitterDurationSeconds", "emitterLoopCount", "bursts",
				"modules", "localReferenceBindings", "moduleCoverage",
				"compilerEvidence",
				"compiledExecutionAdmission", "materialAdmission",
				"geometryBinding" },
			"Effect source recipe", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pBursts = Find_Field(
			Value, "bursts", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pModules = Find_Field(
			Value, "modules", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		const Client::DATA_JSON_VALUE* pCoverage = bSourceContract ?
			Find_Field(Value, "moduleCoverage", Client::DATA_JSON_TYPE::ARRAY,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pLocalReferenceBindings = bSourceContract ?
			Find_Field(Value, "localReferenceBindings",
				Client::DATA_JSON_TYPE::ARRAY, strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pCompilerEvidence = bSourceContract ?
			Find_Field(Value, "compilerEvidence", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pExecutionAdmission = bSourceContract ?
			Find_Field(Value, "compiledExecutionAdmission",
				Client::DATA_JSON_TYPE::OBJECT, strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pMaterialAdmission = bSourceContract ?
			Find_Field(Value, "materialAdmission", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		const Client::DATA_JSON_VALUE* pGeometryBinding = bSourceContract ?
			Find_Field(Value, "geometryBinding", Client::DATA_JSON_TYPE::OBJECT,
				strOutError) : nullptr;
		if (nullptr == pBursts || nullptr == pModules ||
			(bSourceContract && (nullptr == pLocalReferenceBindings ||
				nullptr == pCoverage ||
				nullptr == pCompilerEvidence || nullptr == pExecutionAdmission ||
				nullptr == pMaterialAdmission || nullptr == pGeometryBinding)) ||
			!Read_Bool(Value, "enabled", Out.bEnabled, strOutError) ||
			!Read_String(Value, "rendererShape", Out.strRendererShape,
				strOutError) ||
			!Read_Float(Value, "emitterDelaySeconds",
				Out.fEmitterDelaySeconds, strOutError) ||
			!Read_Float(Value, "emitterDurationSeconds",
				Out.fEmitterDurationSeconds, strOutError) ||
			!Read_UInt(Value, "emitterLoopCount", Out.iEmitterLoopCount,
				strOutError) ||
			(bSourceContract &&
				(!Read_String(Value, "sourceContractProfileId",
					Out.strSourceContractProfileId, strOutError) ||
				 !Read_String(Value, "sourceContractSha256",
					Out.strSourceContractSha256, strOutError) ||
				 !Read_String(Value, "sourceGraphSha256",
					Out.strSourceGraphSha256, strOutError) ||
				 !Read_String(Value, "sourceClosureSha256",
					Out.strSourceClosureSha256, strOutError) ||
				 !Read_String(Value, "sourceMaterialClosureSha256",
					Out.strSourceMaterialClosureSha256, strOutError) ||
				 !Read_UInt(Value, "sourcePeakActiveParticles",
					Out.iSourcePeakActiveParticles, strOutError) ||
				 !Read_SourceCompilerEvidence(*pCompilerEvidence,
					Out.CompilerEvidence, strOutError) ||
				 !Read_SourceAdmission(*pExecutionAdmission,
					Out.CompiledExecutionAdmission, strOutError) ||
				 !Read_SourceMaterialAdmission(*pMaterialAdmission,
					Out.MaterialAdmission, strOutError) ||
				 !Read_SourceGeometryBinding(*pGeometryBinding,
					Out.GeometryBinding, strOutError))))
		{
			return false;
		}
		Out.Bursts.reserve(pBursts->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& BurstValue : pBursts->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(BurstValue,
				{ "timeSeconds", "countMinimum", "countMaximum" },
				"Effect source burst", strOutError))
				return false;
			Client::EFFECT_PARTICLE_BURST_DESC Burst;
			if (!BurstValue.Is_Object() ||
				!Read_Float(BurstValue, "timeSeconds", Burst.fTimeSeconds,
					strOutError) ||
				!Read_UInt(BurstValue, "countMinimum", Burst.iCountMinimum,
					strOutError) ||
				!Read_UInt(BurstValue, "countMaximum", Burst.iCountMaximum,
					strOutError))
			{
				return false;
			}
			Out.Bursts.push_back(Burst);
		}
		Out.Modules.reserve(pModules->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ModuleValue : pModules->Get_Array())
		{
			if (bSourceContract && !Validate_ExactFields(ModuleValue,
				{ "stableId", "className", "objectPath", "literals",
					"distributions" }, "Effect source module", strOutError))
				return false;
			const Client::DATA_JSON_VALUE* pLiterals = ModuleValue.Is_Object() ?
				ModuleValue.Find("literals") : nullptr;
			const Client::DATA_JSON_VALUE* pDistributions =
				ModuleValue.Is_Object() ? ModuleValue.Find("distributions") :
				nullptr;
			Client::EFFECT_SOURCE_MODULE_DESC Module;
			if (nullptr == pLiterals || !pLiterals->Is_Array() ||
				nullptr == pDistributions || !pDistributions->Is_Array() ||
				!Read_String(ModuleValue, "stableId", Module.strStableId,
					strOutError) ||
				!Read_String(ModuleValue, "className", Module.strClassName,
					strOutError) ||
				!Read_String(ModuleValue, "objectPath", Module.strObjectPath,
					strOutError))
			{
				return false;
			}
			for (const Client::DATA_JSON_VALUE& LiteralValue :
				pLiterals->Get_Array())
			{
				if (bSourceContract && !Validate_ExactFields(LiteralValue,
					{ "propertyPath", "kind", "value" },
					"Effect source literal", strOutError))
					return false;
				const Client::DATA_JSON_VALUE* pKind = LiteralValue.Is_Object() ?
					LiteralValue.Find("kind") : nullptr;
				Client::EFFECT_SOURCE_LITERAL_DESC Literal;
				if (nullptr == pKind || !pKind->Is_String() ||
					!Read_String(LiteralValue, "propertyPath",
						Literal.strPropertyPath, strOutError) ||
					!Parse_Token(pKind->Get_String(),
						SOURCE_LITERAL_KIND_TOKENS,
						std::size(SOURCE_LITERAL_KIND_TOKENS), Literal.eKind))
				{
					return false;
				}
				const Client::DATA_JSON_VALUE* pLiteralValue =
					LiteralValue.Find("value");
				if (nullptr == pLiteralValue ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN ==
						Literal.eKind && !pLiteralValue->Is_Boolean()) ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER ==
						Literal.eKind && !pLiteralValue->Is_Number()) ||
					(Client::EFFECT_SOURCE_LITERAL_KIND::STRING ==
						Literal.eKind && !pLiteralValue->Is_String()))
				{
					strOutError = "Effect source literal value is invalid.";
					return false;
				}
				if (Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN == Literal.eKind)
					Literal.bBoolean = pLiteralValue->Get_Boolean();
				else if (Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind)
					Literal.fNumber = pLiteralValue->Get_Number();
				else
					Literal.strString = pLiteralValue->Get_String();
				Module.Literals.push_back(std::move(Literal));
			}
			for (const Client::DATA_JSON_VALUE& DistributionValue :
				pDistributions->Get_Array())
			{
				Client::EFFECT_DISTRIBUTION_DESC Distribution;
				if (!DistributionValue.Is_Object() ||
					!Read_Distribution(DistributionValue, Distribution,
						bSourceContract,
						strOutError))
				{
					return false;
				}
				Module.Distributions.push_back(std::move(Distribution));
			}
			Out.Modules.push_back(std::move(Module));
		}
		if (bSourceContract)
		{
			Out.LocalReferenceBindings.reserve(
				pLocalReferenceBindings->Get_Array().size());
			for (const Client::DATA_JSON_VALUE& BindingValue :
				pLocalReferenceBindings->Get_Array())
			{
				Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC Binding;
				if (!Read_SourceLocalReferenceBinding(
					BindingValue, Binding, strOutError))
				{
					return false;
				}
				Out.LocalReferenceBindings.push_back(std::move(Binding));
			}
			Out.ModuleCoverage.reserve(pCoverage->Get_Array().size());
			for (const Client::DATA_JSON_VALUE& CoverageValue :
				pCoverage->Get_Array())
			{
				if (!Validate_ExactFields(CoverageValue,
					{ "moduleStableId", "exactSourceClass", "aliasId",
						"normalizedClass", "status", "blockers", "properties" },
					"Effect source module coverage",
					strOutError))
					return false;
				const Client::DATA_JSON_VALUE* pExactSourceClass =
					CoverageValue.Find("exactSourceClass");
				const Client::DATA_JSON_VALUE* pAliasId =
					CoverageValue.Find("aliasId");
				if ((nullptr == pExactSourceClass) != (nullptr == pAliasId))
				{
					strOutError = "Effect source module class lineage is incomplete.";
					return false;
				}
				const Client::DATA_JSON_VALUE* pStatus = Find_Field(
					CoverageValue, "status", Client::DATA_JSON_TYPE::STRING,
					strOutError);
				const Client::DATA_JSON_VALUE* pProperties = Find_Field(
					CoverageValue, "properties", Client::DATA_JSON_TYPE::ARRAY,
					strOutError);
				Client::EFFECT_SOURCE_MODULE_COVERAGE_DESC Coverage;
				if (nullptr == pStatus || nullptr == pProperties ||
					!Read_String(CoverageValue, "moduleStableId",
						Coverage.strModuleStableId, strOutError) ||
					(nullptr != pExactSourceClass &&
						(!Read_String(CoverageValue, "exactSourceClass",
							Coverage.strExactSourceClass, strOutError) ||
						 !Read_String(CoverageValue, "aliasId",
							Coverage.strAliasId, strOutError))) ||
					!Read_String(CoverageValue, "normalizedClass",
						Coverage.strNormalizedClass, strOutError) ||
					!Read_StringArray(CoverageValue, "blockers", Coverage.Blockers,
						strOutError) ||
					!Parse_Token(pStatus->Get_String(),
						SOURCE_COVERAGE_STATUS_TOKENS,
						std::size(SOURCE_COVERAGE_STATUS_TOKENS),
						Coverage.eStatus))
					return false;
				if (nullptr != pExactSourceClass &&
					Coverage.strExactSourceClass.empty())
				{
					strOutError = "Effect source exact module class is empty.";
					return false;
				}
				for (const Client::DATA_JSON_VALUE& PropertyValue :
					pProperties->Get_Array())
				{
					if (!Validate_ExactFields(PropertyValue,
						{ "propertyPath", "storage", "status", "provenance",
							"blockers" },
						"Effect source property coverage", strOutError))
						return false;
					const Client::DATA_JSON_VALUE* pPropertyStatus = Find_Field(
						PropertyValue, "status", Client::DATA_JSON_TYPE::STRING,
						strOutError);
					Client::EFFECT_SOURCE_PROPERTY_COVERAGE_DESC Property;
					if (nullptr == pPropertyStatus ||
						!Read_String(PropertyValue, "propertyPath",
							Property.strPropertyPath, strOutError) ||
						!Read_String(PropertyValue, "storage",
							Property.strStorage, strOutError) ||
						!Read_String(PropertyValue, "provenance",
							Property.strProvenance, strOutError) ||
						!Read_StringArray(PropertyValue, "blockers",
							Property.Blockers, strOutError) ||
						!Parse_Token(pPropertyStatus->Get_String(),
							SOURCE_COVERAGE_STATUS_TOKENS,
							std::size(SOURCE_COVERAGE_STATUS_TOKENS),
							Property.eStatus))
						return false;
					Coverage.Properties.push_back(std::move(Property));
				}
				Out.ModuleCoverage.push_back(std::move(Coverage));
			}
		}
		return true;
	}

	void Write_SourceCompilerEvidence(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence)
	{
		Output << "{ \"artifactFileSha256\": \"" << Evidence.strArtifactFileSha256
			<< "\", \"artifactSelfSha256\": \"" << Evidence.strArtifactSelfSha256
			<< "\", \"evidenceId\": \""
			<< Client::CDataJson::Escape(Evidence.strEvidenceId)
			<< "\", \"sourceEvidenceStatus\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEvidenceStatus)
			<< "\", \"sourceCueId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceCueId)
			<< "\", \"sourceOccurrenceId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceOccurrenceId)
			<< "\", \"sourceSystemId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceSystemId)
			<< "\", \"sourceEmitterPath\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEmitterPath)
			<< "\", \"sourceEmitterNodeId\": \""
			<< Client::CDataJson::Escape(Evidence.strSourceEmitterNodeId)
			<< "\", \"lodSelectionPolicy\": \""
			<< Client::CDataJson::Escape(Evidence.strLodSelectionPolicy)
			<< "\", \"selectedLodPath\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodPath)
			<< "\", \"selectedLodNodeId\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodNodeId)
			<< "\", \"selectedLodArrayIndex\": "
			<< Evidence.iSelectedLodArrayIndex
			<< ", \"selectedLodLevelProvenance\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodLevelProvenance)
			<< "\", \"selectedLodEnabledProvenance\": \""
			<< Client::CDataJson::Escape(Evidence.strSelectedLodEnabledProvenance)
			<< "\", \"nonSelectedLodCount\": "
			<< Evidence.iNonSelectedLodCount << ", \"moduleReferenceOrder\": [";
		for (size_t iModule = 0u;
			iModule < Evidence.ModuleReferenceOrder.size(); ++iModule)
		{
			const Client::EFFECT_SOURCE_MODULE_REFERENCE_DESC& Module =
				Evidence.ModuleReferenceOrder[iModule];
			if (iModule > 0u)
				Output << ", ";
			Output << "{ \"order\": " << Module.iOrder
				<< ", \"sourceReferenceIndex\": "
				<< Module.iSourceReferenceIndex << ", \"role\": \""
				<< Client::CDataJson::Escape(Module.strRole)
				<< "\", \"sourceObjectId\": \""
				<< Client::CDataJson::Escape(Module.strSourceObjectId)
				<< "\", \"sourceRecordSha256\": \""
				<< Module.strSourceRecordSha256 << "\" }";
		}
		Output << "], \"cueLocalTransform\": { \"sourcePositionUeUnits\": ";
		Write_Float3(Output, Evidence.vCueSourcePositionUeUnits);
		Output << ", \"position\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vRotationDegrees);
		Output << ", \"scale\": ";
		Write_Float3(Output, Evidence.CueLocalTransform.vScale);
		Output << " }, \"parameterOverrides\": [";
		for (size_t iParameter = 0u;
			iParameter < Evidence.ParameterOverrides.size(); ++iParameter)
		{
			const Client::EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC& Parameter =
				Evidence.ParameterOverrides[iParameter];
			if (iParameter > 0u)
				Output << ", ";
			Output << "{ \"sourceIndex\": " << Parameter.iSourceIndex
				<< ", \"name\": \""
				<< Client::CDataJson::Escape(Parameter.strName)
				<< "\", \"sourceTypeCode\": " << Parameter.iSourceTypeCode
				<< ", \"sourceRecordByteOffset\": "
				<< Parameter.iSourceRecordByteOffset << ", \"type\": \""
				<< Parameter.strType << "\", ";
			if (Parameter.strType == "scalar")
			{
				Output << "\"scalarValue\": "
					<< std::setprecision(std::numeric_limits<f64_t>::max_digits10)
					<< Parameter.fScalarValue << std::setprecision(9);
			}
			else
			{
				Output << "\"vectorValue\": ";
				Write_Float3(Output, Parameter.vVectorValue);
			}
			Output << ", \"sourceValueByteOffset\": "
				<< Parameter.iSourceValueByteOffset << " }";
		}
		Output << "], \"compositionOrder\": ";
		Write_StringArray(Output, Evidence.CompositionOrder);
		Output << ", \"localReferenceClosureFileSha256\": \""
			<< Evidence.strLocalReferenceClosureFileSha256
			<< "\", \"localReferenceClosureSelfSha256\": \""
			<< Evidence.strLocalReferenceClosureSelfSha256
			<< "\", \"geometryParityFileSha256\": \""
			<< Evidence.strGeometryParityFileSha256
			<< "\", \"geometryParitySelfSha256\": \""
			<< Evidence.strGeometryParitySelfSha256 << "\" }";
	}

	void Write_SourceAdmission(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_ADMISSION_DESC& Admission)
	{
		Output << "{ \"allowed\": " << (Admission.bAllowed ? "true" : "false")
			<< ", \"blockers\": ";
		Write_StringArray(Output, Admission.Blockers);
		Output << " }";
	}

	void Write_SourceTypedField(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_TYPED_FIELD_DESC& Field)
	{
		Output << "{ \"propertyPath\": \""
			<< Client::CDataJson::Escape(Field.strPropertyPath)
			<< "\", \"kind\": \""
			<< SOURCE_TYPED_FIELD_KIND_TOKENS[static_cast<size_t>(Field.eKind)]
			<< "\", \"value\": ";
		switch (Field.eKind)
		{
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN:
			Output << (Field.bBoolean ? "true" : "false");
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER:
			Output << std::setprecision(
				std::numeric_limits<f64_t>::max_digits10) << Field.fNumber
				<< std::setprecision(9);
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::STRING:
			Output << '"' << Client::CDataJson::Escape(Field.strString) << '"';
			break;
		case Client::EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR:
			Write_Float4(Output, Field.vVector);
			break;
		default:
			Output << "null";
			break;
		}
		Output << " }";
	}

	void Write_SourceLocalReferenceBinding(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding)
	{
		Output << "{ \"referenceKind\": \""
			<< Client::CDataJson::Escape(Binding.strReferenceKind)
			<< "\", \"referenceId\": \""
			<< Client::CDataJson::Escape(Binding.strReferenceId)
			<< "\", \"definitionId\": \""
			<< Client::CDataJson::Escape(Binding.strDefinitionId)
			<< "\", \"occurrenceId\": \""
			<< Client::CDataJson::Escape(Binding.strOccurrenceId)
			<< "\", \"moduleStableId\": \""
			<< Client::CDataJson::Escape(Binding.strModuleStableId)
			<< "\", \"propertyPath\": \""
			<< Client::CDataJson::Escape(Binding.strPropertyPath)
			<< "\", \"provenance\": \""
			<< Client::CDataJson::Escape(Binding.strProvenance)
			<< "\", \"exactPayload\": [";
		for (size_t iField = 0u; iField < Binding.ExactPayload.size(); ++iField)
		{
			if (iField > 0u)
				Output << ", ";
			Write_SourceTypedField(Output, Binding.ExactPayload[iField]);
		}
		Output << "], \"currentDefaultEvidence\": [";
		for (size_t iField = 0u;
			iField < Binding.CurrentDefaultEvidence.size(); ++iField)
		{
			if (iField > 0u)
				Output << ", ";
			Write_SourceTypedField(
				Output, Binding.CurrentDefaultEvidence[iField]);
		}
		Output << "], \"executionAdmission\": ";
		Write_SourceAdmission(Output, Binding.ExecutionAdmission);
		Output << " }";
	}

	void Write_SourceMaterialAdmission(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_MATERIAL_ADMISSION_DESC& Admission)
	{
		Output << "{ \"status\": \""
			<< Client::CDataJson::Escape(Admission.strStatus)
			<< "\", \"sourceMaterialPaths\": ";
		Write_StringArray(Output, Admission.SourceMaterialPaths);
		Output << ", \"materialRecipeId\": \""
			<< Client::CDataJson::Escape(Admission.strMaterialRecipeId)
			<< "\", \"renderStateRecipeId\": \""
			<< Client::CDataJson::Escape(Admission.strRenderStateRecipeId)
			<< "\", \"blockers\": ";
		Write_StringArray(Output, Admission.Blockers);
		Output << " }";
	}

	void Write_SourceGeometryBinding(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Binding)
	{
		Output << "{ \"enabled\": " << (Binding.bEnabled ? "true" : "false")
			<< ", \"assetId\": \""
			<< Client::CDataJson::Escape(Binding.strAssetId)
			<< "\", \"receiptFileSha256\": \""
			<< Binding.strReceiptFileSha256
			<< "\", \"receiptSelfSha256\": \""
			<< Binding.strReceiptSelfSha256
			<< "\", \"carrierGeometryPreScale\": "
			<< Binding.fCarrierGeometryPreScale
			<< ", \"particleScaleSemantics\": \""
			<< Client::CDataJson::Escape(Binding.strParticleScaleSemantics)
			<< "\", \"status\": \""
			<< Client::CDataJson::Escape(Binding.strStatus)
			<< "\", \"blockers\": ";
		Write_StringArray(Output, Binding.Blockers);
		Output << " }";
	}

	void Write_SourceRecipe(
		std::ostringstream& Output,
		const Client::EFFECT_CASCADE_RECIPE_DESC& Recipe,
		const bool_t bSourceContract)
	{
		Output << "      \"sourceRecipe\": { \"enabled\": "
			<< (Recipe.bEnabled ? "true" : "false")
			<< ", \"rendererShape\": \""
			<< Client::CDataJson::Escape(Recipe.strRendererShape) << '"';
		if (bSourceContract)
		{
			Output << ", \"sourceContractProfileId\": \""
				<< Client::CDataJson::Escape(Recipe.strSourceContractProfileId)
				<< "\", \"sourceContractSha256\": \""
				<< Recipe.strSourceContractSha256
				<< "\", \"sourceGraphSha256\": \""
				<< Recipe.strSourceGraphSha256
				<< "\", \"sourceClosureSha256\": \""
				<< Recipe.strSourceClosureSha256
				<< "\", \"sourceMaterialClosureSha256\": \""
				<< Recipe.strSourceMaterialClosureSha256
				<< "\", \"sourcePeakActiveParticles\": "
				<< Recipe.iSourcePeakActiveParticles;
		}
		Output << ", \"emitterDelaySeconds\": "
			<< Recipe.fEmitterDelaySeconds
			<< ", \"emitterDurationSeconds\": "
			<< Recipe.fEmitterDurationSeconds
			<< ", \"emitterLoopCount\": " << Recipe.iEmitterLoopCount
			<< ",\n        \"bursts\": [";
		for (size_t iBurst = 0u; iBurst < Recipe.Bursts.size(); ++iBurst)
		{
			const Client::EFFECT_PARTICLE_BURST_DESC& Burst =
				Recipe.Bursts[iBurst];
			Output << (0u == iBurst ? "\n" : ",\n")
				<< "          { \"timeSeconds\": " << Burst.fTimeSeconds
				<< ", \"countMinimum\": " << Burst.iCountMinimum
				<< ", \"countMaximum\": " << Burst.iCountMaximum
				<< " }";
		}
		if (!Recipe.Bursts.empty())
			Output << '\n';
		Output << "        ],\n        \"modules\": [";
		for (size_t iModule = 0u; iModule < Recipe.Modules.size(); ++iModule)
		{
			const Client::EFFECT_SOURCE_MODULE_DESC& Module =
				Recipe.Modules[iModule];
			Output << (0u == iModule ? "\n" : ",\n")
				<< "          { \"stableId\": \""
				<< Client::CDataJson::Escape(Module.strStableId)
				<< "\", \"className\": \""
				<< Client::CDataJson::Escape(Module.strClassName)
				<< "\", \"objectPath\": \""
				<< Client::CDataJson::Escape(Module.strObjectPath)
				<< "\",\n            \"literals\": [";
			for (size_t iLiteral = 0u; iLiteral < Module.Literals.size();
				++iLiteral)
			{
				const Client::EFFECT_SOURCE_LITERAL_DESC& Literal =
					Module.Literals[iLiteral];
				Output << (0u == iLiteral ? "\n" : ",\n")
					<< "              { \"propertyPath\": \""
					<< Client::CDataJson::Escape(Literal.strPropertyPath)
					<< "\", \"kind\": \""
					<< SOURCE_LITERAL_KIND_TOKENS[
						static_cast<size_t>(Literal.eKind)]
					<< "\", \"value\": ";
				if (Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN == Literal.eKind)
					Output << (Literal.bBoolean ? "true" : "false");
				else if (Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind)
				{
					Output << std::setprecision(
						std::numeric_limits<f64_t>::max_digits10)
						<< Literal.fNumber << std::setprecision(9);
				}
				else
					Output << '"' << Client::CDataJson::Escape(
						Literal.strString) << '"';
				Output << " }";
			}
			if (!Module.Literals.empty())
				Output << '\n';
			Output << "            ],\n            \"distributions\": [";
			for (size_t iDistribution = 0u;
				iDistribution < Module.Distributions.size(); ++iDistribution)
			{
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution =
					Module.Distributions[iDistribution];
				Output << (0u == iDistribution ? "\n" : ",\n")
					<< "              { \"propertyPath\": \""
					<< Client::CDataJson::Escape(Distribution.strPropertyPath) << '"';
				if (bSourceContract)
				{
					Output << ", \"referenceId\": \""
						<< Client::CDataJson::Escape(Distribution.strReferenceId)
						<< "\", \"occurrenceId\": \""
						<< Client::CDataJson::Escape(Distribution.strOccurrenceId)
						<< "\", \"payloadStatus\": \""
						<< Client::CDataJson::Escape(Distribution.strPayloadStatus)
						<< "\", \"fidelity\": \""
						<< Client::CDataJson::Escape(Distribution.strFidelity)
						<< "\", \"executionAdmission\": ";
					Write_SourceAdmission(
						Output, Distribution.ExecutionAdmission);
				}
				Output << ", \"sourceClass\": \""
					<< Client::CDataJson::Escape(Distribution.strSourceClass)
					<< "\", \"sourceObjectPath\": \""
					<< Client::CDataJson::Escape(
						Distribution.strSourceObjectPath) << '"';
				if (bSourceContract &&
					Is_ParticleParameterDistribution(Distribution.strSourceClass))
				{
					Output << ", \"parameterBinding\": \""
						<< DISTRIBUTION_PARAMETER_BINDING_TOKENS[
							static_cast<size_t>(Distribution.eParameterBinding)]
						<< "\", \"parameterName\": \""
						<< Client::CDataJson::Escape(Distribution.strParameterName)
						<< '"';
				}
				Output << ", \"componentCount\": "
					<< Distribution.iComponentCount
					<< ", \"operation\": " << Distribution.iOperation
					<< ", \"randomLockAxes\": "
					<< Distribution.iRandomLockAxes
					<< ", \"lookupTableChunkSize\": "
					<< Distribution.iLookupTableChunkSize
					<< ", \"lookupTableNumElements\": "
					<< Distribution.iLookupTableNumElements
					<< ", \"lookupTableTimeScale\": "
					<< Distribution.fLookupTableTimeScale
					<< ", \"lookupTableStartTime\": "
					<< Distribution.fLookupTableStartTime
					<< ", \"defaultMinimum\": ";
				Write_Float4(Output, Distribution.vDefaultMinimum);
				Output << ", \"defaultMaximum\": ";
				Write_Float4(Output, Distribution.vDefaultMaximum);
				Output << ", \"lookupTable\": [";
				for (size_t iValue = 0u;
					iValue < Distribution.LookupTable.size(); ++iValue)
				{
					if (iValue > 0u)
						Output << ", ";
					Output << Distribution.LookupTable[iValue];
				}
				Output << "], \"keys\": [";
				for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
				{
					const Client::EFFECT_DISTRIBUTION_KEY_DESC& Key =
						Distribution.Keys[iKey];
					Output << (0u == iKey ? "\n" : ",\n")
						<< "                { \"time\": " << Key.fTime
						<< ", \"minimum\": ";
					Write_Float4(Output, Key.vMinimum);
					Output << ", \"maximum\": ";
					Write_Float4(Output, Key.vMaximum);
					Output << ", \"arriveTangentMinimum\": ";
					Write_Float4(Output, Key.vArriveTangentMinimum);
					Output << ", \"leaveTangentMinimum\": ";
					Write_Float4(Output, Key.vLeaveTangentMinimum);
					Output << ", \"arriveTangentMaximum\": ";
					Write_Float4(Output, Key.vArriveTangentMaximum);
					Output << ", \"leaveTangentMaximum\": ";
					Write_Float4(Output, Key.vLeaveTangentMaximum);
					Output << ", \"interpolation\": \""
						<< DISTRIBUTION_INTERPOLATION_TOKENS[
							static_cast<size_t>(Key.eInterpolation)]
						<< "\" }";
				}
				if (!Distribution.Keys.empty())
					Output << '\n';
				Output << "              ] }";
			}
			if (!Module.Distributions.empty())
				Output << '\n';
			Output << "            ] }";
		}
		if (!Recipe.Modules.empty())
			Output << '\n';
		Output << "        ]";
		if (bSourceContract)
		{
			Output << ",\n        \"localReferenceBindings\": [";
			for (size_t iBinding = 0u;
				iBinding < Recipe.LocalReferenceBindings.size(); ++iBinding)
			{
				Output << (0u == iBinding ? "\n          " : ",\n          ");
				Write_SourceLocalReferenceBinding(
					Output, Recipe.LocalReferenceBindings[iBinding]);
			}
			if (!Recipe.LocalReferenceBindings.empty())
				Output << '\n';
			Output << "        ],\n        \"moduleCoverage\": [";
			for (size_t iCoverage = 0u;
				iCoverage < Recipe.ModuleCoverage.size(); ++iCoverage)
			{
				const Client::EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage =
					Recipe.ModuleCoverage[iCoverage];
				Output << (0u == iCoverage ? "\n" : ",\n")
					<< "          { \"moduleStableId\": \""
					<< Client::CDataJson::Escape(Coverage.strModuleStableId);
				if (!Coverage.strExactSourceClass.empty())
				{
					Output << "\", \"exactSourceClass\": \""
						<< Client::CDataJson::Escape(Coverage.strExactSourceClass)
						<< "\", \"aliasId\": \""
						<< Client::CDataJson::Escape(Coverage.strAliasId);
				}
				Output << "\", \"normalizedClass\": \""
					<< Client::CDataJson::Escape(Coverage.strNormalizedClass)
					<< "\", \"status\": \""
					<< SOURCE_COVERAGE_STATUS_TOKENS[
						static_cast<size_t>(Coverage.eStatus)]
					<< "\", \"blockers\": ";
				Write_StringArray(Output, Coverage.Blockers);
				Output << ", \"properties\": [";
				for (size_t iProperty = 0u;
					iProperty < Coverage.Properties.size(); ++iProperty)
				{
					const Client::EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property =
						Coverage.Properties[iProperty];
					Output << (0u == iProperty ? "\n" : ",\n")
						<< "            { \"propertyPath\": \""
						<< Client::CDataJson::Escape(Property.strPropertyPath)
						<< "\", \"storage\": \""
						<< Client::CDataJson::Escape(Property.strStorage)
						<< "\", \"status\": \""
						<< SOURCE_COVERAGE_STATUS_TOKENS[
							static_cast<size_t>(Property.eStatus)]
						<< "\", \"provenance\": \""
						<< Client::CDataJson::Escape(Property.strProvenance)
						<< "\", \"blockers\": ";
					Write_StringArray(Output, Property.Blockers);
					Output << " }";
				}
				if (!Coverage.Properties.empty())
					Output << '\n';
				Output << "          ] }";
			}
			if (!Recipe.ModuleCoverage.empty())
				Output << '\n';
			Output << "        ],\n        \"compilerEvidence\": ";
			Write_SourceCompilerEvidence(Output, Recipe.CompilerEvidence);
			Output << ",\n        \"compiledExecutionAdmission\": ";
			Write_SourceAdmission(Output, Recipe.CompiledExecutionAdmission);
			Output << ",\n        \"materialAdmission\": ";
			Write_SourceMaterialAdmission(Output, Recipe.MaterialAdmission);
			Output << ",\n        \"geometryBinding\": ";
			Write_SourceGeometryBinding(Output, Recipe.GeometryBinding);
		}
		Output << " }";
	}

	bool_t Read_PresentationDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLight = Find_Field(
			Value, "light", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pScreenPost = Find_Field(
			Value, "screenPost", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pLight || nullptr == pScreenPost ||
			!Read_Bool(*pLight, "enabled", Out.Light.bEnabled, strOutError) ||
			!Read_Bool(*pScreenPost, "enabled", Out.ScreenPost.bEnabled,
				strOutError))
		{
			return false;
		}

		if (Out.Light.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pProfile = Find_Field(
				*pLight, "profileId", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pStatus = Find_Field(
				*pLight, "status", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			if (nullptr == pProfile || nullptr == pStatus ||
				!Parse_Token(pProfile->Get_String(), LIGHT_PROFILE_TOKENS,
					std::size(LIGHT_PROFILE_TOKENS), Out.Light.eProfile) ||
				!Parse_Token(pStatus->Get_String(),
					PRESENTATION_RUNTIME_STATUS_TOKENS,
					std::size(PRESENTATION_RUNTIME_STATUS_TOKENS),
					Out.Light.eStatus) ||
				!Read_Float(*pLight, "range", Out.Light.fRange, strOutError) ||
				!Read_Float(*pLight, "intensity", Out.Light.fIntensity,
					strOutError) ||
				!Read_Array(*pLight, "color", &Out.Light.vColor.x, 4u,
					strOutError) ||
				!Read_Array(*pLight, "ambient", &Out.Light.vAmbient.x, 4u,
					strOutError) ||
				!Read_Float(*pLight, "falloffExponent",
					Out.Light.fFalloffExponent, strOutError))
			{
				return false;
			}
		}

		if (Out.ScreenPost.bEnabled)
		{
			const Client::DATA_JSON_VALUE* pProfile = Find_Field(
				*pScreenPost, "profileId", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			const Client::DATA_JSON_VALUE* pStatus = Find_Field(
				*pScreenPost, "status", Client::DATA_JSON_TYPE::STRING,
				strOutError);
			if (nullptr == pProfile || nullptr == pStatus ||
				!Parse_Token(pProfile->Get_String(), SCREEN_POST_PROFILE_TOKENS,
					std::size(SCREEN_POST_PROFILE_TOKENS),
					Out.ScreenPost.eProfile) ||
				!Parse_Token(pStatus->Get_String(),
					PRESENTATION_RUNTIME_STATUS_TOKENS,
					std::size(PRESENTATION_RUNTIME_STATUS_TOKENS),
					Out.ScreenPost.eStatus) ||
				!Read_Float(*pScreenPost, "intensity",
					Out.ScreenPost.fIntensity, strOutError) ||
				!Read_Float(*pScreenPost, "secondaryIntensity",
					Out.ScreenPost.fSecondaryIntensity, strOutError) ||
				!Read_Float(*pScreenPost, "frequency",
					Out.ScreenPost.fFrequency, strOutError) ||
				!Read_Array(*pScreenPost, "tint",
					&Out.ScreenPost.vTint.x, 4u, strOutError) ||
				!Read_UInt(*pScreenPost, "randomSeed",
					Out.ScreenPost.iRandomSeed, strOutError))
			{
				return false;
			}
		}
		return true;
	}

	const char_t* RuntimeStatusToken(
		const Client::EFFECT_PRESENTATION_RUNTIME_STATUS eStatus)
	{
		const size_t iIndex = static_cast<size_t>(eStatus);
		return iIndex < std::size(PRESENTATION_RUNTIME_STATUS_TOKENS) ?
			PRESENTATION_RUNTIME_STATUS_TOKENS[iIndex] :
			"reconstructed_profile";
	}

	void Write_PresentationDetail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail)
	{
		Output << "        \"light\": { \"enabled\": "
			<< (Detail.Light.bEnabled ? "true" : "false");
		if (Detail.Light.bEnabled)
		{
			Output << ", \"profileId\": \""
				<< LIGHT_PROFILE_TOKENS[static_cast<size_t>(
					Detail.Light.eProfile)]
				<< "\", \"status\": \""
				<< RuntimeStatusToken(Detail.Light.eStatus)
				<< "\", \"range\": " << Detail.Light.fRange
				<< ", \"intensity\": " << Detail.Light.fIntensity
				<< ", \"color\": ";
			Write_Float4(Output, Detail.Light.vColor);
			Output << ", \"ambient\": ";
			Write_Float4(Output, Detail.Light.vAmbient);
			Output << ", \"falloffExponent\": "
				<< Detail.Light.fFalloffExponent;
		}
		Output << " },\n        \"screenPost\": { \"enabled\": "
			<< (Detail.ScreenPost.bEnabled ? "true" : "false");
		if (Detail.ScreenPost.bEnabled)
		{
			Output << ", \"profileId\": \""
				<< SCREEN_POST_PROFILE_TOKENS[static_cast<size_t>(
					Detail.ScreenPost.eProfile)]
				<< "\", \"status\": \""
				<< RuntimeStatusToken(Detail.ScreenPost.eStatus)
				<< "\", \"intensity\": "
				<< Detail.ScreenPost.fIntensity
				<< ", \"secondaryIntensity\": "
				<< Detail.ScreenPost.fSecondaryIntensity
				<< ", \"frequency\": " << Detail.ScreenPost.fFrequency
				<< ", \"tint\": ";
			Write_Float4(Output, Detail.ScreenPost.vTint);
			Output << ", \"randomSeed\": "
				<< Detail.ScreenPost.iRandomSeed;
		}
		Output << " }\n";
	}

	enum class AUTHORING_OVERRIDE_TARGET_STATUS : uint8_t
	{
		FOUND,
		MISSING,
		INVALID_ID,
		AMBIGUOUS
	};

	enum class AUTHORING_RESOURCE_TARGET_STORAGE : uint8_t
	{
		RESOURCE_BINDING,
		MATERIAL_EXECUTION_LANE,
		SOURCE_MATERIAL_TEXTURE
	};

	struct AUTHORING_RESOURCE_TARGET final
	{
		AUTHORING_RESOURCE_TARGET_STORAGE eStorage =
			AUTHORING_RESOURCE_TARGET_STORAGE::RESOURCE_BINDING;
		size_t iIndex = 0u;
		Client::EFFECT_RESOURCE_FILE_KIND eExpectedKind =
			Client::EFFECT_RESOURCE_FILE_KIND::END;
	};

	bool_t Starts_With(const std::string_view strValue,
		const std::string_view strPrefix)
	{
		return strValue.size() >= strPrefix.size() &&
			0 == strValue.compare(0u, strPrefix.size(), strPrefix);
	}

	AUTHORING_OVERRIDE_TARGET_STATUS Resolve_AuthoringResourceTarget(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId,
		AUTHORING_RESOURCE_TARGET& Out)
	{
		Out = {};
		if (strSlotId.empty())
			return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;

		std::string_view strTargetId;
		if (Starts_With(strSlotId,
				Client::EFFECT_MATERIAL_EXECUTION_LANE_STABLE_SLOT_PREFIX))
		{
			if (!Client::Try_ParseEffectMaterialExecutionLaneStableSlotId(
					strSlotId, strTargetId))
			{
				return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;
			}
			size_t iMatchCount = 0u;
			for (size_t i = 0u;
				i < Element.Material.Execution.TextureLanes.size(); ++i)
			{
				if (Element.Material.Execution.TextureLanes[i].strLaneId !=
					strTargetId)
				{
					continue;
				}
				Out.eStorage =
					AUTHORING_RESOURCE_TARGET_STORAGE::MATERIAL_EXECUTION_LANE;
				Out.iIndex = i;
				Out.eExpectedKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
				++iMatchCount;
			}
			return 0u == iMatchCount ?
				AUTHORING_OVERRIDE_TARGET_STATUS::MISSING :
				(1u == iMatchCount ? AUTHORING_OVERRIDE_TARGET_STATUS::FOUND :
					AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS);
		}

		if (Starts_With(strSlotId,
				Client::EFFECT_SOURCE_MATERIAL_TEXTURE_STABLE_SLOT_PREFIX))
		{
			if (!Client::Try_ParseEffectSourceMaterialTextureStableSlotId(
					strSlotId, strTargetId))
			{
				return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;
			}
			size_t iMatchCount = 0u;
			for (size_t i = 0u;
				i < Element.Material.SourceMaterial.Textures.size(); ++i)
			{
				if (Element.Material.SourceMaterial.Textures[i].strName !=
					strTargetId)
				{
					continue;
				}
				Out.eStorage =
					AUTHORING_RESOURCE_TARGET_STORAGE::SOURCE_MATERIAL_TEXTURE;
				Out.iIndex = i;
				Out.eExpectedKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
				++iMatchCount;
			}
			return 0u == iMatchCount ?
				AUTHORING_OVERRIDE_TARGET_STATUS::MISSING :
				(1u == iMatchCount ? AUTHORING_OVERRIDE_TARGET_STATUS::FOUND :
					AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS);
		}

		size_t iMatchCount = 0u;
		for (size_t i = 0u; i < Element.ResourceBindings.size(); ++i)
		{
			if (Element.ResourceBindings[i].strSlotId != strSlotId)
				continue;
			Out.eStorage = AUTHORING_RESOURCE_TARGET_STORAGE::RESOURCE_BINDING;
			Out.iIndex = i;
			++iMatchCount;
		}
		if (0u == iMatchCount)
			return AUTHORING_OVERRIDE_TARGET_STATUS::MISSING;
		if (1u != iMatchCount)
			return AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS;

		if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
		{
			Out.eExpectedKind = Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		}
		else
		{
			const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				Client::Find_EffectMaterialInput(
					Element.Material.strTemplateId, strSlotId);
			if (nullptr == pInput)
				return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;
			Out.eExpectedKind = pInput->eAllowedResourceKind;
		}
		return Out.eExpectedKind == Client::EFFECT_RESOURCE_FILE_KIND::END ?
			AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID :
			AUTHORING_OVERRIDE_TARGET_STATUS::FOUND;
	}

	const std::string& Get_AuthoringResourceTargetAssetId(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_RESOURCE_TARGET& Target)
	{
		switch (Target.eStorage)
		{
		case AUTHORING_RESOURCE_TARGET_STORAGE::MATERIAL_EXECUTION_LANE:
			return Element.Material.Execution.TextureLanes[Target.iIndex].strAssetId;
		case AUTHORING_RESOURCE_TARGET_STORAGE::SOURCE_MATERIAL_TEXTURE:
			return Element.Material.SourceMaterial.Textures[Target.iIndex].strAssetId;
		case AUTHORING_RESOURCE_TARGET_STORAGE::RESOURCE_BINDING:
		default:
			return Element.ResourceBindings[Target.iIndex].strAssetId;
		}
	}

	std::string& Get_AuthoringResourceTargetAssetId(
		Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_RESOURCE_TARGET& Target)
	{
		return const_cast<std::string&>(Get_AuthoringResourceTargetAssetId(
			static_cast<const Client::EFFECT_ELEMENT_DESC&>(Element), Target));
	}

	struct AUTHORING_SCALAR_TARGET final
	{
		bool_t bHasSourceMaterial = false;
		size_t iSourceMaterialIndex = 0u;
		std::vector<size_t> ExecutionIndices;
	};

	AUTHORING_OVERRIDE_TARGET_STATUS Resolve_AuthoringScalarTarget(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strName,
		AUTHORING_SCALAR_TARGET& Out)
	{
		Out = {};
		if (strName.empty())
			return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;
		size_t iSourceMatchCount = 0u;
		for (size_t i = 0u;
			i < Element.Material.SourceMaterial.Scalars.size(); ++i)
		{
			if (Element.Material.SourceMaterial.Scalars[i].strName != strName)
				continue;
			Out.bHasSourceMaterial = true;
			Out.iSourceMaterialIndex = i;
			++iSourceMatchCount;
		}
		if (iSourceMatchCount > 1u)
			return AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS;
		for (size_t i = 0u; i < Element.Material.Execution.Scalars.size(); ++i)
		{
			if (Element.Material.Execution.Scalars[i].strName != strName)
				continue;
			Out.ExecutionIndices.push_back(i);
		}
		return Out.bHasSourceMaterial || !Out.ExecutionIndices.empty() ?
			AUTHORING_OVERRIDE_TARGET_STATUS::FOUND :
			AUTHORING_OVERRIDE_TARGET_STATUS::MISSING;
	}

	f32_t Get_AuthoringScalarTargetValue(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_SCALAR_TARGET& Target)
	{
		return Target.bHasSourceMaterial ?
			Element.Material.SourceMaterial.Scalars[
				Target.iSourceMaterialIndex].fValue :
			Element.Material.Execution.Scalars[
				Target.ExecutionIndices.front()].fValue;
	}

	bool_t Is_AuthoringScalarTargetConsistent(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_SCALAR_TARGET& Target)
	{
		const f32_t fValue = Get_AuthoringScalarTargetValue(Element, Target);
		return std::all_of(Target.ExecutionIndices.begin(),
			Target.ExecutionIndices.end(),
			[&Element, fValue](const size_t iIndex)
			{
				return Element.Material.Execution.Scalars[iIndex].fValue ==
					fValue;
			});
	}

	void Set_AuthoringScalarTargetValue(
		Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_SCALAR_TARGET& Target,
		const f32_t fValue)
	{
		if (Target.bHasSourceMaterial)
		{
			Element.Material.SourceMaterial.Scalars[
				Target.iSourceMaterialIndex].fValue = fValue;
		}
		for (const size_t iIndex : Target.ExecutionIndices)
			Element.Material.Execution.Scalars[iIndex].fValue = fValue;
	}

	enum class AUTHORING_COLOR_EXECUTION_TARGET_STORAGE : uint8_t
	{
		MATERIAL_EXECUTION_VECTOR,
		MATERIAL_EXECUTION_ARTIST_PARAMETER,
		MATERIAL_EXECUTION_COLOR
	};

	struct AUTHORING_COLOR_EXECUTION_TARGET final
	{
		AUTHORING_COLOR_EXECUTION_TARGET_STORAGE eStorage =
			AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::MATERIAL_EXECUTION_VECTOR;
		size_t iIndex = 0u;
	};

	struct AUTHORING_COLOR_TARGET final
	{
		bool_t bHasSourceMaterial = false;
		size_t iSourceMaterialIndex = 0u;
		std::vector<AUTHORING_COLOR_EXECUTION_TARGET> ExecutionTargets;
	};

	const float4_t& Get_AuthoringColorExecutionTargetValue(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_COLOR_EXECUTION_TARGET& Target);

	AUTHORING_OVERRIDE_TARGET_STATUS Resolve_AuthoringColorTarget(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strName,
		AUTHORING_COLOR_TARGET& Out)
	{
		Out = {};
		if (strName.empty())
			return AUTHORING_OVERRIDE_TARGET_STATUS::INVALID_ID;
		size_t iSourceMatchCount = 0u;
		for (size_t i = 0u;
			i < Element.Material.SourceMaterial.Vectors.size(); ++i)
		{
			if (Element.Material.SourceMaterial.Vectors[i].strName != strName)
				continue;
			Out.bHasSourceMaterial = true;
			Out.iSourceMaterialIndex = i;
			++iSourceMatchCount;
		}
		if (iSourceMatchCount > 1u)
			return AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS;
		const auto Find = [&](const auto& Rows,
			const AUTHORING_COLOR_EXECUTION_TARGET_STORAGE eStorage)
		{
			for (size_t i = 0u; i < Rows.size(); ++i)
			{
				if (Rows[i].strName != strName)
					continue;
				Out.ExecutionTargets.push_back({ eStorage, i });
			}
		};
		Find(Element.Material.Execution.Vectors,
			AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::MATERIAL_EXECUTION_VECTOR);
		Find(Element.Material.Execution.ArtistParameters,
			AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
				MATERIAL_EXECUTION_ARTIST_PARAMETER);
		Find(Element.Material.Execution.Colors,
			AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::MATERIAL_EXECUTION_COLOR);
		return Out.bHasSourceMaterial || !Out.ExecutionTargets.empty() ?
			AUTHORING_OVERRIDE_TARGET_STATUS::FOUND :
			AUTHORING_OVERRIDE_TARGET_STATUS::MISSING;
	}

	const float4_t& Get_AuthoringColorTargetValue(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_COLOR_TARGET& Target)
	{
		if (Target.bHasSourceMaterial)
		{
			return Element.Material.SourceMaterial.Vectors[
				Target.iSourceMaterialIndex].vValue;
		}
		return Get_AuthoringColorExecutionTargetValue(
			Element, Target.ExecutionTargets.front());
	}

	const float4_t& Get_AuthoringColorExecutionTargetValue(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_COLOR_EXECUTION_TARGET& Target)
	{
		switch (Target.eStorage)
		{
		case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
			MATERIAL_EXECUTION_VECTOR:
			return Element.Material.Execution.Vectors[Target.iIndex].vValue;
		case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
			MATERIAL_EXECUTION_ARTIST_PARAMETER:
			return Element.Material.Execution.ArtistParameters[Target.iIndex].vValue;
		case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::MATERIAL_EXECUTION_COLOR:
		default:
			return Element.Material.Execution.Colors[Target.iIndex].vValue;
		}
	}

	bool_t Is_AuthoringColorTargetConsistent(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_COLOR_TARGET& Target)
	{
		const float4_t& vValue = Get_AuthoringColorTargetValue(Element, Target);
		return std::all_of(Target.ExecutionTargets.begin(),
			Target.ExecutionTargets.end(),
			[&Element, &vValue](
				const AUTHORING_COLOR_EXECUTION_TARGET& ExecutionTarget)
			{
				const float4_t& vExecutionValue =
					Get_AuthoringColorExecutionTargetValue(
						Element, ExecutionTarget);
				return vExecutionValue.x == vValue.x &&
					vExecutionValue.y == vValue.y &&
					vExecutionValue.z == vValue.z &&
					vExecutionValue.w == vValue.w;
			});
	}

	void Set_AuthoringColorTargetValue(
		Client::EFFECT_ELEMENT_DESC& Element,
		const AUTHORING_COLOR_TARGET& Target,
		const float4_t& vValue)
	{
		if (Target.bHasSourceMaterial)
		{
			Element.Material.SourceMaterial.Vectors[
				Target.iSourceMaterialIndex].vValue = vValue;
		}
		for (const AUTHORING_COLOR_EXECUTION_TARGET& ExecutionTarget :
			Target.ExecutionTargets)
		{
			switch (ExecutionTarget.eStorage)
			{
			case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
				MATERIAL_EXECUTION_VECTOR:
				Element.Material.Execution.Vectors[
					ExecutionTarget.iIndex].vValue = vValue;
				break;
			case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
				MATERIAL_EXECUTION_ARTIST_PARAMETER:
				Element.Material.Execution.ArtistParameters[
					ExecutionTarget.iIndex].vValue = vValue;
				break;
			case AUTHORING_COLOR_EXECUTION_TARGET_STORAGE::
				MATERIAL_EXECUTION_COLOR:
			default:
				Element.Material.Execution.Colors[
					ExecutionTarget.iIndex].vValue = vValue;
				break;
			}
		}
	}

	bool_t Same_Float4(const float4_t& Left, const float4_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y &&
			Left.z == Right.z && Left.w == Right.w;
	}

	/* Artist overrides are only meaningful against slots and parameters the
	   compiler already produced for this element. Anything else would let the
	   tool invent a lane the source never had, so it is rejected here rather
	   than silently dropped. */
	bool_t Read_AuthoringOverrides(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ELEMENT_DESC& Out,
		std::string& strOutError)
	{
		if (!Validate_ExactFields(Value, { "resources", "scalars", "colors" },
			"Effect authoring overrides", strOutError))
		{
			return false;
		}
		const Client::DATA_JSON_VALUE* pResources = Value.Find("resources");
		const Client::DATA_JSON_VALUE* pScalars = Value.Find("scalars");
		const Client::DATA_JSON_VALUE* pColors = Value.Find("colors");
		if (nullptr == pResources || !pResources->Is_Array() ||
			nullptr == pScalars || !pScalars->Is_Array() ||
			nullptr == pColors || !pColors->Is_Array())
		{
			strOutError = "Effect authoring override arrays are invalid.";
			return false;
		}
		if (pResources->Get_Array().empty() &&
			pScalars->Get_Array().empty() && pColors->Get_Array().empty())
		{
			strOutError =
				"Effect authoring overrides must be omitted when no override exists.";
			return false;
		}

		std::set<std::string> Seen;
		for (const Client::DATA_JSON_VALUE& Row : pResources->Get_Array())
		{
			Client::EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC Override;
			if (!Row.Is_Object() ||
				!Validate_ExactFields(Row,
					{ "slotId", "assetId", "compilerAssetId" },
					"Effect authoring override resource", strOutError) ||
				!Read_String(Row, "slotId", Override.strSlotId, strOutError) ||
				!Read_String(Row, "assetId", Override.strAssetId,
					strOutError) ||
				!Read_String(Row, "compilerAssetId",
					Override.strCompilerAssetId, strOutError))
			{
				return false;
			}
			if (!Seen.insert("r:" + Override.strSlotId).second)
			{
				strOutError =
					"Effect authoring override resource slot is duplicated: " +
					Override.strSlotId;
				return false;
			}
			AUTHORING_RESOURCE_TARGET Target;
			const AUTHORING_OVERRIDE_TARGET_STATUS eTargetStatus =
				Resolve_AuthoringResourceTarget(
					Out, Override.strSlotId, Target);
			if (eTargetStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND)
			{
				strOutError =
					"Effect authoring override resource target is missing, invalid, or ambiguous: " +
					Override.strSlotId;
				return false;
			}
			Client::EFFECT_RESOURCE_FILE_KIND eArtistKind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			Client::EFFECT_RESOURCE_FILE_KIND eCompilerKind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			if (Override.strAssetId.empty() ||
				!Client::CEffectDocumentCodec::Is_SafeElementResourceAssetId(
					Out.eKind, Override.strSlotId, Override.strAssetId,
					&eArtistKind) ||
				eArtistKind != Target.eExpectedKind ||
				(!Override.strCompilerAssetId.empty() &&
					(!Client::CEffectDocumentCodec::Is_SafeElementResourceAssetId(
						Out.eKind, Override.strSlotId,
						Override.strCompilerAssetId, &eCompilerKind) ||
						eCompilerKind != Target.eExpectedKind)))
			{
				strOutError =
					"Effect authoring override resource path or declared kind is invalid: " +
					Override.strSlotId;
				return false;
			}
			if (Get_AuthoringResourceTargetAssetId(Out, Target) !=
					Override.strAssetId)
			{
				strOutError =
					"Effect authoring override resource does not match its effective target: " +
					Override.strSlotId;
				return false;
			}
			if (Override.strAssetId == Override.strCompilerAssetId)
			{
				strOutError =
					"Effect authoring override resource is a no-op delta: " +
					Override.strSlotId;
				return false;
			}
			Out.AuthoringOverrides.ResourceBindings.push_back(
				std::move(Override));
		}
		for (const Client::DATA_JSON_VALUE& Row : pScalars->Get_Array())
		{
			Client::EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC Scalar;
			if (!Row.Is_Object() ||
				!Validate_ExactFields(Row,
					{ "name", "value", "compilerValue" },
					"Effect authoring override scalar", strOutError) ||
				!Read_String(Row, "name", Scalar.strName, strOutError) ||
				!Read_Float(Row, "value", Scalar.fValue, strOutError) ||
				!Read_Float(Row, "compilerValue", Scalar.fCompilerValue,
					strOutError))
			{
				return false;
			}
			if (Scalar.strName.empty() || !std::isfinite(Scalar.fValue) ||
				!std::isfinite(Scalar.fCompilerValue) ||
				!Seen.insert("p:" + Scalar.strName).second)
			{
				strOutError = "Effect authoring override scalar is invalid.";
				return false;
			}
			AUTHORING_SCALAR_TARGET Target;
			AUTHORING_COLOR_TARGET WrongTypeTarget;
			const AUTHORING_OVERRIDE_TARGET_STATUS eTargetStatus =
				Resolve_AuthoringScalarTarget(Out, Scalar.strName, Target);
			const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
				Resolve_AuthoringColorTarget(
					Out, Scalar.strName, WrongTypeTarget);
			if (eTargetStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
			{
				strOutError =
					"Effect authoring override scalar target is missing, ambiguous, or declared as another type: " +
					Scalar.strName;
				return false;
			}
			if (!Is_AuthoringScalarTargetConsistent(Out, Target) ||
				Get_AuthoringScalarTargetValue(Out, Target) != Scalar.fValue)
			{
				strOutError =
					"Effect authoring override scalar does not match its effective target: " +
					Scalar.strName;
				return false;
			}
			if (Scalar.fValue == Scalar.fCompilerValue)
			{
				strOutError =
					"Effect authoring override scalar is a no-op delta: " +
					Scalar.strName;
				return false;
			}
			Out.AuthoringOverrides.Scalars.push_back(std::move(Scalar));
		}
		for (const Client::DATA_JSON_VALUE& Row : pColors->Get_Array())
		{
			Client::EFFECT_AUTHORING_COLOR_OVERRIDE_DESC Color;
			if (!Row.Is_Object() ||
				!Validate_ExactFields(Row,
					{ "name", "value", "compilerValue" },
					"Effect authoring override color", strOutError) ||
				!Read_String(Row, "name", Color.strName, strOutError) ||
				!Read_Array(Row, "value", &Color.vValue.x, 4u, strOutError) ||
				!Read_Array(Row, "compilerValue", &Color.vCompilerValue.x, 4u,
					strOutError))
			{
				return false;
			}
			if (Color.strName.empty() || !Is_Finite(Color.vValue) ||
				!Is_Finite(Color.vCompilerValue) ||
				!Seen.insert("p:" + Color.strName).second)
			{
				strOutError = "Effect authoring override color is invalid.";
				return false;
			}
			AUTHORING_COLOR_TARGET Target;
			AUTHORING_SCALAR_TARGET WrongTypeTarget;
			const AUTHORING_OVERRIDE_TARGET_STATUS eTargetStatus =
				Resolve_AuthoringColorTarget(Out, Color.strName, Target);
			const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
				Resolve_AuthoringScalarTarget(
					Out, Color.strName, WrongTypeTarget);
			if (eTargetStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
			{
				strOutError =
					"Effect authoring override color target is missing, ambiguous, or declared as another type: " +
					Color.strName;
				return false;
			}
			if (!Is_AuthoringColorTargetConsistent(Out, Target) ||
				!Same_Float4(
					Get_AuthoringColorTargetValue(Out, Target), Color.vValue))
			{
				strOutError =
					"Effect authoring override color does not match its effective target: " +
					Color.strName;
				return false;
			}
			if (Same_Float4(Color.vValue, Color.vCompilerValue))
			{
				strOutError =
					"Effect authoring override color is a no-op delta: " +
					Color.strName;
				return false;
			}
			Out.AuthoringOverrides.Colors.push_back(std::move(Color));
		}
		return true;
	}

	bool_t Read_SourcePresentation(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_SOURCE_PRESENTATION_DESC& Out,
		std::string& strOutError)
	{
		if (!Read_Bool(Value, "enabled", Out.bEnabled, strOutError))
			return false;
		if (!Out.bEnabled)
			return true;

		const Client::DATA_JSON_VALUE* pStatus = Find_Field(
			Value, "status", Client::DATA_JSON_TYPE::STRING, strOutError);
		const Client::DATA_JSON_VALUE* pParameters = Find_Field(
			Value, "parameters", Client::DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pStatus || nullptr == pParameters ||
			!Read_String(Value, "schema", Out.strSchema, strOutError) ||
			!Read_UInt(Value, "version", Out.iVersion, strOutError) ||
			!Read_String(Value, "profileId", Out.strProfileId, strOutError) ||
			!Parse_Token(pStatus->Get_String(),
				SOURCE_PRESENTATION_STATUS_TOKENS,
				std::size(SOURCE_PRESENTATION_STATUS_TOKENS), Out.eStatus) ||
			!Read_String(Value, "sourceObjectPath", Out.strSourceObjectPath,
				strOutError) ||
			!Read_String(Value, "sourceActionCueId", Out.strSourceActionCueId,
				strOutError) ||
			!Read_String(Value, "sourceEventId", Out.strSourceEventId,
				strOutError) ||
			!Read_UInt(Value, "sourceOccurrenceIndex",
				Out.iSourceOccurrenceIndex, strOutError) ||
			!Read_Float(Value, "sourceTimeSeconds", Out.fSourceTimeSeconds,
				strOutError) ||
			pParameters->Get_Array().size() >
				MAX_SOURCE_PRESENTATION_PARAMETERS)
		{
			return false;
		}

		Out.Parameters.reserve(pParameters->Get_Array().size());
		for (const Client::DATA_JSON_VALUE& ParameterValue :
			pParameters->Get_Array())
		{
			const Client::DATA_JSON_VALUE* pKind = ParameterValue.Is_Object() ?
				ParameterValue.Find("type") : nullptr;
			const Client::DATA_JSON_VALUE* pParameterStatus =
				ParameterValue.Is_Object() ? ParameterValue.Find("status") :
				nullptr;
			const Client::DATA_JSON_VALUE* pNumberValue =
				ParameterValue.Is_Object() ?
					ParameterValue.Find("numberValue") : nullptr;
			Client::EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC Parameter;
			if (nullptr == pKind || !pKind->Is_String() ||
				nullptr == pParameterStatus || !pParameterStatus->Is_String() ||
				nullptr == pNumberValue || !pNumberValue->Is_Number() ||
				!std::isfinite(pNumberValue->Get_Number()) ||
				!Read_String(ParameterValue, "name", Parameter.strName,
					strOutError) ||
				!Parse_Token(pKind->Get_String(),
					SOURCE_PRESENTATION_PARAMETER_KIND_TOKENS,
					std::size(SOURCE_PRESENTATION_PARAMETER_KIND_TOKENS),
					Parameter.eKind) ||
				!Parse_Token(pParameterStatus->Get_String(),
					SOURCE_PRESENTATION_PARAMETER_STATUS_TOKENS,
					std::size(SOURCE_PRESENTATION_PARAMETER_STATUS_TOKENS),
					Parameter.eStatus) ||
				!Read_String(ParameterValue, "sourcePropertyPath",
					Parameter.strSourcePropertyPath, strOutError) ||
				!Read_Bool(ParameterValue, "boolValue", Parameter.bBoolValue,
					strOutError) ||
				!Read_Array(ParameterValue, "vectorValue",
					&Parameter.vVectorValue.x, 4u, strOutError) ||
				!Read_String(ParameterValue, "stringValue",
					Parameter.strStringValue, strOutError))
			{
				return false;
			}
			Parameter.fNumberValue = pNumberValue->Get_Number();
			Out.Parameters.push_back(std::move(Parameter));
		}
		return true;
	}

	const char_t* SourcePresentationStatusToken(
		const Client::EFFECT_SOURCE_PRESENTATION_STATUS eStatus)
	{
		const size_t iIndex = static_cast<size_t>(eStatus);
		return iIndex < std::size(SOURCE_PRESENTATION_STATUS_TOKENS) ?
			SOURCE_PRESENTATION_STATUS_TOKENS[iIndex] : "unresolved";
	}

	void Write_SourcePresentation(
		std::ostringstream& Output,
		const Client::EFFECT_SOURCE_PRESENTATION_DESC& Source)
	{
		Output << "      \"sourcePresentation\": { \"enabled\": "
			<< (Source.bEnabled ? "true" : "false");
		if (!Source.bEnabled)
		{
			Output << " }";
			return;
		}
		Output << ", \"schema\": \""
			<< Client::CDataJson::Escape(Source.strSchema)
			<< "\", \"version\": " << Source.iVersion
			<< ", \"profileId\": \""
			<< Client::CDataJson::Escape(Source.strProfileId)
			<< "\", \"status\": \""
			<< SourcePresentationStatusToken(Source.eStatus)
			<< "\", \"sourceObjectPath\": \""
			<< Client::CDataJson::Escape(Source.strSourceObjectPath)
			<< "\", \"sourceActionCueId\": \""
			<< Client::CDataJson::Escape(Source.strSourceActionCueId)
			<< "\", \"sourceEventId\": \""
			<< Client::CDataJson::Escape(Source.strSourceEventId)
			<< "\", \"sourceOccurrenceIndex\": "
			<< Source.iSourceOccurrenceIndex
			<< ", \"sourceTimeSeconds\": " << Source.fSourceTimeSeconds
			<< ", \"parameters\": [";
		for (size_t iParameter = 0u;
			iParameter < Source.Parameters.size(); ++iParameter)
		{
			const Client::EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC& Parameter =
				Source.Parameters[iParameter];
			Output << (0u == iParameter ? "\n" : ",\n")
				<< "        { \"name\": \""
				<< Client::CDataJson::Escape(Parameter.strName)
				<< "\", \"type\": \""
				<< SOURCE_PRESENTATION_PARAMETER_KIND_TOKENS[
					static_cast<size_t>(Parameter.eKind)]
				<< "\", \"status\": \""
				<< SOURCE_PRESENTATION_PARAMETER_STATUS_TOKENS[
					static_cast<size_t>(Parameter.eStatus)]
				<< "\", \"sourcePropertyPath\": \""
				<< Client::CDataJson::Escape(Parameter.strSourcePropertyPath)
				<< "\", \"numberValue\": " << Parameter.fNumberValue
				<< ", \"boolValue\": "
				<< (Parameter.bBoolValue ? "true" : "false")
				<< ", \"vectorValue\": ";
			Write_Float4(Output, Parameter.vVectorValue);
			Output << ", \"stringValue\": \""
				<< Client::CDataJson::Escape(Parameter.strStringValue)
				<< "\" }";
		}
		if (!Source.Parameters.empty())
			Output << '\n';
		Output << "      ] }";
	}

	bool_t Read_ModelCueTransform(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_MODEL_CUE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLocal = Find_Field(
			Value, "localTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		const Client::DATA_JSON_VALUE* pPre = Find_Field(
			Value, "assetPreTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		return nullptr != pLocal && nullptr != pPre &&
			Read_Array(*pLocal, "position",
				&Out.LocalTransform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pLocal, "rotationDegrees",
				&Out.LocalTransform.vRotationDegrees.x, 3u, strOutError) &&
			Read_OptionalArray(*pLocal, "revolutionDegreesPerSecond",
				&Out.LocalTransform.vRevolutionDegreesPerSecond.x, 3u,
				strOutError) &&
			Read_Array(*pLocal, "scale",
				&Out.LocalTransform.vScale.x, 3u, strOutError) &&
			Read_OptionalArray(*pLocal, "velocityPerSecond",
				&Out.LocalTransform.vVelocityPerSecond.x, 3u, strOutError) &&
			Read_Array(*pPre, "scale",
				&Out.vAssetPreScale.x, 3u, strOutError) &&
			Read_Array(*pPre, "rotationDegrees",
				&Out.vAssetPreRotationDegrees.x, 3u, strOutError);
	}

	bool_t Read_ActionCueAttachment(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_ACTION_CUE_ATTACHMENT_DESC& Out,
		std::string& strOutError)
	{
		if (const Client::DATA_JSON_VALUE* pOrientation = Value.Find("orientation"))
		{
			if (!pOrientation->Is_String() ||
				!Parse_Token(pOrientation->Get_String(), ATTACHMENT_ORIENTATION_TOKENS,
					std::size(ATTACHMENT_ORIENTATION_TOKENS), Out.eOrientation))
			{
				strOutError = "Effect Action cue attachment orientation must be bone or owner_yaw.";
				return false;
			}
		}
		const Client::DATA_JSON_VALUE* pSocketLocal = Find_Field(
			Value, "socketLocalTransform", Client::DATA_JSON_TYPE::OBJECT,
			strOutError);
		return nullptr != pSocketLocal &&
			Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_Bool(Value, "follow", Out.bFollow, strOutError) &&
			Read_String(Value, "sourceAnchorSlotId",
				Out.strSourceAnchorSlotId, strOutError) &&
			Read_String(Value, "runtimeAnchorSlotId",
				Out.strRuntimeAnchorSlotId, strOutError) &&
			Read_String(Value, "runtimeBoneName",
				Out.strRuntimeBoneName, strOutError) &&
			Read_OptionalFloat(Value, "snapshotRootSourceBasisYawDegrees",
				Out.fSnapshotRootSourceBasisYawDegrees, strOutError) &&
			Read_Array(*pSocketLocal, "position",
				&Out.SocketLocalTransform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pSocketLocal, "rotationDegrees",
				&Out.SocketLocalTransform.vRotationDegrees.x, 3u,
				strOutError) &&
			Read_Array(*pSocketLocal, "scale",
				&Out.SocketLocalTransform.vScale.x, 3u, strOutError);
	}

	bool_t Read_TransformInheritance(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_TRANSFORM_INHERITANCE_DESC& Out,
		std::string& strOutError)
	{
		return Read_Bool(Value, "enabled", Out.bEnabled, strOutError) &&
			Read_String(Value, "masterElementId",
				Out.strMasterElementId, strOutError);
	}

	bool_t Is_SafeModelCueAssetIdInternal(const std::string& strAssetId)
	{
		const bool_t bAllowedRoot =
			0u == strAssetId.rfind("Character/", 0u) ||
			0u == strAssetId.rfind("Effect/", 0u);
		if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
			!bAllowedRoot ||
			std::string::npos != strAssetId.find('\\') ||
			std::string::npos != strAssetId.find(':'))
		{
			return false;
		}
		const std::filesystem::path RelativePath(strAssetId);
		if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
			RelativePath.lexically_normal().generic_string() != strAssetId ||
			RelativePath.extension() != ".wmodel")
		{
			return false;
		}
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Value = Component.generic_string();
			if (Value.empty() || Value == "." || Value == "..")
				return false;
		}
		const std::filesystem::path Resolved =
			CRuntimeAssetRoot::Resolve(RelativePath);
		std::error_code Error;
		return !Resolved.empty() &&
			std::filesystem::is_regular_file(Resolved, Error) && !Error;
	}

	bool_t Read_MeshRingFill(
		const Client::DATA_JSON_VALUE& Mesh,
		Client::EFFECT_MESH_RING_FILL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pRingFill = Mesh.Find("ringFill");
		if (nullptr == pRingFill)
			return true;
		const Client::DATA_JSON_VALUE* pDirection = nullptr;
		if (!pRingFill->Is_Object() ||
			nullptr == (pDirection = pRingFill->Find("direction")) ||
			!pDirection->Is_String() ||
			!Parse_Token(pDirection->Get_String(), RING_FILL_DIRECTION_TOKENS,
				std::size(RING_FILL_DIRECTION_TOKENS), Out.eDirection))
		{
			strOutError = "Effect mesh ringFill direction is invalid.";
			return false;
		}
		return Read_Bool(*pRingFill, "enabled", Out.bEnabled, strOutError) &&
			Read_Float(*pRingFill, "progress", Out.fProgress, strOutError) &&
			Read_Float(*pRingFill, "feather", Out.fFeather, strOutError) &&
			Read_Bool(*pRingFill, "invert", Out.bInvert, strOutError);
	}

	bool_t Read_LinearReveal(
		const Client::DATA_JSON_VALUE& Sprite,
		Client::EFFECT_LINEAR_REVEAL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pReveal = Sprite.Find("linearReveal");
		if (nullptr == pReveal)
			return true;
		const Client::DATA_JSON_VALUE* pAxis = nullptr;
		if (!pReveal->Is_Object() ||
			nullptr == (pAxis = pReveal->Find("axis")) ||
			!pAxis->Is_String() ||
			!Parse_Token(pAxis->Get_String(), LINEAR_REVEAL_AXIS_TOKENS,
				std::size(LINEAR_REVEAL_AXIS_TOKENS), Out.eAxis))
		{
			strOutError = "Effect linearReveal axis is invalid.";
			return false;
		}
		return Read_Bool(*pReveal, "enabled", Out.bEnabled, strOutError) &&
			Read_Bool(*pReveal, "invert", Out.bInvert, strOutError) &&
			Read_Float(*pReveal, "startSeconds", Out.fStartSeconds,
				strOutError) &&
			Read_Float(*pReveal, "durationSeconds", Out.fDurationSeconds,
				strOutError) &&
			Read_Float(*pReveal, "edgeWidth", Out.fEdgeWidth, strOutError) &&
			Read_Float(*pReveal, "softness", Out.fSoftness, strOutError) &&
			Read_Array(*pReveal, "edgeColor", &Out.vEdgeColor.x, 4u,
				strOutError) &&
			Read_Float(*pReveal, "edgeEmissive", Out.fEdgeEmissive,
				strOutError);
	}

	bool_t Read_DecalReceiver(
		const Client::DATA_JSON_VALUE& Decal,
		Client::EFFECT_DECAL_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		if (const Client::DATA_JSON_VALUE* pMode =
			Decal.Find("receiverMode"))
		{
			if (!pMode->Is_String() ||
				!Parse_Token(pMode->Get_String(), DECAL_RECEIVER_MODE_TOKENS,
					std::size(DECAL_RECEIVER_MODE_TOKENS), Out.eReceiverMode))
			{
				strOutError = "Effect decal receiverMode is invalid.";
				return false;
			}
		}
		return Read_OptionalFloat(Decal, "normalCutoff",
			Out.fNormalCutoff, strOutError) &&
			Read_OptionalFloat(Decal, "edgeFade", Out.fEdgeFade,
				strOutError);
	}

	bool_t Read_CommonDetail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pTransform = Find_Field(
			Value, "transform", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pColor = Find_Field(
			Value, "color", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pUV = Find_Field(
			Value, "uv", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTiming = Find_Field(
			Value, "timing", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pMesh = Find_Field(
			Value, "mesh", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pSprite = Find_Field(
			Value, "sprite", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pDecal = Find_Field(
			Value, "decal", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pTransform || nullptr == pColor || nullptr == pUV ||
			nullptr == pTiming || nullptr == pMesh || nullptr == pSprite ||
			nullptr == pDecal)
		{
			return false;
		}

		return Read_Array(*pTransform, "position", &Out.Transform.vPosition.x, 3u, strOutError) &&
			Read_Array(*pTransform, "rotationDegrees", &Out.Transform.vRotationDegrees.x, 3u, strOutError) &&
			Read_Array(*pTransform, "revolutionDegreesPerSecond", &Out.Transform.vRevolutionDegreesPerSecond.x, 3u, strOutError) &&
			Read_Array(*pTransform, "scale", &Out.Transform.vScale.x, 3u, strOutError) &&
			Read_Array(*pColor, "offset", &Out.Color.vColorOffset.x, 4u, strOutError) &&
			Read_Array(*pColor, "multiply", &Out.Color.vColorMultiply.x, 4u, strOutError) &&
			Read_Float(*pColor, "clip", Out.Color.fColorClip, strOutError) &&
			Read_Float(*pColor, "emissiveIntensity", Out.Color.fEmissiveIntensity, strOutError) &&
			Read_Float(*pColor, "distortionIntensity", Out.Color.fDistortionIntensity, strOutError) &&
			Read_Bool(*pColor, "distortionOnBaseMaterial", Out.Color.bDistortionOnBaseMaterial, strOutError) &&
			Read_Float(*pColor, "radialTime", Out.Color.fRadialTime, strOutError) &&
			Read_Float(*pColor, "radialIntensity", Out.Color.fRadialIntensity, strOutError) &&
			Read_Array(*pUV, "start", &Out.UV.vStart.x, 2u, strOutError) &&
			Read_Array(*pUV, "speed", &Out.UV.vSpeed.x, 2u, strOutError) &&
			Read_Bool(*pUV, "wave", Out.UV.bWave, strOutError) &&
			Read_Array(*pUV, "waveAmplitude", &Out.UV.vWaveAmplitude.x, 2u, strOutError) &&
			Read_Float(*pUV, "waveFrequency", Out.UV.fWaveFrequency, strOutError) &&
			Read_Bool(*pUV, "sequence", Out.UV.bSequence, strOutError) &&
			Read_Bool(*pUV, "loop", Out.UV.bLoop, strOutError) &&
			Read_Float(*pUV, "sequenceTerm", Out.UV.fSequenceTerm, strOutError) &&
			Read_Int(*pUV, "tileColumns", Out.UV.iTileColumns, strOutError) &&
			Read_Int(*pUV, "tileRows", Out.UV.iTileRows, strOutError) &&
			Read_Int(*pUV, "tileIndex", Out.UV.iTileIndex, strOutError) &&
			Read_Float(*pTiming, "startDelaySeconds", Out.Timing.fStartDelaySeconds, strOutError) &&
			Read_Float(*pTiming, "lifeTimeSeconds", Out.Timing.fLifeTimeSeconds, strOutError) &&
			Read_OptionalFloat(*pTiming, "transformMotionDurationSeconds",
				Out.Timing.fTransformMotionDurationSeconds, strOutError) &&
			Read_Float(*pTiming, "afterImageSeconds", Out.Timing.fAfterImageSeconds, strOutError) &&
			Read_Float(*pTiming, "dissolveStartNormalized", Out.Timing.fDissolveStartNormalized, strOutError) &&
			Read_Bool(*pMesh, "useModelMaterial", Out.Mesh.bUseModelMaterial, strOutError) &&
			Read_OptionalFloat(*pMesh, "modelPreScale",
				Out.Mesh.fModelPreScale, strOutError) &&
			Read_OptionalArray(*pMesh, "sourceTypeDataRotationDegrees",
				&Out.Mesh.vSourceTypeDataRotationDegrees.x, 3u, strOutError) &&
			Read_MeshRingFill(*pMesh, Out.Mesh.RingFill, strOutError) &&
			Read_Bool(*pSprite, "billboard", Out.Sprite.bBillboard, strOutError) &&
			Read_OptionalFloat(*pSprite, "billboardRollDegrees",
				Out.Sprite.fBillboardRollDegrees, strOutError) &&
			Read_OptionalFloat(*pSprite, "billboardRollDegreesPerSecond",
				Out.Sprite.fBillboardRollDegreesPerSecond, strOutError) &&
			Read_LinearReveal(*pSprite, Out.Sprite.LinearReveal, strOutError) &&
			Read_Array(*pDecal, "size", &Out.Decal.vSize.x, 2u, strOutError) &&
			Read_Float(*pDecal, "depth", Out.Decal.fDepth, strOutError) &&
			Read_DecalReceiver(*pDecal, Out.Decal, strOutError);
	}

	/* These optional blocks are absent from documents written before they existed,
	   and absent has to keep meaning POINT/random/FIXED. They are read like
	   modelPreScale and the DynamicParameter triple: optional on the way in,
	   emitted on the way out only when they carry something other than the
	   historical default. */
	bool_t Read_ParticleSpawnShape(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pShape = Particle.Find("spawnShape");
		if (nullptr == pShape)
			return true;
		const Client::DATA_JSON_VALUE* pKind = nullptr;
		if (!pShape->Is_Object() ||
			nullptr == (pKind = pShape->Find("kind")) || !pKind->Is_String() ||
			!Parse_Token(pKind->Get_String(), PARTICLE_SPAWN_SHAPE_TOKENS,
				std::size(PARTICLE_SPAWN_SHAPE_TOKENS), Out.eKind))
		{
			strOutError = "Effect particle spawnShape kind is invalid.";
			return false;
		}
		if (const Client::DATA_JSON_VALUE* pDistribution =
			pShape->Find("distribution"))
		{
			if (!pDistribution->Is_String() ||
				!Parse_Token(pDistribution->Get_String(),
					PARTICLE_SPAWN_DISTRIBUTION_TOKENS,
					std::size(PARTICLE_SPAWN_DISTRIBUTION_TOKENS),
					Out.eDistribution))
			{
				strOutError =
					"Effect particle spawnShape distribution is invalid.";
				return false;
			}
		}
		return Read_OptionalFloat(*pShape, "radius", Out.fRadius, strOutError) &&
			Read_OptionalFloat(*pShape, "innerRadius", Out.fInnerRadius,
				strOutError) &&
			Read_OptionalArray(*pShape, "extents", &Out.vExtents.x, 3u,
				strOutError) &&
			Read_OptionalFloat(*pShape, "arcDegrees", Out.fArcDegrees,
				strOutError);
	}

	bool_t Read_ParticleInitialOrientation(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pOrientation =
			Particle.Find("initialOrientation");
		if (nullptr == pOrientation)
			return true;
		const Client::DATA_JSON_VALUE* pMode = nullptr;
		if (!pOrientation->Is_Object() ||
			nullptr == (pMode = pOrientation->Find("mode")) ||
			!pMode->Is_String() ||
			!Parse_Token(pMode->Get_String(), PARTICLE_ORIENTATION_MODE_TOKENS,
				std::size(PARTICLE_ORIENTATION_MODE_TOKENS), Out.eMode))
		{
			strOutError = "Effect particle initialOrientation mode is invalid.";
			return false;
		}
		return Read_Float(*pOrientation, "offsetDegrees", Out.fOffsetDegrees,
			strOutError);
	}

	bool_t Read_ParticleInitialVelocity(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pVelocity =
			Particle.Find("initialVelocity");
		if (nullptr == pVelocity)
			return true;
		const Client::DATA_JSON_VALUE* pMode = nullptr;
		if (!pVelocity->Is_Object() ||
			nullptr == (pMode = pVelocity->Find("mode")) || !pMode->Is_String() ||
			!Parse_Token(pMode->Get_String(), PARTICLE_VELOCITY_MODE_TOKENS,
				std::size(PARTICLE_VELOCITY_MODE_TOKENS), Out.eMode))
		{
			strOutError = "Effect particle initialVelocity mode is invalid.";
			return false;
		}
		return Read_OptionalArray(*pVelocity, "speed", &Out.vSpeedRange.x, 2u,
				strOutError) &&
			Read_OptionalFloat(*pVelocity, "coneAngleDegrees",
				Out.fConeAngleDegrees, strOutError) &&
			Read_OptionalBool(*pVelocity, "uniformSolidAngle",
				Out.bUniformSolidAngle, strOutError);
	}

	bool_t Read_ParticleTargetAttractor(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pAttractor =
			Particle.Find("targetAttractor");
		if (nullptr == pAttractor)
			return true;
		const Client::DATA_JSON_VALUE* pTargetSpace = nullptr;
		if (!pAttractor->Is_Object() ||
			nullptr == (pTargetSpace = pAttractor->Find("targetSpace")) ||
			!pTargetSpace->Is_String() ||
			!Parse_Token(pTargetSpace->Get_String(),
				PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS,
				std::size(PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS),
				Out.eTargetSpace))
		{
			strOutError =
				"Effect particle targetAttractor targetSpace is invalid.";
			return false;
		}
		return Read_Bool(*pAttractor, "enabled", Out.bEnabled, strOutError) &&
			Read_Array(*pAttractor, "targetOffset", &Out.vTargetOffset.x, 3u,
				strOutError) &&
			Read_Array(*pAttractor, "activeNormalized",
				&Out.vActiveNormalized.x, 2u, strOutError) &&
			Read_Float(*pAttractor, "radialAcceleration",
				Out.fRadialAcceleration, strOutError) &&
			Read_Float(*pAttractor, "tangentialAcceleration",
				Out.fTangentialAcceleration, strOutError) &&
			Read_Float(*pAttractor, "maximumSpeed", Out.fMaximumSpeed,
				strOutError) &&
			Read_Float(*pAttractor, "convergenceRadius",
				Out.fConvergenceRadius, strOutError) &&
			Read_Float(*pAttractor, "arrivalDamping", Out.fArrivalDamping,
				strOutError);
	}

	bool_t Read_ParticleSourceScale(
		const Client::DATA_JSON_VALUE& Particle,
		Client::EFFECT_PARTICLE_SOURCE_SCALE_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pScale = Particle.Find("sourceScale");
		if (nullptr == pScale)
			return true;
		if (!pScale->Is_Object())
		{
			strOutError = "Effect particle sourceScale is not an object.";
			return false;
		}
		/* The four trailing factors were added after documents already existed
		   with the first three, so each one is optional and defaults to 1. */
		return Read_OptionalFloat(*pScale, "count", Out.fCount, strOutError) &&
			Read_OptionalFloat(*pScale, "size", Out.fSize, strOutError) &&
			Read_OptionalFloat(*pScale, "lifeTime", Out.fLifeTime, strOutError) &&
			Read_OptionalFloat(*pScale, "speed", Out.fSpeed, strOutError) &&
			Read_OptionalFloat(*pScale, "rotation", Out.fRotation,
				strOutError) &&
			Read_OptionalFloat(*pScale, "alpha", Out.fAlpha, strOutError) &&
			Read_OptionalFloat(*pScale, "spawnDelay", Out.fSpawnDelay,
				strOutError);
	}

	bool_t Read_V5Detail(
		const Client::DATA_JSON_VALUE& Value,
		Client::EFFECT_DETAIL_DESC& Out,
		std::string& strOutError)
	{
		const Client::DATA_JSON_VALUE* pLerp = Find_Field(
			Value, "linearLerp", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pParticle = Find_Field(
			Value, "particle", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pTrail = Find_Field(
			Value, "trail", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		const Client::DATA_JSON_VALUE* pAfterImage = Find_Field(
			Value, "afterImage", Client::DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pLerp || nullptr == pParticle || nullptr == pTrail ||
			nullptr == pAfterImage)
		{
			return false;
		}

		return Read_Array(*Value.Find("transform"), "velocityPerSecond", &Out.Transform.vVelocityPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "position", Out.LinearLerp.bPosition, strOutError) &&
			Read_Array(*pLerp, "endPosition", &Out.LinearLerp.vEndPosition.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "rotation", Out.LinearLerp.bRotation, strOutError) &&
			Read_Array(*pLerp, "endRotationDegrees", &Out.LinearLerp.vEndRotationDegrees.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "revolution", Out.LinearLerp.bRevolution, strOutError) &&
			Read_Array(*pLerp, "endRevolutionDegreesPerSecond", &Out.LinearLerp.vEndRevolutionDegreesPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "scale", Out.LinearLerp.bScale, strOutError) &&
			Read_Array(*pLerp, "endScale", &Out.LinearLerp.vEndScale.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "velocity", Out.LinearLerp.bVelocity, strOutError) &&
			Read_Array(*pLerp, "endVelocityPerSecond", &Out.LinearLerp.vEndVelocityPerSecond.x, 3u, strOutError) &&
			Read_Bool(*pLerp, "colorOffset", Out.LinearLerp.bColorOffset, strOutError) &&
			Read_Array(*pLerp, "endColorOffset", &Out.LinearLerp.vEndColorOffset.x, 4u, strOutError) &&
			Read_Bool(*pLerp, "colorMultiply", Out.LinearLerp.bColorMultiply, strOutError) &&
			Read_Array(*pLerp, "endColorMultiply", &Out.LinearLerp.vEndColorMultiply.x, 4u, strOutError) &&
			Read_Bool(*pLerp, "emissiveIntensity", Out.LinearLerp.bEmissiveIntensity, strOutError) &&
			Read_Float(*pLerp, "endEmissiveIntensity", Out.LinearLerp.fEndEmissiveIntensity, strOutError) &&
			Read_OptionalBool(*pLerp, "ringFillProgress",
				Out.LinearLerp.bRingFillProgress, strOutError) &&
			Read_OptionalFloat(*pLerp, "endRingFillProgress",
				Out.LinearLerp.fEndRingFillProgress, strOutError) &&
			Read_UInt(*pParticle, "maxParticles", Out.Particle.iMaxParticles, strOutError) &&
			Read_Float(*pParticle, "spawnRatePerSecond", Out.Particle.fSpawnRatePerSecond, strOutError) &&
			Read_OptionalFloat(*pParticle, "fixedCenterSpacingWorldUnits",
				Out.Particle.fFixedCenterSpacingWorldUnits, strOutError) &&
			Read_UInt(*pParticle, "burstCount", Out.Particle.iBurstCount, strOutError) &&
			Read_UInt(*pParticle, "randomSeed", Out.Particle.iRandomSeed, strOutError) &&
			Read_Array(*pParticle, "lifeTimeSeconds", &Out.Particle.vLifeTimeSeconds.x, 2u, strOutError) &&
			Read_OptionalArray(*pParticle, "initialPositionMin", &Out.Particle.vInitialPositionMin.x, 3u, strOutError) &&
			Read_OptionalArray(*pParticle, "initialPositionMax", &Out.Particle.vInitialPositionMax.x, 3u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMin", &Out.Particle.vInitialVelocityMin.x, 3u, strOutError) &&
			Read_Array(*pParticle, "initialVelocityMax", &Out.Particle.vInitialVelocityMax.x, 3u, strOutError) &&
			Read_Array(*pParticle, "acceleration", &Out.Particle.vAcceleration.x, 3u, strOutError) &&
			Read_Array(*pParticle, "startSize", &Out.Particle.vStartSize.x, 2u, strOutError) &&
			Read_Array(*pParticle, "endSize", &Out.Particle.vEndSize.x, 2u, strOutError) &&
			Read_Bool(*pParticle, "localSpace", Out.Particle.bLocalSpace, strOutError) &&
			Read_Bool(*pParticle, "billboard", Out.Particle.bBillboard, strOutError) &&
			Read_OptionalFloat(*pParticle, "drag", Out.Particle.fDrag,
				strOutError) &&
			Read_OptionalArray(*pParticle, "rotationRangeDegrees",
				&Out.Particle.vRotationRangeDegrees.x, 2u, strOutError) &&
			Read_OptionalArray(*pParticle, "spinRangeDegreesPerSecond",
				&Out.Particle.vSpinRangeDegreesPerSecond.x, 2u, strOutError) &&
			Read_OptionalBool(*pParticle, "subUVOverLife",
				Out.Particle.bSubUVOverLife, strOutError) &&
			Read_OptionalUInt(*pParticle, "dynamicParameterComponentMask",
				Out.Particle.iDynamicParameterComponentMask, strOutError) &&
			Read_OptionalArray(*pParticle, "dynamicParameterStart",
				&Out.Particle.vDynamicParameterStart.x, 4u, strOutError) &&
			Read_OptionalArray(*pParticle, "dynamicParameterEnd",
				&Out.Particle.vDynamicParameterEnd.x, 4u, strOutError) &&
			Read_ParticleSpawnShape(*pParticle, Out.Particle.SpawnShape,
				strOutError) &&
			Read_ParticleInitialOrientation(*pParticle,
				Out.Particle.InitialOrientation, strOutError) &&
			Read_ParticleInitialVelocity(*pParticle,
				Out.Particle.InitialVelocity, strOutError) &&
			Read_ParticleTargetAttractor(*pParticle,
				Out.Particle.TargetAttractor, strOutError) &&
			Read_ParticleSourceScale(*pParticle, Out.Particle.SourceScale,
				strOutError) &&
			Read_UInt(*pTrail, "maxPoints", Out.Trail.iMaxPoints, strOutError) &&
			Read_Float(*pTrail, "pointLifeTimeSeconds", Out.Trail.fPointLifeTimeSeconds, strOutError) &&
			Read_Float(*pTrail, "sampleIntervalSeconds", Out.Trail.fSampleIntervalSeconds, strOutError) &&
			Read_Float(*pTrail, "minimumDistance", Out.Trail.fMinimumDistance, strOutError) &&
			Read_Float(*pTrail, "startWidth", Out.Trail.fStartWidth, strOutError) &&
			Read_Float(*pTrail, "endWidth", Out.Trail.fEndWidth, strOutError) &&
			Read_OptionalFloat(*pTrail, "tilingDistanceWorldUnits",
				Out.Trail.fTilingDistanceWorldUnits, strOutError) &&
			Read_OptionalFloat(*pTrail, "distanceTessellationStepWorldUnits",
				Out.Trail.fDistanceTessellationStepWorldUnits, strOutError) &&
			Read_Bool(*pTrail, "faceCamera", Out.Trail.bFaceCamera, strOutError) &&
			Read_Float(*pAfterImage, "sampleIntervalSeconds", Out.AfterImage.fSampleIntervalSeconds, strOutError) &&
			Read_UInt(*pAfterImage, "maxCopies", Out.AfterImage.iMaxCopies, strOutError) &&
			Read_Float(*pAfterImage, "alphaExponent", Out.AfterImage.fAlphaExponent, strOutError);
	}

	void Write_Detail(
		std::ostringstream& Output,
		const Client::EFFECT_DETAIL_DESC& Detail)
	{
		Output << "      \"detail\": {\n"
			<< "        \"transform\": { \"position\": ";
		Write_Float3(Output, Detail.Transform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Detail.Transform.vRotationDegrees);
		Output << ", \"revolutionDegreesPerSecond\": ";
		Write_Float3(Output, Detail.Transform.vRevolutionDegreesPerSecond);
		Output << ", \"scale\": ";
		Write_Float3(Output, Detail.Transform.vScale);
		Output << ", \"velocityPerSecond\": ";
		Write_Float3(Output, Detail.Transform.vVelocityPerSecond);
		Output << " },\n        \"color\": { \"offset\": ";
		Write_Float4(Output, Detail.Color.vColorOffset);
		Output << ", \"multiply\": ";
		Write_Float4(Output, Detail.Color.vColorMultiply);
		Output << ", \"clip\": " << Detail.Color.fColorClip;
		Output
			<< ", \"emissiveIntensity\": " << Detail.Color.fEmissiveIntensity
			<< ", \"distortionIntensity\": " << Detail.Color.fDistortionIntensity
			<< ", \"distortionOnBaseMaterial\": " << (Detail.Color.bDistortionOnBaseMaterial ? "true" : "false")
			<< ", \"radialTime\": " << Detail.Color.fRadialTime
			<< ", \"radialIntensity\": " << Detail.Color.fRadialIntensity
			<< " },\n        \"uv\": { \"start\": ";
		Write_Float2(Output, Detail.UV.vStart);
		Output << ", \"speed\": ";
		Write_Float2(Output, Detail.UV.vSpeed);
		Output << ", \"wave\": " << (Detail.UV.bWave ? "true" : "false")
			<< ", \"waveAmplitude\": ";
		Write_Float2(Output, Detail.UV.vWaveAmplitude);
		Output << ", \"waveFrequency\": " << Detail.UV.fWaveFrequency
			<< ", \"sequence\": " << (Detail.UV.bSequence ? "true" : "false")
			<< ", \"loop\": " << (Detail.UV.bLoop ? "true" : "false")
			<< ", \"sequenceTerm\": " << Detail.UV.fSequenceTerm
			<< ", \"tileColumns\": " << Detail.UV.iTileColumns
			<< ", \"tileRows\": " << Detail.UV.iTileRows
			<< ", \"tileIndex\": " << Detail.UV.iTileIndex
			<< " },\n        \"timing\": { \"startDelaySeconds\": " << Detail.Timing.fStartDelaySeconds
			<< ", \"lifeTimeSeconds\": " << Detail.Timing.fLifeTimeSeconds;
		/* Omission is the legacy identity: old documents remain byte-stable,
		   use Life for authored lerps, and keep velocity/Revolution on local time. */
		if (Detail.Timing.fTransformMotionDurationSeconds != 0.f)
		{
			Output << ", \"transformMotionDurationSeconds\": " <<
				Detail.Timing.fTransformMotionDurationSeconds;
		}
		Output << ", \"afterImageSeconds\": " << Detail.Timing.fAfterImageSeconds
			<< ", \"dissolveStartNormalized\": " << Detail.Timing.fDissolveStartNormalized
			<< " },\n        \"mesh\": { \"useModelMaterial\": " << (Detail.Mesh.bUseModelMaterial ? "true" : "false");
		/* Keep legacy v12/v13 typed-codec identities byte-stable. The optional
		   field is emitted only when an imported WModel carrier actually needs a
		   non-default scale such as Artist F's 0.01. */
		if (Detail.Mesh.fModelPreScale != 1.f)
			Output << ", \"modelPreScale\": " << Detail.Mesh.fModelPreScale;
		Output << ", \"sourceTypeDataRotationDegrees\": ";
		Write_Float3(Output, Detail.Mesh.vSourceTypeDataRotationDegrees);
		if (Detail.Mesh.RingFill.bEnabled)
		{
			Output << ", \"ringFill\": { \"enabled\": true, \"progress\": "
				<< Detail.Mesh.RingFill.fProgress
				<< ", \"direction\": \""
				<< RING_FILL_DIRECTION_TOKENS[static_cast<size_t>(
					Detail.Mesh.RingFill.eDirection)]
				<< "\", \"feather\": " << Detail.Mesh.RingFill.fFeather
				<< ", \"invert\": "
				<< (Detail.Mesh.RingFill.bInvert ? "true" : "false")
				<< " }";
		}
		Output << " },\n        \"sprite\": { \"billboard\": " << (Detail.Sprite.bBillboard ? "true" : "false")
			<< ", \"billboardRollDegrees\": " << Detail.Sprite.fBillboardRollDegrees
			<< ", \"billboardRollDegreesPerSecond\": "
			<< Detail.Sprite.fBillboardRollDegreesPerSecond;
		if (Detail.Sprite.LinearReveal.bEnabled)
		{
			Output << ", \"linearReveal\": { \"enabled\": true, \"axis\": \""
				<< LINEAR_REVEAL_AXIS_TOKENS[static_cast<size_t>(
					Detail.Sprite.LinearReveal.eAxis)]
				<< "\", \"invert\": "
				<< (Detail.Sprite.LinearReveal.bInvert ? "true" : "false")
				<< ", \"startSeconds\": "
				<< Detail.Sprite.LinearReveal.fStartSeconds
				<< ", \"durationSeconds\": "
				<< Detail.Sprite.LinearReveal.fDurationSeconds
				<< ", \"edgeWidth\": " << Detail.Sprite.LinearReveal.fEdgeWidth
				<< ", \"softness\": " << Detail.Sprite.LinearReveal.fSoftness
				<< ", \"edgeColor\": ";
			Write_Float4(Output, Detail.Sprite.LinearReveal.vEdgeColor);
			Output << ", \"edgeEmissive\": "
				<< Detail.Sprite.LinearReveal.fEdgeEmissive << " }";
		}
		Output << " },\n        \"decal\": { \"size\": ";
		Write_Float2(Output, Detail.Decal.vSize);
		Output << ", \"depth\": " << Detail.Decal.fDepth;
		if (Detail.Decal.eReceiverMode !=
			EFFECT_DECAL_RECEIVER_MODE::ALL_OPAQUE)
		{
			Output << ", \"receiverMode\": \""
				<< DECAL_RECEIVER_MODE_TOKENS[static_cast<size_t>(
					Detail.Decal.eReceiverMode)]
				<< "\", \"normalCutoff\": "
				<< Detail.Decal.fNormalCutoff;
		}
		if (Detail.Decal.fEdgeFade != 0.f)
			Output << ", \"edgeFade\": " << Detail.Decal.fEdgeFade;
		Output << " },\n"
			<< "        \"linearLerp\": { \"position\": " << (Detail.LinearLerp.bPosition ? "true" : "false")
			<< ", \"endPosition\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndPosition);
		Output << ", \"rotation\": " << (Detail.LinearLerp.bRotation ? "true" : "false") << ", \"endRotationDegrees\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndRotationDegrees);
		Output << ", \"revolution\": " << (Detail.LinearLerp.bRevolution ? "true" : "false") << ", \"endRevolutionDegreesPerSecond\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndRevolutionDegreesPerSecond);
		Output << ", \"scale\": " << (Detail.LinearLerp.bScale ? "true" : "false") << ", \"endScale\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndScale);
		Output << ", \"velocity\": " << (Detail.LinearLerp.bVelocity ? "true" : "false") << ", \"endVelocityPerSecond\": ";
		Write_Float3(Output, Detail.LinearLerp.vEndVelocityPerSecond);
		Output << ", \"colorOffset\": " << (Detail.LinearLerp.bColorOffset ? "true" : "false") << ", \"endColorOffset\": ";
		Write_Float4(Output, Detail.LinearLerp.vEndColorOffset);
		Output << ", \"colorMultiply\": " << (Detail.LinearLerp.bColorMultiply ? "true" : "false") << ", \"endColorMultiply\": ";
		Write_Float4(Output, Detail.LinearLerp.vEndColorMultiply);
		Output << ", \"emissiveIntensity\": " << (Detail.LinearLerp.bEmissiveIntensity ? "true" : "false")
			<< ", \"endEmissiveIntensity\": " << Detail.LinearLerp.fEndEmissiveIntensity;
		if (Detail.LinearLerp.bRingFillProgress)
		{
			Output << ", \"ringFillProgress\": true, \"endRingFillProgress\": "
				<< Detail.LinearLerp.fEndRingFillProgress;
		}
		Output << " },\n"
			<< "        \"particle\": { \"maxParticles\": " << Detail.Particle.iMaxParticles
			<< ", \"spawnRatePerSecond\": " << Detail.Particle.fSpawnRatePerSecond;
		if (Detail.Particle.fFixedCenterSpacingWorldUnits > 0.f)
		{
			Output << ", \"fixedCenterSpacingWorldUnits\": "
				<< Detail.Particle.fFixedCenterSpacingWorldUnits;
		}
		Output << ", \"burstCount\": " << Detail.Particle.iBurstCount
			<< ", \"randomSeed\": " << Detail.Particle.iRandomSeed
			<< ", \"lifeTimeSeconds\": ";
		Write_Float2(Output, Detail.Particle.vLifeTimeSeconds);
		Output << ", \"initialPositionMin\": ";
		Write_Float3(Output, Detail.Particle.vInitialPositionMin);
		Output << ", \"initialPositionMax\": ";
		Write_Float3(Output, Detail.Particle.vInitialPositionMax);
		Output << ", \"initialVelocityMin\": ";
		Write_Float3(Output, Detail.Particle.vInitialVelocityMin);
		Output << ", \"initialVelocityMax\": ";
		Write_Float3(Output, Detail.Particle.vInitialVelocityMax);
		Output << ", \"acceleration\": ";
		Write_Float3(Output, Detail.Particle.vAcceleration);
		Output << ", \"startSize\": ";
		Write_Float2(Output, Detail.Particle.vStartSize);
		Output << ", \"endSize\": ";
		Write_Float2(Output, Detail.Particle.vEndSize);
		Output << ", \"localSpace\": " << (Detail.Particle.bLocalSpace ? "true" : "false")
			<< ", \"billboard\": " << (Detail.Particle.bBillboard ? "true" : "false");
		if (Detail.Particle.fDrag != 0.f)
			Output << ", \"drag\": " << Detail.Particle.fDrag;
		if (Detail.Particle.vRotationRangeDegrees.x != 0.f ||
			Detail.Particle.vRotationRangeDegrees.y != 0.f)
		{
			Output << ", \"rotationRangeDegrees\": ";
			Write_Float2(Output, Detail.Particle.vRotationRangeDegrees);
		}
		if (Detail.Particle.vSpinRangeDegreesPerSecond.x != 0.f ||
			Detail.Particle.vSpinRangeDegreesPerSecond.y != 0.f)
		{
			Output << ", \"spinRangeDegreesPerSecond\": ";
			Write_Float2(Output, Detail.Particle.vSpinRangeDegreesPerSecond);
		}
		if (Detail.Particle.bSubUVOverLife)
			Output << ", \"subUVOverLife\": true";
		/* Preserve the canonical typed-codec identity of existing v12/v13
		   documents. Dynamic Parameter authoring is optional and is emitted only
		   when at least one component is intentionally owned by the authored
		   particle. */
		if (0u != Detail.Particle.iDynamicParameterComponentMask)
		{
			Output << ", \"dynamicParameterComponentMask\": "
				<< Detail.Particle.iDynamicParameterComponentMask
				<< ", \"dynamicParameterStart\": ";
			Write_Float4(Output, Detail.Particle.vDynamicParameterStart);
			Output << ", \"dynamicParameterEnd\": ";
			Write_Float4(Output, Detail.Particle.vDynamicParameterEnd);
		}
		/* Same reason as modelPreScale above: a document that still spawns from a
		   point with random distribution and a fixed velocity/orientation must
		   serialize byte-identically to the legacy writer. */
		if (EFFECT_PARTICLE_SPAWN_SHAPE::POINT != Detail.Particle.SpawnShape.eKind)
		{
			Output << ", \"spawnShape\": { \"kind\": \""
				<< PARTICLE_SPAWN_SHAPE_TOKENS[
					static_cast<size_t>(Detail.Particle.SpawnShape.eKind)]
				<< "\", \"radius\": " << Detail.Particle.SpawnShape.fRadius
				<< ", \"innerRadius\": "
				<< Detail.Particle.SpawnShape.fInnerRadius
				<< ", \"extents\": ";
			Write_Float3(Output, Detail.Particle.SpawnShape.vExtents);
			Output << ", \"arcDegrees\": "
				<< Detail.Particle.SpawnShape.fArcDegrees;
			if (EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM !=
				Detail.Particle.SpawnShape.eDistribution)
			{
				Output << ", \"distribution\": \""
					<< PARTICLE_SPAWN_DISTRIBUTION_TOKENS[static_cast<size_t>(
						Detail.Particle.SpawnShape.eDistribution)] << "\"";
			}
			Output << " }";
		}
		if (!Detail.Particle.InitialOrientation.Is_Default())
		{
			Output << ", \"initialOrientation\": { \"mode\": \""
				<< PARTICLE_ORIENTATION_MODE_TOKENS[static_cast<size_t>(
					Detail.Particle.InitialOrientation.eMode)]
				<< "\", \"offsetDegrees\": "
				<< Detail.Particle.InitialOrientation.fOffsetDegrees << " }";
		}
		if (EFFECT_PARTICLE_VELOCITY_MODE::FIXED !=
			Detail.Particle.InitialVelocity.eMode)
		{
			Output << ", \"initialVelocity\": { \"mode\": \""
				<< PARTICLE_VELOCITY_MODE_TOKENS[
					static_cast<size_t>(Detail.Particle.InitialVelocity.eMode)]
				<< "\", \"speed\": ";
			Write_Float2(Output, Detail.Particle.InitialVelocity.vSpeedRange);
			Output << ", \"coneAngleDegrees\": "
				<< Detail.Particle.InitialVelocity.fConeAngleDegrees;
			if (Detail.Particle.InitialVelocity.bUniformSolidAngle)
				Output << ", \"uniformSolidAngle\": true";
			Output << " }";
		}
		if (!Detail.Particle.TargetAttractor.Is_Default())
		{
			const EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
				Detail.Particle.TargetAttractor;
			Output << ", \"targetAttractor\": { \"enabled\": "
				<< (Attractor.bEnabled ? "true" : "false")
				<< ", \"targetSpace\": \""
				<< PARTICLE_ATTRACTOR_TARGET_SPACE_TOKENS[
					static_cast<size_t>(Attractor.eTargetSpace)]
				<< "\", \"targetOffset\": ";
			Write_Float3(Output, Attractor.vTargetOffset);
			Output << ", \"activeNormalized\": ";
			Write_Float2(Output, Attractor.vActiveNormalized);
			Output << ", \"radialAcceleration\": "
				<< Attractor.fRadialAcceleration
				<< ", \"tangentialAcceleration\": "
				<< Attractor.fTangentialAcceleration
				<< ", \"maximumSpeed\": " << Attractor.fMaximumSpeed
				<< ", \"convergenceRadius\": "
				<< Attractor.fConvergenceRadius
				<< ", \"arrivalDamping\": "
				<< Attractor.fArrivalDamping << " }";
		}
		/* Untouched trim is the overwhelming majority, and omitting it keeps
		   every document that predates the field byte-identical. */
		if (!Detail.Particle.SourceScale.Is_Default())
		{
			Output << ", \"sourceScale\": { \"count\": "
				<< Detail.Particle.SourceScale.fCount
				<< ", \"size\": " << Detail.Particle.SourceScale.fSize
				<< ", \"lifeTime\": " << Detail.Particle.SourceScale.fLifeTime
				<< ", \"speed\": " << Detail.Particle.SourceScale.fSpeed
				<< ", \"rotation\": " << Detail.Particle.SourceScale.fRotation
				<< ", \"alpha\": " << Detail.Particle.SourceScale.fAlpha
				<< ", \"spawnDelay\": "
				<< Detail.Particle.SourceScale.fSpawnDelay
				<< " }";
		}
		Output << " },\n"
			<< "        \"trail\": { \"maxPoints\": " << Detail.Trail.iMaxPoints
			<< ", \"pointLifeTimeSeconds\": " << Detail.Trail.fPointLifeTimeSeconds
			<< ", \"sampleIntervalSeconds\": " << Detail.Trail.fSampleIntervalSeconds
			<< ", \"minimumDistance\": " << Detail.Trail.fMinimumDistance
			<< ", \"startWidth\": " << Detail.Trail.fStartWidth
			<< ", \"endWidth\": " << Detail.Trail.fEndWidth
			<< ", \"tilingDistanceWorldUnits\": "
			<< Detail.Trail.fTilingDistanceWorldUnits
			<< ", \"distanceTessellationStepWorldUnits\": "
			<< Detail.Trail.fDistanceTessellationStepWorldUnits
			<< ", \"faceCamera\": " << (Detail.Trail.bFaceCamera ? "true" : "false") << " },\n"
			<< "        \"afterImage\": { \"sampleIntervalSeconds\": " << Detail.AfterImage.fSampleIntervalSeconds
			<< ", \"maxCopies\": " << Detail.AfterImage.iMaxCopies
			<< ", \"alphaExponent\": " << Detail.AfterImage.fAlphaExponent
			<< " },\n";
		Write_PresentationDetail(Output, Detail);
		Output << "      }";
	}

	bool_t Validate_ExecutableSourceMaterialCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element,
		bool_t& bOutOwnsDrawableContract,
		std::string& strOutError)
	{
		using namespace Client;
		bOutOwnsDrawableContract = false;
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
		const bool_t bSourceMaterialCarrier = SourceMaterial.bEnabled ||
			Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID;
		if (!bSourceMaterialCarrier)
		{
			if (Element.Material.Execution.bAuthoringApproximate)
			{
				strOutError =
					"Authoring-approximate carrier requires an enabled source Material profile.";
				return false;
			}
			return true;
		}

		if (!SourceMaterial.bEnabled ||
			SourceMaterial.eStatus >= EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED ||
			!Is_SupportedEffectSourceRuntimeShaderProfile(
				SourceMaterial.strRuntimeShaderProfileId))
		{
			strOutError =
				"Active source Material carrier has no ready source profile.";
			return false;
		}
		if (SourceMaterial.strRuntimeShaderProfileId ==
			"effect.ue3.fallback-blocked.v1")
		{
			strOutError =
				"Fallback-blocked source Material profile cannot be activated.";
			return false;
		}
		if (Element.Material.Execution.bAuthoringApproximate)
		{
			const bool_t bHasTextureCarrier = std::any_of(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId != EFFECT_MESH_SHAPE_SLOT_ID;
				});
			const bool_t bRequiresMesh =
				Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
				Element.SourceRecipe.strRendererShape == "mesh";
			const bool_t bHasMeshCarrier = std::any_of(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
			if (!bHasTextureCarrier || (bRequiresMesh && !bHasMeshCarrier))
			{
				strOutError =
					"Authoring-approximate carrier lacks its exact texture or mesh resource.";
				return false;
			}
		}

		const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		if (nullptr == pTemplate)
		{
			strOutError = "Active source Material Template is not registered.";
			return false;
		}
		const auto FindBinding = [&](const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
			-> const EFFECT_RESOURCE_BINDING_DESC*
		{
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				Find_EffectMaterialInput(*pTemplate, eSemantic);
			if (nullptr == pInput)
				return nullptr;
			const auto Iterator = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[pInput](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == pInput->strSlotId;
				});
			return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
		};
		const EFFECT_RESOURCE_BINDING_DESC* pBaseBinding = FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		const bool_t bSafeBase = nullptr != pBaseBinding &&
			!Is_UnsafeEffectBaseTextureAssetId(pBaseBinding->strAssetId);
		const bool_t bHasNoise = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::NOISE);
		const bool_t bHasMask = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::MASK);
		const bool_t bHasEmissive = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE);
		const bool_t bHasDissolve = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE);
		const bool_t bParticleMesh =
			EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const std::string_view strProfile =
			SourceMaterial.strRuntimeShaderProfileId;
		if (strProfile == "effect.ue3.grouped-translucent.v1" &&
			!Is_EffectGroupedTranslucentResourceContractSatisfied(
				SourceMaterial, bSafeBase, bHasMask, bHasEmissive, bHasDissolve))
		{
			strOutError =
				"Grouped-translucent source Material resource contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.linearflow-02.v1" &&
			!Has_EffectLinearFlowNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Linear-flow source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.blackline-aura.v1" &&
			!Has_EffectBlacklineNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Blackline source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID &&
			!Has_EffectWaterTrailNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Water-trail source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile ==
				EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID &&
			!Has_EffectMissileTrailNamedTextureContract(SourceMaterial))
		{
			strOutError =
				"Missile-trail source Material named texture contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.local-crack.v1")
		{
			const bool_t bLegacyContract =
				Is_EffectLegacyLocalCrackResourceContractSatisfied(
					SourceMaterial, bHasDissolve, bParticleMesh);
			const bool_t bNamedContract = !SourceMaterial.Textures.empty() &&
				Is_EffectLocalCrackResourceContractSatisfied(
					Has_EffectLocalCrackNamedTextureContract(SourceMaterial),
					true, true, bParticleMesh);
			if (!bLegacyContract && !bNamedContract)
			{
				strOutError =
					"Local-crack source Material resource contract is not satisfied.";
				return false;
			}
		}
		else if ((strProfile == "effect.ue3.shine.v1" ||
				strProfile == "effect.ue3.slice.v1" ||
				strProfile == EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID ||
				strProfile ==
					EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID ||
				strProfile == EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID ||
				strProfile == "effect.ue3.procedural-center-glow.v1") &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				strProfile, bSafeBase, bHasNoise, bHasMask, bHasEmissive,
				bHasDissolve,
				bParticleMesh))
		{
			strOutError =
				"Finite source Material profile resource contract is not satisfied.";
			return false;
		}
		if (strProfile == "effect.ue3.reconstructed-standard.v1" && !bSafeBase)
		{
			strOutError =
				"Reconstructed-standard source Material requires a safe Base texture.";
			return false;
		}

		bOutOwnsDrawableContract = true;
		return true;
	}

	bool_t Read_Warlord17090TypeDataMeshRotation(
		const Client::EFFECT_ELEMENT_DESC& Element,
		float3_t& vOutRotationDegrees,
		std::string& strOutError)
	{
		using namespace Client;
		const EFFECT_SOURCE_MODULE_DESC* pTypeDataMesh = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Normalize_SourceModuleClass(Module.strClassName) !=
				"particlemoduletypedatamesh")
			{
				continue;
			}
			if (nullptr != pTypeDataMesh)
			{
				strOutError =
					"Warlord 17090 Mesh source has duplicate TypeDataMesh modules.";
				return false;
			}
			pTypeDataMesh = &Module;
		}
		if (nullptr == pTypeDataMesh)
		{
			strOutError =
				"Warlord 17090 Mesh source is missing its TypeDataMesh module.";
			return false;
		}

		float3_t Projected{};
		std::array<bool_t, 3u> Seen{};
		for (const EFFECT_SOURCE_LITERAL_DESC& Literal :
			pTypeDataMesh->Literals)
		{
			std::string Property = Literal.strPropertyPath;
			std::transform(Property.begin(), Property.end(), Property.begin(),
				[](const unsigned char Character)
				{
					return static_cast<char_t>(std::tolower(Character));
				});
			size_t iComponent = 3u;
			if (Property == "roll")
				iComponent = 0u;
			else if (Property == "pitch")
				iComponent = 1u;
			else if (Property == "yaw")
				iComponent = 2u;
			if (iComponent == 3u)
				continue;
			if (Seen[iComponent])
			{
				strOutError =
					"Warlord 17090 TypeDataMesh rotation literal is duplicated.";
				return false;
			}
			if (Literal.eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
				!std::isfinite(Literal.fNumber) ||
				std::abs(Literal.fNumber) > 3600.0)
			{
				strOutError =
					"Warlord 17090 TypeDataMesh rotation literal is not a finite number.";
				return false;
			}
			Seen[iComponent] = true;
			const f32_t fValue = static_cast<f32_t>(Literal.fNumber);
			if (iComponent == 0u)
				Projected.x = fValue;
			else if (iComponent == 1u)
				Projected.y = fValue;
			else
				Projected.z = fValue;
		}
		vOutRotationDegrees = Projected;
		return true;
	}

	const Client::EFFECT_SOURCE_MODULE_DESC*
		Find_UniqueWarlordChainModule(
			const Client::EFFECT_ELEMENT_DESC& Element,
			const std::string_view strClassName,
			std::string& strOutError)
	{
		const Client::EFFECT_SOURCE_MODULE_DESC* pResult = nullptr;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Normalize_SourceModuleClass(Module.strClassName) != strClassName)
				continue;
			if (nullptr != pResult)
			{
				strOutError = "Warlord 17090 chain source has a duplicate " +
					std::string(strClassName) + " module.";
				return nullptr;
			}
			pResult = &Module;
		}
		if (nullptr == pResult)
		{
			strOutError = "Warlord 17090 chain source is missing its " +
				std::string(strClassName) + " module.";
		}
		return pResult;
	}

	const Client::EFFECT_DISTRIBUTION_DESC* Find_WarlordChainDistribution(
		const Client::EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath)
	{
		const auto Iterator = std::find_if(
			Module.Distributions.begin(), Module.Distributions.end(),
			[strPropertyPath](
				const Client::EFFECT_DISTRIBUTION_DESC& Distribution)
			{
				return Distribution.strPropertyPath == strPropertyPath;
			});
		return Iterator == Module.Distributions.end() ? nullptr : &*Iterator;
	}

	bool_t Validate_Warlord17090ChainSourceRecipe(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strMeshAssetId,
		std::string& strOutError)
	{
		using namespace Client;
		constexpr f32_t EPSILON = 1.e-5f;
		const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
		{
			return std::abs(Left - Right) <= EPSILON;
		};
		constexpr std::array<std::string_view, 8u> CHAIN_06_IDS =
		{
			"authored.source-particle.fe0d3291da32d9c797705c72",
			"authored.source-particle.1bb7a50a0c3ff729ca586851",
			"authored.source-particle.eedca4cf57247af0f82fba11",
			"authored.source-particle.cd8d5f0dbedb3e620880378d",
			"authored.source-particle.a3cbc6a5ccb3876669945ebd",
			"authored.source-particle.6ad86a3bed5164050280ee47",
			"authored.source-particle.3903aa4912d4eeeba9d7d951",
			"authored.source-particle.71f0b215e9cff2426ac0ce47"
		};
		constexpr std::array<std::string_view, 4u> CHAIN_07_IDS =
		{
			"authored.source-particle.a7fae01987d89775468563e4",
			"authored.source-particle.a2d9eabb94a5702c933cd727",
			"authored.source-particle.72835f216dd26bb28166301f",
			"authored.source-particle.78efab80dc7c76fc2b416cba"
		};
		const std::string_view OriginElementId =
			Resolve_EffectPortableOriginElementId(Element);
		const auto ContainsId = [OriginElementId](const auto& Ids)
		{
			return std::find(Ids.begin(), Ids.end(), OriginElementId) !=
				Ids.end();
		};
		if ((strMeshAssetId == WARLORD_CHAIN_06_MODEL_ASSET_ID &&
				!ContainsId(CHAIN_06_IDS)) ||
			(strMeshAssetId == WARLORD_CHAIN_07_MODEL_ASSET_ID &&
				!ContainsId(CHAIN_07_IDS)))
		{
			strOutError =
				"Warlord 17090 chain stable Element/Mesh identity is invalid.";
			return false;
		}
		if (Element.SourceRecipe.iEmitterLoopCount != 1u ||
			Element.SourceRecipe.Bursts.size() != 1u ||
			!NearlyEqual(Element.SourceRecipe.Bursts.front().fTimeSeconds, 0.f) ||
			Element.SourceRecipe.Bursts.front().iCountMinimum != 1u ||
			Element.SourceRecipe.Bursts.front().iCountMaximum != 1u ||
			!NearlyEqual(Element.Detail.Timing.fStartDelaySeconds, 0.3668f))
		{
			strOutError =
				"Warlord 17090 chain burst/timing contract is invalid.";
			return false;
		}

		const EFFECT_SOURCE_MODULE_DESC* pLocationDirect =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulelocationdirect", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pLifetime =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulelifetime", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pDynamic =
			Find_UniqueWarlordChainModule(Element,
				"particlemoduleparameterdynamic", strOutError);
		const EFFECT_SOURCE_MODULE_DESC* pMeshRotation =
			Find_UniqueWarlordChainModule(Element,
				"particlemodulemeshrotation", strOutError);
		if (nullptr == pLocationDirect || nullptr == pLifetime ||
			nullptr == pDynamic || nullptr == pMeshRotation)
		{
			return false;
		}

		const EFFECT_DISTRIBUTION_DESC* pLocation =
			Find_WarlordChainDistribution(*pLocationDirect, "location");
		const EFFECT_DISTRIBUTION_DESC* pDirection =
			Find_WarlordChainDistribution(*pLocationDirect, "direction");
		const EFFECT_DISTRIBUTION_DESC* pLocationOffset =
			Find_WarlordChainDistribution(*pLocationDirect, "locationoffset");
		const EFFECT_DISTRIBUTION_DESC* pScaleFactor =
			Find_WarlordChainDistribution(*pLocationDirect, "scalefactor");
		const auto IsExactZeroVectorDefault = [](const auto* pDistribution)
		{
			return nullptr != pDistribution &&
				pDistribution->strSourceClass.empty() &&
				pDistribution->strSourceObjectPath.empty() &&
				pDistribution->iComponentCount == 3u &&
				pDistribution->iOperation == 1u &&
				pDistribution->iRandomLockAxes == 0u &&
				pDistribution->iLookupTableChunkSize == 0u &&
				pDistribution->iLookupTableNumElements == 0u &&
				pDistribution->fLookupTableTimeScale == 0.f &&
				pDistribution->fLookupTableStartTime == 0.f &&
				pDistribution->vDefaultMinimum.x == 0.f &&
				pDistribution->vDefaultMinimum.y == 0.f &&
				pDistribution->vDefaultMinimum.z == 0.f &&
				pDistribution->vDefaultMinimum.w == 0.f &&
				pDistribution->vDefaultMaximum.x == 0.f &&
				pDistribution->vDefaultMaximum.y == 0.f &&
				pDistribution->vDefaultMaximum.z == 0.f &&
				pDistribution->vDefaultMaximum.w == 0.f &&
				pDistribution->LookupTable.empty() &&
				pDistribution->Keys.empty();
		};
		constexpr std::array<f32_t, 8u> LOCATION_TABLE =
			{ -30.f, 35.f, 0.f, 35.f, 0.f, -20.f, -30.f, 0.f };
		if (!IsExactZeroVectorDefault(pDirection) ||
			!IsExactZeroVectorDefault(pLocationOffset) ||
			!IsExactZeroVectorDefault(pScaleFactor) ||
			nullptr == pLocation || pLocation->iComponentCount != 3u ||
			pLocation->iOperation != 1u ||
			pLocation->iLookupTableChunkSize != 3u ||
			pLocation->iLookupTableNumElements != 1u ||
			!NearlyEqual(pLocation->fLookupTableStartTime, 0.9f) ||
			!NearlyEqual(pLocation->fLookupTableTimeScale, 10.f) ||
			pLocation->LookupTable.size() != LOCATION_TABLE.size() ||
			!std::equal(pLocation->LookupTable.begin(),
				pLocation->LookupTable.end(), LOCATION_TABLE.begin(),
				[&NearlyEqual](const f32_t Left, const f32_t Right)
				{
					return NearlyEqual(Left, Right);
				}))
		{
			strOutError =
				"Warlord 17090 LocationDirect launch/return/default identity is invalid.";
			return false;
		}

		const EFFECT_DISTRIBUTION_DESC* pLife =
			Find_WarlordChainDistribution(*pLifetime, "lifetime");
		if (nullptr == pLife || pLife->LookupTable.size() != 4u ||
			!NearlyEqual(pLife->LookupTable[2], 0.6f) ||
			!NearlyEqual(pLife->LookupTable[3], 0.6f))
		{
			strOutError = "Warlord 17090 chain lifetime is not exact 0.6s.";
			return false;
		}

		constexpr std::array<std::string_view, 4u> DYNAMIC_PARAMETER_NAMES =
		{
			"worldpositionoffset_str", "worldposition_uvscale",
			"x.pan", "worldposition_zoffset"
		};
		for (size_t i = 0u; i < DYNAMIC_PARAMETER_NAMES.size(); ++i)
		{
			const std::string Property = "dynamicparams[" +
				std::to_string(i) + "].paramname";
			const auto Literal = std::find_if(
				pDynamic->Literals.begin(), pDynamic->Literals.end(),
				[&Property](const EFFECT_SOURCE_LITERAL_DESC& Candidate)
				{
					return Candidate.strPropertyPath == Property;
				});
			if (Literal == pDynamic->Literals.end() ||
				Literal->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING ||
				Literal->strString != DYNAMIC_PARAMETER_NAMES[i])
			{
				strOutError =
					"Warlord 17090 chain WPO DynamicParameter identity is invalid.";
				return false;
			}
		}
		return true;
	}

	bool_t Apply_Warlord17090RetainedSourceProjection(
		Client::EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError)
	{
		using namespace Client;
		if (InOutDocument.strEffectAssetId != WARLORD_17090_EFFECT_ASSET_ID)
			return true;

		size_t iChainCount = 0u;
		size_t iChain06Count = 0u;
		size_t iChain07Count = 0u;
		const auto ResolveCompilerAssetId = [](
			const EFFECT_ELEMENT_DESC& Element,
			const EFFECT_RESOURCE_BINDING_DESC& Binding,
			std::string_view& strOutCompilerAssetId) -> bool_t
		{
			const auto First = std::find_if(
				Element.AuthoringOverrides.ResourceBindings.begin(),
				Element.AuthoringOverrides.ResourceBindings.end(),
				[&Binding](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{
					return Override.strSlotId == Binding.strSlotId;
				});
			if (First == Element.AuthoringOverrides.ResourceBindings.end())
			{
				strOutCompilerAssetId = Binding.strAssetId;
				return true;
			}
			if (std::find_if(std::next(First),
					Element.AuthoringOverrides.ResourceBindings.end(),
					[&Binding](
						const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
					{
						return Override.strSlotId == Binding.strSlotId;
					}) != Element.AuthoringOverrides.ResourceBindings.end() ||
				First->strAssetId != Binding.strAssetId ||
				First->strCompilerAssetId.empty())
			{
				return false;
			}
			strOutCompilerAssetId = First->strCompilerAssetId;
			return true;
		};
		for (EFFECT_ELEMENT_DESC& Element : InOutDocument.Elements)
		{
			if (!Element.SourceRecipe.bEnabled ||
				Element.SourceRecipe.strRendererShape != "mesh")
			{
				continue;
			}
			float3_t SourceTypeDataRotation{};
			if (!Read_Warlord17090TypeDataMeshRotation(
				Element, SourceTypeDataRotation, strOutError))
			{
				return false;
			}
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees =
				SourceTypeDataRotation;

			if (Element.Material.strSourceMaterialPath !=
				WARLORD_CHAIN_SOURCE_MATERIAL_PATH)
			{
				continue;
			}
			++iChainCount;
			if (Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE)
			{
				strOutError =
					"Warlord 17090 chain source is not a Particle Mesh.";
				return false;
			}

			size_t iMeshBindingCount = 0u;
			size_t iPreviewBaseCount = 0u;
			std::string_view strMeshAssetId;
			for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
				Element.ResourceBindings)
			{
				if (Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID)
				{
					++iMeshBindingCount;
					if (!ResolveCompilerAssetId(
							Element, Binding, strMeshAssetId))
					{
						strOutError =
							"Warlord 17090 chain Mesh override baseline/effective identity is invalid.";
						return false;
					}
					if (strMeshAssetId == WARLORD_CHAIN_06_MODEL_ASSET_ID)
						++iChain06Count;
					else if (strMeshAssetId ==
						WARLORD_CHAIN_07_MODEL_ASSET_ID)
					{
						++iChain07Count;
					}
					else
					{
						strOutError =
							"Warlord 17090 chain Mesh identity is invalid.";
						return false;
					}
				}
				else if (Binding.strSlotId == "base")
				{
					std::string_view strCompilerBaseAssetId;
					if (!ResolveCompilerAssetId(Element, Binding,
							strCompilerBaseAssetId) ||
						strCompilerBaseAssetId !=
							WARLORD_CHAIN_BASE_ALIAS_ASSET_ID)
					{
						strOutError =
							"Warlord 17090 non-exact preview Base override baseline/effective identity is invalid.";
						return false;
					}
					++iPreviewBaseCount;
				}
				else
				{
					strOutError =
						"Warlord 17090 chain carries an unproven Material resource.";
					return false;
				}
			}
			if (iMeshBindingCount != 1u || iPreviewBaseCount != 1u)
			{
				strOutError =
					"Warlord 17090 chain Mesh/non-exact preview Base cardinality is invalid.";
				return false;
			}
			const EFFECT_SOURCE_MATERIAL_DESC& SourceProfile =
				Element.Material.SourceMaterial;
			if (Element.Material.strTemplateId !=
					EFFECT_SOURCE_MATERIAL_TEMPLATE_ID ||
				!SourceProfile.bEnabled ||
				SourceProfile.strProfileId != WARLORD_CHAIN_SOURCE_PROFILE_ID ||
				SourceProfile.strParentMaterialPath !=
					WARLORD_CHAIN_PARENT_MATERIAL_PATH ||
				SourceProfile.strRuntimeShaderProfileId !=
					WARLORD_CHAIN_RUNTIME_PROFILE_ID)
			{
				strOutError =
					"Warlord 17090 non-exact preview source profile is invalid.";
				return false;
			}
			if (!Validate_Warlord17090ChainSourceRecipe(
				Element, strMeshAssetId, strOutError))
			{
				return false;
			}
			Element.Material.Execution = {};
			Element.Material.Execution.bFailClosed = true;
			Element.Material.Execution.bAuthoringApproximate = true;
			/* atypical_028 is a same-group authoring baseline, not an exact parent
			   Base.  Keep the enabled grouped profile and any valid G3 override so
			   the Tool can tune/reset a physical DDS.  The disabled approximate
			   execution records SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE and must
			   never promote this carrier to Full merely because it draws. */
			Element.Detail.Mesh.bUseModelMaterial = false;
		}
		/* Mutable authored documents are legal source-backed subsets.  Exact
		   source cardinality belongs to the immutable Track-A evidence gate,
		   never to ordinary Tool Load/Save.  Every retained chain row has already
		   passed the stable-ID/model/material/recipe checks above; these upper
		   bounds only prevent a future codec caller from exceeding that evidence
		   denominator. */
		if (iChainCount != iChain06Count + iChain07Count ||
			iChain06Count > 8u || iChain07Count > 4u)
		{
			strOutError =
				"Warlord 17090 retained Chain subset exceeds the source evidence denominator.";
			return false;
		}
		return true;
	}
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_ELEMENT_KIND eKind)
{
	return eKind < EFFECT_ELEMENT_KIND::END ?
		KIND_TOKENS[static_cast<size_t>(eKind)] : "invalid";
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	return eSlot < EFFECT_RESOURCE_SLOT::END ?
		SLOT_TOKENS[static_cast<size_t>(eSlot)] : "invalid";
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_RENDER_PROFILE eProfile)
{
	return eProfile < EFFECT_RENDER_PROFILE::END ?
		PROFILE_TOKENS[static_cast<size_t>(eProfile)] : "invalid";
}

const char_t* Client::CEffectDocumentCodec::To_Token(
	const EFFECT_MATERIAL_EXECUTION_BACKEND eBackend)
{
	return eBackend < EFFECT_MATERIAL_EXECUTION_BACKEND::END ?
		MATERIAL_EXECUTION_BACKEND_TOKENS[static_cast<size_t>(eBackend)] :
		"invalid";
}

bool_t Client::CEffectDocumentCodec::Is_ResourceSlotAllowed(
	const EFFECT_ELEMENT_KIND eKind,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (eKind >= EFFECT_ELEMENT_KIND::END || eSlot >= EFFECT_RESOURCE_SLOT::END)
		return false;
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_ELEMENT_KIND::MESH == eKind ||
			EFFECT_ELEMENT_KIND::PARTICLE == eKind;
	return true;
}

bool_t Client::CEffectDocumentCodec::Is_SafeResourceAssetId(
	const std::string& strAssetId,
	EFFECT_RESOURCE_FILE_KIND* pOutKind)
{
	if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
		0u != strAssetId.rfind("Effect/", 0u) ||
		std::string::npos != strAssetId.find('\\') ||
		std::string::npos != strAssetId.find(':'))
	{
		return false;
	}

	const std::filesystem::path RelativePath(strAssetId);
	if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
		RelativePath.lexically_normal().generic_string() != strAssetId)
	{
		return false;
	}
	for (const std::filesystem::path& Component : RelativePath)
	{
		const std::string Value = Component.generic_string();
		if (Value.empty() || Value == "." || Value == "..")
			return false;
	}

	std::string Extension = RelativePath.extension().string();
	std::transform(Extension.begin(), Extension.end(), Extension.begin(),
		[](const char_t Character)
		{
			return static_cast<char_t>(std::tolower(
				static_cast<unsigned char>(Character)));
		});
	EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
	if (Extension == ".wmodel")
		eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
	else if (Extension == ".dds")
		eKind = EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	else
		return false;
	const std::filesystem::path Resolved =
		CRuntimeAssetRoot::Resolve(RelativePath);
	std::error_code Error;
	if (Resolved.empty() ||
		!std::filesystem::is_regular_file(Resolved, Error) || Error)
	{
		return false;
	}
	if (nullptr != pOutKind)
		*pOutKind = eKind;
	return true;
}

bool_t Client::CEffectDocumentCodec::Is_SafeElementResourceAssetId(
	const EFFECT_ELEMENT_KIND eElementKind,
	const std::string_view strSlotId,
	const std::string& strAssetId,
	EFFECT_RESOURCE_FILE_KIND* pOutKind)
{
	EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
	if (strSlotId == EFFECT_MESH_SHAPE_SLOT_ID)
	{
		if (!Is_ResourceSlotAllowed(
				eElementKind, EFFECT_RESOURCE_SLOT::MESH_MODEL))
		{
			return false;
		}
		if (0u == strAssetId.rfind("Character/", 0u))
		{
			if (!Is_SafeModelCueAssetIdInternal(strAssetId))
				return false;
			eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
		}
		else if (!Is_SafeResourceAssetId(strAssetId, &eKind) ||
			eKind != EFFECT_RESOURCE_FILE_KIND::MODEL)
		{
			return false;
		}
	}
	else if (!Is_SafeResourceAssetId(strAssetId, &eKind) ||
		eKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
	{
		return false;
	}

	if (nullptr != pOutKind)
		*pOutKind = eKind;
	return true;
}

bool_t Client::CEffectDocumentCodec::Is_SafeModelCueAssetId(
	const std::string& strAssetId)
{
	return Is_SafeModelCueAssetIdInternal(strAssetId);
}

bool_t Client::CEffectDocumentCodec::Validate(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
		Document.iLoadedFormatVersion > EFFECT_AUTHORING_MAX_SUPPORTED_VERSION)
	{
		strOutError = "Unsupported Effect document version.";
		return false;
	}
	if (Document.bSourceContract ||
		Document.iLoadedFormatVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION)
	{
		strOutError =
			"Native-v14 source contracts are not runtime Effect documents.";
		return false;
	}
	if (!Validate_AuthoredRuntimeExtensions(Document, strOutError))
		return false;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence =
			Recipe.CompilerEvidence;
		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			Recipe.GeometryBinding;
		const bool_t bCompilerEvidencePresent =
			!Evidence.strArtifactFileSha256.empty() ||
			!Evidence.strArtifactSelfSha256.empty() ||
			!Evidence.strEvidenceId.empty() ||
			!Evidence.strSourceEvidenceStatus.empty() ||
			!Evidence.strSourceCueId.empty() ||
			!Evidence.strSourceOccurrenceId.empty() ||
			!Evidence.strSourceSystemId.empty() ||
			!Evidence.strSourceEmitterPath.empty() ||
			!Evidence.strSourceEmitterNodeId.empty() ||
			!Evidence.strLodSelectionPolicy.empty() ||
			!Evidence.strSelectedLodPath.empty() ||
			!Evidence.strSelectedLodNodeId.empty() ||
			0u != Evidence.iSelectedLodArrayIndex ||
			!Evidence.strSelectedLodLevelProvenance.empty() ||
			!Evidence.strSelectedLodEnabledProvenance.empty() ||
			0u != Evidence.iNonSelectedLodCount ||
			!Evidence.ModuleReferenceOrder.empty() ||
			Evidence.vCueSourcePositionUeUnits.x != 0.f ||
			Evidence.vCueSourcePositionUeUnits.y != 0.f ||
			Evidence.vCueSourcePositionUeUnits.z != 0.f ||
			Evidence.CueLocalTransform.vPosition.x != 0.f ||
			Evidence.CueLocalTransform.vPosition.y != 0.f ||
			Evidence.CueLocalTransform.vPosition.z != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.x != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.y != 0.f ||
			Evidence.CueLocalTransform.vRotationDegrees.z != 0.f ||
			Evidence.CueLocalTransform.vScale.x != 1.f ||
			Evidence.CueLocalTransform.vScale.y != 1.f ||
			Evidence.CueLocalTransform.vScale.z != 1.f ||
			!Evidence.ParameterOverrides.empty() ||
			!Evidence.CompositionOrder.empty() ||
			!Evidence.strLocalReferenceClosureFileSha256.empty() ||
			!Evidence.strLocalReferenceClosureSelfSha256.empty() ||
			!Evidence.strGeometryParityFileSha256.empty() ||
			!Evidence.strGeometryParitySelfSha256.empty();
		const bool_t bDistributionEvidencePresent = std::any_of(
			Recipe.Modules.begin(), Recipe.Modules.end(),
			[](const EFFECT_SOURCE_MODULE_DESC& Module)
			{
				return std::any_of(Module.Distributions.begin(),
					Module.Distributions.end(),
					[](const EFFECT_DISTRIBUTION_DESC& Distribution)
					{
						return !Distribution.strReferenceId.empty() ||
							!Distribution.strOccurrenceId.empty() ||
							!Distribution.strPayloadStatus.empty() ||
							!Distribution.strFidelity.empty() ||
							!Distribution.strParameterName.empty() ||
							Distribution.eParameterBinding !=
								EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
							Distribution.ExecutionAdmission.bAllowed ||
							!Distribution.ExecutionAdmission.Blockers.empty();
					});
			});
		if (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			!Recipe.strSourceContractProfileId.empty() ||
			!Recipe.strSourceContractSha256.empty() ||
			!Recipe.strSourceGraphSha256.empty() ||
			!Recipe.strSourceClosureSha256.empty() ||
			!Recipe.strSourceMaterialClosureSha256.empty() ||
			0u != Recipe.iSourcePeakActiveParticles ||
			!Recipe.LocalReferenceBindings.empty() ||
			!Recipe.ModuleCoverage.empty() || bCompilerEvidencePresent ||
			bDistributionEvidencePresent ||
			Recipe.CompiledExecutionAdmission.bAllowed ||
			!Recipe.CompiledExecutionAdmission.Blockers.empty() ||
			!Recipe.MaterialAdmission.strStatus.empty() ||
			!Recipe.MaterialAdmission.SourceMaterialPaths.empty() ||
			!Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
			!Recipe.MaterialAdmission.strRenderStateRecipeId.empty() ||
			!Recipe.MaterialAdmission.Blockers.empty() ||
			Geometry.bEnabled || !Geometry.strAssetId.empty() ||
			!Geometry.strReceiptFileSha256.empty() ||
			!Geometry.strReceiptSelfSha256.empty() ||
			Geometry.fCarrierGeometryPreScale != 1.f ||
			!Geometry.strParticleScaleSemantics.empty() ||
			!Geometry.strStatus.empty() || !Geometry.Blockers.empty())
		{
			strOutError =
				"Legacy Effect documents cannot carry native-v14 source-contract fields.";
			return false;
		}
	}
	if (!Is_StableId(Document.strEffectAssetId))
	{
		strOutError = "Effect Asset ID is invalid.";
		return false;
	}
	if (Document.strDisplayName.size() > 64u ||
		!Has_VisibleCharacter(Document.strDisplayName))
	{
		strOutError = "Display Name must be 1-64 bytes and not blank.";
		return false;
	}
	const EFFECT_PARTICLE_SYSTEM_DESC& ParticleSystem =
		Document.ParticleSystem;
	if (!std::isfinite(ParticleSystem.fUniformScaleMultiplier) ||
		ParticleSystem.fUniformScaleMultiplier <= 0.f ||
		ParticleSystem.fUniformScaleMultiplier > 100.f ||
		!std::isfinite(ParticleSystem.fYawOffsetDegrees) ||
		std::abs(ParticleSystem.fYawOffsetDegrees) > 3600.f ||
		!std::isfinite(ParticleSystem.fDirectionYawDegrees) ||
		std::abs(ParticleSystem.fDirectionYawDegrees) > 3600.f ||
		!std::isfinite(ParticleSystem.fInitialSpeedMultiplier) ||
		ParticleSystem.fInitialSpeedMultiplier < 0.f ||
		ParticleSystem.fInitialSpeedMultiplier > 100.f)
	{
		strOutError = "Particle System modifier contains an invalid number or range.";
		return false;
	}
	if (Document.Elements.size() > MAX_ELEMENTS)
	{
		strOutError = "Effect Element count exceeds 2048.";
		return false;
	}
	if (Document.ModelCues.size() > MAX_MODEL_CUES)
	{
		strOutError = "Effect Model Cue count exceeds 8.";
		return false;
	}
	std::unordered_set<std::string> ModelCueIds;
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		const bool_t bTransformValid =
			Is_Finite(Cue.LocalTransform.vPosition) &&
			Is_Finite(Cue.LocalTransform.vRotationDegrees) &&
			Is_Finite(Cue.LocalTransform.vScale) &&
			Cue.LocalTransform.vScale.x > 0.f &&
			Cue.LocalTransform.vScale.y > 0.f &&
			Cue.LocalTransform.vScale.z > 0.f &&
			Is_Finite(Cue.vAssetPreScale) &&
			Cue.vAssetPreScale.x > 0.f && Cue.vAssetPreScale.y > 0.f &&
			Cue.vAssetPreScale.z > 0.f &&
			Is_Finite(Cue.vAssetPreRotationDegrees) &&
			Is_Finite(Cue.LocalTransform.vRevolutionDegreesPerSecond) &&
			Is_Finite(Cue.LocalTransform.vVelocityPerSecond) &&
			Is_Finite(Cue.vColorMultiply) &&
			std::isfinite(Cue.fOpacity) && Cue.fOpacity >= 0.f &&
			Cue.fOpacity <= 1.f;
		if (!Is_StableId(Cue.strCueId) ||
			!ModelCueIds.insert(Cue.strCueId).second ||
			Cue.strClipName.empty() || Cue.strClipName.size() > 128u ||
			!Has_VisibleCharacter(Cue.strClipName) ||
			!Is_SafeModelCueAssetId(Cue.strModelAssetId) ||
			!std::isfinite(Cue.fStartDelaySeconds) ||
			Cue.fStartDelaySeconds < 0.f ||
			!std::isfinite(Cue.fDurationSeconds) ||
			Cue.fDurationSeconds <= 0.f || Cue.fDurationSeconds > 30.f ||
			Cue.eAlphaMode >= EFFECT_MODEL_CUE_ALPHA_MODE::END ||
			!bTransformValid)
		{
			strOutError =
				"Effect Model Cue identity, resource, time, or transform is invalid.";
			return false;
		}
	}

	std::unordered_set<std::string> ElementIds;
	std::unordered_map<std::string, const EFFECT_ELEMENT_DESC*> ElementsById;
	uint64_t iTotalParticles = 0u;
	uint64_t iTotalTrailPoints = 0u;
	uint64_t iTotalAfterImages = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.strDisplayName.size() > 64u ||
			!Has_VisibleCharacter(Element.strDisplayName))
		{
			strOutError = "Element '" + Element.strElementId +
				"' display name must be 1-64 UTF-8 bytes and not blank (got " +
				std::to_string(Element.strDisplayName.size()) + " bytes).";
			return false;
		}
		if (!Is_StableId(Element.strElementId) ||
			(!Element.strGroupId.empty() && !Is_StableId(Element.strGroupId)) ||
			Element.strSourceNode.size() > 256u ||
			Element.eKind >= EFFECT_ELEMENT_KIND::END ||
			Element.Material.eRenderProfile >= EFFECT_RENDER_PROFILE::END ||
			!ElementIds.insert(Element.strElementId).second)
		{
			strOutError = "Element metadata, kind, profile, or duplicate is invalid.";
			return false;
		}
		ElementsById.emplace(Element.strElementId, &Element);
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		const bool_t bAttachmentTransformValid =
			std::isfinite(Attachment.fSnapshotRootSourceBasisYawDegrees) &&
			std::abs(Attachment.fSnapshotRootSourceBasisYawDegrees) <= 3600.f &&
			Is_Finite(Attachment.SocketLocalTransform.vPosition) &&
			Is_Finite(Attachment.SocketLocalTransform.vRotationDegrees) &&
			Is_Finite(Attachment.SocketLocalTransform.vScale) &&
			Attachment.SocketLocalTransform.vScale.x > 0.f &&
			Attachment.SocketLocalTransform.vScale.y > 0.f &&
			Attachment.SocketLocalTransform.vScale.z > 0.f;
		const bool_t bOwnerYawAttachment = Attachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW;
		if (!bAttachmentTransformValid ||
			Attachment.eOrientation >= EFFECT_ATTACHMENT_ORIENTATION::END ||
			(bOwnerYawAttachment &&
				(!Attachment.bEnabled || !Attachment.bFollow ||
				 Document.bSourceContract || Element.SourceRecipe.bEnabled ||
				 !Element.RuntimeCarrier.Is_Empty())) ||
			((!Attachment.bEnabled || Attachment.bFollow) &&
				0.f != Attachment.fSnapshotRootSourceBasisYawDegrees) ||
			(Attachment.bEnabled &&
				(Attachment.strSourceAnchorSlotId.empty() ||
					Attachment.strSourceAnchorSlotId.size() > 128u ||
					!Has_VisibleCharacter(
						Attachment.strSourceAnchorSlotId) ||
					Attachment.strRuntimeAnchorSlotId.empty() ||
					Attachment.strRuntimeAnchorSlotId.size() > 128u ||
					!Has_VisibleCharacter(
						Attachment.strRuntimeAnchorSlotId) ||
					(Attachment.bFollow &&
						(Attachment.strRuntimeBoneName.empty() ||
							Attachment.strRuntimeBoneName.size() > 128u ||
							!Has_VisibleCharacter(
								Attachment.strRuntimeBoneName))))))
		{
			strOutError = "Effect Action cue attachment contract is invalid.";
			return false;
		}
		const EFFECT_MATERIAL_TEMPLATE_DESC* pMaterialTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		if (nullptr == pMaterialTemplate)
		{
			strOutError = "Effect Material Template is not registered: " +
				Element.Material.strTemplateId;
			return false;
		}
		if (Element.Material.bColorTexturesSRGB &&
			(Element.Material.strTemplateId != EFFECT_STANDARD_MATERIAL_TEMPLATE_ID ||
			 Element.Material.SourceMaterial.bEnabled ||
			 Element.Material.Execution.bEnabled ||
			 Element.Material.Execution.bFailClosed || Element.SourceRecipe.bEnabled))
		{
			strOutError =
				"colorTexturesSRGB requires an ordinary authored standard material: " +
				Element.strElementId;
			return false;
		}
		if (Element.Material.strSourceMaterialPath.size() > 512u ||
			(Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
				(Element.Material.strSourceMaterialPath.empty() ||
					!Has_VisibleCharacter(
						Element.Material.strSourceMaterialPath))))
		{
			strOutError = "Effect source Material identity is invalid.";
			return false;
		}
		if (!Validate_MaterialExecution(Element.Material.Execution,
			strOutError))
		{
			strOutError += " Element: " + Element.strElementId + ".";
			return false;
		}
		const bool_t bStandardColorBackend =
			Element.Material.Execution.bEnabled &&
			Element.Material.Execution.eBackend ==
				EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		const bool_t bStandardColorTemplate =
			Element.Material.strTemplateId == EFFECT_STANDARD_COLOR_V1_TEMPLATE_ID;
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
		if (bStandardColorBackend != bStandardColorTemplate ||
			(bStandardColorBackend &&
			 (Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
			  Element.eKind != EFFECT_ELEMENT_KIND::DECAL &&
			  Element.eKind != EFFECT_ELEMENT_KIND::TRAIL)) ||
			(bStandardColorBackend &&
			 (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			  Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END)) ||
			(bStandardColorBackend && !bStandardColorResourceContract) ||
			(bStandardColorBackend && Element.Material.SourceMaterial.bEnabled) ||
			(bStandardColorBackend &&
			 Element.Material.eRenderProfile ==
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE) ||
			(bStandardColorBackend &&
			 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			 (!Element.SourceRecipe.bEnabled ||
			  (Element.SourceRecipe.strRendererShape != "sprite" &&
			   Element.SourceRecipe.strRendererShape != "mesh"))) ||
			(bStandardColorBackend &&
			 (0.f != Element.Detail.Color.fDistortionIntensity ||
			  Element.Detail.Color.bDistortionOnBaseMaterial ||
			  0.f != Element.Detail.Color.fRadialTime ||
			  0.f != Element.Detail.Color.fRadialIntensity)))
		{
			strOutError =
				"StandardColorV1 template, carrier, or generic-only state is invalid: " +
				Element.strElementId + ".";
			return false;
		}
		const bool_t bAuthoringExecutionTarget =
			Is_EffectAuthoringExecutionTarget(Element.Material.Execution);
		const bool_t bGeometryOnlySourceCarrier =
			Is_EffectFailClosedSourceGeometryCarrier(Element);
		if (Element.bVisible && !bAuthoringExecutionTarget &&
			!bGeometryOnlySourceCarrier)
		{
			strOutError =
				"Hard fail-closed authored Element is not a typed source geometry carrier: " +
				Element.strElementId + ".";
			return false;
		}
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
		if (SourceMaterial.eSourceBlendClass >=
			EFFECT_SOURCE_BLEND_CLASS::END ||
			(!SourceMaterial.bEnabled &&
			 SourceMaterial.eSourceBlendClass !=
				EFFECT_SOURCE_BLEND_CLASS::UNKNOWN))
		{
			strOutError =
				"Effect source Material blend evidence is invalid.";
			return false;
		}
		if (SourceMaterial.bEnabled)
		{
			if (!Is_StableId(SourceMaterial.strProfileId) ||
				!Is_StableId(SourceMaterial.strRuntimeShaderProfileId) ||
				!Is_SupportedEffectSourceRuntimeShaderProfile(
					SourceMaterial.strRuntimeShaderProfileId) ||
				SourceMaterial.strParentMaterialPath.empty() ||
				SourceMaterial.strParentMaterialPath.size() > 512u ||
				!Has_VisibleCharacter(
					SourceMaterial.strParentMaterialPath) ||
				SourceMaterial.eStatus >=
					EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED ||
				!Is_StableId(SourceMaterial.strSubUVMode) ||
				!Is_SupportedEffectSourceSubUVMode(
					SourceMaterial.strSubUVMode) ||
				SourceMaterial.Textures.size() > 32u ||
				SourceMaterial.Scalars.size() > 128u ||
				SourceMaterial.Vectors.size() > 128u ||
				SourceMaterial.StaticSwitches.size() > 128u)
			{
				strOutError = "Effect source Material profile metadata is invalid.";
				return false;
			}
			std::unordered_set<std::string> TextureNames;
			for (const EFFECT_NAMED_TEXTURE_DESC& Texture :
				SourceMaterial.Textures)
			{
				EFFECT_RESOURCE_FILE_KIND eActualKind =
					EFFECT_RESOURCE_FILE_KIND::END;
				if (Texture.strName.empty() || Texture.strName.size() > 128u ||
					!Has_VisibleCharacter(Texture.strName) ||
					Texture.strGroup.size() > 128u ||
					(!Texture.strGroup.empty() &&
						!Has_VisibleCharacter(Texture.strGroup)) ||
					Texture.strSourceObjectPath.size() > 512u ||
					(!Texture.strSourceObjectPath.empty() &&
						!Has_VisibleCharacter(Texture.strSourceObjectPath)) ||
					(!Texture.strAssetId.empty() &&
						Texture.strSourceObjectPath.empty()) ||
					(!Texture.strAssetId.empty() &&
						(!Is_SafeResourceAssetId(Texture.strAssetId, &eActualKind) ||
							eActualKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)) ||
					Texture.eAddressU >= EFFECT_TEXTURE_ADDRESS_MODE::END ||
					Texture.eAddressV >= EFFECT_TEXTURE_ADDRESS_MODE::END ||
					Texture.eColorSpace >= EFFECT_TEXTURE_COLOR_SPACE::END ||
					Texture.strSamplingEvidence.empty() ||
					Texture.strSamplingEvidence.size() > 128u ||
					!Is_StableId(Texture.strSamplingEvidence) ||
					!TextureNames.insert(Texture.strName).second)
				{
					strOutError = "Effect source Material texture is invalid: " +
						Texture.strName + " (" + Texture.strAssetId + ").";
					return false;
				}
			}
			std::unordered_set<std::string> ScalarNames;
			for (const EFFECT_NAMED_FLOAT_DESC& Scalar :
				SourceMaterial.Scalars)
			{
				if (Scalar.strName.empty() || Scalar.strName.size() > 128u ||
					!Has_VisibleCharacter(Scalar.strName) ||
					Scalar.strGroup.size() > 128u ||
					(!Scalar.strGroup.empty() &&
						!Has_VisibleCharacter(Scalar.strGroup)) ||
					!std::isfinite(Scalar.fValue) ||
					!ScalarNames.insert(Scalar.strName).second)
				{
					strOutError = "Effect source Material scalar is invalid.";
					return false;
				}
			}
			std::unordered_set<std::string> VectorNames;
			for (const EFFECT_NAMED_FLOAT4_DESC& Vector :
				SourceMaterial.Vectors)
			{
				if (Vector.strName.empty() || Vector.strName.size() > 128u ||
					!Has_VisibleCharacter(Vector.strName) ||
					Vector.strGroup.size() > 128u ||
					(!Vector.strGroup.empty() &&
						!Has_VisibleCharacter(Vector.strGroup)) ||
					!Is_Finite(Vector.vValue) ||
					!VectorNames.insert(Vector.strName).second)
				{
					strOutError = "Effect source Material vector is invalid.";
					return false;
				}
			}
			std::unordered_set<std::string> SwitchNames;
			for (const EFFECT_NAMED_BOOL_DESC& Switch :
				SourceMaterial.StaticSwitches)
			{
				if (Switch.strName.empty() || Switch.strName.size() > 128u ||
					!Has_VisibleCharacter(Switch.strName) ||
					Switch.strGroup.size() > 128u ||
					(!Switch.strGroup.empty() &&
						!Has_VisibleCharacter(Switch.strGroup)) ||
					!SwitchNames.insert(Switch.strName).second)
				{
					strOutError = "Effect source Material switch is invalid.";
					return false;
				}
			}
			for (const std::string& Semantic :
				SourceMaterial.DynamicParameterSemantics)
			{
				if (!Is_StableId(Semantic) ||
					!Is_SupportedEffectSourceDynamicParameterSemantic(Semantic))
				{
					strOutError =
						"Effect source Material Dynamic Parameter semantic is invalid.";
					return false;
				}
			}
		}

		std::unordered_set<std::string> Slots;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
		{
			const bool_t bMeshShape =
				Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput = bMeshShape ?
				nullptr : Find_EffectMaterialInput(
					*pMaterialTemplate, Binding.strSlotId);
			const EFFECT_RESOURCE_SLOT eRuntimeSlot = bMeshShape ?
				EFFECT_RESOURCE_SLOT::MESH_MODEL :
				(nullptr == pInput ? EFFECT_RESOURCE_SLOT::END :
					pInput->eRuntimeSlot);
			EFFECT_RESOURCE_FILE_KIND eActualKind = EFFECT_RESOURCE_FILE_KIND::END;
			const EFFECT_RESOURCE_FILE_KIND eExpectedKind = bMeshShape ?
				EFFECT_RESOURCE_FILE_KIND::MODEL :
				(nullptr == pInput ? EFFECT_RESOURCE_FILE_KIND::END :
					pInput->eAllowedResourceKind);
			if (!Is_ResourceSlotAllowed(Element.eKind, eRuntimeSlot) ||
				eExpectedKind == EFFECT_RESOURCE_FILE_KIND::END ||
				!Slots.insert(Binding.strSlotId).second ||
				!Is_SafeElementResourceAssetId(Element.eKind,
					Binding.strSlotId, Binding.strAssetId, &eActualKind) ||
				eActualKind != eExpectedKind)
			{
				strOutError = "Effect resource slot, path, file, or duplicate is invalid.";
				return false;
			}
		}
		/* Hard fail-closed rows remain loadable source evidence.  Every execution
		   target, including an authoring-approximate preview, must satisfy the
		   same source-profile/resource contract before it can be activated. */
		if (!Document.bSourceContract && bAuthoringExecutionTarget)
		{
			bool_t bSourceMaterialOwnsDrawableContract = false;
			if (!Validate_ExecutableSourceMaterialCarrier(
					Element, bSourceMaterialOwnsDrawableContract, strOutError))
			{
				strOutError =
					"Ordinary authored source Material is not admitted: " +
					Element.strElementId + ": " + strOutError;
				return false;
			}
		}
		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		if (Recipe.bEnabled)
		{
			const bool_t bRendererShapeValid =
				Recipe.strRendererShape == "sprite" ||
				Recipe.strRendererShape == "mesh" ||
				Recipe.strRendererShape == "decal" ||
				Recipe.strRendererShape == "ribbon" ||
				Recipe.strRendererShape == "light" ||
				Recipe.strRendererShape == "screenPost";
			if (!bRendererShapeValid ||
				!std::isfinite(Recipe.fEmitterDelaySeconds) ||
				Recipe.fEmitterDelaySeconds < 0.f ||
				!std::isfinite(Recipe.fEmitterDurationSeconds) ||
				Recipe.fEmitterDurationSeconds < 0.f ||
				Recipe.fEmitterDurationSeconds > 300.f ||
				Recipe.iEmitterLoopCount > 100000u ||
				Recipe.Bursts.size() > MAX_SOURCE_BURSTS_PER_ELEMENT ||
				Recipe.Modules.size() > MAX_SOURCE_MODULES_PER_ELEMENT)
			{
				strOutError = "Effect source recipe metadata or size is invalid.";
				return false;
			}
			f32_t fPreviousBurstTime = -1.f;
			for (const EFFECT_PARTICLE_BURST_DESC& Burst : Recipe.Bursts)
			{
				if (!std::isfinite(Burst.fTimeSeconds) ||
					Burst.fTimeSeconds < fPreviousBurstTime ||
					Burst.iCountMinimum > Burst.iCountMaximum ||
					Burst.iCountMaximum > 65535u)
				{
					strOutError = "Effect source burst is invalid.";
					return false;
				}
				fPreviousBurstTime = Burst.fTimeSeconds;
			}
			std::unordered_set<std::string> SourceModuleIds;
			for (const EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
			{
				if (Module.strStableId.empty() ||
					Module.strStableId.size() > 256u ||
					!SourceModuleIds.insert(Module.strStableId).second ||
					Module.strClassName.empty() ||
					Module.strClassName.size() > 128u ||
					Module.strObjectPath.empty() ||
					Module.strObjectPath.size() > 512u ||
					Module.Literals.size() > MAX_SOURCE_LITERALS_PER_MODULE ||
					Module.Distributions.size() >
						MAX_SOURCE_DISTRIBUTIONS_PER_MODULE)
				{
					strOutError = "Effect source module metadata or size is invalid.";
					return false;
				}
				std::unordered_set<std::string> PropertyPaths;
				for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
				{
					if (Literal.strPropertyPath.empty() ||
						Literal.strPropertyPath.size() > 512u ||
						Literal.eKind >= EFFECT_SOURCE_LITERAL_KIND::END ||
						!PropertyPaths.insert(Literal.strPropertyPath).second ||
						(EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind &&
							!std::isfinite(Literal.fNumber)) ||
						Literal.strString.size() > 2048u)
					{
						strOutError = "Effect source module literal is invalid.";
						return false;
					}
				}
				for (const EFFECT_DISTRIBUTION_DESC& Distribution :
					Module.Distributions)
				{
					if (!PropertyPaths.insert(
						Distribution.strPropertyPath).second ||
						!CEffectDistribution::Validate(Distribution,
							strOutError))
					{
						if (strOutError.empty())
							strOutError =
								"Effect source distribution is duplicated.";
						return false;
					}
				}
			}
		}
		if (!Document.bSourceContract && Recipe.bEnabled &&
			bAuthoringExecutionTarget &&
			(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
			 Element.eKind == EFFECT_ELEMENT_KIND::DECAL) &&
			!ValidatePortableAuthoredParticleRuntimeCarrier(
				Element, strOutError))
		{
			strOutError =
				"Ordinary authored emitter sourceRecipe is not admitted by the portable runtime: " +
				Element.strElementId + ": " + strOutError;
			return false;
		}
		const EFFECT_SOURCE_PRESENTATION_DESC& SourcePresentation =
			Element.SourcePresentation;
		if (SourcePresentation.bEnabled)
		{
			if (SourcePresentation.strSchema !=
					EFFECT_SOURCE_PRESENTATION_SCHEMA ||
				SourcePresentation.iVersion != 1u ||
				!Is_StableId(SourcePresentation.strProfileId) ||
				SourcePresentation.eStatus >=
					EFFECT_SOURCE_PRESENTATION_STATUS::END ||
				SourcePresentation.strSourceObjectPath.empty() ||
				SourcePresentation.strSourceObjectPath.size() > 512u ||
				!Has_VisibleCharacter(
					SourcePresentation.strSourceObjectPath) ||
				SourcePresentation.strSourceActionCueId.size() > 256u ||
				(!SourcePresentation.strSourceActionCueId.empty() &&
					!Has_VisibleCharacter(
						SourcePresentation.strSourceActionCueId)) ||
				SourcePresentation.strSourceEventId.empty() ||
				SourcePresentation.strSourceEventId.size() > 256u ||
				!Has_VisibleCharacter(
					SourcePresentation.strSourceEventId) ||
				!std::isfinite(SourcePresentation.fSourceTimeSeconds) ||
				SourcePresentation.fSourceTimeSeconds < 0.f ||
				SourcePresentation.Parameters.size() >
					MAX_SOURCE_PRESENTATION_PARAMETERS)
			{
				strOutError =
					"Effect source presentation metadata is invalid.";
				return false;
			}
			std::unordered_set<std::string> ParameterNames;
			for (const EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC& Parameter :
				SourcePresentation.Parameters)
			{
				if (Parameter.strName.empty() ||
					Parameter.strName.size() > 128u ||
					!Has_VisibleCharacter(Parameter.strName) ||
					!ParameterNames.insert(Parameter.strName).second ||
					Parameter.eKind >=
						EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND::END ||
					Parameter.eStatus >=
						EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS::END ||
					Parameter.strSourcePropertyPath.size() > 512u ||
					!std::isfinite(Parameter.fNumberValue) ||
					!Is_Finite(Parameter.vVectorValue) ||
					Parameter.strStringValue.size() > 2048u)
				{
					strOutError =
						"Effect source presentation parameter is invalid.";
					return false;
				}
			}
			/* An unresolved presentation is retained source evidence, not an
			   executable authored carrier.  Keeping this invariant in the codec
			   prevents a saved Visible toggle from bypassing Tool-side locks for
			   deferred AnimationTrail, Light, Dust, or future source families. */
			if (!Document.bSourceContract && bAuthoringExecutionTarget &&
				SourcePresentation.eStatus ==
					EFFECT_SOURCE_PRESENTATION_STATUS::UNRESOLVED &&
				Element.bVisible)
			{
				strOutError =
					"Unresolved source presentation cannot be enabled for ordinary playback: " +
					Element.strElementId + ".";
				return false;
			}
		}
		const EFFECT_DETAIL_DESC& D = Element.Detail;
		const bool_t bMeshParticle =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		const bool_t bManualParticle =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END;
		const bool_t bManualSprite =
			Element.eKind == EFFECT_ELEMENT_KIND::SPRITE &&
			bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END;
		const bool_t bGenericMeshRingFillCarrier =
			bManualParticle && bMeshParticle &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		const bool_t bGenericLinearRevealCarrier =
			(bManualSprite || (bManualParticle && !bMeshParticle)) &&
			Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled &&
			!Element.Material.Execution.bFailClosed &&
			!Element.Material.Execution.bAuthoringApproximate &&
			Element.Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		const bool_t bCompositionLayerValid =
			Element.eCompositionLayer < EFFECT_COMPOSITION_LAYER::END &&
			(Element.eCompositionLayer == EFFECT_COMPOSITION_LAYER::NORMAL ||
			 Is_EffectWorldMarkCarrier(Element));
		const bool_t bMeshTransformMotionCarrier =
			Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
			bMeshParticle;
		const int64_t iTileCount = static_cast<int64_t>(D.UV.iTileColumns) * D.UV.iTileRows;
		const bool_t bCommonValid =
			Is_Finite(D.Transform.vPosition) && Is_Finite(D.Transform.vRotationDegrees) &&
			Is_Finite(D.Transform.vRevolutionDegreesPerSecond) && Is_Finite(D.Transform.vScale) &&
			D.Transform.vScale.x > 0.f && D.Transform.vScale.y > 0.f && D.Transform.vScale.z > 0.f &&
			Is_Finite(D.Transform.vVelocityPerSecond) && Is_Finite(D.Color.vColorOffset) &&
			Is_Finite(D.Color.vColorMultiply) &&
			std::isfinite(D.Color.fColorClip) &&
			D.Color.fColorClip >= 0.f && D.Color.fColorClip <= 1.f &&
			std::isfinite(D.Color.fEmissiveIntensity) && D.Color.fEmissiveIntensity >= 0.f &&
			std::isfinite(D.Color.fDistortionIntensity) && D.Color.fDistortionIntensity >= 0.f &&
			std::isfinite(D.Color.fRadialTime) && std::isfinite(D.Color.fRadialIntensity) &&
			Is_Finite(D.UV.vStart) && Is_Finite(D.UV.vSpeed) && Is_Finite(D.UV.vWaveAmplitude) &&
			std::isfinite(D.UV.fWaveFrequency) && D.UV.fWaveFrequency >= 0.f &&
			std::isfinite(D.UV.fSequenceTerm) && D.UV.fSequenceTerm > 0.f &&
			D.UV.iTileColumns > 0 && D.UV.iTileRows > 0 && D.UV.iTileIndex >= 0 &&
			iTileCount > 0 && D.UV.iTileIndex < iTileCount &&
			std::isfinite(D.Timing.fStartDelaySeconds) && D.Timing.fStartDelaySeconds >= 0.f &&
			std::isfinite(D.Timing.fLifeTimeSeconds) && D.Timing.fLifeTimeSeconds > 0.f &&
			std::isfinite(D.Timing.fTransformMotionDurationSeconds) &&
			D.Timing.fTransformMotionDurationSeconds >= 0.f &&
			D.Timing.fTransformMotionDurationSeconds <= D.Timing.fLifeTimeSeconds &&
			(0.f == D.Timing.fTransformMotionDurationSeconds ||
			 bMeshTransformMotionCarrier) &&
			std::isfinite(D.Timing.fAfterImageSeconds) && D.Timing.fAfterImageSeconds >= 0.f &&
			std::isfinite(D.Timing.fDissolveStartNormalized) &&
			D.Timing.fDissolveStartNormalized >= 0.f && D.Timing.fDissolveStartNormalized <= 1.f &&
			std::isfinite(D.Mesh.fModelPreScale) &&
			D.Mesh.fModelPreScale > 0.f && D.Mesh.fModelPreScale <= 100.f &&
			Is_Finite(D.Mesh.vSourceTypeDataRotationDegrees) &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.x) <= 3600.f &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.y) <= 3600.f &&
			std::abs(D.Mesh.vSourceTypeDataRotationDegrees.z) <= 3600.f &&
			(Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE ||
				(Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
				 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				 Element.SourceRecipe.bEnabled &&
				 Element.SourceRecipe.strRendererShape == "mesh") ||
				(0.f == D.Mesh.vSourceTypeDataRotationDegrees.x &&
				 0.f == D.Mesh.vSourceTypeDataRotationDegrees.y &&
				 0.f == D.Mesh.vSourceTypeDataRotationDegrees.z)) &&
			std::isfinite(D.Sprite.fBillboardRollDegrees) &&
			std::abs(D.Sprite.fBillboardRollDegrees) <= 3600.f &&
			std::isfinite(D.Sprite.fBillboardRollDegreesPerSecond) &&
			std::abs(D.Sprite.fBillboardRollDegreesPerSecond) <= 3600.f &&
			Is_Finite(D.Decal.vSize) && D.Decal.vSize.x > 0.f && D.Decal.vSize.y > 0.f &&
			std::isfinite(D.Decal.fDepth) && D.Decal.fDepth > 0.f;
		const bool_t bLerpValid =
			Is_Finite(D.LinearLerp.vEndPosition) && Is_Finite(D.LinearLerp.vEndRotationDegrees) &&
			Is_Finite(D.LinearLerp.vEndRevolutionDegreesPerSecond) && Is_Finite(D.LinearLerp.vEndScale) &&
			D.LinearLerp.vEndScale.x > 0.f && D.LinearLerp.vEndScale.y > 0.f && D.LinearLerp.vEndScale.z > 0.f &&
			Is_Finite(D.LinearLerp.vEndVelocityPerSecond) && Is_Finite(D.LinearLerp.vEndColorOffset) &&
			Is_Finite(D.LinearLerp.vEndColorMultiply) &&
			std::isfinite(D.LinearLerp.fEndEmissiveIntensity) &&
			D.LinearLerp.fEndEmissiveIntensity >= 0.f &&
			std::isfinite(D.LinearLerp.fEndRingFillProgress) &&
			D.LinearLerp.fEndRingFillProgress >= 0.f &&
			D.LinearLerp.fEndRingFillProgress <= 1.f &&
			(D.LinearLerp.bRingFillProgress ||
			 D.LinearLerp.fEndRingFillProgress == 1.f) &&
			(!D.LinearLerp.bRingFillProgress || D.Mesh.RingFill.bEnabled);
		const bool_t bParticleValid =
			D.Particle.iMaxParticles >= 1u && D.Particle.iMaxParticles <= 2048u &&
			D.Particle.iBurstCount <= D.Particle.iMaxParticles && D.Particle.iRandomSeed != 0u &&
			std::isfinite(D.Particle.fSpawnRatePerSecond) && D.Particle.fSpawnRatePerSecond >= 0.f && D.Particle.fSpawnRatePerSecond <= 2048.f &&
			std::isfinite(D.Particle.fFixedCenterSpacingWorldUnits) &&
			D.Particle.fFixedCenterSpacingWorldUnits >= 0.f &&
			(D.Particle.fFixedCenterSpacingWorldUnits == 0.f ||
			 D.Particle.fFixedCenterSpacingWorldUnits >= 0.001f) &&
			D.Particle.fFixedCenterSpacingWorldUnits <= 1000.f &&
			Is_Finite(D.Particle.vLifeTimeSeconds) && D.Particle.vLifeTimeSeconds.x > 0.f && D.Particle.vLifeTimeSeconds.y >= D.Particle.vLifeTimeSeconds.x && D.Particle.vLifeTimeSeconds.y <= 30.f &&
			Is_Finite(D.Particle.vInitialPositionMin) && Is_Finite(D.Particle.vInitialPositionMax) &&
			D.Particle.vInitialPositionMax.x >= D.Particle.vInitialPositionMin.x &&
			D.Particle.vInitialPositionMax.y >= D.Particle.vInitialPositionMin.y &&
			D.Particle.vInitialPositionMax.z >= D.Particle.vInitialPositionMin.z &&
			Is_Finite(D.Particle.vInitialVelocityMin) && Is_Finite(D.Particle.vInitialVelocityMax) && Is_Finite(D.Particle.vAcceleration) &&
			D.Particle.vInitialVelocityMax.x >= D.Particle.vInitialVelocityMin.x &&
			D.Particle.vInitialVelocityMax.y >= D.Particle.vInitialVelocityMin.y &&
			D.Particle.vInitialVelocityMax.z >= D.Particle.vInitialVelocityMin.z &&
			Is_Finite(D.Particle.vStartSize) && D.Particle.vStartSize.x > 0.f && D.Particle.vStartSize.y > 0.f &&
			Is_Finite(D.Particle.vEndSize) && D.Particle.vEndSize.x >= 0.f && D.Particle.vEndSize.y >= 0.f &&
			D.Particle.iDynamicParameterComponentMask <= 0x0fu &&
			Is_Finite(D.Particle.vDynamicParameterStart) &&
			Is_Finite(D.Particle.vDynamicParameterEnd) &&
			(EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
				0u == D.Particle.iDynamicParameterComponentMask);
		/* A shape that cannot be sampled - a negative or inverted radius, an empty
		   arc, a zero-extent box - would silently collapse every particle onto the
		   origin instead of failing, so it is rejected at load. */
		const EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Shape = D.Particle.SpawnShape;
		const EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC& Orientation =
			D.Particle.InitialOrientation;
		const EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Emission =
			D.Particle.InitialVelocity;
		const bool_t bFixedCenterSpacingEnabled =
			D.Particle.fFixedCenterSpacingWorldUnits > 0.f;
		const bool_t bFixedCenterSpacingValid =
			!bFixedCenterSpacingEnabled ||
			(bManualParticle && !D.Particle.bLocalSpace &&
			 D.Particle.fSpawnRatePerSecond == 0.f &&
			 D.Particle.iBurstCount == 0u &&
			 D.Particle.vInitialPositionMin.x == 0.f &&
			 D.Particle.vInitialPositionMin.y == 0.f &&
			 D.Particle.vInitialPositionMin.z == 0.f &&
			 D.Particle.vInitialPositionMax.x == 0.f &&
			 D.Particle.vInitialPositionMax.y == 0.f &&
			 D.Particle.vInitialPositionMax.z == 0.f &&
			 D.Particle.vInitialVelocityMin.x == 0.f &&
			 D.Particle.vInitialVelocityMin.y == 0.f &&
			 D.Particle.vInitialVelocityMin.z == 0.f &&
			 D.Particle.vInitialVelocityMax.x == 0.f &&
			 D.Particle.vInitialVelocityMax.y == 0.f &&
				 D.Particle.vInitialVelocityMax.z == 0.f &&
				 D.Particle.vAcceleration.x == 0.f &&
				 D.Particle.vAcceleration.y == 0.f &&
				 D.Particle.vAcceleration.z == 0.f &&
				 Shape.eKind == EFFECT_PARTICLE_SPAWN_SHAPE::POINT &&
				 Shape.eDistribution == EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM &&
				 Emission.eMode == EFFECT_PARTICLE_VELOCITY_MODE::FIXED &&
				 D.Particle.TargetAttractor.Is_Default());
		const bool_t bSpawnShapeValid =
			Shape.eKind < EFFECT_PARTICLE_SPAWN_SHAPE::END &&
			Shape.eDistribution < EFFECT_PARTICLE_SPAWN_DISTRIBUTION::END &&
			std::isfinite(Shape.fRadius) && std::isfinite(Shape.fInnerRadius) &&
			Is_Finite(Shape.vExtents) && std::isfinite(Shape.fArcDegrees) &&
			Shape.fRadius >= 0.f && Shape.fInnerRadius >= 0.f &&
			Shape.fInnerRadius <= Shape.fRadius &&
			Shape.fArcDegrees > 0.f && Shape.fArcDegrees <= 360.f &&
			Shape.vExtents.x >= 0.f && Shape.vExtents.y >= 0.f &&
			Shape.vExtents.z >= 0.f &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::POINT == Shape.eKind ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::SPHERE != Shape.eKind || Shape.fRadius > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::RING != Shape.eKind || Shape.fRadius > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_SHAPE::BOX != Shape.eKind ||
				Shape.vExtents.x > 0.f || Shape.vExtents.y > 0.f ||
				Shape.vExtents.z > 0.f) &&
			(EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM ==
				Shape.eDistribution ||
			 (bManualParticle &&
			  EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind &&
			  D.Particle.fSpawnRatePerSecond == 0.f &&
			  D.Particle.iBurstCount >= 2u));
		const bool_t bInitialOrientationValid =
			Orientation.eMode < EFFECT_PARTICLE_ORIENTATION_MODE::END &&
			std::isfinite(Orientation.fOffsetDegrees) &&
			std::abs(Orientation.fOffsetDegrees) <= 3600.f &&
			(EFFECT_PARTICLE_ORIENTATION_MODE::FIXED != Orientation.eMode ||
			 Orientation.fOffsetDegrees == 0.f) &&
			(EFFECT_PARTICLE_ORIENTATION_MODE::FIXED == Orientation.eMode ||
			 (bManualParticle && !bMeshParticle &&
			  EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind &&
			  !D.Particle.bBillboard));
		const bool_t bInitialVelocityValid =
			Emission.eMode < EFFECT_PARTICLE_VELOCITY_MODE::END &&
			Is_Finite(Emission.vSpeedRange) &&
			std::isfinite(Emission.fConeAngleDegrees) &&
			Emission.vSpeedRange.y >= Emission.vSpeedRange.x &&
			Emission.fConeAngleDegrees >= 0.f &&
			Emission.fConeAngleDegrees <= 180.f &&
			(EFFECT_PARTICLE_VELOCITY_MODE::FIXED == Emission.eMode ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind);
		const bool_t bHasNativeSpriteDynamics = D.Particle.fDrag != 0.f ||
			D.Particle.vRotationRangeDegrees.x != 0.f ||
			D.Particle.vRotationRangeDegrees.y != 0.f ||
			D.Particle.vSpinRangeDegreesPerSecond.x != 0.f ||
			D.Particle.vSpinRangeDegreesPerSecond.y != 0.f ||
			D.Particle.bSubUVOverLife || Emission.bUniformSolidAngle;
		const bool_t bNativeSpriteDynamicsValid =
			std::isfinite(D.Particle.fDrag) &&
			D.Particle.fDrag >= 0.f && D.Particle.fDrag <= 1000.f &&
			Is_Finite(D.Particle.vRotationRangeDegrees) &&
			Is_Finite(D.Particle.vSpinRangeDegreesPerSecond) &&
			D.Particle.vRotationRangeDegrees.x >= -3600.f &&
			D.Particle.vRotationRangeDegrees.y <= 3600.f &&
			D.Particle.vRotationRangeDegrees.x <=
				D.Particle.vRotationRangeDegrees.y &&
			D.Particle.vSpinRangeDegreesPerSecond.x >= -3600.f &&
			D.Particle.vSpinRangeDegreesPerSecond.y <= 3600.f &&
			D.Particle.vSpinRangeDegreesPerSecond.x <=
				D.Particle.vSpinRangeDegreesPerSecond.y &&
			(!bHasNativeSpriteDynamics ||
			 (bManualParticle && !bMeshParticle && D.Particle.bBillboard &&
			  Element.Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			  Element.Material.strSourceMaterialPath.empty() &&
			  !Element.Material.SourceMaterial.bEnabled &&
			  !Element.Material.Execution.bEnabled &&
			  !Element.Material.Execution.bFailClosed &&
			  !Element.Material.Execution.bAuthoringApproximate)) &&
			(!Emission.bUniformSolidAngle ||
			 (Emission.eMode == EFFECT_PARTICLE_VELOCITY_MODE::CONE &&
			  Emission.vSpeedRange.x >= 0.f)) &&
			(!D.Particle.bSubUVOverLife ||
			 (!D.UV.bSequence && !D.UV.bLoop && D.UV.iTileIndex == 0 &&
			  iTileCount > 1 && iTileCount <= UINT32_MAX));
		if (!bNativeSpriteDynamicsValid)
		{
			strOutError =
				"Effect native sprite particle dynamics or life SubUV contract is invalid: " +
				Element.strElementId;
			return false;
		}
		const EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
			D.Particle.TargetAttractor;
		const bool_t bTargetAttractorValid =
			Attractor.eTargetSpace <
				EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE::END &&
			Is_Finite(Attractor.vTargetOffset) &&
			std::abs(Attractor.vTargetOffset.x) <= 1000.f &&
			std::abs(Attractor.vTargetOffset.y) <= 1000.f &&
			std::abs(Attractor.vTargetOffset.z) <= 1000.f &&
			Is_Finite(Attractor.vActiveNormalized) &&
			std::isfinite(Attractor.fRadialAcceleration) &&
			std::isfinite(Attractor.fTangentialAcceleration) &&
			std::isfinite(Attractor.fMaximumSpeed) &&
			std::isfinite(Attractor.fConvergenceRadius) &&
			std::isfinite(Attractor.fArrivalDamping) &&
			Attractor.vActiveNormalized.x >= 0.f &&
			Attractor.vActiveNormalized.y > Attractor.vActiveNormalized.x &&
			Attractor.vActiveNormalized.y <= 1.f &&
			Attractor.fRadialAcceleration >= 0.f &&
			Attractor.fRadialAcceleration <= 10000.f &&
			Attractor.fTangentialAcceleration >= -10000.f &&
			Attractor.fTangentialAcceleration <= 10000.f &&
			Attractor.fMaximumSpeed > 0.f &&
			Attractor.fMaximumSpeed <= 1000.f &&
			Attractor.fConvergenceRadius > 0.f &&
			Attractor.fConvergenceRadius <= 1000.f &&
			Attractor.fArrivalDamping >= 0.f &&
			Attractor.fArrivalDamping <= 1000.f &&
			(!Attractor.bEnabled ||
				EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind) &&
			(Attractor.bEnabled || Attractor.Is_Default());
		/* The trim multiplies the source's own numbers. Count, size and lifetime
		   stay positive; speed and rotation may intentionally stop or reverse;
		   alpha and delay may be zero. Ceilings reject accidental extreme input. */
		const EFFECT_PARTICLE_SOURCE_SCALE_DESC& SourceScale =
			D.Particle.SourceScale;
		const bool_t bSourceScaleValid =
			std::isfinite(SourceScale.fCount) &&
			std::isfinite(SourceScale.fSize) &&
			std::isfinite(SourceScale.fLifeTime) &&
			std::isfinite(SourceScale.fSpeed) &&
			std::isfinite(SourceScale.fRotation) &&
			std::isfinite(SourceScale.fAlpha) &&
			std::isfinite(SourceScale.fSpawnDelay) &&
			SourceScale.fCount > 0.f && SourceScale.fCount <= 16.f &&
			SourceScale.fSize > 0.f && SourceScale.fSize <= 16.f &&
			SourceScale.fLifeTime > 0.f && SourceScale.fLifeTime <= 16.f &&
			/* Speed and rotation may legitimately be reversed or stopped, so
			   they allow zero and negative. Rotation is a source-angle/rate
			   multiplier and therefore has the wider authoring range. */
			SourceScale.fSpeed >= -16.f && SourceScale.fSpeed <= 16.f &&
			SourceScale.fRotation >= -360.f && SourceScale.fRotation <= 360.f &&
			SourceScale.fAlpha >= 0.f && SourceScale.fAlpha <= 16.f &&
			SourceScale.fSpawnDelay >= 0.f && SourceScale.fSpawnDelay <= 16.f;
		const bool_t bTrailValid =
			D.Trail.iMaxPoints >= 2u && D.Trail.iMaxPoints <= 512u &&
			std::isfinite(D.Trail.fPointLifeTimeSeconds) && D.Trail.fPointLifeTimeSeconds > 0.f &&
			std::isfinite(D.Trail.fSampleIntervalSeconds) && D.Trail.fSampleIntervalSeconds > 0.f &&
			std::isfinite(D.Trail.fMinimumDistance) && D.Trail.fMinimumDistance >= 0.f &&
			std::isfinite(D.Trail.fStartWidth) && D.Trail.fStartWidth > 0.f &&
			std::isfinite(D.Trail.fEndWidth) && D.Trail.fEndWidth >= 0.f &&
			std::isfinite(D.Trail.fTilingDistanceWorldUnits) &&
			D.Trail.fTilingDistanceWorldUnits >= 0.f &&
			std::isfinite(D.Trail.fDistanceTessellationStepWorldUnits) &&
			D.Trail.fDistanceTessellationStepWorldUnits >= 0.f;
		const bool_t bAfterImageValid =
			std::isfinite(D.AfterImage.fSampleIntervalSeconds) && D.AfterImage.fSampleIntervalSeconds > 0.f &&
			D.AfterImage.iMaxCopies <= 32u && std::isfinite(D.AfterImage.fAlphaExponent) && D.AfterImage.fAlphaExponent > 0.f &&
			(D.Timing.fAfterImageSeconds <= 0.f || D.AfterImage.iMaxCopies == 0u ||
				Element.eKind == EFFECT_ELEMENT_KIND::MESH || Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		const bool_t bLightValid =
			(EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
				!D.Light.bEnabled) &&
			(!D.Light.bEnabled ||
				(D.Light.eProfile < EFFECT_LIGHT_PROFILE::END &&
					D.Light.eStatus ==
						EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE &&
					std::isfinite(D.Light.fRange) && D.Light.fRange > 0.f &&
					std::isfinite(D.Light.fIntensity) &&
					D.Light.fIntensity >= 0.f && Is_Finite(D.Light.vColor) &&
					Is_Finite(D.Light.vAmbient) &&
					std::isfinite(D.Light.fFalloffExponent) &&
					D.Light.fFalloffExponent > 0.f));
		const bool_t bScreenPostValid =
			(EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind ||
				!D.ScreenPost.bEnabled) &&
			(!D.ScreenPost.bEnabled ||
				(D.ScreenPost.eProfile < EFFECT_SCREEN_POST_PROFILE::END &&
					D.ScreenPost.eStatus ==
						EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE &&
					std::isfinite(D.ScreenPost.fIntensity) &&
					D.ScreenPost.fIntensity >= 0.f &&
					std::isfinite(D.ScreenPost.fSecondaryIntensity) &&
					D.ScreenPost.fSecondaryIntensity >= 0.f &&
					std::isfinite(D.ScreenPost.fFrequency) &&
					D.ScreenPost.fFrequency >= 0.f &&
					Is_Finite(D.ScreenPost.vTint) &&
					0u != D.ScreenPost.iRandomSeed));
		const EFFECT_MESH_RING_FILL_DESC& RingFill = D.Mesh.RingFill;
		const bool_t bRingFillValid =
			RingFill.eDirection < EFFECT_RING_FILL_DIRECTION::END &&
			std::isfinite(RingFill.fProgress) &&
			RingFill.fProgress >= 0.f && RingFill.fProgress <= 1.f &&
			std::isfinite(RingFill.fFeather) && RingFill.fFeather >= 0.f &&
			RingFill.fFeather <= 0.5f &&
			(RingFill.bEnabled || RingFill.Is_Default()) &&
			(!RingFill.bEnabled || bGenericMeshRingFillCarrier);
		const EFFECT_LINEAR_REVEAL_DESC& LinearReveal = D.Sprite.LinearReveal;
		const bool_t bFixedParticleRevealClock = !bManualParticle ||
			(std::fabs(D.Particle.vLifeTimeSeconds.x -
				D.Particle.vLifeTimeSeconds.y) <= 0.00001f &&
			 LinearReveal.fStartSeconds + LinearReveal.fDurationSeconds <=
				D.Particle.vLifeTimeSeconds.x);
		const bool_t bLinearRevealValid =
			LinearReveal.eAxis < EFFECT_LINEAR_REVEAL_AXIS::END &&
			std::isfinite(LinearReveal.fStartSeconds) &&
			LinearReveal.fStartSeconds >= 0.f &&
			LinearReveal.fStartSeconds <= 30.f &&
			std::isfinite(LinearReveal.fDurationSeconds) &&
			LinearReveal.fDurationSeconds > 0.f &&
			LinearReveal.fDurationSeconds <= 30.f &&
			std::isfinite(LinearReveal.fEdgeWidth) &&
			LinearReveal.fEdgeWidth >= 0.f &&
			LinearReveal.fEdgeWidth <= 0.5f &&
			std::isfinite(LinearReveal.fSoftness) &&
			LinearReveal.fSoftness >= 0.f &&
			LinearReveal.fSoftness <= 0.25f &&
			LinearReveal.fEdgeWidth + LinearReveal.fSoftness <= 0.5f &&
			Is_Finite(LinearReveal.vEdgeColor) &&
			LinearReveal.vEdgeColor.x >= 0.f &&
			LinearReveal.vEdgeColor.x <= 1.f &&
			LinearReveal.vEdgeColor.y >= 0.f &&
			LinearReveal.vEdgeColor.y <= 1.f &&
			LinearReveal.vEdgeColor.z >= 0.f &&
			LinearReveal.vEdgeColor.z <= 1.f &&
			LinearReveal.vEdgeColor.w >= 0.f &&
			LinearReveal.vEdgeColor.w <= 1.f &&
			std::isfinite(LinearReveal.fEdgeEmissive) &&
			LinearReveal.fEdgeEmissive >= 0.f &&
			LinearReveal.fEdgeEmissive <= 100.f &&
			(LinearReveal.bEnabled || LinearReveal.Is_Default()) &&
			(!LinearReveal.bEnabled ||
				 (bGenericLinearRevealCarrier && bFixedParticleRevealClock &&
				  LinearReveal.fStartSeconds + LinearReveal.fDurationSeconds <=
					D.Timing.fLifeTimeSeconds));
		const EFFECT_DECAL_DETAIL_DESC& Decal = D.Decal;
		const bool_t bDecalReceiverValid =
			Decal.eReceiverMode < EFFECT_DECAL_RECEIVER_MODE::END &&
			std::isfinite(Decal.fNormalCutoff) &&
			std::isfinite(Decal.fEdgeFade) &&
			Decal.fEdgeFade >= 0.f && Decal.fEdgeFade <= 1.f &&
			((Decal.eReceiverMode == EFFECT_DECAL_RECEIVER_MODE::ALL_OPAQUE &&
			  Decal.fNormalCutoff == -1.f) ||
			 (Decal.eReceiverMode ==
				EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES &&
			  Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
			  Decal.fNormalCutoff >= 0.f && Decal.fNormalCutoff <= 1.f)) &&
			(Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
			 Decal.Is_ReceiverDefault());
		if (!bCompositionLayerValid || !bCommonValid ||
			!bDecalReceiverValid || !bLerpValid || !bParticleValid ||
			!bFixedCenterSpacingValid || !bSpawnShapeValid ||
			!bInitialOrientationValid ||
			!bInitialVelocityValid || !bRingFillValid ||
			!bLinearRevealValid ||
			!bTargetAttractorValid ||
			!bSourceScaleValid ||
			!bTrailValid || !bAfterImageValid || !bLightValid ||
			!bScreenPostValid)
		{
			strOutError = "Effect Detail contains an invalid number or range.";
			return false;
		}
		/* Playback also simulates source-visual mesh, sprite, and decal carriers
		   through Detail.Particle.  Count the same carrier set here so a source
		   document cannot bypass the document-wide particle cap merely because
		   its authored Element kind describes the renderer shape. */
		const bool_t bSourceRecipeParticleCarrier =
			Element.SourceRecipe.bEnabled &&
			(Element.SourceRecipe.strRendererShape == "mesh" ||
			 Element.SourceRecipe.strRendererShape == "sprite" ||
			 Element.SourceRecipe.strRendererShape == "decal");
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			bSourceRecipeParticleCarrier)
		{
			iTotalParticles += SourceScaledParticleCeiling(Element);
		}
		if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
			iTotalTrailPoints += D.Trail.iMaxPoints;
		if (D.Timing.fAfterImageSeconds > 0.f &&
			D.AfterImage.iMaxCopies > 0u)
			iTotalAfterImages += D.AfterImage.iMaxCopies;
	}
	if (iTotalParticles > MAX_DOCUMENT_PARTICLES ||
		iTotalTrailPoints > MAX_DOCUMENT_TRAIL_POINTS ||
		iTotalAfterImages > MAX_DOCUMENT_AFTERIMAGES)
	{
		strOutError = "Effect Document exceeds the particle, trail, or after-image budget.";
		return false;
	}
	if (!Document.bSourceContract &&
		!ValidatePortableAuthoredParticleEventRoutes(Document, strOutError))
	{
		return false;
	}

	const auto SameFloat3 = [](const float3_t& Left, const float3_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
	};
	const auto SameTransform = [&SameFloat3](
		const EFFECT_TRANSFORM_DESC& Left,
		const EFFECT_TRANSFORM_DESC& Right)
	{
		return SameFloat3(Left.vPosition, Right.vPosition) &&
			SameFloat3(Left.vRotationDegrees, Right.vRotationDegrees) &&
			SameFloat3(Left.vRevolutionDegreesPerSecond,
				Right.vRevolutionDegreesPerSecond) &&
			SameFloat3(Left.vScale, Right.vScale) &&
			SameFloat3(Left.vVelocityPerSecond, Right.vVelocityPerSecond);
	};
	const auto SameAttachment = [&SameTransform](
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Left,
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Right)
	{
		return Left.bEnabled == Right.bEnabled &&
			Left.bFollow == Right.bFollow &&
			Left.eOrientation == Right.eOrientation &&
			Left.strSourceAnchorSlotId == Right.strSourceAnchorSlotId &&
			Left.strRuntimeAnchorSlotId == Right.strRuntimeAnchorSlotId &&
			Left.strRuntimeBoneName == Right.strRuntimeBoneName &&
			Left.fSnapshotRootSourceBasisYawDegrees ==
				Right.fSnapshotRootSourceBasisYawDegrees &&
			SameTransform(Left.SocketLocalTransform,
				Right.SocketLocalTransform);
	};
	std::unordered_map<std::string, const EFFECT_ACTION_CUE_ATTACHMENT_DESC*>
		FollowAnchorsById;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		if (!Attachment.bEnabled || !Attachment.bFollow)
			continue;
		const auto [Iterator, bInserted] = FollowAnchorsById.emplace(
			Attachment.strRuntimeAnchorSlotId, &Attachment);
		if (!bInserted &&
			(Iterator->second->strRuntimeBoneName != Attachment.strRuntimeBoneName ||
			 Iterator->second->eOrientation != Attachment.eOrientation ||
			 !SameTransform(Iterator->second->SocketLocalTransform,
				Attachment.SocketLocalTransform)))
		{
			strOutError = "Effect follow anchor ID has conflicting bone, socket, or orientation: " +
				Attachment.strRuntimeAnchorSlotId + ".";
			return false;
		}
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		const EFFECT_TRANSFORM_INHERITANCE_DESC& Inheritance =
			Element.TransformInheritance;
		if (!Inheritance.bEnabled)
		{
			if (!Inheritance.strMasterElementId.empty())
			{
				strOutError =
					"Disabled Effect transform inheritance must not name a master Element.";
				return false;
			}
			continue;
		}
		if (!Is_StableId(Inheritance.strMasterElementId) ||
			Inheritance.strMasterElementId == Element.strElementId)
		{
			strOutError =
				"Effect transform inheritance master identity is invalid or self-referential.";
			return false;
		}
		const auto MasterIterator = ElementsById.find(
			Inheritance.strMasterElementId);
		if (MasterIterator == ElementsById.end())
		{
			strOutError = "Effect transform inheritance master Element is missing.";
			return false;
		}
		const EFFECT_ELEMENT_DESC& Master = *MasterIterator->second;
		if (Element.strGroupId.empty() ||
			Element.strGroupId != Master.strGroupId)
		{
			strOutError =
				"Effect transform inheritance must remain inside one Component group.";
			return false;
		}
		if (!Master.bVisible ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Master.eKind ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			strOutError =
				"Effect transform inheritance requires a visible world-space master and companion.";
			return false;
		}
		if (Element.Detail.Timing.fStartDelaySeconds !=
				Master.Detail.Timing.fStartDelaySeconds ||
			Element.SourceRecipe.fEmitterDelaySeconds !=
				Master.SourceRecipe.fEmitterDelaySeconds)
		{
			strOutError =
				"Effect transform inheritance master and companion start times must match.";
			return false;
		}
		if (!SameAttachment(Element.ActionCueAttachment,
			Master.ActionCueAttachment))
		{
			strOutError =
				"Effect transform inheritance master and companion attachment spaces must match.";
			return false;
		}
	}

	std::unordered_map<std::string, uint8_t> VisitStates;
	const auto VisitInheritance = [&](const auto& Self,
		const EFFECT_ELEMENT_DESC& Element) -> bool_t
	{
		uint8_t& State = VisitStates[Element.strElementId];
		if (1u == State)
		{
			strOutError = "Effect transform inheritance cycle is not allowed.";
			return false;
		}
		if (2u == State)
			return true;
		State = 1u;
		if (Element.TransformInheritance.bEnabled)
		{
			const auto MasterIterator = ElementsById.find(
				Element.TransformInheritance.strMasterElementId);
			if (MasterIterator == ElementsById.end() ||
				!Self(Self, *MasterIterator->second))
			{
				return false;
			}
		}
		State = 2u;
		return true;
	};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!VisitInheritance(VisitInheritance, Element))
			return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.TransformInheritance.bEnabled)
			continue;
		const EFFECT_ELEMENT_DESC& Master = *ElementsById.at(
			Element.TransformInheritance.strMasterElementId);
		if (Master.TransformInheritance.bEnabled)
		{
			strOutError =
				"Effect transform inheritance companions must reference one terminal master directly.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Validate_SourceContract(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion != EFFECT_SOURCE_CONTRACT_FORMAT_VERSION ||
		!Document.bSourceContract)
	{
		strOutError = "Effect document is not a native-v14 source contract.";
		return false;
	}
	if (!Validate_AuthoredRuntimeExtensions(Document, strOutError))
		return false;
	if (!Is_StableId(Document.strEffectAssetId) ||
		Document.strDisplayName.empty() || Document.strDisplayName.size() > 128u ||
		Document.Elements.empty() || Document.Elements.size() > MAX_ELEMENTS)
	{
		strOutError = "Source-contract document identity or size is invalid.";
		return false;
	}

	const auto SafeAssetId = [](const std::string& Value)
	{
		return !Value.empty() && Value.size() <= MAX_RESOURCE_ID_BYTES &&
			'/' != Value.front() && std::string::npos == Value.find('\\') &&
			std::string::npos == Value.find(':') &&
			std::string::npos == Value.find("../") &&
			std::string::npos == Value.find("/..");
	};
	const auto ShapeForRenderer = [](const EFFECT_RENDERER_TYPE eType)
		-> std::string_view
	{
		switch (eType)
		{
		case EFFECT_RENDERER_TYPE::MESH_PARTICLE: return "mesh";
		case EFFECT_RENDERER_TYPE::SPRITE_PARTICLE: return "sprite";
		case EFFECT_RENDERER_TYPE::DECAL_PARTICLE: return "decal";
		case EFFECT_RENDERER_TYPE::CASCADE_RIBBON: return "ribbon";
		case EFFECT_RENDERER_TYPE::LIGHT_PARTICLE: return "light";
		case EFFECT_RENDERER_TYPE::SCREEN_POST: return "screenPost";
		default: return {};
		}
	};

	const auto IsFiniteFloat3 = [](const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	};
	const auto IsFiniteFloat4 = [](const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	};
	const auto IsZeroFloat4 = [](const float4_t& Value)
	{
		return Value.x == 0.f && Value.y == 0.f && Value.z == 0.f &&
			Value.w == 0.f;
	};
	const auto IsBlockedAdmission = [](const EFFECT_SOURCE_ADMISSION_DESC& Admission)
	{
		std::unordered_set<std::string> UniqueBlockers;
		return !Admission.bAllowed && !Admission.Blockers.empty() &&
			std::all_of(Admission.Blockers.begin(), Admission.Blockers.end(),
				[&UniqueBlockers](const std::string& Blocker)
				{
					return !Blocker.empty() && Blocker.size() <= 256u &&
						Has_VisibleCharacter(Blocker) &&
						UniqueBlockers.insert(Blocker).second;
				});
	};
	const auto IncludesBlockers = [](const std::vector<std::string>& Superset,
		const std::vector<std::string>& Subset)
	{
		return std::all_of(Subset.begin(), Subset.end(),
			[&Superset](const std::string& Blocker)
			{
				return std::find(Superset.begin(), Superset.end(), Blocker) !=
					Superset.end();
			});
	};
	const auto ValidateTypedFields = [&](const auto& Fields)
	{
		if (Fields.size() > MAX_SOURCE_TYPED_FIELDS_PER_REFERENCE)
			return false;
		std::unordered_set<std::string> PropertyPaths;
		for (const EFFECT_SOURCE_TYPED_FIELD_DESC& Field : Fields)
		{
			if (Field.strPropertyPath.empty() ||
				Field.strPropertyPath.size() > 512u ||
				!Has_VisibleCharacter(Field.strPropertyPath) ||
				Field.eKind >= EFFECT_SOURCE_TYPED_FIELD_KIND::END ||
				!PropertyPaths.insert(Field.strPropertyPath).second ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER == Field.eKind &&
					!std::isfinite(Field.fNumber)) ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::STRING == Field.eKind &&
					Field.strString.size() > 2048u) ||
				(EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR == Field.eKind &&
					!IsFiniteFloat4(Field.vVector)))
			{
				return false;
			}
		}
		return true;
	};
	constexpr std::array<std::string_view, 6u> CompositionOrder = {
		"carrierGeometryPreScale", "signedParticleScaleRotationLocation",
		"emitterElementTransform", "cueLocalTransform",
		"attachmentSocketOrRoot", "actorWorld"
	};
	std::unordered_set<std::string> ElementIds;
	std::unordered_set<std::string> EvidenceIds;
	std::string strEvidenceArtifactFileSha256;
	std::string strEvidenceArtifactSelfSha256;
	std::string strLocalReferenceFileSha256;
	std::string strLocalReferenceSelfSha256;
	std::string strGeometryFileSha256;
	std::string strGeometrySelfSha256;
	std::unordered_set<std::string> LocalReferenceOccurrenceIds;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Is_StableId(Element.strElementId) ||
			!ElementIds.insert(Element.strElementId).second ||
			Element.eKind >= EFFECT_ELEMENT_KIND::END ||
			Element.eCompositionLayer != EFFECT_COMPOSITION_LAYER::NORMAL ||
			!Element.Detail.Decal.Is_ReceiverDefault() ||
			Element.Renderer.eType >= EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace >= EFFECT_SOURCE_SPACE::END ||
			Kind_ForRenderer(Element.Renderer.eType) != Element.eKind)
		{
			strOutError = "Source-contract Element identity or renderer is invalid.";
			return false;
		}
		const EFFECT_SOURCE_SPACE eExpectedSpace =
			EFFECT_RENDERER_TYPE::SCREEN_POST == Element.Renderer.eType ?
				EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
				EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		if (Element.Renderer.eSourceSpace != eExpectedSpace)
		{
			strOutError = "Source-contract renderer source space is inconsistent.";
			return false;
		}
		const f32_t fTransformMotionDuration = Element.Detail.Timing.
			fTransformMotionDurationSeconds;
		if (!std::isfinite(fTransformMotionDuration) ||
			fTransformMotionDuration < 0.f ||
			(fTransformMotionDuration > 0.f &&
			 (!std::isfinite(Element.Detail.Timing.fLifeTimeSeconds) ||
			  fTransformMotionDuration >
				Element.Detail.Timing.fLifeTimeSeconds ||
			  (Element.Renderer.eType !=
				EFFECT_RENDERER_TYPE::STANDALONE_MESH &&
			   Element.Renderer.eType !=
				EFFECT_RENDERER_TYPE::MESH_PARTICLE))))
		{
			strOutError =
				"Source-contract transform motion timing is invalid.";
			return false;
		}
		for (const EFFECT_RESOURCE_BINDING_DESC& Resource :
			Element.ResourceBindings)
		{
			if (!Is_StableId(Resource.strSlotId) ||
				!SafeAssetId(Resource.strAssetId))
			{
				strOutError = "Source-contract resource identity is unsafe.";
				return false;
			}
		}
		if (!Validate_MaterialExecution(Element.Material.Execution,
			strOutError))
		{
			return false;
		}
		if (Element.Material.SourceMaterial.bEnabled ||
			Element.Material.Execution.bEnabled ||
			Element.Material.Execution.bFailClosed)
		{
			strOutError =
				"Source-contract candidates cannot enable runtime SourceMaterial or authored Material execution.";
			return false;
		}

		const EFFECT_CASCADE_RECIPE_DESC& Recipe = Element.SourceRecipe;
		const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& Evidence =
			Recipe.CompilerEvidence;
		if (!Recipe.bEnabled ||
			Recipe.strRendererShape != ShapeForRenderer(Element.Renderer.eType) ||
			Recipe.strSourceContractProfileId != EFFECT_SOURCE_CONTRACT_PROFILE_ID ||
			Recipe.strSourceContractSha256 != EFFECT_SOURCE_CONTRACT_SHA256 ||
			!Is_LowerHexSha256(Recipe.strSourceGraphSha256) ||
			!Is_LowerHexSha256(Recipe.strSourceClosureSha256) ||
			!Is_LowerHexSha256(Recipe.strSourceMaterialClosureSha256) ||
			0u == Recipe.iSourcePeakActiveParticles ||
			Recipe.Modules.empty() ||
			Recipe.Modules.size() > MAX_SOURCE_MODULES_PER_ELEMENT ||
			Recipe.LocalReferenceBindings.size() >
				MAX_SOURCE_LOCAL_REFERENCE_BINDINGS_PER_ELEMENT ||
			Recipe.ModuleCoverage.size() != Recipe.Modules.size())
		{
			strOutError = "Source-contract recipe identity or coverage is invalid.";
			return false;
		}

		const auto SharedSha = [&](const std::string& Value,
			std::string& Shared) -> bool_t
		{
			if (!Is_LowerHexSha256(Value))
				return false;
			if (Shared.empty())
				Shared = Value;
			return Shared == Value;
		};
		if (!SharedSha(Evidence.strArtifactFileSha256,
				strEvidenceArtifactFileSha256) ||
			!SharedSha(Evidence.strArtifactSelfSha256,
				strEvidenceArtifactSelfSha256) ||
			!SharedSha(Evidence.strLocalReferenceClosureFileSha256,
				strLocalReferenceFileSha256) ||
			!SharedSha(Evidence.strLocalReferenceClosureSelfSha256,
				strLocalReferenceSelfSha256) ||
			!SharedSha(Evidence.strGeometryParityFileSha256,
				strGeometryFileSha256) ||
			!SharedSha(Evidence.strGeometryParitySelfSha256,
				strGeometrySelfSha256) ||
			Evidence.strSourceEvidenceStatus.empty() ||
			Evidence.strEvidenceId.empty() ||
			!EvidenceIds.insert(Evidence.strEvidenceId).second ||
			Evidence.strSourceCueId.empty() ||
			Evidence.strSourceOccurrenceId.empty() ||
			Evidence.strSourceSystemId.empty() ||
			Evidence.strSourceEmitterPath.empty() ||
			Evidence.strSourceEmitterNodeId.empty() ||
			Evidence.strLodSelectionPolicy != "FIRST_LOD_ONLY" ||
			Evidence.strSelectedLodPath.empty() ||
			Evidence.strSelectedLodNodeId.empty() ||
			0u != Evidence.iSelectedLodArrayIndex ||
			Evidence.strSelectedLodLevelProvenance.empty() ||
			Evidence.strSelectedLodEnabledProvenance.empty() ||
			Evidence.ModuleReferenceOrder.size() != Recipe.Modules.size() ||
			Evidence.CompositionOrder.size() != CompositionOrder.size() ||
			!std::equal(Evidence.CompositionOrder.begin(),
				Evidence.CompositionOrder.end(), CompositionOrder.begin()) ||
			!IsFiniteFloat3(Evidence.vCueSourcePositionUeUnits) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vPosition) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vRotationDegrees) ||
			!IsFiniteFloat3(Evidence.CueLocalTransform.vScale))
		{
			strOutError = "Source-contract compiler evidence is invalid.";
			return false;
		}
		for (size_t iReference = 0u;
			iReference < Evidence.ModuleReferenceOrder.size(); ++iReference)
		{
			const EFFECT_SOURCE_MODULE_REFERENCE_DESC& Reference =
				Evidence.ModuleReferenceOrder[iReference];
			const std::string strExpectedStableId = Reference.strSourceObjectId +
				"@ref:" + std::to_string(Reference.iSourceReferenceIndex);
			if (Reference.iOrder != iReference ||
				Reference.iSourceReferenceIndex != iReference ||
				(Reference.strRole != "REQUIRED" &&
					Reference.strRole != "MODULE" &&
					Reference.strRole != "SPAWN" &&
					Reference.strRole != "TYPE_DATA") ||
				Reference.strSourceObjectId.empty() ||
				!Is_LowerHexSha256(Reference.strSourceRecordSha256) ||
				Recipe.Modules[iReference].strStableId != strExpectedStableId)
			{
				strOutError = "Source-contract module reference order is invalid.";
				return false;
			}
		}
		std::unordered_set<uint32_t> ParameterIndices;
		for (const EFFECT_SOURCE_PARAMETER_OVERRIDE_DESC& Parameter :
			Evidence.ParameterOverrides)
		{
			if (!ParameterIndices.insert(Parameter.iSourceIndex).second ||
				Parameter.strName.empty() ||
				(Parameter.strType != "scalar" && Parameter.strType != "vector") ||
				(Parameter.strType == "scalar" &&
					!std::isfinite(Parameter.fScalarValue)) ||
				(Parameter.strType == "vector" &&
					!IsFiniteFloat3(Parameter.vVectorValue)))
			{
				strOutError = "Source-contract cue parameter evidence is invalid.";
				return false;
			}
		}

		if (!IsBlockedAdmission(Recipe.CompiledExecutionAdmission))
		{
			strOutError = "Source-contract compiled execution is not fail-closed.";
			return false;
		}

		std::unordered_map<std::string,
			const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC*>
			LocalReferenceBindingsByOccurrence;
		std::unordered_set<std::string> DistributionBindingOccurrenceIds;
		for (const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding :
			Recipe.LocalReferenceBindings)
		{
			const bool_t bDistributionReference =
				Binding.strReferenceKind == "DISTRIBUTION_TARGET";
			const bool_t bTypedDataReference =
				Binding.strReferenceKind == "TYPEDATA_COMPONENT";
			if ((!bDistributionReference && !bTypedDataReference) ||
				Binding.strReferenceId.empty() ||
				Binding.strReferenceId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strReferenceId) ||
				Binding.strDefinitionId.empty() ||
				Binding.strDefinitionId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strDefinitionId) ||
				Binding.strOccurrenceId.empty() ||
				Binding.strOccurrenceId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strOccurrenceId) ||
				Binding.strModuleStableId.empty() ||
				Binding.strModuleStableId.size() > 512u ||
				!Has_VisibleCharacter(Binding.strModuleStableId) ||
				Binding.strPropertyPath.empty() ||
				Binding.strPropertyPath.size() > 512u ||
				!Has_VisibleCharacter(Binding.strPropertyPath) ||
				Binding.strProvenance.empty() ||
				Binding.strProvenance.size() > 2048u ||
				!Has_VisibleCharacter(Binding.strProvenance) ||
				std::none_of(Recipe.Modules.begin(), Recipe.Modules.end(),
					[&Binding](const EFFECT_SOURCE_MODULE_DESC& Module)
					{
						return Module.strStableId == Binding.strModuleStableId;
					}) ||
				!ValidateTypedFields(Binding.ExactPayload) ||
				!ValidateTypedFields(Binding.CurrentDefaultEvidence) ||
				!IsBlockedAdmission(Binding.ExecutionAdmission) ||
				!IncludesBlockers(Recipe.CompiledExecutionAdmission.Blockers,
					Binding.ExecutionAdmission.Blockers) ||
				!LocalReferenceOccurrenceIds.insert(
					Binding.strOccurrenceId).second ||
				!LocalReferenceBindingsByOccurrence.emplace(
					Binding.strOccurrenceId, &Binding).second)
			{
				strOutError =
					"Source-contract local-reference binding is invalid.";
				return false;
			}
			if (bDistributionReference)
				DistributionBindingOccurrenceIds.insert(Binding.strOccurrenceId);
		}
		const bool_t bNonRenderLight =
			Recipe.MaterialAdmission.strStatus ==
				"NON_RENDER_BUILTIN_NOT_APPLICABLE";
		const bool_t bBlockedMaterial = Recipe.MaterialAdmission.strStatus.starts_with(
			"BLOCKED_");
		if (Recipe.MaterialAdmission.strStatus.empty() ||
			(bBlockedMaterial &&
				(Recipe.MaterialAdmission.Blockers.empty() ||
				 !Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
				 !Recipe.MaterialAdmission.strRenderStateRecipeId.empty())) ||
			(!bBlockedMaterial && !bNonRenderLight &&
				(Recipe.MaterialAdmission.strMaterialRecipeId.empty() ||
				 Recipe.MaterialAdmission.strRenderStateRecipeId.empty() ||
				 !Recipe.MaterialAdmission.Blockers.empty())) ||
			(bNonRenderLight &&
				(Element.Renderer.eType != EFFECT_RENDERER_TYPE::LIGHT_PARTICLE ||
				 !Recipe.MaterialAdmission.Blockers.empty())))
		{
			strOutError = "Source-contract material admission is invalid.";
			return false;
		}

		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			Recipe.GeometryBinding;
		if (Geometry.strReceiptFileSha256 != strGeometryFileSha256 ||
			Geometry.strReceiptSelfSha256 != strGeometrySelfSha256 ||
			!std::isfinite(Geometry.fCarrierGeometryPreScale) ||
			Geometry.fCarrierGeometryPreScale <= 0.f)
		{
			strOutError = "Source-contract geometry receipt binding is invalid.";
			return false;
		}
		const auto MeshBinding = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		if (Element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE)
		{
			if (!Geometry.bEnabled || Geometry.strAssetId.empty() ||
				MeshBinding == Element.ResourceBindings.end() ||
				Geometry.strAssetId != MeshBinding->strAssetId ||
				Geometry.strParticleScaleSemantics.empty() ||
				Geometry.strStatus.empty())
			{
				strOutError = "Source-contract Mesh carrier scale contract is invalid.";
				return false;
			}
		}
		else if (Geometry.bEnabled || !Geometry.strAssetId.empty() ||
			std::abs(Geometry.fCarrierGeometryPreScale - 1.f) > 1e-7f ||
			Geometry.strParticleScaleSemantics != "NOT_APPLICABLE" ||
			Geometry.strStatus != "NOT_APPLICABLE" || !Geometry.Blockers.empty())
		{
			strOutError = "Source-contract non-Mesh geometry binding is invalid.";
			return false;
		}
		std::unordered_map<std::string,
			const EFFECT_SOURCE_MODULE_COVERAGE_DESC*> CoverageById;
		for (const EFFECT_SOURCE_MODULE_COVERAGE_DESC& Coverage :
			Recipe.ModuleCoverage)
		{
			std::unordered_set<std::string> CoverageBlockers;
			const bool_t bBlockersValid = std::all_of(
				Coverage.Blockers.begin(), Coverage.Blockers.end(),
				[&CoverageBlockers](const std::string& Blocker)
				{
					return !Blocker.empty() &&
						CoverageBlockers.insert(Blocker).second;
				});
			if (Coverage.strModuleStableId.empty() ||
				Coverage.strModuleStableId.size() > 512u ||
				!Has_VisibleCharacter(Coverage.strModuleStableId) ||
				(!Coverage.strExactSourceClass.empty() &&
					(Coverage.strExactSourceClass.size() > 256u ||
					 !Has_VisibleCharacter(Coverage.strExactSourceClass))) ||
				(Coverage.strExactSourceClass.empty() &&
					!Coverage.strAliasId.empty()) ||
				Coverage.strAliasId.size() > 256u ||
				(!Coverage.strAliasId.empty() &&
					!Has_VisibleCharacter(Coverage.strAliasId)) ||
				Coverage.strNormalizedClass.empty() ||
				Coverage.eStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END ||
				!bBlockersValid ||
				(Coverage.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
					Coverage.Blockers.empty()) ||
				!IncludesBlockers(
					Recipe.CompiledExecutionAdmission.Blockers, Coverage.Blockers) ||
				Coverage.Properties.size() >
					MAX_SOURCE_COVERAGE_PROPERTIES_PER_MODULE ||
				!CoverageById.emplace(
					Coverage.strModuleStableId, &Coverage).second)
			{
				strOutError = "Source-contract module coverage is invalid.";
				return false;
			}
		}

		std::unordered_set<std::string>
			ConsumedDistributionBindingOccurrenceIds;
		for (const EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
		{
			const auto CoverageIterator = CoverageById.find(Module.strStableId);
			if (Module.strStableId.empty() || Module.strStableId.size() > 512u ||
				!Has_VisibleCharacter(Module.strStableId) || Module.strClassName.empty() ||
				Module.strObjectPath.empty() ||
				CoverageIterator == CoverageById.end() ||
				(!CoverageIterator->second->strExactSourceClass.empty() &&
					CoverageIterator->second->strExactSourceClass !=
						Module.strClassName) ||
				CoverageIterator->second->strNormalizedClass !=
					(CoverageIterator->second->strExactSourceClass.empty() ?
						Normalize_SourceModuleClass(Module.strClassName) :
						Canonicalize_ExactSourceModuleClass(
							Module.strClassName)))
			{
				strOutError = "Source-contract module identity is invalid.";
				return false;
			}

			std::unordered_set<std::string> CoveredProperties;
			std::unordered_map<std::string,
				const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC*>
				CoveragePropertiesByKey;
			for (const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property :
				CoverageIterator->second->Properties)
			{
				std::unordered_set<std::string> PropertyBlockers;
				const bool_t bPropertyBlockersValid = std::all_of(
					Property.Blockers.begin(), Property.Blockers.end(),
					[&PropertyBlockers](const std::string& Blocker)
					{
						return !Blocker.empty() && Blocker.size() <= 256u &&
							Has_VisibleCharacter(Blocker) &&
							PropertyBlockers.insert(Blocker).second;
					});
				const std::string strCoverageKey = Property.strStorage + "\n" +
					Property.strPropertyPath;
				if ((Property.strStorage != "literal" &&
						Property.strStorage != "distribution") ||
					Property.strPropertyPath.empty() ||
					Property.strPropertyPath.size() > 512u ||
					Property.strProvenance.empty() ||
					Property.strProvenance.size() > 2048u ||
					Property.eStatus >= EFFECT_SOURCE_COVERAGE_STATUS::END ||
					!bPropertyBlockersValid ||
					(Property.eStatus == EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED &&
						Property.Blockers.empty()) ||
					!IncludesBlockers(
						CoverageIterator->second->Blockers, Property.Blockers) ||
					!CoveredProperties.insert(strCoverageKey).second ||
					!CoveragePropertiesByKey.emplace(
						strCoverageKey, &Property).second)
				{
					strOutError =
						"Source-contract property coverage is invalid.";
					return false;
				}
			}
			for (const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& Binding :
				Recipe.LocalReferenceBindings)
			{
				if (Binding.strModuleStableId != Module.strStableId)
					continue;
				const auto BindingCoverage = std::find_if(
					CoverageIterator->second->Properties.begin(),
					CoverageIterator->second->Properties.end(),
					[&Binding](
						const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& Property)
					{
						const std::string_view ExpectedStorage =
							Binding.strReferenceKind == "DISTRIBUTION_TARGET" ?
								"distribution" : "literal";
						return Property.strStorage == ExpectedStorage &&
							Property.strPropertyPath == Binding.strPropertyPath;
					});
				if (BindingCoverage ==
						CoverageIterator->second->Properties.end() ||
					!IncludesBlockers(BindingCoverage->Blockers,
						Binding.ExecutionAdmission.Blockers))
				{
					strOutError =
						"Source local-reference blockers are not propagated.";
					return false;
				}
			}

			std::unordered_set<std::string> SourceProperties;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath.empty() ||
					Literal.eKind >= EFFECT_SOURCE_LITERAL_KIND::END ||
					(EFFECT_SOURCE_LITERAL_KIND::NUMBER == Literal.eKind &&
						!std::isfinite(Literal.fNumber)) ||
					!SourceProperties.insert(
						"literal\n" + Literal.strPropertyPath).second)
				{
					strOutError = "Source-contract literal is invalid or duplicated.";
					return false;
				}
			}
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				const std::string strCoverageKey = "distribution\n" +
					Distribution.strPropertyPath;
				const auto PropertyCoverageIterator =
					CoveragePropertiesByKey.find(strCoverageKey);
				const bool_t bHasReference =
					!Distribution.strReferenceId.empty();
				const bool_t bHasOccurrence =
					!Distribution.strOccurrenceId.empty();
				if (Distribution.strPropertyPath.empty() ||
					Distribution.strPropertyPath.size() > 256u ||
					Distribution.strSourceClass.size() > 128u ||
					Distribution.strSourceObjectPath.size() > 512u ||
					Distribution.iComponentCount < 1u ||
					Distribution.iComponentCount > 4u ||
					Distribution.strPayloadStatus.empty() ||
					Distribution.strPayloadStatus.size() > 256u ||
					!Has_VisibleCharacter(Distribution.strPayloadStatus) ||
					Distribution.strFidelity.empty() ||
					Distribution.strFidelity.size() > 256u ||
					!Has_VisibleCharacter(Distribution.strFidelity) ||
					bHasReference != bHasOccurrence ||
					!IsBlockedAdmission(Distribution.ExecutionAdmission) ||
					PropertyCoverageIterator == CoveragePropertiesByKey.end() ||
					!IncludesBlockers(
						PropertyCoverageIterator->second->Blockers,
						Distribution.ExecutionAdmission.Blockers) ||
					!SourceProperties.insert(strCoverageKey).second)
				{
					strOutError =
						"Source-contract distribution admission is invalid.";
					return false;
				}

				const bool_t bUnresolved =
					Is_UnresolvedSourceToken(Distribution.strPayloadStatus) ||
					Is_UnresolvedSourceToken(Distribution.strFidelity);
				if (bUnresolved &&
					PropertyCoverageIterator->second->eStatus !=
						EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED)
				{
					strOutError =
						"Unresolved source distribution coverage is not unresolved.";
					return false;
				}
				if (bUnresolved &&
					(Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					 !Distribution.strParameterName.empty() ||
					 0u != Distribution.iOperation ||
					 0u != Distribution.iRandomLockAxes ||
					 0u != Distribution.iLookupTableChunkSize ||
					 0u != Distribution.iLookupTableNumElements ||
					 0.f != Distribution.fLookupTableTimeScale ||
					 0.f != Distribution.fLookupTableStartTime ||
					 !IsZeroFloat4(Distribution.vDefaultMinimum) ||
					 !IsZeroFloat4(Distribution.vDefaultMaximum) ||
					 !Distribution.LookupTable.empty() ||
					 !Distribution.Keys.empty()))
				{
					strOutError =
						"Unresolved source distribution carries executable payload.";
					return false;
				}

				if (bHasOccurrence)
				{
					const auto BindingIterator =
						LocalReferenceBindingsByOccurrence.find(
							Distribution.strOccurrenceId);
					if (BindingIterator ==
							LocalReferenceBindingsByOccurrence.end() ||
						BindingIterator->second->strReferenceKind !=
							"DISTRIBUTION_TARGET" ||
						BindingIterator->second->strReferenceId !=
							Distribution.strReferenceId ||
						BindingIterator->second->strOccurrenceId !=
							Distribution.strOccurrenceId ||
						BindingIterator->second->strModuleStableId !=
							Module.strStableId ||
						BindingIterator->second->strPropertyPath !=
							Distribution.strPropertyPath ||
						!IncludesBlockers(
							BindingIterator->second->ExecutionAdmission.Blockers,
							Distribution.ExecutionAdmission.Blockers) ||
						!IncludesBlockers(
							Distribution.ExecutionAdmission.Blockers,
							BindingIterator->second->ExecutionAdmission.Blockers) ||
						!IncludesBlockers(
							PropertyCoverageIterator->second->Blockers,
							BindingIterator->second->ExecutionAdmission.Blockers) ||
						!ConsumedDistributionBindingOccurrenceIds.insert(
							Distribution.strOccurrenceId).second)
					{
						strOutError =
							"Source distribution local-reference link is invalid.";
						return false;
					}
				}

				if (!bUnresolved &&
					!CEffectDistribution::Validate(Distribution, strOutError))
				{
					return false;
				}
				const bool_t bParticleParameter =
					Is_ParticleParameterDistribution(Distribution.strSourceClass);
				if ((bParticleParameter &&
						Distribution.eParameterBinding >=
							EFFECT_DISTRIBUTION_PARAMETER_BINDING::END) ||
					(!bParticleParameter &&
						(Distribution.eParameterBinding !=
							EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
						 !Distribution.strParameterName.empty())))
				{
					strOutError =
						"Source-contract ParticleParameter binding is invalid.";
					return false;
				}
			}

			if (SourceProperties != CoveredProperties)
			{
				strOutError =
					"Source-contract property coverage does not match source payload.";
				return false;
			}
		}
		if (ConsumedDistributionBindingOccurrenceIds !=
			DistributionBindingOccurrenceIds)
		{
			strOutError =
				"Source-contract distribution bindings are orphaned or missing.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Validate_Drawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	const bool_t bHasVisibleElement = std::any_of(
		Document.Elements.begin(), Document.Elements.end(),
		[](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.bVisible &&
				(Is_EffectAuthoringExecutionTarget(
					Element.Material.Execution) ||
				 Is_EffectPresentationExecutionTarget(Element));
		});
	const bool_t bHasVisibleModelCue = std::any_of(
		Document.ModelCues.begin(), Document.ModelCues.end(),
		[](const EFFECT_MODEL_CUE_DESC& Cue)
		{
			return Cue.bVisible;
		});
	if (!bHasVisibleElement && !bHasVisibleModelCue)
	{
		strOutError =
			"Effect has no visible Element or Model / Summon to preview.";
		return false;
	}
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible ||
			(!Is_EffectAuthoringExecutionTarget(Element.Material.Execution) &&
			 !Is_EffectPresentationExecutionTarget(Element)))
			continue;
		if (EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
			EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			continue;
		}
		const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
			Find_EffectMaterialTemplate(Element.Material.strTemplateId);
		const EFFECT_MATERIAL_INPUT_SLOT_DESC* pBaseInput =
			nullptr == pTemplate ? nullptr : Find_EffectMaterialInput(
				*pTemplate, EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		const auto FindBinding = [&](const EFFECT_MATERIAL_INPUT_SEMANTIC eSemantic)
			-> const EFFECT_RESOURCE_BINDING_DESC*
		{
			if (nullptr == pTemplate)
				return nullptr;
			const EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
				Find_EffectMaterialInput(*pTemplate, eSemantic);
			if (nullptr == pInput)
				return nullptr;
			const auto Iterator = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[pInput](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == pInput->strSlotId;
				});
			return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
		};
		const EFFECT_RESOURCE_BINDING_DESC* pBaseBinding = FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::BASE);
		bool_t bSourceMaterialOwnsDrawableContract = false;
		if (!Validate_ExecutableSourceMaterialCarrier(
				Element, bSourceMaterialOwnsDrawableContract, strOutError))
		{
			return false;
		}
		const bool_t bMaterialOwnsDrawableContract =
			Element.Material.Execution.bEnabled ||
			bSourceMaterialOwnsDrawableContract;
		const bool_t bParticleMesh =
			EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const std::string_view strRequiredSlotId =
			EFFECT_ELEMENT_KIND::MESH == Element.eKind || bParticleMesh ?
			EFFECT_MESH_SHAPE_SLOT_ID :
			(nullptr == pBaseInput ? std::string_view{} :
				pBaseInput->strSlotId);
		const bool_t bBound = std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[strRequiredSlotId](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == strRequiredSlotId;
			});
		const bool_t bMeshElement =
			EFFECT_ELEMENT_KIND::MESH == Element.eKind || bParticleMesh;
		if ((strRequiredSlotId.empty() || !bBound) &&
			!(!bMeshElement && bMaterialOwnsDrawableContract))
		{
			strOutError = bMeshElement ?
				"Mesh or mesh-backed Particle requires a Mesh Model binding." :
				"Sprite/Particle/Decal/Trail Element requires a Base texture binding.";
			return false;
		}
		if (bMeshElement &&
			!Element.Detail.Mesh.bUseModelMaterial)
		{
			if (nullptr == pBaseBinding && !bMaterialOwnsDrawableContract)
			{
				strOutError = "Mesh Element with useModelMaterial=false requires a Base texture binding.";
				return false;
			}
		}
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Build_DuplicatedAuthoredElements(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::vector<std::string>& SourceElementIds,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::unordered_map<std::string, std::string>& OutDuplicatedElementIds,
	std::string& strOutError)
{
	if (SourceDocument.bSourceContract || SourceElementIds.empty())
	{
		strOutError = "Duplicate requires an authored Effect and selected Element IDs.";
		return false;
	}
	if (!Validate(SourceDocument, strOutError))
		return false;

	std::unordered_set<std::string> Targets;
	for (const std::string& ElementId : SourceElementIds)
	{
		if (!Is_StableId(ElementId) || !Targets.insert(ElementId).second)
		{
			strOutError = "Duplicate rejected an invalid or repeated selected Element ID.";
			return false;
		}
	}

	std::unordered_set<std::string> UsedIds;
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
		UsedIds.insert(Element.strElementId);
	std::unordered_map<std::string, std::string> DuplicateIds;
	const std::string Prefix = "authored.copy.";
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		if (!Targets.contains(Element.strElementId))
			continue;
		if (Element.eKind == EFFECT_ELEMENT_KIND::LIGHT ||
			Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST)
		{
			strOutError = "Presentation Light and Screen Post duplication is not admitted.";
			return false;
		}
		std::string DuplicateId;
		for (size_t iCopy = 1u; iCopy <= UsedIds.size() + 1u; ++iCopy)
		{
			const std::string Suffix = "." + std::to_string(iCopy);
			const size_t iMaximumSourceLength = 128u - Prefix.size() - Suffix.size();
			DuplicateId = Prefix +
				Element.strElementId.substr(0u, iMaximumSourceLength) + Suffix;
			if (!UsedIds.contains(DuplicateId))
				break;
			DuplicateId.clear();
		}
		if (DuplicateId.empty())
		{
			strOutError = "Duplicate could not allocate a unique authored Element ID.";
			return false;
		}
		UsedIds.insert(DuplicateId);
		DuplicateIds.emplace(Element.strElementId, std::move(DuplicateId));
	}
	if (DuplicateIds.size() != Targets.size())
	{
		strOutError = "A selected Element no longer exists; nothing was duplicated.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = SourceDocument;
	Staged.Elements.clear();
	Staged.Elements.reserve(SourceDocument.Elements.size() + DuplicateIds.size());
	for (const EFFECT_ELEMENT_DESC& Element : SourceDocument.Elements)
	{
		Staged.Elements.push_back(Element);
		const auto CopyId = DuplicateIds.find(Element.strElementId);
		if (CopyId == DuplicateIds.end())
			continue;
		EFFECT_ELEMENT_DESC Duplicate = Element;
		Duplicate.strElementId = CopyId->second;
		Duplicate.strSourceNode = std::string(EFFECT_PORTABLE_AUTHORED_COPY_PREFIX) +
			std::string(Resolve_EffectPortableOriginElementId(Element));
		Duplicate.SourcePresentation = {};
		if (Duplicate.TransformInheritance.bEnabled)
		{
			const auto MasterCopy = DuplicateIds.find(
				Duplicate.TransformInheritance.strMasterElementId);
			if (MasterCopy != DuplicateIds.end())
				Duplicate.TransformInheritance.strMasterElementId = MasterCopy->second;
		}
		Staged.Elements.push_back(std::move(Duplicate));
	}
	if (!Validate(Staged, strOutError))
		return false;
	InOutDocument = std::move(Staged);
	OutDuplicatedElementIds = std::move(DuplicateIds);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string_view strElementId,
	const std::string_view strTargetEffectAssetId,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (strElementId.empty() || strTargetEffectAssetId.empty())
	{
		strOutError =
			"Generic authored starting copy requires stable source and target IDs.";
		return false;
	}
	const auto First = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (First == SourceDocument.Elements.end() ||
		std::find_if(std::next(First), SourceDocument.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Generic authored starting copy requires exactly one source Element.";
		return false;
	}
	if (First->eKind != EFFECT_ELEMENT_KIND::MESH &&
		First->eKind != EFFECT_ELEMENT_KIND::SPRITE &&
		First->eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		First->eKind != EFFECT_ELEMENT_KIND::DECAL &&
		First->eKind != EFFECT_ELEMENT_KIND::TRAIL)
	{
		strOutError =
			"Generic authored starting copy supports Mesh, Sprite, Particle, Decal, and Trail Elements only.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Candidate;
	Candidate.strEffectAssetId = std::string(strTargetEffectAssetId);
	Candidate.strDisplayName = First->strDisplayName;
	Candidate.ParticleSystem = SourceDocument.ParticleSystem;
	Candidate.Elements.push_back(*First);
	EFFECT_ELEMENT_DESC& Lowered = Candidate.Elements.front();
	const bool_t bSourceMeshParticle =
		Lowered.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Lowered.SourceRecipe.bEnabled &&
		Lowered.SourceRecipe.strRendererShape == "mesh" &&
		std::any_of(Lowered.ResourceBindings.begin(),
			Lowered.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
	EFFECT_DISTRIBUTION_DESC SourceStartSize;
	bool_t bHasSourceStartSize = false;
	size_t iSourceStartSizeCandidateCount = 0u;
	if (bSourceMeshParticle)
	{
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Lowered.SourceRecipe.Modules)
		{
			std::string_view SourceClass = Module.strClassName;
			if (SourceClass.starts_with("efparticlemodule"))
				SourceClass.remove_prefix(2u);
			if (SourceClass.ends_with("_seeded"))
				SourceClass.remove_suffix(7u);
			if (SourceClass != "particlemodulesize")
				continue;
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				if (Distribution.strPropertyPath == "startsize")
				{
					++iSourceStartSizeCandidateCount;
					std::string DistributionError;
					if (1u == iSourceStartSizeCandidateCount &&
						Distribution.iComponentCount >= 2u &&
						Distribution.iOperation == 1u &&
						CEffectDistribution::Validate(
							Distribution, DistributionError))
					{
						SourceStartSize = Distribution;
						bHasSourceStartSize = true;
					}
				}
			}
		}
		/* Multiple or random StartSize modules require the portable source
		   executor's ordered composition.  The generic direct carrier cannot
		   reproduce that composition after SourceRecipe is removed, so keep the
		   existing fallback unchanged instead of guessing. */
		if (1u != iSourceStartSizeCandidateCount)
			bHasSourceStartSize = false;
	}
	/* The document already provides the effect-level namespace.  Keep the
	   composite group stable and bounded even when the target asset ID is at
	   the 128-byte contract limit. */
	Lowered.strGroupId = "manual.authoring";
	Lowered.strSourceNode.clear();
	Lowered.Renderer = {};
	Lowered.ActionCueAttachment = {};
	Lowered.TransformInheritance = {};
	Lowered.SourceRecipe = {};
	Lowered.SourcePresentation = {};
	Lowered.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
	if (bHasSourceStartSize)
	{
		/* Older Track A projections stored the direct Mesh Particle fallback in
		   the WModel carrier's geometry unit even though portable SourceRecipe
		   playback consumes StartSize as a dimensionless instance scale.  Once
		   the recipe is removed, preserving that 0.01-scaled fallback would apply
		   modelPreScale a second time and make the copied mesh about 100x too
		   small.  Compare against the immutable source StartSize distribution so
		   already-normalized documents (for example Artist F) remain unchanged. */
		const float4_t SourceSize = CEffectDistribution::Evaluate(
			SourceStartSize, 0.f, 0.5f);
		const f32_t fModelPreScale = Lowered.Detail.Mesh.fModelPreScale;
		const auto NearlyEqualRelative = [](const f32_t A, const f32_t B)
		{
			const f32_t fTolerance = (std::max)(
				1.0e-5f, (std::max)(std::abs(A), std::abs(B)) * 1.0e-4f);
			return std::abs(A - B) <= fTolerance;
		};
		const float2_t SourceDimensionless = {
			std::abs(SourceSize.x), std::abs(SourceSize.y) };
		const float2_t DirectStart = Lowered.Detail.Particle.vStartSize;
		const bool_t bLegacyGeometryScaledSize =
			std::isfinite(fModelPreScale) && fModelPreScale > 0.f &&
			fModelPreScale < 1.f &&
			SourceDimensionless.x > 0.f && SourceDimensionless.y > 0.f &&
			NearlyEqualRelative(
				DirectStart.x, SourceDimensionless.x * fModelPreScale) &&
			NearlyEqualRelative(
				DirectStart.y, SourceDimensionless.y * fModelPreScale) &&
			(!NearlyEqualRelative(DirectStart.x, SourceDimensionless.x) ||
			 !NearlyEqualRelative(DirectStart.y, SourceDimensionless.y));
		if (bLegacyGeometryScaledSize)
		{
			const f32_t fDimensionlessSizeScale = 1.f / fModelPreScale;
			Lowered.Detail.Particle.vStartSize.x *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vStartSize.y *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vEndSize.x *= fDimensionlessSizeScale;
			Lowered.Detail.Particle.vEndSize.y *= fDimensionlessSizeScale;
		}
	}
	if (bSourceMeshParticle)
	{
		/* SourceScale is consumed only by SourceRecipe playback.  Bake its size
		   authority into the direct fallback even when no unambiguous StartSize
		   distribution was available, then neutralize it before the recipe is
		   discarded. */
		const f32_t fSourceSizeScale =
			Lowered.Detail.Particle.SourceScale.fSize;
		if (std::isfinite(fSourceSizeScale) && fSourceSizeScale > 0.f &&
			fSourceSizeScale != 1.f)
		{
			Lowered.Detail.Particle.vStartSize.x *= fSourceSizeScale;
			Lowered.Detail.Particle.vStartSize.y *= fSourceSizeScale;
			Lowered.Detail.Particle.vEndSize.x *= fSourceSizeScale;
			Lowered.Detail.Particle.vEndSize.y *= fSourceSizeScale;
			Lowered.Detail.Particle.SourceScale.fSize = 1.f;
		}
	}
	if (Lowered.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Lowered.Detail.Particle.fSpawnRatePerSecond <= 0.f &&
		0u == Lowered.Detail.Particle.iBurstCount &&
		Lowered.Detail.Particle.fFixedCenterSpacingWorldUnits <= 0.f)
	{
		/* Source-authored Particle carriers may receive every occurrence from
		   SourceRecipe while their editable direct emission stays at 0/0.  The
		   generic copy intentionally drops that compiler/source ownership, so
		   give the lowered authoring carrier the same bounded one-shot default
		   used by Create Element instead of admitting a drawable that can never
		   instantiate a Sprite/Mesh Particle. */
		Lowered.Detail.Particle.iMaxParticles = (std::max)(
			1u, Lowered.Detail.Particle.iMaxParticles);
		Lowered.Detail.Particle.iBurstCount = 1u;
	}
	const auto IsZeroFloat3 = [](const float3_t& Value)
	{
		return Value.x == 0.f && Value.y == 0.f && Value.z == 0.f;
	};
	const EFFECT_LINEAR_LERP_DESC& LinearLerp = Lowered.Detail.LinearLerp;
	if (Lowered.eKind == EFFECT_ELEMENT_KIND::TRAIL &&
		IsZeroFloat3(Lowered.Detail.Transform.vVelocityPerSecond) &&
		IsZeroFloat3(Lowered.Detail.Transform.vRevolutionDegreesPerSecond) &&
		!LinearLerp.bPosition && !LinearLerp.bRotation &&
		!LinearLerp.bRevolution && !LinearLerp.bScale &&
		!LinearLerp.bVelocity)
	{
		/* A Trail needs at least two distinct carrier samples to become
		   visible.  Source attachment/occurrence motion was intentionally
		   removed above, so seed a bounded authoring-only carrier velocity
		   only when the selected recipe has no surviving motion of its own. */
		Lowered.Detail.Transform.vVelocityPerSecond = { 0.f, 0.f, 1.f };
	}

	/* The ordinary serializer deliberately omits native Renderer, compiler,
	   geometry, admission, and distribution-evidence lanes.  The explicit
	   clearing above also removes source occurrence/attachment ownership while
	   preserving WModel/DDS bindings, an already compiled authored Material
	   execution snapshot, and editable Detail values.  No source receipt or
	   adapter lookup remains necessary after that snapshot exists. */
	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Staged.Elements.size() != 1u ||
		Staged.Elements.front().strElementId != strElementId ||
		Staged.Elements.front().eKind != First->eKind ||
		Staged.Elements.front().Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Staged.Elements.front().Renderer.eSourceSpace !=
			EFFECT_SOURCE_SPACE::END ||
		!Staged.Elements.front().strSourceNode.empty() ||
		Staged.Elements.front().ActionCueAttachment.bEnabled ||
		Staged.Elements.front().TransformInheritance.bEnabled ||
		Staged.Elements.front().SourceRecipe.bEnabled ||
		Staged.Elements.front().SourcePresentation.bEnabled ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored starting copy did not survive ordinary codec validation exactly.";
		}
		return false;
	}
	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Build_PortableAuthoredElementStartingCopy(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string_view strElementId,
	const std::string_view strTargetEffectAssetId,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (SourceDocument.bSourceContract)
	{
		strOutError =
			"Portable authored Saved Element copy accepts only an admitted ordinary v13 authored document, not native source-contract evidence.";
		return false;
	}
	const auto Source = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (Source == SourceDocument.Elements.end() ||
		std::find_if(std::next(Source), SourceDocument.Elements.end(),
			[strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Portable authored Saved Element copy requires exactly one source Element.";
		return false;
	}
	if ((Source->eKind != EFFECT_ELEMENT_KIND::MESH &&
		 Source->eKind != EFFECT_ELEMENT_KIND::SPRITE &&
		 Source->eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		 Source->eKind != EFFECT_ELEMENT_KIND::DECAL &&
		 Source->eKind != EFFECT_ELEMENT_KIND::TRAIL) ||
		!Is_EffectAuthoringExecutionTarget(Source->Material.Execution))
	{
		strOutError =
			"Portable authored Saved Element copy requires a self-contained executable Mesh, Sprite, Particle, Decal, or Trail Element.";
		return false;
	}
	if (!Source->bVisible)
	{
		strOutError =
			"Portable authored Saved Element copy cannot turn an invisible source Element into a visible occurrence; load the complete Effect instead.";
		return false;
	}
	if (Source->eKind == EFFECT_ELEMENT_KIND::TRAIL)
	{
		/* A Trail is geometry made from more than one owner/animation transform
		   history sample.  Copying its editable material and width fields cannot
		   make that history portable, even when the row has no explicit source
		   recipe.  Keep this a single fail-closed boundary instead of inventing a
		   second synthetic-motion Trail Family for Saved Element reuse. */
		strOutError =
			"Portable authored Saved Element copy cannot detach Trail transform history; load the complete Effect instead.";
		return false;
	}
	if (Source->TransformInheritance.bEnabled)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach a transform-inheritance dependent from its master; load the complete Effect instead.";
		return false;
	}
	if (Source->SourcePresentation.bEnabled)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach SourcePresentation occurrence ownership; load the complete Effect instead.";
		return false;
	}
	if (Source->Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Source->Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END)
	{
		strOutError =
			"Portable authored Saved Element copy cannot detach a native Renderer carrier; load the complete source-program Effect instead.";
		return false;
	}
	const bool_t bDetachNativeOwnerYawSprite =
		Source->ActionCueAttachment.bEnabled &&
		Source->ActionCueAttachment.bFollow &&
		Source->ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW &&
		Source->eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
		Source->Detail.Particle.bBillboard &&
		(Source->strSourceNode.empty() ||
		 Source->strSourceNode.starts_with("authored-copy:")) &&
		!Source->SourceRecipe.bEnabled && Source->RuntimeCarrier.Is_Empty() &&
		Source->Material.strTemplateId == EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
		Source->Material.strSourceMaterialPath.empty() &&
		!Source->Material.SourceMaterial.bEnabled &&
		!Source->Material.Execution.bEnabled &&
		!Source->Material.Execution.bFailClosed &&
		!Source->Material.Execution.bAuthoringApproximate &&
		std::none_of(Source->ResourceBindings.begin(), Source->ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{ return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID; });
	if (bDetachNativeOwnerYawSprite && !Validate(SourceDocument, strOutError))
		return false;
	if (Source->ActionCueAttachment.bFollow && !bDetachNativeOwnerYawSprite)
	{
		/* A matching textual owner prefix does not prove that the target preview
		   owns the source bone, clip, or fixed-step anchor history.  Snapshot/root
		   attachments remain portable; source-owned FOLLOW rows require the
		   complete Effect. Native owner-yaw sprite emitters copy only their
		   authored settings and must be attached again in the target Effect. */
		strOutError =
			"Portable authored Saved Element copy cannot detach a FOLLOW attachment from its owner animation history; load the complete Effect instead.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Candidate;
	Candidate.strEffectAssetId = std::string(strTargetEffectAssetId);
	Candidate.strDisplayName = Source->strDisplayName;
	Candidate.ParticleSystem = SourceDocument.ParticleSystem;
	Candidate.Elements.push_back(*Source);
	EFFECT_ELEMENT_DESC& Portable = Candidate.Elements.front();
	/* Saved Element reuse strips only native compiler/occurrence ownership.
	   Unlike the generic fallback path, it must never validate an intermediate
	   recipe-less Particle because StandardColor and other executable materials
	   require their renderer Family recipe as part of material admission. */
	Portable.strGroupId = "manual.authoring";
	Portable.Renderer = {};
	Portable.TransformInheritance = {};
	Portable.SourceRecipe = {};
	Portable.SourcePresentation = {};
	/* Reassert the complete editable payload in one assignment so size,
	   SourceScale, TypeData rotation, timing, color, motion, and every future
	   Effect Detail field cannot drift when this boundary evolves. */
	Portable.Detail = Source->Detail;
	Portable.ActionCueAttachment = Source->ActionCueAttachment;
	if (bDetachNativeOwnerYawSprite)
		Portable.ActionCueAttachment = {};
	constexpr std::string_view AuthoredCopyPrefix = "authored-copy:";
	Portable.strSourceNode =
		Source->strSourceNode.starts_with(AuthoredCopyPrefix) &&
		Source->strSourceNode.size() > AuthoredCopyPrefix.size() ?
			Source->strSourceNode :
			std::string(AuthoredCopyPrefix) + Source->strElementId;

	if (Source->SourceRecipe.bEnabled)
	{
		bool_t bCarrierApplied = false;
		if (EFFECT_ELEMENT_KIND::PARTICLE == Source->eKind)
		{
			bCarrierApplied = Apply_PortableAuthoredParticleRuntimeCarrier(
				*Source, Portable, strOutError);
		}
		else if (EFFECT_ELEMENT_KIND::DECAL == Source->eKind)
		{
			bCarrierApplied = Apply_PortableAuthoredDecalRuntimeCarrier(
				*Source, Portable, strOutError);
		}
		else
		{
			strOutError =
				"Portable authored Saved Element copy has no self-contained runtime carrier for this sourceRecipe Family.";
			return false;
		}
		if (!bCarrierApplied)
			return false;
		/* Generic import bakes emitter delay into its sampled starting state, but
		   Saved Element reuse does not sample or bake. Preserve the source delay
		   so Detail.startDelay + recipe.emitterDelay remains exactly unchanged. */
		Portable.SourceRecipe.fEmitterDelaySeconds =
			Source->SourceRecipe.fEmitterDelaySeconds;
	}

	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate(Staged, strOutError) ||
		Staged.Elements.size() != 1u ||
		Staged.Elements.front().strElementId != strElementId ||
		Staged.Elements.front().eKind != Source->eKind ||
		Staged.Elements.front().Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		Staged.Elements.front().Renderer.eSourceSpace !=
			EFFECT_SOURCE_SPACE::END ||
		Staged.Elements.front().TransformInheritance.bEnabled ||
		Staged.Elements.front().SourcePresentation.bEnabled ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Portable authored Saved Element copy did not survive ordinary codec validation exactly.";
		}
		return false;
	}
	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

void Client::CEffectDocumentCodec::Record_AuthoringResourceOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string& strSlotId,
	const std::string& strAssetId,
	const std::string& strCompilerAssetId)
{
	auto Existing = std::find_if(
		Element.AuthoringOverrides.ResourceBindings.begin(),
		Element.AuthoringOverrides.ResourceBindings.end(),
		[&strSlotId](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Row)
		{
			return Row.strSlotId == strSlotId;
		});
	if (Existing != Element.AuthoringOverrides.ResourceBindings.end())
	{
		// Re-binding an already overridden slot must not move the reset
		// target: the compiler baseline stays whatever the compiler produced.
		if (Existing->strCompilerAssetId == strAssetId)
			Element.AuthoringOverrides.ResourceBindings.erase(Existing);
		else
			Existing->strAssetId = strAssetId;
		return;
	}
	if (strCompilerAssetId == strAssetId)
		return;
	Element.AuthoringOverrides.ResourceBindings.push_back(
		{ strSlotId, strAssetId, strCompilerAssetId });
}

bool_t Client::CEffectDocumentCodec::Set_AuthoringResourceOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strSlotId,
	const std::string_view strAssetId,
	std::string& strOutError)
{
	strOutError.clear();
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strStableSlotId(strSlotId);
	const std::string strArtistAssetId(strAssetId);
	AUTHORING_RESOURCE_TARGET Target;
	if (Resolve_AuthoringResourceTarget(Staged, strStableSlotId, Target) !=
		AUTHORING_OVERRIDE_TARGET_STATUS::FOUND)
	{
		strOutError =
			"Authoring resource override target is missing, invalid, or ambiguous: " +
			strStableSlotId;
		return false;
	}
	EFFECT_RESOURCE_FILE_KIND eArtistKind = EFFECT_RESOURCE_FILE_KIND::END;
	if (strArtistAssetId.empty() ||
		!Is_SafeElementResourceAssetId(Staged.eKind, strStableSlotId,
			strArtistAssetId, &eArtistKind) ||
		eArtistKind != Target.eExpectedKind)
	{
		strOutError =
			"Authoring resource override asset path or kind is invalid: " +
			strStableSlotId;
		return false;
	}

	auto Existing = std::find_if(
		Staged.AuthoringOverrides.ResourceBindings.begin(),
		Staged.AuthoringOverrides.ResourceBindings.end(),
		[&strStableSlotId](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Row)
		{
			return Row.strSlotId == strStableSlotId;
		});
	if (Existing != Staged.AuthoringOverrides.ResourceBindings.end() &&
		std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.ResourceBindings.end(),
			[&strStableSlotId](
				const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Row)
			{
				return Row.strSlotId == strStableSlotId;
			}) != Staged.AuthoringOverrides.ResourceBindings.end())
	{
		strOutError =
			"Authoring resource override target is duplicated: " +
			strStableSlotId;
		return false;
	}

	const std::string strEffectiveBefore =
		Get_AuthoringResourceTargetAssetId(Staged, Target);
	std::string strCompilerAssetId = strEffectiveBefore;
	if (Existing != Staged.AuthoringOverrides.ResourceBindings.end())
	{
		if (Existing->strAssetId != strEffectiveBefore ||
			Existing->strAssetId == Existing->strCompilerAssetId)
		{
			strOutError =
				"Authoring resource override metadata is inconsistent with its effective target: " +
				strStableSlotId;
			return false;
		}
		strCompilerAssetId = Existing->strCompilerAssetId;
	}
	if (!strCompilerAssetId.empty())
	{
		EFFECT_RESOURCE_FILE_KIND eCompilerKind = EFFECT_RESOURCE_FILE_KIND::END;
		if (!Is_SafeElementResourceAssetId(Staged.eKind, strStableSlotId,
				strCompilerAssetId, &eCompilerKind) ||
			eCompilerKind != Target.eExpectedKind)
		{
			strOutError =
				"Authoring resource override compiler baseline path or kind is invalid: " +
				strStableSlotId;
			return false;
		}
	}

	Get_AuthoringResourceTargetAssetId(Staged, Target) = strArtistAssetId;
	if (strArtistAssetId == strCompilerAssetId)
	{
		if (Existing != Staged.AuthoringOverrides.ResourceBindings.end())
			Staged.AuthoringOverrides.ResourceBindings.erase(Existing);
	}
	else if (Existing != Staged.AuthoringOverrides.ResourceBindings.end())
	{
		Existing->strAssetId = strArtistAssetId;
	}
	else
	{
		Staged.AuthoringOverrides.ResourceBindings.push_back(
			{ strStableSlotId, strArtistAssetId, strCompilerAssetId });
	}
	Element = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentCodec::Reset_AuthoringResourceOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strSlotId,
	std::string& strOutError)
{
	strOutError.clear();
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strStableSlotId(strSlotId);
	AUTHORING_RESOURCE_TARGET Target;
	if (Resolve_AuthoringResourceTarget(Staged, strStableSlotId, Target) !=
		AUTHORING_OVERRIDE_TARGET_STATUS::FOUND)
	{
		strOutError =
			"Authoring resource reset target is missing, invalid, or ambiguous: " +
			strStableSlotId;
		return false;
	}
	auto Existing = std::find_if(
		Staged.AuthoringOverrides.ResourceBindings.begin(),
		Staged.AuthoringOverrides.ResourceBindings.end(),
		[&strStableSlotId](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Row)
		{
			return Row.strSlotId == strStableSlotId;
		});
	if (Existing == Staged.AuthoringOverrides.ResourceBindings.end())
		return true;
	if (std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.ResourceBindings.end(),
			[&strStableSlotId](
				const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Row)
			{
				return Row.strSlotId == strStableSlotId;
			}) != Staged.AuthoringOverrides.ResourceBindings.end() ||
		Existing->strAssetId !=
			Get_AuthoringResourceTargetAssetId(Staged, Target))
	{
		strOutError =
			"Authoring resource reset metadata is duplicated or inconsistent: " +
			strStableSlotId;
		return false;
	}
	if (!Existing->strCompilerAssetId.empty())
	{
		EFFECT_RESOURCE_FILE_KIND eCompilerKind = EFFECT_RESOURCE_FILE_KIND::END;
		if (!Is_SafeElementResourceAssetId(Staged.eKind, strStableSlotId,
				Existing->strCompilerAssetId, &eCompilerKind) ||
			eCompilerKind != Target.eExpectedKind)
		{
			strOutError =
				"Authoring resource reset compiler baseline path or kind is invalid: " +
				strStableSlotId;
			return false;
		}
	}
	Get_AuthoringResourceTargetAssetId(Staged, Target) =
		Existing->strCompilerAssetId;
	Staged.AuthoringOverrides.ResourceBindings.erase(Existing);
	Element = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentCodec::Set_AuthoringScalarOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strName,
	const f32_t fValue,
	std::string& strOutError)
{
	strOutError.clear();
	if (!std::isfinite(fValue))
	{
		strOutError = "Authoring scalar override value must be finite.";
		return false;
	}
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strParameterName(strName);
	AUTHORING_SCALAR_TARGET Target;
	AUTHORING_COLOR_TARGET WrongTypeTarget;
	const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
		Resolve_AuthoringScalarTarget(Staged, strParameterName, Target);
	const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
		Resolve_AuthoringColorTarget(
			Staged, strParameterName, WrongTypeTarget);
	if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
	{
		strOutError =
			"Authoring scalar override target is missing, ambiguous, or declared as another type: " +
			strParameterName;
		return false;
	}
	if (std::any_of(Staged.AuthoringOverrides.Colors.begin(),
		Staged.AuthoringOverrides.Colors.end(),
		[&strParameterName](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		}))
	{
		strOutError =
			"Authoring scalar override collides with a color override: " +
			strParameterName;
		return false;
	}
	auto Existing = std::find_if(Staged.AuthoringOverrides.Scalars.begin(),
		Staged.AuthoringOverrides.Scalars.end(),
		[&strParameterName](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		});
	if (Existing != Staged.AuthoringOverrides.Scalars.end() &&
		std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.Scalars.end(),
			[&strParameterName](
				const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Row)
			{
				return Row.strName == strParameterName;
			}) != Staged.AuthoringOverrides.Scalars.end())
	{
		strOutError =
			"Authoring scalar override target is duplicated: " +
			strParameterName;
		return false;
	}
	if (!Is_AuthoringScalarTargetConsistent(Staged, Target))
	{
		strOutError =
			"Authoring scalar override compiler/effective mirrors disagree: " +
			strParameterName;
		return false;
	}
	const f32_t fEffectiveBefore =
		Get_AuthoringScalarTargetValue(Staged, Target);
	f32_t fCompilerValue = fEffectiveBefore;
	if (Existing != Staged.AuthoringOverrides.Scalars.end())
	{
		if (!std::isfinite(Existing->fValue) ||
			!std::isfinite(Existing->fCompilerValue) ||
			Existing->fValue != fEffectiveBefore ||
			Existing->fValue == Existing->fCompilerValue)
		{
			strOutError =
				"Authoring scalar override metadata is inconsistent with its effective target: " +
				strParameterName;
			return false;
		}
		fCompilerValue = Existing->fCompilerValue;
	}
	Set_AuthoringScalarTargetValue(Staged, Target, fValue);
	if (fValue == fCompilerValue)
	{
		if (Existing != Staged.AuthoringOverrides.Scalars.end())
			Staged.AuthoringOverrides.Scalars.erase(Existing);
	}
	else if (Existing != Staged.AuthoringOverrides.Scalars.end())
	{
		Existing->fValue = fValue;
	}
	else
	{
		Staged.AuthoringOverrides.Scalars.push_back(
			{ strParameterName, fValue, fCompilerValue });
	}
	Element = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentCodec::Reset_AuthoringScalarOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strName,
	std::string& strOutError)
{
	strOutError.clear();
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strParameterName(strName);
	AUTHORING_SCALAR_TARGET Target;
	AUTHORING_COLOR_TARGET WrongTypeTarget;
	const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
		Resolve_AuthoringScalarTarget(Staged, strParameterName, Target);
	const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
		Resolve_AuthoringColorTarget(
			Staged, strParameterName, WrongTypeTarget);
	if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
	{
		strOutError =
			"Authoring scalar reset target is missing, ambiguous, or declared as another type: " +
			strParameterName;
		return false;
	}
	if (!Is_AuthoringScalarTargetConsistent(Staged, Target))
	{
		strOutError =
			"Authoring scalar reset compiler/effective mirrors disagree: " +
			strParameterName;
		return false;
	}
	auto Existing = std::find_if(Staged.AuthoringOverrides.Scalars.begin(),
		Staged.AuthoringOverrides.Scalars.end(),
		[&strParameterName](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		});
	if (Existing == Staged.AuthoringOverrides.Scalars.end())
		return true;
	if (std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.Scalars.end(),
			[&strParameterName](
				const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Row)
			{
				return Row.strName == strParameterName;
			}) != Staged.AuthoringOverrides.Scalars.end() ||
		!std::isfinite(Existing->fValue) ||
		!std::isfinite(Existing->fCompilerValue) ||
		Existing->fValue != Get_AuthoringScalarTargetValue(Staged, Target))
	{
		strOutError =
			"Authoring scalar reset metadata is duplicated or inconsistent: " +
			strParameterName;
		return false;
	}
	Set_AuthoringScalarTargetValue(Staged, Target, Existing->fCompilerValue);
	Staged.AuthoringOverrides.Scalars.erase(Existing);
	Element = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentCodec::Set_AuthoringColorOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strName,
	const float4_t& vValue,
	std::string& strOutError)
{
	strOutError.clear();
	if (!Is_Finite(vValue))
	{
		strOutError = "Authoring color override value must be finite.";
		return false;
	}
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strParameterName(strName);
	AUTHORING_COLOR_TARGET Target;
	AUTHORING_SCALAR_TARGET WrongTypeTarget;
	const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
		Resolve_AuthoringColorTarget(Staged, strParameterName, Target);
	const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
		Resolve_AuthoringScalarTarget(
			Staged, strParameterName, WrongTypeTarget);
	if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
	{
		strOutError =
			"Authoring color override target is missing, ambiguous, or declared as another type: " +
			strParameterName;
		return false;
	}
	if (std::any_of(Staged.AuthoringOverrides.Scalars.begin(),
		Staged.AuthoringOverrides.Scalars.end(),
		[&strParameterName](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		}))
	{
		strOutError =
			"Authoring color override collides with a scalar override: " +
			strParameterName;
		return false;
	}
	auto Existing = std::find_if(Staged.AuthoringOverrides.Colors.begin(),
		Staged.AuthoringOverrides.Colors.end(),
		[&strParameterName](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		});
	if (Existing != Staged.AuthoringOverrides.Colors.end() &&
		std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.Colors.end(),
			[&strParameterName](
				const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Row)
			{
				return Row.strName == strParameterName;
			}) != Staged.AuthoringOverrides.Colors.end())
	{
		strOutError =
			"Authoring color override target is duplicated: " +
			strParameterName;
		return false;
	}
	if (!Is_AuthoringColorTargetConsistent(Staged, Target))
	{
		strOutError =
			"Authoring color override compiler/effective mirrors disagree: " +
			strParameterName;
		return false;
	}
	const float4_t vEffectiveBefore =
		Get_AuthoringColorTargetValue(Staged, Target);
	float4_t vCompilerValue = vEffectiveBefore;
	if (Existing != Staged.AuthoringOverrides.Colors.end())
	{
		if (!Is_Finite(Existing->vValue) ||
			!Is_Finite(Existing->vCompilerValue) ||
			!Same_Float4(Existing->vValue, vEffectiveBefore) ||
			Same_Float4(Existing->vValue, Existing->vCompilerValue))
		{
			strOutError =
				"Authoring color override metadata is inconsistent with its effective target: " +
				strParameterName;
			return false;
		}
		vCompilerValue = Existing->vCompilerValue;
	}
	Set_AuthoringColorTargetValue(Staged, Target, vValue);
	if (Same_Float4(vValue, vCompilerValue))
	{
		if (Existing != Staged.AuthoringOverrides.Colors.end())
			Staged.AuthoringOverrides.Colors.erase(Existing);
	}
	else if (Existing != Staged.AuthoringOverrides.Colors.end())
	{
		Existing->vValue = vValue;
	}
	else
	{
		Staged.AuthoringOverrides.Colors.push_back(
			{ strParameterName, vValue, vCompilerValue });
	}
	Element = std::move(Staged);
	return true;
}

bool_t Client::CEffectDocumentCodec::Reset_AuthoringColorOverride(
	EFFECT_ELEMENT_DESC& Element,
	const std::string_view strName,
	std::string& strOutError)
{
	strOutError.clear();
	EFFECT_ELEMENT_DESC Staged = Element;
	const std::string strParameterName(strName);
	AUTHORING_COLOR_TARGET Target;
	AUTHORING_SCALAR_TARGET WrongTypeTarget;
	const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
		Resolve_AuthoringColorTarget(Staged, strParameterName, Target);
	const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
		Resolve_AuthoringScalarTarget(
			Staged, strParameterName, WrongTypeTarget);
	if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
		eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS)
	{
		strOutError =
			"Authoring color reset target is missing, ambiguous, or declared as another type: " +
			strParameterName;
		return false;
	}
	if (!Is_AuthoringColorTargetConsistent(Staged, Target))
	{
		strOutError =
			"Authoring color reset compiler/effective mirrors disagree: " +
			strParameterName;
		return false;
	}
	auto Existing = std::find_if(Staged.AuthoringOverrides.Colors.begin(),
		Staged.AuthoringOverrides.Colors.end(),
		[&strParameterName](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Row)
		{
			return Row.strName == strParameterName;
		});
	if (Existing == Staged.AuthoringOverrides.Colors.end())
		return true;
	if (std::find_if(std::next(Existing),
			Staged.AuthoringOverrides.Colors.end(),
			[&strParameterName](
				const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Row)
			{
				return Row.strName == strParameterName;
			}) != Staged.AuthoringOverrides.Colors.end() ||
		!Is_Finite(Existing->vValue) ||
		!Is_Finite(Existing->vCompilerValue) ||
		!Same_Float4(Existing->vValue,
			Get_AuthoringColorTargetValue(Staged, Target)))
	{
		strOutError =
			"Authoring color reset metadata is duplicated or inconsistent: " +
			strParameterName;
		return false;
	}
	Set_AuthoringColorTargetValue(Staged, Target, Existing->vCompilerValue);
	Staged.AuthoringOverrides.Colors.erase(Existing);
	Element = std::move(Staged);
	return true;
}


bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementReimportStage(
	const EFFECT_DOCUMENT_DESC& CompilerDocument,
	const EFFECT_DOCUMENT_DESC& ExistingDocument,
	const EFFECT_GENERIC_AUTHORED_ELEMENT_REIMPORT_REQUEST& Request,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError,
	EFFECT_GENERIC_AUTHORED_REIMPORT_REPORT* pOutReport)
{
	strOutError.clear();
	EFFECT_GENERIC_AUTHORED_REIMPORT_REPORT StagedReport;
	if (Request.strElementId.empty() ||
		CompilerDocument.strEffectAssetId.empty() ||
		CompilerDocument.strEffectAssetId != ExistingDocument.strEffectAssetId ||
		CompilerDocument.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		CompilerDocument.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		CompilerDocument.bSourceContract ||
		ExistingDocument.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		ExistingDocument.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		ExistingDocument.bSourceContract)
	{
		strOutError =
			"Generic authored reimport requires matching ordinary v13 Effect identities.";
		return false;
	}

	const std::string CompilerCanonicalBefore = Serialize(CompilerDocument);
	const std::string ExistingCanonicalBefore = Serialize(ExistingDocument);
	EFFECT_DOCUMENT_DESC CanonicalCompiler;
	EFFECT_DOCUMENT_DESC CanonicalExisting;
	if (!Parse(CompilerCanonicalBefore, CanonicalCompiler, strOutError) ||
		!Validate(CanonicalCompiler, strOutError) ||
		Serialize(CanonicalCompiler) != CompilerCanonicalBefore ||
		!Parse(ExistingCanonicalBefore, CanonicalExisting, strOutError) ||
		!Validate(CanonicalExisting, strOutError) ||
		Serialize(CanonicalExisting) != ExistingCanonicalBefore)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored reimport inputs did not survive canonical validation.";
		}
		return false;
	}

	const auto FindUniqueElement = [&Request](EFFECT_DOCUMENT_DESC& Document,
		EFFECT_ELEMENT_DESC*& pOutElement)
	{
		pOutElement = nullptr;
		for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId != Request.strElementId)
				continue;
			if (nullptr != pOutElement)
				return false;
			pOutElement = &Element;
		}
		return nullptr != pOutElement;
	};
	EFFECT_ELEMENT_DESC* pCompilerElement = nullptr;
	EFFECT_ELEMENT_DESC* pExistingElement = nullptr;
	if (!FindUniqueElement(CanonicalCompiler, pCompilerElement) ||
		!FindUniqueElement(CanonicalExisting, pExistingElement))
	{
		strOutError =
			"Generic authored reimport requires exactly one compiler and target Element join.";
		return false;
	}
	if (pCompilerElement->eKind != pExistingElement->eKind ||
		(pCompilerElement->eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
		 pCompilerElement->eKind != EFFECT_ELEMENT_KIND::DECAL) ||
		!pCompilerElement->AuthoringOverrides.Is_Empty() ||
		!pCompilerElement->SourceRecipe.bEnabled ||
		pCompilerElement->Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		pCompilerElement->Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
		!pCompilerElement->strSourceNode.empty() ||
		pCompilerElement->ActionCueAttachment.bEnabled ||
		pCompilerElement->TransformInheritance.bEnabled ||
		pCompilerElement->SourcePresentation.bEnabled)
	{
		strOutError =
			"Generic authored reimport compiler Element is not an ordinary Particle/Decal carrier.";
		return false;
	}

	const auto FindUniqueBinding = [](const EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId,
		const EFFECT_RESOURCE_FILE_KIND eExpectedKind,
		const EFFECT_RESOURCE_BINDING_DESC*& pOutBinding)
	{
		pOutBinding = nullptr;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			if (Binding.strSlotId != strSlotId)
				continue;
			if (nullptr != pOutBinding)
				return false;
			EFFECT_RESOURCE_FILE_KIND eActualKind =
				EFFECT_RESOURCE_FILE_KIND::END;
			if (!CEffectDocumentCodec::Is_SafeElementResourceAssetId(
					Element.eKind, strSlotId, Binding.strAssetId,
					&eActualKind) ||
				eActualKind != eExpectedKind)
			{
				return false;
			}
			pOutBinding = &Binding;
		}
		return nullptr != pOutBinding;
	};

	const EFFECT_RESOURCE_BINDING_DESC* pCompilerMesh = nullptr;
	const EFFECT_RESOURCE_BINDING_DESC* pCompilerBase = nullptr;
	if (pCompilerElement->eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		const bool_t bMesh =
			pCompilerElement->SourceRecipe.strRendererShape == "mesh";
		const bool_t bSprite =
			pCompilerElement->SourceRecipe.strRendererShape == "sprite";
		if ((!bMesh && !bSprite) ||
			(bMesh && !FindUniqueBinding(*pCompilerElement,
				EFFECT_MESH_SHAPE_SLOT_ID,
				EFFECT_RESOURCE_FILE_KIND::MODEL, pCompilerMesh)) ||
			(bSprite && !FindUniqueBinding(*pCompilerElement,
				EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId,
				EFFECT_RESOURCE_FILE_KIND::TEXTURE, pCompilerBase)))
		{
			strOutError = bMesh ?
				"Generic authored reimport Mesh Particle has a missing, ambiguous, or unsafe WModel binding." :
				"Generic authored reimport Sprite Particle has a missing, ambiguous, or unsafe Base DDS binding.";
			return false;
		}
	}
	else if (pCompilerElement->SourceRecipe.strRendererShape != "decal")
	{
		strOutError =
			"Generic authored reimport Decal source recipe shape is invalid.";
		return false;
	}
	else
	{
		/* Validate() above already rejects duplicate, unsafe, or wrong-kind
		   resource slots.  Base is optional for a hidden Decal draft: source
		   DDS may seed it when available, while a source-missing row remains
		   editable until the artist assigns diffuse in the Effect Tool. */
		const auto FindOptionalBase = [](const EFFECT_ELEMENT_DESC& Element)
			-> const EFFECT_RESOURCE_BINDING_DESC*
		{
			const auto Iterator = std::find_if(
				Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId ==
						EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId;
				});
			return Iterator == Element.ResourceBindings.end() ?
				nullptr : &*Iterator;
		};
		pCompilerBase = FindOptionalBase(*pCompilerElement);
		if (nullptr == pCompilerBase && pExistingElement->bVisible)
		{
			strOutError =
				"Generic authored reimport refuses a visible Decal when the compiler no longer produces its Base DDS binding.";
			return false;
		}
	}

	EFFECT_ELEMENT_DESC Reimported = *pCompilerElement;
	Reimported.strElementId = pExistingElement->strElementId;
	Reimported.strDisplayName = pExistingElement->strDisplayName;
	Reimported.strGroupId = pExistingElement->strGroupId;
	Reimported.bVisible = pExistingElement->bVisible;
	Reimported.eCompositionLayer = pExistingElement->eCompositionLayer;
	Reimported.ActionCueAttachment = pExistingElement->ActionCueAttachment;
	Reimported.TransformInheritance = pExistingElement->TransformInheritance;
	Reimported.Detail = pExistingElement->Detail;
	Reimported.AuthoringOverrides = pExistingElement->AuthoringOverrides;
	if (Reimported.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		Reimported.SourceRecipe = {};
		if (!Apply_PortableAuthoredParticleRuntimeCarrier(
				*pCompilerElement, Reimported, strOutError))
		{
			return false;
		}
	}

	/* Re-apply artist overrides last, on top of the refreshed compiler stage.
	   Only a target that genuinely vanished is dropped. Invalid identifiers,
	   ambiguous declarations, wrong types and unsafe values fail the complete
	   transaction without changing either input or the prior output. */
	{
		std::vector<EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC>
			SurvivingResources;
		for (EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override :
			Reimported.AuthoringOverrides.ResourceBindings)
		{
			AUTHORING_RESOURCE_TARGET Target;
			const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
				Resolve_AuthoringResourceTarget(
					Reimported, Override.strSlotId, Target);
			if (eStatus == AUTHORING_OVERRIDE_TARGET_STATUS::MISSING)
			{
				StagedReport.DroppedOverrides.push_back({
					EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_KIND::RESOURCE,
					EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_REASON::
						RESOURCE_SLOT_VANISHED,
					Override.strSlotId });
				continue;
			}
			if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND)
			{
				strOutError =
					"Generic authored reimport resource override target is invalid or ambiguous: " +
					Override.strSlotId;
				return false;
			}
			EFFECT_RESOURCE_FILE_KIND eArtistKind =
				EFFECT_RESOURCE_FILE_KIND::END;
			if (Override.strAssetId.empty() ||
				!Is_SafeElementResourceAssetId(Reimported.eKind,
					Override.strSlotId, Override.strAssetId, &eArtistKind) ||
				eArtistKind != Target.eExpectedKind)
			{
				strOutError =
					"Generic authored reimport resource override path or kind is invalid: " +
					Override.strSlotId;
				return false;
			}
			const std::string strCompilerAssetId =
				Get_AuthoringResourceTargetAssetId(Reimported, Target);
			if (!strCompilerAssetId.empty())
			{
				EFFECT_RESOURCE_FILE_KIND eCompilerKind =
					EFFECT_RESOURCE_FILE_KIND::END;
				if (!Is_SafeElementResourceAssetId(Reimported.eKind,
						Override.strSlotId, strCompilerAssetId,
						&eCompilerKind) ||
					eCompilerKind != Target.eExpectedKind)
				{
					strOutError =
						"Generic authored reimport compiler resource baseline path or kind is invalid: " +
						Override.strSlotId;
					return false;
				}
			}
			if (Override.strAssetId == strCompilerAssetId)
				continue;
			Override.strCompilerAssetId = strCompilerAssetId;
			Get_AuthoringResourceTargetAssetId(Reimported, Target) =
				Override.strAssetId;
			SurvivingResources.push_back(Override);
		}
		Reimported.AuthoringOverrides.ResourceBindings =
			std::move(SurvivingResources);

		std::vector<EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC> SurvivingScalars;
		for (EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Override :
			Reimported.AuthoringOverrides.Scalars)
		{
			AUTHORING_SCALAR_TARGET Target;
			const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
				Resolve_AuthoringScalarTarget(
					Reimported, Override.strName, Target);
			if (eStatus == AUTHORING_OVERRIDE_TARGET_STATUS::MISSING)
			{
				AUTHORING_COLOR_TARGET WrongTypeTarget;
				const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
					Resolve_AuthoringColorTarget(
						Reimported, Override.strName, WrongTypeTarget);
				if (eWrongTypeStatus ==
						AUTHORING_OVERRIDE_TARGET_STATUS::MISSING)
				{
					StagedReport.DroppedOverrides.push_back({
						EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_KIND::SCALAR,
						EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_REASON::
							SCALAR_PARAMETER_VANISHED,
						Override.strName });
					continue;
				}
				strOutError =
					"Generic authored reimport scalar override target changed declared type: " +
					Override.strName;
				return false;
			}
			AUTHORING_COLOR_TARGET WrongTypeTarget;
			const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
				Resolve_AuthoringColorTarget(
					Reimported, Override.strName, WrongTypeTarget);
			if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS ||
				!std::isfinite(Override.fValue))
			{
				strOutError =
					"Generic authored reimport scalar override target or value is invalid: " +
					Override.strName;
				return false;
			}
			if (!Is_AuthoringScalarTargetConsistent(Reimported, Target))
			{
				strOutError =
					"Generic authored reimport scalar compiler baselines disagree across effective mirrors: " +
					Override.strName;
				return false;
			}
			const f32_t fCompilerValue =
				Get_AuthoringScalarTargetValue(Reimported, Target);
			if (!std::isfinite(fCompilerValue))
			{
				strOutError =
					"Generic authored reimport scalar compiler baseline is not finite: " +
					Override.strName;
				return false;
			}
			if (Override.fValue == fCompilerValue)
				continue;
			Override.fCompilerValue = fCompilerValue;
			Set_AuthoringScalarTargetValue(
				Reimported, Target, Override.fValue);
			SurvivingScalars.push_back(Override);
		}
		Reimported.AuthoringOverrides.Scalars = std::move(SurvivingScalars);

		std::vector<EFFECT_AUTHORING_COLOR_OVERRIDE_DESC> SurvivingColors;
		for (EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Override :
			Reimported.AuthoringOverrides.Colors)
		{
			AUTHORING_COLOR_TARGET Target;
			const AUTHORING_OVERRIDE_TARGET_STATUS eStatus =
				Resolve_AuthoringColorTarget(
					Reimported, Override.strName, Target);
			if (eStatus == AUTHORING_OVERRIDE_TARGET_STATUS::MISSING)
			{
				AUTHORING_SCALAR_TARGET WrongTypeTarget;
				const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
					Resolve_AuthoringScalarTarget(
						Reimported, Override.strName, WrongTypeTarget);
				if (eWrongTypeStatus ==
						AUTHORING_OVERRIDE_TARGET_STATUS::MISSING)
				{
					StagedReport.DroppedOverrides.push_back({
						EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_KIND::COLOR,
						EFFECT_GENERIC_AUTHORED_REIMPORT_DROP_REASON::
							COLOR_PARAMETER_VANISHED,
						Override.strName });
					continue;
				}
				strOutError =
					"Generic authored reimport color override target changed declared type: " +
					Override.strName;
				return false;
			}
			AUTHORING_SCALAR_TARGET WrongTypeTarget;
			const AUTHORING_OVERRIDE_TARGET_STATUS eWrongTypeStatus =
				Resolve_AuthoringScalarTarget(
					Reimported, Override.strName, WrongTypeTarget);
			if (eStatus != AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::FOUND ||
				eWrongTypeStatus == AUTHORING_OVERRIDE_TARGET_STATUS::AMBIGUOUS ||
				!Is_Finite(Override.vValue))
			{
				strOutError =
					"Generic authored reimport color override target or value is invalid: " +
					Override.strName;
				return false;
			}
			if (!Is_AuthoringColorTargetConsistent(Reimported, Target))
			{
				strOutError =
					"Generic authored reimport color compiler baselines disagree across effective mirrors: " +
					Override.strName;
				return false;
			}
			const float4_t vCompilerValue =
				Get_AuthoringColorTargetValue(Reimported, Target);
			if (!Is_Finite(vCompilerValue))
			{
				strOutError =
					"Generic authored reimport color compiler baseline is not finite: " +
					Override.strName;
				return false;
			}
			if (Same_Float4(Override.vValue, vCompilerValue))
				continue;
			Override.vCompilerValue = vCompilerValue;
			Set_AuthoringColorTargetValue(
				Reimported, Target, Override.vValue);
			SurvivingColors.push_back(Override);
		}
		Reimported.AuthoringOverrides.Colors = std::move(SurvivingColors);
	}

	EFFECT_DOCUMENT_DESC Candidate = CanonicalExisting;
	EFFECT_ELEMENT_DESC* pCandidateElement = nullptr;
	if (!FindUniqueElement(Candidate, pCandidateElement))
	{
		strOutError =
			"Generic authored reimport lost its target during staging.";
		return false;
	}
	*pCandidateElement = std::move(Reimported);
	if (!Validate(Candidate, strOutError))
		return false;

	const std::string CandidateCanonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(CandidateCanonical, Staged, strOutError) ||
		!Validate(Staged, strOutError) ||
		Serialize(Staged) != CandidateCanonical ||
		Serialize(CompilerDocument) != CompilerCanonicalBefore ||
		Serialize(ExistingDocument) != ExistingCanonicalBefore)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored reimport did not survive canonical validation without mutating its inputs.";
		}
		return false;
	}
	const auto SameFloat3 = [](const float3_t& Left, const float3_t& Right)
	{
		return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
	};
	const auto SameTransform = [&SameFloat3](
		const EFFECT_TRANSFORM_DESC& Left,
		const EFFECT_TRANSFORM_DESC& Right)
	{
		return SameFloat3(Left.vPosition, Right.vPosition) &&
			SameFloat3(Left.vRotationDegrees, Right.vRotationDegrees) &&
			SameFloat3(Left.vRevolutionDegreesPerSecond,
				Right.vRevolutionDegreesPerSecond) &&
			SameFloat3(Left.vScale, Right.vScale) &&
			SameFloat3(Left.vVelocityPerSecond, Right.vVelocityPerSecond);
	};
	const auto SameAttachment = [&SameTransform](
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Left,
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Right)
	{
		return Left.bEnabled == Right.bEnabled &&
			Left.bFollow == Right.bFollow &&
			Left.eOrientation == Right.eOrientation &&
			Left.strSourceAnchorSlotId == Right.strSourceAnchorSlotId &&
			Left.strRuntimeAnchorSlotId == Right.strRuntimeAnchorSlotId &&
			Left.strRuntimeBoneName == Right.strRuntimeBoneName &&
			Left.fSnapshotRootSourceBasisYawDegrees ==
				Right.fSnapshotRootSourceBasisYawDegrees &&
			SameTransform(Left.SocketLocalTransform,
				Right.SocketLocalTransform);
	};
	EFFECT_ELEMENT_DESC* pStagedElement = nullptr;
	if (!FindUniqueElement(Staged, pStagedElement) ||
		pStagedElement->strDisplayName != pExistingElement->strDisplayName ||
		pStagedElement->strGroupId != pExistingElement->strGroupId ||
		pStagedElement->bVisible != pExistingElement->bVisible ||
		!SameAttachment(pStagedElement->ActionCueAttachment,
			pExistingElement->ActionCueAttachment) ||
		pStagedElement->TransformInheritance.bEnabled !=
			pExistingElement->TransformInheritance.bEnabled ||
		pStagedElement->TransformInheritance.strMasterElementId !=
			pExistingElement->TransformInheritance.strMasterElementId)
	{
		strOutError =
			"Generic authored reimport changed target identity, display metadata, visibility, attachment, or transform inheritance.";
		return false;
	}

	InOutDocument = std::move(Staged);
	if (nullptr != pOutReport)
		*pOutReport = std::move(StagedReport);
	strOutError.clear();
	return true;
}

namespace
{
	constexpr f32_t GENERIC_STARTING_BAKE_AFFINE_EPSILON = 0.00001f;
	constexpr f32_t GENERIC_STARTING_BAKE_MATRIX_EPSILON = 0.0002f;

	bool_t Is_FiniteMatrix(const float4x4_t& Value)
	{
		const f32_t* pComponent = &Value._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(pComponent[i]))
				return false;
		}
		return true;
	}

	bool_t Is_AffineMatrix(const float4x4_t& Value)
	{
		return std::abs(Value._14) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._24) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._34) <= GENERIC_STARTING_BAKE_AFFINE_EPSILON &&
			std::abs(Value._44 - 1.f) <=
				GENERIC_STARTING_BAKE_AFFINE_EPSILON;
	}

	bool_t Matrices_NearlyEqual(const float4x4_t& Left,
		const float4x4_t& Right)
	{
		const f32_t* pLeft = &Left._11;
		const f32_t* pRight = &Right._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			const f32_t fScale = (std::max)(1.f,
				(std::max)(std::abs(pLeft[i]), std::abs(pRight[i])));
			if (std::abs(pLeft[i] - pRight[i]) >
				GENERIC_STARTING_BAKE_MATRIX_EPSILON * fScale)
			{
				return false;
			}
		}
		return true;
	}

	matrix_t Build_AuthoredTransformMatrix(
		const EFFECT_TRANSFORM_DESC& Transform)
	{
		return XMMatrixScaling(
			Transform.vScale.x, Transform.vScale.y, Transform.vScale.z) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(Transform.vRotationDegrees.x),
				XMConvertToRadians(Transform.vRotationDegrees.y),
				XMConvertToRadians(Transform.vRotationDegrees.z)) *
			XMMatrixTranslation(
				Transform.vPosition.x,
				Transform.vPosition.y,
				Transform.vPosition.z);
	}

	matrix_t Build_SourceTypeDataRotation(
		const float3_t& SourceDegrees)
	{
		/* Source TypeDataMesh rotation is UE roll(X), pitch(Y), yaw(Z).
		   Preserve its source Euler composition, then conjugate it into the
		   Client X-forward/Y-up basis exactly as source playback does. */
		const f32_t Roll = XMConvertToRadians(SourceDegrees.x);
		const f32_t Pitch = XMConvertToRadians(SourceDegrees.y);
		const f32_t Yaw = XMConvertToRadians(SourceDegrees.z);
		const f32_t SP = std::sin(Pitch);
		const f32_t CP = std::cos(Pitch);
		const f32_t SY = std::sin(Yaw);
		const f32_t CY = std::cos(Yaw);
		const f32_t SR = std::sin(Roll);
		const f32_t CR = std::cos(Roll);
		const matrix_t Source = XMMatrixSet(
			CP * CY, CP * SY, SP, 0.f,
			SR * SP * CY - CR * SY,
			SR * SP * SY + CR * CY, -SR * CP, 0.f,
			-CR * SP * CY - SR * SY,
			-CR * SP * SY + SR * CY, CR * CP, 0.f,
			0.f, 0.f, 0.f, 1.f);
		const matrix_t Basis = XMMatrixSet(
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
		return XMMatrixTranspose(Basis) * Source * Basis;
	}

	bool_t Decompose_AuthoredTransform(fmatrix_t Matrix,
		EFFECT_TRANSFORM_DESC& OutTransform, std::string& strOutError)
	{
		float4x4_t Stored{};
		XMStoreFloat4x4(&Stored, Matrix);
		if (!Is_FiniteMatrix(Stored) || !Is_AffineMatrix(Stored))
		{
			strOutError =
				"Generic authored starting-state bake requires a finite affine transform.";
			return false;
		}

		const f32_t fDeterminant = XMVectorGetX(XMMatrixDeterminant(Matrix));
		if (!std::isfinite(fDeterminant) || fDeterminant <= 0.f)
		{
			strOutError =
				"Generic authored starting-state bake rejects degenerate or reflected transforms.";
			return false;
		}

		vector_t Scale{};
		vector_t RotationQuaternion{};
		vector_t Translation{};
		if (!XMMatrixDecompose(
				&Scale, &RotationQuaternion, &Translation, Matrix))
		{
			strOutError =
				"Generic authored starting-state bake could not decompose the transform.";
			return false;
		}

		float3_t DecomposedScale{};
		float3_t DecomposedTranslation{};
		XMStoreFloat3(&DecomposedScale, Scale);
		XMStoreFloat3(&DecomposedTranslation, Translation);
		if (!Is_Finite(DecomposedScale) || !Is_Finite(DecomposedTranslation) ||
			DecomposedScale.x <= 0.f || DecomposedScale.y <= 0.f ||
			DecomposedScale.z <= 0.f)
		{
			strOutError =
				"Generic authored starting-state bake requires positive finite scale.";
			return false;
		}

		const matrix_t RotationMatrix =
			XMMatrixRotationQuaternion(XMQuaternionNormalize(RotationQuaternion));
		float4x4_t StoredRotation{};
		XMStoreFloat4x4(&StoredRotation, RotationMatrix);
		const f32_t fCosPitch = std::sqrt(
			StoredRotation._33 * StoredRotation._33 +
			StoredRotation._31 * StoredRotation._31);
		float3_t EulerRadians{};
		EulerRadians.x = std::atan2(-StoredRotation._32, fCosPitch);
		if (fCosPitch > 16.f * (std::numeric_limits<f32_t>::epsilon)())
		{
			EulerRadians.y =
				std::atan2(StoredRotation._31, StoredRotation._33);
			EulerRadians.z =
				std::atan2(StoredRotation._12, StoredRotation._22);
		}
		else
		{
			EulerRadians.y = 0.f;
			EulerRadians.z =
				std::atan2(-StoredRotation._21, StoredRotation._11);
		}

		EFFECT_TRANSFORM_DESC Candidate = OutTransform;
		Candidate.vPosition = DecomposedTranslation;
		Candidate.vRotationDegrees = {
			XMConvertToDegrees(EulerRadians.x),
			XMConvertToDegrees(EulerRadians.y),
			XMConvertToDegrees(EulerRadians.z)
		};
		Candidate.vScale = DecomposedScale;
		float4x4_t Recomposed{};
		XMStoreFloat4x4(&Recomposed,
			Build_AuthoredTransformMatrix(Candidate));
		if (!Is_Finite(Candidate.vRotationDegrees) ||
			!Matrices_NearlyEqual(Stored, Recomposed))
		{
			strOutError =
				"Generic authored starting-state bake rejects shear or a lossy Euler decomposition.";
			return false;
		}
		OutTransform = Candidate;
		return true;
	}
}

bool_t Client::CEffectDocumentCodec::Bake_GenericAuthoredElementStartingState(
	const EFFECT_ELEMENT_DESC& LoweredElement,
	const EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST& Request,
	EFFECT_ELEMENT_DESC& OutBakedElement,
	std::string& strOutError)
{
	if (LoweredElement.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
		LoweredElement.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
		!LoweredElement.strSourceNode.empty() ||
		LoweredElement.ActionCueAttachment.bEnabled ||
		LoweredElement.TransformInheritance.bEnabled ||
		LoweredElement.SourceRecipe.bEnabled ||
		LoweredElement.SourcePresentation.bEnabled ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.x != 0.f ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.y != 0.f ||
		LoweredElement.Detail.Mesh.vSourceTypeDataRotationDegrees.z != 0.f)
	{
		strOutError =
			"Generic authored starting-state bake requires an already lowered ordinary Element.";
		return false;
	}
	if (Request.bTransformInheritanceEnabled)
	{
		strOutError =
			"Generic authored starting-state bake cannot flatten transform inheritance.";
		return false;
	}
	if ((!Request.bAttachmentEnabled &&
			(Request.bFollowAttachment ||
			 Request.bHasFollowParentLocalTransform ||
			 Request.fSnapshotRootSourceBasisYawDegrees != 0.f)) ||
		(!Request.bFollowAttachment &&
			Request.bHasFollowParentLocalTransform) ||
		(Request.bFollowAttachment &&
			(!Request.bHasFollowParentLocalTransform ||
			 Request.fSnapshotRootSourceBasisYawDegrees != 0.f)))
	{
		strOutError = Request.bFollowAttachment ?
			"Generic authored starting-state bake requires the exact emit-start follow parent-local transform." :
			"Generic authored starting-state bake has an invalid attachment request.";
		return false;
	}

	const EFFECT_TRANSFORM_DESC& Cue = Request.CueLocalTransform;
	const EFFECT_TRANSFORM_DESC& EmitterLocal = Request.EmitterLocalTransform;
	if (!Is_Finite(Cue.vPosition) || !Is_Finite(Cue.vRotationDegrees) ||
		!Is_Finite(Cue.vScale) || Cue.vScale.x <= 0.f ||
		Cue.vScale.y <= 0.f || Cue.vScale.z <= 0.f ||
		!Is_Finite(EmitterLocal.vPosition) ||
		!Is_Finite(EmitterLocal.vRotationDegrees) ||
		!Is_Finite(EmitterLocal.vScale) || EmitterLocal.vScale.x <= 0.f ||
		EmitterLocal.vScale.y <= 0.f || EmitterLocal.vScale.z <= 0.f ||
		!Is_Finite(Request.vSourceTypeDataRotationDegrees) ||
		!std::isfinite(Request.fScheduleStartDelaySeconds) ||
		!std::isfinite(Request.fScheduleLifeTimeSeconds) ||
		!std::isfinite(Request.fEmitterDelaySeconds) ||
		!std::isfinite(Request.fEmitterDurationSeconds) ||
		!std::isfinite(Request.fSnapshotRootSourceBasisYawDegrees) ||
		std::abs(Request.fSnapshotRootSourceBasisYawDegrees) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.x) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.y) > 3600.f ||
		std::abs(Request.vSourceTypeDataRotationDegrees.z) > 3600.f ||
		Request.fScheduleStartDelaySeconds < 0.f ||
		Request.fScheduleLifeTimeSeconds <= 0.f ||
		Request.fEmitterDelaySeconds < 0.f ||
		Request.fEmitterDurationSeconds < 0.f)
	{
		strOutError =
			"Generic authored starting-state bake request contains invalid transform or timing values.";
		return false;
	}

	const f32_t fStartDelay = Request.fScheduleStartDelaySeconds +
		Request.fEmitterDelaySeconds;
	const f32_t fLifeTime =
		Request.fEmitterDurationSeconds > 0.f &&
		Request.iEmitterLoopCount != 0u ?
		Request.fEmitterDurationSeconds *
			static_cast<f32_t>(Request.iEmitterLoopCount) :
		Request.fScheduleLifeTimeSeconds;
	if (!std::isfinite(fStartDelay) || !std::isfinite(fLifeTime) ||
		fStartDelay < 0.f || fLifeTime <= 0.f)
	{
		strOutError =
			"Generic authored starting-state bake timing overflows the ordinary contract.";
		return false;
	}

	matrix_t ParentLocal = XMMatrixIdentity();
	if (Request.bAttachmentEnabled)
	{
		if (Request.bFollowAttachment)
		{
			if (!Is_FiniteMatrix(Request.FollowParentLocalTransform) ||
				!Is_AffineMatrix(Request.FollowParentLocalTransform))
			{
				strOutError =
					"Generic authored starting-state bake follow parent is not finite affine.";
				return false;
			}
			ParentLocal = XMLoadFloat4x4(
				&Request.FollowParentLocalTransform);
		}
		else
		{
			ParentLocal = XMMatrixRotationY(XMConvertToRadians(
				Request.fSnapshotRootSourceBasisYawDegrees));
		}
	}

	const matrix_t BakedMatrix =
		Build_SourceTypeDataRotation(
			Request.vSourceTypeDataRotationDegrees) *
		Build_AuthoredTransformMatrix(EmitterLocal) *
		Build_AuthoredTransformMatrix(Cue) * ParentLocal;
	EFFECT_ELEMENT_DESC Candidate = LoweredElement;
	EFFECT_TRANSFORM_DESC BakedTransform = Candidate.Detail.Transform;
	if (!Decompose_AuthoredTransform(
			BakedMatrix, BakedTransform, strOutError))
	{
		return false;
	}
	Candidate.Detail.Transform.vPosition = BakedTransform.vPosition;
	Candidate.Detail.Transform.vRotationDegrees =
		BakedTransform.vRotationDegrees;
	Candidate.Detail.Transform.vScale = BakedTransform.vScale;
	Candidate.Detail.Timing.fStartDelaySeconds = fStartDelay;
	Candidate.Detail.Timing.fLifeTimeSeconds = fLifeTime;
	Candidate.Detail.Mesh.vSourceTypeDataRotationDegrees = {};

	OutBakedElement = std::move(Candidate);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Merge_GenericAuthoredElements(
	const EFFECT_DOCUMENT_DESC& TargetDocument,
	const std::vector<EFFECT_ELEMENT_DESC>& Elements,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (!Validate_Drawable(TargetDocument, strOutError))
		return false;
	if (Elements.empty())
	{
		strOutError =
			"Generic authored Element merge requires at least one Element.";
		return false;
	}

	std::unordered_set<std::string> ElementIds;
	ElementIds.reserve(TargetDocument.Elements.size() + Elements.size());
	for (const EFFECT_ELEMENT_DESC& Element : TargetDocument.Elements)
		ElementIds.insert(Element.strElementId);

	EFFECT_DOCUMENT_DESC Candidate = TargetDocument;
	Candidate.Elements.reserve(TargetDocument.Elements.size() + Elements.size());
	for (const EFFECT_ELEMENT_DESC& Element : Elements)
	{
		if (!Is_StableId(Element.strElementId))
		{
			strOutError =
				"Generic authored Element merge requires an explicit stable target Element ID.";
			return false;
		}
		if (!ElementIds.insert(Element.strElementId).second)
		{
			strOutError =
				"Generic authored Element merge rejects duplicate target Element IDs.";
			return false;
		}
		if (Element.eKind != EFFECT_ELEMENT_KIND::MESH &&
			Element.eKind != EFFECT_ELEMENT_KIND::SPRITE &&
			Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.eKind != EFFECT_ELEMENT_KIND::DECAL &&
			Element.eKind != EFFECT_ELEMENT_KIND::TRAIL)
		{
			strOutError =
				"Generic authored Element merge supports Mesh, Sprite, Particle, Decal, and Trail Elements only.";
			return false;
		}
		if (Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.x != 0.f ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.y != 0.f ||
			Element.Detail.Mesh.vSourceTypeDataRotationDegrees.z != 0.f)
		{
			strOutError =
				"Generic authored Element merge rejects native renderer state.";
			return false;
		}

		EFFECT_DOCUMENT_DESC ProvenanceProbe = TargetDocument;
		ProvenanceProbe.ModelCues.clear();
		ProvenanceProbe.Elements.assign(1u, Element);
		EFFECT_DOCUMENT_DESC ClearedProbe = ProvenanceProbe;
		EFFECT_ELEMENT_DESC& Cleared = ClearedProbe.Elements.front();
		Cleared.strSourceNode.clear();
		Cleared.Renderer = {};
		Cleared.ActionCueAttachment = {};
		Cleared.TransformInheritance = {};
		Cleared.SourceRecipe = {};
		Cleared.SourcePresentation = {};
		Cleared.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
		if (Serialize(ProvenanceProbe) != Serialize(ClearedProbe))
		{
			strOutError =
				"Generic authored Element merge requires source provenance to remain in the migration binding.";
			return false;
		}

		Candidate.Elements.push_back(Element);
	}

	if (!Validate_Drawable(Candidate, strOutError))
		return false;
	const std::string Canonical = Serialize(Candidate);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Staged.strEffectAssetId != TargetDocument.strEffectAssetId ||
		Staged.Elements.size() != Candidate.Elements.size() ||
		Serialize(Staged) != Canonical)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored Element merge did not survive canonical validation exactly.";
		}
		return false;
	}
	const size_t iFirstMerged = TargetDocument.Elements.size();
	for (size_t i = 0u; i < Elements.size(); ++i)
	{
		const EFFECT_ELEMENT_DESC& Expected = Elements[i];
		const EFFECT_ELEMENT_DESC& Actual = Staged.Elements[iFirstMerged + i];
		if (Actual.strElementId != Expected.strElementId ||
			Actual.strGroupId != Expected.strGroupId ||
			Actual.strDisplayName != Expected.strDisplayName)
		{
			strOutError =
				"Generic authored Element merge changed caller-owned identity or display metadata.";
			return false;
		}
	}

	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

namespace
{
	using namespace Client;

	constexpr std::array<std::string_view, 42u>
		PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES = {
			"particlemoduleacceleration",
			"particlemodulecameraoffset",
			"particlemodulecolor",
			"particlemodulecoloroverlife",
			"particlemodulecolorscaleoverlife",
			"particlemoduleeventgenerator",
			"particlemoduleeventreceiverspawn",
			"particlemodulelifetime",
			"particlemodulelocation",
			"particlemodulelocationcirclesurface",
			"particlemodulelocationdirect",
			"particlemodulelocalvectorfield",
			"particlemodulelocationonground",
			"particlemodulelocationprimitivecylinder",
			"particlemodulelocationprimitivecylinderspin",
			"particlemodulelocationprimitivesphere",
			"particlemodulemeshrotation",
			"particlemodulemeshrotationrate",
			"particlemodulemeshrotationratemultiplylife",
			"particlemodulemeshrotationrateoverlife",
			"particlemoduleorientationaxislock",
			"particlemoduleorbit",
			"particlemoduleparameterdynamic",
			"particlemodulerequired",
			"particlemodulerotation",
			"particlemodulerotationrate",
			"particlemodulerotationratemultiplylife",
			"particlemodulesize",
			"particlemodulesizescale",
			"particlemodulesizescalebytime",
			"particlemodulesizemultiplylife",
			"particlemodulespawn",
			"particlemodulespawnperunit",
			"particlemodulesubuv",
			"particlemoduletypedatamesh",
			"particlemodulevectorfieldrotationrate",
			"particlemodulevectorfieldscale",
			"particlemodulevectorfieldscaleoverlife",
			"particlemodulevelocity",
			"particlemodulevelocityinheritparent",
			"particlemodulevelocityoverlifetime",
			"particlemodulevortex"
		};

	constexpr std::array<std::pair<std::string_view, std::string_view>, 67u>
		PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES = {
			std::pair{ "efparticlemoduleacceleration", "acceldata" },
			std::pair{ "particlemoduleacceleration", "acceleration" },
			std::pair{ "particlemodulecameraoffset", "cameraoffset" },
			std::pair{ "particlemodulecolor", "startalpha" },
			std::pair{ "particlemodulecolor", "startcolor" },
			std::pair{ "particlemodulecoloroverlife", "alphaoverlife" },
			std::pair{ "particlemodulecoloroverlife", "coloroverlife" },
			std::pair{ "particlemodulecolorscaleoverlife", "alphascaleoverlife" },
			std::pair{ "particlemodulecolorscaleoverlife", "colorscaleoverlife" },
			std::pair{ "particlemoduleeventreceiverspawn", "inheritvelocityscale" },
			std::pair{ "particlemoduleeventreceiverspawn", "spawncount" },
			std::pair{ "particlemodulelifetime", "lifetime" },
			std::pair{ "particlemodulelocation", "startlocation" },
			std::pair{ "particlemodulelocationcirclesurface", "startlocation" },
			std::pair{ "particlemodulelocationcirclesurface", "startradius" },
			std::pair{ "particlemodulelocationcirclesurface", "startrot" },
			std::pair{ "particlemodulelocationcirclesurface", "velocityscale" },
			std::pair{ "particlemodulelocationdirect", "direction" },
			std::pair{ "particlemodulelocationdirect", "location" },
			std::pair{ "particlemodulelocationdirect", "locationoffset" },
			std::pair{ "particlemodulelocationdirect", "scalefactor" },
			std::pair{ "particlemodulelocationonground", "adjustlocation" },
			std::pair{ "particlemodulelocationonground", "skiplocation" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startheight" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startlocation" },
			std::pair{ "particlemodulelocationprimitivecylinder", "startradius" },
			std::pair{ "particlemodulelocationprimitivecylinder", "velocityscale" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "spinangle" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startcylinderrot" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startheight" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startlocation" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "startradius" },
			std::pair{ "particlemodulelocationprimitivecylinderspin", "velocityscale" },
			std::pair{ "particlemodulelocationprimitivesphere", "startlocation" },
			std::pair{ "particlemodulelocationprimitivesphere", "startradius" },
			std::pair{ "particlemodulelocationprimitivesphere", "velocityscale" },
			std::pair{ "particlemodulemeshrotation", "startrotation" },
			std::pair{ "particlemodulemeshrotationrate", "startrotationrate" },
			std::pair{ "particlemodulemeshrotationratemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulemeshrotationrateoverlife", "rotrate" },
			std::pair{ "particlemoduleorbit", "offsetamount" },
			std::pair{ "particlemoduleorbit", "rotationamount" },
			std::pair{ "particlemoduleorbit", "rotationrateamount" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[0].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[1].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[2].paramvalue" },
			std::pair{ "particlemoduleparameterdynamic", "dynamicparams[3].paramvalue" },
			std::pair{ "particlemodulerequired", "spawnrate" },
			std::pair{ "particlemodulerotation", "startrotation" },
			std::pair{ "particlemodulerotationrate", "startrotationrate" },
			std::pair{ "particlemodulerotationratemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulesize", "startsize" },
			std::pair{ "particlemodulesizescale", "sizescale" },
			std::pair{ "particlemodulesizescalebytime", "sizescalebytime" },
			std::pair{ "particlemodulesizemultiplylife", "lifemultiplier" },
			std::pair{ "particlemodulespawn", "rate" },
			std::pair{ "particlemodulespawn", "ratescale" },
			std::pair{ "particlemodulespawnperunit", "spawnperunit" },
			std::pair{ "particlemodulesubuv", "subimageindex" },
			std::pair{ "particlemodulevectorfieldscale", "scale" },
			std::pair{ "particlemodulevectorfieldscaleoverlife", "scaleoverlife" },
			std::pair{ "particlemodulevelocity", "startvelocity" },
			std::pair{ "particlemodulevelocity", "startvelocityradial" },
			std::pair{ "particlemodulevelocityinheritparent", "scale" },
			std::pair{ "particlemodulevelocityoverlifetime", "veloverlife" },
			std::pair{ "efparticlemodulevortex", "poweracceleration" }
		};

	constexpr std::array<std::pair<std::string_view, size_t>, 22u>
		PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS = {
			std::pair{ "particlemoduleacceleration", 2u },
			std::pair{ "particlemodulecameraoffset", 2u },
			std::pair{ "particlemodulecolor", 3u },
			std::pair{ "particlemodulecolorscaleoverlife", 5u },
			std::pair{ "particlemoduleeventgenerator", 2u },
			std::pair{ "particlemodulelifetime", 2u },
			std::pair{ "particlemodulelocation", 3u },
			std::pair{ "particlemodulelocationcirclesurface", 2u },
			std::pair{ "particlemodulelocationprimitivecylinder", 2u },
			std::pair{ "particlemodulelocationprimitivecylinderspin", 2u },
			std::pair{ "particlemodulelocationprimitivesphere", 2u },
			std::pair{ "particlemodulemeshrotation", 5u },
			std::pair{ "particlemodulemeshrotationrate", 2u },
			std::pair{ "particlemoduleorientationaxislock", 2u },
			std::pair{ "particlemoduleorbit", 2u },
			std::pair{ "particlemodulerotation", 3u },
			std::pair{ "particlemodulerotationrate", 2u },
			std::pair{ "particlemodulerotationratemultiplylife", 2u },
			std::pair{ "particlemodulesize", 2u },
			std::pair{ "particlemodulesizemultiplylife", 5u },
			std::pair{ "particlemodulevelocity", 2u },
			std::pair{ "particlemodulevelocityoverlifetime", 3u }
		};

	std::string_view NormalizePortableParticleModuleClass(
		const std::string_view Value)
	{
		std::string_view Result = Value;
		if (Result.starts_with("efparticlemodule"))
			Result.remove_prefix(2u);
		if (Result.ends_with("_seeded"))
			Result.remove_suffix(7u);
		return Result;
	}

	std::string_view PortableParticleDistributionCapabilityClass(
		const std::string_view strSourceClass,
		const std::string_view strNormalizedClass)
	{
		if (strNormalizedClass == "particlemoduleacceleration" &&
			strSourceClass.starts_with("efparticlemoduleacceleration"))
		{
			return "efparticlemoduleacceleration";
		}
		if (strNormalizedClass == "particlemodulevortex" &&
			strSourceClass.starts_with("efparticlemodulevortex"))
		{
			return "efparticlemodulevortex";
		}
		return strNormalizedClass;
	}

	bool_t IsPortableAuthoredParticleDistributionProperty(
		const std::string_view strModuleClass,
		const std::string_view strPropertyPath)
	{
		return std::ranges::find(
			PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES,
			std::pair{ strModuleClass, strPropertyPath }) !=
			PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.end();
	}

	bool_t IsPortableNullCdoDistribution(
		const EFFECT_DISTRIBUTION_DESC& Distribution)
	{
		const auto IsZero4 = [](const float4_t& Value)
		{
			return Value.x == 0.f && Value.y == 0.f &&
				Value.z == 0.f && Value.w == 0.f;
		};
		return Distribution.strSourceClass.empty() &&
			Distribution.strSourceObjectPath.empty() &&
			Distribution.iComponentCount == 1u &&
			Distribution.iOperation == 1u &&
			Distribution.iRandomLockAxes == 0u &&
			Distribution.iLookupTableChunkSize == 0u &&
			Distribution.iLookupTableNumElements == 0u &&
			Distribution.fLookupTableTimeScale == 0.f &&
			Distribution.fLookupTableStartTime == 0.f &&
			IsZero4(Distribution.vDefaultMinimum) &&
			IsZero4(Distribution.vDefaultMaximum) &&
			Distribution.LookupTable.empty() && Distribution.Keys.empty();
	}

	void AppendPortableDistributionProbeTimes(
		const EFFECT_DISTRIBUTION_DESC& Distribution,
		const f32_t fDurationSeconds,
		std::vector<f32_t>& InOutTimes)
	{
		const auto Append = [fDurationSeconds, &InOutTimes](const f32_t fTime)
		{
			if (std::isfinite(fTime) && fTime >= 0.f &&
				fTime <= fDurationSeconds)
			{
				InOutTimes.push_back(fTime);
			}
		};
		for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
		{
			Append(Distribution.Keys[iKey].fTime);
			if (iKey + 1u < Distribution.Keys.size())
			{
				Append(0.5f * (Distribution.Keys[iKey].fTime +
					Distribution.Keys[iKey + 1u].fTime));
			}
		}

		if (Distribution.LookupTable.empty() ||
			Distribution.fLookupTableTimeScale <= 0.f)
		{
			return;
		}
		constexpr size_t CookedLookupRangeValueCount = 2u;
		const size_t iChunkSize =
			0u != Distribution.iLookupTableChunkSize ?
				Distribution.iLookupTableChunkSize :
				static_cast<size_t>(Distribution.iComponentCount) *
					(Distribution.iOperation >= 2u ? 2u : 1u);
		const size_t iPayloadCount =
			Distribution.LookupTable.size() >= CookedLookupRangeValueCount ?
				Distribution.LookupTable.size() - CookedLookupRangeValueCount : 0u;
		const size_t iEntryCount = 0u == iChunkSize ? 0u :
			iPayloadCount / iChunkSize;
		for (size_t iEntry = 0u; iEntry < iEntryCount; ++iEntry)
		{
			const f32_t fTime = Distribution.fLookupTableStartTime +
				static_cast<f32_t>(iEntry) /
					Distribution.fLookupTableTimeScale;
			Append(fTime);
			if (iEntry + 1u < iEntryCount)
			{
				Append(fTime + 0.5f /
					Distribution.fLookupTableTimeScale);
			}
		}
	}

	bool_t HasPortableAuthoredAutonomousEmission(
		const EFFECT_ELEMENT_DESC& Element)
	{
		if (std::ranges::any_of(Element.SourceRecipe.Bursts,
			[](const EFFECT_PARTICLE_BURST_DESC& Burst)
			{
				return Burst.iCountMaximum > 0u;
			}))
		{
			return true;
		}

		const EFFECT_SOURCE_MODULE_DESC* pSpawn = nullptr;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (NormalizePortableParticleModuleClass(Module.strClassName) ==
				"particlemodulespawn")
			{
				pSpawn = &Module;
				break;
			}
		}
		if (nullptr == pSpawn)
			return false;
		const auto FindDistribution = [pSpawn](const std::string_view Property)
			-> const EFFECT_DISTRIBUTION_DESC*
		{
			const auto Iterator = std::ranges::find_if(pSpawn->Distributions,
				[Property](const EFFECT_DISTRIBUTION_DESC& Distribution)
				{
					return Distribution.strPropertyPath == Property;
				});
			return Iterator == pSpawn->Distributions.end() ? nullptr : &*Iterator;
		};
		const EFFECT_DISTRIBUTION_DESC* const pRate =
			FindDistribution("rate");
		const EFFECT_DISTRIBUTION_DESC* const pRateScale =
			FindDistribution("ratescale");
		if (nullptr == pRate || nullptr == pRateScale)
			return false;

		const f32_t fDurationSeconds = (std::max)(0.f,
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f ?
				Element.SourceRecipe.fEmitterDurationSeconds :
				Element.Detail.Timing.fLifeTimeSeconds);
		std::vector<f32_t> ProbeTimes = {
			0.f, 0.5f * fDurationSeconds, fDurationSeconds };
		AppendPortableDistributionProbeTimes(
			*pRate, fDurationSeconds, ProbeTimes);
		if (!IsPortableNullCdoDistribution(*pRateScale))
		{
			AppendPortableDistributionProbeTimes(
				*pRateScale, fDurationSeconds, ProbeTimes);
		}
		std::ranges::sort(ProbeTimes);
		ProbeTimes.erase(std::unique(ProbeTimes.begin(), ProbeTimes.end()),
			ProbeTimes.end());

		constexpr std::array<f32_t, 3u> RandomUnits = { 0.f, 0.5f, 1.f };
		for (const f32_t fTime : ProbeTimes)
		{
			for (const f32_t fRateRandom : RandomUnits)
			{
				const f32_t fRate = CEffectDistribution::Evaluate(
					*pRate, fTime, fRateRandom).x;
				if (!std::isfinite(fRate) || fRate <= 0.f)
					continue;
				if (IsPortableNullCdoDistribution(*pRateScale))
					return true;
				for (const f32_t fScaleRandom : RandomUnits)
				{
					const f32_t fRateScale = CEffectDistribution::Evaluate(
						*pRateScale, fTime, fScaleRandom).x;
					if (std::isfinite(fRateScale) && fRateScale > 0.f)
						return true;
				}
			}
		}
		return false;
	}

	bool_t IsPortableVectorFieldAssetId(const std::string& strAssetId)
	{
		if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
			!strAssetId.starts_with("Effect/") ||
			strAssetId.find('\\') != std::string::npos ||
			strAssetId.find(':') != std::string::npos)
		{
			return false;
		}
		const std::filesystem::path RelativePath(strAssetId);
		if (RelativePath.is_absolute() || RelativePath.has_root_path() ||
			RelativePath.lexically_normal().generic_string() != strAssetId ||
			RelativePath.extension() != ".wvectorfield")
		{
			return false;
		}
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Value = Component.generic_string();
			if (Value.empty() || Value == "." || Value == "..")
				return false;
		}
		const std::filesystem::path Resolved =
			CRuntimeAssetRoot::Resolve(RelativePath);
		std::error_code Error;
		return !Resolved.empty() &&
			std::filesystem::is_regular_file(Resolved, Error) && !Error;
	}

	const EFFECT_SOURCE_LITERAL_DESC* FindPortableSourceLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath)
	{
		const auto Iterator = std::ranges::find_if(Module.Literals,
			[strPropertyPath](const EFFECT_SOURCE_LITERAL_DESC& Literal)
			{
				return Literal.strPropertyPath == strPropertyPath;
			});
		return Iterator == Module.Literals.end() ? nullptr : &*Iterator;
	}

	bool_t ReadPortableBoolLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const bool_t bDefault,
		bool_t& bOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			bOutValue = bDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
			return false;
		bOutValue = pLiteral->bBoolean;
		return true;
	}

	bool_t ReadPortableNumberLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const f64_t fDefault,
		f64_t& fOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			fOutValue = fDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::NUMBER ||
			!std::isfinite(pLiteral->fNumber))
		{
			return false;
		}
		fOutValue = pLiteral->fNumber;
		return true;
	}

	bool_t ReadPortableStringLiteral(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strPropertyPath,
		const std::string_view strDefault,
		std::string_view& strOutValue)
	{
		const EFFECT_SOURCE_LITERAL_DESC* pLiteral =
			FindPortableSourceLiteral(Module, strPropertyPath);
		if (nullptr == pLiteral)
		{
			strOutValue = strDefault;
			return true;
		}
		if (pLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING)
			return false;
		strOutValue = pLiteral->strString;
		return true;
	}

	bool_t ValidatePortableParticleModuleSemantics(
		const EFFECT_SOURCE_MODULE_DESC& Module,
		const std::string_view strNormalizedClass,
		std::string& strOutError)
	{
		constexpr std::array<std::string_view, 6u> ExactClasses = {
			"particlemoduleeventgenerator",
			"particlemoduleeventreceiverspawn",
			"particlemoduleorbit",
			"particlemodulesizescale",
			"particlemodulevectorfieldscale",
			"particlemodulevelocityinheritparent"
		};
		if (std::ranges::find(ExactClasses, strNormalizedClass) !=
				ExactClasses.end() &&
			Module.strClassName != strNormalizedClass)
		{
			strOutError =
				"Portable authored particle module requires an exact Playback class identity: " +
				Module.strClassName + ".";
			return false;
		}

		if (strNormalizedClass == "particlemodulelocationcirclesurface")
		{
			std::string_view strAxis;
			f64_t fSplit = 0.0;
			bool_t bHalf = false;
			bool_t bNegative = false;
			bool_t bVelocity = false;
			bool_t bEnabled = true;
			if (Module.strClassName != "efparticlemodulelocationcirclesurface" ||
				!ReadPortableStringLiteral(Module, "surfaceaxis",
					"pmlcs_circle_axis_xy", strAxis) ||
				(strAxis != "pmlcs_circle_axis_xy" &&
				 strAxis != "pmlcs_circle_axis_yz" &&
				 strAxis != "pmlcs_circle_axis_zx") ||
				!ReadPortableNumberLiteral(Module, "splitcirclecount", 0.0,
					fSplit) || fSplit < 0.0 || fSplit != std::floor(fSplit) ||
				!ReadPortableBoolLiteral(Module, "bhalfmode", false, bHalf) ||
				!ReadPortableBoolLiteral(Module, "bnegativeaxis", false,
					bNegative) ||
				!ReadPortableBoolLiteral(Module, "velocity", false, bVelocity) ||
				!ReadPortableBoolLiteral(Module, "benabled", true, bEnabled) ||
				!bEnabled)
			{
				strOutError =
					"Portable authored particle CircleSurface semantics are unsupported: " +
					Module.strClassName + ".";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleeventgenerator")
		{
			constexpr std::array<std::string_view, 9u> EventProperties = {
				"events[0].buseorbitoffset", "events[0].customname",
				"events[0].firsttimeonly", "events[0].frequency",
				"events[0].lasttimeonly", "events[0].lowfreq",
				"events[0].particlefrequency", "events[0].type",
				"events[0].usereflectedimpactvector"
			};
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath.starts_with("events[") &&
					std::ranges::find(EventProperties,
						Literal.strPropertyPath) == EventProperties.end())
				{
					strOutError =
						"Portable authored particle event generator payload is unsupported.";
					return false;
				}
			}
			std::string_view strType;
			std::string_view strName;
			f64_t fFrequency = 0.0;
			f64_t fParticleFrequency = 0.0;
			f64_t fLowFrequency = -1.0;
			bool_t bFirst = false;
			bool_t bLast = false;
			bool_t bReflected = false;
			bool_t bOrbit = false;
			if (!ReadPortableStringLiteral(Module, "events[0].type", "",
					strType) || strType != "epet_spawn" ||
				!ReadPortableStringLiteral(Module, "events[0].customname", "",
					strName) || strName.empty() ||
				!ReadPortableNumberLiteral(Module, "events[0].frequency", 0.0,
					fFrequency) || fFrequency < 0.0 ||
				fFrequency != std::floor(fFrequency) ||
				!ReadPortableNumberLiteral(Module,
					"events[0].particlefrequency", 0.0,
					fParticleFrequency) || fParticleFrequency != 0.0 ||
				!ReadPortableNumberLiteral(Module, "events[0].lowfreq", -1.0,
					fLowFrequency) || fLowFrequency != -1.0 ||
				!ReadPortableBoolLiteral(Module, "events[0].firsttimeonly",
					false, bFirst) || bFirst ||
				!ReadPortableBoolLiteral(Module, "events[0].lasttimeonly",
					false, bLast) || bLast ||
				!ReadPortableBoolLiteral(Module,
					"events[0].usereflectedimpactvector", false,
					bReflected) || bReflected ||
				!ReadPortableBoolLiteral(Module, "events[0].buseorbitoffset",
					false, bOrbit) || bOrbit)
			{
				strOutError =
					"Portable authored particle Spawn-event generator semantics are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleeventreceiverspawn")
		{
			std::string_view strType;
			std::string_view strName;
			bool_t bUseParticleTime = false;
			bool_t bInheritVelocity = false;
			bool_t bUseSystemLocation = false;
			if (!ReadPortableStringLiteral(Module, "eventgeneratortype", "",
					strType) || strType != "epet_spawn" ||
				!ReadPortableStringLiteral(Module, "eventname", "", strName) ||
				strName.empty() ||
				!ReadPortableBoolLiteral(Module, "buseparticletime", false,
					bUseParticleTime) || bUseParticleTime ||
				!ReadPortableBoolLiteral(Module, "binheritvelocity", false,
					bInheritVelocity) ||
				!ReadPortableBoolLiteral(Module, "busepsyslocation", false,
					bUseSystemLocation) || bUseSystemLocation)
			{
				strOutError =
					"Portable authored particle Spawn-event receiver semantics are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduleorbit")
		{
			std::string_view strChainMode;
			if (!ReadPortableStringLiteral(Module, "chainmode",
					"eochainmode_add", strChainMode) ||
				strChainMode != "eochainmode_add" ||
				std::ranges::any_of(Module.Literals,
					[](const EFFECT_SOURCE_LITERAL_DESC& Literal)
					{
						return Literal.strPropertyPath.starts_with(
							"offsetoptions.") ||
							Literal.strPropertyPath.starts_with(
								"rotationoptions.") ||
							Literal.strPropertyPath.starts_with(
								"rotationrateoptions.");
					}))
			{
				strOutError =
					"Portable authored particle Orbit chain/options are unsupported.";
				return false;
			}
		}
		else if (strNormalizedClass == "particlemoduletypedatadecal")
		{
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				const bool_t bNumber =
					Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::NUMBER;
				const bool_t bBoolean =
					Literal.eKind == EFFECT_SOURCE_LITERAL_KIND::BOOLEAN;
				const bool_t bSupported =
					((Literal.strPropertyPath == "lodvalidity" ||
					  Literal.strPropertyPath == "nearplane" ||
					  Literal.strPropertyPath == "farplane") && bNumber) ||
					(Literal.strPropertyPath == "balwaysdecalupdate" &&
					 bBoolean);
				if (!bSupported)
				{
					strOutError =
						"Portable authored decal TypeData has an unsupported literal: " +
						Literal.strPropertyPath + ".";
					return false;
				}
			}
		}
		else if (strNormalizedClass == "particlemodulevortex")
		{
			f64_t fPower = 1.0;
			if (Module.strClassName != "efparticlemodulevortex" ||
				!ReadPortableNumberLiteral(Module, "power", 1.0, fPower))
			{
				strOutError =
					"Portable authored particle Vortex semantics are unsupported.";
				return false;
			}
		}
		strOutError.clear();
		return true;
	}

	bool_t ValidatePortableAuthoredParticleRuntimeCarrier(
		const EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		const size_t iMeshBindingCount = static_cast<size_t>(std::count_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			}));
		const bool_t bMesh = Element.SourceRecipe.strRendererShape == "mesh";
		const bool_t bSprite = Element.SourceRecipe.strRendererShape == "sprite";
		const bool_t bDecal = Element.SourceRecipe.strRendererShape == "decal";
		const bool_t bFamilyValid =
			(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				(bMesh || bSprite)) ||
			(Element.eKind == EFFECT_ELEMENT_KIND::DECAL && bDecal);
		if (!bFamilyValid ||
			Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			!Element.SourceRecipe.bEnabled ||
			(bMesh ? iMeshBindingCount != 1u : iMeshBindingCount != 0u) ||
			!std::isfinite(Element.SourceRecipe.fEmitterDelaySeconds) ||
			Element.SourceRecipe.fEmitterDelaySeconds < 0.f ||
			Element.SourceRecipe.fEmitterDelaySeconds > 300.f ||
			Element.SourceRecipe.Modules.empty())
		{
			strOutError =
				"Portable authored emitter carrier identity, Family, or schedule is invalid.";
			return false;
		}

		std::unordered_set<std::string> ModuleIds;
		std::unordered_map<std::string, size_t> ModuleClassCounts;
		size_t iRequiredCount = 0u;
		size_t iMeshTypeDataCount = 0u;
		size_t iDecalTypeDataCount = 0u;
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			const std::string_view NormalizedClass =
				NormalizePortableParticleModuleClass(Module.strClassName);
			const bool_t bAdmittedDecalTypeData = bDecal &&
				NormalizedClass == "particlemoduletypedatadecal";
			if ((!bAdmittedDecalTypeData && std::ranges::find(
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES,
					NormalizedClass) ==
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES.end()) ||
				Module.strStableId.empty() ||
				!ModuleIds.insert(Module.strStableId).second ||
				(NormalizedClass == "particlemodulerequired" &&
				 Module.strClassName != "particlemodulerequired") ||
				(NormalizedClass == "particlemodulespawn" &&
				 Module.strClassName != "particlemodulespawn") ||
				(NormalizedClass == "particlemoduletypedatamesh" &&
				 Module.strClassName != "particlemoduletypedatamesh") ||
				(NormalizedClass == "particlemoduletypedatadecal" &&
				 Module.strClassName != "efparticlemoduletypedatadecal"))
			{
				strOutError =
					"Portable authored particle carrier has an unsupported or duplicate module: " +
					Module.strClassName + ".";
				return false;
			}
			if (!ValidatePortableParticleModuleSemantics(
					Module, NormalizedClass, strOutError))
			{
				return false;
			}
			iRequiredCount +=
				NormalizedClass == "particlemodulerequired" ? 1u : 0u;
			iMeshTypeDataCount +=
				NormalizedClass == "particlemoduletypedatamesh" ? 1u : 0u;
			iDecalTypeDataCount +=
				NormalizedClass == "particlemoduletypedatadecal" ? 1u : 0u;
			++ModuleClassCounts[std::string(NormalizedClass)];
			std::unordered_set<std::string> PropertyPaths;
			for (const EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (!PropertyPaths.insert(Literal.strPropertyPath).second)
				{
					strOutError =
						"Portable authored particle carrier has a duplicate module property: " +
						Module.strClassName + "/" + Literal.strPropertyPath + ".";
					return false;
				}
			}
			if (NormalizedClass == "particlemodulelocalvectorfield")
			{
				const auto AssetLiteral = std::find_if(
					Module.Literals.begin(), Module.Literals.end(),
					[](const EFFECT_SOURCE_LITERAL_DESC& Literal)
					{
						return Literal.strPropertyPath == "vectorfield.assetid";
					});
				if (AssetLiteral == Module.Literals.end() ||
					AssetLiteral->eKind != EFFECT_SOURCE_LITERAL_KIND::STRING ||
					!IsPortableVectorFieldAssetId(AssetLiteral->strString))
				{
					strOutError =
						"Portable authored particle local vector field asset is missing or unsafe.";
					return false;
				}
			}
			const std::string_view DistributionCapabilityClass =
				PortableParticleDistributionCapabilityClass(
					Module.strClassName, NormalizedClass);
			for (const EFFECT_DISTRIBUTION_DESC& Distribution :
				Module.Distributions)
			{
				const bool_t bNativeEvidence =
					!Distribution.strReferenceId.empty() ||
					!Distribution.strOccurrenceId.empty() ||
					!Distribution.strPayloadStatus.empty() ||
					!Distribution.strFidelity.empty() ||
					Distribution.ExecutionAdmission.bAllowed ||
					!Distribution.ExecutionAdmission.Blockers.empty() ||
					Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					!Distribution.strParameterName.empty();
				const bool_t bIgnoredNullCdo =
					NormalizedClass == "particlemodulerequired" &&
					Distribution.strPropertyPath == "spawnrate";
				if (bNativeEvidence ||
					!IsPortableAuthoredParticleDistributionProperty(
						DistributionCapabilityClass,
						Distribution.strPropertyPath) ||
					(bIgnoredNullCdo &&
					 !IsPortableNullCdoDistribution(Distribution)) ||
					!PropertyPaths.insert(Distribution.strPropertyPath).second)
				{
					strOutError =
						"Portable authored particle carrier has native evidence or a duplicate distribution: " +
						Module.strClassName + "/" +
						Distribution.strPropertyPath + ".";
					return false;
				}
			}
			const size_t iExpectedDistributionCount =
				static_cast<size_t>(std::count_if(
					PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.begin(),
					PORTABLE_AUTHORED_PARTICLE_DISTRIBUTION_PROPERTIES.end(),
					[DistributionCapabilityClass](const auto& Capability)
					{
						return Capability.first ==
							DistributionCapabilityClass;
					}));
			if (Module.Distributions.size() != iExpectedDistributionCount)
			{
				strOutError =
					"Portable authored particle carrier distribution capability is incomplete: " +
					Module.strClassName + ".";
				return false;
			}
		}
		const auto CountClass = [&ModuleClassCounts](
			const std::string_view ClassName)
		{
			const auto Iterator = ModuleClassCounts.find(std::string(ClassName));
			return Iterator == ModuleClassCounts.end() ? 0u : Iterator->second;
		};
		for (const auto& [ClassName, Count] : ModuleClassCounts)
		{
			/* UE3 can retain renderer-irrelevant modules in an emitter.  The
			   portable runtime keeps their source order, but Sprite drawing never
			   consumes MeshRotation state and Mesh drawing never consumes
			   OrientationAxisLock presentation.  TypeDataMesh remains the actual
			   renderer-Family discriminator. */
			const bool_t bMeshOnly =
				ClassName == "particlemoduletypedatamesh";
			const bool_t bDecalOnly =
				ClassName == "particlemoduletypedatadecal";
			const bool_t bSpriteOnly =
				ClassName == "particlemodulerotationratemultiplylife" ||
				ClassName == "particlemodulesubuv";
			const auto Maximum = std::ranges::find_if(
				PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS,
				[&ClassName](const auto& Capability)
				{
					return Capability.first == ClassName;
				});
			const size_t iMaximum =
				Maximum == PORTABLE_AUTHORED_PARTICLE_MODULE_MAX_COUNTS.end() ?
				1u : Maximum->second;
			if ((bMeshOnly && !bMesh) || (bDecalOnly && !bDecal) ||
				(bSpriteOnly && !bSprite) ||
				Count > iMaximum)
			{
				strOutError =
					"Portable authored particle carrier module Family/cardinality is unsupported: " +
					ClassName + ".";
				return false;
			}
		}
		const bool_t bParticleCardinalityValid =
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			iRequiredCount == 1u &&
			CountClass("particlemodulelifetime") != 0u &&
			CountClass("particlemodulespawn") == 1u &&
			(bMesh ? iMeshTypeDataCount == 1u : iMeshTypeDataCount == 0u) &&
			iDecalTypeDataCount == 0u;
		const bool_t bDecalCardinalityValid =
			bDecal && iRequiredCount == 1u &&
			CountClass("particlemodulelifetime") <= 1u &&
			CountClass("particlemodulespawn") <= 1u &&
			iMeshTypeDataCount == 0u && iDecalTypeDataCount == 1u;
		if (!bParticleCardinalityValid && !bDecalCardinalityValid)
		{
			strOutError =
				"Portable authored emitter carrier Required/Lifetime/Spawn/TypeData cardinality is invalid.";
			return false;
		}
		const size_t iLocalVectorFieldCount =
			CountClass("particlemodulelocalvectorfield");
		if ((CountClass("particlemodulevectorfieldrotationrate") != 0u ||
			 CountClass("particlemodulevectorfieldscale") != 0u ||
			 CountClass("particlemodulevectorfieldscaleoverlife") != 0u) &&
			iLocalVectorFieldCount != 1u)
		{
			strOutError =
				"Portable authored particle vector field companion has no unique local field.";
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool_t ValidatePortableAuthoredParticleEventRoutes(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		std::unordered_map<std::string, std::vector<std::string>> Generators;
		std::unordered_map<std::string, std::vector<std::string>> Receivers;
		uint64_t iMaximumQueuedEvents = 0u;
		const auto RouteKey = [](const std::string_view strType,
			const std::string_view strName)
		{
			std::string Result(strType);
			Result.push_back('\0');
			Result.append(strName);
			return Result;
		};
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Element.bVisible ||
				!Is_EffectAuthoringExecutionTarget(
					Element.Material.Execution) ||
				(Element.eKind != EFFECT_ELEMENT_KIND::PARTICLE &&
				 Element.eKind != EFFECT_ELEMENT_KIND::DECAL) ||
				Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
				Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
				!Element.SourceRecipe.bEnabled ||
				((Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
				  Element.SourceRecipe.strRendererShape != "mesh" &&
				  Element.SourceRecipe.strRendererShape != "sprite") ||
				 (Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
				  Element.SourceRecipe.strRendererShape != "decal")))
			{
				continue;
			}
			uint32_t iGeneratorCount = 0u;
			for (const EFFECT_SOURCE_MODULE_DESC& Module :
				Element.SourceRecipe.Modules)
			{
				bool_t bEnabled = true;
				if (!ReadPortableBoolLiteral(
						Module, "benabled", true, bEnabled))
				{
					strOutError =
						"Portable authored particle event module enabled state is invalid.";
					return false;
				}
				if (!bEnabled)
					continue;
				const std::string_view strClass =
					NormalizePortableParticleModuleClass(Module.strClassName);
				if (strClass == "particlemoduleeventgenerator")
				{
					std::string_view strType;
					std::string_view strName;
					if (!ReadPortableStringLiteral(Module, "events[0].type",
							"", strType) || strType != "epet_spawn" ||
						!ReadPortableStringLiteral(Module,
							"events[0].customname", "", strName) ||
						strName.empty())
					{
						strOutError =
							"Portable authored particle event generator route identity is invalid.";
						return false;
					}
					Generators[RouteKey(strType, strName)].push_back(
						Element.strElementId);
					++iGeneratorCount;
				}
				else if (strClass == "particlemoduleeventreceiverspawn")
				{
					std::string_view strType;
					std::string_view strName;
					if (!ReadPortableStringLiteral(Module,
							"eventgeneratortype", "", strType) ||
						strType != "epet_spawn" ||
						!ReadPortableStringLiteral(Module, "eventname", "",
							strName) || strName.empty())
					{
						strOutError =
							"Portable authored particle event receiver route identity is invalid.";
						return false;
					}
					Receivers[RouteKey(strType, strName)].push_back(
						Element.strElementId);
				}
			}
			iMaximumQueuedEvents += SourceScaledParticleCeiling(Element) *
				static_cast<uint64_t>(iGeneratorCount);
		}

		if (iMaximumQueuedEvents > MAX_PORTABLE_SOURCE_EVENTS_PER_STEP)
		{
			strOutError =
				"Portable authored particle event queue has an unbounded per-step upper limit.";
			return false;
		}
		for (const auto& [strRoute, SourceElements] : Generators)
		{
			(void)SourceElements;
			if (!Receivers.contains(strRoute))
			{
				strOutError =
					"Portable authored particle event generator has no same-document receiver.";
				return false;
			}
		}
		for (const auto& [strRoute, TargetElements] : Receivers)
		{
			(void)TargetElements;
			if (!Generators.contains(strRoute))
			{
				strOutError =
					"Portable authored particle event receiver has no same-document generator.";
				return false;
			}
		}

		std::unordered_map<std::string, std::vector<std::string>> Adjacency;
		for (const auto& [strRoute, SourceElements] : Generators)
		{
			const std::vector<std::string>& TargetElements =
				Receivers.at(strRoute);
			for (const std::string& strSourceElement : SourceElements)
			{
				auto& Targets = Adjacency[strSourceElement];
				Targets.insert(Targets.end(), TargetElements.begin(),
					TargetElements.end());
			}
		}
		std::unordered_map<std::string, uint8_t> VisitStates;
		const auto Visit = [&](const auto& Self,
			const std::string& strElementId) -> bool_t
		{
			uint8_t& iState = VisitStates[strElementId];
			if (1u == iState)
			{
				strOutError =
					"Portable authored particle event route cycle is not allowed.";
				return false;
			}
			if (2u == iState)
				return true;
			iState = 1u;
			const auto Iterator = Adjacency.find(strElementId);
			if (Iterator != Adjacency.end())
			{
				for (const std::string& strTarget : Iterator->second)
				{
					if (!Self(Self, strTarget))
						return false;
				}
			}
			iState = 2u;
			return true;
		};
		for (const auto& [strElementId, Targets] : Adjacency)
		{
			(void)Targets;
			if (!Visit(Visit, strElementId))
				return false;
		}
		strOutError.clear();
		return true;
	}
}

namespace
{
	bool_t ApplyPortableAuthoredEmitterRuntimeCarrier(
		const Client::EFFECT_ELEMENT_DESC& SourceElement,
		Client::EFFECT_ELEMENT_DESC& InOutElement,
		const Client::EFFECT_ELEMENT_KIND eExpectedKind,
		const std::string_view strExpectedShape,
		std::string& strOutError)
	{
		using namespace Client;
		if (SourceElement.eKind != eExpectedKind ||
			InOutElement.eKind != eExpectedKind ||
			!SourceElement.SourceRecipe.bEnabled ||
			SourceElement.SourceRecipe.strRendererShape != strExpectedShape)
		{
			strOutError =
				"Portable authored emitter carrier source/target Family does not match.";
			return false;
		}

		/* Legacy v13 sourceRecipe is the portable, already-interpreted runtime
		   carrier. Constructing a fresh descriptor guarantees that native-v14
		   contract hashes, compiler evidence, authority receipts, geometry
		   admission, and local-reference closure cannot cross this seam. */
		EFFECT_CASCADE_RECIPE_DESC Portable;
		Portable.bEnabled = true;
		Portable.strRendererShape = SourceElement.SourceRecipe.strRendererShape;
		/* Generic occurrence import samples/bakes its starting state before this
		   helper and therefore needs a flattened zero delay. Saved Element reuse
		   restores the source delay explicitly after this carrier is admitted. */
		Portable.fEmitterDelaySeconds = 0.f;
		Portable.fEmitterDurationSeconds =
			SourceElement.SourceRecipe.fEmitterDurationSeconds;
		Portable.iEmitterLoopCount =
			SourceElement.SourceRecipe.iEmitterLoopCount;
		Portable.Bursts = SourceElement.SourceRecipe.Bursts;
		Portable.Modules = SourceElement.SourceRecipe.Modules;
		for (EFFECT_SOURCE_MODULE_DESC& Module : Portable.Modules)
		{
			const std::string_view NormalizedClass =
				NormalizePortableParticleModuleClass(Module.strClassName);
			const bool_t bAdmittedDecalTypeData =
				eExpectedKind == EFFECT_ELEMENT_KIND::DECAL &&
				strExpectedShape == "decal" &&
				NormalizedClass == "particlemoduletypedatadecal";
			if ((!bAdmittedDecalTypeData && std::ranges::find(
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES,
					NormalizedClass) ==
					PORTABLE_AUTHORED_PARTICLE_MODULE_CLASSES.end()) ||
				(NormalizedClass == "particlemodulespawn" &&
				 Module.strClassName != "particlemodulespawn"))
			{
				strOutError =
					"Portable authored emitter carrier has an unsupported module class: " +
					Module.strClassName + ".";
				return false;
			}
			for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			{
				if (Distribution.eParameterBinding !=
						EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE ||
					!Distribution.strParameterName.empty())
				{
					strOutError =
						"Portable authored emitter carrier cannot erase an ActionCue parameter binding: " +
						Module.strClassName + "/" +
						Distribution.strPropertyPath + ".";
					return false;
				}
				Distribution.strReferenceId.clear();
				Distribution.strOccurrenceId.clear();
				Distribution.strPayloadStatus.clear();
				Distribution.strFidelity.clear();
				Distribution.ExecutionAdmission = {};
				Distribution.strParameterName.clear();
				Distribution.eParameterBinding =
					EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
			}
		}
		EFFECT_ELEMENT_DESC Staged = InOutElement;
		Staged.SourceRecipe = std::move(Portable);
		if (!ValidatePortableAuthoredParticleRuntimeCarrier(
				Staged, strOutError))
		{
			return false;
		}
		if (!HasPortableAuthoredAutonomousEmission(Staged))
		{
			/* SpawnPerUnit and source events are multi-occurrence/history
			   contracts.  A single copied Element must own a positive burst or a
			   concretely evaluable positive Rate; otherwise a valid document can
			   stage successfully while emitting no drawable at a static preview
			   root. */
			strOutError =
				"Portable authored emitter carrier has no autonomous positive Burst or Rate; SpawnPerUnit/event/history-only emitters require the complete Effect.";
			return false;
		}
		InOutElement.SourceRecipe = std::move(Staged.SourceRecipe);
		strOutError.clear();
		return true;
	}

	bool_t Validate_AuthoredRuntimeExtensions(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		using namespace Client;
		const bool_t bRuntimeExtensionDocument =
			Document.iLoadedFormatVersion ==
				EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION;
		if (!bRuntimeExtensionDocument)
		{
			if (Document.RuntimeExtensions.iFormatVersion !=
					EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
				!Document.RuntimeExtensions.Is_Empty() ||
				std::any_of(Document.Elements.begin(), Document.Elements.end(),
					[](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.RuntimeCarrier.iFormatVersion !=
								EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
							!Element.RuntimeCarrier.Is_Empty();
					}))
			{
				strOutError =
					"Only authored-v15 Effect documents may carry runtimeExtensions/runtimeCarrier data.";
				return false;
			}
			return true;
		}

		if (Document.RuntimeExtensions.iFormatVersion !=
				EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
			Document.RuntimeExtensions.BakedEdgeHistories.size() >
				MAX_AUTHORED_RUNTIME_EDGE_HISTORIES)
		{
			strOutError = "Authored runtimeExtensions version/size is invalid.";
			return false;
		}

		std::unordered_map<std::string,
			const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC*> HistoriesById;
		std::string strPreviousHistoryId;
		size_t iTotalSampleCount = 0u;
		for (const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& History :
			Document.RuntimeExtensions.BakedEdgeHistories)
		{
			if (!Is_StableId(History.strHistoryId) ||
				(!strPreviousHistoryId.empty() &&
				 History.strHistoryId <= strPreviousHistoryId) ||
				History.eCoordinateBasis !=
					EFFECT_AUTHORED_RUNTIME_COORDINATE_BASIS::
						UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS ||
				!std::isfinite(History.fSourceEndTimeSeconds) ||
				History.fSourceEndTimeSeconds <= 0.f ||
				History.fSourceEndTimeSeconds >
					MAX_AUTHORED_RUNTIME_EDGE_TIME_SECONDS ||
				!std::isfinite(History.fPlaybackClampSeconds) ||
				History.fPlaybackClampSeconds <= 0.f ||
				History.fPlaybackClampSeconds >
					History.fSourceEndTimeSeconds ||
				History.Samples.size() < 2u ||
				History.Samples.size() >
					MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_PER_HISTORY ||
				!HistoriesById.emplace(
					History.strHistoryId, &History).second)
			{
				strOutError =
					"Authored baked-edge history identity/timing is invalid.";
				return false;
			}
			strPreviousHistoryId = History.strHistoryId;
			iTotalSampleCount += History.Samples.size();
			if (iTotalSampleCount > MAX_AUTHORED_RUNTIME_EDGE_SAMPLES_TOTAL)
			{
				strOutError =
					"Authored baked-edge history sample budget is exceeded.";
				return false;
			}

			f32_t fPreviousTime = -1.f;
			for (const EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC& Sample :
				History.Samples)
			{
				const auto CoordinatesBounded = [](const float3_t& Coordinates)
				{
					return Is_Finite(Coordinates) &&
						std::abs(Coordinates.x) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.y) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM &&
						std::abs(Coordinates.z) <=
							MAX_AUTHORED_RUNTIME_EDGE_COORDINATE_UE3_CM;
				};
				if (!std::isfinite(Sample.fRelativeTimeSeconds) ||
					Sample.fRelativeTimeSeconds <= fPreviousTime ||
					Sample.fRelativeTimeSeconds < 0.f ||
					Sample.fRelativeTimeSeconds >
						History.fSourceEndTimeSeconds + 5.0e-5f ||
					!CoordinatesBounded(Sample.vFirstEdgeUE3Cm) ||
					!CoordinatesBounded(Sample.vControlPointUE3Cm) ||
					!CoordinatesBounded(Sample.vSecondEdgeUE3Cm))
				{
					strOutError = "Authored baked-edge sample is invalid.";
					return false;
				}
				fPreviousTime = Sample.fRelativeTimeSeconds;
			}
			if (std::abs(History.Samples.front().fRelativeTimeSeconds) >
					1.0e-6f ||
				std::abs(History.Samples.back().fRelativeTimeSeconds -
					History.fSourceEndTimeSeconds) > 5.0e-5f)
			{
				strOutError =
					"Authored baked-edge history does not close at its declared source interval.";
				return false;
			}
		}

		std::unordered_set<std::string> ReferencedHistoryIds;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
				Element.RuntimeCarrier;
			if (Carrier.Is_Empty())
				continue;
			if (Carrier.iFormatVersion !=
					EFFECT_AUTHORED_RUNTIME_EXTENSION_PAYLOAD_VERSION ||
				Carrier.eKind >= EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::END ||
				Carrier.eAdmission !=
					EFFECT_AUTHORED_RUNTIME_CARRIER_ADMISSION::BOUNDED ||
				!Element.bVisible ||
				(!Is_EffectAuthoringExecutionTarget(Element.Material.Execution) &&
				 !Is_EffectPresentationExecutionTarget(Element)))
			{
				strOutError =
					"Authored runtimeCarrier target is not a visible bounded drawable.";
				return false;
			}

			switch (Carrier.eKind)
			{
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
			{
				if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
					!Carrier.strHistoryId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
					Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.strTypeDataModuleStableId.size() > 256u ||
					!Has_VisibleCharacter(
						Carrier.strTypeDataModuleStableId) ||
					!Element.SourceRecipe.bEnabled ||
					Element.SourceRecipe.strRendererShape != "ribbon")
				{
					strOutError =
						"Authored Cascade runtimeCarrier target/shape is invalid.";
					return false;
				}
				size_t iTypeDataMatchCount = 0u;
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					Element.SourceRecipe.Modules)
				{
					if (Module.strStableId !=
						Carrier.strTypeDataModuleStableId)
					{
						continue;
					}
					++iTypeDataMatchCount;
					if (Normalize_SourceModuleClass(Module.strClassName) !=
						"particlemoduletypedataribbon")
					{
						strOutError =
							"Authored Cascade runtimeCarrier joins a non-Ribbon TypeData module.";
						return false;
					}
				}
				if (1u != iTypeDataMatchCount)
				{
					strOutError =
						"Authored Cascade runtimeCarrier TypeData stable join is missing or ambiguous.";
					return false;
				}
				break;
			}
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				ANIMATION_TRAIL_BAKED_EDGE_V1:
				if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL ||
					!Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::END ||
					!Is_StableId(Carrier.strHistoryId) ||
					!HistoriesById.contains(Carrier.strHistoryId))
				{
					strOutError =
						"Authored Animation Trail runtimeCarrier history join is invalid.";
					return false;
				}
				ReferencedHistoryIds.insert(Carrier.strHistoryId);
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				LIGHT_BAKED_EDGE_ATTACHMENT_V1:
				if (Element.eKind != EFFECT_ELEMENT_KIND::LIGHT ||
					!Carrier.strTypeDataModuleStableId.empty() ||
					Carrier.eEdgeLane !=
						EFFECT_AUTHORED_RUNTIME_BAKED_EDGE_LANE::FIRST_EDGE ||
					!Is_StableId(Carrier.strHistoryId) ||
					!HistoriesById.contains(Carrier.strHistoryId) ||
					!Element.Detail.Light.bEnabled)
				{
					strOutError =
						"Authored Light runtimeCarrier history/lane target is invalid.";
					return false;
				}
				ReferencedHistoryIds.insert(Carrier.strHistoryId);
				break;
			default:
				strOutError = "Authored runtimeCarrier kind is invalid.";
				return false;
			}
		}

		if (ReferencedHistoryIds.size() != HistoriesById.size())
		{
			strOutError =
				"Authored runtimeExtensions contain an unreferenced baked-edge history.";
			return false;
		}
		return true;
	}
}

bool_t Client::CEffectDocumentCodec::
	Apply_PortableAuthoredParticleRuntimeCarrier(
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError)
{
	const bool_t bTargetMeshParticle = std::any_of(
		InOutElement.ResourceBindings.begin(),
		InOutElement.ResourceBindings.end(),
		[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
		});
	return ApplyPortableAuthoredEmitterRuntimeCarrier(SourceElement,
		InOutElement, EFFECT_ELEMENT_KIND::PARTICLE,
		bTargetMeshParticle ? "mesh" : "sprite", strOutError);
}

bool_t Client::CEffectDocumentCodec::
	Apply_PortableAuthoredDecalRuntimeCarrier(
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError)
{
	return ApplyPortableAuthoredEmitterRuntimeCarrier(SourceElement,
		InOutElement, EFFECT_ELEMENT_KIND::DECAL, "decal", strOutError);
}

bool_t Client::CEffectDocumentCodec::Build_GenericAuthoredElementImportStage(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const EFFECT_DOCUMENT_DESC& TargetDocument,
	const EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST& Request,
	EFFECT_DOCUMENT_DESC& InOutDocument,
	std::string& strOutError)
{
	if (Request.strSourceElementId.empty() ||
		Request.strTargetElementId.empty() ||
		Request.strTargetGroupId.empty() ||
		Request.strTargetDisplayName.empty() ||
		TargetDocument.strEffectAssetId.empty())
	{
		strOutError =
			"Generic authored import requires explicit source, target, group, display, and Effect IDs.";
		return false;
	}

	const auto Source = std::find_if(SourceDocument.Elements.begin(),
		SourceDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strSourceElementId;
		});
	if (Source == SourceDocument.Elements.end() ||
		std::find_if(std::next(Source), SourceDocument.Elements.end(),
			[&Request](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == Request.strSourceElementId;
			}) != SourceDocument.Elements.end())
	{
		strOutError =
			"Generic authored import requires exactly one source Element.";
		return false;
	}

	const std::string SourceCanonicalBefore = Serialize(SourceDocument);
	const std::string TargetCanonicalBefore = Serialize(TargetDocument);
	if (Source->SourceRecipe.bEnabled)
	{
		const auto& Starting = Request.StartingState;
		if (Source->eKind != EFFECT_ELEMENT_KIND::PARTICLE ||
			Starting.fScheduleStartDelaySeconds !=
				Source->Detail.Timing.fStartDelaySeconds ||
			Starting.fScheduleLifeTimeSeconds !=
				Source->Detail.Timing.fLifeTimeSeconds ||
			Starting.fEmitterDelaySeconds !=
				Source->SourceRecipe.fEmitterDelaySeconds ||
			Starting.fEmitterDurationSeconds !=
				Source->SourceRecipe.fEmitterDurationSeconds ||
			Starting.iEmitterLoopCount !=
				Source->SourceRecipe.iEmitterLoopCount ||
			Starting.bAttachmentEnabled !=
				Source->ActionCueAttachment.bEnabled ||
			Starting.bFollowAttachment !=
				Source->ActionCueAttachment.bFollow ||
			Starting.fSnapshotRootSourceBasisYawDegrees !=
				Source->ActionCueAttachment.
					fSnapshotRootSourceBasisYawDegrees ||
			Starting.bTransformInheritanceEnabled !=
				Source->TransformInheritance.bEnabled)
		{
			strOutError =
				"Generic authored import starting state does not exactly identify its source Particle occurrence.";
			return false;
		}
	}

	EFFECT_DOCUMENT_DESC LoweredDocument;
	if (!Build_GenericAuthoredElementStartingCopy(
			SourceDocument, Request.strSourceElementId,
			TargetDocument.strEffectAssetId, LoweredDocument, strOutError) ||
		LoweredDocument.Elements.size() != 1u)
	{
		return false;
	}

	EFFECT_ELEMENT_DESC BakedElement;
	if (!Bake_GenericAuthoredElementStartingState(
			LoweredDocument.Elements.front(), Request.StartingState,
			BakedElement, strOutError))
	{
		return false;
	}
	BakedElement.strElementId = Request.strTargetElementId;
	BakedElement.strGroupId = Request.strTargetGroupId;
	BakedElement.strDisplayName = Request.strTargetDisplayName;
	if (Request.bOverrideMaterialExecution)
	{
		BakedElement.Material.SourceMaterial = {};
		BakedElement.Material.Execution = Request.MaterialExecution;
	}

	EFFECT_DOCUMENT_DESC MergedDocument;
	if (!Merge_GenericAuthoredElements(TargetDocument, { BakedElement },
			MergedDocument, strOutError))
	{
		return false;
	}
	const auto Imported = std::find_if(MergedDocument.Elements.begin(),
		MergedDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		});
	if (Imported == MergedDocument.Elements.end())
	{
		strOutError =
			"Generic authored import lost its stable target Element identity.";
		return false;
	}
	if (Source->SourceRecipe.bEnabled &&
		!Apply_PortableAuthoredParticleRuntimeCarrier(
			*Source, *Imported, strOutError))
	{
		return false;
	}

	const std::string Canonical = Serialize(MergedDocument);
	EFFECT_DOCUMENT_DESC Staged;
	if (!Parse(Canonical, Staged, strOutError) ||
		!Validate_Drawable(Staged, strOutError) ||
		Serialize(Staged) != Canonical ||
		Serialize(SourceDocument) != SourceCanonicalBefore ||
		Serialize(TargetDocument) != TargetCanonicalBefore)
	{
		if (strOutError.empty())
		{
			strOutError =
				"Generic authored import did not survive canonical validation without mutating its inputs.";
		}
		return false;
	}
	const auto StagedElement = std::find_if(Staged.Elements.begin(),
		Staged.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		});
	if (StagedElement == Staged.Elements.end() ||
		StagedElement->strGroupId != Request.strTargetGroupId ||
		StagedElement->strDisplayName != Request.strTargetDisplayName ||
		StagedElement->SourceRecipe.bEnabled !=
			Source->SourceRecipe.bEnabled ||
		(StagedElement->SourceRecipe.bEnabled &&
		 StagedElement->SourceRecipe.fEmitterDelaySeconds != 0.f))
	{
		strOutError =
			"Generic authored import changed target identity or portable Particle timing.";
		return false;
	}

	InOutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

namespace
{
	using namespace Client;

	enum class ARTIST31470_UNIFIED_FAMILY : uint8_t
	{
		MESH,
		SPRITE,
		DECAL,
		RIBBON,
		END
	};

	bool_t Try_ResolveArtist31470UnifiedFamily(
		const EFFECT_RUNTIME_RENDERER_KIND eRenderer,
		ARTIST31470_UNIFIED_FAMILY& eOutFamily)
	{
		switch (eRenderer)
		{
		case EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::MESH;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::SPRITE;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::DECAL;
			return true;
		case EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::RIBBON;
			return true;
		default:
			eOutFamily = ARTIST31470_UNIFIED_FAMILY::END;
			return false;
		}
	}

	const char_t* Artist31470UnifiedFamilyLabel(
		const ARTIST31470_UNIFIED_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH: return "MeshParticle";
		case ARTIST31470_UNIFIED_FAMILY::SPRITE: return "SpriteParticle";
		case ARTIST31470_UNIFIED_FAMILY::DECAL: return "LocalDecal";
		case ARTIST31470_UNIFIED_FAMILY::RIBBON: return "CascadeRibbon";
		default: return "Invalid";
		}
	}

	std::string Artist31470UnifiedStableElementId(
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::string_view strSourceIdentity)
	{
		const std::string Digest =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				std::string(Artist31470UnifiedFamilyLabel(eFamily)) + "\n" +
				std::string(strSourceIdentity));
		const char_t* pPrefix = nullptr;
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH: pPrefix = "mesh"; break;
		case ARTIST31470_UNIFIED_FAMILY::SPRITE: pPrefix = "sprite"; break;
		case ARTIST31470_UNIFIED_FAMILY::DECAL: pPrefix = "decal"; break;
		case ARTIST31470_UNIFIED_FAMILY::RIBBON: pPrefix = "ribbon"; break;
		default: return {};
		}
		return std::string(pPrefix) + "." + Digest.substr(0u, 16u);
	}

	bool_t Artist31470UnifiedElementMatchesFamily(
		const EFFECT_ELEMENT_DESC& Element,
		const ARTIST31470_UNIFIED_FAMILY eFamily)
	{
		const bool_t bHasMeshShape = std::any_of(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		switch (eFamily)
		{
		case ARTIST31470_UNIFIED_FAMILY::MESH:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE && bHasMeshShape;
		case ARTIST31470_UNIFIED_FAMILY::SPRITE:
			return Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE && !bHasMeshShape;
		case ARTIST31470_UNIFIED_FAMILY::DECAL:
			return Element.eKind == EFFECT_ELEMENT_KIND::DECAL;
		case ARTIST31470_UNIFIED_FAMILY::RIBBON:
			return Element.eKind == EFFECT_ELEMENT_KIND::TRAIL;
		default:
			return false;
		}
	}

	bool_t Try_ResolveArtist31470FixedBurstCount(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		uint32_t& iOutCount,
		std::string& strOutError)
	{
		uint64_t iCount = 0u;
		for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Emitter.Timing.Bursts)
		{
			if (!std::isfinite(Burst.fTimeSeconds) ||
				std::abs(Burst.fTimeSeconds) > 1.0e-9 ||
				Burst.iCountMinimum != Burst.iCountMaximum)
			{
				strOutError =
					"Artist F Track A burst is not a fixed t=0 authored burst.";
				return false;
			}
			iCount += Burst.iCountMaximum;
		}
		if (iCount > (std::numeric_limits<uint32_t>::max)())
		{
			strOutError = "Artist F Track A fixed burst count overflowed uint32.";
			return false;
		}
		iOutCount = static_cast<uint32_t>(iCount);
		return true;
	}

	std::string NormalizeArtist31470MaterialRole(const std::string_view Value)
	{
		std::string Result;
		Result.reserve(Value.size());
		bool_t bLastSeparator = false;
		for (const unsigned char Character : Value)
		{
			if (0 != std::isalnum(Character))
			{
				Result.push_back(static_cast<char_t>(std::tolower(Character)));
				bLastSeparator = false;
			}
			else if (!Result.empty() && !bLastSeparator)
			{
				Result.push_back('_');
				bLastSeparator = true;
			}
		}
		while (!Result.empty() && Result.back() == '_')
			Result.pop_back();
		return Result;
	}

	int32_t ScoreArtist31470GenericMaterialLane(
		const std::string_view strSlotId,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::string_view strRole)
	{
		const std::string Role = NormalizeArtist31470MaterialRole(strRole);
		const auto Has = [&Role](const std::string_view Token)
		{
			return std::string::npos != Role.find(Token);
		};
		if (strSlotId == "base")
		{
			if (eFamily == ARTIST31470_UNIFIED_FAMILY::DECAL &&
				Role == "diffuse")
			{
				return 120;
			}
			if (Role == "base") return 110;
			if (Has("diffuse") || Has("albedo")) return 100;
			if (Has("alpha_tex_01") || Has("main_tex")) return 60;
		}
		else if (strSlotId == "noise")
		{
			if (Role == "noise") return 110;
			if (Has("noise")) return 100;
		}
		else if (strSlotId == "mask")
		{
			if (Role == "mask") return 110;
			if (Has("mask")) return 100;
		}
		else if (strSlotId == "emissive")
		{
			if (Role == "emissive") return 110;
			if (Has("emissive") || Has("emap")) return 100;
		}
		else if (strSlotId == "dissolve")
		{
			if (Role == "dissolve") return 110;
			if (Has("dissolve")) return 100;
		}
		return 0;
	}

	std::optional<size_t> ResolveArtist31470GenericMaterialLane(
		const EFFECT_RESOURCE_BINDING_DESC* pSourceBinding,
		const std::string_view strSlotId,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const EFFECT_MATERIAL_EXECUTION_DESC& Execution)
	{
		/* The generic Decal editor exposes its color texture as Base, while the
		   Track A six-SRV packet names that same authoring intent DIFFUSE.  This
		   semantic bridge is stronger than a source-asset match because Base may
		   already contain an artist-selected DDS. */
		if (eFamily == ARTIST31470_UNIFIED_FAMILY::DECAL &&
			strSlotId == "base")
		{
			std::optional<size_t> DiffuseLane;
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (NormalizeArtist31470MaterialRole(
						Execution.TextureLanes[iLane].strRole) != "diffuse")
				{
					continue;
				}
				if (DiffuseLane.has_value())
					return std::nullopt;
				DiffuseLane = iLane;
			}
			if (DiffuseLane.has_value())
				return DiffuseLane;
		}

		std::vector<size_t> SourceAssetMatches;
		if (nullptr != pSourceBinding && !pSourceBinding->strAssetId.empty())
		{
			for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
			{
				if (Execution.TextureLanes[iLane].strAssetId ==
					pSourceBinding->strAssetId)
				{
					SourceAssetMatches.push_back(iLane);
				}
			}
			if (SourceAssetMatches.size() == 1u)
				return SourceAssetMatches.front();
		}

		const bool_t bRestrictToSourceAsset = SourceAssetMatches.size() > 1u;
		std::optional<size_t> BestLane;
		int32_t iBestScore = 0;
		bool_t bTied = false;
		for (size_t iLane = 0u; iLane < Execution.TextureLanes.size(); ++iLane)
		{
			if (bRestrictToSourceAsset &&
				std::find(SourceAssetMatches.begin(), SourceAssetMatches.end(),
					iLane) == SourceAssetMatches.end())
			{
				continue;
			}
			const int32_t iScore = ScoreArtist31470GenericMaterialLane(
				strSlotId, eFamily, Execution.TextureLanes[iLane].strRole);
			if (iScore > iBestScore)
			{
				iBestScore = iScore;
				BestLane = iLane;
				bTied = false;
			}
			else if (iScore > 0 && iScore == iBestScore)
			{
				bTied = true;
			}
		}
		return iBestScore > 0 && !bTied ? BestLane : std::nullopt;
	}

	bool_t PromoteArtist31470GenericMaterialOverrides(
		const EFFECT_ELEMENT_DESC& SourceElement,
		const ARTIST31470_UNIFIED_FAMILY eFamily,
		const std::set<std::string, std::less<>>& ExistingTypedLaneIds,
		const EFFECT_ELEMENT_DESC& ExistingElement,
		EFFECT_MATERIAL_EXECUTION_DESC& InOutExecution,
		std::string& strOutError)
	{
		for (const EFFECT_MATERIAL_INPUT_SLOT_DESC& Input :
			EFFECT_STANDARD_MATERIAL_INPUTS)
		{
			const auto ExistingBinding = std::find_if(
				ExistingElement.ResourceBindings.begin(),
				ExistingElement.ResourceBindings.end(),
				[&Input](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
				{
					return Candidate.strSlotId == Input.strSlotId;
				});
			if (ExistingBinding == ExistingElement.ResourceBindings.end() ||
				ExistingBinding->strAssetId.empty())
			{
				continue;
			}
			EFFECT_RESOURCE_FILE_KIND FileKind = EFFECT_RESOURCE_FILE_KIND::END;
			if (!CEffectDocumentCodec::Is_SafeResourceAssetId(
					ExistingBinding->strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
			{
				strOutError = "Artist F generic material override is not a safe DDS: " +
					ExistingBinding->strSlotId + ".";
				return false;
			}

			const auto SourceBinding = std::find_if(
				SourceElement.ResourceBindings.begin(),
				SourceElement.ResourceBindings.end(),
				[&Input](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
				{
					return Candidate.strSlotId == Input.strSlotId;
				});
			const EFFECT_RESOURCE_BINDING_DESC* pSourceBinding =
				SourceBinding == SourceElement.ResourceBindings.end() ?
				nullptr : &*SourceBinding;
			const std::optional<size_t> LaneIndex =
				ResolveArtist31470GenericMaterialLane(
					pSourceBinding, Input.strSlotId, eFamily, InOutExecution);
			if (!LaneIndex.has_value())
			{
				if (nullptr != pSourceBinding &&
					pSourceBinding->strAssetId != ExistingBinding->strAssetId)
				{
					strOutError =
						"Artist F generic DDS override has no unambiguous typed lane: " +
						ExistingElement.strElementId + "/" +
						ExistingBinding->strSlotId + ".";
					return false;
				}
				continue;
			}
			EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				InOutExecution.TextureLanes[*LaneIndex];
			if (ExistingTypedLaneIds.contains(Lane.strLaneId))
				continue;
			Lane.strAssetId = ExistingBinding->strAssetId;
		}
		return true;
	}

	bool_t Artist31470CarrierNearlyEqual(
		const f32_t Left, const f32_t Right)
	{
		return std::abs(Left - Right) <= 1.0e-5f *
			(std::max)({ 1.f, std::abs(Left), std::abs(Right) });
	}

	void NormalizeArtist31470LegacyGeneratedParticleCarrier(
		const uint32_t iOrder,
		EFFECT_ELEMENT_DESC& InOutElement)
	{
		if (EFFECT_ELEMENT_KIND::PARTICLE != InOutElement.eKind)
			return;
		EFFECT_DETAIL_DESC& Detail = InOutElement.Detail;
		float4_t& Multiply = Detail.Color.vColorMultiply;
		if ((iOrder == 2u || iOrder == 19u || iOrder == 31u) &&
			Artist31470CarrierNearlyEqual(Multiply.x, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.y, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.z, 1.f) &&
			Artist31470CarrierNearlyEqual(Multiply.w, 50.f) &&
			!Detail.LinearLerp.bColorMultiply)
		{
			/* This alpha-50 value was a bounded generic fallback for op6.
			   SourceRecipe now supplies the exact HDR color/alpha curve. */
			Multiply.w = 1.f;
		}
		if (iOrder == 23u &&
			Artist31470CarrierNearlyEqual(Multiply.x, 0.5f) &&
			Artist31470CarrierNearlyEqual(Multiply.y, 0.7f) &&
			Artist31470CarrierNearlyEqual(Multiply.z, 0.5f) &&
			Artist31470CarrierNearlyEqual(Multiply.w, 0.3f) &&
			Detail.LinearLerp.bColorMultiply &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.x, 0.5f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.y, 0.7f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.z, 0.5f) &&
			Artist31470CarrierNearlyEqual(
				Detail.LinearLerp.vEndColorMultiply.w, 0.f))
		{
			/* Same migration-only fallback: preserve a non-matching value as a
			   user-authored tint, but remove the exact generated green envelope. */
			Multiply = { 1.f, 1.f, 1.f, 1.f };
			Detail.LinearLerp.bColorMultiply = false;
			Detail.LinearLerp.vEndColorMultiply = { 1.f, 1.f, 1.f, 1.f };
		}
	}

	bool_t Try_ResolveArtistVisualV4ParticleColorAbi(
		const uint32_t iOpcode,
		uint32_t& iOutPolicy,
		uint32_t& iOutConsumedMask)
	{
		switch (iOpcode)
		{
		case 1u: /* BasicMissileTrail */
		case 2u: /* MakeFlow */
		case 3u: /* ComplexMissileTrail */
		case 6u: /* SPLA */
		case 7u: /* Flow02 recovered equation */
		case 8u: /* Skull recovered equation */
			iOutPolicy = 2u;
			iOutConsumedMask = 0x0fu;
			return true;
		case 4u: /* DistortionOnly consumes alpha coverage only. */
			iOutPolicy = 1u;
			iOutConsumedMask = 0x08u;
			return true;
		case 5u: /* Explicit zero-draw suppression. */
			iOutPolicy = 0u;
			iOutConsumedMask = 0u;
			return true;
		default:
			iOutPolicy = 0u;
			iOutConsumedMask = 0u;
			return false;
		}
	}

	bool_t ApplyArtist31470TrackAElementData(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_ELEMENT_DESC& SourceElement,
		const std::unordered_map<std::string,
			EFFECT_MATERIAL_EXECUTION_DESC>& MaterialSnapshots,
		EFFECT_ELEMENT_DESC& InOutElement,
		std::string& strOutError)
	{
		ARTIST31470_UNIFIED_FAMILY eFamily =
			ARTIST31470_UNIFIED_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily) ||
			!Artist31470UnifiedElementMatchesFamily(InOutElement, eFamily) ||
			SourceElement.strElementId != Emitter.strSourceElementId)
		{
			strOutError =
				"Artist F Track A seed no longer matches its authored Family/Element identity.";
			return false;
		}
		if (!Emitter.strMaterialOccurrenceId.has_value())
		{
			strOutError = "Artist F Track A seed has no material occurrence ID.";
			return false;
		}
		const auto Registry = Find_Artist31470ShaderRegistry(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
		if (!Registry.has_value() ||
			!Validate_Artist31470ShaderRegistryEmitterIdentity(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
				Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
		{
			strOutError =
				"Artist F Track A seed no longer matches the shader registry.";
			return false;
		}

		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
		{
			uint32_t iFixedBurstCount = 0u;
			if (!Try_ResolveArtist31470FixedBurstCount(
					Emitter, iFixedBurstCount, strOutError))
			{
				return false;
			}
			InOutElement.Detail.Particle.iBurstCount = iFixedBurstCount;
			InOutElement.Detail.Particle.bLocalSpace = Emitter.bLocalSpace;
			InOutElement.Detail.Particle.iRandomSeed =
				Emitter.Random.iEmitterRandomSeed;
			InOutElement.Detail.Particle.iMaxParticles = (std::max)(
				InOutElement.Detail.Particle.iMaxParticles,
				Emitter.iOperationalMaxParticles);
		}
		if (ARTIST31470_UNIFIED_FAMILY::MESH == eFamily)
		{
			const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
				SourceElement.SourceRecipe.GeometryBinding;
			const auto ModelBinding = std::find_if(
				InOutElement.ResourceBindings.begin(),
				InOutElement.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
			constexpr f32_t MODEL_PRE_SCALE = 0.01f;
			EFFECT_RESOURCE_FILE_KIND FileKind = EFFECT_RESOURCE_FILE_KIND::END;
			if (Emitter.strSizeUnitPolicy != "DIMENSIONLESS_AXIS_REORDER_ONLY" ||
				ModelBinding == InOutElement.ResourceBindings.end() ||
				!CEffectDocumentCodec::Is_SafeElementResourceAssetId(
					InOutElement.eKind, EFFECT_MESH_SHAPE_SLOT_ID,
					ModelBinding->strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::MODEL ||
				(Geometry.bEnabled &&
					(Geometry.strParticleScaleSemantics !=
						Emitter.strSizeUnitPolicy ||
					 !std::isfinite(Geometry.fCarrierGeometryPreScale) ||
					 std::abs(Geometry.fCarrierGeometryPreScale - MODEL_PRE_SCALE) >
						 1.0e-7f)))
			{
				strOutError =
					"Artist F MeshParticle lost its WModel geometry pre-scale contract: " +
					Emitter.strSourceElementId + ", source=" +
					Geometry.strAssetId + ", authored=" +
					(ModelBinding == InOutElement.ResourceBindings.end() ?
						std::string("<missing>") : ModelBinding->strAssetId) + ".";
				return false;
			}
			/* The native-v14 generic projection stored Mesh particle sizes in the
			   carrier's 0.01 geometry unit.  CModel already applies that pre-scale,
			   while ordinary particle playback consumes StartSize/EndSize as a
			   dimensionless instance scale.  Restore the source dimensionless value
			   from the immutable source document exactly once.  Assigning from the
			   source keeps repeated Upgrade operations idempotent and leaves authored
			   Transform and resource overrides untouched. */
			/* Some admitted source projections omit their geometry receipt while
			   the authored Element retains the verified WModel binding.  Artist F's
			   carrier contract is still the pinned 0.01 conversion in that case. */
			const f32_t fDimensionlessScale = 1.f / MODEL_PRE_SCALE;
			const float2_t vDimensionlessStart = {
				SourceElement.Detail.Particle.vStartSize.x * fDimensionlessScale,
				SourceElement.Detail.Particle.vStartSize.y * fDimensionlessScale };
			const float2_t vDimensionlessEnd = {
				SourceElement.Detail.Particle.vEndSize.x * fDimensionlessScale,
				SourceElement.Detail.Particle.vEndSize.y * fDimensionlessScale };
			if (!Is_Finite(vDimensionlessStart) ||
				vDimensionlessStart.x <= 0.f || vDimensionlessStart.y <= 0.f ||
				!Is_Finite(vDimensionlessEnd) ||
				vDimensionlessEnd.x < 0.f || vDimensionlessEnd.y < 0.f)
			{
				strOutError =
					"Artist F MeshParticle source dimensionless size is invalid: " +
					Emitter.strSourceElementId + ".";
				return false;
			}
			InOutElement.Detail.Particle.vStartSize = vDimensionlessStart;
			InOutElement.Detail.Particle.vEndSize = vDimensionlessEnd;
			InOutElement.Detail.Mesh.fModelPreScale = MODEL_PRE_SCALE;
		}

		/* The existing unified document already owns the root/follow basis in its
		   authored Transform.  Re-enabling attachment would apply that basis twice. */
		InOutElement.ActionCueAttachment = {};
		InOutElement.TransformInheritance = {};
		const auto MaterialSnapshot = MaterialSnapshots.find(
			SourceElement.strElementId);
		if (MaterialSnapshot == MaterialSnapshots.end())
		{
			strOutError =
				"Artist F material snapshot no longer matches its source Element.";
			return false;
		}
		if (MaterialSnapshot->second.bEnabled)
		{
			if ((Registry->eBackend !=
					EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
				 Registry->eBackend !=
					EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4) ||
				!Registry->bDrawAdmitted)
			{
				strOutError =
					"Artist F typed material snapshot disagrees with the shader registry.";
				return false;
			}
			EFFECT_MATERIAL_EXECUTION_DESC StagedExecution =
				MaterialSnapshot->second;
			/* Every ArtistVisualV4 particle opcode resolves and multiplies the
			   evaluated particle RGBA carrier in HLSL. Older snapshot metadata
			   described several opcodes as policy NONE even though the shader ABI
			   consumed all four channels; that mismatch flattened Track A's dark
			   ink/color-over-life carrier to identity white after authoring. */
			if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind &&
				StagedExecution.eBackend ==
					EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
			{
				if (!Try_ResolveArtistVisualV4ParticleColorAbi(
						StagedExecution.iOpcode,
						StagedExecution.iParticleColorPolicy,
						StagedExecution.iParticleColorConsumedMask))
				{
					strOutError =
						"ArtistVisualV4 particle opcode has no declared color ABI.";
					return false;
				}
				StagedExecution.iParticleColorSuppressedMask = 0u;
			}
			std::set<std::string, std::less<>> ExistingTypedLaneIds;
			if (InOutElement.Material.Execution.bEnabled)
			{
				for (EFFECT_MATERIAL_TEXTURE_LANE_DESC& StagedLane :
					StagedExecution.TextureLanes)
				{
					const auto ExistingLane = std::find_if(
						InOutElement.Material.Execution.TextureLanes.begin(),
						InOutElement.Material.Execution.TextureLanes.end(),
						[&StagedLane](
							const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Candidate)
						{
							return Candidate.strLaneId == StagedLane.strLaneId;
						});
					if (ExistingLane !=
							InOutElement.Material.Execution.TextureLanes.end() &&
						!ExistingLane->strAssetId.empty())
					{
						StagedLane.strAssetId = ExistingLane->strAssetId;
						ExistingTypedLaneIds.insert(StagedLane.strLaneId);
					}
				}
			}
			if (!PromoteArtist31470GenericMaterialOverrides(
					SourceElement, eFamily, ExistingTypedLaneIds, InOutElement,
					StagedExecution, strOutError))
			{
				return false;
			}
			InOutElement.Material.Execution = std::move(StagedExecution);
			InOutElement.Material.SourceMaterial = {};
			InOutElement.Material.strTemplateId =
				std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
			if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
			{
				const uint32_t iConsumedMask =
					InOutElement.Material.Execution.iDynamicConsumedMask & 0x0fu;
				f32_t* pStart =
					&InOutElement.Detail.Particle.vDynamicParameterStart.x;
				f32_t* pEnd =
					&InOutElement.Detail.Particle.vDynamicParameterEnd.x;
				for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
				{
					const uint32_t iBit = 1u << iComponent;
					if (0u == (iConsumedMask & iBit) ||
						0u != (InOutElement.Detail.Particle.
							iDynamicParameterComponentMask & iBit))
					{
						continue;
					}
					pStart[iComponent] = 1.f;
					pEnd[iComponent] = 1.f;
				}
				InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
					iConsumedMask;
			}
		}
		else if (Registry->eBackend ==
				EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
			Registry->eFidelity ==
				EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
			Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
		{
			const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
				SourceElement.Material.SourceMaterial;
			if (!SourceMaterial.bEnabled ||
				SourceMaterial.strRuntimeShaderProfileId !=
					"effect.ue3.missiletrail-01.v1")
			{
				strOutError =
					"Artist F #17 lost its bounded FiniteCommon material profile.";
				return false;
			}
			EFFECT_MATERIAL_DESC StagedMaterial = SourceElement.Material;
			StagedMaterial.Execution = {};
			for (EFFECT_NAMED_TEXTURE_DESC& StagedTexture :
				StagedMaterial.SourceMaterial.Textures)
			{
				const auto ExistingTexture = std::find_if(
					InOutElement.Material.SourceMaterial.Textures.begin(),
					InOutElement.Material.SourceMaterial.Textures.end(),
					[&StagedTexture](const EFFECT_NAMED_TEXTURE_DESC& Candidate)
					{
						return Candidate.strName == StagedTexture.strName;
					});
				if (ExistingTexture !=
						InOutElement.Material.SourceMaterial.Textures.end() &&
					!ExistingTexture->strAssetId.empty())
				{
					StagedTexture.strAssetId = ExistingTexture->strAssetId;
				}
			}
			InOutElement.Material = std::move(StagedMaterial);
		}
		else if (Registry->eBackend ==
				EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
			Registry->eFidelity ==
				EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
			!Registry->bDrawAdmitted &&
			(Emitter.Row.iOrder == 1u || Emitter.Row.iOrder == 16u ||
			 Emitter.Row.iOrder == 26u || Emitter.Row.iOrder == 33u))
		{
			InOutElement.Material.Execution = {};
			InOutElement.Material.Execution.bFailClosed = true;
			InOutElement.Material.SourceMaterial = {};
			InOutElement.Material.strTemplateId =
				std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
			InOutElement.bVisible = false;
		}
		else
		{
			strOutError =
				"Artist F disabled material row has no admitted authored policy.";
			return false;
		}
		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind &&
			!CEffectDocumentCodec::Apply_PortableAuthoredParticleRuntimeCarrier(
				SourceElement, InOutElement, strOutError))
		{
			return false;
		}
		NormalizeArtist31470LegacyGeneratedParticleCarrier(
			Emitter.Row.iOrder, InOutElement);
		return true;
	}

	bool_t InspectArtist31470TrackAUpgrade(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_DOCUMENT_DESC* pSourceDocument,
		const EFFECT_DOCUMENT_DESC& Document,
		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
		std::string& strOutError)
	{
		constexpr std::string_view SOURCE_RUNTIME_ID =
			"effect.artist.skill.31470";
		constexpr std::string_view SOURCE_CANDIDATE_ID =
			"effect.artist.skill.31470.native-v14.source-contract-candidate";
		constexpr std::string_view TARGET_ID =
			"effect.artist.skill.31470.unified";
		if (Program.strRuntimeCatalogAssetId != SOURCE_RUNTIME_ID ||
			(nullptr != pSourceDocument &&
			 pSourceDocument->strEffectAssetId != SOURCE_CANDIDATE_ID) ||
			Document.strEffectAssetId != TARGET_ID ||
			!CEffectDocumentCodec::Validate_Drawable(Document, strOutError))
		{
			if (strOutError.empty())
				strOutError = "Artist F authored migration identity is invalid.";
			return false;
		}

		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
		std::array<size_t, 4u> FamilyCounts{};
		std::set<std::string, std::less<>> JoinedTargetIds;
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
		{
			ARTIST31470_UNIFIED_FAMILY eFamily =
				ARTIST31470_UNIFIED_FAMILY::END;
			if (!Emitter.bVisible ||
				!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily))
			{
				continue;
			}
			const std::string strStableId =
				Artist31470UnifiedStableElementId(eFamily, Emitter.Row.strId);
			const EFFECT_ELEMENT_DESC* pSourceElement = nullptr;
			if (nullptr != pSourceDocument)
			{
				const auto SourceElement = std::find_if(
					pSourceDocument->Elements.begin(),
					pSourceDocument->Elements.end(),
					[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
					{
						return Candidate.strElementId == Emitter.strSourceElementId;
					});
				if (SourceElement != pSourceDocument->Elements.end())
					pSourceElement = &*SourceElement;
			}
			const auto TargetElement = std::find_if(
				Document.Elements.begin(), Document.Elements.end(),
				[&strStableId](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == strStableId;
				});
			if (strStableId.empty() ||
				(nullptr != pSourceDocument && nullptr == pSourceElement) ||
				TargetElement == Document.Elements.end() ||
				!JoinedTargetIds.insert(strStableId).second ||
				!Artist31470UnifiedElementMatchesFamily(*TargetElement, eFamily) ||
				!Emitter.strMaterialOccurrenceId.has_value())
			{
				strOutError =
					"Artist F authored migration lost a stable source/target join.";
				return false;
			}
			const auto Registry = Find_Artist31470ShaderRegistry(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
			if (!Registry.has_value() ||
				!Validate_Artist31470ShaderRegistryEmitterIdentity(
					Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
					Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
			{
				strOutError = "Artist F authored migration lost its registry row.";
				return false;
			}

			++Stats.iCoreElementCount;
			++FamilyCounts[static_cast<size_t>(eFamily)];
			if (EFFECT_ELEMENT_KIND::PARTICLE == TargetElement->eKind)
			{
				uint32_t iFixedBurstCount = 0u;
				if (!Try_ResolveArtist31470FixedBurstCount(
						Emitter, iFixedBurstCount, strOutError) ||
					TargetElement->Detail.Particle.iBurstCount !=
						iFixedBurstCount ||
					TargetElement->Detail.Particle.bLocalSpace !=
						Emitter.bLocalSpace ||
					TargetElement->Detail.Particle.iRandomSeed !=
						Emitter.Random.iEmitterRandomSeed ||
					TargetElement->Detail.Particle.iMaxParticles <
						Emitter.iOperationalMaxParticles)
				{
					if (strOutError.empty())
						strOutError =
							"Artist F authored particle carrier differs from Track A.";
					return false;
				}
				++Stats.iParticleElementCount;
				if (!ValidatePortableAuthoredParticleRuntimeCarrier(
						*TargetElement, strOutError))
				{
					if (strOutError.empty())
						strOutError =
							"Artist F authored particle lost its portable Track A runtime carrier.";
					return false;
				}
				if (nullptr != pSourceElement)
				{
					EFFECT_ELEMENT_DESC ExpectedCarrier = *TargetElement;
					if (!CEffectDocumentCodec::
							Apply_PortableAuthoredParticleRuntimeCarrier(
								*pSourceElement, ExpectedCarrier, strOutError))
					{
						return false;
					}
					std::ostringstream ActualRecipe;
					std::ostringstream ExpectedRecipe;
					Write_SourceRecipe(
						ActualRecipe, TargetElement->SourceRecipe, false);
					Write_SourceRecipe(
						ExpectedRecipe, ExpectedCarrier.SourceRecipe, false);
					if (ActualRecipe.str() != ExpectedRecipe.str())
					{
						strOutError =
							"Artist F authored particle runtime carrier differs from Track A.";
						return false;
					}
				}
				++Stats.iPortableParticleRecipeCount;
				Stats.iPortableParticleModuleCount +=
					TargetElement->SourceRecipe.Modules.size();
				for (const EFFECT_SOURCE_MODULE_DESC& Module :
					TargetElement->SourceRecipe.Modules)
				{
					Stats.iPortableParticleDistributionCount +=
						Module.Distributions.size();
				}
				Stats.iFixedBurstTotal += iFixedBurstCount;
				if (iFixedBurstCount > 0u)
					++Stats.iFixedBurstEmitterCount;
			}
			const bool_t bSourceAttachmentEnabled = nullptr != pSourceElement ?
				pSourceElement->ActionCueAttachment.bEnabled :
				Emitter.ActionCueAttachment.bEnabled;
			const bool_t bSourceAttachmentFollow = nullptr != pSourceElement ?
				pSourceElement->ActionCueAttachment.bFollow :
				Emitter.ActionCueAttachment.bFollow;
			if (bSourceAttachmentEnabled && bSourceAttachmentFollow)
			{
				++Stats.iFollowBasisBakedCount;
			}
			else if (bSourceAttachmentEnabled)
			{
				++Stats.iRootBasisBakedCount;
			}
			if (TargetElement->ActionCueAttachment.bEnabled ||
				TargetElement->TransformInheritance.bEnabled)
			{
				strOutError =
					"Artist F authored migration would apply an already-baked basis twice.";
				return false;
			}
			if (ARTIST31470_UNIFIED_FAMILY::MESH == eFamily)
			{
				if (std::abs(TargetElement->Detail.Mesh.fModelPreScale - 0.01f) >
					1.0e-7f)
				{
					strOutError = "Artist F authored MeshParticle lost pre-scale 0.01.";
					return false;
				}
				if (nullptr != pSourceElement)
				{
					const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
						pSourceElement->SourceRecipe.GeometryBinding;
					if (Emitter.strSizeUnitPolicy !=
							"DIMENSIONLESS_AXIS_REORDER_ONLY" ||
						(Geometry.bEnabled &&
							(Geometry.strParticleScaleSemantics !=
								Emitter.strSizeUnitPolicy ||
							 !std::isfinite(Geometry.fCarrierGeometryPreScale) ||
							 std::abs(Geometry.fCarrierGeometryPreScale - 0.01f) >
								 1.0e-7f)))
					{
						strOutError =
							"Artist F source MeshParticle lost its dimensionless size contract.";
						return false;
					}
					const f32_t fDimensionlessScale = 100.f;
					const float2_t vExpectedStart = {
						pSourceElement->Detail.Particle.vStartSize.x *
							fDimensionlessScale,
						pSourceElement->Detail.Particle.vStartSize.y *
							fDimensionlessScale };
					const float2_t vExpectedEnd = {
						pSourceElement->Detail.Particle.vEndSize.x *
							fDimensionlessScale,
						pSourceElement->Detail.Particle.vEndSize.y *
							fDimensionlessScale };
					const auto NearlyEqual = [](const f32_t Left, const f32_t Right)
					{
						return std::abs(Left - Right) <=
							1.0e-5f * (std::max)({ 1.f, std::abs(Left),
								std::abs(Right) });
					};
					if (!NearlyEqual(TargetElement->Detail.Particle.vStartSize.x,
							vExpectedStart.x) ||
						!NearlyEqual(TargetElement->Detail.Particle.vStartSize.y,
							vExpectedStart.y) ||
						!NearlyEqual(TargetElement->Detail.Particle.vEndSize.x,
							vExpectedEnd.x) ||
						!NearlyEqual(TargetElement->Detail.Particle.vEndSize.y,
							vExpectedEnd.y))
					{
						strOutError =
							"Artist F authored MeshParticle lost its dimensionless size contract.";
						return false;
					}
				}
				++Stats.iMeshPreScaleCount;
			}

			if (TargetElement->Material.Execution.bEnabled)
			{
				const bool_t bBackendMatches =
					(Registry->eBackend ==
						EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
					 (TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 ||
					  TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL)) ||
					(Registry->eBackend ==
						EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4 &&
					 TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4);
				if (!bBackendMatches || !Registry->bDrawAdmitted ||
					TargetElement->Material.SourceMaterial.bEnabled)
				{
					strOutError =
						"Artist F authored typed material has an invalid execution boundary.";
					return false;
				}
				if (TargetElement->eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
					TargetElement->Material.Execution.eBackend ==
						EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4)
				{
					uint32_t iExpectedPolicy = 0u;
					uint32_t iExpectedMask = 0u;
					if (!Try_ResolveArtistVisualV4ParticleColorAbi(
							TargetElement->Material.Execution.iOpcode,
							iExpectedPolicy, iExpectedMask) ||
						TargetElement->Material.Execution.iParticleColorPolicy !=
							iExpectedPolicy ||
						TargetElement->Material.Execution.iParticleColorConsumedMask !=
							iExpectedMask ||
						TargetElement->Material.Execution.
							iParticleColorSuppressedMask != 0u)
					{
						strOutError =
							"Artist F ArtistVisualV4 particle color ABI differs from its shader opcode.";
						return false;
					}
				}
				++Stats.iTypedMaterialCount;
			}
			else if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
				TargetElement->Material.SourceMaterial.bEnabled &&
				TargetElement->Material.SourceMaterial.
					strRuntimeShaderProfileId ==
						"effect.ue3.missiletrail-01.v1")
			{
				++Stats.iFiniteCommonCount;
			}
			else if (Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
				!Registry->bDrawAdmitted && !TargetElement->bVisible &&
				TargetElement->Material.Execution.bFailClosed &&
				!TargetElement->Material.SourceMaterial.bEnabled)
			{
				++Stats.iFailClosedCount;
			}
			else
			{
				strOutError =
					"Artist F authored material is neither typed, FiniteCommon, nor fail-closed.";
				return false;
			}
		}

		if (FamilyCounts != std::array<size_t, 4u>{ 13u, 16u, 3u, 1u } ||
			Stats.iCoreElementCount != 33u ||
			Stats.iParticleElementCount != 29u ||
			Stats.iFixedBurstEmitterCount != 26u ||
			Stats.iFixedBurstTotal != 167u ||
			Stats.iRootBasisBakedCount != 28u ||
			Stats.iFollowBasisBakedCount != 5u ||
			Stats.iTypedMaterialCount != 28u ||
			Stats.iFiniteCommonCount != 1u ||
			Stats.iFailClosedCount != 4u ||
			Stats.iMeshPreScaleCount != 13u ||
			Stats.iPortableParticleRecipeCount != 29u ||
			Stats.iPortableParticleModuleCount != 350u ||
			Stats.iPortableParticleDistributionCount != 564u ||
			JoinedTargetIds.size() != 33u)
		{
			strOutError =
				"Artist F authored migration denominator changed; expected Core33, Particle29 with 29 portable recipes (350 modules/564 distributions), burst 26/167, basis 28/5, pre-scale 13, material 28/1/4.";
			return false;
		}
		OutStats = Stats;
		strOutError.clear();
		return true;
	}
}

bool_t Client::CEffectDocumentCodec::Build_Artist31470UnifiedTrackAUpgrade(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>&
		MaterialSnapshots,
	const EFFECT_DOCUMENT_DESC& ExistingDocument,
	EFFECT_DOCUMENT_DESC& OutDocument,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	EFFECT_DOCUMENT_DESC Staged = ExistingDocument;
	std::set<std::string, std::less<>> JoinedTargetIds;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		ARTIST31470_UNIFIED_FAMILY eFamily =
			ARTIST31470_UNIFIED_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtist31470UnifiedFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const std::string strStableId =
			Artist31470UnifiedStableElementId(eFamily, Emitter.Row.strId);
		const auto SourceElement = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{
				return Candidate.strElementId == Emitter.strSourceElementId;
			});
		auto TargetElement = std::find_if(
			Staged.Elements.begin(), Staged.Elements.end(),
			[&strStableId](const EFFECT_ELEMENT_DESC& Candidate)
			{
				return Candidate.strElementId == strStableId;
			});
		if (strStableId.empty() ||
			SourceElement == SourceDocument.Elements.end() ||
			TargetElement == Staged.Elements.end() ||
			!JoinedTargetIds.insert(strStableId).second)
		{
			strOutError =
				"Artist F authored migration rejected a missing or duplicate stable Element join.";
			return false;
		}
		if (!ApplyArtist31470TrackAElementData(
				Emitter, *SourceElement, MaterialSnapshots,
				*TargetElement, strOutError))
		{
			return false;
		}
	}
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
	if (!InspectArtist31470TrackAUpgrade(
			Program, &SourceDocument, Staged, Stats, strOutError))
	{
		return false;
	}
	OutDocument = std::move(Staged);
	OutStats = Stats;
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Validate_Artist31470UnifiedTrackAUpgrade(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const EFFECT_DOCUMENT_DESC& Document,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	return InspectArtist31470TrackAUpgrade(
		Program, &SourceDocument, Document, OutStats, strOutError);
}

bool_t Client::CEffectDocumentCodec::
	Validate_Artist31470UnifiedAuthoredReadiness(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_DOCUMENT_DESC& Document,
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
	std::string& strOutError)
{
	return InspectArtist31470TrackAUpgrade(
		Program, nullptr, Document, OutStats, strOutError);
}

bool_t Client::CEffectDocumentCodec::
	Validate_ReconstructedRuntimeDrawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.bSourceContract ||
		Document.Elements.empty())
	{
		strOutError =
			"Reconstructed runtime drawable identity is invalid.";
		return false;
	}
	EFFECT_DOCUMENT_DESC LegacyValidationProjection = Document;
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		const EFFECT_SOURCE_SPACE eExpectedSourceSpace =
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::SCREEN_POST ?
				EFFECT_SOURCE_SPACE::SCREEN_SPACE_V1 :
				EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		if (Element.Renderer.eType >= EFFECT_RENDERER_TYPE::END ||
			Kind_ForRenderer(Element.Renderer.eType) != Element.eKind ||
			Element.Renderer.eSourceSpace != eExpectedSourceSpace)
		{
			strOutError =
				"Reconstructed runtime renderer type/source-space does not match "
				"its Element kind.";
			return false;
		}
		LegacyValidationProjection.Elements[iElement].Renderer = {};
		/*
		 * Validate_Drawable below deliberately exercises the legacy carrier
		 * projection after stripping the typed Renderer.  Source TypeDataMesh
		 * rotation is renderer-owned, so retaining it in that legacy-only copy
		 * would create an impossible "no mesh renderer + mesh carrier rotation"
		 * document.  The original typed Document remains unchanged and is
		 * validated by the renderer contract above.
		 */
		LegacyValidationProjection.Elements[iElement].Detail.Mesh.
			vSourceTypeDataRotationDegrees = {};
	}
	if (!Validate_Drawable(LegacyValidationProjection, strOutError))
		return false;
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::
	Validate_Artist31470ReconstructedRuntimeDrawable(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	constexpr std::string_view ARTIST_31470_EFFECT_ID =
		"effect.artist.skill.31470";
	if (Document.strEffectAssetId != ARTIST_31470_EFFECT_ID ||
		Document.Elements.size() != 35u)
	{
		strOutError =
			"Artist 31470 reconstructed runtime drawable identity is invalid.";
		return false;
	}
	if (!Validate_ReconstructedRuntimeDrawable(Document, strOutError))
		return false;
	std::array<uint32_t,
		static_cast<size_t>(EFFECT_RENDERER_TYPE::END)> RendererCounts{};
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		++RendererCounts[static_cast<size_t>(Element.Renderer.eType)];
	if (RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::MESH_PARTICLE)] != 13u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::SPRITE_PARTICLE)] != 16u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::DECAL_PARTICLE)] != 3u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::CASCADE_RIBBON)] != 1u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::LIGHT_PARTICLE)] != 1u ||
		RendererCounts[static_cast<size_t>(
			EFFECT_RENDERER_TYPE::SCREEN_POST)] != 1u)
	{
		strOutError =
			"Artist 31470 reconstructed runtime renderer denominator changed.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectDocumentCodec::Parse(
	const std::string_view Json,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	DATA_JSON_PARSE_LIMITS EffectDocumentLimits;
	EffectDocumentLimits.iMaximumBytes = 64u * 1024u * 1024u;
	EffectDocumentLimits.iMaximumDepth = 64u;
	EffectDocumentLimits.iMaximumValues = 3'000'000u;
	if (!CDataJson::Parse(Json, Root, strOutError, EffectDocumentLimits))
		return false;
	return Parse_Value(Root, OutDocument, strOutError);
}

bool_t Client::CEffectDocumentCodec::Parse_Value(
	const DATA_JSON_VALUE& Root,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	if (!Root.Is_Object())
	{
		strOutError = "Effect document root must be an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("version");
	const DATA_JSON_VALUE* pAssetId = Root.Find("effectAssetId");
	const DATA_JSON_VALUE* pDisplayName = Root.Find("displayName");
	const DATA_JSON_VALUE* pPurpose = Root.Find("purpose");
	const DATA_JSON_VALUE* pRuntimeExtensions = Root.Find("runtimeExtensions");
	const DATA_JSON_VALUE* pElements = Root.Find("elements");
	if ((nullptr != pSchema && (!pSchema->Is_String() || pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA)) ||
		nullptr == pVersion || !pVersion->Is_Number() ||
		nullptr == pAssetId || !pAssetId->Is_String() ||
		nullptr == pDisplayName || !pDisplayName->Is_String() ||
		nullptr == pElements || !pElements->Is_Array())
	{
		strOutError = "Effect document fields or types are invalid.";
		return false;
	}
	const double Version = pVersion->Get_Number();
	if (!std::isfinite(Version) || Version != std::floor(Version) ||
		Version < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
		Version > EFFECT_AUTHORING_MAX_SUPPORTED_VERSION)
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}
	const uint32_t iSourceVersion = static_cast<uint32_t>(Version);
	const bool_t bSourceContract =
		iSourceVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION;
	const bool_t bRuntimeExtensionDocument =
		iSourceVersion == EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION;
	if (bSourceContract)
	{
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA ||
			nullptr == pPurpose || !pPurpose->Is_String() ||
			pPurpose->Get_String() != "source_contract" ||
			!Validate_ExactFields(Root,
				{ "schema", "version", "purpose", "effectAssetId",
					"displayName", "particleSystem", "modelCues", "elements" },
				"Effect source-contract document", strOutError))
		{
			if (strOutError.empty())
				strOutError = "Native-v14 source-contract root is invalid.";
			return false;
		}
	}
	else if (bRuntimeExtensionDocument)
	{
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != EFFECT_DOCUMENT_SCHEMA ||
			nullptr != pPurpose || nullptr == pRuntimeExtensions ||
			!pRuntimeExtensions->Is_Object() ||
			!Validate_ExactFields(Root,
				{ "schema", "version", "effectAssetId", "displayName",
					"particleSystem", "modelCues", "runtimeExtensions",
					"elements" },
				"Effect authored-v15 document", strOutError))
		{
			if (strOutError.empty())
				strOutError = "Effect authored-v15 root is invalid.";
			return false;
		}
	}
	else if (nullptr != pPurpose)
	{
		strOutError = "Legacy Effect documents cannot declare source-contract purpose.";
		return false;
	}
	else if (nullptr != pRuntimeExtensions)
	{
		strOutError =
			"Effect runtimeExtensions require authored document version 15.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Staged.iLoadedFormatVersion = iSourceVersion;
	Staged.bSourceContract = bSourceContract;
	Staged.strEffectAssetId = pAssetId->Get_String();
	Staged.strDisplayName = pDisplayName->Get_String();
	if (bRuntimeExtensionDocument &&
		!Read_AuthoredRuntimeExtensions(
			*pRuntimeExtensions, Staged.RuntimeExtensions, strOutError))
	{
		return false;
	}
	if (iSourceVersion >= 8u)
	{
		const DATA_JSON_VALUE* pParticleSystem = Root.Find("particleSystem");
		if (nullptr == pParticleSystem || !pParticleSystem->Is_Object() ||
			(bSourceContract && !Validate_ExactFields(*pParticleSystem,
				{ "uniformScaleMultiplier", "yawOffsetDegrees",
					"directionYawDegrees", "initialSpeedMultiplier" },
				"Effect source-contract particleSystem", strOutError)) ||
			!Read_Float(*pParticleSystem, "uniformScaleMultiplier",
				Staged.ParticleSystem.fUniformScaleMultiplier, strOutError) ||
			!Read_Float(*pParticleSystem, "yawOffsetDegrees",
				Staged.ParticleSystem.fYawOffsetDegrees, strOutError) ||
			!Read_Float(*pParticleSystem, "directionYawDegrees",
				Staged.ParticleSystem.fDirectionYawDegrees, strOutError) ||
			!Read_Float(*pParticleSystem, "initialSpeedMultiplier",
				Staged.ParticleSystem.fInitialSpeedMultiplier, strOutError))
		{
			if (strOutError.empty())
				strOutError = "Effect particleSystem fields are invalid.";
			return false;
		}
	}
	const DATA_JSON_VALUE* pModelCues = Root.Find("modelCues");
	if (nullptr != pModelCues)
	{
		if (!pModelCues->Is_Array())
		{
			strOutError = "Effect modelCues must be an array.";
			return false;
		}
		Staged.ModelCues.reserve(pModelCues->Get_Array().size());
		for (const DATA_JSON_VALUE& CueValue : pModelCues->Get_Array())
		{
			if (!CueValue.Is_Object() ||
				(bSourceContract && !Validate_ExactFields(CueValue,
					{ "cueId", "modelAssetId", "clipName",
						"startDelaySeconds", "durationSeconds", "alphaMode",
						"opacity", "colorMultiply", "holdLastFrame", "visible",
						"localTransform", "assetPreTransform" },
					"Effect source-contract Model Cue", strOutError)))
			{
				strOutError = "Effect Model Cue must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pCueId = CueValue.Find("cueId");
			const DATA_JSON_VALUE* pModelAssetId =
				CueValue.Find("modelAssetId");
			const DATA_JSON_VALUE* pClipName = CueValue.Find("clipName");
			const DATA_JSON_VALUE* pAlphaMode = CueValue.Find("alphaMode");
			const DATA_JSON_VALUE* pVisible = CueValue.Find("visible");
			EFFECT_MODEL_CUE_DESC Cue;
			if (nullptr == pCueId || !pCueId->Is_String() ||
				nullptr == pModelAssetId || !pModelAssetId->Is_String() ||
				nullptr == pClipName || !pClipName->Is_String() ||
				nullptr == pVisible || !pVisible->Is_Boolean() ||
				!Read_Float(CueValue, "startDelaySeconds",
					Cue.fStartDelaySeconds, strOutError) ||
				!Read_Float(CueValue, "durationSeconds",
					Cue.fDurationSeconds, strOutError) ||
				!Read_OptionalFloat(CueValue, "opacity", Cue.fOpacity,
					strOutError) ||
				!Read_OptionalArray(CueValue, "colorMultiply",
					&Cue.vColorMultiply.x, 4u, strOutError) ||
				!Read_OptionalBool(CueValue, "holdLastFrame",
					Cue.bHoldLastFrame, strOutError) ||
				!Read_ModelCueTransform(CueValue, Cue, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect Model Cue fields are invalid.";
				return false;
			}
			Cue.strCueId = pCueId->Get_String();
			Cue.strModelAssetId = pModelAssetId->Get_String();
			Cue.strClipName = pClipName->Get_String();
			if (nullptr != pAlphaMode &&
				(!pAlphaMode->Is_String() ||
				 !Parse_Token(pAlphaMode->Get_String(),
					 MODEL_CUE_ALPHA_MODE_TOKENS,
					 std::size(MODEL_CUE_ALPHA_MODE_TOKENS),
					 Cue.eAlphaMode)))
			{
				strOutError = "Effect Model Cue alphaMode is invalid.";
				return false;
			}
			Cue.bVisible = pVisible->Get_Boolean();
			Staged.ModelCues.push_back(std::move(Cue));
		}
	}
	Staged.Elements.reserve(pElements->Get_Array().size());
	for (const DATA_JSON_VALUE& ElementValue : pElements->Get_Array())
	{
		if (!bSourceContract && ElementValue.Is_Object() &&
			nullptr != ElementValue.Find("renderer"))
		{
			strOutError =
				"Legacy Effect element contains native-v14 renderer evidence.";
			return false;
		}
		if (!ElementValue.Is_Object() ||
			(bSourceContract && !Validate_ExactFields(ElementValue,
				{ "id", "displayName", "groupId", "sourceNode", "visible",
					"kind", "renderer", "resources", "unboundResources",
					"material",
					"actionCueAttachment", "transformInheritance", "detail",
					"sourceRecipe", "sourcePresentation" },
				"Effect source-contract Element", strOutError)) ||
			(bRuntimeExtensionDocument && !Validate_ExactFields(ElementValue,
				{ "id", "displayName", "groupId", "sourceNode", "visible",
					"kind", "runtimeCarrier", "compositionLayer", "resources",
					"unboundResources", "material", "actionCueAttachment",
					"transformInheritance", "detail", "sourceRecipe",
					"sourcePresentation", "authoringOverrides" },
				"Effect authored-v15 Element", strOutError)))
		{
			strOutError = "Effect Element must be an object.";
			return false;
		}
		const DATA_JSON_VALUE* pId = ElementValue.Find("id");
		const DATA_JSON_VALUE* pKind = ElementValue.Find("kind");
		const DATA_JSON_VALUE* pResources = ElementValue.Find("resources");
		const DATA_JSON_VALUE* pMaterial = ElementValue.Find("material");
		EFFECT_ELEMENT_DESC Element;
		if (nullptr == pId || !pId->Is_String() || nullptr == pKind || !pKind->Is_String() ||
			nullptr == pResources || !pResources->Is_Array() || nullptr == pMaterial || !pMaterial->Is_Object() ||
			!Parse_Token(pKind->Get_String(), KIND_TOKENS, std::size(KIND_TOKENS), Element.eKind))
		{
			strOutError = "Effect Element identity, kind, resources, or material is invalid.";
			return false;
		}
		if (const DATA_JSON_VALUE* pCompositionLayer =
			ElementValue.Find("compositionLayer"))
		{
			if (!pCompositionLayer->Is_String() ||
				!Parse_Token(pCompositionLayer->Get_String(),
					COMPOSITION_LAYER_TOKENS,
					std::size(COMPOSITION_LAYER_TOKENS),
					Element.eCompositionLayer))
			{
				strOutError = "Effect Element compositionLayer is invalid.";
				return false;
			}
		}
		Element.strElementId = pId->Get_String();
		if (const DATA_JSON_VALUE* pRuntimeCarrier =
			ElementValue.Find("runtimeCarrier"))
		{
			if (!bRuntimeExtensionDocument || !pRuntimeCarrier->Is_Object() ||
				!Read_AuthoredRuntimeCarrier(
					*pRuntimeCarrier, Element.RuntimeCarrier, strOutError))
			{
				if (strOutError.empty())
					strOutError =
						"Effect runtimeCarrier is valid only in authored-v15 documents.";
				return false;
			}
		}
		if (bSourceContract)
		{
			const DATA_JSON_VALUE* pRenderer = ElementValue.Find("renderer");
			if (nullptr == pRenderer || !pRenderer->Is_Object() ||
				!Read_Renderer(*pRenderer, Element.Renderer, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source-contract renderer is missing.";
				return false;
			}
		}
		if (iSourceVersion >= 6u)
		{
			const DATA_JSON_VALUE* pElementDisplayName =
				ElementValue.Find("displayName");
			const DATA_JSON_VALUE* pGroupId = ElementValue.Find("groupId");
			const DATA_JSON_VALUE* pSourceNode = ElementValue.Find("sourceNode");
			const DATA_JSON_VALUE* pVisible = ElementValue.Find("visible");
			if (nullptr == pElementDisplayName ||
				!pElementDisplayName->Is_String() ||
				nullptr == pGroupId || !pGroupId->Is_String() ||
				nullptr == pSourceNode || !pSourceNode->Is_String() ||
				nullptr == pVisible || !pVisible->Is_Boolean())
			{
				strOutError = "Effect Element metadata is invalid.";
				return false;
			}
			Element.strDisplayName = pElementDisplayName->Get_String();
			Element.strGroupId = pGroupId->Get_String();
			Element.strSourceNode = pSourceNode->Get_String();
			Element.bVisible = pVisible->Get_Boolean();
		}
		else
		{
			Element.strDisplayName = Element.strElementId;
		}
		if (const DATA_JSON_VALUE* pActionCueAttachment =
			ElementValue.Find("actionCueAttachment"))
		{
			if (!pActionCueAttachment->Is_Object() ||
				!Read_ActionCueAttachment(*pActionCueAttachment,
					Element.ActionCueAttachment, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect Action cue attachment is invalid.";
				return false;
			}
		}
		const DATA_JSON_VALUE* pTransformInheritance =
			ElementValue.Find("transformInheritance");
		if ((iSourceVersion >= 13u && nullptr == pTransformInheritance) ||
			(nullptr != pTransformInheritance &&
				(!pTransformInheritance->Is_Object() ||
					!Read_TransformInheritance(*pTransformInheritance,
						Element.TransformInheritance, strOutError))))
		{
			if (strOutError.empty())
				strOutError = "Effect transform inheritance is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& ResourceValue : pResources->Get_Array())
		{
			if (!ResourceValue.Is_Object() ||
				(bSourceContract && !Validate_ExactFields(ResourceValue,
					{ "slotId", "assetId" },
					"Effect source-contract resource", strOutError)))
			{
				strOutError = "Effect resource must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pSlot = ResourceValue.Find(
				iSourceVersion >= 6u ? "slotId" : "slot");
			const DATA_JSON_VALUE* pResourceId = ResourceValue.Find("assetId");
			EFFECT_RESOURCE_BINDING_DESC Binding;
			if (nullptr == pSlot || !pSlot->Is_String() ||
				nullptr == pResourceId || !pResourceId->Is_String())
			{
				strOutError = "Effect resource binding is invalid.";
				return false;
			}
			Binding.strSlotId = pSlot->Get_String();
			Binding.strAssetId = pResourceId->Get_String();
			Element.ResourceBindings.push_back(std::move(Binding));
		}
		/* Optional: the seeder records every source texture that did not fit a
		   slot so the document keeps the full original reference list. */
		if (const DATA_JSON_VALUE* pUnbound =
			ElementValue.Find("unboundResources"))
		{
			if (!pUnbound->Is_Array())
			{
				strOutError = "Effect unboundResources must be an array.";
				return false;
			}
			for (const DATA_JSON_VALUE& UnboundValue : pUnbound->Get_Array())
			{
				if (!UnboundValue.Is_String())
				{
					strOutError =
						"Effect unboundResources entry must be a string.";
					return false;
				}
				Element.UnboundSourceResources.push_back(
					UnboundValue.Get_String());
			}
		}
		const DATA_JSON_VALUE* pTemplateId = pMaterial->Find("templateId");
		if (const DATA_JSON_VALUE* pColorTexturesSRGB =
			pMaterial->Find("colorTexturesSRGB"))
		{
			if (!pColorTexturesSRGB->Is_Boolean())
			{
				strOutError = "Effect Material colorTexturesSRGB must be a boolean.";
				return false;
			}
			Element.Material.bColorTexturesSRGB = pColorTexturesSRGB->Get_Boolean();
		}
		if (iSourceVersion >= 6u)
		{
			if (nullptr == pTemplateId || !pTemplateId->Is_String())
			{
				strOutError = "Effect Material Template ID is invalid.";
				return false;
			}
			Element.Material.strTemplateId = pTemplateId->Get_String();
		}
		if (iSourceVersion >= 10u)
		{
			const DATA_JSON_VALUE* pSourceMaterialPath =
				pMaterial->Find("sourceMaterialPath");
			if (nullptr == pSourceMaterialPath ||
				!pSourceMaterialPath->Is_String())
			{
				strOutError = "Effect source Material path is invalid.";
				return false;
			}
			Element.Material.strSourceMaterialPath =
				pSourceMaterialPath->Get_String();
		}
		if (iSourceVersion >= 11u)
		{
			const DATA_JSON_VALUE* pSourceProfile =
				pMaterial->Find("sourceProfile");
			if (nullptr == pSourceProfile ||
				!Read_SourceMaterialProfile(*pSourceProfile,
					Element.Material.SourceMaterial, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source Material profile is invalid.";
				return false;
			}
		}
		if (const DATA_JSON_VALUE* pExecution =
			pMaterial->Find("execution"))
		{
			if (!pExecution->Is_Object() ||
				!Read_MaterialExecution(*pExecution,
					Element.Material.Execution, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect authored Material execution is invalid.";
				return false;
			}
		}
		const DATA_JSON_VALUE* pProfile = pMaterial->Find("renderProfile");
		if (nullptr == pProfile || !pProfile->Is_String() ||
			!Parse_Token(pProfile->Get_String(), PROFILE_TOKENS, std::size(PROFILE_TOKENS), Element.Material.eRenderProfile))
		{
			strOutError = "Effect render profile is invalid.";
			return false;
		}
		if (!bSourceContract && iSourceVersion >= 11u &&
			Element.Material.strTemplateId ==
				EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
			!Element.Material.SourceMaterial.bEnabled)
		{
			strOutError =
				"Effect source Material template requires a staged profile.";
			return false;
		}
		if (iSourceVersion >= 4u)
		{
			const DATA_JSON_VALUE* pDetail = ElementValue.Find("detail");
			if (nullptr == pDetail || !pDetail->Is_Object() ||
				!Read_CommonDetail(*pDetail, Element.Detail, strOutError))
			{
				return false;
			}
			if (iSourceVersion >= 5u &&
				!Read_V5Detail(*pDetail, Element.Detail, strOutError))
			{
				return false;
			}
			if (iSourceVersion >= 12u &&
				!Read_PresentationDetail(*pDetail, Element.Detail, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect presentation Detail is invalid.";
				return false;
			}
		}
		if (iSourceVersion >= 9u)
		{
			const DATA_JSON_VALUE* pSourceRecipe =
				ElementValue.Find("sourceRecipe");
			if (nullptr == pSourceRecipe || !pSourceRecipe->Is_Object() ||
				!Read_SourceRecipe(*pSourceRecipe, Element.SourceRecipe,
					bSourceContract,
					strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source recipe is invalid.";
				return false;
			}
		}
		if (iSourceVersion >= 12u)
		{
			const DATA_JSON_VALUE* pSourcePresentation =
				ElementValue.Find("sourcePresentation");
			if (nullptr == pSourcePresentation ||
				!pSourcePresentation->Is_Object() ||
				!Read_SourcePresentation(*pSourcePresentation,
					Element.SourcePresentation, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect source presentation is invalid.";
				return false;
			}
		}
		const DATA_JSON_VALUE* pAuthoringOverrides =
			ElementValue.Find("authoringOverrides");
		if (nullptr != pAuthoringOverrides)
		{
			if (bSourceContract)
			{
				strOutError =
					"Effect source-contract Element cannot carry authoring overrides.";
				return false;
			}
			if (!pAuthoringOverrides->Is_Object() ||
				!Read_AuthoringOverrides(*pAuthoringOverrides, Element,
					strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect authoring overrides are invalid.";
				return false;
			}
		}
		Staged.Elements.push_back(std::move(Element));
	}
	if (!bSourceContract &&
		!Apply_Warlord17090RetainedSourceProjection(Staged, strOutError))
	{
		return false;
	}
	if (!(bSourceContract ? Validate_SourceContract(Staged, strOutError) :
		Validate(Staged, strOutError)))
		return false;
	OutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}

std::string Client::CEffectDocumentCodec::Serialize(
	const EFFECT_DOCUMENT_DESC& Document)
{
	const uint32_t iSerializedVersion = Document.iLoadedFormatVersion;
	const bool_t bSourceContract =
		iSerializedVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION &&
		Document.bSourceContract;
	std::ostringstream Output;
	Output << std::setprecision(9) << "{\n"
		<< "  \"schema\": \"" << EFFECT_DOCUMENT_SCHEMA << "\",\n"
		<< "  \"version\": " << iSerializedVersion << ",\n";
	if (bSourceContract)
		Output << "  \"purpose\": \"source_contract\",\n";
	Output << "  \"effectAssetId\": \"" << CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"displayName\": \"" << CDataJson::Escape(Document.strDisplayName) << "\",\n"
		<< "  \"particleSystem\": { \"uniformScaleMultiplier\": "
		<< Document.ParticleSystem.fUniformScaleMultiplier
		<< ", \"yawOffsetDegrees\": "
		<< Document.ParticleSystem.fYawOffsetDegrees
		<< ", \"directionYawDegrees\": "
		<< Document.ParticleSystem.fDirectionYawDegrees
		<< ", \"initialSpeedMultiplier\": "
		<< Document.ParticleSystem.fInitialSpeedMultiplier << " },\n"
		<< "  \"modelCues\": [";
	for (size_t iCue = 0u; iCue < Document.ModelCues.size(); ++iCue)
	{
		const EFFECT_MODEL_CUE_DESC& Cue = Document.ModelCues[iCue];
		Output << (0u == iCue ? "\n" : ",\n")
			<< "    { \"cueId\": \"" << CDataJson::Escape(Cue.strCueId)
			<< "\", \"modelAssetId\": \""
			<< CDataJson::Escape(Cue.strModelAssetId)
			<< "\", \"clipName\": \""
			<< CDataJson::Escape(Cue.strClipName)
			<< "\", \"startDelaySeconds\": " << Cue.fStartDelaySeconds
			<< ", \"durationSeconds\": " << Cue.fDurationSeconds
			<< ", \"opacity\": " << Cue.fOpacity
			<< ", \"colorMultiply\": ";
		Write_Float4(Output, Cue.vColorMultiply);
		Output << ", \"holdLastFrame\": "
			<< (Cue.bHoldLastFrame ? "true" : "false")
			<< ", \"alphaMode\": \""
			<< MODEL_CUE_ALPHA_MODE_TOKENS[static_cast<size_t>(Cue.eAlphaMode)]
			<< "\""
			<< ", \"visible\": " << (Cue.bVisible ? "true" : "false")
			<< ",\n      \"localTransform\": { \"position\": ";
		Write_Float3(Output, Cue.LocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.LocalTransform.vRotationDegrees);
		Output << ", \"revolutionDegreesPerSecond\": ";
		Write_Float3(Output, Cue.LocalTransform.vRevolutionDegreesPerSecond);
		Output << ", \"scale\": ";
		Write_Float3(Output, Cue.LocalTransform.vScale);
		Output << ", \"velocityPerSecond\": ";
		Write_Float3(Output, Cue.LocalTransform.vVelocityPerSecond);
		Output << " },\n      \"assetPreTransform\": { \"scale\": ";
		Write_Float3(Output, Cue.vAssetPreScale);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.vAssetPreRotationDegrees);
		Output << " } }";
	}
	if (!Document.ModelCues.empty())
		Output << '\n';
	Output << "  ],\n";
	if (iSerializedVersion ==
		EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION)
	{
		Output << "  \"runtimeExtensions\": {\n"
			<< "    \"formatVersion\": "
			<< Document.RuntimeExtensions.iFormatVersion << ",\n"
			<< "    \"bakedEdgeHistories\": [";
		for (size_t iHistory = 0u;
			iHistory < Document.RuntimeExtensions.BakedEdgeHistories.size();
			++iHistory)
		{
			const EFFECT_AUTHORED_RUNTIME_EDGE_HISTORY_DESC& History =
				Document.RuntimeExtensions.BakedEdgeHistories[iHistory];
			Output << (0u == iHistory ? "\n" : ",\n")
				<< "      {\n"
				<< "        \"historyId\": \""
				<< CDataJson::Escape(History.strHistoryId) << "\",\n"
				<< "        \"coordinateBasis\": \""
				<< AUTHORED_RUNTIME_COORDINATE_BASIS_TOKENS[
					static_cast<size_t>(History.eCoordinateBasis)] << "\",\n"
				<< "        \"sourceEndTimeSeconds\": "
				<< History.fSourceEndTimeSeconds << ",\n"
				<< "        \"playbackClampSeconds\": "
				<< History.fPlaybackClampSeconds << ",\n"
				<< "        \"samples\": [";
			for (size_t iSample = 0u; iSample < History.Samples.size();
				++iSample)
			{
				const EFFECT_AUTHORED_RUNTIME_EDGE_SAMPLE_DESC& Sample =
					History.Samples[iSample];
				Output << (0u == iSample ? "\n" : ",\n")
					<< "          { \"relativeTimeSeconds\": "
					<< Sample.fRelativeTimeSeconds
					<< ", \"firstEdgeUE3Cm\": ";
				Write_Float3(Output, Sample.vFirstEdgeUE3Cm);
				Output << ", \"controlPointUE3Cm\": ";
				Write_Float3(Output, Sample.vControlPointUE3Cm);
				Output << ", \"secondEdgeUE3Cm\": ";
				Write_Float3(Output, Sample.vSecondEdgeUE3Cm);
				Output << " }";
			}
			if (!History.Samples.empty())
				Output << '\n';
			Output << "        ]\n      }";
		}
		if (!Document.RuntimeExtensions.BakedEdgeHistories.empty())
			Output << '\n';
		Output << "    ]\n  },\n";
	}
	Output << "  \"elements\": [";
	for (size_t iElement = 0u; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		Output << (0u == iElement ? "\n" : ",\n")
			<< "    {\n      \"id\": \"" << CDataJson::Escape(Element.strElementId) << "\",\n"
			<< "      \"displayName\": \"" << CDataJson::Escape(Element.strDisplayName) << "\",\n"
			<< "      \"groupId\": \"" << CDataJson::Escape(Element.strGroupId) << "\",\n"
			<< "      \"sourceNode\": \"" << CDataJson::Escape(Element.strSourceNode) << "\",\n"
			<< "      \"visible\": " << (Element.bVisible ? "true" : "false") << ",\n"
			<< "      \"kind\": \"" << To_Token(Element.eKind) << "\",\n";
		if (iSerializedVersion ==
				EFFECT_AUTHORED_RUNTIME_EXTENSION_FORMAT_VERSION &&
			!Element.RuntimeCarrier.Is_Empty())
		{
			const EFFECT_AUTHORED_RUNTIME_CARRIER_DESC& Carrier =
				Element.RuntimeCarrier;
			Output << "      \"runtimeCarrier\": { \"formatVersion\": "
				<< Carrier.iFormatVersion << ", \"kind\": \""
				<< AUTHORED_RUNTIME_CARRIER_KIND_TOKENS[
					static_cast<size_t>(Carrier.eKind)]
				<< "\", \"admission\": \""
				<< AUTHORED_RUNTIME_CARRIER_ADMISSION_TOKENS[
					static_cast<size_t>(Carrier.eAdmission)] << "\"";
			switch (Carrier.eKind)
			{
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1:
				Output << ", \"typeDataModuleStableId\": \""
					<< CDataJson::Escape(Carrier.strTypeDataModuleStableId)
					<< "\"";
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				ANIMATION_TRAIL_BAKED_EDGE_V1:
				Output << ", \"historyId\": \""
					<< CDataJson::Escape(Carrier.strHistoryId) << "\"";
				break;
			case EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::
				LIGHT_BAKED_EDGE_ATTACHMENT_V1:
				Output << ", \"historyId\": \""
					<< CDataJson::Escape(Carrier.strHistoryId)
					<< "\", \"edgeLane\": \""
					<< AUTHORED_RUNTIME_BAKED_EDGE_LANE_TOKENS[
						static_cast<size_t>(Carrier.eEdgeLane)] << "\"";
				break;
			default:
				break;
			}
			Output << " },\n";
		}
		if (Element.eCompositionLayer != EFFECT_COMPOSITION_LAYER::NORMAL)
		{
			Output << "      \"compositionLayer\": \""
				<< COMPOSITION_LAYER_TOKENS[static_cast<size_t>(
					Element.eCompositionLayer)] << "\",\n";
		}
		if (bSourceContract)
			Write_Renderer(Output, Element.Renderer);
		Output << "      \"resources\": [";
		for (size_t iResource = 0u; iResource < Element.ResourceBindings.size(); ++iResource)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding = Element.ResourceBindings[iResource];
			Output << (0u == iResource ? "\n" : ",\n")
				<< "        { \"" << (iSerializedVersion >= 6u ?
					"slotId" : "slot") << "\": \""
				<< CDataJson::Escape(Binding.strSlotId)
				<< "\", \"assetId\": \"" << CDataJson::Escape(Binding.strAssetId) << "\" }";
		}
		if (!Element.ResourceBindings.empty())
			Output << '\n';
		Output << "      ],\n";
		if (!Element.UnboundSourceResources.empty())
		{
			Output << "      \"unboundResources\": [";
			for (size_t iUnbound = 0u;
				iUnbound < Element.UnboundSourceResources.size(); ++iUnbound)
			{
				Output << (0u == iUnbound ? "\n" : ",\n")
					<< "        \"" << CDataJson::Escape(
						Element.UnboundSourceResources[iUnbound]) << "\"";
			}
			Output << "\n      ],\n";
		}
		Output << "      \"material\": { \"templateId\": \""
			<< CDataJson::Escape(Element.Material.strTemplateId)
			<< "\", \"sourceMaterialPath\": \""
			<< CDataJson::Escape(Element.Material.strSourceMaterialPath)
			<< "\", \"renderProfile\": \""
			<< To_Token(Element.Material.eRenderProfile)
			<< "\", \"sourceProfile\": ";
		Write_SourceMaterialProfile(Output, Element.Material.SourceMaterial);
		if (Element.Material.bColorTexturesSRGB)
			Output << ", \"colorTexturesSRGB\": true";
		if (Element.Material.Execution.bEnabled ||
			Element.Material.Execution.bFailClosed)
		{
			Output << ", \"execution\": ";
			Write_MaterialExecution(Output, Element.Material.Execution);
		}
		Output << " },\n"
			<< "      \"actionCueAttachment\": { \"enabled\": "
			<< (Element.ActionCueAttachment.bEnabled ? "true" : "false")
			<< ", \"follow\": "
			<< (Element.ActionCueAttachment.bFollow ? "true" : "false");
		if (Element.ActionCueAttachment.eOrientation ==
			EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
		{
			Output << ", \"orientation\": \"owner_yaw\"";
		}
		Output << ", \"sourceAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strSourceAnchorSlotId)
			<< "\", \"runtimeAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeAnchorSlotId)
			<< "\", \"runtimeBoneName\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeBoneName)
			<< "\", \"snapshotRootSourceBasisYawDegrees\": "
			<< Element.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees
			<< ", \"socketLocalTransform\": { \"position\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vRotationDegrees);
		Output << ", \"scale\": ";
		Write_Float3(Output,
			Element.ActionCueAttachment.SocketLocalTransform.vScale);
		Output << " } },\n";
		Output << "      \"transformInheritance\": { \"enabled\": "
			<< (Element.TransformInheritance.bEnabled ? "true" : "false")
			<< ", \"masterElementId\": \""
			<< CDataJson::Escape(
				Element.TransformInheritance.strMasterElementId)
			<< "\" },\n";
		Write_Detail(Output, Element.Detail);
		Output << ",\n";
		Write_SourceRecipe(Output, Element.SourceRecipe, bSourceContract);
		Output << ",\n";
		Write_SourcePresentation(Output, Element.SourcePresentation);
		if (!Element.AuthoringOverrides.Is_Empty())
		{
			// Absent when empty so untouched documents stay byte-identical.
			Output << ",\n      \"authoringOverrides\": { \"resources\": [";
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.ResourceBindings.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC&
					Binding = Element.AuthoringOverrides.ResourceBindings[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"slotId\": \""
					<< Client::CDataJson::Escape(Binding.strSlotId)
					<< "\", \"assetId\": \""
					<< Client::CDataJson::Escape(Binding.strAssetId)
					<< "\", \"compilerAssetId\": \""
					<< Client::CDataJson::Escape(Binding.strCompilerAssetId)
					<< "\" }";
			}
			Output << (Element.AuthoringOverrides.ResourceBindings.empty() ?
				"], \"scalars\": [" : " ], \"scalars\": [");
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.Scalars.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Scalar =
					Element.AuthoringOverrides.Scalars[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"name\": \""
					<< Client::CDataJson::Escape(Scalar.strName)
					<< "\", \"value\": " << Scalar.fValue
					<< ", \"compilerValue\": " << Scalar.fCompilerValue
					<< " }";
			}
			Output << (Element.AuthoringOverrides.Scalars.empty() ?
				"], \"colors\": [" : " ], \"colors\": [");
			for (size_t i = 0u;
				i < Element.AuthoringOverrides.Colors.size(); ++i)
			{
				const Client::EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Color =
					Element.AuthoringOverrides.Colors[i];
				Output << (0u == i ? " " : ", ")
					<< "{ \"name\": \""
					<< Client::CDataJson::Escape(Color.strName)
					<< "\", \"value\": ";
				Write_Float4(Output, Color.vValue);
				Output << ", \"compilerValue\": ";
				Write_Float4(Output, Color.vCompilerValue);
				Output << " }";
			}
			Output << (Element.AuthoringOverrides.Colors.empty() ?
				"] }" : " ] }");
		}
		Output << "\n    }";
	}
	if (!Document.Elements.empty())
		Output << "\n  ";
	Output << "]\n}\n";
	return Output.str();
}

bool_t Client::CEffectDocumentCodec::Load(
	const std::filesystem::path& Path,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		strOutError = "Effect document could not be opened.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (!Input.eof() && Input.fail())
	{
		strOutError = "Effect document read failed.";
		return false;
	}
	return Parse(Buffer.str(), OutDocument, strOutError);
}

namespace
{
	std::filesystem::path Make_EffectSaveTransactionPath(
		const std::filesystem::path& Destination,
		const std::wstring_view strRole)
	{
		static std::atomic_uint64_t TransactionCounter = 0u;
		const uint64_t iCounter = TransactionCounter.fetch_add(
			1u, std::memory_order_relaxed);
		const auto iClock = std::chrono::steady_clock::now()
			.time_since_epoch().count();
		return Destination.wstring() + L"." + std::wstring(strRole) + L"." +
			std::to_wstring(iClock) + L"." + std::to_wstring(iCounter);
	}

	bool_t Save_EffectDocumentAtomic(
		const std::filesystem::path& Path,
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view* pExpectedCanonicalDocument,
		std::string& strOutError)
	{
		using Client::CEffectDocumentCodec;

		// Authoring save preserves valid partial drafts. The publisher/runtime gate
		// still calls Validate_Drawable before a document can ship or render.
		if (!(Document.bSourceContract ?
			CEffectDocumentCodec::Validate_SourceContract(Document, strOutError) :
			CEffectDocumentCodec::Validate(Document, strOutError)))
			return false;
		std::error_code Error;
		std::filesystem::create_directories(Path.parent_path(), Error);
		if (Error)
		{
			strOutError = "Effect authoring directory creation failed.";
			return false;
		}
		const std::filesystem::path Temporary =
			Make_EffectSaveTransactionPath(Path, L"tmp");
		const std::filesystem::path Backup =
			Make_EffectSaveTransactionPath(Path, L"bak");
		const std::string Json = CEffectDocumentCodec::Serialize(Document);
		{
			std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
			Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
			Output.flush();
			if (!Output)
			{
				strOutError = "Effect temporary write failed.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		Client::EFFECT_DOCUMENT_DESC RoundTrip;
		if (!CEffectDocumentCodec::Load(Temporary, RoundTrip, strOutError))
		{
			std::filesystem::remove(Temporary, Error);
			return false;
		}
		if (CEffectDocumentCodec::Serialize(RoundTrip) != Json)
		{
			strOutError =
				"Effect temporary round-trip changed the authoring document.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}

		if (nullptr != pExpectedCanonicalDocument)
		{
			Error.clear();
			const bool_t bDestinationExists =
				std::filesystem::exists(Path, Error);
			if (Error)
			{
				strOutError =
					"Effect destination state could not be checked before save.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
			if (pExpectedCanonicalDocument->empty())
			{
				if (bDestinationExists)
				{
					strOutError =
						"Effect destination appeared after this authoring session began; reload before saving.";
					std::filesystem::remove(Temporary, Error);
					return false;
				}
			}
			else
			{
				Client::EFFECT_DOCUMENT_DESC Current;
				std::string CurrentError;
				if (!bDestinationExists ||
					!CEffectDocumentCodec::Load(Path, Current, CurrentError) ||
					CEffectDocumentCodec::Serialize(Current) !=
						*pExpectedCanonicalDocument)
				{
					strOutError =
						"Effect document changed on disk after it was loaded; Reload Saved before applying this draft.";
					std::filesystem::remove(Temporary, Error);
					return false;
				}
			}
		}

		Error.clear();
		const bool_t bHadDestination =
			std::filesystem::exists(Path, Error) && !Error;
		if (bHadDestination)
		{
			std::filesystem::rename(Path, Backup, Error);
			if (Error)
			{
				strOutError = "Effect destination backup failed.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		std::filesystem::rename(Temporary, Path, Error);
		if (Error)
		{
			std::error_code RestoreError;
			if (bHadDestination)
				std::filesystem::rename(Backup, Path, RestoreError);
			std::filesystem::remove(Temporary, RestoreError);
			strOutError = RestoreError ?
				"Effect document promote and rollback failed." :
				"Effect document promote failed.";
			return false;
		}
		std::filesystem::remove(Backup, Error);
		strOutError.clear();
		return true;
	}
}

bool_t Client::CEffectDocumentCodec::Save_Atomic(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	return Save_EffectDocumentAtomic(Path, Document, nullptr, strOutError);
}

bool_t Client::CEffectDocumentCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_DOCUMENT_DESC& Document,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError)
{
	return Save_EffectDocumentAtomic(
		Path, Document, &strExpectedCanonicalDocument, strOutError);
}

void Client::CEffectDocumentCodec::Collect_ResourceAssetIds(
	const EFFECT_DOCUMENT_DESC& Document,
	std::vector<std::string>& OutAssetIds)
{
	std::unordered_set<std::string> Unique;
	for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
		Unique.insert(Cue.strModelAssetId);
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding : Element.ResourceBindings)
			Unique.insert(Binding.strAssetId);
		for (const EFFECT_NAMED_TEXTURE_DESC& Texture :
			Element.Material.SourceMaterial.Textures)
		{
			if (!Texture.strAssetId.empty())
				Unique.insert(Texture.strAssetId);
		}
		for (const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane :
			Element.Material.Execution.TextureLanes)
		{
			if (!Lane.strAssetId.empty())
				Unique.insert(Lane.strAssetId);
		}
	}
	OutAssetIds.assign(Unique.begin(), Unique.end());
	std::sort(OutAssetIds.begin(), OutAssetIds.end());
}
