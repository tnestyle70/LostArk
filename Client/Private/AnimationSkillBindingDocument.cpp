#include "AnimationSkillBindingDocument.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
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
	constexpr double DOCUMENT_VERSION = 3.0;
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

	void Serialize_Clip(
		std::ostringstream& output,
		const ANIMATION_SKILL_CLIP& clip)
	{
		if (0u == clip.iPlayMs && 1.f == clip.fPlayRate)
		{
			output << '"' << CDataJson::Escape(clip.strClipName) << '"';
			return;
		}
		output << "{ \"clip\": \""
			<< CDataJson::Escape(clip.strClipName) << '"';
		if (0u != clip.iPlayMs)
			output << ", \"playMs\": " << clip.iPlayMs;
		if (1.f != clip.fPlayRate)
			output << ", \"playRate\": " << clip.fPlayRate;
		output << " }";
	}

	std::string Serialize(
		const ANIMATION_SKILL_BINDING_DOCUMENT& document)
	{
		std::ostringstream output;
		output << "{\n"
			<< "  \"schema\": \"" << DOCUMENT_SCHEMA << "\",\n"
			<< "  \"formatVersion\": 3,\n"
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
			const bool_t isNested = 1u != binding.Stages.size();
			output << "    {\n"
				<< "      \"skillId\": " << binding.iSkillId << ",\n"
				<< "      \"clips\": [";
			for (std::size_t stageIndex = 0;
				stageIndex < binding.Stages.size();
				++stageIndex)
			{
				if (stageIndex > 0u)
					output << ", ";
				const ANIMATION_SKILL_STAGE& stage = binding.Stages[stageIndex];
				if (isNested)
					output << '[';
				for (std::size_t clipIndex = 0;
					clipIndex < stage.Clips.size();
					++clipIndex)
				{
					if (clipIndex > 0u)
						output << ", ";
					Serialize_Clip(output, stage.Clips[clipIndex]);
				}
				if (isNested)
					output << ']';
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
				a.Stages != b.Stages)
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
		/* A flat array is one stage, a nested one is a stage per element. Mixing
		the two shapes in a single binding leaves the stage count ambiguous. */
		const bool_t isNested = clips->Get_Array().front().Is_Array();
		std::size_t totalClips = 0u;
		for (const DATA_JSON_VALUE& element : clips->Get_Array())
		{
			if (element.Is_Array() != isNested)
			{
				outStatus = "Skill binding mixes staged and flat clip rows.";
				return false;
			}
			ANIMATION_SKILL_STAGE stagedStage;
			const DATA_JSON_VALUE::ARRAY single{};
			const DATA_JSON_VALUE::ARRAY& stageClips =
				isNested ? element.Get_Array() : single;
			if (isNested &&
				(stageClips.empty() || stageClips.size() > MAX_CLIPS_PER_BINDING))
			{
				outStatus = "Skill binding stage is empty or too long.";
				return false;
			}
			const std::size_t stageClipCount =
				isNested ? stageClips.size() : 1u;
			for (std::size_t index = 0; index < stageClipCount; ++index)
			{
				const DATA_JSON_VALUE& clip =
					isNested ? stageClips[index] : element;
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
				stagedStage.Clips.push_back(std::move(stagedClip));
			}
			totalClips += stagedStage.Clips.size();
			if (totalClips > MAX_CLIPS_PER_BINDING)
			{
				outStatus = "Skill binding row is invalid.";
				return false;
			}
			if (isNested)
			{
				binding.Stages.push_back(std::move(stagedStage));
				continue;
			}
			if (binding.Stages.empty())
				binding.Stages.emplace_back();
			binding.Stages.front().Clips.push_back(
				std::move(stagedStage.Clips.front()));
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
		/* A staged skill owns one stage per Server combo stage; everything else
		owns exactly one stage holding its whole chain. */
		const bool_t isStaged =
			PLAYER_SKILL_KIND::COMBO == definition->eSkillKind ||
			PLAYER_SKILL_KIND::HOLD == definition->eSkillKind ||
			PLAYER_SKILL_KIND::COUNTER == definition->eSkillKind;
		std::size_t totalClips = 0u;
		for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
			totalClips += stage.Clips.size();
		if (binding.Stages.empty() || 0u == totalClips ||
			totalClips > MAX_CLIPS_PER_BINDING ||
			(isStaged &&
				binding.Stages.size() != definition->iComboStageCount) ||
			(!isStaged && 1u != binding.Stages.size()))
		{
			outStatus =
				"COMBO clip count must match the Server-owned combo stage count.";
			return false;
		}
		for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
		{
			if (stage.Clips.empty())
			{
				outStatus = "Skill binding stage binds no clip.";
				return false;
			}
			for (const ANIMATION_SKILL_CLIP& clip : stage.Clips)
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

std::filesystem::path
Client::CValtanPatternAnimationBindingDocument::Resolve_Path(
	const std::string_view animationAssetId)
{
	if (!Is_StableToken(animationAssetId))
		return {};
	const std::string asset{ animationAssetId };
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored") /
		std::filesystem::path(asset) /
		std::filesystem::path(asset + ".patternbindings.json"));
}

bool_t Client::CValtanPatternAnimationBindingDocument::Parse_Text(
	const std::string_view text,
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	constexpr std::string_view BOSS_DOCUMENT_SCHEMA =
		"lostark.valtan-pattern-bindings";
	constexpr double BOSS_DOCUMENT_VERSION = 1.0;
	constexpr std::size_t MAX_BOSS_PATTERN_BINDINGS = 512u;

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "bossArchetypeId", "bindings" }))
	{
		outStatus = "Boss pattern binding JSON is malformed: " + parseError;
		return false;
	}
	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* boss = Required(
		root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* bindings = Required(
		root, "bindings", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || schema->Get_String() != BOSS_DOCUMENT_SCHEMA ||
		nullptr == version || version->Get_Number() != BOSS_DOCUMENT_VERSION ||
		nullptr == boss || !Is_StableToken(boss->Get_String()) ||
		nullptr == bindings || bindings->Get_Array().empty() ||
		bindings->Get_Array().size() > MAX_BOSS_PATTERN_BINDINGS)
	{
		outStatus = "Boss pattern binding header is invalid.";
		return false;
	}

	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT staged;
	staged.strBossArchetypeId = boss->Get_String();
	for (const DATA_JSON_VALUE& value : bindings->Get_Array())
	{
		if (!Has_ExactProperties(value, { "actionId", "clip" }))
		{
			outStatus = "Boss pattern binding row has an unexpected field set.";
			return false;
		}
		const DATA_JSON_VALUE* actionId = Required(
			value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* clip = Required(
			value, "clip", DATA_JSON_TYPE::STRING);
		if (nullptr == actionId || !Is_StableToken(actionId->Get_String()) ||
			nullptr == clip || !Is_StableToken(clip->Get_String()))
		{
			outStatus = "Boss pattern binding row is invalid.";
			return false;
		}
		staged.Bindings.push_back({
			actionId->Get_String(), clip->Get_String() });
	}

	outDocument = std::move(staged);
	outStatus = "Parsed boss pattern animation bindings.";
	return true;
}

bool_t Client::CValtanPatternAnimationBindingDocument::Validate(
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Is_StableToken(expectedBossArchetypeId) ||
		document.strBossArchetypeId != expectedBossArchetypeId ||
		document.Bindings.empty() || document.Bindings.size() > 512u)
	{
		outStatus = "Boss pattern binding owner does not match the target boss.";
		return false;
	}
	std::unordered_set<std::string> claimedActions;
	for (const BOSS_PATTERN_ANIMATION_BINDING& binding : document.Bindings)
	{
		if (!Is_StableToken(binding.strActionId) ||
			!Is_StableToken(binding.strClipName) ||
			!claimedActions.insert(binding.strActionId).second ||
			availableClips.end() == std::find(
				availableClips.begin(), availableClips.end(),
				binding.strClipName))
		{
			outStatus =
				"Boss pattern binding has a duplicate action or missing model clip.";
			return false;
		}
	}
	outStatus = "Validated " + std::to_string(document.Bindings.size()) +
		" boss pattern animation binding(s).";
	return true;
}

bool_t Client::CValtanPatternAnimationBindingDocument::Load(
	const std::string_view animationAssetId,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_Path(animationAssetId);
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input)
	{
		outStatus = "Boss pattern binding document is missing: " + path.string();
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT staged;
	if (!Parse_Text(text, staged, outStatus) ||
		!Validate(staged, expectedBossArchetypeId, availableClips, outStatus))
	{
		return false;
	}
	outDocument = std::move(staged);
	return true;
}

std::filesystem::path
Client::CValtanPatternEffectBindingDocument::Resolve_Path(
	const std::string_view animationAssetId)
{
	if (!Is_StableToken(animationAssetId))
		return {};
	const std::string asset{ animationAssetId };
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation/Authored") /
		std::filesystem::path(asset) /
		std::filesystem::path(asset + ".patterneffects.json"));
}

bool_t Client::CValtanPatternEffectBindingDocument::Parse_Text(
	const std::string_view text,
	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	constexpr std::string_view EFFECT_DOCUMENT_SCHEMA =
		"lostark.boss-pattern-effects";
	constexpr double EFFECT_DOCUMENT_VERSION = 1.0;
	constexpr std::size_t MAX_EFFECT_BINDINGS = 512u;

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "bossArchetypeId",
			  "gameplayAuthority", "bindings" }))
	{
		outStatus = "Boss pattern Effect JSON is malformed: " + parseError;
		return false;
	}
	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* boss = Required(
		root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* gameplayAuthority = Required(
		root, "gameplayAuthority", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* bindings = Required(
		root, "bindings", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || schema->Get_String() != EFFECT_DOCUMENT_SCHEMA ||
		nullptr == version || version->Get_Number() != EFFECT_DOCUMENT_VERSION ||
		nullptr == boss || !Is_StableToken(boss->Get_String()) ||
		nullptr == gameplayAuthority || nullptr == bindings ||
		bindings->Get_Array().empty() ||
		bindings->Get_Array().size() > MAX_EFFECT_BINDINGS)
	{
		outStatus = "Boss pattern Effect header is invalid.";
		return false;
	}

	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT staged;
	staged.strBossArchetypeId = boss->Get_String();
	for (const DATA_JSON_VALUE& value : bindings->Get_Array())
	{
		if (!Has_ExactProperties(value,
				{ "bindingId", "patternId", "semanticStageId", "actionId",
				  "effectAssetId", "effectDocument",
				  "animationBindingDocument", "sourceCatalogDocument",
				  "sourceParticleResourceCatalogDocument", "sourceEvidence",
				  "sourceBranch", "modelBoneEvidence", "sourceOccurrences",
				  "failClosedOccurrences", "productAdmission" }))
		{
			outStatus =
				"Boss pattern Effect row has an unexpected field set.";
			return false;
		}
		const DATA_JSON_VALUE* bindingId = Required(
			value, "bindingId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* patternId = Required(
			value, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* semanticStageId = Required(
			value, "semanticStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* actionId = Required(
			value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* effectAssetId = Required(
			value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* effectDocument = Required(
			value, "effectDocument", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceParticleResourceCatalogDocument = Required(
			value, "sourceParticleResourceCatalogDocument",
			DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceEvidence = Required(
			value, "sourceEvidence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* sourceBranch = Required(
			value, "sourceBranch", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* boneEvidence = Required(
			value, "modelBoneEvidence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* productAdmission = Required(
			value, "productAdmission", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* runtimeClip = nullptr == sourceBranch ? nullptr :
			Required(*sourceBranch, "runtimeClipName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* runtimeBone = nullptr == boneEvidence ? nullptr :
			Required(*boneEvidence, "runtimeBoneName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* boneAdmission = nullptr == boneEvidence ? nullptr :
			Required(*boneEvidence, "admission", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* admissionStatus =
			nullptr == productAdmission ? nullptr :
			Required(*productAdmission, "status", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* catalogMapped =
			nullptr == productAdmission ? nullptr :
			Required(*productAdmission, "productCatalogMapped",
				DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* animationMapped =
			nullptr == productAdmission ? nullptr :
			Required(*productAdmission, "animationEventMapped",
				DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == bindingId || !Is_StableToken(bindingId->Get_String()) ||
			nullptr == patternId || !Is_StableToken(patternId->Get_String()) ||
			nullptr == semanticStageId ||
			!Is_StableToken(semanticStageId->Get_String()) ||
			nullptr == actionId || !Is_StableToken(actionId->Get_String()) ||
			nullptr == effectAssetId ||
			!Is_StableToken(effectAssetId->Get_String()) ||
			nullptr == effectDocument || effectDocument->Get_String().empty() ||
			nullptr == sourceParticleResourceCatalogDocument ||
			sourceParticleResourceCatalogDocument->Get_String().empty() ||
			nullptr == sourceEvidence ||
			nullptr == runtimeClip ||
			!Is_StableToken(runtimeClip->Get_String()) ||
			nullptr == runtimeBone ||
			!Is_StableToken(runtimeBone->Get_String()) ||
			nullptr == boneAdmission ||
			boneAdmission->Get_String() !=
				"ADMITTED_EXPLICIT_RUNTIME_BONE" ||
			nullptr == admissionStatus ||
			!Is_StableToken(admissionStatus->Get_String()) ||
			nullptr == catalogMapped || nullptr == animationMapped)
		{
			outStatus = "Boss pattern Effect row is invalid.";
			return false;
		}
		staged.Bindings.push_back({
			bindingId->Get_String(), patternId->Get_String(),
			semanticStageId->Get_String(), actionId->Get_String(),
			effectAssetId->Get_String(), effectDocument->Get_String(),
			runtimeClip->Get_String(), runtimeBone->Get_String(),
			admissionStatus->Get_String(), catalogMapped->Get_Boolean(),
			animationMapped->Get_Boolean() });
	}

	outDocument = std::move(staged);
	outStatus = "Parsed boss pattern Effect bindings.";
	return true;
}

bool_t Client::CValtanPatternEffectBindingDocument::Validate(
	const BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& document,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	std::string& outStatus)
{
	if (!Is_StableToken(expectedBossArchetypeId) ||
		document.strBossArchetypeId != expectedBossArchetypeId ||
		document.Bindings.empty() || document.Bindings.size() > 512u)
	{
		outStatus =
			"Boss pattern Effect owner does not match the target boss.";
		return false;
	}
	std::unordered_set<std::string> bindingIds;
	std::unordered_set<std::string> actionKeys;
	std::unordered_set<std::string> effectAssets;
	for (const BOSS_PATTERN_EFFECT_BINDING& binding : document.Bindings)
	{
		const bool_t bProductAdmitted =
			binding.strProductAdmissionStatus == "ADMITTED_PRODUCT";
		const std::filesystem::path effectPath{ binding.strEffectDocument };
		if (!Is_StableToken(binding.strBindingId) ||
			!Is_StableToken(binding.strPatternId) ||
			!Is_StableToken(binding.strSemanticStageId) ||
			!Is_StableToken(binding.strActionId) ||
			!Is_StableToken(binding.strEffectAssetId) ||
			!Is_StableToken(binding.strRuntimeClipName) ||
			!Is_StableToken(binding.strRuntimeBoneName) ||
			!Is_StableToken(binding.strProductAdmissionStatus) ||
			effectPath.empty() || effectPath.is_absolute() ||
			binding.strEffectDocument.find("..") != std::string::npos ||
			!bindingIds.insert(binding.strBindingId).second ||
			!actionKeys.insert(binding.strPatternId + "\n" +
				binding.strActionId).second ||
			!effectAssets.insert(binding.strEffectAssetId).second ||
			availableClips.end() == std::find(availableClips.begin(),
				availableClips.end(), binding.strRuntimeClipName) ||
			bProductAdmitted != (binding.bProductCatalogMapped &&
				binding.bAnimationEventMapped))
		{
			outStatus =
				"Boss pattern Effect binding is duplicate, unsafe, or not admitted.";
			return false;
		}
	}
	outStatus = "Validated " + std::to_string(document.Bindings.size()) +
		" boss pattern Effect binding(s).";
	return true;
}

bool_t Client::CValtanPatternEffectBindingDocument::Load(
	const std::string_view animationAssetId,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_Path(animationAssetId);
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input)
	{
		outStatus =
			"Boss pattern Effect document is missing: " + path.string();
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT staged;
	if (!Parse_Text(text, staged, outStatus) ||
		!Validate(staged, expectedBossArchetypeId, availableClips, outStatus))
	{
		return false;
	}
	outDocument = std::move(staged);
	return true;
}

bool_t Client::CValtanPatternEffectBindingDocument::Stage_ValtanEffectToolTree(
	const std::string_view text,
	const std::filesystem::path& projectDataRoot,
	BOSS_PATTERN_EFFECT_TREE_STAGE& outStage,
	std::string& outStatus)
{
	constexpr std::string_view BOSS_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view BINDING_ID =
		"valtan.whirlwind.420633.active";
	constexpr std::string_view PATTERN_ID = "VALTAN_WHIRLWIND";
	constexpr std::string_view SEMANTIC_STAGE_ID = "SPIN";
	constexpr std::string_view ACTION_ID =
		"valtan.attack.whirlwind.active";
	constexpr std::string_view EFFECT_ASSET_ID =
		"effect.valtan.pattern.420633.active";
	constexpr std::string_view EFFECT_DOCUMENT =
		"Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json";
	constexpr std::string_view RUNTIME_CLIP = "mesh_att_battle_20_03";
	constexpr std::string_view RUNTIME_BONE = "b_effectroot";
	constexpr std::string_view ADMISSION_STATUS =
		"FAIL_CLOSED_NON_PRODUCT_CANARY";

	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT bindingDocument;
	if (!Parse_Text(text, bindingDocument, outStatus))
		return false;
	const std::vector<std::string> availableClips{ std::string(RUNTIME_CLIP) };
	if (!Validate(
			bindingDocument, BOSS_ARCHETYPE_ID, availableClips, outStatus))
	{
		return false;
	}
	if (bindingDocument.Bindings.size() != 1u)
	{
		outStatus = "Valtan Boss Patterns must contain exactly one staged binding.";
		return false;
	}

	const BOSS_PATTERN_EFFECT_BINDING& binding =
		bindingDocument.Bindings.front();
	if (binding.strBindingId != BINDING_ID ||
		binding.strPatternId != PATTERN_ID ||
		binding.strSemanticStageId != SEMANTIC_STAGE_ID ||
		binding.strActionId != ACTION_ID ||
		binding.strEffectAssetId != EFFECT_ASSET_ID ||
		binding.strEffectDocument != EFFECT_DOCUMENT ||
		binding.strRuntimeClipName != RUNTIME_CLIP ||
		binding.strRuntimeBoneName != RUNTIME_BONE ||
		binding.strProductAdmissionStatus != ADMISSION_STATUS ||
		binding.bProductCatalogMapped || binding.bAnimationEventMapped)
	{
		outStatus =
			"Valtan Boss Patterns binding does not match the 420633 non-product canary contract.";
		return false;
	}
	if (projectDataRoot.empty())
	{
		outStatus = "Valtan Boss Patterns Data root is empty.";
		return false;
	}

	const std::filesystem::path effectPath = projectDataRoot /
		std::filesystem::path(L"Effects") / L"Authored" /
		L"effect.valtan.pattern.420633.active.effect.json";
	std::error_code fileError;
	if (!std::filesystem::is_regular_file(effectPath, fileError) || fileError)
	{
		outStatus = "Valtan Boss Patterns Effect document is missing: " +
			effectPath.string();
		return false;
	}

	EFFECT_DOCUMENT_DESC effectDocument;
	std::string effectError;
	if (!CEffectDocumentCodec::Load(effectPath, effectDocument, effectError))
	{
		outStatus = "Valtan Boss Patterns Effect document is invalid: " +
			effectError;
		return false;
	}
	if (effectDocument.strEffectAssetId != EFFECT_ASSET_ID)
	{
		outStatus =
			"Valtan Boss Patterns Effect document ID does not match its binding.";
		return false;
	}

	BOSS_PATTERN_EFFECT_TREE_STAGE staged;
	BOSS_PATTERN_EFFECT_TREE_ROW row;
	row.strBossArchetypeId = bindingDocument.strBossArchetypeId;
	row.strBindingId = binding.strBindingId;
	row.strPatternId = binding.strPatternId;
	row.strSemanticStageId = binding.strSemanticStageId;
	row.strActionId = binding.strActionId;
	row.strEffectAssetId = binding.strEffectAssetId;
	row.strRuntimeClipName = binding.strRuntimeClipName;
	row.strRuntimeBoneName = binding.strRuntimeBoneName;
	row.strProductAdmissionStatus = binding.strProductAdmissionStatus;
	row.Path = effectPath;
	row.bProductCatalogMapped = binding.bProductCatalogMapped;
	row.bAnimationEventMapped = binding.bAnimationEventMapped;
	staged.Rows.push_back(std::move(row));
	outStage = std::move(staged);
	outStatus = "Staged one Valtan 420633 Boss Patterns Effect.";
	return true;
}
