#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternTree.h"
#include "ValtanViewAdmission.h"

#include <cstdint>
#include <filesystem>
#include <memory>
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
		bool hasHitAnchor = false;
		std::string hitAnchorKind = "BOSS_CURRENT";
		double hitAnchorForwardOffsetM = 0.0;
		double hitAnchorRightOffsetM = 0.0;
		double hitAnchorYawOffsetDegrees = 0.0;
		bool hasHitActivation = false;
		std::uint32_t hitActivationStartMs = 0u;
		std::uint32_t hitActivationLifetimeMs = 0u;
		std::string damageProfileId;
		double pushRangeM = 0.0;
		std::uint32_t pushMs = 0;
		bool knockdown = false;
		std::uint32_t downMs = 0;
		std::string playerResponse = "DAMAGE";
		std::string attachmentSlot = "NONE";
		bool hasGripLocalOffset = false;
		double gripForwardM = 0.0;
		double gripUpM = 0.0;
		double gripRightM = 0.0;
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
		bool colliderAddAdmitted = false;
		bool colliderTuneAdmitted = false;
		bool colliderRemoveAdmitted = false;
		/* Animation Tool still consumes the former tune-only view.  New Collider
		   authoring must use the three operation-specific admissions above. */
		bool hitEditable = false;
		bool portalRushMotionEditable = false;
		bool animationEditable = false;
	};

	/* One Server-owned combat-object RING hit.  The stable Pattern/Stage owner,
	   combat-object archetype, and hit identity are immutable; only the two
	   canonical radii may be staged through this narrow adapter. */
	struct VALTAN_COMBAT_OBJECT_RING_HIT_EDIT final
	{
		std::string combatObjectArchetypeId;
		std::string hitId;
		double innerRadiusM = 0.0;
		double outerRadiusM = 0.0;
	};

	/* VALTAN_WARP owns one first rush leg and seven cadence legs.  The first
	   waits retargetDelayMs after its portal appears.  Later legs add
	   trailingGapMs before that same lead, allowing the preceding NATURAL
	   portal to linger and the replacement portal to start at the boundary.
	   legDurationMs is the repeated cadence clock; every Stage clock and swept
	   hit schedule is derived atomically for all eight legs. */
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

	/* One typed Server-gameplay fork: a WINDUP Stage, or an authored ACTIVE
	   BOSS_FORWARD_ARC Stage, owns paired COUNTER_HIT/TIMEOUT branches. Success
	   may resolve to a later local Stage or a typed cross-Pattern entry. The
	   generic Counter editor mutates local targets only; cross-Pattern targets
	   are retained as read-only stable identities during unrelated saves. */
	struct VALTAN_COUNTER_WINDOW_EDIT final
	{
		bool enabled = false;
		std::string successPatternId;
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
	~CBalanceTool();
	void Open();
	void Open_Valtan();
	/* Re-stage the exact split Valtan source after another typed authoring
	   transaction (for example Create New Pattern) commits it. Dirty Balance
	   drafts are never discarded implicitly. */
	bool Reload_ValtanSource(std::string& status);
	/* Explicit Workbench navigation boundary.  This is the only reload entry
	   that may replace an unsaved Balance-owned Valtan composition draft, and
	   its underlying Reload stages every physical owner before commit. */
	bool Discard_ValtanCompositionDraftAndReload(std::string& status);
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
	bool Get_ValtanCombatObjectRingHitDraft(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& combatObjectArchetypeId,
		const std::string& hitId,
		VALTAN_COMBAT_OBJECT_RING_HIT_EDIT& hit,
		std::string& status) const;
	bool Set_ValtanCombatObjectRingHitDraft(
		const std::string& patternId,
		const std::string& stageId,
		const VALTAN_COMBAT_OBJECT_RING_HIT_EDIT& hit,
		std::string& status);
	/* Returns the effective in-memory authoring Pattern, including every typed
	   draft staged through this owner.  The Valtan Action Workbench uses a
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
	/* Pattern Flow is an editor over the same gameplay authoring draft.  It
	   stages only the inline scriptedSequence fields; the ordinary canonical
	   Save then commits Pattern definitions, order and generated Products as one
	   source revision. */
	bool Get_ValtanScriptedSequenceDraft(
		std::vector<std::string>& patternIds,
		std::uint32_t& interStepPursuitMs,
		std::vector<std::uint32_t>& transitionPursuitMs,
		std::string& status) const;
	bool Set_ValtanScriptedSequenceDraft(
		const std::vector<std::string>& patternIds,
		std::uint32_t interStepPursuitMs,
		const std::vector<std::uint32_t>& transitionPursuitMs,
		std::string& status);
	bool Get_ValtanCanonicalSourceRevision(
		std::string& repositoryRevision,
		std::string& status) const;
	bool Set_ValtanStageDraft(
		const std::string& patternId,
		const std::string& stageId,
		const PATTERN_STAGE_EDIT& stage,
		std::string& status);
	/* Moving one stable Animation occurrence across Stage boundaries changes two
	   Stage-owned clocks.  Apply both validated drafts as one in-memory command;
	   if either side fails, the Pattern and dirty/revision state are restored. */
	bool Set_ValtanAnimationTransferDrafts(
		const std::string& patternId,
		const std::string& sourceStageId,
		const PATTERN_STAGE_EDIT& sourceStage,
		const std::string& targetStageId,
		const PATTERN_STAGE_EDIT& targetStage,
		std::string& status);
	/* Cross-source Sequence assignment is one authoring transaction: the exact
	   Stage draft and its gameplay/presentation provenance either both enter the
	   in-memory draft or neither does.  The first source remains PRIMARY; this
	   API appends only the selected validated secondary source to the managed
	   Pattern; topology and gameplay authority remain unchanged. */
	bool Set_ValtanStageSequenceDraft(
		const std::string& patternId,
		const std::string& stageId,
		std::uint32_t sourceActionId,
		std::uint32_t sourceSequenceIndex,
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
	/* Promotion keeps the existing Stage/Action IDs and clock, but changes one
	   dependency-free manual WAIT into a real authoring role. Animation remains
	   NONE until Workbench assigns an exact Sequence in the same or later draft. */
	bool Promote_ValtanManualWaitStage(
		const std::string& patternId,
		const std::string& stageId,
		const std::string& stageRole,
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
	bool Get_ValtanDamageRateDraft(
		const std::string& damageProfileId,
		std::uint32_t& ratePercent,
		std::string& status) const;
	bool Get_ValtanDamageProfileStageUserCountDraft(
		const std::string& damageProfileId,
		std::size_t& stageUserCount,
		std::string& status) const;
	bool Set_ValtanDamageRateDraft(
		const std::string& damageProfileId,
		std::uint32_t ratePercent,
		std::string& status);
	bool Get_ValtanBossAttackPowerDraft(
		std::uint32_t& attackPower,
		std::string& status) const;
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
	/* Ordinary branch retarget: an existing outcome of one Stage points at a
	   forward same-pattern Stage action or at the pattern end. Save emits
	   SET_STAGE_BRANCH. COUNTER_HIT/TIMEOUT keep their typed Counter/topology
	   owners and cross-pattern follow-ups stay read-only. */
	struct VALTAN_STAGE_BRANCH_EDIT final
	{
		std::string outcome;
		std::string nextActionId;
		bool crossPattern = false;
	};
	bool Get_ValtanStageBranchDrafts(
		const std::string& patternId,
		const std::string& stageId,
		std::vector<VALTAN_STAGE_BRANCH_EDIT>& branches,
		std::vector<std::string>& targetActionIds,
		std::string& status) const;
	bool Set_ValtanStageBranchDraft(
		const std::string& patternId,
		const std::string& stageId,
		const VALTAN_STAGE_BRANCH_EDIT& branch,
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
	/* Publish/apply-only continuation for a clean, already committed Valtan
	   source: validate, publish the candidate, record the activation
	   expectation and live-apply when the change class allows. Never writes
	   source. Composition Save calls this after its own single commit edge. */
	bool Apply_CommittedValtanProduct(std::string& status);

	/* Canonical data publish job (Run-FullPipeline.ps1 -DataOnly): every Save
	   uses the same receipt-based domain executor as the manual full pipeline,
	   pinned to the exact committed Valtan source revision. It runs as a
	   detached child process polled once per frame so a Save never stalls the
	   Client tick, and it is never terminated by a timeout because each domain
	   transaction must finish or roll back on its own. One job at a time. */
	enum class SERVER_RUNTIME_PUBLISH_STATE : std::uint8_t
	{
		IDLE,
		RUNNING,
		SUCCEEDED,
		FAILED,
		/* WaitForSingleObject failed, so the child may still own the
		   transaction. Keep authoring locked and never start a second job. */
		OBSERVATION_LOST,
	};
	/* Publish is always bound to one immutable canonical source receipt. The
	   child verifies this revision while holding the canonical writer admission
	   both before staging and immediately before commit completion. */
	bool Publish_ServerRuntimeSet(
		const std::string& expectedValtanSourceRevision,
		std::string& status);
	void Update_ServerRuntimeSetPublishJob();
	SERVER_RUNTIME_PUBLISH_STATE Get_ServerRuntimeSetPublishState(
		std::string& status,
		double& elapsedSeconds,
		std::string* expectedValtanSourceRevision = nullptr) const;
	bool Is_ServerRuntimeSetPublishRunning() const
	{
		return SERVER_RUNTIME_PUBLISH_STATE::RUNNING == m_publishJobState ||
			SERVER_RUNTIME_PUBLISH_STATE::OBSERVATION_LOST == m_publishJobState;
	}
	/* Returns the exact durable receipt eligible for Product/runtime publish.
	   After COMMIT_SUCCEEDED_REOPEN_FAILED this is the pending commit receipt,
	   not the last successfully displayed editor revision. */
	bool Get_ValtanPublishSourceRevision(
		std::string& sourceRevision,
		std::string& status) const;
	struct VALTAN_COMPOSITION_OWNER_DRAFTS final
	{
		std::string patternSoundBaselineBytes;
		std::string patternSoundCandidateBytes;
		std::string effectV2BaselineBytes;
		std::string effectV2CandidateBytes;
		std::string effectV2ReadSetBytes;
	};
	/* Save is a durable asynchronous transaction. The immutable request is
	   staged before the child starts; callers observe one job id and consume its
	   terminal receipt exactly once on the main thread. */
	enum class VALTAN_SAVE_JOB_STATE : std::uint8_t
	{
		IDLE,
		RUNNING,
		SUCCEEDED,
		FAILED,
		/* The child may still own the canonical writer. This state deliberately
		   remains authoring-blocking and cannot be consumed in-process. */
		OBSERVATION_LOST,
	};
	struct VALTAN_SAVE_JOB_RECEIPT final
	{
		std::uint64_t jobId = 0u;
		VALTAN_SAVE_JOB_STATE state = VALTAN_SAVE_JOB_STATE::IDLE;
		bool canonicalCommitted = false;
		bool runtimePublishRequested = false;
		std::string expectedSourceRevision;
		std::string committedSourceRevision;
		std::string candidateRevision;
		std::string applyClass;
		std::string status;
		std::vector<std::string> durableLogs;
	};
	/* Balance/Boss Save commits the dirty canonical draft, or publishes the
	   already-clean exact source when no write is needed. Full DataOnly starts
	   only after the caller consumes this receipt and reopens its local owners. */
	bool Begin_ValtanProductSave(
		std::uint64_t& jobId, std::string& status);
	/* Composition uses the same transaction and may add immutable Sound/Effect
	   owner pairs to the single canonical commit. */
	bool Begin_ValtanCompositionSave(
		const VALTAN_COMPOSITION_OWNER_DRAFTS& ownerDrafts,
		bool publishAfterSave,
		std::uint64_t& jobId, std::string& status);
	/* Post-commit retry never repeats a source write. */
	bool Begin_ValtanProductPublishRetry(
		bool publishAfterSave,
		std::uint64_t& jobId, std::string& status);
	void Update_ValtanSaveJob();
	VALTAN_SAVE_JOB_STATE Get_ValtanSaveJobState(
		VALTAN_SAVE_JOB_RECEIPT& receipt) const;
	/* SUCCEEDED consumer order, within one main-thread callback, is exact-
	   revision check -> local owner byte accept -> Consume (returns IDLE) ->
	   Workbench/Boss graph reopen -> optional Publish_ServerRuntimeSet with the
	   same committedSourceRevision. A FAILED committed receipt must use
	   Begin_ValtanProductPublishRetry instead. */
	bool Consume_ValtanSaveJobReceipt(
		std::uint64_t jobId, VALTAN_SAVE_JOB_RECEIPT& receipt,
		std::string& status);
	bool Is_ValtanSaveJobBlockingAuthoring() const
	{
		return VALTAN_SAVE_JOB_STATE::IDLE != m_valtanSaveJobState;
	}
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
	/* Resume only the post-commit Product path. A clean saved source is
	   validated and published without another canonical source write. If the
	   preceding canonical commit succeeded but its editor reopen failed, the
	   exact committed revision may be reopened first while its unchanged draft
	   generation is still pinned. New user edits are never discarded. */
	bool Retry_ValtanProductPublishApply(std::string& status);
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
		std::uint32_t countPerResolvedTarget = 0u;
		std::string layoutKind;
		double radiusM = 0.0;
		double startAngleDegrees = 0.0;
		double angleStepDegrees = 0.0;
		bool allowOverlap = false;
		std::uint32_t maximumTotalObjects = 0u;
		std::string spawnScheduleKind;
		std::uint32_t spawnCount = 0u;
		std::uint32_t spawnFirstOffsetMs = 0u;
		std::uint32_t spawnIntervalMs = 0u;
		std::string arenaRandomKind;
		std::string arenaAnchor;
		std::uint32_t arenaRandomCount = 0u;
		double arenaRandomRadiusM = 0.0;
		double arenaHeightToleranceM = 0.0;
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
	bool LaunchPipelineProcess(const wchar_t* scriptName,
		const wchar_t* arguments, void*& outProcessHandle,
		std::filesystem::path& outOutputPath, std::string& status) const;
	void* m_publishJobProcess = nullptr;
	std::filesystem::path m_publishJobOutputPath;
	std::string m_publishJobExpectedRevision;
	std::uint64_t m_publishJobStartTick = 0u;
	bool m_publishJobWarned = false;
	SERVER_RUNTIME_PUBLISH_STATE m_publishJobState =
		SERVER_RUNTIME_PUBLISH_STATE::IDLE;
	std::string m_publishJobStatus;
	enum class VALTAN_SAVE_JOB_PHASE : std::uint8_t
	{
		NONE,
		SAVE_COMMAND_RUNNING,
		WAITING_RELOAD_MANIFEST_ADMISSION,
		RELOAD_MANIFEST_RUNNING,
	};
	bool Begin_ValtanSaveJob(bool commitCanonical, bool publishAfterSave,
		const VALTAN_COMPOSITION_OWNER_DRAFTS* ownerDrafts,
		std::uint64_t& jobId, std::string& status);
	bool Launch_ValtanSaveCommand(bool commitCanonical,
		const VALTAN_COMPOSITION_OWNER_DRAFTS* ownerDrafts,
		std::string& status);
	bool TryLaunch_ValtanSaveReloadManifest(std::string& status);
	void Fail_ValtanSaveJob(std::string status, bool observationLost = false);
	void Reset_ValtanSaveJob();
	bool Complete_ValtanCandidateActivation(std::string& status);
	void* m_valtanSaveJobProcess = nullptr;
	std::unique_ptr<CValtanCanonicalProductReadAdmission>
		m_valtanSaveJobReadAdmission;
	std::filesystem::path m_valtanSaveJobDirectory;
	std::filesystem::path m_valtanSaveJobResultPath;
	std::filesystem::path m_valtanSaveJobCurrentOutputPath;
	std::vector<std::filesystem::path> m_valtanSaveJobOutputPaths;
	std::uint64_t m_valtanSaveJobNextId = 1u;
	std::uint64_t m_valtanSaveJobId = 0u;
	std::uint64_t m_valtanSaveJobDraftGeneration = 0u;
	std::uint64_t m_valtanSaveJobStartTick = 0u;
	bool m_valtanSaveJobWarned = false;
	bool m_valtanSaveJobCommitCanonicalRequested = false;
	bool m_valtanSaveJobCanonicalCommitted = false;
	bool m_valtanSaveJobPublishAfterSave = true;
	std::string m_valtanSaveJobExpectedSourceRevision;
	std::string m_valtanSaveJobCommittedSourceRevision;
	std::string m_valtanSaveJobCandidateRevision;
	std::string m_valtanSaveJobApplyClass;
	std::string m_valtanSaveJobStatus;
	VALTAN_SAVE_JOB_STATE m_valtanSaveJobState =
		VALTAN_SAVE_JOB_STATE::IDLE;
	VALTAN_SAVE_JOB_PHASE m_valtanSaveJobPhase =
		VALTAN_SAVE_JOB_PHASE::NONE;
	/* Non-empty only while Update_ValtanSaveJob performs a verified, shared-lock
	   canonical reload. Normal Reload still obtains its identity from the
	   source-manifest command. */
	std::string m_valtanVerifiedReloadSourceRevision;
	VALTAN_SOURCE_JOIN_STATUS m_valtanVerifiedReloadSourceJoin;
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
	/* Durable CommitCanonicalDraft success may precede an in-process reopen
	   failure. Keep that exact receipt separate from dirty authoring state so a
	   typed retry can reopen it once, but only while no later edit advanced the
	   draft generation. */
	std::string m_valtanCommittedRevisionPendingReopen;
	std::uint64_t m_valtanCommittedReopenDraftGeneration = 0u;
	VALTAN_SOURCE_JOIN_STATUS m_valtanSourceJoin;
	/* The editable snapshot and its command authority are separate. A failed
	   reload preserves the last-good Balance/Workbench rows for inspection but
	   revokes every mutation until one complete canonical reload commits. */
	VALTAN_VIEW_ADMISSION m_eValtanViewAdmission =
		VALTAN_VIEW_ADMISSION::UNLOADED;
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
