#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;

class CBalanceTool final
{
public:
	explicit CBalanceTool(std::shared_ptr<IPlayerCommandSink> commandSink);
	void Render();
	/* Loads the tracked authoring documents and exercises the same serializer
	used by Save without touching disk or launching the publisher. */
	static bool Run_ReadOnlyRoundTripContractTest(std::string& status);

private:
	struct PLAYER_EDIT
	{
		std::string characterClass;
		std::uint32_t maximumHp = 0;
		std::uint32_t maximumResource = 0;
		std::uint32_t resourceRegenPerSecond = 0;
		std::uint32_t attackPower = 0;
		std::uint32_t defense = 0;
		double moveSpeed = 0.0;
		double defenseStanceMoveSpeedScale = 1.0;
		std::uint32_t maximumIdentity = 0;
		std::uint32_t identityRegenPerSecond = 0;
		std::uint32_t identityDrainPerSecond = 0;
		std::uint32_t identityStanceSwitchCost = 0;
		std::uint32_t identityCyclic = 0;
		std::string defaultStance;
	};

	struct COMBO_STAGE_EDIT
	{
		std::uint32_t actionDurationMs = 0;
		std::uint32_t hitTimeMs = 0;
		std::uint32_t comboAdvanceMs = 0;
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
		std::uint32_t staggerDamage = 0;
		std::uint32_t partDamage = 0;
		std::uint32_t counterPower = 0;
		std::uint32_t cooldownMs = 0;
		std::uint32_t actionDurationMs = 0;
		std::uint32_t hitTimeMs = 0;
		std::uint32_t resourceCost = 0;
		std::uint32_t identityCost = 0;
		double movementDistance = 0.0;
		double maximumRange = 0.0;
		float movementDistance = 0.f;
		float maximumRange = 0.f;
		std::string damageProfileId;
		std::string effectId;
		std::string requiredStance;
		std::string setsStance;
		std::vector<COMBO_STAGE_EDIT> comboStages;
	};

	struct SERVER_MOTION_EDIT
	{
		bool enabled = false;
		std::string kind;
		std::string anchorId;
		double landingX = 0.0;
		double landingY = 0.0;
		double landingZ = 0.0;
		double apexHeight = 0.0;
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
		std::uint32_t maximumHealthBars = 0;
		std::uint32_t attackPower = 0;
		double collisionRadius = 0.0;
		double engageDistance = 0.0;
		double moveSpeed = 0.0;
		std::uint32_t phaseTwoHpPercent = 0;
	};

	struct PATTERN_STAGE_EDIT
	{
		std::string stageId;
		std::string actionId;
		std::string stageKind;
		std::uint32_t durationMs = 0;
		std::string hitShape;
		double hitOuterRadius = 0.0;
		double hitInnerRadius = 0.0;
		double hitAngleDegrees = 0.0;
		double hitLength = 0.0;
		double hitHalfWidth = 0.0;
		std::uint32_t hitCount = 0;
		std::uint32_t hitIntervalMs = 0;
		std::uint32_t hitDelayMs = 0;
		std::string damageProfileId;
		double pushRangeM = 0.0;
		std::uint32_t pushMs = 0;
		bool knockdown = false;
		std::uint32_t downMs = 0;
	};

	struct PATTERN_EDIT
	{
		std::string patternId;
		std::string displayName;
		std::string actionId;
		std::vector<std::uint32_t> sourceActionIds;
		std::string selectionMode;
		/* ANY, ARMORED or STRIPPED. The Server offers a weighted pattern only in
		the matching armour state, so the tool has to round-trip it or a save
		would silently drop the gate. */
		std::string armorRequirement;
		/* ANY, PHASE_ONE or PHASE_TWO, round-tripped for the same reason. */
		std::string phaseRequirement;
		/* The boss cannot be damaged while this pattern runs. */
		bool invulnerableWhileRunning = false;
		std::uint32_t minimumHealthBar = 0;
		std::uint32_t maximumHealthBar = 0;
		std::uint32_t triggerHealthBar = 0;
		std::uint32_t triggerOrder = 0;
		std::uint32_t selectionWeight = 0;
		std::uint32_t maximumConsecutiveUses = 0;
		double minimumRange = 0.0;
		double maximumRange = 0.0;
		SERVER_MOTION_EDIT serverMotion;
		std::vector<PATTERN_STAGE_EDIT> stages;
	};

	struct SERIALIZED_DRAFT_DOCUMENTS
	{
		std::string players;
		std::string skills;
		std::string damage;
		std::string bosses;
		std::string encounter;
	};

	struct ENCOUNTER_STATE_EDIT
	{
		std::string id;
		std::string actionId;
		std::string next;
		bool hasNext = false;
	};

	bool Reload();
	bool Save(SERIALIZED_DRAFT_DOCUMENTS* readOnlyCapture = nullptr);
	bool ValidateDraft(std::string& status) const;
	static void NormalizePatternStageForShape(PATTERN_STAGE_EDIT& stage);
	static void NormalizePatternStagePush(PATTERN_STAGE_EDIT& stage);
	bool RunPipeline(const wchar_t* scriptName, const wchar_t* arguments,
		std::string& status) const;
	void RenderPlayerEditor();
	void RenderBossEditor();
	void RenderLiveVerification();
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
	std::string m_encounterIntroPatternId;
	std::uint32_t m_fixedTickHz = 30;
	std::unordered_map<std::string, std::string> m_basisByField;
	std::size_t m_selectedPlayer = 0;
	std::size_t m_selectedBoss = 0;
	bool m_showPlayers = true;
	bool m_dirty = false;
	bool m_open = true;
	std::string m_status;
	std::shared_ptr<IPlayerCommandSink> m_commandSink;
	std::uint32_t m_reviveSequence = 0u;
};

NS_END
