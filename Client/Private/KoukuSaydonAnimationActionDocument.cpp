#include "KoukuSaydonAnimationActionDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <io.h>

#include <algorithm>
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

	constexpr std::string_view REFERENCE_SCHEMA =
		"lostark.kouku-saydon-animation-action-reference";
	constexpr std::string_view AUTHORED_SCHEMA =
		"lostark.kouku-saydon-animation-action-bindings";
	constexpr std::string_view REFERENCE_ONLY = "REFERENCE_ONLY";
	constexpr std::string_view EXTRACTED_REFERENCE = "EXTRACTED_REFERENCE";
	constexpr std::string_view PROJECT_AUTHORED = "PROJECT_AUTHORED";
	constexpr std::uint32_t DOCUMENT_VERSION = 1u;
	constexpr std::size_t MAX_ACTIONS = 1024u;
	constexpr std::size_t MAX_STAGES_PER_ACTION = 1024u;
	constexpr std::size_t MAX_SLOTS_PER_STAGE = 64u;
	constexpr std::size_t MAX_HOLDOUTS_PER_STAGE = 64u;
	constexpr std::size_t MAX_AUTHORED_BINDINGS = 16384u;
	constexpr std::uint32_t MAX_CLIP_TIME_MS = 600000u;
	constexpr std::uintmax_t MAX_REFERENCE_BYTES = 16u * 1024u * 1024u;
	constexpr std::uintmax_t MAX_AUTHORED_BYTES = 8u * 1024u * 1024u;

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

	bool Is_SafeAssetId(const std::string_view value)
	{
		if (value.empty() || value.size() > 1024u ||
			value.front() == '/' || value.back() == '/' ||
			std::string_view::npos != value.find('\\') ||
			std::string_view::npos != value.find(':'))
		{
			return false;
		}

		std::size_t begin = 0u;
		while (begin < value.size())
		{
			const std::size_t end = value.find('/', begin);
			const std::string_view component = value.substr(
				begin, std::string_view::npos == end ?
					value.size() - begin : end - begin);
			if (!Is_StableToken(component))
				return false;
			if (std::string_view::npos == end)
				break;
			begin = end + 1u;
		}
		return true;
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

	bool Try_ParsePlayMs(
		const DATA_JSON_VALUE& value,
		std::uint32_t& outValue)
	{
		return Try_ParseUnsigned(value, MAX_CLIP_TIME_MS, outValue) &&
			0u != outValue;
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

	std::string Build_SlotKey(
		const std::uint32_t sourceActionId,
		const std::string_view stageId,
		const std::string_view slotId)
	{
		return std::to_string(sourceActionId) + "\n" +
			std::string(stageId) + "\n" + std::string(slotId);
	}

	std::string Serialize_Authored(
		const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& document)
	{
		std::ostringstream output;
		output << std::setprecision(
			std::numeric_limits<f32_t>::max_digits10);
		output << "{\n"
			<< "  \"schema\": \"" << AUTHORED_SCHEMA << "\",\n"
			<< "  \"formatVersion\": 1,\n"
			<< "  \"authority\": \"" << REFERENCE_ONLY << "\",\n"
			<< "  \"profileId\": \""
			<< CDataJson::Escape(document.strProfileId) << "\",\n"
			<< "  \"referenceRevision\": \""
			<< CDataJson::Escape(document.strReferenceRevision) << "\",\n"
			<< "  \"bindings\": [\n";

		for (std::size_t index = 0u;
			index < document.Bindings.size(); ++index)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& binding =
				document.Bindings[index];
			output << "    {\n"
				<< "      \"sourceActionId\": " << binding.iSourceActionId << ",\n"
				<< "      \"stageId\": \""
				<< CDataJson::Escape(binding.strStageId) << "\",\n"
				<< "      \"slotId\": \""
				<< CDataJson::Escape(binding.strSlotId) << "\",\n"
				<< "      \"runtimeClip\": \""
				<< CDataJson::Escape(binding.strRuntimeClip) << "\",\n"
				<< "      \"sourceStartMs\": " << binding.iSourceStartMs << ",\n"
				<< "      \"playMs\": " << binding.iPlayMs << ",\n"
				<< "      \"playRate\": " << binding.fPlayRate << ",\n"
				<< "      \"loop\": "
				<< (binding.bLoop ? "true" : "false") << ",\n"
				<< "      \"mappingBasis\": \"" << PROJECT_AUTHORED << "\",\n"
				<< "      \"authority\": \"" << REFERENCE_ONLY << "\"\n"
				<< "    }"
				<< (index + 1u < document.Bindings.size() ? "," : "")
				<< "\n";
		}
		output << "  ]\n}\n";
		return output.str();
	}

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
				"KoukuSaydon reference") ||
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

	void Remove_Temporary(const std::filesystem::path& path)
	{
		std::error_code cleanupError;
		std::filesystem::remove(path, cleanupError);
	}
}

std::filesystem::path
Client::CKoukuSaydonAnimationActionDocument::Resolve_ReferencePath(
	const std::string_view profileId)
{
	if (!Is_StableToken(profileId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Reference/KoukuSaydon") /
		std::filesystem::path(
			std::string(profileId) + ".actionreference.json"));
}

std::filesystem::path
Client::CKoukuSaydonAnimationActionDocument::Resolve_AuthoredPath(
	const std::string_view profileId)
{
	if (!Is_StableToken(profileId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored/KoukuSaydon") /
		std::filesystem::path(
			std::string(profileId) + ".actionbindings.json"));
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Parse_ReferenceText(
	const std::string_view text,
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "profileId", "modelAssetId",
			  "sourceEvidenceSha256", "referenceRevision", "authority",
			  "actions" }))
	{
		outStatus = "KoukuSaydon action reference JSON is malformed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* profile = Required(
		root, "profileId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* model = Required(
		root, "modelAssetId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* sourceHash = Required(
		root, "sourceEvidenceSha256", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(
		root, "referenceRevision", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* authority = Required(
		root, "authority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* actions = Required(
		root, "actions", DATA_JSON_TYPE::ARRAY);
	std::uint32_t parsedVersion = 0u;
	if (nullptr == schema || schema->Get_String() != REFERENCE_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsigned(*version, DOCUMENT_VERSION, parsedVersion) ||
		parsedVersion != DOCUMENT_VERSION ||
		nullptr == profile || !Is_StableToken(profile->Get_String()) ||
		nullptr == model || !Is_SafeAssetId(model->Get_String()) ||
		nullptr == sourceHash || !Is_LowerHexSha256(sourceHash->Get_String()) ||
		nullptr == revision || !Is_LowerHexSha256(revision->Get_String()) ||
		nullptr == authority || authority->Get_String() != REFERENCE_ONLY ||
		nullptr == actions || actions->Get_Array().empty() ||
		actions->Get_Array().size() > MAX_ACTIONS)
	{
		outStatus = "KoukuSaydon action reference header is invalid.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT staged;
	staged.iFormatVersion = parsedVersion;
	staged.strProfileId = profile->Get_String();
	staged.strModelAssetId = model->Get_String();
	staged.strSourceEvidenceSha256 = sourceHash->Get_String();
	staged.strReferenceRevision = revision->Get_String();
	staged.strAuthority = authority->Get_String();
	staged.Actions.reserve(actions->Get_Array().size());

	for (const DATA_JSON_VALUE& actionValue : actions->Get_Array())
	{
		if (!Has_ExactProperties(actionValue,
				{ "sourceActionId", "displayName", "reviewStatus",
				  "authority", "stages" }))
		{
			outStatus = "KoukuSaydon action reference row has unexpected properties.";
			return false;
		}
		const DATA_JSON_VALUE* actionId = Required(
			actionValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* displayName = Required(
			actionValue, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* reviewStatus = Required(
			actionValue, "reviewStatus", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* actionAuthority = Required(
			actionValue, "authority", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* stages = Required(
			actionValue, "stages", DATA_JSON_TYPE::ARRAY);

		KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE stagedAction;
		if (nullptr == actionId ||
			!Try_ParseUnsigned(*actionId,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedAction.iSourceActionId) ||
			nullptr == displayName || !Is_DisplayName(displayName->Get_String()) ||
			nullptr == reviewStatus ||
			(reviewStatus->Get_String() != "REVIEW_CANDIDATE" &&
			 reviewStatus->Get_String() != "HOLDOUT") ||
			nullptr == actionAuthority ||
			actionAuthority->Get_String() != REFERENCE_ONLY ||
			nullptr == stages || stages->Get_Array().empty() ||
			stages->Get_Array().size() > MAX_STAGES_PER_ACTION)
		{
			outStatus = "KoukuSaydon action reference row is invalid.";
			return false;
		}
		stagedAction.strDisplayName = displayName->Get_String();
		stagedAction.strReviewStatus = reviewStatus->Get_String();
		stagedAction.strAuthority = actionAuthority->Get_String();
		stagedAction.Stages.reserve(stages->Get_Array().size());

		for (const DATA_JSON_VALUE& stageValue : stages->Get_Array())
		{
			if (!Has_ExactProperties(stageValue,
					{ "stageId", "stageOrdinal", "holdoutClipNames", "slots" }))
			{
				outStatus = "KoukuSaydon action stage has unexpected properties.";
				return false;
			}
			const DATA_JSON_VALUE* stageId = Required(
				stageValue, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* stageOrdinal = Required(
				stageValue, "stageOrdinal", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* holdouts = Required(
				stageValue, "holdoutClipNames", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* slots = Required(
				stageValue, "slots", DATA_JSON_TYPE::ARRAY);

			KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE stagedStage;
			if (nullptr == stageId || !Is_StableToken(stageId->Get_String()) ||
				nullptr == stageOrdinal ||
				!Try_ParseUnsigned(*stageOrdinal,
					static_cast<std::uint32_t>(MAX_STAGES_PER_ACTION - 1u),
					stagedStage.iStageOrdinal) ||
				nullptr == holdouts ||
				holdouts->Get_Array().size() > MAX_HOLDOUTS_PER_STAGE ||
				nullptr == slots || slots->Get_Array().size() > MAX_SLOTS_PER_STAGE)
			{
				outStatus = "KoukuSaydon action stage is invalid.";
				return false;
			}
			stagedStage.strStageId = stageId->Get_String();
			stagedStage.HoldoutClipNames.reserve(holdouts->Get_Array().size());
			for (const DATA_JSON_VALUE& holdout : holdouts->Get_Array())
			{
				if (!holdout.Is_String() || !Is_StableToken(holdout.Get_String()))
				{
					outStatus = "KoukuSaydon action holdout clip name is invalid.";
					return false;
				}
				stagedStage.HoldoutClipNames.push_back(holdout.Get_String());
			}

			stagedStage.Slots.reserve(slots->Get_Array().size());
			for (const DATA_JSON_VALUE& slotValue : slots->Get_Array())
			{
				if (!Has_ExactProperties(slotValue,
						{ "slotId", "extractedClip", "runtimeClip",
						  "sourceStartMs", "playMs", "playRate", "loop",
						  "mappingBasis", "authority" }))
				{
					outStatus = "KoukuSaydon action slot has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* slotId = Required(
					slotValue, "slotId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* extractedClip = Required(
					slotValue, "extractedClip", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* runtimeClip = Required(
					slotValue, "runtimeClip", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* sourceStartMs = Required(
					slotValue, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* playMs = Required(
					slotValue, "playMs", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* playRate = Required(
					slotValue, "playRate", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* loop = Required(
					slotValue, "loop", DATA_JSON_TYPE::BOOLEAN);
				const DATA_JSON_VALUE* mappingBasis = Required(
					slotValue, "mappingBasis", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* slotAuthority = Required(
					slotValue, "authority", DATA_JSON_TYPE::STRING);

				KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE stagedSlot;
				if (nullptr == slotId || !Is_StableToken(slotId->Get_String()) ||
					nullptr == extractedClip ||
					!Is_StableToken(extractedClip->Get_String()) ||
					nullptr == runtimeClip ||
					!Is_StableToken(runtimeClip->Get_String()) ||
					nullptr == sourceStartMs ||
					!Try_ParseUnsigned(*sourceStartMs, MAX_CLIP_TIME_MS,
						stagedSlot.iSourceStartMs) ||
					nullptr == playMs ||
					!Try_ParsePlayMs(*playMs, stagedSlot.iPlayMs) ||
					nullptr == playRate ||
					!Try_ParsePlayRate(*playRate, stagedSlot.fPlayRate) ||
					nullptr == loop || nullptr == mappingBasis ||
					mappingBasis->Get_String() != EXTRACTED_REFERENCE ||
					nullptr == slotAuthority ||
					slotAuthority->Get_String() != REFERENCE_ONLY)
				{
					outStatus = "KoukuSaydon action slot is invalid.";
					return false;
				}
				stagedSlot.strSlotId = slotId->Get_String();
				stagedSlot.strExtractedClip = extractedClip->Get_String();
				stagedSlot.strRuntimeClip = runtimeClip->Get_String();
				stagedSlot.bLoop = loop->Get_Boolean();
				stagedSlot.strMappingBasis = mappingBasis->Get_String();
				stagedSlot.strAuthority = slotAuthority->Get_String();
				stagedStage.Slots.push_back(std::move(stagedSlot));
			}
			stagedAction.Stages.push_back(std::move(stagedStage));
		}
		staged.Actions.push_back(std::move(stagedAction));
	}

	outDocument = std::move(staged);
	outStatus = "Parsed KoukuSaydon animation action reference.";
	return true;
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Parse_AuthoredText(
	const std::string_view text,
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "authority", "profileId",
			  "referenceRevision", "bindings" }))
	{
		outStatus = "KoukuSaydon authored action JSON is malformed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* authority = Required(
		root, "authority", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* profile = Required(
		root, "profileId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(
		root, "referenceRevision", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* bindings = Required(
		root, "bindings", DATA_JSON_TYPE::ARRAY);
	std::uint32_t parsedVersion = 0u;
	if (nullptr == schema || schema->Get_String() != AUTHORED_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsigned(*version, DOCUMENT_VERSION, parsedVersion) ||
		parsedVersion != DOCUMENT_VERSION ||
		nullptr == authority || authority->Get_String() != REFERENCE_ONLY ||
		nullptr == profile || !Is_StableToken(profile->Get_String()) ||
		nullptr == revision || !Is_LowerHexSha256(revision->Get_String()) ||
		nullptr == bindings || bindings->Get_Array().size() > MAX_AUTHORED_BINDINGS)
	{
		outStatus = "KoukuSaydon authored action header is invalid.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT staged;
	staged.iFormatVersion = parsedVersion;
	staged.strAuthority = authority->Get_String();
	staged.strProfileId = profile->Get_String();
	staged.strReferenceRevision = revision->Get_String();
	staged.Bindings.reserve(bindings->Get_Array().size());

	for (const DATA_JSON_VALUE& bindingValue : bindings->Get_Array())
	{
		if (!Has_ExactProperties(bindingValue,
				{ "sourceActionId", "stageId", "slotId", "runtimeClip",
				  "sourceStartMs", "playMs", "playRate", "loop",
				  "mappingBasis", "authority" }))
		{
			outStatus = "KoukuSaydon authored binding has unexpected properties.";
			return false;
		}

		const DATA_JSON_VALUE* actionId = Required(
			bindingValue, "sourceActionId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* stageId = Required(
			bindingValue, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* slotId = Required(
			bindingValue, "slotId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* runtimeClip = Required(
			bindingValue, "runtimeClip", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceStartMs = Required(
			bindingValue, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* playMs = Required(
			bindingValue, "playMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* playRate = Required(
			bindingValue, "playRate", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* loop = Required(
			bindingValue, "loop", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* mappingBasis = Required(
			bindingValue, "mappingBasis", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* bindingAuthority = Required(
			bindingValue, "authority", DATA_JSON_TYPE::STRING);

		KOUKU_SAYDON_ANIMATION_ACTION_BINDING stagedBinding;
		if (nullptr == actionId ||
			!Try_ParseUnsigned(*actionId,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedBinding.iSourceActionId) ||
			nullptr == stageId || !Is_StableToken(stageId->Get_String()) ||
			nullptr == slotId || !Is_StableToken(slotId->Get_String()) ||
			nullptr == runtimeClip ||
			!Is_StableToken(runtimeClip->Get_String()) ||
			nullptr == sourceStartMs ||
			!Try_ParseUnsigned(*sourceStartMs, MAX_CLIP_TIME_MS,
				stagedBinding.iSourceStartMs) ||
			nullptr == playMs || !Try_ParsePlayMs(*playMs, stagedBinding.iPlayMs) ||
			nullptr == playRate ||
			!Try_ParsePlayRate(*playRate, stagedBinding.fPlayRate) ||
			nullptr == loop || nullptr == mappingBasis ||
			mappingBasis->Get_String() != PROJECT_AUTHORED ||
			nullptr == bindingAuthority ||
			bindingAuthority->Get_String() != REFERENCE_ONLY)
		{
			outStatus = "KoukuSaydon authored binding is invalid.";
			return false;
		}
		stagedBinding.strStageId = stageId->Get_String();
		stagedBinding.strSlotId = slotId->Get_String();
		stagedBinding.strRuntimeClip = runtimeClip->Get_String();
		stagedBinding.bLoop = loop->Get_Boolean();
		stagedBinding.strMappingBasis = mappingBasis->Get_String();
		stagedBinding.strAuthority = bindingAuthority->Get_String();
		staged.Bindings.push_back(std::move(stagedBinding));
	}

	outDocument = std::move(staged);
	outStatus = "Parsed KoukuSaydon animation action overrides.";
	return true;
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Validate_Reference(
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& document,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Is_StableToken(expectedProfileId) ||
		!Is_SafeAssetId(expectedModelAssetId) ||
		document.iFormatVersion != DOCUMENT_VERSION ||
		document.strProfileId != expectedProfileId ||
		document.strModelAssetId != expectedModelAssetId ||
		!Is_LowerHexSha256(document.strSourceEvidenceSha256) ||
		!Is_LowerHexSha256(document.strReferenceRevision) ||
		document.strAuthority != REFERENCE_ONLY ||
		document.Actions.empty() || document.Actions.size() > MAX_ACTIONS)
	{
		outStatus = "KoukuSaydon reference profile, model, revision, or authority is invalid.";
		return false;
	}

	std::unordered_set<std::string> availableClipSet;
	availableClipSet.reserve(availableClips.size());
	for (const std::string& clip : availableClips)
		availableClipSet.insert(clip);

	std::unordered_set<std::uint32_t> actionIds;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action : document.Actions)
	{
		if (!actionIds.insert(action.iSourceActionId).second ||
			!Is_DisplayName(action.strDisplayName) ||
			(action.strReviewStatus != "REVIEW_CANDIDATE" &&
			 action.strReviewStatus != "HOLDOUT") ||
			action.strAuthority != REFERENCE_ONLY ||
			action.Stages.empty() ||
			action.Stages.size() > MAX_STAGES_PER_ACTION)
		{
			outStatus = "KoukuSaydon reference has a duplicate or invalid action.";
			return false;
		}

		bool_t hasHoldout = false;
		std::unordered_set<std::string> stageIds;
		for (std::size_t stageIndex = 0u;
			stageIndex < action.Stages.size(); ++stageIndex)
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage =
				action.Stages[stageIndex];
			if (!Is_StableToken(stage.strStageId) ||
				stage.iStageOrdinal != stageIndex ||
				!stageIds.insert(stage.strStageId).second ||
				stage.HoldoutClipNames.size() > MAX_HOLDOUTS_PER_STAGE ||
				stage.Slots.size() > MAX_SLOTS_PER_STAGE)
			{
				outStatus = "KoukuSaydon reference has an invalid or duplicate stage.";
				return false;
			}

			std::unordered_set<std::string> holdoutNames;
			for (const std::string& holdout : stage.HoldoutClipNames)
			{
				hasHoldout = true;
				if (!Is_StableToken(holdout) ||
					!holdoutNames.insert(holdout).second)
				{
					outStatus = "KoukuSaydon reference has an invalid holdout clip.";
					return false;
				}
			}

			std::unordered_set<std::string> slotIds;
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				if (!Is_StableToken(slot.strSlotId) ||
					!slotIds.insert(slot.strSlotId).second ||
					!Is_StableToken(slot.strExtractedClip) ||
					!Is_StableToken(slot.strRuntimeClip) ||
					slot.strMappingBasis != EXTRACTED_REFERENCE ||
					slot.strAuthority != REFERENCE_ONLY ||
					!Is_ValidTiming(slot.iSourceStartMs,
						slot.iPlayMs, slot.fPlayRate) ||
					!availableClipSet.contains(slot.strRuntimeClip))
				{
					outStatus = "KoukuSaydon reference slot does not resolve to the target model.";
					return false;
				}
			}
		}

		if (hasHoldout != (action.strReviewStatus == "HOLDOUT"))
		{
			outStatus = "KoukuSaydon action review status does not match its holdouts.";
			return false;
		}
	}

	outStatus = "Validated " + std::to_string(document.Actions.size()) +
		" KoukuSaydon animation action reference(s).";
	return true;
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Validate_Authored(
	const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Validate_Reference(reference, expectedProfileId,
		expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}
	if (document.iFormatVersion != DOCUMENT_VERSION ||
		document.strProfileId != expectedProfileId ||
		document.strReferenceRevision != reference.strReferenceRevision ||
		document.strAuthority != REFERENCE_ONLY ||
		document.Bindings.size() > MAX_AUTHORED_BINDINGS)
	{
		outStatus = "KoukuSaydon authored profile, reference revision, or authority is stale.";
		return false;
	}

	std::unordered_set<std::string> referenceSlots;
	for (const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action : reference.Actions)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage : action.Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot : stage.Slots)
			{
				referenceSlots.insert(Build_SlotKey(
					action.iSourceActionId, stage.strStageId, slot.strSlotId));
			}
		}
	}
	std::unordered_set<std::string> availableClipSet(
		availableClips.begin(), availableClips.end());
	std::unordered_set<std::string> claimedSlots;
	claimedSlots.reserve(document.Bindings.size());
	for (const KOUKU_SAYDON_ANIMATION_ACTION_BINDING& binding : document.Bindings)
	{
		const std::string key = Build_SlotKey(binding.iSourceActionId,
			binding.strStageId, binding.strSlotId);
		if (!Is_StableToken(binding.strStageId) ||
			!Is_StableToken(binding.strSlotId) ||
			!referenceSlots.contains(key) ||
			!claimedSlots.insert(key).second ||
			!Is_StableToken(binding.strRuntimeClip) ||
			!availableClipSet.contains(binding.strRuntimeClip) ||
			!Is_ValidTiming(binding.iSourceStartMs,
				binding.iPlayMs, binding.fPlayRate) ||
			binding.strMappingBasis != PROJECT_AUTHORED ||
			binding.strAuthority != REFERENCE_ONLY)
		{
			outStatus =
				"KoukuSaydon authored override is duplicate, stale, or missing from the target model.";
			return false;
		}
	}

	outStatus = "Validated " + std::to_string(document.Bindings.size()) +
		" sparse KoukuSaydon animation action override(s).";
	return true;
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Load_FromPaths(
	const std::filesystem::path& referencePath,
	const std::filesystem::path& authoredPath,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
	std::string& outStatus)
{
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT stagedReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, stagedReference, outStatus))
	{
		return false;
	}

	std::string authoredText;
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT stagedAuthored;
	if (!Read_Text(authoredPath, MAX_AUTHORED_BYTES, authoredText,
			outStatus, "KoukuSaydon authored") ||
		!Parse_AuthoredText(authoredText, stagedAuthored, outStatus) ||
		!Validate_Authored(stagedAuthored, stagedReference,
			expectedProfileId, expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}

	outReference = std::move(stagedReference);
	outAuthored = std::move(stagedAuthored);
	outStatus = "Loaded KoukuSaydon animation action reference and sparse overrides.";
	return true;
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Load(
	const std::string_view profileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
	std::string& outStatus)
{
	const std::filesystem::path referencePath = Resolve_ReferencePath(profileId);
	const std::filesystem::path authoredPath = Resolve_AuthoredPath(profileId);
	if (referencePath.empty() || authoredPath.empty())
	{
		outStatus = "KoukuSaydon animation action profile path is invalid.";
		return false;
	}
	return Load_FromPaths(referencePath, authoredPath, profileId,
		expectedModelAssetId, availableClips, outReference, outAuthored,
		outStatus);
}

bool_t Client::CKoukuSaydonAnimationActionDocument::Save_Atomic(
	const KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
	const std::string_view expectedProfileId,
	const std::string_view expectedModelAssetId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Validate_Authored(document, reference, expectedProfileId,
		expectedModelAssetId, availableClips, outStatus))
	{
		return false;
	}

	const std::filesystem::path referencePath =
		Resolve_ReferencePath(expectedProfileId);
	const std::filesystem::path destination =
		Resolve_AuthoredPath(expectedProfileId);
	if (referencePath.empty() || destination.empty())
	{
		outStatus = "KoukuSaydon authored destination is invalid.";
		return false;
	}

	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT currentReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, currentReference, outStatus))
	{
		return false;
	}
	if (currentReference != reference)
	{
		outStatus =
			"KoukuSaydon reference changed since the authored document was loaded.";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create the KoukuSaydon authored directory.";
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetCurrentThreadId()) +
		L"." + std::to_wstring(GetTickCount64());
	const std::string serialized = Serialize_Authored(document);
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open the temporary KoukuSaydon authored document.";
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
			"Could not durably write the temporary KoukuSaydon authored document.";
		return false;
	}

	std::string verificationText;
	KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT reparsed;
	std::string verificationStatus;
	if (!Read_Text(temporary, MAX_AUTHORED_BYTES, verificationText,
			verificationStatus, "Temporary KoukuSaydon authored") ||
		!Parse_AuthoredText(verificationText, reparsed, verificationStatus) ||
		!Validate_Authored(reparsed, currentReference, expectedProfileId,
			expectedModelAssetId, availableClips, verificationStatus) ||
		reparsed != document)
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon authored temp verification failed: " +
			verificationStatus;
		return false;
	}

	KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT finalReference;
	if (!Load_ReferenceFromPath(referencePath, expectedProfileId,
		expectedModelAssetId, availableClips, finalReference,
		verificationStatus))
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon reference revalidation failed during authored save: " +
			verificationStatus;
		return false;
	}
	if (finalReference != currentReference)
	{
		Remove_Temporary(temporary);
		outStatus = "KoukuSaydon reference changed during authored save.";
		return false;
	}

	if (!MoveFileExW(temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_Temporary(temporary);
		outStatus = "Could not atomically replace the KoukuSaydon authored document.";
		return false;
	}
	outStatus = "Saved " + std::to_string(document.Bindings.size()) +
		" sparse KoukuSaydon animation action override(s) to " +
		destination.string();
	return true;
}
