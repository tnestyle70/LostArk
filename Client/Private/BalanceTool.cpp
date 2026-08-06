#include "imgui.h"

#include "BalanceTool.h"

#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <io.h>
#include <limits>
#include <sstream>

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

	bool ReadFloat(const DATA_JSON_VALUE& object, const char* name, float& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == value)
			return false;
		output = static_cast<float>(value->Get_Number());
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

	bool EditU32(const char* label, std::uint32_t& value,
		const std::uint32_t minimum, const std::uint32_t maximum)
	{
		const bool changed = ImGui::InputScalar(
			label, ImGuiDataType_U32, &value, nullptr, nullptr, "%u");
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}

	std::string Quote(const std::string& value)
	{
		return "\"" + CDataJson::Escape(value) + "\"";
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

Client::CBalanceTool::CBalanceTool()
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
		!HasSchemaVersion(bossRoot, "lostark.boss-profiles", 2u) ||
		!IsExactObject(encounterRoot, { "schema", "formatVersion", "encounterId",
			"bossArchetypeId", "authority", "fixedTickHz", "states", "patterns" }) ||
		!HasSchemaVersion(encounterRoot, "lostark.encounter-profile", 1u) ||
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
			"defaultStance" }) ||
			!ReadString(value, "characterClass", row.characterClass) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "maximumResource", row.maximumResource) ||
			!ReadU32(value, "resourceRegenPerSecond", row.resourceRegenPerSecond) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadU32(value, "defense", row.defense) ||
			!ReadFloat(value, "moveSpeed", row.moveSpeed) ||
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
			"resourceCost", "movementDistance", "maximumRange", "serverDamageProfileId",
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
			!ReadFloat(value, "movementDistance", row.movementDistance) ||
			!ReadFloat(value, "maximumRange", row.maximumRange) ||
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
				"inputOpenMs", "inputCloseMs" }) ||
				!ReadU32(stageValue, "actionDurationMs", stage.actionDurationMs) ||
				!ReadU32(stageValue, "hitTimeMs", stage.hitTimeMs) ||
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
		if (!IsExactObject(value, { "archetypeId", "encounterId", "displayName", "maximumHp",
			"attackPower", "collisionRadius", "engageDistance", "moveSpeed",
			"phaseTwoHpPercent" }) ||
			!ReadString(value, "archetypeId", row.archetypeId) ||
			!ReadString(value, "encounterId", row.encounterId) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadFloat(value, "collisionRadius", row.collisionRadius) ||
			!ReadFloat(value, "engageDistance", row.engageDistance) ||
			!ReadFloat(value, "moveSpeed", row.moveSpeed) ||
			!ReadU32(value, "phaseTwoHpPercent", row.phaseTwoHpPercent))
			return false;
		bosses.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : patternArray->Get_Array())
	{
		PATTERN_EDIT row{};
		if (!IsExactObject(value, { "patternId", "actionId", "minimumRange", "maximumRange",
			"telegraphMs", "activeMs", "recoveryMs", "serverDamageProfileId" }) ||
			!ReadString(value, "patternId", row.patternId) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadFloat(value, "minimumRange", row.minimumRange) ||
			!ReadFloat(value, "maximumRange", row.maximumRange) ||
			!ReadU32(value, "telegraphMs", row.telegraphMs) ||
			!ReadU32(value, "activeMs", row.activeMs) ||
			!ReadU32(value, "recoveryMs", row.recoveryMs) ||
			!ReadString(value, "serverDamageProfileId", row.damageProfileId))
			return false;
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
	std::uint32_t fixedTickHz = 0;
	if (!ReadString(encounterRoot, "encounterId", encounterId) ||
		!ReadString(encounterRoot, "bossArchetypeId", bossArchetypeId) ||
		!ReadString(encounterRoot, "authority", authority) ||
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
	m_fixedTickHz = fixedTickHz;
	m_basisByField = std::move(bases);
	m_selectedPlayer = (std::min)(m_selectedPlayer,
		m_players.empty() ? 0u : m_players.size() - 1u);
	m_selectedBoss = (std::min)(m_selectedBoss,
		m_bosses.empty() ? 0u : m_bosses.size() - 1u);
	m_dirty = false;
	m_status = "Loaded authoring balance and 1,058 field provenance entries.";
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
	MarkDirty(ImGui::DragFloat("Move speed", &player.moveSpeed, 0.01f, 0.01f, 100.f, "%.2f"));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "moveSpeed");
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
				skill.skillKind == "ACTIVE" ? 1u : 0u, 600000u));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "cooldownMs");
			MarkDirty(EditU32("Resource cost", skill.resourceCost, 0u, player.maximumResource));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "resourceCost");
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
			MarkDirty(ImGui::DragFloat("Maximum range", &skill.maximumRange, 0.1f, 0.1f, 1000.f));
			MarkDirty(ImGui::DragFloat("Skill movement distance", &skill.movementDistance, 0.1f, 0.f, 1000.f));
			if (!skill.comboStages.empty() && ImGui::TreeNode("Basic attack combo stages"))
			{
				for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
				{
					COMBO_STAGE_EDIT& stage = skill.comboStages[index];
					ImGui::PushID(static_cast<int>(index));
					ImGui::SeparatorText(("Stage " + std::to_string(index + 1u)).c_str());
					MarkDirty(EditU32("Duration", stage.actionDurationMs, 1u, 600000u));
					MarkDirty(EditU32("Hit", stage.hitTimeMs, 0u, stage.actionDurationMs));
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
	MarkDirty(EditU32("Attack power", boss.attackPower, 1u, 1000000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "attackPower");
	MarkDirty(ImGui::DragFloat("Collision radius", &boss.collisionRadius, 0.1f, 0.1f, 100.f));
	ImGui::SeparatorText("Detection and movement");
	MarkDirty(ImGui::DragFloat("Engage distance", &boss.engageDistance, 0.1f, 0.1f, 1000.f));
	RenderBasis("Data/Balance/BossProfiles.json", target, "engageDistance");
	MarkDirty(ImGui::DragFloat("Move speed", &boss.moveSpeed, 0.01f, 0.01f, 100.f));
	RenderBasis("Data/Balance/BossProfiles.json", target, "moveSpeed");
	ImGui::SeparatorText("Phase");
	MarkDirty(EditU32("Phase 2 HP %", boss.phaseTwoHpPercent, 1u, 99u));
	ImGui::TextDisabled("Current server behavior: phase byte changes; no separate phase pattern set yet.");
	ImGui::SeparatorText("Patterns");
	for (std::size_t index = 0; index < m_patterns.size(); ++index)
	{
		PATTERN_EDIT& pattern = m_patterns[index];
		ImGui::PushID(static_cast<int>(index));
		if (ImGui::CollapsingHeader(pattern.patternId.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Animation action: %s", pattern.actionId.c_str());
			MarkDirty(ImGui::DragFloat("Minimum range", &pattern.minimumRange, 0.1f, 0.f, 1000.f));
			MarkDirty(ImGui::DragFloat("Maximum range", &pattern.maximumRange, 0.1f, 0.f, 1000.f));
			MarkDirty(EditU32("Telegraph ms", pattern.telegraphMs, 1u, 600000u));
			MarkDirty(EditU32("Active ms", pattern.activeMs, 1u, 600000u));
			MarkDirty(EditU32("Recovery ms", pattern.recoveryMs, 1u, 600000u));
			std::uint32_t* rate = FindDamageRate(pattern.damageProfileId);
			if (nullptr != rate)
			{
				MarkDirty(EditU32("Damage rate %", *rate, 1u, 100000u));
				RenderBasis("Data/Balance/DamageProfiles.json",
					"damage:" + pattern.damageProfileId, "damageRatePercent");
			}
			ImGui::TextDisabled("Target: nearest alive player | hit: radial 2D/all players/once on active start");
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderLiveVerification() const
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
	}
	else
		ImGui::TextDisabled("No replicated player snapshot");
	if (boss.isValid)
	{
		ImGui::SeparatorText("Boss live state");
		ImGui::Text("%s  HP %u / %u", boss.strDisplayName.c_str(),
			boss.iCurrentHp, boss.iMaximumHp);
		ImGui::Text("Phase %u | action %s", boss.iPhase, boss.strActionId.c_str());
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

bool Client::CBalanceTool::ValidateDraft(std::string& status) const
{
	if (m_players.size() != 5u || m_skills.size() != 66u ||
		m_damageProfiles.size() != 63u || m_bosses.empty() || m_patterns.empty())
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
	for (const PLAYER_EDIT& player : m_players)
	{
		if (0u == player.maximumHp || 0u == player.maximumResource ||
			0u == player.resourceRegenPerSecond || 0u == player.attackPower ||
			0u == player.defense || player.resourceRegenPerSecond > player.maximumResource ||
			!std::isfinite(player.moveSpeed) || player.moveSpeed <= 0.f ||
			!isKnownStance(player.defaultStance))
		{
			status = "Player draft is invalid: " + player.characterClass;
			return false;
		}
	}
	for (const SKILL_EDIT& skill : m_skills)
	{
		const bool dealsDamage = !skill.damageProfileId.empty();
		if (0u == skill.skillId || 0u == skill.actionDurationMs ||
			skill.hitTimeMs > skill.actionDurationMs ||
			!std::isfinite(skill.maximumRange) ||
			!std::isfinite(skill.movementDistance) || skill.movementDistance < 0.f ||
			(dealsDamage && (nullptr == FindDamageRate(skill.damageProfileId) ||
				skill.maximumRange <= 0.f)) ||
			(!dealsDamage && (skill.maximumRange != 0.f || 0u != skill.hitTimeMs)) ||
			(skill.skillKind == "ACTIVE" &&
				(0u == skill.cooldownMs || !skill.comboStages.empty())) ||
			(skill.skillKind == "COMBO" &&
				(skill.comboStages.size() < 2u || skill.comboStages.size() > 8u)) ||
			(skill.skillKind != "ACTIVE" && skill.skillKind != "COMBO") ||
			!isKnownStance(skill.requiredStance) || !isKnownStance(skill.setsStance))
		{
			status = "Skill draft is invalid: " + std::to_string(skill.skillId);
			return false;
		}
		for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
		{
			const COMBO_STAGE_EDIT& stage = skill.comboStages[index];
			if (0u == stage.actionDurationMs || stage.hitTimeMs > stage.actionDurationMs ||
				(index + 1u < skill.comboStages.size() &&
					(stage.inputOpenMs >= stage.inputCloseMs ||
						stage.inputCloseMs > stage.actionDurationMs)) ||
				(index + 1u == skill.comboStages.size() &&
					(0u != stage.inputOpenMs || 0u != stage.inputCloseMs)))
			{
				status = "Combo draft is invalid: " + std::to_string(skill.skillId);
				return false;
			}
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
			0u == boss.maximumHp || 0u == boss.attackPower ||
			!std::isfinite(boss.collisionRadius) || boss.collisionRadius <= 0.f ||
			!std::isfinite(boss.engageDistance) || boss.engageDistance <= 0.f ||
			!std::isfinite(boss.moveSpeed) || boss.moveSpeed <= 0.f ||
			boss.phaseTwoHpPercent < 1u || boss.phaseTwoHpPercent > 99u)
		{
			status = "Boss draft is invalid: " + boss.archetypeId;
			return false;
		}
	}
	for (const PATTERN_EDIT& pattern : m_patterns)
	{
		if (pattern.patternId.empty() || pattern.actionId.empty() ||
			!std::isfinite(pattern.minimumRange) || pattern.minimumRange < 0.f ||
			!std::isfinite(pattern.maximumRange) ||
			pattern.maximumRange <= pattern.minimumRange ||
			0u == pattern.telegraphMs || 0u == pattern.activeMs ||
			0u == pattern.recoveryMs || nullptr == FindDamageRate(pattern.damageProfileId))
		{
			status = "Pattern draft is invalid: " + pattern.patternId;
			return false;
		}
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

bool Client::CBalanceTool::Save()
{
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
			<< ",\n      \"moveSpeed\": " << std::setprecision(9) << p.moveSpeed
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
			<< ",\n      \"movementDistance\": " << std::setprecision(9) << s.movementDistance
			<< ",\n      \"maximumRange\": " << s.maximumRange
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
				<< ", \"inputOpenMs\": " << stage.inputOpenMs
				<< ", \"inputCloseMs\": " << stage.inputCloseMs << " }"
				<< (stageIndex + 1u == s.comboStages.size() ? "\n" : ",\n");
		}
		skills << "      ]\n    }" << (i + 1u == m_skills.size() ? "\n" : ",\n");
	}
	skills << "  ]\n}\n";

	std::ostringstream bosses;
	bosses << "{\n  \"schema\": \"lostark.boss-profiles\",\n  \"formatVersion\": 2,\n  \"bosses\": [\n";
	for (std::size_t i = 0; i < m_bosses.size(); ++i)
	{
		const BOSS_EDIT& b = m_bosses[i];
		bosses << "    {\n      \"archetypeId\": " << Quote(b.archetypeId)
			<< ",\n      \"encounterId\": " << Quote(b.encounterId)
			<< ",\n      \"displayName\": " << Quote(b.displayName)
			<< ",\n      \"maximumHp\": " << b.maximumHp
			<< ",\n      \"attackPower\": " << b.attackPower
			<< ",\n      \"collisionRadius\": " << std::setprecision(9) << b.collisionRadius
			<< ",\n      \"engageDistance\": " << b.engageDistance
			<< ",\n      \"moveSpeed\": " << b.moveSpeed
			<< ",\n      \"phaseTwoHpPercent\": " << b.phaseTwoHpPercent << "\n    }"
			<< (i + 1u == m_bosses.size() ? "\n" : ",\n");
	}
	bosses << "  ]\n}\n";

	std::ostringstream encounter;
	encounter << "{\n  \"schema\": \"lostark.encounter-profile\",\n  \"formatVersion\": 1,"
		<< "\n  \"encounterId\": " << Quote(m_encounterId)
		<< ",\n  \"bossArchetypeId\": " << Quote(m_encounterBossArchetypeId)
		<< ",\n  \"authority\": " << Quote(m_encounterAuthority)
		<< ",\n  \"fixedTickHz\": " << m_fixedTickHz << ",\n  \"states\": [\n";
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
			<< ",\n      \"actionId\": " << Quote(p.actionId)
			<< ",\n      \"minimumRange\": " << std::setprecision(9) << p.minimumRange
			<< ",\n      \"maximumRange\": " << p.maximumRange
			<< ",\n      \"telegraphMs\": " << p.telegraphMs
			<< ",\n      \"activeMs\": " << p.activeMs
			<< ",\n      \"recoveryMs\": " << p.recoveryMs
			<< ",\n      \"serverDamageProfileId\": " << Quote(p.damageProfileId) << "\n    }"
			<< (i + 1u == m_patterns.size() ? "\n" : ",\n");
	}
	encounter << "  ]\n}\n";

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
