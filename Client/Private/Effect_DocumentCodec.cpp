#include "Effect_DocumentCodec.h"

#include "DataJson.h"
#include "Effect_MaterialTemplate.h"
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
	constexpr size_t MAX_SOURCE_MODULES_PER_ELEMENT = 256u;
	constexpr size_t MAX_SOURCE_LITERALS_PER_MODULE = 1024u;
	constexpr size_t MAX_SOURCE_DISTRIBUTIONS_PER_MODULE = 128u;
	constexpr size_t MAX_SOURCE_BURSTS_PER_ELEMENT = 1024u;
	constexpr size_t MAX_SOURCE_PRESENTATION_PARAMETERS = 256u;
	constexpr size_t MAX_SOURCE_COVERAGE_PROPERTIES_PER_MODULE = 2048u;
	constexpr size_t MAX_SOURCE_LOCAL_REFERENCE_BINDINGS_PER_ELEMENT = 2048u;
	constexpr size_t MAX_SOURCE_TYPED_FIELDS_PER_REFERENCE = 256u;
	constexpr const char_t* EFFECT_SOURCE_PRESENTATION_SCHEMA =
		"lostark.effect-source-presentation";

	constexpr const char_t* KIND_TOKENS[] =
	{
		"mesh", "sprite", "particle", "decal", "trail", "light",
		"screenPost"
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
		"meshModel", "base", "noise", "mask", "emissive", "dissolve"
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
	constexpr const char_t* TEXTURE_ADDRESS_MODE_TOKENS[] =
	{
		"wrap", "clamp"
	};
	constexpr const char_t* TEXTURE_COLOR_SPACE_TOKENS[] =
	{
		"linear", "srgb"
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
		if (!Out.bEnabled)
			return true;
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
		if (!Source.bEnabled)
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
			<< SourceMaterialStatusToken(Source.eStatus)
			<< "\", \"textures\": [";
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
			Read_Array(*pLocal, "scale",
				&Out.LocalTransform.vScale.x, 3u, strOutError) &&
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
		if (strAssetId.empty() || strAssetId.size() > MAX_RESOURCE_ID_BYTES ||
			0u != strAssetId.rfind("Character/", 0u) ||
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
			Read_Float(*pTiming, "afterImageSeconds", Out.Timing.fAfterImageSeconds, strOutError) &&
			Read_Float(*pTiming, "dissolveStartNormalized", Out.Timing.fDissolveStartNormalized, strOutError) &&
			Read_Bool(*pMesh, "useModelMaterial", Out.Mesh.bUseModelMaterial, strOutError) &&
			Read_Bool(*pSprite, "billboard", Out.Sprite.bBillboard, strOutError) &&
			Read_OptionalFloat(*pSprite, "billboardRollDegrees",
				Out.Sprite.fBillboardRollDegrees, strOutError) &&
			Read_Array(*pDecal, "size", &Out.Decal.vSize.x, 2u, strOutError) &&
			Read_Float(*pDecal, "depth", Out.Decal.fDepth, strOutError);
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
			Read_UInt(*pParticle, "maxParticles", Out.Particle.iMaxParticles, strOutError) &&
			Read_Float(*pParticle, "spawnRatePerSecond", Out.Particle.fSpawnRatePerSecond, strOutError) &&
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
			Read_UInt(*pTrail, "maxPoints", Out.Trail.iMaxPoints, strOutError) &&
			Read_Float(*pTrail, "pointLifeTimeSeconds", Out.Trail.fPointLifeTimeSeconds, strOutError) &&
			Read_Float(*pTrail, "sampleIntervalSeconds", Out.Trail.fSampleIntervalSeconds, strOutError) &&
			Read_Float(*pTrail, "minimumDistance", Out.Trail.fMinimumDistance, strOutError) &&
			Read_Float(*pTrail, "startWidth", Out.Trail.fStartWidth, strOutError) &&
			Read_Float(*pTrail, "endWidth", Out.Trail.fEndWidth, strOutError) &&
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
		Output << ", \"clip\": " << Detail.Color.fColorClip
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
			<< ", \"lifeTimeSeconds\": " << Detail.Timing.fLifeTimeSeconds
			<< ", \"afterImageSeconds\": " << Detail.Timing.fAfterImageSeconds
			<< ", \"dissolveStartNormalized\": " << Detail.Timing.fDissolveStartNormalized
			<< " },\n        \"mesh\": { \"useModelMaterial\": " << (Detail.Mesh.bUseModelMaterial ? "true" : "false")
			<< " },\n        \"sprite\": { \"billboard\": " << (Detail.Sprite.bBillboard ? "true" : "false")
			<< ", \"billboardRollDegrees\": " << Detail.Sprite.fBillboardRollDegrees
			<< " },\n        \"decal\": { \"size\": ";
		Write_Float2(Output, Detail.Decal.vSize);
		Output << ", \"depth\": " << Detail.Decal.fDepth << " },\n"
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
			<< ", \"endEmissiveIntensity\": " << Detail.LinearLerp.fEndEmissiveIntensity << " },\n"
			<< "        \"particle\": { \"maxParticles\": " << Detail.Particle.iMaxParticles
			<< ", \"spawnRatePerSecond\": " << Detail.Particle.fSpawnRatePerSecond
			<< ", \"burstCount\": " << Detail.Particle.iBurstCount
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
			<< ", \"billboard\": " << (Detail.Particle.bBillboard ? "true" : "false") << " },\n"
			<< "        \"trail\": { \"maxPoints\": " << Detail.Trail.iMaxPoints
			<< ", \"pointLifeTimeSeconds\": " << Detail.Trail.fPointLifeTimeSeconds
			<< ", \"sampleIntervalSeconds\": " << Detail.Trail.fSampleIntervalSeconds
			<< ", \"minimumDistance\": " << Detail.Trail.fMinimumDistance
			<< ", \"startWidth\": " << Detail.Trail.fStartWidth
			<< ", \"endWidth\": " << Detail.Trail.fEndWidth
			<< ", \"faceCamera\": " << (Detail.Trail.bFaceCamera ? "true" : "false") << " },\n"
			<< "        \"afterImage\": { \"sampleIntervalSeconds\": " << Detail.AfterImage.fSampleIntervalSeconds
			<< ", \"maxCopies\": " << Detail.AfterImage.iMaxCopies
			<< ", \"alphaExponent\": " << Detail.AfterImage.fAlphaExponent
			<< " },\n";
		Write_PresentationDetail(Output, Detail);
		Output << "      }";
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
		Document.iLoadedFormatVersion > EFFECT_SOURCE_CONTRACT_FORMAT_VERSION)
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
			Is_Finite(Cue.vAssetPreRotationDegrees);
		if (!Is_StableId(Cue.strCueId) ||
			!ModelCueIds.insert(Cue.strCueId).second ||
			Cue.strClipName.empty() || Cue.strClipName.size() > 128u ||
			!Has_VisibleCharacter(Cue.strClipName) ||
			!Is_SafeModelCueAssetId(Cue.strModelAssetId) ||
			!std::isfinite(Cue.fStartDelaySeconds) ||
			Cue.fStartDelaySeconds < 0.f ||
			!std::isfinite(Cue.fDurationSeconds) ||
			Cue.fDurationSeconds <= 0.f || Cue.fDurationSeconds > 30.f ||
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
		if (!Is_StableId(Element.strElementId) ||
			Element.strDisplayName.size() > 64u ||
			!Has_VisibleCharacter(Element.strDisplayName) ||
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
			Is_Finite(Attachment.SocketLocalTransform.vPosition) &&
			Is_Finite(Attachment.SocketLocalTransform.vRotationDegrees) &&
			Is_Finite(Attachment.SocketLocalTransform.vScale) &&
			Attachment.SocketLocalTransform.vScale.x > 0.f &&
			Attachment.SocketLocalTransform.vScale.y > 0.f &&
			Attachment.SocketLocalTransform.vScale.z > 0.f;
		if (!bAttachmentTransformValid ||
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
		if (Element.Material.strSourceMaterialPath.size() > 512u ||
			(Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID &&
				(Element.Material.strSourceMaterialPath.empty() ||
					!Has_VisibleCharacter(
						Element.Material.strSourceMaterialPath))))
		{
			strOutError = "Effect source Material identity is invalid.";
			return false;
		}
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
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
				!Is_SafeResourceAssetId(Binding.strAssetId, &eActualKind) ||
				eActualKind != eExpectedKind)
			{
				strOutError = "Effect resource slot, path, file, or duplicate is invalid.";
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
		}
		const EFFECT_DETAIL_DESC& D = Element.Detail;
		const int64_t iTileCount = static_cast<int64_t>(D.UV.iTileColumns) * D.UV.iTileRows;
		const bool_t bCommonValid =
			Is_Finite(D.Transform.vPosition) && Is_Finite(D.Transform.vRotationDegrees) &&
			Is_Finite(D.Transform.vRevolutionDegreesPerSecond) && Is_Finite(D.Transform.vScale) &&
			D.Transform.vScale.x > 0.f && D.Transform.vScale.y > 0.f && D.Transform.vScale.z > 0.f &&
			Is_Finite(D.Transform.vVelocityPerSecond) && Is_Finite(D.Color.vColorOffset) &&
			Is_Finite(D.Color.vColorMultiply) && std::isfinite(D.Color.fColorClip) &&
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
			std::isfinite(D.Timing.fAfterImageSeconds) && D.Timing.fAfterImageSeconds >= 0.f &&
			std::isfinite(D.Timing.fDissolveStartNormalized) &&
			D.Timing.fDissolveStartNormalized >= 0.f && D.Timing.fDissolveStartNormalized <= 1.f &&
			std::isfinite(D.Sprite.fBillboardRollDegrees) &&
			std::abs(D.Sprite.fBillboardRollDegrees) <= 3600.f &&
			Is_Finite(D.Decal.vSize) && D.Decal.vSize.x > 0.f && D.Decal.vSize.y > 0.f &&
			std::isfinite(D.Decal.fDepth) && D.Decal.fDepth > 0.f;
		const bool_t bLerpValid =
			Is_Finite(D.LinearLerp.vEndPosition) && Is_Finite(D.LinearLerp.vEndRotationDegrees) &&
			Is_Finite(D.LinearLerp.vEndRevolutionDegreesPerSecond) && Is_Finite(D.LinearLerp.vEndScale) &&
			D.LinearLerp.vEndScale.x > 0.f && D.LinearLerp.vEndScale.y > 0.f && D.LinearLerp.vEndScale.z > 0.f &&
			Is_Finite(D.LinearLerp.vEndVelocityPerSecond) && Is_Finite(D.LinearLerp.vEndColorOffset) &&
			Is_Finite(D.LinearLerp.vEndColorMultiply) &&
			std::isfinite(D.LinearLerp.fEndEmissiveIntensity) && D.LinearLerp.fEndEmissiveIntensity >= 0.f;
		const bool_t bParticleValid =
			D.Particle.iMaxParticles >= 1u && D.Particle.iMaxParticles <= 2048u &&
			D.Particle.iBurstCount <= D.Particle.iMaxParticles && D.Particle.iRandomSeed != 0u &&
			std::isfinite(D.Particle.fSpawnRatePerSecond) && D.Particle.fSpawnRatePerSecond >= 0.f && D.Particle.fSpawnRatePerSecond <= 2048.f &&
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
			Is_Finite(D.Particle.vEndSize) && D.Particle.vEndSize.x >= 0.f && D.Particle.vEndSize.y >= 0.f;
		const bool_t bTrailValid =
			D.Trail.iMaxPoints >= 2u && D.Trail.iMaxPoints <= 256u &&
			std::isfinite(D.Trail.fPointLifeTimeSeconds) && D.Trail.fPointLifeTimeSeconds > 0.f &&
			std::isfinite(D.Trail.fSampleIntervalSeconds) && D.Trail.fSampleIntervalSeconds > 0.f &&
			std::isfinite(D.Trail.fMinimumDistance) && D.Trail.fMinimumDistance >= 0.f &&
			std::isfinite(D.Trail.fStartWidth) && D.Trail.fStartWidth > 0.f &&
			std::isfinite(D.Trail.fEndWidth) && D.Trail.fEndWidth >= 0.f;
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
					D.Light.fFalloffExponent >= 0.f));
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
		if (!bCommonValid || !bLerpValid || !bParticleValid ||
			!bTrailValid || !bAfterImageValid || !bLightValid ||
			!bScreenPostValid)
		{
			strOutError = "Effect Detail contains an invalid number or range.";
			return false;
		}
		if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
			iTotalParticles += D.Particle.iMaxParticles;
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
			Left.strSourceAnchorSlotId == Right.strSourceAnchorSlotId &&
			Left.strRuntimeAnchorSlotId == Right.strRuntimeAnchorSlotId &&
			Left.strRuntimeBoneName == Right.strRuntimeBoneName &&
			SameTransform(Left.SocketLocalTransform,
				Right.SocketLocalTransform);
	};
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
		if (Element.Material.SourceMaterial.bEnabled)
		{
			strOutError =
				"Source-contract candidates cannot enable a runtime Material profile.";
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
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
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
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			Element.Material.SourceMaterial;
		const bool_t bFallbackBlocked = SourceMaterial.bEnabled &&
			SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.fallback-blocked.v1";
		const bool_t bGroupedTranslucent = SourceMaterial.bEnabled &&
			SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.grouped-translucent.v1" &&
			Is_EffectGroupedTranslucentResourceContractSatisfied(
				SourceMaterial,
				nullptr != pBaseBinding &&
					!Is_UnsafeEffectBaseTextureAssetId(pBaseBinding->strAssetId),
				nullptr != FindBinding(EFFECT_MATERIAL_INPUT_SEMANTIC::MASK),
				nullptr != FindBinding(EFFECT_MATERIAL_INPUT_SEMANTIC::EMISSIVE),
				nullptr != FindBinding(EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE));
		const bool_t bFiniteProfile = SourceMaterial.bEnabled &&
			(SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.shine.v1" ||
			 SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.blackline-aura.v1" ||
			 SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.slice.v1" ||
			 SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.missiletrail-01.v1" ||
			 SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.local-crack.v1" ||
			 SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.procedural-center-glow.v1");
		const bool_t bMaterialOwnsDrawableContract =
			Element.Material.strTemplateId == EFFECT_SOURCE_MATERIAL_TEMPLATE_ID ||
			bFallbackBlocked || bGroupedTranslucent || bFiniteProfile;
		const bool_t bParticleMesh =
			EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
			std::any_of(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{
					return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
				});
		const bool_t bLocalCrackProfile = SourceMaterial.bEnabled &&
			SourceMaterial.strRuntimeShaderProfileId ==
				"effect.ue3.local-crack.v1";
		const bool_t bHasDissolve = nullptr != FindBinding(
			EFFECT_MATERIAL_INPUT_SEMANTIC::DISSOLVE);
		if (bLocalCrackProfile)
		{
			const bool_t bLegacyContract =
				Is_EffectLegacyLocalCrackResourceContractSatisfied(
					SourceMaterial, bHasDissolve, bParticleMesh);
			const bool_t bNamedContract =
				!SourceMaterial.Textures.empty() &&
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
		else if (bFiniteProfile &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				SourceMaterial.strRuntimeShaderProfileId,
				nullptr != pBaseBinding &&
					!Is_UnsafeEffectBaseTextureAssetId(
						pBaseBinding->strAssetId),
				nullptr != FindBinding(
					EFFECT_MATERIAL_INPUT_SEMANTIC::MASK),
				bHasDissolve,
				bParticleMesh))
		{
			strOutError =
				"Finite source Material profile resource contract is not satisfied.";
			return false;
		}
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
		Version > EFFECT_SOURCE_CONTRACT_FORMAT_VERSION)
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}
	const uint32_t iSourceVersion = static_cast<uint32_t>(Version);
	const bool_t bSourceContract =
		iSourceVersion == EFFECT_SOURCE_CONTRACT_FORMAT_VERSION;
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
	else if (nullptr != pPurpose)
	{
		strOutError = "Legacy Effect documents cannot declare source-contract purpose.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Staged.iLoadedFormatVersion = iSourceVersion;
	Staged.bSourceContract = bSourceContract;
	Staged.strEffectAssetId = pAssetId->Get_String();
	Staged.strDisplayName = pDisplayName->Get_String();
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
						"startDelaySeconds", "durationSeconds", "visible",
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
				!Read_ModelCueTransform(CueValue, Cue, strOutError))
			{
				if (strOutError.empty())
					strOutError = "Effect Model Cue fields are invalid.";
				return false;
			}
			Cue.strCueId = pCueId->Get_String();
			Cue.strModelAssetId = pModelAssetId->Get_String();
			Cue.strClipName = pClipName->Get_String();
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
					"kind", "renderer", "resources", "material",
					"actionCueAttachment", "transformInheritance", "detail",
					"sourceRecipe", "sourcePresentation" },
				"Effect source-contract Element", strOutError)))
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
		Element.strElementId = pId->Get_String();
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
		const DATA_JSON_VALUE* pTemplateId = pMaterial->Find("templateId");
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
		Staged.Elements.push_back(std::move(Element));
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
			<< ", \"visible\": " << (Cue.bVisible ? "true" : "false")
			<< ",\n      \"localTransform\": { \"position\": ";
		Write_Float3(Output, Cue.LocalTransform.vPosition);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.LocalTransform.vRotationDegrees);
		Output << ", \"scale\": ";
		Write_Float3(Output, Cue.LocalTransform.vScale);
		Output << " },\n      \"assetPreTransform\": { \"scale\": ";
		Write_Float3(Output, Cue.vAssetPreScale);
		Output << ", \"rotationDegrees\": ";
		Write_Float3(Output, Cue.vAssetPreRotationDegrees);
		Output << " } }";
	}
	if (!Document.ModelCues.empty())
		Output << '\n';
	Output << "  ],\n"
		<< "  \"elements\": [";
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
		Output << "      ],\n      \"material\": { \"templateId\": \""
			<< CDataJson::Escape(Element.Material.strTemplateId)
			<< "\", \"sourceMaterialPath\": \""
			<< CDataJson::Escape(Element.Material.strSourceMaterialPath)
			<< "\", \"renderProfile\": \""
			<< To_Token(Element.Material.eRenderProfile)
			<< "\", \"sourceProfile\": ";
		Write_SourceMaterialProfile(Output, Element.Material.SourceMaterial);
		Output << " },\n"
			<< "      \"actionCueAttachment\": { \"enabled\": "
			<< (Element.ActionCueAttachment.bEnabled ? "true" : "false")
			<< ", \"follow\": "
			<< (Element.ActionCueAttachment.bFollow ? "true" : "false")
			<< ", \"sourceAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strSourceAnchorSlotId)
			<< "\", \"runtimeAnchorSlotId\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeAnchorSlotId)
			<< "\", \"runtimeBoneName\": \""
			<< CDataJson::Escape(
				Element.ActionCueAttachment.strRuntimeBoneName)
			<< "\", \"socketLocalTransform\": { \"position\": ";
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
	}
	OutAssetIds.assign(Unique.begin(), Unique.end());
	std::sort(OutAssetIds.begin(), OutAssetIds.end());
}
