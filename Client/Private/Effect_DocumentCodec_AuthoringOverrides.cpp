#include "Effect_DocumentCodec_Internal.h"
#include "Effect_MaterialTemplate.h"

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
			Left.strModelCueId == Right.strModelCueId &&
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
