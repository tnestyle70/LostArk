#include "KakulAnimationPatternDocument.h"

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

	constexpr std::string_view PATTERN_SCHEMA =
		"lostark.kakul-animation-pattern-bindings";
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

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

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

	bool Is_SupportedProfile(const std::string_view value)
	{
		return value == "MN_RPCT_05" || value == "MN_RPCT_06" ||
			value == "MN_RPCT_07" || value == "MN_RPCZ_00";
	}

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

	bool Is_DisplayName(const std::string_view value)
	{
		if (value.empty() || value.size() > 1024u)
			return false;
		return std::none_of(value.begin(), value.end(), [](const char character)
		{
			return static_cast<unsigned char>(character) < 0x20u;
		});
	}

	bool Is_EndPolicy(const std::string_view value)
	{
		return value == END_EXACT || value == END_HOLD_LAST_POSE ||
			value == END_LOOP_TO_WINDOW;
	}

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

	std::string Build_PatternPrefix(const std::string_view profileId)
	{
		return "kakul." + std::string(profileId) + ".pattern.";
	}

	std::string Build_OccurrencePrefix(const std::string_view patternId)
	{
		return std::string(patternId) + ".clip.";
	}

	std::string Build_SourceKey(
		const std::uint32_t sourceActionId,
		const std::string_view stageId,
		const std::string_view slotId)
	{
		return std::to_string(sourceActionId) + "\n" +
			std::string(stageId) + "\n" + std::string(slotId);
	}

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

	bool Validate_DocumentShape(
		const KAKUL_ANIMATION_PATTERN_DOCUMENT& document,
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
				"Kakul animation pattern header or next-pattern ordinal is invalid.";
			return false;
		}

		const std::string patternPrefix = Build_PatternPrefix(
			document.strProfileId);
		std::unordered_set<std::string> patternIds;
		std::unordered_set<std::string> occurrenceIds;
		std::size_t totalClips = 0u;
		for (const KAKUL_ANIMATION_PATTERN& pattern : document.Patterns)
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
					"Kakul animation pattern identity, counter, or clip count is invalid.";
				return false;
			}
			totalClips += pattern.Clips.size();

			const std::string occurrencePrefix = Build_OccurrencePrefix(
				pattern.strPatternId);
			for (const KAKUL_ANIMATION_PATTERN_CLIP& clip : pattern.Clips)
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
						"Kakul animation pattern clip identity, timing, or end policy is invalid.";
					return false;
				}
			}
		}

		outStatus = "Validated Kakul animation pattern document structure.";
		return true;
	}

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

	bool Load_ReferenceFromPath(
		const std::filesystem::path& path,
		const std::string_view expectedProfileId,
		const std::string_view expectedModelAssetId,
		const std::vector<std::string>& availableClips,
		KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
		std::string& outStatus)
	{
		std::string text;
		KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT staged;
		if (!Read_Text(path, MAX_REFERENCE_BYTES, text, outStatus,
				"Kakul animation action reference") ||
			!CKakulAnimationActionDocument::Parse_ReferenceText(
				text, staged, outStatus) ||
			!CKakulAnimationActionDocument::Validate_Reference(
				staged, expectedProfileId, expectedModelAssetId,
				availableClips, outStatus))
		{
			return false;
		}
		outReference = std::move(staged);
		return true;
	}

	std::string Serialize(
		const KAKUL_ANIMATION_PATTERN_DOCUMENT& document)
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
			const KAKUL_ANIMATION_PATTERN& pattern =
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
				const KAKUL_ANIMATION_PATTERN_CLIP& clip =
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

	void Remove_Temporary(const std::filesystem::path& path)
	{
		std::error_code cleanupError;
		std::filesystem::remove(path, cleanupError);
	}
}

std::filesystem::path Client::CKakulAnimationPatternDocument::Resolve_Path(
	const std::string_view profileId)
{
	if (!Is_SupportedProfile(profileId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored/KakulSaydon") /
		std::filesystem::path(
			std::string(profileId) + ".patternbindings.json"));
}

bool_t Client::CKakulAnimationPatternDocument::Parse_Text(
	const std::string_view text,
	KAKUL_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	if (text.empty() || text.size() > MAX_PATTERN_BYTES)
	{
		outStatus = "Kakul animation pattern text exceeds its size limit.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "profileId", "referenceRevision",
			  "authority", "nextPatternOrdinal", "patterns" }))
	{
		outStatus = "Kakul animation pattern JSON is malformed: " + parseError;
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

	KAKUL_ANIMATION_PATTERN_DOCUMENT staged;
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
		outStatus = "Kakul animation pattern header is invalid.";
		return false;
	}
	staged.strProfileId = profile->Get_String();
	staged.strReferenceRevision = revision->Get_String();
	staged.strAuthority = authority->Get_String();
	staged.Patterns.reserve(patterns->Get_Array().size());
	std::size_t parsedClipCount = 0u;

	for (const DATA_JSON_VALUE& patternValue : patterns->Get_Array())
	{
		if (!Has_ExactProperties(patternValue,
				{ "patternId", "displayName", "sourceActionId",
				  "nextOccurrenceOrdinal", "clips" }))
		{
			outStatus = "Kakul animation pattern row has unexpected properties.";
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

		KAKUL_ANIMATION_PATTERN stagedPattern;
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
			outStatus = "Kakul animation pattern row is invalid.";
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
					"Kakul animation pattern clip has unexpected properties.";
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

			KAKUL_ANIMATION_PATTERN_CLIP stagedClip;
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
				outStatus = "Kakul animation pattern clip is invalid.";
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
		outStatus = "Kakul animation pattern structure is invalid: " +
			shapeStatus;
		return false;
	}

	outDocument = std::move(staged);
	outStatus = "Parsed Kakul animation pattern document.";
	return true;
}

bool_t Client::CKakulAnimationPatternDocument::Validate(
	const KAKUL_ANIMATION_PATTERN_DOCUMENT& document,
	const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!CKakulAnimationActionDocument::Validate_Reference(
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
			"Kakul animation pattern profile or reference revision is stale.";
		return false;
	}

	std::unordered_set<std::uint32_t> reviewCandidateActions;
	std::unordered_set<std::string> referenceSlots;
	for (const KAKUL_ANIMATION_ACTION_REFERENCE& action : reference.Actions)
	{
		if (action.strReviewStatus != "REVIEW_CANDIDATE")
			continue;
		reviewCandidateActions.insert(action.iSourceActionId);
		for (const KAKUL_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
		{
			for (const KAKUL_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				referenceSlots.insert(Build_SourceKey(action.iSourceActionId,
					stage.strStageId, slot.strSlotId));
			}
		}
	}
	const std::unordered_set<std::string> availableClipSet(
		availableClips.begin(), availableClips.end());

	for (const KAKUL_ANIMATION_PATTERN& pattern : document.Patterns)
	{
		if (!reviewCandidateActions.contains(pattern.iSourceActionId))
		{
			outStatus =
				"Kakul animation pattern source action is not a REVIEW_CANDIDATE.";
			return false;
		}
		for (const KAKUL_ANIMATION_PATTERN_CLIP& clip : pattern.Clips)
		{
			const std::string key = Build_SourceKey(pattern.iSourceActionId,
				clip.strStageId, clip.strSlotId);
			if (!referenceSlots.contains(key) ||
				!availableClipSet.contains(clip.strRuntimeClip))
			{
				outStatus =
					"Kakul animation pattern clip is stale or absent from the target model.";
				return false;
			}
		}
	}

	outStatus = "Validated " + std::to_string(document.Patterns.size()) +
		" Kakul animation pattern(s).";
	return true;
}

bool_t Client::CKakulAnimationPatternDocument::Load_FromPath(
	const std::filesystem::path& path,
	const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KAKUL_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	if (!Is_ExpectedPath(path, expectedProfileId))
	{
		outStatus = "Kakul animation pattern path or profile is invalid.";
		return false;
	}

	std::string text;
	KAKUL_ANIMATION_PATTERN_DOCUMENT staged;
	if (!Read_Text(path, MAX_PATTERN_BYTES, text, outStatus,
			"Kakul animation pattern") ||
		!Parse_Text(text, staged, outStatus) ||
		!Validate(staged, reference, expectedProfileId,
			expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}

	outDocument = std::move(staged);
	outStatus = "Loaded Kakul animation pattern document.";
	return true;
}

bool_t Client::CKakulAnimationPatternDocument::Load(
	const std::string_view profileId,
	const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KAKUL_ANIMATION_PATTERN_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_Path(profileId);
	if (path.empty())
	{
		outStatus = "Kakul animation pattern profile path is invalid.";
		return false;
	}
	return Load_FromPath(path, reference, profileId, expectedModelAssetId,
		availableClips, outDocument, outStatus);
}

bool_t Client::CKakulAnimationPatternDocument::Save_Atomic(
	const KAKUL_ANIMATION_PATTERN_DOCUMENT& document,
	const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
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
		CKakulAnimationActionDocument::Resolve_ReferencePath(
			expectedProfileId);
	const std::filesystem::path destination = Resolve_Path(expectedProfileId);
	if (referencePath.empty() || destination.empty() ||
		!Is_ExpectedPath(destination, expectedProfileId))
	{
		outStatus = "Kakul animation pattern destination is invalid.";
		return false;
	}

	KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT currentReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, currentReference, outStatus))
	{
		return false;
	}
	if (currentReference != reference)
	{
		outStatus =
			"Kakul animation action reference changed before pattern save.";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create the Kakul animation pattern directory.";
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetCurrentThreadId()) +
		L"." + std::to_wstring(GetTickCount64());
	const std::string serialized = Serialize(document);
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		outStatus =
			"Could not open the temporary Kakul animation pattern document.";
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
			"Could not durably write the temporary Kakul animation pattern document.";
		return false;
	}

	std::string verificationText;
	KAKUL_ANIMATION_PATTERN_DOCUMENT reparsed;
	std::string verificationStatus;
	if (!Read_Text(temporary, MAX_PATTERN_BYTES, verificationText,
			verificationStatus, "Temporary Kakul animation pattern") ||
		!Parse_Text(verificationText, reparsed, verificationStatus) ||
		!Validate(reparsed, currentReference, expectedProfileId,
			expectedModelAssetId, availableClips, verificationStatus) ||
		reparsed != document)
	{
		Remove_Temporary(temporary);
		outStatus = "Kakul animation pattern temp verification failed: " +
			verificationStatus;
		return false;
	}

	KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT finalReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, finalReference,
		verificationStatus))
	{
		Remove_Temporary(temporary);
		outStatus =
			"Kakul animation action reference revalidation failed during pattern save: " +
			verificationStatus;
		return false;
	}
	if (finalReference != currentReference)
	{
		Remove_Temporary(temporary);
		outStatus =
			"Kakul animation action reference changed during pattern save.";
		return false;
	}

	if (!MoveFileExW(temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_Temporary(temporary);
		outStatus =
			"Could not atomically replace the Kakul animation pattern document.";
		return false;
	}
	outStatus = "Saved " + std::to_string(document.Patterns.size()) +
		" Kakul animation pattern(s) to " + destination.string();
	return true;
}
