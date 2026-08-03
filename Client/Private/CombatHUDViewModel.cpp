#include "CombatHUDViewModel.h"

#include "DataJson.h"
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
}

Client::CCombatHUDViewModel& Client::CCombatHUDViewModel::Get()
{
	static CCombatHUDViewModel instance;
	return instance;
}

bool Client::CCombatHUDViewModel::Initialize_Definitions()
{
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE bossRoot;
	if (!ReadDocument(L"Balance/DamageProfiles.json", damageRoot) ||
		!ReadDocument(L"Balance/PlayerSkills.json", skillRoot) ||
		!ReadDocument(L"Balance/BossProfiles.json", bossRoot))
	{
		m_strStatus = "Missing combat HUD balance document";
		return false;
	}

	std::unordered_map<std::string, std::uint32_t> damages;
	const DATA_JSON_VALUE* profiles =
		Required(damageRoot, "profiles", DATA_JSON_TYPE::ARRAY);
	if (nullptr == profiles)
		return false;
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
			return false;
		}
	}

	std::unordered_map<LostArk::Shared::SKILL_ID, SKILL_DEFINITION> skills;
	const DATA_JSON_VALUE* skillValues =
		Required(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	if (nullptr == skillValues)
		return false;
	for (const DATA_JSON_VALUE& value : skillValues->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "skillId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* slot = Required(value, "inputSlot", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* action = Required(value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* cooldown = Required(value, "cooldownMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* damageId = Required(
			value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		if (nullptr == id || nullptr == slot || nullptr == name || nullptr == action ||
			nullptr == cooldown || nullptr == damageId)
		{
			return false;
		}
		const auto damage = damages.find(damageId->Get_String());
		SKILL_DEFINITION definition{};
		definition.iSkillId = static_cast<LostArk::Shared::SKILL_ID>(id->Get_Number());
		definition.strInputSlot = slot->Get_String();
		definition.strDisplayName = name->Get_String();
		definition.strActionId = action->Get_String();
		definition.iCooldownMs = static_cast<std::uint32_t>(cooldown->Get_Number());
		definition.iDamage = damages.end() == damage ? 0u : damage->second;
		if (LostArk::Shared::INVALID_SKILL_ID == definition.iSkillId ||
			0u == definition.iCooldownMs || 0u == definition.iDamage ||
			!skills.emplace(definition.iSkillId, std::move(definition)).second)
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

	m_SkillDefinitions = std::move(skills);
	m_BossDisplayNames = std::move(bossNames);
	m_strStatus = "Loaded combat HUD definitions";
	return true;
}

void Client::CCombatHUDViewModel::Apply_LocalPlayer(
	const std::uint32_t serverTick,
	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
	m_Player.isValid = true;
	m_Player.iServerTick = serverTick;
	m_Player.iCurrentHp = snapshot.iCurrentHp;
	m_Player.iMaximumHp = snapshot.iMaximumHp;
	m_Player.iCurrentResource = snapshot.iCurrentResource;
	m_Player.iMaximumResource = snapshot.iMaximumResource;
	m_Player.eAction = snapshot.eAction;
	m_Player.Skills.clear();
	for (const auto& [skillId, definition] : m_SkillDefinitions)
	{
		HUD_SKILL_STATE state{};
		state.iSkillId = skillId;
		state.strInputSlot = definition.strInputSlot;
		state.strDisplayName = definition.strDisplayName;
		state.strActionId = definition.strActionId;
		state.iDamage = definition.iDamage;
		state.iCooldownDurationTicks =
			(definition.iCooldownMs * SERVER_TICK_HZ + 999u) / 1000u;
		const auto cooldown = std::find_if(
			snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[skillId](const LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT& value)
			{ return value.iSkillId == skillId; });
		state.iCooldownEndTick = snapshot.Cooldowns.end() == cooldown ?
			serverTick : cooldown->iCooldownEndTick;
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
