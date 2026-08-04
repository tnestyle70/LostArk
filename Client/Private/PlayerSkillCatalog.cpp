#include "PlayerSkillCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

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

	LostArk::Shared::CHARACTER_CLASS_ID ParseCharacterClass(
		const std::string& value)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (value == "LANCE_MASTER") return CHARACTER_CLASS_ID::LANCE_MASTER;
		if (value == "GUNSLINGER") return CHARACTER_CLASS_ID::GUNSLINGER;
		if (value == "SLAYER") return CHARACTER_CLASS_ID::SLAYER;
		if (value == "ARTIST") return CHARACTER_CLASS_ID::ARTIST;
		if (value == "DIMENSIONIST") return CHARACTER_CLASS_ID::DIMENSIONIST;
		return CHARACTER_CLASS_ID::END;
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
		const DATA_JSON_VALUE* amount = Required(
			value, "amount", DATA_JSON_TYPE::NUMBER);
		if (nullptr == id || nullptr == amount || amount->Get_Number() <= 0.0 ||
			!damages.emplace(id->Get_String(),
				static_cast<std::uint32_t>(amount->Get_Number())).second)
		{
			outStatus = "DamageProfiles.json has an invalid or duplicate profile";
			return false;
		}
	}

	/* Staged into a local first: a document that fails halfway must leave the
	catalog on its previous contents rather than half of the new ones. */
	std::vector<PLAYER_SKILL_DEFINITION> skills;
	std::unordered_map<std::string, std::string> claimedSlots;
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
		const DATA_JSON_VALUE* cooldown = Required(value, "cooldownMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* damageId = Required(
			value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		if (nullptr == id || nullptr == characterClass || nullptr == slot ||
			nullptr == name || nullptr == action ||
			nullptr == cooldown || nullptr == damageId)
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
		definition.iCooldownMs = static_cast<std::uint32_t>(cooldown->Get_Number());
		const auto damage = damages.find(damageId->Get_String());
		if (damages.end() == damage)
		{
			outStatus = "PlayerSkills.json references an unknown damage profile";
			return false;
		}
		definition.iDamage = damage->second;

		if (LostArk::Shared::INVALID_SKILL_ID == definition.iSkillId ||
			LostArk::Shared::CHARACTER_CLASS_ID::END == definition.eCharacterClass ||
			0u == definition.iCooldownMs || definition.strInputSlot.empty())
		{
			outStatus = "PlayerSkills.json has an invalid skill id, class, cooldown or slot";
			return false;
		}

		/* One class cannot bind two skills to the same slot: the input controller
		resolves a key press through exactly one skill, so a duplicate would make
		the winner depend on document order. */
		const std::string slotKey =
			std::to_string(static_cast<std::uint32_t>(definition.eCharacterClass)) +
			":" + definition.strInputSlot;
		if (!claimedSlots.emplace(slotKey, definition.strDisplayName).second)
		{
			outStatus = "PlayerSkills.json binds one class slot twice: " + slotKey;
			return false;
		}
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
	const std::string& inputSlot)
{
	for (const PLAYER_SKILL_DEFINITION& definition : g_Skills)
	{
		if (definition.eCharacterClass == characterClass &&
			definition.strInputSlot == inputSlot)
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
