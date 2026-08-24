#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Where a stage's editable Effect document came from. Product cue identity is
   authoritative for authoring; the evidence binding is recorded on that same
   row when it agrees. The naming rule is only a fallback for a cue-less stage. */
enum class VALTAN_STAGE_EFFECT_ORIGIN : uint8_t
{
	PRODUCT_CUE,
	PATTERN_EFFECT_BINDING,
	NAMING_RULE,
	END
};

struct VALTAN_STAGE_EFFECT_VIEW final
{
	std::string strEffectAssetId;
	std::filesystem::path DocumentPath;
	VALTAN_STAGE_EFFECT_ORIGIN eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::END;
	/* Filled only after the user opens or plays the document. The tree must
	   never decode JSON just to draw a row. */
	uint32_t iElementCount = 0u;
	bool_t bParsed = false;
	bool_t bPatternEffectBinding = false;
};

struct VALTAN_PRODUCT_EFFECT_CUE_VIEW final
{
	std::string strBindingId;
	std::string strOccurrenceId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strClipOccurrenceId;
	std::string strEffectAssetId;
	std::string strV1EffectAssetId;
	std::string strAnchorSlotId;
	EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	std::string strFollowPolicy;
	std::string strStopPolicy;
	std::string strRepeatPolicy;
	uint32_t iSourceStartMs = 0u;
	uint32_t iSourceEndMs = 0u;
	uint32_t iStageDurationMs = 0u;
	bool_t bHasSourceEnd = false;
};

/* A Server-owned combat object is not a boss-root Product cue.  It still
   belongs under the semantic stage that spawns it so All Effects can expose
   the same editable Unified Effect families without pretending the boss
   animation owns the moving world root. */
struct VALTAN_COMBAT_OBJECT_EFFECT_VIEW final
{
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	std::string strEffectAssetId;
	std::string strTrigger;
	uint32_t iSpawnValue = 0u;
};

/* Stable ordered animation occurrence authored by the animation owner.  The
   ordinal is derived after parsing and is display-only; joins always use the
   clipOccurrenceId. */
struct VALTAN_CLIP_OCCURRENCE_VIEW final
{
	std::string strClipOccurrenceId;
	std::string strClipName;
	std::string strMappingBasis;
	uint32_t iSourceStartMs = 0u;
	uint32_t iPlayMs = 0u;
	/* Master-authoring wall budget for this occurrence. Product bindings keep
	   source-local cuts; this derived value lets the Tool stop a loop exactly
	   at the Server stage boundary and makes the full timeline seekable. */
	uint32_t iAuthoringWallMs = 0u;
	f32_t fPlayRate = 1.f;
	bool_t bLoop = false;
	std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;
};

struct VALTAN_STAGE_MOTION_VIEW final
{
	std::string strKind;
	f32_t fDistance = 0.f;
};

struct VALTAN_STAGE_ACTION_VIEW final
{
	std::string strTrigger;
	std::string strKind;
	std::string strTargetId;
	f32_t fValue = 0.f;
	uint32_t iDurationMs = 0u;
};

/* Branch order is part of the Server projection. A missing next action is an
   authored terminal edge, not an empty action identity. */
struct VALTAN_STAGE_BRANCH_VIEW final
{
	std::string strOutcome;
	std::optional<std::string> strNextActionId;
};

/* The stage is where a Valtan pattern actually becomes work: it owns the
   clip that plays, the milliseconds it lasts, the shape the Server tests for
   damage, and the Effect that should be visible while it runs. Those four
   facts live in four different documents joined by the stage actionId. */
struct VALTAN_STAGE_VIEW final
{
	std::string strStageId;
	std::string strSequenceRole;
	std::string strActionId;
	std::string strStageKind;
	uint32_t iDurationMs = 0u;
	uint32_t iAuthoringRepeatCount = 0u;
	std::string strAnimationEndPolicy;

	/* Server owns damage. These are copied for display so the person
	   authoring the Effect can see the window they are filling. */
	std::string strHitShape;
	f32_t fHitOuterRadius = 0.f;
	f32_t fHitInnerRadius = 0.f;
	f32_t fHitAngleDegrees = 0.f;
	f32_t fHitLength = 0.f;
	f32_t fHitHalfWidth = 0.f;
	uint32_t iHitCount = 0u;
	uint32_t iHitIntervalMs = 0u;
	uint32_t iHitDelayMs = 0u;
	/* Ordered stage-relative contacts. Empty means the authored stage uses
	   iHitDelayMs + k * iHitIntervalMs. */
	std::vector<uint32_t> HitOffsetsMs;
	std::string strServerDamageProfileId;
	f32_t fPushRangeM = 0.f;
	uint32_t iPushMs = 0u;
	bool_t bKnockdown = false;
	uint32_t iDownMs = 0u;
	std::optional<VALTAN_STAGE_MOTION_VIEW> Motion;
	std::vector<VALTAN_STAGE_ACTION_VIEW> Actions;
	std::vector<VALTAN_STAGE_BRANCH_VIEW> Branches;

	/* Product authoring uses stable ordered occurrences.  The legacy name
	   views remain populated for compile-compatible callers while format v1
	   is still accepted read-only. */
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;
	std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCues;
	std::vector<VALTAN_COMBAT_OBJECT_EFFECT_VIEW> CombatObjectEffects;
	std::vector<std::string> RuntimeClipNames;
	std::string strRuntimeClipName;
	std::optional<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ProductCue;
	std::vector<VALTAN_STAGE_EFFECT_VIEW> Effects;
	/* Stable references into VALTAN_PATTERN_TREE_VIEW::IndependentEffects.
	   They are intentionally not editable Effect rows inside the pattern. */
	std::vector<std::string> IndependentEffectIds;

	bool_t Has_Effect() const
	{
		return !Effects.empty() || !CombatObjectEffects.empty();
	}
	bool_t Has_ClipBinding() const { return !ClipOccurrences.empty(); }
	bool_t Has_ProductCue() const { return !ProductCues.empty(); }
	bool_t Has_HitShape() const
	{
		return !strHitShape.empty() && "NONE" != strHitShape;
	}
};

struct VALTAN_PATTERN_SERVER_MOTION_VIEW final
{
	std::string strKind;
	std::string strAnchorId;
	std::array<f32_t, 3u> LandingPosition{};
	f32_t fApexHeight = 0.f;
	std::string strTravelStageId;
};

struct VALTAN_PRESENTATION_SOURCE_VIEW final
{
	uint32_t iSourceActionId = 0u;
	uint32_t iSequenceIndex = 0u;
	std::string strRole;
};

struct VALTAN_PATTERN_REACTION_VIEW final
{
	std::string strTriggerKind;
	std::string strStageId;
};

struct VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW final
{
	std::string strPatternId;
	std::string strStageId;
	std::string strTriggerKind;
};

struct VALTAN_NORMAL_SELECTION_RANGE_VIEW final
{
	std::string strRotationId;
	uint32_t iFromHealthBar = 0u;
	uint32_t iToHealthBar = 0u;
};

struct VALTAN_NORMAL_SELECTION_VIEW final
{
	std::string strSelectionMode;
	std::vector<VALTAN_NORMAL_SELECTION_RANGE_VIEW> Ranges;
	std::vector<std::string> PatternIds;
};

/* Reference-only reaction actions join the master identity to the existing
   Encounter stage and Product animation binding. They do not admit the legacy
   owner pattern into the seven-pattern Phase-1 selection pool. */
struct VALTAN_REACTION_ANIMATION_ACTION_VIEW final
{
	std::string strActionId;
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ClipOccurrences;
};

struct VALTAN_COUNTER_REACTION_LAYER_VIEW final
{
	std::string strReactionLayerId;
	std::string strAdmissionScope;
	std::string strOwnerPatternId;
	std::string strOwnerStageId;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Window;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Success;
	VALTAN_REACTION_ANIMATION_ACTION_VIEW Failure;
};

struct VALTAN_PATTERN_VIEW final
{
	std::string strPatternId;
	std::string strCategory;
	uint32_t iMinimumPhase = 0u;
	uint32_t iMaximumPhase = 0u;
	std::string strTargetPolicy;
	std::string strAimPolicy;
	std::string strDisplayName;
	std::string strActionId;
	std::vector<uint32_t> SourceActionIds;
	std::string strSelectionMode;
	int32_t iMinimumHealthBar = 0;
	int32_t iMaximumHealthBar = 0;
	int32_t iTriggerHealthBar = 0;
	uint32_t iTriggerOrder = 0u;
	std::string strArmorRequirement;
	std::string strPhaseRequirement;
	bool_t bInvulnerableWhileRunning = false;
	uint32_t iSelectionWeight = 0u;
	uint32_t iMaximumConsecutiveUses = 0u;
	f32_t fMinimumRange = 0.f;
	f32_t fMaximumRange = 0.f;
	std::optional<VALTAN_PATTERN_SERVER_MOTION_VIEW> ServerMotion;
	uint32_t iSourceSequenceIndex = 0u;
	std::vector<VALTAN_PRESENTATION_SOURCE_VIEW> PresentationSources;
	/* Master-only presentation and reaction contracts stay typed in the
	   shared Tool view. They are not projected into ValtanEncounter.json and
	   therefore must never disappear after master admission. Order is
	   authored and preserved. */
	std::vector<VALTAN_PATTERN_REACTION_VIEW> Reactions;
	std::vector<std::string> CameraCueIds;
	std::vector<VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW> WorldEventTriggerRefs;
	bool_t bAuthoringMasterManaged = false;
	std::vector<VALTAN_STAGE_VIEW> Stages;

	/* A pattern pinned to one health bar is a scripted gimmick; the rest are
	   selected from the rotation while the bar range allows it. */
	bool_t Is_Gimmick() const { return 0 != iTriggerHealthBar; }
};

enum class VALTAN_PATTERN_PREVIEW_PATH : uint8_t
{
	NORMAL,
	WALL_GROGGY,
	PART_BREAK,
	END
};

/* One authoring identity for a reusable Effect whose trigger is owned by the
   Server pattern/combat-object lane. It appears once at the root of All
   Effects; owner pattern rows only show a read-only reference. */
struct VALTAN_INDEPENDENT_EFFECT_VIEW final
{
	std::string strIndependentEffectId;
	std::string strDisplayName;
	std::string strEffectAssetId;
	std::string strOwnership;
	std::string strOwnerPatternId;
	std::string strOwnerStageId;
	std::string strTriggerPolicy;
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	std::string strEffectCueBindingId;
	/* SERVER_PATTERN_STAGE projections are pinned to the exact Product cue
	   occurrence. SERVER_COMBAT_OBJECT identities have no boss-root cue and
	   therefore leave this tuple absent. */
	std::string strCueClipOccurrenceId;
	std::string strCueMappingBasis;
	uint32_t iCueSourceStartMs = 0u;
	uint32_t iCueSourceEndMs = 0u;
	bool_t bHasCueProjection = false;
	bool_t bHasCueSourceEnd = false;
};

/* Valtan has no phase field. What it has is 160 health bars and gimmicks
   pinned to one of them, so a phase is the band of bars that ends when its
   gimmick fires. The band is derived, never stored as an id. */
struct VALTAN_PHASE_VIEW final
{
	static constexpr size_t INVALID_INDEX =
		(std::numeric_limits<size_t>::max)();

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTopHealthBar = 0;
	int32_t iBandBottomHealthBar = 0;
	/* Empty on the final band: nothing gates the bars below the last gimmick. */
	std::string strGatePatternId;
	int32_t iGateTriggerHealthBar = 0;

	/* Indices into VALTAN_PATTERN_TREE_VIEW, never copies. The same rotation
	   pattern shows under several phases and must stay one object. */
	std::vector<size_t> GimmickIndices;
	std::vector<size_t> RotationIndices;
};

struct VALTAN_PATTERN_TREE_VIEW final
{
	std::vector<VALTAN_PATTERN_VIEW> Gimmicks;
	std::vector<VALTAN_PATTERN_VIEW> Rotation;
	std::vector<VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;
	VALTAN_NORMAL_SELECTION_VIEW NormalSelection;
	std::vector<VALTAN_COUNTER_REACTION_LAYER_VIEW> CounterReactionLayers;
	std::vector<VALTAN_PHASE_VIEW> Phases;
	/* introPatternId resolved into Rotation, or INVALID_INDEX. */
	size_t iIntroRotationIndex = VALTAN_PHASE_VIEW::INVALID_INDEX;

	size_t Get_PatternCount() const
	{
		return Gimmicks.size() + Rotation.size();
	}
	size_t Get_StageCount() const;
	size_t Get_EffectCount() const;
	size_t Get_EffectDocumentCount() const;
	size_t Get_ClipBoundStageCount() const;
	size_t Get_ProductCueStageCount() const;
	size_t Get_ClipOccurrenceCount() const;
	size_t Get_ProductCueCount() const;
	size_t Get_CombatObjectEffectCount() const;
};

/* Read-only join of ValtanEncounter.json, Valtan.patternbindings.json and
   Valtan.patterneffects.json. The Effect Tool renders the result; nothing
   here writes, because the encounter document is Server authority. */
class CValtanPatternTree final
{
public:
	static bool_t Load(
		VALTAN_PATTERN_TREE_VIEW& OutView,
		std::string& strOutStatus);
	/* effect.valtan.<pattern-slug>.<stage-slug>, the same rule
	   Tools/EffectPipeline/build_valtan_stage_effects.py emits. */
	static std::string Build_StageEffectAssetId(
		const std::string& strPatternActionId,
		const VALTAN_STAGE_VIEW& Stage);
	/* Resolves the ordered stage graph from action identities. Branch-less
	   stages alone fall through to their next ordinal stage. */
	static bool_t Build_PreviewStagePath(
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		std::vector<const VALTAN_STAGE_VIEW*>& OutStages,
		std::string& strOutStatus);
};

NS_END
