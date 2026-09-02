#include "AnimationSkillBindingDocument.h"

#include "ActionPresentationTimeline.h"
#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <io.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
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

	bool Try_ParseSourceMs(const DATA_JSON_VALUE& value,
		std::uint32_t& outMilliseconds)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(MAX_CLIP_PLAY_MS) ||
			std::floor(number) != number)
		{
			return false;
		}
		outMilliseconds = static_cast<std::uint32_t>(number);
		return true;
	}

	std::string Build_LegacyBossClipOccurrenceId(
		const std::string_view actionId,
		const std::size_t clipIndex)
	{
		return std::string(actionId) + ".legacy.clip." +
			std::to_string(clipIndex + 1u);
	}

	bool Is_BossMappingBasis(const std::string_view value)
	{
		return "ANIMATION_PR_127" == value ||
			"PATTERN_PR_REFERENCE" == value ||
			"CURRENT_PRODUCT_BASELINE" == value ||
			"SOURCE_REVIEWED_DELTA" == value ||
			"PROJECT_AUTHORED" == value;
	}

	void Serialize_Clip(
		std::ostringstream& output,
		const ANIMATION_SKILL_CLIP& clip)
	{
		if (0u == clip.iPlayMs && 1.f == clip.fPlayRate &&
			0u == clip.iSourceStartMs)
		{
			output << '"' << CDataJson::Escape(clip.strClipName) << '"';
			return;
		}
		output << "{ \"clip\": \""
			<< CDataJson::Escape(clip.strClipName) << '"';
		if (0u != clip.iSourceStartMs)
			output << ", \"sourceStartMs\": " << clip.iSourceStartMs;
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

	std::string Serialize_BossPatternBindings(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document)
	{
		std::ostringstream output;
		output << std::setprecision(
			std::numeric_limits<f32_t>::max_digits10);
		output << "{\n"
			<< "  \"schema\": \"lostark.valtan-pattern-bindings\",\n"
			<< "  \"formatVersion\": 4,\n"
			<< "  \"bossArchetypeId\": \""
			<< CDataJson::Escape(document.strBossArchetypeId) << "\",\n"
			<< "  \"bindings\": [\n";
		for (std::size_t bindingIndex = 0u;
			bindingIndex < document.Bindings.size(); ++bindingIndex)
		{
			const BOSS_PATTERN_ANIMATION_BINDING& binding =
				document.Bindings[bindingIndex];
			output << "    {\n"
				<< "      \"actionId\": \""
				<< CDataJson::Escape(binding.strActionId) << "\",\n";
			if (binding.bSuppressAnimation)
				output << "      \"playbackMode\": \"NONE\",\n";
			if (binding.bHasBodyHiddenWindow)
			{
				output << "      \"bodyVisibility\": { \"hiddenFromMs\": "
					<< binding.iBodyHiddenFromMs << ", \"hiddenToMs\": "
					<< binding.iBodyHiddenToMs << " },\n";
			}
			output << "      \"clips\": [\n";
			for (std::size_t clipIndex = 0u;
				clipIndex < binding.Clips.size(); ++clipIndex)
			{
				const BOSS_PATTERN_ANIMATION_CLIP& clip =
					binding.Clips[clipIndex];
				output << "        {\n"
					<< "          \"clipOccurrenceId\": \""
					<< CDataJson::Escape(clip.strClipOccurrenceId) << "\",\n"
					<< "          \"clip\": \""
					<< CDataJson::Escape(clip.strClipName) << "\",\n"
					<< "          \"mappingBasis\": \""
					<< CDataJson::Escape(clip.strMappingBasis) << "\",\n"
					<< "          \"sourceStartMs\": "
					<< clip.iSourceStartMs << ",\n"
					<< "          \"playMs\": " << clip.iPlayMs << ",\n"
					<< "          \"playRate\": " << clip.fPlayRate << ",\n"
					<< "          \"loop\": "
					<< (clip.bLoop ? "true" : "false") << "\n"
					<< "        }"
					<< (clipIndex + 1u < binding.Clips.size() ? "," : "")
					<< "\n";
			}
			output << "      ]\n"
				<< "    }"
				<< (bindingIndex + 1u < document.Bindings.size() ? "," : "")
				<< "\n";
		}
		output << "  ]\n}\n";
		return output.str();
	}

	bool_t Load_BossPatternBindingsFromPath(
		const std::filesystem::path& path,
		const std::string_view expectedBossArchetypeId,
		const std::vector<std::string>& availableClips,
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
		std::string& outStatus,
		std::string* const pOutSourceBytes = nullptr)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			outStatus =
				"Boss pattern binding document is missing: " + path.string();
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad())
		{
			outStatus =
				"Boss pattern binding document could not be read completely: " +
				path.string();
			return false;
		}
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT staged;
		if (!CValtanPatternAnimationBindingDocument::Parse_Text(
				text, staged, outStatus) ||
			!CValtanPatternAnimationBindingDocument::Validate(
				staged, expectedBossArchetypeId, availableClips, outStatus))
		{
			return false;
		}
		outDocument = std::move(staged);
		if (nullptr != pOutSourceBytes)
			*pOutSourceBytes = text;
		return true;
	}

	bool_t Read_BinaryText(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return false;
		std::string staged{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad())
			return false;
		outText = std::move(staged);
		return true;
	}


	enum class DEPENDENT_CUE_KIND : uint8_t
	{
		EFFECT,
		SOUND,
		SHAKE,
	};

	bool_t Build_BossPatternTimings(
		const BOSS_PATTERN_ANIMATION_BINDING& binding,
		const std::unordered_map<std::string, f32_t>&
			clipSourceDurationSecondsByName,
		std::vector<ACTION_PRESENTATION_CLIP_TIMING>& outTimings,
		std::string& outStatus)
	{
		if (binding.bSuppressAnimation || binding.Clips.empty())
		{
			outStatus =
				"Boss pattern model timing requires an active clip sequence: " +
				binding.strActionId;
			return false;
		}
		std::vector<ACTION_PRESENTATION_CLIP_TIMING> staged;
		staged.reserve(binding.Clips.size());
		for (const BOSS_PATTERN_ANIMATION_CLIP& clip : binding.Clips)
		{
			const auto duration =
				clipSourceDurationSecondsByName.find(clip.strClipName);
			if (clipSourceDurationSecondsByName.end() == duration ||
				!std::isfinite(duration->second) || duration->second <= 0.f)
			{
				outStatus =
					"Boss pattern animation Save is missing a current model duration: " +
					clip.strClipName;
				return false;
			}
			ACTION_PRESENTATION_CLIP_TIMING timing{
				duration->second,
				clip.iPlayMs,
				clip.fPlayRate,
				clip.bLoop,
				static_cast<f32_t>(clip.iSourceStartMs) * 0.001f };
			f32_t sourceDurationSeconds = 0.f;
			f32_t wallDurationSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
					timing, sourceDurationSeconds, wallDurationSeconds))
			{
				outStatus =
					"Boss pattern animation Save rejected a current model source window: " +
					clip.strClipOccurrenceId;
				return false;
			}
			staged.push_back(timing);
		}
		outTimings = std::move(staged);
		return true;
	}

	bool_t Is_FiniteVec3(const DATA_JSON_VALUE* const pValue)
	{
		return nullptr != pValue && pValue->Is_Array() &&
			3u == pValue->Get_Array().size() &&
			std::all_of(
				pValue->Get_Array().begin(), pValue->Get_Array().end(),
				[](const DATA_JSON_VALUE& Element)
				{
					return Element.Is_Number() &&
						std::isfinite(Element.Get_Number());
				});
	}

	bool_t Has_DependentCueRowShape(
		const DATA_JSON_VALUE& row,
		const DEPENDENT_CUE_KIND kind)
	{
		if (DEPENDENT_CUE_KIND::EFFECT == kind)
		{
			const bool_t exact = Has_ExactProperties(row,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "effectAssetId",
				  "anchorSlotId", "followPolicy", "stopPolicy",
				  "repeatPolicy", "sourceStartMs", "sourceEndMs",
				  "localTransform" }) || Has_ExactProperties(row,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "effectAssetId",
				  "anchorSlotId", "followPolicy", "stopPolicy",
				  "repeatPolicy", "sourceStartMs", "sourceEndMs",
				  "localTransform", "scalePolicy" });
			const DATA_JSON_VALUE* const transform = row.Find("localTransform");
			if (!exact || nullptr == transform ||
				!Has_ExactProperties(
					*transform, { "position", "rotationDegrees", "scale" }) ||
				!Is_FiniteVec3(transform->Find("position")) ||
				!Is_FiniteVec3(transform->Find("rotationDegrees")) ||
				!Is_FiniteVec3(transform->Find("scale")))
			{
				return false;
			}
			const DATA_JSON_VALUE* const start = row.Find("sourceStartMs");
			const DATA_JSON_VALUE* const end = row.Find("sourceEndMs");
			if (nullptr == start || !start->Is_Number() ||
				!std::isfinite(start->Get_Number()) ||
				(nullptr == end ||
				 (!end->Is_Null() && (!end->Is_Number() ||
				  !std::isfinite(end->Get_Number())))))
			{
				return false;
			}
			const DATA_JSON_VALUE* const scalePolicy = row.Find("scalePolicy");
			if (nullptr != scalePolicy)
			{
				if (!scalePolicy->Is_Object() ||
					(!Has_ExactProperties(*scalePolicy, { "kind" }) &&
					 !Has_ExactProperties(
						 *scalePolicy, { "kind", "worldScale" })) ||
					nullptr == Required(
						*scalePolicy, "kind", DATA_JSON_TYPE::STRING) ||
					(nullptr != scalePolicy->Find("worldScale") &&
					 !Is_FiniteVec3(scalePolicy->Find("worldScale"))))
				{
					return false;
				}
			}
			return true;
		}
		if (DEPENDENT_CUE_KIND::SOUND == kind)
		{
			return Has_ExactProperties(row,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "soundBank",
				  "soundEvent", "repeatPolicy", "startMs" }) &&
				nullptr != Required(row, "soundBank", DATA_JSON_TYPE::STRING) &&
				nullptr != Required(row, "soundEvent", DATA_JSON_TYPE::STRING) &&
				nullptr != Required(row, "repeatPolicy", DATA_JSON_TYPE::STRING) &&
				nullptr != Required(row, "startMs", DATA_JSON_TYPE::NUMBER);
		}
		return Has_ExactProperties(row,
			{ "bindingId", "occurrenceId", "patternId", "stageId",
			  "actionId", "clipOccurrenceId", "repeatPolicy",
			  "startMs", "shake" }) &&
			nullptr != Required(row, "repeatPolicy", DATA_JSON_TYPE::STRING) &&
			nullptr != Required(row, "startMs", DATA_JSON_TYPE::NUMBER) &&
			nullptr != Required(row, "shake", DATA_JSON_TYPE::STRING);
	}

	bool_t Validate_DependentCueOwner(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& baselineDocument,
		const std::unordered_map<std::string, f32_t>&
			clipSourceDurationSecondsByName,
		const std::unordered_map<std::string, uint32_t>&
			stageDurationMsByAction,
		const std::filesystem::path& path,
		const std::string_view expectedSchema,
		const uint32_t expectedVersion,
		const std::string_view expectedBossArchetypeId,
		const DEPENDENT_CUE_KIND kind,
		std::string& outAdmittedSourceBytes,
		std::string& outStatus)
	{
		std::string text;
		if (!Read_BinaryText(path, text))
		{
			outStatus =
				"Boss pattern animation dependency source is missing: " +
				path.string();
			return false;
		}
		DATA_JSON_VALUE root;
		std::string parseError;
		if (!CDataJson::Parse(text, root, parseError) ||
			!Has_ExactProperties(
				root, { "schema", "formatVersion", "ownerArchetypeId", "cues" }))
		{
			outStatus =
				"Boss pattern animation dependency JSON is malformed: " +
				path.string() + ": " + parseError;
			return false;
		}
		const DATA_JSON_VALUE* const schema = Required(
			root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const version = Required(
			root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* const owner = Required(
			root, "ownerArchetypeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* const cues = Required(
			root, "cues", DATA_JSON_TYPE::ARRAY);
		if (nullptr == schema || schema->Get_String() != expectedSchema ||
			nullptr == version || !std::isfinite(version->Get_Number()) ||
			version->Get_Number() != static_cast<double>(expectedVersion) ||
			nullptr == owner || owner->Get_String() != expectedBossArchetypeId ||
			nullptr == cues || cues->Get_Array().empty() ||
			cues->Get_Array().size() > 100000u)
		{
			outStatus =
				"Boss pattern animation dependency header is invalid: " +
				path.string();
			return false;
		}

		std::unordered_set<std::string> bindingIds;
		std::unordered_set<std::string> occurrenceIds;
		for (const DATA_JSON_VALUE& row : cues->Get_Array())
		{
			const DATA_JSON_VALUE* const bindingId = Required(
				row, "bindingId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const occurrenceId = Required(
				row, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const actionId = Required(
				row, "actionId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* const clipOccurrenceId = Required(
				row, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
			if (!Has_DependentCueRowShape(row, kind) ||
				nullptr == bindingId || !Is_StableToken(bindingId->Get_String()) ||
				nullptr == occurrenceId ||
				!Is_StableToken(occurrenceId->Get_String()) ||
				nullptr == actionId || !Is_StableToken(actionId->Get_String()) ||
				nullptr == clipOccurrenceId ||
				(!clipOccurrenceId->Get_String().empty() &&
				 !Is_StableToken(clipOccurrenceId->Get_String())) ||
				!bindingIds.insert(bindingId->Get_String()).second ||
				!occurrenceIds.insert(occurrenceId->Get_String()).second)
			{
				outStatus =
					"Boss pattern animation dependency row identity/shape is invalid or duplicated: " +
					path.string();
				return false;
			}

			const auto binding = std::find_if(
				document.Bindings.begin(), document.Bindings.end(),
				[&actionId](const BOSS_PATTERN_ANIMATION_BINDING& candidate)
				{
					return candidate.strActionId == actionId->Get_String();
				});
			if (document.Bindings.end() == binding)
			{
				outStatus =
					"Boss pattern animation dependency action is missing: " +
					actionId->Get_String();
				return false;
			}
			const auto baselineBinding = std::find_if(
				baselineDocument.Bindings.begin(), baselineDocument.Bindings.end(),
				[&actionId](const BOSS_PATTERN_ANIMATION_BINDING& candidate)
				{
					return candidate.strActionId == actionId->Get_String();
				});
			if (clipOccurrenceId->Get_String().empty())
			{
				if (DEPENDENT_CUE_KIND::EFFECT != kind ||
					!binding->bSuppressAnimation)
				{
					outStatus =
						"Only an Effect stage-clock row may omit clipOccurrenceId on an explicit NONE action: " +
						binding->strActionId;
					return false;
				}
				if (baselineDocument.Bindings.end() == baselineBinding ||
					*baselineBinding != *binding)
				{
					const auto stageDuration =
						stageDurationMsByAction.find(binding->strActionId);
					const DATA_JSON_VALUE* const start =
						row.Find("sourceStartMs");
					if (stageDurationMsByAction.end() == stageDuration ||
						nullptr == start || !start->Is_Number() ||
						!std::isfinite(start->Get_Number()) ||
						start->Get_Number() < 0.0 ||
						start->Get_Number() >=
							static_cast<double>(stageDuration->second))
					{
						outStatus =
							"Boss pattern stage-clock Effect is outside its Encounter stage wall: " +
							occurrenceId->Get_String();
						return false;
					}
				}
				continue;
			}
			const bool_t candidateJoinsOccurrence =
				!binding->bSuppressAnimation &&
				binding->Clips.end() != std::find_if(
					binding->Clips.begin(), binding->Clips.end(),
					[&clipOccurrenceId](
						const BOSS_PATTERN_ANIMATION_CLIP& clip)
					{
						return clip.strClipOccurrenceId ==
							clipOccurrenceId->Get_String();
					});
			const bool_t baselineJoinedOccurrence =
				baselineDocument.Bindings.end() != baselineBinding &&
				!baselineBinding->bSuppressAnimation &&
				baselineBinding->Clips.end() != std::find_if(
					baselineBinding->Clips.begin(), baselineBinding->Clips.end(),
					[&clipOccurrenceId](
						const BOSS_PATTERN_ANIMATION_CLIP& clip)
					{
						return clip.strClipOccurrenceId ==
							clipOccurrenceId->Get_String();
					});
			/* A source owner can contain a pre-existing stale identity which this
			   animation transaction does not own. Preserve that exact defect for
			   an unrelated valid edit, but never allow an admitted join to become
			   stale. The owner-specific publisher remains the authority that must
			   repair the original stale row. */
			if (!candidateJoinsOccurrence && baselineJoinedOccurrence)
			{
				outStatus =
					"Boss pattern animation edit would orphan a dependent cue occurrence: " +
					binding->strActionId + "/" +
					clipOccurrenceId->Get_String();
				return false;
			}
			if (!candidateJoinsOccurrence ||
				(baselineDocument.Bindings.end() != baselineBinding &&
				 *baselineBinding == *binding))
			{
				continue;
			}

			const auto stageDuration =
				stageDurationMsByAction.find(binding->strActionId);
			if (stageDurationMsByAction.end() == stageDuration)
			{
				outStatus =
					"Boss pattern animation dependency has no Encounter stage duration: " +
					binding->strActionId;
				return false;
			}
			std::vector<ACTION_PRESENTATION_CLIP_TIMING> timings;
			if (!Build_BossPatternTimings(*binding,
					clipSourceDurationSecondsByName, timings, outStatus))
			{
				return false;
			}
			const auto clip = std::find_if(
				binding->Clips.begin(), binding->Clips.end(),
				[&clipOccurrenceId](
					const BOSS_PATTERN_ANIMATION_CLIP& candidate)
				{
					return candidate.strClipOccurrenceId ==
						clipOccurrenceId->Get_String();
				});
			const std::size_t clipIndex = static_cast<std::size_t>(
				clip - binding->Clips.begin());
			const DATA_JSON_VALUE* const start = row.Find(
				DEPENDENT_CUE_KIND::EFFECT == kind ?
					"sourceStartMs" : "startMs");
			const DATA_JSON_VALUE* const repeatPolicy = row.Find("repeatPolicy");
			if (nullptr == start || !start->Is_Number() ||
				!std::isfinite(start->Get_Number()) ||
				start->Get_Number() < 0.0 ||
				start->Get_Number() >
					static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
				nullptr == repeatPolicy || !repeatPolicy->Is_String() ||
				clipIndex >= timings.size())
			{
				outStatus =
					"Boss pattern animation dependency timing fields are invalid: " +
					occurrenceId->Get_String();
				return false;
			}
			const f32_t cueSourceSeconds =
				static_cast<f32_t>(start->Get_Number()) * 0.001f;
			f32_t sourceDurationSeconds = 0.f;
			f32_t wallDurationSeconds = 0.f;
			f32_t cueStartWallSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
					timings[clipIndex], sourceDurationSeconds,
					wallDurationSeconds) ||
				cueSourceSeconds >=
					timings[clipIndex].fSourceStartSeconds +
						sourceDurationSeconds ||
				!CActionPresentationTimeline::Resolve_CueWallOffset(
					timings, clipIndex, cueSourceSeconds, 0u,
					cueStartWallSeconds) ||
				cueStartWallSeconds * 1000.f >=
					static_cast<f32_t>(stageDuration->second) ||
				("each_loop" == repeatPolicy->Get_String() &&
				 !timings[clipIndex].bLoop))
			{
				outStatus =
					"Boss pattern animation edit would move a dependent cue outside its runtime source/stage wall: " +
					occurrenceId->Get_String();
				return false;
			}
			if (DEPENDENT_CUE_KIND::EFFECT == kind)
			{
				const DATA_JSON_VALUE* const end = row.Find("sourceEndMs");
				if (nullptr != end && end->Is_Number())
				{
					const f32_t cueEndSourceSeconds =
						static_cast<f32_t>(end->Get_Number()) * 0.001f;
					f32_t cueEndWallSeconds = 0.f;
					if (!std::isfinite(end->Get_Number()) ||
						end->Get_Number() <= start->Get_Number() ||
						!CActionPresentationTimeline::Resolve_CueEndWallOffset(
							timings, clipIndex, cueEndSourceSeconds, 0u,
							cueEndWallSeconds) ||
						cueEndWallSeconds <= cueStartWallSeconds ||
						(!timings[clipIndex].bLoop &&
						 cueEndWallSeconds * 1000.f >
							static_cast<f32_t>(stageDuration->second) + 0.01f))
					{
						outStatus =
							"Boss pattern animation edit would move a dependent Effect end outside its runtime source/stage wall: " +
							occurrenceId->Get_String();
						return false;
					}
				}
			}
		}
		outAdmittedSourceBytes = std::move(text);
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
					clip, { "clip", "sourceStartMs", "playMs", "playRate" }))
				{
					const DATA_JSON_VALUE* clipName = Required(
						clip, "clip", DATA_JSON_TYPE::STRING);
					const DATA_JSON_VALUE* sourceStartMs =
						clip.Find("sourceStartMs");
					const DATA_JSON_VALUE* playMs = clip.Find("playMs");
					const DATA_JSON_VALUE* playRate = clip.Find("playRate");
					if (nullptr == clipName ||
						(nullptr == sourceStartMs && nullptr == playMs &&
							nullptr == playRate) ||
						(nullptr != sourceStartMs &&
							!Try_ParseSourceMs(
								*sourceStartMs, stagedClip.iSourceStartMs)) ||
						(nullptr != playMs &&
							!Try_ParsePlayMs(*playMs, stagedClip.iPlayMs)) ||
						(nullptr != playRate &&
							!Try_ParsePlayRate(*playRate, stagedClip.fPlayRate)))
					{
						outStatus = "Skill binding clip source window is invalid.";
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
					clip.iSourceStartMs > MAX_CLIP_PLAY_MS ||
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
	uint32_t formatVersion = 0u;
	if (nullptr != version && version->Is_Number())
	{
		const double number = version->Get_Number();
		if (std::isfinite(number) && std::floor(number) == number &&
				(number == 1.0 || number == 2.0 || number == 3.0 ||
				 number == 4.0))
		{
			formatVersion = static_cast<uint32_t>(number);
		}
	}
	if (nullptr == schema || schema->Get_String() != BOSS_DOCUMENT_SCHEMA ||
		0u == formatVersion ||
		nullptr == boss || !Is_StableToken(boss->Get_String()) ||
		nullptr == bindings || bindings->Get_Array().empty() ||
		bindings->Get_Array().size() > MAX_BOSS_PATTERN_BINDINGS)
	{
		outStatus = "Boss pattern binding header is invalid.";
		return false;
	}

	constexpr std::size_t MAX_BOSS_PATTERN_CHAIN_CLIPS = 16u;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT staged;
	staged.iFormatVersion = formatVersion;
	staged.strBossArchetypeId = boss->Get_String();
	for (const DATA_JSON_VALUE& value : bindings->Get_Array())
	{
		const bool isLegacy = 1u == formatVersion;
		const bool supportsPlaybackMode = formatVersion >= 3u;
		const bool supportsBodyVisibility = formatVersion >= 4u;
		const bool hasPlaybackMode =
			nullptr != value.Find("playbackMode");
		const bool hasBodyVisibility =
			nullptr != value.Find("bodyVisibility");
		if ((isLegacy &&
			!Has_ExactProperties(value, { "actionId", "clip" })) ||
			(!isLegacy &&
			 ((!supportsPlaybackMode && hasPlaybackMode) ||
			  (!supportsBodyVisibility && hasBodyVisibility) ||
			  (!hasPlaybackMode && !hasBodyVisibility &&
			   !Has_ExactProperties(value, { "actionId", "clips" })) ||
			  (hasPlaybackMode && !hasBodyVisibility &&
			   !Has_ExactProperties(value,
				   { "actionId", "playbackMode", "clips" })) ||
			  (!hasPlaybackMode && hasBodyVisibility &&
			   !Has_ExactProperties(value,
				   { "actionId", "bodyVisibility", "clips" })) ||
			  (hasPlaybackMode && hasBodyVisibility &&
			   !Has_ExactProperties(value,
				   { "actionId", "playbackMode", "bodyVisibility", "clips" })))))
		{
			outStatus = "Boss pattern binding row has an unexpected field set.";
			return false;
		}
		const DATA_JSON_VALUE* actionId = Required(
			value, "actionId", DATA_JSON_TYPE::STRING);
		if (nullptr == actionId || !Is_StableToken(actionId->Get_String()))
		{
			outStatus = "Boss pattern binding row is invalid.";
			return false;
		}

		BOSS_PATTERN_ANIMATION_BINDING stagedBinding;
		stagedBinding.strActionId = actionId->Get_String();
		if (hasBodyVisibility)
		{
			const DATA_JSON_VALUE* bodyVisibility = value.Find("bodyVisibility");
			if (nullptr == bodyVisibility || !bodyVisibility->Is_Object() ||
				!Has_ExactProperties(*bodyVisibility,
					{ "hiddenFromMs", "hiddenToMs" }) ||
				!Try_ParseSourceMs(*bodyVisibility->Find("hiddenFromMs"),
					stagedBinding.iBodyHiddenFromMs) ||
				!Try_ParseSourceMs(*bodyVisibility->Find("hiddenToMs"),
					stagedBinding.iBodyHiddenToMs) ||
				stagedBinding.iBodyHiddenFromMs >= stagedBinding.iBodyHiddenToMs)
			{
				outStatus =
					"Boss pattern body visibility window is invalid.";
				return false;
			}
			stagedBinding.bHasBodyHiddenWindow = true;
		}
		if (isLegacy)
		{
			const DATA_JSON_VALUE* clip = value.Find("clip");
			std::vector<std::string> legacyChain;
			if (nullptr != clip && clip->Is_String())
			{
				legacyChain.push_back(clip->Get_String());
			}
			else if (nullptr != clip && clip->Is_Array())
			{
				for (const DATA_JSON_VALUE& element : clip->Get_Array())
				{
					if (!element.Is_String() ||
						!Is_StableToken(element.Get_String()))
					{
						outStatus =
							"Boss pattern binding chain clip is invalid.";
						return false;
					}
					legacyChain.push_back(element.Get_String());
				}
			}
			if (legacyChain.empty() ||
				legacyChain.size() > MAX_BOSS_PATTERN_CHAIN_CLIPS)
			{
				outStatus = "Boss pattern binding row is invalid.";
				return false;
			}
			stagedBinding.Clips.reserve(legacyChain.size());
			for (std::size_t clipIndex = 0u;
				clipIndex < legacyChain.size(); ++clipIndex)
			{
				BOSS_PATTERN_ANIMATION_CLIP stagedClip;
				stagedClip.strClipOccurrenceId =
					Build_LegacyBossClipOccurrenceId(
						stagedBinding.strActionId, clipIndex);
				stagedClip.strClipName = std::move(legacyChain[clipIndex]);
				stagedClip.strMappingBasis = "LEGACY_V1_MIGRATION";
				stagedClip.bLoop = clipIndex + 1u == legacyChain.size();
				if (!Is_StableToken(stagedClip.strClipOccurrenceId) ||
					!Is_StableToken(stagedClip.strClipName))
				{
					outStatus = "Boss pattern legacy clip migration is invalid.";
					return false;
				}
				stagedBinding.Clips.push_back(std::move(stagedClip));
			}
		}
		else
		{
			const DATA_JSON_VALUE* playbackMode =
				value.Find("playbackMode");
			if (nullptr != playbackMode)
			{
				if (!playbackMode->Is_String())
				{
					outStatus =
						"Boss pattern binding playback mode is invalid.";
					return false;
				}
				if ("NONE" == playbackMode->Get_String())
					stagedBinding.bSuppressAnimation = true;
				else if ("CLIP_SEQUENCE" != playbackMode->Get_String())
				{
					outStatus =
						"Boss pattern binding playback mode is unsupported.";
					return false;
				}
			}
			const DATA_JSON_VALUE* clips = Required(
				value, "clips", DATA_JSON_TYPE::ARRAY);
			if (nullptr == clips ||
				(stagedBinding.bSuppressAnimation ?
					!clips->Get_Array().empty() :
					(clips->Get_Array().empty() ||
					 clips->Get_Array().size() >
						 MAX_BOSS_PATTERN_CHAIN_CLIPS)))
			{
				outStatus = stagedBinding.bSuppressAnimation ?
					"Boss pattern NONE binding must have an empty clip chain." :
					"Boss pattern binding v2 clip chain is invalid.";
				return false;
			}
			stagedBinding.Clips.reserve(clips->Get_Array().size());
			for (const DATA_JSON_VALUE& clipValue : clips->Get_Array())
			{
				if (!Has_ExactProperties(clipValue,
					{ "clipOccurrenceId", "clip", "mappingBasis",
					  "sourceStartMs", "playMs", "playRate", "loop" }))
				{
					outStatus =
						"Boss pattern binding v2 clip has unexpected properties.";
					return false;
				}
				const DATA_JSON_VALUE* occurrenceId = Required(
					clipValue, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* clipName = Required(
					clipValue, "clip", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* mappingBasis = Required(
					clipValue, "mappingBasis", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* playRate = Required(
					clipValue, "playRate", DATA_JSON_TYPE::NUMBER);
				const DATA_JSON_VALUE* loop = Required(
					clipValue, "loop", DATA_JSON_TYPE::BOOLEAN);
				BOSS_PATTERN_ANIMATION_CLIP stagedClip;
				if (nullptr == occurrenceId ||
					!Is_StableToken(occurrenceId->Get_String()) ||
					nullptr == clipName ||
					!Is_StableToken(clipName->Get_String()) ||
					nullptr == mappingBasis ||
					!Is_StableToken(mappingBasis->Get_String()) ||
					nullptr == playRate ||
					!Try_ParsePlayRate(*playRate, stagedClip.fPlayRate) ||
					nullptr == loop ||
					!Try_ParseSourceMs(*clipValue.Find("sourceStartMs"),
						stagedClip.iSourceStartMs) ||
					!Try_ParseSourceMs(*clipValue.Find("playMs"),
						stagedClip.iPlayMs))
				{
					outStatus = "Boss pattern binding v2 clip is invalid.";
					return false;
				}
				stagedClip.strClipOccurrenceId = occurrenceId->Get_String();
				stagedClip.strClipName = clipName->Get_String();
				stagedClip.strMappingBasis = mappingBasis->Get_String();
				stagedClip.bLoop = loop->Get_Boolean();
				stagedBinding.Clips.push_back(std::move(stagedClip));
			}
		}
		staged.Bindings.push_back(std::move(stagedBinding));
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
		(document.iFormatVersion != 1u && document.iFormatVersion != 2u &&
		 document.iFormatVersion != 3u && document.iFormatVersion != 4u) ||
		document.Bindings.empty() || document.Bindings.size() > 512u)
	{
		outStatus = "Boss pattern binding owner does not match the target boss.";
		return false;
	}
	std::unordered_set<std::string> claimedActions;
	std::unordered_set<std::string> claimedOccurrences;
	for (const BOSS_PATTERN_ANIMATION_BINDING& binding : document.Bindings)
	{
		const bool_t bValidPlaybackContract = binding.bSuppressAnimation ?
			(document.iFormatVersion >= 3u && binding.Clips.empty()) :
			(!binding.Clips.empty() && binding.Clips.size() <= 16u);
		const bool_t bValidBodyVisibilityContract =
			binding.bHasBodyHiddenWindow ?
				(document.iFormatVersion >= 4u &&
				 binding.iBodyHiddenFromMs < binding.iBodyHiddenToMs) :
				(0u == binding.iBodyHiddenFromMs &&
				 0u == binding.iBodyHiddenToMs);
		if (!Is_StableToken(binding.strActionId) ||
			!bValidPlaybackContract ||
			!bValidBodyVisibilityContract ||
			!claimedActions.insert(binding.strActionId).second)
		{
			outStatus =
				"Boss pattern binding has a duplicate action or invalid playback contract.";
			return false;
		}
		for (std::size_t clipIndex = 0u;
			clipIndex < binding.Clips.size(); ++clipIndex)
		{
			const BOSS_PATTERN_ANIMATION_CLIP& clip =
				binding.Clips[clipIndex];
			const bool isLegacy = 1u == document.iFormatVersion;
			if (!Is_StableToken(clip.strClipOccurrenceId) ||
				!claimedOccurrences.insert(
					clip.strClipOccurrenceId).second ||
				!Is_StableToken(clip.strClipName) ||
				(isLegacy ?
					clip.strMappingBasis != "LEGACY_V1_MIGRATION" :
					!Is_BossMappingBasis(clip.strMappingBasis)) ||
				(isLegacy &&
				 (0u != clip.iSourceStartMs || 0u != clip.iPlayMs ||
				  1.f != clip.fPlayRate ||
				  clip.bLoop !=
					(clipIndex + 1u == binding.Clips.size()))) ||
				clip.iSourceStartMs > MAX_CLIP_PLAY_MS ||
				clip.iPlayMs > MAX_CLIP_PLAY_MS ||
				!std::isfinite(clip.fPlayRate) ||
				clip.fPlayRate < 0.05f || clip.fPlayRate > 16.f ||
				(clip.bLoop && clipIndex + 1u != binding.Clips.size()) ||
				availableClips.end() == std::find(
					availableClips.begin(), availableClips.end(),
					clip.strClipName))
			{
				outStatus =
					"Boss pattern binding has an invalid occurrence, timing, loop, or model clip.";
				return false;
			}
		}
	}
	outStatus = "Validated " + std::to_string(document.Bindings.size()) +
		" boss pattern animation binding(s).";
	return true;
}

bool_t Client::CValtanPatternAnimationBindingDocument::Validate_RequiredActions(
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
	const std::vector<std::string>& requiredActionIds,
	std::string& outStatus)
{
	if (requiredActionIds.empty() || requiredActionIds.size() > 512u)
	{
		outStatus = "Boss pattern required-action contract is empty or oversized.";
		return false;
	}

	std::unordered_set<std::string> boundActions;
	for (const BOSS_PATTERN_ANIMATION_BINDING& binding : document.Bindings)
	{
		if (!Is_StableToken(binding.strActionId) ||
			!boundActions.insert(binding.strActionId).second)
		{
			outStatus =
				"Boss pattern required-action contract found a duplicate binding.";
			return false;
		}
	}

	std::unordered_set<std::string> requiredActions;
	for (const std::string& actionId : requiredActionIds)
	{
		if (!Is_StableToken(actionId) ||
			!requiredActions.insert(actionId).second ||
			!boundActions.contains(actionId))
		{
			outStatus =
				"Boss pattern binding is missing or repeats a required action.";
			return false;
		}
	}

	outStatus = "Validated " + std::to_string(requiredActions.size()) +
		" required boss pattern action binding(s).";
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
	return Load_BossPatternBindingsFromPath(path,
		expectedBossArchetypeId, availableClips, outDocument, outStatus);
}

bool_t Client::CValtanPatternAnimationBindingDocument::Load_ForAuthoring(
	const std::string_view animationAssetId,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& outDocument,
	std::string& outBaselineSourceBytes,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_Path(animationAssetId);
	return Load_BossPatternBindingsFromPath(
		path, expectedBossArchetypeId, availableClips,
		outDocument, outStatus, &outBaselineSourceBytes);
}

bool_t Client::CValtanPatternAnimationBindingDocument::Save_Atomic(
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& document,
	const std::string_view animationAssetId,
	const std::string_view expectedBossArchetypeId,
	const std::vector<std::string>& availableClips,
	const std::unordered_map<std::string, f32_t>&
		clipSourceDurationSecondsByName,
	const std::string_view expectedBaselineSourceBytes,
	std::string& outCommittedSourceBytes,
	std::string& outStatus)
{
	/* This file is a generated Product. Keeping the legacy function as a hard
	   rejection lets old binary/test callers fail safely while all writable
	   rows move through Valtan.presentation.json -> immutable authoring revision
	   -> projector. No validation or temporary write may precede this guard. */
	(void)document;
	(void)animationAssetId;
	(void)expectedBossArchetypeId;
	(void)availableClips;
	(void)clipSourceDurationSecondsByName;
	(void)expectedBaselineSourceBytes;
	outCommittedSourceBytes.clear();
	outStatus =
		"Valtan.patternbindings.json is a read-only generated Product; edit Data/Valtan/Valtan.presentation.json through the joined authoring revision pipeline.";
	return false;

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

bool_t Client::CValtanPatternEffectBindingDocument::Stage_ValtanPatternTree(
	const std::string_view text,
	const std::filesystem::path& projectDataRoot,
	BOSS_PATTERN_EFFECT_TREE_STAGE& outStage,
	std::string& outStatus)
{
	constexpr std::string_view BOSS_ARCHETYPE_ID = "BOSS_VALTAN";

	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT bindingDocument;
	if (!Parse_Text(text, bindingDocument, outStatus))
		return false;
	std::vector<std::string> availableClips;
	availableClips.reserve(bindingDocument.Bindings.size());
	for (const BOSS_PATTERN_EFFECT_BINDING& binding : bindingDocument.Bindings)
		availableClips.push_back(binding.strRuntimeClipName);
	std::sort(availableClips.begin(), availableClips.end());
	availableClips.erase(std::unique(
		availableClips.begin(), availableClips.end()), availableClips.end());
	if (!Validate(
			bindingDocument, BOSS_ARCHETYPE_ID, availableClips, outStatus))
	{
		return false;
	}
	if (projectDataRoot.empty())
	{
		outStatus = "Valtan Boss Patterns Data root is empty.";
		return false;
	}

	BOSS_PATTERN_EFFECT_TREE_STAGE staged;
	staged.Rows.reserve(bindingDocument.Bindings.size());
	for (const BOSS_PATTERN_EFFECT_BINDING& binding : bindingDocument.Bindings)
	{
		const std::filesystem::path declaredPath{ binding.strEffectDocument };
		auto component = declaredPath.begin();
		if (component == declaredPath.end() ||
			component->generic_string() != "Data")
		{
			outStatus =
				"Valtan Boss Patterns Effect path is not Data-relative: " +
				binding.strEffectDocument;
			return false;
		}
		std::filesystem::path relativePath;
		for (++component; component != declaredPath.end(); ++component)
			relativePath /= *component;
		const std::filesystem::path effectPath =
			(projectDataRoot / relativePath).lexically_normal();
		std::error_code fileError;
		if (relativePath.empty() ||
			!std::filesystem::is_regular_file(effectPath, fileError) || fileError)
		{
			outStatus = "Valtan Boss Patterns Effect document is missing: " +
				effectPath.string();
			return false;
		}

		EFFECT_DOCUMENT_DESC effectDocument;
		std::string effectError;
		if (!CEffectDocumentCodec::Load(
				effectPath, effectDocument, effectError))
		{
			outStatus = "Valtan Boss Patterns Effect document is invalid: " +
				effectError;
			return false;
		}
		if (effectDocument.strEffectAssetId != binding.strEffectAssetId)
		{
			outStatus =
				"Valtan Boss Patterns Effect document ID does not match its binding: " +
				binding.strEffectAssetId;
			return false;
		}

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
	}
	outStage = std::move(staged);
	outStatus = "Staged " + std::to_string(outStage.Rows.size()) +
		" Valtan Boss Pattern Effect(s).";
	return true;
}
