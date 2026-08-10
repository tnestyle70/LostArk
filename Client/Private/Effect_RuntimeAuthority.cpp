#include "Effect_RuntimeAuthority.h"

#include <algorithm>
#include <array>
#include <bcrypt.h>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <unordered_map>

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
		if (!std::is_sorted(Staged.begin(), Staged.end()) ||
			std::adjacent_find(Staged.begin(), Staged.end()) != Staged.end())
		{
			return false;
		}
		OutBlockers = std::move(Staged);
		return true;
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

namespace
{
	bool_t Is_OpaqueRuntimeId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 2048u && std::all_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return Character >= 0x21u && Character <= 0x7eu &&
					Character != '\\';
			});
	}

	bool_t Read_OpaqueRuntimeId(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		std::string& OutValue,
		const bool_t bAllowEmpty = false)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::STRING);
		if (nullptr == Value ||
			(!bAllowEmpty && !Is_OpaqueRuntimeId(Value->Get_String())) ||
			(bAllowEmpty && !Value->Get_String().empty() &&
				!Is_OpaqueRuntimeId(Value->Get_String())))
		{
			return false;
		}
		OutValue = Value->Get_String();
		return true;
	}

	bool_t Read_FiniteNumber(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		double& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == Value || !std::isfinite(Value->Get_Number()))
			return false;
		OutValue = Value->Get_Number();
		return true;
	}

	bool_t Read_NumberArray(
		const DATA_JSON_VALUE& Value,
		std::vector<double>& OutValues)
	{
		if (!Value.Is_Array())
			return false;
		std::vector<double> Staged;
		Staged.reserve(Value.Get_Array().size());
		for (const DATA_JSON_VALUE& Item : Value.Get_Array())
		{
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
				return false;
			Staged.push_back(Item.Get_Number());
		}
		OutValues = std::move(Staged);
		return true;
	}

	bool_t Read_Number4(
		const DATA_JSON_VALUE& Value,
		std::array<double, 4u>& OutValues)
	{
		std::vector<double> Values;
		if (!Read_NumberArray(Value, Values) || Values.size() != 4u)
			return false;
		std::copy(Values.begin(), Values.end(), OutValues.begin());
		return true;
	}

	bool_t Parse_RuntimeLiteral(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_LITERAL& OutLiteral)
	{
		if (!Has_ExactKeys(Value,
			{ "literalId", "propertyPath", "kind", "value" }) ||
			!Read_OpaqueRuntimeId(Value, "literalId", OutLiteral.strLiteralId) ||
			!Read_OpaqueRuntimeId(
				Value, "propertyPath", OutLiteral.strPropertyPath))
		{
			return false;
		}
		const DATA_JSON_VALUE* Kind = Required(
			Value, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Payload = Value.Find("value");
		if (nullptr == Kind || nullptr == Payload)
			return false;
		if (Kind->Get_String() == "boolean" && Payload->Is_Boolean())
		{
			OutLiteral.eKind = EFFECT_RUNTIME_LITERAL_KIND::BOOLEAN;
			OutLiteral.bBoolean = Payload->Get_Boolean();
			return true;
		}
		if (Kind->Get_String() == "number" && Payload->Is_Number() &&
			std::isfinite(Payload->Get_Number()))
		{
			OutLiteral.eKind = EFFECT_RUNTIME_LITERAL_KIND::NUMBER;
			OutLiteral.fNumber = Payload->Get_Number();
			return true;
		}
		if (Kind->Get_String() == "string" && Payload->Is_String())
		{
			OutLiteral.eKind = EFFECT_RUNTIME_LITERAL_KIND::STRING;
			OutLiteral.strString = Payload->Get_String();
			return true;
		}
		return false;
	}

	bool_t Parse_RuntimeDistributionKey(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_DISTRIBUTION_KEY& OutKey)
	{
		if (!Has_ExactKeys(Value, {
			"time", "minimum", "maximum", "arriveTangentMinimum",
			"leaveTangentMinimum", "arriveTangentMaximum",
			"leaveTangentMaximum", "interpolation" }) ||
			!Read_FiniteNumber(Value, "time", OutKey.fTime))
		{
			return false;
		}
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
		const DATA_JSON_VALUE* Interpolation = Required(
			Value, "interpolation", DATA_JSON_TYPE::STRING);
		if (nullptr == Minimum || nullptr == Maximum ||
			nullptr == ArriveMinimum || nullptr == LeaveMinimum ||
			nullptr == ArriveMaximum || nullptr == LeaveMaximum ||
			nullptr == Interpolation ||
			(Interpolation->Get_String() != "linear" &&
				Interpolation->Get_String() != "constant" &&
				Interpolation->Get_String() != "cubic") ||
			!Read_Number4(*Minimum, OutKey.Minimum) ||
			!Read_Number4(*Maximum, OutKey.Maximum) ||
			!Read_Number4(*ArriveMinimum, OutKey.ArriveTangentMinimum) ||
			!Read_Number4(*LeaveMinimum, OutKey.LeaveTangentMinimum) ||
			!Read_Number4(*ArriveMaximum, OutKey.ArriveTangentMaximum) ||
			!Read_Number4(*LeaveMaximum, OutKey.LeaveTangentMaximum))
		{
			return false;
		}
		OutKey.strInterpolation = Interpolation->Get_String();
		return true;
	}

	bool_t Parse_RuntimeDistribution(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_DISTRIBUTION& OutDistribution)
	{
		if (!Has_ExactKeys(Value, { "payloadDistributionId", "descriptor" }) ||
			!Read_OpaqueRuntimeId(Value, "payloadDistributionId",
				OutDistribution.strPayloadDistributionId))
		{
			return false;
		}
		const DATA_JSON_VALUE* Descriptor = Required(
			Value, "descriptor", DATA_JSON_TYPE::OBJECT);
		if (nullptr == Descriptor || !Has_ExactKeys(*Descriptor, {
			"propertyPath", "sourceClass", "sourceObjectPath",
			"componentCount", "operation", "randomLockAxes",
			"lookupTableChunkSize", "lookupTableNumElements",
			"lookupTableTimeScale", "lookupTableStartTime",
			"defaultMinimum", "defaultMaximum", "lookupTable", "keys",
			"referenceId", "occurrenceId", "payloadStatus", "fidelity" }) ||
			!Read_OpaqueRuntimeId(*Descriptor, "propertyPath",
				OutDistribution.strPropertyPath) ||
			!Read_OpaqueRuntimeId(*Descriptor, "sourceClass",
				OutDistribution.strSourceClass, true) ||
			!Read_OpaqueRuntimeId(*Descriptor, "sourceObjectPath",
				OutDistribution.strSourceObjectPath, true) ||
			!Read_U32(*Descriptor, "componentCount",
				OutDistribution.iComponentCount) ||
			OutDistribution.iComponentCount > 4u ||
			!Read_U32(*Descriptor, "operation", OutDistribution.iOperation) ||
			!Read_U32(*Descriptor, "randomLockAxes",
				OutDistribution.iRandomLockAxes) ||
			!Read_U32(*Descriptor, "lookupTableChunkSize",
				OutDistribution.iLookupTableChunkSize) ||
			!Read_U32(*Descriptor, "lookupTableNumElements",
				OutDistribution.iLookupTableNumElements) ||
			!Read_FiniteNumber(*Descriptor, "lookupTableTimeScale",
				OutDistribution.fLookupTableTimeScale) ||
			!Read_FiniteNumber(*Descriptor, "lookupTableStartTime",
				OutDistribution.fLookupTableStartTime) ||
			!Read_OpaqueRuntimeId(*Descriptor, "referenceId",
				OutDistribution.strReferenceId, true) ||
			!Read_OpaqueRuntimeId(*Descriptor, "occurrenceId",
				OutDistribution.strOccurrenceId, true) ||
			!Read_OpaqueRuntimeId(*Descriptor, "payloadStatus",
				OutDistribution.strPayloadStatus) ||
			!Read_OpaqueRuntimeId(*Descriptor, "fidelity",
				OutDistribution.strFidelity))
		{
			return false;
		}
		const DATA_JSON_VALUE* Minimum = Required(
			*Descriptor, "defaultMinimum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Maximum = Required(
			*Descriptor, "defaultMaximum", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Lookup = Required(
			*Descriptor, "lookupTable", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Keys = Required(
			*Descriptor, "keys", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Minimum || nullptr == Maximum || nullptr == Lookup ||
			nullptr == Keys ||
			!Read_Number4(*Minimum, OutDistribution.DefaultMinimum) ||
			!Read_Number4(*Maximum, OutDistribution.DefaultMaximum) ||
			!Read_NumberArray(*Lookup, OutDistribution.LookupTable))
		{
			return false;
		}
		for (const DATA_JSON_VALUE& Item : Keys->Get_Array())
		{
			EFFECT_RUNTIME_DISTRIBUTION_KEY Key;
			if (!Parse_RuntimeDistributionKey(Item, Key))
				return false;
			OutDistribution.Keys.push_back(std::move(Key));
		}
		return true;
	}

	bool_t Parse_RuntimeDistributionAdapter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_DISTRIBUTION_ADAPTER& OutAdapter)
	{
		if (!Has_ExactKeys(Value, { "distributionId", "payloadDistributionId",
			"evaluatorCapabilityId", "decision", "blockers" }) ||
			!Read_OpaqueRuntimeId(Value, "distributionId",
				OutAdapter.strDistributionId) ||
			!Read_OpaqueRuntimeId(Value, "payloadDistributionId",
				OutAdapter.strPayloadDistributionId) ||
			!Read_StableId(Value, "evaluatorCapabilityId",
				OutAdapter.strEvaluatorCapabilityId))
		{
			return false;
		}
		const DATA_JSON_VALUE* Decision = Required(
			Value, "decision", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Decision || nullptr == Blockers ||
			!Parse_Blockers(*Blockers, OutAdapter.Blockers))
		{
			return false;
		}
		OutAdapter.bReady = Decision->Get_String() == "READY_FOR_HANDLER";
		return (OutAdapter.bReady && OutAdapter.Blockers.empty()) ||
			(Decision->Get_String() == "BLOCKED" &&
				!OutAdapter.Blockers.empty());
	}

	bool_t Parse_RuntimeOpcode(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_OPCODE& OutOpcode)
	{
		if (!Has_ExactKeys(Value, {
			"opcodeId", "emitterId", "order", "opcode", "handlerId",
			"exactSourceClass", "sourceObjectId", "sourceRecordSha256",
			"payload", "distributionAdapters", "seed", "implicitDefaults",
			"decision", "blockers" }) ||
			!Read_OpaqueRuntimeId(Value, "opcodeId", OutOpcode.strOpcodeId) ||
			!Read_OpaqueRuntimeId(Value, "emitterId", OutOpcode.strEmitterId) ||
			!Read_U32(Value, "order", OutOpcode.iOrder) ||
			!Read_StableId(Value, "opcode", OutOpcode.strOpcode) ||
			!Read_StableId(Value, "handlerId", OutOpcode.strHandlerId) ||
			!Read_StableId(Value, "exactSourceClass",
				OutOpcode.strExactSourceClass) ||
			!Read_OpaqueRuntimeId(Value, "sourceObjectId",
				OutOpcode.strSourceObjectId) ||
			!Read_Sha(Value, "sourceRecordSha256",
				OutOpcode.strSourceRecordSha256))
		{
			return false;
		}
		const DATA_JSON_VALUE* Payload = Required(
			Value, "payload", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Adapters = Required(
			Value, "distributionAdapters", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Seed = Value.Find("seed");
		const DATA_JSON_VALUE* Defaults = Required(
			Value, "implicitDefaults", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Decision = Required(
			Value, "decision", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Blockers = Required(
			Value, "blockers", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Payload || nullptr == Adapters || nullptr == Seed ||
			nullptr == Defaults || nullptr == Decision || nullptr == Blockers ||
			!Parse_Blockers(*Blockers, OutOpcode.Blockers))
		{
			return false;
		}
		OutOpcode.bReady = Decision->Get_String() == "READY_FOR_HANDLER";
		if (!((OutOpcode.bReady && OutOpcode.Blockers.empty()) ||
			(Decision->Get_String() == "BLOCKED" &&
				!OutOpcode.Blockers.empty())))
		{
			return false;
		}
		if (!Has_ExactKeys(*Payload, { "stableId", "className", "objectPath",
			"literals", "distributions", "payloadSha256" }) ||
			!Read_OpaqueRuntimeId(*Payload, "stableId",
				OutOpcode.strPayloadStableId) ||
			!Read_StableId(*Payload, "className",
				OutOpcode.strPayloadClassName) ||
			OutOpcode.strPayloadClassName != OutOpcode.strExactSourceClass ||
			!Read_OpaqueRuntimeId(*Payload, "objectPath",
				OutOpcode.strPayloadObjectPath) ||
			!Read_Sha(*Payload, "payloadSha256", OutOpcode.strPayloadSha256))
		{
			return false;
		}
		DATA_JSON_VALUE::OBJECT UnsignedPayload = Payload->Get_Object();
		UnsignedPayload.erase("payloadSha256");
		if (CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(
				DATA_JSON_VALUE::Object(std::move(UnsignedPayload)))) !=
			OutOpcode.strPayloadSha256)
		{
			return false;
		}
		const DATA_JSON_VALUE* Literals = Required(
			*Payload, "literals", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* Distributions = Required(
			*Payload, "distributions", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Literals || nullptr == Distributions)
			return false;
		std::set<std::string> LiteralIds;
		for (const DATA_JSON_VALUE& Item : Literals->Get_Array())
		{
			EFFECT_RUNTIME_LITERAL Literal;
			if (!Parse_RuntimeLiteral(Item, Literal) ||
				!LiteralIds.insert(Literal.strLiteralId).second)
			{
				return false;
			}
			OutOpcode.Literals.push_back(std::move(Literal));
		}
		std::set<std::string> PayloadDistributionIds;
		for (const DATA_JSON_VALUE& Item : Distributions->Get_Array())
		{
			EFFECT_RUNTIME_DISTRIBUTION Distribution;
			if (!Parse_RuntimeDistribution(Item, Distribution) ||
				!PayloadDistributionIds.insert(
					Distribution.strPayloadDistributionId).second)
			{
				return false;
			}
			OutOpcode.Distributions.push_back(std::move(Distribution));
		}
		std::set<std::string> DistributionIds;
		std::set<std::string> AdaptedPayloadIds;
		for (const DATA_JSON_VALUE& Item : Adapters->Get_Array())
		{
			EFFECT_RUNTIME_DISTRIBUTION_ADAPTER Adapter;
			if (!Parse_RuntimeDistributionAdapter(Item, Adapter) ||
				!DistributionIds.insert(Adapter.strDistributionId).second ||
				!AdaptedPayloadIds.insert(
					Adapter.strPayloadDistributionId).second ||
				!PayloadDistributionIds.contains(
					Adapter.strPayloadDistributionId))
			{
				return false;
			}
			OutOpcode.DistributionAdapters.push_back(std::move(Adapter));
		}
		if (AdaptedPayloadIds != PayloadDistributionIds)
			return false;
		OutOpcode.strSeedCanonicalJson =
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(*Seed);
		for (const DATA_JSON_VALUE& Item : Defaults->Get_Array())
		{
			if (!Item.Is_Object())
				return false;
			OutOpcode.ImplicitDefaultCanonicalJson.push_back(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Item));
		}
		return true;
	}

	bool_t Parse_RendererFamily(
		const std::string_view Value,
		EFFECT_RUNTIME_RENDERER_FAMILY& OutFamily)
	{
		if (Value == "MeshParticle")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::MESH;
		else if (Value == "SpriteParticle")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::SPRITE;
		else if (Value == "DecalParticle")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::DECAL;
		else if (Value == "CascadeRibbon")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::RIBBON;
		else if (Value == "LightParticle")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::LIGHT;
		else if (Value == "ScreenPost")
			OutFamily = EFFECT_RUNTIME_RENDERER_FAMILY::SCREEN_POST;
		else
			return false;
		return true;
	}

	bool_t Parse_RuntimeActionParameter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_ACTION_PARAMETER& OutParameter)
	{
		if (!Has_ExactKeys(Value,
			{ "name", "kind", "value", "sourceIndex", "sourceValueByteOffset" }) ||
			!Read_OpaqueRuntimeId(Value, "name", OutParameter.strName) ||
			!Read_StableId(Value, "kind", OutParameter.strKind) ||
			!Read_U32(Value, "sourceIndex", OutParameter.iSourceIndex) ||
			!Read_U32(Value, "sourceValueByteOffset",
				OutParameter.iSourceValueByteOffset))
		{
			return false;
		}
		const DATA_JSON_VALUE* Payload = Value.Find("value");
		if (nullptr == Payload)
			return false;
		if (OutParameter.strKind == "scalar" && Payload->Is_Number() &&
			std::isfinite(Payload->Get_Number()))
		{
			OutParameter.Values.push_back(Payload->Get_Number());
			return true;
		}
		return OutParameter.strKind == "vector" &&
			Read_NumberArray(*Payload, OutParameter.Values) &&
			OutParameter.Values.size() == 3u;
	}

	bool_t Parse_RuntimeEmitter(
		const DATA_JSON_VALUE& Value,
		EFFECT_RUNTIME_EMITTER& OutEmitter)
	{
		if (!Has_ExactKeys(Value, {
			"emitterId", "evidenceId", "sourceOccurrenceId", "sourceSystemId",
			"sourceEmitterPath", "rendererType", "selectedLod",
			"actionCueParameterInputs", "orderedOpcodeIds" }) ||
			!Read_OpaqueRuntimeId(Value, "emitterId", OutEmitter.strEmitterId) ||
			!Read_StableId(Value, "evidenceId", OutEmitter.strEvidenceId) ||
			!Read_OpaqueRuntimeId(Value, "sourceOccurrenceId",
				OutEmitter.strSourceOccurrenceId) ||
			!Read_OpaqueRuntimeId(Value, "sourceSystemId",
				OutEmitter.strSourceSystemId) ||
			!Read_OpaqueRuntimeId(Value, "sourceEmitterPath",
				OutEmitter.strSourceEmitterPath))
		{
			return false;
		}
		const DATA_JSON_VALUE* Renderer = Required(
			Value, "rendererType", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* SelectedLod = Required(
			Value, "selectedLod", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* Parameters = Required(
			Value, "actionCueParameterInputs", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* OpcodeIds = Required(
			Value, "orderedOpcodeIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Renderer ||
			!Parse_RendererFamily(Renderer->Get_String(), OutEmitter.eRenderer) ||
			nullptr == SelectedLod || nullptr == Parameters || nullptr == OpcodeIds ||
			!Has_ExactKeys(*SelectedLod, { "sourceLodPath", "sourceLodNodeId",
				"sourceLodRecordSha256", "fields" }) ||
			!Read_OpaqueRuntimeId(*SelectedLod, "sourceLodPath",
				OutEmitter.strSelectedLodPath) ||
			!Read_OpaqueRuntimeId(*SelectedLod, "sourceLodNodeId",
				OutEmitter.strSelectedLodNodeId) ||
			!Read_Sha(*SelectedLod, "sourceLodRecordSha256",
				OutEmitter.strSelectedLodRecordSha256))
		{
			return false;
		}
		const DATA_JSON_VALUE* LodFields = Required(
			*SelectedLod, "fields", DATA_JSON_TYPE::ARRAY);
		if (nullptr == LodFields || LodFields->Get_Array().size() != 2u)
			return false;
		std::set<std::string> LodFieldNames;
		for (const DATA_JSON_VALUE& Field : LodFields->Get_Array())
		{
			if (!Has_ExactKeys(Field,
				{ "fieldId", "fieldName", "decision", "oracleId" }))
			{
				return false;
			}
			std::string FieldId;
			std::string FieldName;
			std::string Decision;
			std::string Oracle;
			if (!Read_OpaqueRuntimeId(Field, "fieldId", FieldId) ||
				!Read_StableId(Field, "fieldName", FieldName) ||
				!Read_StableId(Field, "decision", Decision) ||
				!Read_StableId(Field, "oracleId", Oracle) ||
				Decision != "VERIFIED_IRRELEVANT" ||
				!LodFieldNames.insert(FieldName).second)
			{
				return false;
			}
		}
		if (LodFieldNames != std::set<std::string>{ "enabled", "level" })
			return false;
		for (const DATA_JSON_VALUE& Item : Parameters->Get_Array())
		{
			EFFECT_RUNTIME_ACTION_PARAMETER Parameter;
			if (!Parse_RuntimeActionParameter(Item, Parameter))
				return false;
			OutEmitter.ActionCueParameters.push_back(std::move(Parameter));
		}
		for (const DATA_JSON_VALUE& Item : OpcodeIds->Get_Array())
		{
			if (!Item.Is_String() || !Is_OpaqueRuntimeId(Item.Get_String()))
				return false;
			OutEmitter.OrderedOpcodeIds.push_back(Item.Get_String());
		}
		return !OutEmitter.OrderedOpcodeIds.empty();
	}
}

bool_t Client::CEffectRuntimeAuthorityCodec::Parse_SourceRuntimeProgram(
	const DATA_JSON_VALUE& Value,
	std::shared_ptr<const EFFECT_SOURCE_RUNTIME_PROGRAM>& OutProgram,
	std::string& strOutError)
{
	if (!Has_ExactKeys(Value, {
		"schema", "formatVersion", "effectAssetId",
		"sourceExecutionReceiptSha256",
		"sourceExecutionReceiptCanonicalSha256",
		"sourceExecutionReceiptObservedRawSha256", "runtimeSemanticAuthority",
		"emitters", "opcodes", "handlerReceipts", "executionContract",
		"summary", "runtimeExecutionAdmission", "productAdmission",
		"programSha256" }))
	{
		strOutError = "Source runtime program fields are invalid.";
		return false;
	}
	const DATA_JSON_VALUE* Schema = Required(
		Value, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* Authority = Required(
		Value, "runtimeSemanticAuthority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* Emitters = Required(
		Value, "emitters", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* Opcodes = Required(
		Value, "opcodes", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* HandlerReceipts = Required(
		Value, "handlerReceipts", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* Contract = Required(
		Value, "executionContract", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* Summary = Required(
		Value, "summary", DATA_JSON_TYPE::OBJECT);
	auto Staged = std::make_shared<EFFECT_SOURCE_RUNTIME_PROGRAM>();
	if (nullptr == Schema || Schema->Get_String() !=
			"lostark.effect-source-runtime-program" ||
		!Read_ExactVersion(Value, "formatVersion",
			EFFECT_SOURCE_RUNTIME_PROGRAM_FORMAT_VERSION) ||
		!Read_StableId(Value, "effectAssetId", Staged->strEffectAssetId) ||
		nullptr == Authority ||
		Authority->Get_String() != "TYPED_SOURCE_PROGRAM_CANDIDATE" ||
		!Read_Sha(Value, "sourceExecutionReceiptSha256",
			Staged->strSourceExecutionReceiptSha256) ||
		!Read_Sha(Value, "sourceExecutionReceiptCanonicalSha256",
			Staged->strSourceExecutionReceiptCanonicalSha256) ||
		!Read_Sha(Value, "sourceExecutionReceiptObservedRawSha256",
			Staged->strSourceExecutionReceiptObservedRawSha256) ||
		!Read_Sha(Value, "programSha256", Staged->strProgramSha256) ||
		nullptr == Emitters || nullptr == Opcodes ||
		nullptr == HandlerReceipts || nullptr == Contract || nullptr == Summary ||
		!Parse_HandlerReceipts(*HandlerReceipts, Staged->HandlerReceipts) ||
		!Parse_ExecutionContract(*Contract, Staged->ExecutionContract))
	{
		strOutError = "Source runtime program identity is invalid.";
		return false;
	}
	DATA_JSON_VALUE::OBJECT UnsignedProgram = Value.Get_Object();
	UnsignedProgram.erase("programSha256");
	if (Compute_Sha256Hex(Serialize_CanonicalJson(
		DATA_JSON_VALUE::Object(std::move(UnsignedProgram)))) !=
		Staged->strProgramSha256)
	{
		strOutError = "Source runtime program canonical SHA mismatch.";
		return false;
	}
	std::set<std::string> HandlerIds;
	std::unordered_map<std::string,
		const EFFECT_RUNTIME_HANDLER_RECEIPT*> HandlerById;
	for (const EFFECT_RUNTIME_HANDLER_RECEIPT& Receipt : Staged->HandlerReceipts)
	{
		if (!HandlerIds.insert(Receipt.strHandlerId).second ||
			!std::all_of(
				Receipt.ExecutionContract.ExecutionBlockers.begin(),
				Receipt.ExecutionContract.ExecutionBlockers.end(),
				[&Staged](const std::string& Blocker)
				{
					return std::binary_search(
						Staged->ExecutionContract.ExecutionBlockers.begin(),
						Staged->ExecutionContract.ExecutionBlockers.end(),
						Blocker);
				}))
		{
			strOutError = "Source runtime handler receipt is duplicated.";
			return false;
		}
		HandlerById.emplace(Receipt.strHandlerId, &Receipt);
	}
	std::set<std::string> OpcodeIds;
	std::unordered_map<std::string, const EFFECT_RUNTIME_OPCODE*> OpcodeById;
	uint32_t ReadyOpcodeCount = 0u;
	uint32_t DistributionCount = 0u;
	Staged->Opcodes.reserve(Opcodes->Get_Array().size());
	for (const DATA_JSON_VALUE& Item : Opcodes->Get_Array())
	{
		EFFECT_RUNTIME_OPCODE Opcode;
		if (!Parse_RuntimeOpcode(Item, Opcode) ||
			!OpcodeIds.insert(Opcode.strOpcodeId).second)
		{
			strOutError = "Source runtime opcode is invalid or duplicated.";
			return false;
		}
		const auto Handler = HandlerById.find(Opcode.strHandlerId);
		if (Handler == HandlerById.end() || !std::all_of(
			Opcode.Blockers.begin(), Opcode.Blockers.end(),
			[&Staged](const std::string& Blocker)
			{
				return std::binary_search(
					Staged->ExecutionContract.ExecutionBlockers.begin(),
					Staged->ExecutionContract.ExecutionBlockers.end(),
					Blocker);
			}))
		{
			strOutError = "Source runtime opcode handler receipt mismatch.";
			return false;
		}
		ReadyOpcodeCount += Opcode.bReady ? 1u : 0u;
		DistributionCount += static_cast<uint32_t>(
			Opcode.DistributionAdapters.size());
		Staged->Opcodes.push_back(std::move(Opcode));
		OpcodeById.emplace(Staged->Opcodes.back().strOpcodeId,
			&Staged->Opcodes.back());
	}
	std::set<std::string> EmitterIds;
	std::set<std::string> ReferencedOpcodeIds;
	Staged->Emitters.reserve(Emitters->Get_Array().size());
	for (const DATA_JSON_VALUE& Item : Emitters->Get_Array())
	{
		EFFECT_RUNTIME_EMITTER Emitter;
		if (!Parse_RuntimeEmitter(Item, Emitter) ||
			!EmitterIds.insert(Emitter.strEmitterId).second)
		{
			strOutError = "Source runtime emitter is invalid or duplicated.";
			return false;
		}
		for (uint32_t Index = 0u;
			Index < Emitter.OrderedOpcodeIds.size(); ++Index)
		{
			const std::string& OpcodeId = Emitter.OrderedOpcodeIds[Index];
			const auto Opcode = OpcodeById.find(OpcodeId);
			if (Opcode == OpcodeById.end() ||
				Opcode->second->strEmitterId != Emitter.strEmitterId ||
				Opcode->second->iOrder != Index ||
				!ReferencedOpcodeIds.insert(OpcodeId).second)
			{
				strOutError = "Source runtime emitter opcode order mismatch.";
				return false;
			}
		}
		Staged->Emitters.push_back(std::move(Emitter));
	}
	if (ReferencedOpcodeIds != OpcodeIds)
	{
		strOutError = "Source runtime opcode coverage mismatch.";
		return false;
	}
	if (!Has_ExactKeys(*Summary, { "emitterCount", "opcodeCount",
		"readyOpcodeCount", "blockedOpcodeCount", "distributionCount",
		"handlerReceiptCount", "silentFallbackCount" }))
	{
		strOutError = "Source runtime summary fields are invalid.";
		return false;
	}
	uint32_t EmitterCount = 0u;
	uint32_t OpcodeCount = 0u;
	uint32_t SummaryReady = 0u;
	uint32_t SummaryBlocked = 0u;
	uint32_t SummaryDistributions = 0u;
	uint32_t SummaryHandlers = 0u;
	uint32_t SilentFallback = 0u;
	if (!Read_U32(*Summary, "emitterCount", EmitterCount) ||
		!Read_U32(*Summary, "opcodeCount", OpcodeCount) ||
		!Read_U32(*Summary, "readyOpcodeCount", SummaryReady) ||
		!Read_U32(*Summary, "blockedOpcodeCount", SummaryBlocked) ||
		!Read_U32(*Summary, "distributionCount", SummaryDistributions) ||
		!Read_U32(*Summary, "handlerReceiptCount", SummaryHandlers) ||
		!Read_U32(*Summary, "silentFallbackCount", SilentFallback) ||
		EmitterCount != Staged->Emitters.size() ||
		OpcodeCount != Staged->Opcodes.size() ||
		SummaryReady != ReadyOpcodeCount ||
		SummaryBlocked != OpcodeCount - ReadyOpcodeCount ||
		SummaryDistributions != DistributionCount ||
		SummaryHandlers != Staged->HandlerReceipts.size() ||
		0u != SilentFallback)
	{
		strOutError = "Source runtime summary mismatch.";
		return false;
	}
	const DATA_JSON_VALUE* RuntimeAdmission = Required(
		Value, "runtimeExecutionAdmission", DATA_JSON_TYPE::BOOLEAN);
	const DATA_JSON_VALUE* ProductAdmission = Required(
		Value, "productAdmission", DATA_JSON_TYPE::BOOLEAN);
	if (nullptr == RuntimeAdmission || nullptr == ProductAdmission ||
		RuntimeAdmission->Get_Boolean() !=
			Staged->ExecutionContract.bExecutionAdmission ||
		ProductAdmission->Get_Boolean())
	{
		strOutError = "Source runtime admission mismatch.";
		return false;
	}
	Staged->bSourceIdentitySelfConsistent = true;
	Staged->bRuntimeExecutionAdmission = RuntimeAdmission->Get_Boolean();
	Staged->bProductAdmission = false;
	OutProgram = std::move(Staged);
	strOutError.clear();
	return true;
}

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
