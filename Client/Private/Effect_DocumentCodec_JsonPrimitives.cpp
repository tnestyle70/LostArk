#include "Effect_DocumentCodec_Internal.h"
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


using namespace Client::EffectDocumentCodecDetail;

namespace Client::EffectDocumentCodecDetail
{


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


	bool_t ValidatePortableAuthoredParticleRuntimeCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);

	bool_t ValidatePortableAuthoredParticleEventRoutes(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);

	bool_t Validate_AuthoredRuntimeExtensions(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);


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
		case Client::EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1:
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

}
