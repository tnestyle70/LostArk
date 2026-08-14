#include "Effect_OccurrenceTuning.h"
#include "Effect_DocumentCodec.h"
#include "Effect_VisualProgramCorpus.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <system_error>

namespace
{
	using namespace Client;

	constexpr std::string_view TUNING_SCHEMA =
		"lostark.effect-occurrence-tuning";
	constexpr std::string_view SOURCE_OVERLAY_SCHEMA =
		"lostark.effect-source-authoring-overlay";
	constexpr size_t MAXIMUM_TUNING_BYTES = 8u * 1024u * 1024u;

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* Value = Object.Find(Name);
		return nullptr != Value && Value->Get_Type() == eType ? Value : nullptr;
	}

	bool_t Has_ExactOrderedKeys(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Keys)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Keys.size() ||
			Object.Get_ObjectInsertionOrder().size() != Keys.size())
		{
			return false;
		}
		size_t Index = 0u;
		for (const std::string_view Key : Keys)
		{
			if (!Object.Get_Object().contains(Key) ||
				Object.Get_ObjectInsertionOrder()[Index++] != Key)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_LowerHexSha256(const std::string_view Value)
	{
		return Value.size() == 64u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

	bool_t Is_StableEffectAssetId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 256u && std::all_of(
			Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= 'a' && Character <= 'z') ||
					(Character >= 'A' && Character <= 'Z') ||
					(Character >= '0' && Character <= '9') ||
					Character == '_' || Character == '-' || Character == '.';
			});
	}

	bool_t Is_RuntimeOccurrenceId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 1024u && std::none_of(
			Value.begin(), Value.end(), [](const unsigned char Character)
			{
				return Character < 0x20u || Character == 0x7fu;
			});
	}

	bool_t Read_Float3(
		const DATA_JSON_VALUE& Object,
		const std::string_view Name,
		float3_t& OutValue)
	{
		const DATA_JSON_VALUE* Value = Required(
			Object, Name, DATA_JSON_TYPE::ARRAY);
		if (nullptr == Value || Value->Get_Array().size() != 3u)
			return false;
		float Values[3]{};
		for (size_t Index = 0u; Index < 3u; ++Index)
		{
			const DATA_JSON_VALUE& Component = Value->Get_Array()[Index];
			if (!Component.Is_Number() || !std::isfinite(Component.Get_Number()) ||
				Component.Get_Number() < -static_cast<double>(FLT_MAX) ||
				Component.Get_Number() > static_cast<double>(FLT_MAX))
			{
				return false;
			}
			Values[Index] = static_cast<float>(Component.Get_Number());
		}
		OutValue = { Values[0], Values[1], Values[2] };
		return true;
	}

	bool_t Parse_Value(
		const DATA_JSON_VALUE& Root,
		EFFECT_OCCURRENCE_TUNING_DOCUMENT& OutDocument,
		std::string& strOutError)
	{
		if (!Has_ExactOrderedKeys(Root,
			{ "schema", "formatVersion", "effectAssetId", "entries" }))
		{
			strOutError =
				"Occurrence tuning root fields or order are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* Schema = Required(
			Root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Version = Required(
			Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* EffectId = Required(
			Root, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Entries = Required(
			Root, "entries", DATA_JSON_TYPE::ARRAY);
		if (nullptr == Schema || Schema->Get_String() != TUNING_SCHEMA ||
			nullptr == Version || Version->Was_FloatingPointToken() ||
			Version->Get_Number() !=
				static_cast<double>(EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION) ||
			nullptr == EffectId || nullptr == Entries ||
			Entries->Get_Array().size() > 4096u)
		{
			strOutError = "Occurrence tuning root identity is invalid.";
			return false;
		}

		EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged;
		Staged.iFormatVersion = EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION;
		Staged.strEffectAssetId = EffectId->Get_String();
		Staged.Entries.reserve(Entries->Get_Array().size());
		for (size_t Index = 0u; Index < Entries->Get_Array().size(); ++Index)
		{
			const DATA_JSON_VALUE& Row = Entries->Get_Array()[Index];
			if (!Has_ExactOrderedKeys(Row, {
				"occurrenceId", "sourceOccurrenceRowSha256", "provenance",
				"effectiveLocalTransform" }))
			{
				strOutError = "Occurrence tuning entry fields or order are invalid.";
				return false;
			}
			const DATA_JSON_VALUE* OccurrenceId = Required(
				Row, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* RowSha = Required(
				Row, "sourceOccurrenceRowSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Provenance = Required(
				Row, "provenance", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Transform = Required(
				Row, "effectiveLocalTransform", DATA_JSON_TYPE::OBJECT);
			if (nullptr == OccurrenceId || nullptr == RowSha ||
				nullptr == Provenance || nullptr == Transform ||
				!Has_ExactOrderedKeys(*Transform,
					{ "position", "rotationDegrees", "scale" }))
			{
				strOutError = "Occurrence tuning entry identity is invalid.";
				return false;
			}
			EFFECT_OCCURRENCE_TUNING_ENTRY Parsed;
			Parsed.strOccurrenceId = OccurrenceId->Get_String();
			Parsed.strSourceOccurrenceRowSha256 = RowSha->Get_String();
			Parsed.strProvenance = Provenance->Get_String();
			if (!Read_Float3(*Transform, "position",
					Parsed.EffectiveLocalTransform.vPosition) ||
				!Read_Float3(*Transform, "rotationDegrees",
					Parsed.EffectiveLocalTransform.vRotationDegrees) ||
				!Read_Float3(*Transform, "scale",
					Parsed.EffectiveLocalTransform.vScale))
			{
				strOutError = "Occurrence tuning transform is invalid.";
				return false;
			}
			Staged.Entries.push_back(std::move(Parsed));
		}
		if (!CEffectOccurrenceTuningCodec::Validate(Staged, strOutError))
			return false;
		OutDocument = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Parse_SourceOverlayValue(
		const DATA_JSON_VALUE& Root,
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutDocument,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* Schema = Required(
			Root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Version = Required(
			Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		if (nullptr == Schema || Schema->Get_String() != SOURCE_OVERLAY_SCHEMA ||
			nullptr == Version || Version->Was_FloatingPointToken() ||
			(Version->Get_Number() != 1.0 &&
			 Version->Get_Number() != static_cast<double>(
				 EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION)))
		{
			strOutError = "Source authoring overlay root identity is invalid.";
			return false;
		}
		const bool_t bLegacyV1 = Version->Get_Number() == 1.0;
		if (!(bLegacyV1 ? Has_ExactOrderedKeys(Root, { "schema",
				"formatVersion", "effectAssetId", "sourceProgramSha256",
				"entries" }) :
			Has_ExactOrderedKeys(Root, { "schema", "formatVersion",
				"effectAssetId", "sourceProgramSha256", "entries",
				"supplementalDocument" })))
		{
			strOutError =
				"Source authoring overlay root fields or order are invalid.";
			return false;
		}
		const DATA_JSON_VALUE* EffectId = Required(
			Root, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ProgramSha = Required(
			Root, "sourceProgramSha256", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* Entries = Required(
			Root, "entries", DATA_JSON_TYPE::ARRAY);
		if (nullptr == EffectId || nullptr == ProgramSha || nullptr == Entries ||
			Entries->Get_Array().size() > 4096u)
		{
			strOutError = "Source authoring overlay root identity is invalid.";
			return false;
		}

		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
		Staged.iFormatVersion = EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION;
		Staged.strEffectAssetId = EffectId->Get_String();
		Staged.strSourceProgramSha256 = ProgramSha->Get_String();
		Staged.SupplementalDocument =
			CEffectSourceAuthoringOverlayCodec::Create_EmptySupplementalDocument(
				Staged.strEffectAssetId);
		Staged.Entries.reserve(Entries->Get_Array().size());
		for (const DATA_JSON_VALUE& Row : Entries->Get_Array())
		{
			const bool_t bEntryFieldsValid = bLegacyV1 ?
				Has_ExactOrderedKeys(Row, { "occurrenceId",
					"sourceOccurrenceRowSha256", "sourceElementId",
					"provenance", "effectiveLocalTransform" }) :
				Has_ExactOrderedKeys(Row, { "occurrenceId",
					"sourceOccurrenceRowSha256", "sourceElementId",
					"provenance", "visible", "effectiveLocalTransform" });
			if (!bEntryFieldsValid)
			{
				strOutError =
					"Source authoring overlay entry fields or order are invalid.";
				return false;
			}
			const DATA_JSON_VALUE* OccurrenceId = Required(
				Row, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* RowSha = Required(
				Row, "sourceOccurrenceRowSha256", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* SourceElementId = Required(
				Row, "sourceElementId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Provenance = Required(
				Row, "provenance", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* Visible = bLegacyV1 ? nullptr : Required(
				Row, "visible", DATA_JSON_TYPE::BOOLEAN);
			const DATA_JSON_VALUE* Transform = Required(
				Row, "effectiveLocalTransform", DATA_JSON_TYPE::OBJECT);
			if (nullptr == OccurrenceId || nullptr == RowSha ||
				nullptr == SourceElementId || nullptr == Provenance ||
				(!bLegacyV1 && nullptr == Visible) || nullptr == Transform ||
				!Has_ExactOrderedKeys(*Transform,
					{ "position", "rotationDegrees", "scale" }))
			{
				strOutError =
					"Source authoring overlay entry identity is invalid.";
				return false;
			}
			EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY Parsed;
			Parsed.strOccurrenceId = OccurrenceId->Get_String();
			Parsed.strSourceOccurrenceRowSha256 = RowSha->Get_String();
			Parsed.strSourceElementId = SourceElementId->Get_String();
			Parsed.strProvenance = Provenance->Get_String();
			Parsed.bVisible = bLegacyV1 ? true : Visible->Get_Boolean();
			if (!Read_Float3(*Transform, "position",
					Parsed.EffectiveLocalTransform.vPosition) ||
				!Read_Float3(*Transform, "rotationDegrees",
					Parsed.EffectiveLocalTransform.vRotationDegrees) ||
				!Read_Float3(*Transform, "scale",
					Parsed.EffectiveLocalTransform.vScale))
			{
				strOutError = "Source authoring overlay transform is invalid.";
				return false;
			}
			Staged.Entries.push_back(std::move(Parsed));
		}
		if (!bLegacyV1)
		{
			const DATA_JSON_VALUE* Supplemental = Required(
				Root, "supplementalDocument", DATA_JSON_TYPE::OBJECT);
			if (nullptr == Supplemental || !CEffectDocumentCodec::Parse_Value(
					*Supplemental, Staged.SupplementalDocument, strOutError))
			{
				if (strOutError.empty())
					strOutError =
						"Source authoring overlay supplemental document is invalid.";
				return false;
			}
		}
		if (!CEffectSourceAuthoringOverlayCodec::Validate(Staged, strOutError))
			return false;
		OutDocument = std::move(Staged);
		strOutError.clear();
		return true;
	}

	bool_t Is_FiniteBounded3(
		const float3_t& Value,
		const float Minimum,
		const float Maximum)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && Value.x >= Minimum && Value.x <= Maximum &&
			Value.y >= Minimum && Value.y <= Maximum &&
			Value.z >= Minimum && Value.z <= Maximum;
	}

	void Write_Float(std::ostream& Output, const float Value)
	{
		Output << std::setprecision(std::numeric_limits<float>::max_digits10)
			<< Value;
		if (std::floor(Value) == Value)
			Output << ".0";
	}

	void Write_Float3(std::ostream& Output, const float3_t& Value)
	{
		Output << '[';
		Write_Float(Output, Value.x);
		Output << ", ";
		Write_Float(Output, Value.y);
		Output << ", ";
		Write_Float(Output, Value.z);
		Output << ']';
	}

	void Write_IndentedJson(
		std::ostream& Output,
		const std::string_view Json,
		const size_t iIndent)
	{
		const std::string Indent(iIndent, ' ');
		size_t iLineStart = 0u;
		while (iLineStart < Json.size())
		{
			const size_t iLineEnd = Json.find('\n', iLineStart);
			const size_t iCount = std::string_view::npos == iLineEnd ?
				Json.size() - iLineStart : iLineEnd - iLineStart;
			Output << Indent << Json.substr(iLineStart, iCount);
			if (std::string_view::npos == iLineEnd)
				break;
			Output << '\n';
			iLineStart = iLineEnd + 1u;
		}
	}

	std::filesystem::path Make_TransactionPath(
		const std::filesystem::path& Destination,
		const std::wstring_view Role)
	{
		static std::atomic_uint64_t Counter = 0u;
		const uint64_t Index = Counter.fetch_add(1u, std::memory_order_relaxed);
		const auto Clock = std::chrono::steady_clock::now()
			.time_since_epoch().count();
		return Destination.wstring() + L"." + std::wstring(Role) + L"." +
			std::to_wstring(Clock) + L"." + std::to_wstring(Index);
	}

	bool_t Assign_ProgramFloat3(
		const std::array<double, 3u>& Source,
		float3_t& OutValue)
	{
		for (const double Component : Source)
		{
			if (!std::isfinite(Component) ||
				Component < -static_cast<double>(FLT_MAX) ||
				Component > static_cast<double>(FLT_MAX))
			{
				return false;
			}
		}
		OutValue = { static_cast<float>(Source[0]), static_cast<float>(Source[1]),
			static_cast<float>(Source[2]) };
		return true;
	}

	template <typename ValidateFn>
	bool_t Save_AtomicValidated(
		const std::filesystem::path& Path,
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string_view strExpectedCanonicalDocument,
		ValidateFn&& ValidateDocument,
		std::string& strOutError)
	{
		if (!ValidateDocument(Document, strOutError))
			return false;
		std::error_code Error;
		std::filesystem::create_directories(Path.parent_path(), Error);
		if (Error)
		{
			strOutError =
				"Occurrence tuning authoring directory creation failed.";
			return false;
		}
		const std::filesystem::path Temporary =
			Make_TransactionPath(Path, L"tmp");
		const std::filesystem::path Backup =
			Make_TransactionPath(Path, L"bak");
		const std::string Json =
			CEffectOccurrenceTuningCodec::Serialize(Document);
		{
			std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
			Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
			Output.flush();
			if (!Output)
			{
				strOutError = "Occurrence tuning temporary write failed.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		EFFECT_OCCURRENCE_TUNING_DOCUMENT RoundTrip;
		if (!CEffectOccurrenceTuningCodec::Load(
				Temporary, RoundTrip, strOutError) ||
			!ValidateDocument(RoundTrip, strOutError) ||
			CEffectOccurrenceTuningCodec::Serialize(RoundTrip) != Json)
		{
			if (strOutError.empty())
			{
				strOutError =
					"Occurrence tuning temporary round-trip changed data.";
			}
			std::filesystem::remove(Temporary, Error);
			return false;
		}

		Error.clear();
		const bool_t bDestinationExists =
			std::filesystem::exists(Path, Error);
		if (Error)
		{
			strOutError =
				"Occurrence tuning destination state could not be checked.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
		if (strExpectedCanonicalDocument.empty())
		{
			if (bDestinationExists)
			{
				strOutError =
					"Occurrence tuning appeared after this session began; reload before saving.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		else
		{
			EFFECT_OCCURRENCE_TUNING_DOCUMENT Current;
			std::string CurrentError;
			if (!bDestinationExists ||
				!CEffectOccurrenceTuningCodec::Load(Path, Current, CurrentError) ||
				CEffectOccurrenceTuningCodec::Serialize(Current) !=
					strExpectedCanonicalDocument)
			{
				strOutError =
					"Occurrence tuning changed on disk; reload before saving.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}

		if (bDestinationExists)
		{
			Error.clear();
			std::filesystem::rename(Path, Backup, Error);
			if (Error)
			{
				strOutError = "Occurrence tuning destination backup failed.";
				std::filesystem::remove(Temporary, Error);
				return false;
			}
		}
		Error.clear();
		std::filesystem::rename(Temporary, Path, Error);
		if (Error)
		{
			std::error_code RestoreError;
			if (bDestinationExists)
				std::filesystem::rename(Backup, Path, RestoreError);
			std::filesystem::remove(Temporary, RestoreError);
			strOutError = RestoreError ?
				"Occurrence tuning promote and rollback failed." :
				"Occurrence tuning promote failed.";
			return false;
		}
		std::filesystem::remove(Backup, Error);
		strOutError.clear();
		return true;
	}

	struct VISUAL_TUNING_TARGET final
	{
		std::string strRowSha256;
		std::string strTargetElementId;
		bool_t bTuningEligibleTransform = false;
	};

	bool_t Collect_VisualTuningTargets(
		const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection,
		std::map<std::string, VISUAL_TUNING_TARGET, std::less<>>& OutTargets,
		std::string& strOutError)
	{
		OutTargets.clear();
		for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Projection.Get_AdmittedRows())
		{
			if (!Row.TargetIdentity.has_value() ||
				!OutTargets.emplace(Row.Selector.strOccurrenceId,
					VISUAL_TUNING_TARGET{ Row.strRowSha256,
						Row.TargetIdentity->strTargetElementId,
						Row.bTuningEligibleTransform }).second)
			{
				strOutError =
					"Visual projection contains a malformed or duplicate occurrence.";
				return false;
			}
		}
		for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row :
			Projection.Get_AdmittedSupplementalElements())
		{
			if (Row.TargetIdentity.strTargetElementId.empty() ||
				!OutTargets.emplace(Row.Selector.strOccurrenceId,
					VISUAL_TUNING_TARGET{ Row.strRowSha256,
						Row.TargetIdentity.strTargetElementId,
						Row.bTuningEligibleTransform }).second)
			{
				strOutError =
					"Visual projection contains a malformed or duplicate supplemental occurrence.";
				return false;
			}
		}
		return true;
	}
}

bool_t Client::CEffectOccurrenceTuningCodec::Parse(
	const std::string_view Utf8Json,
	EFFECT_OCCURRENCE_TUNING_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	if (Utf8Json.size() > MAXIMUM_TUNING_BYTES)
	{
		strOutError = "Occurrence tuning exceeds its byte limit.";
		return false;
	}
	if (Utf8Json.size() >= 3u &&
		static_cast<unsigned char>(Utf8Json[0]) == 0xefu &&
		static_cast<unsigned char>(Utf8Json[1]) == 0xbbu &&
		static_cast<unsigned char>(Utf8Json[2]) == 0xbfu)
	{
		strOutError = "Occurrence tuning UTF-8 BOM is forbidden.";
		return false;
	}
	if (Utf8Json.find('\r') != std::string_view::npos)
	{
		strOutError = "Occurrence tuning must use LF-only newlines.";
		return false;
	}
	DATA_JSON_PARSE_LIMITS Limits;
	Limits.iMaximumBytes = MAXIMUM_TUNING_BYTES;
	Limits.iMaximumDepth = 16u;
	Limits.iMaximumValues = 100'000u;
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Utf8Json, Root, strOutError, Limits))
		return false;
	return Parse_Value(Root, OutDocument, strOutError);
}

bool_t Client::CEffectOccurrenceTuningCodec::Parse_RuntimePayload(
	const DATA_JSON_VALUE& Value,
	const std::string_view strExpectedCanonicalSha256,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>& OutDocument,
	std::string& strOutError)
{
	if (!Is_LowerHexSha256(strExpectedCanonicalSha256) ||
		CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(Value)) !=
			strExpectedCanonicalSha256)
	{
		strOutError = "Occurrence tuning canonical SHA-256 binding is invalid.";
		return false;
	}
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged;
	if (!Parse_Value(Value, Staged, strOutError) ||
		!Validate_AgainstProgram(Staged, Program, strOutError))
	{
		return false;
	}
	OutDocument = std::make_shared<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>(
		std::move(Staged));
	return true;
}

bool_t Client::CEffectOccurrenceTuningCodec::Load(
	const std::filesystem::path& Path,
	EFFECT_OCCURRENCE_TUNING_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	std::error_code SizeError;
	const uintmax_t iFileBytes = std::filesystem::file_size(Path, SizeError);
	if (SizeError || iFileBytes > MAXIMUM_TUNING_BYTES)
	{
		strOutError = SizeError ?
			"Occurrence tuning document size could not be read." :
			"Occurrence tuning exceeds its byte limit.";
		return false;
	}
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		strOutError = "Occurrence tuning document could not be opened.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (!Input.eof() && Input.fail())
	{
		strOutError = "Occurrence tuning document read failed.";
		return false;
	}
	return Parse(Buffer.str(), OutDocument, strOutError);
}

bool_t Client::CEffectOccurrenceTuningCodec::Validate(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION ||
		!Is_StableEffectAssetId(Document.strEffectAssetId) ||
		Document.Entries.size() > 4096u)
	{
		strOutError = "Occurrence tuning document identity is invalid.";
		return false;
	}
	std::set<std::string, std::less<>> OccurrenceIds;
	std::string PreviousId;
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry : Document.Entries)
	{
		if (!Is_RuntimeOccurrenceId(Entry.strOccurrenceId) ||
			!Is_LowerHexSha256(Entry.strSourceOccurrenceRowSha256) ||
			Entry.strProvenance != "PROJECT_TUNED" ||
			!OccurrenceIds.emplace(Entry.strOccurrenceId).second ||
			(!PreviousId.empty() && PreviousId >= Entry.strOccurrenceId) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vPosition, -1000.f, 1000.f) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vRotationDegrees, -360.f, 360.f) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vScale, 0.001f, 100.f))
		{
			strOutError =
				"Occurrence tuning entry is invalid, duplicated, or not sorted.";
			return false;
		}
		PreviousId = Entry.strOccurrenceId;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectOccurrenceTuningCodec::Validate_AgainstProgram(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	if (Document.strEffectAssetId != Program.strRuntimeCatalogAssetId)
	{
		strOutError = "Occurrence tuning effect/program identity mismatch.";
		return false;
	}
	std::map<std::string, const EFFECT_RUNTIME_PROGRAM_EMITTER*, std::less<>>
		Emitters;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		if (!Emitters.emplace(Emitter.Row.strId, &Emitter).second)
		{
			strOutError = "Runtime program contains duplicate occurrence IDs.";
			return false;
		}
	}
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry : Document.Entries)
	{
		const auto Found = Emitters.find(Entry.strOccurrenceId);
		if (Found == Emitters.end())
		{
			strOutError = "Occurrence tuning references an unknown runtime occurrence.";
			return false;
		}
		if (Found->second->Row.strRowSha256 !=
			Entry.strSourceOccurrenceRowSha256)
		{
			strOutError = "Occurrence tuning source row SHA-256 is stale.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	if (!Projection.Is_Valid() ||
		Document.strEffectAssetId != Projection.Get_EffectAssetId() ||
		Projection.Get_Document().strEffectAssetId !=
			Projection.Get_EffectAssetId())
	{
		strOutError = "Occurrence tuning visual projection identity mismatch.";
		return false;
	}
	std::map<std::string, VISUAL_TUNING_TARGET, std::less<>> Targets;
	if (!Collect_VisualTuningTargets(Projection, Targets, strOutError))
		return false;
	std::set<std::string, std::less<>> ElementIds;
	for (const EFFECT_ELEMENT_DESC& Element : Projection.Get_Document().Elements)
	{
		if (!ElementIds.emplace(Element.strElementId).second)
		{
			strOutError = "Visual projection contains duplicate Element IDs.";
			return false;
		}
	}
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry : Document.Entries)
	{
		const auto Found = Targets.find(Entry.strOccurrenceId);
		if (Found == Targets.end())
		{
			strOutError =
				"Occurrence tuning references an unknown visual occurrence.";
			return false;
		}
		const VISUAL_TUNING_TARGET& Target = Found->second;
		if (!Target.bTuningEligibleTransform ||
			Target.strTargetElementId.empty())
		{
			strOutError =
				"Visual occurrence is not admitted for Transform tuning.";
			return false;
		}
		if (Target.strRowSha256 != Entry.strSourceOccurrenceRowSha256)
		{
			strOutError = "Occurrence tuning source row SHA-256 is stale.";
			return false;
		}
		if (!ElementIds.contains(Target.strTargetElementId))
		{
			strOutError =
				"Visual occurrence target Element is missing from its projection.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

std::string Client::CEffectOccurrenceTuningCodec::Serialize(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document)
{
	std::ostringstream Output;
	Output << "{\n"
		<< "  \"schema\": \"" << TUNING_SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION
		<< ",\n"
		<< "  \"effectAssetId\": \""
		<< CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"entries\": [";
	for (size_t Index = 0u; Index < Document.Entries.size(); ++Index)
	{
		const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry = Document.Entries[Index];
		Output << (Index == 0u ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"occurrenceId\": \""
			<< CDataJson::Escape(Entry.strOccurrenceId) << "\",\n"
			<< "      \"sourceOccurrenceRowSha256\": \""
			<< Entry.strSourceOccurrenceRowSha256 << "\",\n"
			<< "      \"provenance\": \"PROJECT_TUNED\",\n"
			<< "      \"effectiveLocalTransform\": {\n"
			<< "        \"position\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vPosition);
		Output << ",\n        \"rotationDegrees\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vRotationDegrees);
		Output << ",\n        \"scale\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vScale);
		Output << "\n      }\n    }";
	}
	Output << (Document.Entries.empty() ? "]\n" : "\n  ]\n") << "}\n";
	return Output.str();
}

bool_t Client::CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError)
{
	return Save_AtomicValidated(Path, Document,
		strExpectedCanonicalDocument,
		[&Program](const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Candidate,
			std::string& Error)
		{
			return Validate_AgainstProgram(Candidate, Program, Error);
		}, strOutError);
}

bool_t Client::CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError)
{
	return Save_AtomicValidated(Path, Document,
		strExpectedCanonicalDocument,
		[&Projection](const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Candidate,
			std::string& Error)
		{
			return Validate_AgainstProjection(Candidate, Projection, Error);
		}, strOutError);
}

bool_t Client::CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
	EFFECT_DOCUMENT_DESC& InOutDocument,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
	std::string& strOutError)
{
	if (!Validate_AgainstProgram(Tuning, Program, strOutError) ||
		InOutDocument.strEffectAssetId != Program.strRuntimeCatalogAssetId)
	{
		if (strOutError.empty())
			strOutError = "Projected occurrence tuning identity mismatch.";
		return false;
	}
	EFFECT_DOCUMENT_DESC StagedDocument = InOutDocument;
	std::map<std::string, EFFECT_ELEMENT_DESC*, std::less<>> Elements;
	for (EFFECT_ELEMENT_DESC& Element : StagedDocument.Elements)
	{
		if (!Elements.emplace(Element.strElementId, &Element).second)
		{
			strOutError = "Projected document contains duplicate Element IDs.";
			return false;
		}
	}
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		const auto Found = Elements.find(Emitter.strSourceElementId);
		if (Found == Elements.end())
		{
			strOutError =
				"Projected document is missing a runtime occurrence Element.";
			return false;
		}
		EFFECT_TRANSFORM_DESC& Transform = Found->second->Detail.Transform;
		/* Build_Document projects the immutable cue-local transform into the
		   Element P/R/S.  DetailTransform is identity for the admitted format-13
		   Artist source and is not the reset baseline. */
		if (!Assign_ProgramFloat3(
				Emitter.CueLocalTransform.vPosition, Transform.vPosition) ||
			!Assign_ProgramFloat3(
				Emitter.CueLocalTransform.vRotationDegrees,
				Transform.vRotationDegrees) ||
			!Assign_ProgramFloat3(
				Emitter.CueLocalTransform.vScale, Transform.vScale))
		{
			strOutError = "Runtime occurrence source transform is non-finite.";
			return false;
		}
	}
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry : Tuning.Entries)
	{
		const auto Emitter = std::find_if(
			Program.Emitters.begin(), Program.Emitters.end(),
			[&Entry](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
			{
				return Candidate.Row.strId == Entry.strOccurrenceId;
			});
		if (Emitter == Program.Emitters.end())
		{
			strOutError = "Occurrence tuning join changed after validation.";
			return false;
		}
		const auto Element = Elements.find(Emitter->strSourceElementId);
		if (Element == Elements.end())
		{
			strOutError = "Occurrence tuning projected Element join is missing.";
			return false;
		}
		Element->second->Detail.Transform.vPosition =
			Entry.EffectiveLocalTransform.vPosition;
		Element->second->Detail.Transform.vRotationDegrees =
			Entry.EffectiveLocalTransform.vRotationDegrees;
		Element->second->Detail.Transform.vScale =
			Entry.EffectiveLocalTransform.vScale;
	}
	InOutDocument = std::move(StagedDocument);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
	EFFECT_DOCUMENT_DESC& InOutDocument,
	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& SourceProjection,
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning,
	std::string& strOutError)
{
	if (InOutDocument.strEffectAssetId != SourceProjection.Get_EffectAssetId() ||
		!Validate_AgainstProjection(Tuning, SourceProjection, strOutError))
	{
		if (strOutError.empty())
			strOutError = "Projected occurrence tuning identity mismatch.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = SourceProjection.Get_Document();
	std::map<std::string, VISUAL_TUNING_TARGET, std::less<>> Targets;
	if (!Collect_VisualTuningTargets(SourceProjection, Targets, strOutError))
		return false;
	std::map<std::string, EFFECT_ELEMENT_DESC*, std::less<>> Elements;
	for (EFFECT_ELEMENT_DESC& Element : StagedDocument.Elements)
	{
		if (!Elements.emplace(Element.strElementId, &Element).second)
		{
			strOutError = "Visual projection contains duplicate Element IDs.";
			return false;
		}
	}
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry : Tuning.Entries)
	{
		const auto Target = Targets.find(Entry.strOccurrenceId);
		if (Target == Targets.end() ||
			!Target->second.bTuningEligibleTransform)
		{
			strOutError =
				"Occurrence tuning visual join changed after validation.";
			return false;
		}
		const auto Element = Elements.find(
			Target->second.strTargetElementId);
		if (Element == Elements.end())
		{
			strOutError = "Occurrence tuning projected Element join is missing.";
			return false;
		}
		Element->second->Detail.Transform.vPosition =
			Entry.EffectiveLocalTransform.vPosition;
		Element->second->Detail.Transform.vRotationDegrees =
			Entry.EffectiveLocalTransform.vRotationDegrees;
		Element->second->Detail.Transform.vScale =
			Entry.EffectiveLocalTransform.vScale;
	}
	if (!CEffectDocumentCodec::Validate(StagedDocument, strOutError))
		return false;
	InOutDocument = std::move(StagedDocument);
	strOutError.clear();
	return true;
}

const Client::EFFECT_OCCURRENCE_TUNING_ENTRY*
Client::CEffectOccurrenceTuningCodec::Find_Entry(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
	const std::string_view strOccurrenceId)
{
	const auto Found = std::lower_bound(
		Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
		[](const EFFECT_OCCURRENCE_TUNING_ENTRY& Entry,
			const std::string_view Id)
		{
			return Entry.strOccurrenceId < Id;
		});
	return Found != Document.Entries.end() &&
		Found->strOccurrenceId == strOccurrenceId ? &*Found : nullptr;
}

Client::EFFECT_DOCUMENT_DESC Client::CEffectSourceAuthoringOverlayCodec::
	Create_EmptySupplementalDocument(const std::string_view strEffectAssetId)
{
	EFFECT_DOCUMENT_DESC Document;
	Document.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Document.iLoadedFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	Document.bSourceContract = false;
	Document.strEffectAssetId = strEffectAssetId;
	Document.strDisplayName = "Artist F Authored Decals";
	return Document;
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Parse(
	const std::string_view Utf8Json,
	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	if (Utf8Json.size() > MAXIMUM_TUNING_BYTES)
	{
		strOutError = "Source authoring overlay exceeds its byte limit.";
		return false;
	}
	if (Utf8Json.size() >= 3u &&
		static_cast<unsigned char>(Utf8Json[0]) == 0xefu &&
		static_cast<unsigned char>(Utf8Json[1]) == 0xbbu &&
		static_cast<unsigned char>(Utf8Json[2]) == 0xbfu)
	{
		strOutError = "Source authoring overlay UTF-8 BOM is forbidden.";
		return false;
	}
	if (Utf8Json.find('\r') != std::string_view::npos)
	{
		strOutError = "Source authoring overlay must use LF-only newlines.";
		return false;
	}
	DATA_JSON_PARSE_LIMITS Limits;
	Limits.iMaximumBytes = MAXIMUM_TUNING_BYTES;
	Limits.iMaximumDepth = 16u;
	Limits.iMaximumValues = 100'000u;
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Utf8Json, Root, strOutError, Limits))
		return false;
	return Parse_SourceOverlayValue(Root, OutDocument, strOutError);
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Load(
	const std::filesystem::path& Path,
	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	std::error_code SizeError;
	const uintmax_t iFileBytes = std::filesystem::file_size(Path, SizeError);
	if (SizeError || iFileBytes > MAXIMUM_TUNING_BYTES)
	{
		strOutError = SizeError ?
			"Source authoring overlay size could not be read." :
			"Source authoring overlay exceeds its byte limit.";
		return false;
	}
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		strOutError = "Source authoring overlay could not be opened.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (!Input.eof() && Input.fail())
	{
		strOutError = "Source authoring overlay read failed.";
		return false;
	}
	return Parse(Buffer.str(), OutDocument, strOutError);
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::
	Validate_SupplementalDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		const std::string_view strExpectedEffectAssetId,
		std::string& strOutError)
{
	if (Document.iFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.iLoadedFormatVersion != EFFECT_AUTHORING_FORMAT_VERSION ||
		Document.bSourceContract ||
		Document.strEffectAssetId != strExpectedEffectAssetId ||
		Document.strDisplayName != "Artist F Authored Decals" ||
		Document.ParticleSystem.fUniformScaleMultiplier != 1.f ||
		Document.ParticleSystem.fYawOffsetDegrees != 0.f ||
		Document.ParticleSystem.fDirectionYawDegrees != 0.f ||
		Document.ParticleSystem.fInitialSpeedMultiplier != 1.f ||
		!Document.ModelCues.empty() || Document.Elements.size() > 64u)
	{
		strOutError =
			"Artist F supplemental Decal document shell is invalid.";
		return false;
	}

	std::string PreviousId;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (Element.eKind != EFFECT_ELEMENT_KIND::DECAL ||
			Element.Renderer.eType != EFFECT_RENDERER_TYPE::END ||
			Element.Renderer.eSourceSpace != EFFECT_SOURCE_SPACE::END ||
			(!PreviousId.empty() && PreviousId >= Element.strElementId))
		{
			strOutError =
				"Artist F supplemental Elements must be sorted generic local Decals.";
			return false;
		}
		PreviousId = Element.strElementId;

		EFFECT_DOCUMENT_DESC OriginalProbe = Document;
		OriginalProbe.Elements.assign(1u, Element);
		EFFECT_DOCUMENT_DESC ClearedProbe = OriginalProbe;
		EFFECT_ELEMENT_DESC& Cleared = ClearedProbe.Elements.front();
		Cleared.strSourceNode.clear();
		Cleared.Renderer = {};
		Cleared.Material.strSourceMaterialPath.clear();
		Cleared.Material.SourceMaterial = {};
		Cleared.ActionCueAttachment = {};
		Cleared.TransformInheritance = {};
		Cleared.SourceRecipe = {};
		Cleared.SourcePresentation = {};
		Cleared.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
		if (CEffectDocumentCodec::Serialize(OriginalProbe) !=
			CEffectDocumentCodec::Serialize(ClearedProbe))
		{
			strOutError =
				"Artist F supplemental Decal carries source or native provenance.";
			return false;
		}
	}

	EFFECT_DOCUMENT_DESC EmptyTarget = Document;
	EmptyTarget.Elements.clear();
	if (!CEffectDocumentCodec::Validate_Drawable(EmptyTarget, strOutError))
		return false;
	if (Document.Elements.empty())
	{
		strOutError.clear();
		return true;
	}
	EFFECT_DOCUMENT_DESC RoundTrip;
	if (!CEffectDocumentCodec::Merge_GenericAuthoredElements(
			EmptyTarget, Document.Elements, RoundTrip, strOutError) ||
		CEffectDocumentCodec::Serialize(RoundTrip) !=
			CEffectDocumentCodec::Serialize(Document))
	{
		if (strOutError.empty())
		{
			strOutError =
				"Artist F supplemental Decal document changed during validation.";
		}
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Validate(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
	std::string& strOutError)
{
	strOutError.clear();
	if (Document.iFormatVersion !=
			EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION ||
		!Is_StableEffectAssetId(Document.strEffectAssetId) ||
		!Is_LowerHexSha256(Document.strSourceProgramSha256) ||
		Document.Entries.size() > 4096u ||
		!Validate_SupplementalDocument(Document.SupplementalDocument,
			Document.strEffectAssetId, strOutError))
	{
		if (strOutError.empty())
			strOutError = "Source authoring overlay identity is invalid.";
		return false;
	}
	std::set<std::string, std::less<>> OccurrenceIds;
	std::string PreviousId;
	for (const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry : Document.Entries)
	{
		if (!Is_RuntimeOccurrenceId(Entry.strOccurrenceId) ||
			!Is_LowerHexSha256(Entry.strSourceOccurrenceRowSha256) ||
			!Is_RuntimeOccurrenceId(Entry.strSourceElementId) ||
			Entry.strProvenance != "PROJECT_TUNED" ||
			!OccurrenceIds.emplace(Entry.strOccurrenceId).second ||
			(!PreviousId.empty() && PreviousId >= Entry.strOccurrenceId) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vPosition, -1000.f, 1000.f) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vRotationDegrees, -360.f, 360.f) ||
			!Is_FiniteBounded3(
				Entry.EffectiveLocalTransform.vScale, 0.001f, 100.f))
		{
			strOutError =
				"Source authoring overlay entry is invalid, duplicated, or not sorted.";
			return false;
		}
		PreviousId = Entry.strOccurrenceId;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	std::string& strOutError)
{
	if (!Validate(Document, strOutError))
		return false;
	if (Document.strEffectAssetId != Program.strRuntimeCatalogAssetId)
	{
		strOutError = "Source authoring overlay effect/Program identity mismatch.";
		return false;
	}
	if (Document.strSourceProgramSha256 != Program.Identity.strProgramSha256)
	{
		strOutError = "Source authoring overlay Program SHA-256 is stale.";
		return false;
	}
	std::map<std::string, const EFFECT_RUNTIME_PROGRAM_EMITTER*, std::less<>>
		Emitters;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		if (!Emitters.emplace(Emitter.Row.strId, &Emitter).second)
		{
			strOutError = "Runtime Program contains duplicate occurrence IDs.";
			return false;
		}
	}
	for (const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry : Document.Entries)
	{
		const auto Found = Emitters.find(Entry.strOccurrenceId);
		if (Found == Emitters.end())
		{
			strOutError =
				"Source authoring overlay references an unknown occurrence.";
			return false;
		}
		if (Found->second->Row.strRowSha256 !=
			Entry.strSourceOccurrenceRowSha256)
		{
			strOutError = "Source authoring overlay row SHA-256 is stale.";
			return false;
		}
		if (Found->second->strSourceElementId != Entry.strSourceElementId)
		{
			strOutError = "Source authoring overlay Element identity is stale.";
			return false;
		}
		if (Entry.bVisible && !Found->second->bVisible)
		{
			strOutError =
				"Source authoring overlay cannot enable a deferred Light/Post row.";
			return false;
		}
	}
	std::set<std::string, std::less<>> SourceElementIds;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
		SourceElementIds.emplace(Emitter.strSourceElementId);
	for (const EFFECT_ELEMENT_DESC& Element :
		Document.SupplementalDocument.Elements)
	{
		if (SourceElementIds.contains(Element.strElementId))
		{
			strOutError =
				"Artist F supplemental Decal collides with a source Element ID.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

std::string Client::CEffectSourceAuthoringOverlayCodec::Serialize(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document)
{
	std::ostringstream Output;
	Output << "{\n"
		<< "  \"schema\": \"" << SOURCE_OVERLAY_SCHEMA << "\",\n"
		<< "  \"formatVersion\": "
		<< EFFECT_SOURCE_AUTHORING_OVERLAY_FORMAT_VERSION << ",\n"
		<< "  \"effectAssetId\": \""
		<< CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"sourceProgramSha256\": \""
		<< Document.strSourceProgramSha256 << "\",\n"
		<< "  \"entries\": [";
	for (size_t Index = 0u; Index < Document.Entries.size(); ++Index)
	{
		const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry =
			Document.Entries[Index];
		Output << (Index == 0u ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"occurrenceId\": \""
			<< CDataJson::Escape(Entry.strOccurrenceId) << "\",\n"
			<< "      \"sourceOccurrenceRowSha256\": \""
			<< Entry.strSourceOccurrenceRowSha256 << "\",\n"
			<< "      \"sourceElementId\": \""
			<< CDataJson::Escape(Entry.strSourceElementId) << "\",\n"
			<< "      \"provenance\": \"PROJECT_TUNED\",\n"
			<< "      \"visible\": "
			<< (Entry.bVisible ? "true" : "false") << ",\n"
			<< "      \"effectiveLocalTransform\": {\n"
			<< "        \"position\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vPosition);
		Output << ",\n        \"rotationDegrees\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vRotationDegrees);
		Output << ",\n        \"scale\": ";
		Write_Float3(Output, Entry.EffectiveLocalTransform.vScale);
		Output << "\n      }\n    }";
	}
	Output << (Document.Entries.empty() ? "],\n" : "\n  ],\n")
		<< "  \"supplementalDocument\":\n";
	Write_IndentedJson(Output,
		CEffectDocumentCodec::Serialize(Document.SupplementalDocument), 2u);
	Output << "}\n";
	return Output.str();
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Save_AtomicIfUnchanged(
	const std::filesystem::path& Path,
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const std::string_view strExpectedCanonicalDocument,
	std::string& strOutError)
{
	if (!Validate_AgainstProgram(Document, Program, strOutError))
		return false;
	std::error_code Error;
	std::filesystem::create_directories(Path.parent_path(), Error);
	if (Error)
	{
		strOutError =
			"Source authoring overlay directory creation failed.";
		return false;
	}
	const std::filesystem::path Temporary =
		Make_TransactionPath(Path, L"source-overlay-tmp");
	const std::filesystem::path Backup =
		Make_TransactionPath(Path, L"source-overlay-bak");
	const std::string Json = Serialize(Document);
	{
		std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
		Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
		Output.flush();
		if (!Output)
		{
			strOutError = "Source authoring overlay temporary write failed.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT RoundTrip;
	if (!Load(Temporary, RoundTrip, strOutError) ||
		!Validate_AgainstProgram(RoundTrip, Program, strOutError) ||
		Serialize(RoundTrip) != Json)
	{
		if (strOutError.empty())
			strOutError = "Source authoring overlay round-trip changed data.";
		std::filesystem::remove(Temporary, Error);
		return false;
	}

	Error.clear();
	const bool_t bDestinationExists = std::filesystem::exists(Path, Error);
	if (Error)
	{
		strOutError = "Source authoring overlay destination check failed.";
		std::filesystem::remove(Temporary, Error);
		return false;
	}
	if (strExpectedCanonicalDocument.empty())
	{
		if (bDestinationExists)
		{
			strOutError =
				"Source authoring overlay appeared on disk; reload before saving.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	else
	{
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Current;
		std::string CurrentError;
		if (!bDestinationExists || !Load(Path, Current, CurrentError) ||
			Serialize(Current) != strExpectedCanonicalDocument)
		{
			strOutError =
				"Source authoring overlay changed on disk; reload before saving.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	if (bDestinationExists)
	{
		Error.clear();
		std::filesystem::rename(Path, Backup, Error);
		if (Error)
		{
			strOutError = "Source authoring overlay backup failed.";
			std::filesystem::remove(Temporary, Error);
			return false;
		}
	}
	Error.clear();
	std::filesystem::rename(Temporary, Path, Error);
	if (Error)
	{
		std::error_code RestoreError;
		if (bDestinationExists)
			std::filesystem::rename(Backup, Path, RestoreError);
		std::filesystem::remove(Temporary, RestoreError);
		strOutError = RestoreError ?
			"Source authoring overlay promote and rollback failed." :
			"Source authoring overlay promote failed.";
		return false;
	}
	std::filesystem::remove(Backup, Error);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Apply_ToProjectedDocument(
	EFFECT_DOCUMENT_DESC& InOutDocument,
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay,
	std::string& strOutError)
{
	if (!Validate_AgainstProgram(Overlay, Program, strOutError))
		return false;
	EFFECT_OCCURRENCE_TUNING_DOCUMENT TransformTuning;
	TransformTuning.strEffectAssetId = Overlay.strEffectAssetId;
	TransformTuning.Entries.reserve(Overlay.Entries.size());
	for (const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry : Overlay.Entries)
	{
		EFFECT_OCCURRENCE_TUNING_ENTRY TransformEntry;
		TransformEntry.strOccurrenceId = Entry.strOccurrenceId;
		TransformEntry.strSourceOccurrenceRowSha256 =
			Entry.strSourceOccurrenceRowSha256;
		TransformEntry.strProvenance = Entry.strProvenance;
		TransformEntry.EffectiveLocalTransform = Entry.EffectiveLocalTransform;
		TransformTuning.Entries.push_back(std::move(TransformEntry));
	}
	EFFECT_DOCUMENT_DESC StagedDocument = InOutDocument;
	std::set<std::string, std::less<>> SourceElementIds;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
		SourceElementIds.emplace(Emitter.strSourceElementId);
	std::erase_if(StagedDocument.Elements,
		[&SourceElementIds](const EFFECT_ELEMENT_DESC& Element)
		{
			return !SourceElementIds.contains(Element.strElementId);
		});
	if (!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
			StagedDocument, Program, TransformTuning, strOutError))
	{
		return false;
	}

	std::map<std::string, EFFECT_ELEMENT_DESC*, std::less<>> Elements;
	for (EFFECT_ELEMENT_DESC& Element : StagedDocument.Elements)
		Elements.emplace(Element.strElementId, &Element);
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		const auto Element = Elements.find(Emitter.strSourceElementId);
		if (Element == Elements.end())
		{
			strOutError =
				"Source authoring overlay projected source Element is missing.";
			return false;
		}
		Element->second->bVisible = Emitter.bVisible;
	}
	for (const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry : Overlay.Entries)
	{
		const auto Element = Elements.find(Entry.strSourceElementId);
		if (Element == Elements.end())
		{
			strOutError =
				"Source authoring overlay visibility join changed after validation.";
			return false;
		}
		Element->second->bVisible = Entry.bVisible;
	}
	StagedDocument.Elements.insert(StagedDocument.Elements.end(),
		Overlay.SupplementalDocument.Elements.begin(),
		Overlay.SupplementalDocument.Elements.end());
	InOutDocument = std::move(StagedDocument);
	strOutError.clear();
	return true;
}

bool_t Client::CEffectSourceAuthoringOverlayCodec::Migrate_FromOccurrenceTuning(
	const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& LegacyTuning,
	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& OutOverlay,
	std::string& strOutError)
{
	if (!CEffectOccurrenceTuningCodec::Validate_AgainstProgram(
		LegacyTuning, Program, strOutError) ||
		!Is_LowerHexSha256(Program.Identity.strProgramSha256))
	{
		if (strOutError.empty())
			strOutError = "Legacy tuning Program identity is invalid.";
		return false;
	}
	std::map<std::string, const EFFECT_RUNTIME_PROGRAM_EMITTER*, std::less<>>
		Emitters;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : Program.Emitters)
	{
		if (!Emitters.emplace(Emitter.Row.strId, &Emitter).second)
		{
			strOutError =
				"Runtime Program contains duplicate migration occurrences.";
			return false;
		}
	}
	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
	Staged.strEffectAssetId = LegacyTuning.strEffectAssetId;
	Staged.strSourceProgramSha256 = Program.Identity.strProgramSha256;
	Staged.SupplementalDocument = Create_EmptySupplementalDocument(
		LegacyTuning.strEffectAssetId);
	Staged.Entries.reserve(LegacyTuning.Entries.size());
	for (const EFFECT_OCCURRENCE_TUNING_ENTRY& Legacy : LegacyTuning.Entries)
	{
		const auto Found = Emitters.find(Legacy.strOccurrenceId);
		if (Found == Emitters.end())
		{
			strOutError = "Legacy tuning migration join changed after validation.";
			return false;
		}
		EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY Entry;
		Entry.strOccurrenceId = Legacy.strOccurrenceId;
		Entry.strSourceOccurrenceRowSha256 =
			Legacy.strSourceOccurrenceRowSha256;
		Entry.strSourceElementId = Found->second->strSourceElementId;
		Entry.strProvenance = Legacy.strProvenance;
		Entry.bVisible = Found->second->bVisible;
		Entry.EffectiveLocalTransform = Legacy.EffectiveLocalTransform;
		Staged.Entries.push_back(std::move(Entry));
	}
	if (!Validate_AgainstProgram(Staged, Program, strOutError))
		return false;
	OutOverlay = std::move(Staged);
	strOutError.clear();
	return true;
}

const Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY*
Client::CEffectSourceAuthoringOverlayCodec::Find_Entry(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
	const std::string_view strOccurrenceId)
{
	const auto Found = std::lower_bound(
		Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
		[](const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry,
			const std::string_view Id)
		{
			return Entry.strOccurrenceId < Id;
		});
	return Found != Document.Entries.end() &&
		Found->strOccurrenceId == strOccurrenceId ? &*Found : nullptr;
}

const Client::EFFECT_ELEMENT_DESC*
Client::CEffectSourceAuthoringOverlayCodec::Find_SupplementalDecal(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
	const std::string_view strElementId)
{
	const auto Found = std::lower_bound(
		Document.SupplementalDocument.Elements.begin(),
		Document.SupplementalDocument.Elements.end(), strElementId,
		[](const EFFECT_ELEMENT_DESC& Element, const std::string_view Id)
		{
			return Element.strElementId < Id;
		});
	return Found != Document.SupplementalDocument.Elements.end() &&
		Found->strElementId == strElementId ? &*Found : nullptr;
}
