#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class CBalanceTool final
{
public:
	CBalanceTool();
	void Render();

private:
	struct PLAYER_EDIT
	{
		std::string characterClass;
		std::uint32_t maximumHp = 0;
		std::uint32_t maximumResource = 0;
		std::uint32_t resourceRegenPerSecond = 0;
		std::uint32_t attackPower = 0;
		std::uint32_t defense = 0;
		float moveSpeed = 0.f;
	};

	struct COMBO_STAGE_EDIT
	{
		std::uint32_t actionDurationMs = 0;
		std::uint32_t hitTimeMs = 0;
		std::uint32_t inputOpenMs = 0;
		std::uint32_t inputCloseMs = 0;
	};

	struct SKILL_EDIT
	{
		std::uint32_t skillId = 0;
		std::string characterClass;
		std::string inputSlot;
		std::string displayName;
		std::string actionId;
		std::string skillKind;
		std::uint32_t cooldownMs = 0;
		std::uint32_t actionDurationMs = 0;
		std::uint32_t hitTimeMs = 0;
		std::uint32_t resourceCost = 0;
		float movementDistance = 0.f;
		float maximumRange = 0.f;
		std::string damageProfileId;
		std::string effectId;
		std::vector<COMBO_STAGE_EDIT> comboStages;
	};

	struct DAMAGE_EDIT
	{
		std::string damageProfileId;
		std::uint32_t damageRatePercent = 0;
	};

	struct BOSS_EDIT
	{
		std::string archetypeId;
		std::string encounterId;
		std::string displayName;
		std::uint32_t maximumHp = 0;
		std::uint32_t attackPower = 0;
		float collisionRadius = 0.f;
		float engageDistance = 0.f;
		float moveSpeed = 0.f;
		std::uint32_t phaseTwoHpPercent = 0;
	};

	struct PATTERN_EDIT
	{
		std::string patternId;
		std::string actionId;
		float minimumRange = 0.f;
		float maximumRange = 0.f;
		std::uint32_t telegraphMs = 0;
		std::uint32_t activeMs = 0;
		std::uint32_t recoveryMs = 0;
		std::string damageProfileId;
	};

	struct ENCOUNTER_STATE_EDIT
	{
		std::string id;
		std::string actionId;
		std::string next;
		bool hasNext = false;
	};

	bool Reload();
	bool Save();
	bool ValidateDraft(std::string& status) const;
	bool RunPipeline(const wchar_t* scriptName, const wchar_t* arguments,
		std::string& status) const;
	void RenderPlayerEditor();
	void RenderBossEditor();
	void RenderLiveVerification() const;
	void RenderBasis(const std::string& document, const std::string& targetId,
		const std::string& field) const;
	std::uint32_t* FindDamageRate(const std::string& damageProfileId);
	const std::uint32_t* FindDamageRate(const std::string& damageProfileId) const;
	void MarkDirty(bool changed);

	std::vector<PLAYER_EDIT> m_players;
	std::vector<SKILL_EDIT> m_skills;
	std::vector<DAMAGE_EDIT> m_damageProfiles;
	std::vector<BOSS_EDIT> m_bosses;
	std::vector<PATTERN_EDIT> m_patterns;
	std::vector<ENCOUNTER_STATE_EDIT> m_encounterStates;
	std::string m_encounterId;
	std::string m_encounterBossArchetypeId;
	std::string m_encounterAuthority;
	std::uint32_t m_fixedTickHz = 30;
	std::unordered_map<std::string, std::string> m_basisByField;
	std::size_t m_selectedPlayer = 0;
	std::size_t m_selectedBoss = 0;
	bool m_showPlayers = true;
	bool m_dirty = false;
	bool m_open = true;
	std::string m_status;
};

NS_END
