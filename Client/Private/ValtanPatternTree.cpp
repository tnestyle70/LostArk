// 발탄 편집 트리의 읽기 경계다. gameplay/presentation 저작 문서를 합치고 필요한 경로에서는 게시 Product와 대조한다.
// 이 파일은 Server 전투를 실행하거나 문서를 게시하지 않는다. 외부에 넘기는 뷰는 검증한 임시 값으로 교체한다.
// 아래 내부 parser는 출력을 중간까지 채울 수 있으므로 공개 Load의 임시 문서 안에서 사용한다.
// 공개 Load·표시 보조 함수와 결합 결과 뷰, admission 소유 상태를 구현한다.
#include "ValtanPatternTree.h"

// Product 애니메이션 clip·binding 타입, clip 수 상한과 stage 환경 파서를 사용한다.
#include "AnimationSkillBindingDocument.h"
// JSON 값의 타입별 읽기와 원본 문서 파싱·직렬화를 수행한다.
#include "DataJson.h"
// 기존 Encounter 참조 헤더의 포함 관계를 유지한다. 이 본문은 Encounter JSON을 직접 읽어 결합한다.
#include "EncounterPatternReference.h"
// Effect V2 binding을 읽어 패턴/stage 원본 범위와 연결한다.
#include "EffectV2_Document.h"
// Data 상대 경로를 저작 원본 경로로 해석하고 읽기 admission의 프로젝트 루트를 구한다.
#include "ProjectDataRoot.h"
// 게시된 발탄 Effect cue 문서와 scale/follow/stop 계약을 읽는다.
#include "ValtanPatternEffectCueDocument.h"
// scripted sequence 길이를 기존 Flow 문서의 MAX_SLOTS 제한과 맞춘다.
#include "ValtanPatternFlowDocument.h"

// ID 검색·중복/조건 집계·순서 정렬을 기존 목록 위에서 수행한다.
#include <algorithm>
// 숫자의 유한성·정수 여부·절댓값 범위를 확인한다.
#include <cmath>
// 원본과 Product 파일을 binary 입력 스트림으로 읽는다.
#include <fstream>
// 패턴 후속 그래프의 깊이를 검사하는 재귀 callback을 보관한다.
#include <functional>
// 표시용 순번과 JSON 숫자의 자릿수·정밀도를 지정한다.
#include <iomanip>
// 허용·필수 JSON 필드 이름 묶음을 고정된 호출 인자로 전달한다.
#include <initializer_list>
// 파일 입력 스트림 전체를 문자열로 읽는 iterator 범위를 사용한다.
#include <iterator>
// 정수/실수 변환 상한과 JSON 실수 직렬화 정밀도를 구한다.
#include <limits>
// 표시 요약·JSON 숫자를 지역 설정의 소수점 표기와 분리한다.
#include <locale>
// stable ID의 정의·소유자·stage index를 결합하는 정렬 색인을 만든다.
#include <map>
// admission의 잠금 상태를 임시 unique_ptr로 준비해 성공한 객체에 넘긴다.
#include <memory>
// 중복 ID와 방문 상태·참조 coverage를 검사한다.
#include <set>
// 표시 이름·패턴 요약·접촉 배열 JSON을 메모리 문자열로 구성한다.
#include <sstream>
// 필드 이름과 stable ID를 검사할 때 입력 문자열을 복사 없이 차용한다.
#include <string_view>

namespace
{
	using Client::DATA_JSON_TYPE;
	using Client::DATA_JSON_VALUE;
	constexpr const wchar_t* VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE =
		L"out\\ValtanPatternTransactions\\create-pattern.lock";
	constexpr const wchar_t* VALTAN_PATTERN_ACTIVE_GENERATION_RELATIVE =
		L"out\\ValtanPatternTransactions\\active-generation.json";
	constexpr uint32_t VALTAN_PATTERN_FOLLOWUP_MAX_DEPTH = 32u;

	// Python/C++ 게시자가 배타 잠금을 거는 동일한 1 byte 범위를 공유 잠금으로 읽는다.
	// 엄격한 문서 결합이 끝날 때까지 핸들을 소유하여 잠금에 참여하는 게시자와 세대가 섞이지 않게 한다.
	// 중단된 게시자의 journal은 reader가 복구하지 않는다. journal이 남아 있으면 다음 writer의 복구를 요구한다.
	class SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK final
	{
	public:
		~SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK()
		{
			if (INVALID_HANDLE_VALUE == m_hFile)
				return;
			UnlockFileEx(m_hFile, 0u, 1u, 0u, &m_Overlap);
			CloseHandle(m_hFile);
		}

		// 읽기 admission이 호출한다. 핸들과 byte 잠금은 이 객체가 소유하고 소멸 시 해제한다.
		// 실패 이유를 busy와 I/O로 나누며 기다리는 재시도나 writer 복구를 여기서 수행하지 않는다.
		bool_t Try_Acquire(
			const std::filesystem::path& ProjectRoot,
			std::string& strOutError,
			Client::VALTAN_CANONICAL_READ_FAILURE_KIND& eOutFailure)
		{
			eOutFailure =
				Client::VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_IO;
			if (ProjectRoot.empty())
			{
				strOutError = "project root is unavailable";
				eOutFailure = Client::VALTAN_CANONICAL_READ_FAILURE_KIND::
					ADMISSION_STATE_INVALID;
				return false;
			}
			const std::filesystem::path LockPath =
				ProjectRoot / VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE;
			std::error_code DirectoryError;
			std::filesystem::create_directories(
				LockPath.parent_path(), DirectoryError);
			if (DirectoryError)
			{
				strOutError = "lock directory creation failed: " +
					DirectoryError.message();
				return false;
			}

			m_hFile = CreateFileW(
				LockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				strOutError = "lock open failed with Win32 error " +
					std::to_string(GetLastError());
				return false;
			}

			// Python msvcrt writer와 같은 1 byte 잠금 범위를 초기화한다.
			// 공유 잠금 전에 파일 길이를 확보해 첫 writer가 잠긴 빈 파일에 쓰지 않도록 한다.
			LARGE_INTEGER Size{};
			if (FALSE == GetFileSizeEx(m_hFile, &Size))
			{
				strOutError = "lock size query failed with Win32 error " +
					std::to_string(GetLastError());
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			if (Size.QuadPart < 1)
			{
				const char_t Byte = '\0';
				DWORD iWritten = 0u;
				LARGE_INTEGER Begin{};
				if (FALSE == SetFilePointerEx(
						m_hFile, Begin, nullptr, FILE_BEGIN) ||
					FALSE == WriteFile(
						m_hFile, &Byte, 1u, &iWritten, nullptr) ||
					1u != iWritten || FALSE == FlushFileBuffers(m_hFile))
				{
					strOutError =
						"lock initialization failed with Win32 error " +
						std::to_string(GetLastError());
					CloseHandle(m_hFile);
					m_hFile = INVALID_HANDLE_VALUE;
					return false;
				}
			}

			// LOCKFILE_EXCLUSIVE_LOCK을 생략하면 공유 읽기 잠금이다.
			// FAIL_IMMEDIATELY로 게시자의 배타 잠금이 풀리기를 UI에서 계속 기다리지 않고 실패를 돌려준다.
			if (FALSE == LockFileEx(m_hFile, LOCKFILE_FAIL_IMMEDIATELY,
				0u, 1u, 0u, &m_Overlap))
			{
				const DWORD iError = GetLastError();
				eOutFailure = ERROR_LOCK_VIOLATION == iError ?
					Client::VALTAN_CANONICAL_READ_FAILURE_KIND::WRITER_BUSY :
					Client::VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_IO;
				strOutError =
					"Create/Project transaction is active (Win32 " +
					std::to_string(iError) + ")";
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			eOutFailure = Client::VALTAN_CANONICAL_READ_FAILURE_KIND::NONE;
			return true;
		}

	private:
		HANDLE m_hFile = INVALID_HANDLE_VALUE;
		OVERLAPPED m_Overlap{};
	};

	struct VALTAN_CANONICAL_PRODUCT_READ_STATE final
	{
		SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK Lock;
		std::filesystem::path ProjectRoot;
	};

	// 현재 게시 journal의 존재만 확인한다. 내용을 파싱하거나 생성물 hash를 비교하지 않는다.
	// journal이 있으면 복구가 필요하다고 알리며 reader가 파일을 지우지 않는다.
	bool_t Require_NoActiveValtanPatternGeneration(
		const std::filesystem::path& ProjectRoot,
		std::string& strOutError,
		Client::VALTAN_CANONICAL_READ_FAILURE_KIND& eOutFailure)
	{
		eOutFailure =
			Client::VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_IO;
		const std::filesystem::path ActiveGeneration =
			ProjectRoot / VALTAN_PATTERN_ACTIVE_GENERATION_RELATIVE;
		std::error_code ExistsError;
		const bool_t bExists =
			std::filesystem::exists(ActiveGeneration, ExistsError);
		if (ExistsError)
		{
			strOutError = "active-generation journal query failed: " +
				ExistsError.message();
			return false;
		}
		if (bExists)
		{
			eOutFailure = Client::VALTAN_CANONICAL_READ_FAILURE_KIND::
				WRITER_RECOVERY_REQUIRED;
			strOutError =
				"active-generation journal requires writer recovery: " +
				ActiveGeneration.generic_string();
			return false;
		}
		eOutFailure = Client::VALTAN_CANONICAL_READ_FAILURE_KIND::NONE;
		return true;
	}

	constexpr std::string_view CINEMATIC_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
	constexpr std::string_view IDLE_CINEMATIC_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC_IDLE";

	// 일반 decision 소유자가 없어도 허용하는 입장 패턴 ID를 좁게 한정한다.
	bool_t Is_OptionalEntryPatternId(const std::string_view PatternId)
	{
		return CINEMATIC_ENTRY_PATTERN_ID == PatternId ||
			IDLE_CINEMATIC_ENTRY_PATTERN_ID == PatternId;
	}

	// 명시한 파일을 binary로 읽는 공통 보조 함수다. 경로 해석과 JSON 검증은 호출자가 맡는다.
	// 열기 성공 뒤 iterator 범위를 읽으며 별도 크기 상한, 읽기 완료 상태나 동시 변경 hash 검사는 하지 않는다.
	bool Read_TextDocument(
		const std::filesystem::path& Path,
		std::string& OutText,
		std::string& strOutError)
	{
		if (Path.empty())
		{
			strOutError = "path escaped the Data root";
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		OutText.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return true;
	}

	// Data 루트 상대 경로를 읽고 JSON 객체인지 확인한다. 출력은 상위 Load가 소유한 임시 JSON이다.
	bool Parse_Document(
		const std::filesystem::path& Relative,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(
				Client::CProjectDataRoot::Resolve(Relative), Text, strOutError))
		{
			return false;
		}
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Relative.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	// 명시적인 원본 경로를 읽는 변형이다. 경로의 admission 잠금은 이 함수가 획득하지 않는다.
	bool Parse_DocumentPath(
		const std::filesystem::path& Path,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(Path, Text, strOutError))
			return false;
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Path.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	// 선택 문자열 조회다. 누락과 타입 불일치는 빈 문자열로 돌려주므로 필수 계약은 별도 검증해야 한다.
	std::string Read_String(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::STRING != pValue->Get_Type() ?
			std::string{} : pValue->Get_String();
	}

	// 선택 숫자 조회의 기본값은 0이다. 정수 범위나 필수 여부가 필요한 곳은 전용 reader를 사용한다.
	double Read_Number(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::NUMBER != pValue->Get_Type() ?
			0.0 : pValue->Get_Number();
	}

	// JSON 숫자가 유한한 uint32 범위의 정수 값일 때만 출력한다. 소수부 절삭으로 잘못된 ms를 받아들이지 않는다.
	bool_t Read_RequiredUInt32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		uint32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::floor(pValue->Get_Number()) != pValue->Get_Number() ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	// 유한한 float 범위를 확인하고 성공할 때만 출력값을 바꾼다.
	bool_t Read_RequiredFiniteFloat(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		f32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::abs(pValue->Get_Number()) > static_cast<double>(
				(std::numeric_limits<f32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	// 필수 이름과 타입을 함께 확인한다. 반환 포인터의 메모리는 입력 JSON이 소유한다.
	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	// 필수 키가 정확히 존재하고 추가 키가 없는지 확인해 지원하지 않는 필드를 조용히 버리지 않는다.
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

	// JSON 값의 의미를 재귀 비교한다. 객체의 키 순서는 무시하지만 배열 순서와 숫자 값은 그대로 비교한다.
	bool_t Equal_JsonSemanticValue(
		const DATA_JSON_VALUE& Left,
		const DATA_JSON_VALUE& Right)
	{
		if (Left.Get_Type() != Right.Get_Type())
			return false;
		switch (Left.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE:
			return true;
		case DATA_JSON_TYPE::BOOLEAN:
			return Left.Get_Boolean() == Right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return Left.Get_Number() == Right.Get_Number();
		case DATA_JSON_TYPE::STRING:
			return Left.Get_String() == Right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			const auto& LeftValues = Left.Get_Array();
			const auto& RightValues = Right.Get_Array();
			return LeftValues.size() == RightValues.size() &&
				std::equal(LeftValues.begin(), LeftValues.end(),
					RightValues.begin(), Equal_JsonSemanticValue);
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			const auto& LeftFields = Left.Get_Object();
			const auto& RightFields = Right.Get_Object();
			if (LeftFields.size() != RightFields.size())
				return false;
			for (const auto& [Name, LeftValue] : LeftFields)
			{
				const auto RightField = RightFields.find(Name);
				if (RightFields.end() == RightField ||
					!Equal_JsonSemanticValue(LeftValue, RightField->second))
				{
					return false;
				}
			}
			return true;
		}
		default:
			return false;
		}
	}

	// 필수 키는 모두 요구하고 명시한 선택 키만 생략을 허용한다. 알 수 없는 추가 키는 거절한다.
	bool_t Has_ExactPropertiesWithOptional(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> RequiredNames,
		const std::initializer_list<std::string_view> OptionalNames)
	{
		if (!Object.Is_Object())
			return false;
		for (const std::string_view Name : RequiredNames)
		{
			if (nullptr == Object.Find(Name))
				return false;
		}
		size_t iExpectedPropertyCount = RequiredNames.size();
		for (const std::string_view Name : OptionalNames)
		{
			if (nullptr != Object.Find(Name))
				++iExpectedPropertyCount;
		}
		return Object.Get_Object().size() == iExpectedPropertyCount;
	}

	// 문서 사이를 연결하는 stable token의 길이와 ASCII 문자 집합을 검사한다. 표시 이름과 경로 검사는 별도다.
	bool_t Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				'_' != Character && '-' != Character && '.' != Character)
			{
				return false;
			}
		}
		return true;
	}

	// 선택 JSON 포인터의 숫자 타입과 유한성만 확인한다. 양수와 단위별 상한은 각 계약에서 검사한다.
	bool_t Is_FiniteNumber(const DATA_JSON_VALUE* pValue)
	{
		return nullptr != pValue && pValue->Is_Number() &&
			std::isfinite(pValue->Get_Number());
	}

	// 왼손 잡기 로컬 오프셋의 forward/up/right 세 필드를 읽고 공용 좌표 계약을 검사한다.
	// 시작할 때 출력을 비우므로 실패 시 이전 optional 값 보존을 기대해서는 안 된다.
	bool_t Read_PlayerHandGripLocalOffset(
		const DATA_JSON_VALUE* const pValue,
		std::optional<Client::PLAYER_HAND_GRIP_LOCAL_OFFSET>& Out)
	{
		Out.reset();
		if (nullptr == pValue)
			return true;
		if (!Has_ExactProperties(*pValue,
				{ "forwardM", "upM", "rightM" }) ||
			!Is_FiniteNumber(pValue->Find("forwardM")) ||
			!Is_FiniteNumber(pValue->Find("upM")) ||
			!Is_FiniteNumber(pValue->Find("rightM")))
		{
			return false;
		}
		Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Offset;
		Offset.fForwardM = static_cast<f32_t>(
			pValue->Find("forwardM")->Get_Number());
		Offset.fUpM = static_cast<f32_t>(
			pValue->Find("upM")->Get_Number());
		Offset.fRightM = static_cast<f32_t>(
			pValue->Find("rightM")->Get_Number());
		if (!Client::CPlayerHandGripTransform::Is_ValidGripLocalOffset(Offset))
			return false;
		Out = Offset;
		return true;
	}

	// 본체를 숨기는 선택 구간을 stage 시작 기준 ms로 읽는다. from < to <= stage 길이를 요구한다.
	// 출력은 먼저 기본값으로 초기화되며 제품 렌더 상태를 직접 바꾸지 않는다.
	bool_t Read_StageBodyVisibility(
		const DATA_JSON_VALUE* const pValue,
		const uint32_t iStageDurationMs,
		bool_t& bOutHasWindow,
		uint32_t& iOutHiddenFromMs,
		uint32_t& iOutHiddenToMs)
	{
		bOutHasWindow = false;
		iOutHiddenFromMs = 0u;
		iOutHiddenToMs = 0u;
		if (nullptr == pValue)
			return true;
		const DATA_JSON_VALUE* const pHiddenFromMs =
			pValue->Find("hiddenFromMs");
		const DATA_JSON_VALUE* const pHiddenToMs =
			pValue->Find("hiddenToMs");
		if (!Has_ExactProperties(
				*pValue, { "hiddenFromMs", "hiddenToMs" }) ||
			!Is_FiniteNumber(pHiddenFromMs) ||
			!Is_FiniteNumber(pHiddenToMs) ||
			pHiddenFromMs->Get_Number() < 0.0 ||
			pHiddenToMs->Get_Number() < 0.0 ||
			std::floor(pHiddenFromMs->Get_Number()) !=
				pHiddenFromMs->Get_Number() ||
			std::floor(pHiddenToMs->Get_Number()) !=
				pHiddenToMs->Get_Number() ||
			pHiddenFromMs->Get_Number() > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()) ||
			pHiddenToMs->Get_Number() > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		const uint32_t iHiddenFromMs = static_cast<uint32_t>(
			pHiddenFromMs->Get_Number());
		const uint32_t iHiddenToMs = static_cast<uint32_t>(
			pHiddenToMs->Get_Number());
		if (iHiddenFromMs >= iHiddenToMs || iHiddenToMs > iStageDurationMs)
			return false;
		bOutHasWindow = true;
		iOutHiddenFromMs = iHiddenFromMs;
		iOutHiddenToMs = iHiddenToMs;
		return true;
	}

	// 유한한 uint32 범위의 비음수 정수 값인지 확인한다. JSON의 텍스트 표기 방식 자체를 제한하지는 않는다.
	bool_t Is_NonNegativeInteger(const DATA_JSON_VALUE* pValue)
	{
		return Is_FiniteNumber(pValue) && pValue->Get_Number() >= 0.0 &&
			std::floor(pValue->Get_Number()) == pValue->Get_Number() &&
			pValue->Get_Number() <= static_cast<double>(
				(std::numeric_limits<uint32_t>::max)());
	}

	// 필드는 필수지만 null은 빈 참조로 허용한다. 문자열이면 stable ID 형식을 요구한다.
	bool_t Read_NullableStableToken(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue)
			return false;
		if (pValue->Is_Null())
		{
			OutValue.clear();
			return true;
		}
		if (!pValue->Is_String() || !Is_StableToken(pValue->Get_String()))
			return false;
		OutValue = pValue->Get_String();
		return true;
	}

	// stage 조준 정책과 선택 종료 ms 및 응답 배율을 읽는다. 기본값 초기화 후 검사하므로 실패 시 일부 출력이 바뀔 수 있다.
	bool_t Read_StageAim(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::string& strOutTargetPolicy,
		bool_t& bOutHasEnd, uint32_t& iOutEndMs,
		bool_t& bOutHasResponseScale, f32_t& fOutResponseScale)
	{
		strOutTargetPolicy.clear();
		bOutHasEnd = false;
		iOutEndMs = 0u;
		bOutHasResponseScale = false;
		fOutResponseScale = 1.f;
		if (nullptr == pValue)
			return true;
		const DATA_JSON_VALUE* pPolicy = Required(
			*pValue, "targetPolicy", DATA_JSON_TYPE::STRING);
		if (!Has_ExactPropertiesWithOptional(*pValue, { "targetPolicy" },
				{ "endMs", "responseScale" }) || nullptr == pPolicy ||
			("NEAREST_EACH_TICK" != pPolicy->Get_String() &&
			 "PATTERN_TARGET" != pPolicy->Get_String()))
			return false;
		strOutTargetPolicy = pPolicy->Get_String();
		bOutHasEnd = nullptr != pValue->Find("endMs");
		bOutHasResponseScale = nullptr != pValue->Find("responseScale");
		return (!bOutHasEnd ||
			(Read_RequiredUInt32(*pValue, "endMs", iOutEndMs) &&
			 iOutEndMs <= iStageDurationMs)) &&
			(!bOutHasResponseScale ||
			 (Read_RequiredFiniteFloat(*pValue, "responseScale", fOutResponseScale) &&
			  fOutResponseScale >= 0.01f && fOutResponseScale <= 10.f));
	}

	// stage 이동의 종류별 입력을 읽는다. 포탈 돌진은 지연 ms + 거리 m / 속도 m/s가 stage 시간 안에 있어야 한다.
	// 출력 optional을 먼저 비우고 검증한 이동 값만 마지막에 넣는다.
	bool_t Read_StageMotion(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		Client::VALTAN_STAGE_MOTION_VIEW Motion;
		if (nullptr == pKind)
			return false;
		Motion.strKind = pKind->Get_String();
		if ("PORTAL_TARGET_RUSH" == Motion.strKind)
		{
			if (!Has_ExactProperties(*pValue,
					{ "kind", "retargetDelayMs", "speedMps", "distanceM" }) ||
				!Read_RequiredUInt32(
					*pValue, "retargetDelayMs", Motion.iRetargetDelayMs) ||
				!Read_RequiredFiniteFloat(
					*pValue, "speedMps", Motion.fSpeedMps) ||
				!Read_RequiredFiniteFloat(
					*pValue, "distanceM", Motion.fDistance) ||
				Motion.fSpeedMps <= 0.f || Motion.fSpeedMps > 1000.f ||
				Motion.fDistance <= 0.f || Motion.fDistance > 1000.f ||
				static_cast<double>(Motion.iRetargetDelayMs) +
					static_cast<double>(Motion.fDistance) /
					static_cast<double>(Motion.fSpeedMps) * 1000.0 >
					static_cast<double>(iStageDurationMs) + 0.000001)
				return false;
		}
		else if ("PORTAL_CROSS_ARENA" == Motion.strKind)
		{
			const DATA_JSON_VALUE* pExtents = Required(
				*pValue, "halfExtentsM", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactProperties(*pValue, { "kind", "cornerIndex", "halfExtentsM" }) ||
				!Read_RequiredUInt32(*pValue, "cornerIndex", Motion.iCornerIndex) ||
				Motion.iCornerIndex > 3u || nullptr == pExtents ||
				pExtents->Get_Array().size() != 2u)
				return false;
			for (size_t iAxis = 0u; iAxis < 2u; ++iAxis)
			{
				const DATA_JSON_VALUE& Extent = pExtents->Get_Array()[iAxis];
				if (!Is_FiniteNumber(&Extent) || Extent.Get_Number() < 1.0 ||
					Extent.Get_Number() > 100.0)
					return false;
				Motion.HalfExtentsM[iAxis] = static_cast<f32_t>(Extent.Get_Number());
			}
		}
		else if ("TO_ARENA_CENTER" == Motion.strKind)
		{
			if (!Has_ExactProperties(*pValue, { "kind" }))
				return false;
		}
		else if ("FORWARD" != Motion.strKind ||
			!Has_ExactProperties(*pValue, { "kind", "distance" }) ||
			!Read_RequiredFiniteFloat(*pValue, "distance", Motion.fDistance))
		{
			return false;
		}
		Out = std::move(Motion);
		return true;
	}

	// 기존 stage action 뷰를 만든다. 일반 action, 잡기 해제와 일제 생성은 서로 다른 필드 계약을 사용한다.
	// 목록을 먼저 비우므로 뒤 행에서 실패하면 앞서 추가한 임시 행이 남을 수 있다.
	bool_t Read_StageActions(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;

		// 같은 stage의 예약 일제 생성은 하나의 시계를 공유한다.
		// 첫 시작 ms·횟수·간격을 기억해 뒤의 예약 행과 일치하는지 비교한다.
		bool_t bHasScheduledVolleyClock = false;
		uint32_t iScheduledVolleyFirstOffsetMs = 0u;
		uint32_t iScheduledVolleyCount = 0u;
		uint32_t iScheduledVolleyIntervalMs = 0u;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			// 일제 생성은 일반 action 값으로 축약하지 않는다.
			// 대상별 배치·도형·추가 arena 배치와 스케줄을 검증한 뒤 별도 결합 경로에 남긴다.
			if ("SPAWN_COMBAT_OBJECT_VOLLEY" == Read_String(Value, "kind"))
			{
				const DATA_JSON_VALUE* pTargeting = Required(
					Value, "targetingPolicy", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pLayout = Required(
					Value, "layout", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pAllowOverlap = Required(
					Value, "allowOverlap", DATA_JSON_TYPE::BOOLEAN);
				const DATA_JSON_VALUE* pArenaAnchor = Required(
					Value, "arenaAnchorPolicy", DATA_JSON_TYPE::STRING);
				const std::string strTargeting = nullptr == pTargeting ?
					std::string{} : pTargeting->Get_String();
				const bool_t bPerAlivePlayer =
					"PER_ALIVE_PLAYER" == strTargeting;
				const bool_t bBossRelative = "BOSS_RELATIVE" == strTargeting;
				const bool_t bArenaCenter = "ARENA_CENTER" == strTargeting;

				// 허용 필드와 ENTER trigger를 먼저 확인한다. 이어서 대상별 개수·전체 상한과
				// 마지막 생성 시각이 stage 종료 전인지 검사하며 조건의 평가 순서는 유지한다.
				if (!Has_ExactProperties(Value,
						{ "trigger", "kind", "targetId", "targetingPolicy",
						  "countPerResolvedTarget", "layout", "radiusM",
						  "startAngleDegrees", "angleStepDegrees", "allowOverlap",
						  "maximumTotalObjects", "spawnCount", "firstSpawnOffsetMs",
						  "spawnIntervalMs",
						  "arenaRandomCount", "arenaRandomRadiusM",
						  "arenaHeightToleranceM", "arenaAnchorPolicy" }) ||
					nullptr == Required(Value, "trigger", DATA_JSON_TYPE::STRING) ||
					"ENTER" != Read_String(Value, "trigger") ||
					nullptr == Required(Value, "targetId", DATA_JSON_TYPE::STRING) ||
					nullptr == pTargeting ||
					(!bPerAlivePlayer && !bBossRelative && !bArenaCenter) ||
					nullptr == pLayout ||
					("SINGLE" != pLayout->Get_String() &&
					 "RADIAL" != pLayout->Get_String()) ||
					!Is_NonNegativeInteger(Value.Find("countPerResolvedTarget")) ||
					0.0 == Value.Find("countPerResolvedTarget")->Get_Number() ||
					Value.Find("countPerResolvedTarget")->Get_Number() > 8.0 ||
					!Is_FiniteNumber(Value.Find("radiusM")) ||
					!Is_FiniteNumber(Value.Find("startAngleDegrees")) ||
					!Is_FiniteNumber(Value.Find("angleStepDegrees")) ||
					nullptr == pAllowOverlap ||
					!Is_NonNegativeInteger(Value.Find("maximumTotalObjects")) ||
					0.0 == Value.Find("maximumTotalObjects")->Get_Number() ||
					Value.Find("maximumTotalObjects")->Get_Number() > 64.0 ||

					// 반복이 하나면 간격은 0, 둘 이상이면 양수다. 마지막 생성도 durationMs 미만이어야 한다.
					!Is_NonNegativeInteger(Value.Find("spawnCount")) ||
					0.0 == Value.Find("spawnCount")->Get_Number() ||
					Value.Find("spawnCount")->Get_Number() > 8.0 ||
					!Is_NonNegativeInteger(Value.Find("firstSpawnOffsetMs")) ||
					Value.Find("firstSpawnOffsetMs")->Get_Number() >=
						static_cast<double>(iStageDurationMs) ||
					!Is_NonNegativeInteger(Value.Find("spawnIntervalMs")) ||
					Value.Find("spawnIntervalMs")->Get_Number() >
						static_cast<double>(iStageDurationMs) ||
					(Value.Find("spawnCount")->Get_Number() > 1.0 &&
					 0.0 == Value.Find("spawnIntervalMs")->Get_Number()) ||
					(Value.Find("spawnCount")->Get_Number() == 1.0 &&
					 0.0 != Value.Find("spawnIntervalMs")->Get_Number()) ||
					Value.Find("firstSpawnOffsetMs")->Get_Number() +
						(Value.Find("spawnCount")->Get_Number() - 1.0) *
						Value.Find("spawnIntervalMs")->Get_Number() >=
						static_cast<double>(iStageDurationMs) ||

					// arena 보충 배치의 개수·반경·높이 허용 오차를 검사한다. 길이 단위는 m다.
					!Is_NonNegativeInteger(Value.Find("arenaRandomCount")) ||
					Value.Find("arenaRandomCount")->Get_Number() > 32.0 ||
					!Is_FiniteNumber(Value.Find("arenaRandomRadiusM")) ||
					Value.Find("arenaRandomRadiusM")->Get_Number() < 0.0 ||
					Value.Find("arenaRandomRadiusM")->Get_Number() > 1000.0 ||
					!Is_FiniteNumber(Value.Find("arenaHeightToleranceM")) ||
					Value.Find("arenaHeightToleranceM")->Get_Number() < 0.0 ||
					Value.Find("arenaHeightToleranceM")->Get_Number() > 1000.0 ||
					nullptr == pArenaAnchor ||
					Value.Find("maximumTotalObjects")->Get_Number() <
						Value.Find("countPerResolvedTarget")->Get_Number() +
						Value.Find("arenaRandomCount")->Get_Number())
				{
					return false;
				}

				// 1개 배치는 SINGLE과 0 반경/각도만, 여러 개는 양수 RADIAL 반경/간격만 허용한다.
				// angleStepDegrees × 개수가 360.000001도를 넘거나 allowOverlap이 켜져 있으면 거부한다.
				const double fCount =
					Value.Find("countPerResolvedTarget")->Get_Number();
				const double fRadius = Value.Find("radiusM")->Get_Number();
				const double fStartAngle =
					Value.Find("startAngleDegrees")->Get_Number();
				const double fAngleStep =
					Value.Find("angleStepDegrees")->Get_Number();
				const bool_t bSingle = 1.0 == fCount;
				if ((bSingle && ("SINGLE" != pLayout->Get_String() ||
						0.0 != fRadius || 0.0 != fStartAngle ||
						0.0 != fAngleStep || pAllowOverlap->Get_Boolean())) ||
					(!bSingle && ("RADIAL" != pLayout->Get_String() ||
						fRadius <= 0.0 || fAngleStep <= 0.0 ||
						fAngleStep * fCount > 360.000001 ||
						pAllowOverlap->Get_Boolean())))
				{
					return false;
				}

				// 플레이어별 배치는 arena 보충을 전부 끄거나 유효한 한 세트로 지정한다.
				// 보스/arena 중심 배치는 보충 없이 한 번의 RADIAL 일제 생성만 허용한다.
				const bool_t bNoArenaSupplement =
					"NONE" == pArenaAnchor->Get_String() &&
					0.0 == Value.Find("arenaRandomCount")->Get_Number() &&
					0.0 == Value.Find("arenaRandomRadiusM")->Get_Number() &&
					0.0 == Value.Find("arenaHeightToleranceM")->Get_Number();
				const bool_t bArenaSupplement =
					"BOSS_SPAWN_POSITION" == pArenaAnchor->Get_String() &&
					Value.Find("arenaRandomCount")->Get_Number() > 0.0 &&
					Value.Find("arenaRandomRadiusM")->Get_Number() > 0.0 &&
					Value.Find("arenaHeightToleranceM")->Get_Number() > 0.0;
				const bool_t bPerAlivePlayerContract = bPerAlivePlayer &&
					(bNoArenaSupplement || bArenaSupplement);
				const bool_t bBossRelativeContract = (bBossRelative || bArenaCenter) &&
					"RADIAL" == pLayout->Get_String() && fCount >= 2.0 &&
					fRadius > 0.0 && fAngleStep > 0.0 &&
					fAngleStep * fCount <= 360.000001 &&
					Value.Find("maximumTotalObjects")->Get_Number() >= fCount &&
					1.0 == Value.Find("spawnCount")->Get_Number() &&
					0.0 == Value.Find("spawnIntervalMs")->Get_Number() &&
					0.0 == Value.Find("arenaRandomCount")->Get_Number() &&
					0.0 == Value.Find("arenaRandomRadiusM")->Get_Number() &&
					0.0 == Value.Find("arenaHeightToleranceM")->Get_Number() &&
					"NONE" == pArenaAnchor->Get_String();
				if (!bPerAlivePlayerContract && !bBossRelativeContract)
					return false;

				// 예약 시계를 uint32_t 값으로 읽은 뒤 기존 예약 행과 대조한다.
				// 즉시 한 번 생성하는 행은 이 공유 예약 시계의 소유자가 아니다.
				const uint32_t iFirstSpawnOffsetMs = static_cast<uint32_t>(
					Value.Find("firstSpawnOffsetMs")->Get_Number());
				const uint32_t iSpawnCount = static_cast<uint32_t>(
					Value.Find("spawnCount")->Get_Number());
				const uint32_t iSpawnIntervalMs = static_cast<uint32_t>(
					Value.Find("spawnIntervalMs")->Get_Number());
				if (0u != iFirstSpawnOffsetMs || iSpawnCount > 1u)
				{
					if (!bHasScheduledVolleyClock)
					{
						bHasScheduledVolleyClock = true;
						iScheduledVolleyFirstOffsetMs = iFirstSpawnOffsetMs;
						iScheduledVolleyCount = iSpawnCount;
						iScheduledVolleyIntervalMs = iSpawnIntervalMs;
					}
					else if (iScheduledVolleyFirstOffsetMs != iFirstSpawnOffsetMs ||
						iScheduledVolleyCount != iSpawnCount ||
						iScheduledVolleyIntervalMs != iSpawnIntervalMs)
					{
						return false;
					}
				}
				// 일제 생성 행은 전투 오브젝트 소유자로 별도 검증하고 결합한다.
				// 반경과 스케줄을 잃는 기존 5필드 stage action 뷰로 축약하지 않는다.
				continue;
			}

			// 잡기 해제는 왼손 attachment를 대상으로 속도 m/s·지속 ms·yaw degree를 읽는다.
			// HOLD는 모두 0이고 밀어내기는 양수 속도/시간, ARENA_EJECTION에서만 yaw를 허용한다.
			if ("RELEASE_GRABBED_PLAYERS" == Read_String(Value, "kind"))
			{
				const DATA_JSON_VALUE* pTrigger = Required(
					Value, "trigger", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pKind = Required(
					Value, "kind", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pTarget = Required(
					Value, "targetId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pReleaseMode = Required(
					Value, "releaseMode", DATA_JSON_TYPE::STRING);
				Client::VALTAN_STAGE_ACTION_VIEW Action;
				if (!Has_ExactProperties(Value,
						{ "trigger", "kind", "targetId", "releaseMode",
						  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
					nullptr == pTrigger || nullptr == pKind ||
					nullptr == pTarget || nullptr == pReleaseMode ||
					("ENTER" != pTrigger->Get_String() &&
					 "EXIT" != pTrigger->Get_String()) ||
					"boss.attachment.left-hand" != pTarget->Get_String() ||
					!Read_RequiredFiniteFloat(
						Value, "speedMps", Action.fSpeedMps) ||
					!Read_RequiredUInt32(
						Value, "durationMs", Action.iDurationMs) ||
					!Read_RequiredFiniteFloat(
						Value, "yawOffsetDegrees", Action.fYawOffsetDegrees) ||
					Action.fSpeedMps < 0.f || Action.fSpeedMps > 50.f ||
					Action.iDurationMs > 5000u ||
					std::abs(Action.fYawOffsetDegrees) > 180.f)
				{
					return false;
				}
				const bool_t bHold = "HOLD" == pReleaseMode->Get_String() &&
					0.f == Action.fSpeedMps && 0u == Action.iDurationMs &&
					0.f == Action.fYawOffsetDegrees;
				const bool_t bOppositeKnockback =
					("OPPOSITE_KNOCKBACK" == pReleaseMode->Get_String() ||
					 "ARENA_EJECTION" == pReleaseMode->Get_String()) &&
					Action.fSpeedMps > 0.f && Action.iDurationMs > 0u &&
					("ARENA_EJECTION" == pReleaseMode->Get_String() ||
					 0.f == Action.fYawOffsetDegrees);
				if (!bHold && !bOppositeKnockback)
					return false;
				Action.strTrigger = pTrigger->Get_String();
				Action.strKind = pKind->Get_String();
				Action.strTargetId = pTarget->Get_String();
				Action.strReleaseMode = pReleaseMode->Get_String();
				Out.push_back(std::move(Action));
				continue;
			}

			// 나머지 action은 공통 다섯 필드를 읽는다. 아래에서 명시한 종류에는
			// trigger·target·value·duration 조합 제약을 추가하며 그 외 kind 전체를 열거해 거부하지는 않는다.
			if (!Has_ExactProperties(Value,
					{ "trigger", "kind", "targetId", "value", "durationMs" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* pTrigger = Required(
				Value, "trigger", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pKind = Required(
				Value, "kind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTarget = Required(
				Value, "targetId", DATA_JSON_TYPE::STRING);
			Client::VALTAN_STAGE_ACTION_VIEW Action;
			if (nullptr == pTrigger || nullptr == pKind || nullptr == pTarget ||
				!Read_RequiredFiniteFloat(Value, "value", Action.fValue) ||
				!Read_RequiredUInt32(Value, "durationMs", Action.iDurationMs))
			{
				return false;
			}
			Action.strTrigger = pTrigger->Get_String();
			Action.strKind = pKind->Get_String();
			Action.strTargetId = pTarget->Get_String();
			if ("SUPPRESS_INTER_STEP_PURSUIT" == Action.strKind &&
				("EXIT" != Action.strTrigger ||
				 "boss.sequence.inter-step-pursuit" != Action.strTargetId ||
				 0.f != Action.fValue || 0u != Action.iDurationMs))
			{
				return false;
			}
			if ("RETURN_TO_ARENA_CENTER" == Action.strKind ||
				"SPAWN_COMBAT_OBJECT" == Action.strKind)
			{
				const bool_t bTargetValid = "RETURN_TO_ARENA_CENTER" == Action.strKind ?
					"boss.arena.center" == Action.strTargetId :
					Is_StableToken(Action.strTargetId);
				if ("ENTER" != Action.strTrigger || !bTargetValid ||
					1.f != Action.fValue || 0u != Action.iDurationMs)
					return false;
			}
			if ("DAMAGE_GRABBED_PLAYERS" == Action.strKind ||
				"EXECUTE_GRABBED_PLAYERS" == Action.strKind)
			{
				const bool_t bTargetValid = "DAMAGE_GRABBED_PLAYERS" == Action.strKind ?
					(Is_StableToken(Action.strTargetId) && Action.strTargetId.starts_with("damage.")) :
					"boss.attachment.left-hand" == Action.strTargetId;
				if ("ENTER" != Action.strTrigger || !bTargetValid ||
					0.f != Action.fValue || 0u != Action.iDurationMs)
					return false;
			}
			Out.push_back(std::move(Action));
		}
		return true;
	}

	// outcome별 중복을 거절하고 로컬 action과 다른 패턴 연결을 구분한다. 둘을 동시에 지정할 수 없다.
	// 입력 순서를 보존하며 실제 대상의 존재와 그래프 순환은 상위 결합에서 확인한다.
	bool_t Read_StageBranches(
		const DATA_JSON_VALUE* pValue,
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;
		std::set<std::string, std::less<>> Outcomes;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactPropertiesWithOptional(
					Value, { "outcome", "nextActionId" }, { "nextPatternId" }))
				return false;
			const DATA_JSON_VALUE* pOutcome = Required(
				Value, "outcome", DATA_JSON_TYPE::STRING);
			std::string strNextActionId;
			const DATA_JSON_VALUE* const pNextPatternId =
				Value.Find("nextPatternId");
			if (nullptr == pOutcome || !Is_StableToken(pOutcome->Get_String()) ||
				!Outcomes.insert(pOutcome->Get_String()).second ||
				!Read_NullableStableToken(
					Value, "nextActionId", strNextActionId) ||
				(nullptr != pNextPatternId &&
				 (!pNextPatternId->Is_String() ||
				  !Is_StableToken(pNextPatternId->Get_String()))) ||
				(!strNextActionId.empty() && nullptr != pNextPatternId))
			{
				return false;
			}
			Client::VALTAN_STAGE_BRANCH_VIEW Branch;
			Branch.strOutcome = pOutcome->Get_String();
			if (!strNextActionId.empty())
				Branch.strNextActionId = std::move(strNextActionId);
			if (nullptr != pNextPatternId)
				Branch.strNextPatternId = pNextPatternId->Get_String();
			Out.push_back(std::move(Branch));
		}
		return true;
	}

	// 이동 차단 분기는 잡기 반응에서만 허용한다. 이 검사는 branch 대상의 존재까지 확인하지 않는다.
	bool_t Has_ValidNavigationBlockedCapture(
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Branches,
		const std::string_view strPlayerResponse)
	{
		return "CAPTURE" == strPlayerResponse || std::none_of(
			Branches.begin(), Branches.end(),
			[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "NAVIGATION_BLOCKED" == Branch.strOutcome; });
	}

	// 지정 flag를 켜는 ENTER와 끄는 EXIT가 모두 있는지 확인한다. 같은 행의 중복 개수를 제한하는 검사는 아니다.
	bool_t Has_ClosedSplitStageFlag(
		const DATA_JSON_VALUE& Stage,
		const std::string_view strFlagId)
	{
		const DATA_JSON_VALUE* pEvents = Required(
			Stage, "events", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pEvents)
			return false;
		bool_t bEntered = false;
		bool_t bExited = false;
		for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
		{
			if ("SET_BOSS_FLAG" != Read_String(Event, "kind") ||
				strFlagId != Read_String(Event, "flagId"))
			{
				continue;
			}
			const DATA_JSON_VALUE* pEnabled = Required(
				Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
			if (nullptr == pEnabled)
				continue;
			const std::string strTrigger = Read_String(Event, "trigger");
			bEntered = bEntered ||
				("ENTER" == strTrigger && pEnabled->Get_Boolean());
			bExited = bExited ||
				("EXIT" == strTrigger && !pEnabled->Get_Boolean());
		}
		return bEntered && bExited;
	}

	// 부분 파괴, counter 도형과 누적 피해 응답을 선택 확장으로 읽는다. 이전 기본값은 먼저 초기화한다.
	// 도형의 거리 단위는 m, 각도는 degree이며 stage 종류와 분기의 조합은 상위 검증이 맡는다.
	bool_t Read_StageGameplayExtensions(
		const DATA_JSON_VALUE& Stage,
		std::string& strOutPartDamagePolicy,
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& OutCounterProxy,
		std::optional<Client::VALTAN_BOSS_RESPONSE_VIEW>& OutBossResponse)
	{
		strOutPartDamagePolicy = "NORMAL";
		OutCounterProxy.reset();
		OutBossResponse.reset();

		const DATA_JSON_VALUE* pPartDamagePolicy =
			Stage.Find("partDamagePolicy");
		if (nullptr != pPartDamagePolicy)
		{
			if (!pPartDamagePolicy->Is_String() ||
				("NORMAL" != pPartDamagePolicy->Get_String() &&
				 "DESTROY_FIRST_ELIGIBLE" !=
					pPartDamagePolicy->Get_String()))
			{
				return false;
			}
			strOutPartDamagePolicy = pPartDamagePolicy->Get_String();
		}

		const DATA_JSON_VALUE* pCounterProxy = Stage.Find("counterProxy");
		if (nullptr != pCounterProxy)
		{
			Client::VALTAN_COUNTER_PROXY_VIEW CounterProxy;
			const bool_t bLegacyCircle = Has_ExactProperties(*pCounterProxy,
				{ "space", "forwardOffsetM", "rightOffsetM", "radiusM" });
			const bool_t bTypedShape = Has_ExactProperties(*pCounterProxy,
				{ "kind", "forwardOffsetM", "rightOffsetM", "radiusM",
				  "arcDegrees" });
			if (bLegacyCircle)
			{
				CounterProxy.strKind = "BOSS_LOCAL_CIRCLE";
				CounterProxy.strSpace = Read_String(*pCounterProxy, "space");
			}
			else if (bTypedShape)
			{
				CounterProxy.strKind = Read_String(*pCounterProxy, "kind");
				CounterProxy.strSpace = "BOSS_LOCAL";
			}
			else
			{
				return false;
			}
			if (!Read_RequiredFiniteFloat(*pCounterProxy, "forwardOffsetM",
					CounterProxy.fForwardOffsetM) ||
				!Read_RequiredFiniteFloat(*pCounterProxy, "rightOffsetM",
					CounterProxy.fRightOffsetM) ||
				!Read_RequiredFiniteFloat(*pCounterProxy, "radiusM",
					CounterProxy.fRadiusM) ||
				(bTypedShape && !Read_RequiredFiniteFloat(
					*pCounterProxy, "arcDegrees", CounterProxy.fArcDegrees)) ||
				CounterProxy.fForwardOffsetM < -20.f ||
				CounterProxy.fForwardOffsetM > 20.f ||
				CounterProxy.fRightOffsetM < -20.f ||
				CounterProxy.fRightOffsetM > 20.f)
			{
				return false;
			}
			const bool_t bValidCircle =
				"BOSS_LOCAL_CIRCLE" == CounterProxy.strKind &&
				"BOSS_LOCAL" == CounterProxy.strSpace &&
				CounterProxy.fRadiusM >= 0.1f &&
				CounterProxy.fRadiusM <= 20.f &&
				0.f == CounterProxy.fArcDegrees;
			const bool_t bValidForwardArc =
				"BOSS_FORWARD_ARC" == CounterProxy.strKind &&
				0.f == CounterProxy.fForwardOffsetM &&
				0.f == CounterProxy.fRightOffsetM &&
				0.f == CounterProxy.fRadiusM &&
				180.f == CounterProxy.fArcDegrees;
			if (!bValidCircle && !bValidForwardArc)
				return false;
			OutCounterProxy = std::move(CounterProxy);
		}

		const DATA_JSON_VALUE* pBossResponse = Stage.Find("bossResponse");
		if (nullptr != pBossResponse)
		{
			Client::VALTAN_BOSS_RESPONSE_VIEW BossResponse;
			if (!Has_ExactProperties(
					*pBossResponse, { "kind", "threshold" }) ||
				"ACCUMULATED_HEALTH_DAMAGE" !=
					Read_String(*pBossResponse, "kind") ||
				!Read_RequiredUInt32(*pBossResponse, "threshold",
					BossResponse.iThreshold) ||
				0u == BossResponse.iThreshold)
			{
				return false;
			}
			BossResponse.strKind = "ACCUMULATED_HEALTH_DAMAGE";
			OutBossResponse = std::move(BossResponse);
		}
		return true;
	}

	// hit의 기준 원점과 활성 구간을 읽는다. 기준 오프셋은 m/degree, 활성 시각은 stage 시작 기준 ms다.
	// 출력 기본값을 먼저 설정하고 ACTIVE_WINDOW_ONCE가 stage 길이를 넘지 않는지 검사한다.
	bool_t Read_StageHitAuthority(
		const DATA_JSON_VALUE& Stage,
		const uint32_t iStageDurationMs,
		bool_t& bOutHasAnchor,
		std::string& strOutAnchorKind,
		f32_t& fOutForwardOffsetM,
		f32_t& fOutRightOffsetM,
		f32_t& fOutYawOffsetDegrees,
		bool_t& bOutHasActivation,
		uint32_t& iOutStartMs,
		uint32_t& iOutLifetimeMs)
	{
		bOutHasAnchor = false;
		strOutAnchorKind = "BOSS_CURRENT";
		fOutForwardOffsetM = 0.f;
		fOutRightOffsetM = 0.f;
		fOutYawOffsetDegrees = 0.f;
		bOutHasActivation = false;
		iOutStartMs = 0u;
		iOutLifetimeMs = 0u;

		const DATA_JSON_VALUE* pAnchor = Stage.Find("hitAnchor");
		if (nullptr != pAnchor)
		{
			if (!Has_ExactProperties(*pAnchor,
					{ "kind", "forwardOffsetM", "rightOffsetM",
					  "yawOffsetDegrees" }) ||
				!Read_RequiredFiniteFloat(*pAnchor, "forwardOffsetM",
					fOutForwardOffsetM) ||
				!Read_RequiredFiniteFloat(*pAnchor, "rightOffsetM",
					fOutRightOffsetM) ||
				!Read_RequiredFiniteFloat(*pAnchor, "yawOffsetDegrees",
					fOutYawOffsetDegrees) ||
				std::fabs(fOutForwardOffsetM) > 1000.f ||
				std::fabs(fOutRightOffsetM) > 1000.f ||
				std::fabs(fOutYawOffsetDegrees) > 360.f)
			{
				return false;
			}
			strOutAnchorKind = Read_String(*pAnchor, "kind");
			if ("BOSS_CURRENT" != strOutAnchorKind &&
				"STAGE_ORIGIN" != strOutAnchorKind)
			{
				return false;
			}
			bOutHasAnchor = true;
		}

		const DATA_JSON_VALUE* pActivation = Stage.Find("hitActivation");
		if (nullptr == pActivation)
			return true;
		if (!Has_ExactProperties(*pActivation,
				{ "kind", "startMs", "lifetimeMs", "perTargetPolicy" }) ||
			"ACTIVE_WINDOW" != Read_String(*pActivation, "kind") ||
			"ONCE" != Read_String(*pActivation, "perTargetPolicy") ||
			!Read_RequiredUInt32(*pActivation, "startMs", iOutStartMs) ||
			!Read_RequiredUInt32(*pActivation, "lifetimeMs", iOutLifetimeMs) ||
			0u == iOutLifetimeMs ||
			static_cast<uint64_t>(iOutStartMs) + iOutLifetimeMs >
				iStageDurationMs)
		{
			return false;
		}
		bOutHasActivation = true;
		return true;
	}

	// 무력화 성공 분기와 gauge의 시작/종료 event 또는 action이 함께 있는지 검사한다.
	// 분리 원본과 Product의 stage 수직 offset 검사가 호출하는 읽기 전용 조건식이다.
	// events가 존재하면 actions로 대체하지 않으며 성공 분기와 ENTER 양수/EXIT 0 gauge의 존재만 검사한다.
	bool_t Has_ClosedStaggerStageResponse(const DATA_JSON_VALUE& Stage)
	{
		const auto* branches = Stage.Find("branches");
		const auto* actions = Stage.Find("events");
		if (nullptr == actions)
			actions = Stage.Find("actions");
		if (nullptr == branches || !branches->Is_Array() ||
			nullptr == actions || !actions->Is_Array())
			return false;

		const bool_t hasOutcome = std::any_of(
			branches->Get_Array().begin(), branches->Get_Array().end(),
			[](const DATA_JSON_VALUE& branch)
			{
				return Read_String(branch, "outcome") == "STAGGER_BROKEN";
			});

		// 대응하는 event가 하나라도 있는지 조회한다. 순서·개수·추가 충돌 event를 검사하거나
		// 실제로 gauge 상태를 켜고 끄는 함수는 아니다.
		const auto hasGauge = [actions](const std::string_view trigger, const bool_t enabled)
		{
			return std::any_of(actions->Get_Array().begin(), actions->Get_Array().end(),
				[trigger, enabled](const DATA_JSON_VALUE& action)
				{
					const auto* value = action.Find("value");
					return Read_String(action, "kind") == "SET_STAGGER_GAUGE" &&
						Read_String(action, "trigger") == trigger && Is_FiniteNumber(value) &&
						(enabled ? value->Get_Number() > 0.0 : value->Get_Number() == 0.0);
				});
		};
		return hasOutcome && hasGauge("ENTER", true) && hasGauge("EXIT", false);
	}

	// 분리 gameplay stage의 특수 응답과 분기를 함께 검사한다. 부분 파괴, counter, 높이와 이동의 조합을 제한한다.
	bool_t Validate_SplitGameplayStageExtensions(
		const DATA_JSON_VALUE& Stage,
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Branches,
		const std::string_view strPatternId,
		const std::string_view strStageId,
		std::string& strOutError)
	{
		const auto HasBranch = [&Branches](const std::string_view strOutcome)
		{
			return Branches.end() != std::find_if(
				Branches.begin(), Branches.end(),
				[strOutcome](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return Branch.strOutcome == strOutcome; });
		};

		std::string strPartDamagePolicy;
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
		std::optional<Client::VALTAN_BOSS_RESPONSE_VIEW> BossResponse;
		if (!Read_StageGameplayExtensions(
				Stage, strPartDamagePolicy, CounterProxy, BossResponse))
		{
			strOutError = "split gameplay stage extension values are invalid: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		const DATA_JSON_VALUE* pVerticalOffset = Stage.Find("verticalOffsetM");
		const DATA_JSON_VALUE* pStageMotion = Stage.Find("motion");
		if (nullptr != pVerticalOffset &&
			(!Is_FiniteNumber(pVerticalOffset) ||
			 0.0 == pVerticalOffset->Get_Number() ||
			 std::fabs(pVerticalOffset->Get_Number()) > 100.0 ||
			 (!BossResponse.has_value() && !Has_ClosedStaggerStageResponse(Stage)) ||
			 (nullptr != pStageMotion && !pStageMotion->Is_Null())))
		{
			strOutError =
				"split gameplay stage verticalOffsetM requires an active boss response: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		const DATA_JSON_VALUE* pHit = Stage.Find("hit");
		if (!Has_ValidNavigationBlockedCapture(Branches,
				nullptr == pHit ? std::string{} : Read_String(*pHit, "playerResponse")))
		{
			strOutError = "split navigation-blocked branch requires a capture rush: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		if ("DESTROY_FIRST_ELIGIBLE" == strPartDamagePolicy &&
			(!HasBranch("PART_DESTROYED") ||
			 !Has_ClosedSplitStageFlag(Stage, "boss.flag.groggy")))
		{
			strOutError =
				"split gameplay instant part destruction contract is invalid: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		const std::string strStageKind = Read_String(Stage, "stageKind");
		if (CounterProxy.has_value() &&
			(("BOSS_FORWARD_ARC" == CounterProxy->strKind &&
			  "WINDUP" != strStageKind && "ACTIVE" != strStageKind) ||
			 ("BOSS_LOCAL_CIRCLE" == CounterProxy->strKind &&
			  "WINDUP" != strStageKind)))
		{
			strOutError = "split gameplay counterProxy preset/stage kind is invalid: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		const std::size_t iHealthThresholdCount =
			static_cast<std::size_t>(std::count_if(
				Branches.begin(), Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{
					return "HEALTH_DAMAGE_THRESHOLD_REACHED" ==
						Branch.strOutcome;
				}));
		if ((BossResponse.has_value() &&
			 ("ACTIVE" != strStageKind || 1u != iHealthThresholdCount)) ||
			(!BossResponse.has_value() && 0u != iHealthThresholdCount))
		{
			strOutError =
				"split gameplay accumulated health-damage response is incoherent: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		return true;
	}

	// 분리 원본의 counter 창과 성공/실패 stage를 action 또는 pattern ID로 연결한다.
	// flag의 정확한 ENTER/EXIT 쌍, 허용 stage 종류와 전방 TIMEOUT을 확인하며 실제 counter 판정은 수행하지 않는다.
	bool_t Validate_SplitCounterBranchContract(
		const DATA_JSON_VALUE& GameplayPattern,
		const DATA_JSON_VALUE::ARRAY& GameplayPatterns,
		const std::string_view strPatternId,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pStages = Required(
			GameplayPattern, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pStages)
			return false;

		// flag 이벤트를 세 상태로 분류한다. 0은 미지정, 1은 ENTER true/EXIT false 각 한 번,
		// -1은 배열 누락 또는 잘못된 조합이다. 관련 없는 flag 이벤트는 집계하지 않는다.
		const auto FlagState = [](const DATA_JSON_VALUE& Stage,
			const std::string_view strFlagId)
		{
			const DATA_JSON_VALUE* pEvents = Required(
				Stage, "events", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pEvents)
				return -1;
			uint32_t iTotal = 0u;
			uint32_t iEnter = 0u;
			uint32_t iExit = 0u;
			for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
			{
				if ("SET_BOSS_FLAG" != Read_String(Event, "kind") ||
					strFlagId != Read_String(Event, "flagId"))
				{
					continue;
				}
				++iTotal;
				const DATA_JSON_VALUE* pEnabled = Required(
					Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
				if (nullptr == pEnabled)
					continue;
				const std::string strTrigger = Read_String(Event, "trigger");
				if ("ENTER" == strTrigger && pEnabled->Get_Boolean())
					++iEnter;
				if ("EXIT" == strTrigger && !pEnabled->Get_Boolean())
					++iExit;
			}
			if (0u == iTotal)
				return 0;
			return 2u == iTotal && 1u == iEnter && 1u == iExit ? 1 : -1;
		};

		const auto& Stages = pStages->Get_Array();

		// 다른 패턴으로 가는 성공 분기는 그 패턴의 entryActionId로 stage를 찾는다.
		// 반환 포인터는 GameplayPatterns의 JSON 값을 차용하며 이 검사 밖으로 보관하지 않는다.
		const auto ResolvePatternEntry = [&GameplayPatterns](
			const std::string_view strTargetPatternId)
			-> const DATA_JSON_VALUE*
		{
			const auto Pattern = std::find_if(
				GameplayPatterns.begin(), GameplayPatterns.end(),
				[strTargetPatternId](const DATA_JSON_VALUE& Candidate)
				{
					return strTargetPatternId ==
						Read_String(Candidate, "patternId");
				});
			if (Pattern == GameplayPatterns.end())
				return nullptr;
			const std::string strEntryActionId =
				Read_String(*Pattern, "entryActionId");
			const DATA_JSON_VALUE* pTargetStages = Required(
				*Pattern, "stages", DATA_JSON_TYPE::ARRAY);
			if (!Is_StableToken(strEntryActionId) || nullptr == pTargetStages)
				return nullptr;
			const auto Entry = std::find_if(
				pTargetStages->Get_Array().begin(),
				pTargetStages->Get_Array().end(),
				[&strEntryActionId](const DATA_JSON_VALUE& Candidate)
				{
					return strEntryActionId ==
						Read_String(Candidate, "actionId");
				});
			return Entry == pTargetStages->Get_Array().end() ?
				nullptr : &*Entry;
		};

		// stage별 COUNTER_HIT/TIMEOUT 분기와 counterable flag를 함께 확인한다.
		// COUNTER_HIT 없이 proxy를 가진 대기 stage와 실제 counter 창의 조건을 아래에서 나눈다.
		for (std::size_t iStageIndex = 0u;
			iStageIndex < Stages.size(); ++iStageIndex)
		{
			const DATA_JSON_VALUE& Stage = Stages[iStageIndex];
			const std::string strStageId = Read_String(Stage, "stageId");
			std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> Branches;
			if (!Read_StageBranches(Stage.Find("branches"), Branches))
				return false;
			const std::size_t iCounterCount = static_cast<std::size_t>(
				std::count_if(
					Branches.begin(), Branches.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{ return "COUNTER_HIT" == Branch.strOutcome; }));
			const auto Counter = std::find_if(
				Branches.begin(), Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "COUNTER_HIT" == Branch.strOutcome; });
			const std::size_t iTimeoutCount = static_cast<std::size_t>(
				std::count_if(
					Branches.begin(), Branches.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{ return "TIMEOUT" == Branch.strOutcome; }));
			const auto Timeout = std::find_if(
				Branches.begin(), Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "TIMEOUT" == Branch.strOutcome; });
			const int iCounterState = FlagState(Stage, "boss.flag.counterable");
			std::string strPartDamagePolicy;
			std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
			std::optional<Client::VALTAN_BOSS_RESPONSE_VIEW> BossResponse;
			if (!Read_StageGameplayExtensions(Stage, strPartDamagePolicy,
					CounterProxy, BossResponse))
			{
				return false;
			}

			// COUNTER_HIT가 없으면 counterable flag도 없어야 한다. proxy만 미리 준비했다면
			// 같은 패턴 안의 뒤쪽 stage로 향하는 TIMEOUT 하나를 요구한다.
			if (0u == iCounterCount)
			{
				if (0 != iCounterState)
				{
					strOutError = "split counterable flag has no COUNTER_HIT owner: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}
				if (CounterProxy.has_value())
				{
					const DATA_JSON_VALUE* pTimeoutTarget = nullptr;
					std::size_t iTimeoutTargetIndex = Stages.size();
					if (1u == iTimeoutCount && Timeout != Branches.end() &&
						Timeout->strNextActionId.has_value() &&
						!Timeout->strNextPatternId.has_value())
					{
						for (std::size_t iCandidate = 0u;
							iCandidate < Stages.size(); ++iCandidate)
						{
							if (*Timeout->strNextActionId == Read_String(
									Stages[iCandidate], "actionId"))
							{
								pTimeoutTarget = &Stages[iCandidate];
								iTimeoutTargetIndex = iCandidate;
							}
						}
					}
					if (nullptr == pTimeoutTarget ||
						iTimeoutTargetIndex <= iStageIndex)
					{
						strOutError =
							"split dormant counter proxy requires one forward same-pattern TIMEOUT: " +
							std::string(strPatternId) + "/" + strStageId;
						return false;
					}
				}
			}
			else
			{
				// 실제 counter 창은 성공 분기 하나와 TIMEOUT 하나를 갖는다.
				// 성공은 로컬 action 또는 다른 패턴 중 하나만, TIMEOUT은 로컬 action만 가리켜야 한다.
				const bool_t bCounterUsesLocalSuccess =
					Counter != Branches.end() &&
					Counter->strNextActionId.has_value();
				const bool_t bCounterUsesPatternSuccess =
					Counter != Branches.end() &&
					Counter->strNextPatternId.has_value();
				if (1u != iCounterCount || 1u != iTimeoutCount ||
					Branches.end() == Counter || Branches.end() == Timeout ||
					bCounterUsesLocalSuccess == bCounterUsesPatternSuccess ||
					!Timeout->strNextActionId.has_value() ||
					Timeout->strNextPatternId.has_value())
				{
					strOutError =
						"split Counter source requires exactly one COUNTER_HIT and TIMEOUT edge: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}

				// 로컬 대상은 현재 stage보다 뒤에 있어야 하므로 배열 index를 함께 찾는다.
				// 다른 패턴 성공은 자신에게 되돌아갈 수 없으며 그 패턴의 진입 stage로 해석한다.
				const DATA_JSON_VALUE* pTarget = nullptr;
				const DATA_JSON_VALUE* pTimeoutTarget = nullptr;
				std::size_t iTargetIndex = Stages.size();
				std::size_t iTimeoutTargetIndex = Stages.size();
				for (std::size_t iCandidate = 0u;
					iCandidate < Stages.size(); ++iCandidate)
				{
					const DATA_JSON_VALUE& Candidate = Stages[iCandidate];
					const std::string strActionId =
						Read_String(Candidate, "actionId");
					if (bCounterUsesLocalSuccess &&
						*Counter->strNextActionId == strActionId)
					{
						pTarget = &Candidate;
						iTargetIndex = iCandidate;
					}
					if (*Timeout->strNextActionId == strActionId)
					{
						pTimeoutTarget = &Candidate;
						iTimeoutTargetIndex = iCandidate;
					}
				}
				if (bCounterUsesPatternSuccess &&
					*Counter->strNextPatternId != strPatternId)
				{
					pTarget = ResolvePatternEntry(
						*Counter->strNextPatternId);
				}

				// 성공 대상은 WINDUP/GROGGY/RECOVERY 중 하나다. GROGGY에는 닫힌 groggy flag를,
				// 나머지 종류에는 그 flag의 부재를 요구하고 출발 stage와 proxy 종류도 함께 검사한다.
				const std::string strTargetKind = nullptr == pTarget ?
					std::string{} : Read_String(*pTarget, "stageKind");
				const bool bTypedSuccessTarget = "WINDUP" == strTargetKind ||
					"GROGGY" == strTargetKind || "RECOVERY" == strTargetKind;
				const int iTargetGroggyState = nullptr == pTarget ? -1 :
					FlagState(*pTarget, "boss.flag.groggy");
				const std::string strSourceKind =
					Read_String(Stage, "stageKind");
				const bool_t bValidSourceKind = "WINDUP" == strSourceKind ||
					("ACTIVE" == strSourceKind && CounterProxy.has_value() &&
					 "BOSS_FORWARD_ARC" == CounterProxy->strKind);
				if (!bValidSourceKind ||
					1 != iCounterState || nullptr == pTarget ||
					nullptr == pTimeoutTarget || !bTypedSuccessTarget ||
					(bCounterUsesLocalSuccess && iTargetIndex <= iStageIndex) ||
					iTimeoutTargetIndex <= iStageIndex ||
					("GROGGY" == strTargetKind && 1 != iTargetGroggyState) ||
					("GROGGY" != strTargetKind && 0 != iTargetGroggyState))
				{
					strOutError =
						"split Counter branch requires a closed typed window plus forward local/cross-pattern success and same-pattern TIMEOUT targets: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}
			}

			// 분기 유무와 관계없이 각 stage의 GROGGY 종류와 groggy flag 계약을 마지막으로 확인한다.
			const int iGroggyState = FlagState(Stage, "boss.flag.groggy");
			if (("GROGGY" == Read_String(Stage, "stageKind") &&
				 1 != iGroggyState) ||
				("GROGGY" != Read_String(Stage, "stageKind") &&
				 0 != iGroggyState))
			{
				strOutError = "split groggy stage flag transition is invalid: " +
					std::string(strPatternId) + "/" + strStageId;
				return false;
			}
		}
		return true;
	}

	// 필수 offset 배열을 엄격한 오름차순 ms 목록으로 읽는다. 빈 배열은 이 reader에서 허용한다.
	// 출력을 먼저 비우므로 실패 시 기존 목록을 보존하는 함수는 아니다.
	bool_t Read_RequiredHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& Out)
	{
		Out.clear();
		const DATA_JSON_VALUE* pOffsets = Required(
			Object, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffsets || pOffsets->Get_Array().size() > 1000u)
			return false;
		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Is_NonNegativeInteger(&Value))
				return false;
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			Out.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	// 패턴 단위 도약의 anchor, 착지 위치와 이륙/이동 ms 구간을 읽는다. 위치와 높이는 m 단위다.
	// 실제 travel stage와 첫 stage 시간에 맞는지는 별도 결합에서 검사한다.
	bool_t Read_PatternServerMotion(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		if (!Has_ExactPropertiesWithOptional(*pValue,
				{ "kind", "anchorId", "landingPosition", "apexHeight",
				  "travelStageId", "takeoffStartMs", "takeoffEndMs",
				  "travelStartMs", "travelEndMs" },
				{ "moveToAnchorBeforeTakeoff" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnchor = Required(
			*pValue, "anchorId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTravel = Required(
			*pValue, "travelStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pLanding = Required(
			*pValue, "landingPosition", DATA_JSON_TYPE::ARRAY);
		Client::VALTAN_PATTERN_SERVER_MOTION_VIEW Motion;
		if (nullptr == pKind || nullptr == pAnchor || nullptr == pTravel ||
			("LEAP_TO_ANCHOR" != pKind->Get_String() &&
			 "LEAP_TO_TARGET" != pKind->Get_String()) ||
			!Is_StableToken(pAnchor->Get_String()) ||
			!Is_StableToken(pTravel->Get_String()) || nullptr == pLanding ||
			3u != pLanding->Get_Array().size() ||
			!Is_NonNegativeInteger(pValue->Find("takeoffStartMs")) ||
			!Is_NonNegativeInteger(pValue->Find("takeoffEndMs")) ||
			!Is_NonNegativeInteger(pValue->Find("travelStartMs")) ||
			!Is_NonNegativeInteger(pValue->Find("travelEndMs")) ||
			!Read_RequiredFiniteFloat(*pValue, "apexHeight", Motion.fApexHeight) ||
			Motion.fApexHeight <= 0.f || Motion.fApexHeight > 200.f)
		{
			return false;
		}
		for (size_t i = 0u; i < Motion.LandingPosition.size(); ++i)
		{
			const DATA_JSON_VALUE& Coordinate = pLanding->Get_Array()[i];
			if (!Is_FiniteNumber(&Coordinate) ||
				std::abs(Coordinate.Get_Number()) > static_cast<double>(
					(std::numeric_limits<f32_t>::max)()))
			{
				return false;
			}
			Motion.LandingPosition[i] = static_cast<f32_t>(Coordinate.Get_Number());
		}
		Motion.strKind = pKind->Get_String();
		Motion.strAnchorId = pAnchor->Get_String();
		Motion.strTravelStageId = pTravel->Get_String();
		Motion.iTakeoffStartMs = static_cast<uint32_t>(
			pValue->Find("takeoffStartMs")->Get_Number());
		Motion.iTakeoffEndMs = static_cast<uint32_t>(
			pValue->Find("takeoffEndMs")->Get_Number());
		Motion.iTravelStartMs = static_cast<uint32_t>(
			pValue->Find("travelStartMs")->Get_Number());
		Motion.iTravelEndMs = static_cast<uint32_t>(
			pValue->Find("travelEndMs")->Get_Number());
		if (const DATA_JSON_VALUE* pApproach = pValue->Find("moveToAnchorBeforeTakeoff"))
		{
			if (!pApproach->Is_Boolean())
				return false;
			Motion.bMoveToAnchorBeforeTakeoff = pApproach->Get_Boolean();
		}
		if ((Motion.bMoveToAnchorBeforeTakeoff && 0u == Motion.iTakeoffStartMs) ||
			Motion.iTakeoffStartMs >= Motion.iTakeoffEndMs ||
			Motion.iTravelStartMs >= Motion.iTravelEndMs)
		{
			return false;
		}
		Out = std::move(Motion);
		return true;
	}

	// 유령/포탈 finale의 자식 패턴 ID와 생성 범위를 읽는다. 자식 ID 중복은 거절하고 실제 정의는 상위 문서에서 연결한다.
	bool_t Read_PatternFinale(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_PATTERN_FINALE_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue)
			return true;
		if (!Has_ExactPropertiesWithOptional(*pValue,
				{ "kind", "ghostArchetypeId", "ghostPatternIds",
				  "spawnHalfExtentsM", "maximumActiveGhosts" },
				{ "auxiliarySpawnIntervalMs", "portalSpawnIntervalMs" }))
			return false;
		Client::VALTAN_PATTERN_FINALE_VIEW Finale;
		Finale.strKind = Read_String(*pValue, "kind");
		Finale.strGhostArchetypeId = Read_String(*pValue, "ghostArchetypeId");
		const auto ReadInterval = [&](const char* name, uint32_t minimum, uint32_t& interval)
		{
			return nullptr == pValue->Find(name) ||
				(Read_RequiredUInt32(*pValue, name, interval) &&
				 interval >= minimum && interval <= 600000u);
		};
		if (!ReadInterval("auxiliarySpawnIntervalMs", 1u, Finale.iAuxiliarySpawnIntervalMs) ||
			!ReadInterval("portalSpawnIntervalMs", 0u, Finale.iPortalSpawnIntervalMs))
			return false;
		const DATA_JSON_VALUE* pPatterns = Required(
			*pValue, "ghostPatternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pExtents = Required(
			*pValue, "spawnHalfExtentsM", DATA_JSON_TYPE::ARRAY);
		if ("GHOST_PORTAL_LOOP" != Finale.strKind ||
			!Is_StableToken(Finale.strGhostArchetypeId) ||
			!Read_RequiredUInt32(*pValue, "maximumActiveGhosts", Finale.iMaximumActiveGhosts) ||
			Finale.iMaximumActiveGhosts < 1u || Finale.iMaximumActiveGhosts > 64u ||
			nullptr == pPatterns || pPatterns->Get_Array().empty() ||
			pPatterns->Get_Array().size() > 64u ||
			nullptr == pExtents || 2u != pExtents->Get_Array().size())
			return false;
		std::set<std::string, std::less<>> PatternIds;
		Finale.GhostPatternIds.reserve(pPatterns->Get_Array().size());
		for (const DATA_JSON_VALUE& Pattern : pPatterns->Get_Array())
		{
			if (!Pattern.Is_String() || !Is_StableToken(Pattern.Get_String()) ||
				!PatternIds.insert(Pattern.Get_String()).second)
				return false;
			Finale.GhostPatternIds.push_back(Pattern.Get_String());
		}
		for (size_t iAxis = 0u; iAxis < 2u; ++iAxis)
		{
			const DATA_JSON_VALUE& Extent = pExtents->Get_Array()[iAxis];
			if (!Is_FiniteNumber(&Extent) || Extent.Get_Number() < 1.0 ||
				Extent.Get_Number() > 100.0)
				return false;
			Finale.SpawnHalfExtentsM[iAxis] = static_cast<f32_t>(Extent.Get_Number());
		}
		Out = std::move(Finale);
		return true;
	}

	// 분리 원본을 결합하면서 사용하는 내부 참조 값이다. JSON 객체 포인터나 Server 실행 객체를 보관하지 않는다.
	struct MASTER_EFFECT_REFERENCE final
	{
		std::string strType;
		std::string strId;
		bool_t bUsesStageClock = false;
		uint32_t iStageOffsetMs = 0u;
		std::string strClipOccurrenceId;
		std::string strMappingBasis;
		uint32_t iSourceStartMs = 0u;
		uint32_t iSourceEndMs = 0u;
		bool_t bHasSourceEnd = false;
	};

	// 한 stage의 저작 gameplay와 presentation을 값으로 결합하는 임시 구조다. 공개 뷰를 교체하기 전에 검사한다.
	struct MASTER_STAGE final
	{
		std::string strStageId;
		std::string strSequenceRole;
		std::string strActionId;
		std::string strStageKind;
		uint32_t iDurationMs = 0u;
		f32_t fVerticalOffsetM = 0.f;
		uint32_t iRepeatCount = 0u;
		std::string strAnimationEndPolicy;
		bool_t bSuppressAnimation = false;
		bool_t bHasBodyHiddenWindow = false;
		uint32_t iBodyHiddenFromMs = 0u;
		uint32_t iBodyHiddenToMs = 0u;
		std::string strHitShape;
		f32_t fHitOuterRadius = 0.f;
		f32_t fHitInnerRadius = 0.f;
		f32_t fHitAngleDegrees = 0.f;
		f32_t fHitLength = 0.f;
		f32_t fHitHalfWidth = 0.f;
		uint32_t iHitCount = 0u;
		uint32_t iHitIntervalMs = 0u;
		uint32_t iHitDelayMs = 0u;
		std::vector<uint32_t> HitOffsetsMs;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackContacts;
		bool_t bHasHitAnchor = false;
		std::string strHitAnchorKind = "BOSS_CURRENT";
		f32_t fHitAnchorForwardOffsetM = 0.f;
		f32_t fHitAnchorRightOffsetM = 0.f;
		f32_t fHitAnchorYawOffsetDegrees = 0.f;
		bool_t bHasHitActivation = false;
		uint32_t iHitActivationStartMs = 0u;
		uint32_t iHitActivationLifetimeMs = 0u;
		std::string strServerDamageProfileId;
		std::string strPlayerResponse = "DAMAGE";
		std::string strAttachmentSlot = "NONE";
		std::optional<Client::PLAYER_HAND_GRIP_LOCAL_OFFSET> GripLocalOffset;
		std::string strPartDamagePolicy = "NORMAL";
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
		std::optional<Client::VALTAN_BOSS_RESPONSE_VIEW> BossResponse;
		f32_t fPushRangeM = 0.f;
		uint32_t iPushMs = 0u;
		bool_t bKnockdown = false;
		uint32_t iDownMs = 0u;
		std::string strAimTargetPolicy;
		bool_t bHasAimEnd = false;
		uint32_t iAimEndMs = 0u;
		bool_t bHasAimResponseScale = false;
		f32_t fAimResponseScale = 1.f;
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW> Motion;
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW> Actions;
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> Branches;
		std::vector<Client::VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		std::vector<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW> AuthoredCues;
		std::vector<MASTER_EFFECT_REFERENCE> EffectReferences;
		bool_t bHasStageEffectV2Bindings = false;
		std::vector<Client::VALTAN_CAMERA_INVOCATION_VIEW> CameraInvocations;
        std::vector<Client::BOSS_STAGE_SCENE_PROFILE_OCCURRENCE> SceneProfileOccurrences;
        std::vector<Client::BOSS_STAGE_LIGHT_OCCURRENCE> LightOccurrences;
	};

	template <typename TStage>
	// 도약의 이륙 시각은 첫 stage 안에, travel 구간은 뒤쪽 지정 stage 안에 있어야 한다.
	bool_t Validate_PatternServerMotionStageWindows(
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Motion,
		const std::vector<TStage>& Stages)
	{
		if (!Motion.has_value())
			return true;
		if (Stages.empty() ||
			Motion->iTakeoffStartMs >= Motion->iTakeoffEndMs ||
			Motion->iTakeoffEndMs > Stages.front().iDurationMs ||
			Motion->iTravelStartMs >= Motion->iTravelEndMs)
		{
			return false;
		}
		const auto Travel = std::find_if(
			Stages.begin() + 1u, Stages.end(),
			[&Motion](const TStage& Stage)
			{ return Stage.strStageId == Motion->strTravelStageId; });
		return Stages.end() != Travel &&
			Motion->iTravelEndMs <= Travel->iDurationMs;
	}

	// 하나의 패턴 저작 정의와 출처, stage를 소유한다. 실행 중 HP나 분기 상태를 가진 Server 패턴 인스턴스는 아니다.
	struct MASTER_PATTERN final
	{
		std::string strPatternId;
		std::string strCategory;
		uint32_t iMinimumPhase = 0u;
		uint32_t iMaximumPhase = 0u;
		std::string strTargetPolicy;
		std::string strAimPolicy;
		std::string strDisplayName;
		std::string strActionId;
		std::vector<uint32_t> SourceActionIds;
		std::string strSelectionMode;
		int32_t iMinimumHealthBar = 0;
		int32_t iMaximumHealthBar = 0;
		int32_t iTriggerHealthBar = 0;
		uint32_t iTriggerOrder = 0u;
		std::string strArmorRequirement;
		std::string strPhaseRequirement;
		bool_t bInvulnerableWhileRunning = false;
		uint32_t iSelectionWeight = 0u;
		uint32_t iMaximumConsecutiveUses = 0u;
		f32_t fMinimumRange = 0.f;
		f32_t fMaximumRange = 0.f;
		f32_t fVerticalOffsetM = 0.f;
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW> ServerMotion;
		std::optional<Client::VALTAN_PATTERN_FINALE_VIEW> Finale;
		uint32_t iSourceSequenceIndex = 0u;
		std::vector<Client::VALTAN_PRESENTATION_SOURCE_VIEW> PresentationSources;
		std::vector<Client::VALTAN_PATTERN_REACTION_VIEW> Reactions;
		std::vector<std::string> CameraCueIds;
		std::vector<Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW>
			WorldEventTriggerRefs;
		bool_t bManualServerAudition = false;
		std::string strSourceAnimationChainId;
		uint32_t iAuthoringPhase = 0u;
		std::string strAdmissionState;
		std::vector<MASTER_STAGE> Stages;
	};

	// 종료되어야 하는 선택된 패턴에 적용하는 stage 그래프 검사다. action ID 색인과 인접 목록/진입 차수를 만든다.
	// Kahn 방식으로 모든 노드를 소진할 수 있는지 확인한다. 명시적인 잡기 재시도 간선만 아래에서 제외한다.
	bool_t Validate_FinitePatternStageGraph(
		const MASTER_PATTERN& Pattern, std::string& strOutError)
	{
		std::map<std::string, size_t, std::less<>> StageByAction;
		for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
		{
			if (!StageByAction.emplace(Pattern.Stages[iStage].strActionId, iStage).second)
			{
				strOutError = "finite stage graph has a duplicated action: " + Pattern.strPatternId;
				return false;
			}
		}
		std::vector<std::vector<size_t>> Successors(Pattern.Stages.size());
		std::vector<size_t> Incoming(Pattern.Stages.size(), 0u);
		for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
		{
			const MASTER_STAGE& Stage = Pattern.Stages[iStage];
			const auto AddSuccessor = [&](const std::string& ActionId)
			{
				const auto Target = StageByAction.find(ActionId);
				if (StageByAction.end() == Target)
				{
					strOutError = "finite stage graph has a dangling action: " +
						Pattern.strPatternId + "/" + ActionId;
					return false;
				}
				// 잡기 패턴의 지정 재충전 분기는 Server가 반복을 소유한다. 이 좁은 예외만 유한 경로 검사에서 제외한다.
				if ((Pattern.strPatternId == "VALTAN_TRASH" || Pattern.strPatternId == "VALTAN_TRASH_CATCH_IF") &&
					(Stage.strStageId == "RETRY_EXHAUSTED" || Stage.strStageId == "CATCH_SLAM" ||
					 Stage.strStageId == "EXECUTE_TAIL") &&
					Pattern.Stages[Target->second].strStageId == "RECHARGE_WAIT_02" &&
					Stage.Branches.size() == 1u && Stage.Branches.front().strOutcome == "TIMEOUT")
					return true;
				Successors[iStage].push_back(Target->second);
				++Incoming[Target->second];
				return true;
			};
			const auto Timeout = std::find_if(Stage.Branches.begin(), Stage.Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "TIMEOUT" == Branch.strOutcome; });
			// 분리 원본 검증에서 기본 연결은 TIMEOUT 대상과 같아야 한다.
			// TIMEOUT이 없으면 다음 stage의 순차 연결을 이 그래프에도 추가한다.
			if (Stage.Branches.end() == Timeout && iStage + 1u < Pattern.Stages.size() &&
				!AddSuccessor(Pattern.Stages[iStage + 1u].strActionId))
				return false;
			for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
				if (Branch.strNextActionId.has_value() && !AddSuccessor(*Branch.strNextActionId))
					return false;
		}
		// 도달하지 않는 대체 stage까지 진입 차수로 검사한다.
		// 이 검사는 재귀 호출이나 시간 상한에 의존하지 않고 순환 여부를 확인한다.
		std::vector<size_t> Ready;
		Ready.reserve(Pattern.Stages.size());
		for (size_t iStage = 0u; iStage < Incoming.size(); ++iStage)
			if (0u == Incoming[iStage])
				Ready.push_back(iStage);
		for (size_t iReady = 0u; iReady < Ready.size(); ++iReady)
			for (const size_t iSuccessor : Successors[Ready[iReady]])
				if (0u == --Incoming[iSuccessor])
					Ready.push_back(iSuccessor);
		if (Ready.size() != Pattern.Stages.size())
		{
			strOutError = "finite stage graph contains a cycle: " + Pattern.strPatternId;
			return false;
		}
		return true;
	}

	// 편집 순서와 구간별 추적 ms를 값으로 보관한다. 같은 패턴 ID의 반복도 순서의 일부다.
	struct MASTER_SCRIPTED_SEQUENCE_VIEW final
	{
		std::string strSequenceId;
		std::string strMode;
		std::string strEntranceCinematicPatternId;
		uint32_t iInterStepPursuitMs = 0u;
		std::vector<std::string> PatternIds;
		std::vector<uint32_t> TransitionPursuitMs;
	};

	// 일제 생성 event의 소유 패턴/stage와 typed 투영을 보관한다. 기존 action 뷰에 축약하지 않은 별도 결합 입력이다.
	struct MASTER_VOLLEY_PROJECTION final
	{
		std::string strEventId;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strCombatObjectArchetypeId;
		DATA_JSON_VALUE ProductAction;
	};

	// 분리된 두 저작 문서를 검증해 모은 임시 문서다. map의 차용 포인터는 이 값의 수명 안에서만 사용한다.
	struct MASTER_DOCUMENT final
	{
		std::vector<std::string> RetiredPatternIds;
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		Client::VALTAN_NORMAL_SELECTION_VIEW NormalSelection;
		std::vector<Client::VALTAN_SELECTION_SET_VIEW> SelectionSets;
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW> SelectionWindows;
		std::vector<Client::VALTAN_MECHANIC_VIEW> Mechanics;
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW> ManualAuditions;
		std::vector<Client::VALTAN_COUNTER_REACTION_LAYER_VIEW>
			CounterReactionLayers;
		std::vector<Client::VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;
		std::vector<MASTER_VOLLEY_PROJECTION> VolleyProjections;
		std::vector<MASTER_PATTERN> Patterns;
	};

	// 한 animation occurrence의 stable ID, clip 이름, 원본 ms와 재생 배율을 읽는다. 검증 후 값 필드를 채운다.
	bool_t Parse_MasterOccurrence(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_CLIP_OCCURRENCE_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "clipOccurrenceId", "clip", "mappingBasis",
				  "sourceStartMs", "playMs", "playRate",
				  "repeatUntilStageEnd" }))
		{
			strOutError = "master animation occurrence has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pOccurrenceId = Required(
			Value, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pClip = Required(
			Value, "clip", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pBasis = Required(
			Value, "mappingBasis", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = Value.Find("sourceStartMs");
		const DATA_JSON_VALUE* pPlay = Value.Find("playMs");
		const DATA_JSON_VALUE* pRate = Value.Find("playRate");
		const DATA_JSON_VALUE* pRepeat = Required(
			Value, "repeatUntilStageEnd", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pOccurrenceId || nullptr == pClip || nullptr == pBasis ||
			!Is_StableToken(pOccurrenceId->Get_String()) ||
			!Is_StableToken(pClip->Get_String()) || pBasis->Get_String().empty() ||
			!Is_NonNegativeInteger(pSourceStart) ||
			!Is_NonNegativeInteger(pPlay) || !Is_FiniteNumber(pRate) ||
			pRate->Get_Number() <= 0.05 || pRate->Get_Number() > 16.0 ||
			nullptr == pRepeat)
		{
			strOutError = "master animation occurrence is invalid";
			return false;
		}
		Out.strClipOccurrenceId = pOccurrenceId->Get_String();
		Out.strClipName = pClip->Get_String();
		Out.strMappingBasis = pBasis->Get_String();
		Out.iSourceStartMs = static_cast<uint32_t>(pSourceStart->Get_Number());
		Out.iPlayMs = static_cast<uint32_t>(pPlay->Get_Number());
		Out.fPlayRate = static_cast<f32_t>(pRate->Get_Number());
		Out.bLoop = pRepeat->Get_Boolean();
		return true;
	}

	// 호환 Master stage의 gameplay 필수 필드와 잡기/action 입력 모양을 확인한다. 실제 Server 객체를 만들지 않는다.
	bool_t Validate_MasterStageGameplayShape(
		const DATA_JSON_VALUE& Value,
		std::string& strOutError)
	{
		const std::initializer_list<std::string_view> NumericFields = {
			"hitOuterRadius", "hitInnerRadius", "hitAngleDegrees", "hitLength",
			"hitHalfWidth", "hitCount", "hitIntervalMs", "hitDelayMs",
			"pushRangeM", "pushMs", "downMs" };
		for (const std::string_view Field : NumericFields)
		{
			if (!Is_FiniteNumber(Value.Find(Field)))
			{
				strOutError = "master stage numeric field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		const DATA_JSON_VALUE* pOffsets = Required(
			Value, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* pMotion = Value.Find("motion");
		const DATA_JSON_VALUE* pActions = Required(
			Value, "actions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPlayerResponse = Value.Find("playerResponse");
		const DATA_JSON_VALUE* pAttachmentSlot = Value.Find("attachmentSlot");
		const DATA_JSON_VALUE* pGripLocalOffset = Value.Find("gripLocalOffset");
		std::optional<Client::PLAYER_HAND_GRIP_LOCAL_OFFSET> GripLocalOffset;
		if (nullptr == pOffsets || nullptr == pKnockdown || nullptr == pMotion ||
			nullptr == pActions ||
			(nullptr == pPlayerResponse) != (nullptr == pAttachmentSlot) ||
			(nullptr == pPlayerResponse) != (nullptr == pGripLocalOffset) ||
			!Read_PlayerHandGripLocalOffset(pGripLocalOffset, GripLocalOffset))
		{
			strOutError = "master stage gameplay fields are invalid";
			return false;
		}
		if (nullptr != pPlayerResponse &&
			(!pPlayerResponse->Is_String() || !pAttachmentSlot->Is_String() ||
			 "CAPTURE" != pPlayerResponse->Get_String() ||
			 "BOSS_LEFT_HAND" != pAttachmentSlot->Get_String() ||
			 "NONE" == Read_String(Value, "hitShape") ||
			 0.0 != Read_Number(Value, "pushRangeM") ||
			 0.0 != Read_Number(Value, "pushMs") ||
			 pKnockdown->Get_Boolean() ||
			 0.0 != Read_Number(Value, "downMs")))
		{
			strOutError = "master stage capture hit contract is invalid";
			return false;
		}
		for (const DATA_JSON_VALUE& Offset : pOffsets->Get_Array())
		{
			if (!Is_NonNegativeInteger(&Offset))
			{
				strOutError = "master stage hitOffsetsMs is invalid";
				return false;
			}
		}
		if (!pMotion->Is_Null())
		{
			std::optional<Client::VALTAN_STAGE_MOTION_VIEW> Motion;
			if (!Read_StageMotion(pMotion,
					static_cast<uint32_t>(Read_Number(Value, "durationMs")), Motion))
			{
				strOutError = "master stage motion is invalid";
				return false;
			}
		}
		for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
		{
			if ("RELEASE_GRABBED_PLAYERS" == Read_String(Action, "kind"))
			{
				if (!Has_ExactProperties(Action,
						{ "trigger", "kind", "targetId", "releaseMode",
						  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
					nullptr == Required(
						Action, "trigger", DATA_JSON_TYPE::STRING) ||
					nullptr == Required(
						Action, "targetId", DATA_JSON_TYPE::STRING) ||
					nullptr == Required(
						Action, "releaseMode", DATA_JSON_TYPE::STRING) ||
					!Is_FiniteNumber(Action.Find("speedMps")) ||
					!Is_NonNegativeInteger(Action.Find("durationMs")) ||
					!Is_FiniteNumber(Action.Find("yawOffsetDegrees")))
				{
					strOutError = "master grabbed-player release action is invalid";
					return false;
				}
				const std::string strTrigger = Read_String(Action, "trigger");
				const std::string strTargetId = Read_String(Action, "targetId");
				const std::string strReleaseMode =
					Read_String(Action, "releaseMode");
				const double fSpeedMps = Read_Number(Action, "speedMps");
				const double fDurationMs = Read_Number(Action, "durationMs");
				const double fYawOffsetDegrees =
					Read_Number(Action, "yawOffsetDegrees");
				const bool_t bHold = "HOLD" == strReleaseMode &&
					0.0 == fSpeedMps && 0.0 == fDurationMs &&
					0.0 == fYawOffsetDegrees;
				const bool_t bLaunch =
					("OPPOSITE_KNOCKBACK" == strReleaseMode ||
					 "ARENA_EJECTION" == strReleaseMode) &&
					fSpeedMps > 0.0 && fSpeedMps <= 50.0 &&
					fDurationMs > 0.0 && fDurationMs <= 5000.0 &&
					("ARENA_EJECTION" == strReleaseMode ||
					 0.0 == fYawOffsetDegrees);
				if (("ENTER" != strTrigger && "EXIT" != strTrigger) ||
					"boss.attachment.left-hand" != strTargetId ||
					std::abs(fYawOffsetDegrees) > 180.0 ||
					(!bHold && !bLaunch))
				{
					strOutError =
						"master grabbed-player release action is incoherent";
					return false;
				}
				continue;
			}
			if (!Has_ExactProperties(Action,
					{ "trigger", "kind", "targetId", "value", "durationMs" }) ||
				nullptr == Required(Action, "trigger", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "kind", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "targetId", DATA_JSON_TYPE::STRING) ||
				!Is_FiniteNumber(Action.Find("value")) ||
				!Is_NonNegativeInteger(Action.Find("durationMs")))
			{
				strOutError = "master stage action is invalid";
				return false;
			}
		}
		return true;
	}

	// camera 호출을 stage ms 범위 안에서 읽어 출력 목록에 추가한다. 함수가 기존 목록을 비우지는 않는다.
	bool_t Read_MasterCameraInvocations(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::vector<Client::VALTAN_CAMERA_INVOCATION_VIEW>& Out)
	{
		if (nullptr == pValue || !pValue->Is_Array())
			return false;
		std::set<std::string, std::less<>> InvocationIds;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactProperties(Value,
					{ "cameraInvocationId", "cameraCueId", "trigger",
					  "startOffsetMs", "durationPolicy", "durationMs" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* pInvocationId = Required(
				Value, "cameraInvocationId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pCueId = Required(
				Value, "cameraCueId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTrigger = Required(
				Value, "trigger", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pDurationPolicy = Required(
				Value, "durationPolicy", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStartOffset = Value.Find("startOffsetMs");
			const DATA_JSON_VALUE* pDuration = Value.Find("durationMs");
			if (nullptr == pInvocationId || nullptr == pCueId ||
				nullptr == pTrigger || nullptr == pDurationPolicy ||
				!Is_StableToken(pInvocationId->Get_String()) ||
				!Is_StableToken(pCueId->Get_String()) ||
				"ENTER" != pTrigger->Get_String() ||
				"EXPLICIT" != pDurationPolicy->Get_String() ||
				!Is_NonNegativeInteger(pStartOffset) ||
				!Is_NonNegativeInteger(pDuration) ||
				!InvocationIds.insert(pInvocationId->Get_String()).second)
			{
				return false;
			}
			const uint64_t iEnd = static_cast<uint64_t>(
				pStartOffset->Get_Number()) + static_cast<uint64_t>(
				pDuration->Get_Number());
			if (iEnd > iStageDurationMs)
				return false;
			Client::VALTAN_CAMERA_INVOCATION_VIEW Invocation;
			Invocation.strCameraInvocationId = pInvocationId->Get_String();
			Invocation.strCameraCueId = pCueId->Get_String();
			Invocation.strTrigger = pTrigger->Get_String();
			Invocation.iStartOffsetMs = static_cast<uint32_t>(
				pStartOffset->Get_Number());
			Invocation.strDurationPolicy = pDurationPolicy->Get_String();
			Invocation.iDurationMs = static_cast<uint32_t>(
				pDuration->Get_Number());
			Out.push_back(std::move(Invocation));
		}
		return true;
	}

	// 호환 Master stage의 gameplay, 애니메이션, Effect 참조와 camera를 함께 읽는다.
	// Out은 진행 중 채워지는 임시 값이며 실패 시 기존 stage를 보존하는 공개 저장 경계가 아니다.
	bool_t Parse_MasterStage(
		const DATA_JSON_VALUE& Value,
		MASTER_STAGE& Out,
		std::string& strOutError)
	{
		// 공통 stage 스키마와 잡기 응답 필드를 모두 가진 스키마 중 하나만 받는다.
		// 선택 필드가 아닌 미지의 속성이나 잡기 필드의 일부만 가진 조합은 거부한다.
		const bool_t bBaseShape = Has_ExactPropertiesWithOptional(Value,
				{ "stageId", "sequenceRole", "actionId", "stageKind",
				  "durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				  "hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				  "hitIntervalMs", "hitDelayMs", "hitOffsetsMs",
				  "serverDamageProfileId", "pushRangeM", "pushMs", "knockdown",
				  "downMs", "motion", "actions", "branches", "animation",
				  "effectRefs", "cameraInvocations" },
				{ "partDamagePolicy", "counterProxy", "bossResponse", "hitAnchor",
				  "hitActivation", "bodyVisibility", "verticalOffsetM", "sceneProfileOccurrences", "lightOccurrences", "attackContacts", "aim" });
		const bool_t bCaptureShape = Has_ExactPropertiesWithOptional(Value,
				{ "stageId", "sequenceRole", "actionId", "stageKind",
				  "durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				  "hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				  "hitIntervalMs", "hitDelayMs", "hitOffsetsMs",
				  "serverDamageProfileId", "playerResponse", "attachmentSlot",
				  "gripLocalOffset",
				  "pushRangeM", "pushMs", "knockdown", "downMs", "motion",
				  "actions", "branches", "animation", "effectRefs",
				  "cameraInvocations" },
				{ "partDamagePolicy", "counterProxy", "bossResponse", "hitAnchor",
				  "hitActivation", "bodyVisibility", "verticalOffsetM", "sceneProfileOccurrences", "lightOccurrences", "attackContacts", "aim" });
		if (!bBaseShape && !bCaptureShape)
		{
			strOutError = "master stage has unexpected properties";
			return false;
		}

		// stage/role/action의 stable identity와 필수 배열·객체 타입을 확보한 뒤
		// 공통 gameplay 도형 검증을 호출한다. 아직 Out에 값을 게시하지 않는다.
		const DATA_JSON_VALUE* pStageId = Required(
			Value, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRole = Required(
			Value, "sequenceRole", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pKind = Required(
			Value, "stageKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDuration = Value.Find("durationMs");
		const DATA_JSON_VALUE* pShape = Required(
			Value, "hitShape", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDamage = Required(
			Value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnimation = Required(
			Value, "animation", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pBranches = Required(
			Value, "branches", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pEffects = Required(
			Value, "effectRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pStageId || nullptr == pRole || nullptr == pAction ||
			nullptr == pKind || !Is_NonNegativeInteger(pDuration) ||
			0.0 == pDuration->Get_Number() || nullptr == pShape ||
			nullptr == pDamage || nullptr == pAnimation || nullptr == pBranches ||
			nullptr == pEffects ||
			!Is_StableToken(pStageId->Get_String()) ||
			!Is_StableToken(pRole->Get_String()) ||
			!Is_StableToken(pAction->Get_String()))
		{
			strOutError = "master stage identity is invalid";
			return false;
		}
		if (!Validate_MasterStageGameplayShape(Value, strOutError))
			return false;

		// animation의 NONE은 기존 본체 pose를 유지하는 표현이다.
		// 그 외에는 명시 occurrence 목록·반복 횟수·종료 정책을 함께 요구한다.
		const bool_t bSuppressAnimation = Has_ExactProperties(
			*pAnimation, { "mode" });
		const DATA_JSON_VALUE* pRepeatCount = nullptr;
		const DATA_JSON_VALUE* pEndPolicy = nullptr;
		const DATA_JSON_VALUE* pOccurrences = nullptr;
		if (bSuppressAnimation)
		{
			const DATA_JSON_VALUE* pMode = Required(
				*pAnimation, "mode", DATA_JSON_TYPE::STRING);
			if (nullptr == pMode || "NONE" != pMode->Get_String())
			{
				strOutError = "master stage animation mode is invalid";
				return false;
			}
		}
		else
		{
			if (!Has_ExactProperties(
					*pAnimation, { "repeatCount", "endPolicy", "occurrences" }))
			{
				strOutError = "master stage animation has unexpected properties";
				return false;
			}
			pRepeatCount = pAnimation->Find("repeatCount");
			pEndPolicy = Required(
				*pAnimation, "endPolicy", DATA_JSON_TYPE::STRING);
			pOccurrences = Required(
				*pAnimation, "occurrences", DATA_JSON_TYPE::ARRAY);
			if (!Is_NonNegativeInteger(pRepeatCount) ||
				pRepeatCount->Get_Number() < 1.0 ||
				pRepeatCount->Get_Number() > MAX_BOSS_PATTERN_ANIMATION_CLIPS || nullptr == pOccurrences ||
				pOccurrences->Get_Array().empty() || nullptr == pEndPolicy ||
				("EXACT" != pEndPolicy->Get_String() &&
				 "HOLD_LAST_POSE" != pEndPolicy->Get_String() &&
				 "LOOP_TO_STAGE_END" != pEndPolicy->Get_String()) ||
				pOccurrences->Get_Array().size() > MAX_BOSS_PATTERN_ANIMATION_CLIPS)
			{
				strOutError = "master stage repeatCount/occurrences is invalid";
				return false;
			}
		}

		// 검사한 identity와 시간·애니메이션 정책을 임시 Out에 옮긴다.
		// 이 뒤 실패에서는 여기까지 채운 값이 남으므로 상위 parser가 임시 stage로 사용한다.
		Out.strStageId = pStageId->Get_String();
		Out.strSequenceRole = pRole->Get_String();
		Out.strActionId = pAction->Get_String();
		Out.strStageKind = pKind->Get_String();
		Out.iDurationMs = static_cast<uint32_t>(pDuration->Get_Number());
		const DATA_JSON_VALUE* pVerticalOffset = Value.Find("verticalOffsetM");
		if (nullptr != pVerticalOffset &&
			(!Is_FiniteNumber(pVerticalOffset) ||
			 0.0 == pVerticalOffset->Get_Number() ||
			 std::fabs(pVerticalOffset->Get_Number()) > 100.0))
		{
			strOutError = "master stage verticalOffsetM is invalid";
			return false;
		}
		Out.fVerticalOffsetM = nullptr == pVerticalOffset ? 0.f :
			static_cast<f32_t>(pVerticalOffset->Get_Number());
		Out.bSuppressAnimation = bSuppressAnimation;
		Out.iRepeatCount = bSuppressAnimation ? 0u :
			static_cast<uint32_t>(pRepeatCount->Get_Number());
		Out.strAnimationEndPolicy = bSuppressAnimation ? "NONE" :
			pEndPolicy->Get_String();
		if (!Read_StageBodyVisibility(
				Value.Find("bodyVisibility"), Out.iDurationMs,
				Out.bHasBodyHiddenWindow, Out.iBodyHiddenFromMs,
				Out.iBodyHiddenToMs))
		{
			strOutError = "master stage bodyVisibility is invalid";
			return false;
		}

		// 일반 피해와 잡기 응답을 구분해 손 부착 오프셋 존재 여부를 맞춘다.
		// 잡기 스키마가 아니면 DAMAGE/NONE 기본값을 사용한다.
		Out.strHitShape = pShape->Get_String();
		Out.strServerDamageProfileId = pDamage->Get_String();
		Out.strPlayerResponse = bCaptureShape ?
			Read_String(Value, "playerResponse") : "DAMAGE";
		Out.strAttachmentSlot = bCaptureShape ?
			Read_String(Value, "attachmentSlot") : "NONE";
		if (!Read_PlayerHandGripLocalOffset(
				Value.Find("gripLocalOffset"), Out.GripLocalOffset) ||
			(bCaptureShape != Out.GripLocalOffset.has_value()))
		{
			strOutError = "master stage gripLocalOffset is invalid";
			return false;
		}

		// 도형·hit 일정·밀어내기 수치를 먼저 읽고 이동·action·분기·응답·환경을 결합한다.
		// 각 reader가 Out을 채우므로 전체 조건식의 단락 평가와 호출 순서가 실패 경계를 결정한다.
		if (!Read_RequiredFiniteFloat(
				Value, "hitOuterRadius", Out.fHitOuterRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitInnerRadius", Out.fHitInnerRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitAngleDegrees", Out.fHitAngleDegrees) ||
			!Read_RequiredFiniteFloat(Value, "hitLength", Out.fHitLength) ||
			!Read_RequiredFiniteFloat(
				Value, "hitHalfWidth", Out.fHitHalfWidth) ||
			!Read_RequiredUInt32(Value, "hitCount", Out.iHitCount) ||
			!Read_RequiredUInt32(Value, "hitIntervalMs", Out.iHitIntervalMs) ||
			!Read_RequiredUInt32(Value, "hitDelayMs", Out.iHitDelayMs) ||
			!Read_RequiredHitOffsets(Value, Out.HitOffsetsMs) ||
			!Client::Parse_ValtanStageAttackContacts(Value.Find("attackContacts"), Out.AttackContacts) ||
			!Read_RequiredFiniteFloat(Value, "pushRangeM", Out.fPushRangeM) ||
			!Read_RequiredUInt32(Value, "pushMs", Out.iPushMs) ||
			!Read_RequiredUInt32(Value, "downMs", Out.iDownMs) ||
			!Read_StageAim(Value.Find("aim"), Out.iDurationMs, Out.strAimTargetPolicy,
				Out.bHasAimEnd, Out.iAimEndMs, Out.bHasAimResponseScale, Out.fAimResponseScale) ||
			!Read_StageMotion(
				Value.Find("motion"), Out.iDurationMs, Out.Motion) ||
			!Read_StageActions(
				Value.Find("actions"), Out.iDurationMs, Out.Actions) ||
			!Read_StageBranches(pBranches, Out.Branches) ||
			!Has_ValidNavigationBlockedCapture(Out.Branches, Out.strPlayerResponse) ||
			!Read_StageGameplayExtensions(Value, Out.strPartDamagePolicy,
				Out.CounterProxy, Out.BossResponse) ||
			!Read_StageHitAuthority(
				Value, Out.iDurationMs, Out.bHasHitAnchor,
				Out.strHitAnchorKind, Out.fHitAnchorForwardOffsetM,
				Out.fHitAnchorRightOffsetM, Out.fHitAnchorYawOffsetDegrees,
				Out.bHasHitActivation, Out.iHitActivationStartMs,
				Out.iHitActivationLifetimeMs) ||
			!Read_MasterCameraInvocations(
				Value.Find("cameraInvocations"), Out.iDurationMs,
				Out.CameraInvocations) ||
			!Client::Parse_BossStageEnvironment(Value, Out.SceneProfileOccurrences,
				Out.LightOccurrences, strOutError, Out.iDurationMs))
		{
			strOutError = "master stage gameplay values are invalid";
			return false;
		}

		// 누적 피해 반응은 ACTIVE stage의 threshold 분기 하나와 연결한다.
		// 높이 오프셋·이동·counter proxy의 종류도 stage 역할과 충돌하지 않아야 한다.
		const std::size_t iHealthThresholdCount =
			static_cast<std::size_t>(std::count_if(
				Out.Branches.begin(), Out.Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{
					return "HEALTH_DAMAGE_THRESHOLD_REACHED" ==
						Branch.strOutcome;
				}));
		if ((Out.BossResponse.has_value() &&
			 ("ACTIVE" != Out.strStageKind || 1u != iHealthThresholdCount)) ||
			(!Out.BossResponse.has_value() && 0u != iHealthThresholdCount) ||
			(0.f != Out.fVerticalOffsetM &&
			 ((!Out.BossResponse.has_value() && !Has_ClosedStaggerStageResponse(Value)) || Out.Motion.has_value())) ||
			(Out.CounterProxy.has_value() &&
			 (("BOSS_FORWARD_ARC" == Out.CounterProxy->strKind &&
			   "WINDUP" != Out.strStageKind &&
			   "ACTIVE" != Out.strStageKind) ||
			  ("BOSS_LOCAL_CIRCLE" == Out.CounterProxy->strKind &&
			   "WINDUP" != Out.strStageKind))))
		{
			strOutError = "master stage response/proxy contract is invalid";
			return false;
		}

		// 명시 attackContacts는 Shared의 시각/횟수 검증을 거친다.
		// 지속 활성 hit, 지원하지 않는 이동, 잡기 응답이나 NONE 도형과 함께 사용할 수 없다.
		if (!LostArk::Shared::Validate_StageAttackContacts(Out.AttackContacts,
			Out.iDurationMs, Out.HitOffsetsMs, Out.iHitCount, Out.iHitDelayMs, Out.iHitIntervalMs) ||
			(!Out.AttackContacts.empty() && (Out.bHasHitActivation ||
				(Out.Motion.has_value() && Out.Motion->strKind != "TO_ARENA_CENTER") ||
				Out.strPlayerResponse != "DAMAGE" || Out.strHitShape == "NONE")))
		{
			strOutError = "master stage attackContacts contract is invalid";
			return false;
		}

		// hit 시계는 명시 offset pulse, delay+interval pulse, 지속 활성 구간으로 나눈다.
		// pulse의 마지막 hit는 stage 종료 전이고, NONE 도형의 빈 hit 표현도 별도로 허용한다.
		const bool_t bHasExplicitOffsets = !Out.HitOffsetsMs.empty();
		const bool_t bValidPulse = !Out.bHasHitActivation &&
			((bHasExplicitOffsets && Out.HitOffsetsMs.size() == Out.iHitCount &&
			  0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs &&
			  Out.HitOffsetsMs.back() < Out.iDurationMs) ||
			 (!bHasExplicitOffsets && Out.iHitCount > 0u &&
			  (1u == Out.iHitCount ? 0u == Out.iHitIntervalMs :
				Out.iHitIntervalMs > 0u) &&
			  static_cast<uint64_t>(Out.iHitDelayMs) +
				static_cast<uint64_t>(Out.iHitCount - 1u) *
					Out.iHitIntervalMs < Out.iDurationMs));
		const bool_t bValidActive = Out.bHasHitActivation &&
			!bHasExplicitOffsets && 0u == Out.iHitCount &&
			0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs;
		const bool_t bEmptyHit = "NONE" == Out.strHitShape &&
			!Out.bHasHitAnchor && !Out.bHasHitActivation &&
			!bHasExplicitOffsets && 0u == Out.iHitCount &&
			0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs;
		if (!bEmptyHit && !bValidPulse && !bValidActive)
		{
			strOutError = "master stage hit authority/schedule is invalid";
			return false;
		}
		Out.bKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		// 포탈 돌진의 hit는 retarget 지연부터 이동 종료 전까지 50 ms 간격의 전체 offset과 정확히 대응해야 한다.
		if (Out.Motion.has_value() &&
			"PORTAL_TARGET_RUSH" == Out.Motion->strKind)
		{
			const VALTAN_STAGE_MOTION_VIEW& Motion = *Out.Motion;
			const double fTravelEndMs =
				static_cast<double>(Motion.iRetargetDelayMs) +
				static_cast<double>(Motion.fDistance) /
				static_cast<double>(Motion.fSpeedMps) * 1000.0;
			uint32_t iExpectedOffsetMs = Motion.iRetargetDelayMs;
			if (Out.HitOffsetsMs.empty())
			{
				strOutError =
					"portal target-rush requires explicit swept-hit offsets";
				return false;
			}
			for (const uint32_t iOffsetMs : Out.HitOffsetsMs)
			{
				if (iOffsetMs != iExpectedOffsetMs ||
					static_cast<double>(iOffsetMs) >= fTravelEndMs)
				{
					strOutError =
						"portal target-rush hit offsets must cover only its 50 ms travel samples";
					return false;
				}
				iExpectedOffsetMs += 50u;
			}
			if (static_cast<double>(iExpectedOffsetMs) + 0.000001 <
				fTravelEndMs)
			{
				strOutError =
					"portal target-rush hit offsets do not cover its full travel";
				return false;
			}
		}

		// clip occurrence는 입력 순서로 쌓되 stage 안의 stable ID 중복을 거부한다.
		// 아래 repeatCount 검사는 반복 목록의 크기와 clip 이름까지 대조한다.
		std::set<std::string, std::less<>> OccurrenceIds;
		if (nullptr != pOccurrences)
		{
			for (const DATA_JSON_VALUE& OccurrenceValue :
				pOccurrences->Get_Array())
			{
				Client::VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
				if (!Parse_MasterOccurrence(
						OccurrenceValue, Occurrence, strOutError) ||
					!OccurrenceIds.insert(Occurrence.strClipOccurrenceId).second)
				{
					if (strOutError.empty())
						strOutError = "master stage duplicates a clip occurrence";
					return false;
				}
				Out.Occurrences.push_back(std::move(Occurrence));
			}
		}
		// repeatCount는 한 clip을 반복한다는 저작 계약이다. 임의 occurrence 목록의 표시 개수가 아니다.
		// 공통 Master 검증에서 occurrence 개수와 clip 이름을 맞춰 도구마다 다른 타임라인을 만들지 않게 한다.
		if (1u < Out.iRepeatCount)
		{
			if (Out.Occurrences.size() != Out.iRepeatCount)
			{
				strOutError =
					"master repeatCount must own exactly its explicit occurrences: " +
					Out.strActionId;
				return false;
			}
			const std::string& strRepeatedClip =
				Out.Occurrences.front().strClipName;
			if (std::any_of(Out.Occurrences.begin(), Out.Occurrences.end(),
				[&strRepeatedClip](
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
				{
					return Occurrence.strClipName != strRepeatedClip;
				}))
			{
				strOutError =
					"master repeatCount occurrences must use one clip: " +
					Out.strActionId;
				return false;
			}
		}

		// Effect 참조는 종류와 ID의 쌍으로 중복을 확인한다.
		// INDEPENDENT_EFFECT는 identity만, CUE_BINDING은 별도의 cueProjection도 소유한다.
		std::set<std::string, std::less<>> EffectReferenceIds;
		for (const DATA_JSON_VALUE& EffectValue : pEffects->Get_Array())
		{
			const DATA_JSON_VALUE* pType = Required(
				EffectValue, "refType", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pId = Required(
				EffectValue, "refId", DATA_JSON_TYPE::STRING);
			if (nullptr == pType || nullptr == pId ||
				("CUE_BINDING" != pType->Get_String() &&
				 "INDEPENDENT_EFFECT" != pType->Get_String()) ||
				!Is_StableToken(pId->Get_String()) ||
				!EffectReferenceIds.insert(
					pType->Get_String() + ":" + pId->Get_String()).second)
			{
				strOutError = "master effect reference is invalid or duplicated";
				return false;
			}
			MASTER_EFFECT_REFERENCE Reference;
			Reference.strType = pType->Get_String();
			Reference.strId = pId->Get_String();
			if ("INDEPENDENT_EFFECT" == Reference.strType)
			{
				if (!Has_ExactProperties(EffectValue, { "refType", "refId" }))
				{
					strOutError =
						"master independent Effect reference has unexpected properties";
					return false;
				}
			}
			else
			{
				if (!Has_ExactProperties(
						EffectValue, { "refType", "refId", "cueProjection" }))
				{
					strOutError =
						"master cue reference has unexpected properties";
					return false;
				}
				const DATA_JSON_VALUE* pProjection = Required(
					EffectValue, "cueProjection", DATA_JSON_TYPE::OBJECT);
				if (nullptr == pProjection)
				{
					strOutError = "master cueProjection contract is invalid";
					return false;
				}

				// timingBasis가 있으면 STAGE_CLOCK 표현으로만 읽는다.
				// 시작은 stage 내부 ms이고 선택 끝 시각은 시작 초과·600000ms 이하만 여기서 검사한다.
				if (nullptr != pProjection->Find("timingBasis"))
				{
					const DATA_JSON_VALUE* pTimingBasis = Required(
						*pProjection, "timingBasis", DATA_JSON_TYPE::STRING);
					const DATA_JSON_VALUE* pStageOffset =
						pProjection->Find("stageOffsetMs");
					if (!Has_ExactPropertiesWithOptional(
							*pProjection, { "timingBasis", "stageOffsetMs" }, { "stageEndMs" }) ||
						nullptr == pTimingBasis ||
						"STAGE_CLOCK" != pTimingBasis->Get_String() ||
						!Is_NonNegativeInteger(pStageOffset) ||
						pStageOffset->Get_Number() >=
							static_cast<double>(Out.iDurationMs))
					{
						strOutError =
							"master stage-clock cueProjection values are invalid";
						return false;
					}
					const auto* end = pProjection->Find("stageEndMs");
					Reference.bHasSourceEnd = end && !end->Is_Null();
					if (Reference.bHasSourceEnd && (!Is_NonNegativeInteger(end) ||
						end->Get_Number() <= pStageOffset->Get_Number() || end->Get_Number() > 600000.0))
					{
						strOutError = "master stage-clock end leaves Stage";
						return false;
					}
					Reference.iSourceEndMs = Reference.bHasSourceEnd ? static_cast<uint32_t>(end->Get_Number()) : 0u;
					Reference.bUsesStageClock = true;
					Reference.iStageOffsetMs = static_cast<uint32_t>(
						pStageOffset->Get_Number());
					Out.EffectReferences.push_back(std::move(Reference));
					continue;
				}

				// clip 기준 cue는 clipOccurrenceId와 원본 내부 시작/선택 끝 시각, mappingBasis를 보관한다.
				// 여기서는 필드 형식을 읽으며 실제 occurrence 결합은 상위 문서 대조가 맡는다.
				if (!Has_ExactProperties(*pProjection,
						{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
						  "mappingBasis" }))
				{
					strOutError = "master cueProjection contract is invalid";
					return false;
				}
				const DATA_JSON_VALUE* pClipOccurrence = Required(
					*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pSourceStart =
					pProjection->Find("sourceStartMs");
				const DATA_JSON_VALUE* pSourceEnd =
					pProjection->Find("sourceEndMs");
				const DATA_JSON_VALUE* pMappingBasis = Required(
					*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
				if (nullptr == pClipOccurrence ||
					!Is_StableToken(pClipOccurrence->Get_String()) ||
					!Is_NonNegativeInteger(pSourceStart) ||
					nullptr == pSourceEnd ||
					(!pSourceEnd->Is_Null() &&
					 !Is_NonNegativeInteger(pSourceEnd)) ||
					nullptr == pMappingBasis ||
					pMappingBasis->Get_String().empty())
				{
					strOutError = "master cueProjection values are invalid";
					return false;
				}
				Reference.strClipOccurrenceId = pClipOccurrence->Get_String();
				Reference.iSourceStartMs = static_cast<uint32_t>(
					pSourceStart->Get_Number());
				Reference.bHasSourceEnd = !pSourceEnd->Is_Null();
				Reference.iSourceEndMs = Reference.bHasSourceEnd ?
					static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
				Reference.strMappingBasis = pMappingBasis->Get_String();
			}
			Out.EffectReferences.push_back(std::move(Reference));
		}
		return true;
	}

	// 패턴 도약이 참조하는 stage와 시각의 관계를 Master stage 목록에 대조한다.
	// 보충: 이 함수는 serverMotion 필드의 존재와 Read_PatternServerMotion 구조 검증만 담당한다.
	// stage 목록과 시간창의 실제 대조는 stage 수집 뒤 Validate_PatternServerMotionStageWindows가 수행한다.
	bool_t Validate_MasterServerMotion(
		const DATA_JSON_VALUE& Pattern,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pMotion = Pattern.Find("serverMotion");
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW> Motion;
		if (nullptr == pMotion || !Read_PatternServerMotion(pMotion, Motion))
		{
			strOutError = "master serverMotion is invalid";
			return false;
		}
		return true;
	}

	// 패턴 메타데이터와 stage, 출처 및 외부 참조를 임시 정의에 읽는다.
	// stage ID 중복과 응답 개수, 높이/이동 조합을 검사하며 Out의 부분 변경은 상위 임시 문서 안에 남는다.
	bool_t Parse_MasterPattern(
		const DATA_JSON_VALUE& Value,
		MASTER_PATTERN& Out,
		std::string& strOutError)
	{
		// 호환 Master의 필수/선택 필드를 구분하고 패턴 identity·표시 이름·stage 배열을 확인한다.
		// 실제 선택과 실행은 이 reader가 수행하지 않으며 아래에서 임시 정의를 채운다.
		if (!Has_ExactPropertiesWithOptional(Value,
				{ "patternId", "category", "minimumPhase", "maximumPhase",
				  "targetPolicy", "aimPolicy", "displayName", "actionId",
				  "sourceActionIds", "sourceSequenceIndex", "presentationSources",
				  "selectionMode",
				  "minimumHealthBar", "maximumHealthBar", "triggerHealthBar",
				  "triggerOrder", "armorRequirement", "phaseRequirement",
				  "invulnerableWhileRunning", "selectionWeight",
				  "maximumConsecutiveUses", "minimumRange", "maximumRange",
				  "serverMotion", "reactions", "cameraCueIds",
				  "worldEventTriggerRefs", "stages" },
				{ "finale", "verticalOffsetM" }))
		{
			strOutError = "master pattern has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pPatternId = Required(
			Value, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceSequence = Value.Find("sourceSequenceIndex");
		const DATA_JSON_VALUE* pStages = Required(
			Value, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pPatternId || nullptr == pDisplay || nullptr == pAction ||
			!Is_StableToken(pPatternId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAction->Get_String()) ||
			!Is_NonNegativeInteger(pSourceSequence) ||
			4096.0 < pSourceSequence->Get_Number() || nullptr == pStages ||
			pStages->Get_Array().empty())
		{
			strOutError = "master pattern identity is invalid";
			return false;
		}

		// 선택 정책 문자열의 비어 있음과 정수 필드의 기본 형식부터 검사한다.
		// 범위 실수·선택 높이·무적 여부·serverMotion의 구조 검사가 이어진다.
		for (const std::string_view Field :
			{ "category", "targetPolicy", "aimPolicy", "selectionMode",
			  "armorRequirement", "phaseRequirement" })
		{
			const DATA_JSON_VALUE* pText = Required(
				Value, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pText || pText->Get_String().empty())
			{
				strOutError = "master pattern string field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		for (const std::string_view Field :
			{ "minimumPhase", "maximumPhase", "minimumHealthBar",
			  "maximumHealthBar", "triggerHealthBar", "triggerOrder",
			  "selectionWeight", "maximumConsecutiveUses" })
		{
			if (!Is_NonNegativeInteger(Value.Find(Field)))
			{
				strOutError = "master pattern integer field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		const DATA_JSON_VALUE* pVerticalOffset = Value.Find("verticalOffsetM");
		if (!Is_FiniteNumber(Value.Find("minimumRange")) ||
			!Is_FiniteNumber(Value.Find("maximumRange")) ||
			(nullptr != pVerticalOffset &&
			 (!Is_FiniteNumber(pVerticalOffset) ||
			  0.0 == pVerticalOffset->Get_Number() ||
			  std::fabs(pVerticalOffset->Get_Number()) > 100.0)) ||
			nullptr == Required(
				Value, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN) ||
			!Validate_MasterServerMotion(Value, strOutError))
		{
			if (strOutError.empty())
				strOutError = "master pattern selection/motion fields are invalid";
			return false;
		}

		const DATA_JSON_VALUE* pSourceActions = Required(
			Value, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPresentationSources = Required(
			Value, "presentationSources", DATA_JSON_TYPE::ARRAY);
		// 이름만 만든 수동 패턴은 아직 원본 애니메이션을 소유하지 않는다.
		// PRIMARY 출처나 idle clip을 임의로 채우지 않고 빈 저작 상태를 허용한다.
		const bool_t bUnboundManualPattern =
			"AUDITION_ONLY" == Read_String(Value, "selectionMode") &&
			nullptr != pSourceActions && pSourceActions->Get_Array().empty() &&
			nullptr != pPresentationSources &&
			pPresentationSources->Get_Array().empty() &&
			0.0 == pSourceSequence->Get_Number();
		if (nullptr == pSourceActions ||
			(!bUnboundManualPattern && pSourceActions->Get_Array().empty()) ||
			std::any_of(pSourceActions->Get_Array().begin(),
				pSourceActions->Get_Array().end(),
				[](const DATA_JSON_VALUE& SourceAction)
				{
					return !Is_NonNegativeInteger(&SourceAction) ||
						0.0 == SourceAction.Get_Number();
				}))
		{
			strOutError = "master sourceActionIds is invalid";
			return false;
		}
		if (nullptr == pPresentationSources ||
			(!bUnboundManualPattern && pPresentationSources->Get_Array().empty()))
		{
			strOutError = "master presentationSources is invalid";
			return false;
		}

		// 연출 출처는 sourceActionId·sequenceIndex·role의 조합으로 중복을 거부한다.
		// 동일 sourceActionId의 다른 순번/역할은 별도 identity다.
		std::set<std::string, std::less<>> PresentationSourceIdentities;
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			if (!Has_ExactProperties(
					Source, { "sourceActionId", "sequenceIndex", "role" }) ||
				!Is_NonNegativeInteger(Source.Find("sourceActionId")) ||
				0.0 == Source.Find("sourceActionId")->Get_Number() ||
				!Is_NonNegativeInteger(Source.Find("sequenceIndex")) ||
				4096.0 < Source.Find("sequenceIndex")->Get_Number() ||
				nullptr == Required(Source, "role", DATA_JSON_TYPE::STRING))
			{
				strOutError = "master presentation source row is invalid";
				return false;
			}
			const std::string Identity = std::to_string(static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number())) + ":" +
				std::to_string(static_cast<uint32_t>(
					Source.Find("sequenceIndex")->Get_Number())) + ":" +
				Required(Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			if (!PresentationSourceIdentities.insert(Identity).second)
			{
				strOutError = "master presentation source row is duplicated";
				return false;
			}
		}

		// 반응·카메라·월드 이벤트는 서로 다른 참조 목록이다.
		// 지금은 stable ID와 중복을 확인하고 stage 존재 여부는 stage 수집 이후 대조한다.
		const DATA_JSON_VALUE* pReactions = Required(
			Value, "reactions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pCameraCues = Required(
			Value, "cameraCueIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pWorldRefs = Required(
			Value, "worldEventTriggerRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pReactions || nullptr == pCameraCues ||
			nullptr == pWorldRefs)
		{
			strOutError = "master reactions/cue references are invalid";
			return false;
		}
		std::set<std::string, std::less<>> ReactionIdentities;
		for (const DATA_JSON_VALUE& Reaction : pReactions->Get_Array())
		{
			const DATA_JSON_VALUE* pTrigger = Required(
				Reaction, "triggerKind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStage = Required(
				Reaction, "stageId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(Reaction, { "triggerKind", "stageId" }) ||
				nullptr == pTrigger || nullptr == pStage ||
				!Is_StableToken(pTrigger->Get_String()) ||
				!Is_StableToken(pStage->Get_String()) ||
				!ReactionIdentities.insert(
					pTrigger->Get_String() + ":" + pStage->Get_String()).second)
			{
				strOutError = "master reaction is invalid or duplicated";
				return false;
			}
			Client::VALTAN_PATTERN_REACTION_VIEW ReactionView;
			ReactionView.strTriggerKind = pTrigger->Get_String();
			ReactionView.strStageId = pStage->Get_String();
			Out.Reactions.push_back(std::move(ReactionView));
		}

		// cameraCueIds는 cue ID 자체로 중복을 판정한다. 여기서 카메라 리소스를 열지는 않는다.
		std::set<std::string, std::less<>> CameraCueIdentities;
		for (const DATA_JSON_VALUE& CameraCue : pCameraCues->Get_Array())
		{
			if (!CameraCue.Is_String() ||
				!Is_StableToken(CameraCue.Get_String()) ||
				!CameraCueIdentities.insert(CameraCue.Get_String()).second)
			{
				strOutError = "master cameraCueIds is invalid or duplicated";
				return false;
			}
			Out.CameraCueIds.push_back(CameraCue.Get_String());
		}

		// 월드 이벤트 참조는 patternId·stageId·triggerKind를 묶어 보관한다.
		// 아래 최종 검사에서 현재 패턴과 그 stage를 가리키는지 확인한다.
		std::set<std::string, std::less<>> WorldReferenceIdentities;
		for (const DATA_JSON_VALUE& WorldRef : pWorldRefs->Get_Array())
		{
			const DATA_JSON_VALUE* pWorldPattern = Required(
				WorldRef, "patternId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldStage = Required(
				WorldRef, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldTrigger = Required(
				WorldRef, "triggerKind", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(
					WorldRef, { "patternId", "stageId", "triggerKind" }) ||
				nullptr == pWorldPattern || nullptr == pWorldStage ||
				nullptr == pWorldTrigger ||
				!Is_StableToken(pWorldPattern->Get_String()) ||
				!Is_StableToken(pWorldStage->Get_String()) ||
				!Is_StableToken(pWorldTrigger->Get_String()) ||
				!WorldReferenceIdentities.insert(
					pWorldPattern->Get_String() + ":" +
					pWorldStage->Get_String() + ":" +
					pWorldTrigger->Get_String()).second)
			{
				strOutError =
					"master worldEventTriggerRefs is invalid or duplicated";
				return false;
			}
			Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW Reference;
			Reference.strPatternId = pWorldPattern->Get_String();
			Reference.strStageId = pWorldStage->Get_String();
			Reference.strTriggerKind = pWorldTrigger->Get_String();
			Out.WorldEventTriggerRefs.push_back(std::move(Reference));
		}

		// 검사한 패턴 속성과 출처를 Out의 소유 값으로 복사한다.
		// serverMotion/finale도 여기서 보관하며 finale와 실행 중 무적의 동시 사용은 거부한다.
		Out.strPatternId = pPatternId->Get_String();
		Out.strCategory = Required(
			Value, "category", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumPhase = static_cast<uint32_t>(
			Value.Find("minimumPhase")->Get_Number());
		Out.iMaximumPhase = static_cast<uint32_t>(
			Value.Find("maximumPhase")->Get_Number());
		Out.strTargetPolicy = Required(
			Value, "targetPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strAimPolicy = Required(
			Value, "aimPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strActionId = pAction->Get_String();
		for (const DATA_JSON_VALUE& SourceAction : pSourceActions->Get_Array())
		{
			Out.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}
		Out.strSelectionMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumHealthBar = static_cast<int32_t>(
			Value.Find("minimumHealthBar")->Get_Number());
		Out.iMaximumHealthBar = static_cast<int32_t>(
			Value.Find("maximumHealthBar")->Get_Number());
		Out.iTriggerHealthBar = static_cast<int32_t>(
			Value.Find("triggerHealthBar")->Get_Number());
		Out.iTriggerOrder = static_cast<uint32_t>(
			Value.Find("triggerOrder")->Get_Number());
		Out.strArmorRequirement = Required(
			Value, "armorRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strPhaseRequirement = Required(
			Value, "phaseRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.bInvulnerableWhileRunning = Required(
			Value, "invulnerableWhileRunning",
			DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		Out.iSelectionWeight = static_cast<uint32_t>(
			Value.Find("selectionWeight")->Get_Number());
		Out.iMaximumConsecutiveUses = static_cast<uint32_t>(
			Value.Find("maximumConsecutiveUses")->Get_Number());
		Out.fMinimumRange = static_cast<f32_t>(
			Value.Find("minimumRange")->Get_Number());
		Out.fMaximumRange = static_cast<f32_t>(
			Value.Find("maximumRange")->Get_Number());
		Out.fVerticalOffsetM = nullptr == pVerticalOffset ? 0.f :
			static_cast<f32_t>(pVerticalOffset->Get_Number());
		if (!Read_PatternServerMotion(Value.Find("serverMotion"), Out.ServerMotion) ||
			!Read_PatternFinale(Value.Find("finale"), Out.Finale) ||
			(Out.Finale.has_value() && Out.bInvulnerableWhileRunning))
		{
			strOutError = "master serverMotion/finale values are invalid";
			return false;
		}
		Out.iSourceSequenceIndex = static_cast<uint32_t>(
			pSourceSequence->Get_Number());
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			Client::VALTAN_PRESENTATION_SOURCE_VIEW Presentation;
			Presentation.iSourceActionId = static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number());
			Presentation.iSequenceIndex = static_cast<uint32_t>(
				Source.Find("sequenceIndex")->Get_Number());
			Presentation.strRole = Required(
				Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			Out.PresentationSources.push_back(std::move(Presentation));
		}

		// stage를 원본 순서로 파싱하고 stageId 중복을 거부한다.
		// 모두 수집된 뒤에만 패턴 전체 반응 개수와 stage 간 참조를 검사할 수 있다.
		std::set<std::string, std::less<>> StageIds;
		for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
		{
			MASTER_STAGE Stage;
			if (!Parse_MasterStage(StageValue, Stage, strOutError) ||
				!StageIds.insert(Stage.strStageId).second)
			{
				if (strOutError.empty())
					strOutError = "master pattern duplicates a stageId";
				return false;
			}
			Out.Stages.push_back(std::move(Stage));
		}

		// boss response는 패턴에 최대 하나다. 패턴 높이는 반응 존재를 요구하고
		// stage 높이를 쓴 패턴에는 별도 ServerMotion을 함께 허용하지 않는다.
		const std::size_t iBossResponseCount =
			static_cast<std::size_t>(std::count_if(
				Out.Stages.begin(), Out.Stages.end(),
				[](const MASTER_STAGE& Stage)
				{ return Stage.BossResponse.has_value(); }));
		const bool_t bHasStageVerticalOffset = std::any_of(
			Out.Stages.begin(), Out.Stages.end(),
			[](const MASTER_STAGE& Stage)
			{ return 0.f != Stage.fVerticalOffsetM; });
		if (1u < iBossResponseCount ||
			(0.f != Out.fVerticalOffsetM && 0u == iBossResponseCount) ||
			(bHasStageVerticalOffset && Out.ServerMotion.has_value()))
		{
			strOutError =
				"master verticalOffsetM requires an active boss-response pattern: " +
				Out.strPatternId;
			return false;
		}

		// 반응의 stage ID와 월드 이벤트의 패턴/stage 소유 관계를 완성된 목록에 대조한다.
		for (const Client::VALTAN_PATTERN_REACTION_VIEW& Reaction :
			Out.Reactions)
		{
			if (StageIds.end() == StageIds.find(Reaction.strStageId))
			{
				strOutError = "master reaction targets an unknown stage: " +
					Reaction.strStageId;
				return false;
			}
		}
		for (const Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Reference :
			Out.WorldEventTriggerRefs)
		{
			if (Reference.strPatternId != Out.strPatternId ||
				StageIds.end() == StageIds.find(Reference.strStageId))
			{
				strOutError =
					"master world Event trigger leaves its managed pattern/stage: " +
					Reference.strPatternId + "/" + Reference.strStageId;
				return false;
			}
		}

		// 도약 구조 파싱과 별도로 이륙이 첫 stage 안에, travel이 뒤쪽 지정 stage 안에 드는지 확인한다.
		if (!Validate_PatternServerMotionStageWindows(
				Out.ServerMotion, Out.Stages))
		{
			strOutError =
				"master serverMotion window is outside its authored stage: " +
				Out.strPatternId;
			return false;
		}
		return true;
	}

	// 독립 Effect의 소유 종류에 따라 stage cue 또는 전투 오브젝트 참조를 읽는다.
	// 서로 다른 소유 방식의 필드를 함께 허용하지 않고 실제 owner 결합은 뒤 단계에서 수행한다.
	bool_t Parse_MasterIndependentEffect(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "independentEffectId", "displayName", "effectAssetId",
				  "ownership", "ownerPatternId", "ownerStageId",
				  "triggerPolicy", "combatObjectArchetypeId", "clientVisualId",
				  "effectCueBindingId", "cueProjection" }))
		{
			strOutError = "master independent Effect has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pId = Required(
			Value, "independentEffectId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAsset = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOwnership = Required(
			Value, "ownership", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = Required(
			Value, "ownerPatternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pStage = Required(
			Value, "ownerStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTrigger = Required(
			Value, "triggerPolicy", DATA_JSON_TYPE::STRING);
		if (nullptr == pId || nullptr == pDisplay || nullptr == pAsset ||
			nullptr == pOwnership || nullptr == pPattern || nullptr == pStage ||
			nullptr == pTrigger || !Is_StableToken(pId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAsset->Get_String()) ||
			!Is_StableToken(pPattern->Get_String()) ||
			!Is_StableToken(pStage->Get_String()))
		{
			strOutError = "master independent Effect identity is invalid";
			return false;
		}
		Out.strIndependentEffectId = pId->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strEffectAssetId = pAsset->Get_String();
		Out.strOwnership = pOwnership->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.strTriggerPolicy = pTrigger->Get_String();
		if (!Read_NullableStableToken(Value, "combatObjectArchetypeId",
				Out.strCombatObjectArchetypeId) ||
			!Read_NullableStableToken(Value, "clientVisualId",
				Out.strClientVisualId) ||
			!Read_NullableStableToken(Value, "effectCueBindingId",
				Out.strEffectCueBindingId))
		{
			strOutError = "master independent Effect owner identity is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pProjection = Value.Find("cueProjection");
		if (nullptr == pProjection)
		{
			strOutError = "master independent Effect cueProjection is missing";
			return false;
		}
		const bool_t bCombatObject = "SERVER_COMBAT_OBJECT" == Out.strOwnership;
		const bool_t bPatternStage = "SERVER_PATTERN_STAGE" == Out.strOwnership;
		if ((!bCombatObject && !bPatternStage) ||
			(bCombatObject && (Out.strCombatObjectArchetypeId.empty() ||
				Out.strClientVisualId.empty() ||
				!Out.strEffectCueBindingId.empty())) ||
			(bPatternStage && (!Out.strCombatObjectArchetypeId.empty() ||
				!Out.strClientVisualId.empty() ||
				Out.strEffectCueBindingId.empty())))
		{
			strOutError = "master independent Effect ownership tuple is invalid";
			return false;
		}
		if (bCombatObject)
		{
			if (!pProjection->Is_Null())
			{
				strOutError =
					"master combat-object Effect cannot project a boss-root cue";
				return false;
			}
			return true;
		}
		if (!pProjection->Is_Object())
		{
			strOutError =
				"master pattern-stage Effect cueProjection contract is invalid";
			return false;
		}
		if (Has_ExactPropertiesWithOptional(
				*pProjection, { "timingBasis", "stageOffsetMs" }, { "stageEndMs" }))
		{
			const DATA_JSON_VALUE* pTimingBasis = Required(
				*pProjection, "timingBasis", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStageOffset =
				pProjection->Find("stageOffsetMs");
			if (nullptr == pTimingBasis ||
				"STAGE_CLOCK" != pTimingBasis->Get_String() ||
				!Is_NonNegativeInteger(pStageOffset))
			{
				strOutError =
					"master pattern-stage Effect stage-clock projection is invalid";
				return false;
			}
			Out.bHasCueProjection = true;
			const auto* end = pProjection->Find("stageEndMs");
			Out.bHasCueSourceEnd = end && !end->Is_Null();
			if (Out.bHasCueSourceEnd && (!Is_NonNegativeInteger(end) ||
				end->Get_Number() <= pStageOffset->Get_Number() || end->Get_Number() > 600000.0))
			{
				strOutError = "independent stage-clock end is invalid";
				return false;
			}
			Out.iCueSourceEndMs = Out.bHasCueSourceEnd ? static_cast<uint32_t>(end->Get_Number()) : 0u;
			Out.bUsesCueStageClock = true;
			Out.iCueStageOffsetMs = static_cast<uint32_t>(
				pStageOffset->Get_Number());
			return true;
		}
		if (!Has_ExactProperties(*pProjection,
				{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
				  "mappingBasis" }))
		{
			strOutError =
				"master pattern-stage Effect cueProjection contract is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pClipOccurrence = Required(
			*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = pProjection->Find("sourceStartMs");
		const DATA_JSON_VALUE* pSourceEnd = pProjection->Find("sourceEndMs");
		const DATA_JSON_VALUE* pMappingBasis = Required(
			*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
		if (nullptr == pClipOccurrence ||
			!Is_StableToken(pClipOccurrence->Get_String()) ||
			!Is_NonNegativeInteger(pSourceStart) || nullptr == pSourceEnd ||
			(!pSourceEnd->Is_Null() && !Is_NonNegativeInteger(pSourceEnd)) ||
			nullptr == pMappingBasis || pMappingBasis->Get_String().empty())
		{
			strOutError =
				"master pattern-stage Effect cueProjection values are invalid";
			return false;
		}
		Out.bHasCueProjection = true;
		Out.strCueClipOccurrenceId = pClipOccurrence->Get_String();
		Out.iCueSourceStartMs = static_cast<uint32_t>(
			pSourceStart->Get_Number());
		Out.bHasCueSourceEnd = !pSourceEnd->Is_Null();
		Out.iCueSourceEndMs = Out.bHasCueSourceEnd ?
			static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
		Out.strCueMappingBasis = pMappingBasis->Get_String();
		return true;
	}

	// 호환 일반 선택 목록과 체력 구간을 읽는다. 여기의 PatternIds는 고유 coverage 목록이며 반복 순서 목록과 구분한다.
	bool_t Parse_MasterNormalSelection(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_NORMAL_SELECTION_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(
				Value, { "selectionMode", "ranges", "patternIds" }))
		{
			strOutError = "master normalSelection has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRanges = Required(
			Value, "ranges", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatternIds = Required(
			Value, "patternIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pMode || ("WEIGHTED_POOL" != pMode->Get_String() && "ORDERED_LOOP" != pMode->Get_String()) ||
			nullptr == pRanges || pRanges->Get_Array().empty() ||
			nullptr == pPatternIds || pPatternIds->Get_Array().empty())
		{
			strOutError = "master normalSelection is invalid";
			return false;
		}
		Out.strSelectionMode = pMode->Get_String();
		std::set<std::string, std::less<>> RangeIds;
		uint32_t iPreviousFromHealthBar =
			(std::numeric_limits<uint32_t>::max)();
		for (const DATA_JSON_VALUE& Range : pRanges->Get_Array())
		{
			const DATA_JSON_VALUE* pRangeId = Required(
				Range, "rotationId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pFrom = Range.Find("fromHealthBar");
			const DATA_JSON_VALUE* pTo = Range.Find("toHealthBar");
			if (!Has_ExactProperties(
					Range, { "rotationId", "fromHealthBar", "toHealthBar" }) ||
				nullptr == pRangeId || !Is_StableToken(pRangeId->Get_String()) ||
				!Is_NonNegativeInteger(pFrom) || !Is_NonNegativeInteger(pTo) ||
				0.0 == pFrom->Get_Number() || ("ORDERED_LOOP" != Out.strSelectionMode && 0.0 == pTo->Get_Number()))
			{
				strOutError = "master normalSelection range is invalid";
				return false;
			}
			Client::VALTAN_NORMAL_SELECTION_RANGE_VIEW RangeView;
			RangeView.strRotationId = pRangeId->Get_String();
			RangeView.iFromHealthBar = static_cast<uint32_t>(pFrom->Get_Number());
			RangeView.iToHealthBar = static_cast<uint32_t>(pTo->Get_Number());
			if (!RangeIds.insert(RangeView.strRotationId).second ||
				RangeView.iFromHealthBar <= RangeView.iToHealthBar ||
				("ORDERED_LOOP" != Out.strSelectionMode && RangeView.iFromHealthBar > iPreviousFromHealthBar))
			{
				strOutError = "master normalSelection range is duplicated or inverted";
				return false;
			}
			iPreviousFromHealthBar = RangeView.iFromHealthBar;
			Out.Ranges.push_back(std::move(RangeView));
		}
		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternId : pPatternIds->Get_Array())
		{
			if (!PatternId.Is_String() ||
				!Is_StableToken(PatternId.Get_String()) ||
				!PatternIds.insert(PatternId.Get_String()).second)
			{
				strOutError =
					"master normalSelection patternId is invalid or duplicated";
				return false;
			}
			Out.PatternIds.push_back(PatternId.Get_String());
		}
		return true;
	}

	// 기존 counter의 창/성공/실패 참조를 읽는다. REFERENCE_ONLY_LEGACY 외의 소유 상태는 받지 않는다.
	bool_t Parse_MasterCounterReactionLayer(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "reactionLayerId", "admissionScope", "ownerPatternId",
				  "ownerStageId", "windowActionId", "successActionId",
				  "failureActionId" }))
		{
			strOutError =
				"master counter reaction layer has unexpected properties";
			return false;
		}
		const auto ReadStable = [&Value](const std::string_view Name)
			-> const DATA_JSON_VALUE*
		{
			const DATA_JSON_VALUE* pValue = Required(
				Value, Name, DATA_JSON_TYPE::STRING);
			return nullptr != pValue && Is_StableToken(pValue->Get_String()) ?
				pValue : nullptr;
		};
		const DATA_JSON_VALUE* pLayerId = ReadStable("reactionLayerId");
		const DATA_JSON_VALUE* pScope = Required(
			Value, "admissionScope", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = ReadStable("ownerPatternId");
		const DATA_JSON_VALUE* pStage = ReadStable("ownerStageId");
		const DATA_JSON_VALUE* pWindow = ReadStable("windowActionId");
		const DATA_JSON_VALUE* pSuccess = ReadStable("successActionId");
		const DATA_JSON_VALUE* pFailure = ReadStable("failureActionId");
		if (nullptr == pLayerId || nullptr == pScope ||
			"REFERENCE_ONLY_LEGACY" != pScope->Get_String() ||
			nullptr == pPattern || nullptr == pStage || nullptr == pWindow ||
			nullptr == pSuccess || nullptr == pFailure)
		{
			strOutError = "master counter reaction layer is invalid";
			return false;
		}
		Out.strReactionLayerId = pLayerId->Get_String();
		Out.strAdmissionScope = pScope->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.Window.strActionId = pWindow->Get_String();
		Out.Success.strActionId = pSuccess->Get_String();
		Out.Failure.strActionId = pFailure->Get_String();
		return true;
	}

	// 분리 JSON에서 만든 메모리상의 호환 Master를 검증한다. 과거 단일 Master 파일을 새 정본으로 읽는 경로가 아니다.
	// 전체 패턴을 모은 뒤 참조 대상을 연결하고, 후속 패턴의 순환과 제한 깊이 및 선택 목록의 coverage를 검사한다.
	bool_t Parse_MasterDocument(
		const DATA_JSON_VALUE& Root,
		const std::set<std::string, std::less<>>& ScriptedEntryOnlyPatternIds,
		MASTER_DOCUMENT& Out,
		std::string& strOutError)
	{
		// 분리 원본을 합쳐 만든 호환 객체의 schema·버전·보스/encounter/scope를 확인한다.
		// 이 함수는 해당 객체를 검증하며 별도의 Master 파일을 디스크에서 열지 않는다.
		if (!Has_ExactProperties(Root,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "previewPaths", "retiredPatternIds",
				  "normalSelection", "counterReactionLayers",
				  "independentEffects", "patterns" }) ||
			nullptr == Required(Root, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-master" !=
				Required(Root, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(Root.Find("formatVersion")) ||
			1.0 != Root.Find("formatVersion")->Get_Number() ||
			nullptr == Required(Root, "bossArchetypeId", DATA_JSON_TYPE::STRING) ||
			"BOSS_VALTAN" != Required(
				Root, "bossArchetypeId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "encounterId", DATA_JSON_TYPE::STRING) ||
			"ENCOUNTER_VALTAN" != Required(
				Root, "encounterId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "scope", DATA_JSON_TYPE::STRING) ||
			"PHASE_ONE" != Required(
				Root, "scope", DATA_JSON_TYPE::STRING)->Get_String())
		{
			strOutError = "joined authoring compatibility root is invalid";
			return false;
		}

		// previewPaths는 필드 목록과 Data/ 접두사를 검사한다.
		// 실제 파일 존재나 경로를 여는 작업은 뒤의 공개 Load/결합 경계가 맡는다.
		const DATA_JSON_VALUE* pPreviewPaths = Required(
			Root, "previewPaths", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pPreviewPaths || !Has_ExactProperties(*pPreviewPaths,
				{ "encounter", "animationBindings", "effectCues",
				  "combatObjects", "bossCatalog", "effectCatalog",
				  "damageProfiles", "cinematicCamera", "worldEvents",
				  "patternRotations", "sourceClipSequences" }))
		{
			strOutError = "joined authoring compatibility previewPaths contract is invalid";
			return false;
		}
		for (const auto& [Name, Value] : pPreviewPaths->Get_Object())
		{
			(void)Name;
			if (!Value.Is_String() || !Value.Get_String().starts_with("Data/"))
			{
				strOutError = "joined authoring compatibility preview path is invalid";
				return false;
			}
		}

		const DATA_JSON_VALUE* pRetired = Required(
			Root, "retiredPatternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pIndependent = Required(
			Root, "independentEffects", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pNormalSelection = Required(
			Root, "normalSelection", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pCounterReactions = Required(
			Root, "counterReactionLayers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatterns = Required(
			Root, "patterns", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pRetired || nullptr == pIndependent ||
			nullptr == pNormalSelection || nullptr == pCounterReactions ||
			pCounterReactions->Get_Array().empty() ||
			nullptr == pPatterns || pIndependent->Get_Array().empty() ||
			pPatterns->Get_Array().empty())
		{
			strOutError = "joined authoring compatibility inventories are invalid";
			return false;
		}

		// 먼저 폐기된 패턴 ID를 모아 뒤의 활성 패턴과 중복 소유되지 않게 한다.
		// Out의 목록은 진행 중 채워지므로 실패 시 상위 임시 문서와 함께 폐기해야 한다.
		std::set<std::string, std::less<>> RetiredIds;
		for (const DATA_JSON_VALUE& Retired : pRetired->Get_Array())
		{
			if (!Retired.Is_String() || !Is_StableToken(Retired.Get_String()) ||
				!RetiredIds.insert(Retired.Get_String()).second)
			{
				strOutError = "master retiredPatternIds is invalid or duplicated";
				return false;
			}
			Out.RetiredPatternIds.push_back(Retired.Get_String());
		}
		if (!Parse_MasterNormalSelection(
				*pNormalSelection, Out.NormalSelection, strOutError))
		{
			return false;
		}

		// counter 반응의 자체 ID와 owner pattern/stage 쌍은 각각 중복될 수 없다.
		// 참조용 반응을 읽는 단계이며 실제 owner가 관리 패턴에 들어왔는지는 마지막에 확인한다.
		std::set<std::string, std::less<>> CounterLayerIds;
		std::set<std::string, std::less<>> CounterOwnerStages;
		for (const DATA_JSON_VALUE& LayerValue :
			pCounterReactions->Get_Array())
		{
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer;
			if (!Parse_MasterCounterReactionLayer(
					LayerValue, Layer, strOutError) ||
				!CounterLayerIds.insert(Layer.strReactionLayerId).second ||
				!CounterOwnerStages.insert(
					Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId).second)
			{
				if (strOutError.empty())
					strOutError =
						"master counter reaction identity is duplicated";
				return false;
			}
			Out.CounterReactionLayers.push_back(std::move(Layer));
		}

		// 독립 Effect는 independent ID뿐 아니라 Effect asset ID도 유일해야 한다.
		// 두 색인을 따로 두어 다른 이름으로 같은 리소스를 중복 소유하는 행을 거부한다.
		std::set<std::string, std::less<>> IndependentIds;
		std::set<std::string, std::less<>> IndependentAssets;
		for (const DATA_JSON_VALUE& IndependentValue : pIndependent->Get_Array())
		{
			Client::VALTAN_INDEPENDENT_EFFECT_VIEW Independent;
			if (!Parse_MasterIndependentEffect(
					IndependentValue, Independent, strOutError) ||
				!IndependentIds.insert(
					Independent.strIndependentEffectId).second ||
				!IndependentAssets.insert(Independent.strEffectAssetId).second)
			{
				if (strOutError.empty())
					strOutError = "master independent Effect is duplicated";
				return false;
			}
			Out.IndependentEffects.push_back(std::move(Independent));
		}

		// 각 패턴을 임시 정의로 읽고 중복 또는 이미 폐기된 ID를 거부한다.
		// 참조 대상을 찾는 검사는 모든 패턴이 모인 다음에 수행한다.
		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
		{
			MASTER_PATTERN Pattern;
			if (!Parse_MasterPattern(PatternValue, Pattern, strOutError) ||
				!PatternIds.insert(Pattern.strPatternId).second ||
				RetiredIds.contains(Pattern.strPatternId))
			{
				if (strOutError.empty())
					strOutError = "master pattern identity is duplicated or retired";
				return false;
			}
			Out.Patterns.push_back(std::move(Pattern));
		}

		// 완성된 Out.Patterns의 주소를 차용하는 ID 색인과 후속 관계 표를 만든다.
		// 이 뒤 검증 중에는 패턴 vector에 새 항목을 추가하지 않아 차용 주소를 유지한다.
		std::map<std::string, const MASTER_PATTERN*, std::less<>> PatternById;
		std::map<std::string, std::set<std::string, std::less<>>, std::less<>>
			FollowupSuccessors;
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			PatternById.emplace(Pattern.strPatternId, &Pattern);
			FollowupSuccessors.emplace(
				Pattern.strPatternId, std::set<std::string, std::less<>>{});
		}

		// 호환 action의 flag를 미지정 0·ENTER 1/EXIT 0의 정상 쌍 1·그 외 -1로 분류한다.
		// 분리 원본의 boolean 이벤트가 아니라 변환된 action의 fValue를 검사하는 경계다.
		const auto MasterFlagState = [](const MASTER_STAGE& Stage,
			const std::string_view strFlagId)
		{
			uint32_t iTotal = 0u;
			uint32_t iEnter = 0u;
			uint32_t iExit = 0u;
			for (const Client::VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
			{
				if ("SET_BOSS_FLAG" != Action.strKind ||
					strFlagId != Action.strTargetId)
				{
					continue;
				}
				++iTotal;
				iEnter += "ENTER" == Action.strTrigger && 1.f == Action.fValue ?
					1u : 0u;
				iExit += "EXIT" == Action.strTrigger && 0.f == Action.fValue ?
					1u : 0u;
			}
			if (0u == iTotal)
				return 0;
			return 2u == iTotal && 1u == iEnter && 1u == iExit ? 1 : -1;
		};
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			// 현재 패턴의 action ID를 stage index에 연결한다.
			// 로컬 성공/TIMEOUT이 뒤쪽 stage인지 판정할 때 이 순서 색인을 사용한다.
			std::map<std::string, size_t, std::less<>> StageByAction;
			for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
				StageByAction.emplace(
					Pattern.Stages[iStage].strActionId, iStage);
			for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
			{
				const MASTER_STAGE& Stage = Pattern.Stages[iStage];
				const auto Counter = std::find_if(
					Stage.Branches.begin(), Stage.Branches.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{ return "COUNTER_HIT" == Branch.strOutcome; });
				if (Counter != Stage.Branches.end())
				{
					// COUNTER_HIT가 있는 stage는 성공과 TIMEOUT을 각 하나만 가져야 한다.
					// 성공 대상은 로컬 action 또는 다른 패턴의 첫 stage로 찾고 두 방식을 동시에 허용하지 않는다.
					const std::size_t iCounterCount =
						static_cast<std::size_t>(std::count_if(
							Stage.Branches.begin(), Stage.Branches.end(),
							[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
							{ return "COUNTER_HIT" == Branch.strOutcome; }));
					const auto Timeout = std::find_if(
						Stage.Branches.begin(), Stage.Branches.end(),
						[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
						{ return "TIMEOUT" == Branch.strOutcome; });
					const std::size_t iTimeoutCount =
						static_cast<std::size_t>(std::count_if(
							Stage.Branches.begin(), Stage.Branches.end(),
							[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
							{ return "TIMEOUT" == Branch.strOutcome; }));
					const bool_t bLocalSuccess =
						Counter->strNextActionId.has_value();
					const bool_t bPatternSuccess =
						Counter->strNextPatternId.has_value();
					const MASTER_STAGE* pSuccess = nullptr;
					size_t iSuccessIndex = Pattern.Stages.size();
					if (bLocalSuccess)
					{
						const auto Target = StageByAction.find(
							*Counter->strNextActionId);
						if (Target != StageByAction.end())
						{
							iSuccessIndex = Target->second;
							pSuccess = &Pattern.Stages[iSuccessIndex];
						}
					}
					else if (bPatternSuccess)
					{
						const auto Target = PatternById.find(
							*Counter->strNextPatternId);
						if (Target != PatternById.end() &&
							!Target->second->Stages.empty())
						{
							pSuccess = &Target->second->Stages.front();
						}
					}

					// TIMEOUT은 같은 패턴의 로컬 action만 허용한다.
					// 출발 stage/proxy 종류, 닫힌 counter flag, 성공 stage/groggy flag와 전방 순서를 함께 확인한다.
					const auto TimeoutTarget = Timeout == Stage.Branches.end() ||
						!Timeout->strNextActionId.has_value() ||
						Timeout->strNextPatternId.has_value() ?
						StageByAction.end() : StageByAction.find(
							*Timeout->strNextActionId);
					const bool_t bValidSourceKind =
						"WINDUP" == Stage.strStageKind ||
						("ACTIVE" == Stage.strStageKind &&
						 Stage.CounterProxy.has_value() &&
						 "BOSS_FORWARD_ARC" == Stage.CounterProxy->strKind);
					const bool_t bTypedSuccess = nullptr != pSuccess &&
						("WINDUP" == pSuccess->strStageKind ||
						 "GROGGY" == pSuccess->strStageKind ||
						 "RECOVERY" == pSuccess->strStageKind);
					const int iSuccessGroggyState = nullptr == pSuccess ? -1 :
						MasterFlagState(*pSuccess, "boss.flag.groggy");
					if (1u != iCounterCount || 1u != iTimeoutCount ||
						bLocalSuccess == bPatternSuccess || !bValidSourceKind ||
						1 != MasterFlagState(Stage, "boss.flag.counterable") ||
						!bTypedSuccess ||
						(bLocalSuccess && iSuccessIndex <= iStage) ||
						TimeoutTarget == StageByAction.end() ||
						TimeoutTarget->second <= iStage ||
						("GROGGY" == pSuccess->strStageKind &&
						 1 != iSuccessGroggyState) ||
						("GROGGY" != pSuccess->strStageKind &&
						 0 != iSuccessGroggyState))
					{
						strOutError =
							"master COUNTER_HIT requires a typed Pattern entry success and forward same-pattern TIMEOUT: " +
							Pattern.strPatternId + "/" + Stage.strStageId;
						return false;
					}
				}

				// 모든 nextPatternId를 후속 그래프에 넣기 전에 자기 참조와 없는 패턴을 거부한다.
				// 대상은 자동 선택·조준에 참여하지 않는 AUDITION_ONLY여야 하며 다음 DFS가 순환/깊이를 검사한다.
				for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch :
					Stage.Branches)
				{
					if (!Branch.strNextPatternId.has_value())
						continue;
					const std::string& strNextPatternId =
						*Branch.strNextPatternId;
					if (strNextPatternId == Pattern.strPatternId)
					{
						strOutError =
							"master branch cannot follow up to its owner pattern: " +
							Pattern.strPatternId;
						return false;
					}
					const auto Target = PatternById.find(strNextPatternId);
					if (Target == PatternById.end())
					{
						strOutError =
							"master branch targets an unknown pattern: " +
							strNextPatternId;
						return false;
					}
					const MASTER_PATTERN& Followup = *Target->second;
					if ("AUDITION_ONLY" != Followup.strSelectionMode ||
						0u != Followup.iSelectionWeight ||
						0 != Followup.iMinimumHealthBar ||
						0 != Followup.iMaximumHealthBar ||
						"NONE" != Followup.strTargetPolicy ||
						"NONE" != Followup.strAimPolicy)
					{
						strOutError =
							"master branch follow-up target must be an untargeted "
							"AUDITION_ONLY pattern with zero selection/health bars: " +
							strNextPatternId;
						return false;
					}
					FollowupSuccessors.at(Pattern.strPatternId).insert(
						strNextPatternId);
				}
			}
		}
		// 후속 패턴은 DFS의 방문 중/완료 상태로 순환을 찾고 완료된 깊이를 재사용한다.
		// 깊이는 재귀에서 돌아오며 검사한다. 이 상한을 호출 스택 깊이의 사전 제한으로 해석하면 안 된다.
		std::map<std::string, uint32_t, std::less<>> FollowupVisitState;
		std::map<std::string, uint32_t, std::less<>> FollowupDepth;
		std::function<bool_t(const std::string&)> ValidateFollowupDepth;
		ValidateFollowupDepth = [&](const std::string& strPatternId) -> bool_t
		{
			uint32_t& iVisitState = FollowupVisitState[strPatternId];
			if (1u == iVisitState)
			{
				strOutError = "master pattern follow-up graph contains a cycle: " +
					strPatternId;
				return false;
			}
			if (2u == iVisitState)
				return true;
			iVisitState = 1u;
			uint32_t iMaximumDepth = 0u;
			for (const std::string& strTargetId :
				FollowupSuccessors.at(strPatternId))
			{
				if (!ValidateFollowupDepth(strTargetId))
					return false;
				const uint32_t iTargetDepth = FollowupDepth.at(strTargetId);
				if (iTargetDepth >= VALTAN_PATTERN_FOLLOWUP_MAX_DEPTH)
				{
					strOutError =
						"master pattern follow-up graph exceeds maximum depth " +
						std::to_string(VALTAN_PATTERN_FOLLOWUP_MAX_DEPTH) + ": " +
						strPatternId;
					return false;
				}
				iMaximumDepth = (std::max)(
					iMaximumDepth, iTargetDepth + 1u);
			}
			iVisitState = 2u;
			FollowupDepth.emplace(strPatternId, iMaximumDepth);
			return true;
		};
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if (!ValidateFollowupDepth(Pattern.strPatternId))
				return false;
		}
		// 잡기 계열과 finale 및 그 유령 자식처럼 유한 경로가 필요한 집합만 stage 순환 검사에 넣는다.
		std::set<std::string, std::less<>> FinitePatternIds = {
			"VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF",
			"VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL" };
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if (!Pattern.Finale.has_value())
				continue;
			FinitePatternIds.insert(Pattern.strPatternId);
			for (const std::string& ChildId : Pattern.Finale->GhostPatternIds)
			{
				const auto Child = std::find_if(Out.Patterns.begin(), Out.Patterns.end(),
					[&ChildId](const MASTER_PATTERN& Candidate)
					{ return Candidate.strPatternId == ChildId; });
				if (Child == Out.Patterns.end() || ChildId == Pattern.strPatternId ||
					Child->Finale.has_value() || !Child->WorldEventTriggerRefs.empty())
				{
					strOutError = "master finale has an unresolved, recursive or terrain ghost attack: " + ChildId;
					return false;
				}
				FinitePatternIds.insert(ChildId);
			}
		}
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
			if (FinitePatternIds.contains(Pattern.strPatternId) &&
				!Validate_FinitePatternStageGraph(Pattern, strOutError))
				return false;

		// NORMAL 패턴 전체와 normalSelection의 ID 집합을 비교한다.
		// 아래 entry-only 예외를 제외하면 두 집합이 같아야 하며 단순 부분집합 검사가 아니다.
		std::set<std::string, std::less<>> ManagedNormalPatternIds;
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if ("NORMAL" == Pattern.strSelectionMode)
				ManagedNormalPatternIds.insert(Pattern.strPatternId);
		}
		const std::set<std::string, std::less<>> WeightedPatternIds(
			Out.NormalSelection.PatternIds.begin(),
			Out.NormalSelection.PatternIds.end());
		// 선택적인 입장 패턴은 NORMAL Product 메타데이터를 유지한다.
		// 분리된 decision 소유 모델에서는 이 입장 패턴을 가중치 선택 집합에 넣지 않는다.
		for (const std::string& PatternId : ScriptedEntryOnlyPatternIds)
		{
			if (0u == ManagedNormalPatternIds.erase(PatternId) ||
				WeightedPatternIds.contains(PatternId))
			{
				strOutError =
					"master optional entry-only gate must be an unweighted NORMAL pattern";
				return false;
			}
		}
		if (WeightedPatternIds != ManagedNormalPatternIds)
		{
			strOutError =
				"master normalSelection is not a subset of the managed normal pattern set";
			return false;
		}

		// counterReactionLayers는 관리 패턴 밖의 참조용 반응이다.
		// 그 owner가 위에서 수집한 관리 PatternIds에도 있으면 이중 역할로 보고 거부한다.
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
			Out.CounterReactionLayers)
		{
			if (PatternIds.contains(Layer.strOwnerPatternId))
			{
				strOutError =
					"reference-only counter reaction admits a managed pattern";
				return false;
			}
		}
		return true;
	}

	// 선택 offset은 누락이면 빈 목록을 허용하지만 명시한 빈 배열은 거절한다. 오름차순 ms 값으로 읽는다.
	bool_t Read_OptionalOrderedHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& OutOffsets)
	{
		OutOffsets.clear();
		const DATA_JSON_VALUE* pOffsets = Object.Find("hitOffsetsMs");
		if (nullptr == pOffsets)
			return true;
		if (!pOffsets->Is_Array() || pOffsets->Get_Array().empty() ||
			pOffsets->Get_Array().size() > 1000u)
		{
			return false;
		}

		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Value.Is_Number() || !std::isfinite(Value.Get_Number()) ||
				std::floor(Value.Get_Number()) != Value.Get_Number() ||
				Value.Get_Number() < 0.0 ||
				Value.Get_Number() > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				return false;
			}
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			OutOffsets.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	// 선택 ownerHitChain의 지연 ms와 후속 action/presentation ID를 읽는다. 출력 optional은 먼저 비운다.
	bool Read_CombatObjectOwnerHitChain(
		const DATA_JSON_VALUE& Object,
		std::optional<Client::VALTAN_COMBAT_OBJECT_OWNER_HIT_CHAIN_VIEW>& Out)
	{
		Out.reset();
		const auto* Chain = Object.Find("ownerHitChain");
		if (!Chain)
			return true;
		Client::VALTAN_COMBAT_OBJECT_OWNER_HIT_CHAIN_VIEW Value;
		if (!Chain->Is_Object() || !Has_ExactProperties(*Chain,
			{ "triggerActionId", "delayMs", "armedPresentationEventId" }) ||
			!Read_RequiredUInt32(*Chain, "delayMs", Value.iDelayMs))
			return false;
		Value.strTriggerActionId = Read_String(*Chain, "triggerActionId");
		Value.strArmedPresentationEventId = Read_String(*Chain, "armedPresentationEventId");
		if (!Is_StableToken(Value.strTriggerActionId) ||
			!Is_StableToken(Value.strArmedPresentationEventId) ||
			Value.iDelayMs == 0u || Value.iDelayMs > 600000u)
			return false;
		Out = std::move(Value);
		return true;
	}

	// 전투 오브젝트 gameplay와 visual 결합의 임시 레코드다. 이동/판정/연출을 복사해 stage 표시에서 함께 사용한다.
	struct COMBAT_OBJECT_EFFECT_REFERENCE final
	{
		std::string strPreparationEventId;
		std::string strClientVisualId;
		std::string strEffectAssetId;
		std::string strEffectV2GroupId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		std::string strKind;
		std::string strOriginPolicy;
		std::string strDirectionPolicy;
		f32_t fSpeedMps = 0.f;
		f32_t fMaximumDistanceM = 0.f;
		uint32_t iLifetimeMs = 0u;
		f32_t fCoverRadiusM = 0.f;
		std::optional<Client::VALTAN_COMBAT_OBJECT_OWNER_HIT_CHAIN_VIEW> OwnerHitChain;
		std::vector<std::string> HitIds;
		std::vector<uint32_t> HitOffsetsMs;
		std::vector<Client::VALTAN_COMBAT_OBJECT_HIT_VIEW> Hits;
		std::vector<Client::VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW>
			PresentationEvents;
	};

	// 검증된 animation binding의 clip 값을 도구 occurrence 뷰로 복사한다. 런타임 clip 리소스를 로드하지 않는다.
	Client::VALTAN_CLIP_OCCURRENCE_VIEW Build_ClipOccurrenceView(
		const Client::BOSS_PATTERN_ANIMATION_CLIP& Clip)
	{
		Client::VALTAN_CLIP_OCCURRENCE_VIEW View;
		View.strClipOccurrenceId = Clip.strClipOccurrenceId;
		View.strClipName = Clip.strClipName;
		View.strMappingBasis = Clip.strMappingBasis;
		View.iSourceStartMs = Clip.iSourceStartMs;
		View.iPlayMs = Clip.iPlayMs;
		View.fPlayRate = Clip.fPlayRate;
		View.bLoop = Clip.bLoop;
		return View;
	}

	// 기믹 목록부터 일반 목록 순서로 stable ID를 선형 조회한다. 반환 포인터는 View가 소유한 원소를 차용한다.
	Client::VALTAN_PATTERN_VIEW* Find_Pattern(
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			const auto Found = std::find_if(
				pGroup->begin(), pGroup->end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (Found != pGroup->end())
				return &*Found;
		}
		return nullptr;
	}

	// 읽기 전용 조회도 같은 두 목록을 순서대로 찾는다. View 교체나 vector 변경 뒤에는 반환 포인터를 재사용하지 않는다.
	const Client::VALTAN_PATTERN_VIEW* Find_Pattern(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		for (const std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			const auto Found = std::find_if(
				pGroup->begin(), pGroup->end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (Found != pGroup->end())
				return &*Found;
		}
		return nullptr;
	}

	// 패턴 내부 stage ID를 선형 조회한다. 결과는 패턴 vector 원소의 차용 포인터다.
	Client::VALTAN_STAGE_VIEW* Find_Stage(
		Client::VALTAN_PATTERN_VIEW& Pattern,
		const std::string_view strStageId)
	{
		const auto Found = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[strStageId](const Client::VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == strStageId;
			});
		return Found == Pattern.Stages.end() ? nullptr : &*Found;
	}

	// 게시 occurrence와 저작 occurrence의 계약 필드를 대조한다. 재생 배율만 작은 오차를 허용한다.
	bool_t Equal_MasterOccurrence(
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Product,
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Master)
	{
		return Product.strClipOccurrenceId == Master.strClipOccurrenceId &&
			Product.strClipName == Master.strClipName &&
			Product.strMappingBasis == Master.strMappingBasis &&
			Product.iSourceStartMs == Master.iSourceStartMs &&
			Product.iPlayMs == Master.iPlayMs &&
			std::abs(Product.fPlayRate - Master.fPlayRate) <= 0.000001f &&
			Product.bLoop == Master.bLoop;
	}

	// cue identity와 저작 transform/시각/정책을 대조한다. 선택 종료 시각과 playback offset은 존재 여부도 함께 비교한다.
	bool_t Equal_AuthoredCue(
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Product,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Authored)
	{
		return Product.strBindingId == Authored.strBindingId &&
			Product.strOccurrenceId == Authored.strOccurrenceId &&
			Product.strPatternId == Authored.strPatternId &&
			Product.strStageId == Authored.strStageId &&
			Product.strActionId == Authored.strActionId &&
			Product.strClipOccurrenceId == Authored.strClipOccurrenceId &&
			Product.strEffectAssetId == Authored.strEffectAssetId &&
			Product.strAnchorSlotId == Authored.strAnchorSlotId &&
			Product.eFollowPolicy == Authored.eFollowPolicy &&
			Product.eStopPolicy == Authored.eStopPolicy &&
			Product.strRepeatPolicy == Authored.strRepeatPolicy &&
			Product.eScalePolicy == Authored.eScalePolicy &&
			Product.strScalePolicy == Authored.strScalePolicy &&
			Product.bHasExplicitScalePolicy ==
				Authored.bHasExplicitScalePolicy &&
			Product.bUsesStageClock == Authored.bUsesStageClock &&
			Product.iStageOffsetMs == Authored.iStageOffsetMs &&
			Product.vWorldScale.x == Authored.vWorldScale.x &&
			Product.vWorldScale.y == Authored.vWorldScale.y &&
			Product.vWorldScale.z == Authored.vWorldScale.z &&
			Product.iSourceStartMs == Authored.iSourceStartMs &&
			Product.bHasPlaybackOffset == Authored.bHasPlaybackOffset &&
			(!Product.bHasPlaybackOffset || Product.iPlaybackOffsetMs == Authored.iPlaybackOffsetMs) &&
			Product.bHasSourceEnd == Authored.bHasSourceEnd &&
			(!Product.bHasSourceEnd ||
			 Product.iSourceEndMs == Authored.iSourceEndMs) &&
			Product.LocalTransform.vPosition.x == Authored.LocalTransform.vPosition.x &&
			Product.LocalTransform.vPosition.y == Authored.LocalTransform.vPosition.y &&
			Product.LocalTransform.vPosition.z == Authored.LocalTransform.vPosition.z &&
			Product.LocalTransform.vRotationDegrees.x ==
				Authored.LocalTransform.vRotationDegrees.x &&
			Product.LocalTransform.vRotationDegrees.y ==
				Authored.LocalTransform.vRotationDegrees.y &&
			Product.LocalTransform.vRotationDegrees.z ==
				Authored.LocalTransform.vRotationDegrees.z &&
			Product.LocalTransform.vScale.x == Authored.LocalTransform.vScale.x &&
			Product.LocalTransform.vScale.y == Authored.LocalTransform.vScale.y &&
			Product.LocalTransform.vScale.z == Authored.LocalTransform.vScale.z;
	}

	// 선택 stage 이동의 존재와 typed 값을 대조한다. 같은 optional 유무만으로 동일하다고 보지 않는다.
	bool_t Equal_StageMotion(
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->iRetargetDelayMs == Right->iRetargetDelayMs &&
			 Left->fSpeedMps == Right->fSpeedMps &&
			 Left->fDistance == Right->fDistance &&
			 Left->iCornerIndex == Right->iCornerIndex &&
			 Left->HalfExtentsM == Right->HalfExtentsM);
	}

	// action 순서와 값, 잡기 해제의 전용 필드까지 대조한다.
	bool_t Equal_StageActions(
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strTrigger != Right[i].strTrigger ||
				Left[i].strKind != Right[i].strKind ||
				Left[i].strTargetId != Right[i].strTargetId ||
				Left[i].fValue != Right[i].fValue ||
				Left[i].strReleaseMode != Right[i].strReleaseMode ||
				Left[i].fSpeedMps != Right[i].fSpeedMps ||
				Left[i].iDurationMs != Right[i].iDurationMs ||
				Left[i].fYawOffsetDegrees != Right[i].fYawOffsetDegrees)
			{
				return false;
			}
		}
		return true;
	}

	// branch의 순서, outcome과 로컬/다른 패턴 연결을 대조한다.
	bool_t Equal_StageBranches(
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strOutcome != Right[i].strOutcome ||
				Left[i].strNextActionId != Right[i].strNextActionId ||
				Left[i].strNextPatternId != Right[i].strNextPatternId)
			{
				return false;
			}
		}
		return true;
	}

	// 선택 counter 도형의 존재와 위치/크기/각도를 대조한다.
	bool_t Equal_CounterProxy(
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& Left,
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->strSpace == Right->strSpace &&
			 Left->fForwardOffsetM == Right->fForwardOffsetM &&
			 Left->fRightOffsetM == Right->fRightOffsetM &&
			 Left->fRadiusM == Right->fRadiusM &&
			 Left->fArcDegrees == Right->fArcDegrees);
	}

	// stage gameplay의 게시 결과를 저작 정의와 대조한다.
	// COUNTER_HIT 분기가 없는 저작 counterProxy는 휴면 상태로 취급해 Product에 요구하지 않는다.
	bool_t Equal_MasterStageGameplay(
		const Client::VALTAN_STAGE_VIEW& Product,
		const MASTER_STAGE& Master)
	{
		const bool_t bCounterEnabled = Master.Branches.end() != std::find_if(
			Master.Branches.begin(), Master.Branches.end(),
			[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "COUNTER_HIT" == Branch.strOutcome; });
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> ProductCounterProxy =
			bCounterEnabled ? Master.CounterProxy : std::nullopt;
		return Product.strStageId == Master.strStageId &&
			Product.strActionId == Master.strActionId &&
			Product.strStageKind == Master.strStageKind &&
			Product.iDurationMs == Master.iDurationMs &&
			Product.fVerticalOffsetM == Master.fVerticalOffsetM &&
			Product.strHitShape == Master.strHitShape &&
			Product.fHitOuterRadius == Master.fHitOuterRadius &&
			Product.fHitInnerRadius == Master.fHitInnerRadius &&
			Product.fHitAngleDegrees == Master.fHitAngleDegrees &&
			Product.fHitLength == Master.fHitLength &&
			Product.fHitHalfWidth == Master.fHitHalfWidth &&
			Product.iHitCount == Master.iHitCount &&
			Product.iHitIntervalMs == Master.iHitIntervalMs &&
			Product.iHitDelayMs == Master.iHitDelayMs &&
			Product.HitOffsetsMs == Master.HitOffsetsMs &&
			Product.AttackContacts == Master.AttackContacts &&
			Product.bHasHitAnchor == Master.bHasHitAnchor &&
			Product.strHitAnchorKind == Master.strHitAnchorKind &&
			Product.fHitAnchorForwardOffsetM ==
				Master.fHitAnchorForwardOffsetM &&
			Product.fHitAnchorRightOffsetM == Master.fHitAnchorRightOffsetM &&
			Product.fHitAnchorYawOffsetDegrees ==
				Master.fHitAnchorYawOffsetDegrees &&
			Product.bHasHitActivation == Master.bHasHitActivation &&
			Product.iHitActivationStartMs == Master.iHitActivationStartMs &&
			Product.iHitActivationLifetimeMs ==
				Master.iHitActivationLifetimeMs &&
			Product.strServerDamageProfileId ==
				Master.strServerDamageProfileId &&
			Product.strPlayerResponse == Master.strPlayerResponse &&
			Product.strAttachmentSlot == Master.strAttachmentSlot &&
			Product.GripLocalOffset == Master.GripLocalOffset &&
			Product.strPartDamagePolicy == Master.strPartDamagePolicy &&
			Equal_CounterProxy(Product.CounterProxy, ProductCounterProxy) &&
			Product.BossResponse == Master.BossResponse &&
			Product.fPushRangeM == Master.fPushRangeM &&
			Product.iPushMs == Master.iPushMs &&
			Product.bKnockdown == Master.bKnockdown &&
			Product.iDownMs == Master.iDownMs &&
			Product.strAimTargetPolicy == Master.strAimTargetPolicy &&
			Product.bHasAimEnd == Master.bHasAimEnd &&
			Product.iAimEndMs == Master.iAimEndMs &&
			Product.bHasAimResponseScale == Master.bHasAimResponseScale &&
			Product.fAimResponseScale == Master.fAimResponseScale &&
			Equal_StageMotion(Product.Motion, Master.Motion) &&
			Equal_StageActions(Product.Actions, Master.Actions) &&
			Equal_StageBranches(Product.Branches, Master.Branches);
	}

	// 패턴 단위 도약의 존재, 종류, anchor와 ms/위치 값을 대조한다.
	bool_t Equal_PatternServerMotion(
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->strAnchorId == Right->strAnchorId &&
			 Left->LandingPosition == Right->LandingPosition &&
			 Left->fApexHeight == Right->fApexHeight &&
			 Left->strTravelStageId == Right->strTravelStageId &&
			 Left->iTakeoffStartMs == Right->iTakeoffStartMs &&
			 Left->iTakeoffEndMs == Right->iTakeoffEndMs &&
			 Left->iTravelStartMs == Right->iTravelStartMs &&
			 Left->iTravelEndMs == Right->iTravelEndMs &&
			 Left->bMoveToAnchorBeforeTakeoff == Right->bMoveToAnchorBeforeTakeoff);
	}

	// 패턴의 선택, 대상, 무적과 이동/finale 메타데이터를 대조한다. 개별 stage는 별도 비교한다.
	bool_t Equal_MasterPatternGameplay(
		const Client::VALTAN_PATTERN_VIEW& Product,
		const MASTER_PATTERN& Master)
	{
		return Product.strPatternId == Master.strPatternId &&
			Product.strCategory == Master.strCategory &&
			Product.iMinimumPhase == Master.iMinimumPhase &&
			Product.iMaximumPhase == Master.iMaximumPhase &&
			Product.strTargetPolicy == Master.strTargetPolicy &&
			Product.strAimPolicy == Master.strAimPolicy &&
			Product.strDisplayName == Master.strDisplayName &&
			Product.strActionId == Master.strActionId &&
			Product.SourceActionIds == Master.SourceActionIds &&
			Product.strSelectionMode == Master.strSelectionMode &&
			Product.iMinimumHealthBar == Master.iMinimumHealthBar &&
			Product.iMaximumHealthBar == Master.iMaximumHealthBar &&
			Product.iTriggerHealthBar == Master.iTriggerHealthBar &&
			Product.iTriggerOrder == Master.iTriggerOrder &&
			Product.strArmorRequirement == Master.strArmorRequirement &&
			Product.strPhaseRequirement == Master.strPhaseRequirement &&
			Product.bInvulnerableWhileRunning ==
				Master.bInvulnerableWhileRunning &&
			Product.iSelectionWeight == Master.iSelectionWeight &&
			Product.iMaximumConsecutiveUses ==
				Master.iMaximumConsecutiveUses &&
			Product.fMinimumRange == Master.fMinimumRange &&
			Product.fMaximumRange == Master.fMaximumRange &&
			Product.fVerticalOffsetM == Master.fVerticalOffsetM &&
			Equal_PatternServerMotion(Product.ServerMotion, Master.ServerMotion) &&
			Product.Finale == Master.Finale;
	}

	// clip 원본 재생 ms를 playRate로 나누고 반올림해 stage의 재생 시간 예산을 배분한다.
	// EXACT, HOLD_LAST_POSE, LOOP_TO_STAGE_END의 남는 시간 처리를 구분한다.
	// Stage의 occurrence를 순서대로 변경하므로 실패 시 보존은 상위 Load의 임시 View가 담당한다.
	bool_t Assign_MasterWallBudgets(
		Client::VALTAN_STAGE_VIEW& Stage,
		std::string& strOutError)
	{
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty() ||
				"NONE" != Stage.strAnimationEndPolicy)
			{
				strOutError =
					"NONE animation cannot own a clip wall budget: " +
					Stage.strActionId;
				return false;
			}
			return true;
		}
		uint64_t iKnownWallMs = 0u;
		size_t iUnknownCount = 0u;
		size_t iUnknownIndex = 0u;
		for (size_t i = 0u; i < Stage.ClipOccurrences.size(); ++i)
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[i];
			Clip.iAuthoringWallMs = 0u;
			if (0u == Clip.iPlayMs)
			{
				++iUnknownCount;
				iUnknownIndex = i;
				continue;
			}
			const double WallMs = static_cast<double>(Clip.iPlayMs) /
				static_cast<double>(Clip.fPlayRate);
			if (!std::isfinite(WallMs) || WallMs <= 0.0 ||
				WallMs > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutError = "master animation wall budget is invalid: " +
					Clip.strClipOccurrenceId;
				return false;
			}
			Clip.iAuthoringWallMs = static_cast<uint32_t>(std::llround(WallMs));
			iKnownWallMs += Clip.iAuthoringWallMs;
		}
		if (1u < iUnknownCount || iKnownWallMs >
			static_cast<uint64_t>(Stage.iDurationMs) + 2u)
		{
			strOutError =
				"master stage cannot derive one deterministic animation wall budget: " +
				Stage.strActionId;
			return false;
		}
		const bool_t bUnknownLoops = 1u == iUnknownCount &&
			Stage.ClipOccurrences[iUnknownIndex].bLoop;
		if (bUnknownLoops &&
			"LOOP_TO_STAGE_END" != Stage.strAnimationEndPolicy)
		{
			strOutError =
				"repeatUntilStageEnd requires LOOP_TO_STAGE_END: " +
				Stage.strActionId;
			return false;
		}
		if ("LOOP_TO_STAGE_END" == Stage.strAnimationEndPolicy)
		{
			if (1u != iUnknownCount || !bUnknownLoops ||
				iKnownWallMs >= Stage.iDurationMs)
			{
				strOutError =
					"LOOP_TO_STAGE_END requires one looping remainder: " +
					Stage.strActionId;
				return false;
			}
			const uint64_t iRemaining =
				static_cast<uint64_t>(Stage.iDurationMs) - iKnownWallMs;
			Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
				static_cast<uint32_t>(iRemaining);
			iKnownWallMs += iRemaining;
		}
		else if ("HOLD_LAST_POSE" == Stage.strAnimationEndPolicy)
		{
			if (bUnknownLoops || iKnownWallMs >= Stage.iDurationMs + 2ull)
			{
				strOutError = "HOLD_LAST_POSE animation budget is invalid: " +
					Stage.strActionId;
				return false;
			}
			if (1u == iUnknownCount)
			{
				if (iKnownWallMs >= Stage.iDurationMs)
					return false;
				Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
			else if (iKnownWallMs < Stage.iDurationMs &&
				!Stage.ClipOccurrences.empty())
			{
				Stage.ClipOccurrences.back().iAuthoringWallMs +=
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
		}
		else if ("EXACT" == Stage.strAnimationEndPolicy)
		{
			const int64_t iDifference = static_cast<int64_t>(Stage.iDurationMs) -
				static_cast<int64_t>(iKnownWallMs);
			if (0u != iUnknownCount || std::abs(iDifference) > 2 ||
				Stage.ClipOccurrences.empty())
			{
				strOutError = "EXACT animation does not fill its Server stage: " +
					Stage.strActionId;
				return false;
			}
			const int64_t iCorrected = static_cast<int64_t>(
				Stage.ClipOccurrences.back().iAuthoringWallMs) + iDifference;
			if (iCorrected <= 0)
				return false;
			Stage.ClipOccurrences.back().iAuthoringWallMs =
				static_cast<uint32_t>(iCorrected);
			iKnownWallMs = Stage.iDurationMs;
		}
		else
		{
			strOutError = "master animation endPolicy is invalid: " +
				Stage.strActionId;
			return false;
		}
		if (iKnownWallMs != Stage.iDurationMs)
		{
			strOutError = "master animation wall budget did not commit exactly: " +
				Stage.strActionId;
			return false;
		}
		return true;
	}

	// 체력 기믹의 발동 값과 같은 체력의 순서를 결합할 때 쓰는 임시 메타데이터다.
	struct SPLIT_MECHANIC final
	{
		uint32_t iHealthBar = 0u;
		uint32_t iTriggerOrder = 0u;
	};

	// pCue는 Parse_SplitMasterDocument가 받은 presentation JSON 원소를 차용한다. 결합 함수 밖으로 내보내지 않는다.
	struct SPLIT_CUE_OWNER final
	{
		std::string strPatternId;
		std::string strStageId;
		const DATA_JSON_VALUE* pCue = nullptr;
	};

	// 생성 event가 속한 패턴/stage와 전투 오브젝트 정의 ID를 값으로 보관한다.
	struct SPLIT_SPAWN_OWNER final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strCombatObjectArchetypeId;
	};

	// 독립 Effect 선언은 stage cue 또는 spawn event의 ID를 가리킨다. 내용은 해당 owner에서 가져온다.
	struct SPLIT_INDEPENDENT_DECLARATION final
	{
		std::string strIndependentEffectId;
		std::string strDisplayName;
		std::string strOwnership;
		std::string strReferenceId;
	};

	// 호환 Master에 넘길 문자열 배열을 만든다. 입력 순서와 중복 여부를 그대로 보존한다.
	DATA_JSON_VALUE Build_StringArray(
		const std::vector<std::string>& Values)
	{
		DATA_JSON_VALUE::ARRAY Result;
		Result.reserve(Values.size());
		for (const std::string& Value : Values)
			Result.push_back(DATA_JSON_VALUE::String(Value));
		return DATA_JSON_VALUE::Array(std::move(Result));
	}

	// gameplay와 presentation의 schema/version 및 보스/encounter/scope identity를 맞춘다.
	bool_t Validate_SplitRootIdentity(
		const DATA_JSON_VALUE& Gameplay,
		const DATA_JSON_VALUE& Presentation,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Gameplay,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "previewPaths", "retiredPatternIds", "decisionModel",
				  "counterReactionLayers", "patterns" }) ||
			!Has_ExactProperties(Presentation,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "patterns", "independentEffects" }))
		{
			strOutError = "split authoring root has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pGameplaySchema = Required(
			Gameplay, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPresentationSchema = Required(
			Presentation, "schema", DATA_JSON_TYPE::STRING);
		if (nullptr == pGameplaySchema ||
			"lostark.valtan-gameplay-authoring" != pGameplaySchema->Get_String() ||
			nullptr == pPresentationSchema ||
			"lostark.valtan-pattern-presentation-authoring" !=
				pPresentationSchema->Get_String() ||
			!Is_NonNegativeInteger(Gameplay.Find("formatVersion")) ||
			1.0 != Gameplay.Find("formatVersion")->Get_Number() ||
			!Is_NonNegativeInteger(Presentation.Find("formatVersion")) ||
			1.0 != Presentation.Find("formatVersion")->Get_Number())
		{
			strOutError = "split authoring schema/version is invalid";
			return false;
		}
		for (const std::string_view Field :
			{ "bossArchetypeId", "encounterId", "scope" })
		{
			const DATA_JSON_VALUE* pGameplayValue = Required(
				Gameplay, Field, DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pPresentationValue = Required(
				Presentation, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pGameplayValue || nullptr == pPresentationValue ||
				pGameplayValue->Get_String() != pPresentationValue->Get_String())
			{
				strOutError = "split authoring root identity mismatch: " +
					std::string(Field);
				return false;
			}
		}
		if ("BOSS_VALTAN" != Read_String(Gameplay, "bossArchetypeId") ||
			"ENCOUNTER_VALTAN" != Read_String(Gameplay, "encounterId") ||
			"PHASE_ONE" != Read_String(Gameplay, "scope"))
		{
			strOutError = "split authoring root identity is not Valtan Phase One";
			return false;
		}
		return true;
	}

	// stage 시계와 clip occurrence 시계의 cue를 서로 다른 필드 계약으로 검사한다.
	// transform과 scale 정책의 형식을 확인하며 실제 owner와 stage 길이는 결합 단계에서도 확인한다.
	bool_t Validate_SplitCue(
		const DATA_JSON_VALUE& Cue,
		std::string& strOutError)
	{
		const bool_t bUsesStageClock = nullptr != Cue.Find("timingBasis");
		const bool_t bValidShape = bUsesStageClock ?
			Has_ExactPropertiesWithOptional(Cue,
				{ "cueId", "occurrenceId", "effectAssetId", "timingBasis",
				  "stageOffsetMs", "anchorSlotId", "followPolicy", "stopPolicy",
				  "repeatPolicy", "localTransform", "scalePolicy" }, { "playbackOffsetMs", "stageEndMs" }) :
			Has_ExactPropertiesWithOptional(Cue,
				{ "cueId", "occurrenceId", "effectAssetId", "clipOccurrenceId",
				  "sourceStartMs", "sourceEndMs", "anchorSlotId", "followPolicy",
				  "stopPolicy", "repeatPolicy", "localTransform",
				  "scalePolicy", "mappingBasis" }, { "playbackOffsetMs" });
		if (!bValidShape)
		{
			strOutError = "split presentation cue has unexpected properties";
			return false;
		}
		for (const std::string_view Field :
			{ "cueId", "occurrenceId", "effectAssetId", "anchorSlotId" })
		{
			const DATA_JSON_VALUE* pValue = Required(
				Cue, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pValue || !Is_StableToken(pValue->Get_String()))
			{
				strOutError = "split presentation cue identity is invalid: " +
					std::string(Field);
				return false;
			}
		}
		if (bUsesStageClock)
		{
			if ("STAGE_CLOCK" != Read_String(Cue, "timingBasis") ||
				!Is_NonNegativeInteger(Cue.Find("stageOffsetMs")) ||
				("follow" != Read_String(Cue, "followPolicy") &&
				 "snapshot" != Read_String(Cue, "followPolicy")) ||
				("natural" != Read_String(Cue, "stopPolicy") && "cue_end" != Read_String(Cue, "stopPolicy")) ||
				"once" != Read_String(Cue, "repeatPolicy"))
			{
				strOutError =
					"split stage-clock cue timing/policy is invalid";
				return false;
			}
			const auto* end = Cue.Find("stageEndMs");
			const bool hasEnd = end && !end->Is_Null();
			if (("cue_end" == Read_String(Cue, "stopPolicy")) != hasEnd ||
				(hasEnd && (!Is_NonNegativeInteger(end) ||
				 end->Get_Number() <= Cue.Find("stageOffsetMs")->Get_Number() || end->Get_Number() > 600000.0)))
			{
				strOutError = "split stage-clock cue end is invalid";
				return false;
			}
		}
		else
		{
			const DATA_JSON_VALUE* pClipOccurrence = Required(
				Cue, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pSourceEnd = Cue.Find("sourceEndMs");
			if (nullptr == pClipOccurrence ||
				!Is_StableToken(pClipOccurrence->Get_String()) ||
				!Is_NonNegativeInteger(Cue.Find("sourceStartMs")) ||
				nullptr == pSourceEnd ||
				(!pSourceEnd->Is_Null() && !Is_NonNegativeInteger(pSourceEnd)) ||
				(!pSourceEnd->Is_Null() && pSourceEnd->Get_Number() <
					Cue.Find("sourceStartMs")->Get_Number()) ||
				Read_String(Cue, "mappingBasis").empty() ||
				("follow" != Read_String(Cue, "followPolicy") &&
				 "snapshot" != Read_String(Cue, "followPolicy")) ||
				("natural" != Read_String(Cue, "stopPolicy") &&
				 "cue_end" != Read_String(Cue, "stopPolicy")) ||
				("once" != Read_String(Cue, "repeatPolicy") &&
				 "each_loop" != Read_String(Cue, "repeatPolicy")))
			{
				strOutError = "split presentation cue timing/policy is invalid";
				return false;
			}
		}
		if (const auto* offset = Cue.Find("playbackOffsetMs"))
		{
			if (!Is_NonNegativeInteger(offset) || offset->Get_Number() > 600000.0)
			{
				strOutError = "Effect playbackOffsetMs must be an integer within 0..600000";
				return false;
			}
		}
		const DATA_JSON_VALUE* pTransform = Required(
			Cue, "localTransform", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pTransform || !Has_ExactProperties(*pTransform,
				{ "position", "rotationDegrees", "scale" }))
		{
			strOutError = "split presentation cue transform is invalid";
			return false;
		}
		for (const std::string_view Field :
			{ "position", "rotationDegrees", "scale" })
		{
			const DATA_JSON_VALUE* pValues = Required(
				*pTransform, Field, DATA_JSON_TYPE::ARRAY);
			if (nullptr == pValues || 3u != pValues->Get_Array().size() ||
				std::any_of(pValues->Get_Array().begin(),
					pValues->Get_Array().end(),
					[](const DATA_JSON_VALUE& Value)
					{
						return !Is_FiniteNumber(&Value);
					}))
			{
				strOutError = "split presentation cue transform vector is invalid";
				return false;
			}
		}
		const DATA_JSON_VALUE* pScalePolicy = Required(
			Cue, "scalePolicy", DATA_JSON_TYPE::OBJECT);
		const std::string strScalePolicy = nullptr == pScalePolicy ?
			std::string{} : Read_String(*pScalePolicy, "kind");
		if (nullptr == pScalePolicy ||
			("OWNER_RELATIVE" == strScalePolicy &&
			 !Has_ExactProperties(*pScalePolicy, { "kind" })) ||
			(("GAMEPLAY_FOOTPRINT" == strScalePolicy ||
			  "ARENA_ABSOLUTE" == strScalePolicy) &&
				 (!Has_ExactProperties(*pScalePolicy, { "kind", "worldScale" }) ||
				  nullptr == Required(*pScalePolicy, "worldScale",
					  DATA_JSON_TYPE::ARRAY))) ||
			("OWNER_RELATIVE" != strScalePolicy &&
			 "GAMEPLAY_FOOTPRINT" != strScalePolicy &&
			 "ARENA_ABSOLUTE" != strScalePolicy))
		{
			strOutError = "split presentation cue scalePolicy is invalid";
			return false;
		}
		if ("OWNER_RELATIVE" != strScalePolicy)
		{
			const DATA_JSON_VALUE::ARRAY& WorldScale =
				pScalePolicy->Find("worldScale")->Get_Array();
			if (3u != WorldScale.size() ||
				std::any_of(WorldScale.begin(), WorldScale.end(),
					[](const DATA_JSON_VALUE& Value)
					{
						return !Is_FiniteNumber(&Value) ||
							Value.Get_Number() < 0.000001 || Value.Get_Number() > 1000.0;
					}))
			{
				strOutError =
					"split presentation cue worldScale must be finite and within 0.000001..1000";
				return false;
			}
		}
		return true;
	}

	// 검증한 cue를 소유 stage/action 및 stage 길이와 함께 복사한다. Effect 리소스 내용은 읽지 않는다.
	Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW Build_SplitCueView(
		const DATA_JSON_VALUE& Cue,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		const uint32_t iStageDurationMs)
	{
		Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW View;
		View.strBindingId = Read_String(Cue, "cueId");
		View.strOccurrenceId = Read_String(Cue, "occurrenceId");
		View.strPatternId = strPatternId;
		View.strStageId = strStageId;
		View.strActionId = strActionId;
		View.bUsesStageClock = nullptr != Cue.Find("timingBasis");
		View.strClipOccurrenceId = View.bUsesStageClock ? std::string{} :
			Read_String(Cue, "clipOccurrenceId");
		View.strEffectAssetId = Read_String(Cue, "effectAssetId");
		View.strAnchorSlotId = Read_String(Cue, "anchorSlotId");
		View.strFollowPolicy = Read_String(Cue, "followPolicy");
		View.strStopPolicy = Read_String(Cue, "stopPolicy");
		View.strRepeatPolicy = Read_String(Cue, "repeatPolicy");
		const DATA_JSON_VALUE& ScalePolicy = *Cue.Find("scalePolicy");
		View.strScalePolicy = Read_String(ScalePolicy, "kind");
		View.bHasExplicitScalePolicy = true;
		if ("GAMEPLAY_FOOTPRINT" == View.strScalePolicy)
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT;
		else if ("ARENA_ABSOLUTE" == View.strScalePolicy)
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE;
		else
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
		if (Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE !=
			View.eScalePolicy)
		{
			const DATA_JSON_VALUE::ARRAY& WorldScale =
				ScalePolicy.Find("worldScale")->Get_Array();
			View.vWorldScale = {
				static_cast<f32_t>(WorldScale[0].Get_Number()),
				static_cast<f32_t>(WorldScale[1].Get_Number()),
				static_cast<f32_t>(WorldScale[2].Get_Number()) };
		}
		View.eFollowPolicy = "snapshot" == View.strFollowPolicy ?
			Client::EFFECT_FOLLOW_POLICY::SNAPSHOT :
			Client::EFFECT_FOLLOW_POLICY::FOLLOW;
		View.eStopPolicy = "cue_end" == View.strStopPolicy ?
			Client::EFFECT_STOP_POLICY::CUE_END :
			Client::EFFECT_STOP_POLICY::NATURAL;
		if (View.bUsesStageClock)
		{
			View.iStageOffsetMs = static_cast<uint32_t>(
				Cue.Find("stageOffsetMs")->Get_Number());
			// 기존 표시용 위치에도 stage 시각을 복사한다.
			// 이 값은 clip 원본 내부 시각이 아니며 실제 구분은 bUsesStageClock이 유지한다.
			View.iSourceStartMs = View.iStageOffsetMs;
			const auto* end = Cue.Find("stageEndMs");
			View.bHasSourceEnd = end && !end->Is_Null();
			View.iSourceEndMs = View.bHasSourceEnd ? static_cast<uint32_t>(end->Get_Number()) : 0u;
		}
		else
		{
			View.iSourceStartMs = static_cast<uint32_t>(
				Cue.Find("sourceStartMs")->Get_Number());
			View.bHasSourceEnd = !Cue.Find("sourceEndMs")->Is_Null();
			View.iSourceEndMs = View.bHasSourceEnd ? static_cast<uint32_t>(
				Cue.Find("sourceEndMs")->Get_Number()) : 0u;
		}
		View.bHasPlaybackOffset = nullptr != Cue.Find("playbackOffsetMs");
		View.iPlaybackOffsetMs = View.bHasPlaybackOffset ?
			static_cast<uint32_t>(Cue.Find("playbackOffsetMs")->Get_Number()) : 0u;
		View.iStageDurationMs = iStageDurationMs;
		const DATA_JSON_VALUE& Transform = *Cue.Find("localTransform");
		const DATA_JSON_VALUE::ARRAY& Position =
			Transform.Find("position")->Get_Array();
		const DATA_JSON_VALUE::ARRAY& Rotation =
			Transform.Find("rotationDegrees")->Get_Array();
		const DATA_JSON_VALUE::ARRAY& Scale =
			Transform.Find("scale")->Get_Array();
		View.LocalTransform.vPosition = {
			static_cast<f32_t>(Position[0].Get_Number()),
			static_cast<f32_t>(Position[1].Get_Number()),
			static_cast<f32_t>(Position[2].Get_Number()) };
		View.LocalTransform.vRotationDegrees = {
			static_cast<f32_t>(Rotation[0].Get_Number()),
			static_cast<f32_t>(Rotation[1].Get_Number()),
			static_cast<f32_t>(Rotation[2].Get_Number()) };
		View.LocalTransform.vScale = {
			static_cast<f32_t>(Scale[0].Get_Number()),
			static_cast<f32_t>(Scale[1].Get_Number()),
			static_cast<f32_t>(Scale[2].Get_Number()) };
		return View;
	}

	// 분리 gameplay hit를 호환 Master의 평탄한 필드로 투영한다. anchor와 activation, schedule 및 잡기 정책을 보존한다.
	// OutStage는 임시 JSON이며 검사 중 일부 필드가 추가될 수 있다. 최종 stage 범위 검증은 Master reader가 이어서 맡는다.
	bool_t Build_SplitHitProjection(
		const DATA_JSON_VALUE& Hit,
		DATA_JSON_VALUE::OBJECT& OutStage,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pShape = Required(
			Hit, "shape", DATA_JSON_TYPE::OBJECT);
		const std::string strKind = nullptr == pShape ? std::string{} :
			Read_String(*pShape, "kind");
		if (nullptr == pShape || strKind.empty())
		{
			strOutError = "split gameplay hit shape is invalid";
			return false;
		}
		auto PutNumber = [&OutStage](const std::string& Name, const double Value)
		{
			OutStage.emplace(Name, DATA_JSON_VALUE::Number(Value, true));
		};
		OutStage.emplace("hitShape", DATA_JSON_VALUE::String(strKind));
		for (const std::string& Name :
			{ "hitOuterRadius", "hitInnerRadius", "hitAngleDegrees",
			  "hitLength", "hitHalfWidth" })
		{
			PutNumber(Name, 0.0);
		}

		const auto RequireShape = [pShape, &strOutError](
			const std::initializer_list<std::string_view> Fields)
		{
			if (!Has_ExactProperties(*pShape, Fields))
			{
				strOutError = "split gameplay hit shape has unexpected properties";
				return false;
			}
			return true;
		};
		const auto ReadShapeNumber = [pShape, &OutStage, &strOutError](
			const std::string_view Source, const std::string& Target)
		{
			const DATA_JSON_VALUE* pValue = pShape->Find(Source);
			if (!Is_FiniteNumber(pValue) || pValue->Get_Number() < 0.0)
			{
				strOutError = "split gameplay hit shape numeric value is invalid";
				return false;
			}
			OutStage[Target] = *pValue;
			return true;
		};
		if ("NONE" == strKind)
		{
			if (!RequireShape({ "kind" }) ||
				!Has_ExactProperties(Hit, { "shape" }))
				return false;
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
			OutStage.emplace("serverDamageProfileId", DATA_JSON_VALUE::String(""));
			PutNumber("pushRangeM", 0.0);
			OutStage.emplace("pushMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("knockdown", DATA_JSON_VALUE::Boolean(false));
			OutStage.emplace("downMs", DATA_JSON_VALUE::Number(0));
			return true;
		}

		const DATA_JSON_VALUE* pPlayerResponse = Hit.Find("playerResponse");
		const DATA_JSON_VALUE* pAttachmentSlot = Hit.Find("attachmentSlot");
		const DATA_JSON_VALUE* pGripLocalOffset = Hit.Find("gripLocalOffset");
		const bool_t bCaptureHit = nullptr != pPlayerResponse ||
			nullptr != pAttachmentSlot || nullptr != pGripLocalOffset;
		const DATA_JSON_VALUE* pSchedule = Hit.Find("schedule");
		const DATA_JSON_VALUE* pActivation = Hit.Find("activation");
		const DATA_JSON_VALUE* pAnchor = Hit.Find("anchor");
		if (!Has_ExactPropertiesWithOptional(Hit,
				{ "shape", "serverDamageProfileId", "pushRangeM", "pushMs",
				  "knockdown", "downMs" },
				{ "schedule", "activation", "anchor", "playerResponse",
				  "attachmentSlot", "gripLocalOffset", "contacts" }) ||
			((nullptr == pSchedule) == (nullptr == pActivation)) ||
			((nullptr == pPlayerResponse) != (nullptr == pAttachmentSlot)) ||
			((nullptr == pPlayerResponse) != (nullptr == pGripLocalOffset)))
		{
			strOutError = "split gameplay hit has unexpected properties";
			return false;
		}
		if (const auto* contacts = Hit.Find("contacts"))
			OutStage.emplace("attackContacts", *contacts);
		if ("CIRCLE" == strKind)
		{
			if (!RequireShape({ "kind", "outerRadiusM" }) ||
				!ReadShapeNumber("outerRadiusM", "hitOuterRadius"))
				return false;
		}
		else if ("RING" == strKind)
		{
			if (!RequireShape({ "kind", "innerRadiusM", "outerRadiusM" }) ||
				!ReadShapeNumber("innerRadiusM", "hitInnerRadius") ||
				!ReadShapeNumber("outerRadiusM", "hitOuterRadius"))
				return false;
		}
		else if ("CONE" == strKind)
		{
			if (!RequireShape({ "kind", "angleDegrees", "lengthM" }) ||
				!ReadShapeNumber("angleDegrees", "hitAngleDegrees") ||
				!ReadShapeNumber("lengthM", "hitLength"))
				return false;
		}
		else if ("BOX" == strKind || "CROSS" == strKind ||
			"SIX_DIRECTIONS" == strKind)
		{
			if (!RequireShape({ "kind", "lengthM", "halfWidthM" }) ||
				!ReadShapeNumber("lengthM", "hitLength") ||
				!ReadShapeNumber("halfWidthM", "hitHalfWidth"))
				return false;
		}
		else
		{
			strOutError = "split gameplay hit shape kind is unsupported";
			return false;
		}

		if (nullptr != pAnchor)
		{
			f32_t fForward = 0.f;
			f32_t fRight = 0.f;
			f32_t fYaw = 0.f;
			const std::string strAnchorKind = Read_String(*pAnchor, "kind");
			if (!Has_ExactProperties(*pAnchor,
					{ "kind", "forwardOffsetM", "rightOffsetM",
					  "yawOffsetDegrees" }) ||
				("BOSS_CURRENT" != strAnchorKind &&
				 "STAGE_ORIGIN" != strAnchorKind) ||
				!Read_RequiredFiniteFloat(*pAnchor, "forwardOffsetM", fForward) ||
				!Read_RequiredFiniteFloat(*pAnchor, "rightOffsetM", fRight) ||
				!Read_RequiredFiniteFloat(*pAnchor, "yawOffsetDegrees", fYaw) ||
				std::fabs(fForward) > 1000.f || std::fabs(fRight) > 1000.f ||
				std::fabs(fYaw) > 360.f)
			{
				strOutError = "split gameplay hit anchor is invalid";
				return false;
			}
			OutStage.emplace("hitAnchor", *pAnchor);
		}
		if (nullptr != pActivation)
		{
			if (!Has_ExactProperties(*pActivation,
					{ "kind", "startMs", "lifetimeMs", "perTargetPolicy" }) ||
				"ACTIVE_WINDOW" != Read_String(*pActivation, "kind") ||
				"ONCE" != Read_String(*pActivation, "perTargetPolicy") ||
				!Is_NonNegativeInteger(pActivation->Find("startMs")) ||
				!Is_NonNegativeInteger(pActivation->Find("lifetimeMs")) ||
				0.0 == pActivation->Find("lifetimeMs")->Get_Number())
			{
				strOutError = "split gameplay hit activation is invalid";
				return false;
			}
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
			OutStage.emplace("hitActivation", *pActivation);
		}
		const std::string strSchedule = nullptr == pSchedule ? std::string{} :
			Read_String(*pSchedule, "kind");
		if (nullptr != pSchedule && "INTERVAL" == strSchedule && Has_ExactProperties(*pSchedule,
				{ "kind", "count", "firstOffsetMs", "intervalMs" }) &&
			Is_NonNegativeInteger(pSchedule->Find("count")) &&
			Is_NonNegativeInteger(pSchedule->Find("firstOffsetMs")) &&
			Is_NonNegativeInteger(pSchedule->Find("intervalMs")))
		{
			OutStage.emplace("hitCount", *pSchedule->Find("count"));
			OutStage.emplace("hitIntervalMs", *pSchedule->Find("intervalMs"));
			OutStage.emplace("hitDelayMs", *pSchedule->Find("firstOffsetMs"));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
		}
		else if (nullptr != pSchedule && "EXPLICIT_OFFSETS" == strSchedule &&
			Has_ExactProperties(*pSchedule, { "kind", "offsetsMs" }))
		{
			const DATA_JSON_VALUE* pOffsets = Required(
				*pSchedule, "offsetsMs", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pOffsets || pOffsets->Get_Array().empty())
			{
				strOutError = "split gameplay explicit hit schedule is invalid";
				return false;
			}
			uint32_t iPrevious = 0u;
			for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
			{
				const DATA_JSON_VALUE& Offset = pOffsets->Get_Array()[i];
				if (!Is_NonNegativeInteger(&Offset) ||
					(0u != i && Offset.Get_Number() <= iPrevious))
				{
					strOutError = "split gameplay explicit hit offsets are invalid";
					return false;
				}
				iPrevious = static_cast<uint32_t>(Offset.Get_Number());
			}
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(
				static_cast<double>(pOffsets->Get_Array().size())));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", *pOffsets);
		}
		else if (nullptr != pSchedule)
		{
			strOutError = "split gameplay hit schedule is invalid";
			return false;
		}

		const DATA_JSON_VALUE* pDamage = Required(
			Hit, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pKnockdown = Required(
			Hit, "knockdown", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pDamage || !Is_StableToken(pDamage->Get_String()) ||
			nullptr == pKnockdown || !Is_FiniteNumber(Hit.Find("pushRangeM")) ||
			!Is_NonNegativeInteger(Hit.Find("pushMs")) ||
			!Is_NonNegativeInteger(Hit.Find("downMs")))
		{
			strOutError = "split gameplay hit payload is invalid";
			return false;
		}
		if (bCaptureHit &&
			("CAPTURE" != Read_String(Hit, "playerResponse") ||
			 "BOSS_LEFT_HAND" != Read_String(Hit, "attachmentSlot") ||
			 0.0 != Hit.Find("pushRangeM")->Get_Number() ||
			 0.0 != Hit.Find("pushMs")->Get_Number() ||
			 pKnockdown->Get_Boolean() ||
			 0.0 != Hit.Find("downMs")->Get_Number()))
		{
			strOutError = "split gameplay capture hit contract is invalid";
			return false;
		}
		std::optional<Client::PLAYER_HAND_GRIP_LOCAL_OFFSET> GripLocalOffset;
		if (!Read_PlayerHandGripLocalOffset(
				pGripLocalOffset, GripLocalOffset) ||
			(bCaptureHit != GripLocalOffset.has_value()))
		{
			strOutError = "split gameplay gripLocalOffset is invalid";
			return false;
		}
		OutStage.emplace("serverDamageProfileId", *pDamage);
		if (bCaptureHit)
		{
			OutStage.emplace("playerResponse", *Hit.Find("playerResponse"));
			OutStage.emplace("attachmentSlot", *Hit.Find("attachmentSlot"));
			OutStage.emplace("gripLocalOffset", *pGripLocalOffset);
		}
		OutStage.emplace("pushRangeM", *Hit.Find("pushRangeM"));
		OutStage.emplace("pushMs", *Hit.Find("pushMs"));
		OutStage.emplace("knockdown", *pKnockdown);
		OutStage.emplace("downMs", *Hit.Find("downMs"));
		return true;
	}

	// decision을 선택 집합, 체력 window, 기믹과 수동 시험 목록으로 나눈다.
	// window는 phase별 이어지는 체력 범위를 요구하고 각 선택 집합은 한 window에 연결한다.
	// 순서 반복의 PatternIds는 중복을 보존하지만 coverage용 NormalPatternIds는 중복 없이 만든다.
	// OutSelectionSets/Windows·기믹·수동 목록과 색인은 처리 중 추가된다. 실패 시 이 부분 출력은 남는다.
	// OutNormalSelection과 OutScriptedSequence는 마지막까지 임시로 준비해 false 반환 경로에서는 기존 값을 유지한다.
	// 이는 할당 예외까지 전체 rollback한다는 보장이 아니며 호출자는 상위 임시 문서 안에서 사용한다.
	bool_t Build_SplitDecisionProjection(
		const DATA_JSON_VALUE& Decision,
		DATA_JSON_VALUE& OutNormalSelection,
		MASTER_SCRIPTED_SEQUENCE_VIEW& OutScriptedSequence,
		std::vector<Client::VALTAN_SELECTION_SET_VIEW>& OutSelectionSets,
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW>& OutSelectionWindows,
		std::vector<Client::VALTAN_MECHANIC_VIEW>& OutMechanicViews,
		std::map<std::string, SPLIT_MECHANIC, std::less<>>& OutMechanics,
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW>& OutManualViews,
		std::map<std::string, Client::VALTAN_MANUAL_AUDITION_VIEW,
			std::less<>>& OutManualAuditions,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Decision,
				{ "scriptedSequence", "selectionSets", "selectionWindows",
				  "mechanics", "manualAuditions" }))
		{
			strOutError = "split gameplay decisionModel has unexpected properties";
			return false;
		}

		// 선택 모델의 다섯 구성 요소를 찾고 scripted sequence부터 읽는다.
		// 명시된 패턴 순서와 전환 추적 시간은 아래 선택 집합의 가중치와 별개다.
		const DATA_JSON_VALUE* pScriptedSequence = Required(
			Decision, "scriptedSequence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pSets = Required(
			Decision, "selectionSets", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pWindows = Required(
			Decision, "selectionWindows", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pMechanics = Required(
			Decision, "mechanics", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pManualAuditions = Required(
			Decision, "manualAuditions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pScriptedPatterns = nullptr == pScriptedSequence ?
			nullptr : Required(*pScriptedSequence,
				"patternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pScriptedTransitionPursuit =
			nullptr == pScriptedSequence ? nullptr :
				pScriptedSequence->Find("transitionPursuitMs");
		if (nullptr == pScriptedSequence ||
			!Has_ExactPropertiesWithOptional(*pScriptedSequence,
				{ "sequenceId", "mode", "interStepPursuitMs", "patternIds" },
				{ "transitionPursuitMs", "entranceCinematicPatternId" }) ||
			(nullptr != pScriptedSequence->Find("entranceCinematicPatternId") &&
			 (!pScriptedSequence->Find("entranceCinematicPatternId")->Is_String() ||
			  !Is_StableToken(Read_String(*pScriptedSequence, "entranceCinematicPatternId")))) ||
			!Is_StableToken(Read_String(*pScriptedSequence, "sequenceId")) ||
			("ORDERED_ONCE_THEN_IDLE" != Read_String(*pScriptedSequence, "mode") &&
			 "HEALTH_BAR_ROTATIONS" != Read_String(*pScriptedSequence, "mode")) ||
			!Is_NonNegativeInteger(
				pScriptedSequence->Find("interStepPursuitMs")) ||
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number() < 100.0 ||
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number() > 10000.0 ||
			nullptr == pScriptedPatterns ||
			pScriptedPatterns->Get_Array().empty() ||
			pScriptedPatterns->Get_Array().size() >
				Client::CValtanPatternFlowDocument::MAX_SLOTS)
		{
			strOutError = "split gameplay scriptedSequence is invalid";
			return false;
		}

		// sequence는 아직 지역 값으로 준비한다. patternIds의 순서와 반복 ID를 그대로 보관하며
		// ID가 실제 정의를 가리키는지는 후속 패턴 결합에서 확인한다.
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		ScriptedSequence.strSequenceId = Read_String(
			*pScriptedSequence, "sequenceId");
		ScriptedSequence.strMode = Read_String(*pScriptedSequence, "mode");
		ScriptedSequence.strEntranceCinematicPatternId = Read_String(
			*pScriptedSequence, "entranceCinematicPatternId");
		ScriptedSequence.iInterStepPursuitMs = static_cast<uint32_t>(
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number());
		for (const DATA_JSON_VALUE& PatternId : pScriptedPatterns->Get_Array())
		{
			if (DATA_JSON_TYPE::STRING != PatternId.Get_Type() ||
				!Is_StableToken(PatternId.Get_String()))
			{
				strOutError =
					"split gameplay scriptedSequence pattern is invalid";
				return false;
			}
			ScriptedSequence.PatternIds.push_back(PatternId.Get_String());
		}

		// transitionPursuitMs가 없으면 공통 추적 시간을 패턴 사이마다 복제한다.
		// 명시하면 패턴 수보다 하나 적은 ms 배열이어야 하고 각 값은 0~10000ms를 허용한다.
		if (nullptr == pScriptedTransitionPursuit)
		{
			ScriptedSequence.TransitionPursuitMs.assign(
				ScriptedSequence.PatternIds.size() - 1u,
				ScriptedSequence.iInterStepPursuitMs);
		}
		else
		{
			if (DATA_JSON_TYPE::ARRAY != pScriptedTransitionPursuit->Get_Type() ||
				pScriptedTransitionPursuit->Get_Array().size() + 1u !=
					ScriptedSequence.PatternIds.size())
			{
				strOutError =
					"split gameplay scriptedSequence transition pursuits are invalid";
				return false;
			}
			for (const DATA_JSON_VALUE& PursuitMs :
				pScriptedTransitionPursuit->Get_Array())
			{
				if (!Is_NonNegativeInteger(&PursuitMs) ||
					PursuitMs.Get_Number() > 10000.0)
				{
					strOutError =
						"split gameplay scriptedSequence transition pursuit is invalid";
					return false;
				}
				ScriptedSequence.TransitionPursuitMs.push_back(
					static_cast<uint32_t>(PursuitMs.Get_Number()));
			}
		}
		if (nullptr == pSets || pSets->Get_Array().empty() ||
			nullptr == pWindows || pWindows->Get_Array().empty() ||
			nullptr == pMechanics || nullptr == pManualAuditions)
		{
			strOutError = "split gameplay decisionModel inventories are invalid";
			return false;
		}

		// selectionSetId별 전체 구성원 목록을 준비한다.
		// ORDERED_LOOP는 같은 ID의 재등장과 원본 순서를 보존하고 WEIGHTED_POOL은 ID 중복을 거부한다.
		std::map<std::string, std::vector<std::string>, std::less<>>
			PatternsBySet;
		for (const DATA_JSON_VALUE& Set : pSets->Get_Array())
		{
			if ("ORDERED_LOOP" == Read_String(Set, "mode"))
			{
				const std::string strSetId = Read_String(Set, "selectionSetId");
				const auto* pMembers = Required(Set, "patternIds", DATA_JSON_TYPE::ARRAY);
				if (!Has_ExactProperties(Set, {"selectionSetId", "mode", "patternIds"}) ||
					!Is_StableToken(strSetId) || PatternsBySet.contains(strSetId) ||
					nullptr == pMembers || pMembers->Get_Array().empty() || pMembers->Get_Array().size() > 32u)
				{
					strOutError = "split gameplay ordered loop is invalid";
					return false;
				}
				Client::VALTAN_SELECTION_SET_VIEW SetView;
				SetView.strSelectionSetId = strSetId;
				SetView.strMode = "ORDERED_LOOP";
				for (const auto& Member : pMembers->Get_Array())
				{
					if (!Member.Is_String() || !Is_StableToken(Member.Get_String()))
					{
						strOutError = "split gameplay ordered loop occurrence is invalid";
						return false;
					}
					SetView.PatternIds.push_back(Member.Get_String());
				}
				PatternsBySet.emplace(strSetId, SetView.PatternIds);
				OutSelectionSets.push_back(std::move(SetView));
				continue;
			}

			// 가중치 집합은 각 후보의 양수 weight와 enabled 상태를 보관한다.
			// 여기서는 선택을 실행하지 않으며 최소 하나의 활성 후보가 있어야 한다.
			if (!Has_ExactProperties(Set,
					{ "selectionSetId", "mode", "candidates" }) ||
				"WEIGHTED_POOL" != Read_String(Set, "mode"))
			{
				strOutError = "split gameplay selection set is invalid";
				return false;
			}
			const std::string strSetId = Read_String(Set, "selectionSetId");
			const DATA_JSON_VALUE* pCandidates = Required(
				Set, "candidates", DATA_JSON_TYPE::ARRAY);
			if (!Is_StableToken(strSetId) || nullptr == pCandidates ||
				pCandidates->Get_Array().empty() ||
				PatternsBySet.contains(strSetId))
			{
				strOutError = "split gameplay selection set identity is invalid or duplicated";
				return false;
			}
			std::vector<std::string> PatternIds;
			std::set<std::string, std::less<>> UniquePatterns;
			Client::VALTAN_SELECTION_SET_VIEW SetView;
			SetView.strSelectionSetId = strSetId;
			SetView.strMode = Read_String(Set, "mode");
			uint32_t iEnabledCount = 0u;
			for (const DATA_JSON_VALUE& Candidate : pCandidates->Get_Array())
			{
				const DATA_JSON_VALUE* pEnabled = Required(
					Candidate, "enabled", DATA_JSON_TYPE::BOOLEAN);
				const std::string strPatternId = Read_String(
					Candidate, "patternId");
				if (!Has_ExactProperties(Candidate,
						{ "patternId", "weight", "enabled" }) ||
					!Is_StableToken(strPatternId) ||
					!Is_NonNegativeInteger(Candidate.Find("weight")) ||
					0.0 == Candidate.Find("weight")->Get_Number() ||
					Candidate.Find("weight")->Get_Number() > 100000.0 ||
					nullptr == pEnabled ||
					!UniquePatterns.insert(strPatternId).second)
				{
					strOutError = "split gameplay selection candidate is invalid or duplicated";
					return false;
				}
				const uint32_t iWeight = static_cast<uint32_t>(
					Candidate.Find("weight")->Get_Number());
				Client::VALTAN_SELECTION_CANDIDATE_VIEW CandidateView;
				CandidateView.strPatternId = strPatternId;
				CandidateView.iWeight = iWeight;
				CandidateView.bEnabled = pEnabled->Get_Boolean();
				if (CandidateView.bEnabled)
					++iEnabledCount;

				// 비활성 후보도 구성원 목록에 남겨 전체 패턴 coverage에 포함한다.
				// enabled는 실행 후보 여부이고 정의 소유권 검사에서 행을 제외하는 스위치가 아니다.
				SetView.Candidates.push_back(std::move(CandidateView));
				PatternIds.push_back(strPatternId);
			}
			if (0u == iEnabledCount)
			{
				strOutError = "split gameplay weighted selection set has no enabled candidate";
				return false;
			}
			PatternsBySet.emplace(strSetId, std::move(PatternIds));
			OutSelectionSets.push_back(std::move(SetView));
		}

		// window를 gameplay phase별 체력 구간으로 읽는다. maximum은 포함하고 minimum은 제외하며
		// 같은 phase의 다음 maximum은 앞서 읽은 minimum과 같아야 한다. 서로 다른 phase끼리는 이 경계를 비교하지 않는다.
		DATA_JSON_VALUE::ARRAY Ranges;
		std::vector<std::string> NormalPatternIds;
		std::set<std::string, std::less<>> NormalPatternSet;
		std::set<std::string, std::less<>> WindowIds;
		std::set<std::string, std::less<>> RotationIds;
		std::set<std::string, std::less<>> WindowSetIds;
		std::map<uint32_t, uint32_t> PreviousMinimumByPhase;
		for (const DATA_JSON_VALUE& Window : pWindows->Get_Array())
		{
			if (!Has_ExactProperties(Window,
					{ "windowId", "gameplayPhase", "maximumHealthBarInclusive",
					  "minimumHealthBarExclusive", "selectionSetId",
					  "compatibilityRotationId" }) ||
				!Is_StableToken(Read_String(Window, "windowId")) ||
				!Is_StableToken(Read_String(Window, "compatibilityRotationId")) ||
				!Is_NonNegativeInteger(Window.Find("gameplayPhase")) ||
				0.0 == Window.Find("gameplayPhase")->Get_Number() ||
				Window.Find("gameplayPhase")->Get_Number() > 3.0 ||
				!Is_NonNegativeInteger(
					Window.Find("maximumHealthBarInclusive")) ||
				!Is_NonNegativeInteger(
					Window.Find("minimumHealthBarExclusive")))
			{
				strOutError = "split gameplay selection window is invalid";
				return false;
			}
			const std::string strWindowId = Read_String(Window, "windowId");
			const std::string strRotationId = Read_String(
				Window, "compatibilityRotationId");
			const std::string strSetId = Read_String(Window, "selectionSetId");
			const auto Patterns = PatternsBySet.find(strSetId);
			const uint32_t iPhase = static_cast<uint32_t>(
				Window.Find("gameplayPhase")->Get_Number());
			const uint32_t iMaximum = static_cast<uint32_t>(
				Window.Find("maximumHealthBarInclusive")->Get_Number());
			const uint32_t iMinimum = static_cast<uint32_t>(
				Window.Find("minimumHealthBarExclusive")->Get_Number());

			// 해당 phase의 직전 경계와 ID 중복을 확인한다. 하나의 selection set은
			// 한 window에만 연결하고 compatibilityRotationId도 전체 window에서 유일해야 한다.
			const auto Previous = PreviousMinimumByPhase.find(iPhase);
			if (Patterns == PatternsBySet.end() || iMaximum <= iMinimum ||
				(Previous != PreviousMinimumByPhase.end() &&
				 Previous->second != iMaximum) ||
				!WindowIds.insert(strWindowId).second ||
				!RotationIds.insert(strRotationId).second ||
				!WindowSetIds.insert(strSetId).second)
			{
				strOutError = "split gameplay selection window order/closure is invalid";
				return false;
			}
			PreviousMinimumByPhase[iPhase] = iMinimum;

			// window가 연결한 전체 구성원을 처음 나타난 순서로 모아 중복 없는 호환 목록을 만든다.
			// 이 목록과 별개로 각 selection set의 반복/가중치 메타데이터는 그대로 보존한다.
			for (const std::string& strPatternId : Patterns->second)
			{
				if (NormalPatternSet.insert(strPatternId).second)
					NormalPatternIds.push_back(strPatternId);
			}
			Client::VALTAN_SELECTION_WINDOW_VIEW WindowView;
			WindowView.strWindowId = strWindowId;
			WindowView.iGameplayPhase = iPhase;
			WindowView.iMaximumHealthBarInclusive = iMaximum;
			WindowView.iMinimumHealthBarExclusive = iMinimum;
			WindowView.strSelectionSetId = strSetId;
			WindowView.strCompatibilityRotationId = strRotationId;
			OutSelectionWindows.push_back(std::move(WindowView));
			DATA_JSON_VALUE::OBJECT Range;
			Range.emplace("rotationId", DATA_JSON_VALUE::String(
				strRotationId));
			Range.emplace("fromHealthBar",
				*Window.Find("maximumHealthBarInclusive"));
			Range.emplace("toHealthBar",
				*Window.Find("minimumHealthBarExclusive"));
			Ranges.push_back(DATA_JSON_VALUE::Object(std::move(Range)));
		}

		// 존재하는 모든 set이 정확히 한 window에 연결됐는지 개수를 대조한다.
		// 앞선 존재/중복 검사와 함께 적용되며 사용되지 않은 set을 조용히 버리지 않는다.
		if (WindowSetIds.size() != PatternsBySet.size())
		{
			strOutError = "split gameplay selection windows do not cover every set";
			return false;
		}

		// 체력 기믹은 한 encounter에 한 번, 실패 시 reset 요구 정책만 허용한다.
		// 기믹 ID·체력/순서 쌍·소유 pattern을 중복 없이 저장하고 원본 정렬도 검사한다.
		std::set<std::string, std::less<>> MechanicIds;
		std::set<std::pair<uint32_t, uint32_t>> MechanicTriggers;
		// 체력은 내림차순, 같은 체력의 triggerOrder는 오름차순으로 기믹 순서를 검증한다.
		uint32_t iPreviousHealthBar = (std::numeric_limits<uint32_t>::max)();
		uint32_t iPreviousOrder = 0u;
		for (const DATA_JSON_VALUE& Mechanic : pMechanics->Get_Array())
		{
			if (!Has_ExactProperties(Mechanic,
					{ "mechanicId", "patternId", "trigger", "triggerOrder",
					  "oncePerEncounter", "failurePolicy" }) ||
				!Is_StableToken(Read_String(Mechanic, "mechanicId")) ||
				!Is_StableToken(Read_String(Mechanic, "patternId")) ||
				!Is_NonNegativeInteger(Mechanic.Find("triggerOrder")) ||
				nullptr == Required(
					Mechanic, "oncePerEncounter", DATA_JSON_TYPE::BOOLEAN) ||
				!Required(Mechanic, "oncePerEncounter",
					DATA_JSON_TYPE::BOOLEAN)->Get_Boolean() ||
				"ABORT_ENCOUNTER_REQUIRE_RESET" !=
					Read_String(Mechanic, "failurePolicy"))
			{
				strOutError = "split gameplay mechanic is invalid";
				return false;
			}
			const DATA_JSON_VALUE* pTrigger = Required(
				Mechanic, "trigger", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pTrigger || !Has_ExactProperties(
					*pTrigger, { "kind", "healthBar" }) ||
				"HEALTH_BAR_CROSSING" != Read_String(*pTrigger, "kind") ||
				!Is_NonNegativeInteger(pTrigger->Find("healthBar")) ||
				0.0 == pTrigger->Find("healthBar")->Get_Number() ||
				0.0 == Mechanic.Find("triggerOrder")->Get_Number())
			{
				strOutError = "split gameplay mechanic trigger is invalid";
				return false;
			}
			SPLIT_MECHANIC Projection;
			Projection.iHealthBar = static_cast<uint32_t>(
				pTrigger->Find("healthBar")->Get_Number());
			Projection.iTriggerOrder = static_cast<uint32_t>(
				Mechanic.Find("triggerOrder")->Get_Number());
			const std::string strMechanicId = Read_String(
				Mechanic, "mechanicId");
			const std::string strPatternId = Read_String(
				Mechanic, "patternId");
			if (!MechanicIds.insert(strMechanicId).second ||
				!MechanicTriggers.insert({ Projection.iHealthBar,
					Projection.iTriggerOrder }).second ||
				Projection.iHealthBar > iPreviousHealthBar ||
				(Projection.iHealthBar == iPreviousHealthBar &&
				 Projection.iTriggerOrder <= iPreviousOrder) ||
				!OutMechanics.emplace(strPatternId, Projection).second)
			{
				strOutError = "split gameplay mechanic identity/order is invalid";
				return false;
			}
			iPreviousHealthBar = Projection.iHealthBar;
			iPreviousOrder = Projection.iTriggerOrder;
			Client::VALTAN_MECHANIC_VIEW MechanicView;
			MechanicView.strMechanicId = strMechanicId;
			MechanicView.strPatternId = strPatternId;
			MechanicView.strTriggerKind = Read_String(*pTrigger, "kind");
			MechanicView.iHealthBar = Projection.iHealthBar;
			MechanicView.iTriggerOrder = Projection.iTriggerOrder;
			MechanicView.bOncePerEncounter = true;
			MechanicView.strFailurePolicy = Read_String(
				Mechanic, "failurePolicy");
			OutMechanicViews.push_back(std::move(MechanicView));
		}

		// 수동/파생 시험 패턴은 NORMAL 선택 집합과 기믹의 소유 pattern과 겹칠 수 없다.
		// pattern ID와 source chain ID도 각각 유일해야 하며 조회 map과 표시 vector를 함께 채운다.
		std::set<std::string, std::less<>> ManualSourceChainIds;
		for (const DATA_JSON_VALUE& Manual : pManualAuditions->Get_Array())
		{
			if (!Has_ExactProperties(Manual,
					{ "patternId", "sourceChainId", "authoringPhase",
					  "admissionState" }) ||
				!Is_StableToken(Read_String(Manual, "patternId")) ||
				!Is_StableToken(Read_String(Manual, "sourceChainId")) ||
				!Is_NonNegativeInteger(Manual.Find("authoringPhase")) ||
				0.0 == Manual.Find("authoringPhase")->Get_Number() ||
				Manual.Find("authoringPhase")->Get_Number() > 3.0 ||
				("MANUAL_SERVER_AUDITION" != Read_String(Manual, "admissionState") &&
				 "DERIVED_SERVER_PATTERN" != Read_String(Manual, "admissionState")))
			{
				strOutError = "split gameplay manual audition is invalid";
				return false;
			}
			Client::VALTAN_MANUAL_AUDITION_VIEW View;
			View.strPatternId = Read_String(Manual, "patternId");
			View.strSourceChainId = Read_String(Manual, "sourceChainId");
			View.iAuthoringPhase = static_cast<uint32_t>(
				Manual.Find("authoringPhase")->Get_Number());
			View.strAdmissionState = Read_String(Manual, "admissionState");
			if (NormalPatternSet.contains(View.strPatternId) ||
				OutMechanics.contains(View.strPatternId) ||
				!ManualSourceChainIds.insert(View.strSourceChainId).second ||
				!OutManualAuditions.emplace(
					View.strPatternId, View).second)
			{
				strOutError =
					"split gameplay manual audition ownership is duplicated or overlaps";
				return false;
			}
			OutManualViews.push_back(std::move(View));
		}
		const auto ArenaBreak = OutMechanics.find("VALTAN_ARENA_BREAK_109");
		// 첫 gameplay phase의 마지막 window 경계가 VALTAN_ARENA_BREAK_109 기믹 발동 체력과 맞아야 한다.
		// 기믹 ID에 들어간 숫자를 이 검사에서 고정 체력 값으로 대신 사용하지 않는다.
		uint32_t iPhaseOneBoundary = 0u;
		for (const Client::VALTAN_SELECTION_WINDOW_VIEW& Window :
			OutSelectionWindows)
		{
			if (1u == Window.iGameplayPhase)
				iPhaseOneBoundary = Window.iMinimumHealthBarExclusive;
		}
		if (0u == iPhaseOneBoundary || ArenaBreak == OutMechanics.end() ||
			ArenaBreak->second.iHealthBar != iPhaseOneBoundary)
		{
			strOutError =
				"VALTAN_ARENA_BREAK_109 health bar must match the final phase-1 window boundary";
			return false;
		}

		// 모든 검사 뒤에만 호환 normalSelection과 scripted sequence를 최종 출력에 옮긴다.
		// 다른 Out 목록/색인은 앞에서 이미 채워졌으므로 이 두 대입을 전체 출력의 일괄 commit으로 해석하지 않는다.
		DATA_JSON_VALUE::OBJECT Normal;
		Normal.emplace("selectionMode",
			DATA_JSON_VALUE::String("HEALTH_BAR_ROTATIONS" == ScriptedSequence.strMode ? "ORDERED_LOOP" : "WEIGHTED_POOL"));
		Normal.emplace("ranges", DATA_JSON_VALUE::Array(std::move(Ranges)));
		Normal.emplace("patternIds", Build_StringArray(NormalPatternIds));
		OutNormalSelection = DATA_JSON_VALUE::Object(std::move(Normal));
		OutScriptedSequence = std::move(ScriptedSequence);
		return true;
	}

	// camera invocation의 stable ID와 trigger, stage 내부 시작/길이 ms를 확인한다.
	bool_t Validate_SplitCameraInvocation(
		const DATA_JSON_VALUE& Invocation,
		const uint32_t iStageDurationMs,
		std::string& strOutCameraCueId,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Invocation,
				{ "cameraInvocationId", "cameraCueId", "trigger",
				  "startOffsetMs", "durationPolicy", "durationMs" }) ||
			!Is_StableToken(Read_String(
				Invocation, "cameraInvocationId")) ||
			!Is_StableToken(Read_String(Invocation, "cameraCueId")) ||
			"ENTER" != Read_String(Invocation, "trigger") ||
			"EXPLICIT" != Read_String(Invocation, "durationPolicy") ||
			!Is_NonNegativeInteger(Invocation.Find("startOffsetMs")) ||
			!Is_NonNegativeInteger(Invocation.Find("durationMs")))
		{
			strOutError = "split presentation camera invocation is invalid";
			return false;
		}
		const uint64_t iEnd = static_cast<uint64_t>(
			Invocation.Find("startOffsetMs")->Get_Number()) +
			static_cast<uint64_t>(Invocation.Find("durationMs")->Get_Number());
		if (iEnd > iStageDurationMs)
		{
			strOutError = "split presentation camera invocation leaves its Server stage";
			return false;
		}
		strOutCameraCueId = Read_String(Invocation, "cameraCueId");
		return true;
	}

	// 분리 event를 호환 action 또는 world event/spawn 메타데이터로 투영한다.
	// pOutAction은 호출자가 제공한 유효 출력 포인터이며 먼저 null로 설정한다. 실패 시 부분 출력이 남을 수 있다.
	bool_t Build_SplitEventProjection(
		const DATA_JSON_VALUE& Event,
		const std::string_view strPatternId,
		const std::string_view strStageId,
		const std::string_view strStageActionId,
		const bool_t bTerminalStage,
		const uint32_t iStageDurationMs,
		DATA_JSON_VALUE* pOutAction,
		std::string& strOutSpawnArchetypeId,
		bool_t& bOutWorldEvent,
		std::string& strOutError)
	{
		*pOutAction = DATA_JSON_VALUE::Null();
		strOutSpawnArchetypeId.clear();
		bOutWorldEvent = false;
		const std::string strEventId = Read_String(Event, "eventId");
		const std::string strTrigger = Read_String(Event, "trigger");
		const std::string strKind = Read_String(Event, "kind");
		if (!Is_StableToken(strEventId) || !Is_StableToken(strTrigger) ||
			!Is_StableToken(strKind))
		{
			strOutError = "split gameplay event identity is invalid";
			return false;
		}
		DATA_JSON_VALUE::OBJECT Action;
		if ("SET_BOSS_FLAG" == strKind)
		{
			const DATA_JSON_VALUE* pEnabled = Required(
				Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "flagId", "enabled" }) ||
				!Is_StableToken(Read_String(Event, "flagId")) ||
				nullptr == pEnabled)
			{
				strOutError = "split gameplay SET_BOSS_FLAG event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				Read_String(Event, "flagId")));
			Action.emplace("value", DATA_JSON_VALUE::Number(
				pEnabled->Get_Boolean() ? 1.0 : 0.0));
		}
		else if ("SET_STAGGER_GAUGE" == strKind)
		{
			const DATA_JSON_VALUE* pValue = Event.Find("value");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "value" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_NonNegativeInteger(pValue) ||
				pValue->Get_Number() > 100000.0 ||
				(("ENTER" == strTrigger) !=
				 (pValue->Get_Number() > 0.0)))
			{
				strOutError = "split gameplay SET_STAGGER_GAUGE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.gauge.stagger"));
			Action.emplace("value", *pValue);
		}
		else if ("SET_PLAYER_BIND" == strKind)
		{
			const DATA_JSON_VALUE* pHeightM = Event.Find("heightM");
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "heightM", "durationMs" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_FiniteNumber(pHeightM) ||
				!Is_NonNegativeInteger(pDurationMs) ||
				(("ENTER" == strTrigger &&
				  (5.0 != pHeightM->Get_Number() ||
				   pDurationMs->Get_Number() != iStageDurationMs ||
				   pDurationMs->Get_Number() < 100.0 ||
				   pDurationMs->Get_Number() > 120000.0)) ||
				 ("EXIT" == strTrigger &&
				  (0.0 != pHeightM->Get_Number() ||
				   0.0 != pDurationMs->Get_Number()))))
			{
				strOutError = "split gameplay SET_PLAYER_BIND event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("player.status.bind"));
			Action.emplace("value", DATA_JSON_VALUE::Number(
				pHeightM->Get_Number() * 1000.0));
			Action.emplace("durationMs", *pDurationMs);
		}
		else if ("SET_PLAYER_SILENCE" == strKind)
		{
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "durationMs" }) ||
				"ENTER" != strTrigger ||
				!Is_NonNegativeInteger(pDurationMs) ||
				pDurationMs->Get_Number() < iStageDurationMs ||
				pDurationMs->Get_Number() < 100.0 ||
				pDurationMs->Get_Number() > 120000.0)
			{
				strOutError = "split gameplay SET_PLAYER_SILENCE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("player.status.silence"));
			Action.emplace("value", DATA_JSON_VALUE::Number(1.0));
			Action.emplace("durationMs", *pDurationMs);
		}
		else if ("SET_GAMEPLAY_PHASE" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "gameplayPhase" }) ||
				!Is_NonNegativeInteger(Event.Find("gameplayPhase")))
			{
				strOutError = "split gameplay SET_GAMEPLAY_PHASE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.phase.gameplay"));
			Action.emplace("value", *Event.Find("gameplayPhase"));
		}
		else if ("TRIGGER_WORLD_EVENT_SET" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "worldEventSetId" }) ||
				!Is_StableToken(Read_String(Event, "worldEventSetId")))
			{
				strOutError = "split gameplay world event is invalid";
				return false;
			}
			bOutWorldEvent = true;
			return true;
		}
		else if ("SUPPRESS_INTER_STEP_PURSUIT" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind" }) ||
				"EXIT" != strTrigger ||
				"VALTAN_GHOST_DEATH_AUDITION" != strPatternId ||
				"STEP_01" != strStageId ||
				"valtan.sequence.dead.step-01" != strStageActionId ||
				!bTerminalStage)
			{
				strOutError =
					"split gameplay inter-step pursuit suppression must be the terminal ghost-death EXIT event";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				"boss.sequence.inter-step-pursuit"));
			Action.emplace("value", DATA_JSON_VALUE::Number(0));
		}
		else if ("RETARGET_RANDOM_ALIVE" == strKind ||
			"RETURN_TO_ARENA_CENTER" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind" }) ||
				"ENTER" != strTrigger)
			{
				strOutError = "split gameplay retarget/arena return event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				"RETURN_TO_ARENA_CENTER" == strKind ?
					"boss.arena.center" : "boss.target.pattern"));
			Action.emplace("value", DATA_JSON_VALUE::Number(1));
		}
		else if ("DAMAGE_GRABBED_PLAYERS" == strKind ||
			"EXECUTE_GRABBED_PLAYERS" == strKind)
		{
			const bool_t bDamage = "DAMAGE_GRABBED_PLAYERS" == strKind;
			const std::string strDamageProfileId = Read_String(Event, "damageProfileId");
			const bool_t bPropertiesValid = bDamage ?
				Has_ExactProperties(Event, { "eventId", "trigger", "kind", "damageProfileId" }) :
				Has_ExactProperties(Event, { "eventId", "trigger", "kind" });
			if (!bPropertiesValid || "ENTER" != strTrigger ||
				(bDamage && (!Is_StableToken(strDamageProfileId) ||
					!strDamageProfileId.starts_with("damage."))))
			{
				strOutError = "split gameplay grabbed-player terminal event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				bDamage ? strDamageProfileId : "boss.attachment.left-hand"));
			Action.emplace("value", DATA_JSON_VALUE::Number(0));
		}
		else if ("RELEASE_GRABBED_PLAYERS" == strKind)
		{
			const std::string strReleaseMode = Read_String(Event, "releaseMode");
			const DATA_JSON_VALUE* pSpeedMps = Event.Find("speedMps");
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			const DATA_JSON_VALUE* pYawOffsetDegrees =
				Event.Find("yawOffsetDegrees");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "releaseMode",
					  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_FiniteNumber(pSpeedMps) ||
				!Is_NonNegativeInteger(pDurationMs) ||
				!Is_FiniteNumber(pYawOffsetDegrees) ||
				pSpeedMps->Get_Number() < 0.0 ||
				pSpeedMps->Get_Number() > 50.0 ||
				pDurationMs->Get_Number() > 5000.0 ||
				std::abs(pYawOffsetDegrees->Get_Number()) > 180.0)
			{
				strOutError =
					"split gameplay grabbed-player release event is invalid";
				return false;
			}
			const bool_t bHold = "HOLD" == strReleaseMode &&
				0.0 == pSpeedMps->Get_Number() &&
				0.0 == pDurationMs->Get_Number() &&
				0.0 == pYawOffsetDegrees->Get_Number();
			const bool_t bOppositeKnockback =
				("OPPOSITE_KNOCKBACK" == strReleaseMode ||
				 "ARENA_EJECTION" == strReleaseMode) &&
				pSpeedMps->Get_Number() > 0.0 &&
				pDurationMs->Get_Number() > 0.0 &&
				("ARENA_EJECTION" == strReleaseMode ||
				 0.0 == pYawOffsetDegrees->Get_Number());
			if (!bHold && !bOppositeKnockback)
			{
				strOutError =
					"split gameplay grabbed-player release policy is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.attachment.left-hand"));
			Action.emplace("releaseMode",
				DATA_JSON_VALUE::String(strReleaseMode));
			Action.emplace("speedMps", *pSpeedMps);
			Action.emplace("durationMs", *pDurationMs);
			Action.emplace("yawOffsetDegrees", *pYawOffsetDegrees);
		}
		else if ("SPAWN_COMBAT_OBJECT_VOLLEY" == strKind)
		{
			const DATA_JSON_VALUE* pLayout = Required(
				Event, "layout", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pSpawnSchedule = Required(
				Event, "spawnSchedule", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pArenaRandom = Required(
				Event, "arenaRandom", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pAllowOverlap = Required(
				Event, "allowOverlap", DATA_JSON_TYPE::BOOLEAN);
			const std::string strVolleyPolicy =
				Read_String(Event, "volleyPolicy");
			const bool_t bPerAlivePlayer =
				"PER_ALIVE_PLAYER" == strVolleyPolicy;
			const bool_t bBossRelative = "BOSS_RELATIVE" == strVolleyPolicy;
			const bool_t bArenaCenter = "ARENA_CENTER" == strVolleyPolicy;
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "combatObjectArchetypeId",
					  "volleyPolicy", "countPerResolvedTarget", "layout",
					  "allowOverlap", "maximumTotalObjects", "spawnSchedule",
					  "arenaRandom" }) ||
				"ENTER" != strTrigger ||
				!Is_StableToken(Read_String(
					Event, "combatObjectArchetypeId")) ||
				(!bPerAlivePlayer && !bBossRelative && !bArenaCenter) ||
				!Is_NonNegativeInteger(Event.Find("countPerResolvedTarget")) ||
				0.0 == Event.Find("countPerResolvedTarget")->Get_Number() ||
				Event.Find("countPerResolvedTarget")->Get_Number() > 8.0 ||
				!Is_NonNegativeInteger(Event.Find("maximumTotalObjects")) ||
				0.0 == Event.Find("maximumTotalObjects")->Get_Number() ||
				Event.Find("maximumTotalObjects")->Get_Number() > 64.0 ||
				nullptr == pAllowOverlap || pAllowOverlap->Get_Boolean() ||
				nullptr == pLayout || nullptr == pSpawnSchedule ||
				nullptr == pArenaRandom)
			{
				strOutError = "split gameplay combat-object volley is invalid";
				return false;
			}
			const std::string strLayoutKind = Read_String(*pLayout, "kind");
			const bool_t bTargetCenter = "TARGET_CENTER" == strLayoutKind &&
				Has_ExactProperties(*pLayout, { "kind" });
			const bool_t bRadial = "RADIAL_AROUND_TARGET" == strLayoutKind &&
				Has_ExactProperties(*pLayout,
					{ "kind", "radiusM", "startAngleDegrees",
					  "angleStepDegrees" }) &&
				Is_FiniteNumber(pLayout->Find("radiusM")) &&
				Is_FiniteNumber(pLayout->Find("startAngleDegrees")) &&
				Is_FiniteNumber(pLayout->Find("angleStepDegrees"));
			const bool_t bBossRadial = "RADIAL_AROUND_BOSS" == strLayoutKind &&
				Has_ExactProperties(*pLayout,
					{ "kind", "radiusM", "startAngleDegrees",
					  "angleStepDegrees", "mappingBasis" }) &&
				"PROJECT_TUNED" == Read_String(*pLayout, "mappingBasis") &&
				Is_FiniteNumber(pLayout->Find("radiusM")) &&
				Is_FiniteNumber(pLayout->Find("startAngleDegrees")) &&
				Is_FiniteNumber(pLayout->Find("angleStepDegrees"));
			// ARENA_CENTER는 저작한 아레나 중심 주위에 방사형 배치 값을 사용한다.
			// 각도는 보스의 현재 yaw가 아니라 월드 기준이다.
			const bool_t bArenaCenterRadial =
				"RADIAL_AROUND_ARENA_CENTER" == strLayoutKind &&
				Has_ExactProperties(*pLayout,
					{ "kind", "radiusM", "startAngleDegrees",
					  "angleStepDegrees", "mappingBasis" }) &&
				"PROJECT_TUNED" == Read_String(*pLayout, "mappingBasis") &&
				Is_FiniteNumber(pLayout->Find("radiusM")) &&
				Is_FiniteNumber(pLayout->Find("startAngleDegrees")) &&
				Is_FiniteNumber(pLayout->Find("angleStepDegrees"));
			const double fCount =
				Event.Find("countPerResolvedTarget")->Get_Number();
			if ((bPerAlivePlayer && !bTargetCenter && !bRadial) ||
				(bBossRelative && !bBossRadial) ||
				(bArenaCenter && !bArenaCenterRadial))
			{
				strOutError = "split gameplay combat-object volley layout is invalid";
				return false;
			}
			if ((bPerAlivePlayer && bTargetCenter && 1.0 != fCount) ||
				((bRadial || bBossRadial || bArenaCenterRadial) && (fCount < 2.0 ||
					pLayout->Find("radiusM")->Get_Number() <= 0.0 ||
					pLayout->Find("angleStepDegrees")->Get_Number() <= 0.0 ||
					pLayout->Find("angleStepDegrees")->Get_Number() * fCount >
						360.000001)))
			{
				strOutError =
					"split gameplay combat-object volley layout/count is invalid";
				return false;
			}
			if (!Has_ExactProperties(*pSpawnSchedule,
					{ "kind", "count", "firstOffsetMs", "intervalMs" }) ||
				"INTERVAL" != Read_String(*pSpawnSchedule, "kind") ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("count")) ||
				0.0 == pSpawnSchedule->Find("count")->Get_Number() ||
				pSpawnSchedule->Find("count")->Get_Number() > 8.0 ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("firstOffsetMs")) ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("intervalMs")) ||
				(pSpawnSchedule->Find("count")->Get_Number() > 1.0 &&
				 0.0 == pSpawnSchedule->Find("intervalMs")->Get_Number()) ||
				(pSpawnSchedule->Find("count")->Get_Number() == 1.0 &&
				 0.0 != pSpawnSchedule->Find("intervalMs")->Get_Number()) ||
				pSpawnSchedule->Find("firstOffsetMs")->Get_Number() +
					(pSpawnSchedule->Find("count")->Get_Number() - 1.0) *
						pSpawnSchedule->Find("intervalMs")->Get_Number() >=
					static_cast<double>(iStageDurationMs))
			{
				strOutError =
					"split gameplay combat-object volley spawn schedule is invalid";
				return false;
			}
			const double fArenaRandomCount =
				pArenaRandom->Is_Object() && nullptr != pArenaRandom->Find("count") &&
				Is_NonNegativeInteger(pArenaRandom->Find("count")) ?
				pArenaRandom->Find("count")->Get_Number() : 0.0;
			const bool_t bNoArenaSupplement =
				Has_ExactProperties(*pArenaRandom, { "kind" }) &&
				"NONE" == Read_String(*pArenaRandom, "kind") &&
				Event.Find("maximumTotalObjects")->Get_Number() >= fCount;
			const bool_t bArenaSupplement =
				Has_ExactProperties(*pArenaRandom,
					{ "kind", "anchor", "count", "radiusM",
					  "heightToleranceM" }) &&
				"RANDOM_NAVIGABLE_CIRCLE" ==
					Read_String(*pArenaRandom, "kind") &&
				"BOSS_SPAWN_POSITION" == Read_String(*pArenaRandom, "anchor") &&
				Is_NonNegativeInteger(pArenaRandom->Find("count")) &&
				Is_FiniteNumber(pArenaRandom->Find("radiusM")) &&
				Is_FiniteNumber(pArenaRandom->Find("heightToleranceM")) &&
				fArenaRandomCount > 0.0 &&
				pArenaRandom->Find("radiusM")->Get_Number() > 0.0 &&
				pArenaRandom->Find("heightToleranceM")->Get_Number() > 0.0 &&
				Event.Find("maximumTotalObjects")->Get_Number() >=
					Event.Find("countPerResolvedTarget")->Get_Number() +
						fArenaRandomCount;
			const bool_t bPerAliveArenaContract = bPerAlivePlayer &&
				(bNoArenaSupplement || bArenaSupplement);
			const bool_t bBossRelativeContract =
				(bBossRelative || bArenaCenter) && bNoArenaSupplement &&
				1.0 == pSpawnSchedule->Find("count")->Get_Number() &&
				0.0 == pSpawnSchedule->Find("intervalMs")->Get_Number() &&
				Event.Find("maximumTotalObjects")->Get_Number() >= fCount;
			if (!bPerAliveArenaContract && !bBossRelativeContract)
			{
				strOutError =
					"split gameplay combat-object volley arena random contract is invalid";
				return false;
			}
			// Product 중간 표현에는 typed 일제 생성 행 하나를 만든다.
			// 방사형 배치와 스케줄을 모두 보존해 Master 검증 및 전투 오브젝트 결합에 같은 값을 전달한다.
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				Read_String(Event, "combatObjectArchetypeId")));
			Action.emplace("targetingPolicy",
				DATA_JSON_VALUE::String(strVolleyPolicy));
			Action.emplace("countPerResolvedTarget",
				*Event.Find("countPerResolvedTarget"));
			Action.emplace("layout", DATA_JSON_VALUE::String(
				bTargetCenter ? "SINGLE" : "RADIAL"));
			Action.emplace("radiusM", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 : Read_Number(*pLayout, "radiusM")));
			Action.emplace("startAngleDegrees", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 :
					Read_Number(*pLayout, "startAngleDegrees")));
			Action.emplace("angleStepDegrees", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 :
					Read_Number(*pLayout, "angleStepDegrees")));
			Action.emplace("allowOverlap", *pAllowOverlap);
			Action.emplace("maximumTotalObjects",
				*Event.Find("maximumTotalObjects"));
			Action.emplace("spawnCount", *pSpawnSchedule->Find("count"));
			Action.emplace("firstSpawnOffsetMs",
				*pSpawnSchedule->Find("firstOffsetMs"));
			Action.emplace("spawnIntervalMs",
				*pSpawnSchedule->Find("intervalMs"));
			Action.emplace("arenaRandomCount", DATA_JSON_VALUE::Number(
				bArenaSupplement ? Read_Number(*pArenaRandom, "count") : 0.0));
			Action.emplace("arenaRandomRadiusM", DATA_JSON_VALUE::Number(
				bArenaSupplement ? Read_Number(*pArenaRandom, "radiusM") : 0.0));
			Action.emplace("arenaHeightToleranceM", DATA_JSON_VALUE::Number(
				bArenaSupplement ?
					Read_Number(*pArenaRandom, "heightToleranceM") : 0.0));
			Action.emplace("arenaAnchorPolicy", DATA_JSON_VALUE::String(
				bArenaSupplement ? Read_String(*pArenaRandom, "anchor") : "NONE"));
			strOutSpawnArchetypeId = Read_String(
				Event, "combatObjectArchetypeId");
			Action.emplace("trigger", DATA_JSON_VALUE::String(strTrigger));
			Action.emplace("kind", DATA_JSON_VALUE::String(strKind));
			*pOutAction = DATA_JSON_VALUE::Object(std::move(Action));
			return true;
		}
		else if ("SPAWN_COMBAT_OBJECT" == strKind)
		{
			const std::string strCombatObjectArchetypeId = Read_String(
				Event, "combatObjectArchetypeId");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind",
					  "combatObjectArchetypeId", "count" }) ||
				"ENTER" != strTrigger ||
				!Is_StableToken(strCombatObjectArchetypeId) ||
				!Is_NonNegativeInteger(Event.Find("count")) ||
				1.0 != Event.Find("count")->Get_Number())
			{
				strOutError = "split gameplay combat-object spawn is invalid";
				return false;
			}
			strOutSpawnArchetypeId = strCombatObjectArchetypeId;
			Action.emplace("targetId",
				DATA_JSON_VALUE::String(strCombatObjectArchetypeId));
			Action.emplace("value", DATA_JSON_VALUE::Number(1));
		}
		else
		{
			strOutError = "split gameplay event kind is unsupported: " + strKind;
			return false;
		}
		Action.emplace("trigger", DATA_JSON_VALUE::String(strTrigger));
		Action.emplace("kind", DATA_JSON_VALUE::String(strKind));
		Action.emplace("durationMs", DATA_JSON_VALUE::Number(0));
		*pOutAction = DATA_JSON_VALUE::Object(std::move(Action));
		return true;
	}

	// 저작 gameplay와 presentation을 동일 패턴/stage 순서 및 stable ID로 맞춰 임시 Master로 합친다.
	// 호환 JSON 투영을 기존 Master reader에 넘긴 뒤 cue anchor와 stage 시간 예산을 추가 검사한다.
	// 입력 JSON 차용 포인터는 이 함수 안의 색인에만 보관하고 외부 결과에는 값을 복사한다.
	// 후반의 Parse_MasterDocument부터 Out을 직접 채우고 추가 검증 중에도 이를 수정한다.
	// false가 기존 Out의 보존을 뜻하지 않으므로 호출자는 지역 Master 후보를 넘긴다.
	bool_t Parse_SplitMasterDocument(
		const DATA_JSON_VALUE& Gameplay,
		const DATA_JSON_VALUE& Presentation,
		MASTER_DOCUMENT& Out,
		std::string& strOutError)
	{
		if (!Validate_SplitRootIdentity(Gameplay, Presentation, strOutError))
			return false;

		// preview 경로는 정확한 필드 목록과 Data/ 접두사를 검사한다.
		// 이 단계에서는 파일을 열거나 전체 경로의 안전성·존재를 확정하지 않는다.
		const DATA_JSON_VALUE* pPreviewPaths = Required(
			Gameplay, "previewPaths", DATA_JSON_TYPE::OBJECT);
		const std::initializer_list<std::string_view> PreviewFields = {
			"encounter", "animationBindings", "effectCues", "combatObjects",
			"bossCatalog", "effectCatalog", "damageProfiles", "cinematicCamera",
			"worldEvents", "patternRotations", "sourceClipSequences",
			"combatObjectAuthoring", "worldEventSets", "legacyCompatibility" };
		if (nullptr == pPreviewPaths ||
			!Has_ExactProperties(*pPreviewPaths, PreviewFields))
		{
			strOutError = "split gameplay previewPaths contract is invalid";
			return false;
		}
		for (const auto& [Name, Value] : pPreviewPaths->Get_Object())
		{
			(void)Name;
			if (!Value.Is_String() || !Value.Get_String().starts_with("Data/"))
			{
				strOutError = "split gameplay preview path is invalid";
				return false;
			}
		}

		// 분리 decisionModel을 호환 NORMAL 목록과 sequence, 선택/기믹/수동 보기로 투영한다.
		// 부분 작성될 수 있는 출력은 모두 이 함수의 지역 값으로 받아 실패 시 함께 버린다.
		DATA_JSON_VALUE NormalSelection;
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		std::vector<Client::VALTAN_SELECTION_SET_VIEW> SelectionSets;
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW> SelectionWindows;
		std::vector<Client::VALTAN_MECHANIC_VIEW> MechanicViews;
		std::map<std::string, SPLIT_MECHANIC, std::less<>> Mechanics;
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW> ManualViews;
		std::map<std::string, Client::VALTAN_MANUAL_AUDITION_VIEW,
			std::less<>> ManualAuditions;
		const DATA_JSON_VALUE* pDecision = Required(
			Gameplay, "decisionModel", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pDecision || !Build_SplitDecisionProjection(
				*pDecision, NormalSelection, ScriptedSequence, SelectionSets,
				SelectionWindows,
				MechanicViews, Mechanics, ManualViews, ManualAuditions,
				strOutError))
		{
			return false;
		}

		// 선택 집합의 원본 ID와 가중치 후보를 하나의 소유권 집합으로 모은다.
		// 비활성 후보도 여기에 남아 뒤의 패턴별 decision 소유자 검사에 참여한다.
		std::set<std::string, std::less<>> CandidatePatterns;
		for (const Client::VALTAN_SELECTION_SET_VIEW& Set : SelectionSets)
		{
			CandidatePatterns.insert(Set.PatternIds.begin(), Set.PatternIds.end());
			for (const Client::VALTAN_SELECTION_CANDIDATE_VIEW& Candidate :
				Set.Candidates)
			{
				CandidatePatterns.insert(Candidate.strPatternId);
			}
		}

		// 독립 Effect 선언을 ID로 모으고 소유 종류별 cue/spawn event 역색인을 만든다.
		// 선언 ID와 각 소유 종류 안의 참조 ID는 중복할 수 없으며 실제 owner 존재는 뒤에서 연결한다.
		const DATA_JSON_VALUE* pIndependent = Required(
			Presentation, "independentEffects", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pIndependent || pIndependent->Get_Array().empty())
		{
			strOutError = "split presentation independent Effect inventory is invalid";
			return false;
		}
		std::map<std::string, SPLIT_INDEPENDENT_DECLARATION, std::less<>>
			IndependentById;
		std::map<std::string, std::string, std::less<>>
			IndependentByCue;
		std::map<std::string, std::string, std::less<>>
			IndependentBySpawnEvent;
		for (const DATA_JSON_VALUE& Value : pIndependent->Get_Array())
		{
			SPLIT_INDEPENDENT_DECLARATION Declaration;
			Declaration.strIndependentEffectId = Read_String(
				Value, "independentEffectId");
			Declaration.strDisplayName = Read_String(Value, "displayName");
			Declaration.strOwnership = Read_String(Value, "ownership");
			const bool_t bPatternStage =
				"SERVER_PATTERN_STAGE" == Declaration.strOwnership;
			const bool_t bCombatObject =
				"SERVER_COMBAT_OBJECT" == Declaration.strOwnership;
			if ((!bPatternStage && !bCombatObject) ||
				!Is_StableToken(Declaration.strIndependentEffectId) ||
				Declaration.strDisplayName.empty() ||
				!Has_ExactProperties(Value,
					bPatternStage ?
						std::initializer_list<std::string_view>{
							"independentEffectId", "displayName", "ownership", "cueId" } :
						std::initializer_list<std::string_view>{
							"independentEffectId", "displayName", "ownership", "spawnEventId" }))
			{
				strOutError = "split presentation independent Effect is invalid";
				return false;
			}
			Declaration.strReferenceId = Read_String(
				Value, bPatternStage ? "cueId" : "spawnEventId");
			if (!Is_StableToken(Declaration.strReferenceId) ||
				!IndependentById.emplace(
					Declaration.strIndependentEffectId, Declaration).second)
			{
				strOutError = "split presentation independent Effect identity is duplicated";
				return false;
			}
			auto& ReferenceMap = bPatternStage ?
				IndependentByCue : IndependentBySpawnEvent;
			if (!ReferenceMap.emplace(
					Declaration.strReferenceId,
					Declaration.strIndependentEffectId).second)
			{
				strOutError = "split presentation independent Effect owner reference is duplicated";
				return false;
			}
		}

		// 두 문서의 패턴 수와 배열 순서를 함께 사용한다. ID로 재정렬하지 않으므로
		// 같은 위치의 patternId가 달라지면 결합을 거부한다.
		const DATA_JSON_VALUE* pGameplayPatterns = Required(
			Gameplay, "patterns", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPresentationPatterns = Required(
			Presentation, "patterns", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pGameplayPatterns || nullptr == pPresentationPatterns ||
			pGameplayPatterns->Get_Array().empty() ||
			pGameplayPatterns->Get_Array().size() !=
				pPresentationPatterns->Get_Array().size())
		{
			strOutError = "split authoring patternId closure mismatch";
			return false;
		}

		// cue의 입력 JSON 포인터는 아래 지역 색인에서만 차용한다.
		// cue/event/camera 발생 ID의 전체 중복과 stage별 owner를 수집해 후속 독립 Effect 결합에 쓴다.
		std::map<std::string, SPLIT_CUE_OWNER, std::less<>> CueOwners;
		std::map<std::string, SPLIT_SPAWN_OWNER, std::less<>> SpawnOwners;
		std::vector<MASTER_VOLLEY_PROJECTION> VolleyProjections;
		std::set<std::string, std::less<>> CueOccurrenceIds;
		std::set<std::string, std::less<>> EventIds;
		std::set<std::string, std::less<>> CameraInvocationIds;
		DATA_JSON_VALUE::ARRAY LegacyPatterns;
		std::set<std::string, std::less<>> ScriptedEntryOnlyPatternIds;
		for (size_t iPattern = 0u;
			iPattern < pGameplayPatterns->Get_Array().size(); ++iPattern)
		{
			const DATA_JSON_VALUE& GameplayPattern =
				pGameplayPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& PresentationPattern =
				pPresentationPatterns->Get_Array()[iPattern];
			if (!Has_ExactPropertiesWithOptional(GameplayPattern,
					{ "patternId", "displayName", "category",
					  "compatibilitySelectionWeight", "actionId",
					  "entryActionId", "targetPolicy", "aimPolicy", "eligibility",
					  "invulnerableWhileRunning", "sourceActionIds", "serverMotion",
					  "reactions", "stages" },
					{ "finale", "verticalOffsetM" }) ||
				!Has_ExactProperties(PresentationPattern,
					{ "patternId", "sourceSequenceIndex", "presentationSources",
					  "stages" }))
			{
				strOutError = "split authoring pattern has unexpected properties";
				return false;
			}
			const std::string strPatternId = Read_String(
				GameplayPattern, "patternId");
			if (!Is_StableToken(strPatternId) || strPatternId !=
				Read_String(PresentationPattern, "patternId"))
			{
				strOutError = "split authoring patternId order/identity mismatch";
				return false;
			}

			// 같은 패턴 안의 stage 수를 맞추고 gameplay의 stageId/actionId를 각각 유일하게 모은다.
			// entryActionId는 이 집합에 존재하면서 첫 stage의 action이어야 한다.
			const DATA_JSON_VALUE* pGameplayStages = Required(
				GameplayPattern, "stages", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* pPresentationStages = Required(
				PresentationPattern, "stages", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pGameplayStages || nullptr == pPresentationStages ||
				pGameplayStages->Get_Array().empty() ||
				pGameplayStages->Get_Array().size() !=
					pPresentationStages->Get_Array().size())
			{
				strOutError = "split authoring stageId closure mismatch: " +
					strPatternId;
				return false;
			}
			std::set<std::string, std::less<>> StageIds;
			std::set<std::string, std::less<>> StageActionIds;
			for (const DATA_JSON_VALUE& Stage : pGameplayStages->Get_Array())
			{
				const std::string strStageId = Read_String(Stage, "stageId");
				const std::string strActionId = Read_String(Stage, "actionId");
				if (!Is_StableToken(strStageId) || !Is_StableToken(strActionId) ||
					!StageIds.insert(strStageId).second ||
					!StageActionIds.insert(strActionId).second)
				{
					strOutError = "split gameplay stage identity is invalid or duplicated: " +
						strPatternId;
					return false;
				}
			}
			const std::string strEntryActionId = Read_String(
				GameplayPattern, "entryActionId");
			if (!StageActionIds.contains(strEntryActionId) ||
				strEntryActionId != Read_String(
					pGameplayStages->Get_Array().front(), "actionId"))
			{
				strOutError = "split gameplay entryActionId is not the first joined stage: " +
					strPatternId;
				return false;
			}

			DATA_JSON_VALUE::ARRAY LegacyStages;
			DATA_JSON_VALUE::ARRAY WorldReferences;
			std::vector<std::string> CameraCueIds;
			std::set<std::string, std::less<>> PatternCameraCueIds;

			// stage도 원본 배열의 같은 위치끼리 결합한다. 허용 필드와 stage/action ID를 먼저 맞춘 뒤
			// 양수 ms 기간과 그 기간을 기준으로 한 bodyVisibility를 검사한다.
			for (size_t iStage = 0u;
				iStage < pGameplayStages->Get_Array().size(); ++iStage)
			{
				const DATA_JSON_VALUE& GameplayStage =
					pGameplayStages->Get_Array()[iStage];
				const DATA_JSON_VALUE& PresentationStage =
					pPresentationStages->Get_Array()[iStage];
				if (!Has_ExactPropertiesWithOptional(GameplayStage,
						{ "stageId", "actionId", "stageKind", "durationMs",
						  "defaultNextActionId", "hit", "motion", "events",
						  "branches" },
						{ "partDamagePolicy", "counterProxy", "bossResponse",
						  "verticalOffsetM", "aim" }) ||
					!Has_ExactPropertiesWithOptional(PresentationStage,
						{ "stageId", "actionId", "sequenceRole", "animation",
						  "effectCues", "cameraInvocations" },
						{ "bodyVisibility", "sceneProfileOccurrences", "lightOccurrences" }))
				{
					strOutError = "split authoring stage has unexpected properties: " +
						strPatternId;
					return false;
				}
				const std::string strStageId = Read_String(
					GameplayStage, "stageId");
				const std::string strActionId = Read_String(
					GameplayStage, "actionId");
				if (strStageId != Read_String(PresentationStage, "stageId") ||
					strActionId != Read_String(PresentationStage, "actionId"))
				{
					strOutError = "split authoring stageId/actionId mismatch: " +
						strPatternId + "/" + strStageId;
					return false;
				}
				const DATA_JSON_VALUE* pDuration = GameplayStage.Find("durationMs");
				if (!Is_NonNegativeInteger(pDuration) ||
					0.0 == pDuration->Get_Number())
				{
					strOutError = "split gameplay stage duration is invalid: " +
						strPatternId + "/" + strStageId;
					return false;
				}
				bool_t bHasBodyHiddenWindow = false;
				uint32_t iBodyHiddenFromMs = 0u;
				uint32_t iBodyHiddenToMs = 0u;
				if (!Read_StageBodyVisibility(
						PresentationStage.Find("bodyVisibility"),
						static_cast<uint32_t>(pDuration->Get_Number()),
						bHasBodyHiddenWindow, iBodyHiddenFromMs,
						iBodyHiddenToMs))
				{
					strOutError =
						"split presentation bodyVisibility is invalid: " +
						strPatternId + "/" + strStageId;
					return false;
				}

				// 분기와 proxy/응답 확장을 읽고 defaultNextActionId의 기대값을 구한다.
				// TIMEOUT 분기가 있으면 그 nullable action을 우선하며, 없으면 다음 stage, 마지막이면 없음이다.
				std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> BranchViews;
				const DATA_JSON_VALUE* pBranches = Required(
					GameplayStage, "branches", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pBranches ||
					!Read_StageBranches(pBranches, BranchViews) ||
					!Validate_SplitGameplayStageExtensions(
						GameplayStage, BranchViews, strPatternId, strStageId,
						strOutError))
				{
					if (strOutError.empty())
					{
						strOutError = "split gameplay stage branches are invalid: " +
							strPatternId + "/" + strStageId;
					}
					return false;
				}
				std::optional<std::string> ExpectedDefault;
				const auto Timeout = std::find_if(
					BranchViews.begin(), BranchViews.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{
						return "TIMEOUT" == Branch.strOutcome;
					});
				if (Timeout != BranchViews.end())
					ExpectedDefault = Timeout->strNextActionId;
				else if (iStage + 1u < pGameplayStages->Get_Array().size())
					ExpectedDefault = Read_String(
						pGameplayStages->Get_Array()[iStage + 1u], "actionId");
				std::string strDefault;
				if (!Read_NullableStableToken(
						GameplayStage, "defaultNextActionId", strDefault) ||
					ExpectedDefault.has_value() != !strDefault.empty() ||
					(ExpectedDefault.has_value() &&
					 *ExpectedDefault != strDefault))
				{
					strOutError = "split gameplay defaultNextActionId drifted: " +
						strPatternId + "/" + strStageId;
					return false;
				}

				// 로컬 nextActionId를 가진 분기는 앞서 모은 action 집합 안을 가리켜야 한다.
				// 다른 패턴으로 넘어가는 nextPatternId와 counter 소유 계약은 뒤의 별도 검사에서 확인한다.
				for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch : BranchViews)
				{
					if (Branch.strNextActionId.has_value() &&
						!StageActionIds.contains(*Branch.strNextActionId))
					{
						strOutError = "split gameplay branch targets an unknown action: " +
							*Branch.strNextActionId;
						return false;
					}
				}

				// 현재 두 stage를 기존 Master reader가 이해하는 지역 JSON으로 합친다.
				// 판정·이동·응답은 gameplay에서, 애니메이션과 표현은 presentation에서 가져온다.
				DATA_JSON_VALUE::OBJECT LegacyStage;
				LegacyStage.emplace("stageId", DATA_JSON_VALUE::String(strStageId));
				LegacyStage.emplace("sequenceRole", DATA_JSON_VALUE::String(
					Read_String(PresentationStage, "sequenceRole")));
				LegacyStage.emplace("actionId", DATA_JSON_VALUE::String(strActionId));
				LegacyStage.emplace("stageKind", *GameplayStage.Find("stageKind"));
				LegacyStage.emplace("durationMs", *pDuration);
				if (const DATA_JSON_VALUE* pPartDamagePolicy =
						GameplayStage.Find("partDamagePolicy"))
				{
					LegacyStage.emplace("partDamagePolicy", *pPartDamagePolicy);
				}
				if (const DATA_JSON_VALUE* pCounterProxy =
						GameplayStage.Find("counterProxy"))
				{
					LegacyStage.emplace("counterProxy", *pCounterProxy);
				}
				if (const DATA_JSON_VALUE* pBossResponse =
						GameplayStage.Find("bossResponse"))
				{
					LegacyStage.emplace("bossResponse", *pBossResponse);
				}
				if (const DATA_JSON_VALUE* pVerticalOffset =
						GameplayStage.Find("verticalOffsetM"))
				{
					LegacyStage.emplace("verticalOffsetM", *pVerticalOffset);
				}

				// hit는 호환 필드로 투영하고 motion은 null 또는 객체만 허용한다.
				// 여기서 복사한 세부 값은 뒤의 Master 파서가 추가 검증한다.
				const DATA_JSON_VALUE* pHit = Required(
					GameplayStage, "hit", DATA_JSON_TYPE::OBJECT);
				if (nullptr == pHit || !Build_SplitHitProjection(
						*pHit, LegacyStage, strOutError))
					return false;
				const DATA_JSON_VALUE* pMotion = GameplayStage.Find("motion");
				if (nullptr == pMotion ||
					(!pMotion->Is_Null() && !pMotion->Is_Object()))
				{
					strOutError = "split gameplay stage motion is invalid";
					return false;
				}
				LegacyStage.emplace("motion", *pMotion);
				if (const DATA_JSON_VALUE* pAim = GameplayStage.Find("aim"))
					LegacyStage.emplace("aim", *pAim);

				// stage 이벤트를 호환 action, 독립 Effect 참조, 별도 volley 목록으로 분리한다.
				// 예약 volley 시계는 현재 stage마다 새로 시작하며 다른 stage와 같은 시각을 강제하지 않는다.
				DATA_JSON_VALUE::ARRAY Actions;
				DATA_JSON_VALUE::ARRAY EffectReferences;
				bool_t bHasScheduledVolleyClock = false;
				uint32_t iScheduledVolleyFirstOffsetMs = 0u;
				uint32_t iScheduledVolleyCount = 0u;
				uint32_t iScheduledVolleyIntervalMs = 0u;
				const DATA_JSON_VALUE* pEvents = Required(
					GameplayStage, "events", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pEvents)
				{
					strOutError = "split gameplay stage events are invalid";
					return false;
				}

				// eventId는 전체 문서에서 중복될 수 없다. 각 이벤트의 변환 함수에는
				// 분기·기본 후속이 없는 마지막 stage인지도 넘겨 종료 전용 이벤트의 허용 여부를 검사한다.
				for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
				{
					const std::string strEventId = Read_String(Event, "eventId");
					if (!EventIds.insert(strEventId).second)
					{
						strOutError = "split gameplay eventId is duplicated: " + strEventId;
						return false;
					}
					DATA_JSON_VALUE Action;
					std::string strSpawnArchetypeId;
					bool_t bWorldEvent = false;
					if (!Build_SplitEventProjection(Event,
							strPatternId, strStageId, strActionId,
							iStage + 1u == pGameplayStages->Get_Array().size() &&
								BranchViews.empty() && strDefault.empty(),
							static_cast<uint32_t>(pDuration->Get_Number()), &Action,
							strSpawnArchetypeId, bWorldEvent, strOutError))
						return false;
					const bool_t bVolleyEvent =
						"SPAWN_COMBAT_OBJECT_VOLLEY" ==
							Read_String(Event, "kind");

					// volley는 일반 Actions에 넣지 않고 원래 event/pattern/stage/action 소유 정보와 함께 보관한다.
					// 첫 offset이 0이 아니거나 여러 번 발생하는 예약 volley끼리만 시작·횟수·간격을 같게 제한한다.
					if (bVolleyEvent)
					{
						if (Action.Is_Null())
						{
							strOutError =
								"split gameplay volley Product projection is missing";
							return false;
						}
						const uint32_t iFirstSpawnOffsetMs = static_cast<uint32_t>(
							Read_Number(Action, "firstSpawnOffsetMs"));
						const uint32_t iSpawnCount = static_cast<uint32_t>(
							Read_Number(Action, "spawnCount"));
						const uint32_t iSpawnIntervalMs = static_cast<uint32_t>(
							Read_Number(Action, "spawnIntervalMs"));
						if (0u != iFirstSpawnOffsetMs || iSpawnCount > 1u)
						{
							if (!bHasScheduledVolleyClock)
							{
								bHasScheduledVolleyClock = true;
								iScheduledVolleyFirstOffsetMs = iFirstSpawnOffsetMs;
								iScheduledVolleyCount = iSpawnCount;
								iScheduledVolleyIntervalMs = iSpawnIntervalMs;
							}
							else if (iScheduledVolleyFirstOffsetMs !=
									iFirstSpawnOffsetMs ||
								iScheduledVolleyCount != iSpawnCount ||
								iScheduledVolleyIntervalMs != iSpawnIntervalMs)
							{
								strOutError =
									"split gameplay scheduled volleys do not share one clock";
								return false;
							}
						}
						VolleyProjections.push_back(MASTER_VOLLEY_PROJECTION{
							strEventId,
							strPatternId,
							strStageId,
							strActionId,
							strSpawnArchetypeId,
							Action });
					}
					if (!Action.Is_Null() && !bVolleyEvent)
						Actions.push_back(std::move(Action));

					// spawn 이벤트의 archetype과 owner를 저장한다. 독립 Effect 선언이 해당 event를 가리키면
					// 그 선언 ID로 참조를 만들며, 실제 visual/asset 결합은 더 뒤의 Product 연결이 담당한다.
					if (!strSpawnArchetypeId.empty())
					{
						SPLIT_SPAWN_OWNER Owner;
						Owner.strPatternId = strPatternId;
						Owner.strStageId = strStageId;
						Owner.strCombatObjectArchetypeId = strSpawnArchetypeId;
						if (!SpawnOwners.emplace(strEventId, std::move(Owner)).second)
						{
							strOutError = "split gameplay spawn event owner is duplicated";
							return false;
						}
						const auto Independent = IndependentBySpawnEvent.find(strEventId);
						if (Independent != IndependentBySpawnEvent.end())
						{
							DATA_JSON_VALUE::OBJECT Reference;
							Reference.emplace("refType",
								DATA_JSON_VALUE::String("INDEPENDENT_EFFECT"));
							Reference.emplace("refId",
								DATA_JSON_VALUE::String(Independent->second));
							EffectReferences.push_back(
								DATA_JSON_VALUE::Object(std::move(Reference)));
						}
					}

					// 월드 이벤트는 ENTER만 허용하고 현재 pattern/stage의 STAGE_ENTER 참조로 바꾼다.
					// 이 참조를 만드는 작업이며 여기서 월드 이벤트를 실행하지 않는다.
					if (bWorldEvent)
					{
						if ("ENTER" != Read_String(Event, "trigger"))
						{
							strOutError = "split gameplay world event trigger is unsupported";
							return false;
						}
						DATA_JSON_VALUE::OBJECT Reference;
						Reference.emplace("patternId",
							DATA_JSON_VALUE::String(strPatternId));
						Reference.emplace("stageId",
							DATA_JSON_VALUE::String(strStageId));
						Reference.emplace("triggerKind",
							DATA_JSON_VALUE::String("STAGE_ENTER"));
						WorldReferences.push_back(
							DATA_JSON_VALUE::Object(std::move(Reference)));
					}
				}
				LegacyStage.emplace("actions", DATA_JSON_VALUE::Array(std::move(Actions)));
				LegacyStage.emplace("branches", *pBranches);

				const DATA_JSON_VALUE* pCues = Required(
					PresentationStage, "effectCues", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pCues)
				{
					strOutError = "split presentation effectCues is invalid";
					return false;
				}
				// stage 시계 cue는 stageOffsetMs를, clip 시계 cue는 같은 occurrence와 mappingBasis를 기준으로 연결한다.
				for (const DATA_JSON_VALUE& Cue : pCues->Get_Array())
				{
					if (!Validate_SplitCue(Cue, strOutError))
						return false;
					const bool_t bUsesStageClock =
						nullptr != Cue.Find("timingBasis");

					// stage 시계는 시작 offset이 현재 stage 기간보다 작은지 확인한다.
					// 끝 시각은 최대 600000ms와 비교하므로 stage 기간 안에서 끝난다는 보장은 아니다.
					if (bUsesStageClock)
					{
						const auto* end = Cue.Find("stageEndMs");
						if (Cue.Find("stageOffsetMs")->Get_Number() >= pDuration->Get_Number() ||
							(end && !end->Is_Null() && end->Get_Number() > 600000.0))
						{
							strOutError =
								"split stage-clock cue left its Stage wall";
							return false;
						}
					}
					else
					{
						const DATA_JSON_VALUE* pAnimation = Required(
							PresentationStage, "animation", DATA_JSON_TYPE::OBJECT);
						const DATA_JSON_VALUE* pOccurrences =
							nullptr == pAnimation ? nullptr : Required(
								*pAnimation, "occurrences", DATA_JSON_TYPE::ARRAY);
						const DATA_JSON_VALUE* pCueOccurrence = nullptr;
						if (nullptr != pOccurrences)
						{
							for (const DATA_JSON_VALUE& Occurrence :
								pOccurrences->Get_Array())
							{
								if (Read_String(Occurrence, "clipOccurrenceId") !=
									Read_String(Cue, "clipOccurrenceId"))
								{
									continue;
								}
								if (nullptr != pCueOccurrence)
								{
									strOutError =
										"split cue clip occurrence is duplicated in its stage";
									return false;
								}
								pCueOccurrence = &Occurrence;
							}
						}
						if (nullptr == pCueOccurrence ||
							Read_String(*pCueOccurrence, "mappingBasis") !=
								Read_String(Cue, "mappingBasis") ||
							!Is_NonNegativeInteger(
								pCueOccurrence->Find("sourceStartMs")) ||
							Cue.Find("sourceStartMs")->Get_Number() <
								pCueOccurrence->Find("sourceStartMs")->Get_Number())
						{
							strOutError =
								"split cue did not join its saved animation occurrence";
							return false;
						}
					}

					// cue ID와 occurrence ID를 각각 전체 문서에서 유일하게 등록한다.
					// 독립 선언이 있으면 그 ID를 참조하고, 없으면 cue ID와 원래 시계 정보를 호환 참조에 복사한다.
					const std::string strCueId = Read_String(Cue, "cueId");
					const std::string strOccurrenceId = Read_String(
						Cue, "occurrenceId");
					SPLIT_CUE_OWNER CueOwner;
					CueOwner.strPatternId = strPatternId;
					CueOwner.strStageId = strStageId;
					CueOwner.pCue = &Cue;
					if (!CueOwners.emplace(strCueId, std::move(CueOwner)).second ||
						!CueOccurrenceIds.insert(strOccurrenceId).second)
					{
						strOutError = "split presentation cue identity is duplicated";
						return false;
					}
					DATA_JSON_VALUE::OBJECT Reference;
					const auto Independent = IndependentByCue.find(strCueId);
					if (Independent != IndependentByCue.end())
					{
						Reference.emplace("refType",
							DATA_JSON_VALUE::String("INDEPENDENT_EFFECT"));
						Reference.emplace("refId",
							DATA_JSON_VALUE::String(Independent->second));
					}
					else
					{
						Reference.emplace("refType",
							DATA_JSON_VALUE::String("CUE_BINDING"));
						Reference.emplace("refId", DATA_JSON_VALUE::String(strCueId));
						DATA_JSON_VALUE::OBJECT Projection;
						if (bUsesStageClock)
						{
							Projection.emplace("timingBasis", *Cue.Find("timingBasis"));
							Projection.emplace("stageOffsetMs", *Cue.Find("stageOffsetMs"));
							if (const auto* end = Cue.Find("stageEndMs"))
								Projection.emplace("stageEndMs", *end);
						}
						else
						{
							Projection.emplace("clipOccurrenceId",
								*Cue.Find("clipOccurrenceId"));
							Projection.emplace(
								"sourceStartMs", *Cue.Find("sourceStartMs"));
							Projection.emplace(
								"sourceEndMs", *Cue.Find("sourceEndMs"));
							Projection.emplace(
								"mappingBasis", *Cue.Find("mappingBasis"));
						}
						Reference.emplace("cueProjection",
							DATA_JSON_VALUE::Object(std::move(Projection)));
					}
					EffectReferences.push_back(
						DATA_JSON_VALUE::Object(std::move(Reference)));
				}

				// 애니메이션·환경/조명 occurrence와 선택적 몸체 숨김을 같은 stage에 복사한다.
				// 위에서 모은 Effect 참조와 함께 기존 Master stage 검증으로 넘어갈 값이다.
				LegacyStage.emplace("animation", *PresentationStage.Find("animation"));
				for (const char* field : {"sceneProfileOccurrences", "lightOccurrences"})
					if (const auto* rows = PresentationStage.Find(field))
						LegacyStage.emplace(field, *rows);
				if (bHasBodyHiddenWindow)
				{
					LegacyStage.emplace(
						"bodyVisibility", *PresentationStage.Find("bodyVisibility"));
				}
				LegacyStage.emplace("effectRefs",
					DATA_JSON_VALUE::Array(std::move(EffectReferences)));

				// cameraInvocationId는 문서 전체에서, cameraCueId는 현재 패턴 안에서 중복을 거부한다.
				// 검증된 cue ID 순서를 패턴 목록에도 모으고 stage에는 원본 invocation 값을 보존한다.
				const DATA_JSON_VALUE* pCamera = Required(
					PresentationStage, "cameraInvocations", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pCamera)
				{
					strOutError = "split presentation cameraInvocations is invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& Invocation : pCamera->Get_Array())
				{
					std::string strCameraCueId;
					if (!Validate_SplitCameraInvocation(Invocation,
							static_cast<uint32_t>(pDuration->Get_Number()),
							strCameraCueId, strOutError) ||
						!CameraInvocationIds.insert(Read_String(
							Invocation, "cameraInvocationId")).second ||
						!PatternCameraCueIds.insert(strCameraCueId).second)
					{
						if (strOutError.empty())
							strOutError = "split presentation camera identity is duplicated";
						return false;
					}
					CameraCueIds.push_back(std::move(strCameraCueId));
				}
				LegacyStage.emplace("cameraInvocations", *pCamera);
				LegacyStages.push_back(
					DATA_JSON_VALUE::Object(std::move(LegacyStage)));
			}

			// 모든 stage를 합친 뒤 원래 gameplay 문서의 counter 성공/timeout 및 proxy 관계를 검사한다.
			// 다른 패턴의 성공 대상을 찾을 수 있도록 전체 gameplay 패턴 배열을 전달한다.
			if (!Validate_SplitCounterBranchContract(
					GameplayPattern, pGameplayPatterns->Get_Array(),
					strPatternId, strOutError))
			{
				return false;
			}

			// eligibility의 정확한 필드와 지원하는 반복·cooldown 정책을 검사한다.
			// phase/체력 값은 비음수 정수, 거리 값은 유한 수인지 먼저 확인하고 호환 Master에 전달한다.
			const DATA_JSON_VALUE* pEligibility = Required(
				GameplayPattern, "eligibility", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pEligibility || !Has_ExactProperties(*pEligibility,
					{ "armorRequirement", "phaseRequirement", "minimumGameplayPhase",
					  "maximumGameplayPhase", "minimumHealthBarInclusive",
					  "maximumHealthBarInclusive", "minimumRangeM", "maximumRangeM",
					  "cooldownPolicy", "selectionCooldownMs", "cooldownGroupId",
					  "repeatPolicy" }))
			{
				strOutError = "split gameplay pattern eligibility is invalid: " +
					strPatternId;
				return false;
			}
			const DATA_JSON_VALUE* pRepeatPolicy = Required(
				*pEligibility, "repeatPolicy", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pRepeatPolicy || !Has_ExactProperties(
					*pRepeatPolicy, { "kind", "limit" }) ||
				"SOFT_AVOID_UNLESS_ONLY_ELIGIBLE" !=
					Read_String(*pRepeatPolicy, "kind") ||
				!Is_NonNegativeInteger(pRepeatPolicy->Find("limit")) ||
				"DERIVED_SOURCE_ACTION" !=
					Read_String(*pEligibility, "cooldownPolicy") ||
				nullptr == pEligibility->Find("selectionCooldownMs") ||
				!pEligibility->Find("selectionCooldownMs")->Is_Null() ||
				nullptr == pEligibility->Find("cooldownGroupId") ||
				!pEligibility->Find("cooldownGroupId")->Is_Null())
			{
				strOutError = "split gameplay eligibility policy is invalid: " +
					strPatternId;
				return false;
			}
			for (const std::string_view Field :
				{ "minimumGameplayPhase", "maximumGameplayPhase",
				  "minimumHealthBarInclusive", "maximumHealthBarInclusive" })
			{
				if (!Is_NonNegativeInteger(pEligibility->Find(Field)))
				{
					strOutError = "split gameplay eligibility integer is invalid";
					return false;
				}
			}
			if (!Is_FiniteNumber(pEligibility->Find("minimumRangeM")) ||
				!Is_FiniteNumber(pEligibility->Find("maximumRangeM")))
			{
				strOutError = "split gameplay eligibility range is invalid";
				return false;
			}

			// 패턴 verticalOffsetM이 있으면 0이 아닌 ±100m 범위여야 하고
			// 적어도 한 gameplay stage가 bossResponse 필드를 가져야 한다.
			const DATA_JSON_VALUE* pVerticalOffset =
				GameplayPattern.Find("verticalOffsetM");
			const bool_t bOwnsBossResponse = std::any_of(
				pGameplayStages->Get_Array().begin(),
				pGameplayStages->Get_Array().end(),
				[](const DATA_JSON_VALUE& Stage)
				{ return nullptr != Stage.Find("bossResponse"); });
			if (nullptr != pVerticalOffset &&
				(!Is_FiniteNumber(pVerticalOffset) ||
				 0.0 == pVerticalOffset->Get_Number() ||
				 std::fabs(pVerticalOffset->Get_Number()) > 100.0 ||
				 !bOwnsBossResponse))
			{
				strOutError =
					"split gameplay verticalOffsetM requires an active boss response: " +
					strPatternId;
				return false;
			}

			// 각 일반 패턴은 기믹, 선택 후보, 수동 시험 중 정확한 decision 소유자를 가져야 한다.
			// 검토된 입장 전용 패턴 두 개의 예외는 아래에서 별도로 제한한다.
			const auto Mechanic = Mechanics.find(strPatternId);
			const bool_t bMechanic = Mechanic != Mechanics.end();
			const bool_t bCandidate = CandidatePatterns.contains(strPatternId);
			const auto Manual = ManualAuditions.find(strPatternId);
			const bool_t bManual = Manual != ManualAuditions.end();
			const DATA_JSON_VALUE* pCompatibilityWeight =
				GameplayPattern.Find("compatibilitySelectionWeight");
			const uint32_t iOwnerCount =
				(bMechanic ? 1u : 0u) + (bCandidate ? 1u : 0u) +
				(bManual ? 1u : 0u);
			// 허용된 두 입장 연출은 사용자 Flow에서 빠져도 입장 전용 정의로 남을 수 있다.
			// 이 예외로 decision 소유자가 없는 임의 패턴을 허용하지 않는다.
			const bool_t bOptionalEntryPattern =
				Is_OptionalEntryPatternId(strPatternId);
			const bool_t bScriptedEntryOnly =
				bOptionalEntryPattern && 0u == iOwnerCount;
			const auto EntryInSequence = std::find(ScriptedSequence.PatternIds.begin(),
				ScriptedSequence.PatternIds.end(), strPatternId);
			if (bScriptedEntryOnly && EntryInSequence != ScriptedSequence.PatternIds.end() &&
				(EntryInSequence != ScriptedSequence.PatternIds.begin() ||
				 std::count(ScriptedSequence.PatternIds.begin(),
					 ScriptedSequence.PatternIds.end(), strPatternId) != 1))
			{
				strOutError = "entry-only decision owner must occur only at the first slot: " + strPatternId;
				return false;
			}
			if ((bOptionalEntryPattern && !bScriptedEntryOnly) ||
				(!bOptionalEntryPattern && 1u != iOwnerCount) ||
				!Is_NonNegativeInteger(pCompatibilityWeight) ||
				((bMechanic || bManual) &&
				 0.0 != pCompatibilityWeight->Get_Number()) ||
				((bCandidate || bScriptedEntryOnly) &&
				 0.0 == pCompatibilityWeight->Get_Number()))
			{
				strOutError = "split gameplay pattern requires exactly one decision owner or an optional entry-only gate: " +
					strPatternId;
				return false;
			}
			if (bScriptedEntryOnly)
				ScriptedEntryOnlyPatternIds.insert(strPatternId);

			// decision 소유자에 따라 HEALTH_BAR/AUDITION_ONLY/NORMAL과 기믹 trigger 값을 만든다.
			// 수동 시험의 연속 사용 한도는 0으로 투영하며 일반/입장 후보는 호환 가중치를 유지한다.
			DATA_JSON_VALUE::OBJECT LegacyPattern;
			LegacyPattern.emplace("patternId", DATA_JSON_VALUE::String(strPatternId));
			for (const std::string& Field :
				{ "category", "targetPolicy", "aimPolicy", "displayName",
				  "actionId", "sourceActionIds", "serverMotion", "reactions" })
			{
				const DATA_JSON_VALUE* pValue = GameplayPattern.Find(Field);
				if (nullptr == pValue)
				{
					strOutError = "split gameplay pattern field is missing: " + Field;
					return false;
				}
				LegacyPattern.emplace(Field, *pValue);
			}
			if (const DATA_JSON_VALUE* pFinale = GameplayPattern.Find("finale"))
				LegacyPattern.emplace("finale", *pFinale);
			if (nullptr != pVerticalOffset)
				LegacyPattern.emplace("verticalOffsetM", *pVerticalOffset);
			LegacyPattern.emplace("minimumPhase",
				*pEligibility->Find("minimumGameplayPhase"));
			LegacyPattern.emplace("maximumPhase",
				*pEligibility->Find("maximumGameplayPhase"));
			LegacyPattern.emplace("sourceSequenceIndex",
				*PresentationPattern.Find("sourceSequenceIndex"));
			LegacyPattern.emplace("presentationSources",
				*PresentationPattern.Find("presentationSources"));
			LegacyPattern.emplace("selectionMode",
				DATA_JSON_VALUE::String(bMechanic ? "HEALTH_BAR" :
					(bManual ? "AUDITION_ONLY" : "NORMAL")));
			LegacyPattern.emplace("minimumHealthBar",
				*pEligibility->Find("minimumHealthBarInclusive"));
			LegacyPattern.emplace("maximumHealthBar",
				*pEligibility->Find("maximumHealthBarInclusive"));
			LegacyPattern.emplace("triggerHealthBar", DATA_JSON_VALUE::Number(
				bMechanic ? Mechanic->second.iHealthBar : 0u));
			LegacyPattern.emplace("triggerOrder", DATA_JSON_VALUE::Number(
				bMechanic ? Mechanic->second.iTriggerOrder : 0u));
			LegacyPattern.emplace("armorRequirement",
				*pEligibility->Find("armorRequirement"));
			LegacyPattern.emplace("phaseRequirement",
				*pEligibility->Find("phaseRequirement"));
			LegacyPattern.emplace("invulnerableWhileRunning",
				*GameplayPattern.Find("invulnerableWhileRunning"));
			LegacyPattern.emplace("selectionWeight", DATA_JSON_VALUE::Number(
				pCompatibilityWeight->Get_Number()));
			LegacyPattern.emplace("maximumConsecutiveUses",
				bManual ? DATA_JSON_VALUE::Number(0.0) :
					*pRepeatPolicy->Find("limit"));
			LegacyPattern.emplace("minimumRange",
				*pEligibility->Find("minimumRangeM"));
			LegacyPattern.emplace("maximumRange",
				*pEligibility->Find("maximumRangeM"));
			LegacyPattern.emplace("cameraCueIds", Build_StringArray(CameraCueIds));
			LegacyPattern.emplace("worldEventTriggerRefs",
				DATA_JSON_VALUE::Array(std::move(WorldReferences)));
			LegacyPattern.emplace("stages",
				DATA_JSON_VALUE::Array(std::move(LegacyStages)));
			LegacyPatterns.push_back(
				DATA_JSON_VALUE::Object(std::move(LegacyPattern)));
		}

		// 독립 Effect의 선언 ID를 실제 cue 또는 spawn owner와 연결한다.
		// 전투 오브젝트 쪽 임시 asset/visual 값은 이후 Product owner 결합에서 실제 값과 대조한다.
		DATA_JSON_VALUE::ARRAY LegacyIndependent;
		for (const auto& [IndependentId, Declaration] : IndependentById)
		{
			DATA_JSON_VALUE::OBJECT Legacy;
			Legacy.emplace("independentEffectId",
				DATA_JSON_VALUE::String(IndependentId));
			Legacy.emplace("displayName",
				DATA_JSON_VALUE::String(Declaration.strDisplayName));
			Legacy.emplace("ownership",
				DATA_JSON_VALUE::String(Declaration.strOwnership));
			if ("SERVER_PATTERN_STAGE" == Declaration.strOwnership)
			{
				const auto Owner = CueOwners.find(Declaration.strReferenceId);
				if (Owner == CueOwners.end() || nullptr == Owner->second.pCue)
				{
					strOutError = "split independent Effect cue did not resolve exactly: " +
						IndependentId;
					return false;
				}
				const DATA_JSON_VALUE& Cue = *Owner->second.pCue;
				Legacy.emplace("effectAssetId", *Cue.Find("effectAssetId"));
				Legacy.emplace("ownerPatternId",
					DATA_JSON_VALUE::String(Owner->second.strPatternId));
				Legacy.emplace("ownerStageId",
					DATA_JSON_VALUE::String(Owner->second.strStageId));
				Legacy.emplace("triggerPolicy",
					DATA_JSON_VALUE::String("PATTERN_TIMELINE"));
				Legacy.emplace("combatObjectArchetypeId", DATA_JSON_VALUE::Null());
				Legacy.emplace("clientVisualId", DATA_JSON_VALUE::Null());
				Legacy.emplace("effectCueBindingId",
					DATA_JSON_VALUE::String(Declaration.strReferenceId));
				DATA_JSON_VALUE::OBJECT Projection;
				if (nullptr != Cue.Find("timingBasis"))
				{
					Projection.emplace("timingBasis", *Cue.Find("timingBasis"));
					Projection.emplace("stageOffsetMs", *Cue.Find("stageOffsetMs"));
					if (const auto* end = Cue.Find("stageEndMs"))
						Projection.emplace("stageEndMs", *end);
				}
				else
				{
					Projection.emplace("clipOccurrenceId",
						*Cue.Find("clipOccurrenceId"));
					Projection.emplace("sourceStartMs", *Cue.Find("sourceStartMs"));
					Projection.emplace("sourceEndMs", *Cue.Find("sourceEndMs"));
					Projection.emplace("mappingBasis", *Cue.Find("mappingBasis"));
				}
				Legacy.emplace("cueProjection",
					DATA_JSON_VALUE::Object(std::move(Projection)));
			}
			else
			{
				const auto Owner = SpawnOwners.find(Declaration.strReferenceId);
				if (Owner == SpawnOwners.end())
				{
					strOutError = "split independent Effect spawn did not resolve exactly: " +
						IndependentId;
					return false;
				}
				Legacy.emplace("effectAssetId", DATA_JSON_VALUE::String(
					"pending.effect." + IndependentId));
				Legacy.emplace("ownerPatternId",
					DATA_JSON_VALUE::String(Owner->second.strPatternId));
				Legacy.emplace("ownerStageId",
					DATA_JSON_VALUE::String(Owner->second.strStageId));
				Legacy.emplace("triggerPolicy",
					DATA_JSON_VALUE::String("STAGE_ENTER_PER_ALIVE_PLAYER"));
				Legacy.emplace("combatObjectArchetypeId",
					DATA_JSON_VALUE::String(
						Owner->second.strCombatObjectArchetypeId));
				Legacy.emplace("clientVisualId", DATA_JSON_VALUE::String(
					"pending.visual." + IndependentId));
				Legacy.emplace("effectCueBindingId", DATA_JSON_VALUE::Null());
				Legacy.emplace("cueProjection", DATA_JSON_VALUE::Null());
			}
			LegacyIndependent.push_back(
				DATA_JSON_VALUE::Object(std::move(Legacy)));
		}

		// 분리 previewPaths 중 기존 Master reader가 요구하는 경로만 복사한다.
		// combatObjectAuthoring/worldEventSets/legacyCompatibility는 이 호환 객체의 필드가 아니다.
		DATA_JSON_VALUE::OBJECT LegacyPreview;
		for (const std::string& Field :
			{ "encounter", "animationBindings", "effectCues", "combatObjects",
			  "bossCatalog", "effectCatalog", "damageProfiles", "cinematicCamera",
			  "worldEvents", "patternRotations", "sourceClipSequences" })
		{
			LegacyPreview.emplace(Field, *pPreviewPaths->Find(Field));
		}
		// 기존 Master 검증을 재사용할 메모리 JSON을 만든다. 디스크의 이전 단일 Master 파일을 저장하거나 게시하지 않는다.
		DATA_JSON_VALUE::OBJECT LegacyRoot;
		LegacyRoot.emplace("schema",
			DATA_JSON_VALUE::String("lostark.valtan-pattern-master"));
		LegacyRoot.emplace("formatVersion", DATA_JSON_VALUE::Number(1));
		LegacyRoot.emplace("bossArchetypeId", *Gameplay.Find("bossArchetypeId"));
		LegacyRoot.emplace("encounterId", *Gameplay.Find("encounterId"));
		LegacyRoot.emplace("scope", *Gameplay.Find("scope"));
		LegacyRoot.emplace("previewPaths",
			DATA_JSON_VALUE::Object(std::move(LegacyPreview)));
		LegacyRoot.emplace("retiredPatternIds", *Gameplay.Find("retiredPatternIds"));
		LegacyRoot.emplace("normalSelection", std::move(NormalSelection));
		LegacyRoot.emplace("counterReactionLayers",
			*Gameplay.Find("counterReactionLayers"));
		LegacyRoot.emplace("independentEffects",
			DATA_JSON_VALUE::Array(std::move(LegacyIndependent)));
		LegacyRoot.emplace("patterns",
			DATA_JSON_VALUE::Array(std::move(LegacyPatterns)));

		// 여기부터 Out이 직접 채워진다. Master 검증 뒤에도 sequence 참조·입장 연출·anchor 검사가
		// 실패할 수 있으므로 이 성공을 전체 결과의 최종 commit으로 해석하지 않는다.
		if (!Parse_MasterDocument(
				DATA_JSON_VALUE::Object(std::move(LegacyRoot)),
				ScriptedEntryOnlyPatternIds, Out, strOutError))
		{
			return false;
		}

		// sequence의 모든 pattern ID가 실제 Master에 들어왔는지 확인한다.
		// HEALTH_BAR_ROTATIONS의 별도 입장 연출은 지정된 NORMAL·무적·NONE target/aim 계약을 추가로 요구한다.
		for (const std::string& strPatternId : ScriptedSequence.PatternIds)
		{
			const bool_t bExists = std::any_of(
				Out.Patterns.begin(), Out.Patterns.end(),
				[&strPatternId](const MASTER_PATTERN& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (!bExists)
			{
				strOutError =
					"split gameplay scriptedSequence pattern is missing from Master: " +
					strPatternId;
				return false;
			}
		}
		if (ScriptedSequence.strMode == "HEALTH_BAR_ROTATIONS" &&
			!ScriptedSequence.strEntranceCinematicPatternId.empty())
		{
			const auto Entrance = std::find_if(Out.Patterns.begin(), Out.Patterns.end(),
				[&ScriptedSequence](const MASTER_PATTERN& Pattern)
				{ return Pattern.strPatternId == ScriptedSequence.strEntranceCinematicPatternId; });
			if (ScriptedSequence.strEntranceCinematicPatternId != CINEMATIC_ENTRY_PATTERN_ID ||
				Entrance == Out.Patterns.end() || Entrance->strCategory != "NORMAL" ||
				Entrance->strTargetPolicy != "NONE" || Entrance->strAimPolicy != "NONE" ||
				!Entrance->bInvulnerableWhileRunning)
			{
				strOutError = "split gameplay entrance cinematic requires the managed invulnerable NORMAL entrance with NONE target/aim";
				return false;
			}
		}

		// 분리 문서 전용 선택/기믹/수동/volley 정보를 Out에 옮긴 뒤 각 패턴을 보충한다.
		// 뒤의 cue anchor나 시간 예산 검사 실패가 이 대입을 되돌리지는 않는다.
		Out.SelectionSets = std::move(SelectionSets);
		Out.SelectionWindows = std::move(SelectionWindows);
		Out.Mechanics = std::move(MechanicViews);
		Out.ManualAuditions = std::move(ManualViews);
		Out.ScriptedSequence = std::move(ScriptedSequence);
		Out.VolleyProjections = std::move(VolleyProjections);
		for (size_t iPattern = 0u; iPattern < Out.Patterns.size(); ++iPattern)
		{
			MASTER_PATTERN& Pattern = Out.Patterns[iPattern];

			// 수동 시험 패턴은 앞서 모은 metadata를 복사해 출처 chain과 저작 phase/admission을 표시한다.
			// 실제 Server 실행 승인을 여기서 요청하거나 확정하지 않는다.
			const auto Manual = ManualAuditions.find(Pattern.strPatternId);
			if (Manual != ManualAuditions.end())
			{
				Pattern.bManualServerAudition = true;
				Pattern.strSourceAnimationChainId =
					Manual->second.strSourceChainId;
				Pattern.iAuthoringPhase = Manual->second.iAuthoringPhase;
				Pattern.strAdmissionState =
					Manual->second.strAdmissionState;
			}
			const DATA_JSON_VALUE& PresentationPattern =
				pPresentationPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& GameplayPattern =
				pGameplayPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& PresentationStages =
				*PresentationPattern.Find("stages");
			const DATA_JSON_VALUE& GameplayStages =
				*GameplayPattern.Find("stages");
			for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
			{
				MASTER_STAGE& Stage = Pattern.Stages[iStage];
				const DATA_JSON_VALUE& PresentationStage =
					PresentationStages.Get_Array()[iStage];
				const DATA_JSON_VALUE& GameplayStage =
					GameplayStages.Get_Array()[iStage];
				const DATA_JSON_VALUE& Cues =
					*PresentationStage.Find("effectCues");
				// target/landing/arena anchor가 요구하는 snapshot/follow와 gameplay 대상 또는 이동 계약을 확인한다.
				for (const DATA_JSON_VALUE& Cue : Cues.Get_Array())
				{
					const std::string Anchor = Read_String(Cue, "anchorSlotId");
					if (Anchor.starts_with("pattern.target."))
					{
						const bool_t bLockedTarget =
							Pattern.strTargetPolicy == "LOCK_NEAREST_ON_START" ||
							Pattern.strTargetPolicy == "LOCK_RANDOM_ALIVE_ON_START" ||
							Pattern.strTargetPolicy ==
								"LOCK_RANDOM_ALIVE_BEHIND_ON_START";
						if (Anchor != "pattern.target.snapshot" ||
							Read_String(Cue, "followPolicy") != "snapshot" ||
							!bLockedTarget)
						{
							strOutError =
								"split target cue requires its exact snapshot/locked-target contract: " +
								Pattern.strPatternId + "/" + Stage.strStageId;
							return false;
						}
					}
					else if (Anchor.starts_with("pattern.landing."))
					{
						if (Anchor != "pattern.landing.snapshot" ||
							Read_String(Cue, "followPolicy") != "snapshot" ||
							!Pattern.ServerMotion.has_value() ||
							(Pattern.ServerMotion->strKind != "LEAP_TO_ANCHOR" &&
							 Pattern.ServerMotion->strKind != "LEAP_TO_TARGET"))
						{
							strOutError = "split landing cue requires snapshot follow and a Server leap: " +
								Pattern.strPatternId + "/" + Stage.strStageId;
							return false;
						}
					}

					// arena 중심 anchor는 고정 중심·고정 방향·대상 추종별로 허용 이동과 조준 정책이 다르다.
					// 대상 추종의 stage 경로는 현재 stage까지 TO_ARENA_CENTER가 있었는지와 현재 aim 정책을 함께 본다.
					else if (Anchor.starts_with("arena.center"))
					{
						const bool_t bFixedFacing =
							"arena.center.facing" == Anchor;
						const bool_t bTargetFollow =
							"arena.center.target-follow" == Anchor;
						const bool_t bFixedCenter = "arena.center" == Anchor;
						const bool_t bHasFixedCenterMotion =
							Pattern.ServerMotion.has_value() &&
							("LEAP_TO_ANCHOR" == Pattern.ServerMotion->strKind ||
							 "LEAP_TO_TARGET" == Pattern.ServerMotion->strKind);
						const bool_t bHasCenterApproach =
							Pattern.ServerMotion.has_value() &&
							"LEAP_TO_ANCHOR" == Pattern.ServerMotion->strKind &&
							Pattern.ServerMotion->bMoveToAnchorBeforeTakeoff;
						const bool_t bStageCenterFollow =
							("NEAREST_EACH_TICK" == Stage.strAimTargetPolicy ||
							 "PATTERN_TARGET" == Stage.strAimTargetPolicy) &&
							std::any_of(Pattern.Stages.begin(), Pattern.Stages.begin() + iStage + 1u,
								[](const MASTER_STAGE& Candidate) {
									return Candidate.Motion.has_value() &&
										"TO_ARENA_CENTER" == Candidate.Motion->strKind;
								});
						if (("arena.center" != Anchor && !bFixedFacing &&
							 !bTargetFollow) ||
							(bFixedCenter && !bHasFixedCenterMotion) ||
							(!bFixedCenter && !bHasCenterApproach &&
							 !(bTargetFollow && bStageCenterFollow)) ||
							Read_String(Cue, "followPolicy") !=
								(bTargetFollow ? "follow" : "snapshot") ||
							(bFixedFacing &&
							 (Pattern.strAimPolicy != "LOCK_FACING_ON_START" ||
							  Pattern.strTargetPolicy != "LOCK_RANDOM_ALIVE_ON_START")) ||
							(bTargetFollow && !bStageCenterFollow &&
							 (Pattern.strAimPolicy != "TRACK_TARGET_EACH_TICK" ||
							  Pattern.strTargetPolicy != "LOCK_RANDOM_ALIVE_ON_START")))
						{
							strOutError = "split arena center cue requires its exact fixed/follow approach contract: " +
								Pattern.strPatternId + "/" + Stage.strStageId;
							return false;
						}
					}

					// 현재 cue의 anchor 계약을 통과하면 값 형태의 AuthoredCue를 stage에 추가한다.
					// 이전 cue가 이미 추가된 상태에서 다음 cue가 실패할 수 있으므로 Out 전체는 여전히 후보 상태다.
					Stage.AuthoredCues.push_back(Build_SplitCueView(
						Cue,
						Pattern.strPatternId,
						Stage.strStageId,
						Stage.strActionId,
						static_cast<uint32_t>(GameplayStage.Find(
							"durationMs")->Get_Number())));
				}
			}
		}
		// 모든 stage의 애니메이션 시간 예산을 임시 stage로 계산해 본다. 여기서 계산한 벽시계 값은 원본 JSON을 수정하지 않는다.
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			for (const MASTER_STAGE& SourceStage : Pattern.Stages)
			{
				Client::VALTAN_STAGE_VIEW Stage;
				Stage.strActionId = SourceStage.strActionId;
				Stage.iDurationMs = SourceStage.iDurationMs;
				Stage.bSuppressAnimation = SourceStage.bSuppressAnimation;
				Stage.strAnimationEndPolicy = SourceStage.strAnimationEndPolicy;
				Stage.ClipOccurrences = SourceStage.Occurrences;
				if (!Assign_MasterWallBudgets(Stage, strOutError))
				{
					strOutError = "split presentation/Server stage wall join failed: " +
						strOutError;
					return false;
				}
			}
		}
		return true;
	}

	// 복원 로드는 값 변경을 허용하되 패턴/action/source ID와 stage 개수 및 순서의 identity는 유지해야 한다.
	bool_t Has_StableRestoreTopology(
		const Client::VALTAN_PATTERN_VIEW& Product,
		const MASTER_PATTERN& Master)
	{
		return Product.strPatternId == Master.strPatternId &&
			Product.strActionId == Master.strActionId &&
			Product.SourceActionIds == Master.SourceActionIds &&
			Product.Is_Gimmick() == (0 != Master.iTriggerHealthBar) &&
			Product.strSelectionMode == Master.strSelectionMode &&
			Product.Stages.size() == Master.Stages.size() &&
			std::equal(Product.Stages.begin(), Product.Stages.end(),
				Master.Stages.begin(),
				[](const Client::VALTAN_STAGE_VIEW& ProductStage,
					const MASTER_STAGE& MasterStage)
				{
					return ProductStage.strStageId == MasterStage.strStageId &&
						ProductStage.strActionId == MasterStage.strActionId;
				});
	}

	// 복원 정책에서 topology가 맞는 Product 뷰에 저작 gameplay 값을 덮어쓴다.
	// stage 순서와 개수가 같다는 선행 검사를 전제로 하며 임시 View를 대상으로 호출한다.
	void Overlay_MasterGameplay(
		const MASTER_PATTERN& Master,
		Client::VALTAN_PATTERN_VIEW& Out)
	{
		Out.strCategory = Master.strCategory;
		Out.iMinimumPhase = Master.iMinimumPhase;
		Out.iMaximumPhase = Master.iMaximumPhase;
		Out.strTargetPolicy = Master.strTargetPolicy;
		Out.strAimPolicy = Master.strAimPolicy;
		Out.strDisplayName = Master.strDisplayName;
		Out.iMinimumHealthBar = Master.iMinimumHealthBar;
		Out.iMaximumHealthBar = Master.iMaximumHealthBar;
		Out.iTriggerHealthBar = Master.iTriggerHealthBar;
		Out.iTriggerOrder = Master.iTriggerOrder;
		Out.strArmorRequirement = Master.strArmorRequirement;
		Out.strPhaseRequirement = Master.strPhaseRequirement;
		Out.bInvulnerableWhileRunning = Master.bInvulnerableWhileRunning;
		Out.iSelectionWeight = Master.iSelectionWeight;
		Out.iMaximumConsecutiveUses = Master.iMaximumConsecutiveUses;
		Out.fMinimumRange = Master.fMinimumRange;
		Out.fMaximumRange = Master.fMaximumRange;
		Out.fVerticalOffsetM = Master.fVerticalOffsetM;
		Out.ServerMotion = Master.ServerMotion;
		Out.Finale = Master.Finale;
		for (size_t iStage = 0u; iStage < Master.Stages.size(); ++iStage)
		{
			const MASTER_STAGE& Source = Master.Stages[iStage];
			Client::VALTAN_STAGE_VIEW& Stage = Out.Stages[iStage];
			Stage.strStageKind = Source.strStageKind;
			Stage.iDurationMs = Source.iDurationMs;
			Stage.fVerticalOffsetM = Source.fVerticalOffsetM;
			Stage.strHitShape = Source.strHitShape;
			Stage.fHitOuterRadius = Source.fHitOuterRadius;
			Stage.fHitInnerRadius = Source.fHitInnerRadius;
			Stage.fHitAngleDegrees = Source.fHitAngleDegrees;
			Stage.fHitLength = Source.fHitLength;
			Stage.fHitHalfWidth = Source.fHitHalfWidth;
			Stage.iHitCount = Source.iHitCount;
			Stage.iHitIntervalMs = Source.iHitIntervalMs;
			Stage.iHitDelayMs = Source.iHitDelayMs;
			Stage.HitOffsetsMs = Source.HitOffsetsMs;
			Stage.AttackContacts = Source.AttackContacts;
			Stage.bHasHitAnchor = Source.bHasHitAnchor;
			Stage.strHitAnchorKind = Source.strHitAnchorKind;
			Stage.fHitAnchorForwardOffsetM = Source.fHitAnchorForwardOffsetM;
			Stage.fHitAnchorRightOffsetM = Source.fHitAnchorRightOffsetM;
			Stage.fHitAnchorYawOffsetDegrees =
				Source.fHitAnchorYawOffsetDegrees;
			Stage.bHasHitActivation = Source.bHasHitActivation;
			Stage.iHitActivationStartMs = Source.iHitActivationStartMs;
			Stage.iHitActivationLifetimeMs =
				Source.iHitActivationLifetimeMs;
			Stage.strServerDamageProfileId = Source.strServerDamageProfileId;
			Stage.strPlayerResponse = Source.strPlayerResponse;
			Stage.strAttachmentSlot = Source.strAttachmentSlot;
			Stage.GripLocalOffset = Source.GripLocalOffset;
			Stage.strPartDamagePolicy = Source.strPartDamagePolicy;
			Stage.CounterProxy = Source.CounterProxy;
			Stage.BossResponse = Source.BossResponse;
			Stage.fPushRangeM = Source.fPushRangeM;
			Stage.iPushMs = Source.iPushMs;
			Stage.bKnockdown = Source.bKnockdown;
			Stage.iDownMs = Source.iDownMs;
			Stage.strAimTargetPolicy = Source.strAimTargetPolicy;
			Stage.bHasAimEnd = Source.bHasAimEnd;
			Stage.iAimEndMs = Source.iAimEndMs;
			Stage.bHasAimResponseScale = Source.bHasAimResponseScale;
			Stage.fAimResponseScale = Source.fAimResponseScale;
			Stage.Motion = Source.Motion;
			Stage.Actions = Source.Actions;
			Stage.Branches = Source.Branches;
			Stage.bHasBodyHiddenWindow = Source.bHasBodyHiddenWindow;
			Stage.iBodyHiddenFromMs = Source.iBodyHiddenFromMs;
			Stage.iBodyHiddenToMs = Source.iBodyHiddenToMs;
		}
	}

	// 선택 진단 출력이 있으면 실패 종류와 거절된 패턴/stage ID를 함께 기록한다. null 진단은 허용한다.
	void Set_CanonicalReadFailure(
		Client::VALTAN_CANONICAL_READ_DIAGNOSTIC* const pDiagnostic,
		const Client::VALTAN_CANONICAL_READ_FAILURE_KIND eFailure,
		const std::string& strStatus,
		const std::string_view strPatternId = {},
		const std::string_view strStageId = {})
	{
		if (nullptr == pDiagnostic)
			return;
		pDiagnostic->eFailure = eFailure;
		pDiagnostic->strStatus = strStatus;
		pDiagnostic->strRejectedPatternId.assign(strPatternId);
		pDiagnostic->strRejectedStageId.assign(strStageId);
	}

	// 선택 V2 binding 원본을 읽어 각 stage에 정확한 pattern/stage/action 범위의 binding이 있는지 표시한다.
	// 파일이 없으면 성공하며 Effect 실제 재생이나 리소스 내용 검증을 대신하지 않는다.
	// 두 Load 경로가 지역 Master를 완성할 때 호출한다. 선택 파일이 없으면 기존 stage 표식을 유지하고
	// 성공한다. 파일을 읽은 경우 STAGE 시계와 pattern/stage/action·시작 범위가 맞는 binding 존재로 표식을 갱신한다.
	bool_t Join_StageEffectV2SourceScopes(
		MASTER_DOCUMENT& Master, std::string& strOutError)
	{
		const auto path = Client::CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") /
			L"V2" / L"Bindings" / L"BOSS_VALTAN.effectv2bindings.json");
		std::error_code error;
		if (!std::filesystem::exists(path, error) && !error)
			return true;

		// 파일 존재 확인 오류는 성공으로 처리하지 않고 실제 읽기를 이어 시도한다.
		// 읽기/파싱 실패는 아직 Master를 건드리지 않은 시점이며 하위 reader의 오류 문자열을 그대로 반환한다.
		std::string text;
		std::vector<Client::EFFECT_V2_BINDING> bindings;
		if (!Read_TextDocument(path, text, strOutError) ||
			!Client::CEffectV2Document::Parse_Bindings(
				text, "BOSS_VALTAN", bindings, strOutError))
			return false;

		for (auto& pattern : Master.Patterns)
			for (auto& stage : pattern.Stages)
				stage.bHasStageEffectV2Bindings = std::any_of(
					bindings.begin(), bindings.end(),
					[&](const auto& binding)
					{
						return binding.eClockBasis == Client::EFFECT_V2_CLOCK_BASIS::STAGE &&
							binding.strPatternId == pattern.strPatternId &&
							binding.strStageId == stage.strStageId &&
							binding.strActionId == stage.strActionId &&
							binding.iStartMs < stage.iDurationMs;
					});

		return true;
	}

	// 게시 Product로 만든 임시 View를 저작 Master와 결합한다. strict 정책은 계약 값을 대조하고 restore 정책은 topology 확인 후 저작 값을 반영한다.
	// 함수 도중 View가 바뀔 수 있으므로 호출자가 소유한 최종 뷰에 직접 적용하지 않는다.
	// 퇴역/빈 수동 패턴, 순서/선택 coverage와 독립 Effect owner를 모두 확인한 뒤 상위 Load가 최종 교체한다.
	bool_t Apply_MasterDocument(
		const MASTER_DOCUMENT& Master,
		const DATA_JSON_VALUE& PatternRotations,
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		std::string& strOutError,
		const Client::VALTAN_PATTERN_TREE_LOAD_POLICY ePolicy,
		Client::VALTAN_CANONICAL_READ_DIAGNOSTIC* const pOutDiagnostic)
	{
		// 게시 값 불일치는 상태 문자열과 선택 진단에 STALE_PROJECTION을 함께 남긴다.
		// 일부 기본 형식 실패는 아래에서 문자열만 채우며 상위 Load가 남은 진단 종류를 보충한다.
		const auto RejectStaleProjection =
			[&strOutError, pOutDiagnostic](
				std::string strError,
				const std::string_view strPatternId = {},
				const std::string_view strStageId = {})
			{
				strOutError = std::move(strError);
				Set_CanonicalReadFailure(
					pOutDiagnostic,
					Client::VALTAN_CANONICAL_READ_FAILURE_KIND::
						STALE_PROJECTION,
					strOutError, strPatternId, strStageId);
				return false;
			};

		// strict 대조와 저장 원본 복원 두 정책만 허용한다.
		// 둘 다 같은 임시 View를 결합하며 이 함수가 파일이나 현재 실행 패턴을 교체하지 않는다.
		if (Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				REQUIRE_ACTIVE_PRODUCT_PARITY != ePolicy &&
			Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				RESTORE_AUTHORING_SNAPSHOT != ePolicy)
		{
			strOutError = "Valtan pattern-tree load policy is invalid";
			return false;
		}
		const bool_t bRequireProductParity =
			Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				REQUIRE_ACTIVE_PRODUCT_PARITY == ePolicy;

		// 폐기된 저작 ID가 게시 View에 남으면 두 정책 모두 거부한다.
		// 복원 정책도 이미 퇴역한 실행 정의를 되살리는 경로는 아니다.
		for (const std::string& strRetiredPatternId : Master.RetiredPatternIds)
		{
			if (nullptr != Find_Pattern(View, strRetiredPatternId))
			{
				return RejectStaleProjection(
					"retired master pattern still exists in Product: " +
						strRetiredPatternId,
					strRetiredPatternId);
			}
		}

		// Master 소유의 독립 Effect를 지역 포인터 색인으로 차용한다.
		// stage 참조를 모은 뒤 전체 선언 coverage와 실제 Product owner를 대조한다.
		std::map<std::string,
			const Client::VALTAN_INDEPENDENT_EFFECT_VIEW*, std::less<>>
			IndependentById;
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			IndependentById.emplace(
				Independent.strIndependentEffectId, &Independent);
		}
		std::set<std::string, std::less<>> ReferencedIndependentIds;
		std::set<std::string, std::less<>> UnboundManualPatternIds;

		for (const MASTER_PATTERN& MasterPattern : Master.Patterns)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(
				View, MasterPattern.strPatternId);
			if (MasterPattern.bManualServerAudition &&
				MasterPattern.SourceActionIds.empty() &&
				MasterPattern.PresentationSources.empty() &&
				std::all_of(MasterPattern.Stages.begin(), MasterPattern.Stages.end(),
					[](const auto& Stage) { return Stage.Occurrences.empty() && Stage.EffectReferences.empty() &&
						!Stage.bHasStageEffectV2Bindings && (Stage.strHitShape.empty() || Stage.strHitShape == "NONE"); }))
			{
				// 원본 연결, 애니메이션, Effect와 hit가 모두 비어 있는 수동 패턴은 Product에 없어야 한다.
				// 이 빈 저작 행은 게시본 결합과 재생 목록에서 제외하며 임의의 실행 정의를 만들지 않는다.
				if (nullptr != pPattern)
					return RejectStaleProjection(
						"unbound manual pattern unexpectedly exists in Product: " +
							MasterPattern.strPatternId, MasterPattern.strPatternId);
				UnboundManualPatternIds.insert(MasterPattern.strPatternId);
				continue;
			}

			// strict는 패턴 gameplay 값을 비교하고 restore는 ID·source action·stage 개수/순서를 확인한다.
			// 두 helper의 검증 범위는 같지 않으며, restore에서만 뒤의 gameplay 덮어쓰기를 수행한다.
			if (nullptr == pPattern ||
				(bRequireProductParity &&
				 !Equal_MasterPatternGameplay(*pPattern, MasterPattern)) ||
				(!bRequireProductParity &&
				 !Has_StableRestoreTopology(*pPattern, MasterPattern)))
			{
				if (bRequireProductParity)
				{
					return RejectStaleProjection(
						"master/Product pattern projection changed: " +
							MasterPattern.strPatternId,
						MasterPattern.strPatternId);
				}
				strOutError = "saved authoring pattern topology changed: " +
					MasterPattern.strPatternId;
				return false;
			}
			if (!bRequireProductParity)
				Overlay_MasterGameplay(MasterPattern, *pPattern);

			// 두 정책 모두 저작 출처·카메라·월드 참조와 관리 표시를 임시 패턴에 복사한다.
			// 이 뒤 stage 검증이 실패하더라도 이미 복사한 값을 자체적으로 되돌리지는 않는다.
			pPattern->iSourceSequenceIndex =
				MasterPattern.iSourceSequenceIndex;
			pPattern->PresentationSources = MasterPattern.PresentationSources;
			pPattern->Reactions = MasterPattern.Reactions;
			pPattern->CameraCueIds = MasterPattern.CameraCueIds;
			pPattern->WorldEventTriggerRefs =
				MasterPattern.WorldEventTriggerRefs;
			pPattern->bAuthoringMasterManaged = true;
			pPattern->strEntryActionId = MasterPattern.Stages.empty() ?
				std::string{} : MasterPattern.Stages.front().strActionId;
			pPattern->bManualServerAudition =
				MasterPattern.bManualServerAudition;
			pPattern->strSourceAnimationChainId =
				MasterPattern.strSourceAnimationChainId;
			pPattern->iAuthoringPhase = MasterPattern.iAuthoringPhase;
			pPattern->strAdmissionState = MasterPattern.strAdmissionState;

			// Master의 stage 순서로 같은 index의 Product stage를 사용한다.
			// 현재 strict 경로의 패턴 비교에는 stage 수 대조가 없으므로 이 접근을 크기 검증 완료로 설명할 수 없다.
			for (size_t iStage = 0u; iStage < MasterPattern.Stages.size();
				++iStage)
			{
				const MASTER_STAGE& MasterStage = MasterPattern.Stages[iStage];
				Client::VALTAN_STAGE_VIEW& Stage = pPattern->Stages[iStage];
				if (bRequireProductParity &&
					(!Equal_MasterStageGameplay(Stage, MasterStage) ||
					 Stage.bSuppressAnimation !=
						MasterStage.bSuppressAnimation ||
					 Stage.bHasBodyHiddenWindow !=
						MasterStage.bHasBodyHiddenWindow ||
					 Stage.iBodyHiddenFromMs !=
						MasterStage.iBodyHiddenFromMs ||
					 Stage.iBodyHiddenToMs !=
						MasterStage.iBodyHiddenToMs ||
					 Stage.ClipOccurrences.size() !=
						MasterStage.Occurrences.size() ||
					 Stage.ProductCues.size() != MasterStage.AuthoredCues.size()))
				{
					return RejectStaleProjection(
						"master/Product stage projection changed: " +
							MasterPattern.strPatternId + "/" +
							MasterStage.strStageId,
						MasterPattern.strPatternId, MasterStage.strStageId);
				}

				// strict에서는 occurrence 수를 위에서 맞춘 뒤 같은 순서의 애니메이션 값을 비교한다.
				// cue는 정렬 순서가 바뀔 수 있어 아래의 binding ID 검색으로 따로 대조한다.
				if (bRequireProductParity)
				{
					for (size_t iClip = 0u;
						iClip < MasterStage.Occurrences.size(); ++iClip)
					{
						if (!Equal_MasterOccurrence(
								Stage.ClipOccurrences[iClip],
								MasterStage.Occurrences[iClip]))
						{
							return RejectStaleProjection(
								"master/Product animation occurrence changed: " +
									MasterStage.Occurrences[iClip].
										strClipOccurrenceId,
								MasterPattern.strPatternId,
								MasterStage.strStageId);
						}
					}
					// Product cue는 재생 순서로 정렬될 수 있으므로 저작 배열의 순번으로 짝짓지 않는다.
					// 두 입력에서 중복을 거절한 binding ID로 찾고 저작 필드를 대조한다.
					for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& AuthoredCue :
						MasterStage.AuthoredCues)
					{
						const auto ProductCue = std::find_if(
							Stage.ProductCues.begin(), Stage.ProductCues.end(),
							[&AuthoredCue](const auto& Cue)
							{ return Cue.strBindingId == AuthoredCue.strBindingId; });
						if (Stage.ProductCues.end() == ProductCue ||
							!Equal_AuthoredCue(*ProductCue, AuthoredCue))
						{
							return RejectStaleProjection(
								"master/Product Effect cue changed: " +
									AuthoredCue.strBindingId,
								MasterPattern.strPatternId,
								MasterStage.strStageId);
						}
					}
				}
				else
				{
					// restore에서는 저작 occurrence와 cue를 임시 stage에 다시 채운다.
					// 대표 clip/cue는 첫 항목이고 각 occurrence의 cue 목록은 clipOccurrenceId로 재연결한다.
					Stage.ClipOccurrences = MasterStage.Occurrences;
					Stage.RuntimeClipNames.clear();
					for (const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Stage.RuntimeClipNames.push_back(Occurrence.strClipName);
					}
					Stage.strRuntimeClipName = Stage.RuntimeClipNames.empty() ?
						std::string{} : Stage.RuntimeClipNames.front();
					Stage.ProductCues = MasterStage.AuthoredCues;
					Stage.ProductCue = Stage.ProductCues.empty() ?
						std::optional<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW>{} :
						std::optional<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW>{
							Stage.ProductCues.front() };
					for (Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Occurrence.ProductCues.clear();
						for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
							Stage.ProductCues)
						{
							if (Cue.strClipOccurrenceId ==
								Occurrence.strClipOccurrenceId)
							{
								Occurrence.ProductCues.push_back(Cue);
							}
						}
					}
				}

				// 공통 표현 상태·환경/조명·카메라를 보충하고 occurrence별 벽시계 예산을 다시 계산한다.
				// 계산 실패가 앞선 View 변경까지 rollback하지 않으므로 상위의 지역 Staged가 실패를 격리한다.
				Stage.bHasStageEffectV2Bindings = MasterStage.bHasStageEffectV2Bindings;
				Stage.strSequenceRole = MasterStage.strSequenceRole;
				Stage.iAuthoringRepeatCount = MasterStage.iRepeatCount;
				Stage.strAnimationEndPolicy =
					MasterStage.strAnimationEndPolicy;
				Stage.bSuppressAnimation = MasterStage.bSuppressAnimation;
				Stage.bHasBodyHiddenWindow =
					MasterStage.bHasBodyHiddenWindow;
				Stage.iBodyHiddenFromMs = MasterStage.iBodyHiddenFromMs;
				Stage.iBodyHiddenToMs = MasterStage.iBodyHiddenToMs;
				Stage.CameraInvocations = MasterStage.CameraInvocations;
				Stage.SceneProfileOccurrences = MasterStage.SceneProfileOccurrences;
				Stage.LightOccurrences = MasterStage.LightOccurrences;
				if (!Assign_MasterWallBudgets(Stage, strOutError))
					return false;

				// 일반 cue 참조는 현재 stage의 binding ID와 시계·시작·선택적 끝 값이 일치해야 한다.
				// 독립 참조는 선언 owner의 pattern/stage를 맞춘 뒤 stage 목록과 전체 참조 집합에 추가한다.
				for (const MASTER_EFFECT_REFERENCE& Reference :
					MasterStage.EffectReferences)
				{
					if ("CUE_BINDING" == Reference.strType)
					{
						const bool_t bFound = std::any_of(
							Stage.ProductCues.begin(), Stage.ProductCues.end(),
							[&Reference](
								const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
							{
								return Cue.strBindingId == Reference.strId &&
									Cue.bUsesStageClock ==
										Reference.bUsesStageClock &&
									(Reference.bUsesStageClock ?
										(Cue.iStageOffsetMs ==
										 Reference.iStageOffsetMs && Cue.bHasSourceEnd == Reference.bHasSourceEnd &&
										(!Reference.bHasSourceEnd || Cue.iSourceEndMs == Reference.iSourceEndMs)) :
										(Cue.strClipOccurrenceId ==
											Reference.strClipOccurrenceId &&
										 Cue.iSourceStartMs ==
											Reference.iSourceStartMs &&
										 Cue.bHasSourceEnd ==
											Reference.bHasSourceEnd &&
										 (!Reference.bHasSourceEnd ||
										  Cue.iSourceEndMs ==
											Reference.iSourceEndMs)));
							});
						if (!bFound)
						{
							return RejectStaleProjection(
								"master cue reference left its Product stage: " +
									Reference.strId,
								MasterPattern.strPatternId,
								MasterStage.strStageId);
						}
						continue;
					}
					const auto Independent = IndependentById.find(Reference.strId);
					if (Independent == IndependentById.end() ||
						Independent->second->strOwnerPatternId !=
							pPattern->strPatternId ||
						Independent->second->strOwnerStageId != Stage.strStageId)
					{
						return RejectStaleProjection(
							"master independent Effect reference is stale: " +
								Reference.strId,
							MasterPattern.strPatternId,
							MasterStage.strStageId);
					}
					Stage.IndependentEffectIds.push_back(Reference.strId);
					ReferencedIndependentIds.insert(Reference.strId);
				}
			}
		}

		// 게시 rotation 문서의 v4 형태와 scripted sequence를 읽는다.
		// 복원 정책에서도 이 기본 형태·패턴 존재 검사는 건너뛰지 않으며 값 동등성 검사만 뒤에서 정책에 따라 나뉜다.
		const DATA_JSON_VALUE* pProductScriptedSequence = Required(
			PatternRotations, "scriptedSequence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pProductScriptedPatterns =
			nullptr == pProductScriptedSequence ? nullptr : Required(
				*pProductScriptedSequence, "patternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pProductTransitionPursuit =
			nullptr == pProductScriptedSequence ? nullptr :
				pProductScriptedSequence->Find("transitionPursuitMs");
		const DATA_JSON_VALUE* pRotationRows = Required(
			PatternRotations, "rotations", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactProperties(PatternRotations,
				{ "schema", "formatVersion", "encounterId", "bossArchetypeId",
				  "scriptedSequence", "rotations" }) ||
			nullptr == Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-rotations" != Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(PatternRotations.Find("formatVersion")) ||
			4.0 != PatternRotations.Find("formatVersion")->Get_Number() ||
			nullptr == pProductScriptedSequence ||
			!Has_ExactPropertiesWithOptional(*pProductScriptedSequence,
				{ "sequenceId", "mode", "interStepPursuitMs", "patternIds" },
				{ "transitionPursuitMs", "entranceCinematicPatternId" }) ||
			(nullptr != pProductScriptedSequence->Find("entranceCinematicPatternId") &&
			 (!pProductScriptedSequence->Find("entranceCinematicPatternId")->Is_String() ||
			  !Is_StableToken(Read_String(*pProductScriptedSequence, "entranceCinematicPatternId")))) ||
			!Is_StableToken(Read_String(
				*pProductScriptedSequence, "sequenceId")) ||
			("ORDERED_ONCE_THEN_IDLE" != Read_String(*pProductScriptedSequence, "mode") &&
			 "HEALTH_BAR_ROTATIONS" != Read_String(*pProductScriptedSequence, "mode")) ||
			!Is_NonNegativeInteger(
				pProductScriptedSequence->Find("interStepPursuitMs")) ||
			pProductScriptedSequence->Find(
				"interStepPursuitMs")->Get_Number() < 100.0 ||
			pProductScriptedSequence->Find(
				"interStepPursuitMs")->Get_Number() > 10000.0 ||
			nullptr == pProductScriptedPatterns ||
			pProductScriptedPatterns->Get_Array().empty() ||
			pProductScriptedPatterns->Get_Array().size() >
				Client::CValtanPatternFlowDocument::MAX_SLOTS ||
			nullptr == pRotationRows)
		{
			strOutError =
				"Valtan selection-window/scripted-sequence Product v4 is invalid";
			return false;
		}

		// sequence의 각 ID가 현재 View에 존재하는지 확인하고 배열 순서를 보존한다.
		// 같은 패턴의 반복을 지우지 않으므로 뒤의 strict 배열 비교도 순서와 반복을 포함한다.
		std::vector<std::string> ProductScriptedPatternIds;
		for (const DATA_JSON_VALUE& PatternId :
			pProductScriptedPatterns->Get_Array())
		{
			const Client::VALTAN_PATTERN_VIEW* pPattern = PatternId.Is_String() ?
				Find_Pattern(View, PatternId.Get_String()) : nullptr;
			if (!PatternId.Is_String() ||
				!Is_StableToken(PatternId.Get_String()) ||
				nullptr == pPattern)
			{
				strOutError =
					"Valtan scripted-sequence Product pattern is invalid";
				return false;
			}
			ProductScriptedPatternIds.push_back(PatternId.Get_String());
		}

		// 구간별 추적 시간이 없으면 공통 interStepPursuitMs를 패턴 사이마다 복제한다.
		// 명시 배열은 패턴 수보다 하나 적어야 하며 각 값의 0ms도 허용한다.
		std::vector<uint32_t> ProductTransitionPursuitMs;
		if (nullptr == pProductTransitionPursuit)
		{
			ProductTransitionPursuitMs.assign(
				ProductScriptedPatternIds.size() - 1u,
				static_cast<uint32_t>(pProductScriptedSequence->Find(
					"interStepPursuitMs")->Get_Number()));
		}
		else
		{
			if (DATA_JSON_TYPE::ARRAY != pProductTransitionPursuit->Get_Type() ||
				pProductTransitionPursuit->Get_Array().size() + 1u !=
					ProductScriptedPatternIds.size())
			{
				strOutError =
					"Valtan scripted-sequence Product transition pursuits are invalid";
				return false;
			}
			for (const DATA_JSON_VALUE& PursuitMs :
				pProductTransitionPursuit->Get_Array())
			{
				if (!Is_NonNegativeInteger(&PursuitMs) ||
					PursuitMs.Get_Number() > 10000.0)
				{
					strOutError =
						"Valtan scripted-sequence Product transition pursuit is invalid";
					return false;
				}
				ProductTransitionPursuitMs.push_back(
					static_cast<uint32_t>(PursuitMs.Get_Number()));
			}
		}
		// 편집 가능한 순서 정본은 gameplay의 scriptedSequence이고 rotations는 게시된 결과다.
		// 엄격한 읽기에서는 sequence identity, 시간, 패턴 순서와 구간별 추적 시간을 모두 대조한다.
		if (bRequireProductParity &&
			(Read_String(*pProductScriptedSequence, "sequenceId") !=
				Master.ScriptedSequence.strSequenceId ||
			 Read_String(*pProductScriptedSequence, "mode") !=
				Master.ScriptedSequence.strMode ||
			 Read_String(*pProductScriptedSequence, "entranceCinematicPatternId") !=
				Master.ScriptedSequence.strEntranceCinematicPatternId ||
			 static_cast<uint32_t>(pProductScriptedSequence->Find(
					"interStepPursuitMs")->Get_Number()) !=
					Master.ScriptedSequence.iInterStepPursuitMs ||
				 ProductScriptedPatternIds !=
					Master.ScriptedSequence.PatternIds ||
				 ProductTransitionPursuitMs !=
					Master.ScriptedSequence.TransitionPursuitMs))
		{
			return RejectStaleProjection(
				"Valtan scripted-sequence Product parity drifted");
		}

		// 정책별 대조 뒤 표시 View의 sequence 값은 항상 저작 Master에서 복사한다.
		// 복원 때도 게시 문서를 새로 저장하는 작업은 아니다.
		View.strScriptedSequenceId = Master.ScriptedSequence.strSequenceId;
		View.strScriptedSequenceMode = Master.ScriptedSequence.strMode;
		View.strEntranceCinematicPatternId = Master.ScriptedSequence.strEntranceCinematicPatternId;
		View.iScriptedSequenceInterStepPursuitMs =
			Master.ScriptedSequence.iInterStepPursuitMs;
		View.ScriptedSequencePatternIds = Master.ScriptedSequence.PatternIds;
		View.ScriptedSequenceTransitionPursuitMs =
			Master.ScriptedSequence.TransitionPursuitMs;

		// 저작 window를 게시 rotation ID에, selection set을 set ID에 연결하는 차용 색인이다.
		// 아래 게시 행 검사 동안 Master의 vector를 변경하지 않아 포인터를 유지한다.
		std::map<std::string, const Client::VALTAN_SELECTION_WINDOW_VIEW*,
			std::less<>> WindowByRotation;
		for (const Client::VALTAN_SELECTION_WINDOW_VIEW& Window :
			Master.SelectionWindows)
		{
			WindowByRotation.emplace(Window.strCompatibilityRotationId, &Window);
		}
		std::map<std::string, const Client::VALTAN_SELECTION_SET_VIEW*,
			std::less<>> SetById;
		for (const Client::VALTAN_SELECTION_SET_VIEW& Set : Master.SelectionSets)
			SetById.emplace(Set.strSelectionSetId, &Set);
		struct LEGACY_ROTATION_IDENTITY
		{
			std::string_view strRotationId;
			uint32_t iFromHealthBar;
			uint32_t iToHealthBar;
		};

		// 관리 window 뒤에 허용할 참조용 legacy rotation의 ID·체력 범위·순서다.
		// 실제 개수 요구는 끝에서 sequence mode에 따라 0개 또는 이 여섯 개로 나뉜다.
		static constexpr LEGACY_ROTATION_IDENTITY LegacyRotationOrder[] = {
			{ "rotation.valtan.109.100", 108u, 100u },
			{ "rotation.valtan.100.84", 99u, 84u },
			{ "rotation.valtan.84.73", 83u, 73u },
			{ "rotation.valtan.73.62", 72u, 62u },
			{ "rotation.valtan.62.30", 61u, 30u },
			{ "rotation.valtan.28.14", 29u, 14u },
		};
		static constexpr size_t iLegacyRotationCount =
			sizeof(LegacyRotationOrder) / sizeof(LegacyRotationOrder[0]);
		std::set<std::string, std::less<>> ProductRotationIds;
		std::set<std::string, std::less<>> SeenManagedRotations;
		size_t iManagedRotationOrdinal = 0u;
		size_t iLegacyRotationOrdinal = 0u;

		// 이전 legacy 표시 목록을 지우고 게시 행을 순서대로 읽는다.
		// 이후 실패하면 일부만 다시 채워진 상태이므로 최종 사용자 View가 아니라 임시 View에 적용한다.
		View.LegacyRotations.clear();
		for (const DATA_JSON_VALUE& Row : pRotationRows->Get_Array())
		{
			const std::string strRotationId = Read_String(Row, "rotationId");
			if (!Is_StableToken(strRotationId) ||
				!ProductRotationIds.insert(strRotationId).second)
			{
				strOutError = "Valtan Product rotation identity is invalid/duplicated";
				return false;
			}

			// 관리 rotation은 Master window와 같은 순서로 먼저 나와야 한다.
			// legacy 행을 하나라도 처리한 뒤 관리 행이 다시 등장하면 두 정책 모두 거부한다.
			const auto Managed = WindowByRotation.find(strRotationId);
			if (Managed != WindowByRotation.end())
			{
				if (0u != iLegacyRotationOrdinal ||
					iManagedRotationOrdinal >= Master.SelectionWindows.size() ||
					strRotationId != Master.SelectionWindows[
						iManagedRotationOrdinal].strCompatibilityRotationId)
				{
					return RejectStaleProjection(
						"Valtan managed Product rotation order drifted");
				}
				++iManagedRotationOrdinal;

				// ORDERED_LOOP는 배열 크기와 stable ID를 읽고 원래 순서·반복을 유지한다.
				// window/set/phase/체력 경계와의 정확한 값 일치는 strict에서만 대조한다.
				if ("ORDERED_LOOP" == Read_String(Row, "selectionMode"))
				{
					const auto& Window = *Managed->second;
					const auto Set = SetById.find(Window.strSelectionSetId);
					const auto* pMembers = Required(Row, "patternIds", DATA_JSON_TYPE::ARRAY);
					if (!Has_ExactProperties(Row, {"rotationId", "selectionMode", "fromHealthBar", "toHealthBar",
						"windowId", "gameplayPhase", "selectionSetId", "patternIds"}) ||
						nullptr == pMembers || pMembers->Get_Array().empty() || pMembers->Get_Array().size() > 32u)
					{
						strOutError = "Valtan ordered loop Product shape is invalid";
						return false;
					}
					std::vector<std::string> Members;
					for (const auto& Member : pMembers->Get_Array())
					{
						if (!Member.Is_String() || !Is_StableToken(Member.Get_String()))
						{
							strOutError = "Valtan ordered loop Product occurrence is invalid";
							return false;
						}
						Members.push_back(Member.Get_String());
					}
					if (bRequireProductParity && (Set == SetById.end() || "ORDERED_LOOP" != Set->second->strMode ||
						Members != Set->second->PatternIds || Read_String(Row, "windowId") != Window.strWindowId ||
						Read_String(Row, "selectionSetId") != Window.strSelectionSetId ||
						!Is_NonNegativeInteger(Row.Find("gameplayPhase")) || !Is_NonNegativeInteger(Row.Find("fromHealthBar")) ||
						!Is_NonNegativeInteger(Row.Find("toHealthBar")) ||
						Row.Find("gameplayPhase")->Get_Number() != Window.iGameplayPhase ||
						Row.Find("fromHealthBar")->Get_Number() != Window.iMaximumHealthBarInclusive ||
						Row.Find("toHealthBar")->Get_Number() != Window.iMinimumHealthBarExclusive))
						return RejectStaleProjection("Valtan ordered loop Product parity drifted: " + strRotationId);
					SeenManagedRotations.insert(strRotationId);
					continue;
				}

				// 그 외 관리 행은 candidates 필드가 있는 v4 형태여야 한다.
				// restore는 필드 목록과 coverage까지만 확인하고, strict는 아래에서 모드·window·후보 값까지 대조한다.
				if (!Has_ExactProperties(Row,
						{ "rotationId", "selectionMode", "fromHealthBar",
						  "toHealthBar", "windowId", "gameplayPhase",
						  "selectionSetId", "candidates" }))
				{
					strOutError = "Valtan managed Product row is not the v4 tagged shape";
					return false;
				}
				SeenManagedRotations.insert(strRotationId);
				if (!bRequireProductParity)
					continue;
				const Client::VALTAN_SELECTION_WINDOW_VIEW& Window =
					*Managed->second;
				const auto Set = SetById.find(Window.strSelectionSetId);
				const DATA_JSON_VALUE* pCandidates = Required(
					Row, "candidates", DATA_JSON_TYPE::ARRAY);
				if (Set == SetById.end() ||
					Read_String(Row, "selectionMode") != Set->second->strMode ||
					Read_String(Row, "windowId") != Window.strWindowId ||
					Read_String(Row, "selectionSetId") != Window.strSelectionSetId ||
					!Is_NonNegativeInteger(Row.Find("gameplayPhase")) ||
					!Is_NonNegativeInteger(Row.Find("fromHealthBar")) ||
					!Is_NonNegativeInteger(Row.Find("toHealthBar")) ||
					Window.iGameplayPhase != static_cast<uint32_t>(
						Row.Find("gameplayPhase")->Get_Number()) ||
					Window.iMaximumHealthBarInclusive != static_cast<uint32_t>(
						Row.Find("fromHealthBar")->Get_Number()) ||
					Window.iMinimumHealthBarExclusive != static_cast<uint32_t>(
						Row.Find("toHealthBar")->Get_Number()) ||
					nullptr == pCandidates || pCandidates->Get_Array().size() !=
						Set->second->Candidates.size())
				{
					return RejectStaleProjection(
						"Valtan managed Product window/set projection drifted: " +
							strRotationId);
				}

				// strict 가중치 후보는 동일 index의 ID·weight·enabled를 비교한다.
				// 비활성 행도 제외하지 않으므로 전체 후보 배열과 게시 순서가 일치해야 한다.
				for (size_t i = 0u; i < Set->second->Candidates.size(); ++i)
				{
					const DATA_JSON_VALUE& ProductCandidate =
						pCandidates->Get_Array()[i];
					const Client::VALTAN_SELECTION_CANDIDATE_VIEW& Candidate =
						Set->second->Candidates[i];
					const DATA_JSON_VALUE* pEnabled = Required(
						ProductCandidate, "enabled", DATA_JSON_TYPE::BOOLEAN);
					if (!Has_ExactProperties(ProductCandidate,
							{ "patternId", "weight", "enabled" }) ||
						Read_String(ProductCandidate, "patternId") !=
							Candidate.strPatternId ||
						!Is_NonNegativeInteger(ProductCandidate.Find("weight")) ||
						Candidate.iWeight != static_cast<uint32_t>(
							ProductCandidate.Find("weight")->Get_Number()) ||
						nullptr == pEnabled ||
						Candidate.bEnabled != pEnabled->Get_Boolean())
					{
						return RejectStaleProjection(
							"Valtan managed Product candidate projection drifted: " +
								strRotationId,
							Candidate.strPatternId);
					}
				}
				continue;
			}

			// legacy 행은 모든 관리 window 뒤에서 고정 ID/체력 구간 순서를 따라야 한다.
			// 각 patternId의 현재 View 존재도 검사하고 legacy 표시 목록에는 원본 순서로 보관한다.
			if (iManagedRotationOrdinal != Master.SelectionWindows.size() ||
				iLegacyRotationOrdinal >= iLegacyRotationCount ||
				strRotationId != LegacyRotationOrder[
					iLegacyRotationOrdinal].strRotationId)
			{
				return RejectStaleProjection(
					"Valtan legacy Product rotation order/identity drifted");
			}
			const LEGACY_ROTATION_IDENTITY& ExpectedLegacy =
				LegacyRotationOrder[iLegacyRotationOrdinal];
			const DATA_JSON_VALUE* pPatternIds = Required(
				Row, "patternIds", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactProperties(Row,
					{ "rotationId", "selectionMode", "fromHealthBar",
					  "toHealthBar", "patternIds" }) ||
				!Is_NonNegativeInteger(Row.Find("fromHealthBar")) ||
				!Is_NonNegativeInteger(Row.Find("toHealthBar")) ||
				nullptr == pPatternIds || pPatternIds->Get_Array().empty() ||
				"ORDERED_INTRO_THEN_WEIGHTED" != Read_String(Row, "selectionMode") ||
				ExpectedLegacy.iFromHealthBar != static_cast<uint32_t>(
					Row.Find("fromHealthBar")->Get_Number()) ||
				ExpectedLegacy.iToHealthBar != static_cast<uint32_t>(
					Row.Find("toHealthBar")->Get_Number()))
			{
				strOutError = "Valtan legacy Product rotation shape is invalid";
				return false;
			}
			Client::VALTAN_LEGACY_ROTATION_VIEW Legacy;
			Legacy.strRotationId = strRotationId;
			Legacy.strSelectionMode = Read_String(Row, "selectionMode");
			Legacy.iFromHealthBar = static_cast<uint32_t>(
				Row.Find("fromHealthBar")->Get_Number());
			Legacy.iToHealthBar = static_cast<uint32_t>(
				Row.Find("toHealthBar")->Get_Number());
			for (const DATA_JSON_VALUE& PatternId : pPatternIds->Get_Array())
			{
				if (!PatternId.Is_String() ||
					!Is_StableToken(PatternId.Get_String()) ||
					nullptr == Find_Pattern(View, PatternId.Get_String()))
				{
					strOutError = "Valtan legacy Product rotation patternId is invalid";
					return false;
				}
				Legacy.PatternIds.push_back(PatternId.Get_String());
			}
			View.LegacyRotations.push_back(std::move(Legacy));
			++iLegacyRotationOrdinal;
		}

		// 관리 window coverage와 legacy 개수를 마무리한다.
		// HEALTH_BAR_ROTATIONS이면 legacy가 없어야 하며, 다른 허용 mode는 고정 여섯 행을 모두 요구한다.
		if (SeenManagedRotations.size() != Master.SelectionWindows.size() ||
			iManagedRotationOrdinal != Master.SelectionWindows.size() ||
			iLegacyRotationOrdinal != ("HEALTH_BAR_ROTATIONS" == Master.ScriptedSequence.strMode ? 0u : iLegacyRotationCount))
		{
			return RejectStaleProjection(
				"Valtan Product selection-window/legacy rotation coverage is incomplete");
		}

		// 저작 선택/기믹/수동 목록을 View에 복사한 뒤 빈 수동 패턴을 제외한다.
		// NORMAL 목록은 관리 여부와 모드를, 수동 목록은 그 조건에 더해 출처 metadata까지 확인한다.
		View.SelectionSets = Master.SelectionSets;
		View.SelectionWindows = Master.SelectionWindows;
		View.Mechanics = Master.Mechanics;
		View.ManualAuditions = Master.ManualAuditions;
		std::erase_if(View.ManualAuditions,
			[&UnboundManualPatternIds](const auto& Manual)
			{ return UnboundManualPatternIds.contains(Manual.strPatternId); });
		View.NormalSelection = Master.NormalSelection;
		for (const std::string& PatternId : View.NormalSelection.PatternIds)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(View, PatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				"NORMAL" != pPattern->strSelectionMode)
			{
				return RejectStaleProjection(
					"Valtan normal-selection pool left the managed normal set: " +
						PatternId,
					PatternId);
			}
		}
		for (const Client::VALTAN_MANUAL_AUDITION_VIEW& Manual :
			View.ManualAuditions)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(
				View, Manual.strPatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				!pPattern->bManualServerAudition ||
				"AUDITION_ONLY" != pPattern->strSelectionMode ||
				pPattern->strSourceAnimationChainId != Manual.strSourceChainId ||
				pPattern->iAuthoringPhase != Manual.iAuthoringPhase ||
				pPattern->strAdmissionState != Manual.strAdmissionState)
			{
				return RejectStaleProjection(
					"Valtan manual audition left its managed Product pattern: " +
						Manual.strPatternId,
					Manual.strPatternId);
			}
		}

		// 관리 패턴 밖의 counterable ENTER stage를 Product에서 모은다.
		// 아래 참조용 counter reaction owner 집합과 정확히 일치해야 하며 관리 gameplay counter는 여기서 제외한다.
		std::set<std::string, std::less<>> ProductCounterOwnerStages;
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			for (Client::VALTAN_PATTERN_VIEW& Pattern : *pGroup)
			{
				// counterReactionLayers는 REFERENCE_ONLY_LEGACY 범위다.
				// 분리 원본이 소유하는 counter는 해당 gameplay stage에서 검증하므로 기존 참조 전용 집합에 추가하지 않는다.
				if (Pattern.bAuthoringMasterManaged)
					continue;
				for (Client::VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					const bool_t bCounterable = std::any_of(
						Stage.Actions.begin(), Stage.Actions.end(),
						[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
						{
							return "ENTER" == Action.strTrigger &&
								"SET_BOSS_FLAG" == Action.strKind &&
								"boss.flag.counterable" == Action.strTargetId &&
								1.f == Action.fValue;
						});
					if (bCounterable)
						ProductCounterOwnerStages.insert(
							Pattern.strPatternId + "/" + Stage.strStageId);
				}
			}
		}
		std::set<std::string, std::less<>> MasterCounterOwnerStages;
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& MasterLayer :
			Master.CounterReactionLayers)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, MasterLayer.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pWindow = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, MasterLayer.strOwnerStageId);

			// 같은 owner 패턴에서 action ID에 해당하는 stage 하나를 찾는다.
			// 없거나 중복이면 nullptr이므로 성공/실패 경로를 임의의 첫 stage에 연결하지 않는다.
			const auto FindActionStage = [pOwnerPattern](
				const std::string& ActionId) -> Client::VALTAN_STAGE_VIEW*
			{
				if (nullptr == pOwnerPattern)
					return nullptr;
				Client::VALTAN_STAGE_VIEW* pFound = nullptr;
				for (Client::VALTAN_STAGE_VIEW& Stage : pOwnerPattern->Stages)
				{
					if (Stage.strActionId != ActionId)
						continue;
					if (nullptr != pFound)
						return nullptr;
					pFound = &Stage;
				}
				return pFound;
			};
			Client::VALTAN_STAGE_VIEW* pSuccess = FindActionStage(
				MasterLayer.Success.strActionId);
			Client::VALTAN_STAGE_VIEW* pFailure = FindActionStage(
				MasterLayer.Failure.strActionId);

			// owner window의 ENTER 1/EXIT 0 flag와 COUNTER_HIT/TIMEOUT 대상 존재를 대조한다.
			// window·성공·실패 stage 모두 재생 occurrence가 있어야 참조용 layer로 연결한다.
			const bool_t bHasCounterEnter = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "ENTER" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						1.f == Action.fValue;
				});
			const bool_t bHasCounterExit = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "EXIT" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						0.f == Action.fValue;
				});
			const auto HasBranch = [pWindow](
				const std::string_view Outcome, const std::string& ActionId)
			{
				return nullptr != pWindow && std::any_of(
					pWindow->Branches.begin(), pWindow->Branches.end(),
					[Outcome, &ActionId](
						const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{
						return Branch.strOutcome == Outcome &&
							Branch.strNextActionId.has_value() &&
							*Branch.strNextActionId == ActionId;
					});
			};
			if (nullptr == pOwnerPattern || pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pWindow || pWindow->strActionId !=
					MasterLayer.Window.strActionId ||
				nullptr == pSuccess || nullptr == pFailure ||
				pWindow->ClipOccurrences.empty() ||
				pSuccess->ClipOccurrences.empty() ||
				pFailure->ClipOccurrences.empty() ||
				!bHasCounterEnter || !bHasCounterExit ||
				!HasBranch("COUNTER_HIT", MasterLayer.Success.strActionId) ||
				!HasBranch("TIMEOUT", MasterLayer.Failure.strActionId))
			{
				return RejectStaleProjection(
					"reference-only counter reaction Product join changed: " +
						MasterLayer.strReactionLayerId,
					MasterLayer.strOwnerPatternId,
					MasterLayer.strOwnerStageId);
			}

			// 참조용 Master layer에 실제 Product의 세 occurrence 목록을 복사해 보관한다.
			// 그 뒤 전체 owner 집합을 비교하므로 부분 layer 추가 뒤에도 실패할 수 있다.
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer = MasterLayer;
			Layer.Window.ClipOccurrences = pWindow->ClipOccurrences;
			Layer.Success.ClipOccurrences = pSuccess->ClipOccurrences;
			Layer.Failure.ClipOccurrences = pFailure->ClipOccurrences;
			View.CounterReactionLayers.push_back(std::move(Layer));
			MasterCounterOwnerStages.insert(
				MasterLayer.strOwnerPatternId + "/" +
				MasterLayer.strOwnerStageId);
		}
		if (ProductCounterOwnerStages != MasterCounterOwnerStages)
		{
			return RejectStaleProjection(
				"counter reaction master does not cover exact Product counter stages");
		}

		// 모든 독립 Effect 선언이 어느 stage에서 참조됐는지 coverage를 확인한다.
		// 이어서 owner stage가 실제 관리 패턴에 있고 소유 종류별 Product 값이 맞는지 검사한다.
		if (ReferencedIndependentIds.size() != Master.IndependentEffects.size())
		{
			return RejectStaleProjection(
				"master independent Effect inventory contains an unreferenced identity");
		}
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, Independent.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pOwnerStage = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, Independent.strOwnerStageId);
			if (nullptr == pOwnerPattern || !pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pOwnerStage)
			{
				return RejectStaleProjection(
					"master independent Effect owner is stale: " +
						Independent.strIndependentEffectId,
					Independent.strOwnerPatternId,
					Independent.strOwnerStageId);
			}

			// 전투 오브젝트는 asset/archetype/visual을, stage 소유 cue는 asset/binding/시계 범위를 대조한다.
			// 여기서 any_of는 일치하는 항목의 존재를 검사하며 일치 항목 수를 다시 세지는 않는다.
			bool_t bOwnerJoined = false;
			if ("SERVER_COMBAT_OBJECT" == Independent.strOwnership)
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->CombatObjectEffects.begin(),
					pOwnerStage->CombatObjectEffects.end(),
					[&Independent](
						const Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect)
					{
						return Effect.strEffectAssetId ==
								Independent.strEffectAssetId &&
							Effect.strCombatObjectArchetypeId ==
								Independent.strCombatObjectArchetypeId &&
							Effect.strClientVisualId ==
								Independent.strClientVisualId;
					});
			}
			else
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->ProductCues.begin(),
					pOwnerStage->ProductCues.end(),
					[&Independent](
						const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
					{
						return Cue.strEffectAssetId == Independent.strEffectAssetId &&
							Cue.strBindingId == Independent.strEffectCueBindingId &&
							Independent.bHasCueProjection &&
							(Cue.bUsesStageClock ==
								Independent.bUsesCueStageClock) &&
							(Independent.bUsesCueStageClock ?
								(Cue.iStageOffsetMs ==
									Independent.iCueStageOffsetMs && Cue.bHasSourceEnd == Independent.bHasCueSourceEnd &&
									(!Independent.bHasCueSourceEnd || Cue.iSourceEndMs == Independent.iCueSourceEndMs)) :
								(Cue.strClipOccurrenceId ==
									Independent.strCueClipOccurrenceId &&
								 Cue.iSourceStartMs ==
									Independent.iCueSourceStartMs &&
								 Cue.bHasSourceEnd ==
									Independent.bHasCueSourceEnd &&
								 (!Independent.bHasCueSourceEnd ||
								  Cue.iSourceEndMs ==
									Independent.iCueSourceEndMs)));
					});
			}
			// 독립 Effect는 하나의 Product owner와 일반 Effect 문서 파일이 있어야 한다.
			// 여기서는 파일 존재를 확인하며 Effect 문서 내용의 파싱이나 GPU 표시 성공을 보장하지 않는다.
			const std::filesystem::path EffectDocument =
				Client::CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") / L"Authored" /
					std::filesystem::path(
						Independent.strEffectAssetId + ".effect.json"));
			std::error_code FileError;
			if (!bOwnerJoined || EffectDocument.empty() ||
				!std::filesystem::is_regular_file(EffectDocument, FileError))
			{
				return RejectStaleProjection(
					"master independent Effect did not resolve to one Product owner/document: " +
						Independent.strIndependentEffectId,
					Independent.strOwnerPatternId,
					Independent.strOwnerStageId);
			}
		}

		// 전체 독립 Effect 결합과 문서 존재 검사가 끝나면 그 목록을 View에 복사한다.
		// 이 대입만 최종 단계이며 View의 다른 필드는 앞서 이미 변경됐다.
		View.IndependentEffects = Master.IndependentEffects;
		return true;
	}

	// 뷰의 follow enum을 기존 cue 문서에서 쓰는 문자열로 돌린다. 알 수 없는 값은 빈 문자열이다.
	std::string Describe_FollowPolicy(
		const Client::EFFECT_FOLLOW_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_FOLLOW_POLICY::FOLLOW:
			return "follow";
		case Client::EFFECT_FOLLOW_POLICY::SNAPSHOT:
			return "snapshot";
		default:
			return {};
		}
	}

	// 뷰의 stop enum을 문서 정책 문자열로 돌린다. 알 수 없는 값은 빈 문자열이다.
	std::string Describe_StopPolicy(
		const Client::EFFECT_STOP_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_STOP_POLICY::NATURAL:
			return "natural";
		case Client::EFFECT_STOP_POLICY::CUE_END:
			return "cue_end";
		default:
			return {};
		}
	}

	// cue 반복 정책을 문서 문자열로 돌린다. 알 수 없는 값은 빈 문자열이다.
	std::string Describe_RepeatPolicy(
		const Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE:
			return "once";
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP:
			return "each_loop";
		default:
			return {};
		}
	}

	// cue scale 정책을 문서 문자열로 돌린다. 알 수 없는 값은 빈 문자열이다.
	std::string Describe_ScalePolicy(
		const Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE:
			return "OWNER_RELATIVE";
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT:
			return "GAMEPLAY_FOOTPRINT";
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE:
			return "ARENA_ABSOLUTE";
		default:
			return {};
		}
	}
}

// admission이 소유한 내부 상태를 파괴하며 RAII 잠금과 핸들도 함께 해제한다.
Client::CValtanCanonicalProductReadAdmission::
	~CValtanCanonicalProductReadAdmission()
{
	delete static_cast<VALTAN_CANONICAL_PRODUCT_READ_STATE*>(m_pState);
	m_pState = nullptr;
}

// 문자열 상태만 필요한 호출자를 위한 adapter다. 상세 실패 분류는 typed overload가 담당한다.
bool_t Client::CValtanCanonicalProductReadAdmission::Acquire(
	std::string& strOutStatus)
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bAcquired = Acquire(Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bAcquired;
}

// Load나 도구의 묶음 읽기가 공유 admission을 얻는다. 잠금과 journal 확인을 모두 통과한 State만 멤버로 넘긴다.
// 실패한 임시 unique_ptr은 자동 정리하며 이미 보유한 admission을 다시 획득하지 않는다.
bool_t Client::CValtanCanonicalProductReadAdmission::Acquire(
	VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic)
{
	OutDiagnostic.Clear();
	if (nullptr != m_pState)
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_STATE_INVALID;
		OutDiagnostic.strStatus =
			"Valtan canonical Product read admission is already held.";
		return false;
	}
	std::unique_ptr<VALTAN_CANONICAL_PRODUCT_READ_STATE> State =
		std::make_unique<VALTAN_CANONICAL_PRODUCT_READ_STATE>();
	State->ProjectRoot = CProjectDataRoot::Get().parent_path();
	std::string AdmissionError;
	VALTAN_CANONICAL_READ_FAILURE_KIND eFailure =
		VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_IO;
	if (!State->Lock.Try_Acquire(
			State->ProjectRoot, AdmissionError, eFailure) ||
		!Require_NoActiveValtanPatternGeneration(
			State->ProjectRoot, AdmissionError, eFailure))
	{
		OutDiagnostic.eFailure = eFailure;
		OutDiagnostic.strStatus =
			"Valtan canonical Product read admission failed: " +
			AdmissionError;
		return false;
	}
	m_pState = State.release();
	OutDiagnostic.strStatus =
		"Valtan canonical Product generation admitted for a shared read.";
	return true;
}

// 호출자가 재시도를 판단할 일시 실패만 분류한다. 실제 대기나 재시도 실행은 호출자 책임이다.
bool_t Client::CValtanCanonicalProductReadAdmission::Is_TransientFailure(
	const VALTAN_CANONICAL_READ_FAILURE_KIND eFailure)
{
	return VALTAN_CANONICAL_READ_FAILURE_KIND::WRITER_BUSY == eFailure ||
		VALTAN_CANONICAL_READ_FAILURE_KIND::GENERATION_CHANGED == eFailure;
}

// 재확인의 상세 진단 중 상태 문자열을 전달하는 adapter다.
bool_t Client::CValtanCanonicalProductReadAdmission::Validate_StillCurrent(
	std::string& strOutStatus) const
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bCurrent = Validate_StillCurrent(Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bCurrent;
}

// 보유한 admission의 journal 부재를 다시 확인한다. 파일별 hash나 revision 대조는 하지 않는다.
// 읽는 사이 journal이 생긴 경우 GENERATION_CHANGED로 알려 호출자가 임시 결과를 버리게 한다.
bool_t Client::CValtanCanonicalProductReadAdmission::Validate_StillCurrent(
	VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic) const
{
	OutDiagnostic.Clear();
	const auto* const pState =
		static_cast<const VALTAN_CANONICAL_PRODUCT_READ_STATE*>(m_pState);
	if (nullptr == pState)
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_STATE_INVALID;
		OutDiagnostic.strStatus =
			"Valtan canonical Product read admission is not held.";
		return false;
	}
	std::string AdmissionError;
	VALTAN_CANONICAL_READ_FAILURE_KIND eFailure =
		VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_IO;
	if (!Require_NoActiveValtanPatternGeneration(
			pState->ProjectRoot, AdmissionError, eFailure))
	{
		OutDiagnostic.eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::
				WRITER_RECOVERY_REQUIRED == eFailure ?
			VALTAN_CANONICAL_READ_FAILURE_KIND::GENERATION_CHANGED : eFailure;
		OutDiagnostic.strStatus =
			"Valtan canonical Product read admission became stale: " +
			AdmissionError;
		return false;
	}
	OutDiagnostic.strStatus =
		"Valtan canonical Product generation remained stable through the read.";
	return true;
}

// 두 패턴 목록의 stage 개수를 합친다. 화면용 Phase에 중복 연결된 index는 다시 세지 않는다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_StageCount() const
{
	size_t iCount = 0u;
	for (const VALTAN_PATTERN_VIEW& Pattern : Gimmicks)
		iCount += Pattern.Stages.size();
	for (const VALTAN_PATTERN_VIEW& Pattern : Rotation)
		iCount += Pattern.Stages.size();
	return iCount;
}

// Effect가 있는 stage의 수다. 한 stage의 여러 Effect 문서 개수와 구분한다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_Effect();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// stage Effect와 전투 오브젝트 Effect 행을 합산한다. asset ID를 전역 중복 제거한 고유 파일 수는 아니다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectDocumentCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				iCount += Stage.Effects.size();
				iCount += Stage.CombatObjectEffects.size();
			}
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// 애니메이션 binding이 있는 stage 수를 센다. 실제 clip 리소스 로드 성공을 검사하지 않는다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipBoundStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ClipBinding();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// Product cue가 연결된 stage 수를 센다. cue가 여러 개인 stage도 한 번 센다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ProductCue();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// 저작 occurrence를 모두 센다. 같은 clip 이름을 반복한 occurrence도 각각 별도 행이다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipOccurrenceCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ClipOccurrences.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// stage에 연결된 Product cue 행을 모두 센다. Effect asset ID의 고유 개수와는 다르다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ProductCues.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// stage에 연결한 전투 오브젝트 Effect 행의 합계다. 실제 생성한 오브젝트 수가 아니다.
size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_CombatObjectEffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.CombatObjectEffects.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

// 세 종류의 재생 목록에 해당 stable pattern ID가 있는지 조회한다. 문서나 Server 상태를 바꾸지 않는다.
bool_t Client::VALTAN_TOOL_AUDITION_INVENTORY::Contains(
	const std::string& strPatternId) const
{
	if (strPatternId.empty())
		return false;
	const auto ContainsIn = [&strPatternId](
		const std::vector<std::string>& PatternIds)
	{
		return PatternIds.end() != std::find(
			PatternIds.begin(), PatternIds.end(), strPatternId);
	};
	return ContainsIn(CorePatternIds) || ContainsIn(AnimatorPatternIds) ||
		ContainsIn(DerivedPatternIds);
}

// 원본 복원 clip 문서를 asset ID별 색인으로 읽는다. 임시 색인을 검증한 뒤 교체하므로 실패 시 기존 색인은 유지된다.
// 편집용 시간 재정의는 호출자가 명시한 경우에만 적용하고 원본 대조 기본 경로와 구분한다.
bool_t Client::CValtanPatternTree::Load_FullRestoreSourceClips(
	const std::filesystem::path& Path,
	VALTAN_FULL_RESTORE_CLIP_INDEX& OutIndex, std::string& OutError,
	const bool_t bUseAuthoredPreview)
{
	using namespace Client;
	std::ifstream Input(Path, std::ios::binary);
	if (!Input)
	{
		OutError = "Full Restore source animation index could not be read.";
		return false;
	}
	const std::string Text{ std::istreambuf_iterator<char>(Input), {} };
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Text, Root, OutError))
		return false;
	const auto StringIs = [](const DATA_JSON_VALUE* Value, std::string_view Expected)
		{
			return Value && Value->Is_String() && Value->Get_String() == Expected;
		};
	const auto Unsigned = [](const DATA_JSON_VALUE* Value, uint32_t& Out)
	{
		if (!Value || !Value->Is_Number() || !std::isfinite(Value->Get_Number()) ||
			Value->Get_Number() < 0.0 || Value->Get_Number() > UINT32_MAX ||
			std::floor(Value->Get_Number()) != Value->Get_Number())
			return false;
		Out = static_cast<uint32_t>(Value->Get_Number());
		return true;
	};
	uint32_t Version = 0u;
	const auto* Effects = Root.Find("effects");
	if (!StringIs(Root.Find("schema"), "lostark.valtan-full-restore-animations") ||
		!StringIs(Root.Find("bossArchetypeId"), "BOSS_VALTAN") ||
		!Unsigned(Root.Find("formatVersion"), Version) || Version != 1u ||
		!Effects || !Effects->Is_Array())
	{
		OutError = "Full Restore source animation index has an invalid schema.";
		return false;
	}
	VALTAN_FULL_RESTORE_CLIP_INDEX Staged;
	OutError = "Full Restore source animation index has an invalid action/stage/clip.";
	for (const auto& Effect : Effects->Get_Array())
	{
		uint32_t Action = 0u, Stage = 0u;
		const auto* Clips = Effect.Find("animationClips");
		if (!Unsigned(Effect.Find("sourceActionId"), Action) || !Action ||
			!Unsigned(Effect.Find("sourceStageIndex"), Stage) || Stage > 999u ||
			!Clips || !Clips->Is_Array() || Clips->Get_Array().size() != 1u)
			return false;
		std::ostringstream Id;
		Id << "effect.valtan.action." << Action << ".stage" << std::setw(3)
			<< std::setfill('0') << Stage;
		// 분할 복원도 같은 원본 애니메이션 기록을 유지한다.
		// 선택적인 variant는 별도 asset을 구분하며 원래 기록을 대체하지 않는다.
		if (const auto* Variant = Effect.Find("variantId"))
		{
			if (!Variant->Is_String() || Variant->Get_String().empty() ||
				Variant->Get_String().size() > 64u ||
				Variant->Get_String().find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-") != std::string::npos)
				return false;
			Id << "." << Variant->Get_String();
		}
		Id << ".full.restore";
		if (!StringIs(Effect.Find("effectAssetId"), Id.str()) || Staged.contains(Id.str()))
			return false;
		std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> SourceClips;
		for (const auto& Source : Clips->Get_Array())
		{
			const auto* Name = Source.Find("clipName");
			const auto* Loop = Source.Find("loop");
			VALTAN_CLIP_OCCURRENCE_VIEW Clip;
			if (!Name || !Name->Is_String() || !Name->Get_String().starts_with("mesh_") ||
				!Unsigned(Source.Find("playMs"), Clip.iPlayMs) || !Clip.iPlayMs ||
				!Loop || !Loop->Is_Boolean() ||
				!Unsigned(Source.Find("previewWallMs"), Clip.iAuthoringWallMs) ||
				!Clip.iAuthoringWallMs || Clip.iAuthoringWallMs > 600000u ||
				(!StringIs(Source.Find("previewWallBasis"), "SOURCE_UNCONDITIONAL_STAGE_TRANSITION") &&
				 !StringIs(Source.Find("previewWallBasis"), "PREVIEW_COVERS_SOURCE_CONDITIONAL_CHECK") &&
				 !StringIs(Source.Find("previewWallBasis"), "SOURCE_ANIMATION_WINDOW")))
				return false;
			Clip.strClipName = Name->Get_String();
			Clip.bLoop = Loop->Get_Boolean();
			Clip.strClipOccurrenceId = "editor.full-restore." + Id.str();
			SourceClips.push_back(std::move(Clip));
		}
		// 편집용 시험 시간을 원본 검증 기록 대신 사용하지 않는다.
		// 명시적인 편집기 호출만 이 시간 재정의를 선택하며 Product 대조는 원본 시계를 유지한다.
		if (const auto* Preview = Effect.Find("authoredPreview"))
		{
			uint32_t WallMs = 0u;
			const auto* Loop = Preview->Find("loop");
			if (!Preview->Is_Object() ||
				!StringIs(Preview->Find("mappingBasis"), "PROJECT_AUTHORED") ||
				!Loop || !Loop->Is_Boolean() ||
				!Unsigned(Preview->Find("previewWallMs"), WallMs) ||
				!WallMs || WallMs > 600000u)
				return false;
			if (bUseAuthoredPreview)
			{
				SourceClips.front().bLoop = Loop->Get_Boolean();
				SourceClips.front().iAuthoringWallMs = WallMs;
			}
		}
		Staged.emplace(Id.str(), std::move(SourceClips));
	}
	OutIndex = std::move(Staged);
	OutError.clear();
	return true;
}

// 패턴/action 및 Product cue의 Effect ID를 통해 원본 색인과 연결 가능한지 조회한다. 원본 기록을 수정하지 않는다.
bool_t Client::CValtanPatternTree::Matches_FullRestoreSource(
	const Client::VALTAN_PATTERN_VIEW& Pattern,
	const std::string& strEffectAssetId,
	const VALTAN_FULL_RESTORE_CLIP_INDEX& Index)
{
	const auto Entry = Index.find(strEffectAssetId);
	if (Entry == Index.end())
		return false;
	const bool_t ActionMatches = std::ranges::any_of(Pattern.SourceActionIds,
		[&strEffectAssetId](const uint32_t Id)
		{ return strEffectAssetId.starts_with(
			"effect.valtan.action." + std::to_string(Id) + ".stage"); });
	// Product cue가 명시한 Effect ID로 원본 Action을 찾을 수도 있다.
	// 이 경로는 애니메이션 승격 결과가 기존 action alias를 유지하는 경우를 지원한다.
	const bool_t CueMatches = std::ranges::any_of(Pattern.Stages,
		[&strEffectAssetId](const auto& Stage)
		{ return std::ranges::any_of(Stage.ProductCues,
			[&strEffectAssetId](const auto& Cue)
			{ return Cue.strEffectAssetId == strEffectAssetId; }); });
	if (!ActionMatches && !CueMatches)
		return false;
	for (const auto& Stage : Pattern.Stages)
		for (const auto& Clip : Stage.ClipOccurrences)
			if (std::ranges::find(Entry->second, Clip.strClipName,
				&Client::VALTAN_CLIP_OCCURRENCE_VIEW::strClipName) != Entry->second.end())
				return true;
	return false;
}

// 읽은 트리와 리소스/원본 색인으로 표시용 이름 map을 구성한다. ID나 디스크에 저장한 이름은 바꾸지 않는다.
void Client::CValtanPatternTree::Build_EffectResourceDisplayLabels(
    VALTAN_PATTERN_TREE_VIEW& View,
    const std::unordered_map<std::string, std::string>& SavedNames,
    const VALTAN_FULL_RESTORE_CLIP_INDEX& SourceClips)
{
    // 선택 목록과 편집기 제목이 같은 표시 이름을 사용한다.
    // 겹치는 표시 이름에만 순번을 덧붙이며 저장 ID와 사용자가 저장한 이름은 바꾸지 않는다.
    // 이전 표시 이름을 먼저 비워 Describe_EffectResource가 그 캐시를 재사용하지 않게 한다.
    // 뒤의 임시 map 생성 중 할당 예외가 나면 이전 이름 map으로 복구하지 않는다.
    View.EffectResourceDisplayLabels.clear();
    std::map<std::string, std::vector<std::string>> ByLabel;

    for (const auto& [Id, Name] : SavedNames)
        if (Id.starts_with("effect.valtan.") || Id.starts_with("boss.valtan."))
            ByLabel[Describe_EffectResource(
                View, Id, Name, SourceClips)].push_back(Id);

    // 같은 표시 이름을 가진 asset ID를 문자열 순으로 정렬하고 두 개 이상일 때만 순번을 붙인다.
    // 입력 SavedNames의 unordered 순회 순서를 표시 순번으로 사용하지 않는다.
    std::unordered_map<std::string, std::string> Staged;
    for (auto& [Label, Ids] : ByLabel)
    {
        std::ranges::sort(Ids);
        for (size_t I = 0; I < Ids.size(); ++I)
        {
            std::ostringstream Ordinal;
            if (Ids.size() > 1u)
                Ordinal << "\x20\xC2\xB7\x20" <<
                    std::setw(2) << std::setfill('0') << I + 1u;
            Staged.emplace(Ids[I], Label + Ordinal.str());
        }
    }

    View.EffectResourceDisplayLabels = std::move(Staged);
}

// 리소스의 소유 패턴과 원본 복원 순서를 이용해 화면용 이름을 만든다. 식별자와 저장 이름의 대체 저장 계약은 아니다.
// Effect 도구의 표시 목록과 이름 map 작성이 호출한다. 입력 트리·색인은 읽기 전용으로 차용하며
// 리소스 파일이나 저장 이름을 수정하지 않고 화면용 문자열 값만 반환한다.
std::string Client::CValtanPatternTree::Describe_EffectResource(
    const VALTAN_PATTERN_TREE_VIEW& View, const std::string& Id,
    const std::string& SavedName, const VALTAN_FULL_RESTORE_CLIP_INDEX& Index)
{
    // 발탄 접두사가 아니면 저장 이름 또는 ID를 그대로 사용한다. 발탄이면 이미 만들어 둔
    // EffectResourceDisplayLabels를 가장 먼저 사용해 목록과 상세 제목의 이름을 맞춘다.
    if (!Id.starts_with("effect.valtan.") && !Id.starts_with("boss.valtan."))
        return SavedName.empty() ? Id : SavedName;
    if (const auto Label = View.EffectResourceDisplayLabels.find(Id);
        Label != View.EffectResourceDisplayLabels.end())
        return Label->second;
    const auto Number = [](size_t Value)
    {
        std::ostringstream Out;
        Out << std::setw(2) << std::setfill('0') << Value;
        return Out.str();
    };

    // UTF-8처럼 보이는 연속 세 바이트에서 완성형 한글 음절 범위를 찾는 표시용 판정이다.
    // 문자열 전체의 UTF-8 유효성이나 자모·모든 한국어 문자의 존재를 검증하지 않는다.
    const auto HasKorean = [](const std::string& Text)
    {
        for (size_t I = 0; I + 2 < Text.size(); ++I)
        {
            const auto A = static_cast<unsigned char>(Text[I]);
            const auto B = static_cast<unsigned char>(Text[I + 1]);
            const auto C = static_cast<unsigned char>(Text[I + 2]);
            if ((A & 0xf0u) != 0xe0u || (B & 0xc0u) != 0x80u || (C & 0xc0u) != 0x80u)
                continue;
            const unsigned Code = ((A & 15u) << 12) | ((B & 63u) << 6) | (C & 63u);
            if (Code >= 0xac00u && Code <= 0xd7a3u)
                return true;
        }
        return false;
    };

    // full.restore 접미사가 원본 복원 리소스를 구분한다. 본체/부채꼴 variant는
    // 뒤에서 같은 기본 ID의 순번을 공유하고 표시 접미사만 더한다.
    const bool Restore = Id.ends_with(".full.restore");
    const auto Source = Index.find(Id);
    const auto Variant = [&]() -> std::string
    {
        if (Id.ends_with(".body.full.restore"))
            return "\x20\xC2\xB7\x20\xEB\xB3\xB8\xEC\xB2\xB4";
        if (Id.ends_with(".sectors.full.restore") || Id.ends_with(".sector.full.restore"))
            return "\x20\xC2\xB7\x20\xEB\xB6\x80\xEC\xB1\x84\xEA\xBC\xB4";
        return {};
    };

    // 기믹·일반 패턴의 주소를 차용한 뒤 pattern ID 순으로 정렬한다.
    // 여러 패턴이 같은 리소스를 가리키면 이 순서에서 처음 맞는 패턴의 이름을 사용한다.
    std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
    for (const auto* Rows : { &View.Gimmicks, &View.Rotation })
        for (const auto& Pattern : *Rows)
            Patterns.push_back(&Pattern);

    std::ranges::sort(Patterns, {}, [](const auto* Pattern)
    {
        return Pattern->strPatternId;
    });

    // 일반 리소스는 한글이 들어 있는 독립 Effect 표시 이름을 패턴 이름보다 우선한다.
    // 복원 리소스는 이 단계를 건너뛰고 원본 clip 관계로 이름을 만든다.
    for (const auto& Independent : View.IndependentEffects)
        if (!Restore && Independent.strEffectAssetId == Id && HasKorean(Independent.strDisplayName))
            return Independent.strDisplayName;
    for (const auto* Pattern : Patterns)
    {
        // 패턴 일치는 Product cue·전투 오브젝트 참조 또는 복원 source 관계로 판정한다.
        // 표시 stage 순번은 그 중 처음 맞는 stage이며 clip 일치만으로 패턴 일치 판정을 새로 참으로 바꾸지는 않는다.
        size_t StageOrdinal = 0u;
        bool Matches = Restore && Matches_FullRestoreSource(*Pattern, Id, Index);
        for (size_t I = 0; I < Pattern->Stages.size(); ++I)
        {
            const auto& Stage = Pattern->Stages[I];
            const bool Product = std::ranges::any_of(Stage.ProductCues,
                [&](const auto& Cue) { return Cue.strEffectAssetId == Id || Cue.strV1EffectAssetId == Id; });
            const bool Object = std::ranges::any_of(Stage.CombatObjectEffects,
                [&](const auto& Row) { return Row.strEffectAssetId == Id || Row.strEffectV2GroupId == Id; });
            const bool Clip = Restore && Source != Index.end() && std::ranges::any_of(Stage.ClipOccurrences,
                [&](const auto& Occurrence) { return std::ranges::find(Source->second, Occurrence.strClipName,
                    &VALTAN_CLIP_OCCURRENCE_VIEW::strClipName) != Source->second.end(); });
            if (!StageOrdinal && (Product || Object || Clip))
                StageOrdinal = I + 1u;
            Matches = Matches || Product || Object;
        }
        if (!Matches)
            continue;
        const std::string Name = HasKorean(Pattern->strDisplayName) ? Pattern->strDisplayName : "\xEB\xB0\x9C\xED\x83\x84\x20\xED\x8C\xA8\xED\x84\xB4";
        if (Restore)
        {
            // 소유 패턴의 clip 순서에 맞춰 복원 조각의 표시 순번을 정한다.
            // 원본 variant도 같은 순서를 유지하고 읽기 쉬운 접미사만 추가한다.
            const auto BaseId = [](std::string Value)
            {
                for (const std::string_view Variant : { ".body.full.restore", ".sectors.full.restore", ".sector.full.restore" })
                    if (Value.ends_with(Variant))
                    {
                        Value.resize(Value.size() - Variant.size());
                        return Value + ".full.restore";
                    }
                return Value;
            };

            // stage/clip 순서로 복원 원본을 찾고 variant를 합친 기본 ID를 한 번씩 센다.
            // 같은 clip에서 여러 원본이 맞으면 set의 문자열 순서를 사용한다.
            std::set<std::string> Seen;
            size_t Ordinal = 0u, SelectedOrdinal = 0u;
            for (const auto& Stage : Pattern->Stages)
                for (const auto& Clip : Stage.ClipOccurrences)
                {
                    std::set<std::string> MatchingSources;

                    for (const auto& [RestoreId, Clips] : Index)
                        if (Matches_FullRestoreSource(
                                *Pattern, RestoreId, Index) &&
                            std::ranges::find(Clips, Clip.strClipName,
                                &VALTAN_CLIP_OCCURRENCE_VIEW::strClipName) !=
                                    Clips.end())
                            MatchingSources.insert(BaseId(RestoreId));

                    // 한 clip에서 찾은 기본 복원 ID는 set 순서로 처리한다. 이미 앞선 clip에서 센 ID는
                    // 다시 세지 않으므로 같은 기본 ID의 body/sector variant가 순번을 공유한다.
                    for (const auto& RestoreId : MatchingSources)
                        if (Seen.insert(RestoreId).second)
                        {
                            ++Ordinal;
                            if (RestoreId == BaseId(Id))
                                SelectedOrdinal = Ordinal;
                        }
                }

            // 잡기 패턴의 일부 원본 clip은 잡기 준비/사자후 준비/불어 날리기 이름으로 표시한다.
            // 여러 clip이 조건에 맞으면 뒤에서 확인한 이름이 앞의 이름을 대체한다.
            std::string Role = "\xEC\x9B\x90\xEB\xB3\xB8\x20\xEC\x9D\xB4\xED\x8E\x99\xED\x8A\xB8";
            if (Pattern->strPatternId == "VALTAN_CATCH_BREATH" && Source != Index.end())
                for (const auto& Clip : Source->second)
                {
                    if (Clip.strClipName == "mesh_att_battle_21_01")
                        Role = "\xEC\x9E\xA1\xEA\xB8\xB0\x20\xEC\xA4\x80\xEB\xB9\x84";
                    else if (Clip.strClipName == "mesh_att_battle_21_03")
                        Role = "\xEC\x82\xAC\xEC\x9E\x90\xED\x9B\x84\x20\xEC\xA4\x80\xEB\xB9\x84";
                    else if (Clip.strClipName == "mesh_att_battle_21_04")
                        Role = "\xEB\xB6\x88\xEC\x96\xB4\x20\xEB\x82\xA0\xEB\xA6\xAC\xEA\xB8\xB0";
                }

            // 복원 기본 ID의 순번을 찾았으면 그것을, 없으면 첫 일치 stage 순번을 쓴다.
            // 원본 variant 접미사는 이 순번 뒤에만 붙인다.
            return Name + "\x20\xC2\xB7\x20" + Number(SelectedOrdinal ? SelectedOrdinal : StageOrdinal) + " " + Role + Variant();
        }
        return Name + "\x20\xC2\xB7\x20" + Number(StageOrdinal) + "\x20\xEC\xA0\x9C\xED\x92\x88\x20\xEC\x9D\xB4\xED\x8E\x99\xED\x8A\xB8";
    }
    // 저장한 한국어 이름은 저작자의 값이다. 표시할 때 끝의 기술용 장식만 덜어낸다.
    // 정확한 저장 이름 자체를 변경하지 않으므로 검색과 상세 정보에서 계속 사용할 수 있다.
    if (!Restore && HasKorean(SavedName))
    {
        std::string Name = SavedName;
        for (const char* Marker : { " | ", " / VALTAN_", " / STEP_" })
            if (const auto At = Name.find(Marker); At != std::string::npos && HasKorean(Name.substr(0, At)))
                Name.resize(At);
        return Name;
    }

    // 앞선 label·독립 이름·패턴·한글 저장 이름이 모두 선택되지 않은 마지막 대체 경로다.
    // ID 끝부분의 숫자 묶음을 추출하고 떨어진 숫자 묶음 사이에는 하이픈을 넣는다.
    std::string Tail = Id.substr(Id.rfind('.') + 1u);
    if (Id.starts_with("boss.valtan."))
        Tail = Id.substr(12u);
    std::string Numbers;
    bool PreviousDigit = false;
    for (const unsigned char C : Tail)
    {
        const bool Digit = C >= '0' && C <= '9';
        if (Digit)
        {
            if (!PreviousDigit && !Numbers.empty())
                Numbers += '-';
            Numbers += static_cast<char>(C);
        }
        PreviousDigit = Digit;
    }

    // ID에 포함된 첫 키워드만 표시용 한국어 역할에 추가한다.
    // 이 배열의 순서가 우선순위이며 모든 일치 단어를 이어 붙이지 않는다.
    const std::pair<std::string_view, std::string_view> Terms[] = {
        {"tracking-axe", "\xEC\xB6\x94\xEC\xA0\x81\x20\xEB\x8F\x84\xEB\x81\xBC"}, {"high-jump", "\xEC\xB6\x94\xEC\xA0\x81\x20\xEB\x8F\x84\xEB\x81\xBC"}, {"axe", "\xEB\x8F\x84\xEB\x81\xBC"},
        {"catch-breath", "\xEC\x9E\xA1\xEC\x95\x84\xEC\xB1\x84\xEC\x84\x9C\x20\xEB\xB6\x88\xEC\x96\xB4\x20\xEB\x82\xA0\xEB\xA6\xAC\xEA\xB8\xB0"}, {"breathe.red", "\xEB\xB6\x89\xEC\x9D\x80\x20\xEC\x88\xA8\xEA\xB2\xB0"}, {"breathe", "\xEC\x88\xA8\xEA\xB2\xB0"},
        {"magicball", "\xEB\xA7\x88\xEB\xA0\xA5\xEA\xB5\xAC"}, {"egg", "\xEB\xA7\x88\xEB\xA0\xA5\xEA\xB5\xAC"}, {"rock-pillar", "\xEB\x8F\x8C\x20\xEA\xB8\xB0\xEB\x91\xA5"}, {"rock", "\xEB\x8F\x8C"},
        {"six", "\x36\xEB\xB0\xA9\xED\x96\xA5\x20\xEA\xB3\xB5\xEA\xB2\xA9"}, {"twohand", "\xEC\x96\x91\xEC\x86\x90\x20\xEB\x82\xB4\xEB\xA0\xA4\xEC\xB9\x98\xEA\xB8\xB0"}, {"portal", "\xED\x8F\xAC\xED\x83\x88"},
        {"blackhole", "\xEB\xB8\x94\xEB\x9E\x99\xED\x99\x80"}, {"shout", "\xEC\x82\xAC\xEC\x9E\x90\xED\x9B\x84"}, {"roar", "\xEC\x82\xAC\xEC\x9E\x90\xED\x9B\x84"}, {"pounding", "\xEB\x82\xB4\xEB\xA0\xA4\xEC\xB9\x98\xEA\xB8\xB0"},
        {"whirlwind", "\xED\x9C\xA0\xEC\x9C\x88\xEB\x93\x9C"}, {"wind_", "\xED\x9A\x8C\xEC\x98\xA4\xEB\xA6\xAC"}, {"stone", "\xEB\x8F\x8C\x20\xED\x8C\x8C\xED\x8E\xB8"}, {"dust", "\xEB\xA8\xBC\xEC\xA7\x80"},
        {"zoomblur", "\xED\x99\x95\xEB\x8C\x80\x20\xEC\x9E\x94\xEC\x83\x81"}, {"filmnoise", "\xED\x99\x94\xEB\xA9\xB4\x20\xEB\x85\xB8\xEC\x9D\xB4\xEC\xA6\x88"}, {"distortion", "\xEC\x99\x9C\xEA\xB3\xA1"}, {"blur", "\xEC\x9E\x94\xEC\x83\x81"},
        {"light", "\xEB\xB9\x9B"}, {"wave", "\xEC\xB6\xA9\xEA\xB2\xA9\xED\x8C\x8C"}, {"trail", "\xEA\xB6\xA4\xEC\xA0\x81"}, {"ribbon", "\xEA\xB6\xA4\xEC\xA0\x81"},
        {"hold", "\xEC\x9E\xA1\xEA\xB8\xB0\x20\xEC\x9C\xA0\xEC\xA7\x80"}, {"cast", "\xEC\x8B\x9C\xEC\xA0\x84"}, {"swing", "\xED\x9C\x98\xEB\x91\x90\xEB\xA5\xB4\xEA\xB8\xB0"}, {"dash", "\xEB\x8F\x8C\xEC\xA7\x84"},
        {"foot", "\xEB\xB0\x9C\xEC\x9E\x90\xEA\xB5\xAD"}, {"hand", "\xEC\x86\x90"}, {"hit", "\xED\x83\x80\xEA\xB2\xA9"}, {"impact", "\xEC\xB6\xA9\xEA\xB2\xA9"},
        {"decal", "\xEB\xB0\x94\xEB\x8B\xA5\x20\xEC\x9E\xA5\xED\x8C\x90"}, {"aura", "\xEC\x98\xA4\xEB\x9D\xBC"}, {"smoke", "\xEC\x97\xB0\xEA\xB8\xB0"}, {"flash", "\xEC\x84\xAC\xEA\xB4\x91"},
        {"comet", "\xEB\x82\x99\xED\x95\x98\xEC\xB2\xB4"}, {"fog", "\xEC\x95\x88\xEA\xB0\x9C"}, {"sonic", "\xEC\xB6\xA9\xEA\xB2\xA9\xED\x8C\x8C"}, {"spread", "\xED\x99\x95\xEC\x82\xB0"},
        {"fluid", "\xEC\x9C\xA0\xEC\xB2\xB4"}, {"twinkle", "\xEB\xB0\x98\xEC\xA7\x9D\xEC\x9E\x84"}, {"atk", "\xEA\xB3\xB5\xEA\xB2\xA9"}, {"cinematic", "\xEC\xBB\xB7\xEC\x8B\xA0"}
    };
    std::string Role = Restore ? "\xEC\x9B\x90\xEB\xB3\xB8\x20\xEC\x9D\xB4\xED\x8E\x99\xED\x8A\xB8" : Id.find(".source.") != std::string::npos ? "\xEC\x9B\x90\xEB\xB3\xB8\x20\xEB\x9D\xBC\xEC\x9D\xB4\xEB\xB8\x8C\xEB\x9F\xAC\xEB\xA6\xAC" : "\xEC\xA0\x80\xEC\x9E\xA5\x20\xEC\x9D\xB4\xED\x8E\x99\xED\x8A\xB8";
    for (const auto& [Token, Korean] : Terms)
        if (Id.find(Token) != std::string::npos)
        {
            Role += "\x20\xC2\xB7\x20";
            Role += Korean;
            break;
        }

    // 복원 색인에 clip이 있으면 ID에서 뽑은 숫자를 지우고 첫 clip 이름의 숫자만 모은다.
    // 이 마지막 대체 이름은 원본 파일 내용이나 재생 성공을 확인한 결과가 아니다.
    if (Restore && Source != Index.end() && !Source->second.empty())
    {
        Numbers.clear();
        const auto& Clip = Source->second.front().strClipName;
        for (const unsigned char C : Clip)
            if (C >= '0' && C <= '9')
                Numbers += static_cast<char>(C);
    }
    return "\xEB\xB0\x9C\xED\x83\x84\x20\xC2\xB7\x20" + Role + (Numbers.empty() ? std::string{} : " " + Numbers) + Variant();
}

// 도구 재생 후보를 split 관리 패턴과 수동 소유 메타데이터 양쪽에서 확인한다.
// 빈 수동 저작 행은 제외하고 Core/Animator/Derived 목록을 임시 값으로 만든 뒤 교체한다.
// 이 함수 자체가 디스크 Product를 다시 읽거나 Server 재생을 승인하는 것은 아니다.
bool_t Client::CValtanPatternTree::Build_PlayablePatternInventory(
	const VALTAN_PATTERN_TREE_VIEW& View,
	VALTAN_TOOL_AUDITION_INVENTORY& OutInventory,
	std::string& strOutError)
{
	std::map<std::string, const VALTAN_MANUAL_AUDITION_VIEW*, std::less<>> ManualByPattern;
	std::set<std::string, std::less<>> ManualSourceChains;
	for (const VALTAN_MANUAL_AUDITION_VIEW& Manual : View.ManualAuditions)
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(View, Manual.strPatternId);
		if (!Is_StableToken(Manual.strPatternId) || !Is_StableToken(Manual.strSourceChainId) ||
			0u == Manual.iAuthoringPhase || Manual.iAuthoringPhase > 3u ||
			("MANUAL_SERVER_AUDITION" != Manual.strAdmissionState &&
			 "DERIVED_SERVER_PATTERN" != Manual.strAdmissionState) ||
			!ManualByPattern.emplace(Manual.strPatternId, &Manual).second ||
			!ManualSourceChains.insert(Manual.strSourceChainId).second ||
			nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
			!pPattern->bManualServerAudition || "AUDITION_ONLY" != pPattern->strSelectionMode ||
			pPattern->strSourceAnimationChainId != Manual.strSourceChainId ||
			pPattern->iAuthoringPhase != Manual.iAuthoringPhase ||
			pPattern->strAdmissionState != Manual.strAdmissionState)
		{
			strOutError = "Playable Pattern manual owner is missing, invalid, or duplicated: " +
				Manual.strPatternId;
			return false;
		}
	}

	std::vector<const VALTAN_PATTERN_VIEW*> StagedPatterns;
	std::map<std::string, size_t, std::less<>> IdentityCounts;
	for (const auto* pPatterns : { &View.Gimmicks, &View.Rotation })
		for (const VALTAN_PATTERN_VIEW& Pattern : *pPatterns)
			++IdentityCounts[Pattern.strPatternId];
	for (const auto* pPatterns : { &View.Gimmicks, &View.Rotation })
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : *pPatterns)
		{
			if (!Pattern.bAuthoringMasterManaged)
				continue;
			if (!Is_StableToken(Pattern.strPatternId) ||
				1u != IdentityCounts.at(Pattern.strPatternId) ||
				Find_Pattern(View, Pattern.strPatternId) != &Pattern ||
				Pattern.Stages.empty() || !Is_StableToken(Pattern.strEntryActionId) ||
				Pattern.strEntryActionId != Pattern.Stages.front().strActionId)
			{
				strOutError = "Playable Pattern split-owned identity or entry stage is invalid: " + Pattern.strPatternId;
				return false;
			}
			if (Pattern.bManualServerAudition != ManualByPattern.contains(Pattern.strPatternId) ||
				(!Pattern.bManualServerAudition &&
				 (!Pattern.strAdmissionState.empty() || !Pattern.strSourceAnimationChainId.empty() ||
				  0u != Pattern.iAuthoringPhase)))
			{
				strOutError = "Playable Pattern manual metadata has no matching owner: " + Pattern.strPatternId;
				return false;
			}
			std::set<std::string, std::less<>> StageIds;
			std::set<std::string, std::less<>> ActionIds;
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				if (!Is_StableToken(Stage.strStageId) || !Is_StableToken(Stage.strActionId) ||
					!StageIds.insert(Stage.strStageId).second ||
					!ActionIds.insert(Stage.strActionId).second)
				{
					strOutError = "Playable Pattern stage identity is invalid or duplicated: " + Pattern.strPatternId;
					return false;
				}
			}
			if (Pattern.bManualServerAudition && Pattern.SourceActionIds.empty() &&
				Pattern.PresentationSources.empty() &&
				std::all_of(Pattern.Stages.begin(), Pattern.Stages.end(),
					[](const auto& Stage) { return Stage.ClipOccurrences.empty() && Stage.ProductCues.empty() &&
						!Stage.bHasStageEffectV2Bindings && !Stage.Has_HitShape(); }))
				continue;
			StagedPatterns.push_back(&Pattern);
		}
	}
	if (StagedPatterns.empty())
	{
		strOutError = "Playable Pattern requires at least one strictly joined split-owned Product pattern.";
		return false;
	}
	std::sort(StagedPatterns.begin(), StagedPatterns.end(),
		[](const VALTAN_PATTERN_VIEW* Left, const VALTAN_PATTERN_VIEW* Right)
		{
			return Left->iSourceSequenceIndex != Right->iSourceSequenceIndex ?
				Left->iSourceSequenceIndex < Right->iSourceSequenceIndex :
				Left->strPatternId < Right->strPatternId;
		});
	VALTAN_TOOL_AUDITION_INVENTORY Staged;
	for (const VALTAN_PATTERN_VIEW* pPattern : StagedPatterns)
		if (!pPattern->bManualServerAudition)
			Staged.CorePatternIds.push_back(pPattern->strPatternId);
	// 수동 패턴의 표시 순서는 저작 순서를 유지한다. Next의 원본 sequence 정렬과 구분한다.
	// 위에서 수동 메타데이터와 패턴 소유 관계를 양방향으로 확인했다.
	for (const VALTAN_MANUAL_AUDITION_VIEW& Manual : View.ManualAuditions)
	{
		const auto* Pattern = Find_Pattern(View, Manual.strPatternId);
		if (Pattern->SourceActionIds.empty() && Pattern->PresentationSources.empty() &&
			std::all_of(Pattern->Stages.begin(), Pattern->Stages.end(),
				[](const auto& Stage) { return Stage.ClipOccurrences.empty() && Stage.ProductCues.empty() &&
						!Stage.bHasStageEffectV2Bindings && !Stage.Has_HitShape(); }))
			continue;
		("DERIVED_SERVER_PATTERN" == Manual.strAdmissionState ?
			Staged.DerivedPatternIds : Staged.AnimatorPatternIds).push_back(Manual.strPatternId);
	}
	OutInventory = std::move(Staged);
	strOutError.clear();
	return true;
}

// 공통 재생 목록을 source sequence 순서와 ID 동률 순서로 합쳐 Next 버튼의 순서를 만든다. 실패하면 이전 출력 목록을 유지한다.
bool_t Client::CValtanPatternTree::Build_NextPatternInventory(
	const VALTAN_PATTERN_TREE_VIEW& View,
	std::vector<std::string>& OutPatternIds,
	std::string& strOutError)
{
	VALTAN_TOOL_AUDITION_INVENTORY Inventory;
	if (!Build_PlayablePatternInventory(View, Inventory, strOutError))
		return false;
	std::vector<std::string> StagedIds;
	StagedIds.reserve(Inventory.Get_PatternCount());
	for (const auto* pIds : { &Inventory.CorePatternIds, &Inventory.AnimatorPatternIds,
		&Inventory.DerivedPatternIds })
		StagedIds.insert(StagedIds.end(), pIds->begin(), pIds->end());
	std::sort(StagedIds.begin(), StagedIds.end(),
		[&View](const std::string& LeftId, const std::string& RightId)
		{
			const VALTAN_PATTERN_VIEW* Left = Find_Pattern(View, LeftId);
			const VALTAN_PATTERN_VIEW* Right = Find_Pattern(View, RightId);
			return Left->iSourceSequenceIndex != Right->iSourceSequenceIndex ?
				Left->iSourceSequenceIndex < Right->iSourceSequenceIndex : LeftId < RightId;
		});
	OutPatternIds = std::move(StagedIds);
	strOutError.clear();
	return true;
}

// 패턴과 stage의 identity를 진단 문자열로 만든다. 이 문자열은 저장 ID나 JSON 직렬화 형식이 아니다.
std::string Client::CValtanPatternTree::Build_PatternIdentitySummary(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	std::ostringstream Summary;
	Summary.imbue(std::locale::classic());
	Summary << std::fixed << std::setprecision(3)
		<< "Pattern ID: " << Pattern.strPatternId << "\nAuthoring: ";
	if ("DERIVED_SERVER_PATTERN" == Pattern.strAdmissionState)
		Summary << "Derived server";
	else if (Pattern.bManualServerAudition)
		Summary << "Phase " << Pattern.iAuthoringPhase << " manual audition";
	else if (Pattern.bAuthoringMasterManaged)
		Summary << "Core server";
	else
		Summary << "outside shared audition inventory";
	Summary << " | Runtime eligibility: phase " << Pattern.iMinimumPhase;
	if (Pattern.iMinimumPhase != Pattern.iMaximumPhase)
		Summary << '-' << Pattern.iMaximumPhase;
	Summary << "\nClips (authored order):";
	if (Pattern.Stages.empty())
		Summary << " none";
	for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		Summary << '\n' << iStage + 1u << ". " << Stage.strStageId
			<< " [wall " << Stage.iDurationMs / 1000.0 << "s";
		if (Stage.iAuthoringRepeatCount > 1u)
			Summary << "; repeats " << Stage.iAuthoringRepeatCount;
		Summary << ']';
		if (Stage.bSuppressAnimation)
		{
			Summary << ": animation suppressed";
			continue;
		}
		if (Stage.ClipOccurrences.empty())
		{
			Summary << ": no authored clip occurrence";
			continue;
		}
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
		{
			Summary << "\n  " << Clip.strClipName
				<< " [source start " << Clip.iSourceStartMs / 1000.0 << "s; ";
			if (0u != Clip.iPlayMs)
				Summary << "source duration " << Clip.iPlayMs / 1000.0 << 's';
			else
				Summary << "natural end";
			Summary << "; rate " << Clip.fPlayRate << "x; wall ";
			if (0u != Clip.iAuthoringWallMs)
				Summary << Clip.iAuthoringWallMs / 1000.0 << 's';
			else
				Summary << "unresolved";
			if (Clip.bLoop)
				Summary << "; loop";
			Summary << ']';
		}
	}
	return Summary.str();
}

// cue가 없는 stage의 편집 문서를 찾기 위한 기존 이름 규칙이다. 실제 Product cue ID보다 우선하지 않는다.
std::string Client::CValtanPatternTree::Build_StageEffectAssetId(
	const std::string& strPatternActionId,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (strPatternActionId.empty() || Stage.strActionId.empty())
		return {};
	// valtan.attack.whirlwind에서는 whirlwind를 추출한다.
	// 트리가 이미 보여주는 분류를 stage Effect의 이름 규칙에 중복해서 넣지 않는다.
	std::string strPattern = strPatternActionId;
	size_t iCut = strPattern.find('.');
	if (std::string::npos != iCut)
	{
		iCut = strPattern.find('.', iCut + 1u);
		if (std::string::npos != iCut)
			strPattern = strPattern.substr(iCut + 1u);
	}
	const std::string strPrefix = strPatternActionId + ".";
	std::string strStage;
	if (Stage.strActionId.starts_with(strPrefix) &&
		Stage.strActionId.size() > strPrefix.size())
	{
		strStage = Stage.strActionId.substr(strPrefix.size());
	}
	else
	{
		strStage = Stage.strStageId;
		std::transform(strStage.begin(), strStage.end(), strStage.begin(),
			[](const unsigned char Character)
			{
				return '_' == Character ? '-' :
					static_cast<char>(std::tolower(Character));
			});
	}
	if (strPattern.empty() || strStage.empty())
		return {};
	return "effect.valtan." + strPattern + "." + strStage;
}

// 선택한 미리보기 분기 정책에 맞춰 하나의 로컬 stage 경로를 만든다. 실제 Server의 조건 판정을 실행하지 않는다.
// 결과 포인터는 입력 Pattern의 stage를 차용한다. 실패하면 기존 OutStages를 유지하며 Pattern 변경 뒤에는 재구성해야 한다.
bool_t Client::CValtanPatternTree::Build_PreviewStagePath(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::vector<const VALTAN_STAGE_VIEW*>& OutStages,
	std::string& strOutStatus)
{
	if (VALTAN_PATTERN_PREVIEW_PATH::END == ePath || Pattern.Stages.empty())
	{
		strOutStatus = "Valtan preview path request is invalid.";
		return false;
	}

	std::map<std::string, size_t, std::less<>> StageByAction;
	for (size_t i = 0u; i < Pattern.Stages.size(); ++i)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[i];
		if (Stage.strActionId.empty() ||
			!StageByAction.emplace(Stage.strActionId, i).second)
		{
			strOutStatus = "Valtan preview path has a missing or duplicated action: " +
				Stage.strActionId;
			return false;
		}
		std::set<std::string, std::less<>> Outcomes;
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strOutcome.empty() ||
				!Outcomes.insert(Branch.strOutcome).second)
			{
				strOutStatus = "Valtan preview path has ambiguous branches: " +
					Stage.strActionId;
				return false;
			}
		}
	}
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strNextActionId.has_value() &&
				!StageByAction.contains(*Branch.strNextActionId))
			{
				strOutStatus =
					"Valtan preview path branches to an unknown action: " +
					*Branch.strNextActionId;
				return false;
			}
		}
	}

	// 방문한 action ID를 기억해 경로의 순환을 거절한다. 잡기 재시도는 뒤의 지정 간선에서 한 회차로 끝낸다.
	std::vector<const VALTAN_STAGE_VIEW*> StagedPath;
	std::set<std::string, std::less<>> VisitedActions;
	size_t iStage = 0u;
	bool_t bWallContactTaken = false;
	bool_t bCounterHitTaken = false;
	bool_t bCaptureTaken = false;
	bool_t bCounterRetry = false;
	for (;;)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!VisitedActions.insert(Stage.strActionId).second)
		{
			strOutStatus = "Valtan preview path contains a cycle: " +
				Stage.strActionId;
			return false;
		}
		StagedPath.push_back(&Stage);

		if (Stage.Branches.empty())
		{
			if (iStage + 1u == Pattern.Stages.size())
				break;
			++iStage;
			continue;
		}

		const VALTAN_STAGE_BRANCH_VIEW* pSelected = nullptr;
		const auto SelectOutcome = [&Stage, &pSelected](
			const std::string_view strOutcome) -> bool_t
		{
			for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
			{
				if (Branch.strOutcome != strOutcome)
					continue;
				if (nullptr != pSelected)
					return false;
				pSelected = &Branch;
			}
			return true;
		};
		if (VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS == ePath)
		{
			if (!SelectOutcome("ANY_PLAYER_GRABBED"))
			{
				strOutStatus = "Valtan preview path has ambiguous ANY_PLAYER_GRABBED edges.";
				return false;
			}
			bCaptureTaken = bCaptureTaken || nullptr != pSelected;
		}
		if (VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY == ePath)
		{
			if (!SelectOutcome("COUNTER_HIT"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous COUNTER_HIT edges.";
				return false;
			}
			bCounterHitTaken = bCounterHitTaken || nullptr != pSelected;
		}

		if (VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK == ePath &&
			bWallContactTaken)
		{
			if (!SelectOutcome("PART_DESTROYED"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous PART_DESTROYED edges.";
				return false;
			}
		}
		if (nullptr == pSelected &&
			(VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY == ePath ||
			 VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK == ePath) &&
			!bWallContactTaken)
		{
			if (!SelectOutcome("WALL_CONTACT"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous WALL_CONTACT edges.";
				return false;
			}
			bWallContactTaken = nullptr != pSelected;
		}
		if (nullptr == pSelected && !SelectOutcome("TIMEOUT"))
		{
			strOutStatus =
				"Valtan preview path has ambiguous TIMEOUT edges.";
			return false;
		}
		if (nullptr == pSelected)
		{
			strOutStatus = "Valtan preview path has no policy-selectable edge: " +
				Stage.strActionId;
			return false;
		}
		// 로컬 nextActionId가 없으면 미리보기 경로를 끝낸다. nextPatternId의 다른 패턴까지 이 함수가 펼치지는 않는다.
		if (!pSelected->strNextActionId.has_value())
			break;
		const auto Next = StageByAction.find(*pSelected->strNextActionId);
		if (Next == StageByAction.end())
		{
			strOutStatus = "Valtan preview path branches to an unknown action: " +
				*pSelected->strNextActionId;
			return false;
		}
		if ((Pattern.strPatternId == "VALTAN_TRASH" || Pattern.strPatternId == "VALTAN_TRASH_CATCH_IF") &&
			(Stage.strStageId == "RETRY_EXHAUSTED" || Stage.strStageId == "CATCH_SLAM" ||
			 Stage.strStageId == "EXECUTE_TAIL") && Stage.Branches.size() == 1u &&
			pSelected->strOutcome == "TIMEOUT" && Pattern.Stages[Next->second].strStageId == "RECHARGE_WAIT_02")
		{
			bCounterRetry = true;
			break;
		}
		iStage = Next->second;
	}

	if (VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS == ePath && !bCaptureTaken)
	{
		strOutStatus = "Valtan Capture Success preview path has no ANY_PLAYER_GRABBED branch.";
		return false;
	}
	if (VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY == ePath &&
		!bCounterHitTaken)
	{
		strOutStatus =
			"Valtan Counter/Groggy preview path has no COUNTER_HIT branch.";
		return false;
	}
	OutStages = std::move(StagedPath);
	strOutStatus = "Valtan preview path resolved " +
		std::to_string(OutStages.size()) + (bCounterRetry ?
		" stages in one pass; Server repeats until a counter succeeds." : " stages.");
	return true;
}

// typed Load의 상태 문자열만 전달하는 기존 호출자용 adapter다.
bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bLoaded = Load(OutView, Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bLoaded;
}

// 공유 admission의 수명을 이 Load 호출 전체로 잡고 엄격한 저작/Product 결합을 실행한다.
bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic)
{
	CValtanCanonicalProductReadAdmission Admission;
	if (!Admission.Acquire(OutDiagnostic))
		return false;
	return Load_WhileAdmitted(Admission, OutView, OutDiagnostic);
}

// 호출자가 이미 소유한 admission을 재사용하면서 문자열 상태를 반환한다.
bool_t Client::CValtanPatternTree::Load_WhileAdmitted(
	const CValtanCanonicalProductReadAdmission& Admission,
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	const bool_t bLoaded = Load_WhileAdmitted(
		Admission, OutView, Diagnostic);
	strOutStatus = std::move(Diagnostic.strStatus);
	return bLoaded;
}

// 보유 admission을 앞뒤로 확인하고 임시 뷰로 Product와 저작 원본을 대조한다.
// 모두 성공해야 OutView를 교체하므로 실패 시 이전 화면용 뷰가 유지된다.
bool_t Client::CValtanPatternTree::Load_WhileAdmitted(
	const CValtanCanonicalProductReadAdmission& Admission,
	VALTAN_PATTERN_TREE_VIEW& OutView,
	VALTAN_CANONICAL_READ_DIAGNOSTIC& OutDiagnostic)
{
	OutDiagnostic.Clear();
	VALTAN_CANONICAL_READ_DIAGNOSTIC AdmissionDiagnostic;
	if (!Admission.Is_Acquired() ||
		!Admission.Validate_StillCurrent(AdmissionDiagnostic))
	{
		OutDiagnostic = std::move(AdmissionDiagnostic);
		if (VALTAN_CANONICAL_READ_FAILURE_KIND::NONE ==
			OutDiagnostic.eFailure)
		{
			OutDiagnostic.eFailure = VALTAN_CANONICAL_READ_FAILURE_KIND::
				ADMISSION_STATE_INVALID;
		}
		OutDiagnostic.strStatus =
			"Valtan canonical graph read admission failed: " +
			OutDiagnostic.strStatus;
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW StagedView;
	std::string LoadStatus;
	if (!Load_FromAuthoringPaths(
		CProjectDataRoot::Resolve(
			std::filesystem::path(L"Valtan") / L"Valtan.gameplay.json"),
		CProjectDataRoot::Resolve(
			std::filesystem::path(L"Valtan") / L"Valtan.presentation.json"),
		StagedView, LoadStatus,
		VALTAN_PATTERN_TREE_LOAD_POLICY::REQUIRE_ACTIVE_PRODUCT_PARITY,
		&OutDiagnostic))
	{
		OutDiagnostic.strStatus = std::move(LoadStatus);
		return false;
	}
	// 결합 후 journal 존재를 다시 검사한 뒤에만 호출자의 뷰를 교체한다.
	// 공유 잠금에 참여하는 writer는 읽는 동안 배타 잠금을 얻지 못한다.
	// 이 재검사는 journal을 남긴 외부 변경을 탐지하지만 모든 파일의 hash를 재계산하는 검사는 아니다.
	if (!Admission.Validate_StillCurrent(AdmissionDiagnostic))
	{
		OutDiagnostic = std::move(AdmissionDiagnostic);
		OutDiagnostic.strStatus =
			"Valtan canonical graph read admission failed: " +
			OutDiagnostic.strStatus;
		return false;
	}

	OutView = std::move(StagedView);
	OutDiagnostic.Clear();
	OutDiagnostic.strStatus = std::move(LoadStatus);
	return true;
}


// 편집기의 원본 보기 경로다. admission 아래 gameplay/presentation 및 원본 전투 오브젝트를 읽어 트리를 만든다.
// 현재 게시 Product와의 값 일치를 요구하지 않으며 전투 오브젝트 목록도 최종적으로 원본에서 다시 구성한다.
// 끝의 admission 재확인까지 통과한 Staged만 OutView에 넣는다. 파일 저장이나 publish는 수행하지 않는다.
bool_t Client::CValtanPatternTree::Load_Authoring_WhileAdmitted(
	const CValtanCanonicalProductReadAdmission& Admission,
	VALTAN_PATTERN_TREE_VIEW& OutView, std::string& strOutStatus)
{
	// Balance/Effect/ActionWorkbench가 확보한 admission을 호출 동안 차용한다.
	// 진입에서 획득·현재성을 확인하고 끝에서 다시 검사하므로 명시적 false 경로는 OutView를 보존한다.
	if (!Admission.Is_Acquired() || !Admission.Validate_StillCurrent(strOutStatus))
		return false;
	DATA_JSON_VALUE Gameplay, Presentation;
	MASTER_DOCUMENT Master;
	if (!Parse_Document(std::filesystem::path(L"Valtan") / L"Valtan.gameplay.json",
			Gameplay, strOutStatus) ||
		!Parse_Document(std::filesystem::path(L"Valtan") / L"Valtan.presentation.json",
			Presentation, strOutStatus) ||
		!Parse_SplitMasterDocument(Gameplay, Presentation, Master, strOutStatus) ||
		!Join_StageEffectV2SourceScopes(Master, strOutStatus))
		return false;

	// 표시할 소환 정의와 visual을 원본 문서에서 읽는다.
	// 이 경로는 게시 Product 전체를 읽어 strict parity를 검사하는 실행 승인 경로와 구분한다.
	DATA_JSON_VALUE CombatAuthoring, BossCatalog;
	if (!Parse_Document(std::filesystem::path(L"Valtan") / L"Valtan.combatobjects.json",
			CombatAuthoring, strOutStatus) ||
		!Parse_Document(std::filesystem::path(L"Actors") / L"BossCatalog.json",
			BossCatalog, strOutStatus))
		return false;
	// 아래 포인터 색인은 이번 호출의 JSON 값을 차용한다. 최종 뷰에는 필요한 값을 복사한다.
	std::map<std::string, const DATA_JSON_VALUE*> SourceObjects, SourceVisuals, SourceStageEvents;
	const auto* objects = Required(CombatAuthoring, "objects", DATA_JSON_TYPE::ARRAY);
	const auto* bosses = Required(BossCatalog, "bosses", DATA_JSON_TYPE::ARRAY);
	const auto* sourcePatterns = Required(Gameplay, "patterns", DATA_JSON_TYPE::ARRAY);
	if (!objects || !bosses || !sourcePatterns)
	{
		strOutStatus = "Source Summon owner arrays missing.";
		return false;
	}

	// archetype별 오브젝트 정의·발탄 visual과 pattern/stage별 event의 차용 색인을 만든다.
	// 이 emplace들은 반환값을 검사하지 않으므로 같은 키가 다시 나오면 최초 항목이 유지된다.
	for (const auto& object : objects->Get_Array())
		SourceObjects.emplace(Read_String(object, "combatObjectArchetypeId"), &object);
	for (const auto& boss : bosses->Get_Array())
		if (Read_String(boss, "archetypeId") == "BOSS_VALTAN")
			if (const auto* visuals = Required(boss, "combatObjectVisuals", DATA_JSON_TYPE::ARRAY))
				for (const auto& visual : visuals->Get_Array())
					SourceVisuals.emplace(Read_String(visual, "combatObjectArchetypeId"), &visual);
	for (const auto& pattern : sourcePatterns->Get_Array())
		if (const auto* stages = Required(pattern, "stages", DATA_JSON_TYPE::ARRAY))
			for (const auto& stage : stages->Get_Array())
				SourceStageEvents.emplace(
					Read_String(pattern, "patternId") + "/" + Read_String(stage, "stageId"),
					stage.Find("events"));
	// 생성 event를 전투 오브젝트 원본 및 visual과 연결해 각 stage의 소환 Effect 목록을 만든다.
	// 기존 목록을 비운 뒤 채우므로 임시 Pattern 안에서만 호출한다.
	const auto fillSourceInventory = [&](VALTAN_PATTERN_VIEW& pattern) -> bool
	{
		for (auto& stage : pattern.Stages)
		{
			// 현재 stage의 소환 Effect 목록을 먼저 비운다. event 배열이 없으면 실패하며
			// 지역 Pattern만 부분 변경되므로 호출자가 가진 이전 OutView에는 영향을 주지 않는다.
			stage.CombatObjectEffects.clear();
			const auto found = SourceStageEvents.find(pattern.strPatternId + "/" + stage.strStageId);
			if (found == SourceStageEvents.end() || !found->second || !found->second->Is_Array())
			{
				strOutStatus = "Source Summon Stage events missing.";
				return false;
			}
			for (const auto& event : found->second->Get_Array())
			{
				// 월드 이벤트는 stage action으로 추가하고 소환/volley 외의 종류는 여기서 건너뛴다.
				// 이미 읽은 gameplay를 표시용 목록으로 연결할 뿐 실제 월드 이벤트나 소환을 실행하지 않는다.
				const auto kind = Read_String(event, "kind");
				if (kind == "TRIGGER_WORLD_EVENT_SET")
				{
					VALTAN_STAGE_ACTION_VIEW action;
					action.strKind = kind;
					action.strTrigger = Read_String(event, "trigger");
					action.strTargetId = Read_String(event, "worldEventSetId");
					action.fValue = 1.f;
					stage.Actions.push_back(std::move(action));
					continue;
				}
				if (kind != "SPAWN_COMBAT_OBJECT" && kind != "SPAWN_COMBAT_OBJECT_VOLLEY")
					continue;

				// 소환 archetype에 대응하는 원본 정의와 발탄 visual이 모두 있어야 한다.
				// visual 이름·asset·선택적 V2 group은 복사하며 GPU 리소스를 여기서 로드하지 않는다.
				const auto id = Read_String(event, "combatObjectArchetypeId");
				const auto object = SourceObjects.find(id), visual = SourceVisuals.find(id);
				if (object == SourceObjects.end() || visual == SourceVisuals.end())
				{
					strOutStatus = "Source Summon definition/visual unavailable: " + id;
					return false;
				}
				const auto& definition = *object->second;
				VALTAN_COMBAT_OBJECT_EFFECT_VIEW row;
				row.strCombatObjectArchetypeId = id;
				row.strClientVisualId = Read_String(*visual->second, "clientVisualId");
				row.strEffectAssetId = Read_String(*visual->second, "effectAssetId");
				if (const auto* owns = visual->second->Find("armedEffectOwnsTerminal"); owns && owns->Is_Boolean() && owns->Get_Boolean())
					row.strPreparationEventId = Read_String(*visual->second, "armedPresentationEventId");
				if (const auto* group = visual->second->Find("effectV2Group"))
					row.strEffectV2GroupId = Read_String(*group, "groupId");
				row.strTrigger = Read_String(event, "trigger");

				// 단일 소환은 count, volley는 countPerResolvedTarget을 표시 값으로 쓴다.
				// 선택 문자열/숫자의 누락·타입 불일치는 공통 reader의 빈 문자열/0을 사용하고, lifetime 누락은 stage 기간을 쓴다.
				row.iSpawnValue = static_cast<uint32_t>(Read_Number(event, kind == "SPAWN_COMBAT_OBJECT" ? "count" : "countPerResolvedTarget"));
				row.strKind = Read_String(definition, "kind");
				row.iLifetimeMs = definition.Find("lifetimeMs") ? static_cast<uint32_t>(Read_Number(definition, "lifetimeMs")) : stage.iDurationMs;
				row.fCoverRadiusM = static_cast<float>(Read_Number(definition, "coverRadiusM"));
				if (!Read_CombatObjectOwnerHitChain(definition, row.OwnerHitChain))
				{
					strOutStatus = "Source combat-object owner-hit chain is invalid: " + id;
					return false;
				}

				// 원본 spawn/movement를 표시용 origin·방향·속도 m/s·거리 m로 옮긴다.
				// RESOLVED_VOLLEY_POSITION은 이 View에서 사용하는 LOCKED_TARGET_PER_ALIVE_PLAYER 이름으로 대응한다.
				if (const auto* spawn = definition.Find("spawn"))
				{
					if (const auto* origin = spawn->Find("origin"))
						row.strOriginPolicy = Read_String(*origin, "kind");
					if (row.strOriginPolicy == "RESOLVED_VOLLEY_POSITION")
						row.strOriginPolicy = "LOCKED_TARGET_PER_ALIVE_PLAYER";
					if (const auto* direction = spawn->Find("direction"))
						row.strDirectionPolicy = Read_String(*direction, "kind");
				}
				if (const auto* movement = definition.Find("movement"))
				{
					row.fSpeedMps = static_cast<float>(Read_Number(*movement, "speedMps"));
					row.fMaximumDistanceM = static_cast<float>(Read_Number(*movement, "maximumDistanceM"));
				}

				// 원본 hit 목록을 순서대로 복사해 ID·시각 ms·반복·형상 m·피격 응답을 표시한다.
				// 이 읽기는 별도 owner-hit chain 검사 외 모든 수치를 엄격히 재검증하는 Product 파서가 아니다.
				if (const auto* hits = Required(definition, "hits", DATA_JSON_TYPE::ARRAY))
					for (const auto& hit : hits->Get_Array())
					{
						const auto hitId = Read_String(hit, "hitId");
						row.HitIds.push_back(hitId);
						if (const auto* trigger = hit.Find("trigger"))
							row.HitOffsetsMs.push_back(static_cast<uint32_t>(Read_Number(*trigger, "atMs")));
						VALTAN_COMBAT_OBJECT_HIT_VIEW view;
						view.strHitId = hitId;
						if (const auto* trigger = hit.Find("trigger"))
						{
							view.strTriggerKind = Read_String(*trigger, "kind");
							view.iAtMs = static_cast<uint32_t>(Read_Number(*trigger, "atMs"));
						}
						if (const auto* repeat = hit.Find("repeat"))
						{
							view.iRepeatCount = static_cast<uint32_t>(Read_Number(*repeat, "count"));
							view.iRepeatIntervalMs = static_cast<uint32_t>(Read_Number(*repeat, "intervalMs"));
						}
						if (const auto* shape = hit.Find("shape"))
						{
							view.strHitShape = Read_String(*shape, "kind");
							view.fInnerRadiusM = static_cast<float>(Read_Number(*shape, "innerRadiusM"));
							view.fOuterRadiusM = static_cast<float>(Read_Number(*shape, "outerRadiusM"));
						}
						view.strServerDamageProfileId = Read_String(hit, "serverDamageProfileId");
						view.fPushRangeM = static_cast<float>(Read_Number(hit, "pushRangeM"));
						view.iPushMs = static_cast<uint32_t>(Read_Number(hit, "pushMs"));
						view.iDownMs = static_cast<uint32_t>(Read_Number(hit, "downMs"));
						if (const auto* knockdown = hit.Find("knockdown"))
							view.bKnockdown = knockdown->Get_Boolean();
						row.Hits.push_back(std::move(view));
					}

				// 표현 이벤트는 event ID와 trigger 시각만 이 목록에 복사한다.
				// 아래 volley 정보와 함께 사용자가 원본 소환 구성을 볼 때 소비한다.
				if (const auto* presentations = Required(definition, "presentationEvents", DATA_JSON_TYPE::ARRAY))
					for (const auto& presentation : presentations->Get_Array())
					{
						VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW view;
						view.strPresentationEventId = Read_String(presentation, "presentationEventId");
						if (const auto* trigger = presentation.Find("trigger"))
							view.iAtMs = static_cast<uint32_t>(Read_Number(*trigger, "atMs"));
						row.PresentationEvents.push_back(std::move(view));
					}

				// volley에만 전체 개수 상한·배치·예약 시계·arena random 범위를 추가한다.
				// 선택 필드가 없으면 row 기본값을 유지하며 이 함수는 무작위 위치를 표본화하지 않는다.
				if (kind == "SPAWN_COMBAT_OBJECT_VOLLEY")
				{
					row.strVolleyPolicy = Read_String(event, "volleyPolicy");
					row.iVolleyMaximumTotalObjects = static_cast<uint32_t>(Read_Number(event, "maximumTotalObjects"));
					if (const auto* overlap = event.Find("allowOverlap"))
						row.bVolleyAllowOverlap = overlap->Get_Boolean();
					if (const auto* layout = event.Find("layout"))
					{
						row.strVolleyLayout = Read_String(*layout, "kind") == "TARGET_CENTER" ? "SINGLE" : "RADIAL";
						row.fVolleyRadiusM = static_cast<float>(Read_Number(*layout, "radiusM"));
						row.fVolleyStartAngleDegrees = static_cast<float>(Read_Number(*layout, "startAngleDegrees"));
						row.fVolleyAngleStepDegrees = static_cast<float>(Read_Number(*layout, "angleStepDegrees"));
					}
					if (const auto* schedule = event.Find("spawnSchedule"))
					{
						row.iSpawnScheduleCount = static_cast<uint32_t>(Read_Number(*schedule, "count"));
						row.iFirstSpawnOffsetMs = static_cast<uint32_t>(Read_Number(*schedule, "firstOffsetMs"));
						row.iSpawnIntervalMs = static_cast<uint32_t>(Read_Number(*schedule, "intervalMs"));
					}
					if (const auto* random = event.Find("arenaRandom"))
					{
						row.strArenaRandomKind = Read_String(*random, "kind");
						row.strArenaAnchor = Read_String(*random, "anchor");
						row.iArenaRandomCount = static_cast<uint32_t>(Read_Number(*random, "count"));
						row.fArenaRandomRadiusM = static_cast<float>(Read_Number(*random, "radiusM"));
						row.fArenaHeightToleranceM = static_cast<float>(Read_Number(*random, "heightToleranceM"));
					}
				}
				stage.CombatObjectEffects.push_back(std::move(row));
			}
		}
		return true;
	};

	// 직접 수정할 출력과 분리된 새 트리다. 뒤의 문서 결합이나 admission 재확인 실패 시 그대로 버린다.
	VALTAN_PATTERN_TREE_VIEW Staged;
	Staged.strScriptedSequenceId = Master.ScriptedSequence.strSequenceId;
	Staged.strScriptedSequenceMode = Master.ScriptedSequence.strMode;
	Staged.strEntranceCinematicPatternId = Master.ScriptedSequence.strEntranceCinematicPatternId;
	Staged.iScriptedSequenceInterStepPursuitMs = Master.ScriptedSequence.iInterStepPursuitMs;
	Staged.ScriptedSequencePatternIds = Master.ScriptedSequence.PatternIds;
	Staged.ScriptedSequenceTransitionPursuitMs = Master.ScriptedSequence.TransitionPursuitMs;
	Staged.SelectionSets = Master.SelectionSets;
	Staged.SelectionWindows = Master.SelectionWindows;
	Staged.Mechanics = Master.Mechanics;
	Staged.ManualAuditions = Master.ManualAuditions;
	Staged.NormalSelection = Master.NormalSelection;
	Staged.IndependentEffects = Master.IndependentEffects;
	Staged.CounterReactionLayers = Master.CounterReactionLayers;

	// Master 패턴 순서대로 새 표시 패턴을 만든다. 저작 ID·출처와 표현 목록을 먼저 채운 뒤
	// 같은 순서로 만든 stage에 gameplay 값을 Overlay한다.
	for (const MASTER_PATTERN& Source : Master.Patterns)
	{
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = Source.strPatternId;
		Pattern.strActionId = Source.strActionId;
		Pattern.SourceActionIds = Source.SourceActionIds;
		Pattern.strSelectionMode = Source.strSelectionMode;
		Pattern.bAuthoringMasterManaged = true;
		Pattern.iSourceSequenceIndex = Source.iSourceSequenceIndex;
		Pattern.PresentationSources = Source.PresentationSources;
		Pattern.Reactions = Source.Reactions;
		Pattern.CameraCueIds = Source.CameraCueIds;
		Pattern.WorldEventTriggerRefs = Source.WorldEventTriggerRefs;
		Pattern.bManualServerAudition = Source.bManualServerAudition;
		Pattern.strSourceAnimationChainId = Source.strSourceAnimationChainId;
		Pattern.iAuthoringPhase = Source.iAuthoringPhase;
		Pattern.strAdmissionState = Source.strAdmissionState;
		Pattern.strEntryActionId = Source.Stages.empty() ? std::string{} : Source.Stages.front().strActionId;
		// 같은 패턴이 있으면 기존 CombatObjectEffects를 일시 복사한다. 뒤의 fillSourceInventory가 이 목록을 비우고 원본으로 다시 구성한다.
		// Previous 포인터 자체는 새 뷰에 저장하지 않는다.
		const VALTAN_PATTERN_VIEW* Previous = Find_Pattern(OutView, Source.strPatternId);
		for (const MASTER_STAGE& Row : Source.Stages)
		{
			VALTAN_STAGE_VIEW Stage;
			Stage.strStageId = Row.strStageId;
			Stage.strActionId = Row.strActionId;
			Stage.strSequenceRole = Row.strSequenceRole;
			Stage.iAuthoringRepeatCount = Row.iRepeatCount;
			Stage.strAnimationEndPolicy = Row.strAnimationEndPolicy;
			Stage.bSuppressAnimation = Row.bSuppressAnimation;
			Stage.ClipOccurrences = Row.Occurrences;
			Stage.ProductCues = Row.AuthoredCues;
			Stage.bHasStageEffectV2Bindings = Row.bHasStageEffectV2Bindings;
			Stage.CameraInvocations = Row.CameraInvocations;
			Stage.SceneProfileOccurrences = Row.SceneProfileOccurrences;
			Stage.LightOccurrences = Row.LightOccurrences;
			if (nullptr != Previous)
			{
				const auto OldStage = std::find_if(Previous->Stages.begin(), Previous->Stages.end(),
					[&Row](const auto& Value) { return Value.strStageId == Row.strStageId; });
				if (OldStage != Previous->Stages.end())
					Stage.CombatObjectEffects = OldStage->CombatObjectEffects;
			}

			// occurrence마다 clip 이름을 모으고 같은 clipOccurrenceId의 cue를 다시 연결한다.
			// 대표 runtime clip과 대표 cue는 각 목록이 비어 있지 않을 때 첫 항목으로 정한다.
			for (auto& Clip : Stage.ClipOccurrences)
			{
				Stage.RuntimeClipNames.push_back(Clip.strClipName);
				Clip.ProductCues.clear();
				for (const auto& Cue : Stage.ProductCues)
					if (Cue.strClipOccurrenceId == Clip.strClipOccurrenceId)
						Clip.ProductCues.push_back(Cue);
			}
			if (!Stage.RuntimeClipNames.empty())
				Stage.strRuntimeClipName = Stage.RuntimeClipNames.front();
			if (!Stage.ProductCues.empty())
				Stage.ProductCue = Stage.ProductCues.front();

			// 각 cue asset의 Effect 문서 경로를 표시 목록에 넣는다.
			// 여기서는 파일 존재를 검사하거나 Effect 문서를 파싱하지 않는다.
			for (const auto& Cue : Stage.ProductCues)
			{
				VALTAN_STAGE_EFFECT_VIEW Effect;
				Effect.strEffectAssetId = Cue.strEffectAssetId;
				Effect.DocumentPath = CProjectDataRoot::Resolve(std::filesystem::path(L"Effects") /
					L"Authored" / std::filesystem::path(Cue.strEffectAssetId + ".effect.json"));
				Effect.eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::PRODUCT_CUE;
				Stage.Effects.push_back(std::move(Effect));
			}
			for (const auto& Reference : Row.EffectReferences)
				if (Reference.strType != "CUE_BINDING")
					Stage.IndependentEffectIds.push_back(Reference.strId);
			Pattern.Stages.push_back(std::move(Stage));
		}

		// 새 stage와 Master의 순서·개수가 같은 상태에서 gameplay 값을 복사한다.
		// 이후 원본 소환 목록과 애니메이션 시간 예산을 완성한 패턴만 새 트리에 넣는다.
		Overlay_MasterGameplay(Source, Pattern);
		if (!fillSourceInventory(Pattern))
			return false;
		for (auto& Stage : Pattern.Stages)
			if (!Assign_MasterWallBudgets(Stage, strOutStatus))
				return false;
		if (Pattern.Is_Gimmick())
			Staged.Gimmicks.push_back(std::move(Pattern));
		else
			Staged.Rotation.push_back(std::move(Pattern));
	}
	// 기믹을 발동 체력 내림차순으로 정렬해 표시한다. 같은 체력의 추가 정렬 키는 여기서 사용하지 않는다.
	std::sort(Staged.Gimmicks.begin(), Staged.Gimmicks.end(),
		[](const VALTAN_PATTERN_VIEW& Left, const VALTAN_PATTERN_VIEW& Right)
		{
			return Left.iTriggerHealthBar > Right.iTriggerHealthBar;
		});

	// 아래 Phase는 triggerHealthBar로 나눈 화면용 체력 구간이다.
	// 기믹이 발동하는 체력까지 한 구간으로 묶으며 Server의 gameplay phase와는 별개다.
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	// 정렬된 기믹마다 해당 체력까지의 화면 구간을 만들고 다음 구간의 위쪽을 1줄 낮춘다.
	// 마지막에 체력 1까지 남은 구간이 있으면 관문 없는 구간으로 추가한다.
	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	// 일반 패턴은 자신의 체력 범위와 겹치는 모든 화면 구간에 연결한다.
	// 구간에는 Rotation의 index를 보관하므로 원본 vector 순서와 함께 사용해야 한다.
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	// 모든 원본 읽기와 트리 구성이 끝난 뒤 admission이 여전히 현재인지 재확인한다.
	// 통과한 Staged만 한 번 이동 대입하며 이 성공은 원본 보기 준비이지 게시/Server 실행 승인이 아니다.
	if (!Admission.Validate_StillCurrent(strOutStatus))
		return false;
	OutView = std::move(Staged);
	strOutStatus = "Loaded Valtan authoring source; Product publication is independent.";
	return true;
}

// 명시한 gameplay/presentation 경로와 Data 루트의 Product 문서를 결합한다.
// 이 함수 자체는 admission을 얻지 않는다. canonical 호출자는 외부에서 잠금을 유지하고 결과 후에도 재확인한다.
// strict는 게시 값 일치를 요구하고 restore는 안정된 topology 위에 저작 값을 복원한다. 성공한 임시 트리만 OutView를 교체한다.
// Load_WhileAdmitted와 Balance Tool의 저장본 복원이 호출한다. 지정된 원본 두 경로와 현재 Product를
// 지역 트리로 결합하며 명시적 false에서는 OutView를 보존한다. admission 잠금은 자체 획득하지 않는다.
bool_t Client::CValtanPatternTree::Load_FromAuthoringPaths(
	const std::filesystem::path& GameplayPath,
	const std::filesystem::path& PresentationPath,
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus,
	const VALTAN_PATTERN_TREE_LOAD_POLICY ePolicy,
	VALTAN_CANONICAL_READ_DIAGNOSTIC* const pOutDiagnostic)
{
	// 선택 진단을 초기화하고 기본 실패 종류를 Product 오류로 둔다. 아래 일부 경로만 상세 진단을
	// 갱신하므로 모든 false가 진단의 strStatus까지 채운다는 보장은 없고 문자열 출력도 함께 확인해야 한다.
	if (nullptr != pOutDiagnostic)
	{
		pOutDiagnostic->Clear();
		// 정책 값이 유효한 이후의 실패는 우선 Product 문서 오류로 분류한다.
		// 뒤의 원본 파서나 일치 검사가 원인을 알면 더 구체적인 진단으로 덮어쓴다.
		pOutDiagnostic->eFailure =
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID;
	}
	if (VALTAN_PATTERN_TREE_LOAD_POLICY::REQUIRE_ACTIVE_PRODUCT_PARITY !=
			ePolicy &&
		VALTAN_PATTERN_TREE_LOAD_POLICY::RESTORE_AUTHORING_SNAPSHOT != ePolicy)
	{
		strOutStatus = "Valtan pattern-tree load policy is invalid.";
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::ADMISSION_STATE_INVALID,
			strOutStatus);
		return false;
	}
	DATA_JSON_VALUE Encounter;
	DATA_JSON_VALUE GameplayRoot;
	DATA_JSON_VALUE PresentationRoot;
	DATA_JSON_VALUE PatternRotations;
	std::string Error;

	// Encounter·rotation·binding 등 게시 연결은 현재 Data 정본에서 읽는다.
	// GameplayPath/PresentationPath만 호출자가 지정하므로 저장본 복원도 현재 Product 연결을 필요로 한다.
	const std::filesystem::path EncounterRelative =
		std::filesystem::path(L"Encounters") / L"Valtan" /
		L"ValtanEncounter.json";
	const std::filesystem::path BindingRelative =
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternbindings.json";
	if (!Parse_Document(EncounterRelative, Encounter, Error))
	{
		strOutStatus = "Valtan encounter load failed: " + Error;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID,
			strOutStatus);
		return false;
	}
	if (!Parse_DocumentPath(GameplayPath, GameplayRoot, Error))
	{
		strOutStatus = "Valtan split gameplay authoring load failed: " + Error;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::SOURCE_INVALID,
			strOutStatus);
		return false;
	}
	if (!Parse_DocumentPath(PresentationPath, PresentationRoot, Error))
	{
		strOutStatus =
			"Valtan split presentation authoring load failed: " + Error;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::SOURCE_INVALID,
			strOutStatus);
		return false;
	}
	if (!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanPatternRotations.json", PatternRotations, Error))
	{
		strOutStatus = "Valtan normal-selection product load failed: " + Error;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID,
			strOutStatus);
		return false;
	}

	// 분리 원본을 지역 Master로 합치고 Effect V2 원본 범위를 연결한다.
	// 파서가 부분 출력을 남겨도 지역 값으로 폐기하며 이 단계의 실패는 SOURCE_INVALID로 구분한다.
	MASTER_DOCUMENT MasterDocument;
	if (!Parse_SplitMasterDocument(
			GameplayRoot, PresentationRoot, MasterDocument, Error) ||
		!Join_StageEffectV2SourceScopes(MasterDocument, Error))
	{
		strOutStatus = "Valtan split authoring strict join failed: " + Error;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::SOURCE_INVALID,
			strOutStatus);
		return false;
	}

	// 애니메이션 binding과 Effect cue는 기존 typed 문서 파서로 읽는다.
	// 여기서 별도의 느슨한 JSON 해석으로 같은 계약을 우회하지 않는다.
	std::string BindingText;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT BindingDocument;
	if (!Read_TextDocument(CProjectDataRoot::Resolve(BindingRelative),
			BindingText, Error) ||
		!CValtanPatternAnimationBindingDocument::Parse_Text(
			BindingText, BindingDocument, Error))
	{
		strOutStatus = "Valtan pattern bindings load failed: " + Error;
		return false;
	}

	// binding에 적힌 clip 이름을 그대로 availableClips에 전달해 typed 형식·중복·시간 계약을 검사한다.
	// 실제 모델의 clip 목록을 로드해 대조하는 검사는 이 호출에 포함되지 않는다.
	std::vector<std::string> DeclaredClipNames;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			DeclaredClipNames.push_back(Clip.strClipName);
	}
	if (!CValtanPatternAnimationBindingDocument::Validate(
			BindingDocument, "BOSS_VALTAN",
			DeclaredClipNames, Error))
	{
		strOutStatus = "Valtan pattern bindings validation failed: " + Error;
		return false;
	}

	// 게시 Effect cue는 전용 Product reader로 읽는다. 이후 action별로 모아
	// 각 stage/clip occurrence에 연결하고 마지막에 전체 cue 수가 해소됐는지 확인한다.
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
	if (!CValtanPatternEffectCueDocument::Load_ReadOnlyProduct(CueDocument, Error))
	{
		strOutStatus = "Valtan Product Effect cue load failed: " + Error;
		return false;
	}

	// 이동하는 연출은 보스 본체 cue가 아닌 Server 전투 오브젝트가 소유한다.
	// encounter의 생성 action, 전투 오브젝트 정의와 BossCatalog visual을 명시적으로 연결한다.
	// 도구에서도 실제 소유 stage 아래에 표시하여 관련 없는 항목에 숨기거나 이중 소유하지 않게 한다.
	DATA_JSON_VALUE BossCatalog;
	DATA_JSON_VALUE CombatObjects;
	if (!Parse_Document(std::filesystem::path(L"Actors") /
			L"BossCatalog.json", BossCatalog, Error) ||
		!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanCombatObjects.json", CombatObjects, Error))
	{
		strOutStatus = "Valtan combat-object authoring join failed: " + Error;
		return false;
	}
	std::map<std::string, COMBAT_OBJECT_EFFECT_REFERENCE, std::less<>>
		CombatObjectEffectsByArchetype;
	const DATA_JSON_VALUE* pBosses = BossCatalog.Find("bosses");
	const DATA_JSON_VALUE* pObjects = CombatObjects.Find("objects");
	if (nullptr == pBosses || !pBosses->Is_Array() ||
		nullptr == pObjects || !pObjects->Is_Array())
	{
		strOutStatus =
			"Valtan combat-object authoring documents have no rows.";
		return false;
	}

	// BossCatalog 전체 archetype ID의 안정성과 중복을 검사한 뒤 BOSS_VALTAN 행을 차용한다.
	// 차용 JSON 포인터는 이 함수의 지역 BossCatalog가 살아 있는 동안만 사용한다.
	const DATA_JSON_VALUE* pValtanBoss = nullptr;
	std::set<std::string, std::less<>> BossArchetypeIds;
	for (const DATA_JSON_VALUE& Boss : pBosses->Get_Array())
	{
		const std::string strBossArchetypeId = Boss.Is_Object() ?
			Read_String(Boss, "archetypeId") : std::string{};
		if (!Is_StableToken(strBossArchetypeId) ||
			!BossArchetypeIds.insert(strBossArchetypeId).second)
		{
			strOutStatus = "BossCatalog archetype identity is invalid or duplicated.";
			return false;
		}
		if (strBossArchetypeId == "BOSS_VALTAN")
		{
			pValtanBoss = &Boss;
		}
	}
	const DATA_JSON_VALUE* pVisuals = nullptr == pValtanBoss ? nullptr :
		pValtanBoss->Find("combatObjectVisuals");
	if (nullptr == pVisuals || !pVisuals->Is_Array() ||
		pVisuals->Get_Array().empty())
	{
		strOutStatus = "BOSS_VALTAN has no combat-object Effect visuals.";
		return false;
	}

	// 발탄 visual을 전투 오브젝트 archetype별 값으로 모은다. 이 map의 키 중복은 거절하지만
	// clientVisualId나 Effect asset ID가 서로 다른 archetype 사이에서 유일한지는 여기서 검사하지 않는다.
	for (const DATA_JSON_VALUE& Visual : pVisuals->Get_Array())
	{
		if (!Visual.Is_Object())
		{
			strOutStatus = "BOSS_VALTAN combat-object visual row is invalid.";
			return false;
		}
		const std::string strArchetypeId = Read_String(
			Visual, "combatObjectArchetypeId");
		COMBAT_OBJECT_EFFECT_REFERENCE Reference;
		Reference.strClientVisualId = Read_String(Visual, "clientVisualId");
		Reference.strEffectAssetId = Read_String(Visual, "effectAssetId");
		if (const auto* owns = Visual.Find("armedEffectOwnsTerminal"); owns && owns->Is_Boolean() && owns->Get_Boolean())
			Reference.strPreparationEventId = Read_String(Visual, "armedPresentationEventId");

		// 선택적 V2 group은 허용 필드 집합·group ID·양수 재생률을 검사한다.
		// visualHitMs/serverHitId는 함께 있는지와 필드 목록만 이 블록에서 확인하며 수치·참조를 검증하지 않는다.
		if (const DATA_JSON_VALUE* pEffectV2Group =
			Visual.Find("effectV2Group"); nullptr != pEffectV2Group)
		{
			if (!pEffectV2Group->Is_Object())
			{
				strOutStatus =
					"BOSS_VALTAN combat-object Effect V2 group is invalid.";
				return false;
			}
			const bool_t bHasVisualHit =
				nullptr != pEffectV2Group->Find("visualHitMs");
			const bool_t bHasServerHit =
				nullptr != pEffectV2Group->Find("serverHitId");
			const DATA_JSON_VALUE* pPlaybackRate =
				pEffectV2Group->Find("playbackRate");
			if (bHasVisualHit != bHasServerHit ||
				!(bHasVisualHit ?
					Has_ExactProperties(*pEffectV2Group,
						{ "groupId", "playbackRate", "visualHitMs", "serverHitId" }) :
					Has_ExactProperties(*pEffectV2Group,
						{ "groupId", "playbackRate" })) ||
				!Is_StableToken(Read_String(*pEffectV2Group, "groupId")) ||
				nullptr == pPlaybackRate || !Is_FiniteNumber(pPlaybackRate) ||
				pPlaybackRate->Get_Number() <= 0.0)
			{
				strOutStatus =
					"BOSS_VALTAN combat-object Effect V2 group is invalid.";
				return false;
			}
			Reference.strEffectV2GroupId =
				Read_String(*pEffectV2Group, "groupId");
		}
		if (strArchetypeId.empty() || Reference.strClientVisualId.empty() ||
			Reference.strEffectAssetId.empty() ||
			!CombatObjectEffectsByArchetype.emplace(
				strArchetypeId, std::move(Reference)).second)
		{
			strOutStatus =
				"BOSS_VALTAN combat-object visual identity is invalid or duplicated.";
			return false;
		}
	}

	// 등록한 visual과 같은 archetype을 가진 Product 오브젝트만 결합한다.
	// 객체가 아닌 행과 관련 없는 archetype은 건너뛰며 같은 visual의 중복 owner 행은 뒤에서 거절한다.
	std::set<std::string, std::less<>> DescribedCombatObjects;
	for (const DATA_JSON_VALUE& Object : pObjects->Get_Array())
	{
		if (!Object.Is_Object())
			continue;
		const std::string strArchetypeId = Read_String(
			Object, "combatObjectArchetypeId");
		auto Reference = CombatObjectEffectsByArchetype.find(strArchetypeId);
		if (Reference == CombatObjectEffectsByArchetype.end())
			continue;
		const std::string strClientVisualId = Read_String(
			Object, "clientVisualId");
		Reference->second.strOwnerPatternId = Read_String(
			Object, "ownerPatternId");
		Reference->second.strOwnerStageActionId = Read_String(
			Object, "ownerStageActionId");
		Reference->second.strKind = Read_String(Object, "kind");
		Reference->second.strOriginPolicy = Read_String(
			Object, "originPolicy");
		Reference->second.strDirectionPolicy = Read_String(
			Object, "directionPolicy");
		Reference->second.fCoverRadiusM = static_cast<float>(Read_Number(Object, "coverRadiusM"));
		if (!Read_CombatObjectOwnerHitChain(Object, Reference->second.OwnerHitChain))
		{
			strOutStatus = "Combat-object Product owner-hit chain is invalid: " + strArchetypeId;
			return false;
		}

		// 이동 속도는 m/s, 최대 거리는 m, 수명은 ms로 읽는다. 속도·거리는 유한한 비음수,
		// 수명은 양수 uint32를 요구하며 선택 coverRadiusM에는 이 블록의 동일한 검증을 적용하지 않는다.
		if (!Read_RequiredFiniteFloat(
				Object, "speedMps", Reference->second.fSpeedMps) ||
			!Read_RequiredFiniteFloat(Object, "maximumDistanceM",
				Reference->second.fMaximumDistanceM) ||
			!Read_RequiredUInt32(
				Object, "lifeMs", Reference->second.iLifetimeMs) ||
			Reference->second.strKind.empty() ||
			Reference->second.strOriginPolicy.empty() ||
			Reference->second.strDirectionPolicy.empty() ||
			Reference->second.fSpeedMps < 0.f ||
			Reference->second.fMaximumDistanceM < 0.f ||
			0u == Reference->second.iLifetimeMs)
		{
			strOutStatus =
				"Valtan combat-object Product motion/timing is invalid: " +
				strArchetypeId;
			return false;
		}

		// hit 배열은 필수이고 표현 이벤트 배열은 선택이다. 두 목록의 합을 1~16행으로 제한한 뒤
		// 각 목록의 identity와 시계를 별도 검사해 stage 표시용 값으로 복사한다.
		const DATA_JSON_VALUE* pHits = Object.Find("hits");
		const DATA_JSON_VALUE* pPresentationEvents =
			Object.Find("presentationEvents");
		if (nullptr == pHits || !pHits->Is_Array() ||
			(nullptr != pPresentationEvents && !pPresentationEvents->Is_Array()))
		{
			strOutStatus =
				"Valtan combat-object has no Server hit or presentation rows: " +
				strArchetypeId;
			return false;
		}
		const size_t iPresentationEventCount =
			nullptr == pPresentationEvents ? 0u :
			pPresentationEvents->Get_Array().size();
		const size_t iEventRowCount =
			pHits->Get_Array().size() + iPresentationEventCount;
		if (0u == iEventRowCount || iEventRowCount > 16u)
		{
			strOutStatus =
				"Valtan combat-object Server hit/presentation row count is invalid: " +
				strArchetypeId;
			return false;
		}

		// hit ID는 이 오브젝트 안에서 중복을 거절하고 at/repeat 시계는 uint32로 읽는다.
		// RING 형상만 아래에서 반지름 관계를 추가 검사하며 모든 hit 시각의 수명 내 포함을 여기서 검사하지 않는다.
		for (const DATA_JSON_VALUE& Hit : pHits->Get_Array())
		{
			uint32_t iHitOffsetMs = 0u;
			const std::string strHitId = Read_String(Hit, "hitId");
			const std::string strHitShape = Read_String(Hit, "hitShape");
			if (!Hit.Is_Object() ||
				strHitId.empty() ||
				strHitShape.empty() ||
				!Read_RequiredUInt32(Hit, "atMs", iHitOffsetMs) ||
				std::find(Reference->second.HitIds.begin(),
					Reference->second.HitIds.end(), strHitId) !=
					Reference->second.HitIds.end())
			{
				strOutStatus =
					"Valtan combat-object Server hit identity or clock is invalid: " +
					strArchetypeId;
				return false;
			}
			Client::VALTAN_COMBAT_OBJECT_HIT_VIEW HitView;
			HitView.strHitId = strHitId;
			HitView.strTriggerKind = Read_String(Hit, "trigger");
			HitView.iAtMs = iHitOffsetMs;
			if (!Read_RequiredUInt32(Hit, "repeatCount", HitView.iRepeatCount) ||
				!Read_RequiredUInt32(Hit, "repeatIntervalMs", HitView.iRepeatIntervalMs))
			{
				strOutStatus = "Valtan combat-object Server hit repeat clock is invalid: " + strArchetypeId;
				return false;
			}
			HitView.strHitShape = strHitShape;
			HitView.fOuterRadiusM = static_cast<float>(Read_Number(Hit, "hitOuterRadius"));
			HitView.strServerDamageProfileId = Read_String(Hit, "serverDamageProfileId");
			HitView.fPushRangeM = static_cast<float>(Read_Number(Hit, "pushRangeM"));
			HitView.iPushMs = static_cast<uint32_t>(Read_Number(Hit, "pushMs"));
			HitView.iDownMs = static_cast<uint32_t>(Read_Number(Hit, "downMs"));
			if (const auto* knockdown = Hit.Find("knockdown"))
				HitView.bKnockdown = knockdown->Get_Boolean();
			if ("RING" == strHitShape &&
				(!Read_RequiredFiniteFloat(
					Hit, "hitInnerRadius", HitView.fInnerRadiusM) ||
				 !Read_RequiredFiniteFloat(
					Hit, "hitOuterRadius", HitView.fOuterRadiusM) ||
				 HitView.fInnerRadiusM < 0.f ||
				 HitView.fInnerRadiusM >= HitView.fOuterRadiusM ||
				 HitView.fOuterRadiusM > 100000.f))
			{
				strOutStatus =
					"Valtan combat-object RING geometry is invalid: " +
					strArchetypeId + "/" + strHitId;
				return false;
			}
			Reference->second.HitIds.push_back(strHitId);
			Reference->second.HitOffsetsMs.push_back(iHitOffsetMs);
			Reference->second.Hits.push_back(std::move(HitView));
		}

		// 표현 이벤트는 안정적인 ID와 수명 이하의 시각을 요구한다.
		// 표현 ID끼리의 중복과 앞서 읽은 hit ID와의 충돌을 모두 거절한다.
		if (nullptr != pPresentationEvents)
		{
			std::set<std::string, std::less<>> PresentationEventIds;
			for (const DATA_JSON_VALUE& Event :
				pPresentationEvents->Get_Array())
			{
				uint32_t iEventOffsetMs = 0u;
				const std::string strEventId = Read_String(
					Event, "presentationEventId");
				if (!Event.Is_Object() ||
					!Is_StableToken(strEventId) ||
					!Read_RequiredUInt32(Event, "atMs", iEventOffsetMs) ||
					iEventOffsetMs > Reference->second.iLifetimeMs ||
					!PresentationEventIds.insert(strEventId).second ||
					std::find(Reference->second.HitIds.begin(),
						Reference->second.HitIds.end(), strEventId) !=
						Reference->second.HitIds.end())
				{
					strOutStatus =
						"Valtan combat-object presentation identity or clock is invalid: " +
						strArchetypeId;
					return false;
				}
				Client::VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW View;
				View.strPresentationEventId = strEventId;
				View.iAtMs = iEventOffsetMs;
				Reference->second.PresentationEvents.push_back(std::move(View));
			}
		}
		if (strClientVisualId != Reference->second.strClientVisualId ||
			Reference->second.strOwnerPatternId.empty() ||
			Reference->second.strOwnerStageActionId.empty() ||
			!DescribedCombatObjects.insert(strArchetypeId).second)
		{
			strOutStatus =
				"Valtan combat-object owner or visual identity changed: " +
				strArchetypeId;
			return false;
		}
	}

	// visual마다 일치하는 오브젝트 owner 행이 정확히 하나 연결됐는지 수를 맞춘다.
	// 뒤의 stage action 결합에서도 각 archetype이 한 번만 해소되는지 별도로 검사한다.
	if (DescribedCombatObjects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"BossCatalog combat-object Effect visual has no Valtan owner row.";
		return false;
	}

	// Server 전투 오브젝트가 소유한 독립 Effect는 Product의 owner pattern을 맞춘 뒤
	// 현재 visual/asset 값을 보충한다. 실제 owner stage action과의 결합은 아래 소환 action에서 검사한다.
	for (VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
		MasterDocument.IndependentEffects)
	{
		if ("SERVER_COMBAT_OBJECT" != Independent.strOwnership)
			continue;
		const auto Product = CombatObjectEffectsByArchetype.find(
			Independent.strCombatObjectArchetypeId);
		if (Product == CombatObjectEffectsByArchetype.end() ||
			Product->second.strOwnerPatternId != Independent.strOwnerPatternId ||
			Product->second.strOwnerStageActionId.empty())
		{
			strOutStatus =
				"Valtan split combat-object independent Effect owner changed: " +
				Independent.strIndependentEffectId;
			return false;
		}
		Independent.strClientVisualId = Product->second.strClientVisualId;
		Independent.strEffectAssetId = Product->second.strEffectAssetId;
	}

	// 선택적인 Effect binding 파일은 없을 수 있다.
	// 새 stage Effect 문서가 아직 patterneffects.json에 연결되지 않은 상태만으로 트리 전체를 거절하지 않는다.
	// 선택 파일의 읽기·JSON 파싱 실패도 이 분기는 건너뛴다. 성공한 문서 안에서는 객체/비어 있지 않은
	// action만 사용하고 같은 action이 여러 번 나오면 마지막 행으로 덮어쓴다.
	std::map<std::string, std::pair<std::string, std::string>, std::less<>>
		EffectByAction;
	DATA_JSON_VALUE Effects;
	if (Parse_Document(std::filesystem::path(L"Animation") / L"Authored" /
		L"Valtan" / L"Valtan.patterneffects.json", Effects, Error))
	{
		const DATA_JSON_VALUE* pRows = Effects.Find("bindings");
		if (nullptr != pRows && pRows->Is_Array())
		{
			for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
			{
				if (!Row.Is_Object())
					continue;
				const std::string strAction = Read_String(Row, "actionId");
				if (strAction.empty())
					continue;
				EffectByAction[strAction] = {
					Read_String(Row, "effectAssetId"),
					Read_String(Row, "effectDocument") };
			}
		}
	}

	// 검증한 binding을 action별 occurrence·애니메이션 억제·몸체 숨김 구간으로 나눈다.
	// clip 목록은 값으로 복사하며 모델이나 애니메이션 GPU 자원을 여기서 생성하지 않는다.
	std::map<std::string, std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>, std::less<>>
		ClipsByAction;
	std::set<std::string, std::less<>> SuppressedAnimationActions;
	std::map<std::string, std::array<uint32_t, 2u>, std::less<>>
		BodyHiddenWindowByAction;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		Occurrences.reserve(Binding.Clips.size());
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			Occurrences.push_back(Build_ClipOccurrenceView(Clip));
		ClipsByAction.emplace(Binding.strActionId, std::move(Occurrences));
		if (Binding.bSuppressAnimation)
			SuppressedAnimationActions.insert(Binding.strActionId);
		if (Binding.bHasBodyHiddenWindow)
		{
			BodyHiddenWindowByAction.emplace(
				Binding.strActionId,
				std::array<uint32_t, 2u>{ Binding.iBodyHiddenFromMs,
					Binding.iBodyHiddenToMs });
		}
	}

	// typed cue의 시계·좌표·추종/종료/반복 정책을 표시 View로 복사한다.
	// action별 목록은 원래 cue 순서를 유지하며 같은 action의 여러 cue를 보관한다.
	std::map<std::string, std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW>,
		std::less<>>
		CueByAction;
	const size_t iCueCount = CueDocument.Cues.size();
	for (const VALTAN_PATTERN_EFFECT_CUE& SourceCue : CueDocument.Cues)
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
		Cue.strBindingId = SourceCue.strBindingId;
		Cue.strOccurrenceId = SourceCue.strOccurrenceId;
		Cue.strPatternId = SourceCue.strPatternId;
		Cue.strStageId = SourceCue.strStageId;
		Cue.strActionId = SourceCue.strActionId;
		Cue.strClipOccurrenceId = SourceCue.strClipOccurrenceId;
		Cue.strEffectAssetId = SourceCue.strEffectAssetId;
		Cue.strV1EffectAssetId = SourceCue.strV1EffectAssetId;
		Cue.strAnchorSlotId = SourceCue.strAnchorSlotId;
		Cue.LocalTransform = SourceCue.LocalTransform;
		Cue.eFollowPolicy = SourceCue.eFollowPolicy;
		Cue.eStopPolicy = SourceCue.eStopPolicy;
		Cue.strFollowPolicy = Describe_FollowPolicy(SourceCue.eFollowPolicy);
		Cue.strStopPolicy = Describe_StopPolicy(SourceCue.eStopPolicy);
		Cue.strRepeatPolicy = Describe_RepeatPolicy(SourceCue.eRepeatPolicy);
		Cue.eScalePolicy = SourceCue.eScalePolicy;
		Cue.strScalePolicy = Describe_ScalePolicy(SourceCue.eScalePolicy);
		Cue.vWorldScale = SourceCue.vWorldScale;
		Cue.bHasExplicitScalePolicy = SourceCue.bHasExplicitScalePolicy;
		Cue.bUsesStageClock = SourceCue.bUsesStageClock;
		Cue.iStageOffsetMs = SourceCue.bUsesStageClock ?
			SourceCue.iStartMs : 0u;
		Cue.iSourceStartMs = SourceCue.iStartMs;
		Cue.iSourceEndMs = SourceCue.iEndMs;
		Cue.bHasPlaybackOffset = SourceCue.bHasPlaybackOffset;
		Cue.iPlaybackOffsetMs = SourceCue.iPlaybackOffsetMs;
		Cue.iStageDurationMs = SourceCue.iStageDurationMs;
		Cue.bHasSourceEnd = SourceCue.bHasSourceEnd;
		CueByAction[Cue.strActionId].push_back(std::move(Cue));
	}

	const DATA_JSON_VALUE* pPatterns = Encounter.Find("patterns");
	if (nullptr == pPatterns || !pPatterns->Is_Array())
	{
		strOutStatus = "Valtan encounter has no patterns array.";
		return false;
	}

	// Product의 패턴을 먼저 임시 트리로 만든 뒤 Master와의 일치 또는 복원 결합을 적용한다.
	// 여기서 만든 원소가 실패 중 호출자의 기존 View를 오염시키지 않도록 마지막까지 분리한다.
	VALTAN_PATTERN_TREE_VIEW Staged;
	size_t iResolvedCueCount = 0u;
	std::set<std::string, std::less<>> ResolvedCombatObjectEffects;
	std::set<std::string, std::less<>> ResolvedVolleyProjectionIds;
	const std::string strEncounterBossArchetypeId = Read_String(
		Encounter, "bossArchetypeId");
	const std::string strEncounterId = Read_String(Encounter, "encounterId");
	if (strEncounterBossArchetypeId != "BOSS_VALTAN" ||
		strEncounterId != "ENCOUNTER_VALTAN" ||
		!BossArchetypeIds.contains(strEncounterBossArchetypeId))
	{
		strOutStatus = "Valtan encounter dataset identity is invalid.";
		return false;
	}

	// Product pattern의 gameplay 값을 임시 Pattern에 읽는다. 비객체 행은 건너뛰며
	// 원본 Master와의 identity·값 일치 또는 복원 결합은 아래 Apply_MasterDocument가 이어 처리한다.
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!PatternValue.Is_Object())
			continue;
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = Read_String(PatternValue, "patternId");
		Pattern.strCategory = Read_String(PatternValue, "category");
		Pattern.strTargetPolicy = Read_String(PatternValue, "targetPolicy");
		Pattern.strAimPolicy = Read_String(PatternValue, "aimPolicy");
		Pattern.strDisplayName = Read_String(PatternValue, "displayName");
		Pattern.strActionId = Read_String(PatternValue, "actionId");
		Pattern.strSelectionMode = Read_String(PatternValue, "selectionMode");
		Pattern.strArmorRequirement = Read_String(
			PatternValue, "armorRequirement");
		Pattern.strPhaseRequirement = Read_String(
			PatternValue, "phaseRequirement");

		// 필수 선택/범위 필드는 uint32 또는 유한 수로 읽고 optional verticalOffsetM은 ±100m의 0 아닌 값만 허용한다.
		// 여기서 형식 검사를 통과한 모든 수치가 gameplay 범위 검증까지 끝났다는 뜻은 아니다.
		const DATA_JSON_VALUE* pVerticalOffset =
			PatternValue.Find("verticalOffsetM");
		uint32_t iMinimumHealthBar = 0u;
		uint32_t iMaximumHealthBar = 0u;
		uint32_t iTriggerHealthBar = 0u;
		const DATA_JSON_VALUE* pSourceActionIds = Required(
			PatternValue, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pInvulnerable = Required(
			PatternValue, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN);
		if (Pattern.strPatternId.empty() || Pattern.strCategory.empty() ||
			Pattern.strTargetPolicy.empty() || Pattern.strAimPolicy.empty() ||
			Pattern.strDisplayName.empty() || Pattern.strActionId.empty() ||
			Pattern.strSelectionMode.empty() ||
			Pattern.strArmorRequirement.empty() ||
			Pattern.strPhaseRequirement.empty() || nullptr == pInvulnerable ||
			nullptr == pSourceActionIds ||
			(pSourceActionIds->Get_Array().empty() &&
			 "AUDITION_ONLY" != Pattern.strSelectionMode) ||
			!Read_RequiredUInt32(
				PatternValue, "minimumPhase", Pattern.iMinimumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumPhase", Pattern.iMaximumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "minimumHealthBar", iMinimumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumHealthBar", iMaximumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerHealthBar", iTriggerHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerOrder", Pattern.iTriggerOrder) ||
			!Read_RequiredUInt32(
				PatternValue, "selectionWeight", Pattern.iSelectionWeight) ||
			!Read_RequiredUInt32(PatternValue, "maximumConsecutiveUses",
				Pattern.iMaximumConsecutiveUses) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "minimumRange", Pattern.fMinimumRange) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "maximumRange", Pattern.fMaximumRange) ||
			(nullptr != pVerticalOffset &&
			 (!Is_FiniteNumber(pVerticalOffset) ||
			  0.0 == pVerticalOffset->Get_Number() ||
			  std::fabs(pVerticalOffset->Get_Number()) > 100.0)) ||
			!Read_PatternServerMotion(
				PatternValue.Find("serverMotion"), Pattern.ServerMotion) ||
			!Read_PatternFinale(PatternValue.Find("finale"), Pattern.Finale) ||
			(Pattern.Finale.has_value() &&
				(pInvulnerable->Get_Boolean() ||
				 Pattern.Finale->strGhostArchetypeId == strEncounterBossArchetypeId ||
				 !BossArchetypeIds.contains(Pattern.Finale->strGhostArchetypeId))))
		{
			strOutStatus =
				"Valtan encounter pattern gameplay projection is invalid: " +
				Pattern.strPatternId;
			return false;
		}
		Pattern.iMinimumHealthBar = static_cast<int32_t>(iMinimumHealthBar);
		Pattern.iMaximumHealthBar = static_cast<int32_t>(iMaximumHealthBar);
		Pattern.iTriggerHealthBar = static_cast<int32_t>(iTriggerHealthBar);
		Pattern.bInvulnerableWhileRunning = pInvulnerable->Get_Boolean();
		Pattern.fVerticalOffsetM = nullptr == pVerticalOffset ? 0.f :
			static_cast<f32_t>(pVerticalOffset->Get_Number());

		// sourceActionIds는 uint32 상한 안의 양의 정수 표현인지 공통 helper로 확인해 옮긴다.
		// 이 반복문 자체는 같은 ID의 중복을 거절하지 않는다.
		for (const DATA_JSON_VALUE& SourceAction :
			pSourceActionIds->Get_Array())
		{
			if (!Is_NonNegativeInteger(&SourceAction) ||
				0.0 == SourceAction.Get_Number())
			{
				strOutStatus = "Valtan encounter sourceActionIds is invalid: " +
					Pattern.strPatternId;
				return false;
			}
			Pattern.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}

		// stages가 배열이면 각 객체 행을 지역 Stage로 만든다. 누락·비배열 또는 비객체 행은
		// 이 반복문에서 바로 실패하지 않으므로 뒤의 원본 결합과 같은 검사를 함께 읽어야 한다.
		const DATA_JSON_VALUE* pStages = PatternValue.Find("stages");
		if (nullptr != pStages && pStages->Is_Array())
		{
			for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
			{
				if (!StageValue.Is_Object())
					continue;
				VALTAN_STAGE_VIEW Stage;
				Stage.strStageId = Read_String(StageValue, "stageId");
				Stage.strActionId = Read_String(StageValue, "actionId");
				Stage.strStageKind = Read_String(StageValue, "stageKind");
				Stage.strHitShape = Read_String(StageValue, "hitShape");
				const DATA_JSON_VALUE* pStageVerticalOffset =
					StageValue.Find("verticalOffsetM");
				const bool_t bValidStageKind =
					"WINDUP" == Stage.strStageKind ||
					"ACTIVE" == Stage.strStageKind ||
					"RECOVERY" == Stage.strStageKind ||
					"GROGGY" == Stage.strStageKind ||
					"PART_BREAK" == Stage.strStageKind;
				const bool_t bValidHitShape =
					"NONE" == Stage.strHitShape ||
					"CIRCLE" == Stage.strHitShape ||
					"RING" == Stage.strHitShape ||
					"CONE" == Stage.strHitShape ||
					"BOX" == Stage.strHitShape ||
					"CROSS" == Stage.strHitShape ||
					"SIX_DIRECTIONS" == Stage.strHitShape;
				if (Stage.strStageId.empty() || Stage.strActionId.empty() ||
					!bValidStageKind || !bValidHitShape)
				{
					strOutStatus =
						"Valtan encounter stage identity is missing or invalid";
					return false;
				}

				// stage 기간/공격 시계는 ms, 형상 길이는 m, 부채꼴 각도는 degree 값으로 읽는다.
				// 타격 시각 목록과 접촉 정보는 이어 전용 reader와 schedule 검사를 통과해야 한다.
				if ((nullptr != pStageVerticalOffset &&
					 (!Is_FiniteNumber(pStageVerticalOffset) ||
					  0.0 == pStageVerticalOffset->Get_Number() ||
					  std::fabs(pStageVerticalOffset->Get_Number()) > 100.0)) ||
					!Read_RequiredUInt32(
						StageValue, "durationMs", Stage.iDurationMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitCount", Stage.iHitCount) ||
					!Read_RequiredUInt32(
						StageValue, "hitIntervalMs", Stage.iHitIntervalMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitDelayMs", Stage.iHitDelayMs) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitOuterRadius", Stage.fHitOuterRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitInnerRadius", Stage.fHitInnerRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitAngleDegrees", Stage.fHitAngleDegrees) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitLength", Stage.fHitLength) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitHalfWidth", Stage.fHitHalfWidth))
				{
					strOutStatus =
						"Valtan encounter stage numeric field is missing or invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.fVerticalOffsetM = nullptr == pStageVerticalOffset ? 0.f :
					static_cast<f32_t>(pStageVerticalOffset->Get_Number());
				if (!Read_OptionalOrderedHitOffsets(
						StageValue, Stage.HitOffsetsMs) ||
					!Client::Parse_ValtanStageAttackContacts(StageValue.Find("attackContacts"), Stage.AttackContacts))
				{
					strOutStatus =
						"Valtan encounter stage hitOffsetsMs is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.strServerDamageProfileId = Read_String(
					StageValue, "serverDamageProfileId");

				// 피격 응답·부착 슬롯·손의 지역 offset은 셋 모두 있거나 모두 없어야 한다.
				// 있으면 값을 복사하고 뒤에서 DAMAGE/CAPTURE의 허용 부착·밀림·다운 조합을 확인한다.
				const DATA_JSON_VALUE* pPlayerResponse =
					StageValue.Find("playerResponse");
				const DATA_JSON_VALUE* pAttachmentSlot =
					StageValue.Find("attachmentSlot");
				const DATA_JSON_VALUE* pGripLocalOffset =
					StageValue.Find("gripLocalOffset");
				if ((nullptr == pPlayerResponse) != (nullptr == pAttachmentSlot) ||
					(nullptr == pPlayerResponse) != (nullptr == pGripLocalOffset) ||
					!Read_PlayerHandGripLocalOffset(
						pGripLocalOffset, Stage.GripLocalOffset) ||
					(nullptr != pPlayerResponse &&
					 (!pPlayerResponse->Is_String() ||
					  !pAttachmentSlot->Is_String())))
				{
					strOutStatus =
						"Valtan encounter capture hit fields are invalid: " +
						Stage.strActionId;
					return false;
				}
				if (nullptr != pPlayerResponse)
				{
					Stage.strPlayerResponse = pPlayerResponse->Get_String();
					Stage.strAttachmentSlot = pAttachmentSlot->Get_String();
				}

				// 피해 응답과 조준·이동·action·분기·보스 응답·판정 권한 확장을 각각의 reader로 읽는다.
				// 부분 작성 중 하나라도 실패하면 아직 지역 변수인 Stage와 전체 Staged를 버린다.
				const DATA_JSON_VALUE* pKnockdown = Required(
					StageValue, "knockdown", DATA_JSON_TYPE::BOOLEAN);
				if (nullptr == Required(StageValue, "serverDamageProfileId",
						DATA_JSON_TYPE::STRING) ||
					nullptr == pKnockdown ||
					!Read_RequiredFiniteFloat(
						StageValue, "pushRangeM", Stage.fPushRangeM) ||
					!Read_RequiredUInt32(
						StageValue, "pushMs", Stage.iPushMs) ||
					!Read_RequiredUInt32(
						StageValue, "downMs", Stage.iDownMs) ||
					!Read_StageAim(StageValue.Find("aim"), Stage.iDurationMs, Stage.strAimTargetPolicy,
						Stage.bHasAimEnd, Stage.iAimEndMs, Stage.bHasAimResponseScale, Stage.fAimResponseScale) ||
					!Read_StageMotion(StageValue.Find("motion"),
						Stage.iDurationMs, Stage.Motion) ||
					!Read_StageActions(StageValue.Find("actions"),
						Stage.iDurationMs, Stage.Actions) ||
					!Read_StageBranches(StageValue.Find("branches"), Stage.Branches) ||
					!Has_ValidNavigationBlockedCapture(Stage.Branches, Stage.strPlayerResponse) ||
					!Read_StageGameplayExtensions(StageValue,
						Stage.strPartDamagePolicy, Stage.CounterProxy,
						Stage.BossResponse) ||
					!Read_StageHitAuthority(
						StageValue, Stage.iDurationMs, Stage.bHasHitAnchor,
						Stage.strHitAnchorKind,
						Stage.fHitAnchorForwardOffsetM,
						Stage.fHitAnchorRightOffsetM,
						Stage.fHitAnchorYawOffsetDegrees,
						Stage.bHasHitActivation,
						Stage.iHitActivationStartMs,
						Stage.iHitActivationLifetimeMs))
				{
					strOutStatus =
						"Valtan encounter stage gameplay projection is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.bKnockdown = pKnockdown->Get_Boolean();

				// 체력 피해 threshold 분기는 BossResponse 존재와 ACTIVE stage 조건에 맞춰야 한다.
				// 수직 offset의 응답/이동 제한과 counter proxy 종류별 허용 stage도 함께 검사한다.
				const std::size_t iHealthThresholdCount =
					static_cast<std::size_t>(std::count_if(
						Stage.Branches.begin(), Stage.Branches.end(),
						[](const VALTAN_STAGE_BRANCH_VIEW& Branch)
						{
							return "HEALTH_DAMAGE_THRESHOLD_REACHED" ==
								Branch.strOutcome;
						}));
				if ((Stage.BossResponse.has_value() &&
					 ("ACTIVE" != Stage.strStageKind ||
					  1u != iHealthThresholdCount)) ||
					(!Stage.BossResponse.has_value() &&
					 0u != iHealthThresholdCount) ||
					(0.f != Stage.fVerticalOffsetM &&
					 ((!Stage.BossResponse.has_value() && !Has_ClosedStaggerStageResponse(StageValue)) || Stage.Motion.has_value())) ||
					(Stage.CounterProxy.has_value() &&
					 (("BOSS_FORWARD_ARC" == Stage.CounterProxy->strKind &&
					   "WINDUP" != Stage.strStageKind &&
					   "ACTIVE" != Stage.strStageKind) ||
					  ("BOSS_LOCAL_CIRCLE" == Stage.CounterProxy->strKind &&
					   "WINDUP" != Stage.strStageKind))))
				{
					strOutStatus =
						"Valtan encounter stage response/proxy contract is invalid: " +
						Stage.strActionId;
					return false;
				}

				// 추적 억제 action은 지정된 유령 사망 시험의 마지막 stage에서 한 번만 허용하고 분기를 금지한다.
				// 일반 패턴 전체에 적용하는 이동 옵션으로 해석하지 않는다.
				const std::size_t iPursuitSuppressionCount =
					static_cast<std::size_t>(std::count_if(
						Stage.Actions.begin(), Stage.Actions.end(),
						[](const VALTAN_STAGE_ACTION_VIEW& Action)
						{
							return "SUPPRESS_INTER_STEP_PURSUIT" ==
								Action.strKind;
						}));
				if (0u != iPursuitSuppressionCount &&
					(1u != iPursuitSuppressionCount ||
					 "VALTAN_GHOST_DEATH_AUDITION" != Pattern.strPatternId ||
					 "STEP_01" != Stage.strStageId ||
					 "valtan.sequence.dead.step-01" != Stage.strActionId ||
					 &StageValue != &pStages->Get_Array().back() ||
					 !Stage.Branches.empty()))
				{
					strOutStatus =
						"Valtan encounter inter-step pursuit suppression owner is invalid";
					return false;
				}
				if (("DAMAGE" == Stage.strPlayerResponse &&
					 "NONE" != Stage.strAttachmentSlot) ||
					("CAPTURE" == Stage.strPlayerResponse &&
					 ("BOSS_LEFT_HAND" != Stage.strAttachmentSlot ||
					  "NONE" == Stage.strHitShape ||
					  0.f != Stage.fPushRangeM || 0u != Stage.iPushMs ||
					  Stage.bKnockdown || 0u != Stage.iDownMs)) ||
					("DAMAGE" != Stage.strPlayerResponse &&
					 "CAPTURE" != Stage.strPlayerResponse))
				{
					strOutStatus =
						"Valtan encounter player response contract is invalid: " +
						Stage.strActionId;
					return false;
				}

				// Shared와 같은 접촉 판정 시계 검사를 사용한다. 접촉 목록이 있으면 별도 활성창과 함께 쓰지 않고
				// 중앙 이동 외 motion·CAPTURE·NONE 형상을 거절해 두 판정 소유 방식이 섞이지 않게 한다.
				if (!LostArk::Shared::Validate_StageAttackContacts(Stage.AttackContacts,
					Stage.iDurationMs, Stage.HitOffsetsMs, Stage.iHitCount, Stage.iHitDelayMs, Stage.iHitIntervalMs) ||
					(!Stage.AttackContacts.empty() && (Stage.bHasHitActivation ||
						(Stage.Motion.has_value() && Stage.Motion->strKind != "TO_ARENA_CENTER") ||
						Stage.strPlayerResponse != "DAMAGE" || Stage.strHitShape == "NONE")))
				{
					strOutStatus = "Valtan encounter stage attackContacts contract is invalid";
					return false;
				}

				// 명시적 offset 목록 또는 delay+interval 시계의 마지막 타격은 stage 끝보다 빨라야 한다.
				// 타격이 없는 시계도 허용하지만 지속 활성창은 비어 있는 시계와 NONE이 아닌 형상을 함께 요구한다.
				const bool_t bHasExplicitOffsets = !Stage.HitOffsetsMs.empty();
				const bool_t bValidExplicitSchedule = bHasExplicitOffsets &&
					Stage.HitOffsetsMs.size() == Stage.iHitCount &&
					0u == Stage.iHitIntervalMs && 0u == Stage.iHitDelayMs &&
					Stage.HitOffsetsMs.back() < Stage.iDurationMs;
				const bool_t bValidLegacySchedule = !bHasExplicitOffsets &&
					Stage.iHitCount > 0u &&
					(1u == Stage.iHitCount ? 0u == Stage.iHitIntervalMs :
						Stage.iHitIntervalMs > 0u) &&
					static_cast<uint64_t>(Stage.iHitDelayMs) +
						static_cast<uint64_t>(Stage.iHitCount - 1u) *
							Stage.iHitIntervalMs < Stage.iDurationMs;
				const bool_t bValidEmptySchedule = 0u == Stage.iHitCount &&
					!bHasExplicitOffsets && 0u == Stage.iHitIntervalMs &&
					0u == Stage.iHitDelayMs;
				const bool_t bValidActiveWindow = Stage.bHasHitActivation &&
					bValidEmptySchedule && "NONE" != Stage.strHitShape;
				if ((!Stage.bHasHitActivation && !bValidExplicitSchedule &&
					 !bValidLegacySchedule && !bValidEmptySchedule) ||
					(Stage.bHasHitActivation && !bValidActiveWindow) ||
					("NONE" == Stage.strHitShape &&
					 (Stage.bHasHitAnchor || Stage.bHasHitActivation)))
				{
					strOutStatus =
						"Valtan encounter stage hit schedule is invalid: " +
						Stage.strActionId;
					return false;
				}

				// 소환 action만 골라 앞서 읽은 visual/오브젝트 정의를 소유 stage 아래에 연결한다.
				// 여기서는 표현 View를 구성할 뿐 오브젝트를 생성하거나 Server 소환 명령을 보내지 않는다.
				const DATA_JSON_VALUE* pActions = StageValue.Find("actions");
				if (nullptr != pActions && pActions->Is_Array())
				{
					for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
					{
						const std::string strSpawnKind =
							Read_String(Action, "kind");
						if (!Action.Is_Object() ||
							("SPAWN_COMBAT_OBJECT" != strSpawnKind &&
							 "SPAWN_COMBAT_OBJECT_VOLLEY" != strSpawnKind))
						{
							continue;
						}
						const std::string strTargetId = Read_String(
							Action, "targetId");
						const auto Reference =
							CombatObjectEffectsByArchetype.find(strTargetId);
						const double SpawnValue =
							"SPAWN_COMBAT_OBJECT_VOLLEY" == strSpawnKind ?
								Read_Number(Action, "countPerResolvedTarget") :
								Read_Number(Action, "value");

						// 원본 volley 투영은 pattern/action/archetype으로 찾는다. 엄격 정책은 Product action의 JSON 값과
						// event의 단일 해소까지 비교하고, 저장본 복원 정책은 이 동등성 비교를 건너뛴다.
						const auto ExpectedVolley = std::find_if(
							MasterDocument.VolleyProjections.begin(),
							MasterDocument.VolleyProjections.end(),
							[&Pattern, &Stage, &strTargetId](
								const MASTER_VOLLEY_PROJECTION& Projection)
							{
								return Projection.strPatternId ==
										Pattern.strPatternId &&
									Projection.strActionId == Stage.strActionId &&
									Projection.strCombatObjectArchetypeId ==
										strTargetId;
							});
						const bool_t bProductVolley =
							"SPAWN_COMBAT_OBJECT_VOLLEY" == strSpawnKind;
						if (VALTAN_PATTERN_TREE_LOAD_POLICY::
								REQUIRE_ACTIVE_PRODUCT_PARITY == ePolicy &&
							((bProductVolley &&
								(MasterDocument.VolleyProjections.end() ==
										ExpectedVolley ||
								 !Equal_JsonSemanticValue(
									ExpectedVolley->ProductAction, Action) ||
								 !ResolvedVolleyProjectionIds.insert(
									ExpectedVolley->strEventId).second)) ||
							(!bProductVolley &&
							 MasterDocument.VolleyProjections.end() != ExpectedVolley)))
						{
							strOutStatus =
								"Valtan split authoring/Product volley projection changed: " +
								(MasterDocument.VolleyProjections.end() ==
									ExpectedVolley ? strTargetId :
									ExpectedVolley->strEventId);
							Set_CanonicalReadFailure(
								pOutDiagnostic,
								VALTAN_CANONICAL_READ_FAILURE_KIND::STALE_PROJECTION,
								strOutStatus, Pattern.strPatternId,
								Stage.strStageId);
							return false;
						}

						// 두 정책 모두 Product 소환의 owner pattern/action, 양의 정수 개수, archetype 단일 사용을 검사한다.
						// 즉 복원 정책도 현재 Product의 잘못된 소환 소유 관계를 무조건 허용하지 않는다.
						if (Reference == CombatObjectEffectsByArchetype.end() ||
							Reference->second.strOwnerPatternId !=
								Pattern.strPatternId ||
							Reference->second.strOwnerStageActionId !=
								Stage.strActionId ||
							!std::isfinite(SpawnValue) || SpawnValue < 1.0 ||
							SpawnValue >
								static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
							std::floor(SpawnValue) != SpawnValue ||
							!ResolvedCombatObjectEffects.insert(strTargetId).second)
						{
							strOutStatus =
								"Valtan SPAWN_COMBAT_OBJECT authoring join changed: " +
								strTargetId;
							return false;
						}
						VALTAN_COMBAT_OBJECT_EFFECT_VIEW View;
						View.strCombatObjectArchetypeId = strTargetId;
						View.strPreparationEventId = Reference->second.strPreparationEventId;
						View.strClientVisualId =
							Reference->second.strClientVisualId;
						View.strEffectAssetId =
							Reference->second.strEffectAssetId;
						View.strEffectV2GroupId =
							Reference->second.strEffectV2GroupId;
						View.strTrigger = Read_String(Action, "trigger");
						View.iSpawnValue = static_cast<uint32_t>(SpawnValue);

						// volley에는 대상별 개수 외 전체 상한·배치 각도 degree·반경 m·예약 시각 ms를 복사한다.
						// 무작위 영역 값은 표시 데이터이며 여기서 위치를 표본화하거나 내비게이션을 조회하지 않는다.
						if ("SPAWN_COMBAT_OBJECT_VOLLEY" == strSpawnKind)
						{
							View.strVolleyPolicy = Read_String(
								Action, "targetingPolicy");
							View.strVolleyLayout = Read_String(Action, "layout");
							View.fVolleyRadiusM = static_cast<f32_t>(
								Read_Number(Action, "radiusM"));
							View.fVolleyStartAngleDegrees = static_cast<f32_t>(
								Read_Number(Action, "startAngleDegrees"));
							View.fVolleyAngleStepDegrees = static_cast<f32_t>(
								Read_Number(Action, "angleStepDegrees"));
							View.bVolleyAllowOverlap =
								Action.Find("allowOverlap")->Get_Boolean();
							View.iVolleyMaximumTotalObjects = static_cast<uint32_t>(
								Read_Number(Action, "maximumTotalObjects"));
							View.iSpawnScheduleCount = static_cast<uint32_t>(
								Read_Number(Action, "spawnCount"));
							View.iFirstSpawnOffsetMs = static_cast<uint32_t>(
								Read_Number(Action, "firstSpawnOffsetMs"));
							View.iSpawnIntervalMs = static_cast<uint32_t>(
								Read_Number(Action, "spawnIntervalMs"));
							View.iArenaRandomCount = static_cast<uint32_t>(
								Read_Number(Action, "arenaRandomCount"));
							View.fArenaRandomRadiusM = static_cast<f32_t>(
								Read_Number(Action, "arenaRandomRadiusM"));
							View.fArenaHeightToleranceM = static_cast<f32_t>(
								Read_Number(Action, "arenaHeightToleranceM"));
							View.strArenaAnchor = Read_String(
								Action, "arenaAnchorPolicy");
							View.strArenaRandomKind =
								0u == View.iArenaRandomCount ?
									"NONE" : "RANDOM_NAVIGABLE_CIRCLE";
						}
						View.strKind = Reference->second.strKind;
						View.strOriginPolicy =
							Reference->second.strOriginPolicy;
						View.strDirectionPolicy =
							Reference->second.strDirectionPolicy;
						View.fSpeedMps = Reference->second.fSpeedMps;
						View.fMaximumDistanceM =
							Reference->second.fMaximumDistanceM;
						View.iLifetimeMs = Reference->second.iLifetimeMs;
						View.fCoverRadiusM = Reference->second.fCoverRadiusM;
						View.OwnerHitChain = Reference->second.OwnerHitChain;
						View.HitIds = Reference->second.HitIds;
						View.HitOffsetsMs = Reference->second.HitOffsetsMs;
						View.Hits = Reference->second.Hits;
						View.PresentationEvents =
							Reference->second.PresentationEvents;
						Stage.CombatObjectEffects.push_back(std::move(View));
					}
				}

				// action에 대응하는 clip occurrence 목록을 복사하고 첫 clip 이름을 대표값으로 쓴다.
				// NONE 애니메이션 binding은 비어 있는 clip 목록이어야 하며 모든 stage에 clip을 강제하지는 않는다.
				const auto ClipIterator = ClipsByAction.find(Stage.strActionId);
				if (ClipIterator != ClipsByAction.end())
				{
					Stage.bSuppressAnimation =
						SuppressedAnimationActions.contains(Stage.strActionId);
					Stage.ClipOccurrences = ClipIterator->second;
					if (Stage.bSuppressAnimation &&
						!Stage.ClipOccurrences.empty())
					{
						strOutStatus =
							"Valtan NONE animation binding unexpectedly owns clips: " +
							Stage.strActionId;
						return false;
					}
					Stage.RuntimeClipNames.reserve(
						Stage.ClipOccurrences.size());
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Stage.RuntimeClipNames.push_back(
							Occurrence.strClipName);
					}
					Stage.strRuntimeClipName = Stage.RuntimeClipNames.empty() ?
						std::string{} : Stage.RuntimeClipNames.front();
				}

				// 몸체 숨김은 시작 < 끝 ≤ stage 기간인 ms 구간만 채택한다.
				// 표시 View에 정책을 복사하는 작업이며 여기서 보스 몸체의 visible 상태를 직접 바꾸지 않는다.
				const auto BodyHiddenWindow =
					BodyHiddenWindowByAction.find(Stage.strActionId);
				if (BodyHiddenWindow != BodyHiddenWindowByAction.end())
				{
					if (BodyHiddenWindow->second[0] >=
							BodyHiddenWindow->second[1] ||
						BodyHiddenWindow->second[1] > Stage.iDurationMs)
					{
						strOutStatus =
							"Valtan Product body visibility window exceeds its Stage: " +
							Stage.strActionId;
						return false;
					}
					Stage.bHasBodyHiddenWindow = true;
					Stage.iBodyHiddenFromMs = BodyHiddenWindow->second[0];
					Stage.iBodyHiddenToMs = BodyHiddenWindow->second[1];
				}

				// 각 cue는 action뿐 아니라 pattern/stage 소유자도 일치해야 한다. stage 시계 cue는
				// 시작을 stage 기간과 비교하지만 선택 끝 시각은 600000ms 상한과 비교하므로 stage 끝 안을 보장하지 않는다.
				const auto CueIterator = CueByAction.find(Stage.strActionId);
				if (CueIterator != CueByAction.end())
				{
					for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
						CueIterator->second)
					{
						if (Cue.strPatternId != Pattern.strPatternId ||
							Cue.strStageId != Stage.strStageId)
						{
							strOutStatus =
								"Valtan Product Effect cue pattern/stage tuple changed for " +
								Stage.strActionId + ".";
							return false;
						}
						if (Cue.bUsesStageClock)
						{
							if (!Cue.strClipOccurrenceId.empty() ||
								Cue.iStageOffsetMs >= Stage.iDurationMs ||
								(Cue.bHasSourceEnd && (Cue.iSourceEndMs <= Cue.iStageOffsetMs || Cue.iSourceEndMs > 600000u)))
							{
								strOutStatus =
									"Valtan stage-clock Effect cue left its Stage wall: " +
									Cue.strBindingId;
								return false;
							}
						}
						else
						{
							// clip 시계 cue는 현재 stage에 같은 clipOccurrenceId가 있어야 한다.
							// 그 clip과 stage 양쪽 목록에 같은 값을 복사하고 전체 cue 해소 개수를 증가시킨다.
							const auto Clip = std::find_if(
								Stage.ClipOccurrences.begin(),
								Stage.ClipOccurrences.end(),
								[&Cue](
									const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
								{
									return Occurrence.strClipOccurrenceId ==
										Cue.strClipOccurrenceId;
								});
							if (Clip == Stage.ClipOccurrences.end())
							{
								strOutStatus =
									"Valtan Product Effect cue clip occurrence left its stage: " +
									Cue.strClipOccurrenceId;
								return false;
							}
							Clip->ProductCues.push_back(Cue);
						}
						Stage.ProductCues.push_back(Cue);
						++iResolvedCueCount;

					}
					if (!Stage.ProductCues.empty())
						Stage.ProductCue = Stage.ProductCues.front();
				}

				// 선택 legacy binding은 이미 있는 asset이면 표시 표식만 더하고 없으면 문서 경로를 추가한다.
				// 이 경로 계산은 파일 존재나 Effect 문서 내용의 유효성을 보장하지 않는다.
				const auto EffectIterator =
					EffectByAction.find(Stage.strActionId);
				if (EffectIterator != EffectByAction.end() &&
					!EffectIterator->second.first.empty() &&
					!EffectIterator->second.second.empty())
				{
					const auto Existing = std::find_if(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&EffectIterator](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId ==
								EffectIterator->second.first;
						});
					if (Existing != Stage.Effects.end())
					{
						Existing->bPatternEffectBinding = true;
					}
					else
					{
						VALTAN_STAGE_EFFECT_VIEW Bound;
						Bound.strEffectAssetId = EffectIterator->second.first;
						Bound.DocumentPath = CProjectDataRoot::Get() /
							std::filesystem::path(
								EffectIterator->second.second).lexically_normal();
						Bound.eOrigin =
							VALTAN_STAGE_EFFECT_ORIGIN::PATTERN_EFFECT_BINDING;
						Bound.bPatternEffectBinding = true;
						Stage.Effects.push_back(std::move(Bound));
					}
				}

				// Product cue가 있는 stage는 cue의 Effect ID를 편집 진입점으로 사용한다.
				// 이름 규칙은 Product cue가 없는 stage에만 적용해 과거 seed와 게시된 항목을 함께 소유하지 않게 한다.
				if (!Stage.Has_ProductCue() &&
					Stage.CombatObjectEffects.empty())
				{
					// 이름 규칙 후보는 cue와 전투 오브젝트 표현이 모두 없는 stage에서만 찾는다.
					// exists가 성공하면 표시 목록에 추가하며 regular file 확인이나 내용 파싱은 여기서 수행하지 않는다.
					const std::string strCandidate = Build_StageEffectAssetId(
						Pattern.strActionId, Stage);
					const bool_t bCandidateAlreadyBound = std::any_of(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&strCandidate](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId == strCandidate;
						});
					if (!strCandidate.empty() && !bCandidateAlreadyBound)
					{
						const std::filesystem::path Candidate =
							CProjectDataRoot::Resolve(
								std::filesystem::path(L"Effects") / L"Authored" /
								std::filesystem::path(
									strCandidate + ".effect.json"));
						std::error_code FileError;
						if (!Candidate.empty() && std::filesystem::exists(
								Candidate, FileError))
						{
							VALTAN_STAGE_EFFECT_VIEW Seeded;
							Seeded.strEffectAssetId = strCandidate;
							Seeded.DocumentPath = Candidate;
							Seeded.eOrigin =
								VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE;
							Stage.Effects.push_back(std::move(Seeded));
						}
					}
				}
				Pattern.Stages.push_back(std::move(Stage));
			}
		}

		// stage 목록이 완성된 뒤 serverMotion이 참조한 stage와 시각 범위를 실제 기간에 맞춘다.
		// 원본 Master의 serverMotion 구조 검사만으로 끝내지 않는 Product 연결 지점이다.
		if (!Validate_PatternServerMotionStageWindows(
				Pattern.ServerMotion, Pattern.Stages))
		{
			strOutStatus =
				"Valtan encounter serverMotion window is outside its authored stage: " +
				Pattern.strPatternId;
			return false;
		}
		const bool_t bHasStageVerticalOffset = std::any_of(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[](const VALTAN_STAGE_VIEW& Stage)
			{ return 0.f != Stage.fVerticalOffsetM; });
		if ((0.f != Pattern.fVerticalOffsetM &&
			std::none_of(Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{ return Stage.BossResponse.has_value(); })) ||
			(bHasStageVerticalOffset && Pattern.ServerMotion.has_value()))
		{
			strOutStatus =
				"Valtan encounter verticalOffsetM has no active boss response: " +
				Pattern.strPatternId;
			return false;
		}
		if (Pattern.Is_Gimmick())
			Staged.Gimmicks.push_back(std::move(Pattern));
		else
			Staged.Rotation.push_back(std::move(Pattern));
	}

	// 전체 cue 해소 수와 모든 visual의 소환 해소 수를 확인한다.
	// 원본 volley 전체를 한 번씩 해소했는지의 추가 비교는 엄격 Product 일치 정책에서만 수행한다.
	if (iResolvedCueCount != iCueCount)
	{
		strOutStatus = "Valtan Product Effect cue action did not resolve into the encounter: resolved " +
			std::to_string(iResolvedCueCount) + " of " +
			std::to_string(iCueCount) + ".";
		return false;
	}
	if (VALTAN_PATTERN_TREE_LOAD_POLICY::REQUIRE_ACTIVE_PRODUCT_PARITY ==
			ePolicy &&
		ResolvedVolleyProjectionIds.size() !=
		MasterDocument.VolleyProjections.size())
	{
		const auto Missing = std::find_if(
			MasterDocument.VolleyProjections.begin(),
			MasterDocument.VolleyProjections.end(),
			[&ResolvedVolleyProjectionIds](
				const MASTER_VOLLEY_PROJECTION& Projection)
			{
				return !ResolvedVolleyProjectionIds.contains(
					Projection.strEventId);
			});
		strOutStatus =
			"Valtan split authoring/Product volley projection coverage changed";
		if (MasterDocument.VolleyProjections.end() != Missing)
			strOutStatus += ": " + Missing->strEventId;
		Set_CanonicalReadFailure(
			pOutDiagnostic,
			VALTAN_CANONICAL_READ_FAILURE_KIND::STALE_PROJECTION,
			strOutStatus,
			MasterDocument.VolleyProjections.end() == Missing ?
				std::string_view{} : Missing->strPatternId,
			MasterDocument.VolleyProjections.end() == Missing ?
				std::string_view{} : Missing->strStageId);
		return false;
	}
	if (ResolvedCombatObjectEffects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"Valtan combat-object Effect visual did not resolve into its owner stage.";
		return false;
	}

	// 지역 Product 트리에 원본 Master를 결합한다. 엄격 정책은 값 일치를 요구하고 복원 정책은
	// 허용된 원본 값을 임시 트리에 덮어쓰며 어느 쪽도 원본/게시 파일을 저장하지 않는다.
	if (!Apply_MasterDocument(
			MasterDocument, PatternRotations, Staged, Error, ePolicy,
			pOutDiagnostic))
	{
		strOutStatus = "Valtan split authoring/Product join failed: " + Error;
		if (nullptr != pOutDiagnostic)
		{
			if (VALTAN_CANONICAL_READ_FAILURE_KIND::NONE ==
				pOutDiagnostic->eFailure)
			{
				pOutDiagnostic->eFailure =
					VALTAN_CANONICAL_READ_FAILURE_KIND::PRODUCT_INVALID;
			}
			pOutDiagnostic->strStatus = strOutStatus;
		}
		return false;
	}
	if (Staged.Gimmicks.empty() && Staged.Rotation.empty())
	{
		strOutStatus = "Valtan encounter produced no patterns.";
		return false;
	}
	// 기믹을 발동 체력 내림차순으로 정렬해 표시한다. 같은 체력의 추가 정렬 키는 여기서 사용하지 않는다.
	std::sort(Staged.Gimmicks.begin(), Staged.Gimmicks.end(),
		[](const VALTAN_PATTERN_VIEW& Left, const VALTAN_PATTERN_VIEW& Right)
		{
			return Left.iTriggerHealthBar > Right.iTriggerHealthBar;
		});

	// 아래 Phase는 triggerHealthBar로 나눈 화면용 체력 구간이다.
	// 기믹이 발동하는 체력까지 한 구간으로 묶으며 Server의 gameplay phase와는 별개다.
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	// 일반 패턴은 자신의 체력 범위와 겹치는 모든 화면 구간에 연결한다.
	// 구간에는 Rotation의 index를 보관하므로 원본 vector 순서와 함께 사용해야 한다.
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	// 선택 introPatternId가 Rotation에서 발견되면 그 vector index만 보관한다.
	// 비어 있거나 찾지 못하면 기존 Staged 기본값을 유지하고 이 조건만으로 실패하지 않는다.
	const std::string strIntroPatternId = Read_String(
		Encounter, "introPatternId");
	if (!strIntroPatternId.empty())
	{
		const auto Intro = std::find_if(
			Staged.Rotation.begin(), Staged.Rotation.end(),
			[&strIntroPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strIntroPatternId;
			});
		if (Intro != Staged.Rotation.end())
		{
			Staged.iIntroRotationIndex = static_cast<size_t>(
				std::distance(Staged.Rotation.begin(), Intro));
		}
	}

	// 모든 명시적 실패 검사가 끝나면 임시 트리를 한 번 이동 대입하고 성공 요약과 선택 진단을 작성한다.
	// 이후 문자열 할당 예외까지 기존 OutView로 되돌리는 강한 예외 보장은 제공하지 않는다.
	OutView = std::move(Staged);
	strOutStatus = "Valtan tree: " +
		std::to_string(OutView.Phases.size()) + " phases, " +
		std::to_string(OutView.Get_PatternCount()) + " patterns, " +
		std::to_string(OutView.Get_StageCount()) + " stages, " +
		std::to_string(OutView.Get_ClipBoundStageCount()) + " clip-bound (" +
		std::to_string(OutView.Get_ClipOccurrenceCount()) + " occurrences), " +
		std::to_string(OutView.Get_ProductCueStageCount()) + " cue stages (" +
		std::to_string(OutView.Get_ProductCueCount()) + " Product cues), " +
		std::to_string(OutView.Get_CombatObjectEffectCount()) +
		" combat-object visuals, " +
		std::to_string(OutView.IndependentEffects.size()) +
		" independent Effects, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	if (nullptr != pOutDiagnostic)
	{
		pOutDiagnostic->Clear();
		pOutDiagnostic->strStatus = strOutStatus;
	}
	return true;
}

	// PatternTree와 EncounterPatternReference가 stage의 optional attackContacts를 읽는 경계다.
	// 필드가 없으면 contact 확장을 비우고 성공한다. 값이 있다면 임시 hits에서 모든 검증을 마친 뒤
	// contacts를 교체하므로 잘못된 행이나 Shared 계약 위반은 호출자가 보관한 기존 목록을 바꾸지 않는다.
	// 보충 설명: 필드 누락 시 목록을 비우는 경로와 값이 있을 때 임시 목록으로 검증하는 경로를 구분한다.
	// 값이 있는 입력의 검사 실패는 기존 contacts를 보존한다. 두 경로 모두 Server에 판정을 전송하지 않는다.
	bool Client::Parse_ValtanStageAttackContacts(const DATA_JSON_VALUE* value, std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>& contacts)
	{
		if (!value)
		{
			contacts.clear();
			return true;
		}
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> hits;
		if (!value->Is_Array())
			return false;
		// 명시적으로 제공한 빈 배열은 누락과 다른 입력이며 기존 저장 계약대로 거부한다.
		if (value->Get_Array().empty() || value->Get_Array().size() > 32u)
			return false;

		const auto readUnsigned = [](const DATA_JSON_VALUE& item, uint32_t maximum, uint32_t& out)
		{
			if (!item.Is_Number())
				return false;
			const double parsedValue = item.Get_Number();
			if (!std::isfinite(parsedValue) || parsedValue < 0.0 || parsedValue > maximum)
				return false;
			// JSON 숫자의 소수부를 잘라 시간이나 횟수가 다른 값으로 저장되는 일을 막는다.
			if (std::floor(parsedValue) != parsedValue)
				return false;
			out = static_cast<uint32_t>(parsedValue);
			return true;
		};
		const auto readFinite = [](const DATA_JSON_VALUE& item, double minimum, double maximum, double& out)
		{
			if (!item.Is_Number())
				return false;
			const double parsedValue = item.Get_Number();
			if (!std::isfinite(parsedValue) || parsedValue < minimum || parsedValue > maximum)
				return false;
			out = parsedValue;
			return true;
		};

		for (const auto& row : value->Get_Array())
		{
			LostArk::Shared::ATTACK_HIT_TEMPLATE hit;
			// 선택 필드는 생략 시 ATTACK_HIT_TEMPLATE의 기본값을 사용하고, 존재하는 필드만 덮어쓴다.
			if (!Has_ExactPropertiesWithOptional(row, { "hitId" },
				{ "trigger", "shape", "damageKind", "damageProfileId", "atMs", "endMs",
				  "repeatCount", "repeatIntervalMs", "damagePercent", "radiusM", "innerRadiusM",
				  "lengthM", "halfWidthM", "angleDegrees", "offsetForwardM", "offsetRightM",
				  "yawOffsetDegrees", "riseHeightM", "pushMs", "pushRangeM", "forcePush", "pushDirection" }))
				return false;
			if (const auto* item = row.Find("hitId"))
			{
				if (!item->Is_String())
					return false;
				hit.strHitId = item->Get_String();
			}
			if (const auto* item = row.Find("trigger"))
			{
				if (!item->Is_String())
					return false;
				hit.strTrigger = item->Get_String();
			}
			if (const auto* item = row.Find("shape"))
			{
				if (!item->Is_String())
					return false;
				hit.strShape = item->Get_String();
			}
			if (const auto* item = row.Find("damageKind"))
			{
				if (!item->Is_String())
					return false;
				hit.strDamageKind = item->Get_String();
			}
			if (const auto* item = row.Find("damageProfileId"))
			{
				if (!item->Is_String())
					return false;
				hit.strDamageProfileId = item->Get_String();
			}
			// 시각은 ms, 크기와 이동 거리는 m, 회전은 degree로 읽는다.
			if (const auto* item = row.Find("atMs"))
			{
				if (!readUnsigned(*item, 600000u, hit.iAtMs))
					return false;
			}
			if (const auto* item = row.Find("endMs"))
			{
				if (!readUnsigned(*item, 600000u, hit.iEndMs))
					return false;
			}
			if (const auto* item = row.Find("repeatCount"))
			{
				if (!readUnsigned(*item, 600000u, hit.iRepeatCount))
					return false;
			}
			if (const auto* item = row.Find("repeatIntervalMs"))
			{
				if (!readUnsigned(*item, 600000u, hit.iRepeatIntervalMs))
					return false;
			}
			if (const auto* item = row.Find("damagePercent"))
			{
				if (!readUnsigned(*item, 600000u, hit.iDamagePercent))
					return false;
			}
			if (const auto* item = row.Find("radiusM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fRadiusM))
					return false;
			}
			if (const auto* item = row.Find("innerRadiusM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fInnerRadiusM))
					return false;
			}
			if (const auto* item = row.Find("lengthM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fLengthM))
					return false;
			}
			if (const auto* item = row.Find("halfWidthM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fHalfWidthM))
					return false;
			}
			if (const auto* item = row.Find("angleDegrees"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fAngleDegrees))
					return false;
			}
			if (const auto* item = row.Find("offsetForwardM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fOffsetForwardM))
					return false;
			}
			if (const auto* item = row.Find("offsetRightM"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fOffsetRightM))
					return false;
			}
			if (const auto* item = row.Find("yawOffsetDegrees"))
			{
				if (!readFinite(*item, -1000., 1000., hit.fYawOffsetDegrees))
					return false;
			}
			// forcePush의 생략과 false를 구분해 기존 피격 반응 정책을 보존한다.
			if (const auto* item = row.Find("forcePush"))
			{
				if (!item->Is_Boolean())
					return false;
				hit.ForcePush = item->Get_Boolean();
			}
			if (const auto* item = row.Find("pushDirection"))
			{
				if (!item->Is_String())
					return false;
				hit.strPushDirection = item->Get_String();
			}
			if (const auto* item = row.Find("pushRangeM"))
			{
				if (!readFinite(*item, 0., 100., hit.fPushRangeM))
					return false;
			}
			if (const auto* item = row.Find("riseHeightM"))
			{
				if (!readFinite(*item, 0., 100., hit.fRiseHeightM))
					return false;
			}
			if (const auto* item = row.Find("pushMs"))
			{
				if (!readUnsigned(*item, 5000u, hit.iPushMs))
					return false;
			}
			hits.push_back(std::move(hit));
		}
		// 도형·반복·피해 정책의 의미 검증까지 성공한 목록만 호출자의 stage에 반영한다.
		if (!LostArk::Shared::Validate_AttackHitTemplates(hits))
			return false;
		contacts = std::move(hits);
		return true;
	}

	// 편집 문서 저장을 위해 attackContacts를 기존 JSON 필드와 기본값 생략 규칙으로 직렬화한다.
	// 이 함수는 값을 검증하거나 Server로 전송하지 않으므로 호출자는 저장 경계의 검증을 거쳐야 한다.
	std::string Client::Serialize_ValtanStageAttackContacts(const std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>& hits)
	{
		std::ostringstream output;
		output.imbue(std::locale::classic());
		output << std::setprecision(std::numeric_limits<double>::max_digits10);
		output << "[";
		for (std::size_t index = 0; index < hits.size(); ++index)
		{
			const auto& hit = hits[index];
			output << (index ? ", {" : "{");
			output << "\"hitId\": \"" << CDataJson::Escape(hit.strHitId) << "\"";
			output << ", \"trigger\": \"" << CDataJson::Escape(hit.strTrigger) << "\"";
			output << ", \"shape\": \"" << CDataJson::Escape(hit.strShape) << "\"";
			output << ", \"damageKind\": \"" << CDataJson::Escape(hit.strDamageKind) << "\"";
			output << ", \"damageProfileId\": \"" << CDataJson::Escape(hit.strDamageProfileId) << "\"";
			output << ", \"atMs\": " << hit.iAtMs;
			output << ", \"endMs\": " << hit.iEndMs;
			output << ", \"repeatCount\": " << hit.iRepeatCount;
			output << ", \"repeatIntervalMs\": " << hit.iRepeatIntervalMs;
			output << ", \"damagePercent\": " << hit.iDamagePercent;
			output << ", \"radiusM\": " << hit.fRadiusM;
			output << ", \"innerRadiusM\": " << hit.fInnerRadiusM;
			output << ", \"lengthM\": " << hit.fLengthM;
			output << ", \"halfWidthM\": " << hit.fHalfWidthM;
			output << ", \"angleDegrees\": " << hit.fAngleDegrees;
			output << ", \"offsetForwardM\": " << hit.fOffsetForwardM;
			output << ", \"offsetRightM\": " << hit.fOffsetRightM;
			output << ", \"yawOffsetDegrees\": " << hit.fYawOffsetDegrees;
			if (hit.ForcePush.has_value())
				output << ", \"forcePush\": " << (*hit.ForcePush ? "true" : "false");
			if (hit.strPushDirection != "AWAY_FROM_CONTACT")
				output << ", \"pushDirection\": \"" << hit.strPushDirection << "\"";
			if (hit.fPushRangeM > 0.0)
				output << ", \"pushRangeM\": " << hit.fPushRangeM;
			if (hit.fRiseHeightM > 0.0 || hit.iPushMs != 0u)
				output << ", \"riseHeightM\": " << hit.fRiseHeightM << ", \"pushMs\": " << hit.iPushMs;
			output << "}";
		}
		output << "]";
		return output.str();
	}
