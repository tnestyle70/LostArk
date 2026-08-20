#include "Effect_VisualProgramCorpus.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Effect_RuntimeAuthority.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <unordered_set>

namespace
{
	using namespace Client;

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		return nullptr != Value && Value->Get_Type() == eType ? Value : nullptr;
	}

	bool_t Has_ExactKeys(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(), [&Value](const auto Key)
		{
			return nullptr != Value.Find(Key);
		});
	}

	bool_t Is_LowerSha256(const std::string_view Value)
	{
		return 64u == Value.size() && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool_t Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 256u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '.' || Character == '-';
			});
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& Out)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().empty())
			return false;
		Out = Value->Get_String();
		return true;
	}

	bool_t Read_StringAllowEmpty(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& Out)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value)
			return false;
		Out = Value->Get_String();
		return true;
	}

	bool_t Read_Sha(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& Out)
	{
		return Read_String(Object, Name, Out) && Is_LowerSha256(Out);
	}

	bool_t Read_Bool(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		bool_t& Out)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name, DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == Value)
			return false;
		Out = Value->Get_Boolean();
		return true;
	}

	template <typename Integer>
	bool_t Read_Unsigned(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		Integer& Out)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() != std::floor(Value->Get_Number()) ||
			Value->Get_Number() < 0.0 ||
			Value->Get_Number() > static_cast<double>(
				(std::numeric_limits<Integer>::max)()))
		{
			return false;
		}
		Out = static_cast<Integer>(Value->Get_Number());
		return true;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		double& Out)
	{
		const DATA_JSON_VALUE* Value = Required(Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || !std::isfinite(Value->Get_Number()))
			return false;
		Out = Value->Get_Number();
		return true;
	}

	bool_t Read_NullableString(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.clear();
			return true;
		}
		if (!Value->Is_String() || Value->Get_String().empty())
			return false;
		Out = Value->Get_String();
		return true;
	}

	bool_t Read_StringArray(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::vector<std::string>& Out)
	{
		const DATA_JSON_VALUE* Values = Required(Object, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Values)
			return false;
		std::vector<std::string> Staged;
		Staged.reserve(Values->Get_Array().size());
		for (const DATA_JSON_VALUE& Value : Values->Get_Array())
		{
			if (!Value.Is_String() || Value.Get_String().empty())
				return false;
			Staged.push_back(Value.Get_String());
		}
		Out = std::move(Staged);
		return true;
	}

	template <size_t Size>
	bool_t Read_U32Array(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::array<uint32_t, Size>& Out)
	{
		const DATA_JSON_VALUE* Values = Required(Object, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Values || Values->Get_Array().size() != Size)
			return false;
		std::array<uint32_t, Size> Staged{};
		for (size_t Index = 0u; Index < Size; ++Index)
		{
			const DATA_JSON_VALUE& Value = Values->Get_Array()[Index];
			if (!Value.Is_Number() || Value.Was_FloatingPointToken() ||
				!std::isfinite(Value.Get_Number()) ||
				Value.Get_Number() != std::floor(Value.Get_Number()) ||
				Value.Get_Number() < 0.0 ||
				Value.Get_Number() > static_cast<double>(UINT32_MAX))
			{
				return false;
			}
			Staged[Index] = static_cast<uint32_t>(Value.Get_Number());
		}
		Out = Staged;
		return true;
	}

	template <size_t Size>
	bool_t Read_NumberArray(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::array<double, Size>& Out)
	{
		const DATA_JSON_VALUE* Values = Required(Object, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Values || Values->Get_Array().size() != Size)
			return false;
		std::array<double, Size> Staged{};
		for (size_t Index = 0u; Index < Size; ++Index)
		{
			const DATA_JSON_VALUE& Value = Values->Get_Array()[Index];
			if (!Value.Is_Number() || !std::isfinite(Value.Get_Number()))
				return false;
			Staged[Index] = Value.Get_Number();
		}
		Out = Staged;
		return true;
	}

	bool_t Verify_Seal(
		const DATA_JSON_VALUE& Value,
		const std::string_view Field,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Claimed = Required(Value, Field, DATA_JSON_TYPE::STRING);
		if (nullptr == Claimed || !Is_LowerSha256(Claimed->Get_String()))
		{
			strOutError = "Visual-program seal is missing or invalid: " +
				std::string(Field);
			return false;
		}
		DATA_JSON_VALUE::OBJECT Unsigned = Value.Get_Object();
		Unsigned.erase(std::string(Field));
		const DATA_JSON_VALUE UnsignedValue =
			DATA_JSON_VALUE::Object(std::move(Unsigned));
		const std::string Actual = CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(UnsignedValue));
		if (Actual != Claimed->Get_String())
		{
			strOutError = "Visual-program seal is stale: " + std::string(Field);
			return false;
		}
		return true;
	}

	EFFECT_VISUAL_PROGRAM_FAMILY Parse_Family(const std::string_view Token)
	{
		if (Token == "MESH_PARTICLE") return EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE;
		if (Token == "SPRITE_PARTICLE") return EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE;
		if (Token == "DECAL_PARTICLE") return EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE;
		if (Token == "CASCADE_RIBBON") return EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON;
		if (Token == "ANIMATION_TRAIL") return EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL;
		if (Token == "LIGHT_PARTICLE") return EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE;
		if (Token == "SCREEN_POST") return EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST;
		return EFFECT_VISUAL_PROGRAM_FAMILY::END;
	}

	EFFECT_VISUAL_PROGRAM_VALUE_VARIANT Parse_ValueVariant(
		const std::string_view Token)
	{
		if (Token == "F64") return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::F64;
		if (Token == "F64X4") return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::F64X4;
		if (Token == "TEXTURE_ID") return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::TEXTURE_ID;
		if (Token == "BOOL") return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::BOOLEAN;
		if (Token == "ENUM_STRING") return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::ENUM_STRING;
		return EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::END;
	}

	EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION Parse_ValueDisposition(
		const std::string_view Token)
	{
		if (Token == "CONSUMED") return EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::CONSUMED;
		if (Token == "SUPPRESSED") return EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::SUPPRESSED;
		return EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::END;
	}

	bool_t Parse_Selector(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_SELECTOR& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, { "effectAssetId", "occurrenceId" }) ||
			!Read_String(Value, "effectAssetId", Out.strEffectAssetId) ||
			!Read_String(Value, "occurrenceId", Out.strOccurrenceId) ||
			!Is_StableId(Out.strEffectAssetId) || !Is_StableId(Out.strOccurrenceId))
		{
			strOutError =
				"Visual-program selector must contain only effectAssetId and occurrenceId.";
			return false;
		}
		Out.strSelectorSha256 = CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value));
		return true;
	}

	bool_t Parse_SourceIdentity(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_SOURCE_IDENTITY& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"sourceRowId", "sourceRowSha256", "sourceRecordId",
			"sourceRecordSha256", "sourceRecipeSha256",
			"moduleClosureSha256", "moduleCount" }) ||
			!Read_String(Value, "sourceRowId", Out.strSourceRowId) ||
			!Read_Sha(Value, "sourceRowSha256", Out.strSourceRowSha256) ||
			!Read_String(Value, "sourceRecordId", Out.strSourceRecordId) ||
			!Read_Sha(Value, "sourceRecordSha256", Out.strSourceRecordSha256) ||
			!Read_Sha(Value, "sourceRecipeSha256", Out.strSourceRecipeSha256) ||
			!Read_Sha(Value, "moduleClosureSha256", Out.strModuleClosureSha256) ||
			!Read_Unsigned(Value, "moduleCount", Out.iModuleCount))
		{
			strOutError = "Visual-program source identity is invalid.";
			return false;
		}
		return true;
	}

	bool_t Parse_TargetIdentity(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY>& Out,
		std::string& strOutError)
	{
		if (Value.Is_Null())
		{
			Out.reset();
			return true;
		}
		EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY Staged;
		if (!Has_ExactKeys(Value, {
			"targetElementId", "targetRecordSha256", "targetPayloadRawSha256" }) ||
			!Read_String(Value, "targetElementId", Staged.strTargetElementId) ||
			!Read_Sha(Value, "targetRecordSha256", Staged.strTargetRecordSha256) ||
			!Read_Sha(Value, "targetPayloadRawSha256", Staged.strTargetPayloadRawSha256))
		{
			strOutError = "Visual-program target identity is invalid.";
			return false;
		}
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_Resource(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"role", "slotId", "assetId", "resolutionStatus", "rawSha256",
			"byteCount", "shaderRegister", "sourceChannel" }) ||
			!Read_String(Value, "role", Out.strRole) ||
			!Read_String(Value, "slotId", Out.strSlotId) ||
			!Read_String(Value, "assetId", Out.strAssetId) ||
			!Read_String(Value, "resolutionStatus", Out.strResolutionStatus) ||
			!Read_NullableString(Value, "rawSha256", Out.strRawSha256) ||
			(!Out.strRawSha256.empty() && !Is_LowerSha256(Out.strRawSha256)) ||
			!Read_NullableString(Value, "shaderRegister", Out.strShaderRegister) ||
			!Read_NullableString(Value, "sourceChannel", Out.strSourceChannel))
		{
			strOutError = "Visual-program resource packet row is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* ByteCount = Value.Find("byteCount");
		if (nullptr == ByteCount)
			return false;
		if (ByteCount->Is_Null())
			Out.iByteCount = 0u;
		else if (!Read_Unsigned(Value, "byteCount", Out.iByteCount) || 0u == Out.iByteCount)
		{
			strOutError = "Visual-program resource byte count is invalid.";
			return false;
		}
		return true;
	}

	bool_t Read_NullableNumber(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<double>& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.reset();
			return true;
		}
		if (!Value->Is_Number() || !std::isfinite(Value->Get_Number()))
			return false;
		Out = Value->Get_Number();
		return true;
	}

	bool_t Read_NullableBool(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<bool_t>& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.reset();
			return true;
		}
		if (!Value->Is_Boolean())
			return false;
		Out = Value->Get_Boolean();
		return true;
	}

	bool_t Read_NullableIndex(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<uint32_t>& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.reset();
			return true;
		}
		uint32_t Staged = 0u;
		if (!Read_Unsigned(Object, Name, Staged))
			return false;
		Out = Staged;
		return true;
	}

	bool_t Read_NullableVector4(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<std::array<double, 4u>>& Out)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			Out.reset();
			return true;
		}
		if (!Value->Is_Array() || 4u != Value->Get_Array().size())
			return false;
		std::array<double, 4u> Staged{};
		for (size_t Index = 0u; Index < Staged.size(); ++Index)
		{
			const DATA_JSON_VALUE& Component = Value->Get_Array()[Index];
			if (!Component.Is_Number() || !std::isfinite(Component.Get_Number()))
				return false;
			Staged[Index] = Component.Get_Number();
		}
		Out = Staged;
		return true;
	}

	bool_t Parse_LocalDecalInput(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_INPUT& Out,
		std::string& strOutError)
	{
		std::string Variant;
		std::string Disposition;
		if (!Has_ExactKeys(Value, {
			"inputId", "normalizedParameterName", "rowSha256", "valueVariant",
			"disposition", "scalarValue", "vectorValue", "textureId",
			"packedScalarIndex", "packedVectorIndex" }) ||
			!Read_String(Value, "inputId", Out.strInputId) ||
			!Read_String(Value, "normalizedParameterName", Out.strNormalizedParameterName) ||
			!Read_Sha(Value, "rowSha256", Out.strRowSha256) ||
			!Read_String(Value, "valueVariant", Variant) ||
			!Read_String(Value, "disposition", Disposition) ||
			!Read_NullableNumber(Value, "scalarValue", Out.fScalarValue) ||
			!Read_NullableVector4(Value, "vectorValue", Out.vVectorValue) ||
			!Read_NullableString(Value, "textureId", Out.strTextureId) ||
			!Read_NullableIndex(Value, "packedScalarIndex", Out.iPackedScalarIndex) ||
			!Read_NullableIndex(Value, "packedVectorIndex", Out.iPackedVectorIndex))
		{
			strOutError = "LocalDecal input disposition is invalid.";
			return false;
		}
		Out.eVariant = Parse_ValueVariant(Variant);
		Out.eDisposition = Parse_ValueDisposition(Disposition);
		if (Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::END ||
			Out.eDisposition == EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::END)
		{
			strOutError = "LocalDecal input variant/disposition is unknown.";
			return false;
		}
		const bool_t bScalar =
			Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::F64;
		const bool_t bVector =
			Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::F64X4;
		const bool_t bTexture =
			Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::TEXTURE_ID;
		if ((bScalar != Out.fScalarValue.has_value()) ||
			(bScalar != Out.iPackedScalarIndex.has_value()) ||
			(bVector != Out.vVectorValue.has_value()) ||
			(bVector != Out.iPackedVectorIndex.has_value()) ||
			(bTexture != !Out.strTextureId.empty()) ||
			(!bScalar && !bVector && !bTexture))
		{
			strOutError = "LocalDecal input payload does not match its variant.";
			return false;
		}
		return true;
	}

	bool_t Parse_LocalDecalStatic(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_STATIC& Out,
		std::string& strOutError)
	{
		std::string Disposition;
		if (!Has_ExactKeys(Value, {
			"bindingId", "normalizedParameterName", "rowSha256", "sourceValue",
			"selectedValue", "policyRowId", "disposition" }) ||
			!Read_String(Value, "bindingId", Out.strBindingId) ||
			!Read_String(Value, "normalizedParameterName", Out.strNormalizedParameterName) ||
			!Read_Sha(Value, "rowSha256", Out.strRowSha256) ||
			!Read_Bool(Value, "sourceValue", Out.bSourceValue) ||
			!Read_Bool(Value, "selectedValue", Out.bSelectedValue) ||
			!Read_String(Value, "policyRowId", Out.strPolicyRowId) ||
			!Read_String(Value, "disposition", Disposition) ||
			Disposition != "CONSUMED")
		{
			strOutError = "LocalDecal static disposition is invalid.";
			return false;
		}
		return true;
	}

	bool_t Parse_LocalDecalRender(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_RENDER& Out,
		std::string& strOutError)
	{
		std::string Variant;
		std::string Disposition;
		if (!Has_ExactKeys(Value, {
			"bindingId", "fieldName", "rowSha256", "valueVariant", "boolValue",
			"enumValue", "f64Value", "sourceStatus", "sourceFidelity",
			"disposition" }) ||
			!Read_String(Value, "bindingId", Out.strBindingId) ||
			!Read_String(Value, "fieldName", Out.strFieldName) ||
			!Read_Sha(Value, "rowSha256", Out.strRowSha256) ||
			!Read_String(Value, "valueVariant", Variant) ||
			!Read_NullableBool(Value, "boolValue", Out.bValue) ||
			!Read_NullableString(Value, "enumValue", Out.strEnumValue) ||
			!Read_NullableNumber(Value, "f64Value", Out.fValue) ||
			!Read_String(Value, "sourceStatus", Out.strSourceStatus) ||
			!Read_String(Value, "sourceFidelity", Out.strSourceFidelity) ||
			!Read_String(Value, "disposition", Disposition))
		{
			strOutError = "LocalDecal render disposition is invalid.";
			return false;
		}
		Out.eVariant = Parse_ValueVariant(Variant);
		Out.eDisposition = Parse_ValueDisposition(Disposition);
		const bool_t bBool = Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::BOOLEAN;
		const bool_t bEnum = Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::ENUM_STRING;
		const bool_t bNumber = Out.eVariant == EFFECT_VISUAL_PROGRAM_VALUE_VARIANT::F64;
		if (Out.eDisposition == EFFECT_VISUAL_PROGRAM_VALUE_DISPOSITION::END ||
			(bBool != Out.bValue.has_value()) || (bEnum != !Out.strEnumValue.empty()) ||
			(bNumber != Out.fValue.has_value()) || (!bBool && !bEnum && !bNumber))
		{
			strOutError = "LocalDecal render payload does not match its variant.";
			return false;
		}
		return true;
	}

	bool_t Parse_LocalDecalSrv(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"role", "assetId", "rawSha256", "byteCount", "shaderRegister",
			"sourceChannel", "runtimeSamplerRegister", "sourceSamplerEvidence",
			"samplerPolicy", "linearFormat", "srgb", "width", "height",
			"mipCount", "arraySize" }) ||
			!Read_String(Value, "role", Out.strRole) ||
			!Read_String(Value, "assetId", Out.strAssetId) ||
			!Read_Sha(Value, "rawSha256", Out.strRawSha256) ||
			!Read_Unsigned(Value, "byteCount", Out.iByteCount) || 0u == Out.iByteCount ||
			!Read_String(Value, "shaderRegister", Out.strShaderRegister) ||
			!Read_String(Value, "sourceChannel", Out.strSourceChannel) ||
			!Read_String(Value, "runtimeSamplerRegister", Out.strRuntimeSamplerRegister) ||
			!Read_NullableString(Value, "sourceSamplerEvidence", Out.strSourceSamplerEvidence) ||
			!Read_String(Value, "samplerPolicy", Out.strSamplerPolicy) ||
			!Read_String(Value, "linearFormat", Out.strLinearFormat) ||
			!Read_Bool(Value, "srgb", Out.bSrgb) ||
			!Read_Unsigned(Value, "width", Out.iWidth) || 0u == Out.iWidth ||
			!Read_Unsigned(Value, "height", Out.iHeight) || 0u == Out.iHeight ||
			!Read_Unsigned(Value, "mipCount", Out.iMipCount) || 0u == Out.iMipCount ||
			!Read_Unsigned(Value, "arraySize", Out.iArraySize) || 0u == Out.iArraySize)
		{
			strOutError = "LocalDecal SRV binding is invalid.";
			return false;
		}
		return true;
	}

	bool_t Parse_LocalDecalPacket(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET>& Out,
		std::string& strOutError)
	{
		if (Value.Is_Null())
		{
			Out.reset();
			return true;
		}
		if (!Has_ExactKeys(Value, {
			"packetVersion", "adapterId", "boundedSemanticReplay", "nativeExecution",
			"nativeVertexFactoryAdmitted", "nativeMrtAdmitted", "runtimeCarrier",
			"nativeVertexFactoryCandidate", "nativeVertexShaderSha256",
			"nativePixelShaderSha256", "renderProfile", "passIndex", "pipelineState",
			"opcode", "textureLaneCount", "textureMask", "dynamicConsumedMask",
			"dynamicSuppressedMask", "particleColorPolicy",
			"particleColorConsumedMask", "particleColorSuppressedMask",
			"inputConsumedMask", "inputSuppressedMask",
			"vectorComponentConsumedMask", "vectorComponentSuppressedMask",
			"staticSelectedMask", "staticConsumedMask", "staticSuppressedMask",
			"renderConsumedMask", "renderSuppressedMask", "inputDispositions",
			"staticDispositions", "renderDispositions", "packedScalars",
			"packedVectors", "srvs", "preservedLimitations", "packetSha256" }) ||
			!Verify_Seal(Value, "packetSha256", strOutError))
		{
			if (strOutError.empty())
				strOutError = "LocalDecal packet shape is invalid.";
			return false;
		}

		EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET Staged;
		const DATA_JSON_VALUE* Pipeline = Required(Value, "pipelineState", DATA_JSON_TYPE::OBJECT);
		if (!Read_Unsigned(Value, "packetVersion", Staged.iPacketVersion) ||
			!Read_String(Value, "adapterId", Staged.strAdapterId) ||
			!Read_Bool(Value, "boundedSemanticReplay", Staged.bBoundedSemanticReplay) ||
			!Read_Bool(Value, "nativeExecution", Staged.bNativeExecution) ||
			!Read_Bool(Value, "nativeVertexFactoryAdmitted", Staged.bNativeVertexFactoryAdmitted) ||
			!Read_Bool(Value, "nativeMrtAdmitted", Staged.bNativeMrtAdmitted) ||
			!Read_String(Value, "runtimeCarrier", Staged.strRuntimeCarrier) ||
			!Read_String(Value, "nativeVertexFactoryCandidate", Staged.strNativeVertexFactoryCandidate) ||
			!Read_Sha(Value, "nativeVertexShaderSha256", Staged.strNativeVertexShaderSha256) ||
			!Read_Sha(Value, "nativePixelShaderSha256", Staged.strNativePixelShaderSha256) ||
			!Read_String(Value, "renderProfile", Staged.strRenderProfile) ||
			!Read_Unsigned(Value, "passIndex", Staged.iPassIndex) ||
			nullptr == Pipeline || !Has_ExactKeys(*Pipeline, {
				"rasterizer", "depthStencil", "blend", "stencilReference" }) ||
			!Read_String(*Pipeline, "rasterizer", Staged.strRasterizerState) ||
			!Read_String(*Pipeline, "depthStencil", Staged.strDepthStencilState) ||
			!Read_String(*Pipeline, "blend", Staged.strBlendState) ||
			!Read_Unsigned(*Pipeline, "stencilReference", Staged.iStencilReference) ||
			!Read_Unsigned(Value, "opcode", Staged.iOpcode) ||
			!Read_Unsigned(Value, "textureLaneCount", Staged.iTextureLaneCount) ||
			!Read_Unsigned(Value, "textureMask", Staged.iTextureMask) ||
			!Read_Unsigned(Value, "dynamicConsumedMask", Staged.iDynamicConsumedMask) ||
			!Read_Unsigned(Value, "dynamicSuppressedMask", Staged.iDynamicSuppressedMask) ||
			!Read_Unsigned(Value, "particleColorPolicy", Staged.iParticleColorPolicy) ||
			!Read_Unsigned(Value, "particleColorConsumedMask", Staged.iParticleColorConsumedMask) ||
			!Read_Unsigned(Value, "particleColorSuppressedMask", Staged.iParticleColorSuppressedMask) ||
			!Read_U32Array(Value, "inputConsumedMask", Staged.InputConsumedMask) ||
			!Read_U32Array(Value, "inputSuppressedMask", Staged.InputSuppressedMask) ||
			!Read_U32Array(Value, "vectorComponentConsumedMask", Staged.VectorComponentConsumedMask) ||
			!Read_U32Array(Value, "vectorComponentSuppressedMask", Staged.VectorComponentSuppressedMask) ||
			!Read_Unsigned(Value, "staticSelectedMask", Staged.iStaticSelectedMask) ||
			!Read_Unsigned(Value, "staticConsumedMask", Staged.iStaticConsumedMask) ||
			!Read_Unsigned(Value, "staticSuppressedMask", Staged.iStaticSuppressedMask) ||
			!Read_Unsigned(Value, "renderConsumedMask", Staged.iRenderConsumedMask) ||
			!Read_Unsigned(Value, "renderSuppressedMask", Staged.iRenderSuppressedMask) ||
			!Read_StringArray(Value, "preservedLimitations", Staged.PreservedLimitations) ||
			!Read_Sha(Value, "packetSha256", Staged.strPacketSha256))
		{
			strOutError = "LocalDecal packet header/state is invalid.";
			return false;
		}

		const DATA_JSON_VALUE* Inputs = Required(Value, "inputDispositions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Statics = Required(Value, "staticDispositions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Renders = Required(Value, "renderDispositions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Scalars = Required(Value, "packedScalars", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Vectors = Required(Value, "packedVectors", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Srvs = Required(Value, "srvs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Inputs || 33u != Inputs->Get_Array().size() ||
			nullptr == Statics || 18u != Statics->Get_Array().size() ||
			nullptr == Renders || 6u != Renders->Get_Array().size() ||
			nullptr == Scalars || 22u != Scalars->Get_Array().size() ||
			nullptr == Vectors || 3u != Vectors->Get_Array().size() ||
			nullptr == Srvs || 6u != Srvs->Get_Array().size())
		{
			strOutError = "LocalDecal packet denominator changed.";
			return false;
		}

		for (const DATA_JSON_VALUE& Input : Inputs->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_INPUT Parsed;
			if (!Parse_LocalDecalInput(Input, Parsed, strOutError)) return false;
			Staged.Inputs.push_back(std::move(Parsed));
		}
		for (const DATA_JSON_VALUE& Static : Statics->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_STATIC Parsed;
			if (!Parse_LocalDecalStatic(Static, Parsed, strOutError)) return false;
			Staged.StaticBindings.push_back(std::move(Parsed));
		}
		for (const DATA_JSON_VALUE& Render : Renders->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_RENDER Parsed;
			if (!Parse_LocalDecalRender(Render, Parsed, strOutError)) return false;
			Staged.RenderBindings.push_back(std::move(Parsed));
		}

		for (size_t Index = 0u; Index < Scalars->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Packed = Scalars->Get_Array()[Index];
			std::string InputId;
			double Number = 0.0;
			if (!Has_ExactKeys(Packed, { "inputId", "value" }) ||
				!Read_String(Packed, "inputId", InputId) ||
				!Read_Number(Packed, "value", Number))
			{
				strOutError = "LocalDecal packed scalar is invalid.";
				return false;
			}
			const auto Found = std::find_if(Staged.Inputs.begin(), Staged.Inputs.end(),
				[&InputId, Index](const auto& Input)
				{
					return Input.strInputId == InputId && Input.iPackedScalarIndex == Index;
				});
			if (Found == Staged.Inputs.end() || !Found->fScalarValue.has_value() ||
				Found->fScalarValue.value() != Number)
			{
				strOutError = "LocalDecal packed scalar/input join is stale.";
				return false;
			}
			Staged.PackedScalars[Index] = Number;
		}
		for (size_t Index = 0u; Index < Vectors->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Packed = Vectors->Get_Array()[Index];
			std::string InputId;
			std::optional<std::array<double, 4u>> Vector;
			if (!Has_ExactKeys(Packed, { "inputId", "value" }) ||
				!Read_String(Packed, "inputId", InputId) ||
				!Read_NullableVector4(Packed, "value", Vector) || !Vector.has_value())
			{
				strOutError = "LocalDecal packed vector is invalid.";
				return false;
			}
			const auto Found = std::find_if(Staged.Inputs.begin(), Staged.Inputs.end(),
				[&InputId, Index](const auto& Input)
				{
					return Input.strInputId == InputId && Input.iPackedVectorIndex == Index;
				});
			if (Found == Staged.Inputs.end() || Found->vVectorValue != Vector)
			{
				strOutError = "LocalDecal packed vector/input join is stale.";
				return false;
			}
			Staged.PackedVectors[Index] = Vector.value();
		}
		for (size_t Index = 0u; Index < Srvs->Get_Array().size(); ++Index)
		{
			if (!Parse_LocalDecalSrv(Srvs->Get_Array()[Index], Staged.Srvs[Index], strOutError))
				return false;
		}

		static constexpr std::array<std::string_view, 6u> Roles = {
			"HEIGHT", "DIFFUSE", "DISSOLVE", "NORMAL", "SPECULAR", "EMISSIVE" };
		static constexpr std::array<std::string_view, 6u> Channels = {
			"B", "RGBA", "G", "RG", "RGB", "R" };
		static constexpr std::array<std::string_view, 6u> Formats = {
			"BC1_UNORM", "BC3_UNORM", "BC1_UNORM", "BC5_UNORM", "BC1_UNORM", "BC1_UNORM" };
		static constexpr std::array<bool_t, 6u> Srgb = {
			false, true, false, false, true, true };
		for (size_t Index = 0u; Index < Staged.Srvs.size(); ++Index)
		{
			const auto& Srv = Staged.Srvs[Index];
			if (Srv.strRole != Roles[Index] || Srv.strShaderRegister != "t" + std::to_string(Index) ||
				Srv.strRuntimeSamplerRegister != "s" + std::to_string(5u + Index) ||
				Srv.strSourceChannel != Channels[Index] || Srv.strLinearFormat != Formats[Index] ||
				Srv.bSrgb != Srgb[Index] || Srv.strSamplerPolicy != "LINEAR_CLAMP_UVW_BOUNDED_V1" ||
				(Index == 0u ? Srv.strSourceSamplerEvidence != "s0" :
				 Index == 2u ? Srv.strSourceSamplerEvidence != "s5" :
				 !Srv.strSourceSamplerEvidence.empty()))
			{
				strOutError = "LocalDecal SRV role/register/channel/sampler contract changed.";
				return false;
			}
		}

		if (1u != Staged.iPacketVersion ||
			Staged.strAdapterId != "local-decal-rt0-bounded-v1" ||
			!Staged.bBoundedSemanticReplay || Staged.bNativeExecution ||
			Staged.bNativeVertexFactoryAdmitted || Staged.bNativeMrtAdmitted ||
			Staged.strRuntimeCarrier != "EFFECT_TYPED_DECAL_PROJECTOR_RECT_V1" ||
			Staged.strRenderProfile != "ALPHA_ONE_SIDED_DEPTH_READ" ||
			3u != Staged.iPassIndex || Staged.strRasterizerState != "RS_Default" ||
			Staged.strDepthStencilState != "DSS_ReadOnly" ||
			Staged.strBlendState != "BS_EffectAlpha" || 0u != Staged.iStencilReference ||
			14u != Staged.iOpcode || 6u != Staged.iTextureLaneCount || 0x3fu != Staged.iTextureMask ||
			0u != Staged.iDynamicConsumedMask || 0x0fu != Staged.iDynamicSuppressedMask ||
			0u != Staged.iParticleColorPolicy || 0u != Staged.iParticleColorConsumedMask ||
			0u != Staged.iParticleColorSuppressedMask ||
			Staged.InputConsumedMask != std::array<uint32_t, 2u>{ 0x820ec1ffu, 1u } ||
			Staged.InputSuppressedMask != std::array<uint32_t, 2u>{ 0x7df13e00u, 0u } ||
			Staged.VectorComponentConsumedMask != std::array<uint32_t, 3u>{ 0x0fu, 0x0fu, 0u } ||
			Staged.VectorComponentSuppressedMask != std::array<uint32_t, 3u>{ 0u, 0u, 0x0fu } ||
			0x3fffbu != Staged.iStaticSelectedMask || 0x3ffffu != Staged.iStaticConsumedMask ||
			0u != Staged.iStaticSuppressedMask || 0x03u != Staged.iRenderConsumedMask ||
			0x3cu != Staged.iRenderSuppressedMask)
		{
			strOutError = "LocalDecal bounded adapter admission constants changed.";
			return false;
		}
		Out = std::move(Staged);
		return true;
	}

	struct ADAPTER_RULE final
	{
		EFFECT_VISUAL_PROGRAM_FAMILY eFamily = EFFECT_VISUAL_PROGRAM_FAMILY::END;
		std::set<std::string, std::less<>> PacketLayouts;
		std::set<std::string, std::less<>> ResourceRoles;
	};

	using ADAPTER_RULES = std::map<std::string, ADAPTER_RULE, std::less<>>;

	bool_t Parse_TrailTiming(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_TRAIL_TIMING& Out)
	{
		return Has_ExactKeys(Value, {
			"startDelaySeconds", "lifeTimeSeconds", "afterImageSeconds",
			"dissolveStartNormalized" }) &&
			Read_Number(Value, "startDelaySeconds", Out.fStartDelaySeconds) &&
			Read_Number(Value, "lifeTimeSeconds", Out.fLifeTimeSeconds) &&
			Read_Number(Value, "afterImageSeconds", Out.fAfterImageSeconds) &&
			Read_Number(Value, "dissolveStartNormalized",
				Out.fDissolveStartNormalized) &&
			Out.fStartDelaySeconds >= 0.0 && Out.fLifeTimeSeconds > 0.0 &&
			Out.fAfterImageSeconds >= 0.0 &&
			Out.fDissolveStartNormalized >= 0.0 &&
			Out.fDissolveStartNormalized <= 1.0;
	}

	bool_t Parse_TrailAttachment(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT& Out,
		const bool_t bBakedEdgePacket,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Transform = Required(
			Value, "socketLocalTransform", DATA_JSON_TYPE::OBJECT);
		const bool_t bShapeValid = bBakedEdgePacket ?
			Has_ExactKeys(Value, {
				"enabled", "follow", "sourceAnchorSlotId", "runtimeAnchorSlotId",
				"runtimeBoneName", "snapshotRootSourceBasisYawDegrees",
				"socketLocalTransform" }) :
			Has_ExactKeys(Value, {
				"enabled", "follow", "sourceAnchorSlotId", "runtimeAnchorSlotId",
				"runtimeBoneName", "socketLocalTransform" });
		EFFECT_VISUAL_PROGRAM_TRAIL_ATTACHMENT Staged;
		if (!bShapeValid || nullptr == Transform ||
			!Has_ExactKeys(*Transform, {
				"position", "rotationDegrees", "scale" }) ||
			!Read_Bool(Value, "enabled", Staged.bEnabled) ||
			!Read_Bool(Value, "follow", Staged.bFollow) ||
			!Read_StringAllowEmpty(Value, "sourceAnchorSlotId",
				Staged.strSourceAnchorSlotId) ||
			!Read_StringAllowEmpty(Value, "runtimeAnchorSlotId",
				Staged.strRuntimeAnchorSlotId) ||
			!Read_StringAllowEmpty(Value, "runtimeBoneName",
				Staged.strRuntimeBoneName) ||
			(bBakedEdgePacket &&
				!Read_Number(Value, "snapshotRootSourceBasisYawDegrees",
					Staged.fSnapshotRootSourceBasisYawDegrees)) ||
			!Read_NumberArray(*Transform, "position", Staged.vPosition) ||
			!Read_NumberArray(*Transform, "rotationDegrees",
				Staged.vRotationDegrees) ||
			!Read_NumberArray(*Transform, "scale", Staged.vScale) ||
			!std::all_of(Staged.vScale.begin(), Staged.vScale.end(),
				[](const double Component) { return Component > 0.0; }))
		{
			strOutError = "Trail attachment shape or field type is invalid.";
			return false;
		}

		if (bBakedEdgePacket)
		{
			const bool_t bIdentityTransform =
				Staged.vPosition == std::array<double, 3u>{ 0.0, 0.0, 0.0 } &&
				Staged.vRotationDegrees ==
					std::array<double, 3u>{ 0.0, 0.0, 0.0 } &&
				Staged.vScale == std::array<double, 3u>{ 1.0, 1.0, 1.0 };
			if (Staged.bEnabled || Staged.bFollow ||
				!Staged.strSourceAnchorSlotId.empty() ||
				!Staged.strRuntimeAnchorSlotId.empty() ||
				!Staged.strRuntimeBoneName.empty() ||
				0.0 != Staged.fSnapshotRootSourceBasisYawDegrees ||
				!bIdentityTransform)
			{
				strOutError =
					"Baked-edge trail attachment must be detached with an identity transform.";
				return false;
			}
		}
		else if (Staged.strSourceAnchorSlotId.empty() ||
			Staged.strRuntimeAnchorSlotId.empty() ||
			Staged.strRuntimeBoneName.empty())
		{
			strOutError = "Trail attachment identity is empty.";
			return false;
		}

		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_TrailGeometry(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_TRAIL_GEOMETRY& Out)
	{
		return Has_ExactKeys(Value, {
			"maxPoints", "pointLifeTimeSeconds", "sampleIntervalSeconds",
			"minimumDistance", "startWidth", "endWidth", "faceCamera" }) &&
			Read_Unsigned(Value, "maxPoints", Out.iMaxPoints) &&
			Read_Number(Value, "pointLifeTimeSeconds",
				Out.fPointLifeTimeSeconds) &&
			Read_Number(Value, "sampleIntervalSeconds",
				Out.fSampleIntervalSeconds) &&
			Read_Number(Value, "minimumDistance", Out.fMinimumDistance) &&
			Read_Number(Value, "startWidth", Out.fStartWidth) &&
			Read_Number(Value, "endWidth", Out.fEndWidth) &&
			Read_Bool(Value, "faceCamera", Out.bFaceCamera) &&
			Out.iMaxPoints >= 2u && Out.fPointLifeTimeSeconds > 0.0 &&
			Out.fSampleIntervalSeconds > 0.0 && Out.fMinimumDistance >= 0.0 &&
			Out.fStartWidth > 0.0 && Out.fEndWidth >= 0.0;
	}

	bool_t Parse_CascadeRibbonPacket(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET>& Out,
		std::string& strOutError)
	{
		if (Value.Is_Null()) { Out.reset(); return true; }
		if (!Has_ExactKeys(Value, {
			"packetVersion", "adapterId", "boundedSemanticReplay",
			"nativeExecution", "runtimeCarrier", "typeDataStableId",
			"typeDataClassName", "typeDataObjectPath", "typeDataModuleSha256",
			"resolvedRendererShape", "tilingDistance",
			"distanceTessellationStepSize", "tangentTessellationScalar",
			"lodValidity", "operationalMaxPoints", "targetTiming",
			"attachment", "trail", "sourceRecipeSha256",
			"moduleClosureSha256", "moduleCount", "preservedLimitations",
			"packetSha256" }) || !Verify_Seal(Value, "packetSha256", strOutError))
			return false;
		const DATA_JSON_VALUE* Timing = Required(
			Value, "targetTiming", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Attachment = Required(
			Value, "attachment", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Trail = Required(
			Value, "trail", DATA_JSON_TYPE::OBJECT);
		EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET Staged;
		if (nullptr == Timing || nullptr == Attachment || nullptr == Trail ||
			!Read_Unsigned(Value, "packetVersion", Staged.iPacketVersion) ||
			!Read_String(Value, "adapterId", Staged.strAdapterId) ||
			!Read_Bool(Value, "boundedSemanticReplay",
				Staged.bBoundedSemanticReplay) ||
			!Read_Bool(Value, "nativeExecution", Staged.bNativeExecution) ||
			!Read_String(Value, "runtimeCarrier", Staged.strRuntimeCarrier) ||
			!Read_String(Value, "typeDataStableId", Staged.strTypeDataStableId) ||
			!Read_String(Value, "typeDataClassName", Staged.strTypeDataClassName) ||
			!Read_String(Value, "typeDataObjectPath", Staged.strTypeDataObjectPath) ||
			!Read_Sha(Value, "typeDataModuleSha256",
				Staged.strTypeDataModuleSha256) ||
			!Read_String(Value, "resolvedRendererShape",
				Staged.strResolvedRendererShape) ||
			!Read_Number(Value, "tilingDistance", Staged.fTilingDistance) ||
			!Read_Number(Value, "distanceTessellationStepSize",
				Staged.fDistanceTessellationStepSize) ||
			!Read_Number(Value, "tangentTessellationScalar",
				Staged.fTangentTessellationScalar) ||
			!Read_Number(Value, "lodValidity", Staged.fLodValidity) ||
			!Read_Unsigned(Value, "operationalMaxPoints",
				Staged.iOperationalMaxPoints) ||
			!Parse_TrailTiming(*Timing, Staged.Timing) ||
			!Parse_TrailAttachment(*Attachment, Staged.Attachment, false,
				strOutError) ||
			!Parse_TrailGeometry(*Trail, Staged.Trail) ||
			!Read_Sha(Value, "sourceRecipeSha256",
				Staged.strSourceRecipeSha256) ||
			!Read_Sha(Value, "moduleClosureSha256",
				Staged.strModuleClosureSha256) ||
			!Read_Unsigned(Value, "moduleCount", Staged.iModuleCount) ||
			!Read_StringArray(Value, "preservedLimitations",
				Staged.PreservedLimitations) ||
			!Read_Sha(Value, "packetSha256", Staged.strPacketSha256) ||
			1u != Staged.iPacketVersion ||
			Staged.strAdapterId != "cascade-ribbon-document-v12" ||
			!Staged.bBoundedSemanticReplay || Staged.bNativeExecution ||
			Staged.strRuntimeCarrier != "EFFECT_TYPED_CASCADE_RIBBON_V1" ||
			Staged.strTypeDataClassName != "particlemoduletypedataribbon" ||
			Staged.strResolvedRendererShape != "ribbon" ||
			Staged.iOperationalMaxPoints < 2u || 0u == Staged.iModuleCount ||
			Staged.PreservedLimitations.empty())
			return false;
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_AnimationTrailPacket(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET>& Out,
		std::string& strOutError)
	{
		if (Value.Is_Null()) { Out.reset(); return true; }
		uint32_t iPacketVersion = 0u;
		if (!Read_Unsigned(Value, "packetVersion", iPacketVersion)) return false;
		const bool_t bBakedEdgePacket = 2u == iPacketVersion;
		const bool_t bShapeValid = bBakedEdgePacket ?
			Has_ExactKeys(Value, {
				"packetVersion", "adapterId", "boundedSemanticReplay",
				"nativeExecution", "runtimeCarrier", "sourceNotifyType",
				"sourceEventId", "sourceEventRecordSha256", "sourceAsset", "clip",
				"localTimeSeconds", "globalTimeSeconds", "durationSeconds",
				"targetElementId", "targetTiming", "attachment", "trail",
				"historyId", "historySha256", "playbackClampSeconds",
				"coordinateBasis", "preservedLimitations", "packetSha256" }) :
			Has_ExactKeys(Value, {
				"packetVersion", "adapterId", "boundedSemanticReplay",
				"nativeExecution", "runtimeCarrier", "sourceNotifyType",
				"sourceEventId", "sourceEventRecordSha256", "sourceAsset", "clip",
				"localTimeSeconds", "globalTimeSeconds", "durationSeconds",
				"targetElementId", "targetTiming", "attachment", "trail",
				"preservedLimitations", "packetSha256" });
		if (!bShapeValid || (1u != iPacketVersion && !bBakedEdgePacket) ||
			!Verify_Seal(Value, "packetSha256", strOutError))
			return false;
		const DATA_JSON_VALUE* Timing = Required(
			Value, "targetTiming", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Attachment = Required(
			Value, "attachment", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Trail = Required(
			Value, "trail", DATA_JSON_TYPE::OBJECT);
		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET Staged;
		if (nullptr == Timing || nullptr == Attachment || nullptr == Trail ||
			!Read_Unsigned(Value, "packetVersion", Staged.iPacketVersion) ||
			!Read_String(Value, "adapterId", Staged.strAdapterId) ||
			!Read_Bool(Value, "boundedSemanticReplay",
				Staged.bBoundedSemanticReplay) ||
			!Read_Bool(Value, "nativeExecution", Staged.bNativeExecution) ||
			!Read_String(Value, "runtimeCarrier", Staged.strRuntimeCarrier) ||
			!Read_String(Value, "sourceNotifyType", Staged.strSourceNotifyType) ||
			!Read_String(Value, "sourceEventId", Staged.strSourceEventId) ||
			!Read_Sha(Value, "sourceEventRecordSha256",
				Staged.strSourceEventRecordSha256) ||
			!Read_String(Value, "sourceAsset", Staged.strSourceAsset) ||
			!Read_String(Value, "clip", Staged.strClip) ||
			!Read_Number(Value, "localTimeSeconds", Staged.fLocalTimeSeconds) ||
			!Read_Number(Value, "globalTimeSeconds", Staged.fGlobalTimeSeconds) ||
			!Read_Number(Value, "durationSeconds", Staged.fDurationSeconds) ||
			!Read_String(Value, "targetElementId", Staged.strTargetElementId) ||
			!Parse_TrailTiming(*Timing, Staged.TargetTiming) ||
			!Parse_TrailAttachment(*Attachment, Staged.Attachment,
				bBakedEdgePacket, strOutError) ||
			!Parse_TrailGeometry(*Trail, Staged.Trail) ||
			(bBakedEdgePacket &&
				(!Read_String(Value, "historyId", Staged.strHistoryId) ||
				 !Read_Sha(Value, "historySha256", Staged.strHistorySha256) ||
				 !Read_Number(Value, "playbackClampSeconds",
					 Staged.fPlaybackClampSeconds) ||
				 !Read_String(Value, "coordinateBasis", Staged.strCoordinateBasis))) ||
			!Read_StringArray(Value, "preservedLimitations",
				Staged.PreservedLimitations) ||
			!Read_Sha(Value, "packetSha256", Staged.strPacketSha256) ||
			Staged.strAdapterId != "animation-trail-document-v12" ||
			!Staged.bBoundedSemanticReplay || Staged.bNativeExecution ||
			Staged.strSourceNotifyType != "Trails" ||
			Staged.fLocalTimeSeconds < 0.0 || Staged.fGlobalTimeSeconds < 0.0 ||
			Staged.fDurationSeconds <= 0.0 || Staged.PreservedLimitations.empty())
			return false;
		if (bBakedEdgePacket)
		{
			if (Staged.strRuntimeCarrier !=
					"EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1" ||
				!Is_StableId(Staged.strHistoryId) ||
				Staged.fPlaybackClampSeconds <= 0.0 ||
				Staged.fPlaybackClampSeconds > Staged.fDurationSeconds + 5e-5 ||
				Staged.strCoordinateBasis !=
					"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS")
				return false;
		}
		else if (Staged.strRuntimeCarrier != "EFFECT_TYPED_ANIMATION_TRAIL_V1")
			return false;
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_VisualLightProfile(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_LIGHT_PROFILE& Out)
	{
		if (!Has_ExactKeys(Value, {
			"enabled", "profileId", "status", "range", "intensity", "color",
			"ambient", "falloffExponent" }))
			return false;
		EFFECT_VISUAL_PROGRAM_LIGHT_PROFILE Staged;
		if (!Read_Bool(Value, "enabled", Staged.bEnabled) ||
			!Read_String(Value, "profileId", Staged.strProfileId) ||
			!Read_String(Value, "status", Staged.strStatus) ||
			!Read_Number(Value, "range", Staged.fRange) ||
			!Read_Number(Value, "intensity", Staged.fIntensity) ||
			!Read_NumberArray(Value, "color", Staged.vColor) ||
			!Read_NumberArray(Value, "ambient", Staged.vAmbient) ||
			!Read_Number(Value, "falloffExponent", Staged.fFalloffExponent) ||
			!Staged.bEnabled ||
			Staged.strProfileId != "light.point.reconstructed.v1" ||
			Staged.strStatus != "reconstructed_profile" ||
			Staged.fRange <= 0.0 || Staged.fIntensity < 0.0 ||
			Staged.fFalloffExponent <= 0.0)
			return false;
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_BakedEdgeLightPacket(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET>& Out,
		std::string& strOutError)
	{
		if (Value.Is_Null()) { Out.reset(); return true; }
		if (!Has_ExactKeys(Value, {
			"packetVersion", "adapterId", "boundedSemanticReplay",
			"nativeExecution", "runtimeCarrier", "sourceEventId",
			"sourceEventRecordSha256", "targetElementId", "historyId",
			"historySha256", "lane", "activeStartSeconds",
			"activeDurationSeconds", "activeEndSeconds",
			"historyPlaybackClampSeconds", "coordinateBasis",
			"attachmentEvidenceStatus", "targetLight",
			"preservedLimitations", "packetSha256" }) ||
			!Verify_Seal(Value, "packetSha256", strOutError))
			return false;
		EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LIGHT_PACKET Staged;
		std::string Lane;
		const DATA_JSON_VALUE* TargetLight = Required(
			Value, "targetLight", DATA_JSON_TYPE::OBJECT);
		if (nullptr == TargetLight ||
			!Read_Unsigned(Value, "packetVersion", Staged.iPacketVersion) ||
			!Read_String(Value, "adapterId", Staged.strAdapterId) ||
			!Read_Bool(Value, "boundedSemanticReplay",
				Staged.bBoundedSemanticReplay) ||
			!Read_Bool(Value, "nativeExecution", Staged.bNativeExecution) ||
			!Read_String(Value, "runtimeCarrier", Staged.strRuntimeCarrier) ||
			!Read_String(Value, "sourceEventId", Staged.strSourceEventId) ||
			!Read_Sha(Value, "sourceEventRecordSha256",
				Staged.strSourceEventRecordSha256) ||
			!Read_String(Value, "targetElementId", Staged.strTargetElementId) ||
			!Read_String(Value, "historyId", Staged.strHistoryId) ||
			!Read_Sha(Value, "historySha256", Staged.strHistorySha256) ||
			!Read_String(Value, "lane", Lane) ||
			!Read_Number(Value, "activeStartSeconds", Staged.fActiveStartSeconds) ||
			!Read_Number(Value, "activeDurationSeconds",
				Staged.fActiveDurationSeconds) ||
			!Read_Number(Value, "activeEndSeconds", Staged.fActiveEndSeconds) ||
			!Read_Number(Value, "historyPlaybackClampSeconds",
				Staged.fHistoryPlaybackClampSeconds) ||
			!Read_String(Value, "coordinateBasis", Staged.strCoordinateBasis) ||
			!Read_String(Value, "attachmentEvidenceStatus",
				Staged.strAttachmentEvidenceStatus) ||
			!Parse_VisualLightProfile(*TargetLight, Staged.TargetLight) ||
			!Read_StringArray(Value, "preservedLimitations",
				Staged.PreservedLimitations) ||
			!Read_Sha(Value, "packetSha256", Staged.strPacketSha256))
			return false;
		Staged.eLane = Lane == "FIRST_EDGE" ?
			EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::FIRST_EDGE :
			EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::END;
		if (1u != Staged.iPacketVersion ||
			Staged.strAdapterId != "light-particle-document-v12" ||
			!Staged.bBoundedSemanticReplay || Staged.bNativeExecution ||
			Staged.strRuntimeCarrier !=
				"EFFECT_TYPED_LIGHT_BAKED_EDGE_ATTACHMENT_V1" ||
			Staged.strSourceEventId.empty() ||
			!Is_StableId(Staged.strTargetElementId) ||
			!Is_StableId(Staged.strHistoryId) ||
			Staged.eLane != EFFECT_VISUAL_PROGRAM_BAKED_EDGE_LANE::FIRST_EDGE ||
			Staged.fActiveStartSeconds < 0.0 ||
			Staged.fActiveDurationSeconds <= 0.0 ||
			std::abs(Staged.fActiveEndSeconds -
				(Staged.fActiveStartSeconds + Staged.fActiveDurationSeconds)) > 5e-5 ||
			Staged.fHistoryPlaybackClampSeconds <= 0.0 ||
			Staged.fActiveEndSeconds > Staged.fHistoryPlaybackClampSeconds + 5e-5 ||
			Staged.strCoordinateBasis !=
				"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS" ||
			Staged.strAttachmentEvidenceStatus !=
				"SIBLING_TEMPLATE_INFERRED" ||
			Staged.PreservedLimitations.empty())
			return false;
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_AdapterRules(
		const DATA_JSON_VALUE& Root,
		ADAPTER_RULES& Out,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Values = Required(Root, "adapterContracts", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Values || Values->Get_Array().empty())
		{
			strOutError = "Visual-program adapter allowlist is missing.";
			return false;
		}
		ADAPTER_RULES Staged;
		for (const DATA_JSON_VALUE& Value : Values->Get_Array())
		{
			std::string AdapterId;
			std::string Family;
			std::vector<std::string> Layouts;
			std::vector<std::string> Roles;
			if (!Has_ExactKeys(Value, {
				"adapterId", "family", "packetLayouts", "allowedResourceRoles" }) ||
				!Read_String(Value, "adapterId", AdapterId) ||
				!Read_String(Value, "family", Family) ||
				!Read_StringArray(Value, "packetLayouts", Layouts) || Layouts.empty() ||
				!Read_StringArray(Value, "allowedResourceRoles", Roles) || Roles.empty())
			{
				strOutError = "Visual-program adapter allowlist row is invalid.";
				return false;
			}
			ADAPTER_RULE Rule;
			Rule.eFamily = Parse_Family(Family);
			Rule.PacketLayouts.insert(Layouts.begin(), Layouts.end());
			Rule.ResourceRoles.insert(Roles.begin(), Roles.end());
			if (Rule.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::END ||
				Rule.PacketLayouts.size() != Layouts.size() ||
				Rule.ResourceRoles.size() != Roles.size() ||
				!Staged.emplace(AdapterId, std::move(Rule)).second)
			{
				strOutError = "Visual-program adapter allowlist has an unknown or duplicate value.";
				return false;
			}
		}
		Out = std::move(Staged);
		return true;
	}

	bool_t Parse_RuntimeRow(
		const DATA_JSON_VALUE& Value,
		const ADAPTER_RULES& AdapterRules,
		EFFECT_VISUAL_PROGRAM_ROW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"selector", "selectorSha256", "family", "adapterId", "packetLayout",
			"fidelity", "disposition", "tuningEligibleTransform", "sourceIdentity",
			"targetIdentity", "resourcePacket", "localDecalPacket",
			"admissionBlockers", "rowSha256" }) ||
			!Verify_Seal(Value, "rowSha256", strOutError))
		{
			if (strOutError.empty()) strOutError = "Visual-program row shape is invalid.";
			return false;
		}
		const DATA_JSON_VALUE* Selector = Required(Value, "selector", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Source = Required(Value, "sourceIdentity", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Target = Value.Find("targetIdentity");
		const DATA_JSON_VALUE* Resources = Required(Value, "resourcePacket", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Packet = Value.Find("localDecalPacket");
		std::string ClaimedSelectorSha;
		std::string Family;
		std::string Disposition;
		if (nullptr == Selector || nullptr == Source || nullptr == Target || nullptr == Resources ||
			nullptr == Packet || !Parse_Selector(*Selector, Out.Selector, strOutError) ||
			!Read_Sha(Value, "selectorSha256", ClaimedSelectorSha) ||
			ClaimedSelectorSha != Out.Selector.strSelectorSha256 ||
			!Read_String(Value, "family", Family) ||
			!Read_String(Value, "adapterId", Out.strAdapterId) ||
			!Read_String(Value, "packetLayout", Out.strPacketLayout) ||
			!Read_String(Value, "fidelity", Out.strFidelity) ||
			!Read_String(Value, "disposition", Disposition) ||
			!Read_Bool(Value, "tuningEligibleTransform", Out.bTuningEligibleTransform) ||
			!Parse_SourceIdentity(*Source, Out.SourceIdentity, strOutError) ||
			!Parse_TargetIdentity(*Target, Out.TargetIdentity, strOutError) ||
			!Parse_LocalDecalPacket(*Packet, Out.LocalDecalPacket, strOutError) ||
			!Read_StringArray(Value, "admissionBlockers", Out.AdmissionBlockers) ||
			!Read_Sha(Value, "rowSha256", Out.strRowSha256))
		{
			if (strOutError.empty()) strOutError = "Visual-program row identity is invalid.";
			return false;
		}
		Out.eFamily = Parse_Family(Family);
		if (Disposition == "ADMITTED_BOUNDED")
			Out.eDisposition = EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED;
		else if (Disposition == "FAIL_CLOSED")
			Out.eDisposition = EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED;
		else
			Out.eDisposition = EFFECT_VISUAL_PROGRAM_DISPOSITION::END;
		const auto Adapter = AdapterRules.find(Out.strAdapterId);
		if (Out.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::END ||
			Out.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::END ||
			Adapter == AdapterRules.end() || Adapter->second.eFamily != Out.eFamily ||
			!Adapter->second.PacketLayouts.contains(Out.strPacketLayout))
		{
			strOutError = "Visual-program row uses an unknown adapter/family/layout combination.";
			return false;
		}
		for (const DATA_JSON_VALUE& Resource : Resources->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW Parsed;
			if (!Parse_Resource(Resource, Parsed, strOutError) ||
				!Adapter->second.ResourceRoles.contains(Parsed.strRole))
			{
				if (strOutError.empty()) strOutError = "Visual-program resource role is not allowed.";
				return false;
			}
			Out.Resources.push_back(std::move(Parsed));
		}
		if (Out.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED)
		{
			if (Out.TargetIdentity.has_value() || Out.LocalDecalPacket.has_value() ||
				Out.strPacketLayout != "NONE" || Out.AdmissionBlockers.empty())
			{
				strOutError = "Fail-closed visual row contains executable mutation state.";
				return false;
			}
		}
		else if (!Out.TargetIdentity.has_value() || !Out.AdmissionBlockers.empty())
		{
			strOutError = "Admitted visual row lacks a target or retains blockers.";
			return false;
		}
		if (Out.LocalDecalPacket.has_value())
		{
			if (Out.LocalDecalPacket->strAdapterId != Out.strAdapterId ||
				Out.strPacketLayout != "LOCAL_DECAL_RT0_SIX_SRV_V1" ||
				Out.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE ||
				Out.Resources.size() != Out.LocalDecalPacket->Srvs.size())
			{
				strOutError = "LocalDecal packet/row identity is inconsistent.";
				return false;
			}
			for (size_t Index = 0u; Index < Out.Resources.size(); ++Index)
			{
				const auto& Resource = Out.Resources[Index];
				const auto& Srv = Out.LocalDecalPacket->Srvs[Index];
				if (Resource.strRole != Srv.strRole || Resource.strAssetId != Srv.strAssetId ||
					Resource.strRawSha256 != Srv.strRawSha256 || Resource.iByteCount != Srv.iByteCount ||
					Resource.strShaderRegister != Srv.strShaderRegister ||
					Resource.strSourceChannel != Srv.strSourceChannel)
				{
					strOutError = "LocalDecal packet/resource join is stale.";
					return false;
				}
			}
		}
		return true;
	}

	bool_t Parse_RuntimeSupplementalElement(
		const DATA_JSON_VALUE& Value,
		const ADAPTER_RULES& AdapterRules,
		EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"selector", "selectorSha256", "family", "adapterId", "packetLayout",
			"fidelity", "disposition", "tuningEligibleTransform",
			"sourceIdentity", "targetIdentity", "schedule", "resourcePacket",
			"cascadeRibbonPacket", "animationTrailPacket",
			"bakedEdgeLightPacket", "admissionBlockers",
			"rowSha256" }) || !Verify_Seal(Value, "rowSha256", strOutError))
			return false;
		const DATA_JSON_VALUE* Selector = Required(
			Value, "selector", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Source = Required(
			Value, "sourceIdentity", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Target = Required(
			Value, "targetIdentity", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Schedule = Required(
			Value, "schedule", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Resources = Required(
			Value, "resourcePacket", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Cascade = Value.Find("cascadeRibbonPacket");
		const DATA_JSON_VALUE* Animation = Value.Find("animationTrailPacket");
		const DATA_JSON_VALUE* BakedEdgeLight =
			Value.Find("bakedEdgeLightPacket");
		std::string SelectorSha;
		std::string Family;
		std::string Disposition;
		std::optional<EFFECT_VISUAL_PROGRAM_TARGET_IDENTITY> ParsedTarget;
		if (nullptr == Selector || nullptr == Source || nullptr == Target ||
			nullptr == Schedule || nullptr == Resources || nullptr == Cascade ||
			nullptr == Animation || nullptr == BakedEdgeLight ||
			!Parse_Selector(*Selector, Out.Selector, strOutError) ||
			!Read_Sha(Value, "selectorSha256", SelectorSha) ||
			SelectorSha != Out.Selector.strSelectorSha256 ||
			!Read_String(Value, "family", Family) ||
			!Read_String(Value, "adapterId", Out.strAdapterId) ||
			!Read_String(Value, "packetLayout", Out.strPacketLayout) ||
			!Read_String(Value, "fidelity", Out.strFidelity) ||
			!Read_String(Value, "disposition", Disposition) ||
			!Read_Bool(Value, "tuningEligibleTransform",
				Out.bTuningEligibleTransform) ||
			!Has_ExactKeys(*Source, {
				"sourceRecordId", "sourceRecordSha256", "sourcePayloadRawSha256" }) ||
			!Read_String(*Source, "sourceRecordId", Out.strSourceRecordId) ||
			!Read_Sha(*Source, "sourceRecordSha256", Out.strSourceRecordSha256) ||
			!Read_Sha(*Source, "sourcePayloadRawSha256",
				Out.strSourcePayloadRawSha256) ||
			!Parse_TargetIdentity(*Target, ParsedTarget, strOutError) ||
			!ParsedTarget.has_value() ||
			!Has_ExactKeys(*Schedule, {
				"stageId", "sourceEventId", "sourceTimelineSeconds",
				"localTimeSeconds", "durationSeconds" }) ||
			!Read_String(*Schedule, "stageId", Out.strStageId) ||
			!Read_String(*Schedule, "sourceEventId", Out.strSourceEventId) ||
			!Read_Number(*Schedule, "sourceTimelineSeconds",
				Out.fSourceTimelineSeconds) ||
			!Read_Number(*Schedule, "localTimeSeconds", Out.fLocalTimeSeconds) ||
			!Read_Number(*Schedule, "durationSeconds", Out.fDurationSeconds) ||
			!Parse_CascadeRibbonPacket(*Cascade, Out.CascadeRibbonPacket,
				strOutError) ||
			!Parse_AnimationTrailPacket(*Animation, Out.AnimationTrailPacket,
				strOutError) ||
			!Parse_BakedEdgeLightPacket(*BakedEdgeLight,
				Out.BakedEdgeLightPacket, strOutError) ||
			!Read_StringArray(Value, "admissionBlockers", Out.AdmissionBlockers) ||
			!Read_Sha(Value, "rowSha256", Out.strRowSha256))
			return false;
		Out.TargetIdentity = *ParsedTarget;
		Out.eFamily = Parse_Family(Family);
		Out.eDisposition = Disposition == "ADMITTED_BOUNDED" ?
			EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED :
			EFFECT_VISUAL_PROGRAM_DISPOSITION::END;
		const auto Adapter = AdapterRules.find(Out.strAdapterId);
		if (Out.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Out.bTuningEligibleTransform || !Out.AdmissionBlockers.empty() ||
			Out.strFidelity != "BOUNDED_RECONSTRUCTION" ||
			Adapter == AdapterRules.end() || Adapter->second.eFamily != Out.eFamily ||
			!Adapter->second.PacketLayouts.contains(Out.strPacketLayout) ||
			Out.fSourceTimelineSeconds < 0.0 || Out.fLocalTimeSeconds < 0.0 ||
			Out.fDurationSeconds <= 0.0)
			return false;
		for (const DATA_JSON_VALUE& Resource : Resources->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW Parsed;
			if (!Parse_Resource(Resource, Parsed, strOutError) ||
				!Adapter->second.ResourceRoles.contains(Parsed.strRole))
				return false;
			Out.Resources.push_back(std::move(Parsed));
		}
		if (Out.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
		{
			if (Out.strPacketLayout != "CASCADE_RIBBON_TYPED_PACKET_V1" ||
				!Out.CascadeRibbonPacket.has_value() ||
				Out.AnimationTrailPacket.has_value() ||
				Out.BakedEdgeLightPacket.has_value() ||
				Out.CascadeRibbonPacket->strAdapterId != Out.strAdapterId)
				return false;
		}
		else if (Out.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL)
		{
			const bool_t bLegacyPacket =
				Out.strPacketLayout == "ANIMATION_TRAIL_ELEMENT_V1";
			const bool_t bBakedEdgePacket = Out.strPacketLayout ==
				"ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1";
			if ((!bLegacyPacket && !bBakedEdgePacket) ||
				Out.CascadeRibbonPacket.has_value() ||
				Out.BakedEdgeLightPacket.has_value() ||
				!Out.AnimationTrailPacket.has_value() ||
				Out.AnimationTrailPacket->strAdapterId != Out.strAdapterId ||
				Out.AnimationTrailPacket->iPacketVersion !=
					(bBakedEdgePacket ? 2u : 1u) ||
				Out.AnimationTrailPacket->strSourceEventId != Out.strSourceEventId ||
				Out.AnimationTrailPacket->strTargetElementId !=
					Out.TargetIdentity.strTargetElementId)
				return false;
		}
		else if (Out.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
		{
			if (Out.strPacketLayout != "LIGHT_BAKED_EDGE_ATTACHMENT_V1" ||
				Out.CascadeRibbonPacket.has_value() ||
				Out.AnimationTrailPacket.has_value() ||
				!Out.BakedEdgeLightPacket.has_value() ||
				Out.BakedEdgeLightPacket->strAdapterId != Out.strAdapterId ||
				Out.BakedEdgeLightPacket->strSourceEventId != Out.strSourceEventId ||
				Out.BakedEdgeLightPacket->strTargetElementId !=
					Out.TargetIdentity.strTargetElementId)
				return false;
		}
		else return false;
		return true;
	}

	const DATA_JSON_VALUE* Find_ElementValue(
		const DATA_JSON_VALUE& Document,
		const std::string_view ElementId)
	{
		const DATA_JSON_VALUE* Elements = Required(Document, "elements", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Elements) return nullptr;
		const DATA_JSON_VALUE* Found = nullptr;
		for (const DATA_JSON_VALUE& Element : Elements->Get_Array())
		{
			const DATA_JSON_VALUE* Id = Required(Element, "id", DATA_JSON_TYPE::STRING);
			if (nullptr != Id && Id->Get_String() == ElementId)
			{
				if (nullptr != Found) return nullptr;
				Found = &Element;
			}
		}
		return Found;
	}

	std::string Canonical_Sha(const DATA_JSON_VALUE& Value)
	{
		return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value));
	}

	bool_t Parse_BakedEdgeHistory(
		const DATA_JSON_VALUE& Value,
		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"historyId", "sourceKind", "sourceArtifactPath",
			"sourceArtifactRawSha256", "coordinateBasis",
			"sourceEndTimeSeconds", "playbackClampSeconds", "sampleCount",
			"samples", "samplesSha256", "historySha256" }) ||
			!Verify_Seal(Value, "historySha256", strOutError))
			return false;
		const DATA_JSON_VALUE* Samples = Required(
			Value, "samples", DATA_JSON_TYPE::ARRAY);
		EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY Staged;
		if (nullptr == Samples || Samples->Get_Array().size() < 2u ||
			!Read_String(Value, "historyId", Staged.strHistoryId) ||
			!Read_String(Value, "sourceKind", Staged.strSourceKind) ||
			!Read_String(Value, "sourceArtifactPath", Staged.strSourceArtifactPath) ||
			!Read_Sha(Value, "sourceArtifactRawSha256",
				Staged.strSourceArtifactRawSha256) ||
			!Read_String(Value, "coordinateBasis", Staged.strCoordinateBasis) ||
			!Read_Number(Value, "sourceEndTimeSeconds",
				Staged.fSourceEndTimeSeconds) ||
			!Read_Number(Value, "playbackClampSeconds",
				Staged.fPlaybackClampSeconds) ||
			!Read_Unsigned(Value, "sampleCount", Staged.iSampleCount) ||
			!Read_Sha(Value, "samplesSha256", Staged.strSamplesSha256) ||
			!Read_Sha(Value, "historySha256", Staged.strHistorySha256) ||
			!Is_StableId(Staged.strHistoryId) ||
			Staged.strSourceKind !=
				"UE3_ANIMTRAIL_BAKED_EDGE_HISTORY_V1" ||
			Staged.strCoordinateBasis !=
				"UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS" ||
			Staged.strSourceArtifactPath.find("..") != std::string::npos ||
			Staged.strSourceArtifactPath.find(':') != std::string::npos ||
			Staged.fSourceEndTimeSeconds <= 0.0 ||
			Staged.fPlaybackClampSeconds <= 0.0 ||
			Staged.fPlaybackClampSeconds > Staged.fSourceEndTimeSeconds ||
			Staged.iSampleCount != Samples->Get_Array().size() ||
			Canonical_Sha(*Samples) != Staged.strSamplesSha256)
			return false;

		Staged.Samples.reserve(Samples->Get_Array().size());
		double fPreviousTime = -1.0;
		for (const DATA_JSON_VALUE& SampleValue : Samples->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_SAMPLE Sample;
			if (!Has_ExactKeys(SampleValue, {
				"relativeTimeSeconds", "firstEdgeUE3Cm", "controlPointUE3Cm",
				"secondEdgeUE3Cm" }) ||
				!Read_Number(SampleValue, "relativeTimeSeconds",
					Sample.fRelativeTimeSeconds) ||
				!Read_NumberArray(SampleValue, "firstEdgeUE3Cm",
					Sample.vFirstEdgeUE3Cm) ||
				!Read_NumberArray(SampleValue, "controlPointUE3Cm",
					Sample.vControlPointUE3Cm) ||
				!Read_NumberArray(SampleValue, "secondEdgeUE3Cm",
					Sample.vSecondEdgeUE3Cm) ||
				Sample.fRelativeTimeSeconds <= fPreviousTime)
				return false;
			fPreviousTime = Sample.fRelativeTimeSeconds;
			Staged.Samples.push_back(std::move(Sample));
		}
		if (std::abs(Staged.Samples.front().fRelativeTimeSeconds) > 1e-6 ||
			Staged.Samples.back().fRelativeTimeSeconds + 5e-5 <
				Staged.fPlaybackClampSeconds ||
			Staged.Samples.back().fRelativeTimeSeconds >
				Staged.fSourceEndTimeSeconds + 5e-5)
			return false;
		Out = std::move(Staged);
		return true;
	}

	DATA_JSON_VALUE Normalize_TypedSemanticZero(const DATA_JSON_VALUE& Value)
	{
		switch (Value.Get_Type())
		{
		case DATA_JSON_TYPE::NUMBER:
			return DATA_JSON_VALUE::Number(
				Value.Get_Number() == 0.0 ? 0.0 : Value.Get_Number(),
				Value.Was_FloatingPointToken());
		case DATA_JSON_TYPE::ARRAY:
		{
			DATA_JSON_VALUE::ARRAY Normalized;
			Normalized.reserve(Value.Get_Array().size());
			for (const DATA_JSON_VALUE& Item : Value.Get_Array())
				Normalized.push_back(Normalize_TypedSemanticZero(Item));
			return DATA_JSON_VALUE::Array(std::move(Normalized));
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			DATA_JSON_VALUE::OBJECT Normalized;
			for (const auto& [Name, Item] : Value.Get_Object())
				Normalized.emplace(Name, Normalize_TypedSemanticZero(Item));
			return DATA_JSON_VALUE::Object(
				std::move(Normalized), Value.Get_ObjectInsertionOrder());
		}
		case DATA_JSON_TYPE::BOOLEAN:
			return DATA_JSON_VALUE::Boolean(Value.Get_Boolean());
		case DATA_JSON_TYPE::STRING:
			return DATA_JSON_VALUE::String(Value.Get_String());
		default:
			return DATA_JSON_VALUE::Null();
		}
	}

	bool_t Parse_RuntimeProgram(
		const DATA_JSON_VALUE& Value,
		const ADAPTER_RULES& AdapterRules,
		EFFECT_VISUAL_PROGRAM& Out,
		std::string& strOutError)
	{
		if (!Has_ExactKeys(Value, {
			"effectAssetId", "projectionKind", "baseDocumentIdentity",
			"projectedDocument", "projectedDocumentCanonicalByteCount",
			"projectedDocumentSha256", "projectedDocumentTypedCodecSha256",
			"visualRows", "supplementalElements", "bakedEdgeHistories",
			"programSha256" }) ||
			!Verify_Seal(Value, "programSha256", strOutError) ||
			!Read_String(Value, "effectAssetId", Out.strEffectAssetId) ||
			!Is_StableId(Out.strEffectAssetId) ||
			!Read_Sha(Value, "programSha256", Out.strProgramSha256))
		{
			if (strOutError.empty()) strOutError = "Visual-program entry shape is invalid.";
			return false;
		}
		std::string ProjectionKind;
		if (!Read_String(Value, "projectionKind", ProjectionKind)) return false;
		if (ProjectionKind == "SOURCE_RECIPE_OVERLAY_V1")
			Out.eProjectionKind = EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1;
		else if (ProjectionKind == "ADAPTER_PACKET_V1")
			Out.eProjectionKind = EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
		else
		{
			strOutError = "Visual-program projection kind is unknown.";
			return false;
		}

		const DATA_JSON_VALUE* BaseIdentity = Value.Find("baseDocumentIdentity");
		const DATA_JSON_VALUE* ProjectedValue = Value.Find("projectedDocument");
		const DATA_JSON_VALUE* ByteCount = Value.Find("projectedDocumentCanonicalByteCount");
		const DATA_JSON_VALUE* ProjectedSha = Value.Find("projectedDocumentSha256");
		if (nullptr == BaseIdentity || nullptr == ProjectedValue || nullptr == ByteCount || nullptr == ProjectedSha)
			return false;
		if (Out.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			if (!Has_ExactKeys(*BaseIdentity, {
				"rawSha256", "canonicalSha256", "typedCodecSha256" }) ||
				!Read_Sha(*BaseIdentity, "rawSha256", Out.strBaseDocumentRawSha256) ||
				!Read_Sha(*BaseIdentity, "canonicalSha256", Out.strBaseDocumentCanonicalSha256) ||
				!Read_Sha(*BaseIdentity, "typedCodecSha256", Out.strBaseDocumentTypedCodecSha256) ||
				!ProjectedValue->Is_Object() ||
				!Read_Unsigned(Value, "projectedDocumentCanonicalByteCount",
					Out.iProjectedDocumentCanonicalByteCount) ||
				!Read_Sha(Value, "projectedDocumentSha256", Out.strProjectedDocumentSha256) ||
				!Read_Sha(Value, "projectedDocumentTypedCodecSha256",
					Out.strProjectedDocumentTypedCodecSha256))
			{
				strOutError = "SourceRecipe overlay document identity is invalid.";
				return false;
			}
			const std::string Canonical =
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*ProjectedValue);
			if (Canonical.size() != Out.iProjectedDocumentCanonicalByteCount ||
				Canonical_Sha(*ProjectedValue) != Out.strProjectedDocumentSha256)
			{
				strOutError = "SourceRecipe projected document canonical bytes/SHA are stale.";
				return false;
			}
			EFFECT_DOCUMENT_DESC Parsed;
			if (!CEffectDocumentCodec::Parse_Value(*ProjectedValue, Parsed, strOutError) ||
				Parsed.strEffectAssetId != Out.strEffectAssetId ||
				!CEffectDocumentCodec::Validate_Drawable(Parsed, strOutError))
			{
				if (strOutError.empty()) strOutError = "Projected Effect document is not drawable.";
				return false;
			}
			const std::string TypedCodecSha =
				CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
					Parsed, strOutError);
			if (TypedCodecSha.empty() ||
				TypedCodecSha != Out.strProjectedDocumentTypedCodecSha256)
			{
				if (strOutError.empty())
					strOutError = "Projected Effect document typed-codec SHA is stale.";
				return false;
			}
			Out.pProjectedDocument = std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(Parsed));
		}
		else if (!BaseIdentity->Is_Null() || !ProjectedValue->Is_Null() ||
			!ByteCount->Is_Null() || !ProjectedSha->Is_Null() ||
			!Value.Find("projectedDocumentTypedCodecSha256")->Is_Null())
		{
			strOutError = "Adapter-packet program must not embed a projected document.";
			return false;
		}

		const DATA_JSON_VALUE* Rows = Required(Value, "visualRows", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Supplemental = Required(
			Value, "supplementalElements", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* BakedEdgeHistories = Required(
			Value, "bakedEdgeHistories", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Rows || nullptr == Supplemental ||
			nullptr == BakedEdgeHistories ||
			(Rows->Get_Array().empty() && Supplemental->Get_Array().empty()))
		{
			strOutError = "Visual program contains no executable rows.";
			return false;
		}
		std::string PreviousHistoryId;
		for (const DATA_JSON_VALUE& HistoryValue :
			BakedEdgeHistories->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY History;
			if (!Parse_BakedEdgeHistory(HistoryValue, History, strOutError) ||
				(!PreviousHistoryId.empty() &&
				 History.strHistoryId <= PreviousHistoryId))
			{
				if (strOutError.empty())
					strOutError =
						"Baked-edge histories are invalid, duplicate, or unsorted.";
				return false;
			}
			PreviousHistoryId = History.strHistoryId;
			Out.BakedEdgeHistories.push_back(std::move(History));
		}
		std::string PreviousOccurrence;
		for (const DATA_JSON_VALUE& RowValue : Rows->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_ROW Row;
			if (!Parse_RuntimeRow(RowValue, AdapterRules, Row, strOutError)) return false;
			if (Row.Selector.strEffectAssetId != Out.strEffectAssetId ||
				(!PreviousOccurrence.empty() && Row.Selector.strOccurrenceId <= PreviousOccurrence))
			{
				strOutError = "Visual-program rows are not deterministically grouped/sorted.";
				return false;
			}
			PreviousOccurrence = Row.Selector.strOccurrenceId;
			if (Out.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 &&
				Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED)
			{
				if (!Row.TargetIdentity.has_value() || Row.LocalDecalPacket.has_value() ||
					Row.strPacketLayout != "EFFECT_DOCUMENT_ELEMENT_V12")
				{
					strOutError = "SourceRecipe overlay row has an invalid executable target.";
					return false;
				}
				const DATA_JSON_VALUE* Element = Find_ElementValue(
					*ProjectedValue, Row.TargetIdentity->strTargetElementId);
				const DATA_JSON_VALUE* Recipe = nullptr == Element ? nullptr :
					Required(*Element, "sourceRecipe", DATA_JSON_TYPE::OBJECT);
				const DATA_JSON_VALUE* Modules = nullptr == Recipe ? nullptr :
					Required(*Recipe, "modules", DATA_JSON_TYPE::ARRAY);
				const DATA_JSON_VALUE* Enabled = nullptr == Recipe ? nullptr :
					Required(*Recipe, "enabled", DATA_JSON_TYPE::BOOLEAN);
				const DATA_JSON_VALUE* RendererShape = nullptr == Recipe ? nullptr :
					Required(*Recipe, "rendererShape", DATA_JSON_TYPE::STRING);
				const std::string_view ResolvedExpectedShape =
					Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE ? "mesh" :
					Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE ? "sprite" :
					Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON ? "ribbon" : "";
				if (nullptr == Recipe || nullptr == Modules || nullptr == Enabled ||
					!Enabled->Get_Boolean() || nullptr == RendererShape ||
					(Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE &&
					 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE &&
					 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON) ||
					RendererShape->Get_String() != ResolvedExpectedShape ||
					Canonical_Sha(*Recipe) != Row.SourceIdentity.strSourceRecipeSha256 ||
					Canonical_Sha(*Modules) != Row.SourceIdentity.strModuleClosureSha256 ||
					Modules->Get_Array().size() != Row.SourceIdentity.iModuleCount)
				{
					strOutError = "Projected SourceRecipe/module closure/family is stale.";
					return false;
				}
			}
			else if (Out.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
				Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
				!Row.LocalDecalPacket.has_value())
			{
				strOutError = "Adapter-packet admitted row lacks its typed runtime packet.";
				return false;
			}
			Out.VisualRows.push_back(std::move(Row));
		}
		std::string PreviousSupplementalOccurrence;
		for (const DATA_JSON_VALUE& SupplementalValue : Supplemental->Get_Array())
		{
			EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT Element;
			if (!Parse_RuntimeSupplementalElement(
				SupplementalValue, AdapterRules, Element, strOutError))
			{
				const DATA_JSON_VALUE* Selector =
					SupplementalValue.Find("selector");
				const DATA_JSON_VALUE* Occurrence =
					nullptr != Selector ? Selector->Find("occurrenceId") : nullptr;
				const std::string Identity =
					nullptr != Occurrence && Occurrence->Is_String() ?
					Occurrence->Get_String() : std::string("<unknown>");
				strOutError = "Supplemental visual-program element rejected for " +
					Identity + (strOutError.empty() ? std::string{} :
						": " + strOutError);
				return false;
			}
			if (Element.Selector.strEffectAssetId != Out.strEffectAssetId ||
				(!PreviousSupplementalOccurrence.empty() &&
				 Element.Selector.strOccurrenceId <= PreviousSupplementalOccurrence))
			{
				strOutError = "Supplemental visual-program elements are not sorted/grouped.";
				return false;
			}
			PreviousSupplementalOccurrence = Element.Selector.strOccurrenceId;
			if (Out.eProjectionKind ==
					EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 &&
				Element.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL &&
				Element.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
			{
				strOutError =
					"Source overlay contains an unsupported supplemental element.";
				return false;
			}
			if (Out.eProjectionKind ==
					EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
				Element.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
			{
				strOutError = "Adapter program contains a non-Cascade supplemental element.";
				return false;
			}
			if (Out.eProjectionKind ==
					EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
			{
				const DATA_JSON_VALUE* TargetElement = Find_ElementValue(
					*ProjectedValue, Element.TargetIdentity.strTargetElementId);
				if (nullptr == TargetElement)
				{
					strOutError = "Projected supplemental target is missing.";
					return false;
				}
			}
			Out.SupplementalElements.push_back(std::move(Element));
		}
		std::set<std::string, std::less<>> ReferencedHistoryIds;
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element :
			Out.SupplementalElements)
		{
			std::string_view HistoryId;
			std::string_view HistorySha;
			std::string_view CoordinateBasis;
			double PlaybackClampSeconds = 0.0;
			if (Element.AnimationTrailPacket.has_value() &&
				2u == Element.AnimationTrailPacket->iPacketVersion)
			{
				const auto& Packet = *Element.AnimationTrailPacket;
				HistoryId = Packet.strHistoryId;
				HistorySha = Packet.strHistorySha256;
				CoordinateBasis = Packet.strCoordinateBasis;
				PlaybackClampSeconds = Packet.fPlaybackClampSeconds;
			}
			else if (Element.BakedEdgeLightPacket.has_value())
			{
				const auto& Packet = *Element.BakedEdgeLightPacket;
				HistoryId = Packet.strHistoryId;
				HistorySha = Packet.strHistorySha256;
				CoordinateBasis = Packet.strCoordinateBasis;
				PlaybackClampSeconds = Packet.fHistoryPlaybackClampSeconds;
			}
			else continue;
			const auto History = std::find_if(
				Out.BakedEdgeHistories.begin(), Out.BakedEdgeHistories.end(),
				[HistoryId](const auto& Candidate)
				{
					return Candidate.strHistoryId == HistoryId;
				});
			if (History == Out.BakedEdgeHistories.end() ||
				History->strHistorySha256 != HistorySha ||
				History->strCoordinateBasis != CoordinateBasis ||
				std::abs(History->fPlaybackClampSeconds -
					PlaybackClampSeconds) > 5e-5)
			{
				strOutError =
					"Baked-edge packet/history join is stale.";
				return false;
			}
			ReferencedHistoryIds.emplace(HistoryId);
		}
		if (ReferencedHistoryIds.size() != Out.BakedEdgeHistories.size())
		{
			strOutError = "Visual program contains an unreferenced baked-edge history.";
			return false;
		}
		return true;
	}

	bool_t Parse_ExtensionCanaries(
		const DATA_JSON_VALUE& Root,
		const ADAPTER_RULES& AdapterRules,
		uint32_t& iOutCount,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Canaries = Required(Root, "extensionCanaries", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Canaries || Canaries->Get_Array().size() > UINT32_MAX)
			return false;
		std::set<std::string, std::less<>> CanaryIds;
		std::set<std::pair<std::string, std::string>> Selectors;
		for (const DATA_JSON_VALUE& Canary : Canaries->Get_Array())
		{
			if (!Has_ExactKeys(Canary, {
				"canaryId", "domain", "selector", "selectorSha256", "family",
				"adapterId", "packetLayout", "fidelity", "disposition",
				"productCountContribution", "admissionBlockers", "canarySha256" }) ||
				!Verify_Seal(Canary, "canarySha256", strOutError))
			{
				if (strOutError.empty()) strOutError = "Extension canary shape is invalid.";
				return false;
			}
			std::string CanaryId;
			std::string Domain;
			std::string Family;
			std::string AdapterId;
			std::string PacketLayout;
			std::string Fidelity;
			std::string Disposition;
			std::string SelectorSha;
			bool_t bProduct = true;
			std::vector<std::string> Blockers;
			EFFECT_VISUAL_PROGRAM_SELECTOR Selector;
			const DATA_JSON_VALUE* SelectorValue = Required(Canary, "selector", DATA_JSON_TYPE::OBJECT);
			if (nullptr == SelectorValue ||
				!Read_String(Canary, "canaryId", CanaryId) ||
				!Read_String(Canary, "domain", Domain) ||
				!Parse_Selector(*SelectorValue, Selector, strOutError) ||
				!Read_Sha(Canary, "selectorSha256", SelectorSha) ||
				SelectorSha != Selector.strSelectorSha256 ||
				!Read_String(Canary, "family", Family) ||
				!Read_String(Canary, "adapterId", AdapterId) ||
				!Read_String(Canary, "packetLayout", PacketLayout) ||
				!Read_String(Canary, "fidelity", Fidelity) || Fidelity != "EVIDENCE_ONLY" ||
				!Read_String(Canary, "disposition", Disposition) || Disposition != "FAIL_CLOSED" ||
				!Read_Bool(Canary, "productCountContribution", bProduct) || bProduct ||
				!Read_StringArray(Canary, "admissionBlockers", Blockers) || Blockers.empty())
			{
				if (strOutError.empty()) strOutError = "Extension canary admission boundary is invalid.";
				return false;
			}
			const auto Adapter = AdapterRules.find(AdapterId);
			if (Parse_Family(Family) == EFFECT_VISUAL_PROGRAM_FAMILY::END ||
				Adapter == AdapterRules.end() || Adapter->second.eFamily != Parse_Family(Family) ||
				!Adapter->second.PacketLayouts.contains(PacketLayout) || PacketLayout != "NONE" ||
				!CanaryIds.emplace(CanaryId).second ||
				!Selectors.emplace(Selector.strEffectAssetId, Selector.strOccurrenceId).second)
			{
				strOutError = "Extension canary adapter/selector is unknown or duplicate.";
				return false;
			}
		}
		iOutCount = static_cast<uint32_t>(Canaries->Get_Array().size());
		return true;
	}

	bool_t Parse_Denominators(
		const DATA_JSON_VALUE& Root,
		EFFECT_VISUAL_PROGRAM_CORPUS& Out,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Value = Required(Root, "denominators", DATA_JSON_TYPE::OBJECT);
		uint32_t ProductMutationCount = 1u;
		if (nullptr == Value || !Has_ExactKeys(*Value, {
			"programCount", "sourceRecipeOverlayProgramCount", "adapterPacketProgramCount",
			"visualRowCount", "sourceRecipeOverlayCount", "localDecalAdapterPacketCount",
			"cascadeRibbonVisualRowCount", "supplementalElementCount",
			"artistFCascadeRibbonElementCount", "animationTrailElementCount",
			"bakedEdgeLightElementCount",
			"failClosedCount", "extensionCanaryCount", "productMutationCount" }) ||
			!Read_Unsigned(*Value, "programCount", Out.iDeclaredProgramCount) ||
			!Read_Unsigned(*Value, "sourceRecipeOverlayProgramCount",
				Out.iDeclaredSourceRecipeOverlayProgramCount) ||
			!Read_Unsigned(*Value, "adapterPacketProgramCount",
				Out.iDeclaredAdapterPacketProgramCount) ||
			!Read_Unsigned(*Value, "visualRowCount", Out.iDeclaredVisualRowCount) ||
			!Read_Unsigned(*Value, "sourceRecipeOverlayCount",
				Out.iDeclaredSourceRecipeOverlayCount) ||
			!Read_Unsigned(*Value, "localDecalAdapterPacketCount",
				Out.iDeclaredLocalDecalAdapterPacketCount) ||
			!Read_Unsigned(*Value, "cascadeRibbonVisualRowCount",
				Out.iDeclaredCascadeRibbonVisualRowCount) ||
			!Read_Unsigned(*Value, "supplementalElementCount",
				Out.iDeclaredSupplementalElementCount) ||
			!Read_Unsigned(*Value, "artistFCascadeRibbonElementCount",
				Out.iDeclaredArtistCascadeRibbonElementCount) ||
			!Read_Unsigned(*Value, "animationTrailElementCount",
				Out.iDeclaredAnimationTrailElementCount) ||
			!Read_Unsigned(*Value, "bakedEdgeLightElementCount",
				Out.iDeclaredBakedEdgeLightElementCount) ||
			!Read_Unsigned(*Value, "failClosedCount", Out.iDeclaredFailClosedCount) ||
			!Read_Unsigned(*Value, "extensionCanaryCount",
				Out.iDeclaredExtensionCanaryCount) ||
			!Read_Unsigned(*Value, "productMutationCount", ProductMutationCount) ||
			0u != ProductMutationCount)
		{
			strOutError = "Visual-program declared denominators are invalid.";
			return false;
		}
		return true;
	}

	bool_t Validate_TransactionPolicy(
		const DATA_JSON_VALUE& Root,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Policy = Required(Root, "transactionPolicy", DATA_JSON_TYPE::OBJECT);
		std::vector<std::string> LoadOrder;
		std::string CommitMode;
		std::string FailureAction;
		bool_t bProductMutation = true;
		bool_t bCatalogMutation = true;
		if (nullptr == Policy || !Has_ExactKeys(*Policy, {
			"loadOrder", "commitMode", "failureAction", "productMutation", "catalogMutation" }) ||
			!Read_StringArray(*Policy, "loadOrder", LoadOrder) ||
			LoadOrder != std::vector<std::string>{ "parse", "validate", "stage", "commit" } ||
			!Read_String(*Policy, "commitMode", CommitMode) ||
			CommitMode != "ATOMIC_REPLACE_AFTER_FULL_RUNTIME_VALIDATION" ||
			!Read_String(*Policy, "failureAction", FailureAction) ||
			FailureAction != "PRESERVE_PREVIOUS_SIDECAR_AND_PRODUCT_RUNTIME" ||
			!Read_Bool(*Policy, "productMutation", bProductMutation) || bProductMutation ||
			!Read_Bool(*Policy, "catalogMutation", bCatalogMutation) || bCatalogMutation)
		{
			strOutError = "Visual-program transaction policy is invalid.";
			return false;
		}
		return true;
	}

	bool_t Parse_DocumentJson(
		const EFFECT_DOCUMENT_DESC& Document,
		DATA_JSON_VALUE& Out,
		std::string& strOutError)
	{
		const std::string Json = CEffectDocumentCodec::Serialize(Document);
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = 64u * 1024u * 1024u;
		Limits.iMaximumDepth = 64u;
		Limits.iMaximumValues = 3'000'000u;
		return CDataJson::Parse(Json, Out, strOutError, Limits);
	}

	EFFECT_ELEMENT_DESC* Find_Element(
		EFFECT_DOCUMENT_DESC& Document,
		const std::string_view ElementId)
	{
		EFFECT_ELEMENT_DESC* Found = nullptr;
		for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId == ElementId)
			{
				if (nullptr != Found) return nullptr;
				Found = &Element;
			}
		}
		return Found;
	}

	const EFFECT_ELEMENT_DESC* Find_Element(
		const EFFECT_DOCUMENT_DESC& Document,
		const std::string_view ElementId)
	{
		const EFFECT_ELEMENT_DESC* Found = nullptr;
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.strElementId == ElementId)
			{
				if (nullptr != Found) return nullptr;
				Found = &Element;
			}
		}
		return Found;
	}

	bool_t Materialize_CascadeRibbonSupplemental(
		EFFECT_DOCUMENT_DESC& Document,
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental,
		std::string& strOutError)
	{
		if (Supplemental.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON ||
			!Supplemental.CascadeRibbonPacket.has_value())
			return true;
		EFFECT_ELEMENT_DESC* Target = Find_Element(
			Document, Supplemental.TargetIdentity.strTargetElementId);
		const EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET& Packet =
			*Supplemental.CascadeRibbonPacket;
		if (nullptr == Target || Target->eKind != EFFECT_ELEMENT_KIND::TRAIL ||
			!Target->SourceRecipe.bEnabled ||
			Target->SourceRecipe.strRendererShape != "ribbon" ||
			Target->SourceRecipe.Modules.size() != Packet.iModuleCount)
		{
			strOutError = "CascadeRibbon supplemental target/SourceRecipe is stale.";
			return false;
		}
		Target->Detail.Trail.iMaxPoints = Packet.iOperationalMaxPoints;
		Target->Detail.Trail.fTilingDistanceWorldUnits =
			static_cast<f32_t>(Packet.fTilingDistance);
		Target->Detail.Trail.fDistanceTessellationStepWorldUnits =
			static_cast<f32_t>(Packet.fDistanceTessellationStepSize);
		Target->Detail.Trail.fPointLifeTimeSeconds =
			static_cast<f32_t>(Packet.Trail.fPointLifeTimeSeconds);
		Target->Detail.Trail.fSampleIntervalSeconds =
			static_cast<f32_t>(Packet.Trail.fSampleIntervalSeconds);
		Target->Detail.Trail.fMinimumDistance =
			static_cast<f32_t>(Packet.Trail.fMinimumDistance);
		Target->Detail.Trail.fStartWidth =
			static_cast<f32_t>(Packet.Trail.fStartWidth);
		Target->Detail.Trail.fEndWidth =
			static_cast<f32_t>(Packet.Trail.fEndWidth);
		Target->Detail.Trail.bFaceCamera = Packet.Trail.bFaceCamera;
		return true;
	}

	bool_t Append_LocalDecalRoleTextureEvidence(
		const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET& Packet,
		EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		static constexpr std::array<std::string_view, 6u> Roles = {
			"HEIGHT", "DIFFUSE", "DISSOLVE", "NORMAL", "SPECULAR",
			"EMISSIVE" };
		static constexpr std::array<std::string_view, 6u> CardNames = {
			"Track A LocalDecal | HEIGHT",
			"Track A LocalDecal | DIFFUSE",
			"Track A LocalDecal | DISSOLVE",
			"Track A LocalDecal | NORMAL",
			"Track A LocalDecal | SPECULAR",
			"Track A LocalDecal | EMISSIVE" };
		constexpr std::string_view Group = "Track A LocalDecal DDS Roles";
		constexpr std::string_view SamplingEvidence =
			"track-a-local-decal-six-role-v1";

		EFFECT_SOURCE_MATERIAL_DESC& Source = Element.Material.SourceMaterial;
		if (Element.eKind != EFFECT_ELEMENT_KIND::DECAL || !Source.bEnabled ||
			Source.Textures.size() > 26u)
		{
			strOutError =
				"LocalDecal role evidence requires one ordinary staged Decal Material.";
			return false;
		}

		for (size_t iRole = 0u; iRole < Roles.size(); ++iRole)
		{
			const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Srv =
				Packet.Srvs[iRole];
			EFFECT_RESOURCE_FILE_KIND FileKind =
				EFFECT_RESOURCE_FILE_KIND::END;
			if (Srv.strRole != Roles[iRole] || Srv.strAssetId.empty() ||
				!CEffectDocumentCodec::Is_SafeResourceAssetId(
					Srv.strAssetId, &FileKind) ||
				FileKind != EFFECT_RESOURCE_FILE_KIND::TEXTURE)
			{
				strOutError =
					"LocalDecal role evidence lost its exact DDS role binding: " +
					std::string(Roles[iRole]);
				return false;
			}

			EFFECT_NAMED_TEXTURE_DESC Expected;
			Expected.strName = CardNames[iRole];
			Expected.strGroup = Group;
			/* The packet has a resolved runtime DDS identity, but no source-object
			   path field.  Reuse that exact identity here instead of inventing a
			   package path; this value is shown only as read-only provenance. */
			Expected.strSourceObjectPath = Srv.strAssetId;
			Expected.strAssetId = Srv.strAssetId;
			Expected.eAddressU = EFFECT_TEXTURE_ADDRESS_MODE::CLAMP;
			Expected.eAddressV = EFFECT_TEXTURE_ADDRESS_MODE::CLAMP;
			Expected.eColorSpace = Srv.bSrgb ?
				EFFECT_TEXTURE_COLOR_SPACE::SRGB :
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR;
			Expected.strSamplingEvidence = SamplingEvidence;

			const auto Existing = std::find_if(
				Source.Textures.begin(), Source.Textures.end(),
				[&Expected](const EFFECT_NAMED_TEXTURE_DESC& Texture)
				{ return Texture.strName == Expected.strName; });
			if (Existing == Source.Textures.end())
			{
				Source.Textures.push_back(std::move(Expected));
				continue;
			}
			if (Existing->strGroup != Expected.strGroup ||
				Existing->strSourceObjectPath != Expected.strSourceObjectPath ||
				Existing->strAssetId != Expected.strAssetId ||
				Existing->eAddressU != Expected.eAddressU ||
				Existing->eAddressV != Expected.eAddressV ||
				Existing->eColorSpace != Expected.eColorSpace ||
				Existing->strSamplingEvidence != Expected.strSamplingEvidence)
			{
				strOutError =
					"LocalDecal role evidence alias conflicts with the admitted packet: " +
					Expected.strName;
				return false;
			}
		}
		return true;
	}
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_RowByOccurrenceId(
	const std::string_view strOccurrenceId) const
{
	const auto Found = std::find_if(m_AdmittedRows.begin(), m_AdmittedRows.end(),
		[strOccurrenceId](const EFFECT_VISUAL_PROGRAM_ROW& Row)
		{
			return Row.Selector.strOccurrenceId == strOccurrenceId;
		});
	return Found == m_AdmittedRows.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_BakedEdgeHistory(
	const std::string_view strHistoryId) const
{
	const auto Found = std::find_if(
		m_BakedEdgeHistories.begin(), m_BakedEdgeHistories.end(),
		[strHistoryId](const auto& History)
		{
			return History.strHistoryId == strHistoryId;
		});
	return Found == m_BakedEdgeHistories.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::Find_RowByTargetElementId(
	const std::string_view strTargetElementId) const
{
	const EFFECT_VISUAL_PROGRAM_ROW* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : m_AdmittedRows)
	{
		if (Row.TargetIdentity.has_value() &&
			Row.TargetIdentity->strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Row;
		}
	}
	return Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::
Find_SupplementalElementByOccurrenceId(
	const std::string_view strOccurrenceId) const
{
	const auto Found = std::find_if(
		m_AdmittedSupplementalElements.begin(),
		m_AdmittedSupplementalElements.end(),
		[strOccurrenceId](const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element)
		{
			return Element.Selector.strOccurrenceId == strOccurrenceId;
		});
	return Found == m_AdmittedSupplementalElements.end() ? nullptr : &*Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION::
Find_SupplementalElementByTargetElementId(
	const std::string_view strTargetElementId) const
{
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element :
		m_AdmittedSupplementalElements)
	{
		if (Element.TargetIdentity.strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Element;
		}
	}
	return Found;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Parse(
	const std::string_view Utf8Json,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>& InOutCorpus,
	std::string& strOutError)
{
	strOutError.clear();
	if (Utf8Json.size() >= 3u &&
		static_cast<unsigned char>(Utf8Json[0]) == 0xefu &&
		static_cast<unsigned char>(Utf8Json[1]) == 0xbbu &&
		static_cast<unsigned char>(Utf8Json[2]) == 0xbfu)
	{
		strOutError = "Visual-program runtime sidecar must be UTF-8 without BOM.";
		return false;
	}
	DATA_JSON_PARSE_LIMITS Limits;
	Limits.iMaximumBytes = 16u * 1024u * 1024u;
	Limits.iMaximumDepth = 96u;
	Limits.iMaximumValues = 4'000'000u;
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Utf8Json, Root, strOutError, Limits)) return false;
	if (!Has_ExactKeys(Root, {
		"schema", "formatVersion", "runtimeId", "contractRole", "sourceCorpus",
		"adapterContracts", "programs", "extensionCanaries", "denominators",
		"transactionPolicy", "artifactSha256" }) ||
		!Verify_Seal(Root, "artifactSha256", strOutError))
	{
		if (strOutError.empty()) strOutError = "Visual-program runtime root shape is invalid.";
		return false;
	}

	auto Staged = std::make_shared<EFFECT_VISUAL_PROGRAM_CORPUS>();
	std::string Schema;
	std::string ContractRole;
	if (!Read_String(Root, "schema", Schema) ||
		Schema != "lostark.effect-visual-program-runtime" ||
		!Read_Unsigned(Root, "formatVersion", Staged->iFormatVersion) ||
		1u != Staged->iFormatVersion ||
		!Read_String(Root, "runtimeId", Staged->strRuntimeId) ||
		!Read_String(Root, "contractRole", ContractRole) ||
		ContractRole != "GENERIC_VISUAL_PROGRAM_RUNTIME_SIDECAR_STAGE_INPUT" ||
		!Read_Sha(Root, "artifactSha256", Staged->strArtifactSha256))
	{
		strOutError = "Visual-program runtime header is invalid.";
		return false;
	}
	const DATA_JSON_VALUE* SourceCorpus = Required(Root, "sourceCorpus", DATA_JSON_TYPE::OBJECT);
	std::string SourceCorpusId;
	std::string SourceRawSha;
	if (nullptr == SourceCorpus || !Has_ExactKeys(*SourceCorpus, {
		"corpusId", "artifactSha256", "rawSha256" }) ||
		!Read_String(*SourceCorpus, "corpusId", SourceCorpusId) ||
		!Read_Sha(*SourceCorpus, "artifactSha256", Staged->strSourceCorpusArtifactSha256) ||
		!Read_Sha(*SourceCorpus, "rawSha256", SourceRawSha))
	{
		strOutError = "Visual-program source corpus identity is invalid.";
		return false;
	}
	ADAPTER_RULES AdapterRules;
	if (!Parse_AdapterRules(Root, AdapterRules, strOutError) ||
		!Parse_Denominators(Root, *Staged, strOutError) ||
		!Validate_TransactionPolicy(Root, strOutError))
	{
		return false;
	}
	uint32_t iCanaryCount = 0u;
	if (!Parse_ExtensionCanaries(Root, AdapterRules, iCanaryCount, strOutError) ||
		iCanaryCount != Staged->iDeclaredExtensionCanaryCount)
	{
		if (strOutError.empty()) strOutError = "Extension-canary denominator is stale.";
		return false;
	}
	const DATA_JSON_VALUE* Programs = Required(Root, "programs", DATA_JSON_TYPE::ARRAY);
	if (nullptr == Programs || Programs->Get_Array().empty())
	{
		strOutError = "Visual-program runtime has no programs.";
		return false;
	}
	std::string PreviousEffect;
	for (const DATA_JSON_VALUE& ProgramValue : Programs->Get_Array())
	{
		EFFECT_VISUAL_PROGRAM Program;
		if (!Parse_RuntimeProgram(ProgramValue, AdapterRules, Program, strOutError))
		{
			const DATA_JSON_VALUE* EffectAssetId =
				ProgramValue.Find("effectAssetId");
			const std::string Identity =
				nullptr != EffectAssetId && EffectAssetId->Is_String() ?
				EffectAssetId->Get_String() : std::string("<unknown>");
			strOutError = "Visual-program entry rejected for " + Identity +
				(strOutError.empty() ? std::string{} : ": " + strOutError);
			return false;
		}
		if ((!PreviousEffect.empty() && Program.strEffectAssetId <= PreviousEffect))
		{
			strOutError = "Visual-program entries are not deterministically sorted.";
			return false;
		}
		PreviousEffect = Program.strEffectAssetId;
		Staged->Programs.push_back(std::move(Program));
	}
	if (!Validate(*Staged, strOutError)) return false;
	InOutCorpus = std::move(Staged);
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Load(
	const std::filesystem::path& Path,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS>& InOutCorpus,
	std::string& strOutError)
{
	strOutError.clear();
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		strOutError = "Failed to open visual-program runtime sidecar: " + Path.string();
		return false;
	}
	const std::string Json(
		(std::istreambuf_iterator<char>(Input)), std::istreambuf_iterator<char>());
	if (!Input.good() && !Input.eof())
	{
		strOutError = "Failed to read visual-program runtime sidecar: " + Path.string();
		return false;
	}
	return Parse(Json, InOutCorpus, strOutError);
}

bool_t Client::CEffectVisualProgramCorpusCodec::Validate(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	std::string& strOutError)
{
	strOutError.clear();
	if (1u != Corpus.iFormatVersion || Corpus.strRuntimeId.empty() ||
		!Is_LowerSha256(Corpus.strSourceCorpusArtifactSha256) ||
		!Is_LowerSha256(Corpus.strArtifactSha256) || Corpus.Programs.empty())
	{
		strOutError = "Visual-program corpus identity is invalid.";
		return false;
	}
	uint32_t iOverlayPrograms = 0u;
	uint32_t iAdapterPrograms = 0u;
	uint32_t iRows = 0u;
	uint32_t iOverlayRows = 0u;
	uint32_t iLocalRows = 0u;
	uint32_t iCascadeRibbonRows = 0u;
	uint32_t iSupplementalElements = 0u;
	uint32_t iArtistCascadeRibbonElements = 0u;
	uint32_t iAnimationTrailElements = 0u;
	uint32_t iBakedEdgeLightElements = 0u;
	uint32_t iFailClosedRows = 0u;
	std::string PreviousEffect;
	std::set<std::pair<std::string, std::string>> SeenSelectors;
	for (const EFFECT_VISUAL_PROGRAM& Program : Corpus.Programs)
	{
		if (!Is_StableId(Program.strEffectAssetId) ||
			!Is_LowerSha256(Program.strProgramSha256) ||
			(Program.VisualRows.empty() && Program.SupplementalElements.empty()) ||
			(!PreviousEffect.empty() && Program.strEffectAssetId <= PreviousEffect))
		{
			strOutError = "Visual-program identity/order is invalid.";
			return false;
		}
		PreviousEffect = Program.strEffectAssetId;
		if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			++iOverlayPrograms;
			if (nullptr == Program.pProjectedDocument ||
				Program.pProjectedDocument->strEffectAssetId != Program.strEffectAssetId ||
				!Is_LowerSha256(Program.strBaseDocumentRawSha256) ||
				!Is_LowerSha256(Program.strBaseDocumentCanonicalSha256) ||
				!Is_LowerSha256(Program.strBaseDocumentTypedCodecSha256) ||
				!Is_LowerSha256(Program.strProjectedDocumentSha256) ||
				!Is_LowerSha256(Program.strProjectedDocumentTypedCodecSha256) ||
				0u == Program.iProjectedDocumentCanonicalByteCount)
			{
				strOutError = "SourceRecipe overlay program document identity is invalid.";
				return false;
			}
		}
		else if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			++iAdapterPrograms;
			if (nullptr != Program.pProjectedDocument ||
				!Program.strBaseDocumentRawSha256.empty() ||
				!Program.strBaseDocumentCanonicalSha256.empty() ||
				!Program.strBaseDocumentTypedCodecSha256.empty() ||
				!Program.strProjectedDocumentSha256.empty() ||
				!Program.strProjectedDocumentTypedCodecSha256.empty() ||
				0u != Program.iProjectedDocumentCanonicalByteCount)
			{
				strOutError = "Adapter-packet program contains document projection state.";
				return false;
			}
		}
		else
		{
			strOutError = "Visual-program projection kind is invalid.";
			return false;
		}
		uint32_t iProgramAdmitted = 0u;
		std::set<std::string, std::less<>> SeenTargets;
		std::string PreviousOccurrence;
		for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program.VisualRows)
		{
			++iRows;
			if (Row.Selector.strEffectAssetId != Program.strEffectAssetId ||
				!Is_StableId(Row.Selector.strOccurrenceId) ||
				!Is_LowerSha256(Row.Selector.strSelectorSha256) ||
				!Is_LowerSha256(Row.strRowSha256) ||
				(!PreviousOccurrence.empty() &&
				 Row.Selector.strOccurrenceId <= PreviousOccurrence) ||
				!SeenSelectors.emplace(Row.Selector.strEffectAssetId,
					Row.Selector.strOccurrenceId).second)
			{
				strOutError = "Visual-program selector is duplicate, stale, or unsorted.";
				return false;
			}
			PreviousOccurrence = Row.Selector.strOccurrenceId;
			if (Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
				++iCascadeRibbonRows;
			if (Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED)
			{
				++iFailClosedRows;
				if (Row.TargetIdentity.has_value() || Row.LocalDecalPacket.has_value() ||
					Row.strPacketLayout != "NONE")
				{
					strOutError = "Fail-closed row contains executable payload.";
					return false;
				}
				continue;
			}
			if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Row.TargetIdentity.has_value())
			{
				strOutError = "Visual-program row disposition/target is invalid.";
				return false;
			}
			++iProgramAdmitted;
			if (!SeenTargets.emplace(Row.TargetIdentity->strTargetElementId).second)
			{
				strOutError = "Visual-program admitted target is duplicate.";
				return false;
			}
			if (Program.eProjectionKind == EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
			{
				++iOverlayRows;
				if (Row.LocalDecalPacket.has_value() ||
					Row.strPacketLayout != "EFFECT_DOCUMENT_ELEMENT_V12")
				{
					strOutError = "SourceRecipe overlay row packet boundary is invalid.";
					return false;
				}
			}
			else
			{
				if (!Row.LocalDecalPacket.has_value())
				{
					strOutError = "Adapter row lacks an immutable typed packet.";
					return false;
				}
				++iLocalRows;
			}
		}
		std::set<std::string, std::less<>> SeenSupplementalOccurrences;
		std::set<std::string, std::less<>> ReferencedHistoryIds;
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
			Program.SupplementalElements)
		{
			++iSupplementalElements;
			if (Supplemental.Selector.strEffectAssetId != Program.strEffectAssetId ||
				Supplemental.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Supplemental.AdmissionBlockers.empty() ||
				!SeenSupplementalOccurrences.emplace(
					Supplemental.Selector.strOccurrenceId).second)
			{
				strOutError = "Visual-program supplemental element is invalid.";
				return false;
			}
			if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
				++iArtistCascadeRibbonElements;
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL)
				++iAnimationTrailElements;
			else if (Supplemental.eFamily ==
				EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
				++iBakedEdgeLightElements;
			else
			{
				strOutError = "Visual-program supplemental family is invalid.";
				return false;
			}
			++iProgramAdmitted;
			if (Supplemental.AnimationTrailPacket.has_value() &&
				2u == Supplemental.AnimationTrailPacket->iPacketVersion)
			{
				const auto& Packet = *Supplemental.AnimationTrailPacket;
				const auto History = std::find_if(
					Program.BakedEdgeHistories.begin(),
					Program.BakedEdgeHistories.end(),
					[&Packet](const auto& Candidate)
					{
						return Candidate.strHistoryId == Packet.strHistoryId;
					});
				if (History == Program.BakedEdgeHistories.end() ||
					History->strHistorySha256 != Packet.strHistorySha256)
				{
					strOutError =
						"Visual-program baked-edge history reference is invalid.";
					return false;
				}
				ReferencedHistoryIds.emplace(Packet.strHistoryId);
			}
			if (Supplemental.BakedEdgeLightPacket.has_value())
			{
				const auto& Packet = *Supplemental.BakedEdgeLightPacket;
				const auto History = std::find_if(
					Program.BakedEdgeHistories.begin(),
					Program.BakedEdgeHistories.end(),
					[&Packet](const auto& Candidate)
					{
						return Candidate.strHistoryId == Packet.strHistoryId;
					});
				if (History == Program.BakedEdgeHistories.end() ||
					History->strHistorySha256 != Packet.strHistorySha256 ||
					History->strCoordinateBasis != Packet.strCoordinateBasis ||
					std::abs(History->fPlaybackClampSeconds -
						Packet.fHistoryPlaybackClampSeconds) > 5e-5)
				{
					strOutError =
						"Visual-program baked-edge Light history reference is invalid.";
					return false;
				}
				ReferencedHistoryIds.emplace(Packet.strHistoryId);
			}
		}
		if (ReferencedHistoryIds.size() != Program.BakedEdgeHistories.size())
		{
			strOutError =
				"Visual-program baked-edge history closure is incomplete.";
			return false;
		}
		if (0u == iProgramAdmitted)
		{
			strOutError = "Visual program has no admitted executable row.";
			return false;
		}
	}
	if (Corpus.Programs.size() != Corpus.iDeclaredProgramCount ||
		iOverlayPrograms != Corpus.iDeclaredSourceRecipeOverlayProgramCount ||
		iAdapterPrograms != Corpus.iDeclaredAdapterPacketProgramCount ||
		iRows != Corpus.iDeclaredVisualRowCount ||
		iOverlayRows != Corpus.iDeclaredSourceRecipeOverlayCount ||
		iLocalRows != Corpus.iDeclaredLocalDecalAdapterPacketCount ||
		iCascadeRibbonRows != Corpus.iDeclaredCascadeRibbonVisualRowCount ||
		iSupplementalElements != Corpus.iDeclaredSupplementalElementCount ||
		iArtistCascadeRibbonElements !=
			Corpus.iDeclaredArtistCascadeRibbonElementCount ||
		iAnimationTrailElements != Corpus.iDeclaredAnimationTrailElementCount ||
		iBakedEdgeLightElements != Corpus.iDeclaredBakedEdgeLightElementCount ||
		iFailClosedRows != Corpus.iDeclaredFailClosedCount)
	{
		strOutError = "Visual-program declared denominators do not match internal sums.";
		return false;
	}
	return true;
}

const EFFECT_VISUAL_PROGRAM* Client::CEffectVisualProgramCorpusCodec::Find_Program(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId)
{
	const auto Found = std::lower_bound(Corpus.Programs.begin(), Corpus.Programs.end(),
		strEffectAssetId, [](const EFFECT_VISUAL_PROGRAM& Program, const std::string_view Id)
		{
			return Program.strEffectAssetId < Id;
		});
	return Found != Corpus.Programs.end() && Found->strEffectAssetId == strEffectAssetId ?
		&*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_ROW* Client::CEffectVisualProgramCorpusCodec::Find_Row(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, Selector.strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const auto Found = std::lower_bound(Program->VisualRows.begin(), Program->VisualRows.end(),
		Selector.strOccurrenceId,
		[](const EFFECT_VISUAL_PROGRAM_ROW& Row, const std::string_view Id)
		{
			return Row.Selector.strOccurrenceId < Id;
		});
	return Found != Program->VisualRows.end() &&
		Found->Selector.strOccurrenceId == Selector.strOccurrenceId ? &*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_ROW*
Client::CEffectVisualProgramCorpusCodec::Find_RowByTargetElementId(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId,
	const std::string_view strTargetElementId)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const EFFECT_VISUAL_PROGRAM_ROW* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
	{
		if (Row.TargetIdentity.has_value() &&
			Row.TargetIdentity->strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Row;
		}
	}
	return Found;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::CEffectVisualProgramCorpusCodec::Find_SupplementalElement(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_VISUAL_PROGRAM_SELECTOR& Selector)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(
		Corpus, Selector.strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const auto Found = std::lower_bound(
		Program->SupplementalElements.begin(), Program->SupplementalElements.end(),
		Selector.strOccurrenceId,
		[](const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element,
			const std::string_view Id)
		{
			return Element.Selector.strOccurrenceId < Id;
		});
	return Found != Program->SupplementalElements.end() &&
		Found->Selector.strOccurrenceId == Selector.strOccurrenceId ?
		&*Found : nullptr;
}

const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT*
Client::CEffectVisualProgramCorpusCodec::
Find_SupplementalElementByTargetElementId(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const std::string_view strEffectAssetId,
	const std::string_view strTargetElementId)
{
	const EFFECT_VISUAL_PROGRAM* Program = Find_Program(Corpus, strEffectAssetId);
	if (nullptr == Program) return nullptr;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* Found = nullptr;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Element :
		Program->SupplementalElements)
	{
		if (Element.TargetIdentity.strTargetElementId == strTargetElementId)
		{
			if (nullptr != Found) return nullptr;
			Found = &Element;
		}
	}
	return Found;
}

std::string Client::CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	strOutError.clear();
	DATA_JSON_VALUE Value;
	if (!Parse_DocumentJson(Document, Value, strOutError)) return {};
	return Canonical_Sha(Normalize_TypedSemanticZero(Value));
}

bool_t Client::CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
	const EFFECT_VISUAL_PROGRAM_CORPUS& Corpus,
	const EFFECT_DOCUMENT_DESC& BaseDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& InOutProjection,
	std::string& strOutError)
{
	strOutError.clear();
	if (!Validate(Corpus, strOutError)) return false;
	const EFFECT_VISUAL_PROGRAM* Program =
		Find_Program(Corpus, BaseDocument.strEffectAssetId);
	if (nullptr == Program)
	{
		strOutError = "No visual program exists for the supplied Effect document.";
		return false;
	}
	const bool_t bAdapterPacketProjection =
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				BaseDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		BaseDocument, strOutError))
	{
		return false;
	}
	const std::string BaseSha = Compute_DocumentCanonicalSha256(BaseDocument, strOutError);
	if (BaseSha.empty()) return false;
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 &&
		BaseSha != Program->strBaseDocumentTypedCodecSha256)
	{
		strOutError = "Visual-program base document typed-codec SHA is stale: expected=" +
			Program->strBaseDocumentTypedCodecSha256 + " actual=" + BaseSha + ".";
		return false;
	}
	/* baseDocumentCanonicalSha256 is the sealed raw-JSON identity and is
	   verified while parsing the sidecar.  A typed EFFECT_DOCUMENT_DESC uses
	   f32 fields and cannot reproduce arbitrary source JSON f64 lexemes.  The
	   admission-time identity below is therefore an exact typed-codec compare,
	   not a false equality between those two domains. */
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
	{
		if (nullptr == Program->pProjectedDocument)
		{
			strOutError = "Visual-program projected document is missing.";
			return false;
		}
		EFFECT_DOCUMENT_DESC ExpectedTypedBase = *Program->pProjectedDocument;
		for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
		{
			if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED)
				continue;
			if (!Row.TargetIdentity.has_value())
			{
				strOutError = "Visual-program admitted overlay target is missing.";
				return false;
			}
			if (Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON)
			{
				std::erase_if(ExpectedTypedBase.Elements,
					[&Row](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId ==
							Row.TargetIdentity->strTargetElementId;
					});
				continue;
			}
			EFFECT_ELEMENT_DESC* Target = Find_Element(
				ExpectedTypedBase, Row.TargetIdentity->strTargetElementId);
			if (nullptr == Target)
			{
				strOutError = "Visual-program typed base target is missing.";
				return false;
			}
			Target->SourceRecipe = {};
			/* Legacy v12/v13 carrier rows serialize an explicit loopCount=0. */
			Target->SourceRecipe.iEmitterLoopCount = 0u;
		}
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
			Program->SupplementalElements)
		{
			if (Supplemental.eFamily !=
					EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL &&
				Supplemental.eFamily !=
					EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE)
				continue;
			const EFFECT_ELEMENT_DESC* BaseTarget = Find_Element(
				BaseDocument, Supplemental.TargetIdentity.strTargetElementId);
			if (nullptr == BaseTarget)
			{
				std::erase_if(ExpectedTypedBase.Elements,
					[&Supplemental](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId ==
							Supplemental.TargetIdentity.strTargetElementId;
					});
			}
			else
			{
				EFFECT_ELEMENT_DESC* ProjectedTarget = Find_Element(
					ExpectedTypedBase,
					Supplemental.TargetIdentity.strTargetElementId);
				if (nullptr == ProjectedTarget)
				{
					strOutError =
						"Visual-program projected supplemental target is missing.";
					return false;
				}
				*ProjectedTarget = *BaseTarget;
			}
		}
		std::string ExpectedTypedBaseError;
		std::string ActualTypedBaseError;
		const std::string ExpectedTypedBaseSha =
			CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
				ExpectedTypedBase, ExpectedTypedBaseError);
		const std::string ActualTypedBaseSha =
			CEffectVisualProgramCorpusCodec::Compute_DocumentCanonicalSha256(
				BaseDocument, ActualTypedBaseError);
		if (ExpectedTypedBaseSha.empty() || ActualTypedBaseSha.empty() ||
			ExpectedTypedBaseSha != ActualTypedBaseSha)
		{
			strOutError =
				"Visual-program base document typed codec identity is stale.";
			return false;
		}
	}

	/* The projected document is the immutable ordering authority.  Rebuilding it
	   by appending admitted targets in row/supplemental traversal order changes
	   the typed codec bytes whenever source elements were interleaved.  The
	   reverse projection above already proved that this exact document lowers
	   back to BaseDocument, so stage its complete ordered copy transactionally. */
	EFFECT_DOCUMENT_DESC StagedDocument =
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1 ?
			*Program->pProjectedDocument : BaseDocument;
	std::vector<EFFECT_VISUAL_PROGRAM_ROW> AdmittedRows;
	std::vector<EFFECT_VISUAL_PROGRAM_SELECTOR> AdmittedSelectors;
	std::vector<EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT>
		AdmittedSupplementalElements;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
	{
		if (Row.eDisposition == EFFECT_VISUAL_PROGRAM_DISPOSITION::FAIL_CLOSED)
			continue;
		if (Row.eDisposition != EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Row.TargetIdentity.has_value())
		{
			strOutError = "Visual-program admitted row is malformed.";
			return false;
		}
		if (Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
		{
			if (nullptr == Program->pProjectedDocument)
			{
				strOutError = "Visual-program projected document is missing.";
				return false;
			}
			EFFECT_ELEMENT_DESC* Target = Find_Element(
				StagedDocument, Row.TargetIdentity->strTargetElementId);
			const EFFECT_ELEMENT_DESC* Source = Find_Element(
				*Program->pProjectedDocument, Row.TargetIdentity->strTargetElementId);
			const EFFECT_ELEMENT_KIND ExpectedKind =
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE ?
					EFFECT_ELEMENT_KIND::MESH :
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE ?
					EFFECT_ELEMENT_KIND::SPRITE : EFFECT_ELEMENT_KIND::TRAIL;
			const std::string_view ExpectedShape =
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE ?
					"mesh" :
				Row.eFamily == EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE ?
					"sprite" : "ribbon";
			if (nullptr == Target || nullptr == Source ||
				(Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE &&
				 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE &&
				 Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON) ||
				Target->eKind != ExpectedKind || Source->eKind != ExpectedKind ||
				!Source->SourceRecipe.bEnabled ||
				Source->SourceRecipe.strRendererShape != ExpectedShape ||
				Source->SourceRecipe.Modules.size() != Row.SourceIdentity.iModuleCount)
			{
				strOutError = "Visual-program SourceRecipe target/family/module count is stale.";
				return false;
			}

			/* The sealed row pins targetRecordSha256 and targetPayloadRawSha256,
			   while the full base-document canonical SHA pins every carrier,
			   material, resource, transform, and attachment field.  Only this
			   SourceRecipe assignment is allowed to differ in the staged copy. */
			Target->SourceRecipe = Source->SourceRecipe;
		}
		else if (!Row.LocalDecalPacket.has_value())
		{
			strOutError = "Visual-program adapter row lacks its immutable packet.";
			return false;
		}
		else
		{
			const EFFECT_ELEMENT_DESC* Target = Find_Element(
				StagedDocument, Row.TargetIdentity->strTargetElementId);
			if (Row.eFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE ||
				nullptr == Target || Target->eKind != EFFECT_ELEMENT_KIND::DECAL ||
				Target->Renderer.eType != EFFECT_RENDERER_TYPE::DECAL_PARTICLE ||
				Target->Renderer.eSourceSpace !=
					EFFECT_SOURCE_SPACE::UE3_CASCADE_V1)
			{
				strOutError =
					"Visual-program adapter packet target/family/renderer is stale.";
				return false;
			}
		}
		AdmittedSelectors.push_back(Row.Selector);
		AdmittedRows.push_back(Row);
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		Program->SupplementalElements)
	{
		if (Supplemental.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Supplemental.AdmissionBlockers.empty())
		{
			strOutError = "Visual-program supplemental admission is malformed.";
			return false;
		}
		EFFECT_ELEMENT_DESC* SupplementalTarget = Find_Element(
			StagedDocument, Supplemental.TargetIdentity.strTargetElementId);
		if (nullptr == SupplementalTarget)
		{
			strOutError = "Visual-program supplemental target is missing.";
			return false;
		}
		if (!Materialize_CascadeRibbonSupplemental(
			StagedDocument, Supplemental, strOutError))
			return false;
		if (bAdapterPacketProjection)
		{
			const EFFECT_RENDERER_TYPE eExpectedRenderer =
				Supplemental.CascadeRibbonPacket.has_value() ?
					EFFECT_RENDERER_TYPE::CASCADE_RIBBON :
					EFFECT_RENDERER_TYPE::ANIM_TRAIL;
			if (SupplementalTarget->eKind != EFFECT_ELEMENT_KIND::TRAIL ||
				SupplementalTarget->Renderer.eType != eExpectedRenderer ||
				SupplementalTarget->Renderer.eSourceSpace !=
					EFFECT_SOURCE_SPACE::UE3_CASCADE_V1)
			{
				strOutError =
					"Visual-program supplemental adapter packet renderer is stale.";
				return false;
			}
		}
		AdmittedSupplementalElements.push_back(Supplemental);
	}
	if (AdmittedRows.empty() && AdmittedSupplementalElements.empty())
	{
		strOutError = "Visual program admitted no executable rows.";
		return false;
	}
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				StagedDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		StagedDocument, strOutError))
	{
		return false;
	}
	const std::string ProjectedSha =
		Compute_DocumentCanonicalSha256(StagedDocument, strOutError);
	if (ProjectedSha.empty()) return false;
	if (Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::SOURCE_RECIPE_OVERLAY_V1)
	{
		if (ProjectedSha != Program->strProjectedDocumentTypedCodecSha256 ||
			nullptr == Program->pProjectedDocument ||
			CEffectDocumentCodec::Serialize(StagedDocument) !=
			CEffectDocumentCodec::Serialize(*Program->pProjectedDocument))
		{
			strOutError = "Visual-program projected document codec validation failed.";
			return false;
		}
	}

	std::string TokenMaterial =
		"effect-visual-program-admission-token-v1\n" +
		Program->strEffectAssetId + "\n" + Program->strProgramSha256 + "\n" +
		BaseSha + "\n" + ProjectedSha + "\n";
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : AdmittedRows)
	{
		TokenMaterial += Row.Selector.strEffectAssetId + "\n" +
			Row.Selector.strOccurrenceId + "\n" +
			Row.Selector.strSelectorSha256 + "\n" + Row.strRowSha256 + "\n" +
			Row.TargetIdentity->strTargetElementId + "\n" +
			Row.TargetIdentity->strTargetRecordSha256 + "\n" +
			Row.SourceIdentity.strSourceRecipeSha256 + "\n" +
			Row.SourceIdentity.strModuleClosureSha256 + "\n";
		if (Row.LocalDecalPacket.has_value())
			TokenMaterial += Row.LocalDecalPacket->strPacketSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		AdmittedSupplementalElements)
	{
		TokenMaterial += Supplemental.Selector.strEffectAssetId + "\n" +
			Supplemental.Selector.strOccurrenceId + "\n" +
			Supplemental.Selector.strSelectorSha256 + "\n" +
			Supplemental.strRowSha256 + "\n" +
			Supplemental.TargetIdentity.strTargetElementId + "\n" +
			Supplemental.TargetIdentity.strTargetRecordSha256 + "\n";
		if (Supplemental.CascadeRibbonPacket.has_value())
			TokenMaterial += Supplemental.CascadeRibbonPacket->strPacketSha256 + "\n";
		if (Supplemental.AnimationTrailPacket.has_value())
			TokenMaterial += Supplemental.AnimationTrailPacket->strPacketSha256 + "\n";
		if (Supplemental.BakedEdgeLightPacket.has_value())
			TokenMaterial += Supplemental.BakedEdgeLightPacket->strPacketSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_EDGE_HISTORY& History :
		Program->BakedEdgeHistories)
	{
		TokenMaterial += History.strHistoryId + "\n" +
			History.strHistorySha256 + "\n";
	}
	auto Projection = std::make_shared<EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>();
	Projection->m_pDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(StagedDocument));
	Projection->m_eProjectionKind = Program->eProjectionKind;
	Projection->m_strEffectAssetId = Program->strEffectAssetId;
	Projection->m_strProgramSha256 = Program->strProgramSha256;
	Projection->m_strBaseDocumentCanonicalSha256 = BaseSha;
	Projection->m_strProjectedDocumentSha256 = ProjectedSha;
	Projection->m_strAdmissionTokenSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(TokenMaterial);
	Projection->m_AdmittedSelectors = std::move(AdmittedSelectors);
	Projection->m_AdmittedRows = std::move(AdmittedRows);
	Projection->m_AdmittedSupplementalElements =
		std::move(AdmittedSupplementalElements);
	Projection->m_BakedEdgeHistories = Program->BakedEdgeHistories;
	if (!Projection->Is_Valid())
	{
		strOutError = "Visual-program admission token construction failed.";
		return false;
	}
	InOutProjection = std::move(Projection);
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Derive_TransformTunedProjection(
	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& SourceProjection,
	const EFFECT_DOCUMENT_DESC& TunedDocument,
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>& InOutProjection,
	std::string& strOutError)
{
	strOutError.clear();
	if (!SourceProjection.Is_Valid() ||
		TunedDocument.strEffectAssetId != SourceProjection.Get_EffectAssetId() ||
		(SourceProjection.Get_AdmittedRows().empty() &&
		 SourceProjection.Get_AdmittedSupplementalElements().empty()))
	{
		strOutError = "Transform-tuned visual projection source identity is invalid.";
		return false;
	}
	const bool_t bAdapterPacketProjection =
		SourceProjection.Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProjection)
	{
		if (!CEffectDocumentCodec::
			Validate_ReconstructedRuntimeDrawable(
				TunedDocument, strOutError))
		{
			return false;
		}
	}
	else if (!CEffectDocumentCodec::Validate_Drawable(
		TunedDocument, strOutError))
	{
		return false;
	}
	EFFECT_DOCUMENT_DESC Normalized = TunedDocument;
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		SourceProjection.Get_AdmittedSupplementalElements())
	{
		const EFFECT_ELEMENT_DESC* SourceElement = Find_Element(
			SourceProjection.Get_Document(),
			Supplemental.TargetIdentity.strTargetElementId);
		EFFECT_ELEMENT_DESC* NormalizedElement = Find_Element(
			Normalized, Supplemental.TargetIdentity.strTargetElementId);
		if (nullptr == SourceElement || nullptr == NormalizedElement)
		{
			strOutError = "Transform-tuned supplemental target join is missing.";
			return false;
		}
		if (Supplemental.bTuningEligibleTransform)
			NormalizedElement->Detail.Transform = SourceElement->Detail.Transform;
	}
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : SourceProjection.Get_AdmittedRows())
	{
		if (!Row.TargetIdentity.has_value())
		{
			strOutError = "Transform-tuned visual row target is missing.";
			return false;
		}
		const EFFECT_ELEMENT_DESC* SourceElement = Find_Element(
			SourceProjection.Get_Document(), Row.TargetIdentity->strTargetElementId);
		EFFECT_ELEMENT_DESC* NormalizedElement = Find_Element(
			Normalized, Row.TargetIdentity->strTargetElementId);
		if (nullptr == SourceElement || nullptr == NormalizedElement)
		{
			strOutError = "Transform-tuned visual target join is missing.";
			return false;
		}
		if (Row.bTuningEligibleTransform)
			NormalizedElement->Detail.Transform = SourceElement->Detail.Transform;
	}
	/* Normalizing every eligible target transform back to the source token
	   must make the complete typed document byte-identical.  This rejects
	   material/resource/recipe/attachment edits and transforms on ineligible
	   or fail-closed rows without maintaining a second field-by-field list. */
	if (CEffectDocumentCodec::Serialize(Normalized) !=
		CEffectDocumentCodec::Serialize(SourceProjection.Get_Document()))
	{
		strOutError =
			"Transform-tuned visual projection changed a non-eligible field.";
		return false;
	}
	const std::string TunedSha =
		Compute_DocumentCanonicalSha256(TunedDocument, strOutError);
	if (TunedSha.empty()) return false;
	auto Projection = std::make_shared<EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>();
	Projection->m_pDocument =
		std::make_shared<const EFFECT_DOCUMENT_DESC>(TunedDocument);
	Projection->m_eProjectionKind = SourceProjection.Get_ProjectionKind();
	Projection->m_strEffectAssetId = SourceProjection.Get_EffectAssetId();
	Projection->m_strProgramSha256 = SourceProjection.Get_ProgramSha256();
	Projection->m_strBaseDocumentCanonicalSha256 =
		SourceProjection.Get_BaseDocumentCanonicalSha256();
	Projection->m_strProjectedDocumentSha256 = TunedSha;
	Projection->m_AdmittedSelectors = SourceProjection.Get_AdmittedSelectors();
	Projection->m_AdmittedRows = SourceProjection.Get_AdmittedRows();
	Projection->m_AdmittedSupplementalElements =
		SourceProjection.Get_AdmittedSupplementalElements();
	Projection->m_BakedEdgeHistories =
		SourceProjection.Get_BakedEdgeHistories();
	std::string TokenMaterial =
		"effect-visual-program-transform-tuned-token-v1\n" +
		SourceProjection.Get_AdmissionTokenSha256() + "\n" + TunedSha + "\n";
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Projection->m_AdmittedRows)
	{
		TokenMaterial += Row.Selector.strEffectAssetId + "\n" +
			Row.Selector.strOccurrenceId + "\n" + Row.strRowSha256 + "\n";
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		Projection->m_AdmittedSupplementalElements)
	{
		TokenMaterial += Supplemental.Selector.strEffectAssetId + "\n" +
			Supplemental.Selector.strOccurrenceId + "\n" +
			Supplemental.strRowSha256 + "\n";
	}
	Projection->m_strAdmissionTokenSha256 =
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(TokenMaterial);
	if (!Projection->Is_Valid())
	{
		strOutError = "Transform-tuned visual admission token construction failed.";
		return false;
	}
	InOutProjection = std::move(Projection);
	return true;
}

bool_t Client::CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>&
		pProjection,
	const EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST& Request,
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE& InOutStage,
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == pProjection || !pProjection->Is_Valid())
	{
		strOutError =
			"Element authoring preset requires an admitted visual projection.";
		return false;
	}
	if (Request.strEffectAssetId.empty() || Request.strOccurrenceId.empty() ||
		Request.strRowSha256.empty() || Request.strTargetElementId.empty() ||
		Request.strSourceRecordId.empty())
	{
		strOutError =
			"Element authoring preset requires complete stable row identity.";
		return false;
	}
	if (Request.strEffectAssetId != pProjection->Get_EffectAssetId())
	{
		strOutError =
			"Element authoring preset effect identity does not match its projection.";
		return false;
	}

	const EFFECT_VISUAL_PROGRAM_ROW* pMatchedVisualRow = nullptr;
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pMatchedSupplemental =
		nullptr;
	size_t iExactMatchCount = 0u;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row :
		pProjection->Get_AdmittedRows())
	{
		if (Row.Selector.strEffectAssetId != Request.strEffectAssetId ||
			Row.Selector.strOccurrenceId != Request.strOccurrenceId ||
			Row.strRowSha256 != Request.strRowSha256 ||
			!Row.TargetIdentity.has_value() ||
			Row.TargetIdentity->strTargetElementId !=
				Request.strTargetElementId ||
			Row.SourceIdentity.strSourceRecordId != Request.strSourceRecordId)
		{
			continue;
		}
		if (Row.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Row.AdmissionBlockers.empty())
		{
			strOutError =
				"Element authoring preset visual row is not admitted.";
			return false;
		}
		pMatchedVisualRow = &Row;
		++iExactMatchCount;
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Supplemental :
		pProjection->Get_AdmittedSupplementalElements())
	{
		if (Supplemental.Selector.strEffectAssetId !=
				Request.strEffectAssetId ||
			Supplemental.Selector.strOccurrenceId != Request.strOccurrenceId ||
			Supplemental.strRowSha256 != Request.strRowSha256 ||
			Supplemental.TargetIdentity.strTargetElementId !=
				Request.strTargetElementId ||
			Supplemental.strSourceRecordId != Request.strSourceRecordId)
		{
			continue;
		}
		if (Supplemental.eDisposition !=
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
			!Supplemental.AdmissionBlockers.empty())
		{
			strOutError =
				"Element authoring preset supplemental row is not admitted.";
			return false;
		}
		pMatchedSupplemental = &Supplemental;
		++iExactMatchCount;
	}
	if (1u != iExactMatchCount)
	{
		strOutError =
			"Element authoring preset requires exactly one admitted row identity match.";
		return false;
	}
	const EFFECT_VISUAL_PROGRAM_FAMILY eMatchedFamily =
		nullptr != pMatchedVisualRow ? pMatchedVisualRow->eFamily :
			pMatchedSupplemental->eFamily;
	if (eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON &&
		eMatchedFamily != EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL)
	{
		strOutError =
			"Element authoring preset row family is not authoring-supported.";
		return false;
	}

	const EFFECT_DOCUMENT_DESC& SourceDocument = pProjection->Get_Document();
	const size_t iTargetCount = static_cast<size_t>(std::count_if(
		SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
		[&Request](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Request.strTargetElementId;
		}));
	if (1u != iTargetCount)
	{
		strOutError =
			"Element authoring preset requires exactly one projected target Element.";
		return false;
	}

	EFFECT_DOCUMENT_DESC GenericStartingCopy;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
		SourceDocument, Request.strTargetElementId, Request.strEffectAssetId,
		GenericStartingCopy, strOutError))
	{
		return false;
	}
	if (GenericStartingCopy.Elements.size() != 1u ||
		GenericStartingCopy.Elements.front().strElementId !=
			Request.strTargetElementId)
	{
		strOutError =
			"Element authoring preset generic copy returned an invalid target.";
		return false;
	}
	if (nullptr != pMatchedVisualRow &&
		pMatchedVisualRow->LocalDecalPacket.has_value())
	{
		if (!Append_LocalDecalRoleTextureEvidence(
				*pMatchedVisualRow->LocalDecalPacket,
				GenericStartingCopy.Elements.front(), strOutError) ||
			!CEffectDocumentCodec::Validate_Drawable(
				GenericStartingCopy, strOutError))
		{
			return false;
		}
	}

	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Candidate;
	Candidate.Element = std::move(GenericStartingCopy.Elements.front());
	Candidate.eSourceFamily = eMatchedFamily;
	Candidate.bSupplemental = nullptr != pMatchedSupplemental;
	Candidate.pProjection = pProjection;
	Candidate.Identity = Request;
	if (!Candidate.Is_Valid())
	{
		strOutError = "Element authoring preset stage validation failed.";
		return false;
	}

	InOutStage = std::move(Candidate);
	strOutError.clear();
	return true;
}
