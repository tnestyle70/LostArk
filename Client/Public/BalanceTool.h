#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternTree.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class DATA_JSON_VALUE;

class CBalanceTool final
{
public:
	/* Typed Valtan stage draft shared by Balance Tool and Action Presentation
	   Workbench.  Stable identities and DamageProfile ownership are read-only
	   at this boundary; only the Server gameplay timing, hit geometry/schedule,
	   and player reaction values are staged here. */
	struct PATTERN_STAGE_EDIT final
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
		std::vector<std::uint32_t> hitOffsetsMs;
		std::string damageProfileId;
		double pushRangeM = 0.0;
		std::uint32_t pushMs = 0;
		bool knockdown = false;
		std::uint32_t downMs = 0;
		std::string playerResponse = "DAMAGE";
		std::string attachmentSlot = "NONE";
		bool durationEditable = false;
		bool hitEditable = false;
	};

	CBalanceTool();
	void Open();
	void Render();
	/* Action Presentation Workbench consumes this narrow stable-ID boundary
	   instead of reaching into Balance Tool widgets or constructing a second
	   Valtan draft.  Both windows therefore edit and publish one in-memory
	   transaction. */
	bool Get_ValtanStageDurationDraft(
		const std::string& patternId,
		const std::string& stageId,
		std::uint32_t& durationMs,
		std::string& status) const;
	bool Set_ValtanStageDurationDraft(
		const std::string& patternId,
		const std::string& stageId,
		std::uint32_t durationMs,
		std::string& status);
	bool Get_ValtanStageDraft(
		const std::string& patternId,
		const std::string& stageId,
		PATTERN_STAGE_EDIT& stage,
		std::string& status) const;
	bool Set_ValtanStageDraft(
		const std::string& patternId,
		const std::string& stageId,
		const PATTERN_STAGE_EDIT& stage,
		std::string& status);
	/* One user-facing Save contract: validate the joined draft, durably save
	   authoring when dirty, build the immutable Product runtime bundle, and
	   request the existing two-phase live apply when it is currently safe.
	   Internal stages remain explicit for rollback and diagnostics. */
	bool Save_ValtanProduct(std::string& status);
	bool Validate_ValtanDraft(std::string& status);
	bool Save_ValtanAuthoring(std::string& status);
	bool Publish_ValtanCandidate(std::string& status);
	bool Apply_ValtanRevision(std::string& status);
	bool Is_ValtanDraftDirty() const { return m_dirty; }
	const std::string& Get_ValtanCandidateApplyClass() const
	{
		return m_valtanCandidateApplyClass;
	}
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
		std::string travelStageId;
		std::uint32_t takeoffStartMs = 0;
		std::uint32_t takeoffEndMs = 0;
		std::uint32_t travelStartMs = 0;
		std::uint32_t travelEndMs = 0;
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
		std::string phasePolicyKind;
		std::uint32_t phasePolicyThresholdPercent = 0;
	};

	struct VALTAN_AXE_VOLLEY_EDIT final
	{
		std::string patternId = "VALTAN_HIGH_JUMP";
		std::string stageId = "AIRBORNE";
		std::string eventId =
			"event.valtan.high-jump.airborne.spawn-target-axe";
		std::uint32_t countPerResolvedTarget = 1u;
		std::string layoutKind = "TARGET_CENTER";
		double radiusM = 0.0;
		double startAngleDegrees = 0.0;
		double angleStepDegrees = 0.0;
		bool allowOverlap = false;
		std::uint32_t maximumTotalObjects = 36u;
		std::string spawnScheduleKind = "INTERVAL";
		std::uint32_t spawnCount = 3u;
		std::uint32_t spawnFirstOffsetMs = 0u;
		std::uint32_t spawnIntervalMs = 1333u;
		std::string arenaRandomKind = "RANDOM_NAVIGABLE_CIRCLE";
		std::string arenaAnchor = "BOSS_SPAWN_POSITION";
		std::uint32_t arenaRandomCount = 4u;
		double arenaRandomRadiusM = 14.0;
		double arenaHeightToleranceM = 1.0;
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

	struct LEGACY_PATTERN_SUMMARY final
	{
		std::string patternId;
		std::string displayName;
		std::string actionId;
		std::string selectionMode;
		std::string phaseRequirement;
		std::uint32_t selectionWeight = 0u;
		std::uint32_t stageCount = 0u;
	};

	enum class VALTAN_SOURCE_JOIN_STATE : std::uint8_t
	{
		SPLIT_SOURCE_INCOMPLETE,
		SPLIT_SOURCE_UNVERIFIED,
		JOINED_VALIDATED,
		END
	};

	/* The two authoring lanes have different owners even while the transition
	   pipeline still projects them through one internal v2 document.  Keep the
	   physical source identities and the admitted joined closure separate so
	   the UI can never report a hash-only source manifest as a successful join. */
	struct VALTAN_SOURCE_JOIN_STATUS final
	{
		std::string gameplaySourcePath =
			"Data/Valtan/Valtan.gameplay.json";
		std::string presentationSourcePath =
			"Data/Valtan/Valtan.presentation.json";
		std::string gameplayRevision;
		std::string presentationRevision;
		std::string joinedRevision;
		std::string diagnostic;
		VALTAN_SOURCE_JOIN_STATE state =
			VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_INCOMPLETE;
	};

	bool Reload();
	bool Save(SERIALIZED_DRAFT_DOCUMENTS* readOnlyCapture = nullptr);
	bool ValidateDraft(std::string& status) const;
	static void NormalizePatternStageForShape(PATTERN_STAGE_EDIT& stage);
	static void NormalizePatternStagePush(PATTERN_STAGE_EDIT& stage);
	bool RunPipeline(const wchar_t* scriptName, const wchar_t* arguments,
		std::string& status, std::string* capturedOutput = nullptr) const;
	bool QueryValtanSourceRevision(
		std::string& sourceRevision,
		std::string& authoringRevision,
		VALTAN_SOURCE_JOIN_STATUS& sourceJoin,
		std::string& status) const;
	bool BuildValtanDraftPatch(std::string& output, std::string& status) const;
	bool RunValtanDraftCommand(const wchar_t* mode, std::string& status);
	bool RequestValtanHotReload(std::string& status);
	bool RequestValtanDecisionTrace(
		std::uint32_t serverTick,
		bool force,
		std::string* status = nullptr);
	void RenderPlayerEditor();
	void RenderBossEditor();
	void RenderValtanPatternAuthoring();
	void RenderValtanSourceJoinStatus() const;
	void RenderValtanManagedPattern(
		VALTAN_PATTERN_VIEW& pattern,
		const char* groupLabel,
		std::size_t stableIndex);
	bool ReloadValtanPatternAuthoring(
		const DATA_JSON_VALUE& encounterRoot,
		VALTAN_PATTERN_TREE_VIEW& patternTree,
		std::vector<LEGACY_PATTERN_SUMMARY>& legacyPatterns,
		std::string& patternStatus,
		std::string& status);
	bool RestoreValtanSavedAuthoring(
		std::vector<DAMAGE_EDIT>& damageProfiles,
		std::vector<BOSS_EDIT>& bosses,
		VALTAN_PATTERN_TREE_VIEW& patternTree,
		VALTAN_AXE_VOLLEY_EDIT& axeVolley,
		const std::string& sourceRevision,
		const std::string& authoringRevision,
		std::string& status) const;
	void RenderLiveVerification();
	void RenderValtanDecisionTrace(std::uint32_t serverTick);
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
	VALTAN_PATTERN_TREE_VIEW m_valtanPatternTree;
	std::vector<DAMAGE_EDIT> m_loadedDamageProfiles;
	std::vector<BOSS_EDIT> m_loadedBosses;
	VALTAN_PATTERN_TREE_VIEW m_loadedValtanPatternTree;
	VALTAN_AXE_VOLLEY_EDIT m_valtanAxeVolley;
	VALTAN_AXE_VOLLEY_EDIT m_loadedValtanAxeVolley;
	std::vector<LEGACY_PATTERN_SUMMARY> m_legacyPatterns;
	std::string m_valtanPatternStatus;
	std::string m_valtanSourceRevision;
	std::string m_valtanAuthoringRevision;
	std::string m_valtanCandidateRevision;
	std::string m_valtanCandidateApplyClass;
	VALTAN_SOURCE_JOIN_STATUS m_valtanSourceJoin;
	bool m_valtanDraftValidated = false;
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
	bool m_reloadConfirmationOpen = false;
	bool m_open = true;
	bool m_focusPending = false;
	std::string m_status;
	std::uint32_t m_valtanDecisionTraceRequestSequence = 0u;
	std::uint32_t m_valtanDecisionLastQueryServerTick = 0u;
};

NS_END
