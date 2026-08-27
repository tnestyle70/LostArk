#include "ValtanPatternAuthoringEffectDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <io.h>
#include <iterator>
#include <set>
#include <sstream>
#include <system_error>
#include <utility>
#include <Windows.h>

namespace
{
	using namespace Client;

	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-pattern-authoring-effects";
	constexpr double DOCUMENT_VERSION = 1.0;
	constexpr std::string_view BOSS_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view DRAFT_STATE = "DRAFT_ATTACHED";
	constexpr size_t MAX_BINDINGS = 256u;
	constexpr const wchar_t* TRANSACTION_MUTEX_NAME =
		L"Local\\LostArk.ValtanPatternAuthoringEffects.Transaction.v1";

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Names)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Names.size())
			return false;
		for (const std::string_view Name : Names)
		{
			if (nullptr == Object.Find(Name))
				return false;
		}
		return true;
	}

	bool_t Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				Character != '_' && Character != '-' && Character != '.')
			{
				return false;
			}
		}
		return true;
	}

	bool_t Load_FromPath(
		const std::filesystem::path& Path,
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
		std::string& strOutCanonical,
		std::string& strOutStatus)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus =
				"Valtan pattern authoring Effect document is missing: " +
				Path.string();
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT Staged;
		if (!CValtanPatternAuthoringEffectDocument::Parse_Text(
				Text, Staged, strOutStatus))
		{
			return false;
		}
		strOutCanonical =
			CValtanPatternAuthoringEffectDocument::Serialize(Staged);
		OutDocument = std::move(Staged);
		return true;
	}
}

Client::CValtanPatternAuthoringEffectTransaction::
	~CValtanPatternAuthoringEffectTransaction()
{
	const HANDLE hMutex = static_cast<HANDLE>(m_pMutexHandle);
	if (m_bOwned && nullptr != hMutex)
		ReleaseMutex(hMutex);
	if (nullptr != hMutex)
		CloseHandle(hMutex);
}

bool_t Client::CValtanPatternAuthoringEffectTransaction::Try_Acquire(
	std::string& strOutStatus)
{
	if (m_bOwned)
		return true;
	const HANDLE hMutex = CreateMutexW(
		nullptr, FALSE, TRANSACTION_MUTEX_NAME);
	if (nullptr == hMutex)
	{
		strOutStatus =
			"Could not create the Valtan pattern Effect transaction lock.";
		return false;
	}
	const DWORD iWait = WaitForSingleObject(hMutex, 0u);
	if (WAIT_OBJECT_0 != iWait && WAIT_ABANDONED != iWait)
	{
		CloseHandle(hMutex);
		strOutStatus =
			"Another Valtan pattern Effect transaction is in progress; retry after Refresh.";
		return false;
	}
	m_pMutexHandle = hMutex;
	m_bOwned = true;
	strOutStatus = "Acquired the Valtan pattern Effect transaction lock.";
	return true;
}

std::filesystem::path
Client::CValtanPatternAuthoringEffectDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		L"Effects/ValtanPatternAuthoringEffects.json");
}

std::string Client::CValtanPatternAuthoringEffectDocument::
	Build_AuthoringPath(const std::string_view strEffectAssetId)
{
	if (!Is_StableToken(strEffectAssetId))
		return {};
	return "Effects/Authored/" + std::string(strEffectAssetId) +
		".effect.json";
}

std::filesystem::path Client::CValtanPatternAuthoringEffectDocument::
	Resolve_AuthoringPath(
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding)
{
	if (Binding.strAuthoringPath !=
		Build_AuthoringPath(Binding.strEffectAssetId))
	{
		return {};
	}
	return CProjectDataRoot::Resolve(
		std::filesystem::path(Binding.strAuthoringPath));
}

bool_t Client::CValtanPatternAuthoringEffectDocument::Parse_Text(
	const std::string_view strText,
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
	std::string& strOutStatus)
{
	DATA_JSON_VALUE Root;
	std::string ParseError;
	if (!CDataJson::Parse(strText, Root, ParseError) ||
		!Has_ExactProperties(Root,
			{ "schema", "formatVersion", "bossArchetypeId", "bindings" }))
	{
		strOutStatus =
			"Valtan pattern authoring Effect JSON is malformed: " + ParseError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Required(
		Root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
		Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pBoss = Required(
		Root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pBindings = Required(
		Root, "bindings", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pSchema || pSchema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == pVersion || pVersion->Get_Number() != DOCUMENT_VERSION ||
		nullptr == pBoss || pBoss->Get_String() != BOSS_ARCHETYPE_ID ||
		nullptr == pBindings || pBindings->Get_Array().size() > MAX_BINDINGS)
	{
		strOutStatus = "Valtan pattern authoring Effect header is invalid.";
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT Staged;
	Staged.strBossArchetypeId = pBoss->Get_String();
	Staged.Bindings.reserve(pBindings->Get_Array().size());
	for (const DATA_JSON_VALUE& Value : pBindings->Get_Array())
	{
		if (!Has_ExactProperties(Value,
				{ "patternId", "effectAssetId", "authoringPath", "state" }))
		{
			strOutStatus =
				"Valtan pattern authoring Effect binding has unexpected fields.";
			return false;
		}
		const DATA_JSON_VALUE* pPattern = Required(
			Value, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pEffect = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPath = Required(
			Value, "authoringPath", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pState = Required(
			Value, "state", DATA_JSON_TYPE::STRING);
		if (nullptr == pPattern || nullptr == pEffect || nullptr == pPath ||
			nullptr == pState)
		{
			strOutStatus =
				"Valtan pattern authoring Effect binding is incomplete.";
			return false;
		}
		VALTAN_PATTERN_AUTHORING_EFFECT_BINDING Binding;
		Binding.strPatternId = pPattern->Get_String();
		Binding.strEffectAssetId = pEffect->Get_String();
		Binding.strAuthoringPath = pPath->Get_String();
		Binding.strState = pState->Get_String();
		Staged.Bindings.push_back(std::move(Binding));
	}

	if (!Validate(Staged, strOutStatus))
		return false;
	OutDocument = std::move(Staged);
	strOutStatus = "Parsed Valtan pattern authoring Effect bindings.";
	return true;
}

bool_t Client::CValtanPatternAuthoringEffectDocument::Validate(
	const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
	std::string& strOutStatus)
{
	if (Document.strBossArchetypeId != BOSS_ARCHETYPE_ID ||
		Document.Bindings.size() > MAX_BINDINGS)
	{
		strOutStatus = "Valtan pattern authoring Effect owner is invalid.";
		return false;
	}

	std::set<std::string, std::less<>> PatternIds;
	std::set<std::string, std::less<>> EffectAssetIds;
	std::set<std::string, std::less<>> AuthoringPaths;
	for (const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding :
		Document.Bindings)
	{
		const std::string ExpectedPath =
			Build_AuthoringPath(Binding.strEffectAssetId);
		if (!Is_StableToken(Binding.strPatternId) ||
			!Is_StableToken(Binding.strEffectAssetId) ||
			Binding.strState != DRAFT_STATE || ExpectedPath.empty() ||
			Binding.strAuthoringPath != ExpectedPath ||
			!PatternIds.insert(Binding.strPatternId).second ||
			!EffectAssetIds.insert(Binding.strEffectAssetId).second ||
			!AuthoringPaths.insert(Binding.strAuthoringPath).second ||
			Resolve_AuthoringPath(Binding).empty())
		{
			strOutStatus =
				"Valtan pattern authoring Effect binding identity/path is invalid or duplicated.";
			return false;
		}
	}
	strOutStatus = "Validated " + std::to_string(Document.Bindings.size()) +
		" Valtan pattern authoring Effect binding(s).";
	return true;
}

std::string Client::CValtanPatternAuthoringEffectDocument::Serialize(
	const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document)
{
	std::ostringstream Output;
	Output << "{\n"
		<< "  \"schema\": \"" << DOCUMENT_SCHEMA << "\",\n"
		<< "  \"formatVersion\": 1,\n"
		<< "  \"bossArchetypeId\": \"BOSS_VALTAN\",\n"
		<< "  \"bindings\": [";
	for (size_t i = 0u; i < Document.Bindings.size(); ++i)
	{
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding =
			Document.Bindings[i];
		Output << (0u == i ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"patternId\": \""
			<< CDataJson::Escape(Binding.strPatternId) << "\",\n"
			<< "      \"effectAssetId\": \""
			<< CDataJson::Escape(Binding.strEffectAssetId) << "\",\n"
			<< "      \"authoringPath\": \""
			<< CDataJson::Escape(Binding.strAuthoringPath) << "\",\n"
			<< "      \"state\": \"DRAFT_ATTACHED\"\n"
			<< "    }";
	}
	if (!Document.Bindings.empty())
		Output << '\n';
	Output << "  ]\n}\n";
	return Output.str();
}

bool_t Client::CValtanPatternAuthoringEffectDocument::Load(
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
	std::string& strOutCanonicalBaseline,
	std::string& strOutStatus)
{
	return Load_FromPath(
		Resolve_Path(), OutDocument, strOutCanonicalBaseline, strOutStatus);
}

bool_t Client::CValtanPatternAuthoringEffectDocument::
	Save_AtomicIfUnchanged(
		const VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& Document,
		const std::string_view strExpectedCanonicalBaseline,
		std::string& strOutStatus)
{
	CValtanPatternAuthoringEffectTransaction Transaction;
	if (!Transaction.Try_Acquire(strOutStatus))
		return false;
	if (!Validate(Document, strOutStatus))
		return false;
	const std::filesystem::path Destination = Resolve_Path();
	if (Destination.empty())
	{
		strOutStatus =
			"Valtan pattern authoring Effect destination is invalid.";
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT Current;
	std::string CurrentCanonical;
	if (!Load_FromPath(
			Destination, Current, CurrentCanonical, strOutStatus) ||
		CurrentCanonical != strExpectedCanonicalBaseline)
	{
		if (CurrentCanonical != strExpectedCanonicalBaseline)
		{
			strOutStatus =
				"Valtan pattern authoring Effect document changed on disk; refresh before creating.";
		}
		return false;
	}

	std::filesystem::path Temporary = Destination;
	Temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetTickCount64());
	const std::string Serialized = Serialize(Document);
	FILE* pFile = nullptr;
	if (0 != _wfopen_s(&pFile, Temporary.c_str(), L"wb") ||
		nullptr == pFile)
	{
		strOutStatus =
			"Could not open temporary Valtan pattern authoring Effect document.";
		return false;
	}
	const bool_t bWrote = Serialized.size() == fwrite(
		Serialized.data(), 1u, Serialized.size(), pFile);
	const bool_t bFlushed = 0 == fflush(pFile) && 0 == _commit(_fileno(pFile));
	const bool_t bClosed = 0 == fclose(pFile);
	if (!bWrote || !bFlushed || !bClosed)
	{
		std::error_code CleanupError;
		std::filesystem::remove(Temporary, CleanupError);
		strOutStatus =
			"Could not durably write temporary Valtan pattern authoring Effect document.";
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT Reparsed;
	std::string ReparsedCanonical;
	std::string VerifyStatus;
	if (!Load_FromPath(
			Temporary, Reparsed, ReparsedCanonical, VerifyStatus) ||
		Reparsed != Document || ReparsedCanonical != Serialized)
	{
		std::error_code CleanupError;
		std::filesystem::remove(Temporary, CleanupError);
		strOutStatus =
			"Valtan pattern authoring Effect temp verification failed: " +
			VerifyStatus;
		return false;
	}
	/* A non-Tool editor does not participate in the named mutex. Recheck the
	   canonical baseline at the last replace boundary so its completed edit is
	   not silently overwritten. Tool processes are serialized by Transaction. */
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT BeforeReplace;
	std::string BeforeReplaceCanonical;
	if (!Load_FromPath(
			Destination, BeforeReplace, BeforeReplaceCanonical, VerifyStatus) ||
		BeforeReplaceCanonical != strExpectedCanonicalBaseline)
	{
		std::error_code CleanupError;
		std::filesystem::remove(Temporary, CleanupError);
		strOutStatus =
			"Valtan pattern authoring Effect document changed before replace; refresh before creating.";
		return false;
	}
	if (!MoveFileExW(
			Temporary.c_str(), Destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code CleanupError;
		std::filesystem::remove(Temporary, CleanupError);
		strOutStatus =
			"Could not atomically replace Valtan pattern authoring Effect document.";
		return false;
	}
	strOutStatus = "Saved " + std::to_string(Document.Bindings.size()) +
		" Valtan pattern authoring Effect binding(s).";
	return true;
}
