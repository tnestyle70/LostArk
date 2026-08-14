#include "Effect_RuntimeAuthority.h"

#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <cctype>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace Client;

	constexpr std::string_view SEMANTIC_AUTHORITY =
		"IMMUTABLE_COMPILED_IR";
	constexpr std::string_view CODE_ONLY_PUBLICATION_STATE =
		"CODE_ONLY_NOT_ADMITTED";
	constexpr std::string_view TYPED_ADAPTER_PENDING =
		"TYPED_RUNTIME_PROGRAM_ADAPTER_PENDING";
	constexpr std::string_view EXTERNAL_AUTHENTICATION_PENDING =
		"COMPILED_AUTHORITY_EXTERNAL_AUTHENTICATION_PENDING";
	constexpr std::string_view RECONSTRUCTED_PROGRAM_SCHEMA =
		"lostark.artist-31470-reconstructed-runtime-program";
	constexpr std::string_view RECONSTRUCTED_PROGRAM_ID =
		"effect.artist.skill.31470.reconstructed-approved-v1";
	constexpr std::string_view RECONSTRUCTED_PROGRAM_ROLE =
		"OFFLINE_IMMUTABLE_TYPED_PROGRAM_NOT_RUNTIME_OR_PRODUCT_AUTHORITY";

	// This profile is intentionally the only candidate-specific switch point.
	// It pins the final Material 77/77 corrective. The parser never accepts an
	// earlier profile or a range of texture-resolution states.
	struct RECONSTRUCTED_PROGRAM_PROFILE final
	{
		std::string_view BuilderCommitId;
		std::string_view BuilderTreeId;
		std::string_view CandidateRawSha256;
		std::string_view ProgramSha256;
		std::string_view BlockerOwnershipSha256;
		std::string_view InputArtifactsSha256;
		std::string_view MaterialTextureBindingsSha256;
		std::string_view CapabilitySamplesSha256;
		std::string_view DistributionSamplesSha256;
		std::string_view DistributionProvenanceSha256;
		std::string_view MaterialFamilySamplesSha256;
		std::string_view MaterialRecipeSamplesSha256;
		std::string_view ActionSchedulesSha256;
		std::string_view ActionCueProjectionsSha256;
		std::string_view RendererConfigProjectionsSha256;
		std::string_view RendererTextureResourcesSha256;
		std::string_view GeometryCarriersSha256;
		size_t iCandidateByteCount;
		uint32_t iBlockerOwnerFieldCount;
		uint32_t iBlockerTokenOccurrenceCount;
		uint32_t iResolvedTextureCount;
		uint32_t iUnresolvedTextureCount;
		uint32_t iRuntimeCookReceiptCount;
		uint32_t iReconstructedDeploymentReceiptCount;
		uint32_t iFollowAttachmentCount;
		uint32_t iSnapshotRootAttachmentCount;
		uint32_t iDisabledRootAttachmentCount;
		EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS eRequiredTextureStatus;
	};

	constexpr RECONSTRUCTED_PROGRAM_PROFILE ACTIVE_RECONSTRUCTED_PROFILE{
		"ddef21a5314eb8c3db891d36f702cfeda3149f20",
		"36a36b889dae7be092e0d2f6f3c3aee2c28bc462",
		"430ed1aa42a34e23d1f216a69c6f51e81a8cbcdbb03318930894e0dfe16cd6c6",
		"0666164bce946fd3b7e72dd92422b21a13e58d3388a3e3264ab30b8065e9c802",
		"1fd6038ee3eb09c68d0721cc819e605ce82301c5191fd6e1f5d9b2a03ec5f0ff",
		"ca59713e1bd95fa83224db93df8a712e010cdb8dba684a7a4985b9f9a16ca45f",
		"a265a6f6a24aa81b608cb0b8ddbe2957c73e242cee6497b36bf66a27b581d6bb",
		"d367b8c6f8134bf70fb16bbb7bf071f06bcf0bfcb5b459dad0c687effecbbc94",
		"7e4abfe5346568f053859fc717be94ff9a8b750cd64edad4128288a7adb6d546",
		"07cd536fb3461b2b2cf5b51ab3c90b2a8a9db3d3dc055bca26476625709f178d",
		"5350e20004898130541c6dc147ce2c14fa8a7870bee36a9baeaf14dcda33b5d3",
		"23977bf027c015086393b1f4b2a9495313a992025ffdc4ad3ac769e20595f69b",
		"94b0bc3704e4c9aaca5216cfafbfcb397c924390395597442c6831d096f2bff5",
		"c85338bdeb9ebb997dbc629a0883772a3b64a4496c4c843a3bac93cfbed3208e",
		"42cef303f171ec7be0fc523dfcf78a6f8c86698cde7cb55a8e8c18a52ff222c7",
		"6c4ff35e5323f81e1c7556167e3264fc83c57d8e9390cf89b10c3ac5ac01c023",
		"a602be873f49bb21954a0554c467c6945d26c328a5f46218af7ad6cf22254ba3",
		15'121'873u,
		6133u,
		6775u,
		77u,
		0u,
		73u,
		4u,
		5u,
		30u,
		0u,
		EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET,
	};

	constexpr std::array<std::string_view, 13u> REQUIRED_GLOBAL_PROGRAM_BLOCKERS{
		"CATALOG_PUBLISH_TRANSACTION_NOT_RUN",
		"EXECUTION_ADMISSION_FALSE",
		"MANUAL_HUMAN_EYE_VALIDATION_0_OF_35",
		"PRODUCT_ADMISSION_NOT_OPEN",
		"PRODUCT_ORACLE_35_OF_35_NOT_PASSED",
		"R3_TYPED_EXECUTOR_NOT_COMPLETE",
		"R4_GEOMETRY_AND_MATERIAL_RUNTIME_BINDING_NOT_COMPLETE",
		"R5_SIX_RENDERER_FAMILIES_NOT_COMPLETE",
		"R6_35_OCCURRENCE_RUNTIME_AND_MANUAL_VALIDATION_NOT_COMPLETE",
		"R7_FREEZE_BUILD_AND_TRANSACTION_REGRESSION_NOT_COMPLETE",
		"RENDERER_POLICY_CONSUMER_NOT_IMPLEMENTED",
		"SOURCE_FIDELITY_BLOCKERS_PRESERVED",
		"TYPED_RUNTIME_MATERIAL_POLICY_CONSUMER_NOT_IMPLEMENTED",
	};

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const DATA_JSON_TYPE Type)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		return nullptr != Value && Value->Get_Type() == Type ? Value : nullptr;
	}

	bool_t Has_ExactKeys(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(),
			[&Value](const std::string_view Key)
			{
				return Value.Get_Object().contains(Key);
			});
	}

	[[maybe_unused]] bool_t Has_ExactKeyOrder(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size() ||
			Value.Get_ObjectInsertionOrder().size() != Keys.size())
		{
			return false;
		}
		const std::vector<std::string>& Actual =
			Value.Get_ObjectInsertionOrder();
		size_t Index = 0u;
		for (const std::string_view Expected : Keys)
		{
			if (Actual[Index++] != Expected ||
				!Value.Get_Object().contains(Expected))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_Sha256(const std::string_view Value)
	{
		return Value.size() == 64u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool_t Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 128u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	[[maybe_unused]] bool_t Read_String(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue,
		const bool_t bAllowEmpty = false,
		const size_t MaximumLength = 4096u)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().size() > MaximumLength ||
			(!bAllowEmpty && Value->Get_String().empty()))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Read_FiniteF64(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		double& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || !Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()))
			return false;
		OutValue = Value->Get_Number();
		return true;
	}

	[[maybe_unused]] bool_t Parse_StringArray(
		const DATA_JSON_VALUE& Value,
		std::vector<std::string>& OutValues,
		const bool_t bAllowEmptyItems = false,
		const size_t MaximumCount = 4096u,
		const size_t MaximumItemLength = 4096u)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() > MaximumCount)
			return false;
		std::vector<std::string> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_String() ||
				Item.Get_String().size() > MaximumItemLength ||
				(!bAllowEmptyItems && Item.Get_String().empty()))
			{
				return false;
			}
			Staged.push_back(Item.Get_String());
		}
		OutValues = std::move(Staged);
		return true;
	}

	[[maybe_unused]] bool_t Build_ObjectWithoutKey(
		const DATA_JSON_VALUE& Value,
		const std::string_view RemovedKey,
		DATA_JSON_VALUE& OutValue)
	{
		if (!Value.Is_Object() || !Value.Get_Object().contains(RemovedKey))
			return false;
		DATA_JSON_VALUE::OBJECT Object = Value.Get_Object();
		Object.erase(std::string(RemovedKey));
		std::vector<std::string> Order;
		Order.reserve(Value.Get_ObjectInsertionOrder().size() - 1u);
		for (const std::string& Key : Value.Get_ObjectInsertionOrder())
		{
			if (Key != RemovedKey)
				Order.push_back(Key);
		}
		if (Order.size() != Object.size())
			return false;
		OutValue = DATA_JSON_VALUE::Object(
			std::move(Object), std::move(Order));
		return true;
	}

	[[maybe_unused]] bool_t Build_ObjectProjection(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<std::string_view> Keys,
		DATA_JSON_VALUE& OutValue)
	{
		if (!Value.Is_Object())
			return false;
		DATA_JSON_VALUE::OBJECT Object;
		std::vector<std::string> Order;
		Order.reserve(Keys.size());
		for (const std::string_view Key : Keys)
		{
			const DATA_JSON_VALUE* Field = Value.Find(Key);
			if (nullptr == Field ||
				!Object.emplace(std::string(Key), *Field).second)
			{
				return false;
			}
			Order.emplace_back(Key);
		}
		OutValue = DATA_JSON_VALUE::Object(std::move(Object), std::move(Order));
		return true;
	}

	[[maybe_unused]] bool_t Verify_CanonicalSelfSha256(
		const DATA_JSON_VALUE& Value,
		const std::string_view HashField,
		std::string& OutHash)
	{
		const DATA_JSON_VALUE* Hash = Required(
			Value, HashField, DATA_JSON_TYPE::STRING);
		DATA_JSON_VALUE Unsigned;
		if (nullptr == Hash || !Is_Sha256(Hash->Get_String()) ||
			!Build_ObjectWithoutKey(Value, HashField, Unsigned))
		{
			return false;
		}
		const std::string Computed =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Unsigned));
		if (Computed != Hash->Get_String())
			return false;
		OutHash = Computed;
		return true;
	}

	[[maybe_unused]] bool_t Compute_OrderedSectionSha256(
		const DATA_JSON_VALUE& Section,
		std::string& OutHash)
	{
		if (!Section.Is_Array())
			return false;
		DATA_JSON_VALUE::ARRAY RowHashes;
		RowHashes.reserve(Section.Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section.Get_Array())
		{
			const DATA_JSON_VALUE* RowHash = Required(
				Row, "rowSha256", DATA_JSON_TYPE::STRING);
			if (nullptr == RowHash || !Is_Sha256(RowHash->Get_String()))
				return false;
			RowHashes.push_back(DATA_JSON_VALUE::String(RowHash->Get_String()));
		}
		OutHash = CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
				DATA_JSON_VALUE::Array(std::move(RowHashes))));
		return Is_Sha256(OutHash);
	}

	[[maybe_unused]] bool_t Is_ValidUtf8(const std::string_view Text)
	{
		for (size_t Index = 0u; Index < Text.size();)
		{
			const uint8_t First = static_cast<uint8_t>(Text[Index++]);
			if (First <= 0x7fu)
				continue;
			const auto TakeContinuation = [&Text, &Index](
				uint8_t& OutByte) -> bool_t
			{
				if (Index >= Text.size())
					return false;
				OutByte = static_cast<uint8_t>(Text[Index++]);
				return (OutByte & 0xc0u) == 0x80u;
			};
			uint8_t Second = 0u;
			uint8_t Third = 0u;
			uint8_t Fourth = 0u;
			if (First >= 0xc2u && First <= 0xdfu)
			{
				if (!TakeContinuation(Second))
					return false;
				continue;
			}
			if (First >= 0xe0u && First <= 0xefu)
			{
				if (!TakeContinuation(Second) ||
					!TakeContinuation(Third) ||
					(First == 0xe0u && Second < 0xa0u) ||
					(First == 0xedu && Second > 0x9fu))
				{
					return false;
				}
				continue;
			}
			if (First >= 0xf0u && First <= 0xf4u)
			{
				if (!TakeContinuation(Second) ||
					!TakeContinuation(Third) ||
					!TakeContinuation(Fourth) ||
					(First == 0xf0u && Second < 0x90u) ||
					(First == 0xf4u && Second > 0x8fu))
				{
					return false;
				}
				continue;
			}
			return false;
		}
		return true;
	}

	[[maybe_unused]] bool_t Parse_BoundedRuntimeProgramJson(
		const std::string_view Text,
		DATA_JSON_VALUE& OutValue,
		std::string& OutError)
	{
		constexpr size_t MAXIMUM_PROGRAM_BYTES = 20u * 1024u * 1024u;
		constexpr size_t MAXIMUM_PROGRAM_DEPTH = 64u;
		constexpr size_t MAXIMUM_PROGRAM_VALUES = 1'000'000u;
		if (Text.size() > MAXIMUM_PROGRAM_BYTES)
		{
			OutError = "Reconstructed runtime program exceeds byte limit.";
			return false;
		}
		if (Text.size() >= 3u &&
			static_cast<unsigned char>(Text[0]) == 0xefu &&
			static_cast<unsigned char>(Text[1]) == 0xbbu &&
			static_cast<unsigned char>(Text[2]) == 0xbfu)
		{
			OutError = "Reconstructed runtime program UTF-8 BOM is forbidden.";
			return false;
		}
		if (!Is_ValidUtf8(Text))
		{
			OutError = "Reconstructed runtime program must be valid UTF-8.";
			return false;
		}
		DATA_JSON_PARSE_LIMITS Limits;
		Limits.iMaximumBytes = MAXIMUM_PROGRAM_BYTES;
		Limits.iMaximumDepth = MAXIMUM_PROGRAM_DEPTH;
		Limits.iMaximumValues = MAXIMUM_PROGRAM_VALUES;
		DATA_JSON_VALUE Staged;
		if (!CDataJson::Parse(Text, Staged, OutError, Limits) ||
			!Staged.Is_Object())
		{
			if (OutError.empty())
				OutError = "Reconstructed runtime program root must be an object.";
			return false;
		}
		OutValue = std::move(Staged);
		return true;
	}

	bool_t Read_U32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		uint32_t& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() != std::floor(Value->Get_Number()) ||
			Value->Get_Number() < 0.0 ||
			Value->Get_Number() > static_cast<double>(UINT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(Value->Get_Number());
		return true;
	}

	[[maybe_unused]] bool_t Read_Boolean(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		bool_t& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == Value)
			return false;
		OutValue = Value->Get_Boolean();
		return true;
	}

	[[maybe_unused]] bool_t Read_ClosedString(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const std::initializer_list<std::string_view> Allowed,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || std::find(
			Allowed.begin(), Allowed.end(), Value->Get_String()) == Allowed.end())
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Parse_FiniteF64Array(
		const DATA_JSON_VALUE& Value,
		std::vector<double>& OutValues,
		const size_t MinimumCount,
		const size_t MaximumCount)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() < MinimumCount ||
			Value.Get_Array().size() > MaximumCount)
		{
			return false;
		}
		std::vector<double> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_Number() || !Item.Was_FloatingPointToken() ||
				!std::isfinite(Item.Get_Number()))
				return false;
			Staged.push_back(Item.Get_Number());
		}
		OutValues = std::move(Staged);
		return true;
	}

	[[maybe_unused]] bool_t Parse_U32Array(
		const DATA_JSON_VALUE& Value,
		std::vector<uint32_t>& OutValues,
		const size_t MinimumCount,
		const size_t MaximumCount)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() < MinimumCount ||
			Value.Get_Array().size() > MaximumCount)
		{
			return false;
		}
		std::vector<uint32_t> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_Number() || Item.Was_FloatingPointToken() ||
				!std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() != std::floor(Item.Get_Number()) ||
				Item.Get_Number() < 0.0 ||
				Item.Get_Number() > static_cast<double>(UINT32_MAX))
			{
				return false;
			}
			Staged.push_back(static_cast<uint32_t>(Item.Get_Number()));
		}
		OutValues = std::move(Staged);
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableString(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<std::string>& OutValue,
		const size_t MaximumLength = 4096u)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_String() || Value->Get_String().empty() ||
			Value->Get_String().size() > MaximumLength)
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableSha256(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<std::string>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_String() || !Is_Sha256(Value->Get_String()))
			return false;
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableBoolean(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<bool_t>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_Boolean())
			return false;
		OutValue = Value->Get_Boolean();
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableF64(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<double>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_Number() || !Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()))
		{
			return false;
		}
		OutValue = Value->Get_Number();
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableU32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<uint32_t>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		uint32_t Parsed = 0u;
		if (!Read_U32(Object, Name, Parsed))
			return false;
		OutValue = Parsed;
		return true;
	}

	[[maybe_unused]] bool_t Read_I32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		int32_t& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || Value->Was_FloatingPointToken() ||
			!std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() != std::floor(Value->Get_Number()) ||
			Value->Get_Number() < static_cast<double>(INT32_MIN) ||
			Value->Get_Number() > static_cast<double>(INT32_MAX))
		{
			return false;
		}
		OutValue = static_cast<int32_t>(Value->Get_Number());
		return true;
	}

	[[maybe_unused]] bool_t Read_LowerHex(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const size_t CharacterCount,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().size() != CharacterCount ||
			!std::all_of(Value->Get_String().begin(), Value->Get_String().end(),
				[](const char Character)
				{
					return (Character >= '0' && Character <= '9') ||
						(Character >= 'a' && Character <= 'f');
				}))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Parse_I32Array(
		const DATA_JSON_VALUE& Value,
		std::vector<int32_t>& OutValues,
		const size_t MinimumCount,
		const size_t MaximumCount)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() < MinimumCount ||
			Value.Get_Array().size() > MaximumCount)
		{
			return false;
		}
		std::vector<int32_t> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_Number() || Item.Was_FloatingPointToken() ||
				!std::isfinite(Item.Get_Number()) ||
				Item.Get_Number() != std::floor(Item.Get_Number()) ||
				Item.Get_Number() < static_cast<double>(INT32_MIN) ||
				Item.Get_Number() > static_cast<double>(INT32_MAX))
			{
				return false;
			}
			Staged.push_back(static_cast<int32_t>(Item.Get_Number()));
		}
		OutValues = std::move(Staged);
		return true;
	}

	template <size_t Count>
	[[maybe_unused]] bool_t Parse_FixedF64Array(
		const DATA_JSON_VALUE& Value,
		std::array<double, Count>& OutValues)
	{
		std::vector<double> Staged;
		if (!Parse_FiniteF64Array(Value, Staged, Count, Count))
			return false;
		std::copy(Staged.begin(), Staged.end(), OutValues.begin());
		return true;
	}

	template <size_t Count>
	[[maybe_unused]] bool_t Parse_FixedU32Array(
		const DATA_JSON_VALUE& Value,
		std::array<uint32_t, Count>& OutValues)
	{
		std::vector<uint32_t> Staged;
		if (!Parse_U32Array(Value, Staged, Count, Count))
			return false;
		std::copy(Staged.begin(), Staged.end(), OutValues.begin());
		return true;
	}

	template <size_t Count>
	[[maybe_unused]] bool_t Parse_FixedLowerHexArray(
		const DATA_JSON_VALUE& Value,
		const size_t CharacterCount,
		std::array<std::string, Count>& OutValues)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() != Count)
			return false;
		for (size_t Index = 0u; Index < Count; ++Index)
		{
			const DATA_JSON_VALUE& Item = Value.Get_Array()[Index];
			if (!Item.Is_String() || Item.Get_String().size() != CharacterCount ||
				!std::all_of(Item.Get_String().begin(), Item.Get_String().end(),
					[](const char Character)
					{
						return (Character >= '0' && Character <= '9') ||
							(Character >= 'a' && Character <= 'f');
					}))
			{
				return false;
			}
			OutValues[Index] = Item.Get_String();
		}
		return true;
	}

	[[maybe_unused]] bool_t Read_RuntimeId(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().empty() ||
			Value->Get_String().size() > 1024u ||
			std::any_of(Value->Get_String().begin(), Value->Get_String().end(),
				[](const unsigned char Character)
				{
					return Character < 0x20u || Character == 0x7fu;
				}))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Is_NormalizedRelativePath(
		const std::string_view Value,
		const std::string_view RequiredPrefix = {})
	{
		if (Value.empty() || Value.size() > 4096u || Value.front() == '/' ||
			Value.back() == '/' || Value.find('\\') != std::string_view::npos ||
			Value.find(':') != std::string_view::npos ||
			(!RequiredPrefix.empty() && !Value.starts_with(RequiredPrefix)) ||
			std::any_of(Value.begin(), Value.end(),
				[](const unsigned char Character)
				{
					return Character < 0x20u || Character == 0x7fu;
				}))
		{
			return false;
		}

		size_t SegmentStart = 0u;
		while (SegmentStart < Value.size())
		{
			const size_t Slash = Value.find('/', SegmentStart);
			const size_t SegmentEnd =
				Slash == std::string_view::npos ? Value.size() : Slash;
			const std::string_view Segment =
				Value.substr(SegmentStart, SegmentEnd - SegmentStart);
			if (Segment.empty() || Segment == "." || Segment == "..")
				return false;
			if (Slash == std::string_view::npos)
				break;
			SegmentStart = Slash + 1u;
		}
		return true;
	}

	[[maybe_unused]] bool_t Read_NormalizedRelativePath(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const std::string_view RequiredPrefix,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value ||
			!Is_NormalizedRelativePath(Value->Get_String(), RequiredPrefix))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Read_NullableNormalizedRelativePath(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const std::string_view RequiredPrefix,
		std::optional<std::string>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_String() ||
			!Is_NormalizedRelativePath(Value->Get_String(), RequiredPrefix))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	[[maybe_unused]] bool_t Verify_SealedRow(
		const DATA_JSON_VALUE& Row,
		const std::initializer_list<std::string_view> Keys,
		std::string& OutRowSha256)
	{
		return Has_ExactKeyOrder(Row, Keys) &&
			Verify_CanonicalSelfSha256(Row, "rowSha256", OutRowSha256);
	}

	struct RUNTIME_SECTION_SEAL final
	{
		std::string_view Name;
		uint32_t iRowCount = 0u;
		std::string strOrderedSha256;
	};

	[[maybe_unused]] bool_t Validate_SealedSection(
		const DATA_JSON_VALUE& Root,
		const std::string_view Name,
		const uint32_t ExpectedCount,
		const std::initializer_list<std::string_view> RowKeys,
		RUNTIME_SECTION_SEAL& OutSeal)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section || Section->Get_Array().size() != ExpectedCount)
			return false;
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			std::string RowSha256;
			if (!Verify_SealedRow(Row, RowKeys, RowSha256))
				return false;
		}
		std::string OrderedSha256;
		if (!Compute_OrderedSectionSha256(*Section, OrderedSha256))
			return false;
		OutSeal.Name = Name;
		OutSeal.iRowCount = ExpectedCount;
		OutSeal.strOrderedSha256 = std::move(OrderedSha256);
		return true;
	}

	[[maybe_unused]] bool_t Validate_RuntimeSectionSeals(
		const DATA_JSON_VALUE& Root,
		std::vector<RUNTIME_SECTION_SEAL>& OutSeals)
	{
		std::vector<RUNTIME_SECTION_SEAL> Staged;
		Staged.reserve(23u);
		const auto Add = [&Root, &Staged](
			const std::string_view Name,
			const uint32_t Count,
			const std::initializer_list<std::string_view> Keys) -> bool_t
		{
			RUNTIME_SECTION_SEAL Seal;
			if (!Validate_SealedSection(Root, Name, Count, Keys, Seal))
				return false;
			Staged.push_back(std::move(Seal));
			return true;
		};

		if (!Add("inputArtifacts", 13u, {
			"artifactId", "order", "path", "authorityCommitId",
			"authorityTreeId", "blobId", "schema", "versionField",
			"versionValue", "hashDomain", "trackedTextSha256",
			"canonicalJsonSha256", "selfHashField", "selfSha256",
			"rowSha256" }) ||
			!Add("handlerRegistry", 385u, {
				"handlerRegistryId", "order", "handlerKind", "implementationId",
				"implementationVersion", "implementationSha256", "exactSourceClass",
				"variant", "consumerContract", "contractSha256", "sourceExact",
				"rowSha256" }) ||
			!Add("emitters", 35u, {
				"emitterId", "order", "evidenceId", "sourceOccurrenceId",
				"sourceSystemId", "sourceEmitterPath", "sourceEmitterNodeId",
				"sourceCueId", "sourceElementId", "sourceNode",
				"sourceActionCueProjectionSha256", "visible", "rendererType",
				"rendererSourceSpace", "selectedLodPath", "selectedLodNodeId",
				"selectedLodRecordSha256", "selectedLodDecision",
				"selectedLodOracleId", "sourceRecipeEnabled", "localSpace",
				"sizeUnitPolicy", "operationalMaxParticles",
				"sourcePeakActiveParticles", "spawnRateFallbackPolicy",
				"lifetimeFallbackPolicy", "operationalCapPolicy",
				"operationalCapProjectionSha256", "operationalCapBlockers",
				"rendererRuntimeConfig", "screenPostAdapter", "lightAdapter",
				"decalAdapter", "ribbonAdapter", "timing", "random",
				"cueLocalTransform", "detailTransform", "actionCueAttachment",
				"transformInheritance", "transformCompositionOrder",
				"anchorRequests", "moduleIds", "actionCueParameterInputs",
				"scheduleId", "materialOccurrenceId", "textureResourceIds",
				"geometryUseId", "sourceExact", "rowSha256" }) ||
			!Add("actionSchedules", 7u, {
				"scheduleId", "order", "sourceCueId", "sourceOccurrenceId",
				"sourceSystemId", "sourceReceiptEventIndex", "globalTimeSeconds",
				"durationSeconds", "sourceCueRowSha256", "rowSha256" }) ||
			!Add("modules", 399u, {
				"moduleId", "emitterId", "order", "sourceObjectId",
				"sourceRecordSha256", "exactSourceClass", "selectionKind",
				"handlerRegistryId", "handlerVariant", "capabilityPolicyFamilyId",
				"capabilityImplementationId", "capabilityImplementationVersion",
				"capabilityImplementationSha256", "capabilityFamilySemanticSha256",
				"capabilityInputSchemaSha256", "capabilityOutputSchemaSha256",
				"capabilityDefaultPolicySha256", "capabilityNumericSamples",
				"capabilitySourceRowSha256", "capabilityLiteralBindingsSha256",
				"capabilityDistributionBindingsSha256",
				"capabilityPropertyConsumptionSha256", "capabilitySeedBindingSha256",
				"capabilityActionCueInputNames", "capabilityActionCueInputsSha256",
				"approvalPolicyRowId", "approvalUpstreamIdentitySha256",
				"approvalPolicyBindingSha256", "approvalRequiredMutatedOutput",
				"approvalSourceOrder", "approvalRequiredOracleIds",
				"approvalExecutionBlockers", "approvalSourceRow",
				"approvalSourceRowSha256", "propertyIds", "primitiveLeafIds",
				"literalIds", "distributionIds", "seedPolicyId",
				"implicitDefaultIds", "sourceDecision", "preservedBlockers",
				"sourceExact", "rowSha256" }) ||
			!Add("properties", 1434u, {
				"propertyId", "moduleId", "order", "propertyPath",
				"handlerRegistryId", "consumptionDecision", "irrelevanceOracleId",
				"payloadLiteralIds", "payloadDistributionIds",
				"semanticDistributionIds", "sourceFidelity",
				"capabilityConsumptionDecision", "semanticRole",
				"outputDependencyRequired", "capabilitySourceRowSha256",
				"preservedBlockers", "sourceExact", "rowSha256" }) ||
			!Add("primitiveLeaves", 1572u, {
				"leafId", "moduleId", "propertyId", "order", "propertyPath",
				"topLevelPropertyPath", "literalId", "valueVariant",
				"handlerRegistryId", "consumptionDecision", "preservedBlockers",
				"sourceExact", "rowSha256" }) ||
			!Add("literals", 1590u, {
				"literalId", "moduleId", "propertyId", "order", "propertyPath",
				"variant", "boolValue", "f64Value", "enumStringValue",
				"sourceExact", "rowSha256" }) ||
			!Add("distributions", 629u, {
				"distributionId", "moduleId", "propertyId", "order", "variant",
				"evaluatorRegistryId", "payloadDistributionId", "propertyPath",
				"sourceClass", "sourceObjectPath", "referenceId", "occurrenceId",
				"payloadStatus", "fidelity", "componentCount", "operation",
				"randomLockAxes", "lookupTableChunkSize", "lookupTableNumElements",
				"lookupTableTimeScale", "lookupTableStartTime", "defaultMinimum",
				"defaultMaximum", "lookupTable", "curveKeys", "parameterName",
				"paramModes", "minimumInput", "maximumInput", "minimumOutput",
				"maximumOutput", "constantValues", "isDirty", "actionCueBindings",
				"fieldProvenance", "samples", "capabilityImplementationId",
				"capabilityImplementationVersion", "capabilityImplementationSha256",
				"preservedBlockers", "sourceExact", "rowSha256" }) ||
			!Add("seedPolicies", 14u, {
				"seedPolicyId", "moduleId", "order", "evaluatorId", "randomSeeds",
				"parameterName", "getSeedFromInstance", "instanceSeedIsIndex",
				"resetSeedOnEmitterLooping", "randomlySelectSeedArray",
				"emptyArrayUsesOccurrenceRandomStream", "currentCdoEvidenceKey",
				"source", "sourceFidelity", "sourceExact", "rowSha256" }) ||
			!Add("implicitDefaults", 14u, {
				"defaultId", "moduleId", "order", "family", "fieldPath",
				"variant", "boolValue", "ribbonValues", "decalValues", "decision",
				"provenance", "reason", "valuesOwnedBy", "sourceExact",
				"rowSha256" }) ||
			!Add("pointLightFields", 8u, {
				"fieldId", "moduleId", "order", "fieldPath", "valueVariant",
				"boolValue", "f64Value", "colorRgba8Value", "guid128Value",
				"sourceTier", "sourceFidelity", "decision", "oracleId",
				"sourceExact", "rowSha256" }) ||
			!Add("materialFamilies", 23u, {
				"familyId", "order", "familyIdentitySha256", "evaluatorRegistryId",
				"evaluatorId", "evaluatorVersion", "evaluatorSha256",
				"rendererShapes", "featureMask", "features", "graphProvenance",
				"cpuNumericOracleVerified", "hlslNumericOracleVerified",
				"numericSamples", "sampleProjectionSha256", "approvalPolicyRowId",
				"approvalArithmeticOrder", "approvalPolicyFamilyId",
				"approvalRequiredOracleIds", "approvalExecutionBlockers",
				"approvalArithmeticRow", "approvalArithmeticRowSha256",
				"preservedBlockers", "sourceExact", "rowSha256" }) ||
			!Add("materialRecipes", 27u, {
				"recipeId", "order", "sourceMaterialPath",
				"sourceRecipeCompositionSha256", "familyId", "evaluatorRegistryId",
				"inputIds", "staticBindingIds", "renderBindingIds",
				"numericBindingSamples", "bindingSha256", "preservedBlockers",
				"sourceExact", "rowSha256" }) ||
			!Add("materialInputs", 729u, {
				"fieldId", "recipeId", "order", "fieldKind", "bindingRole",
				"bindingOrigin", "sourceSection", "sourceSectionIndex",
				"parameterName", "normalizedParameterName", "valueVariant",
				"valueF64", "valueF64x4", "valueTextureId", "typedValueSha256",
				"sourceFieldValueSha256", "sourceLineageSha256", "sourceExact",
				"rowSha256" }) ||
			!Add("materialStaticBindings", 94u, {
				"fieldId", "recipeId", "order", "parameterName",
				"normalizedParameterName", "bindingOrigin", "selectionRole",
				"sourceValue", "selectedValue", "policyRowId",
				"sourceFieldValueSha256", "sourceLineageSha256", "sourceExact",
				"rowSha256" }) ||
			!Add("materialRenderBindings", 162u, {
				"renderBindingId", "recipeId", "order", "fieldName",
				"bindingOrigin", "sourceStatus", "sourceFidelity", "valueVariant",
				"boolValue", "enumValue", "f64Value", "sourceRecordSha256",
				"policyRowId", "sourceBlocker", "sourceExact", "rowSha256" }) ||
			!Add("materialOccurrences", 34u, {
				"occurrenceId", "order", "emitterId", "cueId", "rendererType",
				"recipeId", "familyId", "evaluatorRegistryId",
				"sourceOccurrenceIdentitySha256", "bindingSha256",
				"sourceOccurrenceBindingSha256", "preservedBlockers",
				"sourceExact", "rowSha256" }) ||
			!Add("materialPolicyRows", 260u, {
				"policyRowId", "order", "domain", "sourceMatrixRowId", "recipeId",
				"materialOccurrenceIds", "fieldId", "fieldKind", "bindingOrigin",
				"evidenceOwnerRecipeId", "policyFidelity", "sourceExact",
				"evidenceBlockers", "policySelectionAdmission",
				"implementationRegistryId", "implementationId",
				"implementationVersion", "consumerContract", "valueVariant",
				"boolValue", "f64Value", "enumType", "enumValue", "enumOrdinal",
				"samplerDescriptor", "providerBasisSha256", "numericOracleSha256",
				"d3dStateOracleId", "d3dDescriptorOracle", "d3dSrvOracle",
				"approvalPolicyRowId", "approvalUpstreamIdentitySha256",
				"approvalPolicyBindingSha256", "approvalPolicyFamilyId",
				"approvalMaterialOrder", "approvalRequiredOracleIds",
				"approvalExecutionBlockers", "approvalMaterialRow",
				"approvalMaterialRowSha256", "sourceRowSha256", "rowSha256" }) ||
			!Add("materialTextureBindings", 77u, {
				"bindingId", "order", "recipeId", "materialInputFieldId",
				"logicalTexturePath", "samplerPolicyRowId",
				"materialOccurrenceIds", "sourceBindingId",
				"sourceBindingRowSha256", "sourceTextureResourceId",
				"sourceTextureResourceRowSha256", "sourceProvisioningProposalId",
				"sourceProvisioningProposalRowSha256", "sourceDeploymentRowId",
				"sourceDeploymentRowSha256", "sourceReceiptStatus", "runtimeAssetId",
				"resolutionStatus", "bindingBasis", "sourceExact", "blockers",
				"rowSha256" }) ||
			!Add("rendererTextureResources", 57u, {
				"textureResourceId", "order", "emitterId", "sourceNode",
				"materialOccurrenceId", "sourceMaterialPath", "slotId", "assetId",
				"sourceResourceProjectionSha256", "projectionRole", "blockers",
				"sourceExact", "rowSha256" }) ||
			!Add("geometryCarriers", 7u, {
				"carrierId", "order", "bindingId", "sourceObject", "assetId",
				"formatVersion", "candidateResourceByteSize",
				"candidateResourceSha256", "payloadSha256", "provenanceSha256",
				"provenanceRole", "metadataIdentitySha256", "cacheIdentitySha256",
				"geometryPreScale", "geometryPreScaleF32Hex", "channelMask",
				"evidenceFlags", "expectedTupleSha256", "submeshes",
				"approvalGeometryOrder", "approvalGeometryRow",
				"approvalGeometryRowSha256", "preparedCacheIdentitySha256",
				"sourceExact", "preScaleConsumed", "rowSha256" }) ||
			!Add("geometryUses", 13u, {
				"geometryUseId", "order", "emitterId", "moduleId",
				"sourceEmitterPath", "sourceMeshPackageRef", "carrierId", "assetId",
				"sizeSemantics", "preScaleApplication", "preScaleConsumed",
				"sourceExact", "rowSha256" }))
		{
			return false;
		}

		OutSeals = std::move(Staged);
		return true;
	}

	[[maybe_unused]] bool_t Validate_RuntimeSectionDigestTable(
		const DATA_JSON_VALUE& Root,
		const std::vector<RUNTIME_SECTION_SEAL>& Seals)
	{
		const DATA_JSON_VALUE* Table = Required(
			Root, "sectionDigests", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Table || Table->Get_Array().size() != Seals.size())
			return false;
		for (size_t Index = 0u; Index < Seals.size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Table->Get_Array()[Index];
			if (!Has_ExactKeyOrder(Row,
				{ "sectionName", "rowCount", "orderedSha256" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* Name = Required(
				Row, "sectionName", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Digest = Required(
				Row, "orderedSha256", DATA_JSON_TYPE::STRING);
			uint32_t Count = 0u;
			if (nullptr == Name || Name->Get_String() != Seals[Index].Name ||
				nullptr == Digest ||
				Digest->Get_String() != Seals[Index].strOrderedSha256 ||
				!Read_U32(Row, "rowCount", Count) ||
				Count != Seals[Index].iRowCount)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_BlockerOwnerKey(const std::string_view Key)
	{
		std::string Folded;
		Folded.reserve(Key.size());
		for (const unsigned char Character : Key)
			Folded.push_back(static_cast<char>(std::tolower(Character)));
		return Folded.find("blocker") != std::string::npos;
	}

	void Collect_BlockerStrings(
		const DATA_JSON_VALUE& Value,
		std::set<std::string>& OutValues)
	{
		if (Value.Is_String())
		{
			if (!Value.Get_String().empty())
				OutValues.insert(Value.Get_String());
			return;
		}
		if (Value.Is_Array())
		{
			for (const DATA_JSON_VALUE& Item : Value.Get_Array())
				Collect_BlockerStrings(Item, OutValues);
			return;
		}
		if (Value.Is_Object())
		{
			for (const auto& [Key, Item] : Value.Get_Object())
			{
				(void)Key;
				Collect_BlockerStrings(Item, OutValues);
			}
		}
	}

	void Append_BlockerOwnershipProjection(
		const DATA_JSON_VALUE& Value,
		const std::string_view Section,
		const uint32_t RowIndex,
		const std::string_view OwnerId,
		DATA_JSON_VALUE::ARRAY Path,
		DATA_JSON_VALUE::ARRAY& OutProjection,
		std::set<std::string>& OutOwnedBlockers,
		uint32_t& OutTokenOccurrenceCount)
	{
		if (Value.Is_Array())
		{
			for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
			{
				DATA_JSON_VALUE::ARRAY ChildPath = Path;
				ChildPath.push_back(DATA_JSON_VALUE::Number(
					static_cast<double>(Index), false));
				Append_BlockerOwnershipProjection(
					Value.Get_Array()[Index], Section, RowIndex, OwnerId,
					std::move(ChildPath), OutProjection, OutOwnedBlockers,
					OutTokenOccurrenceCount);
			}
			return;
		}
		if (!Value.Is_Object())
			return;

		const std::vector<std::string>& Order =
			Value.Get_ObjectInsertionOrder();
		for (const std::string& Key : Order)
		{
			const auto Item = Value.Get_Object().find(Key);
			if (Item == Value.Get_Object().end())
				continue;
			DATA_JSON_VALUE::ARRAY ChildPath = Path;
			ChildPath.push_back(DATA_JSON_VALUE::String(Key));
			if (Is_BlockerOwnerKey(Key))
			{
				DATA_JSON_VALUE::OBJECT ProjectionRow;
				ProjectionRow.emplace(
					"section", DATA_JSON_VALUE::String(std::string(Section)));
				ProjectionRow.emplace("rowIndex", DATA_JSON_VALUE::Number(
					static_cast<double>(RowIndex), false));
				ProjectionRow.emplace(
					"ownerId", DATA_JSON_VALUE::String(std::string(OwnerId)));
				ProjectionRow.emplace(
					"path", DATA_JSON_VALUE::Array(ChildPath));
				ProjectionRow.emplace("value", Item->second);
				OutProjection.push_back(DATA_JSON_VALUE::Object(
					std::move(ProjectionRow),
					{ "section", "rowIndex", "ownerId", "path", "value" }));

				std::set<std::string> FieldTokens;
				Collect_BlockerStrings(Item->second, FieldTokens);
				OutOwnedBlockers.insert(FieldTokens.begin(), FieldTokens.end());
				OutTokenOccurrenceCount +=
					static_cast<uint32_t>(FieldTokens.size());
			}
			Append_BlockerOwnershipProjection(
				Item->second, Section, RowIndex, OwnerId,
				std::move(ChildPath), OutProjection, OutOwnedBlockers,
				OutTokenOccurrenceCount);
		}
	}

	bool_t Validate_RuntimeBlockerOwnership(
		const DATA_JSON_VALUE& Root,
		const std::vector<std::string>& SerializedUnion,
		EFFECT_RUNTIME_PROGRAM_BLOCKER_OWNERSHIP& OutOwnership)
	{
		constexpr std::array<std::pair<std::string_view, std::string_view>, 23u>
			SectionOwners{{
				{ "inputArtifacts", "artifactId" },
				{ "handlerRegistry", "handlerRegistryId" },
				{ "emitters", "emitterId" },
				{ "actionSchedules", "scheduleId" },
				{ "modules", "moduleId" },
				{ "properties", "propertyId" },
				{ "primitiveLeaves", "leafId" },
				{ "literals", "literalId" },
				{ "distributions", "distributionId" },
				{ "seedPolicies", "seedPolicyId" },
				{ "implicitDefaults", "defaultId" },
				{ "pointLightFields", "fieldId" },
				{ "materialFamilies", "familyId" },
				{ "materialRecipes", "recipeId" },
				{ "materialInputs", "fieldId" },
				{ "materialStaticBindings", "fieldId" },
				{ "materialRenderBindings", "renderBindingId" },
				{ "materialOccurrences", "occurrenceId" },
				{ "materialPolicyRows", "policyRowId" },
				{ "materialTextureBindings", "bindingId" },
				{ "rendererTextureResources", "textureResourceId" },
				{ "geometryCarriers", "carrierId" },
				{ "geometryUses", "geometryUseId" },
			}};

		DATA_JSON_VALUE::ARRAY Projection;
		Projection.reserve(ACTIVE_RECONSTRUCTED_PROFILE.iBlockerOwnerFieldCount);
		std::set<std::string> OwnedBlockers;
		uint32_t TokenOccurrenceCount = 0u;
		for (const auto& [SectionName, IdField] : SectionOwners)
		{
			const DATA_JSON_VALUE* Section = Required(
				Root, SectionName, DATA_JSON_TYPE::ARRAY);
			if (nullptr == Section)
				return false;
			for (size_t Index = 0u; Index < Section->Get_Array().size(); ++Index)
			{
				const DATA_JSON_VALUE& Row = Section->Get_Array()[Index];
				const DATA_JSON_VALUE* Owner = Required(
					Row, IdField, DATA_JSON_TYPE::STRING);
				if (nullptr == Owner || Owner->Get_String().empty() ||
					Index > static_cast<size_t>(UINT32_MAX))
				{
					return false;
				}
				Append_BlockerOwnershipProjection(
					Row, SectionName, static_cast<uint32_t>(Index),
					Owner->Get_String(), {}, Projection, OwnedBlockers,
					TokenOccurrenceCount);
			}
		}

		const DATA_JSON_VALUE* Contract = Required(
			Root, "blockerOwnership", DATA_JSON_TYPE::OBJECT);
		uint32_t Version = 0u;
		uint32_t FieldCount = 0u;
		uint32_t SerializedTokenCount = 0u;
		std::string ProjectionSha256;
		const std::string ComputedProjectionSha256 =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
					DATA_JSON_VALUE::Array(std::move(Projection))));
		if (nullptr == Contract || !Has_ExactKeyOrder(*Contract, {
				"version", "fieldCount", "tokenOccurrenceCount",
				"projectionSha256" }) ||
			!Read_U32(*Contract, "version", Version) || Version != 1u ||
			!Read_U32(*Contract, "fieldCount", FieldCount) ||
			!Read_U32(*Contract, "tokenOccurrenceCount", SerializedTokenCount) ||
			!Read_LowerHex(
				*Contract, "projectionSha256", 64u, ProjectionSha256) ||
			FieldCount != ACTIVE_RECONSTRUCTED_PROFILE.iBlockerOwnerFieldCount ||
			SerializedTokenCount != TokenOccurrenceCount ||
			SerializedTokenCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iBlockerTokenOccurrenceCount ||
			ProjectionSha256 != ComputedProjectionSha256 ||
			ProjectionSha256 !=
				ACTIVE_RECONSTRUCTED_PROFILE.BlockerOwnershipSha256)
		{
			return false;
		}

		std::set<std::string> ExpectedBlockers(
			REQUIRED_GLOBAL_PROGRAM_BLOCKERS.begin(),
			REQUIRED_GLOBAL_PROGRAM_BLOCKERS.end());
		ExpectedBlockers.insert(OwnedBlockers.begin(), OwnedBlockers.end());
		if (SerializedUnion != std::vector<std::string>(
				ExpectedBlockers.begin(), ExpectedBlockers.end()))
		{
			return false;
		}

		OutOwnership.iVersion = Version;
		OutOwnership.iFieldCount = FieldCount;
		OutOwnership.iTokenOccurrenceCount = SerializedTokenCount;
		OutOwnership.strProjectionSha256 = std::move(ProjectionSha256);
		return true;
	}

	[[maybe_unused]] bool_t Validate_RuntimeProgramRootSeals(
		const DATA_JSON_VALUE& Root,
		std::string& OutProgramSha256,
		EFFECT_RUNTIME_PROGRAM_BLOCKER_OWNERSHIP& OutBlockerOwnership)
	{
		if (!Has_ExactKeyOrder(Root, {
			"schema", "formatVersion", "programId", "programVersion", "target",
			"policyRouteBinding", "particleSystemPolicy",
			"materialEvaluatorOracleContract", "programRole", "inputArtifacts",
			"handlerRegistry", "emitters", "actionSchedules", "modules",
			"properties", "primitiveLeaves", "literals", "distributions",
			"seedPolicies", "implicitDefaults", "pointLightFields",
			"materialFamilies", "materialRecipes", "materialInputs",
			"materialStaticBindings", "materialRenderBindings",
			"materialOccurrences", "materialPolicyRows", "materialTextureBindings",
			"rendererTextureResources", "geometryCarriers", "geometryUses",
			"sectionDigests", "blockerOwnership", "blockerUnion", "admission", "summary",
			"programSha256" }))
		{
			return false;
		}

		std::vector<RUNTIME_SECTION_SEAL> Seals;
		if (!Validate_RuntimeSectionSeals(Root, Seals) ||
			!Validate_RuntimeSectionDigestTable(Root, Seals) ||
			!Verify_CanonicalSelfSha256(
				Root, "programSha256", OutProgramSha256))
		{
			return false;
		}

		const DATA_JSON_VALUE* BlockerUnion = Required(
			Root, "blockerUnion", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Admission = Required(
			Root, "admission", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Summary = Required(
			Root, "summary", DATA_JSON_TYPE::OBJECT);
		if (nullptr == BlockerUnion || nullptr == Admission || nullptr == Summary ||
			!Has_ExactKeyOrder(*Admission, {
				"artifactBinding", "policyRoute", "sourceHandlerSelection",
				"distributionEvaluatorSelection", "materialPolicySelection",
				"geometryBinding", "sourceExact", "runtimeExecution", "product",
				"blockers" }) ||
			!Has_ExactKeyOrder(*Summary, {
				"inputArtifactCount", "handlerRegistryCount", "emitterCount",
				"actionScheduleCount", "moduleCount", "moduleSelectionCounts",
				"propertyCount", "primitiveLeafCount", "literalCount",
				"distributionCount", "distributionVariantCounts",
				"seedPolicyCount", "implicitDefaultCount", "pointLightFieldCount",
				"materialFamilyCount", "materialRecipeCount", "materialInputCount",
				"materialStaticBindingCount", "materialRenderBindingCount",
				"materialOccurrenceCount", "materialPolicyCount",
				"materialPolicyDomainCounts", "materialD3dDescriptorCount",
				"materialD3dDescriptorProjectionSha256", "materialSrvDescriptorCount",
				"materialSrvDescriptorProjectionSha256",
				"materialTextureBindingCount",
				"resolvedMaterialTextureBindingCount",
				"rendererTextureResourceCount", "rendererTextureSlotCounts",
				"geometryCarrierCount", "geometryUseCount", "unknownHandlerCount",
				"ownerlessRowCount", "duplicateIdCount", "sourceExactRowCount",
				"runtimeExecution", "product" }))
		{
			return false;
		}

		std::vector<std::string> Union;
		const DATA_JSON_VALUE* AdmissionBlockers = Required(
			*Admission, "blockers", DATA_JSON_TYPE::ARRAY);
		std::vector<std::string> AdmissionUnion;
		if (!Parse_StringArray(*BlockerUnion, Union, false, 256u, 256u) ||
			nullptr == AdmissionBlockers ||
			!Parse_StringArray(
				*AdmissionBlockers, AdmissionUnion, false, 256u, 256u) ||
			Union != AdmissionUnion || !std::is_sorted(Union.begin(), Union.end()) ||
			std::adjacent_find(Union.begin(), Union.end()) != Union.end())
		{
			return false;
		}
		if (!Validate_RuntimeBlockerOwnership(
			Root, Union, OutBlockerOwnership))
		{
			return false;
		}

		const auto IsBoolean = [](const DATA_JSON_VALUE& Object,
			const std::string_view Name, const bool_t Expected) -> bool_t
		{
			const DATA_JSON_VALUE* Value = Required(
				Object, Name, DATA_JSON_TYPE::BOOLEAN);
			return nullptr != Value && Value->Get_Boolean() == Expected;
		};
		for (const std::string_view Name : {
			"artifactBinding", "policyRoute", "sourceHandlerSelection",
			"distributionEvaluatorSelection", "materialPolicySelection",
			"geometryBinding" })
		{
			if (!IsBoolean(*Admission, Name, true))
				return false;
		}
		if (!IsBoolean(*Admission, "sourceExact", false) ||
			!IsBoolean(*Admission, "runtimeExecution", false) ||
			!IsBoolean(*Admission, "product", false))
		{
			return false;
		}
		return true;
	}

	bool_t Read_ExactVersion(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const uint32_t Expected)
	{
		uint32_t Value = 0u;
		return Read_U32(Object, Name, Value) && Value == Expected;
	}

	bool_t Read_ExpectedBoolean(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const bool_t Expected)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::BOOLEAN);
		return nullptr != Value && Value->Get_Boolean() == Expected;
	}

	bool_t Read_Sha(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || !Is_Sha256(Value->Get_String()))
			return false;
		OutValue = Value->Get_String();
		return true;
	}

	bool_t Read_StableId(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || !Is_StableId(Value->Get_String()))
			return false;
		OutValue = Value->Get_String();
		return true;
	}

	bool_t Parse_Blockers(
		const DATA_JSON_VALUE& Value,
		std::vector<std::string>& OutBlockers)
	{
		if (!Value.Is_Array())
			return false;
		std::vector<std::string> Staged;
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_String() || !Is_StableId(Item.Get_String()))
				return false;
			Staged.push_back(Item.Get_String());
		}
		std::vector<std::string> Unique = Staged;
		std::sort(Unique.begin(), Unique.end());
		if (std::adjacent_find(Unique.begin(), Unique.end()) != Unique.end())
			return false;
		OutBlockers = std::move(Staged);
		return true;
	}

	bool_t Read_OptionalSha(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		if (!Read_String(Object, Name, OutValue, true, 64u))
			return false;
		return OutValue.empty() || Is_Sha256(OutValue);
	}

	bool_t Read_OptionalRuntimeId(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value || Value->Get_String().size() > 1024u)
			return false;
		if (!Value->Get_String().empty() &&
			std::any_of(Value->Get_String().begin(), Value->Get_String().end(),
				[](const unsigned char Character)
				{
					return Character < 0x20u || Character == 0x7fu;
				}))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	bool_t Read_NullableRuntimeId(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.clear();
			return true;
		}
		return Read_RuntimeId(Object, Name, OutValue);
	}

	bool_t Read_NullableRuntimeIdValue(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::optional<std::string>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		std::string Parsed;
		if (!Read_RuntimeId(Object, Name, Parsed))
			return false;
		OutValue = std::move(Parsed);
		return true;
	}

	bool_t Read_ExpectedNull(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		return nullptr != Value && Value->Is_Null();
	}

	bool_t Parse_RowIdentity(
		const DATA_JSON_VALUE& Row,
		const std::string_view IdField,
		const uint32_t ExpectedOrder,
		EFFECT_RUNTIME_PROGRAM_ROW_IDENTITY& OutIdentity,
		const bool_t bHasSourceExact = true)
	{
		uint32_t Order = 0u;
		return Read_RuntimeId(Row, IdField, OutIdentity.strId) &&
			Read_U32(Row, "order", Order) &&
			(ExpectedOrder == (std::numeric_limits<uint32_t>::max)() ||
				Order == ExpectedOrder) &&
			Read_Sha(Row, "rowSha256", OutIdentity.strRowSha256) &&
			(!bHasSourceExact || Read_ExpectedBoolean(Row, "sourceExact", false)) &&
			(OutIdentity.iOrder = Order, true) &&
			(OutIdentity.bSourceExact = false, true);
	}

	template <typename Enum>
	bool_t Read_ClosedEnum(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const std::initializer_list<std::pair<std::string_view, Enum>> Values,
		Enum& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value)
			return false;
		for (const auto& [Token, Parsed] : Values)
		{
			if (Value->Get_String() == Token)
			{
				OutValue = Parsed;
				return true;
			}
		}
		return false;
	}

	bool_t Parse_HandlerKind(
		const DATA_JSON_VALUE& Row,
		EFFECT_RUNTIME_HANDLER_KIND& OutValue)
	{
		return Read_ClosedEnum(Row, "handlerKind", {
			{ "SOURCE_MODULE", EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE },
			{ "SOURCE_PROPERTY", EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PROPERTY },
			{ "SOURCE_PRIMITIVE", EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PRIMITIVE },
			{ "RECONSTRUCTED_MODULE", EFFECT_RUNTIME_HANDLER_KIND::RECONSTRUCTED_MODULE },
			{ "DISTRIBUTION", EFFECT_RUNTIME_HANDLER_KIND::DISTRIBUTION },
			{ "MATERIAL_EVALUATOR", EFFECT_RUNTIME_HANDLER_KIND::MATERIAL_EVALUATOR },
			{ "MATERIAL_POLICY", EFFECT_RUNTIME_HANDLER_KIND::MATERIAL_POLICY },
		}, OutValue);
	}

	bool_t Parse_RendererKind(
		const DATA_JSON_VALUE& Row,
		EFFECT_RUNTIME_RENDERER_KIND& OutValue)
	{
		return Read_ClosedEnum(Row, "rendererType", {
			{ "SpriteParticle", EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE },
			{ "MeshParticle", EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE },
			{ "DecalParticle", EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE },
			{ "CascadeRibbon", EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON },
			{ "ScreenPost", EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST },
			{ "LightParticle", EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE },
		}, OutValue);
	}

	bool_t Parse_LiteralVariant(
		const DATA_JSON_VALUE& Row,
		const std::string_view Name,
		EFFECT_RUNTIME_LITERAL_VARIANT& OutValue)
	{
		return Read_ClosedEnum(Row, Name, {
			{ "BOOL", EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN },
			{ "F64", EFFECT_RUNTIME_LITERAL_VARIANT::F64 },
			{ "ENUM_STRING", EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING },
		}, OutValue);
	}

	bool_t Parse_DistributionVariant(
		const DATA_JSON_VALUE& Row,
		EFFECT_RUNTIME_DISTRIBUTION_VARIANT& OutValue)
	{
		return Read_ClosedEnum(Row, "variant", {
			{ "INLINE", EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE },
			{ "FLOAT_PARAMETER", EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER },
			{ "VECTOR_PARAMETER", EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER },
			{ "FLOAT_CURVE", EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE },
			{ "EF_MULTIPLY", EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY },
		}, OutValue);
	}

	bool_t Parse_MaterialValueVariant(
		const DATA_JSON_VALUE& Row,
		const std::string_view Name,
		EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT& OutValue,
		const bool_t bAllowSamplerDescriptor)
	{
		const DATA_JSON_VALUE* Value = Required(
			Row, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value)
			return false;
		const std::string& Token = Value->Get_String();
		if (Token == "BOOL")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN;
		else if (Token == "F64")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64;
		else if (Token == "F64X4")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4;
		else if (Token == "TEXTURE_ID")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID;
		else if (Token == "ENUM_STRING")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::ENUM_STRING;
		else if (bAllowSamplerDescriptor && Token == "SAMPLER_DESCRIPTOR")
			OutValue = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::SAMPLER_DESCRIPTOR;
		else
			return false;
		return true;
	}

	bool_t Parse_MaterialPolicyDomain(
		const DATA_JSON_VALUE& Row,
		EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN& OutValue)
	{
		return Read_ClosedEnum(Row, "domain", {
			{ "RENDER_STATE", EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE },
			{ "STATIC_PERMUTATION", EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::STATIC_PERMUTATION },
			{ "SAMPLER_DESCRIPTOR", EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::SAMPLER_DESCRIPTOR },
		}, OutValue);
	}

	bool_t Parse_TextureResolutionStatus(
		const DATA_JSON_VALUE& Row,
		EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS& OutValue)
	{
		return Read_ClosedEnum(Row, "resolutionStatus", {
			{ "RESOLVED_EXACT_RUNTIME_ASSET",
				EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET },
			{ "UNRESOLVED_RUNTIME_ASSET",
				EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::UNRESOLVED_RUNTIME_ASSET },
		}, OutValue);
	}

	bool_t Parse_ProgramActionCueValue(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE& OutValue)
	{
		const DATA_JSON_VALUE* Vector = Required(
			Value, "vectorValue", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"name", "kind", "scalarValue", "vectorValue", "sourceIndex",
				"sourceValueByteOffset" }) ||
			!Read_String(Value, "name", OutValue.strName, false, 1024u) ||
			!Read_ClosedEnum(Value, "kind", {
				{ "SCALAR", EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR },
				{ "VECTOR", EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR },
			}, OutValue.eKind) ||
			!Read_NullableF64(Value, "scalarValue", OutValue.fScalarValue) ||
			nullptr == Vector ||
			!Parse_FiniteF64Array(*Vector, OutValue.VectorValue, 0u, 4u) ||
			!Read_I32(Value, "sourceIndex", OutValue.iSourceIndex) ||
			!Read_I32(Value, "sourceValueByteOffset", OutValue.iSourceValueByteOffset))
		{
			return false;
		}
		if (OutValue.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR)
			return OutValue.fScalarValue.has_value() && OutValue.VectorValue.empty();
		return !OutValue.fScalarValue.has_value() &&
			(OutValue.VectorValue.size() == 3u || OutValue.VectorValue.size() == 4u);
	}

	bool_t Parse_ProgramActionCueValues(
		const DATA_JSON_VALUE& Value,
		std::vector<EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE>& OutValues)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() > 64u)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_ACTION_CUE_VALUE Parsed;
			if (!Parse_ProgramActionCueValue(Item, Parsed))
				return false;
			Staged.push_back(std::move(Parsed));
		}
		OutValues = std::move(Staged);
		return true;
	}

	bool_t Parse_ProgramSocketTransform(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_SOCKET_TRANSFORM& OutTransform)
	{
		const DATA_JSON_VALUE* Position = Required(
			Value, "position", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Rotation = Required(
			Value, "rotationDegrees", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Scale = Required(
			Value, "scale", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value,
			{ "position", "rotationDegrees", "scale" }) &&
			nullptr != Position && Parse_FixedF64Array(*Position, OutTransform.vPosition) &&
			nullptr != Rotation &&
			Parse_FixedF64Array(*Rotation, OutTransform.vRotationDegrees) &&
			nullptr != Scale && Parse_FixedF64Array(*Scale, OutTransform.vScale);
	}

	bool_t Parse_ProgramCueTransform(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_CUE_TRANSFORM& OutTransform)
	{
		const DATA_JSON_VALUE* Source = Required(
			Value, "sourcePositionUeUnits", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Position = Required(
			Value, "position", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Rotation = Required(
			Value, "rotationDegrees", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Scale = Required(
			Value, "scale", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"sourcePositionUeUnits", "position", "rotationDegrees", "scale" }) &&
			nullptr != Source &&
			Parse_FixedF64Array(*Source, OutTransform.vSourcePositionUeUnits) &&
			nullptr != Position && Parse_FixedF64Array(*Position, OutTransform.vPosition) &&
			nullptr != Rotation &&
			Parse_FixedF64Array(*Rotation, OutTransform.vRotationDegrees) &&
			nullptr != Scale && Parse_FixedF64Array(*Scale, OutTransform.vScale);
	}

	bool_t Parse_ProgramDetailTransform(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DETAIL_TRANSFORM& OutTransform)
	{
		const DATA_JSON_VALUE* Position = Required(
			Value, "position", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Rotation = Required(
			Value, "rotationDegrees", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Revolution = Required(
			Value, "revolutionDegreesPerSecond", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Scale = Required(
			Value, "scale", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Velocity = Required(
			Value, "velocityPerSecond", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"position", "rotationDegrees", "revolutionDegreesPerSecond",
				"scale", "velocityPerSecond", "decision", "consumptionPolicy",
				"projectionSha256" }) ||
			nullptr == Position ||
			!Parse_FixedF64Array(*Position, OutTransform.vPosition) ||
			nullptr == Rotation ||
			!Parse_FixedF64Array(*Rotation, OutTransform.vRotationDegrees) ||
			nullptr == Revolution ||
			!Parse_FixedF64Array(
				*Revolution, OutTransform.vRevolutionDegreesPerSecond) ||
			nullptr == Scale || !Parse_FixedF64Array(*Scale, OutTransform.vScale) ||
			nullptr == Velocity ||
			!Parse_FixedF64Array(*Velocity, OutTransform.vVelocityPerSecond) ||
			!Read_ClosedString(Value, "decision",
				{ "VERIFIED_IDENTITY_IRRELEVANT" }, OutTransform.strDecision) ||
			!Read_ClosedString(Value, "consumptionPolicy",
				{ "IDENTITY_ONLY_NO_RUNTIME_OPERATION" },
				OutTransform.strConsumptionPolicy) ||
			!Verify_CanonicalSelfSha256(
				Value, "projectionSha256", OutTransform.strProjectionSha256))
		{
			return false;
		}
		return OutTransform.vPosition == std::array<double, 3u>{ 0.0, 0.0, 0.0 } &&
			OutTransform.vRotationDegrees ==
				std::array<double, 3u>{ 0.0, 0.0, 0.0 } &&
			OutTransform.vRevolutionDegreesPerSecond ==
				std::array<double, 3u>{ 0.0, 0.0, 0.0 } &&
			OutTransform.vScale == std::array<double, 3u>{ 1.0, 1.0, 1.0 } &&
			OutTransform.vVelocityPerSecond ==
				std::array<double, 3u>{ 0.0, 0.0, 0.0 };
	}

	bool_t Parse_ProgramAttachment(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_ATTACHMENT& OutAttachment)
	{
		const DATA_JSON_VALUE* Socket = Required(
			Value, "socketLocalTransform", DATA_JSON_TYPE::OBJECT);
		return Has_ExactKeyOrder(Value, {
				"enabled", "follow", "sourceAnchorSlotId", "runtimeAnchorSlotId",
				"runtimeBoneName", "snapshotRootSourceBasisYawDegrees",
				"socketLocalTransform" }) &&
			Read_Boolean(Value, "enabled", OutAttachment.bEnabled) &&
			Read_Boolean(Value, "follow", OutAttachment.bFollow) &&
			Read_String(Value, "sourceAnchorSlotId",
				OutAttachment.strSourceAnchorSlotId, true, 256u) &&
			Read_String(Value, "runtimeAnchorSlotId",
				OutAttachment.strRuntimeAnchorSlotId, false, 256u) &&
			Read_String(Value, "runtimeBoneName",
				OutAttachment.strRuntimeBoneName, true, 256u) &&
			Read_FiniteF64(Value, "snapshotRootSourceBasisYawDegrees",
				OutAttachment.fSnapshotRootSourceBasisYawDegrees) &&
			nullptr != Socket &&
			Parse_ProgramSocketTransform(*Socket, OutAttachment.SocketLocalTransform);
	}

	bool_t Parse_ProgramTransformInheritance(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_TRANSFORM_INHERITANCE& OutInheritance)
	{
		return Has_ExactKeyOrder(Value, {
				"enabled", "masterEmitterId", "decision", "consumptionPolicy",
				"projectionSha256" }) &&
			Read_ExpectedBoolean(Value, "enabled", false) &&
			Read_String(Value, "masterEmitterId",
				OutInheritance.strMasterEmitterId, true, 1024u) &&
			OutInheritance.strMasterEmitterId.empty() &&
			Read_ClosedString(Value, "decision",
				{ "RECONSTRUCTED_NO_INHERITANCE_V1" }, OutInheritance.strDecision) &&
			Read_ClosedString(Value, "consumptionPolicy",
				{ "NO_RUNTIME_PARENT_TRANSFORM_INHERITANCE" },
				OutInheritance.strConsumptionPolicy) &&
			Verify_CanonicalSelfSha256(
				Value, "projectionSha256", OutInheritance.strProjectionSha256) &&
			(OutInheritance.bEnabled = false, true);
	}

	bool_t Parse_ProgramAnchorRequests(
		const DATA_JSON_VALUE& Value,
		std::vector<EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST>& OutRequests)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() > 8u)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			const DATA_JSON_VALUE* Socket = Required(
				Item, "socketLocalTransform", DATA_JSON_TYPE::OBJECT);
			EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST Parsed;
			if (!Has_ExactKeyOrder(Item, {
					"anchorRequestId", "order", "sourceKind", "sourceModuleId",
					"sourceAnchorSlotId", "runtimeAnchorSlotId", "runtimeBoneName",
					"socketLocalTransform", "follow" }) ||
				!Read_RuntimeId(Item, "anchorRequestId", Parsed.strAnchorRequestId) ||
				!Read_U32(Item, "order", Parsed.iOrder) ||
				Parsed.iOrder != Staged.size() ||
				!Read_ClosedString(Item, "sourceKind",
					{ "ACTION_CUE" }, Parsed.strSourceKind) ||
				!Read_OptionalRuntimeId(Item, "sourceModuleId", Parsed.strSourceModuleId) ||
				!Read_String(Item, "sourceAnchorSlotId",
					Parsed.strSourceAnchorSlotId, true, 256u) ||
				!Read_String(Item, "runtimeAnchorSlotId",
					Parsed.strRuntimeAnchorSlotId, false, 256u) ||
				!Read_String(Item, "runtimeBoneName",
					Parsed.strRuntimeBoneName, true, 256u) ||
				nullptr == Socket ||
				!Parse_ProgramSocketTransform(*Socket, Parsed.SocketLocalTransform) ||
				!Read_Boolean(Item, "follow", Parsed.bFollow))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutRequests = std::move(Staged);
		return true;
	}

	template <typename ValueType>
	bool_t Parse_NullableProgramObject(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		bool_t (*Parser)(const DATA_JSON_VALUE&, ValueType&),
		std::optional<ValueType>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		if (!Value->Is_Object())
			return false;
		ValueType Parsed;
		if (!Parser(*Value, Parsed))
			return false;
		OutValue = std::move(Parsed);
		return true;
	}

	bool_t Parse_RendererColor(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_COLOR& OutColor)
	{
		const DATA_JSON_VALUE* Offset = Required(Value, "offset", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Multiply = Required(Value, "multiply", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"offset", "multiply", "clip", "emissiveIntensity",
				"distortionIntensity", "distortionOnBaseMaterial", "radialTime",
				"radialIntensity" }) &&
			nullptr != Offset && Parse_FixedF64Array(*Offset, OutColor.vOffset) &&
			nullptr != Multiply && Parse_FixedF64Array(*Multiply, OutColor.vMultiply) &&
			Read_FiniteF64(Value, "clip", OutColor.fClip) &&
			Read_FiniteF64(Value, "emissiveIntensity", OutColor.fEmissiveIntensity) &&
			Read_FiniteF64(Value, "distortionIntensity", OutColor.fDistortionIntensity) &&
			Read_Boolean(Value, "distortionOnBaseMaterial",
				OutColor.bDistortionOnBaseMaterial) &&
			Read_FiniteF64(Value, "radialTime", OutColor.fRadialTime) &&
			Read_FiniteF64(Value, "radialIntensity", OutColor.fRadialIntensity);
	}

	bool_t Parse_RendererUv(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_UV& OutUv)
	{
		const DATA_JSON_VALUE* Start = Required(Value, "start", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Speed = Required(Value, "speed", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Amplitude = Required(
			Value, "waveAmplitude", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"start", "speed", "wave", "waveAmplitude", "waveFrequency",
				"sequence", "loop", "sequenceTerm", "tileColumns", "tileRows",
				"tileIndex" }) &&
			nullptr != Start && Parse_FixedF64Array(*Start, OutUv.vStart) &&
			nullptr != Speed && Parse_FixedF64Array(*Speed, OutUv.vSpeed) &&
			Read_Boolean(Value, "wave", OutUv.bWave) &&
			nullptr != Amplitude &&
			Parse_FixedF64Array(*Amplitude, OutUv.vWaveAmplitude) &&
			Read_FiniteF64(Value, "waveFrequency", OutUv.fWaveFrequency) &&
			Read_Boolean(Value, "sequence", OutUv.bSequence) &&
			Read_Boolean(Value, "loop", OutUv.bLoop) &&
			Read_FiniteF64(Value, "sequenceTerm", OutUv.fSequenceTerm) &&
			Read_U32(Value, "tileColumns", OutUv.iTileColumns) &&
			Read_U32(Value, "tileRows", OutUv.iTileRows) &&
			Read_U32(Value, "tileIndex", OutUv.iTileIndex);
	}

	bool_t Parse_RendererLinearLerp(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_LINEAR_LERP& OutLerp)
	{
		const auto Array = [&Value](const std::string_view Name) -> const DATA_JSON_VALUE*
		{
			return Required(Value, Name, DATA_JSON_TYPE::ARRAY);
		};
		const DATA_JSON_VALUE* EndPosition = Array("endPosition");
		const DATA_JSON_VALUE* EndRotation = Array("endRotationDegrees");
		const DATA_JSON_VALUE* EndRevolution = Array("endRevolutionDegreesPerSecond");
		const DATA_JSON_VALUE* EndScale = Array("endScale");
		const DATA_JSON_VALUE* EndVelocity = Array("endVelocityPerSecond");
		const DATA_JSON_VALUE* EndOffset = Array("endColorOffset");
		const DATA_JSON_VALUE* EndMultiply = Array("endColorMultiply");
		return Has_ExactKeyOrder(Value, {
				"position", "endPosition", "rotation", "endRotationDegrees",
				"revolution", "endRevolutionDegreesPerSecond", "scale", "endScale",
				"velocity", "endVelocityPerSecond", "colorOffset", "endColorOffset",
				"colorMultiply", "endColorMultiply", "emissiveIntensity",
				"endEmissiveIntensity" }) &&
			Read_Boolean(Value, "position", OutLerp.bPosition) &&
			nullptr != EndPosition && Parse_FixedF64Array(*EndPosition, OutLerp.vEndPosition) &&
			Read_Boolean(Value, "rotation", OutLerp.bRotation) &&
			nullptr != EndRotation &&
			Parse_FixedF64Array(*EndRotation, OutLerp.vEndRotationDegrees) &&
			Read_Boolean(Value, "revolution", OutLerp.bRevolution) &&
			nullptr != EndRevolution && Parse_FixedF64Array(
				*EndRevolution, OutLerp.vEndRevolutionDegreesPerSecond) &&
			Read_Boolean(Value, "scale", OutLerp.bScale) &&
			nullptr != EndScale && Parse_FixedF64Array(*EndScale, OutLerp.vEndScale) &&
			Read_Boolean(Value, "velocity", OutLerp.bVelocity) &&
			nullptr != EndVelocity &&
			Parse_FixedF64Array(*EndVelocity, OutLerp.vEndVelocityPerSecond) &&
			Read_Boolean(Value, "colorOffset", OutLerp.bColorOffset) &&
			nullptr != EndOffset &&
			Parse_FixedF64Array(*EndOffset, OutLerp.vEndColorOffset) &&
			Read_Boolean(Value, "colorMultiply", OutLerp.bColorMultiply) &&
			nullptr != EndMultiply &&
			Parse_FixedF64Array(*EndMultiply, OutLerp.vEndColorMultiply) &&
			Read_Boolean(Value, "emissiveIntensity", OutLerp.bEmissiveIntensity) &&
			Read_FiniteF64(Value, "endEmissiveIntensity",
				OutLerp.fEndEmissiveIntensity);
	}

	bool_t Parse_RendererMesh(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_MESH& OutMesh)
	{
		const DATA_JSON_VALUE* Rotation = Required(
			Value, "sourceTypeDataRotationDegrees", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value,
			{ "useModelMaterial", "sourceTypeDataRotationDegrees" }) &&
			Read_Boolean(Value, "useModelMaterial", OutMesh.bUseModelMaterial) &&
			nullptr != Rotation && Parse_FixedF64Array(
				*Rotation, OutMesh.vSourceTypeDataRotationDegrees) &&
			std::all_of(OutMesh.vSourceTypeDataRotationDegrees.begin(),
				OutMesh.vSourceTypeDataRotationDegrees.end(), [](const double Value)
				{
					return std::abs(Value) <= 3600.0;
				});
	}

	bool_t Parse_RendererSprite(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_SPRITE& OutSprite)
	{
		return Has_ExactKeyOrder(Value, { "billboard", "billboardRollDegrees" }) &&
			Read_Boolean(Value, "billboard", OutSprite.bBillboard) &&
			Read_FiniteF64(Value, "billboardRollDegrees",
				OutSprite.fBillboardRollDegrees);
	}

	bool_t Parse_RendererDecal(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_DECAL& OutDecal)
	{
		const DATA_JSON_VALUE* Size = Required(Value, "size", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, { "size", "depth" }) &&
			nullptr != Size && Parse_FixedF64Array(*Size, OutDecal.vSize) &&
			Read_FiniteF64(Value, "depth", OutDecal.fDepth);
	}

	bool_t Parse_RendererTrail(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_TRAIL& OutTrail)
	{
		return Has_ExactKeyOrder(Value, {
				"maxPoints", "pointLifeTimeSeconds", "sampleIntervalSeconds",
				"minimumDistance", "startWidth", "endWidth", "faceCamera" }) &&
			Read_U32(Value, "maxPoints", OutTrail.iMaxPoints) &&
			Read_FiniteF64(Value, "pointLifeTimeSeconds",
				OutTrail.fPointLifeTimeSeconds) &&
			Read_FiniteF64(Value, "sampleIntervalSeconds",
				OutTrail.fSampleIntervalSeconds) &&
			Read_FiniteF64(Value, "minimumDistance", OutTrail.fMinimumDistance) &&
			Read_FiniteF64(Value, "startWidth", OutTrail.fStartWidth) &&
			Read_FiniteF64(Value, "endWidth", OutTrail.fEndWidth) &&
			Read_Boolean(Value, "faceCamera", OutTrail.bFaceCamera);
	}

	bool_t Parse_RendererAfterImage(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_AFTER_IMAGE& OutAfterImage)
	{
		return Has_ExactKeyOrder(Value,
			{ "sampleIntervalSeconds", "maxCopies", "alphaExponent" }) &&
			Read_FiniteF64(Value, "sampleIntervalSeconds",
				OutAfterImage.fSampleIntervalSeconds) &&
			Read_U32(Value, "maxCopies", OutAfterImage.iMaxCopies) &&
			Read_FiniteF64(Value, "alphaExponent", OutAfterImage.fAlphaExponent);
	}

	bool_t Parse_RendererScreenPost(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RENDERER_SCREEN_POST& OutScreenPost)
	{
		const DATA_JSON_VALUE* Tint = Required(Value, "tint", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"enabled", "profileId", "status", "intensity", "secondaryIntensity",
				"frequency", "tint", "randomSeed" }) ||
			!Read_ExpectedBoolean(Value, "enabled", false) ||
			!Read_String(Value, "profileId", OutScreenPost.strProfileId, true, 1024u) ||
			!OutScreenPost.strProfileId.empty() ||
			!Read_ClosedString(Value, "status",
				{ "reconstructed_profile" }, OutScreenPost.strStatus) ||
			!Read_FiniteF64(Value, "intensity", OutScreenPost.fIntensity) ||
			!Read_FiniteF64(Value, "secondaryIntensity",
				OutScreenPost.fSecondaryIntensity) ||
			!Read_FiniteF64(Value, "frequency", OutScreenPost.fFrequency) ||
			nullptr == Tint || !Parse_FixedF64Array(*Tint, OutScreenPost.vTint) ||
			!Read_U32(Value, "randomSeed", OutScreenPost.iRandomSeed))
		{
			return false;
		}
		OutScreenPost.bEnabled = false;
		return OutScreenPost.fIntensity == 0.0 &&
			OutScreenPost.fSecondaryIntensity == 0.0 &&
			OutScreenPost.fFrequency == 0.0 &&
			OutScreenPost.vTint == std::array<double, 4u>{ 1.0, 1.0, 1.0, 1.0 } &&
			OutScreenPost.iRandomSeed == 1u;
	}

	bool_t Parse_RendererRuntimeConfig(
		const DATA_JSON_VALUE& Value,
		const EFFECT_RUNTIME_RENDERER_KIND Renderer,
		EFFECT_RUNTIME_PROGRAM_RENDERER_CONFIG& OutConfig)
	{
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"color", "uv", "linearLerp", "mesh", "sprite", "decal", "trail",
				"afterImage", "screenPost", "sourceProjectionSha256", "fidelity",
				"sourceExact", "consumptionPolicy", "blockers" }) ||
			!Parse_NullableProgramObject(
				Value, "color", Parse_RendererColor, OutConfig.Color) ||
			!Parse_NullableProgramObject(Value, "uv", Parse_RendererUv, OutConfig.Uv) ||
			!Parse_NullableProgramObject(
				Value, "linearLerp", Parse_RendererLinearLerp, OutConfig.LinearLerp) ||
			!Parse_NullableProgramObject(
				Value, "mesh", Parse_RendererMesh, OutConfig.Mesh) ||
			!Parse_NullableProgramObject(
				Value, "sprite", Parse_RendererSprite, OutConfig.Sprite) ||
			!Parse_NullableProgramObject(
				Value, "decal", Parse_RendererDecal, OutConfig.Decal) ||
			!Parse_NullableProgramObject(
				Value, "trail", Parse_RendererTrail, OutConfig.Trail) ||
			!Parse_NullableProgramObject(
				Value, "afterImage", Parse_RendererAfterImage, OutConfig.AfterImage) ||
			!Parse_NullableProgramObject(
				Value, "screenPost", Parse_RendererScreenPost, OutConfig.ScreenPost) ||
			!Read_Sha(Value, "sourceProjectionSha256",
				OutConfig.strSourceProjectionSha256) ||
			!Read_ClosedString(Value, "fidelity",
				{ "RECONSTRUCTED_SOURCE_CANDIDATE_PROJECTION" },
				OutConfig.strFidelity) ||
			!Read_ExpectedBoolean(Value, "sourceExact", false) ||
			!Read_ClosedString(Value, "consumptionPolicy",
				{ "DIAGNOSTIC_ONLY_FORBIDDEN_CONSUMPTION" },
				OutConfig.strConsumptionPolicy) ||
			nullptr == Blockers || !Parse_Blockers(*Blockers, OutConfig.Blockers) ||
			OutConfig.Blockers != std::vector<std::string>{
				"SOURCE_CANDIDATE_PRESENTATION_VALUES_NOT_APPROVAL_ROUTED" })
		{
			return false;
		}
		OutConfig.bSourceExact = false;
		const bool_t Light = Renderer == EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE;
		if (Light)
		{
			return !OutConfig.Color && !OutConfig.Uv && !OutConfig.LinearLerp &&
				!OutConfig.Mesh && !OutConfig.Sprite && !OutConfig.Decal &&
				!OutConfig.Trail && !OutConfig.AfterImage && !OutConfig.ScreenPost;
		}
		if (!OutConfig.Color || !OutConfig.Uv || !OutConfig.LinearLerp ||
			!OutConfig.AfterImage)
		{
			return false;
		}
		return OutConfig.Mesh.has_value() ==
				(Renderer == EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE) &&
			OutConfig.Sprite.has_value() ==
				(Renderer == EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE) &&
			OutConfig.Decal.has_value() ==
				(Renderer == EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE) &&
			OutConfig.Trail.has_value() ==
				(Renderer == EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON) &&
			OutConfig.ScreenPost.has_value() ==
				(Renderer == EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST);
	}

	bool_t Parse_ProgramTiming(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_TIMING& OutTiming)
	{
		const DATA_JSON_VALUE* Bursts = Required(Value, "bursts", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"requiredModuleId", "spawnModuleId", "lifetimeModuleId",
				"emitterDelaySeconds", "emitterDelayPolicy",
				"emitterDurationSeconds", "emitterDurationPolicy",
				"emitterLoopCount", "bursts", "sourceProjectionSha256",
				"fidelity", "sourceExact", "blockers", "timingSha256" }) ||
			!Read_RuntimeId(Value, "requiredModuleId", OutTiming.strRequiredModuleId) ||
			!Read_RuntimeId(Value, "spawnModuleId", OutTiming.strSpawnModuleId) ||
			!Read_RuntimeId(Value, "lifetimeModuleId", OutTiming.strLifetimeModuleId) ||
			!Read_FiniteF64(Value, "emitterDelaySeconds", OutTiming.fEmitterDelaySeconds) ||
			OutTiming.fEmitterDelaySeconds < 0.0 ||
			!Read_ClosedEnum(Value, "emitterDelayPolicy", {
				{ "EXPLICIT_REQUIRED_LITERAL",
					EFFECT_RUNTIME_EMITTER_DELAY_POLICY::EXPLICIT_REQUIRED_LITERAL },
				{ "RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT_V1",
					EFFECT_RUNTIME_EMITTER_DELAY_POLICY::
						RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT },
			}, OutTiming.eEmitterDelayPolicy) ||
			!Read_FiniteF64(
				Value, "emitterDurationSeconds", OutTiming.fEmitterDurationSeconds) ||
			OutTiming.fEmitterDurationSeconds < 0.0 ||
			!Read_ClosedEnum(Value, "emitterDurationPolicy", {
				{ "EXPLICIT_REQUIRED_LITERAL",
					EFFECT_RUNTIME_EMITTER_DURATION_POLICY::EXPLICIT_REQUIRED_LITERAL },
				{ "CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT_V1",
					EFFECT_RUNTIME_EMITTER_DURATION_POLICY::
						CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT },
			}, OutTiming.eEmitterDurationPolicy) ||
			!Read_U32(Value, "emitterLoopCount", OutTiming.iEmitterLoopCount) ||
			nullptr == Bursts || Bursts->Get_Array().size() > 256u ||
			!Read_Sha(Value, "sourceProjectionSha256",
				OutTiming.strSourceProjectionSha256) ||
			!Read_ClosedString(Value, "fidelity",
				{ "RECONSTRUCTED_TYPED_SOURCE_AND_CURRENT_CDO_DEFAULTS_V1" },
				OutTiming.strFidelity) ||
			!Read_ExpectedBoolean(Value, "sourceExact", false) ||
			nullptr == Blockers || !Parse_Blockers(*Blockers, OutTiming.Blockers) ||
			!Verify_CanonicalSelfSha256(
				Value, "timingSha256", OutTiming.strTimingSha256))
		{
			return false;
		}
		OutTiming.bSourceExact = false;
		std::vector<std::string> ExpectedBlockers{
			"R3_TYPED_TIMING_EXECUTOR_NOT_COMPLETE" };
		if (OutTiming.eEmitterDelayPolicy ==
			EFFECT_RUNTIME_EMITTER_DELAY_POLICY::
				RECONSTRUCTED_UE3_ZERO_DISTRIBUTION_DEFAULT)
		{
			ExpectedBlockers.push_back("SOURCE_ERA_REQUIRED_DELAY_DEFAULT_UNPROVEN");
		}
		if (OutTiming.eEmitterDurationPolicy ==
			EFFECT_RUNTIME_EMITTER_DURATION_POLICY::
				CURRENT_REVISION_CDO_RECONSTRUCTED_DEFAULT)
		{
			ExpectedBlockers.push_back("SOURCE_ERA_REQUIRED_DURATION_DEFAULT_UNPROVEN");
		}
		if (OutTiming.Blockers != ExpectedBlockers)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_BURST> Staged;
		Staged.reserve(Bursts->Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Bursts->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_BURST Parsed;
			if (!Has_ExactKeyOrder(Item, {
					"spawnModuleId", "burstIndex", "timeSeconds", "countMinimum",
					"countMaximum", "sourceLiteralProjectionSha256" }) ||
				!Read_RuntimeId(Item, "spawnModuleId", Parsed.strSpawnModuleId) ||
				!Read_U32(Item, "burstIndex", Parsed.iBurstIndex) ||
				!Read_FiniteF64(Item, "timeSeconds", Parsed.fTimeSeconds) ||
				Parsed.fTimeSeconds < 0.0 ||
				!Read_U32(Item, "countMinimum", Parsed.iCountMinimum) ||
				!Read_U32(Item, "countMaximum", Parsed.iCountMaximum) ||
				Parsed.iCountMaximum == 0u ||
				Parsed.iCountMinimum > Parsed.iCountMaximum ||
				!Read_Sha(Item, "sourceLiteralProjectionSha256",
					Parsed.strSourceLiteralProjectionSha256))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutTiming.Bursts = std::move(Staged);
		return true;
	}

	bool_t Parse_ProgramRandom(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RANDOM& OutRandom)
	{
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"policyId", "seedDerivationInputSha256", "emitterRandomSeed",
				"sourceExact", "blockers", "policySha256" }) &&
			Read_ClosedString(Value, "policyId",
				{ "DETERMINISTIC_OCCURRENCE_RNG_FROM_SOURCE_CANDIDATE_V1" },
				OutRandom.strPolicyId) &&
			Read_Sha(Value, "seedDerivationInputSha256",
				OutRandom.strSeedDerivationInputSha256) &&
			Read_U32(Value, "emitterRandomSeed", OutRandom.iEmitterRandomSeed) &&
			Read_ExpectedBoolean(Value, "sourceExact", false) &&
			nullptr != Blockers && Parse_Blockers(*Blockers, OutRandom.Blockers) &&
			OutRandom.Blockers == std::vector<std::string>{
				"R3_OCCURRENCE_RANDOM_STREAM_NOT_EXECUTED" } &&
			Verify_CanonicalSelfSha256(
				Value, "policySha256", OutRandom.strPolicySha256) &&
			(OutRandom.bSourceExact = false, true);
	}

	bool_t Parse_AdapterCommon(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_ADAPTER& OutAdapter)
	{
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		return Read_RuntimeId(Value, "adapterId", OutAdapter.strAdapterId) &&
			Read_ExpectedBoolean(Value, "enabled", true) &&
			Read_ExpectedBoolean(Value, "sourceExact", false) &&
			nullptr != Blockers && Parse_Blockers(*Blockers, OutAdapter.Blockers) &&
			Verify_CanonicalSelfSha256(
				Value, "adapterSha256", OutAdapter.strAdapterSha256) &&
			(OutAdapter.bEnabled = true, OutAdapter.bSourceExact = false, true);
	}

	bool_t Parse_ScreenPostAdapter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_SCREEN_POST_ADAPTER& OutAdapter)
	{
		const DATA_JSON_VALUE* Tint = Required(Value, "tint", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"adapterId", "enabled", "occurrenceId", "materialOccurrenceId",
				"recipeId", "familyId", "intensityDistributionId",
				"alphaDistributionId", "secondaryIntensity", "frequencyPolicy", "tint",
				"sourceSpace", "fidelity", "sourceExact", "blockers", "adapterSha256" }) &&
			Parse_AdapterCommon(Value, OutAdapter.Common) &&
			Read_RuntimeId(Value, "occurrenceId", OutAdapter.strOccurrenceId) &&
			Read_RuntimeId(Value, "materialOccurrenceId",
				OutAdapter.strMaterialOccurrenceId) &&
			Read_RuntimeId(Value, "recipeId", OutAdapter.strRecipeId) &&
			Read_RuntimeId(Value, "familyId", OutAdapter.strFamilyId) &&
			Read_RuntimeId(Value, "intensityDistributionId",
				OutAdapter.strIntensityDistributionId) &&
			Read_RuntimeId(Value, "alphaDistributionId",
				OutAdapter.strAlphaDistributionId) &&
			Read_FiniteF64(Value, "secondaryIntensity",
				OutAdapter.fSecondaryIntensity) &&
			OutAdapter.fSecondaryIntensity == 0.0 &&
			Read_ClosedString(Value, "frequencyPolicy",
				{ "IRRELEVANT_ZOOM_PROFILE" }, OutAdapter.strFrequencyPolicy) &&
			nullptr != Tint && Parse_FixedF64Array(*Tint, OutAdapter.vTint) &&
			OutAdapter.vTint == std::array<double, 4u>{ 1.0, 1.0, 1.0, 1.0 } &&
			Read_ClosedString(Value, "sourceSpace",
				{ "screenSpaceV1" }, OutAdapter.strSourceSpace) &&
			Read_ClosedString(Value, "fidelity",
				{ "RECONSTRUCTED_APPROVED_V1" }, OutAdapter.strFidelity) &&
			OutAdapter.Common.strAdapterId == "ZOOM_BLUR_RECONSTRUCTED_V1" &&
			OutAdapter.Common.Blockers == std::vector<std::string>{
				"R5_SCREEN_POST_RUNTIME_PROBE_NOT_COMPLETE" };
	}

	bool_t Parse_LightAdapter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_LIGHT_ADAPTER& OutAdapter)
	{
		const DATA_JSON_VALUE* FieldIds = Required(
			Value, "fieldIds", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"adapterId", "enabled", "moduleId", "fieldIds", "positionSourcePolicy",
				"ueUnitScale", "sourceExact", "blockers", "adapterSha256" }) &&
			Parse_AdapterCommon(Value, OutAdapter.Common) &&
			Read_RuntimeId(Value, "moduleId", OutAdapter.strModuleId) &&
			nullptr != FieldIds &&
			Parse_StringArray(*FieldIds, OutAdapter.FieldIds, false, 8u, 1024u) &&
			OutAdapter.FieldIds.size() == 8u &&
			Read_ClosedString(Value, "positionSourcePolicy",
				{ "EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION" },
				OutAdapter.strPositionSourcePolicy) &&
			Read_FiniteF64(Value, "ueUnitScale", OutAdapter.fUeUnitScale) &&
			OutAdapter.fUeUnitScale == 0.01 &&
			OutAdapter.Common.strAdapterId == "POINT_LIGHT_TYPED_COMPONENT_V1" &&
			OutAdapter.Common.Blockers == std::vector<std::string>{
				"R5_POINT_LIGHT_RUNTIME_PROBE_NOT_COMPLETE" };
	}

	bool_t Parse_DecalAdapter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DECAL_ADAPTER& OutAdapter)
	{
		const DATA_JSON_VALUE* DefaultSize = Required(
			Value, "defaultSize", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* BlendRange = Required(
			Value, "blendRange", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"adapterId", "enabled", "moduleId", "defaultId", "sizeDistributionId",
				"nearPlane", "farPlane", "defaultSize", "blendRange",
				"yawOnlyCdoDefault", "yawOnlyCapabilityOutput", "yawOnlyDecision",
				"executionAdmission", "supports3dDrawMode", "depthWorldUnits",
				"depthRuntimeUnits", "fidelity", "sourceExact", "blockers",
				"adapterSha256" }) ||
			!Parse_AdapterCommon(Value, OutAdapter.Common) ||
			!Read_RuntimeId(Value, "moduleId", OutAdapter.strModuleId) ||
			!Read_RuntimeId(Value, "defaultId", OutAdapter.strDefaultId) ||
			!Read_RuntimeId(Value, "sizeDistributionId",
				OutAdapter.strSizeDistributionId) ||
			!Read_FiniteF64(Value, "nearPlane", OutAdapter.fNearPlane) ||
			!Read_FiniteF64(Value, "farPlane", OutAdapter.fFarPlane) ||
			nullptr == DefaultSize ||
			!Parse_FixedF64Array(*DefaultSize, OutAdapter.vDefaultSize) ||
			nullptr == BlendRange ||
			!Parse_FixedF64Array(*BlendRange, OutAdapter.vBlendRange) ||
			!Read_ExpectedBoolean(Value, "yawOnlyCdoDefault", true) ||
			!Read_ExpectedBoolean(Value, "yawOnlyCapabilityOutput", true) ||
			!Read_ClosedString(Value, "yawOnlyDecision",
				{ "RECONSTRUCTED_CDO_AND_CAPABILITY_AGREE" },
				OutAdapter.strYawOnlyDecision) ||
			!Read_ExpectedBoolean(Value, "executionAdmission", true) ||
			!Read_ExpectedBoolean(Value, "supports3dDrawMode", true) ||
			!Read_FiniteF64(Value, "depthWorldUnits", OutAdapter.fDepthWorldUnits) ||
			!Read_FiniteF64(Value, "depthRuntimeUnits", OutAdapter.fDepthRuntimeUnits) ||
			!Read_ClosedString(Value, "fidelity",
				{ "RECONSTRUCTED_APPROVED_V1" }, OutAdapter.strFidelity))
		{
			return false;
		}
		OutAdapter.bYawOnlyCdoDefault = true;
		OutAdapter.bYawOnlyCapabilityOutput = true;
		OutAdapter.bExecutionAdmission = true;
		OutAdapter.bSupports3dDrawMode = true;
		return OutAdapter.Common.strAdapterId == "EF_DECAL_TYPED_DESCRIPTOR_V1" &&
			OutAdapter.Common.Blockers == std::vector<std::string>{
				"R5_DECAL_RUNTIME_PROBE_NOT_COMPLETE" } &&
			OutAdapter.fNearPlane == -300.0 && OutAdapter.fFarPlane == 300.0 &&
			OutAdapter.vDefaultSize == std::array<double, 2u>{ 50.0, 50.0 } &&
			OutAdapter.vBlendRange == std::array<double, 2u>{ 100.0, 100.0 } &&
			OutAdapter.fDepthWorldUnits == 600.0 &&
			OutAdapter.fDepthRuntimeUnits == 6.0;
	}

	bool_t Parse_RibbonAdapter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RIBBON_ADAPTER& OutAdapter)
	{
		const auto StringArray = [&Value](const std::string_view Name)
		{
			return Required(Value, Name, DATA_JSON_TYPE::ARRAY);
		};
		const DATA_JSON_VALUE* ColorModules = StringArray("colorModuleIds");
		const DATA_JSON_VALUE* Spawn = StringArray("spawnDistributionIds");
		const DATA_JSON_VALUE* Lifetime = StringArray("lifetimeDistributionIds");
		const DATA_JSON_VALUE* Size = StringArray("sizeDistributionIds");
		const DATA_JSON_VALUE* Color = StringArray("colorDistributionIds");
		const DATA_JSON_VALUE* Dynamic = StringArray("dynamicDistributionIds");
		if (!Has_ExactKeyOrder(Value, {
				"adapterId", "enabled", "typeDataModuleId", "defaultId", "spawnModuleId",
				"lifetimeModuleId", "sizeModuleId", "colorModuleIds",
				"dynamicParameterModuleId", "spawnDistributionIds",
				"lifetimeDistributionIds", "sizeDistributionIds", "colorDistributionIds",
				"dynamicDistributionIds", "tilingDistance",
				"distanceTessellationStepSize", "typedMaxParticleInTrailCount",
				"operationalMaxPoints", "widthPolicy", "geometryPolicy",
				"orientationPolicy", "fidelity", "sourceExact", "blockers",
				"adapterSha256" }) ||
			!Parse_AdapterCommon(Value, OutAdapter.Common) ||
			!Read_RuntimeId(Value, "typeDataModuleId", OutAdapter.strTypeDataModuleId) ||
			!Read_RuntimeId(Value, "defaultId", OutAdapter.strDefaultId) ||
			!Read_RuntimeId(Value, "spawnModuleId", OutAdapter.strSpawnModuleId) ||
			!Read_RuntimeId(Value, "lifetimeModuleId", OutAdapter.strLifetimeModuleId) ||
			!Read_RuntimeId(Value, "sizeModuleId", OutAdapter.strSizeModuleId) ||
			nullptr == ColorModules || !Parse_StringArray(
				*ColorModules, OutAdapter.ColorModuleIds, false, 2u, 1024u) ||
			OutAdapter.ColorModuleIds.size() != 2u ||
			!Read_RuntimeId(Value, "dynamicParameterModuleId",
				OutAdapter.strDynamicParameterModuleId) ||
			nullptr == Spawn || !Parse_StringArray(
				*Spawn, OutAdapter.SpawnDistributionIds, false, 16u, 1024u) ||
			nullptr == Lifetime || !Parse_StringArray(
				*Lifetime, OutAdapter.LifetimeDistributionIds, false, 16u, 1024u) ||
			nullptr == Size || !Parse_StringArray(
				*Size, OutAdapter.SizeDistributionIds, false, 16u, 1024u) ||
			nullptr == Color || !Parse_StringArray(
				*Color, OutAdapter.ColorDistributionIds, false, 16u, 1024u) ||
			nullptr == Dynamic || !Parse_StringArray(
				*Dynamic, OutAdapter.DynamicDistributionIds, false, 16u, 1024u) ||
			!Read_FiniteF64(Value, "tilingDistance", OutAdapter.fTilingDistance) ||
			!Read_FiniteF64(Value, "distanceTessellationStepSize",
				OutAdapter.fDistanceTessellationStepSize) ||
			!Read_U32(Value, "typedMaxParticleInTrailCount",
				OutAdapter.iTypedMaxParticleInTrailCount) ||
			!Read_U32(Value, "operationalMaxPoints", OutAdapter.iOperationalMaxPoints) ||
			!Read_ClosedString(Value, "widthPolicy",
				{ "TYPED_SIZE_DISTRIBUTION_X_AXIS" }, OutAdapter.strWidthPolicy) ||
			!Read_ClosedString(Value, "geometryPolicy",
				{ "TYPED_RIBBON_DEFAULTS_RENDER_GEOMETRY" },
				OutAdapter.strGeometryPolicy) ||
			!Read_ClosedString(Value, "orientationPolicy",
				{ "CAMERA_FACING_SINGLE_SHEET_RECONSTRUCTED_V1" },
				OutAdapter.strOrientationPolicy) ||
			!Read_ClosedString(Value, "fidelity",
				{ "RECONSTRUCTED_TYPED_SOURCE_WITH_EXPLICIT_ORIENTATION_POLICY_V1" },
				OutAdapter.strFidelity))
		{
			return false;
		}
		return OutAdapter.Common.strAdapterId == "CASCADE_RIBBON_TYPED_ADAPTER_V1" &&
			OutAdapter.Common.Blockers == std::vector<std::string>{
				"R5_RIBBON_RUNTIME_PROBE_NOT_COMPLETE",
				"RIBBON_ORIENTATION_POLICY_REQUIRES_RUNTIME_PROBE" } &&
			OutAdapter.SpawnDistributionIds.size() == 2u &&
			OutAdapter.LifetimeDistributionIds.size() == 1u &&
			OutAdapter.SizeDistributionIds.size() == 1u &&
			OutAdapter.ColorDistributionIds.size() == 4u &&
			OutAdapter.DynamicDistributionIds.size() == 4u &&
			OutAdapter.fTilingDistance == 600.0 &&
			OutAdapter.fDistanceTessellationStepSize == 5.0 &&
			OutAdapter.iTypedMaxParticleInTrailCount > 0u &&
			OutAdapter.iOperationalMaxPoints == OutAdapter.iTypedMaxParticleInTrailCount;
	}

	bool_t Read_ExpectedString(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const std::string_view Expected)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		return nullptr != Value && Value->Get_String() == Expected;
	}

	bool_t Validate_FixedFloatVector(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const size_t Count)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Value || Value->Get_Array().size() != Count)
			return false;
		return std::all_of(
			Value->Get_Array().begin(), Value->Get_Array().end(),
			[](const DATA_JSON_VALUE& Item)
			{
				return Item.Is_Number() && Item.Was_FloatingPointToken() &&
					std::isfinite(Item.Get_Number());
			});
	}

	bool_t Parse_ReconstructedRootContracts(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram)
	{
		if (!Read_ExpectedString(Root, "schema", RECONSTRUCTED_PROGRAM_SCHEMA) ||
			!Read_ExactVersion(Root, "formatVersion", 1u) ||
			!Read_ExpectedString(Root, "programId", RECONSTRUCTED_PROGRAM_ID) ||
			!Read_ExactVersion(Root, "programVersion", 1u) ||
			!Read_ExpectedString(Root, "programRole", RECONSTRUCTED_PROGRAM_ROLE))
		{
			return false;
		}

		const DATA_JSON_VALUE* Target = Required(
			Root, "target", DATA_JSON_TYPE::OBJECT);
		uint32_t SkillId = 0u;
		std::string TargetSha256;
		if (nullptr == Target || !Has_ExactKeyOrder(*Target, {
				"characterClass", "skillId", "inputSlot", "policyTargetId",
				"sourceCandidateId", "geometryBindingAssetId",
				"runtimeCatalogAssetId", "identityRouteId",
				"identityRouteSha256" }) ||
			!Read_ExpectedString(*Target, "characterClass", "ARTIST") ||
			!Read_U32(*Target, "skillId", SkillId) || SkillId != 31470u ||
			!Read_ExpectedString(*Target, "inputSlot", "F") ||
			!Read_ExpectedString(
				*Target, "policyTargetId", "effect.artist.skill.31470.f") ||
			!Read_ExpectedString(*Target, "sourceCandidateId",
				"effect.artist.skill.31470.native-v14.source-contract-candidate") ||
			!Read_ExpectedString(*Target, "geometryBindingAssetId",
				"effect.artist.skill.31470") ||
			!Read_ExpectedString(*Target, "runtimeCatalogAssetId",
				"effect.artist.skill.31470") ||
			!Read_ExpectedString(*Target, "identityRouteId",
				"ARTIST_31470_F_RECONSTRUCTED_ROUTE_V1") ||
			!Verify_CanonicalSelfSha256(
				*Target, "identityRouteSha256", TargetSha256))
		{
			return false;
		}

		OutProgram.strCharacterClass = "ARTIST";
		OutProgram.iSkillId = SkillId;
		OutProgram.strInputSlot = "F";
		OutProgram.strPolicyTargetId = "effect.artist.skill.31470.f";
		OutProgram.strSourceCandidateId =
			"effect.artist.skill.31470.native-v14.source-contract-candidate";
		OutProgram.strGeometryBindingAssetId = "effect.artist.skill.31470";
		OutProgram.strRuntimeCatalogAssetId = "effect.artist.skill.31470";
		OutProgram.strIdentityRouteId = "ARTIST_31470_F_RECONSTRUCTED_ROUTE_V1";

		const DATA_JSON_VALUE* Route = Required(
			Root, "policyRouteBinding", DATA_JSON_TYPE::OBJECT);
		std::string RouteSha256;
		uint32_t SourceFamilyCount = 0u;
		uint32_t SourceRowCount = 0u;
		uint32_t MaterialFamilyCount = 0u;
		uint32_t MaterialRowCount = 0u;
		uint32_t ArithmeticRowCount = 0u;
		uint32_t GeometryRowCount = 0u;
		const DATA_JSON_VALUE* SourceFamilies = nullptr;
		const DATA_JSON_VALUE* MaterialFamilies = nullptr;
		if (nullptr == Route || !Has_ExactKeyOrder(*Route, {
				"approvalPolicyId", "approvalPolicyVersion", "approvalReceiptSha256",
				"sourceExecutionFamilyCount", "sourceExecutionFamilies",
				"sourceExecutionFamilyProjectionSha256", "sourcePolicyRowCount",
				"sourcePolicyRowProjectionSha256", "materialExecutionFamilyCount",
				"materialExecutionFamilies", "materialExecutionFamilyProjectionSha256",
				"materialPolicyRowCount", "materialPolicyRowProjectionSha256",
				"materialArithmeticRowCount", "materialArithmeticRowProjectionSha256",
				"geometryPolicyRowCount", "geometryPolicyRowProjectionSha256",
				"sourceCapabilityReceiptSha256", "sourceCapabilityCanonicalSha256",
				"materialPolicyReceiptSha256", "materialPolicyCanonicalSha256",
				"bindingSha256" }) ||
			!Read_ExpectedString(*Route, "approvalPolicyId",
				"artist.31470.f.reconstructed-approved-v1") ||
			!Read_ExactVersion(*Route, "approvalPolicyVersion", 1u) ||
			!Read_U32(*Route, "sourceExecutionFamilyCount", SourceFamilyCount) ||
			SourceFamilyCount != 7u ||
			!Read_U32(*Route, "sourcePolicyRowCount", SourceRowCount) ||
			SourceRowCount != 29u ||
			!Read_U32(*Route, "materialExecutionFamilyCount", MaterialFamilyCount) ||
			MaterialFamilyCount != 8u ||
			!Read_U32(*Route, "materialPolicyRowCount", MaterialRowCount) ||
			MaterialRowCount != 260u ||
			!Read_U32(*Route, "materialArithmeticRowCount", ArithmeticRowCount) ||
			ArithmeticRowCount != 23u ||
			!Read_U32(*Route, "geometryPolicyRowCount", GeometryRowCount) ||
			GeometryRowCount != 7u ||
			nullptr == (SourceFamilies = Required(
				*Route, "sourceExecutionFamilies", DATA_JSON_TYPE::ARRAY)) ||
			SourceFamilies->Get_Array().size() != SourceFamilyCount ||
			nullptr == (MaterialFamilies = Required(
				*Route, "materialExecutionFamilies", DATA_JSON_TYPE::ARRAY)) ||
			MaterialFamilies->Get_Array().size() != MaterialFamilyCount ||
			!Verify_CanonicalSelfSha256(*Route, "bindingSha256", RouteSha256))
		{
			return false;
		}
		EFFECT_RUNTIME_PROGRAM_POLICY_ROUTE ParsedRoute;
		ParsedRoute.strApprovalPolicyId = "artist.31470.f.reconstructed-approved-v1";
		ParsedRoute.iApprovalPolicyVersion = 1u;
		ParsedRoute.iSourcePolicyRowCount = SourceRowCount;
		ParsedRoute.iMaterialPolicyRowCount = MaterialRowCount;
		ParsedRoute.iMaterialArithmeticRowCount = ArithmeticRowCount;
		ParsedRoute.iGeometryPolicyRowCount = GeometryRowCount;
		for (const DATA_JSON_VALUE& Family : SourceFamilies->Get_Array())
		{
			const DATA_JSON_VALUE* MutatedOutputs = Required(
				Family, "requiredMutatedOutputs", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* OracleIds = Required(
				Family, "requiredOracleIds", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_SOURCE_POLICY_FAMILY Parsed;
			if (!Has_ExactKeyOrder(Family, {
					"upstreamClusterId", "upstreamNativeFamily", "policyFamilyId",
					"closureBasis", "moduleOccurrenceCount", "requiredMutatedOutputs",
					"requiredOracleIds", "sourceExact", "executionAdmission" }) ||
				!Read_RuntimeId(Family, "upstreamClusterId", Parsed.strUpstreamClusterId) ||
				!Read_String(Family, "upstreamNativeFamily",
					Parsed.strUpstreamNativeFamily, false, 256u) ||
				!Read_RuntimeId(Family, "policyFamilyId", Parsed.strPolicyFamilyId) ||
				!Read_String(Family, "closureBasis", Parsed.strClosureBasis,
					false, 256u) ||
				!Read_U32(Family, "moduleOccurrenceCount",
					Parsed.iModuleOccurrenceCount) ||
				nullptr == MutatedOutputs || nullptr == OracleIds ||
				!Parse_StringArray(*MutatedOutputs, Parsed.RequiredMutatedOutputs,
					false, 16u, 256u) ||
				!Parse_StringArray(*OracleIds, Parsed.RequiredOracleIds,
					false, 16u, 256u) ||
				!Read_Boolean(Family, "sourceExact", Parsed.bSourceExact) ||
				Parsed.bSourceExact ||
				!Read_Boolean(Family, "executionAdmission",
					Parsed.bExecutionAdmission) || Parsed.bExecutionAdmission)
			{
				return false;
			}
			ParsedRoute.SourceExecutionFamilies.push_back(std::move(Parsed));
		}
		for (const DATA_JSON_VALUE& Family : MaterialFamilies->Get_Array())
		{
			const DATA_JSON_VALUE* OracleIds = Required(
				Family, "requiredOracleIds", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY_FAMILY Parsed;
			if (!Has_ExactKeyOrder(Family, {
					"policyFamilyId", "closureBasis", "requiredOracleIds" }) ||
				!Read_RuntimeId(Family, "policyFamilyId", Parsed.strPolicyFamilyId) ||
				!Read_String(Family, "closureBasis", Parsed.strClosureBasis,
					false, 256u) || nullptr == OracleIds ||
				!Parse_StringArray(*OracleIds, Parsed.RequiredOracleIds,
					false, 16u, 256u))
			{
				return false;
			}
			ParsedRoute.MaterialExecutionFamilies.push_back(std::move(Parsed));
		}
		const std::array<std::pair<std::string_view, std::string*>, 11u> ShaFields{{
			{ "approvalReceiptSha256", &ParsedRoute.strApprovalReceiptSha256 },
			{ "sourceExecutionFamilyProjectionSha256",
				&ParsedRoute.strSourceExecutionFamilyProjectionSha256 },
			{ "sourcePolicyRowProjectionSha256",
				&ParsedRoute.strSourcePolicyRowProjectionSha256 },
			{ "materialExecutionFamilyProjectionSha256",
				&ParsedRoute.strMaterialExecutionFamilyProjectionSha256 },
			{ "materialPolicyRowProjectionSha256",
				&ParsedRoute.strMaterialPolicyRowProjectionSha256 },
			{ "materialArithmeticRowProjectionSha256",
				&ParsedRoute.strMaterialArithmeticRowProjectionSha256 },
			{ "geometryPolicyRowProjectionSha256",
				&ParsedRoute.strGeometryPolicyRowProjectionSha256 },
			{ "sourceCapabilityReceiptSha256",
				&ParsedRoute.strSourceCapabilityReceiptSha256 },
			{ "sourceCapabilityCanonicalSha256",
				&ParsedRoute.strSourceCapabilityCanonicalSha256 },
			{ "materialPolicyReceiptSha256",
				&ParsedRoute.strMaterialPolicyReceiptSha256 },
			{ "materialPolicyCanonicalSha256",
				&ParsedRoute.strMaterialPolicyCanonicalSha256 },
		}};
		for (const auto& [ShaField, Destination] : ShaFields)
		{
			if (!Read_Sha(*Route, ShaField, *Destination))
				return false;
		}
		ParsedRoute.strBindingSha256 = RouteSha256;
		OutProgram.strPolicyRouteBindingSha256 = RouteSha256;
		OutProgram.PolicyRoute = std::move(ParsedRoute);

		const DATA_JSON_VALUE* Particle = Required(
			Root, "particleSystemPolicy", DATA_JSON_TYPE::OBJECT);
		std::string ParticleSha256;
		double UniformScale = 0.0;
		double YawOffset = 0.0;
		double DirectionYaw = 0.0;
		double InitialSpeed = 0.0;
		if (nullptr == Particle || !Has_ExactKeyOrder(*Particle, {
				"uniformScaleMultiplier", "yawOffsetDegrees", "directionYawDegrees",
				"initialSpeedMultiplier", "policySha256" }) ||
			!Read_FiniteF64(*Particle, "uniformScaleMultiplier", UniformScale) ||
			!Read_FiniteF64(*Particle, "yawOffsetDegrees", YawOffset) ||
			!Read_FiniteF64(*Particle, "directionYawDegrees", DirectionYaw) ||
			!Read_FiniteF64(*Particle, "initialSpeedMultiplier", InitialSpeed) ||
			UniformScale != 1.0 || YawOffset != 0.0 || DirectionYaw != 0.0 ||
			InitialSpeed != 1.0 ||
			!Verify_CanonicalSelfSha256(
				*Particle, "policySha256", ParticleSha256))
		{
			return false;
		}
		OutProgram.strParticleSystemPolicySha256 = ParticleSha256;
		OutProgram.ParticleSystemPolicy.fUniformScaleMultiplier = UniformScale;
		OutProgram.ParticleSystemPolicy.fYawOffsetDegrees = YawOffset;
		OutProgram.ParticleSystemPolicy.fDirectionYawDegrees = DirectionYaw;
		OutProgram.ParticleSystemPolicy.fInitialSpeedMultiplier = InitialSpeed;
		OutProgram.ParticleSystemPolicy.strPolicySha256 = std::move(ParticleSha256);

		const DATA_JSON_VALUE* Oracle = Required(
			Root, "materialEvaluatorOracleContract", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* OperationOrder = nullptr;
		const DATA_JSON_VALUE* InputSamples = nullptr;
		uint32_t InputSampleCount = 0u;
		double NumericTolerance = 0.0;
		std::string OracleSha256;
		constexpr std::array<std::string_view, 10u> ExpectedOperations{
			"SECOND_TEXTURE_MULTIPLY", "UV_TRANSFORM_PHASE", "PANNER_PHASE",
			"COLOR_MULTIPLY", "DESATURATION", "SIGNED_POWER", "FRESNEL_GAIN",
			"DISTORTION_OFFSET", "DISSOLVE_ALPHA", "ALPHA_MULTIPLY" };
		if (nullptr == Oracle || !Has_ExactKeyOrder(*Oracle, {
				"version", "operationOrder", "inputSampleCountPerFamily",
				"inputSamples", "numericTolerance", "fidelity", "sourceExact",
				"contractSha256" }) ||
			!Read_ExactVersion(*Oracle, "version", 1u) ||
			nullptr == (OperationOrder = Required(
				*Oracle, "operationOrder", DATA_JSON_TYPE::ARRAY)) ||
			OperationOrder->Get_Array().size() != ExpectedOperations.size() ||
			!Read_U32(*Oracle, "inputSampleCountPerFamily", InputSampleCount) ||
			InputSampleCount != 4u ||
			nullptr == (InputSamples = Required(
				*Oracle, "inputSamples", DATA_JSON_TYPE::ARRAY)) ||
			InputSamples->Get_Array().size() != InputSampleCount ||
			!Read_FiniteF64(*Oracle, "numericTolerance", NumericTolerance) ||
			NumericTolerance != 2.0e-5 ||
			!Read_ExpectedString(
				*Oracle, "fidelity", "RECONSTRUCTED_NUMERICALLY_VERIFIED") ||
			!Read_ExpectedBoolean(*Oracle, "sourceExact", false) ||
			!Verify_CanonicalSelfSha256(
				*Oracle, "contractSha256", OracleSha256))
		{
			return false;
		}
		for (size_t Index = 0u; Index < ExpectedOperations.size(); ++Index)
		{
			const DATA_JSON_VALUE& Operation = OperationOrder->Get_Array()[Index];
			if (!Operation.Is_String() ||
				Operation.Get_String() != ExpectedOperations[Index])
			{
				return false;
			}
		}
		EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_CONTRACT ParsedOracle;
		ParsedOracle.iVersion = 1u;
		ParsedOracle.fNumericTolerance = NumericTolerance;
		ParsedOracle.strFidelity = "RECONSTRUCTED_NUMERICALLY_VERIFIED";
		ParsedOracle.bSourceExact = false;
		for (size_t Index = 0u; Index < ExpectedOperations.size(); ++Index)
			ParsedOracle.OperationOrder[Index] = ExpectedOperations[Index];
		for (size_t Index = 0u; Index < InputSamples->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Sample = InputSamples->Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_MATERIAL_EVALUATOR_INPUT Parsed;
			if (!Has_ExactKeyOrder(Sample, {
					"sampleId", "time", "uvScale", "panRotationAux", "texture0",
					"texture1", "color", "params0", "params1" }) ||
				!Read_ExpectedString(Sample, "sampleId",
					"sample-" + std::to_string(Index)) ||
				!Read_FiniteF64(Sample, "time", Parsed.fTime))
			{
				return false;
			}
			Parsed.strSampleId = "sample-" + std::to_string(Index);
			const DATA_JSON_VALUE* UvScale = Required(
				Sample, "uvScale", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Pan = Required(
				Sample, "panRotationAux", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Texture0 = Required(
				Sample, "texture0", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Texture1 = Required(
				Sample, "texture1", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Color = Required(
				Sample, "color", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Params0 = Required(
				Sample, "params0", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Params1 = Required(
				Sample, "params1", DATA_JSON_TYPE::ARRAY);
			if (nullptr == UvScale || nullptr == Pan || nullptr == Texture0 ||
				nullptr == Texture1 || nullptr == Color || nullptr == Params0 ||
				nullptr == Params1 || !Parse_FixedF64Array(*UvScale, Parsed.vUvScale) ||
				!Parse_FixedF64Array(*Pan, Parsed.vPanRotationAux) ||
				!Parse_FixedF64Array(*Texture0, Parsed.vTexture0) ||
				!Parse_FixedF64Array(*Texture1, Parsed.vTexture1) ||
				!Parse_FixedF64Array(*Color, Parsed.vColor) ||
				!Parse_FixedF64Array(*Params0, Parsed.vParams0) ||
				!Parse_FixedF64Array(*Params1, Parsed.vParams1))
			{
				return false;
			}
			ParsedOracle.InputSamples.push_back(std::move(Parsed));
		}
		ParsedOracle.strContractSha256 = OracleSha256;
		OutProgram.strMaterialEvaluatorOracleContractSha256 = OracleSha256;
		OutProgram.MaterialEvaluatorOracleContract = std::move(ParsedOracle);
		return true;
	}

	bool_t Parse_CapabilitySample(
		const DATA_JSON_VALUE& Value,
		const uint32_t ExpectedOrder,
		const std::string& ExpectedOwnerModuleId,
		const std::string& ExpectedVariant,
		EFFECT_RUNTIME_PROGRAM_CAPABILITY_SAMPLE& OutSample)
	{
		const DATA_JSON_VALUE* RandomUnits = Required(
			Value, "randomUnits", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* InputValues = Required(
			Value, "inputValues", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* OutputValues = Required(
			Value, "outputValues", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"sampleId", "order", "ownerModuleId", "time", "fixedSeed",
				"fixedSeedSource", "randomUnits", "inputVariant",
				"inputLiteralProjectionSha256", "inputDistributionProjectionSha256",
				"inputValues", "outputVariant", "outputValues", "typedInputSha256",
				"outputSha256", "absoluteTolerance", "relativeTolerance" }) ||
			!Read_RuntimeId(Value, "sampleId", OutSample.strSampleId) ||
			!Read_U32(Value, "order", OutSample.iOrder) ||
			OutSample.iOrder != ExpectedOrder ||
			!Read_RuntimeId(Value, "ownerModuleId", OutSample.strOwnerModuleId) ||
			OutSample.strOwnerModuleId != ExpectedOwnerModuleId ||
			!Read_FiniteF64(Value, "time", OutSample.fTime) ||
			!Read_I32(Value, "fixedSeed", OutSample.iFixedSeed) ||
			!Read_ClosedString(Value, "fixedSeedSource", {
					"POLICY_FIXED_OCCURRENCE_SEED",
					"SOURCE_DECODED_FIRST_RANDOM_SEED" },
				OutSample.strFixedSeedSource) || nullptr == RandomUnits ||
			!Parse_FixedF64Array(*RandomUnits, OutSample.RandomUnits) ||
			!Read_String(Value, "inputVariant", OutSample.strInputVariant,
				false, 256u) || OutSample.strInputVariant != ExpectedVariant ||
			!Read_Sha(Value, "inputLiteralProjectionSha256",
				OutSample.strInputLiteralProjectionSha256) ||
			!Read_Sha(Value, "inputDistributionProjectionSha256",
				OutSample.strInputDistributionProjectionSha256) ||
			nullptr == InputValues || !Parse_FiniteF64Array(
				*InputValues, OutSample.InputValues, 1u, 256u) ||
			!Read_String(Value, "outputVariant", OutSample.strOutputVariant,
				false, 256u) || OutSample.strOutputVariant != ExpectedVariant ||
			nullptr == OutputValues || !Parse_FiniteF64Array(
				*OutputValues, OutSample.OutputValues, 1u, 256u) ||
			!Read_Sha(Value, "typedInputSha256", OutSample.strTypedInputSha256) ||
			!Read_Sha(Value, "outputSha256", OutSample.strOutputSha256) ||
			!Read_FiniteF64(Value, "absoluteTolerance",
				OutSample.fAbsoluteTolerance) ||
			!Read_FiniteF64(Value, "relativeTolerance",
				OutSample.fRelativeTolerance) ||
			OutSample.fAbsoluteTolerance != 1.0e-6 ||
			OutSample.fRelativeTolerance != 1.0e-6)
		{
			return false;
		}
		return true;
	}

	bool_t Parse_ApprovalSourceReceipt(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_APPROVAL_SOURCE_RECEIPT& OutReceipt)
	{
		const DATA_JSON_VALUE* Preserved = Required(
			Value, "preservedEvidenceBlockers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Guards = Required(
			Value, "policyFidelityGuards", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Oracles = Required(
			Value, "requiredOracleIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Execution = Required(
			Value, "executionBlockers", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"policyRowId", "upstreamIdentitySha256", "policyBindingSha256",
				"exactSourceClass", "moduleOccurrenceId", "upstreamClusterId",
				"requiredMutatedOutput", "upstreamDecision", "policyFamilyId",
				"evidenceFidelity", "executionFidelity", "sourceExact",
				"preservedEvidenceBlockers", "policyFidelityGuards",
				"requiredOracleIds", "executionBlockers", "executionAdmission",
				"productAdmission" }) &&
			Read_RuntimeId(Value, "policyRowId", OutReceipt.strPolicyRowId) &&
			Read_Sha(Value, "upstreamIdentitySha256",
				OutReceipt.strUpstreamIdentitySha256) &&
			Read_Sha(Value, "policyBindingSha256",
				OutReceipt.strPolicyBindingSha256) &&
			Read_String(Value, "exactSourceClass", OutReceipt.strExactSourceClass,
				false, 256u) &&
			Read_RuntimeId(Value, "moduleOccurrenceId",
				OutReceipt.strModuleOccurrenceId) &&
			Read_RuntimeId(Value, "upstreamClusterId",
				OutReceipt.strUpstreamClusterId) &&
			Read_String(Value, "requiredMutatedOutput",
				OutReceipt.strRequiredMutatedOutput, false, 1024u) &&
			Read_ExpectedString(Value, "upstreamDecision",
				"BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER") &&
			(OutReceipt.strUpstreamDecision =
				"BLOCKED_NO_SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER", true) &&
			Read_RuntimeId(Value, "policyFamilyId", OutReceipt.strPolicyFamilyId) &&
			Read_ExpectedString(Value, "evidenceFidelity",
				"SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED") &&
			(OutReceipt.strEvidenceFidelity =
				"SOURCE_ERA_ACTUAL_OUTPUT_PROVIDER_NOT_ACQUIRED", true) &&
			Read_ExpectedString(Value, "executionFidelity",
				"RECONSTRUCTED_APPROVED_V1_NOT_IMPLEMENTED") &&
			(OutReceipt.strExecutionFidelity =
				"RECONSTRUCTED_APPROVED_V1_NOT_IMPLEMENTED", true) &&
			Read_Boolean(Value, "sourceExact", OutReceipt.bSourceExact) &&
			!OutReceipt.bSourceExact && nullptr != Preserved &&
			Parse_Blockers(*Preserved, OutReceipt.PreservedEvidenceBlockers) &&
			nullptr != Guards && Parse_StringArray(
				*Guards, OutReceipt.PolicyFidelityGuards, false, 16u, 256u) &&
			nullptr != Oracles && Parse_StringArray(
				*Oracles, OutReceipt.RequiredOracleIds, false, 16u, 256u) &&
			nullptr != Execution &&
			Parse_Blockers(*Execution, OutReceipt.ExecutionBlockers) &&
			Read_Boolean(Value, "executionAdmission",
				OutReceipt.bExecutionAdmission) && !OutReceipt.bExecutionAdmission &&
			Read_Boolean(Value, "productAdmission",
				OutReceipt.bProductAdmission) && !OutReceipt.bProductAdmission;
	}

	bool_t Parse_ReconstructedCoreRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram)
	{
		const DATA_JSON_VALUE* InputArtifacts = Required(
			Root, "inputArtifacts", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Handlers = Required(
			Root, "handlerRegistry", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Emitters = Required(
			Root, "emitters", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Schedules = Required(
			Root, "actionSchedules", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Modules = Required(
			Root, "modules", DATA_JSON_TYPE::ARRAY);
		if (nullptr == InputArtifacts || nullptr == Handlers || nullptr == Emitters ||
			nullptr == Schedules || nullptr == Modules)
		{
			return false;
		}

		OutProgram.InputArtifacts.clear();
		OutProgram.InputArtifacts.reserve(InputArtifacts->Get_Array().size());
		for (size_t Index = 0u; Index < InputArtifacts->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = InputArtifacts->Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_INPUT_ARTIFACT Parsed;
			uint32_t Order = 0u;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Read_RuntimeId(Row, "artifactId", Parsed.Row.strId) ||
				!Read_U32(Row, "order", Order) || Order != Index ||
				!Read_NormalizedRelativePath(Row, "path", "Data/Effects/",
					Parsed.strPath) ||
				!Read_LowerHex(Row, "authorityCommitId", 40u,
					Parsed.strAuthorityCommitId) ||
				!Read_LowerHex(Row, "authorityTreeId", 40u,
					Parsed.strAuthorityTreeId) ||
				!Read_LowerHex(Row, "blobId", 40u, Parsed.strBlobId) ||
				!Read_String(Row, "schema", Parsed.strSchema, true, 256u) ||
				!Read_String(Row, "versionField", Parsed.strVersionField, true, 64u) ||
				!Read_U32(Row, "versionValue", Parsed.iVersionValue) ||
				!Read_ExpectedString(
					Row, "hashDomain", "GIT_OBJECT_CANONICAL_LF_JSON") ||
				!Read_Sha(Row, "trackedTextSha256", Parsed.strTrackedTextSha256) ||
				!Read_Sha(Row, "canonicalJsonSha256", Parsed.strCanonicalJsonSha256) ||
				!Read_String(Row, "selfHashField", Parsed.strSelfHashField, true, 64u) ||
				!Read_OptionalSha(Row, "selfSha256", Parsed.strSelfSha256) ||
				!Read_Sha(Row, "rowSha256", Parsed.Row.strRowSha256))
			{
				return false;
			}
			Parsed.Row.iOrder = Order;
			Parsed.Row.bSourceExact = false;
			Parsed.strHashDomain = "GIT_OBJECT_CANONICAL_LF_JSON";
			OutProgram.InputArtifacts.push_back(std::move(Parsed));
		}
		if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
					*InputArtifacts)) != ACTIVE_RECONSTRUCTED_PROFILE.InputArtifactsSha256)
		{
			return false;
		}

		OutProgram.Handlers.clear();
		OutProgram.Handlers.reserve(Handlers->Get_Array().size());
		for (size_t Index = 0u; Index < Handlers->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Handlers->Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_HANDLER Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "handlerRegistryId",
					static_cast<uint32_t>(Index), Parsed.Row) ||
				!Parse_HandlerKind(Row, Parsed.eKind) ||
				!Read_RuntimeId(Row, "implementationId", Parsed.strImplementationId) ||
				!Read_U32(Row, "implementationVersion", Parsed.iImplementationVersion) ||
				!Read_Sha(Row, "implementationSha256", Parsed.strImplementationSha256) ||
				!Read_String(Row, "exactSourceClass", Parsed.strExactSourceClass,
					true, 256u) ||
				!Read_String(Row, "variant", Parsed.strVariant, false, 256u) ||
				!Read_String(Row, "consumerContract", Parsed.strConsumerContract,
					true, 1024u) ||
				!Read_Sha(Row, "contractSha256", Parsed.strContractSha256))
			{
				return false;
			}
			OutProgram.Handlers.push_back(std::move(Parsed));
		}

		OutProgram.Emitters.clear();
		OutProgram.Emitters.reserve(Emitters->Get_Array().size());
		for (size_t Index = 0u; Index < Emitters->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Emitters->Get_Array()[Index];
			const DATA_JSON_VALUE* RendererConfig = Required(
				Row, "rendererRuntimeConfig", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Timing = Required(
				Row, "timing", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Random = Required(
				Row, "random", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* CueTransform = Required(
				Row, "cueLocalTransform", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* DetailTransform = Required(
				Row, "detailTransform", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Attachment = Required(
				Row, "actionCueAttachment", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Inheritance = Required(
				Row, "transformInheritance", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* CapBlockers = Required(
				Row, "operationalCapBlockers", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Composition = Required(
				Row, "transformCompositionOrder", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Anchors = Required(
				Row, "anchorRequests", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ModuleIds = Required(
				Row, "moduleIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ActionCueInputs = Required(
				Row, "actionCueParameterInputs", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* TextureIds = Required(
				Row, "textureResourceIds", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_EMITTER Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "emitterId",
					static_cast<uint32_t>(Index), Parsed.Row) ||
				!Read_RuntimeId(Row, "evidenceId", Parsed.strEvidenceId) ||
				!Read_RuntimeId(Row, "sourceOccurrenceId", Parsed.strSourceOccurrenceId) ||
				!Read_RuntimeId(Row, "sourceSystemId", Parsed.strSourceSystemId) ||
				!Read_RuntimeId(Row, "sourceEmitterPath", Parsed.strSourceEmitterPath) ||
				!Read_RuntimeId(Row, "sourceEmitterNodeId", Parsed.strSourceEmitterNodeId) ||
				!Read_RuntimeId(Row, "sourceCueId", Parsed.strSourceCueId) ||
				!Read_RuntimeId(Row, "sourceElementId", Parsed.strSourceElementId) ||
				!Read_RuntimeId(Row, "sourceNode", Parsed.strSourceNode) ||
				!Read_Sha(Row, "sourceActionCueProjectionSha256",
					Parsed.strSourceActionCueProjectionSha256) ||
				!Read_Boolean(Row, "visible", Parsed.bVisible) ||
				!Parse_RendererKind(Row, Parsed.eRenderer) ||
				!Read_String(Row, "rendererSourceSpace",
					Parsed.strRendererSourceSpace, false, 256u) ||
				!Read_String(Row, "selectedLodPath",
					Parsed.strSelectedLodPath, false, 1024u) ||
				!Read_RuntimeId(Row, "selectedLodNodeId", Parsed.strSelectedLodNodeId) ||
				!Read_Sha(Row, "selectedLodRecordSha256",
					Parsed.strSelectedLodRecordSha256) ||
				!Read_ClosedString(Row, "selectedLodDecision",
					{ "VERIFIED_IRRELEVANT" }, Parsed.strSelectedLodDecision) ||
				!Read_RuntimeId(Row, "selectedLodOracleId", Parsed.strSelectedLodOracleId) ||
				!Read_ExpectedBoolean(Row, "sourceRecipeEnabled", true) ||
				!Read_Boolean(Row, "localSpace", Parsed.bLocalSpace) ||
				!Read_ClosedString(Row, "sizeUnitPolicy", {
					"DIMENSIONLESS_AXIS_REORDER_ONLY", "SCREEN_SPACE_UNITLESS",
					"UE3_LENGTH_XZY_0P01" }, Parsed.strSizeUnitPolicy) ||
				!Read_U32(Row, "operationalMaxParticles",
					Parsed.iOperationalMaxParticles) ||
				Parsed.iOperationalMaxParticles == 0u ||
				!Read_U32(Row, "sourcePeakActiveParticles",
					Parsed.iSourcePeakActiveParticles) ||
				Parsed.iSourcePeakActiveParticles != Parsed.iOperationalMaxParticles ||
				!Read_ClosedString(Row, "spawnRateFallbackPolicy",
					{ "FORBIDDEN_TYPED_OPCODE_REQUIRED" },
					Parsed.strSpawnRateFallbackPolicy) ||
				!Read_ClosedString(Row, "lifetimeFallbackPolicy",
					{ "FORBIDDEN_TYPED_OPCODE_REQUIRED" },
					Parsed.strLifetimeFallbackPolicy) ||
				!Read_ClosedString(Row, "operationalCapPolicy",
					{ "SOURCE_LOD_PEAK_ACTIVE_PARTICLES_V1" },
					Parsed.strOperationalCapPolicy) ||
				!Read_Sha(Row, "operationalCapProjectionSha256",
					Parsed.strOperationalCapProjectionSha256) ||
				nullptr == CapBlockers ||
				!Parse_Blockers(*CapBlockers, Parsed.OperationalCapBlockers) ||
				Parsed.OperationalCapBlockers != std::vector<std::string>{
					"R3_OPERATIONAL_PARTICLE_CAP_NOT_EXECUTED" } ||
				nullptr == RendererConfig ||
				!Parse_RendererRuntimeConfig(
					*RendererConfig, Parsed.eRenderer, Parsed.RendererRuntimeConfig) ||
				!Parse_NullableProgramObject(
					Row, "screenPostAdapter", Parse_ScreenPostAdapter,
					Parsed.ScreenPostAdapter) ||
				!Parse_NullableProgramObject(
					Row, "lightAdapter", Parse_LightAdapter, Parsed.LightAdapter) ||
				!Parse_NullableProgramObject(
					Row, "decalAdapter", Parse_DecalAdapter, Parsed.DecalAdapter) ||
				!Parse_NullableProgramObject(
					Row, "ribbonAdapter", Parse_RibbonAdapter, Parsed.RibbonAdapter) ||
				nullptr == Timing || !Parse_ProgramTiming(*Timing, Parsed.Timing) ||
				nullptr == Random || !Parse_ProgramRandom(*Random, Parsed.Random) ||
				nullptr == CueTransform ||
				!Parse_ProgramCueTransform(*CueTransform, Parsed.CueLocalTransform) ||
				nullptr == DetailTransform ||
				!Parse_ProgramDetailTransform(*DetailTransform, Parsed.DetailTransform) ||
				nullptr == Attachment ||
				!Parse_ProgramAttachment(*Attachment, Parsed.ActionCueAttachment) ||
				nullptr == Inheritance || !Parse_ProgramTransformInheritance(
					*Inheritance, Parsed.TransformInheritance) ||
				nullptr == Composition || !Parse_StringArray(
					*Composition, Parsed.TransformCompositionOrder, false, 7u, 128u) ||
				Parsed.TransformCompositionOrder != std::vector<std::string>{
					"carrierGeometryPreScale", "signedParticleScaleRotationLocation",
					"emitterElementTransform", "cueLocalTransform",
					"snapshotRootSourceBasis", "attachmentSocketOrRoot", "actorWorld" } ||
				nullptr == Anchors ||
				!Parse_ProgramAnchorRequests(*Anchors, Parsed.AnchorRequests) ||
				nullptr == ModuleIds ||
				!Parse_StringArray(*ModuleIds, Parsed.ModuleIds, false, 64u, 1024u) ||
				nullptr == ActionCueInputs || !Parse_ProgramActionCueValues(
					*ActionCueInputs, Parsed.ActionCueParameterInputs) ||
				!Read_RuntimeId(Row, "scheduleId", Parsed.strScheduleId) ||
				!Read_NullableRuntimeIdValue(
					Row, "materialOccurrenceId", Parsed.strMaterialOccurrenceId) ||
				nullptr == TextureIds ||
				!Parse_StringArray(
					*TextureIds, Parsed.TextureResourceIds, false, 16u, 1024u) ||
				!Read_NullableRuntimeIdValue(
					Row, "geometryUseId", Parsed.strGeometryUseId))
			{
				return false;
			}
			Parsed.bSourceRecipeEnabled = true;
			const bool_t ScreenExpected =
				Parsed.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST;
			const bool_t LightExpected =
				Parsed.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE;
			const bool_t DecalExpected =
				Parsed.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE;
			const bool_t RibbonExpected =
				Parsed.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON;
			if (Parsed.ScreenPostAdapter.has_value() != ScreenExpected ||
				Parsed.LightAdapter.has_value() != LightExpected ||
				Parsed.DecalAdapter.has_value() != DecalExpected ||
				Parsed.RibbonAdapter.has_value() != RibbonExpected ||
				Parsed.strMaterialOccurrenceId.has_value() == LightExpected ||
				Parsed.strGeometryUseId.has_value() !=
					(Parsed.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE))
			{
				return false;
			}
			OutProgram.Emitters.push_back(std::move(Parsed));
		}

		OutProgram.ActionSchedules.clear();
		OutProgram.ActionSchedules.reserve(Schedules->Get_Array().size());
		for (size_t Index = 0u; Index < Schedules->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Schedules->Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "scheduleId",
					static_cast<uint32_t>(Index), Parsed.Row, false) ||
				!Read_RuntimeId(Row, "sourceCueId", Parsed.strSourceCueId) ||
				!Read_RuntimeId(Row, "sourceOccurrenceId", Parsed.strSourceOccurrenceId) ||
				!Read_RuntimeId(Row, "sourceSystemId", Parsed.strSourceSystemId) ||
				!Read_U32(Row, "sourceReceiptEventIndex",
					Parsed.iSourceReceiptEventIndex) ||
				!Read_FiniteF64(Row, "globalTimeSeconds", Parsed.fGlobalTimeSeconds) ||
				!Read_FiniteF64(Row, "durationSeconds", Parsed.fDurationSeconds) ||
				!Read_Sha(Row, "sourceCueRowSha256", Parsed.strSourceCueRowSha256))
			{
				return false;
			}
			OutProgram.ActionSchedules.push_back(std::move(Parsed));
		}

		OutProgram.Modules.clear();
		OutProgram.Modules.reserve(Modules->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Modules->Get_Array())
		{
			const DATA_JSON_VALUE* PropertyIds = Required(
				Row, "propertyIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* LeafIds = Required(
				Row, "primitiveLeafIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* LiteralIds = Required(
				Row, "literalIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* DistributionIds = Required(
				Row, "distributionIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* DefaultIds = Required(
				Row, "implicitDefaultIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* CapabilitySamples = Required(
				Row, "capabilityNumericSamples", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* CapabilityActionCueNames = Required(
				Row, "capabilityActionCueInputNames", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalOracles = Required(
				Row, "approvalRequiredOracleIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalBlockers = Required(
				Row, "approvalExecutionBlockers", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalSourceRow = Row.Find("approvalSourceRow");
			const DATA_JSON_VALUE* PreservedBlockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MODULE Parsed;
			if (!Parse_RowIdentity(Row, "moduleId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "emitterId", Parsed.strEmitterId) ||
				!Read_RuntimeId(Row, "sourceObjectId", Parsed.strSourceObjectId) ||
				!Read_Sha(Row, "sourceRecordSha256", Parsed.strSourceRecordSha256) ||
				!Read_String(Row, "exactSourceClass", Parsed.strExactSourceClass,
					false, 256u) ||
				!Read_ClosedEnum(Row, "selectionKind", {
					{ "SOURCE_HANDLER", EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER },
					{ "RECONSTRUCTED_HANDLER",
						EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER },
				}, Parsed.eSelection) ||
				!Read_RuntimeId(Row, "handlerRegistryId", Parsed.strHandlerRegistryId) ||
				!Read_String(Row, "handlerVariant", Parsed.strHandlerVariant,
					false, 256u) ||
				!Read_OptionalRuntimeId(Row, "capabilityPolicyFamilyId",
					Parsed.strCapabilityPolicyFamilyId) ||
				!Read_OptionalRuntimeId(Row, "capabilityImplementationId",
					Parsed.strCapabilityImplementationId) ||
				!Read_U32(Row, "capabilityImplementationVersion",
					Parsed.iCapabilityImplementationVersion) ||
				!Read_OptionalSha(Row, "capabilityImplementationSha256",
					Parsed.strCapabilityImplementationSha256) ||
				!Read_OptionalSha(Row, "capabilityFamilySemanticSha256",
					Parsed.strCapabilityFamilySemanticSha256) ||
				!Read_OptionalSha(Row, "capabilityInputSchemaSha256",
					Parsed.strCapabilityInputSchemaSha256) ||
				!Read_OptionalSha(Row, "capabilityOutputSchemaSha256",
					Parsed.strCapabilityOutputSchemaSha256) ||
				!Read_OptionalSha(Row, "capabilityDefaultPolicySha256",
					Parsed.strCapabilityDefaultPolicySha256) ||
				nullptr == CapabilitySamples || CapabilitySamples->Get_Array().size() > 3u ||
				!Read_OptionalSha(Row, "capabilitySourceRowSha256",
					Parsed.strCapabilitySourceRowSha256) ||
				!Read_OptionalSha(Row, "capabilityLiteralBindingsSha256",
					Parsed.strCapabilityLiteralBindingsSha256) ||
				!Read_OptionalSha(Row, "capabilityDistributionBindingsSha256",
					Parsed.strCapabilityDistributionBindingsSha256) ||
				!Read_OptionalSha(Row, "capabilityPropertyConsumptionSha256",
					Parsed.strCapabilityPropertyConsumptionSha256) ||
				!Read_OptionalSha(Row, "capabilitySeedBindingSha256",
					Parsed.strCapabilitySeedBindingSha256) ||
				nullptr == CapabilityActionCueNames ||
				!Parse_StringArray(*CapabilityActionCueNames,
					Parsed.CapabilityActionCueInputNames, false, 16u, 256u) ||
				!Read_OptionalSha(Row, "capabilityActionCueInputsSha256",
					Parsed.strCapabilityActionCueInputsSha256) ||
				!Read_OptionalRuntimeId(Row, "approvalPolicyRowId",
					Parsed.strApprovalPolicyRowId) ||
				!Read_OptionalSha(Row, "approvalUpstreamIdentitySha256",
					Parsed.strApprovalUpstreamIdentitySha256) ||
				!Read_OptionalSha(Row, "approvalPolicyBindingSha256",
					Parsed.strApprovalPolicyBindingSha256) ||
				!Read_String(Row, "approvalRequiredMutatedOutput",
					Parsed.strApprovalRequiredMutatedOutput, true, 1024u) ||
				!Read_I32(Row, "approvalSourceOrder", Parsed.iApprovalSourceOrder) ||
				nullptr == ApprovalOracles || !Parse_StringArray(*ApprovalOracles,
					Parsed.ApprovalRequiredOracleIds, false, 16u, 256u) ||
				nullptr == ApprovalBlockers || !Parse_Blockers(
					*ApprovalBlockers, Parsed.ApprovalExecutionBlockers) ||
				nullptr == ApprovalSourceRow ||
				!Read_OptionalSha(Row, "approvalSourceRowSha256",
					Parsed.strApprovalSourceRowSha256) ||
				nullptr == PropertyIds ||
				!Parse_StringArray(*PropertyIds, Parsed.PropertyIds, false, 64u, 1024u) ||
				nullptr == LeafIds ||
				!Parse_StringArray(*LeafIds, Parsed.PrimitiveLeafIds, false, 64u, 1024u) ||
				nullptr == LiteralIds ||
				!Parse_StringArray(*LiteralIds, Parsed.LiteralIds, false, 64u, 1024u) ||
				nullptr == DistributionIds ||
				!Parse_StringArray(
					*DistributionIds, Parsed.DistributionIds, false, 64u, 1024u) ||
				!Read_OptionalRuntimeId(Row, "seedPolicyId", Parsed.strSeedPolicyId) ||
				nullptr == DefaultIds ||
				!Parse_StringArray(
					*DefaultIds, Parsed.ImplicitDefaultIds, false, 16u, 1024u) ||
				nullptr == PreservedBlockers ||
				!Parse_Blockers(*PreservedBlockers, Parsed.PreservedBlockers) ||
				!Read_ClosedString(Row, "sourceDecision",
					{ "READY_FOR_HANDLER", "BLOCKED" },
					Parsed.strSourceDecision))
			{
				return false;
			}
			const bool_t Reconstructed = Parsed.eSelection ==
				EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER;
			if (Reconstructed)
			{
				const std::array<const std::string*, 17u> RequiredStrings{
					&Parsed.strCapabilityPolicyFamilyId,
					&Parsed.strCapabilityImplementationId,
					&Parsed.strCapabilityImplementationSha256,
					&Parsed.strCapabilityFamilySemanticSha256,
					&Parsed.strCapabilityInputSchemaSha256,
					&Parsed.strCapabilityOutputSchemaSha256,
					&Parsed.strCapabilityDefaultPolicySha256,
					&Parsed.strCapabilitySourceRowSha256,
					&Parsed.strCapabilityLiteralBindingsSha256,
					&Parsed.strCapabilityDistributionBindingsSha256,
					&Parsed.strCapabilityPropertyConsumptionSha256,
					&Parsed.strCapabilitySeedBindingSha256,
					&Parsed.strCapabilityActionCueInputsSha256,
					&Parsed.strApprovalPolicyRowId,
					&Parsed.strApprovalUpstreamIdentitySha256,
					&Parsed.strApprovalPolicyBindingSha256,
					&Parsed.strApprovalSourceRowSha256,
				};
				if (Parsed.iCapabilityImplementationVersion == 0u ||
					Parsed.iApprovalSourceOrder < 0 ||
					Parsed.strApprovalRequiredMutatedOutput.empty() ||
					Parsed.ApprovalRequiredOracleIds.empty() ||
					Parsed.ApprovalExecutionBlockers.empty() ||
					CapabilitySamples->Get_Array().size() != 3u ||
					std::any_of(RequiredStrings.begin(), RequiredStrings.end(),
						[](const std::string* Value) { return Value->empty(); }) ||
					!ApprovalSourceRow->Is_Object())
				{
					return false;
				}
				for (size_t Index = 0u;
					Index < CapabilitySamples->Get_Array().size(); ++Index)
				{
					EFFECT_RUNTIME_PROGRAM_CAPABILITY_SAMPLE Sample;
					if (!Parse_CapabilitySample(CapabilitySamples->Get_Array()[Index],
							static_cast<uint32_t>(Index), Parsed.Row.strId,
							Parsed.strHandlerVariant, Sample))
					{
						return false;
					}
					Parsed.CapabilityNumericSamples.push_back(std::move(Sample));
				}
				EFFECT_RUNTIME_PROGRAM_APPROVAL_SOURCE_RECEIPT Receipt;
				if (!Parse_ApprovalSourceReceipt(*ApprovalSourceRow, Receipt) ||
					Receipt.strPolicyRowId != Parsed.strApprovalPolicyRowId ||
					Receipt.strUpstreamIdentitySha256 !=
						Parsed.strApprovalUpstreamIdentitySha256 ||
					Receipt.strPolicyBindingSha256 !=
						Parsed.strApprovalPolicyBindingSha256 ||
					Receipt.strExactSourceClass != Parsed.strExactSourceClass ||
					Receipt.strModuleOccurrenceId != Parsed.Row.strId ||
					Receipt.strRequiredMutatedOutput !=
						Parsed.strApprovalRequiredMutatedOutput ||
					Receipt.RequiredOracleIds != Parsed.ApprovalRequiredOracleIds ||
					Receipt.ExecutionBlockers != Parsed.ApprovalExecutionBlockers ||
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							*ApprovalSourceRow)) != Parsed.strApprovalSourceRowSha256)
				{
					return false;
				}
				Parsed.ApprovalSourceReceipt = std::move(Receipt);
			}
			else
			{
				const std::array<const std::string*, 17u> EmptyStrings{
					&Parsed.strCapabilityPolicyFamilyId,
					&Parsed.strCapabilityImplementationId,
					&Parsed.strCapabilityImplementationSha256,
					&Parsed.strCapabilityFamilySemanticSha256,
					&Parsed.strCapabilityInputSchemaSha256,
					&Parsed.strCapabilityOutputSchemaSha256,
					&Parsed.strCapabilityDefaultPolicySha256,
					&Parsed.strCapabilitySourceRowSha256,
					&Parsed.strCapabilityLiteralBindingsSha256,
					&Parsed.strCapabilityDistributionBindingsSha256,
					&Parsed.strCapabilityPropertyConsumptionSha256,
					&Parsed.strCapabilitySeedBindingSha256,
					&Parsed.strCapabilityActionCueInputsSha256,
					&Parsed.strApprovalPolicyRowId,
					&Parsed.strApprovalUpstreamIdentitySha256,
					&Parsed.strApprovalPolicyBindingSha256,
					&Parsed.strApprovalSourceRowSha256,
				};
				if (Parsed.iCapabilityImplementationVersion != 0u ||
					Parsed.iApprovalSourceOrder != -1 ||
					!CapabilitySamples->Get_Array().empty() ||
					!Parsed.CapabilityActionCueInputNames.empty() ||
					!Parsed.strApprovalRequiredMutatedOutput.empty() ||
					!Parsed.ApprovalRequiredOracleIds.empty() ||
					!Parsed.ApprovalExecutionBlockers.empty() ||
					!ApprovalSourceRow->Is_Null() ||
					std::any_of(EmptyStrings.begin(), EmptyStrings.end(),
						[](const std::string* Value) { return !Value->empty(); }))
				{
					return false;
				}
			}
			OutProgram.Modules.push_back(std::move(Parsed));
		}
		return true;
	}

	bool_t Parse_PropertyRows(
		const DATA_JSON_VALUE& Root,
		std::vector<EFFECT_RUNTIME_PROGRAM_PROPERTY>& OutRows)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "properties", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_PROPERTY> Staged;
		Staged.reserve(Section->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			const DATA_JSON_VALUE* PayloadLiterals = Required(
				Row, "payloadLiteralIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* PayloadDistributions = Required(
				Row, "payloadDistributionIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* SemanticDistributions = Required(
				Row, "semanticDistributionIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_PROPERTY Parsed;
			if (!Parse_RowIdentity(Row, "propertyId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "moduleId", Parsed.strModuleId) ||
				!Read_String(Row, "propertyPath", Parsed.strPropertyPath,
					false, 4096u) ||
				!Read_RuntimeId(Row, "handlerRegistryId",
					Parsed.strHandlerRegistryId) ||
				!Read_ClosedString(Row, "consumptionDecision", {
						"READY_FOR_HANDLER", "VERIFIED_IRRELEVANT", "BLOCKED" },
					Parsed.strConsumptionDecision) ||
				!Read_OptionalRuntimeId(Row, "irrelevanceOracleId",
					Parsed.strIrrelevanceOracleId) || nullptr == PayloadLiterals ||
				!Parse_StringArray(*PayloadLiterals, Parsed.PayloadLiteralIds,
					false, 4096u, 1024u) || nullptr == PayloadDistributions ||
				!Parse_StringArray(*PayloadDistributions,
					Parsed.PayloadDistributionIds, false, 4096u, 1024u) ||
				nullptr == SemanticDistributions ||
				!Parse_StringArray(*SemanticDistributions,
					Parsed.SemanticDistributionIds, false, 4096u, 1024u) ||
				!Read_ClosedString(Row, "sourceFidelity",
					{ "SOURCE_EXACT", "CURRENT_REVISION_EVIDENCE" },
					Parsed.strSourceFidelity) ||
				!Read_String(Row, "capabilityConsumptionDecision",
					Parsed.strCapabilityConsumptionDecision, true, 256u) ||
				!Read_String(Row, "semanticRole", Parsed.strSemanticRole,
					true, 256u) ||
				!Read_Boolean(Row, "outputDependencyRequired",
					Parsed.bOutputDependencyRequired) ||
				!Read_OptionalSha(Row, "capabilitySourceRowSha256",
					Parsed.strCapabilitySourceRowSha256) || nullptr == Blockers ||
				!Parse_Blockers(*Blockers, Parsed.PreservedBlockers))
			{
				return false;
			}
			const bool_t CapabilityDecision =
				Parsed.strCapabilityConsumptionDecision.empty() ||
				Parsed.strCapabilityConsumptionDecision ==
					"RECONSTRUCTED_POLICY_INPUT_CONSUMED" ||
				Parsed.strCapabilityConsumptionDecision ==
					"PRESERVED_VERIFIED_IRRELEVANT";
			const bool_t SemanticRole = Parsed.strSemanticRole.empty() ||
				Parsed.strSemanticRole ==
					"EVALUATED_DISTRIBUTION_AND_VARIANT_OUTPUT" ||
				Parsed.strSemanticRole == "UPSTREAM_IRRELEVANCE_ORACLE" ||
				Parsed.strSemanticRole == "FIXED_SEED_AND_RANDOM_STREAM_INPUT" ||
				Parsed.strSemanticRole ==
					"TYPED_LITERAL_AND_VARIANT_OUTPUT_OR_CONTEXT" ||
				Parsed.strSemanticRole ==
					"EXACT_TYPED_IDENTITY_BINDING_SELECTS_VARIANT_INPUT";
			if (!CapabilityDecision || !SemanticRole ||
				(Parsed.strCapabilityConsumptionDecision.empty() !=
					Parsed.strSemanticRole.empty()) ||
				(Parsed.strCapabilityConsumptionDecision.empty() !=
					Parsed.strCapabilitySourceRowSha256.empty()))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutRows = std::move(Staged);
		return true;
	}

	bool_t Parse_PrimitiveLeafRows(
		const DATA_JSON_VALUE& Root,
		std::vector<EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF>& OutRows)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "primitiveLeaves", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF> Staged;
		Staged.reserve(Section->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF Parsed;
			if (!Parse_RowIdentity(Row, "leafId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "moduleId", Parsed.strModuleId) ||
				!Read_RuntimeId(Row, "propertyId", Parsed.strPropertyId) ||
				!Read_String(Row, "propertyPath", Parsed.strPropertyPath,
					false, 4096u) ||
				!Read_String(Row, "topLevelPropertyPath",
					Parsed.strTopLevelPropertyPath, false, 4096u) ||
				!Read_RuntimeId(Row, "literalId", Parsed.strLiteralId) ||
				!Parse_LiteralVariant(Row, "valueVariant", Parsed.eValueVariant) ||
				!Read_RuntimeId(Row, "handlerRegistryId",
					Parsed.strHandlerRegistryId) ||
				!Read_ClosedString(Row, "consumptionDecision",
					{ "READY_FOR_HANDLER", "VERIFIED_IRRELEVANT" },
					Parsed.strConsumptionDecision) || nullptr == Blockers ||
				!Parse_Blockers(*Blockers, Parsed.PreservedBlockers))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutRows = std::move(Staged);
		return true;
	}

	bool_t Parse_DistributionCurveKey(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY& OutKey)
	{
		const DATA_JSON_VALUE* Minimum = Required(
			Value, "minimum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Maximum = Required(
			Value, "maximum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ArriveMinimum = Required(
			Value, "arriveTangentMinimum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LeaveMinimum = Required(
			Value, "leaveTangentMinimum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ArriveMaximum = Required(
			Value, "arriveTangentMaximum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LeaveMaximum = Required(
			Value, "leaveTangentMaximum", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"time", "minimum", "maximum", "arriveTangentMinimum",
				"leaveTangentMinimum", "arriveTangentMaximum",
				"leaveTangentMaximum", "interpolation" }) &&
			Read_FiniteF64(Value, "time", OutKey.fTime) &&
			nullptr != Minimum && Parse_FixedF64Array(*Minimum, OutKey.vMinimum) &&
			nullptr != Maximum && Parse_FixedF64Array(*Maximum, OutKey.vMaximum) &&
			nullptr != ArriveMinimum &&
			Parse_FixedF64Array(*ArriveMinimum, OutKey.vArriveTangentMinimum) &&
			nullptr != LeaveMinimum &&
			Parse_FixedF64Array(*LeaveMinimum, OutKey.vLeaveTangentMinimum) &&
			nullptr != ArriveMaximum &&
			Parse_FixedF64Array(*ArriveMaximum, OutKey.vArriveTangentMaximum) &&
			nullptr != LeaveMaximum &&
			Parse_FixedF64Array(*LeaveMaximum, OutKey.vLeaveTangentMaximum) &&
			Read_ClosedEnum(Value, "interpolation", {
				{ "cubic", EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::CUBIC },
				{ "linear", EFFECT_RUNTIME_DISTRIBUTION_CURVE_INTERPOLATION::LINEAR },
			}, OutKey.eInterpolation);
	}

	bool_t Parse_DistributionFieldProvenance(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_FIELD_PROVENANCE& OutProvenance)
	{
		if (!Has_ExactKeyOrder(Value, {
				"fieldPath", "provenanceTier", "evidenceStatus", "valueSha256",
				"typedValueBindingSha256" }) ||
			!Read_String(Value, "fieldPath", OutProvenance.strFieldPath,
				false, 1024u) ||
			!Read_ClosedString(Value, "provenanceTier", {
				"CURRENT_UE3_RAW_DISTRIBUTION_DEFAULT_AND_PAYLOAD_SHAPE",
				"CURRENT_ENGINE_CDO_RECONSTRUCTED",
				"NULL_RAW_DISTRIBUTION_FAIL_CLOSED",
				"CURRENT_REVISION_INSTANCE_EXPLICIT", "EVALUATOR_DEFAULT",
				"PARENT_CDO_HIERARCHY", "CLASS_CDO",
				"CURRENT_NATIVE_EVALUATOR_DEFAULT", "NESTED_ARCHETYPE_TEMPLATE" },
				OutProvenance.strProvenanceTier) ||
			!Read_ClosedString(Value, "evidenceStatus", {
				"CURRENT_REVISION_EVIDENCE", "SOURCE_EXACT",
				"CURRENT_REVISION_TARGET_EVIDENCE",
				"CURRENT_NATIVE_EVALUATOR_SHAPE_ONLY",
				"CURRENT_REVISION_SCRIPT_EVIDENCE",
				"CURRENT_REVISION_ARCHETYPE_EVIDENCE" },
				OutProvenance.strEvidenceStatus) ||
			!Read_OptionalSha(Value, "valueSha256", OutProvenance.strValueSha256) ||
			!Read_OptionalSha(Value, "typedValueBindingSha256",
				OutProvenance.strTypedValueBindingSha256))
		{
			return false;
		}
		if (OutProvenance.strValueSha256.empty() ==
			OutProvenance.strTypedValueBindingSha256.empty())
		{
			return false;
		}
		if (!OutProvenance.strTypedValueBindingSha256.empty())
		{
			const bool_t CurrentEngineCdo =
				OutProvenance.strProvenanceTier ==
					"CURRENT_ENGINE_CDO_RECONSTRUCTED";
			return (OutProvenance.strFieldPath == "operation" ||
					OutProvenance.strFieldPath == "lookupTableChunkSize" ||
					OutProvenance.strFieldPath == "lookupTableNumElements") &&
				((CurrentEngineCdo &&
				  OutProvenance.strEvidenceStatus == "CURRENT_REVISION_EVIDENCE") ||
				 (!CurrentEngineCdo &&
				  OutProvenance.strProvenanceTier ==
					  "CURRENT_UE3_RAW_DISTRIBUTION_DEFAULT_AND_PAYLOAD_SHAPE" &&
				  (OutProvenance.strEvidenceStatus == "SOURCE_EXACT" ||
				   OutProvenance.strEvidenceStatus == "CURRENT_REVISION_EVIDENCE")));
		}
		return true;
	}

	bool_t Parse_ProgramParameterInput(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT& OutInput)
	{
		if (!Has_ExactKeyOrder(Value, {
				"name", "kind", "value", "sourceIndex", "sourceValueByteOffset" }) ||
			!Read_String(Value, "name", OutInput.strName, false, 1024u) ||
			!Read_ClosedEnum(Value, "kind", {
				{ "scalar", EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR },
				{ "vector", EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::VECTOR },
			}, OutInput.eKind) ||
			!Read_I32(Value, "sourceIndex", OutInput.iSourceIndex) ||
			!Read_I32(Value, "sourceValueByteOffset", OutInput.iSourceValueByteOffset) ||
			OutInput.iSourceIndex < 0 || OutInput.iSourceValueByteOffset < 0)
		{
			return false;
		}
		const DATA_JSON_VALUE* TypedValue = Value.Find("value");
		if (nullptr == TypedValue)
			return false;
		if (OutInput.eKind == EFFECT_RUNTIME_ACTION_CUE_VALUE_KIND::SCALAR)
		{
			if (!TypedValue->Is_Number() || !TypedValue->Was_FloatingPointToken() ||
				!std::isfinite(TypedValue->Get_Number()))
			{
				return false;
			}
			OutInput.fScalarValue = TypedValue->Get_Number();
			OutInput.VectorValue.clear();
			return true;
		}
		OutInput.fScalarValue.reset();
		return Parse_FiniteF64Array(*TypedValue, OutInput.VectorValue, 3u, 4u);
	}

	bool_t Parse_NullableParameterInput(
		const DATA_JSON_VALUE& Value,
		std::optional<EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT>& OutInput)
	{
		if (Value.Is_Null())
		{
			OutInput.reset();
			return true;
		}
		if (!Value.Is_Object())
			return false;
		EFFECT_RUNTIME_PROGRAM_PARAMETER_INPUT Parsed;
		if (!Parse_ProgramParameterInput(Value, Parsed))
			return false;
		OutInput = std::move(Parsed);
		return true;
	}

	bool_t Parse_DistributionSample(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_SAMPLE& OutSample)
	{
		const DATA_JSON_VALUE* RandomUnits = Required(
			Value, "randomUnits", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ParameterInput = Value.Find("parameterInput");
		const DATA_JSON_VALUE* OutputValues = Required(
			Value, "outputValues", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"sampleId", "domain", "inputVariant", "time", "randomUnits",
				"sourceCueId", "branch", "parameterInput",
				"diagnosticStandardBaseValue", "blocked", "actionCueBindingsSha256",
				"inputSha256", "outputSha256", "outputValues", "absoluteTolerance",
				"relativeTolerance" }) ||
			!Read_String(Value, "sampleId", OutSample.strSampleId, false, 2048u) ||
			!Read_ClosedEnum(Value, "domain", {
				{ "DISTRIBUTION_EVALUATOR",
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::DISTRIBUTION_EVALUATOR },
				{ "PARTICLE_PARAMETER_BRANCH",
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::PARTICLE_PARAMETER_BRANCH },
			}, OutSample.eDomain) ||
			!Read_ClosedEnum(Value, "inputVariant", {
				{ "TIME_RANDOM_UNITS",
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::TIME_RANDOM_UNITS },
				{ "PARTICLE_PARAMETER_INPUT",
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::PARTICLE_PARAMETER_INPUT },
			}, OutSample.eInputVariant) ||
			!Read_FiniteF64(Value, "time", OutSample.fTime) ||
			nullptr == RandomUnits ||
			!Parse_FiniteF64Array(*RandomUnits, OutSample.RandomUnits, 0u, 4u) ||
			!Read_NullableRuntimeIdValue(
				Value, "sourceCueId", OutSample.strSourceCueId) ||
			nullptr == ParameterInput ||
			!Parse_NullableParameterInput(*ParameterInput, OutSample.ParameterInput) ||
			!Read_ExpectedNull(Value, "diagnosticStandardBaseValue") ||
			!Read_NullableBoolean(Value, "blocked", OutSample.bBlocked) ||
			!Read_Sha(Value, "actionCueBindingsSha256",
				OutSample.strActionCueBindingsSha256) ||
			!Read_Sha(Value, "inputSha256", OutSample.strInputSha256) ||
			!Read_Sha(Value, "outputSha256", OutSample.strOutputSha256) ||
			nullptr == OutputValues || !Parse_FiniteF64Array(
				*OutputValues, OutSample.OutputValues, 1u, 4u) ||
			!Read_FiniteF64(Value, "absoluteTolerance",
				OutSample.fAbsoluteTolerance) ||
			!Read_FiniteF64(Value, "relativeTolerance",
				OutSample.fRelativeTolerance) ||
			OutSample.fAbsoluteTolerance != 1.0e-6 ||
			OutSample.fRelativeTolerance != 1.0e-6)
		{
			return false;
		}
		OutSample.fDiagnosticStandardBaseValue.reset();
		const DATA_JSON_VALUE* Branch = Value.Find("branch");
		if (nullptr == Branch)
			return false;
		if (Branch->Is_Null())
			OutSample.eBranch.reset();
		else if (Branch->Is_String() && Branch->Get_String() == "PARAMETER_INPUT")
			OutSample.eBranch = EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::PARAMETER_INPUT;
		else if (Branch->Is_String() && Branch->Get_String() == "CONSTANT_FALLBACK")
			OutSample.eBranch = EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::CONSTANT_FALLBACK;
		else
			return false;

		if (OutSample.eDomain ==
			EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::DISTRIBUTION_EVALUATOR)
		{
			return OutSample.eInputVariant ==
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::TIME_RANDOM_UNITS &&
				OutSample.RandomUnits.size() == 4u &&
				!OutSample.strSourceCueId.has_value() && !OutSample.eBranch.has_value() &&
				!OutSample.ParameterInput.has_value() && !OutSample.bBlocked.has_value() &&
				OutSample.OutputValues.size() == 4u;
		}
		if (OutSample.eInputVariant !=
				EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_INPUT_VARIANT::PARTICLE_PARAMETER_INPUT ||
			OutSample.fTime != 0.0 || !OutSample.RandomUnits.empty() ||
			!OutSample.strSourceCueId.has_value() || !OutSample.eBranch.has_value() ||
			!OutSample.bBlocked.has_value() || *OutSample.bBlocked)
		{
			return false;
		}
		return (*OutSample.eBranch ==
				EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_BRANCH::PARAMETER_INPUT) ==
			OutSample.ParameterInput.has_value();
	}

	bool_t Parse_SeedPolicyRow(
		const DATA_JSON_VALUE& Row,
		const uint32_t ExpectedOrder,
		EFFECT_RUNTIME_PROGRAM_SEED_POLICY& OutPolicy)
	{
		const DATA_JSON_VALUE* Seeds = Required(
			Row, "randomSeeds", DATA_JSON_TYPE::ARRAY);
		if (!Parse_RowIdentity(
				Row, "seedPolicyId", ExpectedOrder, OutPolicy.Row) ||
			!Read_RuntimeId(Row, "moduleId", OutPolicy.strModuleId) ||
			!Read_ClosedString(Row, "evaluatorId",
				{ "ue3.particle-random-seed-info.current-default.v1" },
				OutPolicy.strEvaluatorId) ||
			nullptr == Seeds ||
			!Parse_I32Array(*Seeds, OutPolicy.RandomSeeds, 0u, 64u) ||
			!Read_NullableString(
				Row, "parameterName", OutPolicy.strParameterName, 1024u) ||
			OutPolicy.strParameterName.has_value() ||
			!Read_ExpectedBoolean(Row, "getSeedFromInstance", false) ||
			!Read_ExpectedBoolean(Row, "instanceSeedIsIndex", false) ||
			!Read_ExpectedBoolean(Row, "resetSeedOnEmitterLooping", true) ||
			!Read_ExpectedBoolean(Row, "randomlySelectSeedArray", false) ||
			!Read_ExpectedBoolean(Row, "emptyArrayUsesOccurrenceRandomStream", true) ||
			!Read_RuntimeId(Row, "currentCdoEvidenceKey",
				OutPolicy.strCurrentCdoEvidenceKey) ||
			!Read_ClosedString(Row, "source", {
				"SOURCE_DECODED_SEED_ARRAY", "SOURCE_OPAQUE_STRUCT_ARRAY_DECODED",
				"CURRENT_CLASS_DEFAULT_EMPTY_ARRAY" }, OutPolicy.strSource) ||
			!Read_ClosedString(Row, "sourceFidelity", {
				"MODULE_SOURCE_EVIDENCE", "CURRENT_REVISION_DEFAULT_RECONSTRUCTION" },
				OutPolicy.strSourceFidelity))
		{
			return false;
		}
		OutPolicy.bGetSeedFromInstance = false;
		OutPolicy.bInstanceSeedIsIndex = false;
		OutPolicy.bResetSeedOnEmitterLooping = true;
		OutPolicy.bRandomlySelectSeedArray = false;
		OutPolicy.bEmptyArrayUsesOccurrenceRandomStream = true;
		const bool_t CurrentDefault =
			OutPolicy.strSource == "CURRENT_CLASS_DEFAULT_EMPTY_ARRAY";
		return OutPolicy.RandomSeeds.empty() == CurrentDefault &&
			OutPolicy.strSourceFidelity == (CurrentDefault ?
				"CURRENT_REVISION_DEFAULT_RECONSTRUCTION" :
				"MODULE_SOURCE_EVIDENCE");
	}

	bool_t Parse_RibbonDefaults(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_RIBBON_DEFAULTS& OutDefaults)
	{
		return Has_ExactKeyOrder(Value, {
				"maxTessellationBetweenParticles", "sheetsPerTrail", "maxTrailCount",
				"maxParticleInTrailCount", "deadTrailsOnDeactivate",
				"deadTrailsOnSourceLoss", "clipSourceSegment",
				"enablePreviousTangentRecalculation", "renderGeometry",
				"distanceTessellationStepSize", "tangentTessellationScalar" }) &&
			Read_U32(Value, "maxTessellationBetweenParticles",
				OutDefaults.iMaxTessellationBetweenParticles) &&
			Read_U32(Value, "sheetsPerTrail", OutDefaults.iSheetsPerTrail) &&
			Read_U32(Value, "maxTrailCount", OutDefaults.iMaxTrailCount) &&
			Read_U32(Value, "maxParticleInTrailCount",
				OutDefaults.iMaxParticleInTrailCount) &&
			Read_Boolean(Value, "deadTrailsOnDeactivate",
				OutDefaults.bDeadTrailsOnDeactivate) &&
			Read_Boolean(Value, "deadTrailsOnSourceLoss",
				OutDefaults.bDeadTrailsOnSourceLoss) &&
			Read_Boolean(Value, "clipSourceSegment", OutDefaults.bClipSourceSegment) &&
			Read_Boolean(Value, "enablePreviousTangentRecalculation",
				OutDefaults.bEnablePreviousTangentRecalculation) &&
			Read_Boolean(Value, "renderGeometry", OutDefaults.bRenderGeometry) &&
			Read_FiniteF64(Value, "distanceTessellationStepSize",
				OutDefaults.fDistanceTessellationStepSize) &&
			Read_FiniteF64(Value, "tangentTessellationScalar",
				OutDefaults.fTangentTessellationScalar);
	}

	bool_t Parse_DecalDefaults(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_DECAL_DEFAULTS& OutDefaults)
	{
		const DATA_JSON_VALUE* DefaultSize = Required(
			Value, "defaultSize", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* BlendRange = Required(
			Value, "blendRange", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"defaultSize", "farPlane", "blendRange", "onlyCalculateRotationYaw",
				"supports3dDrawMode" }) &&
			nullptr != DefaultSize &&
			Parse_FixedF64Array(*DefaultSize, OutDefaults.vDefaultSize) &&
			Read_FiniteF64(Value, "farPlane", OutDefaults.fFarPlane) &&
			nullptr != BlendRange &&
			Parse_FixedF64Array(*BlendRange, OutDefaults.vBlendRange) &&
			Read_Boolean(Value, "onlyCalculateRotationYaw",
				OutDefaults.bOnlyCalculateRotationYaw) &&
			Read_Boolean(Value, "supports3dDrawMode", OutDefaults.bSupports3dDrawMode);
	}

	bool_t Parse_ImplicitDefaultRow(
		const DATA_JSON_VALUE& Row,
		const uint32_t ExpectedOrder,
		EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT& OutDefault)
	{
		const DATA_JSON_VALUE* Owned = Required(
			Row, "valuesOwnedBy", DATA_JSON_TYPE::ARRAY);
		if (!Parse_RowIdentity(Row, "defaultId", ExpectedOrder, OutDefault.Row) ||
			!Read_RuntimeId(Row, "moduleId", OutDefault.strModuleId) ||
			!Read_String(Row, "family", OutDefault.strFamily, false, 256u) ||
			!Read_String(Row, "fieldPath", OutDefault.strFieldPath, false, 1024u) ||
			!Read_ClosedEnum(Row, "variant", {
				{ "BOOL", EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::BOOLEAN },
				{ "RIBBON_DEFAULTS",
					EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::RIBBON_DEFAULTS },
				{ "DECAL_DEFAULTS",
					EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::DECAL_DEFAULTS },
				{ "SCREEN_POST_IRRELEVANT",
					EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::SCREEN_POST_IRRELEVANT },
				{ "POINT_LIGHT_OWNER_REFERENCE",
					EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::POINT_LIGHT_OWNER_REFERENCE },
			}, OutDefault.eVariant) ||
			!Read_NullableBoolean(Row, "boolValue", OutDefault.bValue) ||
			!Parse_NullableProgramObject(
				Row, "ribbonValues", Parse_RibbonDefaults, OutDefault.RibbonValues) ||
			!Parse_NullableProgramObject(
				Row, "decalValues", Parse_DecalDefaults, OutDefault.DecalValues) ||
			!Read_ClosedString(Row, "decision",
				{ "READY_FOR_HANDLER", "VERIFIED_IRRELEVANT" }, OutDefault.strDecision) ||
			!Read_String(Row, "provenance", OutDefault.strProvenance, false, 1024u) ||
			!Read_String(Row, "reason", OutDefault.strReason, true, 1024u) ||
			nullptr == Owned || !Parse_StringArray(
				*Owned, OutDefault.ValuesOwnedBy, false, 16u, 1024u))
		{
			return false;
		}
		switch (OutDefault.eVariant)
		{
		case EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::BOOLEAN:
			return OutDefault.strFamily == "RequiredLocalSpace" &&
				OutDefault.strFieldPath == "buselocalspace" &&
				OutDefault.bValue.has_value() && !*OutDefault.bValue &&
				!OutDefault.RibbonValues &&
				!OutDefault.DecalValues && OutDefault.strReason.empty() &&
				OutDefault.strDecision == "READY_FOR_HANDLER" &&
				OutDefault.strProvenance == "CURRENT_INHERITED_LANGUAGE_DEFAULT" &&
				OutDefault.ValuesOwnedBy.empty();
		case EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::RIBBON_DEFAULTS:
			return OutDefault.strFamily == "Ribbon" && !OutDefault.bValue &&
				OutDefault.strFieldPath == "typedata.ribbon.class-default-set" &&
				OutDefault.RibbonValues.has_value() && !OutDefault.DecalValues &&
				OutDefault.strDecision == "READY_FOR_HANDLER" &&
				OutDefault.strProvenance == "CURRENT_ENGINE_CDO" &&
				OutDefault.strReason.empty() && OutDefault.ValuesOwnedBy.empty();
		case EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::DECAL_DEFAULTS:
			return OutDefault.strFamily == "Decal" && !OutDefault.bValue &&
				OutDefault.strFieldPath == "typedata.decal.class-default-set" &&
				!OutDefault.RibbonValues && OutDefault.DecalValues.has_value() &&
				OutDefault.strDecision == "READY_FOR_HANDLER" &&
				OutDefault.strProvenance == "CURRENT_EFGAME_CDO" &&
				OutDefault.strReason.empty() && OutDefault.ValuesOwnedBy.empty();
		case EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::SCREEN_POST_IRRELEVANT:
			return OutDefault.strFamily == "ScreenPost" && !OutDefault.bValue &&
				OutDefault.strFieldPath == "screenpost.class-default-set" &&
				!OutDefault.RibbonValues && !OutDefault.DecalValues &&
				OutDefault.strDecision == "VERIFIED_IRRELEVANT" &&
				OutDefault.strProvenance == "NO_IMPLICIT_SOURCE_FIELD" &&
				OutDefault.strReason == "NO_IMPLICIT_SOURCE_FIELD" &&
				OutDefault.ValuesOwnedBy.empty();
		case EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::POINT_LIGHT_OWNER_REFERENCE:
			return OutDefault.strFamily == "Light" && !OutDefault.bValue &&
				OutDefault.strFieldPath == "typedata.light.source-era-default-set" &&
				!OutDefault.RibbonValues && !OutDefault.DecalValues &&
				OutDefault.strDecision == "READY_FOR_HANDLER" &&
				OutDefault.strProvenance ==
					"CURRENT_DEFAULT_CHAIN_IN_LOCAL_REFERENCE_CLOSURE" &&
				OutDefault.strReason.empty() && OutDefault.ValuesOwnedBy.size() == 8u;
		default:
			return false;
		}
	}

	bool_t Read_NullableColorRgba8(
		const DATA_JSON_VALUE& Row,
		const std::string_view Name,
		std::optional<std::array<uint32_t, 4u>>& OutValue)
	{
		const DATA_JSON_VALUE* Value = Row.Find(Name);
		if (nullptr == Value)
			return false;
		if (Value->Is_Null())
		{
			OutValue.reset();
			return true;
		}
		std::array<uint32_t, 4u> Parsed{};
		if (!Parse_FixedU32Array(*Value, Parsed) ||
			std::any_of(Parsed.begin(), Parsed.end(),
				[](const uint32_t Component) { return Component > 255u; }))
		{
			return false;
		}
		OutValue = Parsed;
		return true;
	}

	bool_t Parse_PointLightFieldRow(
		const DATA_JSON_VALUE& Row,
		const uint32_t ExpectedOrder,
		EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD& OutField)
	{
		if (!Parse_RowIdentity(Row, "fieldId", ExpectedOrder, OutField.Row) ||
			!Read_RuntimeId(Row, "moduleId", OutField.strModuleId) ||
			!Read_String(Row, "fieldPath", OutField.strFieldPath, false, 1024u) ||
			!Read_ClosedEnum(Row, "valueVariant", {
				{ "BOOL", EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::BOOLEAN },
				{ "F64", EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::F64 },
				{ "COLOR_RGBA8",
					EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::COLOR_RGBA8 },
				{ "GUID128", EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::GUID128 },
			}, OutField.eVariant) ||
			!Read_NullableBoolean(Row, "boolValue", OutField.bValue) ||
			!Read_NullableF64(Row, "f64Value", OutField.fValue) ||
			!Read_NullableColorRgba8(
				Row, "colorRgba8Value", OutField.ColorRgba8Value) ||
			!Read_String(Row, "guid128Value", OutField.strGuid128Value, true, 32u) ||
			!Read_String(Row, "sourceTier", OutField.strSourceTier, false, 256u) ||
			!Read_String(Row, "sourceFidelity", OutField.strSourceFidelity,
				false, 256u) ||
			!Read_ClosedString(Row, "decision",
				{ "READY_FOR_HANDLER", "VERIFIED_IRRELEVANT" }, OutField.strDecision) ||
			!Read_String(Row, "oracleId", OutField.strOracleId, true, 1024u))
		{
			return false;
		}
		if (!OutField.strGuid128Value.empty() &&
			(OutField.strGuid128Value.size() != 32u ||
				!std::all_of(OutField.strGuid128Value.begin(),
					OutField.strGuid128Value.end(), [](const char Character)
					{
						return (Character >= '0' && Character <= '9') ||
							(Character >= 'a' && Character <= 'f');
					})))
		{
			return false;
		}
		const uint32_t Populated = static_cast<uint32_t>(OutField.bValue.has_value()) +
			static_cast<uint32_t>(OutField.fValue.has_value()) +
			static_cast<uint32_t>(OutField.ColorRgba8Value.has_value()) +
			static_cast<uint32_t>(!OutField.strGuid128Value.empty());
		if (Populated != 1u)
			return false;
		const bool_t SourcePairValid =
			(OutField.strSourceTier == "INSTANCE_EXPLICIT" &&
				OutField.strSourceFidelity == "SOURCE_EXACT_PHYSICAL_PACKAGE") ||
			(OutField.strSourceTier == "NESTED_ARCHETYPE_TEMPLATE" &&
				OutField.strSourceFidelity ==
					"CURRENT_REVISION_ARCHETYPE_EVIDENCE") ||
			((OutField.strSourceTier == "CLASS_CDO" ||
				OutField.strSourceTier == "PARENT_CDO_HIERARCHY") &&
				OutField.strSourceFidelity == "CURRENT_REVISION_SCRIPT_EVIDENCE");
		if (!SourcePairValid)
			return false;
		const bool_t Guid = OutField.eVariant ==
			EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::GUID128;
		if ((Guid && (OutField.strDecision != "VERIFIED_IRRELEVANT" ||
				OutField.strOracleId != "ue3.runtime-light-guid-irrelevance.v1")) ||
			(!Guid && (OutField.strDecision != "READY_FOR_HANDLER" ||
				!OutField.strOracleId.empty())))
		{
			return false;
		}
		switch (OutField.eVariant)
		{
		case EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::BOOLEAN:
			return OutField.bValue.has_value();
		case EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::F64:
			return OutField.fValue.has_value();
		case EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::COLOR_RGBA8:
			return OutField.ColorRgba8Value.has_value();
		case EFFECT_RUNTIME_POINT_LIGHT_VALUE_VARIANT::GUID128:
			return !OutField.strGuid128Value.empty();
		default:
			return false;
		}
	}

	bool_t Parse_DistributionRow(
		const DATA_JSON_VALUE& Row,
		const uint32_t ExpectedOrder,
		EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& OutDistribution)
	{
		const DATA_JSON_VALUE* DefaultMinimum = Required(
			Row, "defaultMinimum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* DefaultMaximum = Required(
			Row, "defaultMaximum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* LookupTable = Required(
			Row, "lookupTable", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* CurveKeys = Required(
			Row, "curveKeys", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ParamModes = Required(
			Row, "paramModes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* MinimumInput = Required(
			Row, "minimumInput", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* MaximumInput = Required(
			Row, "maximumInput", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* MinimumOutput = Required(
			Row, "minimumOutput", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* MaximumOutput = Required(
			Row, "maximumOutput", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ConstantValues = Required(
			Row, "constantValues", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ActionCues = Required(
			Row, "actionCueBindings", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Provenance = Required(
			Row, "fieldProvenance", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Samples = Required(
			Row, "samples", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Blockers = Required(
			Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
		if (!Parse_RowIdentity(
				Row, "distributionId", ExpectedOrder, OutDistribution.Row) ||
			!Read_RuntimeId(Row, "moduleId", OutDistribution.strModuleId) ||
			!Read_RuntimeId(Row, "propertyId", OutDistribution.strPropertyId) ||
			!Parse_DistributionVariant(Row, OutDistribution.eVariant) ||
			!Read_RuntimeId(Row, "evaluatorRegistryId",
				OutDistribution.strEvaluatorRegistryId) ||
			!Read_RuntimeId(Row, "payloadDistributionId",
				OutDistribution.strPayloadDistributionId) ||
			!Read_String(Row, "propertyPath", OutDistribution.strPropertyPath,
				false, 4096u) ||
			!Read_String(Row, "sourceClass", OutDistribution.strSourceClass,
				true, 256u) ||
			!Read_String(Row, "sourceObjectPath", OutDistribution.strSourceObjectPath,
				true, 4096u) ||
			!Read_String(Row, "referenceId", OutDistribution.strReferenceId,
				true, 1024u) ||
			!Read_String(Row, "occurrenceId", OutDistribution.strOccurrenceId,
				true, 1024u) ||
			!Read_ClosedString(Row, "payloadStatus", {
				"INLINE_SOURCE_PAYLOAD", "DECODED_EXTERNAL_RECORD_WITHOUT_PACKAGE_IDENTITY",
				"UNRESOLVED_SEMANTIC_CLOSURE", "CURRENT_ENGINE_CDO_RECONSTRUCTED" },
				OutDistribution.strPayloadStatus) ||
			!Read_ClosedString(Row, "fidelity", {
				"DETERMINISTIC_SOURCE_RECIPE",
				"CURRENT_ENGINE_CDO_RECONSTRUCTED",
				"RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED",
				"SOURCE_EXACT_PHYSICAL_PACKAGE",
				"PINNED_SOURCE_RECORD_PHYSICAL_ABSENT",
				"UNRESOLVED_CROSS_REVISION" }, OutDistribution.strFidelity) ||
			!Read_U32(Row, "componentCount", OutDistribution.iComponentCount) ||
			OutDistribution.iComponentCount == 0u ||
			OutDistribution.iComponentCount > 4u ||
			!Read_NullableU32(Row, "operation", OutDistribution.iOperation) ||
			!Read_NullableU32(Row, "randomLockAxes", OutDistribution.iRandomLockAxes) ||
			!Read_NullableU32(
				Row, "lookupTableChunkSize", OutDistribution.iLookupTableChunkSize) ||
			!Read_NullableU32(
				Row, "lookupTableNumElements", OutDistribution.iLookupTableNumElements) ||
			!Read_NullableF64(
				Row, "lookupTableTimeScale", OutDistribution.fLookupTableTimeScale) ||
			!Read_NullableF64(
				Row, "lookupTableStartTime", OutDistribution.fLookupTableStartTime) ||
			nullptr == DefaultMinimum || !Parse_FiniteF64Array(
				*DefaultMinimum, OutDistribution.DefaultMinimum, 0u, 4u) ||
			nullptr == DefaultMaximum || !Parse_FiniteF64Array(
				*DefaultMaximum, OutDistribution.DefaultMaximum, 0u, 4u) ||
			nullptr == LookupTable || !Parse_FiniteF64Array(
				*LookupTable, OutDistribution.LookupTable, 0u, 4096u) ||
			nullptr == CurveKeys || CurveKeys->Get_Array().size() > 64u ||
			!Read_String(Row, "parameterName", OutDistribution.strParameterName,
				true, 1024u) ||
			nullptr == ParamModes || !Parse_StringArray(
				*ParamModes, OutDistribution.ParamModes, false, 4u, 256u) ||
			nullptr == MinimumInput || !Parse_FiniteF64Array(
				*MinimumInput, OutDistribution.MinimumInput, 0u, 4u) ||
			nullptr == MaximumInput || !Parse_FiniteF64Array(
				*MaximumInput, OutDistribution.MaximumInput, 0u, 4u) ||
			nullptr == MinimumOutput || !Parse_FiniteF64Array(
				*MinimumOutput, OutDistribution.MinimumOutput, 0u, 4u) ||
			nullptr == MaximumOutput || !Parse_FiniteF64Array(
				*MaximumOutput, OutDistribution.MaximumOutput, 0u, 4u) ||
			nullptr == ConstantValues || !Parse_FiniteF64Array(
				*ConstantValues, OutDistribution.ConstantValues, 0u, 4u) ||
			!Read_NullableBoolean(Row, "isDirty", OutDistribution.bIsDirty) ||
			nullptr == ActionCues ||
			!Parse_ProgramActionCueValues(*ActionCues, OutDistribution.ActionCueBindings) ||
			nullptr == Provenance || Provenance->Get_Array().size() > 64u ||
			nullptr == Samples || Samples->Get_Array().size() > 4u ||
			!Read_OptionalRuntimeId(Row, "capabilityImplementationId",
				OutDistribution.strCapabilityImplementationId) ||
			!Read_U32(Row, "capabilityImplementationVersion",
				OutDistribution.iCapabilityImplementationVersion) ||
			!Read_OptionalSha(Row, "capabilityImplementationSha256",
				OutDistribution.strCapabilityImplementationSha256) ||
			nullptr == Blockers ||
			!Parse_Blockers(*Blockers, OutDistribution.PreservedBlockers))
		{
			return false;
		}

		OutDistribution.CurveKeys.clear();
		OutDistribution.CurveKeys.reserve(CurveKeys->Get_Array().size());
		for (const DATA_JSON_VALUE& Item : CurveKeys->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_CURVE_KEY Parsed;
			if (!Parse_DistributionCurveKey(Item, Parsed))
				return false;
			OutDistribution.CurveKeys.push_back(std::move(Parsed));
		}
		OutDistribution.FieldProvenance.clear();
		OutDistribution.FieldProvenance.reserve(Provenance->Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Provenance->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_FIELD_PROVENANCE Parsed;
			if (!Parse_DistributionFieldProvenance(Item, Parsed))
				return false;
			OutDistribution.FieldProvenance.push_back(std::move(Parsed));
		}
		OutDistribution.Samples.clear();
		OutDistribution.Samples.reserve(Samples->Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Samples->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_DISTRIBUTION_SAMPLE Parsed;
			if (!Parse_DistributionSample(Item, Parsed))
				return false;
			OutDistribution.Samples.push_back(std::move(Parsed));
		}
		if (std::any_of(OutDistribution.ParamModes.begin(),
			OutDistribution.ParamModes.end(), [](const std::string& Mode)
			{
				return Mode != "dpm_direct" && Mode != "dpm_normal";
			}))
		{
			return false;
		}

		const bool_t Inline = OutDistribution.eVariant ==
			EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE;
		const bool_t FloatCurve = OutDistribution.eVariant ==
			EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE;
		const bool_t TableVariant =
			Inline || FloatCurve;
		const bool_t EfMultiply =
			OutDistribution.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
		if (TableVariant)
		{
			const bool_t CurrentEngineCdoInline = Inline &&
				OutDistribution.strPayloadStatus ==
					"CURRENT_ENGINE_CDO_RECONSTRUCTED" &&
				OutDistribution.strFidelity ==
					"CURRENT_ENGINE_CDO_RECONSTRUCTED";
			const std::vector<std::string> BlockedDefaultTokens{
				"DEFAULT_DEPENDENT_DISTRIBUTION_REQUIRES_TYPED_DEFAULT_POLICY",
				"DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED",
				"DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
				"DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
				"INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
			};
			const bool_t BlockedDefaultInline = Inline &&
				OutDistribution.strPayloadStatus ==
					"UNRESOLVED_SEMANTIC_CLOSURE" &&
				OutDistribution.strFidelity == "UNRESOLVED_CROSS_REVISION" &&
				OutDistribution.PreservedBlockers == BlockedDefaultTokens;
			if ((Inline &&
					(!OutDistribution.strSourceClass.empty() ||
					 (!CurrentEngineCdoInline && !BlockedDefaultInline &&
					  !OutDistribution.strSourceObjectPath.empty()) ||
					 !OutDistribution.strReferenceId.empty() ||
					 !OutDistribution.strOccurrenceId.empty() ||
					 (!CurrentEngineCdoInline && !BlockedDefaultInline &&
					  (OutDistribution.strPayloadStatus != "INLINE_SOURCE_PAYLOAD" ||
					   OutDistribution.strFidelity !=
						   "DETERMINISTIC_SOURCE_RECIPE")))) ||
				(FloatCurve &&
					(OutDistribution.strSourceClass != "distributionfloatconstantcurve" ||
					 OutDistribution.strSourceObjectPath.empty() ||
					 OutDistribution.strReferenceId.empty() ||
					 OutDistribution.strOccurrenceId.empty() ||
					 OutDistribution.strPayloadStatus !=
						"DECODED_EXTERNAL_RECORD_WITHOUT_PACKAGE_IDENTITY" ||
					 OutDistribution.strFidelity !=
						"RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED")) ||
				!OutDistribution.iOperation || !OutDistribution.iRandomLockAxes ||
				!OutDistribution.iLookupTableChunkSize ||
				!OutDistribution.iLookupTableNumElements ||
				!OutDistribution.fLookupTableTimeScale ||
				!OutDistribution.fLookupTableStartTime ||
				OutDistribution.DefaultMinimum.size() != 4u ||
				OutDistribution.DefaultMaximum.size() != 4u ||
				!OutDistribution.strParameterName.empty() ||
				!OutDistribution.ParamModes.empty() ||
				!OutDistribution.MinimumInput.empty() ||
				!OutDistribution.MaximumInput.empty() ||
				!OutDistribution.MinimumOutput.empty() ||
				!OutDistribution.MaximumOutput.empty() ||
				!OutDistribution.ConstantValues.empty() || OutDistribution.bIsDirty ||
				!OutDistribution.ActionCueBindings.empty() ||
				!OutDistribution.strCapabilityImplementationId.empty() ||
				OutDistribution.iCapabilityImplementationVersion != 0u ||
				!OutDistribution.strCapabilityImplementationSha256.empty() ||
				(!BlockedDefaultInline &&
				 !OutDistribution.PreservedBlockers.empty()) ||
				OutDistribution.Samples.size() !=
					(BlockedDefaultInline ? 0u : 3u))
			{
				return false;
			}
			if (CurrentEngineCdoInline &&
				(OutDistribution.strPropertyPath != "ratescale" ||
				 OutDistribution.strSourceObjectPath !=
					 "Default__ParticleModuleSpawn" ||
				 OutDistribution.iComponentCount != 1u ||
				 *OutDistribution.iOperation != 1u ||
				 *OutDistribution.iRandomLockAxes != 0u ||
				 *OutDistribution.iLookupTableChunkSize != 1u ||
				 *OutDistribution.iLookupTableNumElements != 1u ||
				 *OutDistribution.fLookupTableTimeScale != 0.0 ||
				 *OutDistribution.fLookupTableStartTime != 0.0 ||
				 OutDistribution.LookupTable !=
					 std::vector<double>{ 1.0, 1.0, 1.0, 1.0 }))
			{
				return false;
			}
			if (BlockedDefaultInline &&
				(!OutDistribution.strSourceObjectPath.empty() ||
				 !OutDistribution.LookupTable.empty() ||
				 !OutDistribution.CurveKeys.empty() ||
				 !OutDistribution.FieldProvenance.empty()))
			{
				return false;
			}
			return Inline ? OutDistribution.CurveKeys.empty() :
				OutDistribution.CurveKeys.size() == 3u;
		}

		if (OutDistribution.iOperation || OutDistribution.iRandomLockAxes ||
			OutDistribution.iLookupTableChunkSize ||
			OutDistribution.iLookupTableNumElements ||
			OutDistribution.fLookupTableTimeScale ||
			OutDistribution.fLookupTableStartTime ||
			!OutDistribution.LookupTable.empty() || !OutDistribution.CurveKeys.empty() ||
			OutDistribution.strParameterName.empty() || !OutDistribution.bIsDirty ||
			OutDistribution.ParamModes.size() != OutDistribution.iComponentCount ||
			OutDistribution.MinimumInput.size() != OutDistribution.iComponentCount ||
			OutDistribution.MaximumInput.size() != OutDistribution.iComponentCount ||
			OutDistribution.MinimumOutput.size() != OutDistribution.iComponentCount ||
			OutDistribution.MaximumOutput.size() != OutDistribution.iComponentCount ||
			OutDistribution.ConstantValues.size() != OutDistribution.iComponentCount)
		{
			return false;
		}
		if (EfMultiply)
		{
			return OutDistribution.strSourceClass ==
					"efdistributionvectormultiplyparticleparameter" &&
				OutDistribution.strSourceObjectPath.empty() &&
				!OutDistribution.strReferenceId.empty() &&
				!OutDistribution.strOccurrenceId.empty() &&
				OutDistribution.strPayloadStatus == "UNRESOLVED_SEMANTIC_CLOSURE" &&
				(OutDistribution.strFidelity == "UNRESOLVED_CROSS_REVISION" ||
					OutDistribution.strFidelity ==
						"RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED") &&
				OutDistribution.iComponentCount == 3u &&
				OutDistribution.DefaultMinimum.size() == 4u &&
				OutDistribution.DefaultMaximum.size() == 4u &&
				OutDistribution.Samples.empty() &&
				OutDistribution.strCapabilityImplementationId ==
					"source.reconstructed.ef-vector-multiply.v1.implementation" &&
				OutDistribution.iCapabilityImplementationVersion == 1u &&
				!OutDistribution.strCapabilityImplementationSha256.empty() &&
				OutDistribution.PreservedBlockers == std::vector<std::string>{
					"CUSTOM_EF_DISTRIBUTION_EVALUATOR_UNPROVEN" };
		}
		if (!OutDistribution.DefaultMinimum.empty() ||
			!OutDistribution.DefaultMaximum.empty() ||
			!OutDistribution.strCapabilityImplementationId.empty() ||
			OutDistribution.iCapabilityImplementationVersion != 0u ||
			!OutDistribution.strCapabilityImplementationSha256.empty() ||
			!OutDistribution.PreservedBlockers.empty() ||
			OutDistribution.Samples.size() != 1u)
		{
			return false;
		}
		const bool_t FloatParameter = OutDistribution.eVariant ==
			EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER;
		const bool_t ParameterIdentity = FloatParameter ?
			OutDistribution.strSourceClass == "distributionfloatparticleparameter" :
			OutDistribution.strSourceClass == "distributionvectorparticleparameter";
		const bool_t ParameterFidelity =
			OutDistribution.strFidelity == "RECORD_DECODED_PACKAGE_IDENTITY_UNPINNED" ||
			OutDistribution.strFidelity == "SOURCE_EXACT_PHYSICAL_PACKAGE" ||
			OutDistribution.strFidelity == "PINNED_SOURCE_RECORD_PHYSICAL_ABSENT";
		return ParameterIdentity && !OutDistribution.strSourceObjectPath.empty() &&
			!OutDistribution.strReferenceId.empty() &&
			!OutDistribution.strOccurrenceId.empty() &&
			OutDistribution.strPayloadStatus == "INLINE_SOURCE_PAYLOAD" &&
			ParameterFidelity &&
			OutDistribution.iComponentCount == (FloatParameter ? 1u : 3u) &&
			OutDistribution.Samples[0].OutputValues.size() ==
				(FloatParameter ? 1u : 3u);
	}

	bool_t Parse_ReconstructedValueRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram,
		std::string& OutFailure)
	{
		if (!Parse_PropertyRows(Root, OutProgram.Properties))
		{
			OutFailure = "properties";
			return false;
		}
		if (!Parse_PrimitiveLeafRows(Root, OutProgram.PrimitiveLeaves))
		{
			OutFailure = "primitiveLeaves";
			return false;
		}
		const DATA_JSON_VALUE* Literals = Required(
			Root, "literals", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Distributions = Required(
			Root, "distributions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* SeedPolicies = Required(
			Root, "seedPolicies", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ImplicitDefaults = Required(
			Root, "implicitDefaults", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* PointLightFields = Required(
			Root, "pointLightFields", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Literals || nullptr == Distributions ||
			nullptr == SeedPolicies || nullptr == ImplicitDefaults ||
			nullptr == PointLightFields)
			return false;

		OutProgram.SeedPolicies.clear();
		OutProgram.SeedPolicies.reserve(SeedPolicies->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : SeedPolicies->Get_Array())
		{
			if (OutProgram.SeedPolicies.size() >
				(std::numeric_limits<uint32_t>::max)())
			{
				OutFailure = "seedPolicies-count";
				return false;
			}
			EFFECT_RUNTIME_PROGRAM_SEED_POLICY Parsed;
			if (!Parse_SeedPolicyRow(Row,
					static_cast<uint32_t>(OutProgram.SeedPolicies.size()), Parsed))
			{
				OutFailure = "seedPolicies:" +
					std::to_string(OutProgram.SeedPolicies.size());
				return false;
			}
			OutProgram.SeedPolicies.push_back(std::move(Parsed));
		}

		OutProgram.ImplicitDefaults.clear();
		OutProgram.ImplicitDefaults.reserve(ImplicitDefaults->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : ImplicitDefaults->Get_Array())
		{
			if (OutProgram.ImplicitDefaults.size() >
				(std::numeric_limits<uint32_t>::max)())
			{
				OutFailure = "implicitDefaults-count";
				return false;
			}
			EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT Parsed;
			if (!Parse_ImplicitDefaultRow(Row,
					static_cast<uint32_t>(OutProgram.ImplicitDefaults.size()), Parsed))
			{
				OutFailure = "implicitDefaults:" +
					std::to_string(OutProgram.ImplicitDefaults.size());
				return false;
			}
			OutProgram.ImplicitDefaults.push_back(std::move(Parsed));
		}

		OutProgram.PointLightFields.clear();
		OutProgram.PointLightFields.reserve(PointLightFields->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : PointLightFields->Get_Array())
		{
			if (OutProgram.PointLightFields.size() >
				(std::numeric_limits<uint32_t>::max)())
			{
				OutFailure = "pointLightFields-count";
				return false;
			}
			EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD Parsed;
			if (!Parse_PointLightFieldRow(Row,
					static_cast<uint32_t>(OutProgram.PointLightFields.size()), Parsed))
			{
				OutFailure = "pointLightFields:" +
					std::to_string(OutProgram.PointLightFields.size());
				return false;
			}
			OutProgram.PointLightFields.push_back(std::move(Parsed));
		}

		OutProgram.Literals.clear();
		OutProgram.Literals.reserve(Literals->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Literals->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_LITERAL Parsed;
			std::string EnumValue;
			if (!Parse_RowIdentity(Row, "literalId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "moduleId", Parsed.strModuleId) ||
				!Read_RuntimeId(Row, "propertyId", Parsed.strPropertyId) ||
				!Read_String(Row, "propertyPath", Parsed.strPropertyPath, false, 4096u) ||
				!Parse_LiteralVariant(Row, "variant", Parsed.eVariant) ||
				!Read_NullableBoolean(Row, "boolValue", Parsed.bValue) ||
				!Read_NullableF64(Row, "f64Value", Parsed.fValue) ||
				!Read_String(Row, "enumStringValue", EnumValue, true, 4096u))
			{
				OutFailure = "literals:" + std::to_string(OutProgram.Literals.size());
				return false;
			}
			Parsed.strEnumValue = std::move(EnumValue);
			const uint32_t Populated = static_cast<uint32_t>(Parsed.bValue.has_value()) +
				static_cast<uint32_t>(Parsed.fValue.has_value()) +
				static_cast<uint32_t>(!Parsed.strEnumValue.empty());
			if (Populated != 1u ||
				(Parsed.eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::BOOLEAN &&
					!Parsed.bValue.has_value()) ||
				(Parsed.eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::F64 &&
					!Parsed.fValue.has_value()) ||
				(Parsed.eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::ENUM_STRING &&
					Parsed.strEnumValue.empty()))
			{
				OutFailure = "literals-oneof:" +
					std::to_string(OutProgram.Literals.size());
				return false;
			}
			OutProgram.Literals.push_back(std::move(Parsed));
		}

		OutProgram.Distributions.clear();
		OutProgram.Distributions.reserve(Distributions->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Distributions->Get_Array())
		{
			if (OutProgram.Distributions.size() >
				(std::numeric_limits<uint32_t>::max)())
			{
				OutFailure = "distributions-count";
				return false;
			}
			EFFECT_RUNTIME_PROGRAM_DISTRIBUTION Parsed;
			if (!Parse_DistributionRow(Row,
					(std::numeric_limits<uint32_t>::max)(), Parsed))
			{
				OutFailure = "distributions:" +
					std::to_string(OutProgram.Distributions.size());
				return false;
			}
			OutProgram.Distributions.push_back(std::move(Parsed));
		}
		return true;
	}

	bool_t Parse_MaterialInputRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "materialInputs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> Staged;
		Staged.reserve(Section->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			const DATA_JSON_VALUE* VectorValue = Required(
				Row, "valueF64x4", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE Parsed;
			std::optional<double> F64;
			std::vector<double> Vector;
			std::string TextureId;
			if (!Parse_RowIdentity(Row, "fieldId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				!Read_String(Row, "fieldKind", Parsed.strFieldKind, false, 256u) ||
				!Read_String(Row, "bindingRole", Parsed.strBindingRole, false, 256u) ||
				!Read_String(Row, "bindingOrigin", Parsed.strBindingOrigin,
					false, 256u) ||
				!Read_String(Row, "sourceSection", Parsed.strSourceSection,
					false, 256u) ||
				!Read_U32(Row, "sourceSectionIndex", Parsed.iSourceSectionIndex) ||
				!Read_String(Row, "parameterName", Parsed.strParameterName,
					true, 1024u) ||
				!Read_String(Row, "normalizedParameterName",
					Parsed.strNormalizedParameterName, true, 1024u) ||
				!Parse_MaterialValueVariant(
					Row, "valueVariant", Parsed.eVariant, false) ||
				!Read_NullableF64(Row, "valueF64", F64) ||
				nullptr == VectorValue ||
				!Parse_FiniteF64Array(*VectorValue, Vector, 0u, 4u) ||
				!Read_String(Row, "valueTextureId", TextureId, true, 4096u) ||
				!Read_Sha(Row, "typedValueSha256", Parsed.strTypedValueSha256) ||
				!Read_Sha(Row, "sourceFieldValueSha256",
					Parsed.strSourceFieldValueSha256) ||
				!Read_Sha(Row, "sourceLineageSha256", Parsed.strSourceLineageSha256))
			{
				return false;
			}
			if (Parsed.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64)
			{
				if (!F64.has_value() || !Vector.empty() || !TextureId.empty())
					return false;
				Parsed.fValue = F64;
			}
			else if (Parsed.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64X4)
			{
				if (F64.has_value() || Vector.size() != Parsed.vValue.size() ||
					!TextureId.empty())
				{
					return false;
				}
				std::copy(Vector.begin(), Vector.end(), Parsed.vValue.begin());
			}
			else if (Parsed.eVariant ==
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID)
			{
				if (F64.has_value() || !Vector.empty() || TextureId.empty())
					return false;
				Parsed.strStringValue = std::move(TextureId);
			}
			else
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutProgram.MaterialInputs = std::move(Staged);
		return true;
	}

	bool_t Parse_MaterialStaticRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "materialStaticBindings", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> Staged;
		Staged.reserve(Section->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE Parsed;
			bool_t SourceValue = false;
			bool_t SelectedValue = false;
			if (!Parse_RowIdentity(Row, "fieldId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				!Read_String(Row, "parameterName", Parsed.strParameterName,
					true, 1024u) ||
				!Read_String(Row, "normalizedParameterName",
					Parsed.strNormalizedParameterName, true, 1024u) ||
				!Read_String(Row, "bindingOrigin", Parsed.strBindingOrigin,
					false, 256u) ||
				!Read_String(Row, "selectionRole", Parsed.strSelectionRole,
					false, 256u) ||
				!Read_Boolean(Row, "sourceValue", SourceValue) ||
				!Read_Boolean(Row, "selectedValue", SelectedValue) ||
				!Read_RuntimeId(Row, "policyRowId", Parsed.strPolicyRowId) ||
				!Read_Sha(Row, "sourceFieldValueSha256",
					Parsed.strSourceFieldValueSha256) ||
				!Read_Sha(Row, "sourceLineageSha256", Parsed.strSourceLineageSha256))
			{
				return false;
			}
			Parsed.eVariant = EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN;
			Parsed.bValue = SelectedValue;
			Parsed.bSourceValue = SourceValue;
			Parsed.bSelectedValue = SelectedValue;
			Staged.push_back(std::move(Parsed));
		}
		OutProgram.MaterialStaticBindings = std::move(Staged);
		return true;
	}

	bool_t Parse_MaterialRenderRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "materialRenderBindings", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE> Staged;
		Staged.reserve(Section->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Section->Get_Array())
		{
			EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE Parsed;
			std::optional<bool_t> Boolean;
			std::optional<double> F64;
			std::string EnumValue;
			if (!Parse_RowIdentity(Row, "renderBindingId",
					(std::numeric_limits<uint32_t>::max)(), Parsed.Row) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				!Read_String(Row, "fieldName", Parsed.strFieldName, false, 256u) ||
				!Read_String(Row, "bindingOrigin", Parsed.strBindingOrigin,
					false, 256u) ||
				!Read_String(Row, "sourceStatus", Parsed.strSourceStatus,
					false, 256u) ||
				!Read_String(Row, "sourceFidelity", Parsed.strSourceFidelity,
					false, 256u) ||
				!Parse_MaterialValueVariant(
					Row, "valueVariant", Parsed.eVariant, false) ||
				!Read_NullableBoolean(Row, "boolValue", Boolean) ||
				!Read_NullableF64(Row, "f64Value", F64) ||
				!Read_String(Row, "enumValue", EnumValue, true, 4096u) ||
				!Read_OptionalSha(Row, "sourceRecordSha256",
					Parsed.strSourceRecordSha256) ||
				!Read_OptionalRuntimeId(Row, "policyRowId", Parsed.strPolicyRowId) ||
				!Read_OptionalRuntimeId(Row, "sourceBlocker", Parsed.strSourceBlocker))
			{
				return false;
			}
			const uint32_t Populated = static_cast<uint32_t>(Boolean.has_value()) +
				static_cast<uint32_t>(F64.has_value()) +
				static_cast<uint32_t>(!EnumValue.empty());
			if (Populated != 1u)
				return false;
			if (Parsed.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN &&
				Boolean.has_value())
			{
				Parsed.bValue = Boolean;
			}
			else if (Parsed.eVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
				F64.has_value())
			{
				Parsed.fValue = F64;
			}
			else if (Parsed.eVariant ==
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::ENUM_STRING && !EnumValue.empty())
			{
				Parsed.strStringValue = std::move(EnumValue);
			}
			else
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutProgram.MaterialRenderBindings = std::move(Staged);
		return true;
	}

	bool_t Parse_MaterialFamilySamples(
		const DATA_JSON_VALUE& Value,
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY_SAMPLE>& OutSamples)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() != 4u)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY_SAMPLE> Staged;
		Staged.reserve(4u);
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Sample = Value.Get_Array()[Index];
			const DATA_JSON_VALUE* Expected = Required(
				Sample, "expectedFloat4", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY_SAMPLE Parsed;
			if (!Has_ExactKeyOrder(Sample, {
					"sampleId", "order", "inputSha256", "expectedFloat4" }) ||
				!Read_RuntimeId(Sample, "sampleId", Parsed.strSampleId) ||
				!Read_U32(Sample, "order", Parsed.iOrder) || Parsed.iOrder != Index ||
				!Read_Sha(Sample, "inputSha256", Parsed.strInputSha256) ||
				nullptr == Expected || !Parse_FixedF64Array(*Expected, Parsed.vExpected))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutSamples = std::move(Staged);
		return true;
	}

	bool_t Parse_MaterialRecipeSamples(
		const DATA_JSON_VALUE& Value,
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE_SAMPLE>& OutSamples)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() != 4u)
			return false;
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE_SAMPLE> Staged;
		Staged.reserve(4u);
		for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Sample = Value.Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE_SAMPLE Parsed;
			const DATA_JSON_VALUE* UvScale = Required(
				Sample, "uvScale", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Pan = Required(
				Sample, "panRotationAux", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Texture0 = Required(
				Sample, "texture0", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Texture1 = Required(
				Sample, "texture1", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Color = Required(
				Sample, "color", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Params0 = Required(
				Sample, "params0", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Params1 = Required(
				Sample, "params1", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Expected = Required(
				Sample, "expectedFloat4", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactKeyOrder(Sample, {
					"sampleId", "order", "time", "uvScale", "panRotationAux",
					"texture0", "texture1", "color", "params0", "params1",
					"inputSha256", "expectedFloat4" }) ||
				!Read_RuntimeId(Sample, "sampleId", Parsed.strSampleId) ||
				!Read_U32(Sample, "order", Parsed.iOrder) || Parsed.iOrder != Index ||
				!Read_FiniteF64(Sample, "time", Parsed.fTime) ||
				nullptr == UvScale || nullptr == Pan || nullptr == Texture0 ||
				nullptr == Texture1 || nullptr == Color || nullptr == Params0 ||
				nullptr == Params1 || nullptr == Expected ||
				!Parse_FixedF64Array(*UvScale, Parsed.vUvScale) ||
				!Parse_FixedF64Array(*Pan, Parsed.vPanRotationAux) ||
				!Parse_FixedF64Array(*Texture0, Parsed.vTexture0) ||
				!Parse_FixedF64Array(*Texture1, Parsed.vTexture1) ||
				!Parse_FixedF64Array(*Color, Parsed.vColor) ||
				!Parse_FixedF64Array(*Params0, Parsed.vParams0) ||
				!Parse_FixedF64Array(*Params1, Parsed.vParams1) ||
				!Read_Sha(Sample, "inputSha256", Parsed.strInputSha256) ||
				!Parse_FixedF64Array(*Expected, Parsed.vExpected))
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		OutSamples = std::move(Staged);
		return true;
	}

	bool_t Parse_ApprovalArithmeticReceipt(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_APPROVAL_ARITHMETIC_RECEIPT& OutReceipt)
	{
		const DATA_JSON_VALUE* Preserved = Required(
			Value, "preservedEvidenceBlockers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Oracles = Required(
			Value, "requiredOracleIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Execution = Required(
			Value, "executionBlockers", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"policyRowId", "upstreamFamilyId", "familyIdentitySha256",
				"evaluatorId", "evaluatorVersion", "evaluatorSha256",
				"policyFamilyId", "evidenceFidelity", "sourceExact",
				"cpuNumericOracleVerified", "hlslNumericOracleVerified",
				"preservedEvidenceBlockers", "requiredOracleIds",
				"executionBlockers", "executionAdmission", "productAdmission" }) &&
			Read_RuntimeId(Value, "policyRowId", OutReceipt.strPolicyRowId) &&
			Read_RuntimeId(Value, "upstreamFamilyId", OutReceipt.strUpstreamFamilyId) &&
			Read_Sha(Value, "familyIdentitySha256",
				OutReceipt.strFamilyIdentitySha256) &&
			Read_RuntimeId(Value, "evaluatorId", OutReceipt.strEvaluatorId) &&
			Read_U32(Value, "evaluatorVersion", OutReceipt.iEvaluatorVersion) &&
			OutReceipt.iEvaluatorVersion != 0u &&
			Read_Sha(Value, "evaluatorSha256", OutReceipt.strEvaluatorSha256) &&
			Read_RuntimeId(Value, "policyFamilyId", OutReceipt.strPolicyFamilyId) &&
			Read_String(Value, "evidenceFidelity", OutReceipt.strEvidenceFidelity,
				false, 256u) &&
			Read_Boolean(Value, "sourceExact", OutReceipt.bSourceExact) &&
			!OutReceipt.bSourceExact &&
			Read_Boolean(Value, "cpuNumericOracleVerified",
				OutReceipt.bCpuNumericOracleVerified) &&
			OutReceipt.bCpuNumericOracleVerified &&
			Read_Boolean(Value, "hlslNumericOracleVerified",
				OutReceipt.bHlslNumericOracleVerified) &&
			OutReceipt.bHlslNumericOracleVerified && nullptr != Preserved &&
			Parse_Blockers(*Preserved, OutReceipt.PreservedEvidenceBlockers) &&
			nullptr != Oracles && Parse_StringArray(
				*Oracles, OutReceipt.RequiredOracleIds, false, 16u, 256u) &&
			nullptr != Execution &&
			Parse_Blockers(*Execution, OutReceipt.ExecutionBlockers) &&
			Read_Boolean(Value, "executionAdmission",
				OutReceipt.bExecutionAdmission) && !OutReceipt.bExecutionAdmission &&
			Read_Boolean(Value, "productAdmission", OutReceipt.bProductAdmission) &&
			!OutReceipt.bProductAdmission;
	}

	bool_t Parse_ReconstructedMaterialRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram,
		std::string& OutFailure)
	{
		const DATA_JSON_VALUE* Families = Required(
			Root, "materialFamilies", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Recipes = Required(
			Root, "materialRecipes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Occurrences = Required(
			Root, "materialOccurrences", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Families || nullptr == Recipes || nullptr == Occurrences)
		{
			OutFailure = "sections";
			return false;
		}
		if (!Parse_MaterialInputRows(Root, OutProgram))
		{
			OutFailure = "materialInputs";
			return false;
		}
		if (!Parse_MaterialStaticRows(Root, OutProgram))
		{
			OutFailure = "materialStaticBindings";
			return false;
		}
		if (!Parse_MaterialRenderRows(Root, OutProgram))
		{
			OutFailure = "materialRenderBindings";
			return false;
		}

		OutProgram.MaterialFamilies.clear();
		OutProgram.MaterialFamilies.reserve(Families->Get_Array().size());
		for (size_t Index = 0u; Index < Families->Get_Array().size(); ++Index)
		{
			OutFailure = "materialFamilies:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Families->Get_Array()[Index];
			const DATA_JSON_VALUE* RendererShapes = Required(
				Row, "rendererShapes", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Features = Required(
				Row, "features", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Samples = Required(
				Row, "numericSamples", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalOracles = Required(
				Row, "approvalRequiredOracleIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalBlockers = Required(
				Row, "approvalExecutionBlockers", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalReceipt = Required(
				Row, "approvalArithmeticRow", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "familyId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_Sha(Row, "familyIdentitySha256",
					Parsed.strFamilyIdentitySha256) ||
				!Read_RuntimeId(Row, "evaluatorRegistryId",
					Parsed.strEvaluatorRegistryId) ||
				!Read_RuntimeId(Row, "evaluatorId", Parsed.strEvaluatorId) ||
				!Read_U32(Row, "evaluatorVersion", Parsed.iEvaluatorVersion) ||
				Parsed.iEvaluatorVersion == 0u ||
				!Read_Sha(Row, "evaluatorSha256", Parsed.strEvaluatorSha256) ||
				nullptr == RendererShapes || !Parse_StringArray(
					*RendererShapes, Parsed.RendererShapes, false, 8u, 64u) ||
				!Read_U32(Row, "featureMask", Parsed.iFeatureMask) ||
				nullptr == Features || !Parse_StringArray(
					*Features, Parsed.Features, false, 32u, 128u) ||
				!Read_String(Row, "graphProvenance", Parsed.strGraphProvenance,
					false, 256u) ||
				!Read_Boolean(Row, "cpuNumericOracleVerified",
					Parsed.bCpuNumericOracleVerified) ||
				!Parsed.bCpuNumericOracleVerified ||
				!Read_Boolean(Row, "hlslNumericOracleVerified",
					Parsed.bHlslNumericOracleVerified) ||
				!Parsed.bHlslNumericOracleVerified || nullptr == Samples ||
				!Parse_MaterialFamilySamples(*Samples, Parsed.NumericSamples) ||
				!Read_Sha(Row, "sampleProjectionSha256",
					Parsed.strSampleProjectionSha256) ||
				!Read_RuntimeId(Row, "approvalPolicyRowId",
					Parsed.strApprovalPolicyRowId) ||
				!Read_U32(Row, "approvalArithmeticOrder",
					Parsed.iApprovalArithmeticOrder) ||
				!Read_RuntimeId(Row, "approvalPolicyFamilyId",
					Parsed.strApprovalPolicyFamilyId) || nullptr == ApprovalOracles ||
				!Parse_StringArray(*ApprovalOracles,
					Parsed.ApprovalRequiredOracleIds, false, 16u, 256u) ||
				nullptr == ApprovalBlockers || !Parse_Blockers(
					*ApprovalBlockers, Parsed.ApprovalExecutionBlockers) ||
				nullptr == ApprovalReceipt || !Parse_ApprovalArithmeticReceipt(
					*ApprovalReceipt, Parsed.ApprovalArithmeticReceipt) ||
				!Read_Sha(Row, "approvalArithmeticRowSha256",
					Parsed.strApprovalArithmeticRowSha256) ||
				nullptr == Blockers ||
				!Parse_Blockers(*Blockers, Parsed.PreservedBlockers))
			{
				return false;
			}
			if (Parsed.iApprovalArithmeticOrder != Index ||
				Parsed.ApprovalArithmeticReceipt.strPolicyRowId !=
					Parsed.strApprovalPolicyRowId ||
				Parsed.ApprovalArithmeticReceipt.strUpstreamFamilyId !=
					Parsed.Row.strId ||
				Parsed.ApprovalArithmeticReceipt.strFamilyIdentitySha256 !=
					Parsed.strFamilyIdentitySha256 ||
				Parsed.ApprovalArithmeticReceipt.strEvaluatorId !=
					Parsed.strEvaluatorId ||
				Parsed.ApprovalArithmeticReceipt.iEvaluatorVersion !=
					Parsed.iEvaluatorVersion ||
				Parsed.ApprovalArithmeticReceipt.strEvaluatorSha256 !=
					Parsed.strEvaluatorSha256 ||
				Parsed.ApprovalArithmeticReceipt.strPolicyFamilyId !=
					Parsed.strApprovalPolicyFamilyId ||
				Parsed.ApprovalArithmeticReceipt.RequiredOracleIds !=
					Parsed.ApprovalRequiredOracleIds ||
				Parsed.ApprovalArithmeticReceipt.ExecutionBlockers !=
					Parsed.ApprovalExecutionBlockers ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
						*ApprovalReceipt)) != Parsed.strApprovalArithmeticRowSha256)
			{
				return false;
			}
			OutProgram.MaterialFamilies.push_back(std::move(Parsed));
		}

		OutProgram.MaterialRecipes.clear();
		OutProgram.MaterialRecipes.reserve(Recipes->Get_Array().size());
		for (size_t Index = 0u; Index < Recipes->Get_Array().size(); ++Index)
		{
			OutFailure = "materialRecipes:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Recipes->Get_Array()[Index];
			const DATA_JSON_VALUE* Inputs = Required(Row, "inputIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Static = Required(
				Row, "staticBindingIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Render = Required(
				Row, "renderBindingIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Samples = Required(
				Row, "numericBindingSamples", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "recipeId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_String(Row, "sourceMaterialPath", Parsed.strSourceMaterialPath,
					false, 4096u) ||
				!Read_Sha(Row, "sourceRecipeCompositionSha256",
					Parsed.strSourceRecipeCompositionSha256) ||
				!Read_RuntimeId(Row, "familyId", Parsed.strFamilyId) ||
				!Read_RuntimeId(Row, "evaluatorRegistryId",
					Parsed.strEvaluatorRegistryId) ||
				nullptr == Inputs ||
				!Parse_StringArray(*Inputs, Parsed.InputIds, false, 256u, 1024u) ||
				nullptr == Static ||
				!Parse_StringArray(*Static, Parsed.StaticBindingIds, false, 64u, 1024u) ||
				nullptr == Render ||
				!Parse_StringArray(*Render, Parsed.RenderBindingIds, false, 64u, 1024u) ||
				nullptr == Samples ||
				!Parse_MaterialRecipeSamples(*Samples, Parsed.NumericBindingSamples) ||
				!Read_Sha(Row, "bindingSha256", Parsed.strBindingSha256) ||
				nullptr == Blockers ||
				!Parse_Blockers(*Blockers, Parsed.PreservedBlockers))
			{
				return false;
			}
			OutProgram.MaterialRecipes.push_back(std::move(Parsed));
		}

		OutProgram.MaterialOccurrences.clear();
		OutProgram.MaterialOccurrences.reserve(Occurrences->Get_Array().size());
		for (size_t Index = 0u; Index < Occurrences->Get_Array().size(); ++Index)
		{
			OutFailure = "materialOccurrences:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Occurrences->Get_Array()[Index];
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "preservedBlockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "occurrenceId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_RuntimeId(Row, "emitterId", Parsed.strEmitterId) ||
				!Read_RuntimeId(Row, "cueId", Parsed.strCueId) ||
				!Parse_RendererKind(Row, Parsed.eRenderer) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				!Read_RuntimeId(Row, "familyId", Parsed.strFamilyId) ||
				!Read_RuntimeId(Row, "evaluatorRegistryId",
					Parsed.strEvaluatorRegistryId) ||
				!Read_Sha(Row, "sourceOccurrenceIdentitySha256",
					Parsed.strSourceOccurrenceIdentitySha256) ||
				!Read_Sha(Row, "bindingSha256", Parsed.strBindingSha256) ||
				!Read_Sha(Row, "sourceOccurrenceBindingSha256",
					Parsed.strSourceOccurrenceBindingSha256) ||
				nullptr == Blockers ||
				!Parse_Blockers(*Blockers, Parsed.PreservedBlockers))
			{
				return false;
			}
			OutProgram.MaterialOccurrences.push_back(std::move(Parsed));
		}
		OutFailure.clear();
		return true;
	}

	bool_t Parse_SamplerDescriptor(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_SAMPLER_DESCRIPTOR& OutDescriptor)
	{
		const DATA_JSON_VALUE* Border = Required(
			Value, "borderColor", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"type", "filterUe3", "filterD3d11", "addressUUe3",
				"addressUD3d11", "addressVUe3", "addressVD3d11", "addressWUe3",
				"addressWD3d11", "mipLODBias", "maxAnisotropy",
				"comparisonFuncName", "comparisonFuncD3d11", "borderColor",
				"minLOD", "maxLOD", "sRgb", "srvColorSpace", "lodGroup" }) &&
			Read_ExpectedString(
				Value, "type", "D3D11_SAMPLER_DESC_AND_SRV_COLOR_SPACE") &&
			(OutDescriptor.strType =
				"D3D11_SAMPLER_DESC_AND_SRV_COLOR_SPACE", true) &&
			Read_ClosedString(Value, "filterUe3", { "tf_linear" },
				OutDescriptor.strFilterUe3) &&
			Read_U32(Value, "filterD3d11", OutDescriptor.iFilterD3d11) &&
			OutDescriptor.iFilterD3d11 == 21u &&
			Read_ClosedString(Value, "addressUUe3", { "ta_wrap", "ta_clamp" },
				OutDescriptor.strAddressUUe3) &&
			Read_U32(Value, "addressUD3d11", OutDescriptor.iAddressUD3d11) &&
			(OutDescriptor.iAddressUD3d11 == 1u ||
				OutDescriptor.iAddressUD3d11 == 3u) &&
			Read_ClosedString(Value, "addressVUe3", { "ta_wrap", "ta_clamp" },
				OutDescriptor.strAddressVUe3) &&
			Read_U32(Value, "addressVD3d11", OutDescriptor.iAddressVD3d11) &&
			(OutDescriptor.iAddressVD3d11 == 1u ||
				OutDescriptor.iAddressVD3d11 == 3u) &&
			Read_ClosedString(Value, "addressWUe3", { "ta_wrap" },
				OutDescriptor.strAddressWUe3) &&
			Read_U32(Value, "addressWD3d11", OutDescriptor.iAddressWD3d11) &&
			OutDescriptor.iAddressWD3d11 == 1u &&
			Read_FiniteF64(Value, "mipLODBias", OutDescriptor.fMipLodBias) &&
			Read_U32(Value, "maxAnisotropy", OutDescriptor.iMaxAnisotropy) &&
			Read_ClosedString(Value, "comparisonFuncName",
				{ "D3D11_COMPARISON_NEVER" }, OutDescriptor.strComparisonFuncName) &&
			Read_U32(Value, "comparisonFuncD3d11",
				OutDescriptor.iComparisonFuncD3d11) &&
			OutDescriptor.iComparisonFuncD3d11 == 1u && nullptr != Border &&
			Parse_FixedF64Array(*Border, OutDescriptor.vBorderColor) &&
			Read_FiniteF64(Value, "minLOD", OutDescriptor.fMinLod) &&
			Read_FiniteF64(Value, "maxLOD", OutDescriptor.fMaxLod) &&
			Read_Boolean(Value, "sRgb", OutDescriptor.bSrgb) &&
			Read_ClosedString(Value, "srvColorSpace", { "SRGB", "LINEAR" },
				OutDescriptor.strSrvColorSpace) &&
			((OutDescriptor.bSrgb && OutDescriptor.strSrvColorSpace == "SRGB") ||
				(!OutDescriptor.bSrgb && OutDescriptor.strSrvColorSpace == "LINEAR")) &&
			Read_ClosedString(Value, "lodGroup", {
					"texturegroup_effects", "texturegroup_effectsnormalmap",
					"texturegroup_character", "texturegroup_characternormalmap",
					"texturegroup_characterspecular" }, OutDescriptor.strLodGroup);
	}

	bool_t Parse_D3dStencilFace(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_D3D_STENCIL_FACE& OutFace)
	{
		return Has_ExactKeyOrder(Value, {
				"StencilFailOp", "StencilDepthFailOp", "StencilPassOp",
				"StencilFunc" }) &&
			Read_U32(Value, "StencilFailOp", OutFace.iStencilFailOp) &&
			Read_U32(Value, "StencilDepthFailOp", OutFace.iStencilDepthFailOp) &&
			Read_U32(Value, "StencilPassOp", OutFace.iStencilPassOp) &&
			Read_U32(Value, "StencilFunc", OutFace.iStencilFunc);
	}

	bool_t Parse_D3dDescriptor(
		const DATA_JSON_VALUE& Value,
		const EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND Kind,
		EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR& OutDescriptor)
	{
		OutDescriptor.eKind = Kind;
		if (Kind == EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL)
		{
			const DATA_JSON_VALUE* Front = Required(
				Value, "FrontFace", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* Back = Required(
				Value, "BackFace", DATA_JSON_TYPE::OBJECT);
			EFFECT_RUNTIME_PROGRAM_D3D_DEPTH_STENCIL Parsed;
			if (!Has_ExactKeyOrder(Value, {
					"DepthEnable", "DepthWriteMask", "DepthFunc", "StencilEnable",
					"StencilReadMask", "StencilWriteMask", "FrontFace", "BackFace" }) ||
				!Read_Boolean(Value, "DepthEnable", Parsed.bDepthEnable) ||
				!Read_U32(Value, "DepthWriteMask", Parsed.iDepthWriteMask) ||
				!Read_U32(Value, "DepthFunc", Parsed.iDepthFunc) ||
				!Read_Boolean(Value, "StencilEnable", Parsed.bStencilEnable) ||
				!Read_U32(Value, "StencilReadMask", Parsed.iStencilReadMask) ||
				!Read_U32(Value, "StencilWriteMask", Parsed.iStencilWriteMask) ||
				nullptr == Front || nullptr == Back ||
				!Parse_D3dStencilFace(*Front, Parsed.FrontFace) ||
				!Parse_D3dStencilFace(*Back, Parsed.BackFace))
			{
				return false;
			}
			OutDescriptor.DepthStencil = std::move(Parsed);
			return true;
		}
		if (Kind == EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER)
		{
			EFFECT_RUNTIME_PROGRAM_D3D_RASTERIZER Parsed;
			if (!Has_ExactKeyOrder(Value, {
					"FillMode", "CullMode", "FrontCounterClockwise", "DepthBias",
					"DepthBiasClamp", "SlopeScaledDepthBias", "DepthClipEnable",
					"ScissorEnable", "MultisampleEnable", "AntialiasedLineEnable" }) ||
				!Read_U32(Value, "FillMode", Parsed.iFillMode) ||
				!Read_U32(Value, "CullMode", Parsed.iCullMode) ||
				!Read_Boolean(Value, "FrontCounterClockwise",
					Parsed.bFrontCounterClockwise) ||
				!Read_I32(Value, "DepthBias", Parsed.iDepthBias) ||
				!Read_FiniteF64(Value, "DepthBiasClamp", Parsed.fDepthBiasClamp) ||
				!Read_FiniteF64(Value, "SlopeScaledDepthBias",
					Parsed.fSlopeScaledDepthBias) ||
				!Read_Boolean(Value, "DepthClipEnable", Parsed.bDepthClipEnable) ||
				!Read_Boolean(Value, "ScissorEnable", Parsed.bScissorEnable) ||
				!Read_Boolean(Value, "MultisampleEnable",
					Parsed.bMultisampleEnable) ||
				!Read_Boolean(Value, "AntialiasedLineEnable",
					Parsed.bAntialiasedLineEnable))
			{
				return false;
			}
			OutDescriptor.Rasterizer = std::move(Parsed);
			return true;
		}
		const DATA_JSON_VALUE* Border = Required(
			Value, "BorderColor", DATA_JSON_TYPE::ARRAY);
		EFFECT_RUNTIME_PROGRAM_D3D_SAMPLER Parsed;
		if (!Has_ExactKeyOrder(Value, {
				"Filter", "AddressU", "AddressV", "AddressW", "MipLODBias",
				"MaxAnisotropy", "ComparisonFunc", "BorderColor", "MinLOD",
				"MaxLOD" }) ||
			!Read_U32(Value, "Filter", Parsed.iFilter) ||
			!Read_U32(Value, "AddressU", Parsed.iAddressU) ||
			!Read_U32(Value, "AddressV", Parsed.iAddressV) ||
			!Read_U32(Value, "AddressW", Parsed.iAddressW) ||
			!Read_FiniteF64(Value, "MipLODBias", Parsed.fMipLodBias) ||
			!Read_U32(Value, "MaxAnisotropy", Parsed.iMaxAnisotropy) ||
			!Read_U32(Value, "ComparisonFunc", Parsed.iComparisonFunc) ||
			nullptr == Border || !Parse_FixedF64Array(*Border, Parsed.vBorderColor) ||
			!Read_FiniteF64(Value, "MinLOD", Parsed.fMinLod) ||
			!Read_FiniteF64(Value, "MaxLOD", Parsed.fMaxLod))
		{
			return false;
		}
		OutDescriptor.Sampler = std::move(Parsed);
		return true;
	}

	bool_t Parse_D3dDescriptorOracle(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR_ORACLE& OutOracle)
	{
		const DATA_JSON_VALUE* Expected = Required(
			Value, "expectedDescriptor", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Actual = Required(
			Value, "actualDescriptor", DATA_JSON_TYPE::OBJECT);
		std::string Kind;
		if (!Has_ExactKeyOrder(Value, {
				"policyRowId", "descriptorKind", "expectedDescriptor",
				"actualDescriptor", "numericTolerance", "decision" }) ||
			!Read_RuntimeId(Value, "policyRowId", OutOracle.strPolicyRowId) ||
			!Read_ClosedString(Value, "descriptorKind", {
					"D3D11_DEPTH_STENCIL_DESC", "D3D11_RASTERIZER_DESC",
					"D3D11_SAMPLER_DESC" }, Kind) || nullptr == Expected ||
			nullptr == Actual || !Read_FiniteF64(
				Value, "numericTolerance", OutOracle.fNumericTolerance) ||
			OutOracle.fNumericTolerance != 0.0 ||
			!Read_ExpectedString(Value, "decision", "PASS"))
		{
			return false;
		}
		OutOracle.strDecision = "PASS";
		OutOracle.eKind = Kind == "D3D11_DEPTH_STENCIL_DESC" ?
			EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL :
			(Kind == "D3D11_RASTERIZER_DESC" ?
				EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER :
				EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::SAMPLER);
		return Parse_D3dDescriptor(*Expected, OutOracle.eKind, OutOracle.Expected) &&
			Parse_D3dDescriptor(*Actual, OutOracle.eKind, OutOracle.Actual) &&
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Expected) ==
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Actual);
	}

	bool_t Parse_D3dSrv(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_D3D_SRV& OutSrv)
	{
		return Has_ExactKeyOrder(Value, {
				"Format", "ViewDimension", "MostDetailedMip", "MipLevels",
				"srvColorSpace" }) &&
			Read_U32(Value, "Format", OutSrv.iFormat) &&
			Read_U32(Value, "ViewDimension", OutSrv.iViewDimension) &&
			Read_U32(Value, "MostDetailedMip", OutSrv.iMostDetailedMip) &&
			Read_U32(Value, "MipLevels", OutSrv.iMipLevels) &&
			Read_ClosedString(Value, "srvColorSpace", { "SRGB", "LINEAR" },
				OutSrv.strColorSpace);
	}

	bool_t Parse_D3dSrvOracle(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_D3D_SRV_ORACLE& OutOracle)
	{
		const DATA_JSON_VALUE* Expected = Required(
			Value, "expectedSrv", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Actual = Required(
			Value, "actualSrv", DATA_JSON_TYPE::OBJECT);
		return Has_ExactKeyOrder(Value, {
				"policyRowId", "expectedSrv", "actualSrv", "numericTolerance",
				"decision" }) &&
			Read_RuntimeId(Value, "policyRowId", OutOracle.strPolicyRowId) &&
			nullptr != Expected && nullptr != Actual &&
			Parse_D3dSrv(*Expected, OutOracle.Expected) &&
			Parse_D3dSrv(*Actual, OutOracle.Actual) &&
			Read_FiniteF64(Value, "numericTolerance", OutOracle.fNumericTolerance) &&
			OutOracle.fNumericTolerance == 0.0 &&
			Read_ExpectedString(Value, "decision", "PASS") &&
			(OutOracle.strDecision = "PASS", true) &&
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Expected) ==
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Actual);
	}

	bool_t Parse_ApprovalMaterialReceipt(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_APPROVAL_MATERIAL_RECEIPT& OutReceipt)
	{
		const DATA_JSON_VALUE* Occurrences = Required(
			Value, "materialOccurrenceIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Partial = Required(
			Value, "partialSourceExactFields", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Preserved = Required(
			Value, "preservedEvidenceBlockers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Guards = Required(
			Value, "policyFidelityGuards", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Oracles = Required(
			Value, "requiredOracleIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Execution = Required(
			Value, "executionBlockers", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"policyRowId", "upstreamIdentitySha256", "policyBindingSha256",
				"domain", "upstreamMatrixRowId", "materialRecipeId",
				"materialOccurrenceIds", "fieldId", "fieldKind", "bindingOrigin",
				"upstreamDecision", "policyFamilyId", "evidenceFidelity",
				"executionFidelity", "sourceValueAcquired", "partialSourceExactFields",
				"previousSamplerAdmission", "fullDescriptorSourceExact", "sourceExact",
				"preservedEvidenceBlockers", "policyFidelityGuards",
				"requiredOracleIds", "executionBlockers", "executionAdmission",
				"productAdmission" }) &&
			Read_RuntimeId(Value, "policyRowId", OutReceipt.strPolicyRowId) &&
			Read_Sha(Value, "upstreamIdentitySha256",
				OutReceipt.strUpstreamIdentitySha256) &&
			Read_Sha(Value, "policyBindingSha256", OutReceipt.strPolicyBindingSha256) &&
			Read_String(Value, "domain", OutReceipt.strDomain, false, 128u) &&
			Read_RuntimeId(Value, "upstreamMatrixRowId",
				OutReceipt.strUpstreamMatrixRowId) &&
			Read_RuntimeId(Value, "materialRecipeId",
				OutReceipt.strMaterialRecipeId) && nullptr != Occurrences &&
			Parse_StringArray(*Occurrences, OutReceipt.MaterialOccurrenceIds,
				false, 64u, 1024u) &&
			Read_String(Value, "fieldId", OutReceipt.strFieldId, false, 1024u) &&
			Read_String(Value, "fieldKind", OutReceipt.strFieldKind, false, 256u) &&
			Read_String(Value, "bindingOrigin", OutReceipt.strBindingOrigin,
				false, 256u) &&
			Read_String(Value, "upstreamDecision", OutReceipt.strUpstreamDecision,
				false, 256u) &&
			Read_RuntimeId(Value, "policyFamilyId", OutReceipt.strPolicyFamilyId) &&
			Read_String(Value, "evidenceFidelity", OutReceipt.strEvidenceFidelity,
				false, 256u) &&
			Read_String(Value, "executionFidelity", OutReceipt.strExecutionFidelity,
				false, 256u) &&
			Read_Boolean(Value, "sourceValueAcquired", OutReceipt.bSourceValueAcquired) &&
			nullptr != Partial && Parse_StringArray(*Partial,
				OutReceipt.PartialSourceExactFields, false, 32u, 256u) &&
			Read_NullableString(Value, "previousSamplerAdmission",
				OutReceipt.PreviousSamplerAdmission, 256u) &&
			Read_Boolean(Value, "fullDescriptorSourceExact",
				OutReceipt.bFullDescriptorSourceExact) &&
			Read_Boolean(Value, "sourceExact", OutReceipt.bSourceExact) &&
			!OutReceipt.bSourceExact && nullptr != Preserved &&
			Parse_Blockers(*Preserved, OutReceipt.PreservedEvidenceBlockers) &&
			nullptr != Guards && Parse_StringArray(
				*Guards, OutReceipt.PolicyFidelityGuards, false, 16u, 256u) &&
			nullptr != Oracles && Parse_StringArray(
				*Oracles, OutReceipt.RequiredOracleIds, false, 16u, 256u) &&
			nullptr != Execution && Parse_Blockers(
				*Execution, OutReceipt.ExecutionBlockers) &&
			Read_Boolean(Value, "executionAdmission",
				OutReceipt.bExecutionAdmission) && !OutReceipt.bExecutionAdmission &&
			Read_Boolean(Value, "productAdmission", OutReceipt.bProductAdmission) &&
			!OutReceipt.bProductAdmission;
	}

	bool_t Parse_ReconstructedMaterialPolicyRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram,
		std::string& OutFailure)
	{
		const DATA_JSON_VALUE* Section = Required(
			Root, "materialPolicyRows", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Section)
		{
			OutFailure = "section";
			return false;
		}
		std::vector<EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY> Staged;
		Staged.reserve(Section->Get_Array().size());
		std::set<uint32_t> ApprovalMaterialOrders;
		for (size_t Index = 0u; Index < Section->Get_Array().size(); ++Index)
		{
			OutFailure = "materialPolicyRows:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Section->Get_Array()[Index];
			const DATA_JSON_VALUE* OccurrenceIds = Required(
				Row, "materialOccurrenceIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* EvidenceBlockers = Required(
				Row, "evidenceBlockers", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalBlockers = Required(
				Row, "approvalExecutionBlockers", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalOracles = Required(
				Row, "approvalRequiredOracleIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalReceipt = Required(
				Row, "approvalMaterialRow", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* SamplerDescriptor = Row.Find("samplerDescriptor");
			const DATA_JSON_VALUE* D3dDescriptor = Row.Find("d3dDescriptorOracle");
			const DATA_JSON_VALUE* D3dSrv = Row.Find("d3dSrvOracle");
			EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY Parsed;
			std::optional<bool_t> Boolean;
			std::optional<double> F64;
			std::optional<uint32_t> EnumOrdinal;
			std::string EnumType;
			std::string EnumValue;
			bool_t SelectionAdmission = false;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "policyRowId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Parse_MaterialPolicyDomain(Row, Parsed.eDomain) ||
				!Read_RuntimeId(Row, "sourceMatrixRowId",
					Parsed.strSourceMatrixRowId) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				nullptr == OccurrenceIds ||
				!Parse_StringArray(
					*OccurrenceIds, Parsed.MaterialOccurrenceIds, false, 64u, 1024u) ||
				!Read_String(Row, "fieldId", Parsed.strFieldId, false, 1024u) ||
				!Read_ClosedString(Row, "fieldKind", {
						"STATIC_PERMUTATION_SELECTION", "RENDER_STATE_DEFAULT",
						"DIRECT_TEXTURE_SAMPLER", "PARENT_DEFAULT_TEXTURE_SAMPLER",
						"SELF_DEFAULT_TEXTURE_SAMPLER" },
					Parsed.strFieldKind) ||
				!Read_ClosedString(Row, "bindingOrigin", {
						"PARENT_DEFAULT", "PARENT_MATERIAL", "INSTANCE_OVERRIDE",
						"SELF_MATERIAL", "SELF_DEFAULT" }, Parsed.strBindingOrigin) ||
				!Read_RuntimeId(Row, "evidenceOwnerRecipeId",
					Parsed.strEvidenceOwnerRecipeId) ||
				!Read_ExpectedString(Row, "policyFidelity",
					"RECONSTRUCTED_APPROVED_V1") ||
				!Read_Boolean(Row, "policySelectionAdmission", SelectionAdmission) ||
				!SelectionAdmission ||
				!Read_RuntimeId(Row, "implementationRegistryId",
					Parsed.strImplementationRegistryId) ||
				!Read_RuntimeId(Row, "implementationId", Parsed.strImplementationId) ||
				!Read_U32(Row, "implementationVersion", Parsed.iImplementationVersion) ||
				Parsed.iImplementationVersion == 0u ||
				!Read_ClosedString(Row, "consumerContract", {
						"MATERIAL_STATIC_SHADER_PERMUTATION",
						"D3D11_SAMPLER_DESC_AND_SRGB_SRV_FORMAT",
						"D3D11_DEPTH_STENCIL_DESC.DepthEnable",
						"PIXEL_SHADER_OPACITY_MASK_CLIP_THRESHOLD",
						"EFFECT_DISTORTION_COMPOSITE_PATH",
						"D3D11_RASTERIZER_DESC.CullMode",
						"MATERIAL_SHADER_LIGHTING_PERMUTATION" },
					Parsed.strConsumerContract) ||
				!Parse_MaterialValueVariant(
					Row, "valueVariant", Parsed.eValueVariant, true) ||
				!Read_NullableBoolean(Row, "boolValue", Boolean) ||
				!Read_NullableF64(Row, "f64Value", F64) ||
				!Read_String(Row, "enumType", EnumType, true, 256u) ||
				!Read_String(Row, "enumValue", EnumValue, true, 4096u) ||
				!Read_NullableU32(Row, "enumOrdinal", EnumOrdinal) ||
				!Read_Sha(Row, "providerBasisSha256", Parsed.strProviderBasisSha256) ||
				!Read_Sha(Row, "numericOracleSha256", Parsed.strNumericOracleSha256) ||
				!Read_OptionalRuntimeId(Row, "d3dStateOracleId",
					Parsed.strD3dStateOracleId) ||
				!Read_RuntimeId(Row, "approvalPolicyRowId",
					Parsed.strApprovalPolicyRowId) ||
				!Read_Sha(Row, "approvalUpstreamIdentitySha256",
					Parsed.strApprovalUpstreamIdentitySha256) ||
				!Read_Sha(Row, "approvalPolicyBindingSha256",
					Parsed.strApprovalPolicyBindingSha256) ||
				!Read_RuntimeId(Row, "approvalPolicyFamilyId",
					Parsed.strApprovalPolicyFamilyId) ||
				!Read_U32(Row, "approvalMaterialOrder",
					Parsed.iApprovalMaterialOrder) || nullptr == ApprovalOracles ||
				!Parse_StringArray(*ApprovalOracles,
					Parsed.ApprovalRequiredOracleIds, false, 16u, 256u) ||
				nullptr == EvidenceBlockers ||
				!Parse_Blockers(*EvidenceBlockers, Parsed.EvidenceBlockers) ||
				nullptr == ApprovalBlockers ||
				!Parse_Blockers(
					*ApprovalBlockers, Parsed.ApprovalExecutionBlockers) ||
				nullptr == ApprovalReceipt || !Parse_ApprovalMaterialReceipt(
					*ApprovalReceipt, Parsed.ApprovalMaterialReceipt) ||
				!Read_Sha(Row, "approvalMaterialRowSha256",
					Parsed.strApprovalMaterialRowSha256) ||
				!Read_Sha(Row, "sourceRowSha256", Parsed.strSourceRowSha256) ||
				nullptr == SamplerDescriptor || nullptr == D3dDescriptor ||
				nullptr == D3dSrv)
			{
				return false;
			}
			Parsed.bPolicySelectionAdmission = SelectionAdmission;
			Parsed.strPolicyFidelity = "RECONSTRUCTED_APPROVED_V1";
			const bool_t Sampler = Parsed.eValueVariant ==
				EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::SAMPLER_DESCRIPTOR;
			if (SamplerDescriptor->Is_Object() != Sampler ||
				SamplerDescriptor->Is_Null() == Sampler)
			{
				return false;
			}
			const uint32_t ScalarPopulated =
				static_cast<uint32_t>(Boolean.has_value()) +
				static_cast<uint32_t>(F64.has_value()) +
				static_cast<uint32_t>(!EnumValue.empty());
			if ((Parsed.eValueVariant ==
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::BOOLEAN &&
					(!Boolean.has_value() || ScalarPopulated != 1u ||
						!EnumType.empty() || EnumOrdinal.has_value())) ||
				(Parsed.eValueVariant == EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::F64 &&
					(!F64.has_value() || ScalarPopulated != 1u ||
						!EnumType.empty() || EnumOrdinal.has_value())) ||
				(Parsed.eValueVariant ==
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::ENUM_STRING &&
					(EnumValue.empty() || ScalarPopulated != 1u ||
						!EnumOrdinal.has_value() || EnumType != "EMaterialLightingModel" ||
						EnumValue != "mlm_unlit" || *EnumOrdinal != 0u)) ||
				(Sampler && (ScalarPopulated != 0u || !EnumType.empty() ||
					EnumOrdinal.has_value())))
			{
				return false;
			}
			Parsed.bValue = Boolean;
			Parsed.fValue = F64;
			Parsed.strEnumType = std::move(EnumType);
			Parsed.strEnumValue = std::move(EnumValue);
			Parsed.iEnumOrdinal = EnumOrdinal;
			if (Sampler)
			{
				EFFECT_RUNTIME_PROGRAM_SAMPLER_DESCRIPTOR Descriptor;
				if (!Parse_SamplerDescriptor(*SamplerDescriptor, Descriptor))
					return false;
				Parsed.SamplerDescriptor = std::move(Descriptor);
			}
			if (D3dDescriptor->Is_Object())
			{
				EFFECT_RUNTIME_PROGRAM_D3D_DESCRIPTOR_ORACLE Oracle;
				if (!Parse_D3dDescriptorOracle(*D3dDescriptor, Oracle) ||
					Oracle.strPolicyRowId != Parsed.Row.strId)
				{
					return false;
				}
				Parsed.D3dDescriptorOracle = std::move(Oracle);
			}
			else if (!D3dDescriptor->Is_Null())
			{
				return false;
			}
			if (D3dSrv->Is_Object())
			{
				EFFECT_RUNTIME_PROGRAM_D3D_SRV_ORACLE Oracle;
				if (!Parse_D3dSrvOracle(*D3dSrv, Oracle) ||
					Oracle.strPolicyRowId != Parsed.Row.strId)
				{
					return false;
				}
				Parsed.D3dSrvOracle = std::move(Oracle);
			}
			else if (!D3dSrv->Is_Null())
			{
				return false;
			}
			const bool_t HasDescriptor = Parsed.D3dDescriptorOracle.has_value();
			const bool_t HasSrv = Parsed.D3dSrvOracle.has_value();
			const std::string ApprovalDomain = Parsed.eDomain ==
				EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::SAMPLER_DESCRIPTOR ?
				"SAMPLER" : (Parsed.eDomain ==
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::STATIC_PERMUTATION ?
					"STATIC_PERMUTATION" : "RENDER_STATE");
			if ((Parsed.strD3dStateOracleId.empty() == HasDescriptor) ||
				(Sampler != HasSrv) ||
				Parsed.iApprovalMaterialOrder >= Section->Get_Array().size() ||
				!ApprovalMaterialOrders.emplace(
					Parsed.iApprovalMaterialOrder).second ||
				Parsed.ApprovalMaterialReceipt.strPolicyRowId !=
					Parsed.strApprovalPolicyRowId ||
				Parsed.ApprovalMaterialReceipt.strUpstreamIdentitySha256 !=
					Parsed.strApprovalUpstreamIdentitySha256 ||
				Parsed.ApprovalMaterialReceipt.strPolicyBindingSha256 !=
					Parsed.strApprovalPolicyBindingSha256 ||
				Parsed.ApprovalMaterialReceipt.strDomain != ApprovalDomain ||
				Parsed.ApprovalMaterialReceipt.strUpstreamMatrixRowId !=
					Parsed.strSourceMatrixRowId ||
				Parsed.ApprovalMaterialReceipt.strMaterialRecipeId !=
					Parsed.strRecipeId ||
				Parsed.ApprovalMaterialReceipt.MaterialOccurrenceIds !=
					Parsed.MaterialOccurrenceIds ||
				Parsed.ApprovalMaterialReceipt.strFieldId != Parsed.strFieldId ||
				Parsed.ApprovalMaterialReceipt.strFieldKind != Parsed.strFieldKind ||
				Parsed.ApprovalMaterialReceipt.strBindingOrigin !=
					Parsed.strBindingOrigin ||
				Parsed.ApprovalMaterialReceipt.strPolicyFamilyId !=
					Parsed.strApprovalPolicyFamilyId ||
				Parsed.ApprovalMaterialReceipt.RequiredOracleIds !=
					Parsed.ApprovalRequiredOracleIds ||
				Parsed.ApprovalMaterialReceipt.ExecutionBlockers !=
					Parsed.ApprovalExecutionBlockers ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
						*ApprovalReceipt)) != Parsed.strApprovalMaterialRowSha256)
			{
				return false;
			}
			Staged.push_back(std::move(Parsed));
		}
		if (ApprovalMaterialOrders.size() != Section->Get_Array().size())
			return false;
		OutProgram.MaterialPolicies = std::move(Staged);
		OutFailure.clear();
		return true;
	}

	bool_t Parse_ReconstructedTextureRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram,
		std::string& OutFailure)
	{
		const DATA_JSON_VALUE* Bindings = Required(
			Root, "materialTextureBindings", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Resources = Required(
			Root, "rendererTextureResources", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Bindings || nullptr == Resources)
			return false;

		uint32_t ResolvedCount = 0u;
		uint32_t UnresolvedCount = 0u;
		uint32_t RuntimeCookReceiptCount = 0u;
		uint32_t ReconstructedDeploymentReceiptCount = 0u;
		OutProgram.MaterialTextureBindings.clear();
		OutProgram.MaterialTextureBindings.reserve(Bindings->Get_Array().size());
		for (size_t Index = 0u; Index < Bindings->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Bindings->Get_Array()[Index];
			const DATA_JSON_VALUE* OccurrenceIds = Required(
				Row, "materialOccurrenceIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "blockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING Parsed;
			std::string ReceiptStatus;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "bindingId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_RuntimeId(Row, "recipeId", Parsed.strRecipeId) ||
				!Read_RuntimeId(
					Row, "materialInputFieldId", Parsed.strMaterialInputFieldId) ||
				!Read_String(Row, "logicalTexturePath", Parsed.strLogicalTexturePath,
					false, 4096u) ||
				!Read_RuntimeId(Row, "samplerPolicyRowId",
					Parsed.strSamplerPolicyRowId) ||
				nullptr == OccurrenceIds ||
				!Parse_StringArray(*OccurrenceIds, Parsed.MaterialOccurrenceIds,
					false, 64u, 1024u) ||
				!Read_RuntimeId(Row, "sourceBindingId", Parsed.strSourceBindingId) ||
				!Read_Sha(Row, "sourceBindingRowSha256",
					Parsed.strSourceBindingRowSha256) ||
				!Read_RuntimeId(Row, "sourceTextureResourceId",
					Parsed.strSourceTextureResourceId) ||
				!Read_Sha(Row, "sourceTextureResourceRowSha256",
					Parsed.strSourceTextureResourceRowSha256) ||
				!Read_OptionalRuntimeId(Row, "sourceProvisioningProposalId",
					Parsed.strSourceProvisioningProposalId) ||
				!Read_OptionalSha(Row, "sourceProvisioningProposalRowSha256",
					Parsed.strSourceProvisioningProposalRowSha256) ||
				!Read_OptionalRuntimeId(Row, "sourceDeploymentRowId",
					Parsed.strSourceDeploymentRowId) ||
				!Read_OptionalSha(Row, "sourceDeploymentRowSha256",
					Parsed.strSourceDeploymentRowSha256) ||
				!Read_String(Row, "sourceReceiptStatus", ReceiptStatus, false, 128u) ||
				!Read_NullableNormalizedRelativePath(
					Row, "runtimeAssetId", "Effect/", Parsed.strRuntimeAssetId) ||
				!Parse_TextureResolutionStatus(Row, Parsed.eResolutionStatus) ||
				!Read_String(Row, "bindingBasis", Parsed.strBindingBasis, false, 256u) ||
				nullptr == Blockers || !Parse_Blockers(*Blockers, Parsed.Blockers))
			{
				OutFailure = "binding:" + std::to_string(Index);
				return false;
			}
			Parsed.strSourceReceiptStatus = ReceiptStatus;
			if (Parsed.eResolutionStatus !=
					EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET ||
				!Parsed.strRuntimeAssetId.has_value() ||
				Parsed.Blockers != std::vector<std::string>{
					"R4_TEXTURE_SRV_CONSUMER_NOT_COMPLETE" })
			{
				OutFailure = "resolution-contract:" + std::to_string(Index);
				return false;
			}
			++ResolvedCount;
			if (ReceiptStatus == "RESOLVED_EXACT_RUNTIME_COOK_RECEIPT")
			{
				if (Parsed.strBindingBasis !=
						"EXACT_FULL_LOGICAL_PATH_RUNTIME_COOK_RECEIPT" ||
					!Parsed.strSourceProvisioningProposalId.empty() ||
					!Parsed.strSourceProvisioningProposalRowSha256.empty() ||
					!Parsed.strSourceDeploymentRowId.empty() ||
					!Parsed.strSourceDeploymentRowSha256.empty())
				{
					OutFailure = "runtime-cook-contract:" + std::to_string(Index);
					return false;
				}
				++RuntimeCookReceiptCount;
			}
			else if (ReceiptStatus ==
				"RESOLVED_RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT")
			{
				if (Parsed.strSourceProvisioningProposalId.empty() ||
					Parsed.strSourceProvisioningProposalRowSha256.empty() ||
					Parsed.strSourceDeploymentRowId.empty() ||
					Parsed.strSourceDeploymentRowSha256.empty() ||
					Parsed.strBindingBasis !=
						"RECONSTRUCTED_EXACT_DDS_DEPLOYMENT_RECEIPT")
				{
					OutFailure = "deployment-contract:" + std::to_string(Index);
					return false;
				}
				++ReconstructedDeploymentReceiptCount;
			}
			else
			{
				OutFailure = "receipt-status:" + std::to_string(Index);
				return false;
			}
			OutProgram.MaterialTextureBindings.push_back(std::move(Parsed));
		}
		if (ResolvedCount != ACTIVE_RECONSTRUCTED_PROFILE.iResolvedTextureCount ||
			UnresolvedCount != ACTIVE_RECONSTRUCTED_PROFILE.iUnresolvedTextureCount ||
			RuntimeCookReceiptCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iRuntimeCookReceiptCount ||
			ReconstructedDeploymentReceiptCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iReconstructedDeploymentReceiptCount ||
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Bindings)) !=
				ACTIVE_RECONSTRUCTED_PROFILE.MaterialTextureBindingsSha256 ||
			(ACTIVE_RECONSTRUCTED_PROFILE.iUnresolvedTextureCount == 0u &&
				ACTIVE_RECONSTRUCTED_PROFILE.eRequiredTextureStatus !=
					EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::RESOLVED_EXACT_RUNTIME_ASSET) ||
			(ACTIVE_RECONSTRUCTED_PROFILE.iUnresolvedTextureCount != 0u &&
				ACTIVE_RECONSTRUCTED_PROFILE.eRequiredTextureStatus !=
					EFFECT_RUNTIME_TEXTURE_RESOLUTION_STATUS::UNRESOLVED_RUNTIME_ASSET))
		{
			OutFailure = "profile-count";
			return false;
		}

		OutProgram.RendererTextureResources.clear();
		OutProgram.RendererTextureResources.reserve(Resources->Get_Array().size());
		for (size_t Index = 0u; Index < Resources->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Resources->Get_Array()[Index];
			const DATA_JSON_VALUE* Blockers = Required(
				Row, "blockers", DATA_JSON_TYPE::ARRAY);
			EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "textureResourceId",
					static_cast<uint32_t>(Index), Parsed.Row) ||
				!Read_RuntimeId(Row, "emitterId", Parsed.strEmitterId) ||
				!Read_RuntimeId(Row, "sourceNode", Parsed.strSourceNode) ||
				!Read_RuntimeId(Row, "materialOccurrenceId",
					Parsed.strMaterialOccurrenceId) ||
				!Read_String(Row, "sourceMaterialPath", Parsed.strSourceMaterialPath,
					false, 4096u) ||
				!Read_ClosedString(Row, "slotId",
					{ "base", "noise", "mask", "emissive", "dissolve" },
					Parsed.strSlotId) ||
				!Read_NormalizedRelativePath(Row, "assetId", "Effect/",
					Parsed.strAssetId) ||
				!Read_Sha(Row, "sourceResourceProjectionSha256",
					Parsed.strSourceResourceProjectionSha256) ||
				!Read_ExpectedString(Row, "projectionRole",
					"RECONSTRUCTED_RENDERER_SLOT_PROJECTION") ||
				nullptr == Blockers || !Parse_Blockers(*Blockers, Parsed.Blockers))
			{
				OutFailure = "renderer-resource:" + std::to_string(Index);
				return false;
			}
			Parsed.strProjectionRole = "RECONSTRUCTED_RENDERER_SLOT_PROJECTION";
			OutProgram.RendererTextureResources.push_back(std::move(Parsed));
		}
		return true;
	}

	bool_t Parse_GeometrySubmesh(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_GEOMETRY_SUBMESH& OutSubmesh)
	{
		const DATA_JSON_VALUE* Counts = Required(
			Value, "channelCounts", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Hashes = Required(
			Value, "channelSha256", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Bounds = Required(
			Value, "boundsF32Hex", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactKeyOrder(Value, {
				"name", "materialIndex", "vertexCount", "indexCount",
				"channelCounts", "channelSha256", "boundsF32Hex" }) ||
			!Read_String(Value, "name", OutSubmesh.strName, false, 1024u) ||
			!Read_U32(Value, "materialIndex", OutSubmesh.iMaterialIndex) ||
			!Read_U32(Value, "vertexCount", OutSubmesh.iVertexCount) ||
			OutSubmesh.iVertexCount == 0u ||
			!Read_U32(Value, "indexCount", OutSubmesh.iIndexCount) ||
			OutSubmesh.iIndexCount == 0u || nullptr == Counts ||
			!Has_ExactKeyOrder(*Counts, {
					"position", "normal", "tangentXyz", "tangentW", "uv0",
					"color0" }) ||
			!Read_U32(*Counts, "position", OutSubmesh.ChannelCounts.iPosition) ||
			!Read_U32(*Counts, "normal", OutSubmesh.ChannelCounts.iNormal) ||
			!Read_U32(*Counts, "tangentXyz",
				OutSubmesh.ChannelCounts.iTangentXyz) ||
			!Read_U32(*Counts, "tangentW", OutSubmesh.ChannelCounts.iTangentW) ||
			!Read_U32(*Counts, "uv0", OutSubmesh.ChannelCounts.iUv0) ||
			!Read_U32(*Counts, "color0", OutSubmesh.ChannelCounts.iColor0) ||
			nullptr == Hashes || !Has_ExactKeyOrder(*Hashes, {
					"position", "normal", "tangentXyz", "tangentW", "uv0",
					"color0", "indicesU32" }) ||
			!Read_Sha(*Hashes, "position", OutSubmesh.ChannelSha256.strPosition) ||
			!Read_Sha(*Hashes, "normal", OutSubmesh.ChannelSha256.strNormal) ||
			!Read_Sha(*Hashes, "tangentXyz",
				OutSubmesh.ChannelSha256.strTangentXyz) ||
			!Read_Sha(*Hashes, "tangentW", OutSubmesh.ChannelSha256.strTangentW) ||
			!Read_Sha(*Hashes, "uv0", OutSubmesh.ChannelSha256.strUv0) ||
			!Read_NullableSha256(
				*Hashes, "color0", OutSubmesh.ChannelSha256.strColor0) ||
			!Read_Sha(*Hashes, "indicesU32",
				OutSubmesh.ChannelSha256.strIndicesU32) || nullptr == Bounds ||
			!Parse_FixedLowerHexArray(*Bounds, 8u, OutSubmesh.BoundsF32Hex))
		{
			return false;
		}
		const uint32_t VertexCount = OutSubmesh.iVertexCount;
		return OutSubmesh.ChannelCounts.iPosition == VertexCount &&
			OutSubmesh.ChannelCounts.iNormal == VertexCount &&
			OutSubmesh.ChannelCounts.iTangentXyz == VertexCount &&
			OutSubmesh.ChannelCounts.iTangentW == VertexCount &&
			OutSubmesh.ChannelCounts.iUv0 == VertexCount &&
			((OutSubmesh.ChannelCounts.iColor0 == 0u &&
				!OutSubmesh.ChannelSha256.strColor0.has_value()) ||
			 (OutSubmesh.ChannelCounts.iColor0 == VertexCount &&
				OutSubmesh.ChannelSha256.strColor0.has_value()));
	}

	bool_t Parse_ApprovalGeometryReceipt(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_PROGRAM_APPROVAL_GEOMETRY_RECEIPT& OutReceipt)
	{
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "executionBlockers", DATA_JSON_TYPE::ARRAY);
		return Has_ExactKeyOrder(Value, {
				"assetId", "sourceObject", "candidateResourceSha256", "payloadSha256",
				"metadataIdentitySha256", "geometryPreScale",
				"artifactBindingIntegrity", "sourceFidelity", "sourceExact",
				"executionBlockers", "executionAdmission", "productAdmission" }) &&
			Read_NormalizedRelativePath(Value, "assetId", "Effect/",
				OutReceipt.strAssetId) &&
			Read_RuntimeId(Value, "sourceObject", OutReceipt.strSourceObject) &&
			Read_Sha(Value, "candidateResourceSha256",
				OutReceipt.strCandidateResourceSha256) &&
			Read_Sha(Value, "payloadSha256", OutReceipt.strPayloadSha256) &&
			Read_Sha(Value, "metadataIdentitySha256",
				OutReceipt.strMetadataIdentitySha256) &&
			Read_FiniteF64(Value, "geometryPreScale",
				OutReceipt.fGeometryPreScale) &&
			Read_ExpectedString(Value, "artifactBindingIntegrity",
				"EXPECTED_G02_TUPLE_MATCHES_STAGED_BYTES") &&
			(OutReceipt.strArtifactBindingIntegrity =
				"EXPECTED_G02_TUPLE_MATCHES_STAGED_BYTES", true) &&
			Read_ExpectedString(Value, "sourceFidelity",
				"GEOMETRY_ARTIFACT_BINDING_NOT_SOURCE_EXACT") &&
			(OutReceipt.strSourceFidelity =
				"GEOMETRY_ARTIFACT_BINDING_NOT_SOURCE_EXACT", true) &&
			Read_Boolean(Value, "sourceExact", OutReceipt.bSourceExact) &&
			!OutReceipt.bSourceExact && nullptr != Blockers &&
			Parse_Blockers(*Blockers, OutReceipt.ExecutionBlockers) &&
			Read_Boolean(Value, "executionAdmission",
				OutReceipt.bExecutionAdmission) && !OutReceipt.bExecutionAdmission &&
			Read_Boolean(Value, "productAdmission", OutReceipt.bProductAdmission) &&
			!OutReceipt.bProductAdmission;
	}

	bool_t Parse_ReconstructedGeometryRows(
		const DATA_JSON_VALUE& Root,
		EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& OutProgram,
		std::string& OutFailure)
	{
		const DATA_JSON_VALUE* Carriers = Required(
			Root, "geometryCarriers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Uses = Required(
			Root, "geometryUses", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Carriers || nullptr == Uses)
		{
			OutFailure = "sections";
			return false;
		}

		OutProgram.GeometryCarriers.clear();
		OutProgram.GeometryCarriers.reserve(Carriers->Get_Array().size());
		std::set<uint32_t> ApprovalGeometryOrders;
		for (size_t Index = 0u; Index < Carriers->Get_Array().size(); ++Index)
		{
			OutFailure = "geometryCarriers:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Carriers->Get_Array()[Index];
			const DATA_JSON_VALUE* Submeshes = Required(
				Row, "submeshes", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* ApprovalReceipt = Required(
				Row, "approvalGeometryRow", DATA_JSON_TYPE::OBJECT);
			EFFECT_RUNTIME_PROGRAM_GEOMETRY_CARRIER Parsed;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "carrierId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_String(Row, "bindingId", Parsed.strBindingId, false, 4096u) ||
				!Read_RuntimeId(Row, "sourceObject", Parsed.strSourceObject) ||
				!Read_NormalizedRelativePath(
					Row, "assetId", "Effect/", Parsed.strAssetId) ||
				!Read_ExpectedString(Row, "formatVersion", "1.1") ||
				!Read_U32(Row, "candidateResourceByteSize",
					Parsed.iCandidateResourceByteSize) ||
				Parsed.iCandidateResourceByteSize == 0u ||
				!Read_Sha(Row, "candidateResourceSha256",
					Parsed.strCandidateResourceSha256) ||
				!Read_Sha(Row, "payloadSha256", Parsed.strPayloadSha256) ||
				!Read_Sha(Row, "provenanceSha256", Parsed.strProvenanceSha256) ||
				!Read_ExpectedString(Row, "provenanceRole",
					"WMODEL_METADATA_IDENTITY_SHA256") ||
				!Read_Sha(Row, "metadataIdentitySha256",
					Parsed.strMetadataIdentitySha256) ||
				!Read_Sha(Row, "cacheIdentitySha256", Parsed.strCacheIdentitySha256) ||
				!Read_FiniteF64(
					Row, "geometryPreScale", Parsed.fGeometryPreScale) ||
				Parsed.fGeometryPreScale <= 0.0 ||
				!Read_LowerHex(Row, "geometryPreScaleF32Hex", 8u,
					Parsed.strGeometryPreScaleF32Hex) ||
				!Read_U32(Row, "channelMask", Parsed.iChannelMask) ||
				!Read_U32(Row, "evidenceFlags", Parsed.iEvidenceFlags) ||
				!Read_Sha(Row, "expectedTupleSha256", Parsed.strExpectedTupleSha256) ||
				nullptr == Submeshes ||
				Submeshes->Get_Array().empty() ||
				Submeshes->Get_Array().size() > 128u ||
				!Read_U32(Row, "approvalGeometryOrder",
					Parsed.iApprovalGeometryOrder) ||
				Parsed.iApprovalGeometryOrder >= Carriers->Get_Array().size() ||
				!ApprovalGeometryOrders.emplace(
					Parsed.iApprovalGeometryOrder).second ||
				nullptr == ApprovalReceipt ||
				!Parse_ApprovalGeometryReceipt(
					*ApprovalReceipt, Parsed.ApprovalGeometryReceipt) ||
				!Read_Sha(Row, "approvalGeometryRowSha256",
					Parsed.strApprovalGeometryRowSha256) ||
				!Read_Sha(Row, "preparedCacheIdentitySha256",
					Parsed.strPreparedCacheIdentitySha256) ||
				!Read_ExpectedBoolean(Row, "preScaleConsumed", false))
			{
				return false;
			}
			Parsed.strFormatVersion = "1.1";
			Parsed.strProvenanceRole = "WMODEL_METADATA_IDENTITY_SHA256";
			for (const DATA_JSON_VALUE& Submesh : Submeshes->Get_Array())
			{
				EFFECT_RUNTIME_PROGRAM_GEOMETRY_SUBMESH ParsedSubmesh;
				if (!Parse_GeometrySubmesh(Submesh, ParsedSubmesh))
					return false;
				Parsed.Submeshes.push_back(std::move(ParsedSubmesh));
			}
			if (Parsed.ApprovalGeometryReceipt.strAssetId != Parsed.strAssetId ||
				Parsed.ApprovalGeometryReceipt.strSourceObject != Parsed.strSourceObject ||
				Parsed.ApprovalGeometryReceipt.strCandidateResourceSha256 !=
					Parsed.strCandidateResourceSha256 ||
				Parsed.ApprovalGeometryReceipt.strPayloadSha256 != Parsed.strPayloadSha256 ||
				Parsed.ApprovalGeometryReceipt.strMetadataIdentitySha256 !=
					Parsed.strMetadataIdentitySha256 ||
				Parsed.ApprovalGeometryReceipt.fGeometryPreScale !=
					Parsed.fGeometryPreScale ||
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
						*ApprovalReceipt)) != Parsed.strApprovalGeometryRowSha256)
			{
				return false;
			}
			Parsed.bPreScaleConsumed = false;
			OutProgram.GeometryCarriers.push_back(std::move(Parsed));
		}
		if (ApprovalGeometryOrders.size() != Carriers->Get_Array().size())
			return false;

		OutProgram.GeometryUses.clear();
		OutProgram.GeometryUses.reserve(Uses->Get_Array().size());
		for (size_t Index = 0u; Index < Uses->Get_Array().size(); ++Index)
		{
			OutFailure = "geometryUses:" + std::to_string(Index);
			const DATA_JSON_VALUE& Row = Uses->Get_Array()[Index];
			EFFECT_RUNTIME_PROGRAM_GEOMETRY_USE Parsed;
			int32_t SourceMeshPackageRef = 0;
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				!Parse_RowIdentity(Row, "geometryUseId", static_cast<uint32_t>(Index),
					Parsed.Row) ||
				!Read_RuntimeId(Row, "emitterId", Parsed.strEmitterId) ||
				!Read_RuntimeId(Row, "moduleId", Parsed.strModuleId) ||
				!Read_String(Row, "sourceEmitterPath",
					Parsed.strSourceEmitterPath, false, 4096u) ||
				!Read_RuntimeId(Row, "carrierId", Parsed.strCarrierId) ||
				!Read_NormalizedRelativePath(
					Row, "assetId", "Effect/", Parsed.strAssetId) ||
				!Read_I32(Row, "sourceMeshPackageRef", SourceMeshPackageRef) ||
				SourceMeshPackageRef >= 0 ||
				!Read_ExpectedString(
					Row, "sizeSemantics", "DIMENSIONLESS_PARTICLE_SIZE") ||
				!Read_ExpectedString(Row, "preScaleApplication",
					"VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED") ||
				!Read_ExpectedBoolean(Row, "preScaleConsumed", false))
			{
				return false;
			}
			Parsed.iSourceMeshPackageRef = SourceMeshPackageRef;
			Parsed.strSizeSemantics = "DIMENSIONLESS_PARTICLE_SIZE";
			Parsed.strPreScaleApplication =
				"VERTEX_AND_BOUNDS_EXACTLY_ONCE_REQUIRED";
			Parsed.bPreScaleConsumed = false;
			OutProgram.GeometryUses.push_back(std::move(Parsed));
		}
		OutFailure.clear();
		return true;
	}

	std::string Fold_AsciiLower(const std::string_view Value)
	{
		std::string Result;
		Result.reserve(Value.size());
		for (const unsigned char Character : Value)
			Result.push_back(static_cast<char>(std::tolower(Character)));
		return Result;
	}

	template <typename Row>
	std::unordered_map<std::string, const Row*> Index_ById(
		const std::vector<Row>& Rows)
	{
		std::unordered_map<std::string, const Row*> Result;
		Result.reserve(Rows.size());
		for (const Row& RowValue : Rows)
			Result.emplace(RowValue.Row.strId, &RowValue);
		return Result;
	}

	template <typename Row, typename Owner>
	bool_t Validate_PerOwnerOrder(
		const std::vector<Row>& Rows,
		Owner&& GetOwner)
	{
		std::unordered_map<std::string, uint32_t> NextOrder;
		for (const Row& RowValue : Rows)
		{
			const std::string& OwnerId = GetOwner(RowValue);
			uint32_t& Expected = NextOrder[OwnerId];
			if (RowValue.Row.iOrder != Expected)
				return false;
			++Expected;
		}
		return true;
	}

	template <typename Row>
	bool_t Validate_GlobalOrder(const std::vector<Row>& Rows)
	{
		for (size_t Index = 0u; Index < Rows.size(); ++Index)
		{
			if (Index > static_cast<size_t>(UINT32_MAX) ||
				Rows[Index].Row.iOrder != static_cast<uint32_t>(Index))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Validate_ReconstructedGraph(
		const DATA_JSON_VALUE& Root,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		std::string& OutFailure)
	{
		OutFailure = "global-ids";
		std::unordered_set<std::string> GlobalIds;
		GlobalIds.reserve(8192u);
		const auto Register = [&GlobalIds](const auto& Rows) -> bool_t
		{
			for (const auto& Row : Rows)
			{
				if (Row.Row.strId.empty() || !GlobalIds.emplace(Row.Row.strId).second)
					return false;
			}
			return true;
		};
		if (!Register(Program.InputArtifacts) || !Register(Program.Handlers) ||
			!Register(Program.Emitters) || !Register(Program.ActionSchedules) ||
			!Register(Program.Modules) || !Register(Program.Properties) ||
			!Register(Program.PrimitiveLeaves) || !Register(Program.Literals) ||
			!Register(Program.Distributions) || !Register(Program.SeedPolicies) ||
			!Register(Program.ImplicitDefaults) || !Register(Program.PointLightFields) ||
			!Register(Program.MaterialFamilies) || !Register(Program.MaterialRecipes) ||
			!Register(Program.MaterialInputs) ||
			!Register(Program.MaterialStaticBindings) ||
			!Register(Program.MaterialRenderBindings) ||
			!Register(Program.MaterialOccurrences) ||
			!Register(Program.MaterialPolicies) ||
			!Register(Program.MaterialTextureBindings) ||
			!Register(Program.RendererTextureResources) ||
			!Register(Program.GeometryCarriers) || !Register(Program.GeometryUses))
		{
			return false;
		}

		const auto Handlers = Index_ById(Program.Handlers);
		const auto Emitters = Index_ById(Program.Emitters);
		const auto Schedules = Index_ById(Program.ActionSchedules);
		const auto Modules = Index_ById(Program.Modules);
		const auto Properties = Index_ById(Program.Properties);
		const auto Literals = Index_ById(Program.Literals);
		const auto Distributions = Index_ById(Program.Distributions);
		const auto Seeds = Index_ById(Program.SeedPolicies);
		const auto Defaults = Index_ById(Program.ImplicitDefaults);
		const auto PointLights = Index_ById(Program.PointLightFields);
		const auto Families = Index_ById(Program.MaterialFamilies);
		const auto Recipes = Index_ById(Program.MaterialRecipes);
		const auto Inputs = Index_ById(Program.MaterialInputs);
		const auto StaticBindings = Index_ById(Program.MaterialStaticBindings);
		const auto RenderBindings = Index_ById(Program.MaterialRenderBindings);
		const auto Occurrences = Index_ById(Program.MaterialOccurrences);
		const auto Policies = Index_ById(Program.MaterialPolicies);
		const auto TextureBindings = Index_ById(Program.MaterialTextureBindings);
		const auto RendererTextures = Index_ById(Program.RendererTextureResources);
		const auto Carriers = Index_ById(Program.GeometryCarriers);
		const auto GeometryUses = Index_ById(Program.GeometryUses);

		if (Handlers.size() != Program.Handlers.size() ||
			Emitters.size() != Program.Emitters.size() ||
			Schedules.size() != Program.ActionSchedules.size() ||
			Modules.size() != Program.Modules.size() ||
			Properties.size() != Program.Properties.size() ||
			Literals.size() != Program.Literals.size() ||
			Distributions.size() != Program.Distributions.size() ||
			Seeds.size() != Program.SeedPolicies.size() ||
			Defaults.size() != Program.ImplicitDefaults.size() ||
			PointLights.size() != Program.PointLightFields.size() ||
			Families.size() != Program.MaterialFamilies.size() ||
			Recipes.size() != Program.MaterialRecipes.size() ||
			Inputs.size() != Program.MaterialInputs.size() ||
			StaticBindings.size() != Program.MaterialStaticBindings.size() ||
			RenderBindings.size() != Program.MaterialRenderBindings.size() ||
			Occurrences.size() != Program.MaterialOccurrences.size() ||
			Policies.size() != Program.MaterialPolicies.size() ||
			TextureBindings.size() != Program.MaterialTextureBindings.size() ||
			RendererTextures.size() != Program.RendererTextureResources.size() ||
			Carriers.size() != Program.GeometryCarriers.size() ||
			GeometryUses.size() != Program.GeometryUses.size())
		{
			return false;
		}
		const size_t ReadySourceModuleCount = static_cast<size_t>(std::count_if(
			Program.Modules.begin(), Program.Modules.end(), [](const auto& Row)
			{
				return Row.strSourceDecision == "READY_FOR_HANDLER";
			}));
		const size_t BlockedSourceModuleCount = Program.Modules.size() -
			ReadySourceModuleCount;
		const size_t CurrentEngineCdoDistributionCount =
			static_cast<size_t>(std::count_if(
				Program.Distributions.begin(), Program.Distributions.end(),
				[](const auto& Row)
				{
					return Row.strPayloadStatus ==
						"CURRENT_ENGINE_CDO_RECONSTRUCTED";
				}));
		const size_t BlockedDefaultDistributionCount =
			static_cast<size_t>(std::count_if(
				Program.Distributions.begin(), Program.Distributions.end(),
				[](const auto& Row)
				{
					return Row.eVariant ==
							EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE &&
						Row.strPayloadStatus ==
							"UNRESOLVED_SEMANTIC_CLOSURE" &&
						Row.strFidelity == "UNRESOLVED_CROSS_REVISION" &&
						Row.PreservedBlockers == std::vector<std::string>{
							"DEFAULT_DEPENDENT_DISTRIBUTION_REQUIRES_TYPED_DEFAULT_POLICY",
							"DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED",
							"DISTRIBUTION_OPERATION_RECONSTRUCTION_UNVERIFIED",
							"DOWNSTREAM_EVALUATOR_RECEIPT_REQUIRED",
							"INDEPENDENT_NUMERIC_ORACLE_REQUIRED",
						};
				}));
		if ((0u == CurrentEngineCdoDistributionCount &&
			 (0u != BlockedDefaultDistributionCount ||
			  ReadySourceModuleCount != 370u || BlockedSourceModuleCount != 29u)) ||
			(0u != CurrentEngineCdoDistributionCount &&
			 (CurrentEngineCdoDistributionCount != 35u ||
			  BlockedDefaultDistributionCount != 102u ||
			  ReadySourceModuleCount != 291u || BlockedSourceModuleCount != 108u)))
		{
			OutFailure = "source-semantic-denominators";
			return false;
		}
		for (const auto& Row : Program.Distributions)
		{
			if (Row.strPayloadStatus != "CURRENT_ENGINE_CDO_RECONSTRUCTED")
				continue;
			const auto Module = Modules.find(Row.strModuleId);
			const auto Property = Properties.find(Row.strPropertyId);
			if (Row.eVariant != EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ||
				Module == Modules.end() ||
				Module->second->strExactSourceClass != "particlemodulespawn" ||
				Property == Properties.end() ||
				Property->second->strModuleId != Row.strModuleId ||
				Property->second->strPropertyPath != "ratescale" ||
				Row.strPropertyPath != "ratescale" ||
				Row.DefaultMinimum != std::vector<double>(4u, 0.0) ||
				Row.DefaultMaximum != std::vector<double>(4u, 0.0) ||
				!Row.CurveKeys.empty() || !Row.strReferenceId.empty() ||
				!Row.strOccurrenceId.empty() || Row.FieldProvenance.size() != 1u)
			{
				OutFailure = "current-engine-cdo-distribution-owner";
				return false;
			}
			const auto& Provenance = Row.FieldProvenance.front();
			if (Provenance.strFieldPath != "operation" ||
				Provenance.strProvenanceTier !=
					"CURRENT_ENGINE_CDO_RECONSTRUCTED" ||
				Provenance.strEvidenceStatus != "CURRENT_REVISION_EVIDENCE" ||
				!Provenance.strValueSha256.empty() ||
				Provenance.strTypedValueBindingSha256 !=
					"df4ce246be322ec67ebe59726aa2757b7f8383eed117ca1928476e699c1dc124")
			{
				OutFailure = "current-engine-cdo-distribution-provenance";
				return false;
			}
		}

		OutFailure = "orders";
		if (!Validate_PerOwnerOrder(Program.Modules,
				[](const auto& Row) -> const std::string& { return Row.strEmitterId; }) ||
			!Validate_PerOwnerOrder(Program.Properties,
				[](const auto& Row) -> const std::string& { return Row.strModuleId; }) ||
			!Validate_PerOwnerOrder(Program.PrimitiveLeaves,
				[](const auto& Row) -> const std::string& { return Row.strModuleId; }) ||
			!Validate_PerOwnerOrder(Program.Literals,
				[](const auto& Row) -> const std::string& { return Row.strModuleId; }) ||
			!Validate_PerOwnerOrder(Program.Distributions,
				[](const auto& Row) -> const std::string& { return Row.strModuleId; }) ||
			!Validate_GlobalOrder(Program.ActionSchedules) ||
			!Validate_GlobalOrder(Program.SeedPolicies) ||
			!Validate_GlobalOrder(Program.ImplicitDefaults) ||
			!Validate_GlobalOrder(Program.PointLightFields) ||
			!Validate_GlobalOrder(Program.MaterialInputs) ||
			!Validate_GlobalOrder(Program.MaterialStaticBindings) ||
			!Validate_GlobalOrder(Program.MaterialRenderBindings) ||
			!Validate_GlobalOrder(Program.MaterialFamilies) ||
			!Validate_GlobalOrder(Program.MaterialRecipes) ||
			!Validate_GlobalOrder(Program.MaterialOccurrences) ||
			!Validate_GlobalOrder(Program.MaterialPolicies) ||
			!Validate_GlobalOrder(Program.MaterialTextureBindings) ||
			!Validate_GlobalOrder(Program.RendererTextureResources) ||
			!Validate_GlobalOrder(Program.GeometryCarriers) ||
			!Validate_GlobalOrder(Program.GeometryUses))
		{
			return false;
		}

		OutFailure = "handler-registry";
		const DATA_JSON_VALUE* HandlerRows = Required(
			Root, "handlerRegistry", DATA_JSON_TYPE::ARRAY);
		if (nullptr == HandlerRows ||
			HandlerRows->Get_Array().size() != Program.Handlers.size())
		{
			return false;
		}
		for (size_t Index = 0u; Index < Program.Handlers.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_HANDLER& Handler = Program.Handlers[Index];
			const DATA_JSON_VALUE& Row = HandlerRows->Get_Array()[Index];
			DATA_JSON_VALUE Contract;
			if (!Build_ObjectProjection(Row, {
					"handlerKind", "implementationId", "implementationVersion",
					"implementationSha256", "exactSourceClass", "variant",
					"consumerContract" }, Contract))
			{
				return false;
			}
			const std::string ContractSha =
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Contract));
			if (Handler.strContractSha256 != ContractSha ||
				Handler.Row.strId != "handler-" + ContractSha.substr(0u, 24u) ||
				Handler.iImplementationVersion == 0u)
			{
				return false;
			}
		}

		OutFailure = "frozen-subprojections";
		const auto VerifyProjection = [](DATA_JSON_VALUE::ARRAY Projection,
			const std::string_view Expected) -> bool_t
		{
			return CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
					DATA_JSON_VALUE::Array(std::move(Projection)))) == Expected;
		};
		const DATA_JSON_VALUE* ModuleRows = Required(
			Root, "modules", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ScheduleRows = Required(
			Root, "actionSchedules", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* DistributionRows = Required(
			Root, "distributions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* FamilyRows = Required(
			Root, "materialFamilies", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* RecipeRows = Required(
			Root, "materialRecipes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* RendererProjectionRows = Required(
			Root, "rendererTextureResources", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* GeometryProjectionRows = Required(
			Root, "geometryCarriers", DATA_JSON_TYPE::ARRAY);
		if (nullptr == ModuleRows || nullptr == ScheduleRows ||
			nullptr == DistributionRows ||
			nullptr == FamilyRows || nullptr == RecipeRows ||
			nullptr == RendererProjectionRows || nullptr == GeometryProjectionRows ||
			ScheduleRows->Get_Array().size() != Program.ActionSchedules.size())
		{
			return false;
		}
		DATA_JSON_VALUE::ARRAY ScheduleProjection;
		ScheduleProjection.reserve(Program.ActionSchedules.size());
		for (size_t Index = 0u; Index < Program.ActionSchedules.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE& Schedule =
				Program.ActionSchedules[Index];
			DATA_JSON_VALUE Projected;
			if (!Build_ObjectProjection(ScheduleRows->Get_Array()[Index], {
					"order", "sourceCueId", "sourceOccurrenceId", "sourceSystemId",
					"sourceReceiptEventIndex", "globalTimeSeconds", "durationSeconds",
					"sourceCueRowSha256" }, Projected) ||
				Schedule.fGlobalTimeSeconds < 0.0 || Schedule.fDurationSeconds < 0.0)
			{
				return false;
			}
			const std::string ProjectionSha =
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Projected));
			if (ProjectionSha.empty() || Schedule.Row.strId !=
				"action-schedule-" + ProjectionSha.substr(0u, 24u))
			{
				return false;
			}
			ScheduleProjection.push_back(std::move(Projected));
		}
		DATA_JSON_VALUE::ARRAY CapabilityProjection;
		for (const DATA_JSON_VALUE& Row : ModuleRows->Get_Array())
		{
			const DATA_JSON_VALUE* Selection = Required(
				Row, "selectionKind", DATA_JSON_TYPE::STRING);
			if (nullptr == Selection)
				return false;
			if (Selection->Get_String() == "RECONSTRUCTED_HANDLER")
			{
				DATA_JSON_VALUE Projected;
				if (!Build_ObjectProjection(Row,
						{ "moduleId", "capabilityNumericSamples" }, Projected))
				{
					return false;
				}
				CapabilityProjection.push_back(std::move(Projected));
			}
		}
		DATA_JSON_VALUE::ARRAY DistributionProjection;
		DATA_JSON_VALUE::ARRAY DistributionProvenanceProjection;
		for (const DATA_JSON_VALUE& Row : DistributionRows->Get_Array())
		{
			DATA_JSON_VALUE Projected;
			DATA_JSON_VALUE ProvenanceProjected;
			if (!Build_ObjectProjection(
					Row, { "distributionId", "samples" }, Projected) ||
				!Build_ObjectProjection(Row,
					{ "distributionId", "fieldProvenance" }, ProvenanceProjected))
			{
				return false;
			}
			DistributionProjection.push_back(std::move(Projected));
			DistributionProvenanceProjection.push_back(
				std::move(ProvenanceProjected));
		}
		DATA_JSON_VALUE::ARRAY FamilyProjection;
		for (const DATA_JSON_VALUE& Row : FamilyRows->Get_Array())
		{
			DATA_JSON_VALUE Projected;
			if (!Build_ObjectProjection(
					Row, { "familyId", "numericSamples" }, Projected))
			{
				return false;
			}
			FamilyProjection.push_back(std::move(Projected));
		}
		DATA_JSON_VALUE::ARRAY RecipeProjection;
		for (const DATA_JSON_VALUE& Row : RecipeRows->Get_Array())
		{
			DATA_JSON_VALUE Projected;
			if (!Build_ObjectProjection(
					Row, { "recipeId", "numericBindingSamples" }, Projected))
			{
				return false;
			}
			RecipeProjection.push_back(std::move(Projected));
		}
		DATA_JSON_VALUE::ARRAY RendererProjection;
		for (const DATA_JSON_VALUE& Row : RendererProjectionRows->Get_Array())
		{
			DATA_JSON_VALUE Projected;
			if (!Build_ObjectWithoutKey(Row, "rowSha256", Projected))
				return false;
			RendererProjection.push_back(std::move(Projected));
		}
		DATA_JSON_VALUE::ARRAY GeometryProjection;
		for (const DATA_JSON_VALUE& Row : GeometryProjectionRows->Get_Array())
		{
			DATA_JSON_VALUE Projected;
			if (!Build_ObjectWithoutKey(Row, "rowSha256", Projected))
				return false;
			GeometryProjection.push_back(std::move(Projected));
		}
		if (!VerifyProjection(std::move(ScheduleProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.ActionSchedulesSha256) ||
			!VerifyProjection(std::move(CapabilityProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.CapabilitySamplesSha256) ||
			!VerifyProjection(std::move(DistributionProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.DistributionSamplesSha256) ||
			!VerifyProjection(std::move(DistributionProvenanceProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.DistributionProvenanceSha256) ||
			!VerifyProjection(std::move(FamilyProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.MaterialFamilySamplesSha256) ||
			!VerifyProjection(std::move(RecipeProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.MaterialRecipeSamplesSha256) ||
			!VerifyProjection(std::move(RendererProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.RendererTextureResourcesSha256) ||
			!VerifyProjection(std::move(GeometryProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.GeometryCarriersSha256))
		{
			return false;
		}

		std::unordered_map<std::string, std::vector<std::string>> ModuleIdsByEmitter;
		std::unordered_map<std::string, std::vector<std::string>> PropertiesByModule;
		std::unordered_map<std::string, std::vector<std::string>> LeavesByModule;
		std::unordered_map<std::string, std::vector<std::string>> LiteralsByModule;
		std::unordered_map<std::string, std::vector<std::string>> DistributionsByModule;
		std::unordered_map<std::string, std::vector<std::string>> DefaultsByModule;
		std::set<int32_t> ApprovalSourceOrders;
		OutFailure = "module-reverse";
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Row : Program.Modules)
		{
			const auto Handler = Handlers.find(Row.strHandlerRegistryId);
			if (!Emitters.contains(Row.strEmitterId) || Handler == Handlers.end())
			{
				return false;
			}
			const bool_t Reconstructed = Row.eSelection ==
				EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER;
			if (Handler->second->eKind != (Reconstructed ?
					EFFECT_RUNTIME_HANDLER_KIND::RECONSTRUCTED_MODULE :
					EFFECT_RUNTIME_HANDLER_KIND::SOURCE_MODULE) ||
				Handler->second->strExactSourceClass != Row.strExactSourceClass ||
				Handler->second->strVariant != Row.strHandlerVariant ||
				(Reconstructed &&
					(Handler->second->strImplementationId !=
						Row.strCapabilityImplementationId ||
					 Handler->second->iImplementationVersion !=
						Row.iCapabilityImplementationVersion ||
					 Handler->second->strImplementationSha256 !=
						Row.strCapabilityImplementationSha256 ||
					 !Row.ApprovalSourceReceipt.has_value() ||
					 Row.CapabilityNumericSamples.size() != 3u ||
					 !ApprovalSourceOrders.emplace(Row.iApprovalSourceOrder).second)) ||
				(!Reconstructed &&
					(!Row.strCapabilityPolicyFamilyId.empty() ||
					 !Row.strCapabilityImplementationId.empty() ||
					 Row.iCapabilityImplementationVersion != 0u ||
					 !Row.strCapabilitySourceRowSha256.empty())))
			{
				return false;
			}
			if (Reconstructed)
			{
				std::vector<std::string> ScopedActionCueNames;
				const auto Emitter = Emitters.find(Row.strEmitterId);
				if (Emitter == Emitters.end())
					return false;
				for (const auto& Cue : Emitter->second->ActionCueParameterInputs)
				{
					if (std::find(Row.CapabilityActionCueInputNames.begin(),
							Row.CapabilityActionCueInputNames.end(), Cue.strName) !=
						Row.CapabilityActionCueInputNames.end())
					{
						ScopedActionCueNames.push_back(Cue.strName);
					}
				}
				if (ScopedActionCueNames != Row.CapabilityActionCueInputNames ||
					std::set<std::string>(Row.CapabilityActionCueInputNames.begin(),
						Row.CapabilityActionCueInputNames.end()).size() !=
						Row.CapabilityActionCueInputNames.size())
				{
					return false;
				}
			}
			ModuleIdsByEmitter[Row.strEmitterId].push_back(Row.Row.strId);
		}
		if (ApprovalSourceOrders.size() != 29u ||
			*ApprovalSourceOrders.begin() != 0 ||
			*ApprovalSourceOrders.rbegin() != 28)
		{
			return false;
		}
		OutFailure = "property-reverse";
		for (const EFFECT_RUNTIME_PROGRAM_PROPERTY& Row : Program.Properties)
		{
			const auto Module = Modules.find(Row.strModuleId);
			const auto Handler = Handlers.find(Row.strHandlerRegistryId);
			if (Module == Modules.end() || Handler == Handlers.end() ||
				Handler->second->eKind !=
					EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PROPERTY ||
				Handler->second->strExactSourceClass !=
					Module->second->strExactSourceClass ||
				Handler->second->strVariant != Row.strPropertyPath ||
				(Module->second->eSelection ==
					EFFECT_RUNTIME_MODULE_SELECTION::SOURCE_HANDLER &&
					(!Row.strCapabilityConsumptionDecision.empty() ||
					 !Row.strSemanticRole.empty() || Row.bOutputDependencyRequired ||
					 !Row.strCapabilitySourceRowSha256.empty())) ||
				(Module->second->eSelection ==
					EFFECT_RUNTIME_MODULE_SELECTION::RECONSTRUCTED_HANDLER &&
					(Row.strCapabilityConsumptionDecision.empty() ||
					 Row.strSemanticRole.empty() ||
					 Row.strCapabilitySourceRowSha256.empty())))
			{
				return false;
			}
			PropertiesByModule[Row.strModuleId].push_back(Row.Row.strId);
		}
		OutFailure = "primitive-leaf-reverse";
		for (const EFFECT_RUNTIME_PROGRAM_PRIMITIVE_LEAF& Row : Program.PrimitiveLeaves)
		{
			const auto Module = Modules.find(Row.strModuleId);
			const auto Property = Properties.find(Row.strPropertyId);
			const auto Literal = Literals.find(Row.strLiteralId);
			const auto LeafHandler = Handlers.find(Row.strHandlerRegistryId);
			if (Module == Modules.end() || Property == Properties.end() ||
				Literal == Literals.end() || LeafHandler == Handlers.end() ||
				Property->second->strModuleId != Row.strModuleId ||
				Literal->second->strModuleId != Row.strModuleId ||
				Literal->second->strPropertyId != Row.strPropertyId ||
				Literal->second->eVariant != Row.eValueVariant ||
				Row.strTopLevelPropertyPath != Property->second->strPropertyPath ||
				LeafHandler->second->eKind !=
					EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PRIMITIVE ||
				LeafHandler->second->strExactSourceClass !=
					Module->second->strExactSourceClass ||
				LeafHandler->second->strVariant != Row.strTopLevelPropertyPath ||
				Row.strPropertyPath != Literal->second->strPropertyPath)
			{
				return false;
			}
			LeavesByModule[Row.strModuleId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_LITERAL& Row : Program.Literals)
		{
			const auto Property = Properties.find(Row.strPropertyId);
			if (!Modules.contains(Row.strModuleId) || Property == Properties.end() ||
				Property->second->strModuleId != Row.strModuleId)
			{
				return false;
			}
			LiteralsByModule[Row.strModuleId].push_back(Row.Row.strId);
		}
		OutFailure = "distribution-reverse";
		for (size_t DistributionIndex = 0u;
			DistributionIndex < Program.Distributions.size(); ++DistributionIndex)
		{
			const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Row =
				Program.Distributions[DistributionIndex];
			const DATA_JSON_VALUE& RawDistribution =
				DistributionRows->Get_Array()[DistributionIndex];
			const DATA_JSON_VALUE* RawActionCueBindings = Required(
				RawDistribution, "actionCueBindings", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* RawProvenance = Required(
				RawDistribution, "fieldProvenance", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* RawSamples = Required(
				RawDistribution, "samples", DATA_JSON_TYPE::ARRAY);
			const auto Property = Properties.find(Row.strPropertyId);
			const auto Handler = Handlers.find(Row.strEvaluatorRegistryId);
			if (!Modules.contains(Row.strModuleId) || Property == Properties.end() ||
				Property->second->strModuleId != Row.strModuleId ||
				Handler == Handlers.end() ||
				Handler->second->eKind != EFFECT_RUNTIME_HANDLER_KIND::DISTRIBUTION ||
				Handler->second->strExactSourceClass != Row.strSourceClass ||
				nullptr == RawActionCueBindings || nullptr == RawProvenance ||
				nullptr == RawSamples ||
				RawProvenance->Get_Array().size() != Row.FieldProvenance.size() ||
				RawSamples->Get_Array().size() != Row.Samples.size())
			{
				return false;
			}
			const bool_t Multiply = Row.eVariant ==
				EFFECT_RUNTIME_DISTRIBUTION_VARIANT::EF_MULTIPLY;
			const std::string_view ExpectedVariant =
				Row.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::INLINE ? "INLINE" :
				Row.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_PARAMETER ?
					"FLOAT_PARAMETER" :
				Row.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::VECTOR_PARAMETER ?
					"VECTOR_PARAMETER" :
				Row.eVariant == EFFECT_RUNTIME_DISTRIBUTION_VARIANT::FLOAT_CURVE ?
					"FLOAT_CURVE" : "";
			if ((!Multiply && Handler->second->strVariant != ExpectedVariant) ||
				(Multiply &&
					(Handler->second->strImplementationId !=
						Row.strCapabilityImplementationId ||
					 Handler->second->iImplementationVersion !=
						Row.iCapabilityImplementationVersion ||
					 Handler->second->strImplementationSha256 !=
						Row.strCapabilityImplementationSha256 ||
					 Handler->second->strVariant.empty())))
			{
				return false;
			}
			for (size_t ProvenanceIndex = 0u;
				ProvenanceIndex < Row.FieldProvenance.size(); ++ProvenanceIndex)
			{
				const auto& Provenance = Row.FieldProvenance[ProvenanceIndex];
				if (Provenance.strTypedValueBindingSha256.empty())
					continue;
				const DATA_JSON_VALUE& RawItem =
					RawProvenance->Get_Array()[ProvenanceIndex];
				const DATA_JSON_VALUE* RawFieldPath = RawItem.Find("fieldPath");
				const DATA_JSON_VALUE* RawTypedValue =
					RawDistribution.Find(Provenance.strFieldPath);
				if (nullptr == RawFieldPath || nullptr == RawTypedValue)
					return false;
				DATA_JSON_VALUE::OBJECT BindingObject;
				BindingObject.emplace("fieldPath", *RawFieldPath);
				BindingObject.emplace("typedValue", *RawTypedValue);
				if (Provenance.strTypedValueBindingSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							DATA_JSON_VALUE::Object(std::move(BindingObject)))))
				{
					return false;
				}
			}
			for (size_t SampleIndex = 0u;
				SampleIndex < Row.Samples.size(); ++SampleIndex)
			{
				const auto& Sample = Row.Samples[SampleIndex];
				const DATA_JSON_VALUE& RawSample = RawSamples->Get_Array()[SampleIndex];
				const DATA_JSON_VALUE* RawOutput = Required(
					RawSample, "outputValues", DATA_JSON_TYPE::ARRAY);
				if (nullptr == RawOutput ||
					Sample.strActionCueBindingsSha256 !=
						CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
							CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
								*RawActionCueBindings)) ||
					Sample.strOutputSha256 !=
						CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
							CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
								*RawOutput)))
				{
					return false;
				}
				DATA_JSON_VALUE::OBJECT InputObject;
				const auto AddInput = [&RawSample, &InputObject](
					const std::string_view Name) -> bool_t
				{
					const DATA_JSON_VALUE* Value = RawSample.Find(Name);
					return nullptr != Value &&
						InputObject.emplace(std::string(Name), *Value).second;
				};
				if (!AddInput("domain") ||
					!InputObject.emplace("distributionId",
						DATA_JSON_VALUE::String(Row.Row.strId)).second ||
					!AddInput("inputVariant"))
				{
					return false;
				}
				if (Sample.eDomain ==
					EFFECT_RUNTIME_DISTRIBUTION_SAMPLE_DOMAIN::DISTRIBUTION_EVALUATOR)
				{
					if (!AddInput("time") || !AddInput("randomUnits"))
						return false;
				}
				else
				{
					for (const std::string_view Name : {
							"sourceCueId", "branch", "parameterInput",
							"diagnosticStandardBaseValue", "blocked" })
					{
						if (!AddInput(Name))
							return false;
					}
					InputObject.emplace("actionCueBindings", *RawActionCueBindings);
				}
				if (Sample.strInputSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							DATA_JSON_VALUE::Object(std::move(InputObject)))))
				{
					return false;
				}
			}
			DistributionsByModule[Row.strModuleId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_IMPLICIT_DEFAULT& Row : Program.ImplicitDefaults)
		{
			if (!Modules.contains(Row.strModuleId))
				return false;
			DefaultsByModule[Row.strModuleId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_SEED_POLICY& Row : Program.SeedPolicies)
		{
			const auto Module = Modules.find(Row.strModuleId);
			if (Module == Modules.end() || Module->second->strSeedPolicyId != Row.Row.strId)
				return false;
		}
		for (const EFFECT_RUNTIME_PROGRAM_POINT_LIGHT_FIELD& Row : Program.PointLightFields)
		{
			if (!Modules.contains(Row.strModuleId))
				return false;
		}
		std::unordered_set<std::string> ReferencedSeedIds;
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Row : Program.Modules)
		{
			if (!Row.strSeedPolicyId.empty())
			{
				const auto Seed = Seeds.find(Row.strSeedPolicyId);
				if (Seed == Seeds.end() ||
					Seed->second->strModuleId != Row.Row.strId ||
					!ReferencedSeedIds.emplace(Row.strSeedPolicyId).second)
				{
					return false;
				}
			}
		}
		if (ReferencedSeedIds.size() != Program.SeedPolicies.size())
			return false;

		std::vector<std::string> LightModuleIds;
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Row : Program.Modules)
		{
			if (Row.strExactSourceClass == "efparticlemoduletypedatalight")
				LightModuleIds.push_back(Row.Row.strId);
		}
		if (LightModuleIds.size() != 1u ||
			std::any_of(Program.PointLightFields.begin(),
				Program.PointLightFields.end(), [&LightModuleIds](const auto& Row)
				{
					return Row.strModuleId != LightModuleIds.front();
				}))
		{
			return false;
		}

		OutFailure = "emitter-timing-renderer-reverse";
		std::unordered_map<std::string, uint32_t> ScheduleCoverage;
		for (const EFFECT_RUNTIME_PROGRAM_ACTION_SCHEDULE& Row :
			Program.ActionSchedules)
		{
			ScheduleCoverage.emplace(Row.Row.strId, 0u);
		}
		const auto SourceCandidateArtifact = std::find_if(
			Program.InputArtifacts.begin(), Program.InputArtifacts.end(),
			[](const EFFECT_RUNTIME_PROGRAM_INPUT_ARTIFACT& Row)
			{
				return Row.Row.strId == "sourceCandidate";
			});
		if (SourceCandidateArtifact == Program.InputArtifacts.end() ||
			!Is_Sha256(SourceCandidateArtifact->strCanonicalJsonSha256))
		{
			OutFailure = "emitter-timing-renderer-reverse:source-candidate";
			return false;
		}
		const DATA_JSON_VALUE* EmitterRows = Required(
			Root, "emitters", DATA_JSON_TYPE::ARRAY);
		if (nullptr == EmitterRows ||
			EmitterRows->Get_Array().size() != Program.Emitters.size())
		{
			return false;
		}
		uint32_t TotalBurstCount = 0u;
		uint32_t FollowAttachmentCount = 0u;
		uint32_t SnapshotRootAttachmentCount = 0u;
		uint32_t DisabledRootAttachmentCount = 0u;
		uint32_t OperationalParticleTotal = 0u;
		DATA_JSON_VALUE::ARRAY ActionCueAuthorityProjection;
		DATA_JSON_VALUE::ARRAY RendererConfigAuthorityProjection;
		ActionCueAuthorityProjection.reserve(Program.Emitters.size());
		RendererConfigAuthorityProjection.reserve(Program.Emitters.size());
		for (size_t EmitterIndex = 0u;
			EmitterIndex < Program.Emitters.size(); ++EmitterIndex)
		{
			const std::string EmitterFailurePrefix =
				"emitter-timing-renderer-reverse:emitter:" +
				std::to_string(EmitterIndex) + ":";
			const EFFECT_RUNTIME_PROGRAM_EMITTER& Row =
				Program.Emitters[EmitterIndex];
			const DATA_JSON_VALUE& RawEmitter =
				EmitterRows->Get_Array()[EmitterIndex];
			const DATA_JSON_VALUE* RawRenderer = Required(
				RawEmitter, "rendererRuntimeConfig", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* RawTiming = Required(
				RawEmitter, "timing", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* RawRandom = Required(
				RawEmitter, "random", DATA_JSON_TYPE::OBJECT);
			if (nullptr == RawRenderer || nullptr == RawTiming || nullptr == RawRandom)
			{
				OutFailure = EmitterFailurePrefix + "raw-nested";
				return false;
			}
			DATA_JSON_VALUE ActionCueAuthorityRow;
			DATA_JSON_VALUE RendererConfigAuthorityRow;
			if (!Build_ObjectProjection(RawEmitter, {
					"emitterId", "sourceActionCueProjectionSha256" },
					ActionCueAuthorityRow) ||
				!Build_ObjectProjection(RawEmitter, {
					"emitterId", "rendererRuntimeConfig" },
					RendererConfigAuthorityRow))
			{
				OutFailure = EmitterFailurePrefix + "authority-projection";
				return false;
			}
			ActionCueAuthorityProjection.push_back(
				std::move(ActionCueAuthorityRow));
			RendererConfigAuthorityProjection.push_back(
				std::move(RendererConfigAuthorityRow));
			DATA_JSON_VALUE RendererNested;
			DATA_JSON_VALUE TimingCore;
			DATA_JSON_VALUE TimingUnsigned;
			DATA_JSON_VALUE RandomUnsigned;
			if (!Build_ObjectProjection(*RawRenderer, {
					"color", "uv", "linearLerp", "mesh", "sprite", "decal",
					"trail", "afterImage", "screenPost" }, RendererNested) ||
				!Build_ObjectProjection(*RawTiming, {
					"requiredModuleId", "spawnModuleId", "lifetimeModuleId",
					"emitterDelaySeconds", "emitterDelayPolicy",
					"emitterDurationSeconds", "emitterDurationPolicy",
					"emitterLoopCount", "bursts" }, TimingCore) ||
				!Build_ObjectWithoutKey(*RawTiming, "timingSha256", TimingUnsigned) ||
				!Build_ObjectWithoutKey(*RawRandom, "policySha256", RandomUnsigned))
			{
				OutFailure = EmitterFailurePrefix + "nested-projection";
				return false;
			}
			DATA_JSON_VALUE::OBJECT RendererProjectionObject =
				RendererNested.Get_Object();
			const DATA_JSON_VALUE* SourceNode = RawEmitter.Find("sourceNode");
			const DATA_JSON_VALUE* RendererType = RawEmitter.Find("rendererType");
			if (nullptr == SourceNode || nullptr == RendererType)
			{
				OutFailure = EmitterFailurePrefix + "renderer-source";
				return false;
			}
			RendererProjectionObject.emplace("sourceNode", *SourceNode);
			RendererProjectionObject.emplace("rendererType", *RendererType);
			const DATA_JSON_VALUE RendererProjection = DATA_JSON_VALUE::Object(
				std::move(RendererProjectionObject));
			const DATA_JSON_VALUE* RawEmitterId = RawEmitter.Find("emitterId");
			const DATA_JSON_VALUE* RawSourcePeak =
				RawEmitter.Find("sourcePeakActiveParticles");
			const DATA_JSON_VALUE* RawOperationalMaximum =
				RawEmitter.Find("operationalMaxParticles");
			const DATA_JSON_VALUE* RawOperationalPolicy =
				RawEmitter.Find("operationalCapPolicy");
			if (nullptr == RawEmitterId || nullptr == RawSourcePeak ||
				nullptr == RawOperationalMaximum || nullptr == RawOperationalPolicy)
			{
				OutFailure = EmitterFailurePrefix + "operational-source";
				return false;
			}
			DATA_JSON_VALUE::OBJECT CapProjectionObject;
			CapProjectionObject.emplace("emitterId", *RawEmitterId);
			CapProjectionObject.emplace("sourcePeakActiveParticles", *RawSourcePeak);
			CapProjectionObject.emplace(
				"operationalMaxParticles", *RawOperationalMaximum);
			CapProjectionObject.emplace("policy", *RawOperationalPolicy);
			const DATA_JSON_VALUE CapProjection = DATA_JSON_VALUE::Object(
				std::move(CapProjectionObject));
			DATA_JSON_VALUE::OBJECT CueProjectionObject;
			for (const std::string_view Field : {
					"sourceCueId", "sourceOccurrenceId", "cueLocalTransform",
					"actionCueAttachment", "actionCueParameterInputs" })
			{
				const DATA_JSON_VALUE* Value = RawEmitter.Find(Field);
				if (nullptr == Value)
				{
					OutFailure = EmitterFailurePrefix + "action-cue-source";
					return false;
				}
				CueProjectionObject.emplace(std::string(Field), *Value);
			}
			const DATA_JSON_VALUE* RawSystemId = RawEmitter.Find("sourceSystemId");
			if (nullptr == RawSystemId || !RawSystemId->Is_String())
			{
				OutFailure = EmitterFailurePrefix + "source-system";
				return false;
			}
			CueProjectionObject.emplace("sourceSystemId",
				DATA_JSON_VALUE::String(Fold_AsciiLower(RawSystemId->Get_String())));
			const DATA_JSON_VALUE CueProjection = DATA_JSON_VALUE::Object(
				std::move(CueProjectionObject));
			DATA_JSON_VALUE::OBJECT RandomDerivationObject;
			RandomDerivationObject.emplace("policyId",
				DATA_JSON_VALUE::String(Row.Random.strPolicyId));
			RandomDerivationObject.emplace("sourceCandidateCanonicalSha256",
				DATA_JSON_VALUE::String(
					SourceCandidateArtifact->strCanonicalJsonSha256));
			RandomDerivationObject.emplace("emitterId",
				DATA_JSON_VALUE::String(Row.Row.strId));
			const DATA_JSON_VALUE RandomDerivation = DATA_JSON_VALUE::Object(
				std::move(RandomDerivationObject));
			const std::string ExpectedRandomDerivationSha =
				CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
					CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
						RandomDerivation));
			uint32_t DerivedSeed = 0u;
			const std::string SeedPrefix =
				Row.Random.strSeedDerivationInputSha256.substr(0u, 8u);
			const auto SeedParse = std::from_chars(
				SeedPrefix.data(), SeedPrefix.data() + SeedPrefix.size(),
				DerivedSeed, 16);
			if (Row.RendererRuntimeConfig.strSourceProjectionSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							RendererProjection)) ||
				Row.Timing.strSourceProjectionSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(TimingCore)) ||
				Row.Timing.strTimingSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							TimingUnsigned)) ||
				Row.Random.strPolicySha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							RandomUnsigned)) ||
				Row.Random.strSeedDerivationInputSha256 !=
					ExpectedRandomDerivationSha ||
				SeedParse.ec != std::errc{} ||
				SeedParse.ptr != SeedPrefix.data() + SeedPrefix.size() ||
				Row.Random.iEmitterRandomSeed != DerivedSeed ||
				Row.strOperationalCapProjectionSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							CapProjection)) ||
				Row.strSourceActionCueProjectionSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							CueProjection)))
			{
				OutFailure = EmitterFailurePrefix + "authority-digest";
				return false;
			}
			TotalBurstCount += static_cast<uint32_t>(Row.Timing.Bursts.size());
			OperationalParticleTotal += Row.iOperationalMaxParticles;
			if (Row.ActionCueAttachment.bEnabled)
			{
				if (Row.ActionCueAttachment.bFollow)
				{
					if (Row.AnchorRequests.size() != 1u ||
						Row.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees != 0.0)
					{
						OutFailure = EmitterFailurePrefix + "attachment-cardinality";
						return false;
					}
					const auto& Anchor = Row.AnchorRequests.front();
					if (Anchor.strSourceAnchorSlotId !=
							Row.ActionCueAttachment.strSourceAnchorSlotId ||
						Anchor.strRuntimeAnchorSlotId !=
							Row.ActionCueAttachment.strRuntimeAnchorSlotId ||
						Anchor.strRuntimeBoneName !=
							Row.ActionCueAttachment.strRuntimeBoneName ||
						Anchor.SocketLocalTransform.vPosition !=
							Row.ActionCueAttachment.SocketLocalTransform.vPosition ||
						Anchor.SocketLocalTransform.vRotationDegrees !=
							Row.ActionCueAttachment.SocketLocalTransform.vRotationDegrees ||
						Anchor.SocketLocalTransform.vScale !=
							Row.ActionCueAttachment.SocketLocalTransform.vScale ||
						!Anchor.bFollow)
					{
						OutFailure = EmitterFailurePrefix + "attachment-reverse";
						return false;
					}
					++FollowAttachmentCount;
				}
				else
				{
					if (Row.ActionCueAttachment.strSourceAnchorSlotId != "root" ||
						Row.ActionCueAttachment.strRuntimeAnchorSlotId != "root" ||
						!Row.ActionCueAttachment.strRuntimeBoneName.empty() ||
						Row.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees != -90.0 ||
						!Row.AnchorRequests.empty())
					{
						OutFailure = EmitterFailurePrefix + "snapshot-root-attachment";
						return false;
					}
					++SnapshotRootAttachmentCount;
				}
			}
			else
			{
				if (Row.ActionCueAttachment.bFollow ||
					!Row.ActionCueAttachment.strSourceAnchorSlotId.empty() ||
					Row.ActionCueAttachment.strRuntimeAnchorSlotId != "root" ||
					!Row.ActionCueAttachment.strRuntimeBoneName.empty() ||
					Row.ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees != 0.0 ||
					Row.ActionCueAttachment.SocketLocalTransform.vPosition !=
						std::array<double, 3u>{} ||
					Row.ActionCueAttachment.SocketLocalTransform.vRotationDegrees !=
						std::array<double, 3u>{} ||
					Row.ActionCueAttachment.SocketLocalTransform.vScale !=
						std::array<double, 3u>{ 1.0, 1.0, 1.0 } ||
					!Row.AnchorRequests.empty())
				{
					OutFailure = EmitterFailurePrefix + "disabled-root-attachment";
					return false;
				}
				++DisabledRootAttachmentCount;
			}
			const auto Schedule = Schedules.find(Row.strScheduleId);
			const auto RequiredModule = Modules.find(Row.Timing.strRequiredModuleId);
			const auto SpawnModule = Modules.find(Row.Timing.strSpawnModuleId);
			const auto LifetimeModule = Modules.find(Row.Timing.strLifetimeModuleId);
			if (Schedule == Schedules.end() ||
				Schedule->second->strSourceCueId != Row.strSourceCueId ||
				Schedule->second->strSourceOccurrenceId != Row.strSourceOccurrenceId ||
				Schedule->second->strSourceSystemId != Fold_AsciiLower(Row.strSourceSystemId) ||
				Row.ModuleIds != ModuleIdsByEmitter[Row.Row.strId] ||
				RequiredModule == Modules.end() || SpawnModule == Modules.end() ||
				LifetimeModule == Modules.end() ||
				RequiredModule->second->strEmitterId != Row.Row.strId ||
				RequiredModule->second->strExactSourceClass != "particlemodulerequired" ||
				SpawnModule->second->strEmitterId != Row.Row.strId ||
				SpawnModule->second->strExactSourceClass != "particlemodulespawn" ||
				LifetimeModule->second->strEmitterId != Row.Row.strId ||
				(LifetimeModule->second->strExactSourceClass != "particlemodulelifetime" &&
				 LifetimeModule->second->strExactSourceClass !=
					"particlemodulelifetime_seeded"))
			{
				OutFailure = EmitterFailurePrefix + "schedule-module-reverse";
				return false;
			}
			const size_t LifetimeDistributionCount = static_cast<size_t>(std::count_if(
				Program.Distributions.begin(), Program.Distributions.end(),
				[&Row](const EFFECT_RUNTIME_PROGRAM_DISTRIBUTION& Distribution)
				{
					return Distribution.strModuleId == Row.Timing.strLifetimeModuleId &&
						Distribution.strPropertyPath == "lifetime";
				}));
			if (LifetimeDistributionCount != 1u)
			{
				OutFailure = EmitterFailurePrefix + "lifetime-distribution";
				return false;
			}
			std::unordered_map<std::string, const EFFECT_RUNTIME_PROGRAM_LITERAL*>
				RequiredLiterals;
			std::unordered_map<std::string, const EFFECT_RUNTIME_PROGRAM_LITERAL*>
				SpawnLiterals;
			for (const EFFECT_RUNTIME_PROGRAM_LITERAL& Literal : Program.Literals)
			{
				if (Literal.strModuleId == Row.Timing.strRequiredModuleId &&
					!RequiredLiterals.emplace(
						Literal.strPropertyPath, &Literal).second)
				{
					return false;
				}
				if (Literal.strModuleId == Row.Timing.strSpawnModuleId &&
					Literal.strPropertyPath.starts_with("burstlist[") &&
					!SpawnLiterals.emplace(Literal.strPropertyPath, &Literal).second)
				{
					return false;
				}
			}
			const auto Loops = RequiredLiterals.find("emitterloops");
			if (Loops == RequiredLiterals.end() ||
				Loops->second->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
				!Loops->second->fValue.has_value() ||
				std::trunc(*Loops->second->fValue) != *Loops->second->fValue ||
				*Loops->second->fValue < 0.0 ||
				*Loops->second->fValue > static_cast<double>(UINT32_MAX) ||
				Row.Timing.iEmitterLoopCount !=
					static_cast<uint32_t>(*Loops->second->fValue))
			{
				OutFailure = EmitterFailurePrefix + "loop-literal";
				return false;
			}
			const auto ValidateRequiredTime = [&RequiredLiterals](
				const std::string_view PropertyPath,
				const double RuntimeValue,
				const bool_t ExplicitPolicy,
				const double DefaultValue) -> bool_t
			{
				const auto Literal = RequiredLiterals.find(std::string(PropertyPath));
				if (Literal == RequiredLiterals.end())
					return !ExplicitPolicy && RuntimeValue == DefaultValue;
				return ExplicitPolicy &&
					Literal->second->eVariant == EFFECT_RUNTIME_LITERAL_VARIANT::F64 &&
					Literal->second->fValue.has_value() &&
					RuntimeValue == *Literal->second->fValue;
			};
			if (!ValidateRequiredTime("emitterdelay",
					Row.Timing.fEmitterDelaySeconds,
					Row.Timing.eEmitterDelayPolicy ==
						EFFECT_RUNTIME_EMITTER_DELAY_POLICY::EXPLICIT_REQUIRED_LITERAL,
					0.0) ||
				!ValidateRequiredTime("emitterduration",
					Row.Timing.fEmitterDurationSeconds,
					Row.Timing.eEmitterDurationPolicy ==
						EFFECT_RUNTIME_EMITTER_DURATION_POLICY::EXPLICIT_REQUIRED_LITERAL,
					1.0))
			{
				OutFailure = EmitterFailurePrefix + "required-time";
				return false;
			}
			std::set<uint32_t> BurstIndices;
			for (const auto& [Path, Literal] : SpawnLiterals)
			{
				const size_t Close = Path.find(']', 10u);
				if (Close == std::string::npos || Close == 10u)
					return false;
				uint32_t BurstIndex = 0u;
				const auto ParsedIndex = std::from_chars(
					Path.data() + 10u, Path.data() + Close, BurstIndex, 10);
				if (ParsedIndex.ec != std::errc{} ||
					ParsedIndex.ptr != Path.data() + Close)
				{
					return false;
				}
				BurstIndices.emplace(BurstIndex);
			}
			for (uint32_t ExpectedIndex = 0u;
				ExpectedIndex < BurstIndices.size(); ++ExpectedIndex)
			{
				if (!BurstIndices.contains(ExpectedIndex))
				{
					OutFailure = EmitterFailurePrefix + "burst-index";
					return false;
				}
			}
			std::vector<EFFECT_RUNTIME_PROGRAM_BURST> ExpectedBursts;
			for (const uint32_t BurstIndex : BurstIndices)
			{
				const std::string Prefix =
					"burstlist[" + std::to_string(BurstIndex) + "]";
				std::array<const EFFECT_RUNTIME_PROGRAM_LITERAL*, 3u> Triplet{};
				for (size_t FieldIndex = 0u; FieldIndex < Triplet.size(); ++FieldIndex)
				{
					const std::array<std::string_view, 3u> Suffixes{
						".count", ".countlow", ".time" };
					const auto Literal = SpawnLiterals.find(
						Prefix + std::string(Suffixes[FieldIndex]));
					if (Literal == SpawnLiterals.end() ||
						Literal->second->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
						!Literal->second->fValue.has_value())
					{
						OutFailure = EmitterFailurePrefix + "burst-triplet";
						return false;
					}
					Triplet[FieldIndex] = Literal->second;
				}
				const double Count = *Triplet[0]->fValue;
				const double CountLow = *Triplet[1]->fValue;
				const double BurstTime = *Triplet[2]->fValue;
				if (std::trunc(Count) != Count || std::trunc(CountLow) != CountLow ||
					Count < 0.0 || Count > static_cast<double>(UINT32_MAX) ||
					CountLow > static_cast<double>(UINT32_MAX) || BurstTime < 0.0)
				{
					OutFailure = EmitterFailurePrefix + "burst-range";
					return false;
				}
				const uint32_t Maximum = static_cast<uint32_t>(Count);
				const uint32_t Minimum = CountLow < 0.0 ? Maximum :
					static_cast<uint32_t>(CountLow);
				if (Minimum > Maximum)
				{
					OutFailure = EmitterFailurePrefix + "burst-minmax";
					return false;
				}
				if (Maximum == 0u)
					continue;
				DATA_JSON_VALUE::ARRAY LiteralProjection;
				for (const EFFECT_RUNTIME_PROGRAM_LITERAL* Literal : Triplet)
				{
					DATA_JSON_VALUE::OBJECT LiteralObject;
					LiteralObject.emplace("propertyPath",
						DATA_JSON_VALUE::String(Literal->strPropertyPath));
					LiteralObject.emplace("variant", DATA_JSON_VALUE::String("F64"));
					LiteralObject.emplace("f64Value",
						DATA_JSON_VALUE::Number(*Literal->fValue, true));
					LiteralProjection.push_back(
						DATA_JSON_VALUE::Object(std::move(LiteralObject)));
				}
				EFFECT_RUNTIME_PROGRAM_BURST Expected;
				Expected.strSpawnModuleId = Row.Timing.strSpawnModuleId;
				Expected.iBurstIndex = BurstIndex;
				Expected.fTimeSeconds = BurstTime;
				Expected.iCountMinimum = Minimum;
				Expected.iCountMaximum = Maximum;
				Expected.strSourceLiteralProjectionSha256 =
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							DATA_JSON_VALUE::Array(std::move(LiteralProjection))));
				ExpectedBursts.push_back(std::move(Expected));
			}
			if (ExpectedBursts.size() != Row.Timing.Bursts.size())
			{
				OutFailure = EmitterFailurePrefix + "burst-count";
				return false;
			}
			for (size_t BurstIndex = 0u; BurstIndex < ExpectedBursts.size(); ++BurstIndex)
			{
				const auto& Expected = ExpectedBursts[BurstIndex];
				const auto& Actual = Row.Timing.Bursts[BurstIndex];
				if (Expected.strSpawnModuleId != Actual.strSpawnModuleId ||
					Expected.iBurstIndex != Actual.iBurstIndex ||
					Expected.fTimeSeconds != Actual.fTimeSeconds ||
					Expected.iCountMinimum != Actual.iCountMinimum ||
					Expected.iCountMaximum != Actual.iCountMaximum ||
					Expected.strSourceLiteralProjectionSha256 !=
						Actual.strSourceLiteralProjectionSha256)
				{
					OutFailure = EmitterFailurePrefix + "burst-reverse";
					return false;
				}
			}
			++ScheduleCoverage.at(Row.strScheduleId);

			if (Row.ScreenPostAdapter.has_value())
			{
				const auto Intensity = Distributions.find(
					Row.ScreenPostAdapter->strIntensityDistributionId);
				const auto Alpha = Distributions.find(
					Row.ScreenPostAdapter->strAlphaDistributionId);
				if (Row.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST ||
					Intensity == Distributions.end() || Alpha == Distributions.end() ||
					!Modules.contains(Intensity->second->strModuleId) ||
					!Modules.contains(Alpha->second->strModuleId) ||
					Modules.at(Intensity->second->strModuleId)->strEmitterId !=
						Row.Row.strId ||
					Modules.at(Alpha->second->strModuleId)->strEmitterId != Row.Row.strId)
				{
					OutFailure = EmitterFailurePrefix + "screen-post-adapter";
					return false;
				}
			}
			if (Row.LightAdapter.has_value())
			{
				const auto Module = Modules.find(Row.LightAdapter->strModuleId);
				std::vector<std::string> OwnedFields;
				for (const auto& Field : Program.PointLightFields)
				{
					if (Field.strModuleId == Row.LightAdapter->strModuleId)
						OwnedFields.push_back(Field.Row.strId);
				}
				if (Row.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE ||
					Module == Modules.end() || Module->second->strEmitterId != Row.Row.strId ||
					Module->second->strExactSourceClass !=
						"efparticlemoduletypedatalight" ||
					Row.LightAdapter->FieldIds != OwnedFields)
				{
					OutFailure = EmitterFailurePrefix + "light-adapter";
					return false;
				}
			}
			if (Row.DecalAdapter.has_value())
			{
				const auto& Adapter = *Row.DecalAdapter;
				const auto Module = Modules.find(Adapter.strModuleId);
				const auto Default = Defaults.find(Adapter.strDefaultId);
				const auto Size = Distributions.find(Adapter.strSizeDistributionId);
				const auto SizeModule = Size == Distributions.end() ? Modules.end() :
					Modules.find(Size->second->strModuleId);
				const auto NearPlaneLiteral = std::find_if(
					Program.Literals.begin(), Program.Literals.end(),
					[&Adapter](const EFFECT_RUNTIME_PROGRAM_LITERAL& Literal)
					{
						return Literal.strModuleId == Adapter.strModuleId &&
							Literal.strPropertyPath == "nearplane";
					});
				if (Row.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE ||
					Module == Modules.end() || Module->second->strEmitterId != Row.Row.strId ||
					Module->second->strExactSourceClass !=
						"efparticlemoduletypedatadecal" ||
					Module->second->strHandlerVariant != "EF_DECAL_DESCRIPTOR" ||
					Default == Defaults.end() ||
					Default->second->strModuleId != Module->second->Row.strId ||
					Default->second->eVariant !=
						EFFECT_RUNTIME_IMPLICIT_DEFAULT_VARIANT::DECAL_DEFAULTS ||
					!Default->second->DecalValues.has_value() ||
					Size == Distributions.end() ||
					Size->second->strPropertyPath != "startsize" ||
					SizeModule == Modules.end() ||
					SizeModule->second->strEmitterId != Row.Row.strId ||
					SizeModule->second->strExactSourceClass != "particlemodulesize" ||
					NearPlaneLiteral == Program.Literals.end() ||
					NearPlaneLiteral->eVariant != EFFECT_RUNTIME_LITERAL_VARIANT::F64 ||
					!NearPlaneLiteral->fValue.has_value() ||
					Adapter.fNearPlane != *NearPlaneLiteral->fValue ||
					Adapter.fFarPlane != Default->second->DecalValues->fFarPlane ||
					Adapter.vDefaultSize != Default->second->DecalValues->vDefaultSize ||
					Adapter.vBlendRange != Default->second->DecalValues->vBlendRange ||
					Adapter.bYawOnlyCdoDefault !=
						Default->second->DecalValues->bOnlyCalculateRotationYaw ||
					Adapter.bSupports3dDrawMode !=
						Default->second->DecalValues->bSupports3dDrawMode ||
					Adapter.fDepthWorldUnits != Adapter.fFarPlane - Adapter.fNearPlane ||
					Adapter.fDepthRuntimeUnits != Adapter.fDepthWorldUnits * 0.01)
				{
					OutFailure = EmitterFailurePrefix + "decal-adapter";
					return false;
				}
			}
			if (Row.RibbonAdapter.has_value())
			{
				const auto& Adapter = *Row.RibbonAdapter;
				const auto TypeData = Modules.find(Adapter.strTypeDataModuleId);
				const auto Default = Defaults.find(Adapter.strDefaultId);
				if (Row.eRenderer != EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON ||
					TypeData == Modules.end() ||
					TypeData->second->strEmitterId != Row.Row.strId ||
					Default == Defaults.end() ||
					Default->second->strModuleId != Adapter.strTypeDataModuleId)
				{
					OutFailure = EmitterFailurePrefix + "ribbon-adapter-owner";
					return false;
				}
				const auto RequireModule = [&Modules, &Row](
					const std::string& Id) -> bool_t
				{
					const auto Found = Modules.find(Id);
					return Found != Modules.end() &&
						Found->second->strEmitterId == Row.Row.strId;
				};
				if (!RequireModule(Adapter.strSpawnModuleId) ||
					!RequireModule(Adapter.strLifetimeModuleId) ||
					!RequireModule(Adapter.strSizeModuleId) ||
					!RequireModule(Adapter.strDynamicParameterModuleId) ||
					std::any_of(Adapter.ColorModuleIds.begin(), Adapter.ColorModuleIds.end(),
						[&RequireModule](const std::string& Id)
						{
							return !RequireModule(Id);
						}))
				{
					return false;
				}
				const auto RequireDistribution = [&Distributions, &Modules, &Row](
					const std::string& Id) -> bool_t
				{
					const auto Distribution = Distributions.find(Id);
					return Distribution != Distributions.end() &&
						Modules.contains(Distribution->second->strModuleId) &&
						Modules.at(Distribution->second->strModuleId)->strEmitterId ==
							Row.Row.strId;
				};
				for (const auto* Ids : {
						&Adapter.SpawnDistributionIds, &Adapter.LifetimeDistributionIds,
						&Adapter.SizeDistributionIds, &Adapter.ColorDistributionIds,
						&Adapter.DynamicDistributionIds })
				{
					if (std::any_of(Ids->begin(), Ids->end(),
						[&RequireDistribution](const std::string& Id)
						{
							return !RequireDistribution(Id);
						}))
					{
						OutFailure = EmitterFailurePrefix + "ribbon-adapter-distribution";
						return false;
					}
				}
			}
		}
		if (!VerifyProjection(std::move(ActionCueAuthorityProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.ActionCueProjectionsSha256) ||
			!VerifyProjection(std::move(RendererConfigAuthorityProjection),
				ACTIVE_RECONSTRUCTED_PROFILE.RendererConfigProjectionsSha256) ||
			std::any_of(ScheduleCoverage.begin(), ScheduleCoverage.end(),
				[](const auto& Entry) { return Entry.second == 0u; }) ||
			TotalBurstCount != 31u ||
			FollowAttachmentCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iFollowAttachmentCount ||
			SnapshotRootAttachmentCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iSnapshotRootAttachmentCount ||
			DisabledRootAttachmentCount !=
				ACTIVE_RECONSTRUCTED_PROFILE.iDisabledRootAttachmentCount ||
			OperationalParticleTotal != 1291u)
		{
			OutFailure = "emitter-timing-renderer-reverse:aggregate";
			return false;
		}
		for (const EFFECT_RUNTIME_PROGRAM_MODULE& Row : Program.Modules)
		{
			if (Row.PropertyIds != PropertiesByModule[Row.Row.strId] ||
				Row.PrimitiveLeafIds != LeavesByModule[Row.Row.strId] ||
				Row.LiteralIds != LiteralsByModule[Row.Row.strId] ||
				Row.DistributionIds != DistributionsByModule[Row.Row.strId] ||
				Row.ImplicitDefaultIds != DefaultsByModule[Row.Row.strId])
			{
				return false;
			}
		}

		OutFailure = "material-recipe-reverse";
		std::unordered_map<std::string, std::vector<std::string>> InputsByRecipe;
		std::unordered_map<std::string, std::vector<std::string>> StaticByRecipe;
		std::unordered_map<std::string, std::vector<std::string>> RenderByRecipe;
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE& Row : Program.MaterialInputs)
		{
			if (!Recipes.contains(Row.strRecipeId))
				return false;
			InputsByRecipe[Row.strRecipeId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE& Row :
			Program.MaterialStaticBindings)
		{
			if (!Recipes.contains(Row.strRecipeId) ||
				!Policies.contains(Row.strPolicyRowId))
			{
				return false;
			}
			StaticByRecipe[Row.strRecipeId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_VALUE& Row :
			Program.MaterialRenderBindings)
		{
			if (!Recipes.contains(Row.strRecipeId) ||
				(!Row.strPolicyRowId.empty() && !Policies.contains(Row.strPolicyRowId)))
			{
				return false;
			}
			RenderByRecipe[Row.strRecipeId].push_back(Row.Row.strId);
		}
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_FAMILY& Row :
			Program.MaterialFamilies)
		{
			const auto Handler = Handlers.find(Row.strEvaluatorRegistryId);
			if (Handler == Handlers.end() ||
				Handler->second->eKind !=
					EFFECT_RUNTIME_HANDLER_KIND::MATERIAL_EVALUATOR ||
				Handler->second->strImplementationId != Row.strEvaluatorId ||
				Handler->second->iImplementationVersion != Row.iEvaluatorVersion ||
				Handler->second->strImplementationSha256 != Row.strEvaluatorSha256 ||
				Handler->second->strVariant != Row.Row.strId ||
				Handler->second->strConsumerContract !=
					"ARTIST_F_COMMON_MATERIAL_EVALUATOR_V1")
			{
				return false;
			}
		}
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_RECIPE& Row : Program.MaterialRecipes)
		{
			const auto Family = Families.find(Row.strFamilyId);
			if (Family == Families.end() ||
				!Handlers.contains(Row.strEvaluatorRegistryId) ||
				Row.strEvaluatorRegistryId != Family->second->strEvaluatorRegistryId ||
				Row.InputIds != InputsByRecipe[Row.Row.strId] ||
				Row.StaticBindingIds != StaticByRecipe[Row.Row.strId] ||
				Row.RenderBindingIds != RenderByRecipe[Row.Row.strId])
			{
				return false;
			}
		}
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Row :
			Program.MaterialOccurrences)
		{
			const auto Recipe = Recipes.find(Row.strRecipeId);
			const auto Emitter = Emitters.find(Row.strEmitterId);
			if (Emitter == Emitters.end() || Recipe == Recipes.end() ||
				Row.strFamilyId != Recipe->second->strFamilyId ||
				Row.strEvaluatorRegistryId != Recipe->second->strEvaluatorRegistryId ||
				Row.strBindingSha256 != Recipe->second->strBindingSha256 ||
				Row.eRenderer != Emitter->second->eRenderer)
			{
				return false;
			}
		}
		const auto EqualStencilFace = [](const auto& Left, const auto& Right)
		{
			return Left.iStencilFailOp == Right.iStencilFailOp &&
				Left.iStencilDepthFailOp == Right.iStencilDepthFailOp &&
				Left.iStencilPassOp == Right.iStencilPassOp &&
				Left.iStencilFunc == Right.iStencilFunc;
		};
		const auto EqualDepthStencil = [&EqualStencilFace](
			const auto& Left, const auto& Right)
		{
			return Left.bDepthEnable == Right.bDepthEnable &&
				Left.iDepthWriteMask == Right.iDepthWriteMask &&
				Left.iDepthFunc == Right.iDepthFunc &&
				Left.bStencilEnable == Right.bStencilEnable &&
				Left.iStencilReadMask == Right.iStencilReadMask &&
				Left.iStencilWriteMask == Right.iStencilWriteMask &&
				EqualStencilFace(Left.FrontFace, Right.FrontFace) &&
				EqualStencilFace(Left.BackFace, Right.BackFace);
		};
		const auto EqualRasterizer = [](const auto& Left, const auto& Right)
		{
			return Left.iFillMode == Right.iFillMode &&
				Left.iCullMode == Right.iCullMode &&
				Left.bFrontCounterClockwise == Right.bFrontCounterClockwise &&
				Left.iDepthBias == Right.iDepthBias &&
				Left.fDepthBiasClamp == Right.fDepthBiasClamp &&
				Left.fSlopeScaledDepthBias == Right.fSlopeScaledDepthBias &&
				Left.bDepthClipEnable == Right.bDepthClipEnable &&
				Left.bScissorEnable == Right.bScissorEnable &&
				Left.bMultisampleEnable == Right.bMultisampleEnable &&
				Left.bAntialiasedLineEnable == Right.bAntialiasedLineEnable;
		};
		const auto EqualD3dSampler = [](const auto& Left, const auto& Right)
		{
			return Left.iFilter == Right.iFilter &&
				Left.iAddressU == Right.iAddressU &&
				Left.iAddressV == Right.iAddressV &&
				Left.iAddressW == Right.iAddressW &&
				Left.fMipLodBias == Right.fMipLodBias &&
				Left.iMaxAnisotropy == Right.iMaxAnisotropy &&
				Left.iComparisonFunc == Right.iComparisonFunc &&
				Left.vBorderColor == Right.vBorderColor &&
				Left.fMinLod == Right.fMinLod && Left.fMaxLod == Right.fMaxLod;
		};
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY& Row : Program.MaterialPolicies)
		{
			const auto Handler = Handlers.find(Row.strImplementationRegistryId);
			const auto Recipe = Recipes.find(Row.strRecipeId);
			const std::string ExpectedVariant = Row.eDomain ==
				EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE ?
					"RENDER_STATE" : Row.eDomain ==
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::STATIC_PERMUTATION ?
						"STATIC_PERMUTATION" : "SAMPLER_DESCRIPTOR";
			if (Recipe == Recipes.end() || Handler == Handlers.end() ||
				Handler->second->eKind != EFFECT_RUNTIME_HANDLER_KIND::MATERIAL_POLICY ||
				Handler->second->strImplementationId != Row.strImplementationId ||
				Handler->second->iImplementationVersion != Row.iImplementationVersion ||
				Handler->second->strVariant != ExpectedVariant ||
				Handler->second->strConsumerContract != Row.strConsumerContract ||
				Row.strEvidenceOwnerRecipeId != Row.strRecipeId)
			{
				return false;
			}
			std::vector<std::string> ExpectedOccurrences;
			for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_OCCURRENCE& Occurrence :
				Program.MaterialOccurrences)
			{
				if (Occurrence.strRecipeId == Row.strRecipeId)
					ExpectedOccurrences.push_back(Occurrence.Row.strId);
			}
			std::sort(ExpectedOccurrences.begin(), ExpectedOccurrences.end());
			if (Row.MaterialOccurrenceIds != ExpectedOccurrences)
				return false;
			for (const std::string& OccurrenceId : Row.MaterialOccurrenceIds)
			{
				const auto Occurrence = Occurrences.find(OccurrenceId);
				if (Occurrence == Occurrences.end() ||
					Occurrence->second->strRecipeId != Row.strRecipeId)
				{
					return false;
				}
			}

			if (Row.eDomain == EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::RENDER_STATE)
			{
				const auto Matches = static_cast<size_t>(std::count_if(
					Program.MaterialRenderBindings.begin(),
					Program.MaterialRenderBindings.end(), [&Row](const auto& Binding)
					{
						return Binding.strRecipeId == Row.strRecipeId &&
							Row.strFieldId == Row.strRecipeId + ":" +
								Binding.strFieldName &&
							Binding.strPolicyRowId == Row.Row.strId;
					}));
				if (Matches != 1u)
					return false;
			}
			else if (Row.eDomain ==
				EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::STATIC_PERMUTATION)
			{
				const auto Matches = static_cast<size_t>(std::count_if(
					Program.MaterialStaticBindings.begin(),
					Program.MaterialStaticBindings.end(), [&Row](const auto& Binding)
					{
						return Binding.strRecipeId == Row.strRecipeId &&
							Binding.Row.strId == Row.strFieldId &&
							Binding.strPolicyRowId == Row.Row.strId;
					}));
				if (Matches != 1u)
					return false;
			}
			else
			{
				const auto Matches = static_cast<size_t>(std::count_if(
					Program.MaterialInputs.begin(), Program.MaterialInputs.end(),
					[&Row](const auto& Input)
					{
						return Input.strRecipeId == Row.strRecipeId &&
							Input.Row.strId == Row.strFieldId &&
							Input.eVariant ==
								EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID;
					}));
				if (Matches != 1u)
					return false;
			}

			if (Row.D3dDescriptorOracle.has_value())
			{
				const auto& Oracle = *Row.D3dDescriptorOracle;
				bool_t Equal = false;
				if (Oracle.eKind == EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::DEPTH_STENCIL &&
					Oracle.Expected.DepthStencil.has_value() &&
					Oracle.Actual.DepthStencil.has_value())
				{
					Equal = EqualDepthStencil(*Oracle.Expected.DepthStencil,
						*Oracle.Actual.DepthStencil);
				}
				else if (Oracle.eKind ==
						EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::RASTERIZER &&
					Oracle.Expected.Rasterizer.has_value() &&
					Oracle.Actual.Rasterizer.has_value())
				{
					Equal = EqualRasterizer(*Oracle.Expected.Rasterizer,
						*Oracle.Actual.Rasterizer);
				}
				else if (Oracle.eKind ==
						EFFECT_RUNTIME_D3D_DESCRIPTOR_KIND::SAMPLER &&
					Oracle.Expected.Sampler.has_value() &&
					Oracle.Actual.Sampler.has_value())
				{
					Equal = EqualD3dSampler(*Oracle.Expected.Sampler,
						*Oracle.Actual.Sampler);
				}
				if (!Equal)
					return false;
			}
			if (Row.D3dSrvOracle.has_value())
			{
				const auto& Expected = Row.D3dSrvOracle->Expected;
				const auto& Actual = Row.D3dSrvOracle->Actual;
				if (Expected.iFormat != Actual.iFormat ||
					Expected.iViewDimension != Actual.iViewDimension ||
					Expected.iMostDetailedMip != Actual.iMostDetailedMip ||
					Expected.iMipLevels != Actual.iMipLevels ||
					Expected.strColorSpace != Actual.strColorSpace)
				{
					return false;
				}
			}
			if (Row.SamplerDescriptor.has_value())
			{
				if (!Row.D3dDescriptorOracle.has_value() ||
					!Row.D3dDescriptorOracle->Expected.Sampler.has_value() ||
					!Row.D3dSrvOracle.has_value())
				{
					return false;
				}
				const auto& Source = *Row.SamplerDescriptor;
				const auto& D3d = *Row.D3dDescriptorOracle->Expected.Sampler;
				if (Source.iFilterD3d11 != D3d.iFilter ||
					Source.iAddressUD3d11 != D3d.iAddressU ||
					Source.iAddressVD3d11 != D3d.iAddressV ||
					Source.iAddressWD3d11 != D3d.iAddressW ||
					Source.fMipLodBias != D3d.fMipLodBias ||
					Source.iMaxAnisotropy != D3d.iMaxAnisotropy ||
					Source.iComparisonFuncD3d11 != D3d.iComparisonFunc ||
					Source.vBorderColor != D3d.vBorderColor ||
					Source.fMinLod != D3d.fMinLod ||
					Source.fMaxLod != D3d.fMaxLod ||
					Source.strSrvColorSpace !=
						Row.D3dSrvOracle->Expected.strColorSpace)
				{
					return false;
				}
			}
		}

		OutFailure = "material-texture-reverse";
		std::unordered_set<std::string> TextureInputCoverage;
		std::unordered_set<std::string> TexturePolicyCoverage;
		std::unordered_set<std::string> SourceBindingIds;
		std::unordered_set<std::string> ProvisioningProposalIds;
		std::unordered_map<std::string, std::tuple<std::string, std::string,
			std::optional<std::string>>> SourceResourceIdentity;
		for (const EFFECT_RUNTIME_PROGRAM_TEXTURE_BINDING& Row :
			Program.MaterialTextureBindings)
		{
			const auto Input = Inputs.find(Row.strMaterialInputFieldId);
			const auto Policy = Policies.find(Row.strSamplerPolicyRowId);
			if (!Recipes.contains(Row.strRecipeId) || Input == Inputs.end() ||
				Policy == Policies.end() || Input->second->strRecipeId != Row.strRecipeId ||
				Input->second->eVariant !=
					EFFECT_RUNTIME_MATERIAL_VALUE_VARIANT::TEXTURE_ID ||
				Input->second->strStringValue != Row.strLogicalTexturePath ||
				Policy->second->eDomain !=
					EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::SAMPLER_DESCRIPTOR ||
				Policy->second->strRecipeId != Row.strRecipeId ||
				Policy->second->strFieldId != Row.strMaterialInputFieldId ||
				Policy->second->MaterialOccurrenceIds != Row.MaterialOccurrenceIds ||
				Row.Row.strId != Row.strMaterialInputFieldId +
					"::runtime-texture-binding" ||
				!SourceBindingIds.emplace(Row.strSourceBindingId).second ||
				!TextureInputCoverage.emplace(Row.strMaterialInputFieldId).second ||
				!TexturePolicyCoverage.emplace(Row.strSamplerPolicyRowId).second)
			{
				return false;
			}
			if (!Row.strSourceProvisioningProposalId.empty() &&
				!ProvisioningProposalIds.emplace(
					Row.strSourceProvisioningProposalId).second)
			{
				return false;
			}
			const auto ResourceIdentity = std::make_tuple(
				Row.strSourceTextureResourceRowSha256,
				Row.strLogicalTexturePath, Row.strRuntimeAssetId);
			const auto [Resource, Inserted] = SourceResourceIdentity.emplace(
				Row.strSourceTextureResourceId, ResourceIdentity);
			if (!Inserted && Resource->second != ResourceIdentity)
				return false;
		}
		std::unordered_set<std::string> ExpectedTextureInputs;
		std::unordered_set<std::string> ExpectedTexturePolicies;
		for (const EFFECT_RUNTIME_PROGRAM_MATERIAL_POLICY& Policy :
			Program.MaterialPolicies)
		{
			if (Policy.eDomain ==
				EFFECT_RUNTIME_MATERIAL_POLICY_DOMAIN::SAMPLER_DESCRIPTOR)
			{
				ExpectedTextureInputs.emplace(Policy.strFieldId);
				ExpectedTexturePolicies.emplace(Policy.Row.strId);
			}
		}
		if (TextureInputCoverage != ExpectedTextureInputs ||
			TexturePolicyCoverage != ExpectedTexturePolicies ||
			SourceBindingIds.size() != Program.MaterialTextureBindings.size())
		{
			return false;
		}

		OutFailure = "renderer-texture-reverse";
		std::unordered_map<std::string, std::vector<std::string>> TexturesByEmitter;
		std::set<std::pair<std::string, std::string>> EmitterSlots;
		std::unordered_map<std::string, uint32_t> RendererSlotCounts;
		const DATA_JSON_VALUE* RendererRows = Required(
			Root, "rendererTextureResources", DATA_JSON_TYPE::ARRAY);
		if (nullptr == RendererRows || RendererRows->Get_Array().size() !=
			Program.RendererTextureResources.size())
		{
			return false;
		}
		for (size_t Index = 0u;
			Index < Program.RendererTextureResources.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_RENDERER_TEXTURE& Row =
				Program.RendererTextureResources[Index];
			const auto Emitter = Emitters.find(Row.strEmitterId);
			const auto Occurrence = Occurrences.find(Row.strMaterialOccurrenceId);
			DATA_JSON_VALUE SourceProjection;
			if (Emitter == Emitters.end() || Occurrence == Occurrences.end() ||
				Occurrence->second->strEmitterId != Row.strEmitterId ||
				Emitter->second->eRenderer == EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE ||
				Row.strSourceNode != Emitter->second->strSourceNode ||
				!Recipes.contains(Occurrence->second->strRecipeId) ||
				Row.strSourceMaterialPath !=
					Recipes.at(Occurrence->second->strRecipeId)->strSourceMaterialPath ||
				Row.Row.strId != Row.strEmitterId +
					"::renderer-texture:" + Row.strSlotId ||
				!Build_ObjectProjection(RendererRows->Get_Array()[Index], {
						"sourceNode", "sourceMaterialPath", "slotId", "assetId" },
					SourceProjection) ||
				Row.strSourceResourceProjectionSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							SourceProjection)) ||
				!EmitterSlots.emplace(Row.strEmitterId, Row.strSlotId).second)
			{
				return false;
			}
			TexturesByEmitter[Row.strEmitterId].push_back(Row.Row.strId);
			++RendererSlotCounts[Row.strSlotId];
		}
		if (RendererSlotCounts != std::unordered_map<std::string, uint32_t>{
				{ "base", 19u }, { "dissolve", 5u }, { "emissive", 3u },
				{ "mask", 15u }, { "noise", 15u } })
		{
			return false;
		}
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Row : Program.Emitters)
		{
			const bool_t Light =
				Row.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE;
			const bool_t Mesh =
				Row.eRenderer == EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE;
			if (Row.TextureResourceIds != TexturesByEmitter[Row.Row.strId] ||
				(Light == Row.strMaterialOccurrenceId.has_value()) ||
				(!Light && (!Occurrences.contains(*Row.strMaterialOccurrenceId) ||
					Occurrences.at(*Row.strMaterialOccurrenceId)->strEmitterId !=
						Row.Row.strId)) ||
				(Mesh != Row.strGeometryUseId.has_value()) ||
				(Mesh && (!GeometryUses.contains(*Row.strGeometryUseId) ||
					GeometryUses.at(*Row.strGeometryUseId)->strEmitterId != Row.Row.strId)))
			{
				return false;
			}
		}

		OutFailure = "geometry-reverse";
		std::unordered_set<std::string> UsedCarriers;
		std::unordered_set<std::string> GeometryEmitterCoverage;
		for (const EFFECT_RUNTIME_PROGRAM_GEOMETRY_USE& Row : Program.GeometryUses)
		{
			const auto Emitter = Emitters.find(Row.strEmitterId);
			const auto Module = Modules.find(Row.strModuleId);
			const auto Carrier = Carriers.find(Row.strCarrierId);
			if (Emitter == Emitters.end() || Module == Modules.end() ||
				Carrier == Carriers.end() || Module->second->strEmitterId != Row.strEmitterId ||
				Module->second->strExactSourceClass != "particlemoduletypedatamesh" ||
				Emitter->second->eRenderer != EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE ||
				Emitter->second->strSourceEmitterPath != Row.strSourceEmitterPath ||
				Carrier->second->strAssetId != Row.strAssetId ||
				Row.Row.strId != Row.strEmitterId + "::geometry-use" ||
				!GeometryEmitterCoverage.emplace(Row.strEmitterId).second)
			{
				return false;
			}
			UsedCarriers.emplace(Row.strCarrierId);
		}
		if (UsedCarriers.size() != Program.GeometryCarriers.size())
			return false;

		const DATA_JSON_VALUE* CarrierRows = Required(
			Root, "geometryCarriers", DATA_JSON_TYPE::ARRAY);
		if (nullptr == CarrierRows || CarrierRows->Get_Array().size() !=
			Program.GeometryCarriers.size())
		{
			return false;
		}
		for (size_t Index = 0u; Index < Program.GeometryCarriers.size(); ++Index)
		{
			const EFFECT_RUNTIME_PROGRAM_GEOMETRY_CARRIER& Carrier =
				Program.GeometryCarriers[Index];
			const DATA_JSON_VALUE& Row = CarrierRows->Get_Array()[Index];
			DATA_JSON_VALUE ExpectedTuple;
			DATA_JSON_VALUE CachePayload;
			if (!Build_ObjectProjection(Row, {
					"formatVersion", "channelMask", "evidenceFlags",
					"geometryPreScale", "geometryPreScaleF32Hex", "payloadSha256",
					"provenanceSha256", "provenanceRole", "metadataIdentitySha256",
					"submeshes" }, ExpectedTuple) ||
				!Build_ObjectProjection(Row, {
					"assetId", "payloadSha256", "provenanceSha256",
					"geometryPreScaleF32Hex", "channelMask", "evidenceFlags",
					"submeshes" }, CachePayload) ||
				Carrier.strExpectedTupleSha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							ExpectedTuple)) ||
				Carrier.strCacheIdentitySha256 !=
					CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
						CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
							CachePayload)) ||
				Carrier.strMetadataIdentitySha256 != Carrier.strProvenanceSha256)
			{
				return false;
			}
		}

		OutFailure = "property-payload-reverse";
		const DATA_JSON_VALUE* PropertyRows = Required(
			Root, "properties", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Leaves = Required(
			Root, "primitiveLeaves", DATA_JSON_TYPE::ARRAY);
		if (nullptr == PropertyRows || nullptr == Leaves ||
			PropertyRows->Get_Array().size() != Program.Properties.size() ||
			Leaves->Get_Array().size() != Program.PrimitiveLeaves.size())
			return false;
		for (size_t Index = 0u; Index < PropertyRows->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = PropertyRows->Get_Array()[Index];
			const EFFECT_RUNTIME_PROGRAM_PROPERTY& Property =
				Program.Properties[Index];
			const DATA_JSON_VALUE* PayloadLiterals = Required(
				Row, "payloadLiteralIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* PayloadDistributions = Required(
				Row, "payloadDistributionIds", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* SemanticDistributions = Required(
				Row, "semanticDistributionIds", DATA_JSON_TYPE::ARRAY);
			std::vector<std::string> LiteralIds;
			std::vector<std::string> PayloadIds;
			std::vector<std::string> SemanticIds;
			if (nullptr == PayloadLiterals || nullptr == PayloadDistributions ||
				nullptr == SemanticDistributions ||
				!Parse_StringArray(*PayloadLiterals, LiteralIds, false, 4096u, 1024u) ||
				!Parse_StringArray(*PayloadDistributions, PayloadIds, false, 4096u, 1024u) ||
				!Parse_StringArray(*SemanticDistributions, SemanticIds,
					false, 4096u, 1024u))
			{
				return false;
			}
			for (const std::string& Id : LiteralIds)
			{
				const auto Literal = Literals.find(Id);
				if (Literal == Literals.end() ||
					Literal->second->strModuleId != Property.strModuleId ||
					Literal->second->strPropertyId != Property.Row.strId)
				{
					return false;
				}
			}
			for (const std::string& Id : PayloadIds)
			{
				const bool_t Exists = std::any_of(
					Program.Distributions.begin(), Program.Distributions.end(),
					[&Id, &Property](const auto& Distribution)
					{
						return Distribution.strPayloadDistributionId == Id &&
							Distribution.strPropertyId == Property.Row.strId &&
							Distribution.strModuleId == Property.strModuleId;
					});
				if (!Exists)
					return false;
			}
			for (const std::string& Id : SemanticIds)
			{
				const auto Distribution = Distributions.find(Id);
				if (Distribution == Distributions.end() ||
					Distribution->second->strPropertyId != Property.Row.strId ||
					Distribution->second->strModuleId != Property.strModuleId)
				{
					return false;
				}
			}
		}
		for (size_t Index = 0u; Index < Leaves->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Leaves->Get_Array()[Index];
			std::string ModuleId;
			std::string PropertyId;
			std::string LiteralId;
			std::string HandlerId;
			std::string PropertyPath;
			const auto& ParsedLeaf = Program.PrimitiveLeaves[Index];
			if (!Read_RuntimeId(Row, "moduleId", ModuleId) ||
				!Read_RuntimeId(Row, "propertyId", PropertyId) ||
				!Read_RuntimeId(Row, "literalId", LiteralId) ||
				!Read_RuntimeId(Row, "handlerRegistryId", HandlerId) ||
				!Read_String(Row, "propertyPath", PropertyPath, false, 4096u) ||
				!Properties.contains(PropertyId) || !Literals.contains(LiteralId) ||
				!Modules.contains(ModuleId) || !Handlers.contains(HandlerId) ||
				ParsedLeaf.strModuleId != ModuleId ||
				ParsedLeaf.strPropertyId != PropertyId ||
				ParsedLeaf.strLiteralId != LiteralId ||
				ParsedLeaf.strHandlerRegistryId != HandlerId ||
				ParsedLeaf.strPropertyPath != PropertyPath ||
				Properties.at(PropertyId)->strModuleId != ModuleId ||
				Literals.at(LiteralId)->strModuleId != ModuleId ||
				Literals.at(LiteralId)->strPropertyId != PropertyId ||
				ParsedLeaf.strTopLevelPropertyPath !=
					Properties.at(PropertyId)->strPropertyPath ||
				ParsedLeaf.strPropertyPath != Literals.at(LiteralId)->strPropertyPath ||
				ParsedLeaf.eValueVariant != Literals.at(LiteralId)->eVariant ||
				Handlers.at(HandlerId)->eKind !=
					EFFECT_RUNTIME_HANDLER_KIND::SOURCE_PRIMITIVE ||
				Handlers.at(HandlerId)->strExactSourceClass !=
					Modules.at(ModuleId)->strExactSourceClass ||
				Handlers.at(HandlerId)->strVariant !=
					ParsedLeaf.strTopLevelPropertyPath)
			{
				return false;
			}
		}
		OutFailure.clear();
		return true;
	}

	bool_t Validate_ReconstructedSummary(
		const DATA_JSON_VALUE& Root,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program)
	{
		const DATA_JSON_VALUE* Summary = Required(
			Root, "summary", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Summary)
			return false;
		const auto CountEquals = [Summary](
			const std::string_view Name, const size_t Expected) -> bool_t
		{
			uint32_t Value = 0u;
			return Expected <= static_cast<size_t>(UINT32_MAX) &&
				Read_U32(*Summary, Name, Value) && Value == Expected;
		};
		if (!CountEquals("inputArtifactCount", Program.InputArtifacts.size()) ||
			!CountEquals("handlerRegistryCount", Program.Handlers.size()) ||
			!CountEquals("emitterCount", Program.Emitters.size()) ||
			!CountEquals("actionScheduleCount", Program.ActionSchedules.size()) ||
			!CountEquals("moduleCount", Program.Modules.size()) ||
			!CountEquals("propertyCount", Program.Properties.size()) ||
			!CountEquals("primitiveLeafCount", Program.PrimitiveLeaves.size()) ||
			!CountEquals("literalCount", Program.Literals.size()) ||
			!CountEquals("distributionCount", Program.Distributions.size()) ||
			!CountEquals("seedPolicyCount", Program.SeedPolicies.size()) ||
			!CountEquals("implicitDefaultCount", Program.ImplicitDefaults.size()) ||
			!CountEquals("pointLightFieldCount", Program.PointLightFields.size()) ||
			!CountEquals("materialFamilyCount", Program.MaterialFamilies.size()) ||
			!CountEquals("materialRecipeCount", Program.MaterialRecipes.size()) ||
			!CountEquals("materialInputCount", Program.MaterialInputs.size()) ||
			!CountEquals(
				"materialStaticBindingCount", Program.MaterialStaticBindings.size()) ||
			!CountEquals(
				"materialRenderBindingCount", Program.MaterialRenderBindings.size()) ||
			!CountEquals(
				"materialOccurrenceCount", Program.MaterialOccurrences.size()) ||
			!CountEquals("materialPolicyCount", Program.MaterialPolicies.size()) ||
			!CountEquals("materialTextureBindingCount",
				Program.MaterialTextureBindings.size()) ||
			!CountEquals("resolvedMaterialTextureBindingCount",
				ACTIVE_RECONSTRUCTED_PROFILE.iResolvedTextureCount) ||
			!CountEquals("rendererTextureResourceCount",
				Program.RendererTextureResources.size()) ||
			!CountEquals("geometryCarrierCount", Program.GeometryCarriers.size()) ||
			!CountEquals("geometryUseCount", Program.GeometryUses.size()) ||
			!CountEquals("unknownHandlerCount", 0u) ||
			!CountEquals("ownerlessRowCount", 0u) ||
			!CountEquals("duplicateIdCount", 0u) ||
			!CountEquals("sourceExactRowCount", 0u) ||
			!Read_ExpectedBoolean(*Summary, "runtimeExecution", false) ||
			!Read_ExpectedBoolean(*Summary, "product", false))
		{
			return false;
		}
		uint32_t D3dDescriptorCount = 0u;
		uint32_t SrvDescriptorCount = 0u;
		std::string D3dDescriptorSha256;
		std::string SrvDescriptorSha256;
		if (!Read_U32(*Summary, "materialD3dDescriptorCount", D3dDescriptorCount) ||
			D3dDescriptorCount != 112u ||
			!Read_Sha(*Summary, "materialD3dDescriptorProjectionSha256",
				D3dDescriptorSha256) ||
			D3dDescriptorSha256 !=
				"8eec266215ab6df354babe26e192d6e0c1fd4f3dbe0631c4ef7c5ffc939f7a9c" ||
			!Read_U32(*Summary, "materialSrvDescriptorCount", SrvDescriptorCount) ||
			SrvDescriptorCount != 77u ||
			!Read_Sha(*Summary, "materialSrvDescriptorProjectionSha256",
				SrvDescriptorSha256) ||
			SrvDescriptorSha256 !=
				"ca2dcf3e7d1dbc407103eeb213b0c3e5f04fca002ae2fe8f269fc77dd5a01c67")
		{
			return false;
		}

		const auto ValidateCountMap = [Summary](
			const std::string_view Name,
			const std::initializer_list<std::pair<std::string_view, uint32_t>> Values)
			-> bool_t
		{
			const DATA_JSON_VALUE* Map = Required(*Summary, Name, DATA_JSON_TYPE::OBJECT);
			if (nullptr == Map || Map->Get_Object().size() != Values.size() ||
				Map->Get_ObjectInsertionOrder().size() != Values.size())
			{
				return false;
			}
			size_t Index = 0u;
			for (const auto& [Key, Expected] : Values)
			{
				uint32_t Actual = 0u;
				if (Map->Get_ObjectInsertionOrder()[Index++] != Key ||
					!Read_U32(*Map, Key, Actual) || Actual != Expected)
				{
					return false;
				}
			}
			return true;
		};
		return ValidateCountMap("moduleSelectionCounts", {
				{ "SOURCE_HANDLER", 370u }, { "RECONSTRUCTED_HANDLER", 29u } }) &&
			ValidateCountMap("distributionVariantCounts", {
				{ "INLINE", 612u }, { "FLOAT_PARAMETER", 8u },
				{ "VECTOR_PARAMETER", 5u }, { "FLOAT_CURVE", 1u },
				{ "EF_MULTIPLY", 3u } }) &&
			ValidateCountMap("materialPolicyDomainCounts", {
				{ "RENDER_STATE", 89u }, { "STATIC_PERMUTATION", 94u },
				{ "SAMPLER_DESCRIPTOR", 77u } }) &&
			ValidateCountMap("rendererTextureSlotCounts", {
				{ "base", 19u }, { "dissolve", 5u }, { "emissive", 3u },
				{ "mask", 15u }, { "noise", 15u } });
	}

	bool_t Parse_ReconstructedAdmission(
		const DATA_JSON_VALUE& Root,
		EFFECT_RUNTIME_PROGRAM_ADMISSION& OutAdmission)
	{
		const DATA_JSON_VALUE* Admission = Required(
			Root, "admission", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Blockers = nullptr;
		return nullptr != Admission &&
			Read_Boolean(*Admission, "artifactBinding", OutAdmission.bArtifactBinding) &&
			Read_Boolean(*Admission, "policyRoute", OutAdmission.bPolicyRoute) &&
			Read_Boolean(*Admission, "sourceHandlerSelection",
				OutAdmission.bSourceHandlerSelection) &&
			Read_Boolean(*Admission, "distributionEvaluatorSelection",
				OutAdmission.bDistributionEvaluatorSelection) &&
			Read_Boolean(*Admission, "materialPolicySelection",
				OutAdmission.bMaterialPolicySelection) &&
			Read_Boolean(*Admission, "geometryBinding", OutAdmission.bGeometryBinding) &&
			Read_Boolean(*Admission, "sourceExact", OutAdmission.bSourceExact) &&
			Read_Boolean(*Admission, "runtimeExecution",
				OutAdmission.bRuntimeExecution) &&
			Read_Boolean(*Admission, "product", OutAdmission.bProduct) &&
			nullptr != (Blockers = Required(
				*Admission, "blockers", DATA_JSON_TYPE::ARRAY)) &&
			Parse_Blockers(*Blockers, OutAdmission.Blockers);
	}

	bool_t Parse_ExecutionContract(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_EXECUTION_CONTRACT& OutContract)
	{
		if (!Has_ExactKeys(Value, {
			"artifactBindingBlockerSet", "artifactBindingBlockerCount",
			"executionBlockerSet", "executionBlockerCount",
			"executionAdmission" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* ArtifactBlockers = Required(
			Value, "artifactBindingBlockerSet", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* ExecutionBlockers = Required(
			Value, "executionBlockerSet", DATA_JSON_TYPE::ARRAY);
		uint32_t ArtifactCount = 0u;
		uint32_t ExecutionCount = 0u;
		if (nullptr == ArtifactBlockers || nullptr == ExecutionBlockers ||
			!Parse_Blockers(*ArtifactBlockers, OutContract.ArtifactBindingBlockers) ||
			!Parse_Blockers(*ExecutionBlockers, OutContract.ExecutionBlockers) ||
			!Read_U32(Value, "artifactBindingBlockerCount", ArtifactCount) ||
			!Read_U32(Value, "executionBlockerCount", ExecutionCount) ||
			ArtifactCount != OutContract.ArtifactBindingBlockers.size() ||
			ExecutionCount != OutContract.ExecutionBlockers.size())
		{
			return false;
		}
		const bool_t ExpectedAdmission =
			OutContract.ArtifactBindingBlockers.empty() &&
			OutContract.ExecutionBlockers.empty();
		if (!Read_ExpectedBoolean(
			Value, "executionAdmission", ExpectedAdmission))
		{
			return false;
		}
		OutContract.bExecutionAdmission = ExpectedAdmission;
		return true;
	}

	bool_t Equal(
		const EFFECT_RUNTIME_EXECUTION_CONTRACT& Left,
		const EFFECT_RUNTIME_EXECUTION_CONTRACT& Right)
	{
		return Left.ArtifactBindingBlockers == Right.ArtifactBindingBlockers &&
			Left.ExecutionBlockers == Right.ExecutionBlockers &&
			Left.bExecutionAdmission == Right.bExecutionAdmission;
	}

	EFFECT_RUNTIME_EXECUTION_CONTRACT Combine(
		const std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT>& Receipts)
	{
		std::set<std::string> Artifact;
		std::set<std::string> Execution;
		for (const EFFECT_RUNTIME_HANDLER_RECEIPT& Receipt : Receipts)
		{
			Artifact.insert(
				Receipt.ExecutionContract.ArtifactBindingBlockers.begin(),
				Receipt.ExecutionContract.ArtifactBindingBlockers.end());
			Execution.insert(
				Receipt.ExecutionContract.ExecutionBlockers.begin(),
				Receipt.ExecutionContract.ExecutionBlockers.end());
		}
		EFFECT_RUNTIME_EXECUTION_CONTRACT Result;
		Result.ArtifactBindingBlockers.assign(Artifact.begin(), Artifact.end());
		Result.ExecutionBlockers.assign(Execution.begin(), Execution.end());
		Result.bExecutionAdmission = Artifact.empty() && Execution.empty();
		return Result;
	}

	bool_t Parse_DerivedIdentity(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_DERIVED_IDENTITY& OutIdentity)
	{
		const DATA_JSON_VALUE* Schema = Required(
			Value, "schema", DATA_JSON_TYPE::STRING);
		return Has_ExactKeys(Value, {
				"schema", "formatVersion", "sourceContractHash",
				"sourceSemanticClosureHash", "geometryContractHash",
				"materialContractHash", "resourceBindingHash",
				"compilerInputHash" }) &&
			nullptr != Schema &&
			Schema->Get_String() == "lostark.effect-derived-identity" &&
			Read_ExactVersion(Value, "formatVersion", 1u) &&
			Read_Sha(Value, "sourceContractHash",
				OutIdentity.strSourceContractHash) &&
			Read_Sha(Value, "sourceSemanticClosureHash",
				OutIdentity.strSourceSemanticClosureHash) &&
			Read_Sha(Value, "geometryContractHash",
				OutIdentity.strGeometryContractHash) &&
			Read_Sha(Value, "materialContractHash",
				OutIdentity.strMaterialContractHash) &&
			Read_Sha(Value, "resourceBindingHash",
				OutIdentity.strResourceBindingHash) &&
			Read_Sha(Value, "compilerInputHash",
				OutIdentity.strCompilerInputHash);
	}

	bool_t Equal(
		const EFFECT_RUNTIME_DERIVED_IDENTITY& Left,
		const EFFECT_RUNTIME_DERIVED_IDENTITY& Right)
	{
		return Left.strSourceContractHash == Right.strSourceContractHash &&
			Left.strSourceSemanticClosureHash ==
				Right.strSourceSemanticClosureHash &&
			Left.strGeometryContractHash == Right.strGeometryContractHash &&
			Left.strMaterialContractHash == Right.strMaterialContractHash &&
			Left.strResourceBindingHash == Right.strResourceBindingHash &&
			Left.strCompilerInputHash == Right.strCompilerInputHash;
	}

	bool_t Contains_ReservedExecutionField(const DATA_JSON_VALUE& Value)
	{
		if (Value.Is_Array())
		{
			return std::any_of(Value.Get_Array().begin(), Value.Get_Array().end(),
				Contains_ReservedExecutionField);
		}
		if (!Value.Is_Object())
			return false;
		for (const auto& [Key, Child] : Value.Get_Object())
		{
			if (Key == "artifactBindingBlockerSet" ||
				Key == "artifactBindingBlockerCount" ||
				Key == "executionBlockerSet" ||
				Key == "executionBlockerCount" ||
				Key == "executionAdmission" || Key == "blockers" ||
				Contains_ReservedExecutionField(Child))
			{
				return true;
			}
		}
		return false;
	}

	bool_t Parse_HandlerReceipts(
		const DATA_JSON_VALUE& Value,
		std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT>& OutReceipts)
	{
		if (!Value.Is_Array())
			return false;
		std::set<std::string> Ids;
		std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT> Staged;
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Has_ExactKeys(Item, {
				"handlerId", "handlerSha256", "executionContract" }))
			{
				return false;
			}
			EFFECT_RUNTIME_HANDLER_RECEIPT Receipt;
			const DATA_JSON_VALUE* Contract = Required(
				Item, "executionContract", DATA_JSON_TYPE::OBJECT);
			if (!Read_StableId(Item, "handlerId", Receipt.strHandlerId) ||
				!Read_Sha(Item, "handlerSha256", Receipt.strHandlerSha256) ||
				nullptr == Contract ||
				!Parse_ExecutionContract(*Contract, Receipt.ExecutionContract) ||
				!Ids.insert(Receipt.strHandlerId).second)
			{
				return false;
			}
			Staged.push_back(std::move(Receipt));
		}
		OutReceipts = std::move(Staged);
		return true;
	}

	void Serialize_Canonical(
		const DATA_JSON_VALUE& Value,
		std::string& Out)
	{
		switch (Value.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE:
			Out += "null";
			break;
		case DATA_JSON_TYPE::BOOLEAN:
			Out += Value.Get_Boolean() ? "true" : "false";
			break;
		case DATA_JSON_TYPE::NUMBER:
		{
			const double Number = Value.Get_Number();
			if (Number == std::floor(Number) &&
				Number >= -9007199254740991.0 &&
				Number <= 9007199254740991.0)
			{
				if (Value.Was_FloatingPointToken())
				{
					if (0.0 == Number && std::signbit(Number))
						Out += "-0.0";
					else
					{
						char Buffer[64]{};
						const auto Result = std::to_chars(
							Buffer, std::end(Buffer), Number);
						Out.append(Buffer, Result.ptr);
						if (Out.find_first_of(".eE", Out.size() -
							static_cast<size_t>(Result.ptr - Buffer)) ==
							std::string::npos)
						{
							Out += ".0";
						}
					}
				}
				else
				{
					char Buffer[64]{};
					const auto Result = std::to_chars(
						Buffer, std::end(Buffer), static_cast<int64_t>(Number));
					Out.append(Buffer, Result.ptr);
				}
			}
			else
			{
				char Buffer[64]{};
				const auto Result = std::to_chars(
					Buffer, std::end(Buffer), Number);
				Out.append(Buffer, Result.ptr);
			}
			break;
		}
		case DATA_JSON_TYPE::STRING:
			Out.push_back('"');
			Out += CDataJson::Escape(Value.Get_String());
			Out.push_back('"');
			break;
		case DATA_JSON_TYPE::ARRAY:
			Out.push_back('[');
			for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
			{
				if (0u != Index)
					Out.push_back(',');
				Serialize_Canonical(Value.Get_Array()[Index], Out);
			}
			Out.push_back(']');
			break;
		case DATA_JSON_TYPE::OBJECT:
			Out.push_back('{');
			{
				size_t Index = 0u;
				for (const auto& [Key, Child] : Value.Get_Object())
				{
					if (0u != Index++)
						Out.push_back(',');
					Out.push_back('"');
					Out += CDataJson::Escape(Key);
					Out += "\":";
					Serialize_Canonical(Child, Out);
				}
			}
			Out.push_back('}');
			break;
		}
	}

	void Serialize_Pretty(
		const DATA_JSON_VALUE& Value,
		const uint32_t Indent,
		std::string& Out)
	{
		if (!Value.Is_Array() && !Value.Is_Object())
		{
			Serialize_Canonical(Value, Out);
			return;
		}
		if (Value.Is_Array())
		{
			if (Value.Get_Array().empty())
			{
				Out += "[]";
				return;
			}
			Out += "[\n";
			for (size_t Index = 0u; Index < Value.Get_Array().size(); ++Index)
			{
				Out.append(static_cast<size_t>(Indent + 1u) * 2u, ' ');
				Serialize_Pretty(Value.Get_Array()[Index], Indent + 1u, Out);
				Out += Index + 1u == Value.Get_Array().size() ? "\n" : ",\n";
			}
			Out.append(static_cast<size_t>(Indent) * 2u, ' ');
			Out.push_back(']');
			return;
		}
		if (Value.Get_Object().empty())
		{
			Out += "{}";
			return;
		}
		const std::vector<std::string>& Order =
			Value.Get_ObjectInsertionOrder();
		Out += "{\n";
		for (size_t Index = 0u; Index < Order.size(); ++Index)
		{
			const auto Item = Value.Get_Object().find(Order[Index]);
			if (Item == Value.Get_Object().end())
				continue;
			Out.append(static_cast<size_t>(Indent + 1u) * 2u, ' ');
			Out.push_back('"');
			Out += CDataJson::Escape(Item->first);
			Out += "\": ";
			Serialize_Pretty(Item->second, Indent + 1u, Out);
			Out += Index + 1u == Order.size() ? "\n" : ",\n";
		}
		Out.append(static_cast<size_t>(Indent) * 2u, ' ');
		Out.push_back('}');
	}

	bool_t Parse_ReconstructedRuntimeProgramInternal(
		const std::string_view Utf8Json,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY& ExpectedIdentity,
		const bool_t bRequireFrozenCandidateIdentity,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>&
			OutValidatedProgram,
		std::string& strOutError)
	{
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY Frozen{
			std::string(ACTIVE_RECONSTRUCTED_PROFILE.BuilderCommitId),
			std::string(ACTIVE_RECONSTRUCTED_PROFILE.BuilderTreeId),
			std::string(ACTIVE_RECONSTRUCTED_PROFILE.CandidateRawSha256),
			std::string(RECONSTRUCTED_PROGRAM_ID),
			1u,
			std::string(ACTIVE_RECONSTRUCTED_PROFILE.ProgramSha256),
		};
		if (ExpectedIdentity.strBuilderAuthorityCommitId !=
				Frozen.strBuilderAuthorityCommitId ||
			ExpectedIdentity.strBuilderAuthorityTreeId !=
				Frozen.strBuilderAuthorityTreeId ||
			ExpectedIdentity.strProgramId != Frozen.strProgramId ||
			ExpectedIdentity.iProgramVersion != Frozen.iProgramVersion ||
			!Is_Sha256(ExpectedIdentity.strCandidateRawSha256) ||
			!Is_Sha256(ExpectedIdentity.strProgramSha256) ||
			(bRequireFrozenCandidateIdentity &&
				(ExpectedIdentity.strCandidateRawSha256 !=
					Frozen.strCandidateRawSha256 ||
				 ExpectedIdentity.strProgramSha256 != Frozen.strProgramSha256)))
		{
			strOutError = bRequireFrozenCandidateIdentity ?
				"Reconstructed runtime program expected identity is not frozen." :
				"Reconstructed runtime semantic test identity is invalid.";
			return false;
		}
		if (bRequireFrozenCandidateIdentity &&
			Utf8Json.size() != ACTIVE_RECONSTRUCTED_PROFILE.iCandidateByteCount)
		{
			strOutError = "Reconstructed runtime program byte count mismatch.";
			return false;
		}
		const std::string RawSha256 =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(Utf8Json);
		if (RawSha256.empty() ||
			RawSha256 != ExpectedIdentity.strCandidateRawSha256)
		{
			strOutError = "Reconstructed runtime program raw SHA mismatch.";
			return false;
		}

		DATA_JSON_VALUE Root;
		if (!Parse_BoundedRuntimeProgramJson(Utf8Json, Root, strOutError))
			return false;

		auto Staged = std::make_shared<EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>();
		std::string ProgramSha256;
		if (!Validate_RuntimeProgramRootSeals(
				Root, ProgramSha256, Staged->BlockerOwnership) ||
			ProgramSha256 != ExpectedIdentity.strProgramSha256)
		{
			strOutError =
				"Reconstructed runtime program root/row/section seal mismatch.";
			return false;
		}
		if (!Parse_ReconstructedRootContracts(Root, *Staged))
		{
			strOutError = "Reconstructed runtime program root contract mismatch.";
			return false;
		}
		if (!Parse_ReconstructedCoreRows(Root, *Staged))
		{
			strOutError = "Reconstructed runtime program core row parse failed.";
			return false;
		}
		std::string ValueRowFailure;
		if (!Parse_ReconstructedValueRows(Root, *Staged, ValueRowFailure))
		{
			strOutError = "Reconstructed runtime program value row parse failed: " +
				ValueRowFailure;
			return false;
		}
		std::string MaterialRowFailure;
		if (!Parse_ReconstructedMaterialRows(Root, *Staged, MaterialRowFailure))
		{
			strOutError = "Reconstructed runtime program Material row parse failed: " +
				MaterialRowFailure;
			return false;
		}
		std::string MaterialPolicyFailure;
		if (!Parse_ReconstructedMaterialPolicyRows(
				Root, *Staged, MaterialPolicyFailure))
		{
			strOutError =
				"Reconstructed runtime program Material policy parse failed: " +
				MaterialPolicyFailure;
			return false;
		}
		std::string TextureRowFailure;
		if (!Parse_ReconstructedTextureRows(Root, *Staged, TextureRowFailure))
		{
			strOutError = "Reconstructed runtime program texture row parse failed: " +
				TextureRowFailure;
			return false;
		}
		std::string GeometryRowFailure;
		if (!Parse_ReconstructedGeometryRows(Root, *Staged, GeometryRowFailure))
		{
			strOutError = "Reconstructed runtime program Geometry row parse failed: " +
				GeometryRowFailure;
			return false;
		}
		std::string GraphFailure;
		if (!Validate_ReconstructedGraph(Root, *Staged, GraphFailure))
		{
			strOutError = "Reconstructed runtime program ownership graph mismatch: " +
				GraphFailure;
			return false;
		}
		if (!Validate_ReconstructedSummary(Root, *Staged))
		{
			strOutError = "Reconstructed runtime program summary mismatch.";
			return false;
		}
		if (!Parse_ReconstructedAdmission(Root, Staged->Admission))
		{
			strOutError = "Reconstructed runtime program admission parse failed.";
			return false;
		}
		if (!Staged->Admission.bArtifactBinding || !Staged->Admission.bPolicyRoute ||
			!Staged->Admission.bSourceHandlerSelection ||
			!Staged->Admission.bDistributionEvaluatorSelection ||
			!Staged->Admission.bMaterialPolicySelection ||
			!Staged->Admission.bGeometryBinding || Staged->Admission.bSourceExact ||
			Staged->Admission.bRuntimeExecution || Staged->Admission.bProduct)
		{
			strOutError = "Reconstructed runtime program admission state is invalid.";
			return false;
		}

		Staged->Identity = ExpectedIdentity;
		OutValidatedProgram = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Validate_ToolDependencies(const DATA_JSON_VALUE& Value)
	{
		if (!Value.Is_Array() || Value.Get_Array().size() != 3u)
			return false;
		std::set<std::string> Roles;
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Has_ExactKeys(Item, {
				"role", "path", "rawSha256", "canonicalSha256",
				"hashDomain", "verificationRole" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* Role = Required(
				Item, "role", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Path = Required(
				Item, "path", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Domain = Required(
				Item, "hashDomain", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Verification = Required(
				Item, "verificationRole", DATA_JSON_TYPE::STRING);
			std::string Raw;
			std::string Canonical;
			if (nullptr == Role || !Is_StableId(Role->Get_String()) ||
				nullptr == Path || Path->Get_String().empty() ||
				nullptr == Domain || Domain->Get_String().empty() ||
				nullptr == Verification || Verification->Get_String() !=
					"CANONICAL_REQUIRED_RAW_OBSERVED" ||
				!Read_Sha(Item, "rawSha256", Raw) ||
				!Read_Sha(Item, "canonicalSha256", Canonical) ||
				!Roles.insert(Role->Get_String()).second)
			{
				return false;
			}
		}
		return true;
	}
}

std::string Client::CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
	const DATA_JSON_VALUE& Value)
{
	std::string Result;
	Serialize_Canonical(Value, Result);
	return Result;
}

std::string Client::CEffectRuntimeAuthorityCodec::Serialize_PrettyJson(
	const DATA_JSON_VALUE& Value)
{
	std::string Result;
	Serialize_Pretty(Value, 0u, Result);
	Result.push_back('\n');
	return Result;
}

std::string Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
	const std::string_view Value)
{
	BCRYPT_ALG_HANDLE Algorithm = nullptr;
	BCRYPT_HASH_HANDLE Hash = nullptr;
	DWORD ObjectSize = 0u;
	DWORD ResultSize = 0u;
	std::vector<uint8_t> Object;
	std::array<uint8_t, 32u> Digest{};
	bool_t Success = false;
	if (BCryptOpenAlgorithmProvider(
		&Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) >= 0 &&
		BCryptGetProperty(Algorithm, BCRYPT_OBJECT_LENGTH,
			reinterpret_cast<PUCHAR>(&ObjectSize), sizeof(ObjectSize),
			&ResultSize, 0u) >= 0)
	{
		Object.resize(ObjectSize);
		Success = BCryptCreateHash(
			Algorithm, &Hash, Object.data(), ObjectSize,
			nullptr, 0u, 0u) >= 0 &&
			BCryptHashData(Hash,
				reinterpret_cast<PUCHAR>(const_cast<char*>(Value.data())),
				static_cast<ULONG>(Value.size()), 0u) >= 0 &&
			BCryptFinishHash(Hash, Digest.data(),
				static_cast<ULONG>(Digest.size()), 0u) >= 0;
	}
	if (nullptr != Hash)
		BCryptDestroyHash(Hash);
	if (nullptr != Algorithm)
		BCryptCloseAlgorithmProvider(Algorithm, 0u);
	if (!Success)
		return {};
	std::ostringstream Output;
	Output << std::hex << std::setfill('0');
	for (const uint8_t Byte : Digest)
		Output << std::setw(2) << static_cast<uint32_t>(Byte);
	return Output.str();
}

Client::EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY
Client::CEffectRuntimeAuthorityCodec::Get_FrozenArtist31470FProgramIdentity()
{
	EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY Result;
	Result.strBuilderAuthorityCommitId = ACTIVE_RECONSTRUCTED_PROFILE.BuilderCommitId;
	Result.strBuilderAuthorityTreeId = ACTIVE_RECONSTRUCTED_PROFILE.BuilderTreeId;
	Result.strCandidateRawSha256 = ACTIVE_RECONSTRUCTED_PROFILE.CandidateRawSha256;
	Result.strProgramId = RECONSTRUCTED_PROGRAM_ID;
	Result.iProgramVersion = 1u;
	Result.strProgramSha256 = ACTIVE_RECONSTRUCTED_PROFILE.ProgramSha256;
	return Result;
}

bool_t Client::CEffectRuntimeAuthorityCodec::Parse_ReconstructedRuntimeProgram(
	const std::string_view Utf8Json,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY& ExpectedIdentity,
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>& InOutProgram,
	std::string& strOutError)
{
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Staged;
	if (!Parse_ReconstructedRuntimeProgramInternal(
			Utf8Json, ExpectedIdentity, true, Staged, strOutError))
	{
		return false;
	}
	InOutProgram = std::move(Staged);
	return true;
}

#if defined(LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS)
namespace Client
{
	bool_t Validate_ReconstructedRuntimeProgramForHarness(
		const std::string_view Utf8Json,
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY& ExpectedIdentity,
		std::string& strOutError)
	{
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> Discarded;
		return Parse_ReconstructedRuntimeProgramInternal(
			Utf8Json, ExpectedIdentity, false, Discarded, strOutError);
	}
}
#endif

bool_t Client::CEffectRuntimeAuthorityCodec::Parse_DerivedEntry(
	const DATA_JSON_VALUE& Value,
	std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>& OutDocument,
	std::string& strOutError)
{
	if (!Has_ExactKeys(Value, {
		"payloadKind", "effectAssetId", "authoringFormatVersion",
		"runtimeSemanticAuthority", "derivedIdentity",
		"authoringCarrierSha256", "assemblySha256",
		"compiledArtifactSha256", "compiledReceiptSha256",
		"artifactRevision", "compilerRevision", "compiledIrSha256",
		"compilerReceiptTokenSha256", "executionAdmission",
		"productAdmission", "compiledArtifact", "compiledReceipt" }))
	{
		strOutError = "Derived runtime entry fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* PayloadKind = Required(
		Value, "payloadKind", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* Authority = Required(
		Value, "runtimeSemanticAuthority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* IdentityValue = Required(
		Value, "derivedIdentity", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* Artifact = Required(
		Value, "compiledArtifact", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* Receipt = Required(
		Value, "compiledReceipt", DATA_JSON_TYPE::OBJECT);
	EFFECT_RUNTIME_AUTHORITY_IDENTITY Identity;
	if (nullptr == PayloadKind || PayloadKind->Get_String() != SEMANTIC_AUTHORITY ||
		nullptr == Authority || Authority->Get_String() != SEMANTIC_AUTHORITY ||
		!Read_ExactVersion(Value, "authoringFormatVersion", 13u) ||
		!Read_StableId(Value, "effectAssetId", Identity.strEffectAssetId) ||
		nullptr == IdentityValue ||
		!Parse_DerivedIdentity(*IdentityValue, Identity.Derived) ||
		!Read_U32(Value, "artifactRevision", Identity.iArtifactRevision) ||
		0u == Identity.iArtifactRevision ||
		!Read_StableId(Value, "compilerRevision", Identity.strCompilerRevision) ||
		!Read_Sha(Value, "authoringCarrierSha256",
			Identity.strAuthoringCarrierSha256) ||
		!Read_Sha(Value, "assemblySha256", Identity.strAssemblySha256) ||
		!Read_Sha(Value, "compiledArtifactSha256",
			Identity.strCompiledArtifactSha256) ||
		!Read_Sha(Value, "compiledReceiptSha256",
			Identity.strCompiledReceiptSha256) ||
		!Read_Sha(Value, "compiledIrSha256", Identity.strCompiledIrSha256) ||
		!Read_Sha(Value, "compilerReceiptTokenSha256",
			Identity.strCompilerReceiptTokenSha256) ||
		!Read_ExpectedBoolean(Value, "executionAdmission", true) ||
		!Read_ExpectedBoolean(Value, "productAdmission", false) ||
		nullptr == Artifact || nullptr == Receipt)
	{
		strOutError = "Derived runtime entry identity is invalid.";
		return false;
	}

	if (!Has_ExactKeys(*Artifact, {
		"schema", "formatVersion", "effectAssetId", "artifactRevision",
		"compilerRevision", "runtimeSemanticAuthority", "derivedIdentity",
		"compiledIrSha256", "compilerReceiptTokenSha256", "compiledIr",
		"executionAdmission", "productAdmission" }))
	{
		strOutError = "Compiled Effect artifact fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* ArtifactSchema = Required(
		*Artifact, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* ArtifactAuthority = Required(
		*Artifact, "runtimeSemanticAuthority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* ArtifactIdentityValue = Required(
		*Artifact, "derivedIdentity", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* CompiledIr = Required(
		*Artifact, "compiledIr", DATA_JSON_TYPE::OBJECT);
	EFFECT_RUNTIME_DERIVED_IDENTITY ArtifactIdentity;
	std::string ArtifactEffectId;
	std::string ArtifactCompiler;
	std::string ArtifactIrSha;
	std::string ArtifactToken;
	uint32_t ArtifactRevision = 0u;
	if (nullptr == ArtifactSchema || ArtifactSchema->Get_String() !=
			"lostark.effect-compiled-artifact" ||
		!Read_ExactVersion(*Artifact, "formatVersion", 1u) ||
		!Read_StableId(*Artifact, "effectAssetId", ArtifactEffectId) ||
		!Read_U32(*Artifact, "artifactRevision", ArtifactRevision) ||
		!Read_StableId(*Artifact, "compilerRevision", ArtifactCompiler) ||
		nullptr == ArtifactAuthority ||
		ArtifactAuthority->Get_String() != SEMANTIC_AUTHORITY ||
		nullptr == ArtifactIdentityValue ||
		!Parse_DerivedIdentity(*ArtifactIdentityValue, ArtifactIdentity) ||
		!Read_Sha(*Artifact, "compiledIrSha256", ArtifactIrSha) ||
		!Read_Sha(*Artifact, "compilerReceiptTokenSha256", ArtifactToken) ||
		nullptr == CompiledIr ||
		!Read_ExpectedBoolean(*Artifact, "executionAdmission", true) ||
		!Read_ExpectedBoolean(*Artifact, "productAdmission", false) ||
		ArtifactEffectId != Identity.strEffectAssetId ||
		ArtifactRevision != Identity.iArtifactRevision ||
		ArtifactCompiler != Identity.strCompilerRevision ||
		ArtifactIrSha != Identity.strCompiledIrSha256 ||
		ArtifactToken != Identity.strCompilerReceiptTokenSha256 ||
		!Equal(ArtifactIdentity, Identity.Derived))
	{
		strOutError = "Compiled Effect artifact identity mismatch.";
		return false;
	}

	if (!Has_ExactKeys(*CompiledIr, {
		"schema", "formatVersion", "effectAssetId", "artifactRevision",
		"compilerRevision", "runtimeSemanticAuthority", "derivedIdentity",
		"executionContract", "program" }))
	{
		strOutError = "Compiled Effect IR fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* IrSchema = Required(
		*CompiledIr, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* IrAuthority = Required(
		*CompiledIr, "runtimeSemanticAuthority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* IrIdentityValue = Required(
		*CompiledIr, "derivedIdentity", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* IrContractValue = Required(
		*CompiledIr, "executionContract", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* Program = Required(
		*CompiledIr, "program", DATA_JSON_TYPE::OBJECT);
	EFFECT_RUNTIME_DERIVED_IDENTITY IrIdentity;
	EFFECT_RUNTIME_EXECUTION_CONTRACT ExecutionContract;
	std::string IrEffectId;
	std::string IrCompiler;
	uint32_t IrRevision = 0u;
	if (nullptr == IrSchema || IrSchema->Get_String() !=
			"lostark.effect-compiled-ir" ||
		!Read_ExactVersion(*CompiledIr, "formatVersion", 1u) ||
		!Read_StableId(*CompiledIr, "effectAssetId", IrEffectId) ||
		!Read_U32(*CompiledIr, "artifactRevision", IrRevision) ||
		!Read_StableId(*CompiledIr, "compilerRevision", IrCompiler) ||
		nullptr == IrAuthority || IrAuthority->Get_String() != SEMANTIC_AUTHORITY ||
		nullptr == IrIdentityValue ||
		!Parse_DerivedIdentity(*IrIdentityValue, IrIdentity) ||
		nullptr == IrContractValue ||
		!Parse_ExecutionContract(*IrContractValue, ExecutionContract) ||
		nullptr == Program ||
		IrEffectId != Identity.strEffectAssetId ||
		IrRevision != Identity.iArtifactRevision ||
		IrCompiler != Identity.strCompilerRevision ||
		!Equal(IrIdentity, Identity.Derived))
	{
		strOutError = "Compiled Effect IR identity mismatch.";
		return false;
	}
	if (!Has_ExactKeys(*Program,
		{ "opcodes", "resourceBindings", "handlerReceipts" }))
	{
		strOutError = "Compiled Effect program fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* Opcodes = Required(
		*Program, "opcodes", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* ResourceBindings = Required(
		*Program, "resourceBindings", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* HandlerReceipts = Required(
		*Program, "handlerReceipts", DATA_JSON_TYPE::ARRAY);
	std::vector<EFFECT_RUNTIME_HANDLER_RECEIPT> ParsedHandlerReceipts;
	if (nullptr == Opcodes || nullptr == ResourceBindings ||
		nullptr == HandlerReceipts ||
		Contains_ReservedExecutionField(*Opcodes) ||
		Contains_ReservedExecutionField(*ResourceBindings) ||
		!Parse_HandlerReceipts(*HandlerReceipts, ParsedHandlerReceipts) ||
		!Equal(ExecutionContract, Combine(ParsedHandlerReceipts)))
	{
		strOutError = "Compiled Effect handler execution contract mismatch.";
		return false;
	}
	const std::string CanonicalIr = Serialize_CanonicalJson(*CompiledIr);
	if (Compute_Sha256Hex(CanonicalIr) != Identity.strCompiledIrSha256)
	{
		strOutError = "Compiled Effect IR canonical SHA mismatch.";
		return false;
	}
	if (Compute_Sha256Hex(Serialize_PrettyJson(*Artifact)) !=
		Identity.strCompiledArtifactSha256)
	{
		strOutError = "Compiled Effect artifact raw SHA mismatch.";
		return false;
	}

	if (!Has_ExactKeys(*Receipt, {
		"schema", "formatVersion", "effectAssetId", "artifactRevision",
		"compilerRevision", "runtimeSemanticAuthority", "derivedIdentity",
		"sourceContractVersion", "authoringCarrierSha256", "assemblySha256",
		"compiledArtifactSha256", "compiledIrSha256",
		"compilerReceiptRawSha256", "compilerReceiptCanonicalSha256",
		"compilerReceiptTokenSha256", "toolDependencies",
		"artifactBindingBlockerSet", "artifactBindingBlockerCount",
		"executionBlockerSet", "executionBlockerCount", "executionAdmission",
		"productAdmission", "publicationState" }))
	{
		strOutError = "Compiled Effect receipt fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* ReceiptSchema = Required(
		*Receipt, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* ReceiptAuthority = Required(
		*Receipt, "runtimeSemanticAuthority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* ReceiptIdentityValue = Required(
		*Receipt, "derivedIdentity", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* ToolDependencies = Required(
		*Receipt, "toolDependencies", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* PublicationState = Required(
		*Receipt, "publicationState", DATA_JSON_TYPE::STRING);
	EFFECT_RUNTIME_DERIVED_IDENTITY ReceiptIdentity;
	EFFECT_RUNTIME_EXECUTION_CONTRACT ReceiptContract;
	std::string ReceiptEffectId;
	std::string ReceiptCompiler;
	std::string ReceiptAuthoring;
	std::string ReceiptAssembly;
	std::string ReceiptArtifact;
	std::string ReceiptIr;
	std::string ReceiptRaw;
	std::string ReceiptCanonical;
	std::string ReceiptToken;
	uint32_t ReceiptRevision = 0u;
	uint32_t SourceContractVersion = 0u;
	const DATA_JSON_VALUE* ReceiptArtifactBlockers = Required(
		*Receipt, "artifactBindingBlockerSet", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* ReceiptExecutionBlockers = Required(
		*Receipt, "executionBlockerSet", DATA_JSON_TYPE::ARRAY);
	DATA_JSON_VALUE::OBJECT ReceiptContractObject;
	if (nullptr != ReceiptArtifactBlockers && nullptr != ReceiptExecutionBlockers)
	{
		ReceiptContractObject.emplace(
			"artifactBindingBlockerSet", *ReceiptArtifactBlockers);
		ReceiptContractObject.emplace("artifactBindingBlockerCount",
			*Receipt->Find("artifactBindingBlockerCount"));
		ReceiptContractObject.emplace("executionBlockerSet",
			*ReceiptExecutionBlockers);
		ReceiptContractObject.emplace("executionBlockerCount",
			*Receipt->Find("executionBlockerCount"));
		ReceiptContractObject.emplace("executionAdmission",
			*Receipt->Find("executionAdmission"));
	}
	const DATA_JSON_VALUE ReceiptContractValue =
		DATA_JSON_VALUE::Object(std::move(ReceiptContractObject));
	if (nullptr == ReceiptSchema || ReceiptSchema->Get_String() !=
			"lostark.effect-compiled-artifact-receipt" ||
		!Read_ExactVersion(*Receipt, "formatVersion", 1u) ||
		!Read_StableId(*Receipt, "effectAssetId", ReceiptEffectId) ||
		!Read_U32(*Receipt, "artifactRevision", ReceiptRevision) ||
		!Read_StableId(*Receipt, "compilerRevision", ReceiptCompiler) ||
		nullptr == ReceiptAuthority ||
		ReceiptAuthority->Get_String() != SEMANTIC_AUTHORITY ||
		nullptr == ReceiptIdentityValue ||
		!Parse_DerivedIdentity(*ReceiptIdentityValue, ReceiptIdentity) ||
		!Read_U32(*Receipt, "sourceContractVersion", SourceContractVersion) ||
		14u != SourceContractVersion ||
		!Read_Sha(*Receipt, "authoringCarrierSha256", ReceiptAuthoring) ||
		!Read_Sha(*Receipt, "assemblySha256", ReceiptAssembly) ||
		!Read_Sha(*Receipt, "compiledArtifactSha256", ReceiptArtifact) ||
		!Read_Sha(*Receipt, "compiledIrSha256", ReceiptIr) ||
		!Read_Sha(*Receipt, "compilerReceiptRawSha256", ReceiptRaw) ||
		!Read_Sha(*Receipt, "compilerReceiptCanonicalSha256", ReceiptCanonical) ||
		!Read_Sha(*Receipt, "compilerReceiptTokenSha256", ReceiptToken) ||
		nullptr == ToolDependencies || !Validate_ToolDependencies(*ToolDependencies) ||
		!Parse_ExecutionContract(ReceiptContractValue, ReceiptContract) ||
		nullptr == PublicationState ||
		PublicationState->Get_String() != CODE_ONLY_PUBLICATION_STATE ||
		!Read_ExpectedBoolean(*Receipt, "productAdmission", false) ||
		ReceiptEffectId != Identity.strEffectAssetId ||
		ReceiptRevision != Identity.iArtifactRevision ||
		ReceiptCompiler != Identity.strCompilerRevision ||
		!Equal(ReceiptIdentity, Identity.Derived) ||
		ReceiptAuthoring != Identity.strAuthoringCarrierSha256 ||
		ReceiptAssembly != Identity.strAssemblySha256 ||
		ReceiptArtifact != Identity.strCompiledArtifactSha256 ||
		ReceiptIr != Identity.strCompiledIrSha256 ||
		ReceiptToken != Identity.strCompilerReceiptTokenSha256 ||
		!Equal(ReceiptContract, ExecutionContract))
	{
		strOutError = "Compiled Effect receipt identity mismatch.";
		return false;
	}
	if (Compute_Sha256Hex(Serialize_PrettyJson(*Receipt)) !=
		Identity.strCompiledReceiptSha256)
	{
		strOutError = "Compiled Effect receipt raw SHA mismatch.";
		return false;
	}

	auto Staged = std::make_shared<EFFECT_COMPILED_RUNTIME_DOCUMENT>();
	Staged->Identity = std::move(Identity);
	Staged->ExecutionContract = std::move(ExecutionContract);
	Staged->HandlerReceipts = std::move(ParsedHandlerReceipts);
	Staged->iOpcodeCount = static_cast<uint32_t>(Opcodes->Get_Array().size());
	Staged->iResourceBindingCount =
		static_cast<uint32_t>(ResourceBindings->Get_Array().size());
	Staged->bArtifactBindingSelfConsistent = true;
	Staged->bExternalIdentityAuthenticated = false;
	Staged->bArtifactExecutionAdmission = true;
	Staged->bTypedProgramMaterialized = false;
	Staged->bRuntimeExecutionAdmission = false;
	Staged->bProductAdmission = false;
	Staged->RuntimeBlockers.emplace_back(EXTERNAL_AUTHENTICATION_PENDING);
	Staged->RuntimeBlockers.emplace_back(TYPED_ADAPTER_PENDING);
	OutDocument = std::move(Staged);
	strOutError.clear();
	return true;
}
