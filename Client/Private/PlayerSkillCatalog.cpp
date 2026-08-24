#include "PlayerSkillCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <unordered_map>

namespace
{
	std::vector<Client::PLAYER_SKILL_DEFINITION> g_Skills;

	bool ReadDocument(
		const std::filesystem::path& relativePath,
		DATA_JSON_VALUE& output)
	{
		const std::filesystem::path path = CProjectDataRoot::Resolve(relativePath);
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return false;
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		std::string error;
		return CDataJson::Parse(text, output, error) && output.Is_Object();
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool IsStableEffectId(const std::string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		return std::all_of(value.begin(), value.end(), [](const char character)
		{
			return std::isalnum(static_cast<unsigned char>(character)) ||
				character == '_' || character == '-' || character == '.';
		});
	}

	bool HasExactProperties(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char*> names)
	{
		if (!value.Is_Object() || value.Get_Object().size() != names.size())
			return false;
		for (const char* name : names)
		{
			if (nullptr == value.Find(name))
				return false;
		}
		return true;
	}

	bool IsEffectTextureAssetId(const std::string& value)
	{
		if (value.empty() || value.size() > 260u ||
			0u != value.rfind("Effect/", 0u) ||
			value.size() < 4u || value.substr(value.size() - 4u) != ".dds" ||
			std::string::npos != value.find('\\') ||
			std::string::npos != value.find(':'))
		{
			return false;
		}
		std::size_t begin = 0u;
		while (begin <= value.size())
		{
			const std::size_t end = value.find('/', begin);
			if (value.substr(begin, end - begin) == "..")
				return false;
			if (std::string::npos == end)
				break;
			begin = end + 1u;
		}
		return true;
	}

	bool ReadFiniteFloat(const DATA_JSON_VALUE* value, float& output)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			std::abs(value->Get_Number()) >
				static_cast<double>((std::numeric_limits<float>::max)()))
		{
			return false;
		}
		output = static_cast<float>(value->Get_Number());
		return std::isfinite(output);
	}

	bool ReadTint(const DATA_JSON_VALUE* value, float4_t& output)
	{
		if (nullptr == value || !value->Is_Array() ||
			4u != value->Get_Array().size())
		{
			return false;
		}
		float components[4]{};
		for (std::size_t index = 0u; index < 4u; ++index)
		{
			if (!ReadFiniteFloat(&value->Get_Array()[index], components[index]) ||
				components[index] < 0.f || components[index] > 1.f)
			{
				return false;
			}
		}
		output = { components[0], components[1], components[2], components[3] };
		return true;
	}

	LostArk::Shared::CHARACTER_CLASS_ID ParseCharacterClass(
		const std::string& value)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (value == "LANCE_MASTER") return CHARACTER_CLASS_ID::LANCE_MASTER;
		if (value == "GUNSLINGER") return CHARACTER_CLASS_ID::GUNSLINGER;
		if (value == "SLAYER") return CHARACTER_CLASS_ID::SLAYER;
		if (value == "ARTIST") return CHARACTER_CLASS_ID::ARTIST;
		if (value == "DIMENSIONMASTER") return CHARACTER_CLASS_ID::DIMENSIONMASTER;
		if (value == "WARLORD") return CHARACTER_CLASS_ID::WARLORD;
		return CHARACTER_CLASS_ID::END;
	}

	bool ParseStance(
		const std::string& value,
		LostArk::Shared::PLAYER_STANCE_ID& output)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		if (value == "NONE") output = PLAYER_STANCE_ID::NONE;
		else if (value == "LANCE_MASTER_LONG_SPEAR")
			output = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		else if (value == "LANCE_MASTER_SHORT_SPEAR")
			output = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		else if (value == "WARLORD_NORMAL")
			output = PLAYER_STANCE_ID::WARLORD_NORMAL;
		else if (value == "WARLORD_DEFENSE")
			output = PLAYER_STANCE_ID::WARLORD_DEFENSE;
		else
			return false;
		return true;
	}

	/* A stale document must fail on its version, not on whichever field the new
	schema happened to rename; a missing-field error would send the editor to the
	wrong fix. */
	bool HasFormatVersion(
		const DATA_JSON_VALUE& document,
		const double expected)
	{
		const DATA_JSON_VALUE* version =
			document.Find("formatVersion");
		return nullptr != version &&
			version->Get_Type() == DATA_JSON_TYPE::NUMBER &&
			version->Get_Number() == expected;
	}

	bool ReadU32(
		const DATA_JSON_VALUE* value,
		std::uint32_t& output)
	{
		if (nullptr == value || value->Get_Type() != DATA_JSON_TYPE::NUMBER)
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(
				(std::numeric_limits<std::uint32_t>::max)()) ||
			std::floor(number) != number)
		{
			return false;
		}
		output = static_cast<std::uint32_t>(number);
		return true;
	}

	bool ReadComboStage(
		const DATA_JSON_VALUE& value,
		Client::PLAYER_COMBO_STAGE_TIMING& output)
	{
		if (!HasExactProperties(value,
			{ "actionDurationMs", "hitTimeMs", "comboAdvanceMs",
				"inputOpenMs", "inputCloseMs" }))
		{
			return false;
		}

		return ReadU32(value.Find("actionDurationMs"), output.iActionDurationMs) &&
			ReadU32(value.Find("hitTimeMs"), output.iHitTimeMs) &&
			ReadU32(value.Find("comboAdvanceMs"), output.iComboAdvanceMs) &&
			ReadU32(value.Find("inputOpenMs"), output.iInputOpenMs) &&
			ReadU32(value.Find("inputCloseMs"), output.iInputCloseMs);
	}

	bool HasValidStageBounds(
		const Client::PLAYER_COMBO_STAGE_TIMING& stage)
	{
		return 0u != stage.iActionDurationMs &&
			stage.iHitTimeMs <= stage.iComboAdvanceMs &&
			stage.iComboAdvanceMs <= stage.iActionDurationMs;
	}
}

bool Client::CPlayerSkillCatalog::Load(std::string& outStatus)
{
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE targetingRoot;
	if (!ReadDocument(L"Balance/DamageProfiles.json", damageRoot) ||
		!ReadDocument(L"Balance/PlayerSkills.json", skillRoot) ||
		!ReadDocument(L"Balance/PlayerSkillTargeting.json", targetingRoot))
	{
		outStatus = "Missing player skill balance document";
		return false;
	}
	if (!HasFormatVersion(damageRoot, 2.0) || !HasFormatVersion(skillRoot, 3.0) ||
		!HasFormatVersion(targetingRoot, 1.0))
	{
		outStatus = "Player skill balance document version mismatch";
		return false;
	}

	std::unordered_map<std::string, std::uint32_t> damages;
	const DATA_JSON_VALUE* profiles =
		Required(damageRoot, "profiles", DATA_JSON_TYPE::ARRAY);
	if (nullptr == profiles)
	{
		outStatus = "DamageProfiles.json has no profiles array";
		return false;
	}
	for (const DATA_JSON_VALUE& value : profiles->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(
			value, "damageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* ratePercent = Required(
			value, "damageRatePercent", DATA_JSON_TYPE::NUMBER);
		if (nullptr == id || nullptr == ratePercent ||
			ratePercent->Get_Number() <= 0.0 ||
			!damages.emplace(id->Get_String(),
				static_cast<std::uint32_t>(ratePercent->Get_Number())).second)
		{
			outStatus = "DamageProfiles.json has an invalid or duplicate profile";
			return false;
		}
	}

	/* Staged into a local first: a document that fails halfway must leave the
	catalog on its previous contents rather than half of the new ones. */
	std::vector<PLAYER_SKILL_DEFINITION> skills;
	std::unordered_map<std::string, std::vector<LostArk::Shared::PLAYER_STANCE_ID>>
		claimedSlots;
	const DATA_JSON_VALUE* skillValues =
		Required(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	if (nullptr == skillValues)
	{
		outStatus = "PlayerSkills.json has no skills array";
		return false;
	}
	for (const DATA_JSON_VALUE& value : skillValues->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "skillId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* staggerDamage = Required(
			value, "staggerDamage", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* partDamage = Required(
			value, "partDamage", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* counterPower = Required(
			value, "counterPower", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* characterClass = Required(
			value, "characterClass", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* slot = Required(value, "inputSlot", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* action = Required(value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* effect = value.Find("effectId");
		const DATA_JSON_VALUE* cooldown = Required(value, "cooldownMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* resourceCost = Required(
			value, "resourceCost", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* maximumRange = Required(
			value, "maximumRange", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* damageId = Required(
			value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* kind = Required(value, "skillKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* requiredStance = Required(
			value, "requiredStance", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* setsStance = Required(
			value, "setsStance", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* comboStages = Required(
			value, "comboStages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == id || nullptr == characterClass || nullptr == slot ||
			nullptr == name || nullptr == action ||
			nullptr == cooldown || nullptr == resourceCost ||
			nullptr == maximumRange ||
			resourceCost->Get_Number() < 0.0 ||
			nullptr == damageId || nullptr == kind ||
			nullptr == requiredStance || nullptr == setsStance ||
			nullptr == comboStages)
		{
			outStatus = "PlayerSkills.json is missing a required skill field";
			return false;
		}

		PLAYER_SKILL_DEFINITION definition{};
		if (!ReadU32(staggerDamage, definition.iStaggerDamage) ||
			!ReadU32(partDamage, definition.iPartDamage) ||
			!ReadU32(counterPower, definition.iCounterPower))
		{
			outStatus = "PlayerSkills.json has an invalid combat trait";
			return false;
		}
		definition.iSkillId = static_cast<LostArk::Shared::SKILL_ID>(id->Get_Number());
		definition.eCharacterClass = ParseCharacterClass(characterClass->Get_String());
		definition.strInputSlot = slot->Get_String();
		definition.strDisplayName = name->Get_String();
		definition.strActionId = action->Get_String();
		if (nullptr != effect)
		{
			if (!effect->Is_String() ||
				(!effect->Get_String().empty() &&
					!IsStableEffectId(effect->Get_String())))
			{
				outStatus = "PlayerSkills.json has an invalid effectId";
				return false;
			}
			definition.strEffectId = effect->Get_String();
		}
		definition.iCooldownMs = static_cast<std::uint32_t>(cooldown->Get_Number());
		definition.iResourceCost =
			static_cast<std::uint32_t>(resourceCost->Get_Number());
		if (!ReadFiniteFloat(maximumRange, definition.fMaximumRange) ||
			definition.fMaximumRange < 0.f)
		{
			outStatus = "PlayerSkills.json has an invalid maximumRange";
			return false;
		}
		if (!damageId->Get_String().empty())
		{
			const auto damage = damages.find(damageId->Get_String());
			if (damages.end() == damage)
			{
				outStatus = "PlayerSkills.json references an unknown damage profile";
				return false;
			}
			definition.iDamageRatePercent = damage->second;
		}

		const std::string& kindText = kind->Get_String();
		if ("ACTIVE" == kindText)
			definition.eSkillKind = LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		else if ("COMBO" == kindText)
			definition.eSkillKind = LostArk::Shared::PLAYER_SKILL_KIND::COMBO;
		else if ("HOLD" == kindText)
			definition.eSkillKind = LostArk::Shared::PLAYER_SKILL_KIND::HOLD;
		else if ("COUNTER" == kindText)
			definition.eSkillKind = LostArk::Shared::PLAYER_SKILL_KIND::COUNTER;
		else if ("STANDUP" == kindText)
			definition.eSkillKind = LostArk::Shared::PLAYER_SKILL_KIND::STANDUP;
		else
		{
			outStatus = "PlayerSkills.json has an unknown skillKind";
			return false;
		}
		if (!ParseStance(requiredStance->Get_String(), definition.eRequiredStance) ||
			!ParseStance(setsStance->Get_String(), definition.eSetsStance))
		{
			outStatus = "PlayerSkills.json has an unknown stance";
			return false;
		}
		definition.ComboStages.reserve(comboStages->Get_Array().size());
		for (const DATA_JSON_VALUE& stageValue : comboStages->Get_Array())
		{
			PLAYER_COMBO_STAGE_TIMING stage{};
			if (!ReadComboStage(stageValue, stage))
			{
				outStatus = "PlayerSkills.json has an invalid combo stage field";
				return false;
			}
			definition.ComboStages.push_back(stage);
		}
		definition.iComboStageCount = definition.ComboStages.size();

		/* A combo runs off its stages and carries no cooldown, so only an ACTIVE
		skill has to declare one. */
		if (LostArk::Shared::INVALID_SKILL_ID == definition.iSkillId ||
			LostArk::Shared::CHARACTER_CLASS_ID::END == definition.eCharacterClass ||
			definition.strInputSlot.empty() ||
			(LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE == definition.eSkillKind &&
				(0u == definition.iCooldownMs || 0u != definition.iComboStageCount)) ||
			(LostArk::Shared::PLAYER_SKILL_KIND::COMBO == definition.eSkillKind &&
				(definition.iComboStageCount < 2u ||
					definition.iComboStageCount > 8u)) ||
			(LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition.eSkillKind &&
				(0u == definition.iCooldownMs ||
					3u != definition.iComboStageCount)) ||
			(LostArk::Shared::PLAYER_SKILL_KIND::STANDUP == definition.eSkillKind &&
				(0u == definition.iCooldownMs || 0u != definition.iComboStageCount)))
		{
			outStatus = "PlayerSkills.json has an invalid skill id, class, cooldown or slot";
			return false;
		}

		if (!std::all_of(
			definition.ComboStages.begin(), definition.ComboStages.end(),
			HasValidStageBounds))
		{
			outStatus = "PlayerSkills.json has invalid combo stage timing bounds";
			return false;
		}

		if (LostArk::Shared::PLAYER_SKILL_KIND::COMBO == definition.eSkillKind)
		{
			for (std::size_t stageIndex = 0u;
				stageIndex < definition.ComboStages.size(); ++stageIndex)
			{
				const PLAYER_COMBO_STAGE_TIMING& stage =
					definition.ComboStages[stageIndex];
				const bool isFinalStage =
					stageIndex + 1u == definition.ComboStages.size();
				if ((isFinalStage &&
						(stage.iComboAdvanceMs != stage.iActionDurationMs ||
							0u != stage.iInputOpenMs || 0u != stage.iInputCloseMs)) ||
					(!isFinalStage &&
						(stage.iInputOpenMs >= stage.iInputCloseMs ||
							stage.iInputCloseMs > stage.iActionDurationMs)))
				{
					outStatus = "PlayerSkills.json has an invalid combo input window";
					return false;
				}
			}
		}
		else if (LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
			definition.eSkillKind)
		{
			if (2u != definition.ComboStages.size() || 0u == definition.iCooldownMs)
			{
				outStatus = "PlayerSkills.json has an invalid counter stage count";
				return false;
			}
			const PLAYER_COMBO_STAGE_TIMING& guard = definition.ComboStages[0u];
			const PLAYER_COMBO_STAGE_TIMING& counter = definition.ComboStages[1u];
			if (0u != guard.iHitTimeMs ||
				guard.iInputOpenMs >= guard.iInputCloseMs ||
				guard.iInputCloseMs > guard.iActionDurationMs ||
				0u == counter.iHitTimeMs || 0u != counter.iInputOpenMs ||
				0u != counter.iInputCloseMs)
			{
				outStatus = "PlayerSkills.json has an invalid counter stage timing";
				return false;
			}
		}
		else if (LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition.eSkillKind)
		{
			for (std::size_t stageIndex = 0u;
				stageIndex < definition.ComboStages.size(); ++stageIndex)
			{
				const PLAYER_COMBO_STAGE_TIMING& stage =
					definition.ComboStages[stageIndex];
				const bool isFinalStage =
					stageIndex + 1u == definition.ComboStages.size();
				if (0u != stage.iInputOpenMs || 0u != stage.iInputCloseMs ||
					(isFinalStage != (0u != stage.iHitTimeMs)))
				{
					outStatus = "PlayerSkills.json has an invalid hold stage timing";
					return false;
				}
			}
		}

		/* One class cannot bind two skills to the same slot in the same stance:
		the input controller resolves a key press through exactly one skill, so a
		duplicate would make the winner depend on document order. A stance-free
		(NONE) binding claims the whole slot since nothing then distinguishes it
		from a stance-specific one at resolve time. A STANDUP skill is resolved by
		the KNOCKDOWN action instead of a stance, so it claims its own per-slot
		domain next to the normal claims. */
		const std::string slotKey =
			std::to_string(static_cast<std::uint32_t>(definition.eCharacterClass)) +
			":" + definition.strInputSlot +
			(LostArk::Shared::PLAYER_SKILL_KIND::STANDUP ==
				definition.eSkillKind ? "#STANDUP" : "");
		std::vector<LostArk::Shared::PLAYER_STANCE_ID>& claimedStances =
			claimedSlots[slotKey];
		const bool slotConflicts = std::any_of(
			claimedStances.begin(), claimedStances.end(),
			[&definition](const LostArk::Shared::PLAYER_STANCE_ID claimed)
			{
				return claimed == definition.eRequiredStance ||
					LostArk::Shared::PLAYER_STANCE_ID::NONE == claimed ||
					LostArk::Shared::PLAYER_STANCE_ID::NONE ==
						definition.eRequiredStance;
			});
		if (slotConflicts)
		{
			outStatus = "PlayerSkills.json binds one class slot twice: " + slotKey;
			return false;
		}
		claimedStances.push_back(definition.eRequiredStance);
		for (const PLAYER_SKILL_DEFINITION& existing : skills)
		{
			if (existing.iSkillId == definition.iSkillId)
			{
				outStatus = "PlayerSkills.json repeats a skill id";
				return false;
			}
		}

		skills.push_back(std::move(definition));
	}

	if (!HasExactProperties(targetingRoot, { "schema", "formatVersion", "skills" }))
	{
		outStatus = "PlayerSkillTargeting.json header fields are invalid";
		return false;
	}
	const DATA_JSON_VALUE* targetingSchema = Required(
		targetingRoot, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* targetingRows = Required(
		targetingRoot, "skills", DATA_JSON_TYPE::ARRAY);
	if (nullptr == targetingSchema ||
		targetingSchema->Get_String() != "lostark.player-skill-targeting" ||
		nullptr == targetingRows)
	{
		outStatus = "PlayerSkillTargeting.json header is invalid";
		return false;
	}
	std::vector<LostArk::Shared::SKILL_ID> claimedTargetSkills;
	for (const DATA_JSON_VALUE& value : targetingRows->Get_Array())
	{
		if (!HasExactProperties(value,
			{ "skillId", "targetingKind", "maximumRange", "requiresWalkable",
			  "rangePreview", "targetPreview" }))
		{
			outStatus = "PlayerSkillTargeting.json row fields are invalid";
			return false;
		}
		std::uint32_t skillId = 0u;
		float maximumRange = 0.f;
		const DATA_JSON_VALUE* kind = Required(
			value, "targetingKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* walkable = Required(
			value, "requiresWalkable", DATA_JSON_TYPE::BOOLEAN);
		if (!ReadU32(value.Find("skillId"), skillId) || 0u == skillId ||
			!ReadFiniteFloat(value.Find("maximumRange"), maximumRange) ||
			maximumRange <= 0.f || nullptr == kind ||
			kind->Get_String() != "GROUND_POINT" || nullptr == walkable ||
			!walkable->Get_Boolean() ||
			std::find(claimedTargetSkills.begin(), claimedTargetSkills.end(), skillId) !=
				claimedTargetSkills.end())
		{
			outStatus = "PlayerSkillTargeting.json row is invalid or duplicated";
			return false;
		}
		auto owner = std::find_if(skills.begin(), skills.end(),
			[skillId](const PLAYER_SKILL_DEFINITION& skill)
			{
				return skill.iSkillId == skillId;
			});
		if (skills.end() == owner ||
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE != owner->eSkillKind ||
			std::abs(owner->fMaximumRange - maximumRange) > 0.0001f)
		{
			outStatus = "PlayerSkillTargeting.json references a non-active skill";
			return false;
		}
		auto parsePreview = [&outStatus, skillId](
			const DATA_JSON_VALUE* preview,
			PLAYER_SKILL_TARGET_PREVIEW& output) -> bool
		{
			if (nullptr == preview || !HasExactProperties(*preview,
				{ "assetId", "diameter", "coverageChannel", "validTint",
				  "invalidTint", "assetIdentityBasis", "usageBasis",
				  "sourceEvidence" }))
			{
				outStatus = "PlayerSkillTargeting.json preview fields are invalid";
				return false;
			}
			const DATA_JSON_VALUE* assetId = Required(
				*preview, "assetId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* coverage = Required(
				*preview, "coverageChannel", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* identityBasis = Required(
				*preview, "assetIdentityBasis", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* usageBasis = Required(
				*preview, "usageBasis", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* evidence = Required(
				*preview, "sourceEvidence", DATA_JSON_TYPE::STRING);
			if (nullptr == assetId || !IsEffectTextureAssetId(assetId->Get_String()) ||
				!ReadFiniteFloat(preview->Find("diameter"), output.fDiameter) ||
				output.fDiameter <= 0.f || nullptr == coverage ||
				coverage->Get_String() != "R" ||
				!ReadTint(preview->Find("validTint"), output.vValidTint) ||
				!ReadTint(preview->Find("invalidTint"), output.vInvalidTint) ||
				nullptr == identityBasis || nullptr == usageBasis || nullptr == evidence ||
				(identityBasis->Get_String() != "SOURCE_EXTRACTED" &&
				 identityBasis->Get_String() != "RUNTIME_RESOURCE") ||
				usageBasis->Get_String() != "PROJECT_TUNED" ||
				(identityBasis->Get_String() == "SOURCE_EXTRACTED") !=
					!evidence->Get_String().empty())
			{
				outStatus = "PlayerSkillTargeting.json preview semantics are invalid for " +
					std::to_string(skillId);
				return false;
			}
			output.strAssetId = assetId->Get_String();
			output.strAssetIdentityBasis = identityBasis->Get_String();
			output.strUsageBasis = usageBasis->Get_String();
			output.strSourceEvidence = evidence->Get_String();
			return true;
		};
		if (!parsePreview(value.Find("rangePreview"), owner->RangePreview) ||
			!parsePreview(value.Find("targetPreview"), owner->TargetPreview) ||
			std::abs(owner->RangePreview.fDiameter - maximumRange * 2.f) > 0.0001f)
		{
			return false;
		}
		owner->eTargetIntent =
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT;
		owner->fTargetMaximumRange = maximumRange;
		owner->requiresWalkableTarget = true;
		claimedTargetSkills.push_back(skillId);
	}

	g_Skills = std::move(skills);
	outStatus = "Loaded " + std::to_string(g_Skills.size()) + " player skills";
	return true;
}

const std::vector<Client::PLAYER_SKILL_DEFINITION>&
Client::CPlayerSkillCatalog::Get_Skills()
{
	return g_Skills;
}

const Client::PLAYER_SKILL_DEFINITION* Client::CPlayerSkillCatalog::Find_BySlot(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string& inputSlot,
	const LostArk::Shared::PLAYER_STANCE_ID stance,
	const bool isKnockedDown)
{
	for (const PLAYER_SKILL_DEFINITION& definition : g_Skills)
	{
		/* A knocked-down player's press means the slot's STANDUP skill and
		nothing else; on its feet the same key means everything but. */
		if ((LostArk::Shared::PLAYER_SKILL_KIND::STANDUP ==
			definition.eSkillKind) != isKnockedDown)
		{
			continue;
		}
		if (definition.eCharacterClass == characterClass &&
			definition.strInputSlot == inputSlot &&
			(LostArk::Shared::PLAYER_STANCE_ID::NONE ==
				definition.eRequiredStance ||
				definition.eRequiredStance == stance))
		{
			return &definition;
		}
	}
	return nullptr;
}

const Client::PLAYER_SKILL_DEFINITION* Client::CPlayerSkillCatalog::Find_ById(
	const LostArk::Shared::SKILL_ID skillId)
{
	for (const PLAYER_SKILL_DEFINITION& definition : g_Skills)
	{
		if (definition.iSkillId == skillId)
			return &definition;
	}
	return nullptr;
}
