#include "PlayerSkillCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <fstream>
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

	LostArk::Shared::CHARACTER_CLASS_ID ParseCharacterClass(
		const std::string& value)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (value == "LANCE_MASTER") return CHARACTER_CLASS_ID::LANCE_MASTER;
		if (value == "GUNSLINGER") return CHARACTER_CLASS_ID::GUNSLINGER;
		if (value == "SLAYER") return CHARACTER_CLASS_ID::SLAYER;
		if (value == "ARTIST") return CHARACTER_CLASS_ID::ARTIST;
		if (value == "DIMENSIONMASTER") return CHARACTER_CLASS_ID::DIMENSIONMASTER;
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
}

bool Client::CPlayerSkillCatalog::Load(std::string& outStatus)
{
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE skillRoot;
	if (!ReadDocument(L"Balance/DamageProfiles.json", damageRoot) ||
		!ReadDocument(L"Balance/PlayerSkills.json", skillRoot))
	{
		outStatus = "Missing player skill balance document";
		return false;
	}
	if (!HasFormatVersion(damageRoot, 2.0) || !HasFormatVersion(skillRoot, 2.0))
	{
		outStatus = "Player skill balance document is not formatVersion 2";
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
		const DATA_JSON_VALUE* characterClass = Required(
			value, "characterClass", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* slot = Required(value, "inputSlot", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* action = Required(value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* effect = value.Find("effectId");
		const DATA_JSON_VALUE* cooldown = Required(value, "cooldownMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* resourceCost = Required(
			value, "resourceCost", DATA_JSON_TYPE::NUMBER);
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
			resourceCost->Get_Number() < 0.0 ||
			nullptr == damageId || nullptr == kind ||
			nullptr == requiredStance || nullptr == setsStance ||
			nullptr == comboStages)
		{
			outStatus = "PlayerSkills.json is missing a required skill field";
			return false;
		}

		PLAYER_SKILL_DEFINITION definition{};
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
		definition.iComboStageCount = comboStages->Get_Array().size();

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
					3u != definition.iComboStageCount)))
		{
			outStatus = "PlayerSkills.json has an invalid skill id, class, cooldown or slot";
			return false;
		}

		/* One class cannot bind two skills to the same slot in the same stance:
		the input controller resolves a key press through exactly one skill, so a
		duplicate would make the winner depend on document order. A stance-free
		(NONE) binding claims the whole slot since nothing then distinguishes it
		from a stance-specific one at resolve time. */
		const std::string slotKey =
			std::to_string(static_cast<std::uint32_t>(definition.eCharacterClass)) +
			":" + definition.strInputSlot;
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
	const LostArk::Shared::PLAYER_STANCE_ID stance)
{
	for (const PLAYER_SKILL_DEFINITION& definition : g_Skills)
	{
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
