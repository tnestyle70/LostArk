#include "CombatHUDViewModel.h"

#include "DataJson.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>
#include <set>

namespace
{
	using namespace Client;
	constexpr std::uint32_t SERVER_TICK_HZ = 30;

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
		if (value == "DIMENSIONMASTER") return CHARACTER_CLASS_ID::DIMENSIONMASTER;
		return CHARACTER_CLASS_ID::END;
	}
}

Client::CCombatHUDViewModel& Client::CCombatHUDViewModel::Get()
{
	static CCombatHUDViewModel instance;
	return instance;
}

bool Client::CCombatHUDViewModel::Initialize_Definitions()
{
	std::string skillStatus;
	if (!CPlayerSkillCatalog::Load(skillStatus))
	{
		m_strStatus = skillStatus;
		return false;
	}

	DATA_JSON_VALUE bossRoot;
	DATA_JSON_VALUE playerRoot;
	if (!ReadDocument(L"Balance/BossProfiles.json", bossRoot) ||
		!ReadDocument(L"Balance/PlayerProfiles.json", playerRoot))
	{
		m_strStatus = "Missing combat HUD balance document";
		return false;
	}

	std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
		PLAYER_PROFILE_DEFINITION> playerProfiles;
	const DATA_JSON_VALUE* playerValues =
		Required(playerRoot, "players", DATA_JSON_TYPE::ARRAY);
	if (nullptr == playerValues)
		return false;
	for (const DATA_JSON_VALUE& value : playerValues->Get_Array())
	{
		const DATA_JSON_VALUE* characterClass = Required(
			value, "characterClass", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* maximumHp = Required(
			value, "maximumHp", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* maximumResource = Required(
			value, "maximumResource", DATA_JSON_TYPE::NUMBER);
		if (nullptr == characterClass || nullptr == maximumHp ||
			nullptr == maximumResource || maximumHp->Get_Number() <= 0.0 ||
			maximumResource->Get_Number() <= 0.0)
		{
			return false;
		}

		const LostArk::Shared::CHARACTER_CLASS_ID parsedClass =
			ParseCharacterClass(characterClass->Get_String());
		PLAYER_PROFILE_DEFINITION profile{};
		profile.iMaximumHp = static_cast<std::uint32_t>(
			maximumHp->Get_Number());
		profile.iMaximumResource = static_cast<std::uint32_t>(
			maximumResource->Get_Number());
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			parsedClass) || !playerProfiles.emplace(parsedClass, profile).second)
		{
			return false;
		}
	}

	std::unordered_map<std::string, std::string> bossNames;
	const DATA_JSON_VALUE* bossValues =
		Required(bossRoot, "bosses", DATA_JSON_TYPE::ARRAY);
	if (nullptr == bossValues)
		return false;
	for (const DATA_JSON_VALUE& value : bossValues->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "archetypeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		if (nullptr == id || nullptr == name ||
			!bossNames.emplace(id->Get_String(), name->Get_String()).second)
		{
			return false;
		}
	}

	m_PlayerProfiles = std::move(playerProfiles);
	m_BossDisplayNames = std::move(bossNames);
	m_strStatus = "Loaded combat HUD definitions. " + skillStatus;
	return true;
}

bool Client::CCombatHUDViewModel::Apply_CharacterPreview(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	const auto profile = m_PlayerProfiles.find(characterClass);
	if (m_PlayerProfiles.end() == profile)
		return false;

	m_Player = {};
	m_Player.isValid = true;
	m_Player.isPreview = true;
	m_Player.eCharacterClass = characterClass;
	m_Player.iCurrentHp = profile->second.iMaximumHp;
	m_Player.iMaximumHp = profile->second.iMaximumHp;
	m_Player.iCurrentResource = profile->second.iMaximumResource;
	m_Player.iMaximumResource = profile->second.iMaximumResource;
	Build_PlayerSkills(characterClass, 0u, nullptr);
	m_Boss = {};
	return true;
}

void Client::CCombatHUDViewModel::Apply_LocalPlayer(
	const std::uint32_t serverTick,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
	m_Player.isValid = true;
	m_Player.isPreview = false;
	m_Player.eCharacterClass = characterClass;
	m_Player.iServerTick = serverTick;
	m_Player.iCurrentHp = snapshot.iCurrentHp;
	m_Player.iMaximumHp = snapshot.iMaximumHp;
	m_Player.iCurrentResource = snapshot.iCurrentResource;
	m_Player.iMaximumResource = snapshot.iMaximumResource;
	m_Player.eAction = snapshot.eAction;
	Build_PlayerSkills(characterClass, serverTick, &snapshot.Cooldowns);
}

void Client::CCombatHUDViewModel::Build_PlayerSkills(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>* pCooldowns)
{
	m_Player.Skills.clear();
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;

		const LostArk::Shared::SKILL_ID skillId = definition.iSkillId;
		HUD_SKILL_STATE state{};
		state.iSkillId = skillId;
		state.strInputSlot = definition.strInputSlot;
		state.strDisplayName = definition.strDisplayName;
		state.strActionId = definition.strActionId;
		state.iDamage = definition.iDamage;
		state.iCooldownDurationTicks =
			(definition.iCooldownMs * SERVER_TICK_HZ + 999u) / 1000u;
		state.iCooldownEndTick = serverTick;
		if (nullptr != pCooldowns)
		{
			const auto cooldown = std::find_if(
				pCooldowns->begin(), pCooldowns->end(),
				[skillId](const LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT& value)
				{ return value.iSkillId == skillId; });
			if (pCooldowns->end() != cooldown)
				state.iCooldownEndTick = cooldown->iCooldownEndTick;
		}
		m_Player.Skills.push_back(std::move(state));
	}
	std::sort(m_Player.Skills.begin(), m_Player.Skills.end(),
		[](const HUD_SKILL_STATE& left, const HUD_SKILL_STATE& right)
		{ return left.strInputSlot < right.strInputSlot; });
}

void Client::CCombatHUDViewModel::Apply_Boss(
	const std::string& archetypeId,
	const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot)
{
	m_Boss.isValid = true;
	m_Boss.strArchetypeId = archetypeId;
	const auto name = m_BossDisplayNames.find(archetypeId);
	m_Boss.strDisplayName = m_BossDisplayNames.end() == name ?
		archetypeId : name->second;
	m_Boss.iCurrentHp = snapshot.iCurrentHp;
	m_Boss.iMaximumHp = snapshot.iMaximumHp;
	m_Boss.iPhase = snapshot.iPhase;
	m_Boss.eAction = snapshot.eAction;
	m_Boss.strActionId = snapshot.strActionId;
}

void Client::CCombatHUDViewModel::Reset_RuntimeState()
{
	m_Player = {};
	m_Boss = {};
}
