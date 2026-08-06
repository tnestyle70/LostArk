#include "AnimationSkillBindingDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <io.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;
	using namespace LostArk::Shared;

	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.animation-skill-bindings";
	constexpr double DOCUMENT_VERSION = 2.0;
	constexpr std::size_t MAX_BINDINGS = 64u;
	constexpr std::size_t MAX_CLIPS_PER_BINDING = 16u;
	constexpr std::uint32_t MAX_CLIP_PLAY_MS = 60000u;

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
		return true;
	}

	CHARACTER_CLASS_ID Parse_CharacterClass(const std::string_view value)
	{
		if ("LANCE_MASTER" == value) return CHARACTER_CLASS_ID::LANCE_MASTER;
		if ("GUNSLINGER" == value) return CHARACTER_CLASS_ID::GUNSLINGER;
		if ("SLAYER" == value) return CHARACTER_CLASS_ID::SLAYER;
		if ("ARTIST" == value) return CHARACTER_CLASS_ID::ARTIST;
		if ("DIMENSIONMASTER" == value) return CHARACTER_CLASS_ID::DIMENSIONMASTER;
		if ("WARLORD" == value) return CHARACTER_CLASS_ID::WARLORD;
		return CHARACTER_CLASS_ID::END;
	}

	const char* CharacterClass_Name(const CHARACTER_CLASS_ID value)
	{
		switch (value)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LANCE_MASTER";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "GUNSLINGER";
		case CHARACTER_CLASS_ID::SLAYER: return "SLAYER";
		case CHARACTER_CLASS_ID::ARTIST: return "ARTIST";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DIMENSIONMASTER";
		case CHARACTER_CLASS_ID::WARLORD: return "WARLORD";
		default: return nullptr;
		}
	}

	bool Try_ParseSkillId(const DATA_JSON_VALUE& value, SKILL_ID& outSkillId)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number <= 0.0 ||
			number > static_cast<double>((std::numeric_limits<SKILL_ID>::max)()) ||
			std::floor(number) != number)
		{
			return false;
		}
		outSkillId = static_cast<SKILL_ID>(number);
		return INVALID_SKILL_ID != outSkillId;
	}

	bool Has_OnlyKnownProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().empty())
			return false;
		for (const auto& property : object.Get_Object())
		{
			if (names.end() == std::find(
				names.begin(), names.end(), std::string_view(property.first)))
			{
				return false;
			}
		}
		return true;
	}

	bool Try_ParsePlayRate(const DATA_JSON_VALUE& value, f32_t& outPlayRate)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.05 || number > 16.0)
			return false;
		outPlayRate = static_cast<f32_t>(number);
		return true;
	}

	bool Try_ParsePlayMs(const DATA_JSON_VALUE& value, std::uint32_t& outPlayMs)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number <= 0.0 ||
			number > static_cast<double>(MAX_CLIP_PLAY_MS) ||
			std::floor(number) != number)
		{
			return false;
		}
		outPlayMs = static_cast<std::uint32_t>(number);
		return true;
	}

	std::string Serialize(
		const ANIMATION_SKILL_BINDING_DOCUMENT& document)
	{
		std::ostringstream output;
		output << "{\n"
			<< "  \"schema\": \"" << DOCUMENT_SCHEMA << "\",\n"
			<< "  \"formatVersion\": 2,\n"
			<< "  \"animationAssetId\": \""
			<< CDataJson::Escape(document.strAnimationAssetId) << "\",\n"
			<< "  \"characterClass\": \""
			<< CharacterClass_Name(document.eCharacterClass) << "\",\n"
			<< "  \"bindings\": [\n";

		for (std::size_t bindingIndex = 0;
			bindingIndex < document.Bindings.size();
			++bindingIndex)
		{
			const ANIMATION_SKILL_BINDING& binding =
				document.Bindings[bindingIndex];
			output << "    {\n"
				<< "      \"skillId\": " << binding.iSkillId << ",\n"
				<< "      \"clips\": [";
			for (std::size_t clipIndex = 0;
				clipIndex < binding.Clips.size();
				++clipIndex)
			{
				if (clipIndex > 0u)
					output << ", ";
				const ANIMATION_SKILL_CLIP& clip = binding.Clips[clipIndex];
				if (0u == clip.iPlayMs && 1.f == clip.fPlayRate)
				{
					output << '"' << CDataJson::Escape(clip.strClipName) << '"';
					continue;
				}
				output << "{ \"clip\": \""
					<< CDataJson::Escape(clip.strClipName) << '"';
				if (0u != clip.iPlayMs)
					output << ", \"playMs\": " << clip.iPlayMs;
				if (1.f != clip.fPlayRate)
					output << ", \"playRate\": " << clip.fPlayRate;
				output << " }";
			}
			output << "]\n    }"
				<< (bindingIndex + 1u < document.Bindings.size() ? "," : "")
				<< "\n";
		}
		output << "  ]\n}\n";
		return output.str();
	}

	bool Documents_AreEqual(
		const ANIMATION_SKILL_BINDING_DOCUMENT& left,
		const ANIMATION_SKILL_BINDING_DOCUMENT& right)
	{
		if (left.strAnimationAssetId != right.strAnimationAssetId ||
			left.eCharacterClass != right.eCharacterClass ||
			left.Bindings.size() != right.Bindings.size())
		{
			return false;
		}
		for (std::size_t index = 0; index < left.Bindings.size(); ++index)
		{
			const ANIMATION_SKILL_BINDING& a = left.Bindings[index];
			const ANIMATION_SKILL_BINDING& b = right.Bindings[index];
			if (a.iSkillId != b.iSkillId ||
				a.Clips != b.Clips)
			{
				return false;
			}
		}
		return true;
	}
}

std::filesystem::path Client::CAnimationSkillBindingDocument::Resolve_Path(
	const std::string_view animationAssetId)
{
	if (!Is_StableToken(animationAssetId))
		return {};
	const std::string asset{ animationAssetId };
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored") /
		std::filesystem::path(asset) /
		std::filesystem::path(asset + ".skillbindings.json"));
}

bool_t Client::CAnimationSkillBindingDocument::Parse_Text(
	const std::string_view text,
	ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "animationAssetId",
				"characterClass", "bindings" }))
	{
		outStatus = "Skill binding JSON is malformed: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* asset = Required(
		root, "animationAssetId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* characterClass = Required(
		root, "characterClass", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* bindings = Required(
		root, "bindings", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || schema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == version || version->Get_Number() != DOCUMENT_VERSION ||
		nullptr == asset || !Is_StableToken(asset->Get_String()) ||
		nullptr == characterClass || nullptr == bindings ||
		bindings->Get_Array().empty() ||
		bindings->Get_Array().size() > MAX_BINDINGS)
	{
		outStatus = "Skill binding header is invalid.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	staged.strAnimationAssetId = asset->Get_String();
	staged.eCharacterClass = Parse_CharacterClass(
		characterClass->Get_String());
	if (CHARACTER_CLASS_ID::END == staged.eCharacterClass)
	{
		outStatus = "Skill binding characterClass is unknown.";
		return false;
	}

	for (const DATA_JSON_VALUE& value : bindings->Get_Array())
	{
		if (!Has_ExactProperties(value, { "skillId", "clips" }))
		{
			outStatus = "Skill binding row has an unexpected field set.";
			return false;
		}
		const DATA_JSON_VALUE* skillId = Required(
			value, "skillId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* clips = Required(
			value, "clips", DATA_JSON_TYPE::ARRAY);
		ANIMATION_SKILL_BINDING binding;
		if (nullptr == skillId || !Try_ParseSkillId(*skillId, binding.iSkillId) ||
			nullptr == clips ||
			clips->Get_Array().empty() ||
			clips->Get_Array().size() > MAX_CLIPS_PER_BINDING)
		{
			outStatus = "Skill binding row is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& clip : clips->Get_Array())
		{
			ANIMATION_SKILL_CLIP stagedClip;
			if (clip.Is_String())
			{
				stagedClip.strClipName = clip.Get_String();
			}
			else if (Has_OnlyKnownProperties(
				clip, { "clip", "playMs", "playRate" }))
			{
				const DATA_JSON_VALUE* clipName = Required(
					clip, "clip", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* playMs = clip.Find("playMs");
				const DATA_JSON_VALUE* playRate = clip.Find("playRate");
				if (nullptr == clipName ||
					(nullptr == playMs && nullptr == playRate) ||
					(nullptr != playMs &&
						!Try_ParsePlayMs(*playMs, stagedClip.iPlayMs)) ||
					(nullptr != playRate &&
						!Try_ParsePlayRate(*playRate, stagedClip.fPlayRate)))
				{
					outStatus = "Skill binding clip play length is invalid.";
					return false;
				}
				stagedClip.strClipName = clipName->Get_String();
			}
			else
			{
				outStatus = "Skill binding clip row has an unexpected field set.";
				return false;
			}
			if (!Is_StableToken(stagedClip.strClipName))
			{
				outStatus = "Skill binding clip name is invalid.";
				return false;
			}
			binding.Clips.push_back(std::move(stagedClip));
		}
		staged.Bindings.push_back(std::move(binding));
	}

	outDocument = std::move(staged);
	outStatus = "Parsed skill animation bindings.";
	return true;
}

bool_t Client::CAnimationSkillBindingDocument::Validate(
	const ANIMATION_SKILL_BINDING_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
	const std::vector<PLAYER_SKILL_DEFINITION>& skills,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	if (!Is_StableToken(document.strAnimationAssetId) ||
		document.strAnimationAssetId != expectedAnimationAssetId ||
		document.eCharacterClass != expectedCharacterClass ||
		document.Bindings.empty() || document.Bindings.size() > MAX_BINDINGS)
	{
		outStatus = "Skill binding owner does not match the target Character.";
		return false;
	}

	std::unordered_set<SKILL_ID> claimedSkills;
	std::size_t expectedBindingCount = 0u;
	for (const PLAYER_SKILL_DEFINITION& skill : skills)
	{
		if (skill.eCharacterClass == expectedCharacterClass)
			++expectedBindingCount;
	}
	if (0u == expectedBindingCount ||
		document.Bindings.size() != expectedBindingCount)
	{
		outStatus = "Skill binding document is incomplete for this class.";
		return false;
	}

	for (const ANIMATION_SKILL_BINDING& binding : document.Bindings)
	{
		const PLAYER_SKILL_DEFINITION* definition = nullptr;
		for (const PLAYER_SKILL_DEFINITION& skill : skills)
		{
			if (skill.iSkillId == binding.iSkillId)
			{
				definition = &skill;
				break;
			}
		}
		if (nullptr == definition ||
			definition->eCharacterClass != expectedCharacterClass ||
			!claimedSkills.insert(binding.iSkillId).second)
		{
			outStatus =
				"Skill binding does not match the PlayerSkills class.";
			return false;
		}
		if (binding.Clips.empty() ||
			binding.Clips.size() > MAX_CLIPS_PER_BINDING ||
			(PLAYER_SKILL_KIND::COMBO == definition->eSkillKind &&
				binding.Clips.size() != definition->iComboStageCount))
		{
			outStatus =
				"COMBO clip count must match the Server-owned combo stage count.";
			return false;
		}
		for (const ANIMATION_SKILL_CLIP& clip : binding.Clips)
		{
			if (!Is_StableToken(clip.strClipName) ||
				clip.iPlayMs > MAX_CLIP_PLAY_MS ||
				!std::isfinite(clip.fPlayRate) ||
				clip.fPlayRate < 0.05f || clip.fPlayRate > 16.f ||
				availableClips.end() == std::find(
					availableClips.begin(), availableClips.end(),
					clip.strClipName))
			{
				outStatus = "Skill binding references a clip missing from the target model.";
				return false;
			}
		}
	}

	outStatus = "Validated " + std::to_string(document.Bindings.size()) +
		" skill animation binding(s).";
	return true;
}

bool_t Client::CAnimationSkillBindingDocument::Load_FromPath(
	const std::filesystem::path& path,
	const std::string_view expectedAnimationAssetId,
	const LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
	const std::vector<PLAYER_SKILL_DEFINITION>& skills,
	const std::vector<std::string>& availableClips,
	ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input)
	{
		outStatus = "Skill binding document is missing: " + path.string();
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	if (!Parse_Text(text, staged, outStatus) ||
		!Validate(staged, expectedAnimationAssetId, expectedCharacterClass,
			skills, availableClips, outStatus))
	{
		return false;
	}
	outDocument = std::move(staged);
	return true;
}

bool_t Client::CAnimationSkillBindingDocument::Load(
	const std::string_view animationAssetId,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::vector<PLAYER_SKILL_DEFINITION>& skills,
	const std::vector<std::string>& availableClips,
	ANIMATION_SKILL_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	return Load_FromPath(
		Resolve_Path(animationAssetId), animationAssetId, characterClass,
		skills, availableClips, outDocument, outStatus);
}

bool_t Client::CAnimationSkillBindingDocument::Save_Atomic(
	const ANIMATION_SKILL_BINDING_DOCUMENT& document,
	const std::string_view expectedAnimationAssetId,
	const LostArk::Shared::CHARACTER_CLASS_ID expectedCharacterClass,
	const std::vector<PLAYER_SKILL_DEFINITION>& skills,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Validate(document, expectedAnimationAssetId,
		expectedCharacterClass, skills, availableClips, outStatus))
	{
		return false;
	}
	const std::filesystem::path destination = Resolve_Path(
		expectedAnimationAssetId);
	if (destination.empty())
	{
		outStatus = "Skill binding destination is invalid.";
		return false;
	}
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create the skill binding authoring directory.";
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetTickCount64());
	const std::string serialized = Serialize(document);
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open the temporary skill binding document.";
		return false;
	}
	const bool_t wrote = serialized.size() == fwrite(
		serialized.data(), 1u, serialized.size(), file);
	const bool_t flushed = 0 == fflush(file) && 0 == _commit(_fileno(file));
	const bool_t closed = 0 == fclose(file);
	if (!wrote || !flushed || !closed)
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		outStatus = "Could not durably write the temporary skill binding document.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT reparsed;
	std::string verifyStatus;
	if (!Load_FromPath(temporary, expectedAnimationAssetId,
		expectedCharacterClass, skills, availableClips,
		reparsed, verifyStatus) ||
		!Documents_AreEqual(document, reparsed))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		outStatus = "Skill binding temp verification failed: " + verifyStatus;
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		outStatus = "Could not atomically replace the skill binding document.";
		return false;
	}
	outStatus = "Saved " + std::to_string(document.Bindings.size()) +
		" skill animation binding(s) to " + destination.string();
	return true;
}
