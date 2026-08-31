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
	struct ANIMATION_SLOT_EDIT final
	{
		std::string clipOccurrenceId;
		std::string clip;
		std::string mappingBasis = "PROJECT_AUTHORED";
		std::uint32_t sourceStartMs = 0u;
		std::uint32_t playMs = 0u;
		double playRate = 1.0;
		bool repeatUntilStageEnd = false;
	};

	/* Typed Valtan Stage draft shared by Balance Tool and Action Composition
	   Workbench. Stable identities/topology and DamageProfile ownership remain
	   read-only. Server timing/hit/reaction, presentation Sequence slots, and
	   MANUAL_SERVER_AUDITION Stage kind are staged through explicit adapters. */
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
		std::string motionKind;
		std::uint32_t portalRetargetDelayMs = 0;
		double portalSpeedMps = 0.0;
		double portalDistanceM = 0.0;
		/* Full joined family rows are copied into the shared draft so Workbench
		   can show every saved action/effect value. The broad Stage setter keeps
		   joined Effect rows read-only; the exact Add/Update/Remove adapters below
		   own full invocation timing, anchor, transform and scale-policy edits. */
		std::vector<VALTAN_STAGE_ACTION_VIEW> actions;
		std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> productCues;
		std::string animationEndPolicy;
		std::uint32_t animationRepeatCount = 0u;
		std::vector<ANIMATION_SLOT_EDIT> animationSlots;
		/* Create New Pattern promotes a MANUAL_SERVER_AUDITION with stable
		   Stage/Action identities.  Only that admission class may retag an
		   existing Stage for typed Counter -> Groggy composition. */
		bool stageKindEditable = false;
		bool durationEditable = false;
		bool hitEditable = false;
		bool portalRushMotionEditable = false;
		bool animationEditable = false;
	};

	/* VALTAN_WARP owns eight identical Server rush legs (STEP_02..STEP_09).
	   Expose them as one typed authoring value so a UI can never leave a
	   half-updated portal pattern behind.  Delay/speed/distance/trailing gap
	   are authored inputs; the Stage clock, travel, and hit count are derived
	   atomically for all eight legs. */
	struct VALTAN_WARP_RUSH_EDIT final
	{
		std::uint32_t legDurationMs = 0u;
		std::uint32_t retargetDelayMs = 0u;
		double speedMps = 0.0;
		double distanceM = 0.0;
		double travelMs = 0.0;
		std::uint32_t trailingGapMs = 0u;
		std::uint32_t hitCount = 0u;
	};

	/* One typed Server-gameplay fork: a WINDUP stage owns the paired
	   counterable flag plus COUNTER_HIT/TIMEOUT branches.  Success resolves to
	   a later same-pattern WINDUP, GROGGY, or RECOVERY Stage; only a GROGGY
	   success target owns the paired groggy flag transition. */
	struct VALTAN_COUNTER_WINDOW_EDIT final
	{
		bool enabled = false;
		std::string successStageId;
		std::string successActionId;
		std::string timeoutStageId;
		std::string timeoutActionId;
	};

	/* Optional Server counter-hit area owned by the same WINDUP Stage.  The
	   counter clock remains the Stage duration; this preset only controls the
	   BOSS_LOCAL source area accepted while that clock is open. */
	struct VALTAN_COUNTER_PROXY_EDIT final
	{
		bool exists = false;
		float forwardOffsetM = 1.f;
		float rightOffsetM = 0.f;
		float radiusM = 2.25f;
	};

	CBalanceTool();
	void Open();
	void Open_Valtan();
	/* Re-stage the exact split Valtan source after another typed authoring
	   transaction (for example Create New Pattern) commits it. Dirty Balance
	   drafts are never discarded implicitly. */
	bool Reload_ValtanSource(std::string& status);
	/* Verify the physical repository source/Product identity while the caller
	   holds the same canonical read admission used for its separate tree load.
	   This prevents a Workbench from pairing a Balance draft from generation A
	   with a canonical tree loaded from generation B. */
	bool Verify_ValtanCanonicalSourceRevision_WhileAdmitted(
		const CValtanCanonicalProductReadAdmission& admission,
		const std::string& expectedRepositoryRevision,
		std::string& status) const;
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
	/* Returns the effective in-memory authoring Pattern, including every typed
	   draft staged through this owner.  The Action Composition Workbench uses a
	   value copy so its Details, Sequencer, and local model preview can never
	   fall back to a stale repository-only projection while edits are pending. */
	bool Get_ValtanPatternDraft(
		const std::string& patternId,
		VALTAN_PATTERN_VIEW& pattern,
		std::string& status) const;
	bool Get_ValtanAuthoringState(
		std::string& sourceRevision,
		bool_t& dirty,
		std::string& status) const;
	bool Get_ValtanCanonicalSourceRevision(
		std::string& repositoryRevision,
		std::string& status) const;
	bool Set_ValtanStageDraft(
		const std::string& patternId,
		const std::string& stageId,
		const PATTERN_STAGE_EDIT& stage,
		std::string& status);
	/* The normalizer is shared by every portal-rush editor. It validates the
	   four authored inputs, derives ceil(delay + travel + trailing gap), and
	   regenerates the exact 50 ms travel-only swept-hit schedule. */
	static bool Normalize_ValtanPortalRushDraft(
		PATTERN_STAGE_EDIT& stage,
		std::uint32_t trailingGapMs,
		std::string& status);
	bool Get_ValtanWarpRushDraft(
		VALTAN_WARP_RUSH_EDIT& rush,
		std::string& status) const;
	bool Set_ValtanWarpRushDraft(
		const VALTAN_WARP_RUSH_EDIT& rush,
		std::string& status);
	/* Pattern Effect invocations are presentation-source rows, not generated
	   Product rows and not Effect document bodies.  These explicit adapters
	   keep the broad Stage editor's joined inventory read-only while staging
	   one exact cue identity against one animation occurrence. */
	bool Add_ValtanStageEffectCue(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& actionId,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue,
		std::string& status);
	bool Update_ValtanStageEffectCue(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& actionId,
		const std::string& cueId,
		const std::string& occurrenceId,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue,
		std::string& status);
	bool Remove_ValtanStageEffectCue(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& actionId,
		const std::string& cueId,
		const std::string& occurrenceId,
		const std::string& effectAssetId,
		const std::string& clipOccurrenceId,
		std::string& status);
	/* Manual Server auditions alone may change their ordered Stage topology.
	   WAIT is a presentation/authoring role stored as an ACTIVE Server Stage
	   with animation NONE; it does not introduce a second runtime stage kind.
	   Every operation preserves existing stable Stage/Action identities and is
	   serialized through the same canonical source transaction as field edits. */
	bool Insert_ValtanManualStageAfter(
		const std::string& patternId,
		const std::string& afterStageId,
		const std::string& stageId,
		const std::string& actionId,
		const std::string& stageRole,
		std::uint32_t durationMs,
		std::string& status);
	bool Can_Edit_ValtanManualStageTopology(
		const std::string& patternId,
		std::string& status) const;
	bool Remove_ValtanManualStage(
		const std::string& patternId,
		const std::string& stageId,
		std::string& status);
	bool Move_ValtanManualStage(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& anchorStageId,
		bool beforeAnchor,
		std::string& status);
	std::vector<std::string> Get_ValtanDamageProfileIds() const;
	bool Get_ValtanCounterWindowDraft(
		const std::string& patternId,
		const std::string& stageId,
		VALTAN_COUNTER_WINDOW_EDIT& counter,
		std::string& status) const;
	bool Set_ValtanCounterWindowDraft(
		const std::string& patternId,
		const std::string& stageId,
		const VALTAN_COUNTER_WINDOW_EDIT& counter,
		std::string& status);
	bool Get_ValtanCounterProxyDraft(
		const std::string& patternId,
		const std::string& stageId,
		VALTAN_COUNTER_PROXY_EDIT& proxy,
		std::string& status) const;
	bool Set_ValtanCounterProxyDraft(
		const std::string& patternId,
		const std::string& stageId,
		const VALTAN_COUNTER_PROXY_EDIT& proxy,
		std::string& status);
	bool Get_ValtanHighJumpAxeCountDraft(
		std::uint32_t& draftCount,
		std::uint32_t& savedCount,
		std::uint32_t& arenaRandomCount,
		std::uint32_t& maximumTotalObjects,
		std::string& status) const;
	bool Set_ValtanHighJumpAxeCountDraft(
		std::uint32_t countPerAlivePlayer,
		std::string& status);
	/* One user-facing Save contract: validate the joined draft, atomically
	   commit canonical split JSON/Product data when dirty, build the immutable
	   runtime bundle, and
	   request the existing two-phase live apply when it is currently safe.
	   Internal stages remain explicit for rollback and diagnostics. */
	bool Save_ValtanProduct(std::string& status);
	struct VALTAN_COMPOSITION_OWNER_DRAFTS final
	{
		std::string patternSoundBaselineBytes;
		std::string patternSoundCandidateBytes;
		std::string effectV2BaselineBytes;
		std::string effectV2CandidateBytes;
	};
	/* Commits the Pattern split source/Product closure and optional independent
	   Sound/V2 owners through the existing shared Valtan writer generation.
	   Every candidate is staged and validated before one rollback-safe commit. */
	bool Save_ValtanCompositionProduct(
		const VALTAN_COMPOSITION_OWNER_DRAFTS& ownerDrafts,
		std::string& status);
	/* Action Composition owns repository authoring, not a live gameplay-only
	   candidate.  This commits the typed joined draft to the split gameplay and
	   presentation sources plus every generated Product in one shared-writer
	   transaction.  Runtime activation remains explicitly restart/version gated. */
	bool Save_ValtanCanonicalProduct(std::string& status);
	bool Validate_ValtanDraft(std::string& status);
	bool Save_ValtanAuthoring(std::string& status);
	bool Publish_ValtanCandidate(std::string& status);
	bool Apply_ValtanRevision(std::string& status);
	bool Is_ValtanDraftDirty() const { return m_dirty; }
	std::uint64_t Get_ValtanDraftGeneration() const
	{
		return m_valtanDraftGeneration;
	}
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
		std::string repositoryRevision;
		std::string joinedRevision;
		std::string diagnostic;
		VALTAN_SOURCE_JOIN_STATE state =
			VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_INCOMPLETE;
	};

	bool Reload();
	bool Require_ValtanAuthoringAdmission(
		const char* operation, std::string& status) const;
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
	bool RunValtanDraftCommand(
		const wchar_t* mode,
		std::string& status,
		const VALTAN_COMPOSITION_OWNER_DRAFTS* pOwnerDrafts = nullptr);
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
		const CValtanCanonicalProductReadAdmission& canonicalAdmission,
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
	std::uint64_t m_valtanDraftGeneration = 1u;
	bool m_reloadConfirmationOpen = false;
	bool m_open = true;
	bool m_focusPending = false;
	std::string m_status;
	std::uint32_t m_valtanDecisionTraceRequestSequence = 0u;
	std::uint32_t m_valtanDecisionLastQueryServerTick = 0u;
};

NS_END
