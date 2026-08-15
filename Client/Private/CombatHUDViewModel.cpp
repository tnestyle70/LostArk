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
		const DATA_JSON_VALUE* attackPower = Required(
			value, "attackPower", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* defaultStance = Required(
			value, "defaultStance", DATA_JSON_TYPE::STRING);
		LostArk::Shared::PLAYER_STANCE_ID parsedStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		if (nullptr == characterClass || nullptr == maximumHp ||
			nullptr == maximumResource || maximumHp->Get_Number() <= 0.0 ||
			maximumResource->Get_Number() <= 0.0 ||
			nullptr == attackPower || attackPower->Get_Number() <= 0.0 ||
			nullptr == defaultStance ||
			!ParseStance(defaultStance->Get_String(), parsedStance))
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
		profile.iAttackPower = static_cast<std::uint32_t>(
			attackPower->Get_Number());
		profile.eDefaultStance = parsedStance;
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			parsedClass) || !playerProfiles.emplace(parsedClass, profile).second)
		{
			return false;
		}
	}

	std::unordered_map<std::string, BOSS_PROFILE_DEFINITION> bossProfiles;
	const DATA_JSON_VALUE* bossValues =
		Required(bossRoot, "bosses", DATA_JSON_TYPE::ARRAY);
	if (nullptr == bossValues)
		return false;
	for (const DATA_JSON_VALUE& value : bossValues->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "archetypeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* maximumHealthBars = Required(
			value, "maximumHealthBars", DATA_JSON_TYPE::NUMBER);
		if (nullptr == id || nullptr == name || nullptr == maximumHealthBars ||
			maximumHealthBars->Get_Number() < 1.0)
		{
			return false;
		}
		BOSS_PROFILE_DEFINITION profile{};
		profile.strDisplayName = name->Get_String();
		profile.iMaximumHealthBars = static_cast<std::uint32_t>(
			maximumHealthBars->Get_Number());
		if (!bossProfiles.emplace(id->Get_String(), std::move(profile)).second)
			return false;
	}

	m_PlayerProfiles = std::move(playerProfiles);
	m_BossProfiles = std::move(bossProfiles);
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
	m_Player.eStance = profile->second.eDefaultStance;
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
	m_Player.iCurrentIdentity = snapshot.iCurrentIdentity;
	m_Player.iMaximumIdentity = snapshot.iMaximumIdentity;
	m_Player.isCombatReady = snapshot.isCombatReady;
	m_Player.eAction = snapshot.eAction;
	m_Player.eStance = snapshot.eStance;
	Build_PlayerSkills(characterClass, serverTick, &snapshot.Cooldowns);
}

void Client::CCombatHUDViewModel::Build_PlayerSkills(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>* pCooldowns)
{
	m_Player.Skills.clear();
	/* Same formula as the server's Resolve_Damage, for display only: the number a
	tooltip shows has to match the number the snapshot will subtract. */
	const auto ownProfile = m_PlayerProfiles.find(characterClass);
	const std::uint64_t attackPower = m_PlayerProfiles.end() == ownProfile ?
		0ull : ownProfile->second.iAttackPower;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;
		/* The basic attack is always available and has no cooldown to count
		down, so it takes no quick-slot tile. The test is the slot, not the
		kind: Artist R is a COMBO that does hold a tile. */
		if ("LMB" == definition.strInputSlot)
			continue;
		if (LostArk::Shared::PLAYER_STANCE_ID::NONE != definition.eRequiredStance &&
			definition.eRequiredStance != m_Player.eStance)
		{
			continue;
		}

		const LostArk::Shared::SKILL_ID skillId = definition.iSkillId;
		HUD_SKILL_STATE state{};
		state.iSkillId = skillId;
		state.strInputSlot = definition.strInputSlot;
		state.strDisplayName = definition.strDisplayName;
		state.strActionId = definition.strActionId;
		state.iDamage = static_cast<std::uint32_t>(
			attackPower * definition.iDamageRatePercent / 100ull);
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
	const auto profile = m_BossProfiles.find(archetypeId);
	m_Boss.strDisplayName = m_BossProfiles.end() == profile ?
		archetypeId : profile->second.strDisplayName;
	m_Boss.iMaximumHealthBars = m_BossProfiles.end() == profile ?
		0u : profile->second.iMaximumHealthBars;
	m_Boss.iCurrentHp = snapshot.iCurrentHp;
	m_Boss.iMaximumHp = snapshot.iMaximumHp;
	m_Boss.iPhase = snapshot.iPhase;
	m_Boss.eAction = snapshot.eAction;
	m_Boss.strActionId = snapshot.strActionId;
	m_Boss.strPatternId = snapshot.strPatternId;
	m_Boss.iPatternSequence = snapshot.iPatternSequence;
	m_Boss.iPatternStageIndex = snapshot.iPatternStageIndex;
}

void Client::CCombatHUDViewModel::Apply_DamageEvents(
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::DAMAGE_EVENT>& events)
{
	constexpr std::size_t MAX_RETAINED_DAMAGE_EVENTS = 128u;
	for (const LostArk::Shared::DAMAGE_EVENT& event : events)
	{
		HUD_DAMAGE_EVENT retained{};
		retained.iServerTick = serverTick;
		retained.Event = event;
		m_DamageEvents.push_back(std::move(retained));
	}
	if (m_DamageEvents.size() > MAX_RETAINED_DAMAGE_EVENTS)
	{
		m_DamageEvents.erase(
			m_DamageEvents.begin(),
			m_DamageEvents.begin() +
				(m_DamageEvents.size() - MAX_RETAINED_DAMAGE_EVENTS));
	}
}

void Client::CCombatHUDViewModel::Reset_RuntimeState()
{
	m_Player = {};
	m_Boss = {};
	m_DamageEvents.clear();
	m_iEstherGauge = 0;
	m_iEstherGaugeMaximum = 0;
}
