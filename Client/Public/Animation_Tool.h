#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "AnimationSkillBindingDocument.h"
#include "KakulAnimationActionDocument.h"
#include "KakulAnimationPatternDocument.h"
#include "AnimationEffectCueDocument.h"
#include "CharacterPreviewPanel.h"
#include "EncounterPatternReference.h"
#include "ValtanPatternPreviewDocument.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanPatternShakeCueDocument.h"
#include "ValtanCombatObjectSoundCueDocument.h"
#include "ValtanPatternTree.h"
#include "GameplayDataRevision.h"

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CCharacter;
class CValtan;
class CBalanceTool;
class CBossTool;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;
struct CAMERA_TOOL_OPEN_REQUEST;

class CAnimation_Tool final
{
public:
	/* Action Composition Workbench consumes only this preview transport.  The
	   Workbench owns Pattern/Sequencer/Details UI and canonical selection; this
	   class remains the one owner of the real CModel pose and clip playlist. */
	struct COMPOSITION_PREVIEW_STATE final
	{
		bool_t bModelReady = false;
		bool_t bPlaying = false;
		bool_t bPaused = false;
		bool_t bSourceSequencePlaying = false;
		uint32_t iPositionMs = 0u;
		uint32_t iDurationMs = 0u;
		std::string strPatternId;
		std::string strStatus;
		std::string strSourceSequenceStatus;
	};

	struct COMPOSITION_SEQUENCE_CLIP_VIEW final
	{
		std::string strClipName;
		uint32_t iDurationMs = 0u;
		bool_t bUsesNativeDuration = false;
	};

	struct COMPOSITION_SEQUENCE_VIEW final
	{
		int32_t iSkillId = 0;
		int32_t iSequenceIndex = 0;
		std::string strStableId;
		std::string strCategory;
		std::string strProfileId;
		std::string strDisplayName;
		std::string strMode;
		bool_t bValtanPatternCompatible = false;
		std::vector<COMPOSITION_SEQUENCE_CLIP_VIEW> Clips;
	};

private:
	/* One exact occurrence on the admitted split Valtan gameplay/presentation
	   wall-clock. Source time remains independent: sourceStart/playMs/playRate
	   select the pose, while authoringWallMs decides when the Server stage
	   advances. */
	struct VALTAN_PATTERN_MASTER_PLAY_ITEM final
	{
		std::string strPatternId;
		std::string strPatternDisplayName;
		std::string strStageId;
		std::string strSequenceRole;
		std::string strStageKind;
		std::string strActionId;
		std::string strClipOccurrenceId;
		std::string strClipName;
		uint32_t iSourceStartMs = 0u;
		uint32_t iPlayMs = 0u;
		uint32_t iAuthoringWallMs = 0u;
		uint32_t iTimelineStartMs = 0u;
		uint32_t iStageTimelineStartMs = 0u;
		uint32_t iOccurrenceNumber = 0u;
		uint32_t iOccurrenceCount = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bRepeatUntilStageEnd = false;
		bool_t bSuppressAnimation = false;
	};

	/* Window kinds own a start..end span; point kinds fire on one instant and
	carry a payload naming the cue or particle. Everything before SOUND is a
	window (see Is_Window). Keeping SOUND and EFFECT apart rather than folding
	them into one payload kind is deliberate: different subsystems fire them. */
	enum class EVENT_KIND
	{
		/* Window kinds first; Is_Window relies on the ordering. */
		HIT,
		CANCEL,
		SUPERARMOR,
		INVULN,
		MOVE,
		COUNTER,
		/* Point kinds. */
		SOUND,
		EFFECT,
		SHAKE,
		STAGE,
		END,
	};

	enum class EFFECT_REFERENCE_KIND
	{
		NONE,
		SOURCE_REFERENCE,
		EFFECT_ASSET_ID,
	};

	/* The Valtan Outliner persists semantic identity, never a vector position.
	   m_iValtanPatternMasterSelected remains a derived compatibility cursor for
	   the existing preview transport only. */
	enum class VALTAN_WORKBENCH_SELECTION_KIND
	{
		TARGET,
		PATTERN,
		STAGE,
	};

	enum class VALTAN_WORKBENCH_DETAIL_OWNER
	{
		GAMEPLAY,
		ANIMATION,
		EFFECT,
		SOUND,
		CAMERA,
		WORLD,
	};

	enum class VALTAN_PATTERN_MASTER_ADMISSION_STATE
	{
		UNLOADED,
		ADMITTED,
		STALE_PRESERVED,
		REJECTED,
	};

	/* Extracted combat values shared by an authored HIT and a reference row, so
	stamping is a copy rather than a field-by-field translation. Distances stay in
	raw game units; whoever consumes them owns the conversion to our world scale. */
	struct HIT_PARAMS
	{
		/* Hits after the first fire every iRepeatMs; the window is the tolerance
		of each hit, not a span the hits are spread across. */
		int32_t iRepeatCount = { 1 };
		int32_t iRepeatMs = {};

		/* Hitstop: hold for iFreezeMs, easing in and out over the blend times. */
		int32_t iFreezeMs = {};
		int32_t iFreezeInMs = {};
		int32_t iFreezeOutMs = {};

		/* Knockback. A negative iPushMs is a pull toward the caster. */
		int32_t iPushMs = {};
		int32_t iPushRange = {};

		/* 0 none, 1 circle/ring, 2 box (iAreaAngle is the width in cm), 3 fan
		(iAreaAngle is the sweep in degrees); iAreaInner is the ring hole. */
		int32_t iAreaType = {};
		int32_t iAreaRange = {};
		int32_t iAreaAngle = {};
		int32_t iAreaHeight = {};
		int32_t iAreaOffsetX = {};
		int32_t iAreaInner = {};
		int32_t iMaxTargets = {};
	};

	struct ANIM_EVENT
	{
		std::string clipName;
		EVENT_KIND eKind = EVENT_KIND::HIT;
		/* Milliseconds from the clip start. Frames are only a view: the tool
		converts with the clip's own tick rate, so a clip re-exported at another
		rate keeps its events where they were authored. */
		int32_t iStartMs = {};
		int32_t iEndMs = {};
		/* Point kinds only: which cue/particle to fire. */
		std::string sPayload;
		EFFECT_REFERENCE_KIND eEffectReferenceKind =
			EFFECT_REFERENCE_KIND::NONE;
		std::string sAnchorSlotId = "root";
		EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
		EFFECT_ORIENTATION_POLICY eOrientationPolicy =
			EFFECT_ORIENTATION_POLICY::ANCHOR;
		EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
		EFFECT_TRANSFORM_DESC EffectLocalTransform{};
		/* HIT only. */
		HIT_PARAMS hit;
		/* Came from Import_Notifies rather than being authored here. Re-importing
		drops these and keeps everything else, so the button stays idempotent
		without discarding hand-made work. */
		bool_t bImported = false;
	};

	/* One row of the .animnotify timeline lifted out of the game's Action table.
	Times are seconds from the clip start, the same clock the tool authors in. */
	struct NOTIFY_ROW
	{
		f32_t fTime = {};
		f32_t fDuration = {};
		EVENT_KIND eKind = EVENT_KIND::EFFECT;
		std::string sAsset;
		/* What the window is for, straight from the game's own comment on the
		notify ("이동캔슬", "[선콤]"). Only the input-timing windows carry one. */
		std::string sLabel;
	};

	/* One playback chain of a skill: the clips in the order the game plays them.
	A skill has several because each tripod build takes a different route, so a
	clip can appear in more than one chain. */
	struct CLIP_SEQ
	{
		int32_t iSkillId = {};
		int32_t iSeqIndex = {};
		std::string name;
		/* How the chain is driven: COMBO advances a step per press, HOLD is a
		charge, ONESHOT and SEQUENCE run to the end on one press. Inferred from
		clip naming by the extractor -- the game data has no such field. */
		std::string sMode;
		std::vector<std::string> clips;
		/* Per-step playback seconds from the source stage's unconditional
		handover notify, aligned with clips; empty when the sidecar is absent
		or misaligned. */
		std::vector<f32_t> cuts;
	};

	/* One step of a chain the animator assembles by hand instead of reading it
	out of .clipseq. The duration carries the same meaning the source cuts do:
	zero plays the clip's native length, shorter cuts it, longer loops it until
	the step is filled. Session-scoped -- nothing here is authoring data yet. */
	struct CUSTOM_CHAIN_STEP
	{
		std::string clipName;
		f32_t fDurationSeconds = {};
	};

	/* A saved chain in Valtan.presentation.debug.json. The target ids are the
	stage this chain is meant for and stay free text on purpose: the pattern may
	not be promoted into Valtan.gameplay.json yet, and the debug file is a
	handoff note rather than a document the publisher joins. */
	struct CUSTOM_CHAIN_ENTRY
	{
		std::string chainId;
		std::string targetPatternId;
		std::string targetStageId;
		std::vector<CUSTOM_CHAIN_STEP> steps;
	};

	/* One hit of a .skilltiming row. v2 files spell these out on "hit" lines; a
	v1 file only has windows, so the rest stays zero. */
	struct SKILL_HIT
	{
		int32_t iTimeMs = {};
		int32_t iWidthMs = {};
		bool_t bTimed = { true };
		std::string sSourceKeys;
		HIT_PARAMS hit;
	};

	struct SKILL_TIMING
	{
		/* iSkillId is the tripod variant; iBaseSkillId is the skill it belongs to
		and is what the clip map is keyed by. */
		int32_t iSkillId = {};
		int32_t iBaseSkillId = {};
		std::string name;
		std::vector<SKILL_HIT> hits;
		/* Per-skill flags from the Skill table, shown as context only. */
		int32_t iSuperArmor = {};
		int32_t iMoveSpeed = {};
	};

	/* Which skill a model clip belongs to, read from the .clipmap the extractor
	builds out of the game's Action table. Clip names are English and the skill
	names are Korean, so without this the two lists cannot be lined up by eye. */
	struct CLIP_INFO
	{
		int32_t iSkillId = {};
		std::string name;
	};

public:
	explicit CAnimation_Tool(
		shared_ptr<CCharacterPreviewPanel> pPreviewPanel,
		CBalanceTool* pBalanceTool,
		CBossTool* pBossTool);
	~CAnimation_Tool();

	void Update(f32_t fTimeDelta, bool_t bIsActiveTool);
	void Render();
	void On_LevelChanged();
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest);
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& outRequest);
	/* Opens the canonical Valtan data workspace even when a local preview model
	   cannot be staged. The Product data shell and Server Complete Play remain
	   available; only model-dependent preview/edit capabilities are disabled. */
	bool_t Open_ValtanWorkspace();
	/* Typed Resource Files handoff for one extracted Kakul profile. 07 is an
	   authoring profile alias whose physical preview body is MN_RPCT_05. */
	bool_t Open_KakulProfile(const std::string& profileId);
	bool_t Open_KakulAction(
		const std::string& profileId,
		std::uint32_t iSourceActionId);
	bool_t Stage_ValtanCompositionPreview(std::string& strOutStatus);
	bool_t Play_ValtanCompositionPattern(
		const std::string& strPatternId,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		std::string& strOutStatus);
	/* Local-only preview of the effective typed authoring draft.  It never
	   changes Server state or reads a generated Product as an owner; the value
	   copy is compiled into the same real-model playlist as canonical preview. */
	bool_t Play_ValtanCompositionDraftPattern(
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		std::string& strOutStatus);
	bool_t Seek_ValtanCompositionPattern(
		const std::string& strPatternId,
		uint32_t iPositionMs,
		bool_t bPause,
		std::string& strOutStatus);
	void Stop_ValtanCompositionPattern(std::string& strOutStatus);
	COMPOSITION_PREVIEW_STATE Get_ValtanCompositionPreviewState() const;
	bool_t Get_ValtanCompositionSequences(
		std::vector<COMPOSITION_SEQUENCE_VIEW>& OutSequences,
		std::string& strOutStatus);
	/* One model-independent authoring catalog assembled only from the fixed
	   Valtan and Kakul/Saydon typed source documents. It never scans Resources
	   or the repository, and is re-read only when its owning UI explicitly
	   reloads the catalog. */
	bool_t Get_ActionCompositionSequenceCatalog(
		std::vector<COMPOSITION_SEQUENCE_VIEW>& OutSequences,
		std::string& strOutStatus);
	/* Native source-window admission uses the exact body + attached AnimSet
	   CModel already staged for Valtan.  It never substitutes a scene model and
	   never treats the extracted .clipcuts wall duration as source play time. */
	bool_t Resolve_ValtanCompositionNativeClipDurationMs(
		const std::string& strClipName,
		uint32_t& iOutRoundedDurationMs,
		std::string& strOutStatus) const;
	bool_t Validate_ValtanCompositionAnimationStageMutation(
		const VALTAN_STAGE_VIEW& BaselineStage,
		const VALTAN_STAGE_VIEW& CandidateStage,
		std::string& strOutStatus) const;
	bool_t Validate_ValtanCompositionAnimationGraphMutations(
		const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
		const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
		std::string& strOutStatus) const;
	bool_t Preview_ValtanCompositionSequence(
		int32_t iSkillId,
		int32_t iSequenceIndex,
		std::string& strOutStatus);
	bool_t Stage_ValtanCompositionIntakeSequence(
		int32_t iSkillId,
		int32_t iSequenceIndex,
		const std::string& strTargetPatternId,
		const std::string& strTargetStageId,
		std::string& strOutStatus);
	void Set_ValtanCompositionLoop(bool_t bLoop);
	void Render_ValtanCompositionPatternCreator();
	bool_t Consume_ValtanCompositionPatternCreated(
		std::string& strOutPatternId);
	bool_t Is_ValtanCompositionPatternTransactionActive() const;
	/* Pattern Sound is a separate typed authoring owner from the split
	   gameplay/presentation Pattern transaction.  Action Composition Workbench
	   borrows this one draft instead of loading a second editable document. */
	bool_t Ensure_ValtanCompositionPatternSounds(std::string& strOutStatus);
	bool_t Reload_ValtanCompositionPatternSounds(std::string& strOutStatus);
	/* Explicit Save/Discard/Cancel navigation boundary.  Ordinary reload
	   callers never imply consent to lose a Pattern Sound draft. */
	bool_t Discard_ValtanCompositionPatternSoundDraftAndReload(
		std::string& strOutStatus);
	/* Save/reload/runtime Apply alter the exact S generation or consumer cue
	   cache.  Draft edits remain local, but these operations are fail-closed
	   while any Server audition, Restart, Next or Flow occurrence owns it. */
	bool_t Can_CommitValtanCompositionPatternSoundGeneration(
		std::string& strOutStatus);
	bool_t Retry_ValtanCompositionPatternSoundRuntimeApply(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus);
	bool_t Is_ValtanCompositionPatternSoundRuntimeReady(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus) const;
	void Invalidate_ValtanCompositionPatternSoundRuntimeApply(
		const std::string& strStatus);
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT*
		Get_ValtanCompositionPatternSoundDraft(
			bool_t& bOutDirty,
			std::string& strOutStatus) const;
	[[nodiscard]] uint64_t Get_ValtanCompositionPatternSoundDraftGeneration() const
	{
		return m_iValtanPatternSoundDraftGeneration;
	}
	std::vector<std::string> Collect_ValtanCompositionPatternSoundEvents() const;
	bool_t Resolve_ValtanCompositionPatternSoundWindow(
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		uint32_t& iOutMinimumStartMs,
		uint32_t& iOutMaximumStartMs,
		bool_t& bOutLoop,
		std::string& strOutStatus) const;
	/* Cross-owner dependency admission.  A Pattern/Animation mutation may not
	   delete a Sound-qualified clip occurrence or move its cue outside the
	   candidate Stage/model timeline.  An explicit PROJECT_AUTHORED resource
	   replacement may preserve the logical occurrence ID only after the new
	   source window/repeat policy is revalidated.  Stages with no Sound rows
	   remain valid without a model. */
	bool_t Validate_ValtanCompositionPatternSoundStageDependencies(
		const VALTAN_PATTERN_VIEW& BaselinePattern,
		const VALTAN_STAGE_VIEW& BaselineStage,
		const VALTAN_STAGE_VIEW& CandidateStage,
		std::string& strOutStatus) const;
	/* Canonical Save uses the complete effective Pattern draft, not just the
	   selected Stage, so mutations staged through another typed view cannot
	   leave the separate Pattern Sound source pointing at an old generation. */
	bool_t Validate_ValtanCompositionPatternSoundGraphDependencies(
		const std::vector<VALTAN_PATTERN_VIEW>& BaselinePatterns,
		const std::vector<VALTAN_PATTERN_VIEW>& CandidatePatterns,
		std::string& strOutStatus) const;
	bool_t Patch_ValtanCompositionPatternSound(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strOccurrenceId,
		const std::string& strSoundEvent,
		uint32_t iStartMs,
		VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
		std::string& strOutStatus);
	bool_t Add_ValtanCompositionPatternSound(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		const std::string& strSoundEvent,
		uint32_t iStartMs,
		VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy,
		VALTAN_PATTERN_SOUND_CUE_ROW_ID& OutCreatedRowId,
		std::string& strOutStatus);
	bool_t Remove_ValtanCompositionPatternSound(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PATTERN_SOUND_CUE_ROW_ID& RowId,
		std::string& strOutStatus);
	/* Deleting one Animation occurrence may intentionally retire the Sound rows
	   that are qualified by that exact occurrence.  The Workbench snapshots the
	   typed Sound draft here, stages every exact dependent removal together, and
	   restores the snapshot if the paired Pattern mutation is rejected. */
	bool_t Stage_ValtanCompositionPatternSoundCascadeForAnimationDelete(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT& OutPreviousDraft,
		bool_t& bOutPreviousDirty,
		uint64_t& iOutMutationGeneration,
		std::size_t& iOutRemovedRowCount,
		std::string& strOutStatus);
	bool_t Restore_ValtanCompositionPatternSoundCascade(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& PreviousDraft,
		bool_t bPreviousDirty,
		uint64_t iExpectedMutationGeneration,
		std::string& strOutStatus);
	bool_t Prepare_ValtanCompositionPatternSoundSave(
		std::string& strOutBaselineBytes,
		std::string& strOutCandidateBytes,
		uint64_t& iOutDraftGeneration,
		bool_t& bOutDirty,
		std::string& strOutStatus) const;
	bool_t Accept_ValtanCompositionPatternSoundSave(
		uint64_t iExpectedDraftGeneration,
		const std::string& strExpectedCandidateBytes,
		std::string& strOutStatus);
	bool_t Save_ValtanCompositionPatternSounds(std::string& strOutStatus);

private:
	bool_t Resolve_ValtanCompositionNativeModel(
		shared_ptr<Engine::CModel>& pOutModel,
		std::string& strOutStatus) const;
	bool_t Apply_ValtanCompositionPatternSoundsToActiveConsumers(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus);
	shared_ptr<Engine::CModel> Resolve_Model() const;
	/* The character owning that model. Its spec names the data files, so the tool
	follows whichever class the level placed instead of assuming one. */
	shared_ptr<CCharacter> Resolve_Character() const;
	/* Points the file paths at the resolved character's asset and drops anything
	loaded for a previous one. */
	bool_t Sync_AssetName();
	void Adopt_AssetName(const std::string& assetName);
	void Render_TargetConflict();
	bool_t Is_ValtanDocumentDirty() const;
	void Render_Playback(const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanWorkspaceTabs(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanAnimationSourceWorkspace(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternPreview(const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternMaster(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternMasterUnavailableShell(
		std::size_t iAdmittedPatternCount,
		bool_t bHasPreviewModel);
	void Render_ValtanSelectedResourceUsage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW* pStage);
	const char_t* ValtanPatternMasterAdmissionLabel() const;
	void Render_ValtanPatternReferenceWindow(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanCustomChainWindow(
		const shared_ptr<Engine::CModel>& pModel);
	/* Feeds the hand-built steps through the same preview playlist the source
	   sequences use, so transport, cuts and looping behave identically. */
	bool_t Start_ValtanCustomChainPreview(
		const shared_ptr<Engine::CModel>& pModel);
	std::filesystem::path Get_CustomChainFilePath() const;
	/* One reader for every chain document, so an imported file and a reloaded
	   one are admitted by exactly the same rules. */
	bool_t Parse_CustomChainDocument(
		const std::string& text,
		std::vector<CUSTOM_CHAIN_ENTRY>& Out,
		std::string& strOutError) const;
	bool_t Read_CustomChainDocument(
		const std::filesystem::path& source,
		std::vector<CUSTOM_CHAIN_ENTRY>& Out,
		std::string& strOutError) const;
	bool_t Load_CustomChainLibrary();
	/* Whole-file atomic replace. A rejected write leaves the previous library
	   on disk and in memory so a failed save never costs saved chains. */
	bool_t Save_CustomChainLibrary();
	/* Exact source identity is attached only while the current hand-built chain
	   is still the untouched copy staged from one extracted Sequence.  Any
	   manual edit drops the identity instead of publishing a misleading
	   (actionId, sequenceIndex) owner for a different chain. */
	void Invalidate_ValtanPatternCreateExactSourceSelection();
	void Render_ValtanPatternCreatePanel();
	bool_t Build_ValtanPatternCreateRequest(
		std::string& strOutRequest,
		std::string& strOutError) const;
	bool_t Start_ValtanPatternCreateCommand(bool_t bApply);
	void Poll_ValtanPatternCreateCommand();
	bool_t Parse_ValtanPatternCreateResult(
		const std::string& strDiagnostic,
		std::string& strOutError) const;
	bool_t Reload_ValtanPatternMaster();
	bool_t Reload_ValtanPatternSoundCues();
	bool_t Reload_ValtanPatternShakeCues();
	bool_t Reload_ValtanCombatObjectSoundCues();
	bool_t Load_ValtanAnimationBindingDraft(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Render_ValtanAnimationBindingInspector(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	void Render_ValtanCounterWindowInspector(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& SavedStage);
	bool_t Render_ValtanPatternSoundInspector(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	void Render_ValtanStageDraftInspector(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& SavedStage);
	void Render_ValtanPresentationLanes(
		const VALTAN_PATTERN_VIEW& Pattern,
		std::string_view strStageFilter = {});
	bool_t Preview_ValtanSoundAsset(
		const std::string& strResourceAssetId);
	std::vector<const VALTAN_PATTERN_VIEW*>
		Collect_ValtanPatternMasterPatterns() const;
	bool_t Build_ValtanPatternMasterTimeline(
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		const shared_ptr<Engine::CModel>& pModel,
		std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>& OutPlaylist,
		uint32_t& iOutDurationMs,
		std::string& strOutStatus) const;
	bool_t Start_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath);
	bool_t Activate_ValtanPatternMasterItem(
		const shared_ptr<Engine::CModel>& pModel,
		std::size_t iItem,
		f32_t fLocalWallSeconds);
	bool_t Apply_ValtanPatternMasterPose(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item,
		f32_t fLocalWallSeconds,
		bool_t bForceAnimationEdge) const;
	bool_t Seek_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		f32_t fTimelineSeconds,
		bool_t bPause,
		bool_t bResetPresentationTransport);
	void Advance_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel);
	void Stop_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const std::string& status);
	void Reset_ValtanPatternMasterPreviewState(const std::string& status);
	void Update_ValtanPatternMasterHitAreaPreview();
	static const char_t* ValtanPatternMasterPathName(
		VALTAN_PATTERN_PREVIEW_PATH ePath);
	bool_t Start_ValtanSequencePreview(
		const shared_ptr<Engine::CModel>& pModel,
		std::size_t iSequenceIndex);
	bool_t Start_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		uint32_t iFirstPattern,
		uint32_t iLastPattern);
	bool_t Activate_ValtanPatternPreviewItem(
		const shared_ptr<Engine::CModel>& pModel);
	void Advance_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel);
	void Stop_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const std::string& status);
	void Reset_ValtanPatternPreviewState(const std::string& status);
	/* Mirrors the arena's pattern hit wires during tool playback: joins the
	   playing item's source action to its encounter pattern and hands the
	   boss preview the active stage actionId and stage-local clock. */
	void Update_ValtanPatternHitAreaPreview();
	void Render_ClipChain(const shared_ptr<Engine::CModel>& pModel);
	void Render_NotifyReference(const shared_ptr<Engine::CModel>& pModel);
	void Bind_ReferenceWire(const std::string& sourceKey);
	void Render_HitEvents(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitDetail(ANIM_EVENT& evt);
	/* Wire overlay of the selected and playhead-active HIT areas, drawn on the
	scene character so an authored box/fan/circle can be judged against the
	pose. Reads the same ANIM_EVENT rows the list edits; nothing new is
	stored. */
	void Render_HitAreaWires(const shared_ptr<Engine::CModel>& pModel) const;
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);
	void Consume_EffectTransfer(const shared_ptr<Engine::CModel>& pModel);
	void Render_SkillReference(
		const shared_ptr<Engine::CModel>& pModel,
		bool_t bReadOnly);
	void Render_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		const shared_ptr<CCharacter>& pCharacter);
	void Render_SkillBindingReloadConfirmation(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	void Render_KakulActionBindings(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_KakulActionBindings(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Save_KakulActionBindings(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Save_KakulAnimationPatterns(
		const shared_ptr<Engine::CModel>& pModel);
	KAKUL_ANIMATION_ACTION_BINDING* Find_KakulActionBinding(
		std::uint32_t iSourceActionId,
		const std::string& strStageId,
		const std::string& strSlotId);
	const KAKUL_ANIMATION_ACTION_BINDING* Find_KakulActionBinding(
		std::uint32_t iSourceActionId,
		const std::string& strStageId,
		const std::string& strSlotId) const;
	void Upsert_KakulActionBinding(
		const KAKUL_ANIMATION_ACTION_SLOT_REFERENCE& ReferenceSlot,
		std::uint32_t iSourceActionId,
		const std::string& strStageId,
		const std::string& strRuntimeClip,
		std::uint32_t iSourceStartMs,
		std::uint32_t iPlayMs,
		f32_t fPlayRate,
		bool_t bLoop);
	void Remove_KakulActionBinding(
		std::uint32_t iSourceActionId,
		const std::string& strStageId,
		const std::string& strSlotId);
	void Render_KakulPatternAuthoring(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Build_KakulPatternFromAction(
		const shared_ptr<Engine::CModel>& pModel,
		const KAKUL_ANIMATION_ACTION_REFERENCE& Action,
		const std::string& strPatternId,
		KAKUL_ANIMATION_PATTERN& outPattern,
		std::string& strOutStatus) const;
	bool_t Start_KakulPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const KAKUL_ANIMATION_PATTERN& Pattern,
		const std::string& strLabel);
	bool_t Activate_KakulPatternPreviewClip(
		const shared_ptr<Engine::CModel>& pModel);
	void Advance_KakulPatternPreview(
		const shared_ptr<Engine::CModel>& pModel);
	void Stop_KakulPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const std::string& strStatus);
	void Reset_KakulPatternPreviewState(const std::string& strStatus);
	std::string Resolve_KakulIdleClip() const;
	void Reset_KakulActionDocumentState(bool_t bClearProfile);

	bool_t Save_Events(const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_Events(const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_EventsFromPath(
		const std::filesystem::path& path,
		const shared_ptr<Engine::CModel>& pModel,
		std::vector<ANIM_EVENT>& outEvents,
		int32_t& outSourceVersion,
		std::string& outStatus) const;
	bool_t Write_EventsToPath(
		const std::filesystem::path& path,
		const std::vector<ANIM_EVENT>& events,
		std::string& outStatus) const;
	bool_t Validate_Events(
		const shared_ptr<Engine::CModel>& pModel,
		const std::vector<ANIM_EVENT>& events,
		std::string& outStatus) const;
	bool_t Events_AreEqual(
		const std::vector<ANIM_EVENT>& left,
		const std::vector<ANIM_EVENT>& right) const;
	void Render_ReloadConfirmation(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_SkillReference();
	bool_t Load_ClipMap();
	bool_t Load_ClipNotify();
	bool_t Load_ClipSeq();
	void Load_ClipCuts();
	bool_t Load_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool_t Save_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		const shared_ptr<CCharacter>& pCharacter);
	bool_t Create_SkillBindingDraft(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	std::vector<std::string> Collect_ClipNames(
		const shared_ptr<Engine::CModel>& pModel) const;
	ANIMATION_SKILL_BINDING* Find_SkillBinding(
		LostArk::Shared::SKILL_ID skillId);
	bool_t Is_AnyDocumentDirty() const;
	/* Milliseconds of clip that play before iIndex in seq, i.e. where that clip
	starts on the whole-cast clock the .skilltiming rows use. */
	int32_t Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const;
	/* Replaces this clip's previously imported events with the original notifies
	of the kinds currently ticked. Returns how many were added. */
	int32_t Import_Notifies(const char_t* pClipName, f32_t fTickRate,
		int32_t& iShapedHits);
	const SKILL_TIMING* Find_ReferenceRow(const char_t* pClipName) const;
	int32_t Count_PrecedingChainHits(const char_t* pClipName) const;
	static int32_t Count_DistinctHitNotifies(const std::vector<NOTIFY_ROW>& rows);
	/* Korean skill name owning the clip, or nullptr when it is unmapped. */
	const CLIP_INFO* Find_ClipInfo(const char_t* pClipName) const;
	/* Selects the named clip in the model, or does nothing if it has no such clip. */
	void Select_Clip(const shared_ptr<Engine::CModel>& pModel, const std::string& clipName);
	std::string Get_EventFilePath() const;
	std::string Get_SkillReferencePath() const;
	std::string Get_ClipMapPath() const;
	std::string Get_ClipNotifyPath() const;
	std::string Get_ClipSeqPath() const;

	/* Tick rate of the named clip, falling back to DEFAULT_TICK_RATE when the
	clip is unknown or the asset carries no rate. */
	static f32_t Get_ClipTickRate(const shared_ptr<Engine::CModel>& pModel,
		const std::string& clipName);

	static bool_t Is_Window(EVENT_KIND eKind);
	static const char_t* Kind_Name(EVENT_KIND eKind);
	static const char_t* Area_Name(int32_t iAreaType);
	static int32_t Ms_To_Frame(int32_t iMs, f32_t fTickRate);
	static int32_t Frame_To_Ms(int32_t iFrame, f32_t fTickRate);
	/* When the iTickIndex'th hit of evt lands. */
	static int32_t Get_TickMs(const ANIM_EVENT& evt, int32_t iTickIndex);
	/* Which hit covers iMs, or -1 when the event is not active there. */
	static int32_t Get_ActiveTick(const ANIM_EVENT& evt, int32_t iMs);

private:
	char m_Filter[128]{};
	bool_t m_bLoop = true;
	bool_t m_bShowHitAreas = true;
	VALTAN_PATTERN_TREE_VIEW m_ValtanPatternMasterView;
	VALTAN_PATTERN_VIEW m_ValtanCompositionDraftPreview;
	bool_t m_bValtanCompositionDraftPreviewReady = false;
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>
		m_ValtanPatternMasterPlaylist;
	bool_t m_bValtanPatternMasterLoadAttempted = false;
	VALTAN_PATTERN_MASTER_ADMISSION_STATE m_eValtanPatternMasterAdmission =
		VALTAN_PATTERN_MASTER_ADMISSION_STATE::UNLOADED;
	bool_t m_bValtanPatternMasterPlaying = false;
	bool_t m_bValtanPatternMasterPaused = false;
	bool_t m_bValtanCompositionLoop = false;
	bool_t m_bShowValtanSourceReferenceWindow = false;
	bool_t m_bShowValtanCustomChainWindow = false;
	std::vector<CUSTOM_CHAIN_STEP> m_CustomChainSteps;
	std::vector<CUSTOM_CHAIN_ENTRY> m_CustomChainLibrary;
	bool_t m_bCustomChainLibraryLoadAttempted = false;
	char m_CustomChainFilter[128]{};
	char m_CustomChainId[64]{};
	char m_CustomChainTargetPatternId[64]{};
	char m_CustomChainTargetStageId[64]{};
	std::string m_strCustomChainStatus;
	/* Create New Pattern is a separate explicit transaction over either the
	   current unsaved steps or one stable saved intake chain. The validated
	   request digest gates Apply so changed fields can never inherit an earlier
	   green result. Request and diagnostic files remain on disk for inspection. */
	int32_t m_iValtanPatternCreateSourceKind = 0;
	int32_t m_iValtanPatternCreateSavedIndex = 0;
	char m_ValtanPatternCreatePatternId[161]{};
	char m_ValtanPatternCreateDisplayName[256]{};
	int32_t m_iValtanPatternCreateAuthoringPhase = 1;
	int32_t m_iValtanPatternCreateTargetPolicy = 0;
	int32_t m_iValtanPatternCreateAimPolicy = 0;
	void* m_hValtanPatternCreateProcess = nullptr;
	std::filesystem::path m_ValtanPatternCreateRequestPath;
	std::filesystem::path m_ValtanPatternCreateDiagnosticPath;
	std::string m_strValtanPatternCreateStatus;
	std::string m_strValtanPatternCreateDiagnostic;
	std::string m_strValtanPatternCreateActiveRequestSha256;
	std::string m_strValtanPatternCreateValidatedRequestSha256;
	std::string m_strValtanPatternCreateActivePatternId;
	bool_t m_bValtanPatternCreateExactSourceSelection = false;
	int32_t m_iValtanPatternCreateSourceActionId = -1;
	int32_t m_iValtanPatternCreateSourceSequenceIndex = -1;
	bool_t m_bValtanPatternCreateActiveApply = false;
	bool_t m_bValtanPatternCreateHasExitCode = false;
	uint32_t m_iValtanPatternCreateExitCode = 0u;
	uint32_t m_iValtanPatternCreateCommandSequence = 0u;
	uint64_t m_iValtanPatternCreateStartedAtMilliseconds = 0u;
	bool_t m_bValtanCompositionPatternCreatedPending = false;
	std::string m_strValtanCompositionPatternCreatedId;
	/* Preview-only cross-fade between chain steps. CCharacter already blends
	   its clips at 0.12 s; the product Valtan does not blend at all yet, so a
	   sequence judged here reads smoother than the live boss until the boss
	   owner adopts the same value. */
	f32_t m_fPreviewBlendSeconds = 0.12f;
	VALTAN_WORKBENCH_SELECTION_KIND m_eValtanWorkbenchSelection =
		VALTAN_WORKBENCH_SELECTION_KIND::STAGE;
	VALTAN_WORKBENCH_DETAIL_OWNER m_eValtanWorkbenchDetailOwner =
		VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY;
	bool_t m_bValtanWorkbenchFocusDetailRequested = false;
	std::string m_strValtanWorkbenchPatternId;
	std::string m_strValtanWorkbenchStageId;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT
		m_ValtanPatternAnimationBindingDraft;
	bool_t m_bValtanPatternAnimationBindingLoadAttempted = false;
	bool_t m_bValtanPatternAnimationBindingReady = false;
	bool_t m_bValtanPatternAnimationBindingDirty = false;
	std::string m_strValtanPatternAnimationBindingBaselineSourceBytes;
	std::string m_strValtanPatternAnimationBindingStatus;
	std::string m_strValtanAnimationBindingDetailActionId;
	std::string m_strValtanAnimationBindingNewClip;
	int32_t m_iValtanPatternMasterSelected = 0;
	std::size_t m_iValtanPatternMasterItem = 0u;
	f32_t m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	uint32_t m_iValtanPatternMasterDurationMs = 0u;
	VALTAN_PATTERN_PREVIEW_PATH m_eValtanPatternMasterPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	std::string m_strValtanPatternMasterStatus;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT m_ValtanPatternSoundCues;
	bool_t m_bValtanPatternSoundCuesReady = false;
	bool_t m_bValtanPatternSoundCuesDirty = false;
	uint64_t m_iValtanPatternSoundDraftGeneration = 0u;
	bool_t m_bValtanPatternSoundRuntimeApplyReady = false;
	LostArk::Shared::GameplayDataRevision
		m_ValtanPatternSoundRuntimeAppliedRevision{};
	std::string m_strValtanPatternSoundCueBaselineSourceBytes;
	std::string m_strValtanPatternSoundCueStatus;
	/* Sound Detail can ask for the same clip window every ImGui frame.  Cache
	   immutable native durations per resolved model instead of walking every
	   animation on each draw; authoring mutations still revalidate on submit. */
	mutable std::weak_ptr<Engine::CModel> m_ValtanPatternSoundDurationModel;
	mutable std::unordered_map<std::string, f32_t>
		m_ValtanPatternSoundClipDurations;
	std::string m_strValtanPatternSoundAddClipOccurrenceId;
	std::string m_strValtanPatternSoundAddEvent;
	uint32_t m_iValtanPatternSoundAddStartMs = 0u;
	VALTAN_PATTERN_SOUND_REPEAT_POLICY m_eValtanPatternSoundAddRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT m_ValtanPatternShakeCues;
	bool_t m_bValtanPatternShakeCuesReady = false;
	std::string m_strValtanPatternShakeCueStatus;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT m_ValtanCombatObjectSoundCues;
	bool_t m_bValtanCombatObjectSoundCuesReady = false;
	bool_t m_bValtanCombatObjectSoundCuesDirty = false;
	std::string m_strValtanCombatObjectSoundCueStatus;
	bool_t m_hasEffectToolOpenRequest = false;
	std::string m_strEffectToolOpenPatternId;
	std::string m_strEffectToolOpenStageId;
	std::string m_strEffectToolOpenCueOccurrenceId;
	std::string m_strEffectToolOpenEffectAssetId;
	bool_t m_hasCameraToolOpenRequest = false;
	std::string m_strCameraToolOpenCueId;
	std::weak_ptr<Engine::CModel> m_ValtanPatternMasterModel;
	std::weak_ptr<CValtan> m_ValtanPatternMasterBoss;
	uint64_t m_iValtanPatternMasterTargetGeneration = 0u;
	VALTAN_PATTERN_PREVIEW_DOCUMENT m_ValtanPatternPreviewDocument;
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> m_ValtanPatternPreviewPlaylist;
	bool_t m_bValtanPatternPreviewLoadAttempted = false;
	bool_t m_bValtanPatternPreviewPlaying = false;
	bool_t m_bValtanPatternPreviewPaused = false;
	int32_t m_iValtanPatternPreviewSelected = 0;
	std::size_t m_iValtanPatternPreviewItem = 0u;
	f32_t m_fValtanPatternPreviewElapsedSeconds = 0.f;
	f32_t m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	f32_t m_fValtanPatternPreviewSpeed = 1.f;
	std::string m_strValtanPatternPreviewStatus;
	std::weak_ptr<Engine::CModel> m_ValtanPatternPreviewModel;
	uint64_t m_iValtanPatternPreviewTargetGeneration = 0u;
	char_t m_ValtanPatternFilter[128]{};
	int32_t m_iValtanSequenceSelected = -1;
	bool_t m_bValtanRaidSequencesOnly = false;
	CEncounterPatternReference m_ValtanEncounterReference;
	bool_t m_bValtanEncounterReferenceLoadAttempted = false;
	/* Seconds of finished same-sequence items before the current one, so the
	   encounter stage lookup runs on the whole-pattern clock. */
	f32_t m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	/* Shared with Effect Tool through MainApp. This tool only contributes the
	unsaved Animation document lock to that one preview session. */
	shared_ptr<CCharacterPreviewPanel> m_pPreviewPanel;
	/* Non-owning orchestration endpoints. MainApp creates these before the
	   Workbench and owns all three until Client shutdown. */
	CBalanceTool* m_pBalanceTool = nullptr;
	CBossTool* m_pBossTool = nullptr;
	bool_t m_bResetWorkbenchLayoutRequested = false;
	bool_t m_bValtanDataWorkspaceRequested = false;
	bool_t m_bValtanWorkspaceTabInitialized = false;
	uint64_t m_iValtanAutoPreviewAttemptGeneration = 0u;
	uint64_t m_iValtanAutoPreviewSuccessGeneration = 0u;

	std::vector<ANIM_EVENT> m_Events;
	/* Empty until a character resolves; Sync_AssetName fills it from the spec. */
	std::string m_AssetName;
	std::string m_Status;
	bool_t m_bDirty = false;
	bool_t m_bReloadConfirmationRequested = false;
	bool_t m_bLoadAttempted = false;
	std::string m_PendingAssetName;
	int32_t m_iSelectedEvent = -1;

	/* Buffer backing the payload text field of the selected point event. */
	char m_PayloadEdit[128]{};

	std::vector<SKILL_TIMING> m_SkillRef;
	bool_t m_bRefLoadAttempted = false;
	char m_RefFilter[128]{};
	int32_t m_iRefWireSkillId = 0;
	int32_t m_iRefWireHitIndex = -1;

	std::map<std::string, CLIP_INFO> m_ClipMap;
	bool_t m_bClipMapLoadAttempted = false;

	std::map<std::string, std::vector<NOTIFY_ROW>> m_ClipNotify;
	/* Clip length in seconds, off the .animnotify headers. The game's own value,
	which is what the chain offsets have to be summed from. */
	std::map<std::string, f32_t> m_ClipLength;
	bool_t m_bClipNotifyLoadAttempted = false;

	std::vector<CLIP_SEQ> m_ClipSeqs;
	bool_t m_bClipSeqLoadAttempted = false;
	std::unordered_map<std::string, int32_t> m_ClipChainCounts;
	std::unordered_set<std::string> m_DuplicateBodyClips;
	bool_t m_bDuplicateScanDone = false;

	ANIMATION_SKILL_BINDING_DOCUMENT m_SkillBindingDocument;
	bool_t m_bSkillBindingLoadAttempted = false;
	bool_t m_bSkillBindingDirty = false;
	bool_t m_bSkillBindingReloadConfirmationRequested = false;
	int32_t m_iSelectedSkillBinding = -1;
	int32_t m_iSelectedSkillStage = 0;
	int32_t m_iSelectedSkillClip = 0;
	std::string m_SkillBindingStatus;
	std::string m_strKakulProfileId;
	KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT m_KakulActionReference;
	KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT m_KakulActionAuthored;
	KAKUL_ANIMATION_PATTERN_DOCUMENT m_KakulAnimationPatterns;
	bool_t m_bKakulActionLoadAttempted = false;
	bool_t m_bKakulActionDirty = false;
	bool_t m_bKakulPatternDirty = false;
	bool_t m_bKakulActionReloadConfirmationRequested = false;
	int32_t m_iSelectedKakulAction = -1;
	std::uint32_t m_iRequestedKakulSourceActionId = 0u;
	int32_t m_iSelectedKakulStage = 0;
	int32_t m_iSelectedKakulSlot = 0;
	int32_t m_iSelectedKakulActionClip = 0;
	int32_t m_iSelectedKakulPattern = -1;
	int32_t m_iSelectedKakulPatternClip = 0;
	f32_t m_fKakulActionListWidth = 320.f;
	char m_KakulActionFilter[128]{};
	char m_KakulPatternFilter[128]{};
	std::string m_strKakulActionStatus;
	std::string m_strKakulPatternStatus;
	std::vector<KAKUL_ANIMATION_PATTERN_CLIP> m_KakulPatternPreviewClips;
	std::string m_strKakulPatternPreviewId;
	std::string m_strKakulPatternPreviewLabel;
	std::weak_ptr<Engine::CModel> m_KakulPatternPreviewModel;
	std::uint64_t m_iKakulPatternPreviewTargetGeneration = 0u;
	std::size_t m_iKakulPatternPreviewClip = 0u;
	f32_t m_fKakulPatternPreviewElapsedSeconds = 0.f;
	f32_t m_fKakulPatternPreviewClipDurationSeconds = 0.f;
	bool_t m_bKakulPatternPreviewPlaying = false;
	bool_t m_bKakulPatternPreviewPaused = false;
	/* Which kinds Import_Notifies takes. Effects alone run to a few thousand
	rows, so being able to pull in just hits and cancels matters. */
	bool_t m_bImportKind[ETOI(EVENT_KIND::END)]{};
	/* Reference times run from the skill's first clip, so a mid-cast clip needs
	its own start subtracted before a stamp lands on the right local time. In
	milliseconds, matching both the reference file and ANIM_EVENT. */
	int32_t m_iCastOffsetMs = {};
};

NS_END
