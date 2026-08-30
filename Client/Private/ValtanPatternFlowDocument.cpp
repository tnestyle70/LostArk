#include "ValtanPatternFlowDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <bcrypt.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace Client;

	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-boss-audition-flows";
	constexpr std::uint32_t DOCUMENT_VERSION = 1u;
	constexpr std::size_t MAX_DOCUMENT_BYTES = 256u * 1024u;
	constexpr std::size_t MAX_STABLE_ID_LENGTH = 128u;
	constexpr std::uint64_t MAX_SLOT_ORDINAL = 999999u;
	constexpr std::size_t SHA256_BYTE_COUNT = 32u;
	constexpr std::string_view OPTIONAL_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
	constexpr std::string_view OPTIONAL_IDLE_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC_IDLE";

	bool_t Is_OptionalEntryPatternId(const std::string_view patternId)
	{
		return OPTIONAL_ENTRY_PATTERN_ID == patternId ||
			OPTIONAL_IDLE_ENTRY_PATTERN_ID == patternId;
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		for (const std::string_view name : names)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		return true;
	}

	bool_t Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > MAX_STABLE_ID_LENGTH)
			return false;
		for (const unsigned char character : value)
		{
			if (!(character >= 'a' && character <= 'z') &&
				!(character >= 'A' && character <= 'Z') &&
				!(character >= '0' && character <= '9') &&
				character != '_' && character != '-' && character != '.')
			{
				return false;
			}
		}
		return true;
	}

	template <typename T>
	bool_t Try_ParseUnsignedInteger(
		const DATA_JSON_VALUE& value,
		const std::uint64_t maximum,
		T& outValue)
	{
		if (!value.Is_Number() || value.Was_FloatingPointToken())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) ||
			std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<T>(number);
		return true;
	}

	bool_t Try_ParseSlotOrdinal(
		const std::string_view flowId,
		const std::string_view slotId,
		std::uint64_t& outOrdinal)
	{
		const std::string prefix = std::string(flowId) + ".slot.";
		if (!slotId.starts_with(prefix) ||
			slotId.size() != prefix.size() + 6u)
		{
			return false;
		}
		std::uint64_t ordinal = 0u;
		for (std::size_t index = prefix.size(); index < slotId.size(); ++index)
		{
			const char character = slotId[index];
			if (character < '0' || character > '9')
				return false;
			ordinal = ordinal * 10u +
				static_cast<std::uint64_t>(character - '0');
		}
		if (0u == ordinal || ordinal > MAX_SLOT_ORDINAL)
			return false;
		outOrdinal = ordinal;
		return true;
	}

	std::string Build_SlotId(
		const std::string_view flowId,
		const std::uint64_t ordinal)
	{
		std::ostringstream output;
		output << flowId << ".slot." << std::setw(6) << std::setfill('0')
			<< ordinal;
		return output.str();
	}

	bool_t Read_Bytes(
		const std::filesystem::path& path,
		std::string& outBytes,
		std::string& outStatus)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			outStatus = "Valtan Boss Flow document is missing: " + path.string();
			return false;
		}
		std::string staged{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad() || staged.empty() ||
			staged.size() > MAX_DOCUMENT_BYTES)
		{
			outStatus = "Valtan Boss Flow document is empty, oversized, or unreadable.";
			return false;
		}
		outBytes = std::move(staged);
		return true;
	}

	bool_t Compute_Sha256(
		const std::string_view bytes,
		std::array<std::uint8_t, SHA256_BYTE_COUNT>& outDigest)
	{
		if (bytes.size() > static_cast<std::size_t>(ULONG_MAX))
			return false;

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectSize = 0u;
		DWORD resultSize = 0u;
		std::vector<std::uint8_t> object;
		bool_t succeeded = false;
		if (BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) < 0)
		{
			goto cleanup;
		}
		if (BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize),
				&resultSize, 0u) < 0 ||
			resultSize != sizeof(objectSize) || 0u == objectSize)
		{
			goto cleanup;
		}
		object.resize(objectSize);
		if (BCryptCreateHash(
				algorithm, &hash, object.data(), objectSize,
				nullptr, 0u, 0u) < 0)
		{
			goto cleanup;
		}
		if (BCryptHashData(
				hash,
				reinterpret_cast<PUCHAR>(
					const_cast<char*>(bytes.data())),
				static_cast<ULONG>(bytes.size()), 0u) < 0 ||
			BCryptFinishHash(
				hash, outDigest.data(),
				static_cast<ULONG>(outDigest.size()), 0u) < 0)
		{
			goto cleanup;
		}
		succeeded = true;

	cleanup:
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}

	bool_t Build_Revision(
		const std::string_view bytes,
		std::string& outRevision)
	{
		std::array<std::uint8_t, SHA256_BYTE_COUNT> digest{};
		if (!Compute_Sha256(bytes, digest))
			return false;
		constexpr char HEX[] = "0123456789abcdef";
		std::string revision;
		revision.reserve(digest.size() * 2u);
		for (const std::uint8_t byte : digest)
		{
			revision.push_back(HEX[(byte >> 4u) & 0x0fu]);
			revision.push_back(HEX[byte & 0x0fu]);
		}
		outRevision = std::move(revision);
		return true;
	}

	bool_t Write_Durable(
		const std::filesystem::path& path,
		const std::string_view bytes)
	{
		FILE* file = nullptr;
		if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
			return false;
		const bool_t wrote = bytes.size() ==
			fwrite(bytes.data(), 1u, bytes.size(), file);
		const bool_t flushed = 0 == fflush(file) &&
			0 == _commit(_fileno(file));
		const bool_t closed = 0 == fclose(file);
		return wrote && flushed && closed;
	}

	void Remove_BestEffort(const std::filesystem::path& path)
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}

	bool_t Documents_AreEqual(
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& left,
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& right)
	{
		return left == right;
	}
}

std::filesystem::path Client::CValtanPatternFlowDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Encounters/Valtan/ValtanBossAuditionFlows.json"));
}

bool_t Client::CValtanPatternFlowDocument::Compute_SourceRevision(
	const std::string_view text,
	std::string& outRevision)
{
	return Build_Revision(text, outRevision);
}

bool_t Client::CValtanPatternFlowDocument::Parse_Text(
	const std::string_view text,
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_PARSE_LIMITS limits;
	limits.iMaximumBytes = MAX_DOCUMENT_BYTES;
	limits.iMaximumDepth = 8u;
	// Root/flow metadata consumes fewer than sixteen values and every slot
	// contributes one object plus two strings.
	limits.iMaximumValues = 16u + MAX_SLOTS * 3u;
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root, { "schema", "formatVersion", "flows" }))
	{
		outStatus = "Valtan Boss Flow JSON is malformed or has unknown properties: " +
			parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* flows = Required(
		root, "flows", DATA_JSON_TYPE::ARRAY);
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged;
	if (nullptr == schema || schema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsignedInteger(*version, DOCUMENT_VERSION,
			staged.iFormatVersion) ||
		staged.iFormatVersion != DOCUMENT_VERSION ||
		nullptr == flows || 1u != flows->Get_Array().size())
	{
		outStatus = "Valtan Boss Flow header or version is invalid.";
		return false;
	}

	for (const DATA_JSON_VALUE& flowValue : flows->Get_Array())
	{
		if (!Has_ExactProperties(flowValue,
			{ "flowId", "nextSlotOrdinal", "interStepPursuitMs", "slots" }))
		{
			outStatus = "Valtan Boss Flow row has unknown or missing properties.";
			return false;
		}
		const DATA_JSON_VALUE* flowId = Required(
			flowValue, "flowId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* nextSlotOrdinal = Required(
			flowValue, "nextSlotOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pursuit = Required(
			flowValue, "interStepPursuitMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* slots = Required(
			flowValue, "slots", DATA_JSON_TYPE::ARRAY);
		VALTAN_PATTERN_FLOW_DEFINITION stagedFlow;
		if (nullptr == flowId || !Is_StableId(flowId->Get_String()) ||
			nullptr == nextSlotOrdinal ||
			!Try_ParseUnsignedInteger(
				*nextSlotOrdinal, MAX_SLOT_ORDINAL + 1u,
				stagedFlow.iNextSlotOrdinal) ||
			nullptr == pursuit ||
			!Try_ParseUnsignedInteger(
				*pursuit,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedFlow.iInterStepPursuitMs) ||
			nullptr == slots || slots->Get_Array().size() > MAX_SLOTS)
		{
			outStatus = "Valtan Boss Flow row is invalid or oversized.";
			return false;
		}
		stagedFlow.strFlowId = flowId->Get_String();
		stagedFlow.Slots.reserve(slots->Get_Array().size());
		for (const DATA_JSON_VALUE& slotValue : slots->Get_Array())
		{
			if (!Has_ExactProperties(slotValue, { "slotId", "patternId" }))
			{
				outStatus = "Valtan Boss Flow slot has unknown or missing properties.";
				return false;
			}
			const DATA_JSON_VALUE* slotId = Required(
				slotValue, "slotId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* patternId = Required(
				slotValue, "patternId", DATA_JSON_TYPE::STRING);
			if (nullptr == slotId || !Is_StableId(slotId->Get_String()) ||
				nullptr == patternId || !Is_StableId(patternId->Get_String()))
			{
				outStatus = "Valtan Boss Flow slot identity is invalid.";
				return false;
			}
			stagedFlow.Slots.push_back(
				{ slotId->Get_String(), patternId->Get_String() });
		}
		staged.Flows.push_back(std::move(stagedFlow));
	}

	outDocument = std::move(staged);
	outStatus = "Parsed Valtan Boss Flow authoring document.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Validate(
	const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	if (DOCUMENT_VERSION != document.iFormatVersion ||
		1u != document.Flows.size())
	{
		outStatus = "Valtan Boss Flow document version or flow count is invalid.";
		return false;
	}

	std::unordered_set<std::string> admitted;
	admitted.reserve(admittedPatternIds.size());
	for (const std::string& patternId : admittedPatternIds)
	{
		if (!Is_StableId(patternId) || !admitted.insert(patternId).second)
		{
			outStatus = "Valtan Boss Flow inventory is empty, invalid, or duplicate.";
			return false;
		}
	}
	if (admitted.empty())
	{
		outStatus = "Valtan Boss Flow inventory is empty, invalid, or duplicate.";
		return false;
	}

	const VALTAN_PATTERN_FLOW_DEFINITION& flow = document.Flows.front();
	if (flow.Slots.empty())
	{
		outStatus = "The default Valtan Flow must contain at least one slot.";
		return false;
	}
	if (flow.strFlowId != DEFAULT_FLOW_ID ||
		flow.iInterStepPursuitMs < MIN_INTER_STEP_PURSUIT_MS ||
		flow.iInterStepPursuitMs > MAX_INTER_STEP_PURSUIT_MS ||
		0u == flow.iNextSlotOrdinal ||
		flow.iNextSlotOrdinal > MAX_SLOT_ORDINAL + 1u ||
		flow.Slots.size() > MAX_SLOTS)
	{
		outStatus = "Valtan Boss Flow identity, pursuit interval, or size is invalid.";
		return false;
	}

	std::unordered_set<std::string> slotIds;
	std::uint64_t maximumUsedOrdinal = 0u;
	bool_t bHasEntryPattern = false;
	for (std::size_t slotIndex = 0u; slotIndex < flow.Slots.size(); ++slotIndex)
	{
		const VALTAN_PATTERN_FLOW_SLOT& slot = flow.Slots[slotIndex];
		if (Is_OptionalEntryPatternId(slot.strPatternId))
		{
			if (bHasEntryPattern || 0u != slotIndex)
			{
				outStatus =
					"At most one optional entry cinematic may occur, and only at the first step.";
				return false;
			}
			bHasEntryPattern = true;
		}
		std::uint64_t ordinal = 0u;
		if (!Is_StableId(slot.strSlotId) ||
			!Try_ParseSlotOrdinal(flow.strFlowId, slot.strSlotId, ordinal) ||
			!slotIds.insert(slot.strSlotId).second ||
			!Is_StableId(slot.strPatternId) ||
			!admitted.contains(slot.strPatternId))
		{
			outStatus =
				"Valtan Boss Flow slot is duplicate, malformed, or not in All Effects inventory.";
			return false;
		}
		maximumUsedOrdinal = (std::max)(maximumUsedOrdinal, ordinal);
	}
	if (flow.iNextSlotOrdinal <= maximumUsedOrdinal)
	{
		outStatus = "Valtan Boss Flow nextSlotOrdinal would reuse a stable slot ID.";
		return false;
	}

	outStatus = "Validated " + std::to_string(flow.Slots.size()) +
		" Valtan Boss Flow slot(s).";
	return true;
}

std::string Client::CValtanPatternFlowDocument::Serialize_Text(
	const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document)
{
	std::ostringstream output;
	output << "{\n"
		<< "  \"schema\": \"" << DOCUMENT_SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << document.iFormatVersion << ",\n"
		<< "  \"flows\": [\n";
	for (std::size_t flowIndex = 0u;
		flowIndex < document.Flows.size(); ++flowIndex)
	{
		const VALTAN_PATTERN_FLOW_DEFINITION& flow =
			document.Flows[flowIndex];
		output << "    {\n"
			<< "      \"flowId\": \""
			<< CDataJson::Escape(flow.strFlowId) << "\",\n"
			<< "      \"nextSlotOrdinal\": "
			<< flow.iNextSlotOrdinal << ",\n"
			<< "      \"interStepPursuitMs\": "
			<< flow.iInterStepPursuitMs << ",\n"
			<< "      \"slots\": [\n";
		for (std::size_t slotIndex = 0u;
			slotIndex < flow.Slots.size(); ++slotIndex)
		{
			const VALTAN_PATTERN_FLOW_SLOT& slot = flow.Slots[slotIndex];
			output << "        { \"slotId\": \""
				<< CDataJson::Escape(slot.strSlotId)
				<< "\", \"patternId\": \""
				<< CDataJson::Escape(slot.strPatternId) << "\" }"
				<< (slotIndex + 1u < flow.Slots.size() ? "," : "")
				<< "\n";
		}
		output << "      ]\n"
			<< "    }"
			<< (flowIndex + 1u < document.Flows.size() ? "," : "")
			<< "\n";
	}
	output << "  ]\n}\n";
	return output.str();
}

bool_t Client::CValtanPatternFlowDocument::Load(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const std::filesystem::path path = Resolve_Path();
	std::string bytes;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged;
	std::string revision;
	if (path.empty())
	{
		outStatus = "Could not resolve the Valtan Boss Flow document path.";
		return false;
	}
	if (!Read_Bytes(path, bytes, outStatus) ||
		!Parse_Text(bytes, staged, outStatus) ||
		!Validate(staged, admittedPatternIds, outStatus))
	{
		return false;
	}
	if (!Build_Revision(bytes, revision))
	{
		outStatus = "Could not compute the Valtan Boss Flow source revision.";
		return false;
	}

	m_Path = path;
	m_Baseline = staged;
	m_Draft = std::move(staged);
	m_strSourceRevision = std::move(revision);
	m_bReady = true;
	m_bExternalConflict = false;
	outStatus = "Loaded Valtan Boss Flow revision " + m_strSourceRevision + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Reload(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	return Load(admittedPatternIds, outStatus);
}

bool_t Client::CValtanPatternFlowDocument::Save(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || m_Path.empty())
	{
		outStatus = "Valtan Boss Flow must be loaded before Save.";
		return false;
	}
	if (!Validate(m_Draft, admittedPatternIds, outStatus))
		return false;

	std::string diskBytes;
	std::string diskRevision;
	if (!Read_Bytes(m_Path, diskBytes, outStatus))
	{
		return false;
	}
	if (!Build_Revision(diskBytes, diskRevision))
	{
		outStatus = "Could not compute the current Valtan Boss Flow revision.";
		return false;
	}
	if (diskRevision != m_strSourceRevision)
	{
		m_bExternalConflict = true;
		outStatus = "Valtan Boss Flow changed on disk; Reload before Save.";
		return false;
	}

	const std::string serialized = Serialize_Text(m_Draft);
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT verifiedDraft;
	std::string verifyStatus;
	if (!Parse_Text(serialized, verifiedDraft, verifyStatus) ||
		!Validate(verifiedDraft, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, verifiedDraft))
	{
		outStatus = "Valtan Boss Flow serialization verification failed: " +
			verifyStatus;
		return false;
	}

	const std::wstring uniqueSuffix =
		L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64());
	std::filesystem::path temporary = m_Path;
	temporary += L".tmp" + uniqueSuffix;
	std::filesystem::path backup = m_Path;
	backup += L".backup" + uniqueSuffix;
	if (!Write_Durable(temporary, serialized))
	{
		Remove_BestEffort(temporary);
		outStatus = "Could not durably write the temporary Valtan Boss Flow document.";
		return false;
	}

	std::string temporaryBytes;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT temporaryDocument;
	if (!Read_Bytes(temporary, temporaryBytes, verifyStatus) ||
		temporaryBytes != serialized ||
		!Parse_Text(temporaryBytes, temporaryDocument, verifyStatus) ||
		!Validate(temporaryDocument, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, temporaryDocument))
	{
		Remove_BestEffort(temporary);
		outStatus = "Valtan Boss Flow temporary verification failed: " +
			verifyStatus;
		return false;
	}

	if (!CopyFileW(m_Path.c_str(), backup.c_str(), TRUE))
	{
		Remove_BestEffort(temporary);
		outStatus = "Could not create the Valtan Boss Flow recovery backup.";
		return false;
	}

	std::string preCommitBytes;
	std::string preCommitRevision;
	if (!Read_Bytes(m_Path, preCommitBytes, verifyStatus) ||
		!Build_Revision(preCommitBytes, preCommitRevision) ||
		preCommitRevision != m_strSourceRevision)
	{
		Remove_BestEffort(temporary);
		Remove_BestEffort(backup);
		m_bExternalConflict = true;
		outStatus = "Valtan Boss Flow changed during Save; disk and draft were preserved.";
		return false;
	}

	if (!MoveFileExW(
			temporary.c_str(), m_Path.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_BestEffort(temporary);
		Remove_BestEffort(backup);
		outStatus = "Could not atomically replace the Valtan Boss Flow document.";
		return false;
	}

	std::string committedBytes;
	std::string committedRevision;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT committedDocument;
	if (!Read_Bytes(m_Path, committedBytes, verifyStatus) ||
		committedBytes != serialized ||
		!Parse_Text(committedBytes, committedDocument, verifyStatus) ||
		!Validate(committedDocument, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, committedDocument) ||
		!Build_Revision(committedBytes, committedRevision))
	{
		if (MoveFileExW(
				backup.c_str(), m_Path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			outStatus =
				"Valtan Boss Flow post-replace verification failed; the baseline was restored: " +
				verifyStatus;
		}
		else
		{
			m_bExternalConflict = true;
			outStatus =
				"Valtan Boss Flow post-replace verification and rollback failed; recovery backup remains at " +
				backup.string() + ": " + verifyStatus;
		}
		return false;
	}

	Remove_BestEffort(backup);
	m_Baseline = committedDocument;
	m_Draft = std::move(committedDocument);
	m_strSourceRevision = std::move(committedRevision);
	m_bExternalConflict = false;
	outStatus = "Saved Valtan Boss Flow revision " + m_strSourceRevision + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Verify_SourceRevision(
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || m_Path.empty() || m_strSourceRevision.empty())
	{
		outStatus =
			"Valtan Boss Flow must be loaded before checking its source revision.";
		return false;
	}

	std::string diskBytes;
	std::string diskRevision;
	if (!Read_Bytes(m_Path, diskBytes, outStatus))
	{
		m_bExternalConflict = true;
		return false;
	}
	if (!Build_Revision(diskBytes, diskRevision))
	{
		m_bExternalConflict = true;
		outStatus = "Could not compute the current Valtan Boss Flow revision.";
		return false;
	}
	if (diskRevision != m_strSourceRevision)
	{
		m_bExternalConflict = true;
		outStatus =
			"Valtan Boss Flow changed on disk; Reload before Server playback.";
		return false;
	}

	m_bExternalConflict = false;
	outStatus = "Valtan Boss Flow source revision is current.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Add_Slot(
	const std::string_view patternId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outSlotId,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current || current->Slots.size() >= MAX_SLOTS ||
		current->iNextSlotOrdinal > MAX_SLOT_ORDINAL ||
		admittedPatternIds.end() == std::find(
			admittedPatternIds.begin(), admittedPatternIds.end(), patternId))
	{
		outStatus = "Valtan Boss Flow cannot add this pattern or has reached 255 slots.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const std::string slotId = Build_SlotId(
		flow.strFlowId, flow.iNextSlotOrdinal);
	const bool_t bEntryPattern = Is_OptionalEntryPatternId(patternId);
	const VALTAN_PATTERN_FLOW_SLOT StagedSlot{
		slotId, std::string(patternId) };
	if (bEntryPattern)
		flow.Slots.insert(flow.Slots.begin(), StagedSlot);
	else
		flow.Slots.push_back(StagedSlot);
	++flow.iNextSlotOrdinal;
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outSlotId = slotId;
	outStatus = "Added " + std::string(patternId) + " as " + slotId +
		(bEntryPattern ? " at the first Flow slot." : ".");
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Move_Slot(
	const std::string_view slotId,
	const std::int32_t delta,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == current || (delta != -1 && delta != 1))
	{
		outStatus = "Valtan Boss Flow move requires a loaded flow and delta -1 or +1.";
		return false;
	}
	const auto found = std::find_if(
		current->Slots.begin(), current->Slots.end(),
		[slotId](const VALTAN_PATTERN_FLOW_SLOT& slot)
		{
			return slot.strSlotId == slotId;
		});
	if (found == current->Slots.end())
	{
		outStatus = "Valtan Boss Flow slot no longer exists.";
		return false;
	}
	const std::ptrdiff_t source = found - current->Slots.begin();
	const std::ptrdiff_t destination = source + delta;
	if (destination < 0 ||
		destination >= static_cast<std::ptrdiff_t>(current->Slots.size()))
	{
		outStatus = "Valtan Boss Flow slot is already at that boundary.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	std::iter_swap(
		flow.Slots.begin() + source,
		flow.Slots.begin() + destination);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Moved Valtan Boss Flow slot " + std::string(slotId) + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Remove_Slot(
	const std::string_view slotId,
	std::string& outStatus)
{
	outStatus.clear();
	VALTAN_PATTERN_FLOW_DEFINITION* flow =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == flow)
	{
		outStatus = "Valtan Boss Flow must be loaded before removing a slot.";
		return false;
	}
	const auto found = std::find_if(
		flow->Slots.begin(), flow->Slots.end(),
		[slotId](const VALTAN_PATTERN_FLOW_SLOT& slot)
		{
			return slot.strSlotId == slotId;
		});
	if (found == flow->Slots.end())
	{
		outStatus = "Valtan Boss Flow slot no longer exists.";
		return false;
	}
	flow->Slots.erase(found);
	outStatus = "Removed Valtan Boss Flow slot " + std::string(slotId) + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_InterStepPursuitMs(
	const std::uint32_t milliseconds,
	std::string& outStatus)
{
	outStatus.clear();
	VALTAN_PATTERN_FLOW_DEFINITION* flow =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == flow || milliseconds < MIN_INTER_STEP_PURSUIT_MS ||
		milliseconds > MAX_INTER_STEP_PURSUIT_MS)
	{
		outStatus = "Valtan Boss Flow pursuit interval must be 100..10000 ms.";
		return false;
	}
	flow->iInterStepPursuitMs = milliseconds;
	outStatus = "Updated Valtan Boss Flow pursuit interval.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Is_Dirty() const noexcept
{
	return m_bReady && !Documents_AreEqual(m_Baseline, m_Draft);
}

const Client::VALTAN_PATTERN_FLOW_DEFINITION*
Client::CValtanPatternFlowDocument::Get_DefaultFlow() const noexcept
{
	return m_bReady && 1u == m_Draft.Flows.size() ?
		&m_Draft.Flows.front() : nullptr;
}
