// v1 Boss/Arena descriptor의 관계를 검사하는 reader다. 실제 보스 패턴 편집 문서나 Server 실행 catalog와 구분한다.
// 현재 Load_Pair의 직접 호출은 BossToolTests에서 확인되며 이 파일이 gameplay를 실행하지 않는다.
#include "BossCompositionDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <new>
#include <unordered_set>
#include <utility>

namespace
{
	using namespace Client;

	// 두 descriptor의 schema, 형식과 입력 크기를 제한한다. 트랙 시계의 단위는 ms다.
	constexpr std::string_view BOSS_SCHEMA = "lostark.boss-composition";
	constexpr std::string_view ARENA_SCHEMA = "lostark.arena-sequencer";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr uintmax_t MAX_DOCUMENT_BYTES = 32u * 1024u * 1024u;
	constexpr size_t MAX_SOURCE_DOCUMENTS = 64u;
	constexpr size_t MAX_PATTERNS = 1024u;
	constexpr size_t MAX_REFERENCE_PROFILES = 64u;
	constexpr size_t MAX_TRACKS = 8192u;
	constexpr uint32_t MAX_DURATION_MS = 3'600'000u;

	// 필수 키와 JSON 타입을 함께 확인한다. 반환 포인터는 입력 JSON이 소유하며 호출 중에만 차용한다.
	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char_t* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	// 정확히 일치하는 키 집합을 요구하여 누락과 알 수 없는 추가 필드를 거절한다.
	bool_t Has_ExactRequiredProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> required)
	{
		if (!object.Is_Object() || object.Get_Object().size() != required.size())
			return false;
		return std::all_of(required.begin(), required.end(),
			[&object](const std::string_view name)
			{
				return nullptr != object.Find(name);
			});
	}

	// 필수 키는 모두 요구하고 선택 키만 생략을 허용한다. 목록에 없는 키는 거절한다.
	bool_t Has_RequiredAndOptionalProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> required,
		const std::initializer_list<std::string_view> optional)
	{
		if (!object.Is_Object() ||
			object.Get_Object().size() < required.size() ||
			object.Get_Object().size() > required.size() + optional.size())
		{
			return false;
		}
		for (const std::string_view name : required)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		for (const auto& [name, unused] : object.Get_Object())
		{
			(void)unused;
			if (std::find(required.begin(), required.end(), name) == required.end() &&
				std::find(optional.begin(), optional.end(), name) == optional.end())
			{
				return false;
			}
		}
		return true;
	}

	// 정수 토큰의 유한한 범위 값을 성공할 때만 출력한다. 값이 정수여도 실수 표기의 JSON 토큰은 거절한다.
	bool_t Read_U32(
		const DATA_JSON_VALUE* value,
		uint32_t& outValue,
		const uint32_t maximum = (std::numeric_limits<uint32_t>::max)())
	{
		if (nullptr == value || !value->Is_Number() ||
			value->Was_FloatingPointToken())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	// ID의 ASCII 문자와 byte 길이를 검사한다. 저장소 경로의 안전성은 별도 함수가 확인한다.
	bool_t Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > 255u)
			return false;
		return std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					character == '_' || character == '-' || character == '.';
			});
	}

	// 원본 파일 역할은 대문자로 시작하고 대문자, 숫자와 밑줄만 사용한다.
	bool_t Is_SourceRole(const std::string_view value)
	{
		if (value.empty() || value.size() > 127u ||
			value.front() < 'A' || value.front() > 'Z')
		{
			return false;
		}
		return std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return (character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					character == '_';
			});
	}

	// 표시 문자열의 길이와 제어 문자를 검사한다. 탭은 허용하며 UTF-8 전체 검증을 수행하지는 않는다.
	bool_t Is_DisplayText(const std::string_view value)
	{
		return !value.empty() && value.size() <= 1024u &&
			std::none_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return character < 0x20u && character != '\t';
				});
	}

	// 선언된 해시의 소문자 16진수 형식을 확인한다. 원본 파일의 내용을 해시하지 않는다.
	bool_t Is_LowerHexSha256(const std::string_view value)
	{
		return 64u == value.size() &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return (character >= '0' && character <= '9') ||
						(character >= 'a' && character <= 'f');
				});
	}

	// Data 상대 경로의 형식을 검사하고 프로젝트 Data 루트에서 실제 일반 파일이 존재하는지도 확인한다.
	// 파일 내용의 schema나 선언된 기대 개수까지 읽는 함수는 아니다.
	bool_t Is_RepositoryDataPath(const std::string_view value)
	{
		if (!value.starts_with("Data/") || value.size() > 1024u ||
			std::string_view::npos != value.find('\\') ||
			std::string_view::npos != value.find(':'))
		{
			return false;
		}
		size_t begin = 0u;
		while (begin < value.size())
		{
			const size_t end = value.find('/', begin);
			const std::string_view component = value.substr(begin,
				std::string_view::npos == end ? value.size() - begin : end - begin);
			if (component.empty() || "." == component || ".." == component)
				return false;
			if (std::string_view::npos == end)
				break;
			begin = end + 1u;
		}
		const std::filesystem::path resolved = CProjectDataRoot::Resolve(
			std::filesystem::path(value.substr(5u)));
		std::error_code error;
		return !resolved.empty() &&
			std::filesystem::is_regular_file(resolved, error) && !error;
	}

	// 상태 문자열을 enum으로 읽는다. AUTHORITATIVE 문자열을 해석할 수 있어도 각 문서 Load는 이를 별도로 거절한다.
	bool_t Parse_Status(
		const DATA_JSON_VALUE* value,
		COMPOSITION_SOURCE_STATUS& outStatus)
	{
		if (nullptr == value || !value->Is_String())
			return false;
		if ("SHADOW" == value->Get_String())
			outStatus = COMPOSITION_SOURCE_STATUS::SHADOW;
		else if ("REFERENCE_ONLY" == value->Get_String())
			outStatus = COMPOSITION_SOURCE_STATUS::REFERENCE_ONLY;
		else if ("AUTHORITATIVE" == value->Get_String())
			outStatus = COMPOSITION_SOURCE_STATUS::AUTHORITATIVE;
		else
			return false;
		return true;
	}

	// 처음에 출력을 비우고 생략 또는 null은 빈 ID로 허용한다. 실패 시 출력 보존 계약이 아니므로 임시 문서에서 사용한다.
	bool_t Read_OptionalStableId(
		const DATA_JSON_VALUE* value,
		std::string& outValue)
	{
		outValue.clear();
		if (nullptr == value || value->Is_Null())
			return true;
		if (!value->Is_String() || !Is_StableId(value->Get_String()))
			return false;
		outValue = value->Get_String();
		return true;
	}

	// 32 MiB 이하의 일반 파일을 읽고 JSON 객체인지 확인한다. 크기 확인 뒤 파일이 자라거나 줄어든 경우도 거절한다.
	// 입력 문자열 할당 실패는 상태로 반환한다. JSON 출력은 임시 값으로 받아 상위 Load에서 실제 문서와 분리한다.
	bool_t Read_Document(
		const std::filesystem::path& path,
		DATA_JSON_VALUE& outRoot,
		std::string& outStatus)
	{
		std::error_code error;
		if (!std::filesystem::is_regular_file(path, error) || error)
		{
			outStatus = "Composition source is not a regular file: " +
				path.generic_string();
			return false;
		}
		const uintmax_t bytes = std::filesystem::file_size(path, error);
		if (error || bytes > MAX_DOCUMENT_BYTES)
		{
			outStatus = error ? "Could not inspect Composition source size" :
				"Composition source exceeds the 32 MiB parse limit";
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = "Could not open Composition source";
			return false;
		}
		std::string text;
		try
		{
			text.resize(static_cast<size_t>(bytes));
		}
		catch (const std::bad_alloc&)
		{
			outStatus = "Could not allocate bounded Composition source input";
			return false;
		}
		if (!text.empty())
			input.read(text.data(), static_cast<std::streamsize>(text.size()));
		if (input.bad() ||
			input.gcount() != static_cast<std::streamsize>(text.size()) ||
			std::char_traits<char_t>::eof() != input.peek())
		{
			outStatus = "Composition source changed or failed while reading";
			return false;
		}
		std::string parseError;
		if (!CDataJson::Parse(text, outRoot, parseError) || !outRoot.Is_Object())
		{
			outStatus = "Composition source JSON is invalid: " + parseError;
			return false;
		}
		return true;
	}

	// 원본 역할과 경로가 중복되지 않고 파일이 존재하는지 검사한다. 성공한 임시 목록만 outRows에 교체한다.
	bool_t Parse_SourceDocuments(
		const DATA_JSON_VALUE* rows,
		std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>& outRows,
		std::string& outStatus)
	{
		if (nullptr == rows || !rows->Is_Array() || rows->Get_Array().empty() ||
			rows->Get_Array().size() > MAX_SOURCE_DOCUMENTS)
		{
			outStatus = "Composition sourceDocuments must be a non-empty bounded array";
			return false;
		}
		std::unordered_set<std::string> roles;
		std::unordered_set<std::string> paths;
		std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT> staged;
		for (const DATA_JSON_VALUE& source : rows->Get_Array())
		{
			if (!Has_ExactRequiredProperties(source, { "role", "path" }))
			{
				outStatus = "Composition source document shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* role = Required(
				source, "role", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* path = Required(
				source, "path", DATA_JSON_TYPE::STRING);
			if (nullptr == role || !Is_SourceRole(role->Get_String()) ||
				nullptr == path || !Is_RepositoryDataPath(path->Get_String()) ||
				!roles.emplace(role->Get_String()).second ||
				!paths.emplace(path->Get_String()).second)
			{
				outStatus = "Composition source role/path is invalid, missing, or duplicated";
				return false;
			}
			staged.push_back({ role->Get_String(), path->Get_String() });
		}
		outRows = std::move(staged);
		return true;
	}

	using SOURCE_ROLE_PATH = std::pair<std::string_view, std::string_view>;

	// descriptor가 정확한 역할과 경로의 집합을 선언했는지 확인한다. 목록 순서는 비교 조건이 아니다.
	bool_t Has_ExactSourceClosure(
		const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>& sources,
		const std::initializer_list<SOURCE_ROLE_PATH> expected)
	{
		if (sources.size() != expected.size())
			return false;
		return std::all_of(expected.begin(), expected.end(),
			[&sources](const SOURCE_ROLE_PATH& value)
			{
				return sources.end() != std::find_if(
					sources.begin(), sources.end(),
					[&value](const BOSS_COMPOSITION_SOURCE_DOCUMENT& source)
					{
						return source.role == value.first &&
							source.path == value.second;
					});
			});
	}

	// 각 쿠크 프로필에 참조, 슬롯 binding과 pattern binding 세 파일이 정확한 역할로 연결되어야 한다.
	bool_t Has_KoukuSaydonSourceClosure(
		const std::vector<BOSS_COMPOSITION_SOURCE_DOCUMENT>& sources,
		const std::vector<BOSS_COMPOSITION_REFERENCE_PROFILE>& profiles)
	{
		if (sources.size() != profiles.size() * 3u)
			return false;
		for (const BOSS_COMPOSITION_REFERENCE_PROFILE& profile : profiles)
		{
			const std::string actionReferenceRole =
				"ACTION_REFERENCE_" + profile.profileId;
			const std::string actionBindingRole =
				"ACTION_BINDING_" + profile.profileId;
			const std::string patternBindingRole =
				"PATTERN_BINDING_" + profile.profileId;
			for (const SOURCE_ROLE_PATH expected : {
				SOURCE_ROLE_PATH{ actionReferenceRole, profile.actionReferencePath },
				SOURCE_ROLE_PATH{ actionBindingRole, profile.actionBindingPath },
				SOURCE_ROLE_PATH{ patternBindingRole, profile.patternBindingPath } })
			{
				const bool_t found = sources.end() != std::find_if(
					sources.begin(), sources.end(),
					[&expected](const BOSS_COMPOSITION_SOURCE_DOCUMENT& source)
					{
						return source.role == expected.first &&
							source.path == expected.second;
					});
				if (!found)
					return false;
			}
		}
		return true;
	}

	// 트랙 종류에 맞는 variant와 참조 ID를 함께 채운다. ACTOR_PATTERN은 소유 descriptor ID와 일치해야 한다.
	// World와 Camera 참조의 실제 payload 내용은 여기서 읽지 않는다.
	bool_t Parse_TrackPayload(
		const std::string& kind,
		const DATA_JSON_VALUE& payload,
		const std::string& bossCompositionId,
		ARENA_SEQUENCER_TRACK_KIND& outKind,
		ARENA_SEQUENCER_TRACK_REFERENCE& outReference)
	{
		if ("WORLD_SEQUENCE" == kind)
		{
			const DATA_JSON_VALUE* instanceId = Required(
				payload, "instanceId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactRequiredProperties(payload, { "instanceId" }) ||
				nullptr == instanceId || !Is_StableId(instanceId->Get_String()))
			{
				return false;
			}
			outKind = ARENA_SEQUENCER_TRACK_KIND::WORLD_SEQUENCE;
			outReference = ARENA_WORLD_SEQUENCE_REFERENCE{
				instanceId->Get_String() };
			return true;
		}
		if ("CAMERA_SHOT" == kind)
		{
			const DATA_JSON_VALUE* shotId = Required(
				payload, "shotId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactRequiredProperties(payload, { "shotId" }) ||
				nullptr == shotId || !Is_StableId(shotId->Get_String()))
			{
				return false;
			}
			outKind = ARENA_SEQUENCER_TRACK_KIND::CAMERA_SHOT;
			outReference = ARENA_CAMERA_SHOT_REFERENCE{ shotId->Get_String() };
			return true;
		}
		if ("ACTOR_PATTERN" == kind)
		{
			const DATA_JSON_VALUE* bossId = Required(
				payload, "bossCompositionId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* patternId = Required(
				payload, "patternId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactRequiredProperties(payload,
					{ "bossCompositionId", "patternId" }) ||
				nullptr == bossId || bossId->Get_String() != bossCompositionId ||
				nullptr == patternId || !Is_StableId(patternId->Get_String()))
			{
				return false;
			}
			outKind = ARENA_SEQUENCER_TRACK_KIND::ACTOR_PATTERN;
			outReference = ARENA_ACTOR_PATTERN_REFERENCE{
				bossId->Get_String(), patternId->Get_String() };
			return true;
		}
		// 위 세 종류 이외의 typed adapter는 지원하지 않는다. 해석하지 못한 payload를 그대로 받아들이지 않는다.
		return false;
	}
}

// 보스 descriptor를 임시 객체로 읽고 지원 identity별 계약을 검사한다. false 경로에서는 기존 멤버를 유지한다.
// 원본 파일의 존재와 선언한 coverage 내부 합계는 확인하지만 실제 gameplay payload를 로드하거나 전체 내용을 재집계하지 않는다.
bool_t Client::CBossCompositionDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	outStatus.clear();
	DATA_JSON_VALUE root;
	if (!Read_Document(path, root, outStatus))
		return false;
	if (!Has_ExactRequiredProperties(root,
		{ "schema", "formatVersion", "compositionId", "status", "revision",
		  "displayName", "bossArchetypeId", "encounterId", "areaId",
		  "sourceDocuments", "coverage", "patterns" }))
	{
		outStatus = "Boss Composition root shape is invalid";
		return false;
	}
	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* compositionId = Required(
		root, "compositionId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* displayName = Required(
		root, "displayName", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* areaId = Required(
		root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* coverage = Required(
		root, "coverage", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* patterns = Required(
		root, "patterns", DATA_JSON_TYPE::ARRAY);
	CBossCompositionDocument staged;
	uint32_t formatVersion = 0u;
	if (nullptr == schema || BOSS_SCHEMA != schema->Get_String() ||
		!Read_U32(root.Find("formatVersion"), formatVersion, FORMAT_VERSION) ||
		FORMAT_VERSION != formatVersion || nullptr == compositionId ||
		!Is_StableId(compositionId->Get_String()) ||
		!Read_U32(root.Find("revision"), staged.m_Revision) ||
		0u == staged.m_Revision ||
		!Parse_Status(root.Find("status"), staged.m_Status) ||
		nullptr == displayName || !Is_DisplayText(displayName->Get_String()) ||
		!Read_OptionalStableId(root.Find("bossArchetypeId"),
			staged.m_BossArchetypeId) ||
		!Read_OptionalStableId(root.Find("encounterId"),
			staged.m_EncounterId) ||
		nullptr == areaId || !Is_StableId(areaId->Get_String()) ||
		nullptr == coverage || nullptr == patterns ||
		patterns->Get_Array().size() > MAX_PATTERNS ||
		!Parse_SourceDocuments(root.Find("sourceDocuments"),
			staged.m_SourceDocuments, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Boss Composition header is invalid";
		return false;
	}
	staged.m_CompositionId = compositionId->Get_String();
	staged.m_DisplayName = displayName->Get_String();
	staged.m_AreaId = areaId->Get_String();
	std::unordered_set<std::string> patternIds;
	for (const DATA_JSON_VALUE& pattern : patterns->Get_Array())
	{
		const DATA_JSON_VALUE* patternId = Required(
			pattern, "patternId", DATA_JSON_TYPE::STRING);
		if (!Has_ExactRequiredProperties(pattern, { "patternId" }) ||
			nullptr == patternId || !Is_StableId(patternId->Get_String()) ||
			!patternIds.emplace(patternId->Get_String()).second)
		{
			outStatus = "Boss Composition Pattern index is invalid or duplicated";
			return false;
		}
		staged.m_Patterns.push_back({ patternId->Get_String() });
	}

	const DATA_JSON_VALUE* coverageKind = Required(
		*coverage, "kind", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* identity = Required(
		*coverage, "expectedIdentitySha256", DATA_JSON_TYPE::STRING);
	if (nullptr == coverageKind || nullptr == identity ||
		!Is_LowerHexSha256(identity->Get_String()))
	{
		outStatus = "Boss Composition coverage identity is invalid";
		return false;
	}
	staged.m_Coverage.kind = coverageKind->Get_String();
	staged.m_Coverage.expectedIdentitySha256 = identity->Get_String();
	// 발탄은 SHADOW와 고정 원본 파일 목록을 요구한다. 선언한 패턴 개수는 descriptor 자체의 ID 색인 개수와 대조한다.
	if ("boss.composition.valtan" == staged.m_CompositionId)
	{
		if (!Has_ExactRequiredProperties(*coverage,
			{ "kind", "expectedPatternCount", "expectedStageCount",
			  "expectedIdentitySha256" }) ||
			"VALTAN_SPLIT_JOIN" != staged.m_Coverage.kind ||
			COMPOSITION_SOURCE_STATUS::SHADOW != staged.m_Status ||
			"BOSS_VALTAN" != staged.m_BossArchetypeId ||
			"ENCOUNTER_VALTAN" != staged.m_EncounterId ||
			"LV_LUT_HEARTRB_ED" != staged.m_AreaId ||
			!Read_U32(coverage->Find("expectedPatternCount"),
				staged.m_Coverage.expectedPatternCount) ||
			!Read_U32(coverage->Find("expectedStageCount"),
				staged.m_Coverage.expectedStageCount) ||
			0u == staged.m_Coverage.expectedPatternCount ||
			0u == staged.m_Coverage.expectedStageCount ||
			staged.m_Coverage.expectedPatternCount != staged.m_Patterns.size() ||
			!Has_ExactSourceClosure(staged.m_SourceDocuments,
				{
					{ "GAMEPLAY", "Data/Valtan/Valtan.gameplay.json" },
					{ "PRESENTATION", "Data/Valtan/Valtan.presentation.json" },
					{ "COMBAT_OBJECTS", "Data/Valtan/Valtan.combatobjects.json" },
					{ "WORLD_EVENT_SETS", "Data/Valtan/Valtan.worldeventsets.json" },
					{ "ANIMATION_BINDINGS", "Data/Animation/Authored/Valtan/Valtan.patternbindings.json" },
					{ "EFFECT_V1_CUES", "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json" },
					{ "EFFECT_V1_ALIASES", "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json" },
					{ "EFFECT_V2_BINDINGS", "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json" },
					{ "PATTERN_SOUND_CUES", "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json" },
					{ "PATTERN_SHAKE_CUES", "Data/Animation/Authored/Valtan/Valtan.patternshakecues.json" },
					{ "COMBAT_OBJECT_SOUND_CUES", "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json" },
				}))
		{
			outStatus =
				"Valtan Boss Composition identity, coverage, or source role/path closure is invalid";
			return false;
		}
	}
	// 쿠크 v1 descriptor는 REFERENCE_ONLY이며 패턴 색인을 비워 둔다. 실제 제품 Composition과 같은 문서로 해석하지 않는다.
	else if ("boss.composition.koukusaydon" == staged.m_CompositionId)
	{
		const DATA_JSON_VALUE* profiles = Required(
			*coverage, "profiles", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactRequiredProperties(*coverage,
			{ "kind", "expectedProfileCount", "expectedActionCount",
			  "expectedIdentitySha256", "profiles" }) ||
			"KOUKU_SAYDON_ACTION_REFERENCE" != staged.m_Coverage.kind ||
			COMPOSITION_SOURCE_STATUS::REFERENCE_ONLY != staged.m_Status ||
			!staged.m_BossArchetypeId.empty() || !staged.m_EncounterId.empty() ||
			"LV_LUT_MIDNIGHTC_ED" != staged.m_AreaId ||
			!staged.m_Patterns.empty() || nullptr == profiles ||
			profiles->Get_Array().empty() ||
			profiles->Get_Array().size() > MAX_REFERENCE_PROFILES ||
			!Read_U32(coverage->Find("expectedProfileCount"),
				staged.m_Coverage.expectedProfileCount) ||
			!Read_U32(coverage->Find("expectedActionCount"),
				staged.m_Coverage.expectedActionCount))
		{
			outStatus = "KoukuSaydon Boss Composition identity or coverage is invalid";
			return false;
		}
		std::unordered_set<std::string> profileIds;
		std::unordered_set<std::string> sourcePaths;
		for (const BOSS_COMPOSITION_SOURCE_DOCUMENT& source :
			staged.m_SourceDocuments)
		{
			sourcePaths.emplace(source.path);
		}
		uint64_t actionCount = 0u;
		for (const DATA_JSON_VALUE& profile : profiles->Get_Array())
		{
			if (!Has_ExactRequiredProperties(profile,
				{ "profileId", "actionReferencePath", "actionBindingPath",
				  "patternBindingPath", "expectedActionCount",
				  "expectedReferenceRevision" }))
			{
				outStatus = "KoukuSaydon coverage profile shape is invalid";
				return false;
			}
			const DATA_JSON_VALUE* profileId = Required(
				profile, "profileId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* actionReferencePath = Required(
				profile, "actionReferencePath", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* actionBindingPath = Required(
				profile, "actionBindingPath", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* patternBindingPath = Required(
				profile, "patternBindingPath", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* referenceRevision = Required(
				profile, "expectedReferenceRevision", DATA_JSON_TYPE::STRING);
			BOSS_COMPOSITION_REFERENCE_PROFILE parsed;
			if (nullptr == profileId || !Is_StableId(profileId->Get_String()) ||
				!profileIds.emplace(profileId->Get_String()).second ||
				nullptr == actionReferencePath ||
				!Is_RepositoryDataPath(actionReferencePath->Get_String()) ||
				!sourcePaths.contains(actionReferencePath->Get_String()) ||
				nullptr == actionBindingPath ||
				!Is_RepositoryDataPath(actionBindingPath->Get_String()) ||
				!sourcePaths.contains(actionBindingPath->Get_String()) ||
				nullptr == patternBindingPath ||
				!Is_RepositoryDataPath(patternBindingPath->Get_String()) ||
				!sourcePaths.contains(patternBindingPath->Get_String()) ||
				nullptr == referenceRevision ||
				!Is_LowerHexSha256(referenceRevision->Get_String()) ||
				!Read_U32(profile.Find("expectedActionCount"),
					parsed.expectedActionCount) || 0u == parsed.expectedActionCount)
			{
				outStatus = "KoukuSaydon coverage profile is invalid or duplicated";
				return false;
			}
			parsed.profileId = profileId->Get_String();
			parsed.actionReferencePath = actionReferencePath->Get_String();
			parsed.actionBindingPath = actionBindingPath->Get_String();
			parsed.patternBindingPath = patternBindingPath->Get_String();
			parsed.expectedReferenceRevision = referenceRevision->Get_String();
			actionCount += parsed.expectedActionCount;
			staged.m_Coverage.profiles.push_back(std::move(parsed));
		}
		if (staged.m_Coverage.expectedProfileCount !=
				staged.m_Coverage.profiles.size() ||
			actionCount != staged.m_Coverage.expectedActionCount ||
			!Has_KoukuSaydonSourceClosure(staged.m_SourceDocuments,
				staged.m_Coverage.profiles))
		{
			outStatus =
				"KoukuSaydon coverage totals or source role/path closure disagree with its profiles";
			return false;
		}
	}
	else
	{
		outStatus = "Unsupported Boss Composition identity";
		return false;
	}

	*this = std::move(staged);
	outStatus = "Boss Composition source descriptor parsed: " +
		m_CompositionId + " (" + Status_ToString(m_Status) +
		", descriptor revision " +
		std::to_string(m_Revision) + ")";
	return true;
}

// 이 객체의 값을 초기 상태로 되돌린다. 저장소 파일을 변경하지 않는다.
void Client::CBossCompositionDocument::Clear()
{
	*this = {};
}

// 패턴 색인의 원소를 차용해 반환한다. 없으면 nullptr이며 이후 Load나 Clear가 원소의 수명을 끝낼 수 있다.
const Client::BOSS_COMPOSITION_PATTERN*
Client::CBossCompositionDocument::Find_Pattern(
	const std::string& patternId) const
{
	const auto found = std::find_if(m_Patterns.begin(), m_Patterns.end(),
		[&patternId](const BOSS_COMPOSITION_PATTERN& value)
		{
			return value.patternId == patternId;
		});
	return m_Patterns.end() == found ? nullptr : &*found;
}

// 진단용 이름을 정적 문자열로 반환한다. 호출자는 반환 포인터를 해제하지 않는다.
const char_t* Client::CBossCompositionDocument::Status_ToString(
	const COMPOSITION_SOURCE_STATUS status)
{
	switch (status)
	{
	case COMPOSITION_SOURCE_STATUS::SHADOW:
		return "SHADOW";
	case COMPOSITION_SOURCE_STATUS::REFERENCE_ONLY:
		return "REFERENCE_ONLY";
	case COMPOSITION_SOURCE_STATUS::AUTHORITATIVE:
		return "AUTHORITATIVE";
	default:
		return "?";
	}
}

// 지원하는 아레나 SHADOW descriptor의 identity, 원본 목록과 트랙을 임시 객체에 읽는다.
// false를 반환하는 경로에서는 이전 문서가 유지된다. 이 로드는 트랙 실행을 시작하지 않는다.
bool_t Client::CArenaSequencerDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	outStatus.clear();
	DATA_JSON_VALUE root;
	if (!Read_Document(path, root, outStatus))
		return false;
	if (!Has_ExactRequiredProperties(root,
		{ "schema", "formatVersion", "sequencerId", "status", "revision",
		  "displayName", "areaId", "bossCompositionId", "durationMs",
		  "sourceDocuments", "tracks" }))
	{
		outStatus = "Arena Sequencer root shape is invalid";
		return false;
	}
	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* sequencerId = Required(
		root, "sequencerId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* displayName = Required(
		root, "displayName", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* areaId = Required(
		root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* bossCompositionId = Required(
		root, "bossCompositionId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* tracks = Required(
		root, "tracks", DATA_JSON_TYPE::ARRAY);
	CArenaSequencerDocument staged;
	uint32_t formatVersion = 0u;
	if (nullptr == schema || ARENA_SCHEMA != schema->Get_String() ||
		!Read_U32(root.Find("formatVersion"), formatVersion, FORMAT_VERSION) ||
		FORMAT_VERSION != formatVersion || nullptr == sequencerId ||
		!Is_StableId(sequencerId->Get_String()) ||
		!Read_U32(root.Find("revision"), staged.m_Revision) ||
		0u == staged.m_Revision ||
		!Parse_Status(root.Find("status"), staged.m_Status) ||
		COMPOSITION_SOURCE_STATUS::AUTHORITATIVE == staged.m_Status ||
		nullptr == displayName || !Is_DisplayText(displayName->Get_String()) ||
		nullptr == areaId || !Is_StableId(areaId->Get_String()) ||
		nullptr == bossCompositionId ||
		!Is_StableId(bossCompositionId->Get_String()) ||
		!Read_U32(root.Find("durationMs"), staged.m_DurationMs,
			MAX_DURATION_MS) || nullptr == tracks ||
		tracks->Get_Array().size() > MAX_TRACKS ||
		!Parse_SourceDocuments(root.Find("sourceDocuments"),
			staged.m_SourceDocuments, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Arena Sequencer header is invalid";
		return false;
	}
	staged.m_SequencerId = sequencerId->Get_String();
	staged.m_DisplayName = displayName->Get_String();
	staged.m_AreaId = areaId->Get_String();
	staged.m_BossCompositionId = bossCompositionId->Get_String();
	if (COMPOSITION_SOURCE_STATUS::SHADOW != staged.m_Status ||
		("arena.sequencer.valtan" == staged.m_SequencerId &&
			("LV_LUT_HEARTRB_ED" != staged.m_AreaId ||
			 "boss.composition.valtan" != staged.m_BossCompositionId)) ||
		("arena.sequencer.koukusaydon" == staged.m_SequencerId &&
			("LV_LUT_MIDNIGHTC_ED" != staged.m_AreaId ||
			 "boss.composition.koukusaydon" != staged.m_BossCompositionId)) ||
		("arena.sequencer.valtan" != staged.m_SequencerId &&
		 "arena.sequencer.koukusaydon" != staged.m_SequencerId))
	{
		outStatus = "Arena Sequencer identity, Area, or Boss Composition link is invalid";
		return false;
	}
	const bool_t sourceClosureValid =
		("arena.sequencer.valtan" == staged.m_SequencerId &&
			Has_ExactSourceClosure(staged.m_SourceDocuments,
				{
					{ "BOSS_COMPOSITION", "Data/Compositions/Bosses/Valtan.bosscomposition.json" },
					{ "MAP_EFFECTS", "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json" },
					{ "MAP_LIGHTS", "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.maplights.json" },
				})) ||
		("arena.sequencer.koukusaydon" == staged.m_SequencerId &&
			Has_ExactSourceClosure(staged.m_SourceDocuments,
				{
					{ "BOSS_COMPOSITION", "Data/Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json" },
					{ "WORLD_SEQUENCES", "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json" },
					{ "CAMERA_SHOTS", "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json" },
					{ "SCENE_PROFILES", "Data/Rendering/Authored/RenderingProfiles.json" },
				}));
	if (!sourceClosureValid)
	{
		outStatus = "Arena Sequencer source role/path closure is invalid";
		return false;
	}
	if (!tracks->Get_Array().empty() && 0u == staged.m_DurationMs)
	{
		outStatus = "Arena Sequencer duration may be zero only when tracks are empty";
		return false;
	}

	// 트랙 ID는 중복을 거절하고 선택 종료 시각은 시작보다 크며 아레나 전체 길이 안에 있어야 한다.
	std::unordered_set<std::string> trackIds;
	for (const DATA_JSON_VALUE& trackValue : tracks->Get_Array())
	{
		if (!Has_RequiredAndOptionalProperties(trackValue,
			{ "trackId", "kind", "startMs", "payload" }, { "endMs" }))
		{
			outStatus = "Arena Sequencer track shape is invalid";
			return false;
		}
		const DATA_JSON_VALUE* trackId = Required(
			trackValue, "trackId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* kind = Required(
			trackValue, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* payload = Required(
			trackValue, "payload", DATA_JSON_TYPE::OBJECT);
		ARENA_SEQUENCER_TRACK track;
		if (nullptr == trackId || !Is_StableId(trackId->Get_String()) ||
			!trackId->Get_String().starts_with("track.") || nullptr == kind ||
			!Is_StableId(kind->Get_String()) || nullptr == payload ||
			!Read_U32(trackValue.Find("startMs"), track.startMs,
				staged.m_DurationMs) ||
			!trackIds.emplace(trackId->Get_String()).second ||
			!Parse_TrackPayload(kind->Get_String(), *payload,
				staged.m_BossCompositionId, track.kind, track.reference))
		{
			outStatus = "Arena Sequencer track identity, clock, or typed payload is invalid";
			return false;
		}
		track.trackId = trackId->Get_String();
		const DATA_JSON_VALUE* endMs = trackValue.Find("endMs");
		track.hasEndMs = nullptr != endMs;
		if (track.hasEndMs &&
			(!Read_U32(endMs, track.endMs, staged.m_DurationMs) ||
				track.endMs <= track.startMs))
		{
			outStatus = "Arena Sequencer track endMs is invalid";
			return false;
		}
		staged.m_Tracks.push_back(std::move(track));
	}
	*this = std::move(staged);
	outStatus = "Arena Sequencer source descriptor parsed: " +
		m_SequencerId + " (" +
		CBossCompositionDocument::Status_ToString(m_Status) +
		", descriptor revision " +
		std::to_string(m_Revision) + ")";
	return true;
}

// 아레나 descriptor의 값만 비운다. 다른 도메인의 payload나 디스크 파일은 변경하지 않는다.
void Client::CArenaSequencerDocument::Clear()
{
	*this = {};
}

// 지원 트랙 종류의 정적 표시 이름을 반환한다. enum 범위 밖의 값은 물음표로 표현한다.
const char_t* Client::CArenaSequencerDocument::TrackKind_ToString(
	const ARENA_SEQUENCER_TRACK_KIND kind)
{
	switch (kind)
	{
	case ARENA_SEQUENCER_TRACK_KIND::WORLD_SEQUENCE:
		return "WORLD_SEQUENCE";
	case ARENA_SEQUENCER_TRACK_KIND::CAMERA_SHOT:
		return "CAMERA_SHOT";
	case ARENA_SEQUENCER_TRACK_KIND::ACTOR_PATTERN:
		return "ACTOR_PATTERN";
	default:
		return "?";
	}
}

// 계약 검증 호출자가 선택한 알려진 보스와 아레나 한 쌍만 읽는다. 무관한 다른 보스 파일은 실패 조건에 포함하지 않는다.
// 임시 문서 둘의 identity와 Area 및 ACTOR_PATTERN 연결을 확인한 뒤에만 기존 목록을 교체한다.
bool_t Client::CCompositionDocumentCatalog::Load_Pair(
	const std::string& compositionId,
	const std::string& sequencerId,
	std::string& outStatus)
{
	outStatus.clear();
	std::filesystem::path bossRelativePath;
	std::filesystem::path arenaRelativePath;
	if ("boss.composition.valtan" == compositionId &&
		"arena.sequencer.valtan" == sequencerId)
	{
		bossRelativePath = L"Compositions/Bosses/Valtan.bosscomposition.json";
		arenaRelativePath = L"Compositions/Sequences/ValtanArena.sequencer.json";
	}
	else if ("boss.composition.koukusaydon" == compositionId &&
		"arena.sequencer.koukusaydon" == sequencerId)
	{
		bossRelativePath =
			L"Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json";
		arenaRelativePath =
			L"Compositions/Sequences/KoukuSaydonArena.sequencer.json";
	}
	else
	{
		outStatus = "Unsupported or mismatched Composition descriptor pair";
		return false;
	}
	const std::filesystem::path bossPath =
		CProjectDataRoot::Resolve(bossRelativePath);
	const std::filesystem::path arenaPath =
		CProjectDataRoot::Resolve(arenaRelativePath);
	if (bossPath.empty() || arenaPath.empty())
	{
		outStatus = "Composition catalog path escaped the project Data root";
		return false;
	}

	CBossCompositionDocument stagedBoss;
	CArenaSequencerDocument stagedArena;
	std::string status;
	if (!stagedBoss.Load(bossPath, status))
	{
		outStatus = "Boss Composition descriptor parse failed: " + status;
		return false;
	}
	if (!stagedArena.Load(arenaPath, status))
	{
		outStatus = "Arena Sequencer descriptor parse failed: " + status;
		return false;
	}
	if (stagedBoss.Get_CompositionId() != compositionId ||
		stagedArena.Get_SequencerId() != sequencerId ||
		stagedArena.Get_BossCompositionId() != compositionId ||
		stagedBoss.Get_AreaId() != stagedArena.Get_AreaId())
	{
		outStatus =
			"Composition descriptor pair identity or Area link does not agree";
		return false;
	}
	// actor 트랙은 같은 보스 descriptor의 패턴 색인으로 연결되어야 한다. World와 Camera의 내용 검증으로 확대하지 않는다.
	for (const ARENA_SEQUENCER_TRACK& track : stagedArena.Get_Tracks())
	{
		if (ARENA_SEQUENCER_TRACK_KIND::ACTOR_PATTERN != track.kind)
			continue;
		const ARENA_ACTOR_PATTERN_REFERENCE* actor =
			std::get_if<ARENA_ACTOR_PATTERN_REFERENCE>(&track.reference);
		if (nullptr == actor || actor->bossCompositionId != compositionId ||
			nullptr == stagedBoss.Find_Pattern(actor->patternId))
		{
			outStatus =
				"Arena ACTOR_PATTERN does not resolve in the linked Boss descriptor Pattern index";
			return false;
		}
	}
	// 두 목록을 먼저 준비하고 성공한 선택 쌍으로 기존 카탈로그를 교체한다. 이전 쌍에 누적해서 추가하지 않는다.
	std::vector<CBossCompositionDocument> stagedBosses;
	std::vector<CArenaSequencerDocument> stagedArenas;
	stagedBosses.push_back(std::move(stagedBoss));
	stagedArenas.push_back(std::move(stagedArena));
	m_BossDocuments = std::move(stagedBosses);
	m_ArenaDocuments = std::move(stagedArenas);
	outStatus = "Composition source descriptor pair parsed: " +
		compositionId + " + " + sequencerId;
	return true;
}

// 소유한 두 문서 목록을 비운다. Find로 차용한 기존 원소 포인터는 더 이상 사용할 수 없다.
void Client::CCompositionDocumentCatalog::Clear()
{
	m_BossDocuments.clear();
	m_ArenaDocuments.clear();
}

// 현재 보유한 보스 문서를 stable ID로 선형 조회한다. 반환 원소의 수명은 카탈로그가 소유한다.
const Client::CBossCompositionDocument*
Client::CCompositionDocumentCatalog::Find_Boss(
	const std::string& compositionId) const
{
	const auto found = std::find_if(m_BossDocuments.begin(), m_BossDocuments.end(),
		[&compositionId](const CBossCompositionDocument& value)
		{
			return value.Get_CompositionId() == compositionId;
		});
	return m_BossDocuments.end() == found ? nullptr : &*found;
}

// 현재 보유한 아레나 문서를 stable ID로 선형 조회한다. 없으면 nullptr이며 호출자는 포인터를 해제하지 않는다.
const Client::CArenaSequencerDocument*
Client::CCompositionDocumentCatalog::Find_Arena(
	const std::string& sequencerId) const
{
	const auto found = std::find_if(m_ArenaDocuments.begin(), m_ArenaDocuments.end(),
		[&sequencerId](const CArenaSequencerDocument& value)
		{
			return value.Get_SequencerId() == sequencerId;
		});
	return m_ArenaDocuments.end() == found ? nullptr : &*found;
}
