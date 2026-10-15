// Animation Tool의 원본 참조 탭이 사용하는 읽기 전용 문서 변환 계층이다.
// 패턴 번호별 source 참조를 해석할 뿐 제품 패턴 master, Server 판정, Effect 게시 데이터는 쓰지 않는다.
#include "ValtanPatternPreviewDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <unordered_set>
#include <utility>

namespace
{
	using namespace Client;

	// JSON v2와 .clipseq v2의 입력 계약이다. 67개 참조 행, sequence/반복/clip 수와 펼친 clip 목록을 검사한다.
	// 파일 전체 byte 수를 제한하거나 실행 중 Tool 모델을 잠그는 계약은 포함하지 않는다.
	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-pattern-preview";
	constexpr double DOCUMENT_VERSION = 2.0;
	constexpr std::string_view CLIP_SEQUENCE_MAGIC = "LOSTARK_CLIP_SEQ";
	constexpr uint32_t CLIP_SEQUENCE_VERSION = 2u;
	constexpr uint32_t PATTERN_COUNT = 67u;
	constexpr uint32_t MAX_SOURCE_SEQUENCES = 4096u;
	constexpr size_t MAX_SEQUENCES_PER_PATTERN = 16u;
	constexpr uint32_t MAX_SEQUENCE_REPEAT = 8u;
	constexpr size_t MAX_CLIPS_PER_SOURCE_SEQUENCE = 64u;
	constexpr size_t MAX_PLAYLIST_ITEMS = 2048u;

	// .clipseq 한 행의 값을 소유한다. 디스크 reader 안에서 만들고 검증된 map으로 옮긴다.
	struct SOURCE_SEQUENCE final
	{
		uint32_t iSourceActionId = 0u;
		int32_t iSequenceIndex = -1;
		std::string strName;
		std::string strMode;
		std::vector<std::string> Clips;
	};

	// 같은 action 안에도 여러 sequence가 있으므로 두 값을 함께 조회/중복 검사 키로 사용한다.
	// std::map의 정렬 순서가 재생 순서는 아니며 실제 재생 순서는 JSON 참조와 각 Clips vector가 결정한다.
	using SOURCE_SEQUENCE_KEY = std::pair<uint32_t, int32_t>;
	using SOURCE_SEQUENCE_INDEX =
		std::map<SOURCE_SEQUENCE_KEY, SOURCE_SEQUENCE>;

	// 필수 키와 JSON 타입을 함께 확인한다. 반환 포인터는 호출자가 소유한 JSON 노드의 수명 동안만 차용한다.
	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	// 키 개수와 각 이름을 모두 맞춰 빠진 필드와 지원하지 않는 추가 필드를 거부한다.
	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> Names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != Names.size())
			return false;
		for (const std::string_view Name : Names)
		{
			if (nullptr == object.Find(Name))
				return false;
		}
		return true;
	}

	// asset/clip ID를 최대 255-byte의 영문자, 숫자, 밑줄, 하이픈, 점으로 제한한다. 이름의 실제 존재는 검사하지 않는다.
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

	// 표시 문자열의 byte 길이와 제어 문자를 검사한다. 탭은 허용하며 UTF-8 코드 포인트의 유효성 검사는 하지 않는다.
	bool_t Is_DisplayText(
		const std::string_view Value,
		const size_t iMaximumBytes)
	{
		if (Value.empty() || Value.size() > iMaximumBytes)
			return false;
		return std::none_of(
			Value.begin(), Value.end(),
			[](const unsigned char Character)
			{
				return Character < 0x20u && Character != '\t';
			});
	}

	// .clipseq에서 허용하는 세 mode 이름이다. 이 문서가 mode별 전투 실행 규칙을 만드는 것은 아니다.
	bool_t Is_SourceSequenceMode(const std::string_view Value)
	{
		return "SEQUENCE" == Value || "HOLD" == Value || "COMBO" == Value;
	}

	// JSON 숫자가 유한한 비음수 정수이며 uint32 범위인지 확인한 뒤 출력에 대입한다. false이면 출력 숫자는 유지한다.
	bool_t Parse_U32(
		const DATA_JSON_VALUE& Value,
		uint32_t& outValue)
	{
		if (!Value.Is_Number())
			return false;
		const double Number = Value.Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(Number);
		return true;
	}

	// 저장 타입은 int32지만 sequence index 입력은 비음수만 허용한다. -1은 미설정 기본값으로만 쓰고 JSON에서 받지 않는다.
	bool_t Parse_I32(
		const DATA_JSON_VALUE& Value,
		int32_t& outValue)
	{
		if (!Value.Is_Number())
			return false;
		const double Number = Value.Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>((std::numeric_limits<int32_t>::max)()) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		outValue = static_cast<int32_t>(Number);
		return true;
	}

	// .clipseq의 10진 숫자 문자열을 문자별로 읽고 uint32 상한을 확인한다. 성공한 경우에만 outValue를 바꾼다.
	bool_t Parse_U32Token(
		const std::string_view Value,
		uint32_t& outValue)
	{
		if (Value.empty())
			return false;
		uint64_t Result = 0u;
		for (const unsigned char Character : Value)
		{
			if (Character < '0' || Character > '9')
				return false;
			Result = Result * 10u + static_cast<uint64_t>(Character - '0');
			if (Result > (std::numeric_limits<uint32_t>::max)())
				return false;
		}
		outValue = static_cast<uint32_t>(Result);
		return true;
	}

	// 문자열 sequence index도 비음수로 읽고 int32 상한을 추가 적용한다. 실패 시 기존 출력은 유지한다.
	bool_t Parse_I32Token(
		const std::string_view Value,
		int32_t& outValue)
	{
		uint32_t Parsed = 0u;
		if (!Parse_U32Token(Value, Parsed) ||
			Parsed > static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()))
		{
			return false;
		}
		outValue = static_cast<int32_t>(Parsed);
		return true;
	}

	// JSON confidence 이름을 내부 enum으로 바꾼다. 알 수 없는 이름은 END로 남겨 이후 행 검증에서 거부한다.
	VALTAN_PATTERN_PREVIEW_EVIDENCE Parse_Evidence(
		const std::string_view Value)
	{
		if ("SOURCE_FAMILY_DIRECT" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::SOURCE_FAMILY_DIRECT;
		if ("USER_CONFIRMED_FAMILY" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::USER_CONFIRMED_FAMILY;
		if ("CANDIDATE" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::CANDIDATE;
		if ("COMPOSITE" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::COMPOSITE;
		if ("UNRESOLVED" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED;
		if ("NO_ANIMATION" == Value)
			return VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION;
		return VALTAN_PATTERN_PREVIEW_EVIDENCE::END;
	}

	// getline으로 분리한 한 행 안에서 공백/탭/CR만 건너뛴다. 커서는 호출자의 값을 직접 전진시킨다.
	void Skip_Space(
		const std::string_view Text,
		size_t& iCursor)
	{
		while (iCursor < Text.size() &&
			(Text[iCursor] == ' ' || Text[iCursor] == '\t' ||
				Text[iCursor] == '\r'))
		{
			++iCursor;
		}
	}

	// 공백으로 나뉜 토큰을 소유 문자열로 복사한다. 실패 시에도 앞의 공백을 건너뛴 커서는 되돌리지 않는다.
	bool_t Read_Token(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outToken)
	{
		Skip_Space(Text, iCursor);
		const size_t iStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != ' ' &&
			Text[iCursor] != '\t' && Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iStart == iCursor)
			return false;
		outToken.assign(Text.substr(iStart, iCursor - iStart));
		return true;
	}

	// 큰따옴표 안을 복사하되 역슬래시 escape와 CR은 지원하지 않는다. 빈 문자열 허용 여부는 상위 필드 검증이 정한다.
	// 실패한 행은 상위 reader가 버리므로 진행 중인 커서를 rollback하지 않는다.
	bool_t Read_Quoted(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outValue)
	{
		Skip_Space(Text, iCursor);
		if (iCursor >= Text.size() || '"' != Text[iCursor])
			return false;
		++iCursor;
		const size_t iStart = iCursor;
		while (iCursor < Text.size() && '"' != Text[iCursor])
		{
			if ('\\' == Text[iCursor] || '\r' == Text[iCursor])
				return false;
			++iCursor;
		}
		if (iCursor >= Text.size())
			return false;
		outValue.assign(Text.substr(iStart, iCursor - iStart));
		++iCursor;
		return true;
	}

	// seq/mode/clips의 key=value 하나를 읽는다. key와 커서는 value 실패 전에 바뀔 수 있으므로 상위에서 행 전체를 폐기한다.
	bool_t Read_Pair(
		const std::string_view Text,
		size_t& iCursor,
		std::string& outKey,
		std::string& outValue)
	{
		Skip_Space(Text, iCursor);
		const size_t iKeyStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != '=' &&
			Text[iCursor] != ' ' && Text[iCursor] != '\t' &&
			Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iKeyStart == iCursor || iCursor >= Text.size() ||
			'=' != Text[iCursor])
		{
			return false;
		}
		outKey.assign(Text.substr(iKeyStart, iCursor - iKeyStart));
		++iCursor;
		if (iCursor < Text.size() && '"' == Text[iCursor])
			return Read_Quoted(Text, iCursor, outValue);

		const size_t iValueStart = iCursor;
		while (iCursor < Text.size() && Text[iCursor] != ' ' &&
			Text[iCursor] != '\t' && Text[iCursor] != '\r')
		{
			++iCursor;
		}
		if (iValueStart == iCursor)
			return false;
		outValue.assign(Text.substr(iValueStart, iCursor - iValueStart));
		return true;
	}

	// 남은 공백을 제외한 문자가 없는지 확인한다. 커서를 값으로 받아 원래 커서 위치는 바꾸지 않는다.
	bool_t At_End(
		const std::string_view Text,
		size_t iCursor)
	{
		Skip_Space(Text, iCursor);
		return iCursor == Text.size();
	}

	// 쉼표로 나눈 clip ID를 원래 순서대로 보관한다. 빈 토큰/허용 문자/최대 64개를 검사하고 성공한 vector만 출력에 옮긴다.
	bool_t Parse_ClipList(
		const std::string_view Value,
		std::vector<std::string>& outClips)
	{
		std::vector<std::string> Staged;
		size_t iStart = 0u;
		while (iStart <= Value.size())
		{
			const size_t iComma = Value.find(',', iStart);
			const std::string_view Clip = Value.substr(
				iStart,
				std::string_view::npos == iComma ?
					std::string_view::npos : iComma - iStart);
			if (!Is_StableToken(Clip))
				return false;
			Staged.emplace_back(Clip);
			if (Staged.size() > MAX_CLIPS_PER_SOURCE_SEQUENCE)
				return false;
			if (std::string_view::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		if (Staged.empty())
			return false;
		outClips = std::move(Staged);
		return true;
	}

	// Resolve_SourceSequences가 사용하는 .clipseq reader다. header의 소유 asset/version/선언 개수와 모든 행의 복합 키를 검증한다.
	// 임시 map 전체가 통과한 뒤 outIndex를 교체하므로 false 반환 시 이전 map은 유지한다. 파일을 수정하거나 캐시하지 않는다.
	bool_t Load_SourceSequenceIndex(
		const std::string_view animationAssetId,
		SOURCE_SEQUENCE_INDEX& outIndex,
		std::string& outStatus)
	{
		const std::filesystem::path Path =
			CValtanPatternPreviewDocument::Resolve_ClipSequencePath(
				animationAssetId);
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			outStatus = "Valtan source clip sequence is missing: " + Path.string();
			return false;
		}

		// 첫 행은 magic, version, 큰따옴표 owner, 선언 행 수의 고정 순서다. 뒤에 남는 토큰도 거부한다.
		std::string Header;
		if (!std::getline(Input, Header))
		{
			outStatus = "Valtan source clip sequence header is missing.";
			return false;
		}
		if (!Header.empty() && '\r' == Header.back())
			Header.pop_back();

		size_t iCursor = 0u;
		std::string Magic;
		std::string VersionToken;
		std::string Owner;
		std::string CountToken;
		uint32_t iVersion = 0u;
		uint32_t iDeclaredCount = 0u;
		if (!Read_Token(Header, iCursor, Magic) ||
			!Read_Token(Header, iCursor, VersionToken) ||
			!Read_Quoted(Header, iCursor, Owner) ||
			!Read_Token(Header, iCursor, CountToken) ||
			!At_End(Header, iCursor) || Magic != CLIP_SEQUENCE_MAGIC ||
			!Parse_U32Token(VersionToken, iVersion) ||
			CLIP_SEQUENCE_VERSION != iVersion || Owner != animationAssetId ||
			!Parse_U32Token(CountToken, iDeclaredCount) ||
			0u == iDeclaredCount || iDeclaredCount > MAX_SOURCE_SEQUENCES)
		{
			outStatus = "Valtan source clip sequence header is invalid.";
			return false;
		}

		// 선언 개수 상한은 header에서 확인한다. 실제 읽은 행 수와 선언 개수의 일치는 반복을 마친 뒤 비교한다.
		SOURCE_SEQUENCE_INDEX Staged;
		std::string Line;
		uint32_t iPhysicalRows = 0u;
		while (std::getline(Input, Line))
		{
			if (!Line.empty() && '\r' == Line.back())
				Line.pop_back();
			if (Line.empty())
			{
				outStatus = "Valtan source clip sequence contains a blank row.";
				return false;
			}

			iCursor = 0u;
			std::string ActionToken;
			SOURCE_SEQUENCE Sequence;
			if (!Read_Token(Line, iCursor, ActionToken) ||
				!Parse_U32Token(ActionToken, Sequence.iSourceActionId) ||
				0u == Sequence.iSourceActionId ||
				!Read_Quoted(Line, iCursor, Sequence.strName) ||
				!Is_DisplayText(Sequence.strName, 512u))
			{
				outStatus = "Valtan source clip sequence row identity is invalid.";
				return false;
			}

			// 같은 필드가 두 번 나오거나 지원하지 않는 키가 나오면 즉시 실패한다. 필드 순서는 고정하지 않는다.
			bool_t bSawSequence = false;
			bool_t bSawMode = false;
			bool_t bSawClips = false;
			while (!At_End(Line, iCursor))
			{
				std::string Key;
				std::string Value;
				if (!Read_Pair(Line, iCursor, Key, Value))
				{
					outStatus = "Valtan source clip sequence row field is malformed.";
					return false;
				}
				if ("seq" == Key && !bSawSequence)
				{
					bSawSequence = Parse_I32Token(
						Value, Sequence.iSequenceIndex);
					if (!bSawSequence)
					{
						outStatus = "Valtan source sequence index is invalid.";
						return false;
					}
				}
				else if ("mode" == Key && !bSawMode)
				{
					bSawMode = Is_SourceSequenceMode(Value);
					if (!bSawMode)
					{
						outStatus = "Valtan source sequence mode is invalid.";
						return false;
					}
					Sequence.strMode = std::move(Value);
				}
				else if ("clips" == Key && !bSawClips)
				{
					bSawClips = Parse_ClipList(Value, Sequence.Clips);
					if (!bSawClips)
					{
						outStatus = "Valtan source sequence clip list is invalid.";
						return false;
					}
				}
				else
				{
					outStatus =
						"Valtan source clip sequence row has a duplicate or unknown field.";
					return false;
				}
			}

			// 필수 필드 누락과 동일 (action, sequence) 중복을 여기서 함께 거부한다. 상태 문구는 두 경우 모두 같은 문자열을 사용한다.
			if (!bSawSequence || !bSawMode || !bSawClips ||
				!Staged.emplace(
					SOURCE_SEQUENCE_KEY{
						Sequence.iSourceActionId, Sequence.iSequenceIndex },
					std::move(Sequence)).second)
			{
				outStatus =
					"Valtan source clip sequence has a duplicate action/sequence key.";
				return false;
			}
			++iPhysicalRows;
		}

		// 행 수와 고유 키 수를 선언값에 맞춘다. 반복 종료 뒤 stream의 bad/fail 상태를 별도로 검사하지는 않는다.
		if (iPhysicalRows != iDeclaredCount || Staged.size() != iDeclaredCount)
		{
			outStatus = "Valtan source clip sequence declared count does not match.";
			return false;
		}

		outIndex = std::move(Staged);
		outStatus = "Loaded exact Valtan source action/sequence index.";
		return true;
	}

	// Parse_Text, 연결, 목록 작성이 공유하는 메모리 행 규칙이다. 정확히 67행이며 배열 위치+1이 패턴 번호여야 한다.
	// clip 연결 완료 여부와 대상 모델의 존재는 여기서 확인하지 않는다.
	bool_t Validate_RowPolicy(
		const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
		std::string& outStatus)
	{
		if (!Is_StableToken(document.strAnimationAssetId) ||
			document.Patterns.size() != PATTERN_COUNT)
		{
			outStatus = "Valtan pattern preview owner or pattern count is invalid.";
			return false;
		}

		for (size_t iPattern = 0u;
			iPattern < document.Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
				document.Patterns[iPattern];
			// 미해결/동작 없음인 두 근거만 빈 sequence를 허용한다. 그 외 근거는 하나 이상의 참조가 필요하다.
			const bool_t bMarkerEvidence =
				VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED ==
					Pattern.eEvidence ||
				VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION ==
					Pattern.eEvidence;
			if (Pattern.iNumber != static_cast<uint32_t>(iPattern + 1u) ||
				!Is_DisplayText(Pattern.strLabel, 256u) ||
				!Is_DisplayText(Pattern.strNote, 512u) ||
				Pattern.eEvidence >= VALTAN_PATTERN_PREVIEW_EVIDENCE::END ||
				Pattern.Sequences.size() > MAX_SEQUENCES_PER_PATTERN ||
				(bMarkerEvidence != Pattern.Sequences.empty()))
			{
				outStatus =
					"Valtan pattern preview row order, evidence, or sequence policy is invalid.";
				return false;
			}
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
				Pattern.Sequences)
			{
				if (0u == Reference.iSourceActionId ||
					Reference.iSequenceIndex < 0 || 0u == Reference.iRepeat ||
					Reference.iRepeat > MAX_SEQUENCE_REPEAT)
				{
					outStatus =
						"Valtan pattern preview source sequence reference is invalid.";
					return false;
				}
			}
		}
		return true;
	}
}

// 원본 참조 JSON의 Data 루트 경로를 반환한다. 디렉터리나 파일을 만드는 동작은 없다.
std::filesystem::path Client::CValtanPatternPreviewDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		L"Animation/Authored/Valtan/Valtan.patternpreview.json");
}

// asset token을 검증한 다음 정해진 Reference 하위 경로를 구성한다. 지원하지 않는 token은 빈 경로로 반환한다.
std::filesystem::path
Client::CValtanPatternPreviewDocument::Resolve_ClipSequencePath(
	const std::string_view animationAssetId)
{
	if (!Is_StableToken(animationAssetId))
		return {};
	const std::string Asset{ animationAssetId };
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Reference") /
		std::filesystem::path(Asset) /
		std::filesystem::path(Asset + ".clipseq"));
}

// 저작 JSON의 root/행/참조 필드 집합과 타입을 확인하고 임시 문서에 복사한다.
// 행 규칙까지 통과한 마지막 지점에서 outDocument를 교체한다. 반환 false와 outStatus 변경 외에 이전 출력 문서는 건드리지 않는다.
bool_t Client::CValtanPatternPreviewDocument::Parse_Text(
	const std::string_view text,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE Root;
	std::string ParseError;
	if (!CDataJson::Parse(text, Root, ParseError))
	{
		outStatus = "Valtan pattern preview JSON is malformed: " + ParseError;
		return false;
	}
	if (!Has_ExactProperties(
			Root,
			{ "schema", "formatVersion", "animationAssetId", "patterns" }))
	{
		outStatus =
			"Valtan pattern preview root has an unexpected field set.";
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Required(
		Root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
		Root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pAsset = Required(
		Root, "animationAssetId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pPatterns = Required(
		Root, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pSchema || pSchema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == pVersion || pVersion->Get_Number() != DOCUMENT_VERSION ||
		nullptr == pAsset || !Is_StableToken(pAsset->Get_String()) ||
		nullptr == pPatterns ||
		pPatterns->Get_Array().size() != PATTERN_COUNT)
	{
		outStatus = "Valtan pattern preview header is invalid.";
		return false;
	}

	// JSON 노드 포인터는 이 함수의 Root에만 의존한다. 외부에 반환할 문자열/vector는 임시 문서에 값으로 복사한다.
	VALTAN_PATTERN_PREVIEW_DOCUMENT Staged;
	Staged.strAnimationAssetId = pAsset->Get_String();
	Staged.Patterns.reserve(pPatterns->Get_Array().size());
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!Has_ExactProperties(
				PatternValue,
				{ "number", "label", "confidence", "note", "sequences" }))
		{
			outStatus =
				"Valtan pattern preview row has an unexpected field set.";
			return false;
		}

		const DATA_JSON_VALUE* pNumber = Required(
			PatternValue, "number", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* pLabel = Required(
			PatternValue, "label", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pEvidence = Required(
			PatternValue, "confidence", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pNote = Required(
			PatternValue, "note", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSequences = Required(
			PatternValue, "sequences", DATA_JSON_TYPE::ARRAY);

		VALTAN_PATTERN_PREVIEW_ENTRY Pattern;
		if (nullptr == pNumber || !Parse_U32(*pNumber, Pattern.iNumber) ||
			nullptr == pLabel ||
			!Is_DisplayText(pLabel->Get_String(), 256u) ||
			nullptr == pEvidence ||
			nullptr == pNote ||
			!Is_DisplayText(pNote->Get_String(), 512u) ||
			nullptr == pSequences ||
			pSequences->Get_Array().size() > MAX_SEQUENCES_PER_PATTERN)
		{
			outStatus = "Valtan pattern preview row is invalid.";
			return false;
		}
		Pattern.strLabel = pLabel->Get_String();
		Pattern.eEvidence = Parse_Evidence(pEvidence->Get_String());
		Pattern.strNote = pNote->Get_String();
		Pattern.Sequences.reserve(pSequences->Get_Array().size());

		// 저작 참조에는 세 식별/반복 필드만 허용한다. resolved 이름/mode/clip 배열을 JSON에서 직접 받을 수는 없다.
		for (const DATA_JSON_VALUE& SequenceValue :
			pSequences->Get_Array())
		{
			if (!Has_ExactProperties(
					SequenceValue,
					{ "sourceActionId", "sequenceIndex", "repeat" }))
			{
				outStatus =
					"Valtan pattern preview source reference has an unexpected field set.";
				return false;
			}

			const DATA_JSON_VALUE* pSourceActionId = Required(
				SequenceValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* pSequenceIndex = Required(
				SequenceValue, "sequenceIndex", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* pRepeat = Required(
				SequenceValue, "repeat", DATA_JSON_TYPE::NUMBER);
			VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF Reference;
			if (nullptr == pSourceActionId ||
				!Parse_U32(*pSourceActionId, Reference.iSourceActionId) ||
				nullptr == pSequenceIndex ||
				!Parse_I32(*pSequenceIndex, Reference.iSequenceIndex) ||
				nullptr == pRepeat ||
				!Parse_U32(*pRepeat, Reference.iRepeat))
			{
				outStatus =
					"Valtan pattern preview source reference is invalid.";
				return false;
			}
			Pattern.Sequences.push_back(std::move(Reference));
		}
		Staged.Patterns.push_back(std::move(Pattern));
	}

	if (!Validate_RowPolicy(Staged, outStatus))
		return false;
	outDocument = std::move(Staged);
	outStatus = "Parsed 67 Valtan source-sequence preview rows.";
	return true;
}

// Load/Validate가 저작 참조를 실제 source sequence와 모델 clip 목록에 연결한다.
// 입력 문서를 복사해 이전 연결 결과를 버리고 재구성하므로 stale 문자열을 성공 결과로 재사용하지 않는다.
bool_t Client::CValtanPatternPreviewDocument::Resolve_SourceSequences(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outResolvedDocument,
	std::string& outStatus)
{
	if (!Is_StableToken(expectedAnimationAssetId) ||
		document.strAnimationAssetId != expectedAnimationAssetId)
	{
		outStatus =
			"Valtan pattern preview owner does not match the target model.";
		return false;
	}
	if (availableClips.empty())
	{
		outStatus = "Valtan target model exposes no animation clips.";
		return false;
	}
	if (!Validate_RowPolicy(document, outStatus))
		return false;

	// 모델이 제공한 이름을 set으로 만들어 존재 확인에 쓰고 중복 입력도 거부한다. 모델 객체 자체를 보관하거나 잠그지는 않는다.
	std::unordered_set<std::string> AvailableClipSet;
	AvailableClipSet.reserve(availableClips.size());
	for (const std::string& Clip : availableClips)
	{
		if (!Is_StableToken(Clip) || !AvailableClipSet.insert(Clip).second)
		{
			outStatus =
				"Valtan target model clip catalog is invalid or contains duplicates.";
			return false;
		}
	}

	SOURCE_SEQUENCE_INDEX SourceIndex;
	if (!Load_SourceSequenceIndex(
			expectedAnimationAssetId, SourceIndex, outStatus))
	{
		return false;
	}

	// 결과 문서가 모든 문자열/vector를 소유한다. SourceIndex와 AvailableClipSet이 함수 끝에서 소멸해도 결과는 유효하다.
	VALTAN_PATTERN_PREVIEW_DOCUMENT Staged = document;
	size_t iResolvedSequenceCount = 0u;
	size_t iExpandedItemCount = 0u;
	for (VALTAN_PATTERN_PREVIEW_ENTRY& Pattern : Staged.Patterns)
	{
		if (Pattern.Sequences.empty())
		{
			++iExpandedItemCount;
			continue;
		}
		for (VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
			Pattern.Sequences)
		{
			// 입력 문서가 이전 연결 결과를 갖고 있어도 이번 source 파일 기준으로 다시 채운다.
			Reference.strSequenceName.clear();
			Reference.strSequenceMode.clear();
			Reference.ResolvedClips.clear();

			const auto Found = SourceIndex.find(
				SOURCE_SEQUENCE_KEY{
					Reference.iSourceActionId, Reference.iSequenceIndex });
			if (SourceIndex.end() == Found)
			{
				outStatus =
					"Valtan pattern " + std::to_string(Pattern.iNumber) +
					" references missing source action " +
					std::to_string(Reference.iSourceActionId) + " sequence " +
					std::to_string(Reference.iSequenceIndex) + ".";
				return false;
			}

			// 원본 한 행의 모든 clip이 대상 모델 목록에 있어야 해당 참조를 채운다. 하나라도 없으면 임시 문서 전체를 버린다.
			const SOURCE_SEQUENCE& Source = Found->second;
			for (const std::string& Clip : Source.Clips)
			{
				if (!AvailableClipSet.contains(Clip))
				{
					outStatus =
						"Valtan pattern " + std::to_string(Pattern.iNumber) +
						" source action " +
						std::to_string(Reference.iSourceActionId) +
						" sequence " +
						std::to_string(Reference.iSequenceIndex) +
						" references clip missing from the target model: " +
						Clip + ".";
					return false;
				}
			}

			Reference.strSequenceName = Source.strName;
			Reference.strSequenceMode = Source.strMode;
			Reference.ResolvedClips = Source.Clips;
			++iResolvedSequenceCount;
			// clip 수에 반복 횟수를 곱해 늘어난 항목 수를 검사한다. 빈 sequence의 marker 증가에는 별도 상한 검사가 없다.
			iExpandedItemCount +=
				Source.Clips.size() * static_cast<size_t>(Reference.iRepeat);
			if (iExpandedItemCount > MAX_PLAYLIST_ITEMS)
			{
				outStatus =
					"Valtan pattern preview expands beyond the bounded playlist budget.";
				return false;
			}
		}
	}

	outResolvedDocument = std::move(Staged);
	outStatus = "Resolved " + std::to_string(iResolvedSequenceCount) +
		" Valtan source action/sequence reference(s) against the target model.";
	return true;
}

// 동일한 source 파일/모델 clip 대조를 수행하되 완성한 임시 문서는 버린다. 디스크 읽기가 없는 순수 필드 검사는 아니다.
bool_t Client::CValtanPatternPreviewDocument::Validate(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	VALTAN_PATTERN_PREVIEW_DOCUMENT Resolved;
	return Resolve_SourceSequences(
		document,
		expectedAnimationAssetId,
		availableClips,
		Resolved,
		outStatus);
}

// Render_ValtanPatternPreview가 대상 모델의 clip 이름 목록을 전달하는 로드 진입점이다.
// 열기 실패나 parse/연결의 false는 이전 outDocument를 유지한다. 읽은 뒤 stream 오류 상태를 별도로 검사하지는 않는다.
bool_t Client::CValtanPatternPreviewDocument::Load(
	const std::string_view expectedAnimationAssetId,
	const std::vector<std::string>& availableClips,
	VALTAN_PATTERN_PREVIEW_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path Path = Resolve_Path();
	std::ifstream Input(Path, std::ios::binary);
	if (Path.empty() || !Input)
	{
		outStatus = "Valtan pattern preview document is missing: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	// JSON만 읽은 값과 source/model까지 연결한 값을 구분한다. 두 단계가 모두 성공해야 caller가 사용할 문서를 넘긴다.
	VALTAN_PATTERN_PREVIEW_DOCUMENT Authored;
	VALTAN_PATTERN_PREVIEW_DOCUMENT Resolved;
	if (!Parse_Text(Text, Authored, outStatus) ||
		!Resolve_SourceSequences(
			Authored,
			expectedAnimationAssetId,
			availableClips,
			Resolved,
			outStatus))
	{
		return false;
	}
	outDocument = std::move(Resolved);
	return true;
}

// Start_ValtanPatternPreview가 선택한 양끝 포함 번호 범위를 clip 단위 목록으로 만든다.
// 이미 연결된 문서를 전제로 세 결과 필드의 비어 있음만 확인하며 .clipseq나 실제 모델을 다시 열지는 않는다.
// 문서/목록은 값으로 분리하고 false 반환 시 이전 outPlaylist를 보존한다. 실제 재생 전 모델과 시간 검사는 Tool이 수행한다.
bool_t Client::CValtanPatternPreviewDocument::Build_Playlist(
	const VALTAN_PATTERN_PREVIEW_DOCUMENT& resolvedDocument,
	const uint32_t iFirstPattern,
	const uint32_t iLastPattern,
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM>& outPlaylist,
	std::string& outStatus)
{
	if (!Validate_RowPolicy(resolvedDocument, outStatus))
		return false;
	if (0u == iFirstPattern || iFirstPattern > iLastPattern ||
		iLastPattern > resolvedDocument.Patterns.size())
	{
		outStatus = "Valtan pattern preview playlist range is invalid.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> Staged;
	for (uint32_t iPattern = iFirstPattern;
		iPattern <= iLastPattern; ++iPattern)
	{
		const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
			resolvedDocument.Patterns[static_cast<size_t>(iPattern - 1u)];
		// 현재 패턴이 시작하는 목록 위치를 기억해 모든 반복을 펼친 뒤 이 패턴의 step 번호/개수를 채운다.
		const size_t iPatternStart = Staged.size();
		if (Pattern.Sequences.empty())
		{
			// 미해결/동작 없음 행을 목록에서 지우지 않고 marker 하나로 남긴다. idle clip과 0.45초 길이는 Tool 재생기가 정한다.
			VALTAN_PATTERN_PREVIEW_PLAY_ITEM Marker;
			Marker.iPatternNumber = Pattern.iNumber;
			Marker.strPatternLabel = Pattern.strLabel;
			Marker.eEvidence = Pattern.eEvidence;
			Marker.strNote = Pattern.strNote;
			Marker.bPatternMarker = true;
			Staged.push_back(std::move(Marker));
		}
		else
		{
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Reference :
				Pattern.Sequences)
			{
				if (!Reference.Is_Resolved())
				{
					outStatus =
						"Valtan playlist requires a fully resolved source sequence document.";
					return false;
				}
				// JSON 참조 순서, 반복 번호, 원본 clip 순서로 펼친다. mode는 표시 정보로 복사하며 이 루프의 순서를 바꾸지 않는다.
				for (uint32_t iRepeat = 1u;
					iRepeat <= Reference.iRepeat; ++iRepeat)
				{
					for (size_t iSourceStep = 0u;
						iSourceStep < Reference.ResolvedClips.size(); ++iSourceStep)
					{
						VALTAN_PATTERN_PREVIEW_PLAY_ITEM Item;
						Item.iPatternNumber = Pattern.iNumber;
						Item.strPatternLabel = Pattern.strLabel;
						Item.eEvidence = Pattern.eEvidence;
						Item.strNote = Pattern.strNote;
						Item.iSourceActionId = Reference.iSourceActionId;
						Item.iSequenceIndex = Reference.iSequenceIndex;
						Item.iSequenceRepeatNumber = iRepeat;
						Item.iSequenceRepeatCount = Reference.iRepeat;
						Item.iSourceStepNumber =
							static_cast<uint32_t>(iSourceStep + 1u);
						Item.iSourceStepCount = static_cast<uint32_t>(
							Reference.ResolvedClips.size());
						Item.strSequenceName = Reference.strSequenceName;
						Item.strSequenceMode = Reference.strSequenceMode;
						Item.strClipName = Reference.ResolvedClips[iSourceStep];
						Staged.push_back(std::move(Item));
						// 일반 clip을 추가할 때 전체 항목 수 상한을 확인한다. marker 분기에는 같은 검사가 없어 마지막 marker로 상한을 넘을 수 있다.
						if (Staged.size() > MAX_PLAYLIST_ITEMS)
						{
							outStatus =
								"Valtan pattern preview playlist exceeds its bounded item budget.";
							return false;
						}
					}
				}
			}
		}

		// 패턴 내부의 펼친 step은 1부터 센다. 반복/source 내부 step과 별도로 UI에서 전체 진행을 표시한다.
		const size_t iPatternStepCount = Staged.size() - iPatternStart;
		if (0u == iPatternStepCount)
		{
			outStatus = "Valtan pattern preview produced an empty pattern.";
			return false;
		}
		for (size_t iStep = 0u; iStep < iPatternStepCount; ++iStep)
		{
			VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
				Staged[iPatternStart + iStep];
			Item.iStepNumber = static_cast<uint32_t>(iStep + 1u);
			Item.iStepCount = static_cast<uint32_t>(iPatternStepCount);
		}
	}

	// 완성한 목록을 한 번 교체한다. 항목의 문자열은 독립 값이며 Tool은 이후 자신의 목록과 모델 weak_ptr을 함께 관리한다.
	outPlaylist = std::move(Staged);
	outStatus = "Built bounded Valtan source-sequence playlist with " +
		std::to_string(outPlaylist.size()) + " item(s).";
	return true;
}

// 참조 패널이 표시할 enum 이름을 정적 문자열로 반환한다. 정의되지 않은 값은 INVALID로 표시한다.
const char_t* Client::CValtanPatternPreviewDocument::Evidence_Name(
	const VALTAN_PATTERN_PREVIEW_EVIDENCE eEvidence)
{
	switch (eEvidence)
	{
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::SOURCE_FAMILY_DIRECT:
		return "SOURCE_FAMILY_DIRECT";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::USER_CONFIRMED_FAMILY:
		return "USER_CONFIRMED_FAMILY";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::CANDIDATE:
		return "CANDIDATE";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::COMPOSITE:
		return "COMPOSITE";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::UNRESOLVED:
		return "UNRESOLVED";
	case VALTAN_PATTERN_PREVIEW_EVIDENCE::NO_ANIMATION:
		return "NO_ANIMATION";
	default:
		return "INVALID";
	}
}
