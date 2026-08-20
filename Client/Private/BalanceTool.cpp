#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
#include "imgui.h"
#endif

#include "BalanceTool.h"

#include "DataJson.h"
#include "PlayerCommandSink.h"
#include "ProjectDataRoot.h"
#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
#include "CombatHUDViewModel.h"
#endif

#include <Windows.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	bool ReadJson(const std::filesystem::path& relativePath,
		DATA_JSON_VALUE& output, std::string& status)
	{
		const std::filesystem::path path = CProjectDataRoot::Resolve(relativePath);
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			status = "Missing data document: " + relativePath.string();
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		return CDataJson::Parse(text, output, status) && output.Is_Object();
	}

	const DATA_JSON_VALUE* Field(const DATA_JSON_VALUE& object,
		const char* name, const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool ReadString(const DATA_JSON_VALUE& object, const char* name,
		std::string& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::STRING);
		if (nullptr == value)
			return false;
		output = value->Get_String();
		return true;
	}

	bool ReadU32(const DATA_JSON_VALUE& object, const char* name,
		std::uint32_t& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == value || !std::isfinite(value->Get_Number()) ||
			std::floor(value->Get_Number()) != value->Get_Number() ||
			value->Get_Number() < 0.0 ||
			value->Get_Number() >
			static_cast<double>((std::numeric_limits<std::uint32_t>::max)()))
		{
			return false;
		}
		output = static_cast<std::uint32_t>(value->Get_Number());
		return true;
	}

	bool ReadDouble(const DATA_JSON_VALUE& object, const char* name, double& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == value)
			return false;
		output = value->Get_Number();
		return std::isfinite(output);
	}

	bool IsExactObject(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char* key) { return nullptr != value.Find(key); });
	}

	bool HasSchemaVersion(const DATA_JSON_VALUE& root, const char* schema,
		const std::uint32_t version)
	{
		std::string actualSchema;
		std::uint32_t actualVersion = 0u;
		return ReadString(root, "schema", actualSchema) && actualSchema == schema &&
			ReadU32(root, "formatVersion", actualVersion) && actualVersion == version;
	}

	#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
	bool EditU32(const char* label, std::uint32_t& value,
		const std::uint32_t minimum, const std::uint32_t maximum)
	{
		const bool changed = ImGui::InputScalar(
			label, ImGuiDataType_U32, &value, nullptr, nullptr, "%u");
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}

	bool EditDouble(const char* label, double& value, const float speed,
		const double minimum, const double maximum, const char* format = "%.3f")
	{
		const bool changed = ImGui::DragScalar(
			label, ImGuiDataType_Double, &value, speed, &minimum, &maximum, format);
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}
	#endif

	std::string Quote(const std::string& value)
	{
		return "\"" + CDataJson::Escape(value) + "\"";
	}

	std::string FormatJsonNumber(double value)
	{
		if (0.0 == value)
			value = 0.0;
		char buffer[64]{};
		const auto converted = std::to_chars(
			buffer, buffer + sizeof(buffer), value, std::chars_format::general);
		if (std::errc{} != converted.ec)
			return "0";
		return std::string(buffer, converted.ptr);
	}

	bool DurableWrite(const std::filesystem::path& path,
		const std::string& text, std::string& status)
	{
		FILE* file = nullptr;
		if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
		{
			status = "Could not open balance staging file.";
			return false;
		}
		const bool wrote = text.size() ==
			fwrite(text.data(), 1u, text.size(), file);
		const bool flushed = 0 == fflush(file) && 0 == _commit(_fileno(file));
		const bool closed = 0 == fclose(file);
		if (!wrote || !flushed || !closed)
		{
			std::error_code error;
			std::filesystem::remove(path, error);
			status = "Could not durably write balance staging file.";
			return false;
		}
		return true;
	}

	bool ParseStagedJson(const std::filesystem::path& path, std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Balance staging file disappeared.";
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE root;
		return CDataJson::Parse(text, root, status) && root.Is_Object();
	}
}


Client::CBalanceTool::CBalanceTool(
	std::shared_ptr<IPlayerCommandSink> commandSink)
	: m_commandSink(std::move(commandSink))
{
	Reload();
}

void Client::CBalanceTool::MarkDirty(const bool changed)
{
	if (changed)
		m_dirty = true;
}

bool Client::CBalanceTool::Reload()
{
	DATA_JSON_VALUE playerRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE bossRoot;
	DATA_JSON_VALUE encounterRoot;
	DATA_JSON_VALUE receiptRoot;
	std::string status;
	if (!ReadJson(L"Balance/PlayerProfiles.json", playerRoot, status) ||
		!ReadJson(L"Balance/PlayerSkills.json", skillRoot, status) ||
		!ReadJson(L"Balance/DamageProfiles.json", damageRoot, status) ||
		!ReadJson(L"Balance/BossProfiles.json", bossRoot, status) ||
		!ReadJson(L"Encounters/Valtan/ValtanEncounter.json", encounterRoot, status) ||
		!ReadJson(L"Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
			receiptRoot, status))
	{
		m_status = "Reload failed: " + status;
		return false;
	}
	if (!IsExactObject(playerRoot, { "schema", "formatVersion", "players" }) ||
		!HasSchemaVersion(playerRoot, "lostark.player-profiles", 2u) ||
		!IsExactObject(skillRoot, { "schema", "formatVersion", "skills" }) ||
		!HasSchemaVersion(skillRoot, "lostark.player-skills", 2u) ||
		!IsExactObject(damageRoot, { "schema", "formatVersion", "profiles" }) ||
		!HasSchemaVersion(damageRoot, "lostark.damage-profiles", 2u) ||
		!IsExactObject(bossRoot, { "schema", "formatVersion", "bosses" }) ||
		!HasSchemaVersion(bossRoot, "lostark.boss-profiles", 3u) ||
		!IsExactObject(encounterRoot, { "schema", "formatVersion", "encounterId",
			"bossArchetypeId", "authority", "fixedTickHz", "introPatternId",
			"states", "patterns" }) ||
		!HasSchemaVersion(encounterRoot, "lostark.encounter-profile", 3u) ||
		!IsExactObject(receiptRoot, { "schema", "formatVersion", "sourceBuildId",
			"referenceSkillLevel", "extractorSha256", "sourceFiles", "coverage", "entries" }) ||
		!HasSchemaVersion(receiptRoot, "lostark.balance-provenance-receipt", 1u))
	{
		m_status = "Reload failed: schema/version or root fields are not exact.";
		return false;
	}

	std::vector<PLAYER_EDIT> players;
	std::vector<SKILL_EDIT> skills;
	std::vector<DAMAGE_EDIT> damageProfiles;
	std::vector<BOSS_EDIT> bosses;
	std::vector<PATTERN_EDIT> patterns;
	std::vector<ENCOUNTER_STATE_EDIT> states;
	std::unordered_map<std::string, std::string> bases;
	const DATA_JSON_VALUE* playerArray = Field(playerRoot, "players", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* skillArray = Field(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* damageArray = Field(damageRoot, "profiles", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* bossArray = Field(bossRoot, "bosses", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* patternArray = Field(encounterRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* stateArray = Field(encounterRoot, "states", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* receiptArray = Field(receiptRoot, "entries", DATA_JSON_TYPE::ARRAY);
	if (nullptr == playerArray || nullptr == skillArray || nullptr == damageArray ||
		nullptr == bossArray || nullptr == patternArray || nullptr == stateArray ||
		nullptr == receiptArray)
	{
		m_status = "Reload failed: balance array is missing.";
		return false;
	}

	for (const DATA_JSON_VALUE& value : playerArray->Get_Array())
	{
		PLAYER_EDIT row{};
		if (!IsExactObject(value, { "characterClass", "maximumHp", "maximumResource",
			"resourceRegenPerSecond", "attackPower", "defense", "moveSpeed",
			"defenseStanceMoveSpeedScale", "maximumIdentity",
			"identityRegenPerSecond", "identityDrainPerSecond",
			"identityStanceSwitchCost", "identityCyclic", "defaultStance" }) ||
			!ReadString(value, "characterClass", row.characterClass) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "maximumResource", row.maximumResource) ||
			!ReadU32(value, "resourceRegenPerSecond", row.resourceRegenPerSecond) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadU32(value, "defense", row.defense) ||
			!ReadDouble(value, "moveSpeed", row.moveSpeed) ||
			!ReadDouble(value, "defenseStanceMoveSpeedScale",
				row.defenseStanceMoveSpeedScale) ||
			!ReadU32(value, "maximumIdentity", row.maximumIdentity) ||
			!ReadU32(value, "identityRegenPerSecond",
				row.identityRegenPerSecond) ||
			!ReadU32(value, "identityDrainPerSecond",
				row.identityDrainPerSecond) ||
			!ReadU32(value, "identityStanceSwitchCost",
				row.identityStanceSwitchCost) ||
			!ReadU32(value, "identityCyclic", row.identityCyclic) ||
			!ReadString(value, "defaultStance", row.defaultStance))
		{
			m_status = "Reload failed: invalid player profile.";
			return false;
		}
		players.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : skillArray->Get_Array())
	{
		SKILL_EDIT row{};
		const DATA_JSON_VALUE* stagesValue = Field(value, "comboStages", DATA_JSON_TYPE::ARRAY);
		if (!IsExactObject(value, { "skillId", "characterClass", "inputSlot", "displayName",
			"actionId", "skillKind", "cooldownMs", "actionDurationMs", "hitTimeMs",
			"resourceCost", "identityCost", "movementDistance", "maximumRange", "serverDamageProfileId",
			"effectId", "requiredStance", "setsStance", "comboStages" }) ||
			!ReadU32(value, "skillId", row.skillId) ||
			!ReadString(value, "characterClass", row.characterClass) ||
			!ReadString(value, "inputSlot", row.inputSlot) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadString(value, "skillKind", row.skillKind) ||
			!ReadU32(value, "cooldownMs", row.cooldownMs) ||
			!ReadU32(value, "actionDurationMs", row.actionDurationMs) ||
			!ReadU32(value, "hitTimeMs", row.hitTimeMs) ||
			!ReadU32(value, "resourceCost", row.resourceCost) ||
			!ReadU32(value, "identityCost", row.identityCost) ||
			!ReadDouble(value, "movementDistance", row.movementDistance) ||
			!ReadDouble(value, "maximumRange", row.maximumRange) ||
			!ReadString(value, "serverDamageProfileId", row.damageProfileId) ||
			!ReadString(value, "effectId", row.effectId) ||
			!ReadString(value, "requiredStance", row.requiredStance) ||
			!ReadString(value, "setsStance", row.setsStance) ||
			nullptr == stagesValue)
		{
			m_status = "Reload failed: invalid skill definition.";
			return false;
		}
		for (const DATA_JSON_VALUE& stageValue : stagesValue->Get_Array())
		{
			COMBO_STAGE_EDIT stage{};
			if (!IsExactObject(stageValue, { "actionDurationMs", "hitTimeMs",
				"comboAdvanceMs", "inputOpenMs", "inputCloseMs" }) ||
				!ReadU32(stageValue, "actionDurationMs", stage.actionDurationMs) ||
				!ReadU32(stageValue, "hitTimeMs", stage.hitTimeMs) ||
				!ReadU32(stageValue, "comboAdvanceMs", stage.comboAdvanceMs) ||
				!ReadU32(stageValue, "inputOpenMs", stage.inputOpenMs) ||
				!ReadU32(stageValue, "inputCloseMs", stage.inputCloseMs))
			{
				m_status = "Reload failed: invalid combo stage.";
				return false;
			}
			row.comboStages.push_back(stage);
		}
		skills.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : damageArray->Get_Array())
	{
		DAMAGE_EDIT row{};
		if (!IsExactObject(value, { "damageProfileId", "damageRatePercent" }) ||
			!ReadString(value, "damageProfileId", row.damageProfileId) ||
			!ReadU32(value, "damageRatePercent", row.damageRatePercent))
			return false;
		damageProfiles.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : bossArray->Get_Array())
	{
		BOSS_EDIT row{};
		if (!IsExactObject(value, { "archetypeId", "encounterId", "displayName", "maximumHp", "maximumHealthBars",
			"attackPower", "collisionRadius", "engageDistance", "moveSpeed",
			"phaseTwoHpPercent" }) ||
			!ReadString(value, "archetypeId", row.archetypeId) ||
			!ReadString(value, "encounterId", row.encounterId) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "maximumHealthBars", row.maximumHealthBars) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadDouble(value, "collisionRadius", row.collisionRadius) ||
			!ReadDouble(value, "engageDistance", row.engageDistance) ||
			!ReadDouble(value, "moveSpeed", row.moveSpeed) ||
			!ReadU32(value, "phaseTwoHpPercent", row.phaseTwoHpPercent))
			return false;
		bosses.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : patternArray->Get_Array())
	{
		PATTERN_EDIT row{};
		const DATA_JSON_VALUE* sourceActions =
			Field(value, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* stages = Field(value, "stages", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* serverMotion = value.Find("serverMotion");
		const bool hasServerMotion = nullptr != serverMotion;
		const bool hasExactPatternFields = hasServerMotion ?
			IsExactObject(value, { "patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "armorRequirement",
				"phaseRequirement", "invulnerableWhileRunning",
				"selectionWeight", "maximumConsecutiveUses",
				"minimumRange", "maximumRange", "serverMotion", "stages" }) :
			IsExactObject(value, { "patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "armorRequirement",
				"phaseRequirement", "invulnerableWhileRunning",
				"selectionWeight", "maximumConsecutiveUses",
				"minimumRange", "maximumRange", "stages" });
		if (!hasExactPatternFields ||
			nullptr == sourceActions || sourceActions->Get_Array().empty() ||
			nullptr == stages || stages->Get_Array().empty() ||
			!ReadString(value, "patternId", row.patternId) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadString(value, "selectionMode", row.selectionMode) ||
			!ReadU32(value, "minimumHealthBar", row.minimumHealthBar) ||
			!ReadU32(value, "maximumHealthBar", row.maximumHealthBar) ||
			!ReadU32(value, "triggerHealthBar", row.triggerHealthBar) ||
			!ReadU32(value, "triggerOrder", row.triggerOrder) ||
			!ReadString(value, "armorRequirement", row.armorRequirement) ||
			(row.armorRequirement != "ANY" &&
				row.armorRequirement != "ARMORED" &&
				row.armorRequirement != "STRIPPED") ||
			!ReadString(value, "phaseRequirement", row.phaseRequirement) ||
			(row.phaseRequirement != "ANY" &&
				row.phaseRequirement != "PHASE_ONE" &&
				row.phaseRequirement != "PHASE_TWO") ||
			nullptr == Field(value, "invulnerableWhileRunning",
				DATA_JSON_TYPE::BOOLEAN) ||
			!ReadU32(value, "selectionWeight", row.selectionWeight) ||
			!ReadU32(value, "maximumConsecutiveUses", row.maximumConsecutiveUses) ||
			!ReadDouble(value, "minimumRange", row.minimumRange) ||
			!ReadDouble(value, "maximumRange", row.maximumRange))
			return false;
		if (hasServerMotion)
		{
			if (!serverMotion->Is_Object())
				return false;
			const DATA_JSON_VALUE* landingPosition =
				Field(*serverMotion, "landingPosition", DATA_JSON_TYPE::ARRAY);
			if (!IsExactObject(*serverMotion,
				{ "kind", "anchorId", "landingPosition", "apexHeight" }) ||
				!ReadString(*serverMotion, "kind", row.serverMotion.kind) ||
				!ReadString(*serverMotion, "anchorId", row.serverMotion.anchorId) ||
				nullptr == landingPosition ||
				3u != landingPosition->Get_Array().size() ||
				!landingPosition->Get_Array()[0].Is_Number() ||
				!landingPosition->Get_Array()[1].Is_Number() ||
				!landingPosition->Get_Array()[2].Is_Number() ||
				!ReadDouble(*serverMotion, "apexHeight", row.serverMotion.apexHeight))
			{
				return false;
			}
			row.serverMotion.landingX =
				landingPosition->Get_Array()[0].Get_Number();
			row.serverMotion.landingY =
				landingPosition->Get_Array()[1].Get_Number();
			row.serverMotion.landingZ =
				landingPosition->Get_Array()[2].Get_Number();
			row.serverMotion.enabled =
				std::isfinite(row.serverMotion.landingX) &&
				std::isfinite(row.serverMotion.landingY) &&
				std::isfinite(row.serverMotion.landingZ);
			if (!row.serverMotion.enabled)
				return false;
		}
		for (const DATA_JSON_VALUE& sourceAction : sourceActions->Get_Array())
		{
			if (!sourceAction.Is_Number() ||
				!std::isfinite(sourceAction.Get_Number()) ||
				std::floor(sourceAction.Get_Number()) != sourceAction.Get_Number() ||
				sourceAction.Get_Number() <= 0.0 ||
				sourceAction.Get_Number() >
					static_cast<double>((std::numeric_limits<std::uint32_t>::max)()))
			{
				return false;
			}
			row.sourceActionIds.push_back(
				static_cast<std::uint32_t>(sourceAction.Get_Number()));
		}
		for (const DATA_JSON_VALUE& stageValue : stages->Get_Array())
		{
			PATTERN_STAGE_EDIT stage{};
			if (!IsExactObject(stageValue, { "stageId", "actionId", "stageKind",
				"durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				"hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				"hitIntervalMs", "hitDelayMs", "serverDamageProfileId",
				"pushRangeM", "pushMs", "knockdown", "downMs" }) ||
				!ReadString(stageValue, "stageId", stage.stageId) ||
				!ReadString(stageValue, "actionId", stage.actionId) ||
				!ReadString(stageValue, "stageKind", stage.stageKind) ||
				!ReadU32(stageValue, "durationMs", stage.durationMs) ||
				!ReadString(stageValue, "hitShape", stage.hitShape) ||
				!ReadDouble(stageValue, "hitOuterRadius", stage.hitOuterRadius) ||
				!ReadDouble(stageValue, "hitInnerRadius", stage.hitInnerRadius) ||
				!ReadDouble(stageValue, "hitAngleDegrees", stage.hitAngleDegrees) ||
				!ReadDouble(stageValue, "hitLength", stage.hitLength) ||
				!ReadDouble(stageValue, "hitHalfWidth", stage.hitHalfWidth) ||
				!ReadU32(stageValue, "hitCount", stage.hitCount) ||
				!ReadU32(stageValue, "hitIntervalMs", stage.hitIntervalMs) ||
				!ReadU32(stageValue, "hitDelayMs", stage.hitDelayMs) ||
				!ReadString(stageValue, "serverDamageProfileId", stage.damageProfileId) ||
				!ReadDouble(stageValue, "pushRangeM", stage.pushRangeM) ||
				!ReadU32(stageValue, "pushMs", stage.pushMs) ||
				!ReadU32(stageValue, "downMs", stage.downMs))
			{
				return false;
			}
			const DATA_JSON_VALUE* knockdownValue = stageValue.Find("knockdown");
			if (nullptr == knockdownValue || !knockdownValue->Is_Boolean())
				return false;
			stage.knockdown = knockdownValue->Get_Boolean();
			row.stages.push_back(std::move(stage));
		}
		row.invulnerableWhileRunning =
			Field(value, "invulnerableWhileRunning",
				DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		patterns.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : stateArray->Get_Array())
	{
		ENCOUNTER_STATE_EDIT row{};
		if (!IsExactObject(value, { "id", "actionId", "next" }) ||
			!ReadString(value, "id", row.id) || !ReadString(value, "actionId", row.actionId))
			return false;
		const DATA_JSON_VALUE* next = value.Find("next");
		if (nullptr == next || (!next->Is_Null() && !next->Is_String()))
			return false;
		row.hasNext = next->Is_String();
		if (row.hasNext)
			row.next = next->Get_String();
		states.push_back(std::move(row));
	}
	std::string encounterId;
	std::string bossArchetypeId;
	std::string authority;
	std::string introPatternId;
	std::uint32_t fixedTickHz = 0;
	if (!ReadString(encounterRoot, "encounterId", encounterId) ||
		!ReadString(encounterRoot, "bossArchetypeId", bossArchetypeId) ||
		!ReadString(encounterRoot, "authority", authority) ||
		!ReadString(encounterRoot, "introPatternId", introPatternId) ||
		!ReadU32(encounterRoot, "fixedTickHz", fixedTickHz))
		return false;
	for (const DATA_JSON_VALUE& value : receiptArray->Get_Array())
	{
		std::string document;
		std::string targetId;
		std::string field;
		std::string basis;
		if (!ReadString(value, "targetDocument", document) ||
			!ReadString(value, "targetId", targetId) ||
			!ReadString(value, "targetField", field) ||
			!ReadString(value, "basis", basis))
			return false;
		bases.emplace(document + "#" + targetId + "." + field, basis);
	}

	m_players = std::move(players);
	m_skills = std::move(skills);
	m_damageProfiles = std::move(damageProfiles);
	m_bosses = std::move(bosses);
	m_patterns = std::move(patterns);
	m_encounterStates = std::move(states);
	m_encounterId = std::move(encounterId);
	m_encounterBossArchetypeId = std::move(bossArchetypeId);
	m_encounterAuthority = std::move(authority);
	m_encounterIntroPatternId = std::move(introPatternId);
	m_fixedTickHz = fixedTickHz;
	m_basisByField = std::move(bases);
	m_selectedPlayer = (std::min)(m_selectedPlayer,
		m_players.empty() ? 0u : m_players.size() - 1u);
	m_selectedBoss = (std::min)(m_selectedBoss,
		m_bosses.empty() ? 0u : m_bosses.size() - 1u);
	m_dirty = false;
	m_status = "Loaded authoring balance and field provenance entries.";
	return true;
}

std::uint32_t* Client::CBalanceTool::FindDamageRate(
	const std::string& damageProfileId)
{
	const auto found = std::find_if(m_damageProfiles.begin(), m_damageProfiles.end(),
		[&](const DAMAGE_EDIT& row) { return row.damageProfileId == damageProfileId; });
	return m_damageProfiles.end() == found ? nullptr : &found->damageRatePercent;
}

const std::uint32_t* Client::CBalanceTool::FindDamageRate(
	const std::string& damageProfileId) const
{
	const auto found = std::find_if(m_damageProfiles.begin(), m_damageProfiles.end(),
		[&](const DAMAGE_EDIT& row) { return row.damageProfileId == damageProfileId; });
	return m_damageProfiles.end() == found ? nullptr : &found->damageRatePercent;
}

void Client::CBalanceTool::NormalizePatternStagePush(PATTERN_STAGE_EDIT& stage)
{
	if (stage.damageProfileId.empty())
	{
		stage.pushRangeM = 0.0;
		stage.pushMs = 0u;
		stage.knockdown = false;
		stage.downMs = 0u;
		return;
	}
	if (0.0 == stage.pushRangeM)
		stage.pushMs = 0u;
	else if (0u == stage.pushMs)
		stage.pushMs = 1u;
	if (!stage.knockdown)
		stage.downMs = 0u;
	else if (0u == stage.downMs)
		stage.downMs = 1u;
}

void Client::CBalanceTool::NormalizePatternStageForShape(
	PATTERN_STAGE_EDIT& stage)
{
	const bool none = "NONE" == stage.hitShape;
	const bool circle = "CIRCLE" == stage.hitShape;
	const bool ring = "RING" == stage.hitShape;
	const bool cone = "CONE" == stage.hitShape;
	const bool boxLike = "BOX" == stage.hitShape ||
		"CROSS" == stage.hitShape || "SIX_DIRECTIONS" == stage.hitShape;

	if (!circle && !ring)
		stage.hitOuterRadius = 0.0;
	if (!ring)
		stage.hitInnerRadius = 0.0;
	if (!cone)
		stage.hitAngleDegrees = 0.0;
	if (!cone && !boxLike)
		stage.hitLength = 0.0;
	if (!boxLike)
		stage.hitHalfWidth = 0.0;
	if (none)
	{
		stage.hitCount = 0u;
		stage.hitIntervalMs = 0u;
		stage.hitDelayMs = 0u;
		stage.damageProfileId.clear();
	}
	NormalizePatternStagePush(stage);
}

#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
void Client::CBalanceTool::RenderBasis(const std::string& document,
	const std::string& targetId, const std::string& field) const
{
	const auto found = m_basisByField.find(document + "#" + targetId + "." + field);
	if (m_basisByField.end() != found)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("[%s]", found->second.c_str());
	}
}

void Client::CBalanceTool::RenderPlayerEditor()
{
	if (m_players.empty())
		return;
	PLAYER_EDIT& player = m_players[m_selectedPlayer];
	const std::string profileTarget = "player:" + player.characterClass;
	ImGui::Text("%s", player.characterClass.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("Reference row: level 10; fixed single-level skills use level 1");
	ImGui::SeparatorText("Basic stats");
	MarkDirty(EditU32("Maximum HP", player.maximumHp, 1u, 100000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "maximumHp");
	MarkDirty(EditU32("Attack power", player.attackPower, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "attackPower");
	MarkDirty(EditU32("Defense", player.defense, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "defense");
	MarkDirty(EditU32("Maximum resource", player.maximumResource, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "maximumResource");
	MarkDirty(EditU32("Resource / sec", player.resourceRegenPerSecond, 1u, player.maximumResource));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "resourceRegenPerSecond");
	ImGui::SeparatorText("Movement");
	MarkDirty(EditDouble("Move speed", player.moveSpeed, 0.01f, 0.01, 100.0, "%.2f"));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "moveSpeed");
	MarkDirty(EditDouble("Defense stance speed scale",
		player.defenseStanceMoveSpeedScale, 0.01f, 0.01, 1.0, "%.2f"));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"defenseStanceMoveSpeedScale");
	ImGui::SeparatorText("Identity");
	MarkDirty(EditU32("Maximum identity", player.maximumIdentity, 0u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"maximumIdentity");
	MarkDirty(EditU32("Identity / sec", player.identityRegenPerSecond,
		0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityRegenPerSecond");
	MarkDirty(EditU32("Identity drain / sec", player.identityDrainPerSecond,
		0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityDrainPerSecond");
	MarkDirty(EditU32("Stance switch identity cost",
		player.identityStanceSwitchCost, 0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityStanceSwitchCost");
	MarkDirty(EditU32("Cyclic identity", player.identityCyclic, 0u, 1u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityCyclic");
	ImGui::SeparatorText("Skills");
	for (SKILL_EDIT& skill : m_skills)
	{
		if (skill.characterClass != player.characterClass)
			continue;
		ImGui::PushID(static_cast<int>(skill.skillId));
		const std::string label = "[" + skill.inputSlot + "] " + skill.displayName +
			"##" + std::to_string(skill.skillId);
		if (ImGui::CollapsingHeader(label.c_str()))
		{
			const std::string target = "skill:" + std::to_string(skill.skillId);
			ImGui::TextDisabled("skillId %u | %s | %s", skill.skillId,
				skill.skillKind.c_str(), skill.actionId.c_str());
			MarkDirty(EditU32("Cooldown ms", skill.cooldownMs,
				skill.skillKind == "COMBO" ? 0u : 1u, 600000u));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "cooldownMs");
			MarkDirty(EditU32("Resource cost", skill.resourceCost, 0u, player.maximumResource));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "resourceCost");
			MarkDirty(EditU32("Identity cost", skill.identityCost,
				0u, player.maximumIdentity));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "identityCost");
			std::uint32_t* damageRate = FindDamageRate(skill.damageProfileId);
			if (nullptr != damageRate)
			{
				MarkDirty(EditU32("Damage rate %", *damageRate, 1u, 100000u));
				RenderBasis("Data/Balance/DamageProfiles.json",
					"damage:" + skill.damageProfileId, "damageRatePercent");
				const std::uint64_t expected =
					static_cast<std::uint64_t>(player.attackPower) * *damageRate / 100ull;
				ImGui::TextDisabled("Expected raw hit: %llu",
					static_cast<unsigned long long>((std::max<std::uint64_t>)(1ull, expected)));
			}
			MarkDirty(EditU32("Action duration ms", skill.actionDurationMs, 1u, 600000u));
			MarkDirty(EditU32("Hit time ms", skill.hitTimeMs, 0u, skill.actionDurationMs));
			MarkDirty(EditDouble("Maximum range", skill.maximumRange,
				0.1f, 0.1, 1000.0));
			MarkDirty(EditDouble("Skill movement distance", skill.movementDistance,
				0.1f, 0.0, 1000.0));
			if (!skill.comboStages.empty() && ImGui::TreeNode("Staged action timing"))
			{
				for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
				{
					COMBO_STAGE_EDIT& stage = skill.comboStages[index];
					ImGui::PushID(static_cast<int>(index));
					ImGui::SeparatorText(("Stage " + std::to_string(index + 1u)).c_str());
					MarkDirty(EditU32("Duration", stage.actionDurationMs, 1u, 600000u));
					MarkDirty(EditU32("Hit", stage.hitTimeMs, 0u, stage.actionDurationMs));
					MarkDirty(EditU32("Combo advance", stage.comboAdvanceMs,
						stage.hitTimeMs, stage.actionDurationMs));
					MarkDirty(EditU32("Input open", stage.inputOpenMs, 0u, stage.actionDurationMs));
					MarkDirty(EditU32("Input close", stage.inputCloseMs, 0u, stage.actionDurationMs));
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::TextDisabled("Effect binding: %s", skill.effectId.empty() ?
				"not authored" : skill.effectId.c_str());
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderBossEditor()
{
	if (m_bosses.empty())
		return;
	BOSS_EDIT& boss = m_bosses[m_selectedBoss];
	const std::string target = "boss:" + boss.archetypeId;
	ImGui::Text("%s (%s)", boss.displayName.c_str(), boss.archetypeId.c_str());
	ImGui::SeparatorText("Base stats");
	MarkDirty(EditU32("Maximum HP", boss.maximumHp, 1u, 4000000000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "maximumHp");
	MarkDirty(EditU32("Maximum health bars", boss.maximumHealthBars, 1u, 1000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "maximumHealthBars");
	MarkDirty(EditU32("Attack power", boss.attackPower, 1u, 1000000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "attackPower");
	MarkDirty(EditDouble("Collision radius", boss.collisionRadius,
		0.1f, 0.1, 100.0));
	ImGui::SeparatorText("Detection and movement");
	MarkDirty(EditDouble("Engage distance", boss.engageDistance,
		0.1f, 0.1, 1000.0));
	RenderBasis("Data/Balance/BossProfiles.json", target, "engageDistance");
	MarkDirty(EditDouble("Move speed", boss.moveSpeed, 0.01f, 0.01, 100.0));
	RenderBasis("Data/Balance/BossProfiles.json", target, "moveSpeed");
	ImGui::SeparatorText("Phase");
	MarkDirty(EditU32("Phase 2 HP %", boss.phaseTwoHpPercent, 1u, 99u));
	ImGui::TextDisabled("Current server behavior: phase byte changes; no separate phase pattern set yet.");
	ImGui::SeparatorText("Patterns");
	const HUD_BOSS_STATE& liveBoss = CCombatHUDViewModel::Get().Get_Boss();
	std::uint32_t liveHealthBar = 0u;
	if (liveBoss.isValid && liveBoss.iCurrentHp > 0u && liveBoss.iMaximumHp > 0u)
	{
		const std::uint64_t scaled = static_cast<std::uint64_t>(liveBoss.iCurrentHp) *
			boss.maximumHealthBars;
		liveHealthBar = static_cast<std::uint32_t>(
			(scaled + liveBoss.iMaximumHp - 1u) / liveBoss.iMaximumHp);
	}
	for (std::size_t index = 0; index < m_patterns.size(); ++index)
	{
		PATTERN_EDIT& pattern = m_patterns[index];
		ImGui::PushID(static_cast<int>(index));
		const std::string header = pattern.displayName + "##" + pattern.patternId;
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Pattern ID: %s", pattern.patternId.c_str());
			ImGui::TextDisabled("Semantic action: %s", pattern.actionId.c_str());
			std::ostringstream sourceActions;
			for (std::size_t sourceIndex = 0;
				sourceIndex < pattern.sourceActionIds.size(); ++sourceIndex)
			{
				if (sourceIndex > 0u)
					sourceActions << ", ";
				sourceActions << pattern.sourceActionIds[sourceIndex];
			}
			ImGui::TextWrapped("Original Actions: %s", sourceActions.str().c_str());
			int selectionMode = pattern.selectionMode == "HEALTH_BAR" ? 1 : 0;
			if (ImGui::Combo("Selection mode", &selectionMode,
				"Normal pool\0Health bar trigger\0"))
			{
				pattern.selectionMode = 0 == selectionMode ? "NORMAL" : "HEALTH_BAR";
				m_dirty = true;
			}
			if (pattern.selectionMode == "NORMAL")
			{
				MarkDirty(EditU32("Minimum health bar", pattern.minimumHealthBar, 1u,
					boss.maximumHealthBars));
				MarkDirty(EditU32("Maximum health bar", pattern.maximumHealthBar,
					pattern.minimumHealthBar, boss.maximumHealthBars));
				MarkDirty(EditU32("Selection weight", pattern.selectionWeight, 1u, 100000u));
				MarkDirty(EditU32("Maximum consecutive uses",
					pattern.maximumConsecutiveUses, 1u, 100u));
				if (liveBoss.isValid)
				{
					std::uint64_t eligibleWeight = 0u;
					for (const PATTERN_EDIT& candidate : m_patterns)
					{
						if (candidate.selectionMode == "NORMAL" &&
							liveHealthBar >= candidate.minimumHealthBar &&
							liveHealthBar <= candidate.maximumHealthBar)
						{
							eligibleWeight += candidate.selectionWeight;
						}
					}
					const bool barEligible = liveHealthBar >= pattern.minimumHealthBar &&
						liveHealthBar <= pattern.maximumHealthBar;
					ImGui::TextDisabled("Live decision: bar gate %s | weight %.1f%% | target range checked on Server",
						barEligible ? "PASS" : "BLOCK",
						barEligible && eligibleWeight > 0u ?
							100.0 * static_cast<double>(pattern.selectionWeight) /
							static_cast<double>(eligibleWeight) : 0.0);
				}
			}
			else
			{
				MarkDirty(EditU32("Trigger health bar", pattern.triggerHealthBar, 1u,
					boss.maximumHealthBars));
				MarkDirty(EditU32("Trigger order", pattern.triggerOrder, 1u, 100u));
				if (liveBoss.isValid)
					ImGui::TextDisabled("Live decision: threshold %s | once-only queue order %u",
						liveHealthBar > pattern.triggerHealthBar ? "ARMED" : "REACHED",
						pattern.triggerOrder);
			}
			MarkDirty(EditDouble("Minimum range", pattern.minimumRange,
				0.1f, 0.0, 1000.0));
			MarkDirty(EditDouble("Maximum range", pattern.maximumRange,
				0.1f, 0.0, 1000.0));
			ImGui::SeparatorText("Server stages");
			for (std::size_t stageIndex = 0;
				stageIndex < pattern.stages.size(); ++stageIndex)
			{
				PATTERN_STAGE_EDIT& stage = pattern.stages[stageIndex];
				ImGui::PushID(static_cast<int>(stageIndex));
				const std::string stageHeader = stage.stageId + " | " + stage.actionId;
				if (ImGui::TreeNode(stageHeader.c_str()))
				{
					int stageKind = stage.stageKind == "ACTIVE" ? 1 :
						(stage.stageKind == "RECOVERY" ? 2 : 0);
					if (ImGui::Combo("Kind", &stageKind,
						"Windup\0Active\0Recovery\0"))
					{
						stage.stageKind = 0 == stageKind ? "WINDUP" :
							(1 == stageKind ? "ACTIVE" : "RECOVERY");
						m_dirty = true;
					}
					MarkDirty(EditU32("Duration ms", stage.durationMs, 1u, 600000u));
					int hitShape = 0;
					if (stage.hitShape == "CIRCLE") hitShape = 1;
					else if (stage.hitShape == "RING") hitShape = 2;
					else if (stage.hitShape == "CONE") hitShape = 3;
					else if (stage.hitShape == "BOX") hitShape = 4;
					else if (stage.hitShape == "CROSS") hitShape = 5;
					else if (stage.hitShape == "SIX_DIRECTIONS") hitShape = 6;
					if (ImGui::Combo("Collider", &hitShape,
						"None\0Circle\0Ring\0Cone\0Box\0Cross\0Six directions\0"))
					{
						static const char* shapes[] =
							{ "NONE", "CIRCLE", "RING", "CONE", "BOX", "CROSS",
							  "SIX_DIRECTIONS" };
						stage.hitShape = shapes[hitShape];
						NormalizePatternStageForShape(stage);
						m_dirty = true;
					}
					if (stage.hitShape == "CIRCLE" || stage.hitShape == "RING")
						MarkDirty(EditDouble("Outer radius", stage.hitOuterRadius,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape == "RING")
						MarkDirty(EditDouble("Inner radius", stage.hitInnerRadius,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape == "CONE")
						MarkDirty(EditDouble("Angle degrees", stage.hitAngleDegrees,
							1.f, 1.0, 180.0));
					if (stage.hitShape == "CONE" || stage.hitShape == "BOX" ||
						stage.hitShape == "CROSS" ||
						stage.hitShape == "SIX_DIRECTIONS")
					{
						MarkDirty(EditDouble("Length", stage.hitLength,
							0.1f, 0.0, 1000.0));
					}
					if (stage.hitShape == "BOX" || stage.hitShape == "CROSS" ||
						stage.hitShape == "SIX_DIRECTIONS")
						MarkDirty(EditDouble("Half width", stage.hitHalfWidth,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape != "NONE")
					{
						MarkDirty(EditU32("Hit count", stage.hitCount, 1u, 100u));
						MarkDirty(EditU32("Hit interval ms", stage.hitIntervalMs,
							1u == stage.hitCount ? 0u : 1u, 600000u));
						MarkDirty(EditU32("Hit delay ms", stage.hitDelayMs,
							0u, 600000u));
						std::uint32_t* rate = FindDamageRate(stage.damageProfileId);
						if (nullptr != rate)
						{
							MarkDirty(EditU32("Damage rate %", *rate, 1u, 100000u));
							RenderBasis("Data/Balance/DamageProfiles.json",
								"damage:" + stage.damageProfileId, "damageRatePercent");
						}
						const bool pushRangeChanged = EditDouble(
							"Push range m (neg pulls)", stage.pushRangeM,
							0.1f, -20.0, 20.0);
						if (pushRangeChanged)
						{
							NormalizePatternStagePush(stage);
							m_dirty = true;
						}
						MarkDirty(EditU32("Push ms",
							stage.pushMs, 0.0 == stage.pushRangeM ? 0u : 1u, 600000u));
						if (ImGui::Checkbox("Knockdown", &stage.knockdown))
						{
							NormalizePatternStagePush(stage);
							m_dirty = true;
						}
						MarkDirty(EditU32("Down ms",
							stage.downMs, stage.knockdown ? 1u : 0u, 600000u));
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TextDisabled("Authority: Server 30 Hz | target: nearest alive player | geometry: XZ plane");
			if (liveBoss.isValid && liveBoss.strPatternId == pattern.patternId)
				ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "LIVE: selected by Server snapshot");
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderLiveVerification()
{
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	ImGui::TextUnformatted("Server snapshot");
	ImGui::Separator();
	if (player.isValid)
	{
		ImGui::Text("Player HP %u / %u", player.iCurrentHp, player.iMaximumHp);
		ImGui::Text("Resource %u / %u", player.iCurrentResource, player.iMaximumResource);
		ImGui::Text("Server tick %u", player.iServerTick);
		ImGui::Text("Combat ready: %s", player.isCombatReady ? "YES" : "PROTECTED");
		if (0u == player.iCurrentHp)
		{
			ImGui::BeginDisabled(nullptr == m_commandSink);
			if (ImGui::Button("Revive at death position"))
			{
				m_reviveSequence = m_reviveSequence ==
					(std::numeric_limits<std::uint32_t>::max)() ?
					1u : m_reviveSequence + 1u;
				m_status = m_commandSink->Request_RevivePlayer(m_reviveSequence) ?
					"Revive requested. Waiting for Server snapshot." :
					"Revive request failed: no active Server connection.";
			}
			ImGui::EndDisabled();
		}
	}
	else
		ImGui::TextDisabled("No replicated player snapshot");
	if (boss.isValid)
	{
		ImGui::SeparatorText("Boss live state");
		std::uint32_t currentHealthBar = 0u;
		if (!m_bosses.empty() && boss.iCurrentHp > 0u && boss.iMaximumHp > 0u)
		{
			const std::uint64_t scaled = static_cast<std::uint64_t>(boss.iCurrentHp) *
				m_bosses[m_selectedBoss].maximumHealthBars;
			currentHealthBar = static_cast<std::uint32_t>(
				(scaled + boss.iMaximumHp - 1u) / boss.iMaximumHp);
		}
		ImGui::Text("%s  HP %u / %u | bars %u / %u", boss.strDisplayName.c_str(),
			boss.iCurrentHp, boss.iMaximumHp, currentHealthBar,
			m_bosses.empty() ? 0u : m_bosses[m_selectedBoss].maximumHealthBars);
		ImGui::Text("Phase %u | pattern %s", boss.iPhase,
			boss.strPatternId.empty() ? "IDLE" : boss.strPatternId.c_str());
		ImGui::Text("Sequence %u | stage %u | action %s",
			boss.iPatternSequence, boss.iPatternStageIndex,
			boss.strActionId.empty() ? "-" : boss.strActionId.c_str());
		const auto selected = std::find_if(m_patterns.begin(), m_patterns.end(),
			[&boss](const PATTERN_EDIT& pattern)
			{ return pattern.patternId == boss.strPatternId; });
		if (m_patterns.end() != selected)
			ImGui::Text("Selected pattern: %s (%s)", selected->displayName.c_str(),
				selected->selectionMode.c_str());
	}
	else
		ImGui::TextDisabled("No replicated boss snapshot");
	ImGui::SeparatorText("Damage events");
	const auto& events = CCombatHUDViewModel::Get().Get_DamageEvents();
	const std::size_t begin = events.size() > 16u ? events.size() - 16u : 0u;
	for (std::size_t index = events.size(); index > begin; --index)
	{
		const HUD_DAMAGE_EVENT& event = events[index - 1u];
		ImGui::Text("t%u %s %u -> entity %u", event.iServerTick,
			event.Event.isOutgoing ? "OUT" : "IN",
			event.Event.iAmount, event.Event.iTargetNetEntityId);
	}
	if (events.empty())
		ImGui::TextDisabled("Use a server-approved skill or let Valtan hit a player.");
	ImGui::SeparatorText("Apply policy");
	ImGui::TextWrapped("Save updates Data authoring and provenance. Publish validates and cooks the Server bootstrap. Restart Server to apply; live hot reload is intentionally disabled.");
}
#endif

bool Client::CBalanceTool::ValidateDraft(std::string& status) const
{
	if (m_players.size() != 6u || m_skills.size() != 94u ||
		m_damageProfiles.size() != 108u || m_bosses.size() != 1u ||
		m_patterns.size() != 33u)
	{
		status = "Draft object counts are incomplete.";
		return false;
	}
	const auto isKnownStance = [](const std::string& value)
	{
		return value == "NONE" || value == "LANCE_MASTER_LONG_SPEAR" ||
			value == "LANCE_MASTER_SHORT_SPEAR" || value == "WARLORD_NORMAL" ||
			value == "WARLORD_DEFENSE";
	};
	std::uint32_t maximumPlayerResource = 0u;
	std::uint32_t maximumPlayerIdentity = 0u;
	for (const PLAYER_EDIT& player : m_players)
	{
		if (0u == player.maximumHp || 0u == player.maximumResource ||
			0u == player.resourceRegenPerSecond || 0u == player.attackPower ||
			0u == player.defense || player.resourceRegenPerSecond > player.maximumResource ||
			!std::isfinite(player.moveSpeed) || player.moveSpeed <= 0.f ||
			!std::isfinite(player.defenseStanceMoveSpeedScale) ||
			player.defenseStanceMoveSpeedScale <= 0.f ||
			player.defenseStanceMoveSpeedScale > 1.f ||
			player.identityCyclic > 1u ||
			player.identityStanceSwitchCost > player.maximumIdentity ||
			(0u == player.maximumIdentity &&
				(0u != player.identityRegenPerSecond ||
				 0u != player.identityDrainPerSecond ||
				 0u != player.identityStanceSwitchCost ||
				 0u != player.identityCyclic)) ||
			(0u != player.identityCyclic &&
				(0u == player.identityRegenPerSecond ||
				 0u != player.identityDrainPerSecond ||
				 0u != player.identityStanceSwitchCost)) ||
			!isKnownStance(player.defaultStance))
		{
			status = "Player draft is invalid: " + player.characterClass;
			return false;
		}
		maximumPlayerResource = (std::max)(
			maximumPlayerResource, player.maximumResource);
		maximumPlayerIdentity = (std::max)(
			maximumPlayerIdentity, player.maximumIdentity);
	}
	std::unordered_set<std::uint32_t> skillIds;
	std::unordered_set<std::string> classesWithIdentityCost;
	for (const SKILL_EDIT& skill : m_skills)
	{
		const bool dealsDamage = !skill.damageProfileId.empty();
		const bool isActive = "ACTIVE" == skill.skillKind;
		const bool isCombo = "COMBO" == skill.skillKind;
		const bool isHold = "HOLD" == skill.skillKind;
		const bool isCounter = "COUNTER" == skill.skillKind;
		const bool isStandup = "STANDUP" == skill.skillKind;
		if (0u == skill.skillId || 0u == skill.actionDurationMs ||
			!skillIds.insert(skill.skillId).second ||
			skill.characterClass.empty() || skill.inputSlot.empty() ||
			skill.displayName.empty() || skill.actionId.empty() ||
			skill.hitTimeMs > skill.actionDurationMs ||
			skill.resourceCost > maximumPlayerResource ||
			skill.identityCost > maximumPlayerIdentity ||
			!std::isfinite(skill.maximumRange) ||
			!std::isfinite(skill.movementDistance) || skill.movementDistance < 0.f ||
			(dealsDamage && (nullptr == FindDamageRate(skill.damageProfileId) ||
				skill.maximumRange <= 0.f)) ||
			(!dealsDamage && (skill.maximumRange != 0.f || 0u != skill.hitTimeMs)) ||
			!(isActive || isCombo || isHold || isCounter || isStandup) ||
			((isActive || isStandup) &&
				(0u == skill.cooldownMs || !skill.comboStages.empty())) ||
			(isStandup && (dealsDamage || "NONE" != skill.requiredStance ||
				"NONE" != skill.setsStance)) ||
			(isCombo &&
				(skill.comboStages.size() < 2u || skill.comboStages.size() > 8u)) ||
			(isHold && (0u == skill.cooldownMs || 3u != skill.comboStages.size())) ||
			(isCounter && (0u == skill.cooldownMs || 2u != skill.comboStages.size())) ||
			!isKnownStance(skill.requiredStance) || !isKnownStance(skill.setsStance))
		{
			status = "Skill draft is invalid: " + std::to_string(skill.skillId);
			return false;
		}
		if (0u != skill.identityCost)
			classesWithIdentityCost.insert(skill.characterClass);

		std::uint64_t stagedDurationMs = 0u;
		for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
		{
			const COMBO_STAGE_EDIT& stage = skill.comboStages[index];
			const bool basicTimingInvalid = 0u == stage.actionDurationMs ||
				stage.hitTimeMs > stage.comboAdvanceMs ||
				stage.comboAdvanceMs > stage.actionDurationMs;
			const bool comboWindowInvalid = isCombo &&
				((index + 1u < skill.comboStages.size() &&
					(stage.inputOpenMs >= stage.inputCloseMs ||
						stage.inputCloseMs > stage.actionDurationMs)) ||
				(index + 1u == skill.comboStages.size() &&
					(stage.comboAdvanceMs != stage.actionDurationMs ||
					 0u != stage.inputOpenMs || 0u != stage.inputCloseMs)));
			const bool holdStageInvalid = isHold &&
				(0u != stage.inputOpenMs || 0u != stage.inputCloseMs ||
					((index + 1u == skill.comboStages.size()) !=
						(0u != stage.hitTimeMs)));
			const bool counterStageInvalid = isCounter &&
				((0u == index &&
					(0u != stage.hitTimeMs ||
					 stage.inputOpenMs >= stage.inputCloseMs ||
					 stage.inputCloseMs > stage.actionDurationMs)) ||
				 (1u == index &&
					(0u == stage.hitTimeMs || 0u != stage.inputOpenMs ||
					 0u != stage.inputCloseMs)));
			if (basicTimingInvalid || comboWindowInvalid ||
				holdStageInvalid || counterStageInvalid)
			{
				status = "Staged skill draft is invalid: " +
					std::to_string(skill.skillId);
				return false;
			}
			stagedDurationMs += stage.actionDurationMs;
		}
		if (isHold && stagedDurationMs != skill.actionDurationMs)
		{
			status = "Hold stage duration sum is invalid: " +
				std::to_string(skill.skillId);
			return false;
		}
	}
	for (const PLAYER_EDIT& player : m_players)
	{
		if (0u != player.maximumIdentity &&
			0u == player.identityDrainPerSecond &&
			0u == player.identityStanceSwitchCost &&
			0u == player.identityCyclic &&
			classesWithIdentityCost.end() ==
				classesWithIdentityCost.find(player.characterClass))
		{
			status = "Player identity gauge has no spending path: " +
				player.characterClass;
			return false;
		}
	}
	for (const DAMAGE_EDIT& damage : m_damageProfiles)
	{
		if (damage.damageProfileId.empty() || 0u == damage.damageRatePercent)
		{
			status = "Damage profile draft is invalid: " + damage.damageProfileId;
			return false;
		}
	}
	for (const BOSS_EDIT& boss : m_bosses)
	{
		if (boss.archetypeId.empty() || boss.encounterId.empty() ||
			0u == boss.maximumHp || 0u == boss.maximumHealthBars ||
			boss.maximumHealthBars > 1000u || 0u == boss.attackPower ||
			!std::isfinite(boss.collisionRadius) || boss.collisionRadius <= 0.f ||
			!std::isfinite(boss.engageDistance) || boss.engageDistance <= 0.f ||
			!std::isfinite(boss.moveSpeed) || boss.moveSpeed <= 0.f ||
			boss.phaseTwoHpPercent < 1u || boss.phaseTwoHpPercent > 99u)
		{
			status = "Boss draft is invalid: " + boss.archetypeId;
			return false;
		}
	}
	if (m_encounterId.empty() || m_encounterBossArchetypeId.empty() ||
		"server" != m_encounterAuthority || 30u != m_fixedTickHz ||
		m_encounterIntroPatternId.empty() ||
		m_encounterBossArchetypeId != m_bosses.front().archetypeId ||
		m_encounterId != m_bosses.front().encounterId)
	{
		status = "Encounter draft header is invalid.";
		return false;
	}
	std::unordered_set<std::string> patternIds;
	std::unordered_set<std::string> serverMotionAnchorIds;
	bool foundIntroPattern = false;
	for (const PATTERN_EDIT& pattern : m_patterns)
	{
		foundIntroPattern = foundIntroPattern ||
			pattern.patternId == m_encounterIntroPatternId;
		const bool normal = pattern.selectionMode == "NORMAL";
		const bool healthBar = pattern.selectionMode == "HEALTH_BAR";
		const bool validSelection = normal ?
			(pattern.minimumHealthBar >= 1u &&
				pattern.maximumHealthBar >= pattern.minimumHealthBar &&
				pattern.maximumHealthBar <= m_bosses[m_selectedBoss].maximumHealthBars &&
				0u == pattern.triggerHealthBar && 0u == pattern.triggerOrder &&
				pattern.selectionWeight > 0u && pattern.maximumConsecutiveUses > 0u) :
			(healthBar && 0u == pattern.minimumHealthBar &&
				0u == pattern.maximumHealthBar && pattern.triggerHealthBar >= 1u &&
				pattern.triggerHealthBar <= m_bosses[m_selectedBoss].maximumHealthBars &&
				pattern.triggerOrder > 0u && 0u == pattern.selectionWeight &&
				0u == pattern.maximumConsecutiveUses);
		if (pattern.patternId.empty() ||
			!patternIds.insert(pattern.patternId).second ||
			pattern.displayName.empty() ||
			pattern.actionId.empty() || pattern.sourceActionIds.empty() ||
			pattern.stages.empty() || !validSelection ||
			!std::isfinite(pattern.minimumRange) || pattern.minimumRange < 0.f ||
			!std::isfinite(pattern.maximumRange) ||
			pattern.maximumRange <= pattern.minimumRange)
		{
			status = "Pattern draft is invalid: " + pattern.patternId;
			return false;
		}
		if (pattern.serverMotion.enabled &&
			("LEAP_TO_ANCHOR" != pattern.serverMotion.kind ||
			 pattern.serverMotion.anchorId.empty() ||
			 !serverMotionAnchorIds.insert(
				 pattern.serverMotion.anchorId).second ||
			 !std::isfinite(pattern.serverMotion.landingX) ||
			 !std::isfinite(pattern.serverMotion.landingY) ||
			 !std::isfinite(pattern.serverMotion.landingZ) ||
			 std::abs(pattern.serverMotion.landingX) > 100000.f ||
			 std::abs(pattern.serverMotion.landingY) > 100000.f ||
			 std::abs(pattern.serverMotion.landingZ) > 100000.f ||
			 !std::isfinite(pattern.serverMotion.apexHeight) ||
			 pattern.serverMotion.apexHeight <= 0.f ||
			 pattern.serverMotion.apexHeight > 200.f))
		{
			status = "Pattern server motion draft is invalid: " +
				pattern.patternId;
			return false;
		}
		for (const PATTERN_STAGE_EDIT& stage : pattern.stages)
		{
			const bool none = stage.hitShape == "NONE";
			const bool circle = stage.hitShape == "CIRCLE";
			const bool ring = stage.hitShape == "RING";
			const bool cone = stage.hitShape == "CONE";
			const bool box = stage.hitShape == "BOX";
			const bool cross = stage.hitShape == "CROSS";
			const bool sixDirections = stage.hitShape == "SIX_DIRECTIONS";
			const bool validKind = stage.stageKind == "WINDUP" ||
				stage.stageKind == "ACTIVE" || stage.stageKind == "RECOVERY";
			const bool finiteGeometry = std::isfinite(stage.hitOuterRadius) &&
				std::isfinite(stage.hitInnerRadius) &&
				std::isfinite(stage.hitAngleDegrees) &&
				std::isfinite(stage.hitLength) &&
				std::isfinite(stage.hitHalfWidth);
			const bool validGeometry = none ?
				(0.0 == stage.hitOuterRadius && 0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees && 0.0 == stage.hitLength &&
					0.0 == stage.hitHalfWidth) :
				(circle ? stage.hitOuterRadius > 0.0 &&
					0.0 == stage.hitInnerRadius && 0.0 == stage.hitAngleDegrees &&
					0.0 == stage.hitLength && 0.0 == stage.hitHalfWidth :
				(ring ? stage.hitInnerRadius > 0.0 &&
					stage.hitOuterRadius > stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees && 0.0 == stage.hitLength &&
					0.0 == stage.hitHalfWidth :
				(cone ? stage.hitAngleDegrees > 0.0 &&
					stage.hitAngleDegrees <= 180.0 && stage.hitLength > 0.0 &&
					0.0 == stage.hitOuterRadius && 0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitHalfWidth :
				((box || cross || sixDirections) ? stage.hitLength > 0.0 &&
					stage.hitHalfWidth > 0.0 && 0.0 == stage.hitOuterRadius &&
					0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees : false))));
			const bool validHit = none ?
				(0u == stage.hitCount && 0u == stage.hitIntervalMs &&
					0u == stage.hitDelayMs &&
					stage.damageProfileId.empty()) :
				(stage.hitCount > 0u &&
					(1u == stage.hitCount ? 0u == stage.hitIntervalMs :
						stage.hitIntervalMs > 0u) &&
					stage.hitDelayMs < stage.durationMs &&
					static_cast<std::uint64_t>(stage.hitCount - 1u) *
						stage.hitIntervalMs < stage.durationMs &&
					nullptr != FindDamageRate(stage.damageProfileId));
			const bool validPush = std::isfinite(stage.pushRangeM) &&
				std::abs(stage.pushRangeM) <= 20.0 &&
				((0.0 == stage.pushRangeM) == (0u == stage.pushMs)) &&
				(stage.knockdown == (0u != stage.downMs)) &&
				(!stage.damageProfileId.empty() ||
					(0.0 == stage.pushRangeM && !stage.knockdown));
			if (stage.stageId.empty() || stage.actionId.empty() || !validKind ||
				0u == stage.durationMs || !finiteGeometry || !validGeometry ||
				!validHit || !validPush)
			{
				status = "Pattern stage draft is invalid: " + pattern.patternId +
					"/" + stage.stageId;
				return false;
			}
		}
	}
	if (!foundIntroPattern)
	{
		status = "Encounter intro pattern does not exist.";
		return false;
	}
	status = "Draft validation passed.";
	return true;
}

bool Client::CBalanceTool::RunPipeline(const wchar_t* scriptName,
	const wchar_t* arguments, std::string& status) const
{
	const std::filesystem::path projectRoot = CProjectDataRoot::Get().parent_path();
	const std::filesystem::path script = projectRoot / L"Tools" / scriptName;
	if (!std::filesystem::is_regular_file(script))
	{
		status = "Pipeline script is missing.";
		return false;
	}
	std::wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		script.wstring() + L"\"";
	if (nullptr != arguments && L'\0' != arguments[0])
		command += L" " + std::wstring(arguments);
	std::vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	if (!CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
		CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup, &process))
	{
		status = "Could not start the balance pipeline.";
		return false;
	}
	CloseHandle(process.hThread);
	const DWORD wait = WaitForSingleObject(process.hProcess, 120000u);
	if (WAIT_TIMEOUT == wait)
	{
		TerminateProcess(process.hProcess, 124u);
		WaitForSingleObject(process.hProcess, 5000u);
		CloseHandle(process.hProcess);
		status = "Balance pipeline timed out and its owned process was terminated.";
		return false;
	}
	DWORD exitCode = 1u;
	const bool succeeded = WAIT_OBJECT_0 == wait &&
		GetExitCodeProcess(process.hProcess, &exitCode) && 0u == exitCode;
	CloseHandle(process.hProcess);
	status = succeeded ? "Balance pipeline succeeded." :
		"Balance pipeline failed with exit code " + std::to_string(exitCode) + ".";
	return succeeded;
}

bool Client::CBalanceTool::Save(
	SERIALIZED_DRAFT_DOCUMENTS* const readOnlyCapture)
{
	if (nullptr == readOnlyCapture && !m_dirty)
	{
		m_status = "No balance changes to save; authoring bytes were left untouched.";
		return true;
	}
	std::string status;
	if (!ValidateDraft(status))
	{
		m_status = status;
		return false;
	}
	std::ostringstream players;
	players << "{\n  \"schema\": \"lostark.player-profiles\",\n  \"formatVersion\": 2,\n  \"players\": [\n";
	for (std::size_t i = 0; i < m_players.size(); ++i)
	{
		const PLAYER_EDIT& p = m_players[i];
		players << "    {\n      \"characterClass\": " << Quote(p.characterClass)
			<< ",\n      \"maximumHp\": " << p.maximumHp
			<< ",\n      \"maximumResource\": " << p.maximumResource
			<< ",\n      \"resourceRegenPerSecond\": " << p.resourceRegenPerSecond
			<< ",\n      \"attackPower\": " << p.attackPower
			<< ",\n      \"defense\": " << p.defense
			<< ",\n      \"moveSpeed\": " << FormatJsonNumber(p.moveSpeed)
			<< ",\n      \"defenseStanceMoveSpeedScale\": "
			<< FormatJsonNumber(p.defenseStanceMoveSpeedScale)
			<< ",\n      \"maximumIdentity\": " << p.maximumIdentity
			<< ",\n      \"identityRegenPerSecond\": " << p.identityRegenPerSecond
			<< ",\n      \"identityDrainPerSecond\": " << p.identityDrainPerSecond
			<< ",\n      \"identityStanceSwitchCost\": " << p.identityStanceSwitchCost
			<< ",\n      \"identityCyclic\": " << p.identityCyclic
			<< ",\n      \"defaultStance\": " << Quote(p.defaultStance) << "\n    }"
			<< (i + 1u == m_players.size() ? "\n" : ",\n");
	}
	players << "  ]\n}\n";

	std::ostringstream damage;
	damage << "{\n  \"schema\": \"lostark.damage-profiles\",\n  \"formatVersion\": 2,\n  \"profiles\": [\n";
	for (std::size_t i = 0; i < m_damageProfiles.size(); ++i)
	{
		const DAMAGE_EDIT& p = m_damageProfiles[i];
		damage << "    { \"damageProfileId\": " << Quote(p.damageProfileId)
			<< ", \"damageRatePercent\": " << p.damageRatePercent << " }"
			<< (i + 1u == m_damageProfiles.size() ? "\n" : ",\n");
	}
	damage << "  ]\n}\n";

	std::ostringstream skills;
	skills << "{\n  \"schema\": \"lostark.player-skills\",\n  \"formatVersion\": 2,\n  \"skills\": [\n";
	for (std::size_t i = 0; i < m_skills.size(); ++i)
	{
		const SKILL_EDIT& s = m_skills[i];
		skills << "    {\n      \"skillId\": " << s.skillId
			<< ",\n      \"characterClass\": " << Quote(s.characterClass)
			<< ",\n      \"inputSlot\": " << Quote(s.inputSlot)
			<< ",\n      \"displayName\": " << Quote(s.displayName)
			<< ",\n      \"actionId\": " << Quote(s.actionId)
			<< ",\n      \"skillKind\": " << Quote(s.skillKind)
			<< ",\n      \"cooldownMs\": " << s.cooldownMs
			<< ",\n      \"actionDurationMs\": " << s.actionDurationMs
			<< ",\n      \"hitTimeMs\": " << s.hitTimeMs
			<< ",\n      \"resourceCost\": " << s.resourceCost
			<< ",\n      \"identityCost\": " << s.identityCost
			<< ",\n      \"movementDistance\": " << FormatJsonNumber(s.movementDistance)
			<< ",\n      \"maximumRange\": " << FormatJsonNumber(s.maximumRange)
			<< ",\n      \"serverDamageProfileId\": " << Quote(s.damageProfileId)
			<< ",\n      \"effectId\": " << Quote(s.effectId)
			<< ",\n      \"requiredStance\": " << Quote(s.requiredStance)
			<< ",\n      \"setsStance\": " << Quote(s.setsStance)
			<< ",\n      \"comboStages\": [";
		if (!s.comboStages.empty()) skills << "\n";
		for (std::size_t stageIndex = 0; stageIndex < s.comboStages.size(); ++stageIndex)
		{
			const COMBO_STAGE_EDIT& stage = s.comboStages[stageIndex];
			skills << "        { \"actionDurationMs\": " << stage.actionDurationMs
				<< ", \"hitTimeMs\": " << stage.hitTimeMs
				<< ", \"comboAdvanceMs\": " << stage.comboAdvanceMs
				<< ", \"inputOpenMs\": " << stage.inputOpenMs
				<< ", \"inputCloseMs\": " << stage.inputCloseMs << " }"
				<< (stageIndex + 1u == s.comboStages.size() ? "\n" : ",\n");
		}
		skills << "      ]\n    }" << (i + 1u == m_skills.size() ? "\n" : ",\n");
	}
	skills << "  ]\n}\n";

	std::ostringstream bosses;
	bosses << "{\n  \"schema\": \"lostark.boss-profiles\",\n  \"formatVersion\": 3,\n  \"bosses\": [\n";
	for (std::size_t i = 0; i < m_bosses.size(); ++i)
	{
		const BOSS_EDIT& b = m_bosses[i];
		bosses << "    {\n      \"archetypeId\": " << Quote(b.archetypeId)
			<< ",\n      \"encounterId\": " << Quote(b.encounterId)
			<< ",\n      \"displayName\": " << Quote(b.displayName)
			<< ",\n      \"maximumHp\": " << b.maximumHp
			<< ",\n      \"maximumHealthBars\": " << b.maximumHealthBars
			<< ",\n      \"attackPower\": " << b.attackPower
			<< ",\n      \"collisionRadius\": " << FormatJsonNumber(b.collisionRadius)
			<< ",\n      \"engageDistance\": " << FormatJsonNumber(b.engageDistance)
			<< ",\n      \"moveSpeed\": " << FormatJsonNumber(b.moveSpeed)
			<< ",\n      \"phaseTwoHpPercent\": " << b.phaseTwoHpPercent << "\n    }"
			<< (i + 1u == m_bosses.size() ? "\n" : ",\n");
	}
	bosses << "  ]\n}\n";

	std::ostringstream encounter;
	encounter << "{\n  \"schema\": \"lostark.encounter-profile\",\n  \"formatVersion\": 3,"
		<< "\n  \"encounterId\": " << Quote(m_encounterId)
		<< ",\n  \"bossArchetypeId\": " << Quote(m_encounterBossArchetypeId)
		<< ",\n  \"authority\": " << Quote(m_encounterAuthority)
		<< ",\n  \"fixedTickHz\": " << m_fixedTickHz
		<< ",\n  \"introPatternId\": " << Quote(m_encounterIntroPatternId)
		<< ",\n  \"states\": [\n";
	for (std::size_t i = 0; i < m_encounterStates.size(); ++i)
	{
		const ENCOUNTER_STATE_EDIT& state = m_encounterStates[i];
		encounter << "    { \"id\": " << Quote(state.id)
			<< ", \"actionId\": " << Quote(state.actionId) << ", \"next\": "
			<< (state.hasNext ? Quote(state.next) : "null") << " }"
			<< (i + 1u == m_encounterStates.size() ? "\n" : ",\n");
	}
	encounter << "  ],\n  \"patterns\": [\n";
	for (std::size_t i = 0; i < m_patterns.size(); ++i)
	{
		const PATTERN_EDIT& p = m_patterns[i];
		encounter << "    {\n      \"patternId\": " << Quote(p.patternId)
			<< ",\n      \"displayName\": " << Quote(p.displayName)
			<< ",\n      \"actionId\": " << Quote(p.actionId)
			<< ",\n      \"sourceActionIds\": [";
		for (std::size_t sourceIndex = 0;
			sourceIndex < p.sourceActionIds.size(); ++sourceIndex)
		{
			if (sourceIndex > 0u)
				encounter << ", ";
			encounter << p.sourceActionIds[sourceIndex];
		}
		encounter << "],\n      \"selectionMode\": " << Quote(p.selectionMode)
			<< ",\n      \"minimumHealthBar\": " << p.minimumHealthBar
			<< ",\n      \"maximumHealthBar\": " << p.maximumHealthBar
			<< ",\n      \"triggerHealthBar\": " << p.triggerHealthBar
			<< ",\n      \"triggerOrder\": " << p.triggerOrder
			<< ",\n      \"armorRequirement\": "
			<< Quote(p.armorRequirement)
			<< ",\n      \"phaseRequirement\": "
			<< Quote(p.phaseRequirement)
			<< ",\n      \"invulnerableWhileRunning\": "
			<< (p.invulnerableWhileRunning ? "true" : "false")
			<< ",\n      \"selectionWeight\": " << p.selectionWeight
			<< ",\n      \"maximumConsecutiveUses\": " << p.maximumConsecutiveUses
			<< ",\n      \"minimumRange\": " << FormatJsonNumber(p.minimumRange)
			<< ",\n      \"maximumRange\": " << FormatJsonNumber(p.maximumRange);
		if (p.serverMotion.enabled)
		{
			encounter << ",\n      \"serverMotion\": {\n"
				<< "        \"kind\": " << Quote(p.serverMotion.kind)
				<< ",\n        \"anchorId\": " << Quote(p.serverMotion.anchorId)
				<< ",\n        \"landingPosition\": ["
				<< FormatJsonNumber(p.serverMotion.landingX) << ", "
				<< FormatJsonNumber(p.serverMotion.landingY) << ", "
				<< FormatJsonNumber(p.serverMotion.landingZ)
				<< "],\n        \"apexHeight\": "
				<< FormatJsonNumber(p.serverMotion.apexHeight)
				<< "\n      }";
		}
		encounter << ",\n      \"stages\": [\n";
		for (std::size_t stageIndex = 0; stageIndex < p.stages.size(); ++stageIndex)
		{
			const PATTERN_STAGE_EDIT& stage = p.stages[stageIndex];
			encounter << "        { \"stageId\": " << Quote(stage.stageId)
				<< ", \"actionId\": " << Quote(stage.actionId)
				<< ", \"stageKind\": " << Quote(stage.stageKind)
				<< ", \"durationMs\": " << stage.durationMs
				<< ", \"hitShape\": " << Quote(stage.hitShape)
				<< ", \"hitOuterRadius\": " << FormatJsonNumber(stage.hitOuterRadius)
				<< ", \"hitInnerRadius\": " << FormatJsonNumber(stage.hitInnerRadius)
				<< ", \"hitAngleDegrees\": " << FormatJsonNumber(stage.hitAngleDegrees)
				<< ", \"hitLength\": " << FormatJsonNumber(stage.hitLength)
				<< ", \"hitHalfWidth\": " << FormatJsonNumber(stage.hitHalfWidth)
				<< ", \"hitCount\": " << stage.hitCount
				<< ", \"hitIntervalMs\": " << stage.hitIntervalMs
				<< ", \"hitDelayMs\": " << stage.hitDelayMs
				<< ", \"serverDamageProfileId\": " << Quote(stage.damageProfileId)
				<< ", \"pushRangeM\": " << FormatJsonNumber(stage.pushRangeM)
				<< ", \"pushMs\": " << stage.pushMs
				<< ", \"knockdown\": " << (stage.knockdown ? "true" : "false")
				<< ", \"downMs\": " << stage.downMs
				<< " }" << (stageIndex + 1u == p.stages.size() ? "\n" : ",\n");
		}
		encounter << "      ]\n    }"
			<< (i + 1u == m_patterns.size() ? "\n" : ",\n");
	}
	encounter << "  ]\n}\n";
	if (nullptr != readOnlyCapture)
	{
		readOnlyCapture->players = players.str();
		readOnlyCapture->skills = skills.str();
		readOnlyCapture->damage = damage.str();
		readOnlyCapture->bosses = bosses.str();
		readOnlyCapture->encounter = encounter.str();
		return true;
	}

	struct WRITE final { std::filesystem::path relative; std::string text; };
	const std::vector<WRITE> writes{
		{ L"Balance/PlayerProfiles.json", players.str() },
		{ L"Balance/PlayerSkills.json", skills.str() },
		{ L"Balance/DamageProfiles.json", damage.str() },
		{ L"Balance/BossProfiles.json", bosses.str() },
		{ L"Encounters/Valtan/ValtanEncounter.json", encounter.str() } };
	std::vector<std::filesystem::path> temporaries;
	std::vector<std::filesystem::path> backups;
	std::size_t promotedCount = 0u;
	const std::wstring suffix = L"." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetTickCount64());
	for (const WRITE& write : writes)
	{
		const std::filesystem::path destination = CProjectDataRoot::Resolve(write.relative);
		std::filesystem::path temporary = destination;
		temporary += L".balance.tmp" + suffix;
		if (destination.empty() || !DurableWrite(temporary, write.text, status) ||
			!ParseStagedJson(temporary, status))
		{
			for (const auto& path : temporaries) { std::error_code error; std::filesystem::remove(path, error); }
			m_status = "Save failed before commit: " + status;
			return false;
		}
		temporaries.push_back(temporary);
	}
	for (std::size_t i = 0; i < writes.size(); ++i)
	{
		const std::filesystem::path destination = CProjectDataRoot::Resolve(writes[i].relative);
		std::filesystem::path backup = destination;
		backup += L".balance.rollback" + suffix;
		if (!CopyFileW(destination.c_str(), backup.c_str(), TRUE))
		{
			status = "Could not create balance rollback copy.";
			break;
		}
		backups.push_back(backup);
		if (!MoveFileExW(temporaries[i].c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			status = "Could not promote balance staging file.";
			break;
		}
		++promotedCount;
	}
	if (promotedCount != writes.size())
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination = CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		for (const auto& path : temporaries) { std::error_code error; std::filesystem::remove(path, error); }
		m_status = "Save rolled back: " + status;
		return false;
	}
	const std::filesystem::path receipt = CProjectDataRoot::Resolve(
		L"Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json");
	std::filesystem::path receiptBackup = receipt;
	receiptBackup += L".balance.rollback" + suffix;
	if (receipt.empty() || !CopyFileW(receipt.c_str(), receiptBackup.c_str(), TRUE))
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination =
				CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		m_status = "Save rolled back: could not protect the provenance receipt.";
		return false;
	}
	if (!RunPipeline(L"GameplayPipeline\\Update-BalanceProvenanceReceipt.ps1", L"", status) ||
		!RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1", L"-Mode Validate", status))
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination =
				CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		MoveFileExW(receiptBackup.c_str(), receipt.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		m_status = "Save rolled back after provenance/validation failure: " + status;
		return false;
	}
	for (const auto& path : backups)
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}
	{
		std::error_code error;
		std::filesystem::remove(receiptBackup, error);
	}
	m_dirty = false;
	m_status = "Saved authoring, synchronized changed fields to PROJECT_TUNED, and validated.";
	Reload();
	return true;
}

#if defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
bool Client::CBalanceTool::Run_ReadOnlyRoundTripContractTest(
	std::string& status)
{
	CBalanceTool tool(nullptr);
	std::string validationStatus;
	if (!tool.Reload() || !tool.ValidateDraft(validationStatus))
	{
		status = "Balance Tool could not load/validate current authoring: " +
			(tool.m_status.empty() ? validationStatus : tool.m_status);
		return false;
	}

	SERIALIZED_DRAFT_DOCUMENTS serialized;
	if (!tool.Save(&serialized))
	{
		status = "Balance Tool read-only serialization failed: " + tool.m_status;
		return false;
	}
	const auto parseRoot = [&status](const std::string& text,
		DATA_JSON_VALUE& root, const char* name)
	{
		std::string parseStatus;
		if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
		{
			status = std::string(name) + " serialization is invalid JSON: " +
				parseStatus;
			return false;
		}
		return true;
	};

	DATA_JSON_VALUE playerRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE bossRoot;
	DATA_JSON_VALUE encounterRoot;
	if (!parseRoot(serialized.players, playerRoot, "PlayerProfiles") ||
		!parseRoot(serialized.skills, skillRoot, "PlayerSkills") ||
		!parseRoot(serialized.damage, damageRoot, "DamageProfiles") ||
		!parseRoot(serialized.bosses, bossRoot, "BossProfiles") ||
		!parseRoot(serialized.encounter, encounterRoot, "ValtanEncounter"))
	{
		return false;
	}

	const auto semanticallyEqual = [](const auto& self,
		const DATA_JSON_VALUE& left, const DATA_JSON_VALUE& right) -> bool
	{
		if (left.Get_Type() != right.Get_Type())
			return false;
		switch (left.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE:
			return true;
		case DATA_JSON_TYPE::BOOLEAN:
			return left.Get_Boolean() == right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return left.Get_Number() == right.Get_Number();
		case DATA_JSON_TYPE::STRING:
			return left.Get_String() == right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			if (left.Get_Array().size() != right.Get_Array().size())
				return false;
			for (std::size_t index = 0u; index < left.Get_Array().size(); ++index)
			{
				if (!self(self, left.Get_Array()[index], right.Get_Array()[index]))
					return false;
			}
			return true;
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			if (left.Get_Object().size() != right.Get_Object().size())
				return false;
			for (const auto& [key, value] : left.Get_Object())
			{
				const DATA_JSON_VALUE* other = right.Find(key);
				if (nullptr == other || !self(self, value, *other))
					return false;
			}
			return true;
		}
		default:
			return false;
		}
	};
	DATA_JSON_VALUE originalPlayerRoot;
	DATA_JSON_VALUE originalSkillRoot;
	DATA_JSON_VALUE originalDamageRoot;
	DATA_JSON_VALUE originalBossRoot;
	DATA_JSON_VALUE originalEncounterRoot;
	std::string readStatus;
	if (!ReadJson(L"Balance/PlayerProfiles.json", originalPlayerRoot, readStatus) ||
		!ReadJson(L"Balance/PlayerSkills.json", originalSkillRoot, readStatus) ||
		!ReadJson(L"Balance/DamageProfiles.json", originalDamageRoot, readStatus) ||
		!ReadJson(L"Balance/BossProfiles.json", originalBossRoot, readStatus) ||
		!ReadJson(L"Encounters/Valtan/ValtanEncounter.json",
			originalEncounterRoot, readStatus) ||
		!semanticallyEqual(semanticallyEqual, originalPlayerRoot, playerRoot) ||
		!semanticallyEqual(semanticallyEqual, originalSkillRoot, skillRoot) ||
		!semanticallyEqual(semanticallyEqual, originalDamageRoot, damageRoot) ||
		!semanticallyEqual(semanticallyEqual, originalBossRoot, bossRoot) ||
		!semanticallyEqual(semanticallyEqual, originalEncounterRoot, encounterRoot))
	{
		status = "Balance Tool serializer did not preserve all five authoring documents: " +
			readStatus;
		return false;
	}

	const DATA_JSON_VALUE* players =
		Field(playerRoot, "players", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* skills =
		Field(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* patterns =
		Field(encounterRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == players || players->Get_Array().size() != 6u ||
		nullptr == skills || skills->Get_Array().size() != 94u ||
		nullptr == patterns || patterns->Get_Array().size() != 33u)
	{
		status = "Balance Tool round-trip changed current object counts.";
		return false;
	}

	for (std::size_t index = 0; index < tool.m_players.size(); ++index)
	{
		const PLAYER_EDIT& expected = tool.m_players[index];
		const DATA_JSON_VALUE& actual = players->Get_Array()[index];
		double stanceScale = 0.0;
		std::uint32_t maximumIdentity = 0u;
		std::uint32_t regen = 0u;
		std::uint32_t drain = 0u;
		std::uint32_t switchCost = 0u;
		std::uint32_t cyclic = 0u;
		if (!ReadDouble(actual, "defenseStanceMoveSpeedScale", stanceScale) ||
			!ReadU32(actual, "maximumIdentity", maximumIdentity) ||
			!ReadU32(actual, "identityRegenPerSecond", regen) ||
			!ReadU32(actual, "identityDrainPerSecond", drain) ||
			!ReadU32(actual, "identityStanceSwitchCost", switchCost) ||
			!ReadU32(actual, "identityCyclic", cyclic) ||
			stanceScale != expected.defenseStanceMoveSpeedScale ||
			maximumIdentity != expected.maximumIdentity ||
			regen != expected.identityRegenPerSecond ||
			drain != expected.identityDrainPerSecond ||
			switchCost != expected.identityStanceSwitchCost ||
			cyclic != expected.identityCyclic)
		{
			status = "Balance Tool round-trip changed player identity fields.";
			return false;
		}
	}

	std::size_t activeCount = 0u;
	std::size_t comboCount = 0u;
	std::size_t holdCount = 0u;
	std::size_t counterCount = 0u;
	std::size_t standupCount = 0u;
	bool artistMoonCost = false;
	bool artistSunCost = false;
	for (std::size_t index = 0; index < tool.m_skills.size(); ++index)
	{
		const SKILL_EDIT& expected = tool.m_skills[index];
		const DATA_JSON_VALUE& actual = skills->Get_Array()[index];
		std::uint32_t skillId = 0u;
		std::uint32_t identityCost = 0u;
		const DATA_JSON_VALUE* stages =
			Field(actual, "comboStages", DATA_JSON_TYPE::ARRAY);
		if (!ReadU32(actual, "skillId", skillId) || skillId != expected.skillId ||
			!ReadU32(actual, "identityCost", identityCost) ||
			identityCost != expected.identityCost || nullptr == stages ||
			stages->Get_Array().size() != expected.comboStages.size())
		{
			status = "Balance Tool round-trip changed skill identity/stage fields.";
			return false;
		}
		for (std::size_t stageIndex = 0;
			stageIndex < expected.comboStages.size(); ++stageIndex)
		{
			std::uint32_t comboAdvanceMs = 0u;
			if (!ReadU32(stages->Get_Array()[stageIndex], "comboAdvanceMs",
				comboAdvanceMs) ||
				comboAdvanceMs != expected.comboStages[stageIndex].comboAdvanceMs)
			{
				status = "Balance Tool round-trip changed comboAdvanceMs.";
				return false;
			}
		}
		activeCount += "ACTIVE" == expected.skillKind ? 1u : 0u;
		comboCount += "COMBO" == expected.skillKind ? 1u : 0u;
		holdCount += "HOLD" == expected.skillKind ? 1u : 0u;
		counterCount += "COUNTER" == expected.skillKind ? 1u : 0u;
		standupCount += "STANDUP" == expected.skillKind ? 1u : 0u;
		artistMoonCost = artistMoonCost ||
			(31110u == expected.skillId && 33u == identityCost);
		artistSunCost = artistSunCost ||
			(31050u == expected.skillId && 66u == identityCost);
	}
	if (76u != activeCount || 11u != comboCount || 2u != holdCount ||
		1u != counterCount || 4u != standupCount ||
		!artistMoonCost || !artistSunCost)
	{
		status = "Balance Tool did not preserve all skill kinds/identity costs.";
		return false;
	}

	std::string introPatternId;
	if (!ReadString(encounterRoot, "introPatternId", introPatternId) ||
		introPatternId != tool.m_encounterIntroPatternId)
	{
		status = "Balance Tool round-trip changed introPatternId.";
		return false;
	}
	std::size_t serverMotionCount = 0u;
	for (std::size_t index = 0; index < tool.m_patterns.size(); ++index)
	{
		const PATTERN_EDIT& expected = tool.m_patterns[index];
		const DATA_JSON_VALUE* motion =
			patterns->Get_Array()[index].Find("serverMotion");
		if (expected.serverMotion.enabled)
		{
			++serverMotionCount;
			if (nullptr == motion || !motion->Is_Object())
			{
				status = "Balance Tool round-trip changed serverMotion.";
				return false;
			}
			std::string kind;
			std::string anchorId;
			double apexHeight = 0.0;
			const DATA_JSON_VALUE* landing =
				Field(*motion, "landingPosition", DATA_JSON_TYPE::ARRAY);
			const bool exactLanding = nullptr != landing &&
				3u == landing->Get_Array().size() &&
				landing->Get_Array()[0].Is_Number() &&
				landing->Get_Array()[1].Is_Number() &&
				landing->Get_Array()[2].Is_Number() &&
				landing->Get_Array()[0].Get_Number() == expected.serverMotion.landingX &&
				landing->Get_Array()[1].Get_Number() == expected.serverMotion.landingY &&
				landing->Get_Array()[2].Get_Number() == expected.serverMotion.landingZ;
			if (!ReadString(*motion, "kind", kind) ||
				!ReadString(*motion, "anchorId", anchorId) ||
				!ReadDouble(*motion, "apexHeight", apexHeight) ||
				!exactLanding ||
				kind != expected.serverMotion.kind ||
				anchorId != expected.serverMotion.anchorId ||
				apexHeight != expected.serverMotion.apexHeight)
			{
				status = "Balance Tool round-trip changed serverMotion.";
				return false;
			}
		}
		else if (nullptr != motion)
		{
			status = "Balance Tool invented an optional serverMotion.";
			return false;
		}
	}
	if (1u != serverMotionCount)
	{
		status = "Balance Tool did not preserve the one authored serverMotion.";
		return false;
	}

	std::size_t normalizedPatternIndex = tool.m_patterns.size();
	std::size_t normalizedStageIndex = 0u;
	for (std::size_t patternIndex = 0u;
		patternIndex < tool.m_patterns.size(); ++patternIndex)
	{
		for (std::size_t stageIndex = 0u;
			stageIndex < tool.m_patterns[patternIndex].stages.size(); ++stageIndex)
		{
			const PATTERN_STAGE_EDIT& candidate =
				tool.m_patterns[patternIndex].stages[stageIndex];
			if ("NONE" != candidate.hitShape && !candidate.damageProfileId.empty())
			{
				normalizedPatternIndex = patternIndex;
				normalizedStageIndex = stageIndex;
				break;
			}
		}
		if (normalizedPatternIndex != tool.m_patterns.size())
			break;
	}
	if (normalizedPatternIndex == tool.m_patterns.size())
	{
		status = "Balance Tool normalization test could not find a damage stage.";
		return false;
	}
	PATTERN_STAGE_EDIT& normalizedStage =
		tool.m_patterns[normalizedPatternIndex].stages[normalizedStageIndex];
	const PATTERN_STAGE_EDIT originalStage = normalizedStage;
	normalizedStage.hitShape = "CONE";
	normalizedStage.hitOuterRadius = 123.0;
	normalizedStage.hitInnerRadius = 45.0;
	normalizedStage.hitAngleDegrees = 73.25;
	normalizedStage.hitLength = 4.75;
	normalizedStage.hitHalfWidth = 67.0;
	NormalizePatternStageForShape(normalizedStage);
	normalizedStage.pushRangeM = 0.0;
	normalizedStage.pushMs = 999u;
	normalizedStage.knockdown = false;
	normalizedStage.downMs = 999u;
	NormalizePatternStagePush(normalizedStage);
	SERIALIZED_DRAFT_DOCUMENTS normalizedDocuments;
	if (!tool.Save(&normalizedDocuments))
	{
		status = "A publisher-compatible collider/push edit was rejected: " +
			tool.m_status;
		return false;
	}
	DATA_JSON_VALUE normalizedEncounter;
	if (!parseRoot(normalizedDocuments.encounter, normalizedEncounter,
		"Normalized ValtanEncounter"))
	{
		return false;
	}
	const DATA_JSON_VALUE* normalizedPatterns =
		Field(normalizedEncounter, "patterns", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* normalizedStages = nullptr;
	if (nullptr != normalizedPatterns &&
		normalizedPatternIndex < normalizedPatterns->Get_Array().size())
	{
		normalizedStages = Field(
			normalizedPatterns->Get_Array()[normalizedPatternIndex],
			"stages", DATA_JSON_TYPE::ARRAY);
	}
	double outer = -1.0;
	double inner = -1.0;
	double angle = -1.0;
	double length = -1.0;
	double halfWidth = -1.0;
	double pushRange = -1.0;
	std::uint32_t hitDelayMs =
		(std::numeric_limits<std::uint32_t>::max)();
	std::uint32_t pushMs = 1u;
	std::uint32_t downMs = 1u;
	const DATA_JSON_VALUE* normalizedStageValue =
		nullptr != normalizedStages &&
		normalizedStageIndex < normalizedStages->Get_Array().size() ?
		&normalizedStages->Get_Array()[normalizedStageIndex] : nullptr;
	const DATA_JSON_VALUE* knockdownValue = nullptr == normalizedStageValue ?
		nullptr : normalizedStageValue->Find("knockdown");
	const bool normalizedRoundTrip = nullptr != normalizedStageValue &&
		ReadDouble(*normalizedStageValue, "hitOuterRadius", outer) &&
		ReadDouble(*normalizedStageValue, "hitInnerRadius", inner) &&
		ReadDouble(*normalizedStageValue, "hitAngleDegrees", angle) &&
		ReadDouble(*normalizedStageValue, "hitLength", length) &&
		ReadDouble(*normalizedStageValue, "hitHalfWidth", halfWidth) &&
		ReadU32(*normalizedStageValue, "hitDelayMs", hitDelayMs) &&
		ReadDouble(*normalizedStageValue, "pushRangeM", pushRange) &&
		ReadU32(*normalizedStageValue, "pushMs", pushMs) &&
		ReadU32(*normalizedStageValue, "downMs", downMs) &&
		nullptr != knockdownValue && knockdownValue->Is_Boolean() &&
		0.0 == outer && 0.0 == inner && 73.25 == angle &&
		4.75 == length && 0.0 == halfWidth && 0.0 == pushRange &&
		hitDelayMs == originalStage.hitDelayMs &&
		0u == pushMs && !knockdownValue->Get_Boolean() && 0u == downMs;
	PATTERN_STAGE_EDIT noneStage = originalStage;
	noneStage.hitShape = "NONE";
	noneStage.hitOuterRadius = 1.0;
	noneStage.hitInnerRadius = 1.0;
	noneStage.hitAngleDegrees = 1.0;
	noneStage.hitLength = 1.0;
	noneStage.hitHalfWidth = 1.0;
	noneStage.hitDelayMs = 1u;
	noneStage.pushRangeM = 1.0;
	noneStage.pushMs = 1u;
	noneStage.knockdown = true;
	noneStage.downMs = 1u;
	NormalizePatternStageForShape(noneStage);
	const bool noneWasCleared = 0.0 == noneStage.hitOuterRadius &&
		0.0 == noneStage.hitInnerRadius && 0.0 == noneStage.hitAngleDegrees &&
		0.0 == noneStage.hitLength && 0.0 == noneStage.hitHalfWidth &&
		0u == noneStage.hitCount && 0u == noneStage.hitIntervalMs &&
		0u == noneStage.hitDelayMs && noneStage.damageProfileId.empty() &&
		0.0 == noneStage.pushRangeM &&
		0u == noneStage.pushMs && !noneStage.knockdown && 0u == noneStage.downMs;
	PATTERN_STAGE_EDIT pairedPush = originalStage;
	pairedPush.pushRangeM = 1.0;
	pairedPush.pushMs = 0u;
	pairedPush.knockdown = true;
	pairedPush.downMs = 0u;
	NormalizePatternStagePush(pairedPush);
	const bool pushWasPaired = 1u == pairedPush.pushMs && 1u == pairedPush.downMs;
	PATTERN_STAGE_EDIT hiddenGeometry = originalStage;
	hiddenGeometry.hitShape = "CIRCLE";
	hiddenGeometry.hitOuterRadius = 2.0;
	hiddenGeometry.hitInnerRadius = 0.0;
	hiddenGeometry.hitAngleDegrees = 45.0;
	hiddenGeometry.hitLength = 0.0;
	hiddenGeometry.hitHalfWidth = 0.0;
	normalizedStage = hiddenGeometry;
	const bool hiddenGeometryRejected = !tool.ValidateDraft(validationStatus);
	PATTERN_STAGE_EDIT unpairedPush = originalStage;
	unpairedPush.pushRangeM = 0.0;
	unpairedPush.pushMs = 50u;
	normalizedStage = unpairedPush;
	const bool unpairedPushRejected = !tool.ValidateDraft(validationStatus);
	normalizedStage = originalStage;
	if (!normalizedRoundTrip || !noneWasCleared || !pushWasPaired ||
		!hiddenGeometryRejected || !unpairedPushRejected)
	{
		status = "Balance Tool collider/push normalization did not round-trip exactly.";
		return false;
	}

	auto dimensionMaster = std::find_if(tool.m_skills.begin(), tool.m_skills.end(),
		[](const SKILL_EDIT& skill) { return 2050010u == skill.skillId; });
	if (tool.m_skills.end() == dimensionMaster ||
		dimensionMaster->comboStages.empty())
	{
		status = "DimensionMaster BA was absent from the Balance Tool draft.";
		return false;
	}
	dimensionMaster->comboStages.front().comboAdvanceMs = 1300u;
	SERIALIZED_DRAFT_DOCUMENTS edited;
	if (!tool.Save(&edited))
	{
		status = "A valid comboAdvanceMs edit was rejected.";
		return false;
	}
	DATA_JSON_VALUE editedSkillRoot;
	if (!parseRoot(edited.skills, editedSkillRoot, "Edited PlayerSkills"))
		return false;
	const DATA_JSON_VALUE* editedSkills =
		Field(editedSkillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	bool foundEditedAdvance = false;
	if (nullptr != editedSkills)
	{
		for (const DATA_JSON_VALUE& value : editedSkills->Get_Array())
		{
			std::uint32_t skillId = 0u;
			if (!ReadU32(value, "skillId", skillId) || 2050010u != skillId)
				continue;
			const DATA_JSON_VALUE* stages =
				Field(value, "comboStages", DATA_JSON_TYPE::ARRAY);
			std::uint32_t advance = 0u;
			foundEditedAdvance = nullptr != stages && !stages->Get_Array().empty() &&
				ReadU32(stages->Get_Array().front(), "comboAdvanceMs", advance) &&
				1300u == advance;
			break;
		}
	}
	if (!foundEditedAdvance)
	{
		status = "A valid comboAdvanceMs edit was not serialized.";
		return false;
	}
	dimensionMaster->comboStages.front().comboAdvanceMs =
		dimensionMaster->comboStages.front().hitTimeMs - 1u;
	if (tool.ValidateDraft(validationStatus))
	{
		status = "An invalid comboAdvanceMs edit was accepted.";
		return false;
	}

	status = "Balance Tool read-only round-trip preserved 6 players, 94 skills, "
		"108 damage profiles, 1 boss, 33 patterns, identityCost, all skill kinds, "
		"comboAdvanceMs, introPatternId, and serverMotion.";
	return true;
}
#else
bool Client::CBalanceTool::Run_ReadOnlyRoundTripContractTest(
	std::string& status)
{
	status = "Balance Tool read-only contract test is harness-only.";
	return false;
}
#endif

#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
void Client::CBalanceTool::Render()
{
	if (!m_open)
		return;
	ImGui::SetNextWindowSize(ImVec2(1180.f, 760.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Balance Tool", &m_open))
	{
		ImGui::End();
		return;
	}
	if (ImGui::Button("Players")) m_showPlayers = true;
	ImGui::SameLine();
	if (ImGui::Button("Bosses")) m_showPlayers = false;
	ImGui::SameLine();
	ImGui::TextDisabled("skill L10 / fixed L1 | %s", m_dirty ? "UNSAVED" : "saved");
	ImGui::SameLine();
	if (ImGui::Button("Reload")) Reload();
	ImGui::SameLine();
	if (ImGui::Button("Save + Validate")) Save();
	ImGui::SameLine();
	if (ImGui::Button("Validate"))
	{
		std::string status;
		m_status = RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1",
			L"-Mode Validate", status) ?
			"Validation succeeded." : status;
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(m_dirty);
	if (ImGui::Button("Publish Server Data"))
	{
		std::string status;
		m_status = RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1",
			L"-Mode Publish", status) ?
			"Published. Restart Server.exe to apply." : status;
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped("%s", m_status.c_str());

	const float listWidth = 210.f;
	const float liveWidth = 300.f;
	ImGui::BeginChild("##BalanceTargets", ImVec2(listWidth, 0.f), true);
	ImGui::SeparatorText(m_showPlayers ? "Characters" : "Bosses");
	if (m_showPlayers)
	{
		for (std::size_t i = 0; i < m_players.size(); ++i)
			if (ImGui::Selectable(m_players[i].characterClass.c_str(), i == m_selectedPlayer))
				m_selectedPlayer = i;
	}
	else
	{
		for (std::size_t i = 0; i < m_bosses.size(); ++i)
			if (ImGui::Selectable(m_bosses[i].displayName.c_str(), i == m_selectedBoss))
				m_selectedBoss = i;
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##BalanceEditor", ImVec2(-liveWidth - 8.f, 0.f), true);
	if (m_showPlayers) RenderPlayerEditor(); else RenderBossEditor();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##BalanceLive", ImVec2(0.f, 0.f), true);
	RenderLiveVerification();
	ImGui::EndChild();
	ImGui::End();
}
#endif
