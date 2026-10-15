// Effect Tool의 New Effect 및 Draft 삭제가 사용하는 패턴별 편집용 소유 관계 문서다.
// Effect payload와 이 목록은 별도 파일이며 두 파일의 실패 보상은 상위 Tool이 처리한다.
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

	// 허용 schema/state와 행 수 상한이다. 이름 있는 mutex의 Local 범위는 같은 Windows 세션이다.
	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-pattern-authoring-effects";
	constexpr double DOCUMENT_VERSION = 1.0;
	constexpr std::string_view BOSS_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view DRAFT_STATE = "DRAFT_ATTACHED";
	constexpr size_t MAX_BINDINGS = 256u;
	constexpr const wchar_t* TRANSACTION_MUTEX_NAME =
		L"Local\\LostArk.ValtanPatternAuthoringEffects.Transaction.v1";

	// 필수 키와 JSON 타입을 확인한다. 반환 포인터는 입력 JSON의 수명 동안만 차용한다.
	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	// 정확한 키 집합만 허용해 runtime trigger 같은 지원하지 않는 필드가 소유 문서에 섞이지 않게 한다.
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

	// ID의 길이와 ASCII 문자 집합을 검사한다. 경로 구분자와 공백은 허용하지 않는다.
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

	// Load와 저장 중 재검증이 사용하는 reader다. 임시 문서의 Parse/Validate 성공 뒤 출력 문서와 정규 baseline을 바꾼다.
	// 원본 byte hash 대신 Serialize 결과를 비교하므로 JSON 공백과 객체 키 순서는 baseline에 포함되지 않는다.
	// 읽은 뒤 stream 오류 상태는 별도로 검사하지 않는다.
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

// 소유한 mutex 획득을 한 번 해제하고 HANDLE을 닫는다. Win32 mutex의 해제는 소유 스레드에서 수행해야 한다.
Client::CValtanPatternAuthoringEffectTransaction::
	~CValtanPatternAuthoringEffectTransaction()
{
	const HANDLE hMutex = static_cast<HANDLE>(m_pMutexHandle);
	if (m_bOwned && nullptr != hMutex)
		ReleaseMutex(hMutex);
	if (nullptr != hMutex)
		CloseHandle(hMutex);
}

// Effect Tool의 바깥 트랜잭션과 Save가 사용하는 이름 있는 mutex다. 0 ms 대기로 UI를 계속 막지 않는다.
// 동일 스레드의 다른 Transaction 객체는 Win32 재귀 획득을 사용하며 각 소멸자가 자기 획득을 해제한다.
// WAIT_ABANDONED도 소유권 획득으로 받지만 이전 두 파일의 자동 복구를 수행하는 코드는 아니다.
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

// 소유 목록의 Data 루트 경로다. 실제 파일의 존재와 내용은 Load가 확인한다.
std::filesystem::path
Client::CValtanPatternAuthoringEffectDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		L"Effects/ValtanPatternAuthoringEffects.json");
}

// 입력 asset ID가 stable token이면 정해진 Authored 하위 경로만 만든다.
std::string Client::CValtanPatternAuthoringEffectDocument::
	Build_AuthoringPath(const std::string_view strEffectAssetId)
{
	if (!Is_StableToken(strEffectAssetId))
		return {};
	return "Effects/Authored/" + std::string(strEffectAssetId) +
		".effect.json";
}

// 저장 경로와 asset ID로 계산한 경로를 맞춘다. ID와 state 등 전체 binding 검증은 Validate가 맡는다.
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

// 정확한 root/binding 필드 집합과 타입을 읽고 임시 문서를 Validate한다.
// 실패 시 기존 OutDocument는 건드리지 않으며 성공한 값만 마지막에 옮긴다.
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

// 패턴 ID, asset ID와 저작 경로가 각각 고유한지 set으로 확인한다. 최대 256개이며 빈 목록도 허용한다.
// 경로는 ID로부터 결정되지만 여기서 대상 패턴 정의나 Effect 파일의 존재/내용을 읽지는 않는다.
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

// 정해진 schema/version/보스/state를 쓰고 binding 순서를 유지한다. 비교 baseline과 저장 문자열이 같은 규칙을 사용한다.
// 직렬화 자체는 Validate를 호출하지 않으므로 저장 호출자는 먼저 계약을 검사한다.
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

// Effect Tool의 Refresh와 생성/삭제 전 확인이 소유 목록 및 정규 baseline을 읽는 진입점이다. mutex는 이 reader가 얻지 않는다.
bool_t Client::CValtanPatternAuthoringEffectDocument::Load(
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT& OutDocument,
	std::string& strOutCanonicalBaseline,
	std::string& strOutStatus)
{
	return Load_FromPath(
		Resolve_Path(), OutDocument, strOutCanonicalBaseline, strOutStatus);
}

// Effect Tool의 생성/삭제 및 보상 저장이 호출한다. 현재 파일과 기대 baseline이 같은 경우에만 새 소유 목록을 준비한다.
// 쓰기/flush/재파싱/교체 전 비교를 통과한 임시 파일만 MoveFileExW로 교체한다.
// false 경로에서 이 함수는 기존 목적 파일을 교체하지 않으며 자신이 만든 임시 파일은 삭제를 시도한다.
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

	// 기존 목록을 다시 읽어 편집 시작 때의 정규 baseline과 비교한다. 파일이 없으면 새 목록으로 간주하지 않고 실패한다.
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

	// 같은 목적 디렉터리의 임시 파일에 먼저 쓴다. process ID와 tick 값으로 이름을 구분한다.
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
	// 전체 쓰기, C buffer flush와 OS 파일 commit 및 닫기를 각각 확인한다. 실패하면 임시 파일 정리를 시도한다.
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

	// 방금 쓴 임시 파일을 다시 읽어 typed 문서와 정규 문자열이 모두 의도한 값인지 확인한다.
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
	// 같은 mutex에 참여하지 않는 외부 편집기의 완료된 저장도 교체 직전에 다시 확인한다.
	// 비교 기준은 정규 직렬화 문자열이므로 공백과 객체 키 순서만 바뀐 저장은 같은 내용이다.
	// 이 확인과 MoveFileExW 사이의 외부 쓰기까지 잠그는 파일 단위 CAS는 아니다.
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
	// 검증한 소유 목록 파일 하나를 교체한다. 관련 Effect payload 파일까지 같은 호출로 교체되는 것은 아니다.
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
