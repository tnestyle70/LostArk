#include "Effect_MaterialProgramRegistry.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace
{
	using namespace Client;

	constexpr size_t MAX_STABLE_ID_BYTES = 255u;
	constexpr size_t MAX_ASSET_ID_BYTES = 512u;
	constexpr size_t MAX_PROGRAMS = 4096u;
	constexpr size_t MAX_LAYOUTS = 4096u;
	constexpr size_t MAX_DESCRIPTORS = 4096u;
	constexpr size_t MAX_BINDINGS = 65536u;
	constexpr size_t MAX_TEXTURE_LANES = 6u;
	constexpr size_t MAX_SCALAR_ROWS = 52u;
	constexpr size_t MAX_VECTOR_ROWS = 3u;
	constexpr size_t MAX_ARTIST_PARAMETER_ROWS = 64u;
	constexpr size_t MAX_COLOR_ROWS = 64u;
	constexpr uint32_t MATERIAL_SAMPLER_REGISTER_BASE = 5u;
	constexpr uint32_t MAX_D3D11_SAMPLER_REGISTER = 15u;

	struct PROGRAM_RECORD final
	{
		std::string strProgramId;
		EFFECT_MATERIAL_EXECUTION_BACKEND eBackend =
			EFFECT_MATERIAL_EXECUTION_BACKEND::END;
		uint32_t iOpcode = 0u;
	};

	struct LAYOUT_TEXTURE_LANE final
	{
		std::string strLaneId;
		std::string strRole;
		uint32_t iTextureRegister = 0u;
		uint32_t iSamplerRegister = 0u;
		std::string strSourceChannel;
		EFFECT_TEXTURE_COLOR_SPACE eColorSpace =
			EFFECT_TEXTURE_COLOR_SPACE::END;
	};

	struct LAYOUT_PARAMETER_ROW final
	{
		std::string strName;
		uint32_t iPackedIndex = 0u;
	};

	struct LAYOUT_RECORD final
	{
		std::string strLayoutId;
		uint32_t iExecutionVersion = 0u;
		uint32_t iTextureLaneCount = 0u;
		uint32_t iTextureMask = 0u;
		std::vector<LAYOUT_TEXTURE_LANE> TextureLanes;
		uint32_t iDynamicConsumedMask = 0u;
		uint32_t iDynamicSuppressedMask = 0u;
		uint32_t iParticleColorPolicy = 0u;
		uint32_t iParticleColorConsumedMask = 0u;
		uint32_t iParticleColorSuppressedMask = 0u;
		uint32_t iScalarCount = 0u;
		uint32_t iVectorCount = 0u;
		uint32_t iInputCount = 0u;
		std::array<uint32_t, 2u> InputConsumedMask{};
		std::array<uint32_t, 2u> InputSuppressedMask{};
		std::array<uint32_t, 3u> VectorComponentConsumedMask{};
		std::array<uint32_t, 3u> VectorComponentSuppressedMask{};
		uint32_t iStaticInputCount = 0u;
		uint32_t iStaticSelectedMask = 0u;
		uint32_t iStaticConsumedMask = 0u;
		uint32_t iStaticSuppressedMask = 0u;
		uint32_t iRenderInputCount = 0u;
		uint32_t iRenderConsumedMask = 0u;
		uint32_t iRenderSuppressedMask = 0u;
		std::vector<LAYOUT_PARAMETER_ROW> ScalarRows;
		std::vector<LAYOUT_PARAMETER_ROW> VectorRows;
		std::vector<LAYOUT_PARAMETER_ROW> ArtistParameterRows;
		std::vector<LAYOUT_PARAMETER_ROW> ColorRows;
	};

	struct DESCRIPTOR_TEXTURE_LANE final
	{
		std::string strLaneId;
		std::string strAssetId;
		EFFECT_MATERIAL_SAMPLER_DESC Sampler;
	};

	struct DESCRIPTOR_SCALAR_ROW final
	{
		std::string strName;
		f32_t fValue = 0.f;
	};

	struct DESCRIPTOR_VECTOR_ROW final
	{
		std::string strName;
		float4_t vValue{};
	};

	struct DESCRIPTOR_RECORD final
	{
		std::string strDescriptorId;
		std::string strLayoutId;
		std::vector<DESCRIPTOR_TEXTURE_LANE> TextureLanes;
		std::vector<DESCRIPTOR_SCALAR_ROW> Scalars;
		std::vector<DESCRIPTOR_VECTOR_ROW> Vectors;
		std::vector<DESCRIPTOR_VECTOR_ROW> ArtistParameters;
		std::vector<DESCRIPTOR_VECTOR_ROW> Colors;
	};

	struct BINDING_RECORD final
	{
		std::string strEffectAssetId;
		std::string strElementId;
		std::string strProgramId;
		std::string strLayoutId;
		std::string strDescriptorId;
		std::string strAdapterId;
		EFFECT_MATERIAL_INLINE_MIRROR_POLICY eInlineMirrorPolicy =
			EFFECT_MATERIAL_INLINE_MIRROR_POLICY::END;
	};

	bool_t Validate_ExactOrderedObject(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> ExpectedKeys,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (!Value.Is_Object())
		{
			strOutError = std::string(strContext) + " must be an object.";
			return false;
		}
		if (Value.Get_Object().size() != ExpectedKeys.size() ||
			Value.Get_ObjectInsertionOrder().size() != ExpectedKeys.size())
		{
			strOutError = std::string(strContext) + " must contain exactly " +
				std::to_string(ExpectedKeys.size()) + " ordered fields; found " +
				std::to_string(Value.Get_Object().size()) + ".";
			return false;
		}
		size_t iKey = 0u;
		for (const std::string_view strExpected : ExpectedKeys)
		{
			const std::string& strActual =
				Value.Get_ObjectInsertionOrder()[iKey];
			if (strActual != strExpected)
			{
				strOutError = std::string(strContext) +
					" field order mismatch at index " + std::to_string(iKey) +
					": expected '" + std::string(strExpected) + "', found '" +
					strActual + "'.";
				return false;
			}
			++iKey;
		}
		return true;
	}

	const DATA_JSON_VALUE* Require_Field(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		const DATA_JSON_TYPE eType,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(strName);
		if (nullptr == pValue || pValue->Get_Type() != eType)
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strName) + "' is missing or has the wrong type.";
			return nullptr;
		}
		return pValue;
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		std::string& strOutValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Require_Field(Object, strName,
			DATA_JSON_TYPE::STRING, strContext, strOutError);
		if (nullptr == pValue)
			return false;
		strOutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_UInt32(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		uint32_t& iOutValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Require_Field(Object, strName,
			DATA_JSON_TYPE::NUMBER, strContext, strOutError);
		if (nullptr == pValue)
			return false;
		const double fValue = pValue->Get_Number();
		if (pValue->Was_FloatingPointToken() || !std::isfinite(fValue) ||
			fValue != std::floor(fValue) || fValue < 0.0 ||
			fValue > static_cast<double>((std::numeric_limits<uint32_t>::max)()))
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strName) + "' must be a canonical uint32 integer.";
			return false;
		}
		iOutValue = static_cast<uint32_t>(fValue);
		return true;
	}

	bool_t Read_FloatValue(
		const DATA_JSON_VALUE& Value,
		f32_t& fOutValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (!Value.Is_Number() || !std::isfinite(Value.Get_Number()))
		{
			strOutError = std::string(strContext) +
				" must be a finite float32 number.";
			return false;
		}
		fOutValue = static_cast<f32_t>(Value.Get_Number());
		if (!std::isfinite(fOutValue))
		{
			strOutError = std::string(strContext) +
				" cannot be represented as float32.";
			return false;
		}
		return true;
	}

	bool_t Read_Float(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		f32_t& fOutValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Require_Field(Object, strName,
			DATA_JSON_TYPE::NUMBER, strContext, strOutError);
		return nullptr != pValue && Read_FloatValue(*pValue, fOutValue,
			std::string(strContext) + " field '" + std::string(strName) + "'",
			strOutError);
	}

	template<size_t COUNT>
	bool_t Read_UInt32Array(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		std::array<uint32_t, COUNT>& OutValues,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Require_Field(Object, strName,
			DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pValue)
			return false;
		if (pValue->Get_Array().size() != COUNT)
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strName) + "' must contain exactly " +
				std::to_string(COUNT) + " integers.";
			return false;
		}
		for (size_t iValue = 0u; iValue < COUNT; ++iValue)
		{
			const DATA_JSON_VALUE& Value = pValue->Get_Array()[iValue];
			const double fValue = Value.Is_Number() ? Value.Get_Number() : -1.0;
			if (!Value.Is_Number() || Value.Was_FloatingPointToken() ||
				!std::isfinite(fValue) || fValue != std::floor(fValue) ||
				fValue < 0.0 || fValue > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutError = std::string(strContext) + " field '" +
					std::string(strName) + "' contains an invalid uint32 at index " +
					std::to_string(iValue) + ".";
				return false;
			}
			OutValues[iValue] = static_cast<uint32_t>(fValue);
		}
		return true;
	}

	bool_t Read_Float4(
		const DATA_JSON_VALUE& Object,
		const std::string_view strName,
		float4_t& vOutValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Require_Field(Object, strName,
			DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pValue)
			return false;
		if (pValue->Get_Array().size() != 4u)
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strName) + "' must contain exactly four floats.";
			return false;
		}
		f32_t* pComponents = &vOutValue.x;
		for (size_t iComponent = 0u; iComponent < 4u; ++iComponent)
		{
			if (!Read_FloatValue(pValue->Get_Array()[iComponent],
				pComponents[iComponent], std::string(strContext) + " field '" +
					std::string(strName) + "' component " +
					std::to_string(iComponent), strOutError))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_LowerAlphaNumeric(const char_t Character)
	{
		return (Character >= 'a' && Character <= 'z') ||
			(Character >= '0' && Character <= '9');
	}

	bool_t Validate_StableId(
		const std::string_view strValue,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (strValue.empty() || strValue.size() > MAX_STABLE_ID_BYTES)
		{
			strOutError = std::string(strContext) +
				" must contain 1..255 bytes.";
			return false;
		}
		bool_t bNeedsAlphaNumeric = true;
		for (const char_t Character : strValue)
		{
			if (Is_LowerAlphaNumeric(Character))
			{
				bNeedsAlphaNumeric = false;
				continue;
			}
			if ((Character == '.' || Character == '_' || Character == '-') &&
				!bNeedsAlphaNumeric)
			{
				bNeedsAlphaNumeric = true;
				continue;
			}
			strOutError = std::string(strContext) +
				" must match ^[a-z0-9]+(?:[._-][a-z0-9]+)*$: " +
				std::string(strValue);
			return false;
		}
		if (bNeedsAlphaNumeric)
		{
			strOutError = std::string(strContext) +
				" must end with a lowercase alphanumeric character: " +
				std::string(strValue);
			return false;
		}
		return true;
	}

	bool_t Validate_PlainText(
		const std::string_view strValue,
		const size_t iMaximumBytes,
		const bool_t bAllowEmpty,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if ((!bAllowEmpty && strValue.empty()) ||
			strValue.size() > iMaximumBytes ||
			std::any_of(strValue.begin(), strValue.end(), [](const char_t Character)
			{
				const unsigned char Value = static_cast<unsigned char>(Character);
				return Value < 0x20u || Value == 0x7fu;
			}))
		{
			strOutError = std::string(strContext) +
				" contains an empty, oversized, or control-character value.";
			return false;
		}
		return true;
	}

	bool_t Validate_TextureAssetId(
		const std::string_view strAssetId,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (!Validate_PlainText(strAssetId, MAX_ASSET_ID_BYTES, false,
			strContext, strOutError) ||
			!strAssetId.starts_with("Effect/") ||
			!strAssetId.ends_with(".dds") || strAssetId.front() == '/' ||
			strAssetId.find('\\') != std::string_view::npos ||
			strAssetId.find(':') != std::string_view::npos)
		{
			if (strOutError.empty())
				strOutError = std::string(strContext) +
					" must be a Resources-relative Effect/.../*.dds asset ID.";
			return false;
		}
		size_t iStart = 0u;
		while (iStart <= strAssetId.size())
		{
			const size_t iSlash = strAssetId.find('/', iStart);
			const size_t iEnd = iSlash == std::string_view::npos ?
				strAssetId.size() : iSlash;
			const std::string_view Segment =
				strAssetId.substr(iStart, iEnd - iStart);
			if (Segment.empty() || Segment == "." || Segment == "..")
			{
				strOutError = std::string(strContext) +
					" contains an empty or traversal path segment: " +
					std::string(strAssetId);
				return false;
			}
			if (iSlash == std::string_view::npos)
				break;
			iStart = iSlash + 1u;
		}
		return true;
	}

	bool_t Parse_ColorSpace(
		const std::string_view strValue,
		EFFECT_TEXTURE_COLOR_SPACE& eOutValue)
	{
		if (strValue == "linear")
			eOutValue = EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
		else if (strValue == "srgb")
			eOutValue = EFFECT_TEXTURE_COLOR_SPACE::SRGB;
		else
			return false;
		return true;
	}

	bool_t Parse_Filter(
		const std::string_view strValue,
		EFFECT_MATERIAL_TEXTURE_FILTER& eOutValue)
	{
		if (strValue == "point")
			eOutValue = EFFECT_MATERIAL_TEXTURE_FILTER::POINT;
		else if (strValue == "linear")
			eOutValue = EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR;
		else if (strValue == "anisotropic")
			eOutValue = EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC;
		else
			return false;
		return true;
	}

	bool_t Parse_AddressMode(
		const std::string_view strValue,
		EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE& eOutValue)
	{
		if (strValue == "wrap")
			eOutValue = EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::WRAP;
		else if (strValue == "mirror")
			eOutValue = EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::MIRROR;
		else if (strValue == "clamp")
			eOutValue = EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::CLAMP;
		else if (strValue == "border")
			eOutValue = EFFECT_MATERIAL_TEXTURE_ADDRESS_MODE::BORDER;
		else
			return false;
		return true;
	}

	bool_t Parse_Comparison(
		const std::string_view strValue,
		EFFECT_MATERIAL_COMPARISON_FUNCTION& eOutValue)
	{
		if (strValue == "never")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::NEVER;
		else if (strValue == "less")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::LESS;
		else if (strValue == "equal")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::EQUAL;
		else if (strValue == "lessEqual")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::LESS_EQUAL;
		else if (strValue == "greater")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::GREATER;
		else if (strValue == "notEqual")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::NOT_EQUAL;
		else if (strValue == "greaterEqual")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::GREATER_EQUAL;
		else if (strValue == "always")
			eOutValue = EFFECT_MATERIAL_COMPARISON_FUNCTION::ALWAYS;
		else
			return false;
		return true;
	}

	bool_t Parse_Sampler(
		const DATA_JSON_VALUE& Value,
		EFFECT_MATERIAL_SAMPLER_DESC& OutSampler,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (!Validate_ExactOrderedObject(Value,
			{ "filter", "addressU", "addressV", "addressW", "mipLodBias",
				"maxAnisotropy", "comparison", "borderColor", "minLod",
				"maxLod" }, strContext, strOutError))
		{
			return false;
		}
		std::string strFilter;
		std::string strAddressU;
		std::string strAddressV;
		std::string strAddressW;
		std::string strComparison;
		if (!Read_String(Value, "filter", strFilter, strContext, strOutError) ||
			!Read_String(Value, "addressU", strAddressU, strContext,
				strOutError) ||
			!Read_String(Value, "addressV", strAddressV, strContext,
				strOutError) ||
			!Read_String(Value, "addressW", strAddressW, strContext,
				strOutError) ||
			!Read_String(Value, "comparison", strComparison, strContext,
				strOutError) ||
			!Parse_Filter(strFilter, OutSampler.eFilter) ||
			!Parse_AddressMode(strAddressU, OutSampler.eAddressU) ||
			!Parse_AddressMode(strAddressV, OutSampler.eAddressV) ||
			!Parse_AddressMode(strAddressW, OutSampler.eAddressW) ||
			!Parse_Comparison(strComparison, OutSampler.eComparison) ||
			!Read_Float(Value, "mipLodBias", OutSampler.fMipLodBias,
				strContext, strOutError) ||
			!Read_UInt32(Value, "maxAnisotropy", OutSampler.iMaxAnisotropy,
				strContext, strOutError) ||
			!Read_Float4(Value, "borderColor", OutSampler.vBorderColor,
				strContext, strOutError) ||
			!Read_Float(Value, "minLod", OutSampler.fMinLod, strContext,
				strOutError) ||
			!Read_Float(Value, "maxLod", OutSampler.fMaxLod, strContext,
				strOutError))
		{
			if (strOutError.empty())
				strOutError = std::string(strContext) +
					" contains an unsupported sampler token.";
			return false;
		}
		if (OutSampler.iMaxAnisotropy < 1u ||
			OutSampler.iMaxAnisotropy > 16u ||
			OutSampler.fMinLod > OutSampler.fMaxLod)
		{
			strOutError = std::string(strContext) +
				" anisotropy or LOD range is invalid.";
			return false;
		}
		return true;
	}

	uint32_t Low_BitMask(const uint32_t iCount)
	{
		return iCount >= 32u ? (std::numeric_limits<uint32_t>::max)() :
			(iCount == 0u ? 0u : ((uint32_t{ 1u } << iCount) - 1u));
	}

	bool_t Validate_MaskPair(
		const uint32_t iConsumed,
		const uint32_t iSuppressed,
		const uint32_t iAllowed,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (0u != ((iConsumed | iSuppressed) & ~iAllowed) ||
			0u != (iConsumed & iSuppressed))
		{
			strOutError = std::string(strContext) +
				" masks overlap or address fields outside their count.";
			return false;
		}
		return true;
	}

	bool_t Parse_Program(
		const DATA_JSON_VALUE& Value,
		const size_t iIndex,
		PROGRAM_RECORD& OutProgram,
		std::string& strOutError)
	{
		const std::string strContext = "Material program row " +
			std::to_string(iIndex);
		if (!Validate_ExactOrderedObject(Value,
			{ "programId", "backend", "opcode" }, strContext, strOutError))
		{
			return false;
		}
		std::string strBackend;
		if (!Read_String(Value, "programId", OutProgram.strProgramId,
				strContext, strOutError) ||
			!Validate_StableId(OutProgram.strProgramId,
				strContext + " programId", strOutError) ||
			!Read_String(Value, "backend", strBackend, strContext, strOutError) ||
			!Read_UInt32(Value, "opcode", OutProgram.iOpcode, strContext,
				strOutError))
		{
			return false;
		}
		if (strBackend == "runtimeMaterialV2")
		{
			const bool_t bCompiledOpcode = OutProgram.iOpcode == 3u ||
				OutProgram.iOpcode == 6u;
			if (!bCompiledOpcode)
			{
				strOutError = strContext +
					" names an opcode without a compiled ABI receipt.";
				return false;
			}
			OutProgram.eBackend =
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2;
		}
		else if (strBackend == "localDecal" && OutProgram.iOpcode == 14u)
		{
			OutProgram.eBackend =
				EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL;
		}
		else if (strBackend == "standardColorV1" && OutProgram.iOpcode == 1u)
		{
			OutProgram.eBackend =
				EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1;
		}
		else
		{
			strOutError = strContext +
				" has an unsupported backend/opcode dispatch pair.";
			return false;
		}
		std::string_view strExpectedProgramId;
		if (strBackend == "runtimeMaterialV2" && OutProgram.iOpcode == 6u)
			strExpectedProgramId =
				"effect.program.runtime-material-v2.opcode-6.v1";
		else if (strBackend == "runtimeMaterialV2" && OutProgram.iOpcode == 3u)
			strExpectedProgramId =
				"effect.program.runtime-material-v2.opcode-3.v1";
		else if (strBackend == "localDecal" && OutProgram.iOpcode == 14u)
			strExpectedProgramId = "effect.program.local-decal.opcode-14.v1";
		else
			strExpectedProgramId =
				"effect.program.standard-color-v1.opcode-1.v1";
		if (OutProgram.strProgramId != strExpectedProgramId)
		{
			strOutError = strContext +
				" does not use its canonical compiled Program ID.";
			return false;
		}
		return true;
	}

	bool_t Parse_LayoutParameterRows(
		const DATA_JSON_VALUE& Layout,
		const std::string_view strFieldName,
		const size_t iMaximumCount,
		std::vector<LAYOUT_PARAMETER_ROW>& OutRows,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pRows = Require_Field(Layout, strFieldName,
			DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pRows)
			return false;
		if (pRows->Get_Array().size() > iMaximumCount)
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strFieldName) + "' exceeds " +
				std::to_string(iMaximumCount) + " rows.";
			return false;
		}
		std::set<std::string, std::less<>> Names;
		OutRows.reserve(pRows->Get_Array().size());
		for (size_t iRow = 0u; iRow < pRows->Get_Array().size(); ++iRow)
		{
			const DATA_JSON_VALUE& RowValue = pRows->Get_Array()[iRow];
			const std::string strRowContext = std::string(strContext) + " " +
				std::string(strFieldName) + " row " + std::to_string(iRow);
			if (!Validate_ExactOrderedObject(RowValue,
				{ "name", "packedIndex" }, strRowContext, strOutError))
			{
				return false;
			}
			LAYOUT_PARAMETER_ROW Row;
			if (!Read_String(RowValue, "name", Row.strName, strRowContext,
					strOutError) ||
				!Validate_StableId(Row.strName, strRowContext + " name",
					strOutError) ||
				!Read_UInt32(RowValue, "packedIndex", Row.iPackedIndex,
					strRowContext, strOutError))
			{
				return false;
			}
			if (Row.iPackedIndex != iRow || !Names.emplace(Row.strName).second)
			{
				strOutError = strRowContext +
					" must have a unique name and contiguous packedIndex.";
				return false;
			}
			OutRows.push_back(std::move(Row));
		}
		return true;
	}

	bool_t Validate_LayoutMasks(
		const LAYOUT_RECORD& Layout,
		const std::string_view strContext,
		std::string& strOutError)
	{
		if (Layout.iExecutionVersion != 1u ||
			Layout.iTextureLaneCount != Layout.TextureLanes.size() ||
			Layout.iTextureLaneCount > MAX_TEXTURE_LANES ||
			Layout.iTextureMask != Low_BitMask(Layout.iTextureLaneCount) ||
			Layout.iScalarCount != Layout.ScalarRows.size() ||
			Layout.iScalarCount > MAX_SCALAR_ROWS ||
			Layout.iVectorCount != Layout.VectorRows.size() ||
			Layout.iVectorCount > MAX_VECTOR_ROWS ||
			Layout.iInputCount > 64u || Layout.iStaticInputCount > 32u ||
			Layout.iRenderInputCount > 32u ||
			Layout.iParticleColorPolicy > 3u)
		{
			strOutError = std::string(strContext) +
				" count/version/texture-mask contract is invalid.";
			return false;
		}
		if (!Validate_MaskPair(Layout.iDynamicConsumedMask,
				Layout.iDynamicSuppressedMask, 0x0fu,
				std::string(strContext) + " dynamic", strOutError) ||
			!Validate_MaskPair(Layout.iParticleColorConsumedMask,
				Layout.iParticleColorSuppressedMask, 0x0fu,
				std::string(strContext) + " particle color", strOutError))
		{
			return false;
		}
		for (size_t iWord = 0u; iWord < Layout.InputConsumedMask.size(); ++iWord)
		{
			const uint32_t iWordStart = static_cast<uint32_t>(iWord * 32u);
			const uint32_t iAvailable = Layout.iInputCount <= iWordStart ? 0u :
				(std::min)(32u, Layout.iInputCount - iWordStart);
			if (!Validate_MaskPair(Layout.InputConsumedMask[iWord],
				Layout.InputSuppressedMask[iWord], Low_BitMask(iAvailable),
				std::string(strContext) + " input word " +
					std::to_string(iWord), strOutError))
			{
				return false;
			}
		}
		for (size_t iVector = 0u;
			iVector < Layout.VectorComponentConsumedMask.size(); ++iVector)
		{
			const uint32_t iAllowed = iVector < Layout.iVectorCount ? 0x0fu : 0u;
			if (!Validate_MaskPair(
				Layout.VectorComponentConsumedMask[iVector],
				Layout.VectorComponentSuppressedMask[iVector], iAllowed,
				std::string(strContext) + " vector component row " +
					std::to_string(iVector), strOutError))
			{
				return false;
			}
		}
		const uint32_t iStaticAllowed = Low_BitMask(Layout.iStaticInputCount);
		if (0u != (Layout.iStaticSelectedMask & ~iStaticAllowed) ||
			!Validate_MaskPair(Layout.iStaticConsumedMask,
				Layout.iStaticSuppressedMask, iStaticAllowed,
				std::string(strContext) + " static", strOutError) ||
			!Validate_MaskPair(Layout.iRenderConsumedMask,
				Layout.iRenderSuppressedMask,
				Low_BitMask(Layout.iRenderInputCount),
				std::string(strContext) + " render", strOutError))
		{
			if (strOutError.empty())
				strOutError = std::string(strContext) +
					" static selection mask exceeds its input count.";
			return false;
		}
		return true;
	}

	bool_t Parse_Layout(
		const DATA_JSON_VALUE& Value,
		const size_t iIndex,
		LAYOUT_RECORD& OutLayout,
		std::string& strOutError)
	{
		const std::string strContext = "Material layout row " +
			std::to_string(iIndex);
		if (!Validate_ExactOrderedObject(Value,
			{ "layoutId", "executionVersion", "textureLaneCount", "textureMask",
				"textureLanes", "dynamicConsumedMask", "dynamicSuppressedMask",
				"particleColorPolicy", "particleColorConsumedMask",
				"particleColorSuppressedMask", "scalarCount", "vectorCount",
				"inputCount", "inputConsumedMask", "inputSuppressedMask",
				"vectorComponentConsumedMask", "vectorComponentSuppressedMask",
				"staticInputCount", "staticSelectedMask", "staticConsumedMask",
				"staticSuppressedMask", "renderInputCount", "renderConsumedMask",
				"renderSuppressedMask", "scalarRows", "vectorRows",
				"artistParameterRows", "colorRows" }, strContext, strOutError))
		{
			return false;
		}
		if (!Read_String(Value, "layoutId", OutLayout.strLayoutId,
				strContext, strOutError) ||
			!Validate_StableId(OutLayout.strLayoutId,
				strContext + " layoutId", strOutError) ||
			!Read_UInt32(Value, "executionVersion",
				OutLayout.iExecutionVersion, strContext, strOutError) ||
			!Read_UInt32(Value, "textureLaneCount",
				OutLayout.iTextureLaneCount, strContext, strOutError) ||
			!Read_UInt32(Value, "textureMask", OutLayout.iTextureMask,
				strContext, strOutError) ||
			!Read_UInt32(Value, "dynamicConsumedMask",
				OutLayout.iDynamicConsumedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "dynamicSuppressedMask",
				OutLayout.iDynamicSuppressedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "particleColorPolicy",
				OutLayout.iParticleColorPolicy, strContext, strOutError) ||
			!Read_UInt32(Value, "particleColorConsumedMask",
				OutLayout.iParticleColorConsumedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "particleColorSuppressedMask",
				OutLayout.iParticleColorSuppressedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "scalarCount", OutLayout.iScalarCount,
				strContext, strOutError) ||
			!Read_UInt32(Value, "vectorCount", OutLayout.iVectorCount,
				strContext, strOutError) ||
			!Read_UInt32(Value, "inputCount", OutLayout.iInputCount,
				strContext, strOutError) ||
			!Read_UInt32Array(Value, "inputConsumedMask",
				OutLayout.InputConsumedMask, strContext, strOutError) ||
			!Read_UInt32Array(Value, "inputSuppressedMask",
				OutLayout.InputSuppressedMask, strContext, strOutError) ||
			!Read_UInt32Array(Value, "vectorComponentConsumedMask",
				OutLayout.VectorComponentConsumedMask, strContext, strOutError) ||
			!Read_UInt32Array(Value, "vectorComponentSuppressedMask",
				OutLayout.VectorComponentSuppressedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "staticInputCount", OutLayout.iStaticInputCount,
				strContext, strOutError) ||
			!Read_UInt32(Value, "staticSelectedMask",
				OutLayout.iStaticSelectedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "staticConsumedMask",
				OutLayout.iStaticConsumedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "staticSuppressedMask",
				OutLayout.iStaticSuppressedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "renderInputCount", OutLayout.iRenderInputCount,
				strContext, strOutError) ||
			!Read_UInt32(Value, "renderConsumedMask",
				OutLayout.iRenderConsumedMask, strContext, strOutError) ||
			!Read_UInt32(Value, "renderSuppressedMask",
				OutLayout.iRenderSuppressedMask, strContext, strOutError))
		{
			return false;
		}

		const DATA_JSON_VALUE* pTextureLanes = Require_Field(Value,
			"textureLanes", DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pTextureLanes ||
			pTextureLanes->Get_Array().size() > MAX_TEXTURE_LANES)
		{
			if (nullptr != pTextureLanes)
				strOutError = strContext + " exceeds six texture lanes.";
			return false;
		}
		std::set<std::string, std::less<>> LaneIds;
		std::set<uint32_t> SamplerRegisters;
		OutLayout.TextureLanes.reserve(pTextureLanes->Get_Array().size());
		for (size_t iLane = 0u; iLane < pTextureLanes->Get_Array().size();
			++iLane)
		{
			const DATA_JSON_VALUE& LaneValue = pTextureLanes->Get_Array()[iLane];
			const std::string strLaneContext = strContext + " texture lane " +
				std::to_string(iLane);
			if (!Validate_ExactOrderedObject(LaneValue,
				{ "laneId", "role", "textureRegister", "samplerRegister",
					"sourceChannel", "colorSpace" }, strLaneContext,
				strOutError))
			{
				return false;
			}
			LAYOUT_TEXTURE_LANE Lane;
			std::string strColorSpace;
			if (!Read_String(LaneValue, "laneId", Lane.strLaneId,
					strLaneContext, strOutError) ||
				!Validate_StableId(Lane.strLaneId,
					strLaneContext + " laneId", strOutError) ||
				!Read_String(LaneValue, "role", Lane.strRole, strLaneContext,
					strOutError) ||
				!Validate_StableId(Lane.strRole,
					strLaneContext + " role", strOutError) ||
				!Read_UInt32(LaneValue, "textureRegister",
					Lane.iTextureRegister, strLaneContext, strOutError) ||
				!Read_UInt32(LaneValue, "samplerRegister",
					Lane.iSamplerRegister, strLaneContext, strOutError) ||
				!Read_String(LaneValue, "sourceChannel", Lane.strSourceChannel,
					strLaneContext, strOutError) ||
				!Validate_PlainText(Lane.strSourceChannel, 32u, true,
					strLaneContext + " sourceChannel", strOutError) ||
				!Read_String(LaneValue, "colorSpace", strColorSpace,
					strLaneContext, strOutError) ||
				!Parse_ColorSpace(strColorSpace, Lane.eColorSpace))
			{
				if (strOutError.empty())
					strOutError = strLaneContext +
						" has an unsupported colorSpace.";
				return false;
			}
			if (Lane.iTextureRegister != iLane ||
				Lane.iSamplerRegister != MATERIAL_SAMPLER_REGISTER_BASE + iLane ||
				Lane.iSamplerRegister > MAX_D3D11_SAMPLER_REGISTER ||
				!LaneIds.emplace(Lane.strLaneId).second ||
				!SamplerRegisters.emplace(Lane.iSamplerRegister).second)
			{
				strOutError = strLaneContext +
					" must have unique IDs/registers and contiguous tN/s(5+N).";
				return false;
			}
			OutLayout.TextureLanes.push_back(std::move(Lane));
		}

		return Parse_LayoutParameterRows(Value, "scalarRows",
				MAX_SCALAR_ROWS, OutLayout.ScalarRows, strContext, strOutError) &&
			Parse_LayoutParameterRows(Value, "vectorRows", MAX_VECTOR_ROWS,
				OutLayout.VectorRows, strContext, strOutError) &&
			Parse_LayoutParameterRows(Value, "artistParameterRows",
				MAX_ARTIST_PARAMETER_ROWS, OutLayout.ArtistParameterRows,
				strContext, strOutError) &&
			Parse_LayoutParameterRows(Value, "colorRows", MAX_COLOR_ROWS,
				OutLayout.ColorRows, strContext, strOutError) &&
			Validate_LayoutMasks(OutLayout, strContext, strOutError);
	}

	bool_t Parse_DescriptorVectorRows(
		const DATA_JSON_VALUE& Descriptor,
		const std::string_view strFieldName,
		const size_t iMaximumCount,
		std::vector<DESCRIPTOR_VECTOR_ROW>& OutRows,
		const std::string_view strContext,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pRows = Require_Field(Descriptor, strFieldName,
			DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pRows)
			return false;
		if (pRows->Get_Array().size() > iMaximumCount)
		{
			strOutError = std::string(strContext) + " field '" +
				std::string(strFieldName) + "' exceeds " +
				std::to_string(iMaximumCount) + " rows.";
			return false;
		}
		std::set<std::string, std::less<>> Names;
		OutRows.reserve(pRows->Get_Array().size());
		for (size_t iRow = 0u; iRow < pRows->Get_Array().size(); ++iRow)
		{
			const DATA_JSON_VALUE& RowValue = pRows->Get_Array()[iRow];
			const std::string strRowContext = std::string(strContext) + " " +
				std::string(strFieldName) + " row " + std::to_string(iRow);
			if (!Validate_ExactOrderedObject(RowValue, { "name", "value" },
				strRowContext, strOutError))
			{
				return false;
			}
			DESCRIPTOR_VECTOR_ROW Row;
			if (!Read_String(RowValue, "name", Row.strName, strRowContext,
					strOutError) ||
				!Validate_StableId(Row.strName, strRowContext + " name",
					strOutError) ||
				!Read_Float4(RowValue, "value", Row.vValue, strRowContext,
					strOutError))
			{
				return false;
			}
			if (!Names.emplace(Row.strName).second)
			{
				strOutError = strRowContext + " repeats parameter name '" +
					Row.strName + "'.";
				return false;
			}
			OutRows.push_back(std::move(Row));
		}
		return true;
	}

	bool_t Parse_Descriptor(
		const DATA_JSON_VALUE& Value,
		const size_t iIndex,
		DESCRIPTOR_RECORD& OutDescriptor,
		std::string& strOutError)
	{
		const std::string strContext = "Material descriptor row " +
			std::to_string(iIndex);
		if (!Validate_ExactOrderedObject(Value,
			{ "descriptorId", "layoutId", "textureLanes", "scalars", "vectors",
				"artistParameters", "colors" }, strContext, strOutError) ||
			!Read_String(Value, "descriptorId", OutDescriptor.strDescriptorId,
				strContext, strOutError) ||
			!Validate_StableId(OutDescriptor.strDescriptorId,
				strContext + " descriptorId", strOutError) ||
			!Read_String(Value, "layoutId", OutDescriptor.strLayoutId,
				strContext, strOutError) ||
			!Validate_StableId(OutDescriptor.strLayoutId,
				strContext + " layoutId", strOutError))
		{
			return false;
		}

		const DATA_JSON_VALUE* pTextureLanes = Require_Field(Value,
			"textureLanes", DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pTextureLanes ||
			pTextureLanes->Get_Array().size() > MAX_TEXTURE_LANES)
		{
			if (nullptr != pTextureLanes)
				strOutError = strContext + " exceeds six texture lanes.";
			return false;
		}
		std::set<std::string, std::less<>> LaneIds;
		OutDescriptor.TextureLanes.reserve(pTextureLanes->Get_Array().size());
		for (size_t iLane = 0u; iLane < pTextureLanes->Get_Array().size();
			++iLane)
		{
			const DATA_JSON_VALUE& LaneValue = pTextureLanes->Get_Array()[iLane];
			const std::string strLaneContext = strContext + " texture lane " +
				std::to_string(iLane);
			if (!Validate_ExactOrderedObject(LaneValue,
				{ "laneId", "assetId", "sampler" }, strLaneContext,
				strOutError))
			{
				return false;
			}
			DESCRIPTOR_TEXTURE_LANE Lane;
			const DATA_JSON_VALUE* pSampler = Require_Field(LaneValue, "sampler",
				DATA_JSON_TYPE::OBJECT, strLaneContext, strOutError);
			if (!Read_String(LaneValue, "laneId", Lane.strLaneId,
					strLaneContext, strOutError) ||
				!Validate_StableId(Lane.strLaneId,
					strLaneContext + " laneId", strOutError) ||
				!Read_String(LaneValue, "assetId", Lane.strAssetId,
					strLaneContext, strOutError) ||
				!Validate_TextureAssetId(Lane.strAssetId,
					strLaneContext + " assetId", strOutError) ||
				nullptr == pSampler ||
				!Parse_Sampler(*pSampler, Lane.Sampler,
					strLaneContext + " sampler", strOutError))
			{
				return false;
			}
			if (!LaneIds.emplace(Lane.strLaneId).second)
			{
				strOutError = strLaneContext + " repeats laneId '" +
					Lane.strLaneId + "'.";
				return false;
			}
			OutDescriptor.TextureLanes.push_back(std::move(Lane));
		}

		const DATA_JSON_VALUE* pScalars = Require_Field(Value, "scalars",
			DATA_JSON_TYPE::ARRAY, strContext, strOutError);
		if (nullptr == pScalars || pScalars->Get_Array().size() > MAX_SCALAR_ROWS)
		{
			if (nullptr != pScalars)
				strOutError = strContext + " exceeds 52 scalar rows.";
			return false;
		}
		std::set<std::string, std::less<>> ScalarNames;
		OutDescriptor.Scalars.reserve(pScalars->Get_Array().size());
		for (size_t iScalar = 0u; iScalar < pScalars->Get_Array().size();
			++iScalar)
		{
			const DATA_JSON_VALUE& ScalarValue = pScalars->Get_Array()[iScalar];
			const std::string strScalarContext = strContext + " scalar row " +
				std::to_string(iScalar);
			if (!Validate_ExactOrderedObject(ScalarValue, { "name", "value" },
					strScalarContext, strOutError))
			{
				return false;
			}
			DESCRIPTOR_SCALAR_ROW Scalar;
			if (!Read_String(ScalarValue, "name", Scalar.strName,
					strScalarContext, strOutError) ||
				!Validate_StableId(Scalar.strName,
					strScalarContext + " name", strOutError) ||
				!Read_Float(ScalarValue, "value", Scalar.fValue,
					strScalarContext, strOutError))
			{
				return false;
			}
			if (!ScalarNames.emplace(Scalar.strName).second)
			{
				strOutError = strScalarContext + " repeats parameter name '" +
					Scalar.strName + "'.";
				return false;
			}
			OutDescriptor.Scalars.push_back(std::move(Scalar));
		}

		return Parse_DescriptorVectorRows(Value, "vectors", MAX_VECTOR_ROWS,
				OutDescriptor.Vectors, strContext, strOutError) &&
			Parse_DescriptorVectorRows(Value, "artistParameters",
				MAX_ARTIST_PARAMETER_ROWS, OutDescriptor.ArtistParameters,
				strContext, strOutError) &&
			Parse_DescriptorVectorRows(Value, "colors", MAX_COLOR_ROWS,
				OutDescriptor.Colors, strContext, strOutError);
	}

	bool_t Validate_DescriptorLayout(
		const DESCRIPTOR_RECORD& Descriptor,
		const LAYOUT_RECORD& Layout,
		std::string& strOutError)
	{
		const std::string strContext = "Material descriptor '" +
			Descriptor.strDescriptorId + "'";
		if (Descriptor.strLayoutId != Layout.strLayoutId ||
			Descriptor.TextureLanes.size() != Layout.TextureLanes.size() ||
			Descriptor.Scalars.size() != Layout.ScalarRows.size() ||
			Descriptor.Vectors.size() != Layout.VectorRows.size() ||
			Descriptor.ArtistParameters.size() !=
				Layout.ArtistParameterRows.size() ||
			Descriptor.Colors.size() != Layout.ColorRows.size())
		{
			strOutError = strContext +
				" value-row counts do not match layout '" + Layout.strLayoutId +
				"'.";
			return false;
		}
		for (size_t iLane = 0u; iLane < Layout.TextureLanes.size(); ++iLane)
		{
			if (Descriptor.TextureLanes[iLane].strLaneId !=
				Layout.TextureLanes[iLane].strLaneId)
			{
				strOutError = strContext +
					" texture lane order/identity differs from layout at index " +
					std::to_string(iLane) + ".";
				return false;
			}
		}
		const auto SameNames = [](const auto& Values, const auto& Rows)
		{
			for (size_t iRow = 0u; iRow < Rows.size(); ++iRow)
			{
				if (Values[iRow].strName != Rows[iRow].strName)
					return false;
			}
			return true;
		};
		if (!SameNames(Descriptor.Scalars, Layout.ScalarRows) ||
			!SameNames(Descriptor.Vectors, Layout.VectorRows) ||
			!SameNames(Descriptor.ArtistParameters,
				Layout.ArtistParameterRows) ||
			!SameNames(Descriptor.Colors, Layout.ColorRows))
		{
			strOutError = strContext +
				" ordered parameter names differ from its layout.";
			return false;
		}
		return true;
	}

	bool_t Parse_Binding(
		const DATA_JSON_VALUE& Value,
		const size_t iIndex,
		BINDING_RECORD& OutBinding,
		std::string& strOutError)
	{
		const std::string strContext = "Material binding row " +
			std::to_string(iIndex);
		if (!Validate_ExactOrderedObject(Value,
			{ "effectAssetId", "elementId", "programId", "layoutId",
				"descriptorId", "adapterId", "inlineMirrorPolicy" }, strContext,
			strOutError) ||
			!Read_String(Value, "effectAssetId", OutBinding.strEffectAssetId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strEffectAssetId,
				strContext + " effectAssetId", strOutError) ||
			!Read_String(Value, "elementId", OutBinding.strElementId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strElementId,
				strContext + " elementId", strOutError) ||
			!Read_String(Value, "programId", OutBinding.strProgramId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strProgramId,
				strContext + " programId", strOutError) ||
			!Read_String(Value, "layoutId", OutBinding.strLayoutId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strLayoutId,
				strContext + " layoutId", strOutError) ||
			!Read_String(Value, "descriptorId", OutBinding.strDescriptorId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strDescriptorId,
				strContext + " descriptorId", strOutError) ||
			!Read_String(Value, "adapterId", OutBinding.strAdapterId,
				strContext, strOutError) ||
			!Validate_StableId(OutBinding.strAdapterId,
				strContext + " adapterId", strOutError))
		{
			return false;
		}
		std::string strInlineMirrorPolicy;
		if (!Read_String(Value, "inlineMirrorPolicy", strInlineMirrorPolicy,
				strContext, strOutError))
		{
			return false;
		}
		if (strInlineMirrorPolicy == "INLINE_MIRROR_REQUIRED")
		{
			OutBinding.eInlineMirrorPolicy =
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::INLINE_MIRROR_REQUIRED;
		}
		else
		{
			strOutError = strContext +
				" requires INLINE_MIRROR_REQUIRED in registry format v1.";
			return false;
		}
		return true;
	}

	EFFECT_COMPILED_MATERIAL_ADAPTER_DESC Make_CompiledAdapter(
		const EFFECT_COMPILED_MATERIAL_ADAPTER_ID eAdapterId,
		const EFFECT_COMPILED_MATERIAL_CARRIER eCarrier,
		const std::string_view strAdapterId,
		const std::string_view strShaderId,
		const std::string_view strVertexLayoutId,
		const EFFECT_RENDER_PROFILE eRenderProfile,
		const uint32_t iPassIndex,
		const std::string_view strRasterizerState,
		const std::string_view strDepthStencilState,
		const std::string_view strBlendState) noexcept
	{
		EFFECT_COMPILED_MATERIAL_ADAPTER_DESC Adapter;
		Adapter.eAdapterId = eAdapterId;
		Adapter.eCarrier = eCarrier;
		Adapter.strAdapterId = strAdapterId;
		Adapter.strShaderId = strShaderId;
		Adapter.strVertexLayoutId = strVertexLayoutId;
		Adapter.eRenderProfile = eRenderProfile;
		Adapter.iPassIndex = iPassIndex;
		Adapter.strMrtId = "MRT_SceneHDR";
		Adapter.iSceneColorRenderTargetIndex = 0u;
		Adapter.strSceneColorSemantic = "SV_TARGET0";
		Adapter.iDistortionRenderTargetIndex = 1u;
		Adapter.strDistortionSemantic = "SV_TARGET1";
		Adapter.bDistortionDeterministicZero = true;
		Adapter.strRasterizerState = strRasterizerState;
		Adapter.strDepthStencilState = strDepthStencilState;
		Adapter.strBlendState = strBlendState;
		Adapter.iStencilReference = 0u;
		return Adapter;
	}

	const std::array<EFFECT_COMPILED_MATERIAL_ADAPTER_DESC, 8u>&
		Compiled_Adapters() noexcept
	{
		using ADAPTER_ID = EFFECT_COMPILED_MATERIAL_ADAPTER_ID;
		using CARRIER = EFFECT_COMPILED_MATERIAL_CARRIER;
		static const std::array<EFFECT_COMPILED_MATERIAL_ADAPTER_DESC, 8u>
			ADAPTERS = {{
			Make_CompiledAdapter(
				ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
				CARRIER::SPRITE_PARTICLE,
				EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ, 1u,
				"RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
			Make_CompiledAdapter(
				ADAPTER_ID::MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
				CARRIER::MESH_PARTICLE_CMODEL,
				EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ, 1u,
				"RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
			Make_CompiledAdapter(
				ADAPTER_ID::LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
				CARRIER::LOCAL_DECAL_PROJECTOR,
				EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectDecal.hlsl", "VTXTEX",
				EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ, 3u,
				"RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
			Make_CompiledAdapter(
				ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
				CARRIER::SPRITE_PARTICLE,
				EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ, 3u,
				"RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
			Make_CompiledAdapter(
				ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_TWO_SIDED_V1,
				CARRIER::SPRITE_PARTICLE,
				EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ, 2u,
				"RS_Cull_None", "DSS_ReadOnly", "BS_EffectAdditive"),
			Make_CompiledAdapter(
				ADAPTER_ID::SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_ONE_SIDED_V1,
				CARRIER::SPRITE_PARTICLE,
				EFFECT_SPRITE_PARTICLE_SCENE_COLOR_ADDITIVE_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectParticle.hlsl", "VTXEFFECT_PARTICLE",
				EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ, 4u,
				"RS_Default", "DSS_ReadOnly", "BS_EffectAdditive"),
			Make_CompiledAdapter(
				ADAPTER_ID::MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1,
				CARRIER::MESH_PARTICLE_CMODEL,
				EFFECT_MESH_PARTICLE_SCENE_COLOR_ALPHA_ONE_SIDED_ADAPTER_ID,
				"Shader_VtxEffectMeshPreview.hlsl", "VTXMESH",
				EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ, 3u,
				"RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
			Make_CompiledAdapter(
				ADAPTER_ID::LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
				CARRIER::LOCAL_DECAL_PROJECTOR,
				EFFECT_LOCAL_DECAL_SCENE_COLOR_ALPHA_TWO_SIDED_ADAPTER_ID,
				"Shader_VtxEffectDecal.hlsl", "VTXTEX",
				EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ, 1u,
				"RS_Cull_None", "DSS_ZNone", "BS_EffectAlpha")
		}};
		return ADAPTERS;
	}

	const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC* Find_CompiledAdapter(
		const std::string_view strAdapterId) noexcept
	{
		const auto& Adapters = Compiled_Adapters();
		const auto Adapter = std::find_if(Adapters.begin(), Adapters.end(),
			[strAdapterId](const auto& Candidate)
			{
				return Candidate.strAdapterId == strAdapterId;
			});
		return Adapter == Adapters.end() ? nullptr : &*Adapter;
	}

	struct COMPILED_LAYOUT_LANE_ABI_RECEIPT final
	{
		std::string_view strRole;
		uint32_t iTextureRegister = 0u;
		uint32_t iSamplerRegister = 0u;
		std::string_view strSourceChannel;
		EFFECT_TEXTURE_COLOR_SPACE eColorSpace =
			EFFECT_TEXTURE_COLOR_SPACE::END;
	};

	struct COMPILED_LAYOUT_ABI_RECEIPT final
	{
		EFFECT_MATERIAL_EXECUTION_BACKEND eBackend =
			EFFECT_MATERIAL_EXECUTION_BACKEND::END;
		uint32_t iOpcode = 0u;
		uint32_t iExecutionVersion = 0u;
		uint32_t iTextureLaneCount = 0u;
		uint32_t iTextureMask = 0u;
		std::array<COMPILED_LAYOUT_LANE_ABI_RECEIPT, MAX_TEXTURE_LANES>
			TextureLanes{};
		uint32_t iDynamicConsumedMask = 0u;
		uint32_t iDynamicSuppressedMask = 0u;
		uint32_t iParticleColorPolicy = 0u;
		uint32_t iParticleColorConsumedMask = 0u;
		uint32_t iParticleColorSuppressedMask = 0u;
		uint32_t iScalarCount = 0u;
		uint32_t iVectorCount = 0u;
		uint32_t iInputCount = 0u;
		std::array<uint32_t, 2u> InputConsumedMask{};
		std::array<uint32_t, 2u> InputSuppressedMask{};
		std::array<uint32_t, 3u> VectorComponentConsumedMask{};
		std::array<uint32_t, 3u> VectorComponentSuppressedMask{};
		uint32_t iStaticInputCount = 0u;
		uint32_t iStaticSelectedMask = 0u;
		uint32_t iStaticConsumedMask = 0u;
		uint32_t iStaticSuppressedMask = 0u;
		uint32_t iRenderInputCount = 0u;
		uint32_t iRenderConsumedMask = 0u;
		uint32_t iRenderSuppressedMask = 0u;
		uint32_t iArtistParameterRowCount = 0u;
		uint32_t iColorRowCount = 0u;
	};

	const std::array<COMPILED_LAYOUT_ABI_RECEIPT, 3u>&
		Compiled_LayoutAbiReceipts() noexcept
	{
		static const std::array<COMPILED_LAYOUT_ABI_RECEIPT, 3u> RECEIPTS = []
		{
			std::array<COMPILED_LAYOUT_ABI_RECEIPT, 3u> Result{};
			COMPILED_LAYOUT_ABI_RECEIPT& Sprite = Result[0u];
			Sprite.eBackend =
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2;
			Sprite.iOpcode = 6u;
			Sprite.iExecutionVersion = 1u;
			Sprite.iTextureLaneCount = 2u;
			Sprite.iTextureMask = 0x03u;
			Sprite.TextureLanes[0u] = { "sparkle_tex", 0u, 5u, "",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Sprite.TextureLanes[1u] = { "edgedeco.texture01", 1u, 6u, "",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Sprite.iDynamicConsumedMask = 0x0fu;
			Sprite.iParticleColorPolicy = 3u;
			Sprite.iParticleColorConsumedMask = 0x0fu;
			Sprite.iScalarCount = 3u;
			Sprite.iInputCount = 6u;
			Sprite.InputConsumedMask = { 55u, 0u };
			Sprite.InputSuppressedMask = { 8u, 0u };
			Sprite.iRenderInputCount = 6u;
			Sprite.iRenderConsumedMask = 47u;
			Sprite.iRenderSuppressedMask = 16u;

			COMPILED_LAYOUT_ABI_RECEIPT& Mesh = Result[1u];
			Mesh.eBackend =
				EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2;
			Mesh.iOpcode = 3u;
			Mesh.iExecutionVersion = 1u;
			Mesh.iTextureLaneCount = 2u;
			Mesh.iTextureMask = 0x03u;
			Mesh.TextureLanes[0u] = { "maintex", 0u, 5u, "",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Mesh.TextureLanes[1u] = { "uv_noise_tex", 1u, 6u, "",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Mesh.iDynamicConsumedMask = 0x0fu;
			Mesh.iParticleColorPolicy = 2u;
			Mesh.iParticleColorConsumedMask = 0x0fu;
			Mesh.iScalarCount = 29u;
			Mesh.iVectorCount = 1u;
			Mesh.iInputCount = 32u;
			Mesh.InputConsumedMask = { 3489660919u, 0u };
			Mesh.InputSuppressedMask = { 805306376u, 0u };
			Mesh.VectorComponentSuppressedMask = { 15u, 0u, 0u };
			Mesh.iStaticInputCount = 14u;
			Mesh.iStaticSelectedMask = 13311u;
			Mesh.iStaticConsumedMask = 16383u;
			Mesh.iRenderInputCount = 6u;
			Mesh.iRenderConsumedMask = 47u;
			Mesh.iRenderSuppressedMask = 16u;

			COMPILED_LAYOUT_ABI_RECEIPT& Decal = Result[2u];
			Decal.eBackend = EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL;
			Decal.iOpcode = 14u;
			Decal.iExecutionVersion = 1u;
			Decal.iTextureLaneCount = 6u;
			Decal.iTextureMask = 0x3fu;
			Decal.TextureLanes[0u] = { "height", 0u, 5u, "B",
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR };
			Decal.TextureLanes[1u] = { "diffuse", 1u, 6u, "RGBA",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Decal.TextureLanes[2u] = { "dissolve", 2u, 7u, "G",
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR };
			Decal.TextureLanes[3u] = { "normal", 3u, 8u, "RG",
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR };
			Decal.TextureLanes[4u] = { "specular", 4u, 9u, "RGB",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Decal.TextureLanes[5u] = { "emissive", 5u, 10u, "R",
				EFFECT_TEXTURE_COLOR_SPACE::SRGB };
			Decal.iDynamicSuppressedMask = 0x0fu;
			Decal.iScalarCount = 22u;
			Decal.iVectorCount = 3u;
			Decal.iInputCount = 33u;
			Decal.InputConsumedMask = { 2182005247u, 1u };
			Decal.InputSuppressedMask = { 2112962048u, 0u };
			Decal.VectorComponentConsumedMask = { 15u, 15u, 0u };
			Decal.VectorComponentSuppressedMask = { 0u, 0u, 15u };
			Decal.iStaticInputCount = 18u;
			Decal.iStaticSelectedMask = 262139u;
			Decal.iStaticConsumedMask = 262143u;
			Decal.iRenderInputCount = 6u;
			Decal.iRenderConsumedMask = 3u;
			Decal.iRenderSuppressedMask = 60u;
			return Result;
		}();
		return RECEIPTS;
	}

	bool_t Matches_CompiledPackedRows(
		const std::vector<LAYOUT_PARAMETER_ROW>& Rows,
		const std::string_view strPrefix,
		const uint32_t iExpectedCount)
	{
		if (Rows.size() != iExpectedCount)
			return false;
		for (size_t iRow = 0u; iRow < Rows.size(); ++iRow)
		{
			if (Rows[iRow].iPackedIndex != iRow || Rows[iRow].strName !=
				std::string(strPrefix) + "." + std::to_string(iRow))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Matches_CompiledLayoutAbiReceipt(
		const PROGRAM_RECORD& Program,
		const LAYOUT_RECORD& Layout)
	{
		if (Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Program.iOpcode == 1u)
		{
			const auto AllZero = [](const auto& Values)
			{
				return std::all_of(Values.begin(), Values.end(),
					[](const uint32_t iValue) { return 0u == iValue; });
			};
			const auto IsBaseChannel = [](const std::string_view strChannel)
			{
				return strChannel == "R" || strChannel == "G" ||
					strChannel == "B" || strChannel == "RGB";
			};
			const auto IsCoverageChannel = [](const std::string_view strChannel)
			{
				return strChannel == "R" || strChannel == "G" ||
					strChannel == "B" || strChannel == "A";
			};
			const bool_t bHasDissolve = Layout.iTextureLaneCount == 3u;
			if (Layout.iExecutionVersion != 1u ||
				(Layout.iTextureLaneCount != 2u && !bHasDissolve) ||
				Layout.iTextureMask != Low_BitMask(Layout.iTextureLaneCount) ||
				Layout.TextureLanes.size() != Layout.iTextureLaneCount ||
				0u != Layout.iDynamicConsumedMask ||
				0u != Layout.iDynamicSuppressedMask ||
				0u != Layout.iParticleColorPolicy ||
				0u != Layout.iParticleColorConsumedMask ||
				0u != Layout.iParticleColorSuppressedMask ||
				0u != Layout.iScalarCount || 0u != Layout.iVectorCount ||
				0u != Layout.iInputCount ||
				!AllZero(Layout.InputConsumedMask) ||
				!AllZero(Layout.InputSuppressedMask) ||
				!AllZero(Layout.VectorComponentConsumedMask) ||
				!AllZero(Layout.VectorComponentSuppressedMask) ||
				0u != Layout.iStaticInputCount ||
				0u != Layout.iStaticSelectedMask ||
				0u != Layout.iStaticConsumedMask ||
				0u != Layout.iStaticSuppressedMask ||
				0u != Layout.iRenderInputCount ||
				0u != Layout.iRenderConsumedMask ||
				0u != Layout.iRenderSuppressedMask ||
				!Layout.ScalarRows.empty() || !Layout.VectorRows.empty() ||
				!Layout.ArtistParameterRows.empty() || !Layout.ColorRows.empty())
			{
				return false;
			}
			for (size_t iLane = 0u; iLane < Layout.TextureLanes.size(); ++iLane)
			{
				const LAYOUT_TEXTURE_LANE& Lane = Layout.TextureLanes[iLane];
				const std::string_view strExpectedRole = iLane == 0u ?
					"base_radiance" : (iLane == 1u ? "coverage" : "dissolve");
				const bool_t bChannelValid = iLane == 0u ?
					IsBaseChannel(Lane.strSourceChannel) :
					IsCoverageChannel(Lane.strSourceChannel);
				const bool_t bLinearRequired = iLane != 0u &&
					Lane.strSourceChannel != "A";
				if (Lane.strLaneId != "lane." + std::to_string(iLane) ||
					Lane.strRole != strExpectedRole ||
					Lane.iTextureRegister != iLane ||
					Lane.iSamplerRegister !=
						MATERIAL_SAMPLER_REGISTER_BASE + iLane ||
					!bChannelValid ||
					(bLinearRequired && Lane.eColorSpace !=
						EFFECT_TEXTURE_COLOR_SPACE::LINEAR))
				{
					return false;
				}
			}
			return true;
		}

		const auto& Receipts = Compiled_LayoutAbiReceipts();
		const auto Receipt = std::find_if(Receipts.begin(), Receipts.end(),
			[&Program](const COMPILED_LAYOUT_ABI_RECEIPT& Candidate)
			{
				return Candidate.eBackend == Program.eBackend &&
					Candidate.iOpcode == Program.iOpcode;
			});
		if (Receipt == Receipts.end() ||
			Layout.iExecutionVersion != Receipt->iExecutionVersion ||
			Layout.iTextureLaneCount != Receipt->iTextureLaneCount ||
			Layout.iTextureMask != Receipt->iTextureMask ||
			Layout.TextureLanes.size() != Receipt->iTextureLaneCount ||
			Layout.iDynamicConsumedMask != Receipt->iDynamicConsumedMask ||
			Layout.iDynamicSuppressedMask != Receipt->iDynamicSuppressedMask ||
			Layout.iParticleColorPolicy != Receipt->iParticleColorPolicy ||
			Layout.iParticleColorConsumedMask !=
				Receipt->iParticleColorConsumedMask ||
			Layout.iParticleColorSuppressedMask !=
				Receipt->iParticleColorSuppressedMask ||
			Layout.iScalarCount != Receipt->iScalarCount ||
			Layout.iVectorCount != Receipt->iVectorCount ||
			Layout.iInputCount != Receipt->iInputCount ||
			Layout.InputConsumedMask != Receipt->InputConsumedMask ||
			Layout.InputSuppressedMask != Receipt->InputSuppressedMask ||
			Layout.VectorComponentConsumedMask !=
				Receipt->VectorComponentConsumedMask ||
			Layout.VectorComponentSuppressedMask !=
				Receipt->VectorComponentSuppressedMask ||
			Layout.iStaticInputCount != Receipt->iStaticInputCount ||
			Layout.iStaticSelectedMask != Receipt->iStaticSelectedMask ||
			Layout.iStaticConsumedMask != Receipt->iStaticConsumedMask ||
			Layout.iStaticSuppressedMask != Receipt->iStaticSuppressedMask ||
			Layout.iRenderInputCount != Receipt->iRenderInputCount ||
			Layout.iRenderConsumedMask != Receipt->iRenderConsumedMask ||
			Layout.iRenderSuppressedMask != Receipt->iRenderSuppressedMask ||
			!Matches_CompiledPackedRows(
				Layout.ScalarRows, "scalar", Receipt->iScalarCount) ||
			!Matches_CompiledPackedRows(
				Layout.VectorRows, "vector", Receipt->iVectorCount) ||
			!Matches_CompiledPackedRows(Layout.ArtistParameterRows,
				"artist-parameter", Receipt->iArtistParameterRowCount) ||
			!Matches_CompiledPackedRows(
				Layout.ColorRows, "color", Receipt->iColorRowCount))
		{
			return false;
		}
		for (size_t iLane = 0u; iLane < Layout.TextureLanes.size(); ++iLane)
		{
			const LAYOUT_TEXTURE_LANE& Actual = Layout.TextureLanes[iLane];
			const COMPILED_LAYOUT_LANE_ABI_RECEIPT& Expected =
				Receipt->TextureLanes[iLane];
			if (Actual.strLaneId != "lane." + std::to_string(iLane) ||
				Actual.strRole != Expected.strRole ||
				Actual.iTextureRegister != Expected.iTextureRegister ||
				Actual.iSamplerRegister != Expected.iSamplerRegister ||
				Actual.strSourceChannel != Expected.strSourceChannel ||
				Actual.eColorSpace != Expected.eColorSpace)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_CompiledProgramCompatible(
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC& Adapter,
		const PROGRAM_RECORD& Program) noexcept
	{
		const bool_t bStandardColor = Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Program.iOpcode == 1u;
		using ADAPTER_ID = EFFECT_COMPILED_MATERIAL_ADAPTER_ID;
		switch (Adapter.eCarrier)
		{
		case EFFECT_COMPILED_MATERIAL_CARRIER::SPRITE_PARTICLE:
			return bStandardColor ?
				(Adapter.eAdapterId == ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1 ||
				 Adapter.eAdapterId == ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1 ||
				 Adapter.eAdapterId == ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_TWO_SIDED_V1 ||
				 Adapter.eAdapterId == ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ADDITIVE_ONE_SIDED_V1) :
				(Program.eBackend ==
					EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				 Program.iOpcode == 6u && Adapter.eAdapterId == ADAPTER_ID::
					SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1);
		case EFFECT_COMPILED_MATERIAL_CARRIER::MESH_PARTICLE_CMODEL:
			return bStandardColor ?
				(Adapter.eAdapterId == ADAPTER_ID::
					MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1 ||
				 Adapter.eAdapterId == ADAPTER_ID::
					MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1) :
				(Program.eBackend ==
					EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
				 Program.iOpcode == 3u && Adapter.eAdapterId == ADAPTER_ID::
					MESH_PARTICLE_CMODEL_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1);
		case EFFECT_COMPILED_MATERIAL_CARRIER::LOCAL_DECAL_PROJECTOR:
			return bStandardColor ?
				(Adapter.eAdapterId == ADAPTER_ID::
					LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1 ||
				 Adapter.eAdapterId == ADAPTER_ID::
					LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1) :
				(Program.eBackend == EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL &&
				 Program.iOpcode == 14u && Adapter.eAdapterId == ADAPTER_ID::
					LOCAL_DECAL_PROJECTOR_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_ONE_SIDED_V1);
		case EFFECT_COMPILED_MATERIAL_CARRIER::END:
		default:
			return false;
		}
	}

	bool_t Materialize_Binding(
		const BINDING_RECORD& Binding,
		const PROGRAM_RECORD& Program,
		const LAYOUT_RECORD& Layout,
		const DESCRIPTOR_RECORD& Descriptor,
		const uint64_t iCatalogRevision,
		const uint64_t iGenerationId,
		std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>&
			pOutResolved,
		std::string& strOutError)
	{
		const EFFECT_COMPILED_MATERIAL_ADAPTER_DESC* pAdapter =
			Find_CompiledAdapter(Binding.strAdapterId);
		const bool_t bVariableStandardColorLayout = Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1 &&
			Program.iOpcode == 1u;
		std::string_view strExpectedLayoutId;
		if (Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Program.iOpcode == 6u)
		{
			strExpectedLayoutId =
				"effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1";
		}
		else if (Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
			Program.iOpcode == 3u)
		{
			strExpectedLayoutId =
				"effect.layout.runtime-material-v2.opcode-3.abi-85c02e5f1f646d22.v1";
		}
		else if (!bVariableStandardColorLayout)
		{
			strExpectedLayoutId =
				"effect.layout.local-decal.opcode-14.abi-c6b52a791b98f0c5.v1";
		}
		if ((!bVariableStandardColorLayout &&
			 Layout.strLayoutId != strExpectedLayoutId) ||
			!Matches_CompiledLayoutAbiReceipt(Program, Layout))
		{
			strOutError = "Material binding '" + Binding.strEffectAssetId + "/" +
				Binding.strElementId +
				"' layout differs from its compiled backend/opcode ABI receipt.";
			return false;
		}
		if (nullptr == pAdapter ||
			!Is_CompiledProgramCompatible(*pAdapter, Program) ||
			Binding.eInlineMirrorPolicy ==
				EFFECT_MATERIAL_INLINE_MIRROR_POLICY::END ||
			Layout.iExecutionVersion != 1u ||
			Descriptor.strLayoutId != Layout.strLayoutId)
		{
			strOutError = "Material binding '" + Binding.strEffectAssetId + "/" +
				Binding.strElementId +
				"' is incompatible with its compiled carrier/program adapter.";
			return false;
		}
		auto pResolved =
			std::make_shared<EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>();
		pResolved->iCatalogRevision = iCatalogRevision;
		pResolved->iRegistryGenerationId = iGenerationId;
		pResolved->strEffectAssetId = Binding.strEffectAssetId;
		pResolved->strElementId = Binding.strElementId;
		pResolved->strProgramId = Binding.strProgramId;
		pResolved->strLayoutId = Binding.strLayoutId;
		pResolved->strDescriptorId = Binding.strDescriptorId;
		pResolved->strAdapterId = Binding.strAdapterId;
		pResolved->eInlineMirrorPolicy = Binding.eInlineMirrorPolicy;
		pResolved->Adapter = *pAdapter;

		EFFECT_MATERIAL_EXECUTION_DESC& Execution = pResolved->Execution;
		Execution.bEnabled = true;
		Execution.bFailClosed = false;
		Execution.bAuthoringApproximate = false;
		Execution.iVersion = Layout.iExecutionVersion;
		Execution.eBackend = Program.eBackend;
		Execution.iOpcode = Program.iOpcode;
		Execution.iPassIndex = pResolved->Adapter.iPassIndex;
		Execution.strRasterizerState = pResolved->Adapter.strRasterizerState;
		Execution.strDepthStencilState = pResolved->Adapter.strDepthStencilState;
		Execution.strBlendState = pResolved->Adapter.strBlendState;
		Execution.iStencilReference = pResolved->Adapter.iStencilReference;
		Execution.iTextureLaneCount = Layout.iTextureLaneCount;
		Execution.iTextureMask = Layout.iTextureMask;
		Execution.iDynamicConsumedMask = Layout.iDynamicConsumedMask;
		Execution.iDynamicSuppressedMask = Layout.iDynamicSuppressedMask;
		Execution.iParticleColorPolicy = Layout.iParticleColorPolicy;
		Execution.iParticleColorConsumedMask =
			Layout.iParticleColorConsumedMask;
		Execution.iParticleColorSuppressedMask =
			Layout.iParticleColorSuppressedMask;
		Execution.iScalarCount = Layout.iScalarCount;
		Execution.iVectorCount = Layout.iVectorCount;
		Execution.iInputCount = Layout.iInputCount;
		Execution.InputConsumedMask = Layout.InputConsumedMask;
		Execution.InputSuppressedMask = Layout.InputSuppressedMask;
		Execution.VectorComponentConsumedMask =
			Layout.VectorComponentConsumedMask;
		Execution.VectorComponentSuppressedMask =
			Layout.VectorComponentSuppressedMask;
		Execution.iStaticInputCount = Layout.iStaticInputCount;
		Execution.iStaticSelectedMask = Layout.iStaticSelectedMask;
		Execution.iStaticConsumedMask = Layout.iStaticConsumedMask;
		Execution.iStaticSuppressedMask = Layout.iStaticSuppressedMask;
		Execution.iRenderInputCount = Layout.iRenderInputCount;
		Execution.iRenderConsumedMask = Layout.iRenderConsumedMask;
		Execution.iRenderSuppressedMask = Layout.iRenderSuppressedMask;

		Execution.TextureLanes.reserve(Layout.TextureLanes.size());
		for (size_t iLane = 0u; iLane < Layout.TextureLanes.size(); ++iLane)
		{
			const LAYOUT_TEXTURE_LANE& Structural = Layout.TextureLanes[iLane];
			const DESCRIPTOR_TEXTURE_LANE& Values = Descriptor.TextureLanes[iLane];
			EFFECT_MATERIAL_TEXTURE_LANE_DESC Lane;
			Lane.strLaneId = Structural.strLaneId;
			Lane.strRole = Structural.strRole;
			Lane.strAssetId = Values.strAssetId;
			Lane.iTextureRegister = Structural.iTextureRegister;
			Lane.iSamplerRegister = Structural.iSamplerRegister;
			Lane.strSourceChannel = Structural.strSourceChannel;
			Lane.eColorSpace = Structural.eColorSpace;
			Lane.Sampler = Values.Sampler;
			Execution.TextureLanes.push_back(std::move(Lane));
		}
		Execution.Scalars.reserve(Layout.ScalarRows.size());
		for (size_t iRow = 0u; iRow < Layout.ScalarRows.size(); ++iRow)
		{
			Execution.Scalars.push_back({ Layout.ScalarRows[iRow].strName,
				Layout.ScalarRows[iRow].iPackedIndex,
				Descriptor.Scalars[iRow].fValue });
		}
		const auto MaterializeVectors = [](const auto& LayoutRows,
			const auto& DescriptorRows, auto& OutRows)
		{
			OutRows.reserve(LayoutRows.size());
			for (size_t iRow = 0u; iRow < LayoutRows.size(); ++iRow)
			{
				OutRows.push_back({ LayoutRows[iRow].strName,
					LayoutRows[iRow].iPackedIndex,
					DescriptorRows[iRow].vValue });
			}
		};
		MaterializeVectors(Layout.VectorRows, Descriptor.Vectors,
			Execution.Vectors);
		MaterializeVectors(Layout.ArtistParameterRows,
			Descriptor.ArtistParameters, Execution.ArtistParameters);
		MaterializeVectors(Layout.ColorRows, Descriptor.Colors,
			Execution.Colors);
		if (Program.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1)
		{
			const auto ChannelFromToken = [](const std::string_view strChannel)
			{
				if (strChannel == "R")
					return EFFECT_STANDARD_COLOR_CHANNEL::R;
				if (strChannel == "G")
					return EFFECT_STANDARD_COLOR_CHANNEL::G;
				if (strChannel == "B")
					return EFFECT_STANDARD_COLOR_CHANNEL::B;
				if (strChannel == "A")
					return EFFECT_STANDARD_COLOR_CHANNEL::A;
				if (strChannel == "RGB")
					return EFFECT_STANDARD_COLOR_CHANNEL::RGB;
				return EFFECT_STANDARD_COLOR_CHANNEL::INVALID;
			};
			EFFECT_STANDARD_COLOR_V1_DESC& Packet = Execution.StandardColorV1;
			Packet.iPacketVersion = 1u;
			Packet.strBaseRadianceLaneId = Layout.TextureLanes[0u].strLaneId;
			Packet.eBaseRadianceChannel =
				ChannelFromToken(Layout.TextureLanes[0u].strSourceChannel);
			Packet.strCoverageLaneId = Layout.TextureLanes[1u].strLaneId;
			Packet.eCoverageChannel =
				ChannelFromToken(Layout.TextureLanes[1u].strSourceChannel);
			Packet.eEmissiveMode =
				EFFECT_STANDARD_COLOR_EMISSIVE_MODE::BASE_RADIANCE;
			Packet.eLifetimeEnvelope =
				EFFECT_STANDARD_COLOR_LIFETIME_ENVELOPE::CARRIER_ALPHA;
			Packet.eMissingLanePolicy =
				EFFECT_STANDARD_COLOR_MISSING_LANE_POLICY::FAIL_CLOSED;
			if (Layout.TextureLanes.size() == 3u)
			{
				Packet.eDissolveMode =
					EFFECT_STANDARD_COLOR_DISSOLVE_MODE::LANE_THRESHOLD;
				Packet.strDissolveLaneId = Layout.TextureLanes[2u].strLaneId;
				Packet.eDissolveChannel =
					ChannelFromToken(Layout.TextureLanes[2u].strSourceChannel);
				Packet.fDissolveSoftness = 0.1f;
			}
		}

		pOutResolved = std::move(pResolved);
		return true;
	}

	bool_t Same_FloatBits(const f32_t Left, const f32_t Right) noexcept
	{
		static_assert(sizeof(f32_t) == sizeof(uint32_t));
		return std::bit_cast<uint32_t>(Left) ==
			std::bit_cast<uint32_t>(Right);
	}

	bool_t Same_Float4Bits(
		const float4_t& Left,
		const float4_t& Right) noexcept
	{
		return Same_FloatBits(Left.x, Right.x) &&
			Same_FloatBits(Left.y, Right.y) &&
			Same_FloatBits(Left.z, Right.z) &&
			Same_FloatBits(Left.w, Right.w);
	}

	bool_t Same_SamplerBits(
		const EFFECT_MATERIAL_SAMPLER_DESC& Left,
		const EFFECT_MATERIAL_SAMPLER_DESC& Right) noexcept
	{
		return Left.eFilter == Right.eFilter &&
			Left.eAddressU == Right.eAddressU &&
			Left.eAddressV == Right.eAddressV &&
			Left.eAddressW == Right.eAddressW &&
			Same_FloatBits(Left.fMipLodBias, Right.fMipLodBias) &&
			Left.iMaxAnisotropy == Right.iMaxAnisotropy &&
			Left.eComparison == Right.eComparison &&
			Same_Float4Bits(Left.vBorderColor, Right.vBorderColor) &&
			Same_FloatBits(Left.fMinLod, Right.fMinLod) &&
			Same_FloatBits(Left.fMaxLod, Right.fMaxLod);
	}

	bool_t Same_StandardColorBits(
		const EFFECT_STANDARD_COLOR_V1_DESC& Left,
		const EFFECT_STANDARD_COLOR_V1_DESC& Right) noexcept
	{
		return Left.iPacketVersion == Right.iPacketVersion &&
			Left.strBaseRadianceLaneId == Right.strBaseRadianceLaneId &&
			Left.eBaseRadianceChannel == Right.eBaseRadianceChannel &&
			Left.strCoverageLaneId == Right.strCoverageLaneId &&
			Left.eCoverageChannel == Right.eCoverageChannel &&
			Left.eEmissiveMode == Right.eEmissiveMode &&
			Left.eLifetimeEnvelope == Right.eLifetimeEnvelope &&
			Left.eDissolveMode == Right.eDissolveMode &&
			Left.strDissolveLaneId == Right.strDissolveLaneId &&
			Left.eDissolveChannel == Right.eDissolveChannel &&
			Same_FloatBits(Left.fDissolveSoftness, Right.fDissolveSoftness) &&
			Left.eMissingLanePolicy == Right.eMissingLanePolicy;
	}

	bool_t Same_TextureLanes(
		const std::vector<EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Left,
		const std::vector<EFFECT_MATERIAL_TEXTURE_LANE_DESC>& Right) noexcept
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iLane = 0u; iLane < Left.size(); ++iLane)
		{
			const auto& A = Left[iLane];
			const auto& B = Right[iLane];
			if (A.strLaneId != B.strLaneId || A.strRole != B.strRole ||
				A.strAssetId != B.strAssetId ||
				A.iTextureRegister != B.iTextureRegister ||
				A.iSamplerRegister != B.iSamplerRegister ||
				A.strSourceChannel != B.strSourceChannel ||
				A.eColorSpace != B.eColorSpace ||
				!Same_SamplerBits(A.Sampler, B.Sampler))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Same_Scalars(
		const std::vector<EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Left,
		const std::vector<EFFECT_MATERIAL_SCALAR_PARAMETER_DESC>& Right) noexcept
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iRow = 0u; iRow < Left.size(); ++iRow)
		{
			if (Left[iRow].strName != Right[iRow].strName ||
				Left[iRow].iPackedIndex != Right[iRow].iPackedIndex ||
				!Same_FloatBits(Left[iRow].fValue, Right[iRow].fValue))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Same_Vectors(
		const std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Left,
		const std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Right) noexcept
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iRow = 0u; iRow < Left.size(); ++iRow)
		{
			if (Left[iRow].strName != Right[iRow].strName ||
				Left[iRow].iPackedIndex != Right[iRow].iPackedIndex ||
				!Same_Float4Bits(Left[iRow].vValue, Right[iRow].vValue))
			{
				return false;
			}
		}
		return true;
	}
}

struct Client::CEffectMaterialProgramRegistry::IMPLEMENTATION final
{
	using RESOLVED_PTR =
		std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>;
	using ELEMENT_BINDINGS =
		std::map<std::string, RESOLVED_PTR, std::less<>>;

	std::map<std::string, ELEMENT_BINDINGS, std::less<>> Bindings;
	std::vector<EFFECT_MATERIAL_PROGRAM_BINDING_TARGET> BindingTargets;
};

Client::CEffectMaterialProgramRegistry::CEffectMaterialProgramRegistry(
	const uint64_t iCatalogRevision,
	const uint64_t iGenerationId,
	std::shared_ptr<const IMPLEMENTATION> pImplementation) noexcept
	: m_iCatalogRevision(iCatalogRevision)
	, m_iGenerationId(iGenerationId)
	, m_pImplementation(std::move(pImplementation))
{
}

std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
Client::CEffectMaterialProgramRegistry::Create_Empty(
	const uint64_t iCatalogRevision,
	const uint64_t iGenerationId,
	std::string& strOutError)
{
	strOutError.clear();
	if (0u == iCatalogRevision || 0u == iGenerationId)
	{
		strOutError =
			"Empty material-program registry generation identity is invalid.";
		return nullptr;
	}
	auto pImplementation = std::make_shared<IMPLEMENTATION>();
	strOutError.clear();
	return std::shared_ptr<const CEffectMaterialProgramRegistry>(
		new CEffectMaterialProgramRegistry(iCatalogRevision, iGenerationId,
			std::move(pImplementation)));
}

std::shared_ptr<const Client::CEffectMaterialProgramRegistry>
Client::CEffectMaterialProgramRegistry::Create(
	const DATA_JSON_VALUE& Root,
	const uint64_t iCatalogRevision,
	const uint64_t iGenerationId,
	std::string& strOutError)
{
	strOutError.clear();
	if (0u == iCatalogRevision || 0u == iGenerationId)
	{
		strOutError = "Material-program registry generation identity is invalid.";
		return nullptr;
	}
	if (!Validate_ExactOrderedObject(Root,
		{ "schema", "formatVersion", "programs", "layouts", "descriptors",
			"adapters", "bindings" }, "Material-program registry root",
		strOutError))
	{
		return nullptr;
	}
	std::string strSchema;
	uint32_t iFormatVersion = 0u;
	if (!Read_String(Root, "schema", strSchema,
			"Material-program registry root", strOutError) ||
		!Read_UInt32(Root, "formatVersion", iFormatVersion,
			"Material-program registry root", strOutError) ||
		strSchema != EFFECT_MATERIAL_PROGRAM_REGISTRY_SCHEMA ||
		iFormatVersion != EFFECT_MATERIAL_PROGRAM_REGISTRY_FORMAT_VERSION)
	{
		if (strOutError.empty())
			strOutError = "Material-program registry schema/version is invalid.";
		return nullptr;
	}

	const DATA_JSON_VALUE* pPrograms = Require_Field(Root, "programs",
		DATA_JSON_TYPE::ARRAY, "Material-program registry root", strOutError);
	const DATA_JSON_VALUE* pLayouts = Require_Field(Root, "layouts",
		DATA_JSON_TYPE::ARRAY, "Material-program registry root", strOutError);
	const DATA_JSON_VALUE* pDescriptors = Require_Field(Root, "descriptors",
		DATA_JSON_TYPE::ARRAY, "Material-program registry root", strOutError);
	const DATA_JSON_VALUE* pAdapters = Require_Field(Root, "adapters",
		DATA_JSON_TYPE::ARRAY, "Material-program registry root", strOutError);
	const DATA_JSON_VALUE* pBindings = Require_Field(Root, "bindings",
		DATA_JSON_TYPE::ARRAY, "Material-program registry root", strOutError);
	if (nullptr == pPrograms || nullptr == pLayouts || nullptr == pDescriptors ||
		nullptr == pAdapters || nullptr == pBindings)
	{
		return nullptr;
	}
	if (pPrograms->Get_Array().empty() ||
		pPrograms->Get_Array().size() > MAX_PROGRAMS ||
		pLayouts->Get_Array().empty() ||
		pLayouts->Get_Array().size() > MAX_LAYOUTS ||
		pDescriptors->Get_Array().empty() ||
		pDescriptors->Get_Array().size() > MAX_DESCRIPTORS ||
		pAdapters->Get_Array().empty() ||
		pAdapters->Get_Array().size() > Compiled_Adapters().size() ||
		pBindings->Get_Array().size() > MAX_BINDINGS)
	{
		strOutError =
			"Material-program registry collection count is outside its bound.";
		return nullptr;
	}

	std::map<std::string, PROGRAM_RECORD, std::less<>> Programs;
	for (size_t iProgram = 0u; iProgram < pPrograms->Get_Array().size();
		++iProgram)
	{
		PROGRAM_RECORD Program;
		if (!Parse_Program(pPrograms->Get_Array()[iProgram], iProgram,
				Program, strOutError))
		{
			return nullptr;
		}
		const std::string strId = Program.strProgramId;
		if (!Programs.emplace(strId, std::move(Program)).second)
		{
			strOutError = "Duplicate material programId: " + strId;
			return nullptr;
		}
	}

	std::map<std::string, LAYOUT_RECORD, std::less<>> Layouts;
	for (size_t iLayout = 0u; iLayout < pLayouts->Get_Array().size(); ++iLayout)
	{
		LAYOUT_RECORD Layout;
		if (!Parse_Layout(pLayouts->Get_Array()[iLayout], iLayout, Layout,
				strOutError))
		{
			return nullptr;
		}
		const std::string strId = Layout.strLayoutId;
		if (!Layouts.emplace(strId, std::move(Layout)).second)
		{
			strOutError = "Duplicate material layoutId: " + strId;
			return nullptr;
		}
	}

	std::map<std::string, DESCRIPTOR_RECORD, std::less<>> Descriptors;
	for (size_t iDescriptor = 0u;
		iDescriptor < pDescriptors->Get_Array().size(); ++iDescriptor)
	{
		DESCRIPTOR_RECORD Descriptor;
		if (!Parse_Descriptor(pDescriptors->Get_Array()[iDescriptor],
			iDescriptor, Descriptor, strOutError))
		{
			return nullptr;
		}
		const std::string strId = Descriptor.strDescriptorId;
		if (!Descriptors.emplace(strId, std::move(Descriptor)).second)
		{
			strOutError = "Duplicate material descriptorId: " + strId;
			return nullptr;
		}
	}
	for (const auto& [strDescriptorId, Descriptor] : Descriptors)
	{
		const auto Layout = Layouts.find(Descriptor.strLayoutId);
		if (Layout == Layouts.end())
		{
			strOutError = "Material descriptor '" + strDescriptorId +
				"' references unknown layoutId '" + Descriptor.strLayoutId + "'.";
			return nullptr;
		}
		if (!Validate_DescriptorLayout(Descriptor, Layout->second, strOutError))
			return nullptr;
	}

	std::set<std::string, std::less<>> DeclaredAdapters;
	for (size_t iAdapter = 0u;
		iAdapter < pAdapters->Get_Array().size(); ++iAdapter)
	{
		const DATA_JSON_VALUE& AdapterValue =
			pAdapters->Get_Array()[iAdapter];
		const std::string strContext = "Compiled material adapter row " +
			std::to_string(iAdapter);
		if (!Validate_ExactOrderedObject(AdapterValue, { "adapterId" },
				strContext, strOutError))
		{
			return nullptr;
		}
		std::string strAdapterId;
		if (!Read_String(AdapterValue, "adapterId", strAdapterId,
				strContext, strOutError) ||
			!Validate_StableId(strAdapterId,
				strContext + " adapterId", strOutError))
		{
			return nullptr;
		}
		if (nullptr == Find_CompiledAdapter(strAdapterId))
		{
			strOutError = "Unsupported compiled material adapterId: " +
				strAdapterId;
			return nullptr;
		}
		if (!DeclaredAdapters.emplace(std::move(strAdapterId)).second)
		{
			strOutError = "Duplicate compiled material adapterId at row " +
				std::to_string(iAdapter) + ".";
			return nullptr;
		}
	}

	auto pImplementation = std::make_shared<IMPLEMENTATION>();
	pImplementation->BindingTargets.reserve(pBindings->Get_Array().size());
	for (size_t iBinding = 0u; iBinding < pBindings->Get_Array().size();
		++iBinding)
	{
		BINDING_RECORD Binding;
		if (!Parse_Binding(pBindings->Get_Array()[iBinding], iBinding,
				Binding, strOutError))
		{
			return nullptr;
		}
		const auto Program = Programs.find(Binding.strProgramId);
		const auto Layout = Layouts.find(Binding.strLayoutId);
		const auto Descriptor = Descriptors.find(Binding.strDescriptorId);
		if (Program == Programs.end() || Layout == Layouts.end() ||
			Descriptor == Descriptors.end() ||
			!DeclaredAdapters.contains(Binding.strAdapterId))
		{
			strOutError = "Material binding '" + Binding.strEffectAssetId + "/" +
				Binding.strElementId +
				"' has a dangling program/layout/descriptor/adapter reference.";
			return nullptr;
		}
		if (Descriptor->second.strLayoutId != Binding.strLayoutId)
		{
			strOutError = "Material binding '" + Binding.strEffectAssetId + "/" +
				Binding.strElementId +
				"' selects a descriptor owned by another layout.";
			return nullptr;
		}
		std::shared_ptr<const EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
			pResolved;
		if (!Materialize_Binding(Binding, Program->second, Layout->second,
			Descriptor->second, iCatalogRevision, iGenerationId, pResolved,
			strOutError))
		{
			return nullptr;
		}
		auto& ElementBindings =
			pImplementation->Bindings[Binding.strEffectAssetId];
		if (!ElementBindings.emplace(Binding.strElementId,
				std::move(pResolved)).second)
		{
			strOutError = "Duplicate material binding target: " +
				Binding.strEffectAssetId + "/" + Binding.strElementId;
			return nullptr;
		}
		pImplementation->BindingTargets.push_back({
			Binding.strEffectAssetId, Binding.strElementId });
	}

	strOutError.clear();
	return std::shared_ptr<const CEffectMaterialProgramRegistry>(
		new CEffectMaterialProgramRegistry(iCatalogRevision, iGenerationId,
			std::move(pImplementation)));
}

std::shared_ptr<const Client::EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING>
Client::CEffectMaterialProgramRegistry::Resolve(
	const std::string_view strEffectAssetId,
	const std::string_view strElementId) const noexcept
{
	if (nullptr == m_pImplementation)
		return nullptr;
	const auto Effect = m_pImplementation->Bindings.find(strEffectAssetId);
	if (Effect == m_pImplementation->Bindings.end())
		return nullptr;
	const auto Element = Effect->second.find(strElementId);
	return Element == Effect->second.end() ? nullptr : Element->second;
}

size_t Client::CEffectMaterialProgramRegistry::Get_BindingCount() const noexcept
{
	return nullptr == m_pImplementation ? 0u :
		m_pImplementation->BindingTargets.size();
}

std::span<const Client::EFFECT_MATERIAL_PROGRAM_BINDING_TARGET>
Client::CEffectMaterialProgramRegistry::Get_BindingTargets() const noexcept
{
	return nullptr == m_pImplementation ?
		std::span<const EFFECT_MATERIAL_PROGRAM_BINDING_TARGET>{} :
		std::span<const EFFECT_MATERIAL_PROGRAM_BINDING_TARGET>(
			m_pImplementation->BindingTargets);
}

bool_t Client::CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
	const EFFECT_MATERIAL_EXECUTION_DESC& Left,
	const EFFECT_MATERIAL_EXECUTION_DESC& Right) noexcept
{
	return Left.bEnabled == Right.bEnabled &&
		Left.bFailClosed == Right.bFailClosed &&
		Left.bAuthoringApproximate == Right.bAuthoringApproximate &&
		Left.iVersion == Right.iVersion &&
		Left.eBackend == Right.eBackend &&
		Left.iOpcode == Right.iOpcode &&
		Left.iPassIndex == Right.iPassIndex &&
		Left.strRasterizerState == Right.strRasterizerState &&
		Left.strDepthStencilState == Right.strDepthStencilState &&
		Left.strBlendState == Right.strBlendState &&
		Left.iStencilReference == Right.iStencilReference &&
		Left.iTextureLaneCount == Right.iTextureLaneCount &&
		Left.iTextureMask == Right.iTextureMask &&
		Same_TextureLanes(Left.TextureLanes, Right.TextureLanes) &&
		Same_StandardColorBits(Left.StandardColorV1, Right.StandardColorV1) &&
		Left.iDynamicConsumedMask == Right.iDynamicConsumedMask &&
		Left.iDynamicSuppressedMask == Right.iDynamicSuppressedMask &&
		Left.iParticleColorPolicy == Right.iParticleColorPolicy &&
		Left.iParticleColorConsumedMask == Right.iParticleColorConsumedMask &&
		Left.iParticleColorSuppressedMask ==
			Right.iParticleColorSuppressedMask &&
		Left.iScalarCount == Right.iScalarCount &&
		Left.iVectorCount == Right.iVectorCount &&
		Left.iInputCount == Right.iInputCount &&
		Left.InputConsumedMask == Right.InputConsumedMask &&
		Left.InputSuppressedMask == Right.InputSuppressedMask &&
		Left.VectorComponentConsumedMask ==
			Right.VectorComponentConsumedMask &&
		Left.VectorComponentSuppressedMask ==
			Right.VectorComponentSuppressedMask &&
		Left.iStaticInputCount == Right.iStaticInputCount &&
		Left.iStaticSelectedMask == Right.iStaticSelectedMask &&
		Left.iStaticConsumedMask == Right.iStaticConsumedMask &&
		Left.iStaticSuppressedMask == Right.iStaticSuppressedMask &&
		Left.iRenderInputCount == Right.iRenderInputCount &&
		Left.iRenderConsumedMask == Right.iRenderConsumedMask &&
		Left.iRenderSuppressedMask == Right.iRenderSuppressedMask &&
		Same_Scalars(Left.Scalars, Right.Scalars) &&
		Same_Vectors(Left.Vectors, Right.Vectors) &&
		Same_Vectors(Left.ArtistParameters, Right.ArtistParameters) &&
		Same_Vectors(Left.Colors, Right.Colors);
}
