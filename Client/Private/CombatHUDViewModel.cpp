#include "CombatHUDViewModel.h"

#include "DataJson.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <limits>

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
		if (value == "GUARDIANKNIGHT") return CHARACTER_CLASS_ID::GUARDIANKNIGHT;
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
		else if (value == "GUARDIANKNIGHT_HUMAN")
			output = PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN;
		else if (value == "GUARDIANKNIGHT_DRAGON")
			output = PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON;
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
	m_KoukuGimmick = {};
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
	m_Player.iShield = snapshot.iShield;
	m_Player.iActiveBuffCount = snapshot.iActiveBuffCount;
	for (std::size_t buffIndex = 0;
		buffIndex < LostArk::Shared::MAX_ACTIVE_BUFFS; ++buffIndex)
	{
		m_Player.ActiveBuffs[buffIndex] = snapshot.ActiveBuffs[buffIndex];
	}
	m_Player.iCurrentResource = snapshot.iCurrentResource;
	m_Player.iMaximumResource = snapshot.iMaximumResource;
	m_Player.iCurrentIdentity = snapshot.iCurrentIdentity;
	m_Player.iMaximumIdentity = snapshot.iMaximumIdentity;
	m_Player.iEmberOrbs = snapshot.iEmberOrbs;
	m_Player.iEmberLockedSockets = snapshot.iEmberLockedSockets;
	m_Player.iEmberMaximumSockets = snapshot.iEmberMaximumSockets;
	m_Player.iCurrentMadness = snapshot.iCurrentMadness;
	m_Player.iMaximumMadness = snapshot.iMaximumMadness;
	m_Player.eMadnessForm = snapshot.eMadnessForm;
	m_Player.iVehicleId = snapshot.iVehicleId;
	m_Player.isWaterpangArmed = snapshot.isWaterpangArmed;
	m_Player.fMoveSpeed = snapshot.fMoveSpeed;
	m_Player.hasMoveGoal = snapshot.hasMoveGoal;
	m_Player.eVehicleFlightPhase = snapshot.eVehicleFlightPhase;
	m_Player.iHonorTitleId = snapshot.iHonorTitleId;
	m_Player.eKoukuHudMode = snapshot.eKoukuHudMode;
	m_KoukuGimmick = {};
	m_KoukuGimmick.isValid = 0u != snapshot.iMaximumMadness;
	m_KoukuGimmick.iMadnessGauge = snapshot.iCurrentMadness;
	m_KoukuGimmick.iMadnessMaximum = snapshot.iMaximumMadness;
	m_KoukuGimmick.eHudMode = static_cast<HUD_KOUKU_HUD_MODE>(snapshot.eKoukuHudMode);
	m_KoukuGimmick.eCardMazeRole = snapshot.eCardMazeRole;
	m_KoukuGimmick.eCardMazeSuit = snapshot.eCardMazeSuit;
	m_KoukuGimmick.iCardMazeKills = snapshot.iCardMazeKills;
	m_KoukuGimmick.iCardMazeKillTarget = snapshot.iCardMazeKillTarget;
	m_KoukuGimmick.CardMaze = snapshot.CardMaze;
	for (std::size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const auto index = snapshot.ModeSkillIndexBySlot[i];
		m_KoukuGimmick.ModeSkillIndexBySlot[i] = index;
		if (index < 0) continue;
		const auto id = LostArk::Shared::Kouku_InteractionCooldownSkillId(snapshot.eKoukuHudMode, index);
		for (const auto& cooldown : snapshot.Cooldowns)
			if (cooldown.iSkillId == id)
			{
				m_KoukuGimmick.CooldownEndTicks[i] = cooldown.iCooldownEndTick;
				/* The ring has to measure the same window the Server enforces, and
				the maze hammer runs on its own shorter one. */
				m_KoukuGimmick.CooldownDurationTicks[i] =
					(LostArk::Shared::KOUKU_HUD_MODE::MAZE == snapshot.eKoukuHudMode ?
						LostArk::Shared::KOUKU_MAZE_HAMMER_COOLDOWN_MS :
						LostArk::Shared::KOUKU_INTERACTION_COOLDOWN_MS) * 30u / 1000u;
				break;
			}
	}
	m_Player.iMarioStage = snapshot.iMarioStage;
	m_Player.iKoukuMinigameEndTick = snapshot.iKoukuMinigameEndTick;
	m_Player.iMarioLayoutVariant = snapshot.iMarioLayoutVariant;
	m_Player.iMarioPoppedBallMask = snapshot.iMarioPoppedBallMask;
	m_Player.iMarioCurseReleasedMask = snapshot.iMarioCurseReleasedMask;
	m_Player.iMarioRequiredColor = snapshot.iMarioRequiredColor;
	m_Player.iMarioMatchingBallCount = snapshot.iMarioMatchingBallCount;
	m_Player.isCombatReady = snapshot.isCombatReady;
	m_Player.isPatternBound = snapshot.isPatternBound;
	m_Player.iPatternBindEndTick = snapshot.iPatternBindEndTick;
	m_Player.iSilenceEndTick = snapshot.iSilenceEndTick;
	m_Player.iSilenceDurationTicks = snapshot.iSilenceDurationTicks;
	m_Player.iFearEndTick = snapshot.iFearEndTick;
	m_Player.eAction = snapshot.eAction;
	m_Player.iCurrentSkillId = snapshot.iSkillId;
	m_Player.eStance = snapshot.eStance;
	m_Player.iComboStage = snapshot.iComboStage;
	m_Player.iActionStartTick = snapshot.iActionStartTick;
	m_Player.Cooldowns = snapshot.Cooldowns;
	m_Player.eCooldownMode = snapshot.eCooldownMode;
	Build_PlayerSkills(characterClass, serverTick, &snapshot.Cooldowns);
}

Client::HUD_KOUKU_GIMMICK_STATE Client::CCombatHUDViewModel::Get_KoukuGimmick() const
{
#ifdef _DEBUG
	if (m_KoukuGimmickPreview.isValid)
		return m_KoukuGimmickPreview;
#endif
	return m_KoukuGimmick;
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
	std::uint64_t attackPower = m_PlayerProfiles.end() == ownProfile ?
		0ull : ownProfile->second.iAttackPower;
	for (const auto& entry : m_ServerNumbers)
		if (entry.eDomain == LostArk::Shared::BALANCE_DOMAIN::PLAYER && entry.strField == "attackPower" &&
			ParseCharacterClass(entry.strId) == characterClass) attackPower = static_cast<std::uint64_t>(entry.fValue);
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
		std::uint64_t displayDamage = definition.iAttackCoefficientBp || definition.iDamageAddend ?
			attackPower * definition.iAttackCoefficientBp / 10000ull + definition.iDamageAddend :
			attackPower * definition.iDamageRatePercent / 100ull;
		if (!displayDamage && (definition.iAttackCoefficientBp || definition.iDamageAddend ||
			(attackPower && definition.iDamageRatePercent))) displayDamage = 1u;
		state.iDamage = static_cast<std::uint32_t>((std::min)(displayDamage,
			static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)())));
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
			{
				state.iCooldownEndTick = cooldown->iCooldownEndTick;
				state.iCooldownDurationTicks = cooldown->iCooldownDurationTicks;
			}
		}
		m_Player.Skills.push_back(std::move(state));
	}
	std::sort(m_Player.Skills.begin(), m_Player.Skills.end(),
		[](const HUD_SKILL_STATE& left, const HUD_SKILL_STATE& right)
		{ return left.strInputSlot < right.strInputSlot; });
}

void Client::CCombatHUDViewModel::Set_BossFocusArchetype(
	const std::string& archetypeId)
{
	if (m_strBossFocusArchetype == archetypeId)
		return;
	m_strBossFocusArchetype = archetypeId;
	if (m_Boss.isValid && !archetypeId.empty() &&
		m_Boss.strArchetypeId != archetypeId)
	{
		m_Boss = {};
	}
}

void Client::CCombatHUDViewModel::Clear_BossIfArchetype(
	const std::string& archetypeId)
{
	if (m_Boss.isValid && m_Boss.strArchetypeId == archetypeId)
		m_Boss = {};
}

void Client::CCombatHUDViewModel::Apply_Boss(
	const std::uint32_t serverTick,
	const std::string& archetypeId,
	const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot)
{
	if (m_bBossHidden || (!m_strBossFocusArchetype.empty() &&
		archetypeId != m_strBossFocusArchetype))
	{
		return;
	}
	m_Boss.isValid = true;
	m_Boss.iNetEntityId = snapshot.iNetEntityId;
	m_Boss.strArchetypeId = archetypeId;
	// The primary entity keeps its archetype during ghost revival. Its HUD
	// bar count follows the replicated phase while HP remains Server-owned.
	const auto profile = m_BossProfiles.find(
		archetypeId == "BOSS_VALTAN" && snapshot.BossCombat.iGameplayPhase == 3u &&
		snapshot.strPatternId != "VALTAN_GHOST_RESPAWN_AUDITION" ?
			"BOSS_VALTAN_GHOST" : archetypeId);
	m_Boss.strDisplayName = m_BossProfiles.end() == profile ?
		archetypeId : profile->second.strDisplayName;
	m_Boss.iMaximumHealthBars = m_BossProfiles.end() == profile ?
		0u : profile->second.iMaximumHealthBars;
	const std::string numericBossId = m_BossProfiles.end() == profile ? archetypeId : profile->first;
	for (const auto& entry : m_ServerNumbers)
		if (entry.eDomain == LostArk::Shared::BALANCE_DOMAIN::BOSS && entry.strId == numericBossId &&
			entry.strField == "maximumHealthBars") m_Boss.iMaximumHealthBars = static_cast<std::uint32_t>(entry.fValue);
	m_Boss.iActiveBuffCount = snapshot.iActiveBuffCount;
	for (std::size_t buffIndex = 0;
		buffIndex < LostArk::Shared::MAX_ACTIVE_BUFFS; ++buffIndex)
	{
		m_Boss.ActiveBuffs[buffIndex] = snapshot.ActiveBuffs[buffIndex];
	}
	m_Boss.iCurrentHp = snapshot.iCurrentHp;
	m_Boss.iMaximumHp = snapshot.iMaximumHp;
	m_Boss.iPhase = snapshot.BossCombat.iGameplayPhase;
	m_Boss.iBossCombatStateRevision =
		snapshot.BossCombat.iStateRevision;
	m_Boss.iAlivePartMask = snapshot.BossCombat.iAlivePartMask;
	m_Boss.iBossCombatFlags = snapshot.BossCombat.iFlags;
	m_Boss.iCurrentStagger = snapshot.BossCombat.iCurrentStagger;
	m_Boss.iMaximumStagger = snapshot.BossCombat.iMaximumStagger;
	m_Boss.iCurrentShield = snapshot.BossCombat.iCurrentShield;
	m_Boss.iMaximumShield = snapshot.BossCombat.iMaximumShield;
	m_Boss.iResponseProgress = snapshot.BossCombat.iResponseProgress;
	m_Boss.iResponseThreshold = snapshot.BossCombat.iResponseThreshold;
	m_Boss.eMechanicGaugeKind = snapshot.BossCombat.eMechanicGaugeKind;
	m_Boss.iCurrentMechanicGauge = snapshot.BossCombat.iCurrentMechanicGauge;
	m_Boss.iMaximumMechanicGauge = snapshot.BossCombat.iMaximumMechanicGauge;
	/* Authored magic-orb channels use the Server's independent stagger counter.
	Only the legacy HP-response window still consumes response progress.
	Exact action IDs hide the bar immediately on success, timeout or cancel. */
	const bool legacyMagicOrbWindow = snapshot.strPatternId == "VALTAN_MAGIC_ORB_STAGGER_76" &&
		snapshot.strActionId == "valtan.mechanic.magic-orb-stagger-76.window";
	const bool authoredMagicOrbChannel = snapshot.strPatternId == "VALTAN_STAGGER_SLOT" &&
		snapshot.strActionId == "valtan.authoring.stagger-slot.channel";
	if ((archetypeId == "BOSS_VALTAN" || archetypeId == "BOSS_VALTAN_GHOST") &&
		(legacyMagicOrbWindow || authoredMagicOrbChannel))
	{
		const bool legacyResponse = legacyMagicOrbWindow && snapshot.BossCombat.iResponseThreshold > 0u;
		const auto maximum = legacyResponse ?
			snapshot.BossCombat.iResponseThreshold : snapshot.BossCombat.iMaximumStagger;
		const auto progress = legacyResponse ?
			snapshot.BossCombat.iResponseProgress : snapshot.BossCombat.iCurrentStagger;
		m_Boss.eMechanicGaugeKind = maximum > 0u ? LostArk::Shared::BOSS_MECHANIC_GAUGE_KIND::STAGGER :
			LostArk::Shared::BOSS_MECHANIC_GAUGE_KIND::NONE;
		m_Boss.iMaximumMechanicGauge = maximum;
		m_Boss.iCurrentMechanicGauge = maximum - (std::min)(progress, maximum);
	}
	m_Boss.hasPosition = true;
	m_Boss.fPositionX = snapshot.fPositionX;
	m_Boss.fPositionY = snapshot.fPositionY;
	m_Boss.fPositionZ = snapshot.fPositionZ;
	m_Boss.iServerTick = serverTick;
	m_Boss.eAction = snapshot.eAction;
	m_Boss.strActionId = snapshot.strActionId;
	m_Boss.strPatternId = snapshot.strPatternId;
	m_Boss.iPatternSequence = snapshot.iPatternSequence;
	m_Boss.iPatternStageIndex = snapshot.iPatternStageIndex;
	m_Boss.iActionStartTick = snapshot.iActionStartTick;
	m_Boss.PinnedDefinitionRevision = snapshot.PinnedDefinitionRevision;
}

void Client::CCombatHUDViewModel::Debug_Set_Boss_Preview(const bool enable)
{
	if (!enable)
	{
		m_Boss.isValid = false;
		return;
	}

	m_Boss.isValid = true;
	m_Boss.iNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
	m_Boss.strArchetypeId = "DEBUG_PREVIEW_VALTAN";
	/* UTF-8 bytes for "\xBC1C\xD0C4" (Valtan's display name) -- strDisplayName is std::string, not
	wstring, so this needs the UTF-8 encoding of the two Hangul codepoints, not their raw values. */
	m_Boss.strDisplayName = "\xEB\xB0\x9C\xED\x83\x84";
	m_Boss.iMaximumHealthBars = 160u;
	m_Boss.iCurrentHp = 848096077u;
	m_Boss.iMaximumHp = 1991561183u;
	m_Boss.iPhase = 1u;
	m_Boss.iBossCombatStateRevision = 1u;
	m_Boss.iAlivePartMask = 0x3u;
	m_Boss.iBossCombatFlags = 0u;
	m_Boss.iCurrentStagger = 420u;
	m_Boss.iMaximumStagger = 1000u;
	m_Boss.iCurrentShield = 0u;
	m_Boss.iMaximumShield = 0u;
	/* Sample pattern check so the HUD Layout Tool can place the immune gauge. */
	m_Boss.iResponseProgress = 420u;
	m_Boss.iResponseThreshold = 1000u;
	m_Boss.hasPosition = false;
	m_Boss.fPositionX = 0.f;
	m_Boss.fPositionY = 0.f;
	m_Boss.fPositionZ = 0.f;
	m_Boss.iServerTick = 0u;
	m_Boss.eAction = LostArk::Shared::WORLD_ENTITY_ACTION::IDLE;
	m_Boss.strActionId.clear();
	m_Boss.strPatternId.clear();
	m_Boss.iPatternSequence = 0u;
	m_Boss.iPatternStageIndex = 0u;
	m_Boss.iActionStartTick = 0u;
	m_Boss.PinnedDefinitionRevision = {};
}

void Client::CCombatHUDViewModel::Debug_Set_Esther_Preview(const bool enable)
{
	if (!enable)
	{
		m_iEstherGaugeMaximum = 0u;
		return;
	}

	m_Player.isValid = true;
	m_iEstherGauge = 1000u;
	m_iEstherGaugeMaximum = 1000u;
}

void Client::CCombatHUDViewModel::Apply_DamageEvents(
	const std::uint32_t serverTick,
	const std::vector<LostArk::Shared::DAMAGE_EVENT>& events,
	const LostArk::Shared::PLAYER_ID localPlayerId)
{
	constexpr std::size_t MAX_RETAINED_DAMAGE_EVENTS = 128u;
	for (const LostArk::Shared::DAMAGE_EVENT& event : events)
	{
		if (LostArk::Shared::INVALID_PLAYER_ID != localPlayerId &&
			event.iSourcePlayerId == localPlayerId && event.isOutgoing &&
			event.eHitFlag != LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB &&
			event.eHitFlag != LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE)
		{
			/* Own hit: the raid's clock starts on the first one; nothing resets until the
			level is left. */
			if (!m_CombatAnalysis.isActive)
			{
				m_CombatAnalysis.isActive = true;
				m_CombatAnalysis.iStartTick = serverTick;
			}
			m_CombatAnalysis.iLastHitTick = serverTick;
			m_CombatAnalysis.iTotalDamage += event.iAmount;
			m_CombatAnalysis.iTotalStagger += event.iStaggerAmount;
			if (event.isCounterSuccess)
				++m_CombatAnalysis.iCounterSuccesses;
		}
		/* Keep Server verdict text even when no HP damage or mechanic credit exists. */
		if (0u == event.iAmount && !event.isCounterSuccess && !event.isStaggerSuccess &&
			event.eHitFlag != LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE)
			continue;
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
	m_WorldHealthBars.clear();
	m_Player = {};
	m_KoukuGimmick = {};
	m_Boss = {};
	m_bBossDeadRaw = false;
	m_DamageEvents.clear();
	m_CombatAnalysis = {};
	m_iEstherGauge = 0;
	m_iEstherGaugeMaximum = 0;
	m_EstherCutinRequest = {};
	m_DeadSceneTextRects = {};
	m_RaidClearTextRects = {};
	m_ItemAnnounceTextRects = {};
#ifdef _DEBUG
	m_KoukuGimmickPreview = {};
#endif
	m_Inventory = {};
	m_bHasInventory = false;
}

void Client::CCombatHUDViewModel::Apply_WorldHealthBars(
	std::vector<HUD_WORLD_HEALTH_BAR_STATE>&& states)
{
	m_WorldHealthBars = std::move(states);
}

void Client::CCombatHUDViewModel::Remove_WorldHealthBar(
	const LostArk::Shared::NET_ENTITY_ID entityId)
{
	m_WorldHealthBars.erase(std::remove_if(m_WorldHealthBars.begin(), m_WorldHealthBars.end(),
		[entityId](const HUD_WORLD_HEALTH_BAR_STATE& state) { return state.iNetEntityId == entityId; }),
		m_WorldHealthBars.end());
}

void Client::CCombatHUDViewModel::Apply_ServerNumericSnapshot(
	const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries)
{
	m_ServerNumbers = entries;
	if (m_Player.isValid) Build_PlayerSkills(m_Player.eCharacterClass, m_Player.iServerTick, &m_Player.Cooldowns);
}
