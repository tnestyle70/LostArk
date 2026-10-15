// Animation Tool이 참조 Action 슬롯으로 구성한 로컬 클립 순서를 읽고 저장한다.
// 판정 Logic과 Server 권위 전투는 Composition 및 게시 데이터의 별도 책임이다.
#include "KoukuSaydonAnimationPatternDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <io.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	// 애니메이션 패턴 schema와 편집 상한이다. 원본 Action 참조 및 보스 Composition 문서와 구분한다.
	constexpr std::string_view PATTERN_SCHEMA =
		"lostark.kouku-saydon-animation-pattern-bindings";
	constexpr std::string_view REFERENCE_ONLY = "REFERENCE_ONLY";
	constexpr std::string_view END_EXACT = "EXACT";
	constexpr std::string_view END_HOLD_LAST_POSE = "HOLD_LAST_POSE";
	constexpr std::string_view END_LOOP_TO_WINDOW = "LOOP_TO_WINDOW";
	constexpr std::uint32_t DOCUMENT_VERSION = 1u;
	constexpr std::size_t MAX_PATTERNS = 4096u;
	constexpr std::size_t MAX_CLIPS_PER_PATTERN = 4096u;
	constexpr std::size_t MAX_DOCUMENT_CLIPS = 16384u;
	constexpr std::uint32_t MAX_CLIP_TIME_MS = 600000u;
	constexpr std::uint32_t MAX_NEXT_ORDINAL = 1000000u;
	constexpr std::uintmax_t MAX_PATTERN_BYTES = 8u * 1024u * 1024u;
	constexpr std::uintmax_t MAX_REFERENCE_BYTES = 16u * 1024u * 1024u;

	// 필수 키와 타입을 함께 검사한다. 반환 포인터는 입력 JSON의 원소를 차용한다.
	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	// 필수 키의 누락과 알 수 없는 추가 키를 함께 거절한다.
	bool Has_ExactProperties(
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

	// 저장 identity와 클립 이름의 ASCII 문자 및 길이를 검사한다. 점 경로 토큰은 거절한다.
	bool Is_StableToken(const std::string_view value)
	{
		if (value.empty() || value.size() > 255u)
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
		return value != "." && value != "..";
	}

	// 이 문서가 지원하는 네 쿠크 프로필만 허용한다. 임의 프로필의 파일 경로를 만들지 않는다.
	bool Is_SupportedProfile(const std::string_view value)
	{
		return value == "MN_RPCT_05" || value == "MN_RPCT_06" ||
			value == "MN_RPCT_07" || value == "MN_RPCZ_00";
	}

	// 참조 revision이 소문자 16진수 64자리인지 검사한다. 참조 파일을 해시하는 함수는 아니다.
	bool Is_LowerHexSha256(const std::string_view value)
	{
		if (64u != value.size())
			return false;
		return std::all_of(value.begin(), value.end(), [](const char character)
		{
			return (character >= '0' && character <= '9') ||
				(character >= 'a' && character <= 'f');
		});
	}

	// 표시 이름의 byte 길이와 제어 문자를 검사한다. UTF-8 전체 바이트열 검증은 하지 않는다.
	bool Is_DisplayName(const std::string_view value)
	{
		if (value.empty() || value.size() > 1024u)
			return false;
		return std::none_of(value.begin(), value.end(), [](const char character)
		{
			return static_cast<unsigned char>(character) < 0x20u;
		});
	}

	// 저장된 클립 종료 정책을 알려진 세 가지로 제한한다. 실제 포즈 재생은 Animation Tool이 수행한다.
	bool Is_EndPolicy(const std::string_view value)
	{
		return value == END_EXACT || value == END_HOLD_LAST_POSE ||
			value == END_LOOP_TO_WINDOW;
	}

	// 유한하고 범위 안의 정수 값이면 출력한다. 실수 토큰 표기라도 값이 정수이면 이 파서는 허용한다.
	bool Try_ParseUnsigned(
		const DATA_JSON_VALUE& value,
		const std::uint32_t maximum,
		std::uint32_t& outValue)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) ||
			std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<std::uint32_t>(number);
		return true;
	}

	// 원본 클립 시계의 배율을 0.01부터 16까지 유한한 값으로 읽는다.
	bool Try_ParsePlayRate(
		const DATA_JSON_VALUE& value,
		f32_t& outValue)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.01 || number > 16.0)
			return false;
		outValue = static_cast<f32_t>(number);
		return std::isfinite(outValue);
	}

	// 시작 ms, 양수 재생 길이 ms와 배율의 허용 범위를 검사한다. 실제 모델 클립의 끝 시각은 여기서 비교하지 않는다.
	bool Is_ValidTiming(
		const std::uint32_t sourceStartMs,
		const std::uint32_t playMs,
		const f32_t playRate)
	{
		return sourceStartMs <= MAX_CLIP_TIME_MS &&
			playMs > 0u && playMs <= MAX_CLIP_TIME_MS &&
			std::isfinite(playRate) && playRate >= 0.01f &&
			playRate <= 16.f;
	}

	// 저장 ID가 지정 prefix와 양의 십진 번호로 구성되었는지 확인한다. 선행 0과 남는 문자를 거절하며 성공 시에만 번호를 출력한다.
	bool Try_ParseIdOrdinal(
		const std::string_view value,
		const std::string_view prefix,
		std::uint32_t& outOrdinal)
	{
		if (!Is_StableToken(value) || !value.starts_with(prefix) ||
			value.size() == prefix.size())
		{
			return false;
		}
		const std::string_view suffix = value.substr(prefix.size());
		if (suffix.front() == '0')
			return false;
		std::uint32_t ordinal = 0u;
		const std::from_chars_result result = std::from_chars(
			suffix.data(), suffix.data() + suffix.size(), ordinal);
		if (result.ec != std::errc{} ||
			result.ptr != suffix.data() + suffix.size() ||
			0u == ordinal || ordinal >= MAX_NEXT_ORDINAL)
		{
			return false;
		}
		outOrdinal = ordinal;
		return true;
	}

	// 프로필별 패턴 ID의 생성 범위를 만든다. UI 목록 index를 저장 identity로 사용하지 않는다.
	std::string Build_PatternPrefix(const std::string_view profileId)
	{
		return "koukusaydon." + std::string(profileId) + ".pattern.";
	}

	// 배치 ID를 소유 패턴 아래에 두어 다른 패턴의 배치와 섞이지 않게 한다.
	std::string Build_OccurrencePrefix(const std::string_view patternId)
	{
		return std::string(patternId) + ".clip.";
	}

	// Action ID, Stage ID와 Slot ID를 하나의 참조 키로 결합한다. ID에 허용하지 않는 개행을 구분자로 사용한다.
	std::string Build_SourceKey(
		const std::uint32_t sourceActionId,
		const std::string_view stageId,
		const std::string_view slotId)
	{
		return std::to_string(sourceActionId) + "\n" +
			std::string(stageId) + "\n" + std::string(slotId);
	}

	// 프로필에 맞는 파일 이름과 점 경로 요소를 검사한다. 프로젝트 루트 내부에 있는지까지 판정하는 함수는 아니다.
	bool Is_ExpectedPath(
		const std::filesystem::path& path,
		const std::string_view profileId)
	{
		if (path.empty() || !Is_SupportedProfile(profileId) ||
			path.filename() != std::filesystem::path(
				std::string(profileId) + ".patternbindings.json"))
		{
			return false;
		}
		for (const std::filesystem::path& component : path)
		{
			if (component == L"." || component == L"..")
				return false;
		}
		return true;
	}

	// Parse_Text와 Validate가 공유하는 문서 구조 검사다. 실제 참조와 모델 없이 ID, 카운터, 개수와 재생 값만 확인한다.
	bool Validate_DocumentShape(
		const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
		std::string& outStatus)
	{
		if (document.iFormatVersion != DOCUMENT_VERSION ||
			!Is_SupportedProfile(document.strProfileId) ||
			!Is_LowerHexSha256(document.strReferenceRevision) ||
			document.strAuthority != REFERENCE_ONLY ||
			document.iNextPatternOrdinal < 1u ||
			document.iNextPatternOrdinal > MAX_NEXT_ORDINAL ||
			document.Patterns.size() > MAX_PATTERNS)
		{
			outStatus =
				"KoukuSaydon animation pattern header or next-pattern ordinal is invalid.";
			return false;
		}

		const std::string patternPrefix = Build_PatternPrefix(
			document.strProfileId);
		// 패턴과 배치 ID의 중복을 따로 검사한다. totalClips는 문서 전체의 누적 상한을 검사하는 데 사용한다.
		std::unordered_set<std::string> patternIds;
		std::unordered_set<std::string> occurrenceIds;
		std::size_t totalClips = 0u;
		for (const KOUKU_SAYDON_ANIMATION_PATTERN& pattern : document.Patterns)
		{
			std::uint32_t patternOrdinal = 0u;
			if (!Try_ParseIdOrdinal(pattern.strPatternId,
					patternPrefix, patternOrdinal) ||
				patternOrdinal >= document.iNextPatternOrdinal ||
				!patternIds.insert(pattern.strPatternId).second ||
				!Is_DisplayName(pattern.strDisplayName) ||
				pattern.iNextOccurrenceOrdinal < 1u ||
				pattern.iNextOccurrenceOrdinal > MAX_NEXT_ORDINAL ||
				pattern.Clips.empty() ||
				pattern.Clips.size() > MAX_CLIPS_PER_PATTERN ||
				pattern.Clips.size() > MAX_DOCUMENT_CLIPS - totalClips)
			{
				outStatus =
					"KoukuSaydon animation pattern identity, counter, or clip count is invalid.";
				return false;
			}
			totalClips += pattern.Clips.size();

			const std::string occurrencePrefix = Build_OccurrencePrefix(
				pattern.strPatternId);
			for (const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& clip : pattern.Clips)
			{
				std::uint32_t occurrenceOrdinal = 0u;
				if (!Try_ParseIdOrdinal(clip.strOccurrenceId,
						occurrencePrefix, occurrenceOrdinal) ||
					occurrenceOrdinal >= pattern.iNextOccurrenceOrdinal ||
					!occurrenceIds.insert(clip.strOccurrenceId).second ||
					!Is_StableToken(clip.strStageId) ||
					!Is_StableToken(clip.strSlotId) ||
					!Is_StableToken(clip.strRuntimeClip) ||
					!Is_ValidTiming(clip.iSourceStartMs,
						clip.iPlayMs, clip.fPlayRate) ||
					!Is_EndPolicy(clip.strEndPolicy))
				{
					outStatus =
						"KoukuSaydon animation pattern clip identity, timing, or end policy is invalid.";
					return false;
				}
			}
		}

		outStatus = "Validated KoukuSaydon animation pattern document structure.";
		return true;
	}

	// 파일 크기를 먼저 확인한 뒤 임시 문자열로 읽는다. 완전한 읽기에 성공했을 때만 outText를 교체한다.
	bool Read_Text(
		const std::filesystem::path& path,
		const std::uintmax_t maximumBytes,
		std::string& outText,
		std::string& outStatus,
		const std::string_view label)
	{
		std::error_code fileError;
		const std::uintmax_t fileBytes = std::filesystem::file_size(
			path, fileError);
		if (path.empty() || fileError || fileBytes > maximumBytes)
		{
			outStatus = std::string(label) +
				" document is missing or exceeds its size limit: " +
				path.string();
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = std::string(label) +
				" document could not be opened: " + path.string();
			return false;
		}
		std::string staged{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad() || staged.size() != fileBytes)
		{
			outStatus = std::string(label) +
				" document could not be read completely: " + path.string();
			return false;
		}
		outText = std::move(staged);
		return true;
	}

	// 저장 시점의 Action 참조를 읽고 엄격한 파싱 및 모델 검증을 수행한다. 성공한 임시 문서만 outReference에 교체한다.
	bool Load_ReferenceFromPath(
		const std::filesystem::path& path,
		const std::string_view expectedProfileId,
		const std::string_view expectedModelAssetId,
		const std::vector<std::string>& availableClips,
		KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
		std::string& outStatus)
	{
		std::string text;
		KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT staged;
		if (!Read_Text(path, MAX_REFERENCE_BYTES, text, outStatus,
				"KoukuSaydon animation action reference") ||
			!CKoukuSaydonAnimationActionDocument::Parse_ReferenceText(
				text, staged, outStatus) ||
			!CKoukuSaydonAnimationActionDocument::Validate_Reference(
				staged, expectedProfileId, expectedModelAssetId,
				availableClips, outStatus))
		{
			return false;
		}
		outReference = std::move(staged);
		return true;
	}

	// 저작한 클립 목록을 같은 순서의 JSON으로 쓴다. float 재파싱의 값 보존을 위해 충분한 자릿수를 사용한다.
	// 이 helper는 검증이나 파일 교체를 하지 않는다.
	std::string Serialize(
		const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document)
	{
		std::ostringstream output;
		output << std::setprecision(
			std::numeric_limits<f32_t>::max_digits10);
		output << "{\n"
			<< "  \"schema\": \"" << PATTERN_SCHEMA << "\",\n"
			<< "  \"formatVersion\": 1,\n"
			<< "  \"profileId\": \""
			<< CDataJson::Escape(document.strProfileId) << "\",\n"
			<< "  \"referenceRevision\": \""
			<< CDataJson::Escape(document.strReferenceRevision) << "\",\n"
			<< "  \"authority\": \"" << REFERENCE_ONLY << "\",\n"
			<< "  \"nextPatternOrdinal\": "
			<< document.iNextPatternOrdinal << ",\n"
			<< "  \"patterns\": [\n";

		for (std::size_t patternIndex = 0u;
			patternIndex < document.Patterns.size(); ++patternIndex)
		{
			const KOUKU_SAYDON_ANIMATION_PATTERN& pattern =
				document.Patterns[patternIndex];
			output << "    {\n"
				<< "      \"patternId\": \""
				<< CDataJson::Escape(pattern.strPatternId) << "\",\n"
				<< "      \"displayName\": \""
				<< CDataJson::Escape(pattern.strDisplayName) << "\",\n"
				<< "      \"sourceActionId\": "
				<< pattern.iSourceActionId << ",\n"
				<< "      \"nextOccurrenceOrdinal\": "
				<< pattern.iNextOccurrenceOrdinal << ",\n"
				<< "      \"clips\": [\n";
			for (std::size_t clipIndex = 0u;
				clipIndex < pattern.Clips.size(); ++clipIndex)
			{
				const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& clip =
					pattern.Clips[clipIndex];
				output << "        {\n"
					<< "          \"occurrenceId\": \""
					<< CDataJson::Escape(clip.strOccurrenceId) << "\",\n"
					<< "          \"stageId\": \""
					<< CDataJson::Escape(clip.strStageId) << "\",\n"
					<< "          \"slotId\": \""
					<< CDataJson::Escape(clip.strSlotId) << "\",\n"
					<< "          \"runtimeClip\": \""
					<< CDataJson::Escape(clip.strRuntimeClip) << "\",\n"
					<< "          \"sourceStartMs\": "
					<< clip.iSourceStartMs << ",\n"
					<< "          \"playMs\": " << clip.iPlayMs << ",\n"
					<< "          \"playRate\": " << clip.fPlayRate << ",\n"
					<< "          \"endPolicy\": \""
					<< CDataJson::Escape(clip.strEndPolicy) << "\"\n"
					<< "        }"
					<< (clipIndex + 1u < pattern.Clips.size() ? "," : "")
					<< "\n";
			}
			output << "      ]\n"
				<< "    }"
				<< (patternIndex + 1u < document.Patterns.size() ? "," : "")
				<< "\n";
		}
		output << "  ]\n}\n";
		return output.str();
	}

	// 저장에 실패한 임시 파일을 정리한다. error_code를 사용해 정리 실패가 원래 오류를 가리지 않게 한다.
	void Remove_Temporary(const std::filesystem::path& path)
	{
		std::error_code cleanupError;
		std::filesystem::remove(path, cleanupError);
	}
}

// 지원 프로필의 저작 patternbindings 경로를 프로젝트 Data 루트에서 구한다. Bin 게시 파일을 선택하지 않는다.
std::filesystem::path Client::CKoukuSaydonAnimationPatternDocument::Resolve_Path(
	const std::string_view profileId)
{
	if (!Is_SupportedProfile(profileId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored/KoukuSaydon") /
		std::filesystem::path(
			std::string(profileId) + ".patternbindings.json"));
}

// Load와 저장 재검증이 JSON을 임시 문서로 파싱한다. 마지막 구조 검증까지 성공한 경우에만 출력을 교체한다.
// 실패하면 기존 outDocument를 유지하며 개별 오류 행을 남기는 Composition의 부분 격리 방식은 사용하지 않는다.
bool_t Client::CKoukuSaydonAnimationPatternDocument::Parse_Text(
	const std::string_view text,
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	if (text.empty() || text.size() > MAX_PATTERN_BYTES)
	{
		outStatus = "KoukuSaydon animation pattern text exceeds its size limit.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "profileId", "referenceRevision",
			  "authority", "nextPatternOrdinal", "patterns" }))
	{
		outStatus = "KoukuSaydon animation pattern JSON is malformed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* profile = Required(
		root, "profileId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(
		root, "referenceRevision", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* authority = Required(
		root, "authority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* nextPatternOrdinal = Required(
		root, "nextPatternOrdinal", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* patterns = Required(
		root, "patterns", DATA_JSON_TYPE::ARRAY);

	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT staged;
	if (nullptr == schema || schema->Get_String() != PATTERN_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsigned(*version, DOCUMENT_VERSION,
			staged.iFormatVersion) ||
		staged.iFormatVersion != DOCUMENT_VERSION ||
		nullptr == profile || !Is_SupportedProfile(profile->Get_String()) ||
		nullptr == revision ||
		!Is_LowerHexSha256(revision->Get_String()) ||
		nullptr == authority || authority->Get_String() != REFERENCE_ONLY ||
		nullptr == nextPatternOrdinal ||
		!Try_ParseUnsigned(*nextPatternOrdinal, MAX_NEXT_ORDINAL,
			staged.iNextPatternOrdinal) ||
		0u == staged.iNextPatternOrdinal ||
		nullptr == patterns || patterns->Get_Array().size() > MAX_PATTERNS)
	{
		outStatus = "KoukuSaydon animation pattern header is invalid.";
		return false;
	}
	staged.strProfileId = profile->Get_String();
	staged.strReferenceRevision = revision->Get_String();
	staged.strAuthority = authority->Get_String();
	staged.Patterns.reserve(patterns->Get_Array().size());
	// 패턴마다의 클립 상한과 별도로 문서 전체의 누적 개수를 제한한다.
	std::size_t parsedClipCount = 0u;

	for (const DATA_JSON_VALUE& patternValue : patterns->Get_Array())
	{
		if (!Has_ExactProperties(patternValue,
				{ "patternId", "displayName", "sourceActionId",
				  "nextOccurrenceOrdinal", "clips" }))
		{
			outStatus = "KoukuSaydon animation pattern row has unexpected properties.";
			return false;
		}

		const DATA_JSON_VALUE* patternId = Required(
			patternValue, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* displayName = Required(
			patternValue, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceActionId = Required(
			patternValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* nextOccurrenceOrdinal = Required(
			patternValue, "nextOccurrenceOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* clips = Required(
			patternValue, "clips", DATA_JSON_TYPE::ARRAY);

		KOUKU_SAYDON_ANIMATION_PATTERN stagedPattern;
		if (nullptr == patternId ||
			!Is_StableToken(patternId->Get_String()) ||
			nullptr == displayName ||
			!Is_DisplayName(displayName->Get_String()) ||
			nullptr == sourceActionId ||
			!Try_ParseUnsigned(*sourceActionId,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedPattern.iSourceActionId) ||
			nullptr == nextOccurrenceOrdinal ||
			!Try_ParseUnsigned(*nextOccurrenceOrdinal, MAX_NEXT_ORDINAL,
				stagedPattern.iNextOccurrenceOrdinal) ||
			0u == stagedPattern.iNextOccurrenceOrdinal ||
			nullptr == clips || clips->Get_Array().empty() ||
			clips->Get_Array().size() > MAX_CLIPS_PER_PATTERN ||
			clips->Get_Array().size() > MAX_DOCUMENT_CLIPS - parsedClipCount)
		{
			outStatus = "KoukuSaydon animation pattern row is invalid.";
			return false;
		}
		parsedClipCount += clips->Get_Array().size();
		stagedPattern.strPatternId = patternId->Get_String();
		stagedPattern.strDisplayName = displayName->Get_String();
		stagedPattern.Clips.reserve(clips->Get_Array().size());

		for (const DATA_JSON_VALUE& clipValue : clips->Get_Array())
		{
			if (!Has_ExactProperties(clipValue,
					{ "occurrenceId", "stageId", "slotId", "runtimeClip",
					  "sourceStartMs", "playMs", "playRate", "endPolicy" }))
			{
				outStatus =
					"KoukuSaydon animation pattern clip has unexpected properties.";
				return false;
			}

			const DATA_JSON_VALUE* occurrenceId = Required(
				clipValue, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* stageId = Required(
				clipValue, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* slotId = Required(
				clipValue, "slotId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* runtimeClip = Required(
				clipValue, "runtimeClip", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* sourceStartMs = Required(
				clipValue, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playMs = Required(
				clipValue, "playMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playRate = Required(
				clipValue, "playRate", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* endPolicy = Required(
				clipValue, "endPolicy", DATA_JSON_TYPE::STRING);

			KOUKU_SAYDON_ANIMATION_PATTERN_CLIP stagedClip;
			if (nullptr == occurrenceId ||
				!Is_StableToken(occurrenceId->Get_String()) ||
				nullptr == stageId || !Is_StableToken(stageId->Get_String()) ||
				nullptr == slotId || !Is_StableToken(slotId->Get_String()) ||
				nullptr == runtimeClip ||
				!Is_StableToken(runtimeClip->Get_String()) ||
				nullptr == sourceStartMs ||
				!Try_ParseUnsigned(*sourceStartMs, MAX_CLIP_TIME_MS,
					stagedClip.iSourceStartMs) ||
				nullptr == playMs ||
				!Try_ParseUnsigned(*playMs, MAX_CLIP_TIME_MS,
					stagedClip.iPlayMs) ||
				0u == stagedClip.iPlayMs ||
				nullptr == playRate ||
				!Try_ParsePlayRate(*playRate, stagedClip.fPlayRate) ||
				nullptr == endPolicy ||
				!Is_EndPolicy(endPolicy->Get_String()))
			{
				outStatus = "KoukuSaydon animation pattern clip is invalid.";
				return false;
			}
			stagedClip.strOccurrenceId = occurrenceId->Get_String();
			stagedClip.strStageId = stageId->Get_String();
			stagedClip.strSlotId = slotId->Get_String();
			stagedClip.strRuntimeClip = runtimeClip->Get_String();
			stagedClip.strEndPolicy = endPolicy->Get_String();
			stagedPattern.Clips.push_back(std::move(stagedClip));
		}
		staged.Patterns.push_back(std::move(stagedPattern));
	}

	std::string shapeStatus;
	if (!Validate_DocumentShape(staged, shapeStatus))
	{
		outStatus = "KoukuSaydon animation pattern structure is invalid: " +
			shapeStatus;
		return false;
	}

	outDocument = std::move(staged);
	outStatus = "Parsed KoukuSaydon animation pattern document.";
	return true;
}

// 참조 문서를 먼저 검증하고 profile과 revision을 맞춘다. 다음으로 각 패턴의 원본 슬롯과 모델 클립을 연결한다.
// 입력 문서는 읽기 전용이며 성공해도 Server PRODUCT 권한을 부여하지 않는다.
bool_t Client::CKoukuSaydonAnimationPatternDocument::Validate(
	const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!CKoukuSaydonAnimationActionDocument::Validate_Reference(
		reference, expectedProfileId, expectedModelAssetId,
		availableClips, outStatus))
	{
		return false;
	}
	if (!Validate_DocumentShape(document, outStatus))
		return false;
	if (!Is_SupportedProfile(expectedProfileId) ||
		document.strProfileId != expectedProfileId ||
		document.strReferenceRevision != reference.strReferenceRevision)
	{
		outStatus =
			"KoukuSaydon animation pattern profile or reference revision is stale.";
		return false;
	}

	// REVIEW_CANDIDATE Action만 원본으로 허용한다. HOLDOUT Action의 슬롯은 연결 후보 집합에 넣지 않는다.
	std::unordered_set<std::uint32_t> reviewCandidateActions;
	std::unordered_set<std::string> referenceSlots;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action : reference.Actions)
	{
		if (action.strReviewStatus != "REVIEW_CANDIDATE")
			continue;
		reviewCandidateActions.insert(action.iSourceActionId);
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				referenceSlots.insert(Build_SourceKey(action.iSourceActionId,
					stage.strStageId, slot.strSlotId));
			}
		}
	}
	const std::unordered_set<std::string> availableClipSet(
		availableClips.begin(), availableClips.end());

	for (const KOUKU_SAYDON_ANIMATION_PATTERN& pattern : document.Patterns)
	{
		if (!reviewCandidateActions.contains(pattern.iSourceActionId))
		{
			outStatus =
				"KoukuSaydon animation pattern source action is not a REVIEW_CANDIDATE.";
			return false;
		}
		for (const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& clip : pattern.Clips)
		{
			const std::string key = Build_SourceKey(pattern.iSourceActionId,
				clip.strStageId, clip.strSlotId);
			if (!referenceSlots.contains(key) ||
				!availableClipSet.contains(clip.strRuntimeClip))
			{
				outStatus =
					"KoukuSaydon animation pattern clip is stale or absent from the target model.";
				return false;
			}
		}
	}

	outStatus = "Validated " + std::to_string(document.Patterns.size()) +
		" KoukuSaydon animation pattern(s).";
	return true;
}

// 경로, 읽기, 파싱과 참조 연결을 차례로 확인한다. 임시 문서가 모두 통과한 뒤에만 기존 출력과 교체한다.
bool_t Client::CKoukuSaydonAnimationPatternDocument::Load_FromPath(
	const std::filesystem::path& path,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	if (!Is_ExpectedPath(path, expectedProfileId))
	{
		outStatus = "KoukuSaydon animation pattern path or profile is invalid.";
		return false;
	}

	std::string text;
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT staged;
	if (!Read_Text(path, MAX_PATTERN_BYTES, text, outStatus,
			"KoukuSaydon animation pattern") ||
		!Parse_Text(text, staged, outStatus) ||
		!Validate(staged, reference, expectedProfileId,
			expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}

	outDocument = std::move(staged);
	outStatus = "Loaded KoukuSaydon animation pattern document.";
	return true;
}

// Animation Tool의 프로필 선택에서 기본 경로를 구해 동일한 Load_FromPath 경로로 위임한다.
bool_t Client::CKoukuSaydonAnimationPatternDocument::Load(
	const std::string_view profileId,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_Path(profileId);
	if (path.empty())
	{
		outStatus = "KoukuSaydon animation pattern profile path is invalid.";
		return false;
	}
	return Load_FromPath(path, reference, profileId, expectedModelAssetId,
		availableClips, outDocument, outStatus);
}

// Animation Tool이 편집한 로컬 패턴만 저장한다. Action 참조의 변경을 저장 준비와 최종 교체 전에 검사한다.
// 목적 pattern 파일의 bytes CAS나 세 문서 병합은 하지 않으며 참조 파일 자체도 수정하지 않는다.
bool_t Client::CKoukuSaydonAnimationPatternDocument::Save_Atomic(
	const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Validate(document, reference, expectedProfileId,
		expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}

	const std::filesystem::path referencePath =
		CKoukuSaydonAnimationActionDocument::Resolve_ReferencePath(
			expectedProfileId);
	const std::filesystem::path destination = Resolve_Path(expectedProfileId);
	if (referencePath.empty() || destination.empty() ||
		!Is_ExpectedPath(destination, expectedProfileId))
	{
		outStatus = "KoukuSaydon animation pattern destination is invalid.";
		return false;
	}

	// 사용자가 보던 Action 참조가 최신 파일과 같아야 저장 준비를 계속한다.
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT currentReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, currentReference, outStatus))
	{
		return false;
	}
	if (currentReference != reference)
	{
		outStatus =
			"KoukuSaydon animation action reference changed before pattern save.";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create the KoukuSaydon animation pattern directory.";
		return false;
	}

	// 같은 디렉터리의 고유 임시 파일에 쓰고 write, flush, commit과 close 결과를 확인한다.
	std::filesystem::path temporary = destination;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetCurrentThreadId()) +
		L"." + std::to_wstring(GetTickCount64());
	const std::string serialized = Serialize(document);
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		outStatus =
			"Could not open the temporary KoukuSaydon animation pattern document.";
		return false;
	}
	const bool_t wrote = serialized.size() ==
		fwrite(serialized.data(), 1u, serialized.size(), file);
	const bool_t flushed = 0 == fflush(file) && 0 == _commit(_fileno(file));
	const bool_t closed = 0 == fclose(file);
	if (!wrote || !flushed || !closed)
	{
		Remove_Temporary(temporary);
		outStatus =
			"Could not durably write the temporary KoukuSaydon animation pattern document.";
		return false;
	}

	// 임시 파일을 재파싱하여 의미 연결을 검사하고 원문서와 구조체 값이 같은지도 확인한다.
	std::string verificationText;
	KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT reparsed;
	std::string verificationStatus;
	if (!Read_Text(temporary, MAX_PATTERN_BYTES, verificationText,
			verificationStatus, "Temporary KoukuSaydon animation pattern") ||
		!Parse_Text(verificationText, reparsed, verificationStatus) ||
		!Validate(reparsed, currentReference, expectedProfileId,
			expectedModelAssetId, availableClips, verificationStatus) ||
		reparsed != document)
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon animation pattern temp verification failed: " +
			verificationStatus;
		return false;
	}

	// 임시 파일을 만드는 동안 Action 참조가 바뀌었으면 목적 파일을 교체하지 않는다. 비교 대상은 저장할 pattern 파일이 아니다.
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT finalReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, finalReference,
		verificationStatus))
	{
		Remove_Temporary(temporary);
		outStatus =
			"KoukuSaydon animation action reference revalidation failed during pattern save: " +
			verificationStatus;
		return false;
	}
	if (finalReference != currentReference)
	{
		Remove_Temporary(temporary);
		outStatus =
			"KoukuSaydon animation action reference changed during pattern save.";
		return false;
	}

	// 검증한 임시 파일로 목적 파일을 원자적으로 교체한다. 교체 성공 뒤 별도의 재열기 없이 true를 반환한다.
	if (!MoveFileExW(temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_Temporary(temporary);
		outStatus =
			"Could not atomically replace the KoukuSaydon animation pattern document.";
		return false;
	}
	outStatus = "Saved " + std::to_string(document.Patterns.size()) +
		" KoukuSaydon animation pattern(s) to " + destination.string();
	return true;
}
