#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ActionCompositionGraphModel.h"
#include "Animation_Tool.h"
#include "ValtanCombatObjectSoundCueDocument.h"
#include "ValtanPatternShakeCueDocument.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanPatternTree.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CBalanceTool;
class CBossTool;
struct CAMERA_TOOL_OPEN_REQUEST;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;

/* One upper-level authoring surface over existing typed domain owners.  It
   owns no runtime or generated Product document: CValtanPatternTree provides
   the canonical read model, CBalanceTool owns gameplay drafts, Animation Tool
   owns model pose preview, and Effect/Camera requests deep-link to their
   existing owner tools. */
class CActionCompositionWorkbench final
{
private:
	enum class ADMISSION_STATE : uint8_t
	{
		UNLOADED,
		ADMITTED,
		STALE_PRESERVED,
		REJECTED,
	};

	enum class DETAIL_OWNER : uint8_t
	{
		PATTERN,
		GAMEPLAY_STAGE,
		ANIMATION,
		EFFECT,
		SOUND,
		CAMERA,
		WORLD,
		COMBAT_OBJECT,
	};

	enum class TIMELINE_LANE : uint8_t
	{
		STAGE,
		ANIMATION,
		EFFECT,
		SOUND,
		LOGIC,
		COLLIDER,
		CAMERA,
		COUNT,
	};

	enum class RESOURCE_DOMAIN : uint8_t
	{
		ANIMATION,
		EFFECT,
		SOUND,
		CAMERA,
		LOGIC,
	};

	struct TIMELINE_ITEM final
	{
		DETAIL_OWNER eOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		TIMELINE_LANE eLane = TIMELINE_LANE::STAGE;
		std::string strPatternId;
		std::string strStageId;
		std::string strStableId;
		std::string strAssetId;
		std::string strLabel;
		uint32_t iStartMs = 0u;
		uint32_t iEndMs = 0u;
		bool_t bEditable = false;
		std::size_t iSubrow = 0u;
	};

	/* Cached reverse projection from an exact PRIMARY source Sequence to the
	   immutable, Complete-Play-eligible Product Patterns that own it.  Pointers
	   are valid only for m_iCanonicalDisplayGeneration and are discarded before
	   a newly committed canonical view can be rendered. */
	struct SOURCE_SEQUENCE_OWNER_INDEX_ENTRY final
	{
		uint32_t iSourceActionId = 0u;
		uint32_t iSequenceIndex = 0u;
		std::vector<const VALTAN_PATTERN_VIEW*> Owners;
	};

public:
	CActionCompositionWorkbench(
		CAnimation_Tool* pAnimationTool,
		CBalanceTool* pBalanceTool,
		CBossTool* pBossTool);

	bool_t Open_Valtan();
	[[nodiscard]] bool_t Has_DisplaySnapshot() const
	{
		return 0u != m_CanonicalView.Get_PatternCount();
	}
	[[nodiscard]] bool_t Is_FullyAdmitted() const
	{
		return ADMISSION_STATE::ADMITTED == m_eAdmission;
	}
	void On_LevelChanged();
	void Render();
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& OutRequest);
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& OutRequest);
	bool_t Consume_AnimationToolOpenRequest();
	bool_t Consume_PreviewOwnerClaimRequest();
	void Set_PreviewOwnerActive(bool_t bActive)
	{
		m_bPreviewOwnerActive = bActive;
	}

private:
	bool_t Reload_Canonical();
	bool_t Save_Publish_Reload();
	bool_t Is_PatternSoundDraftDirty(std::string& strOutStatus) const;
	bool_t Validate_ManualStageTopologySoundDependencies(
		const VALTAN_PATTERN_VIEW& CandidatePattern,
		std::string& strOutStatus) const;
	void Normalize_Selection();
	void Select_Pattern(const VALTAN_PATTERN_VIEW& Pattern);
	void Select_Stage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		DETAIL_OWNER eOwner = DETAIL_OWNER::GAMEPLAY_STAGE,
		const std::string& strStableId = {});
	const VALTAN_PATTERN_VIEW* Find_SelectedPattern() const;
	const VALTAN_STAGE_VIEW* Find_SelectedStage(
		const VALTAN_PATTERN_VIEW* pPattern) const;
	std::vector<const VALTAN_PATTERN_VIEW*> Collect_Patterns() const;
	std::vector<const VALTAN_PATTERN_VIEW*>
		Collect_CanonicalPatternsForDependencyValidation() const;
	void Invalidate_SourceSequenceOwnerIndex();
	void Ensure_SourceSequenceOwnerIndex();
	const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY* Find_SourceSequenceOwners(
		uint32_t iSourceActionId,
		uint32_t iSequenceIndex) const;
	void Invalidate_TimelineCache();
	void Invalidate_EffectivePatternCache();
	void Ensure_TimelineCache(const VALTAN_PATTERN_VIEW* pPattern);
	void Build_Timeline(const VALTAN_PATTERN_VIEW& Pattern);
	void Pack_TimelineSubrows();
	uint32_t Resolve_ClipSourceToStageMs(
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		uint32_t iSourceMs) const;

	/* True means a canonical reload/save was attempted and any previously
	   resolved Pattern/Stage pointers must not be reused in this ImGui frame. */
	bool_t Render_Toolbar(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted);
	void Render_Browser(
		const VALTAN_PATTERN_VIEW* pEffectiveSelectedPattern);
	void Render_SequenceBrowser(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted);
	bool_t Reload_AnimationSequences();
	bool_t Apply_SelectedSequenceToStage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bAppend);
	bool_t Apply_AnimationOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		uint32_t iSourceStartMs,
		uint32_t iPlayMs,
		std::string& strOutStatus);
	void Render_SelectedAnimationTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		bool_t bMutationAdmitted);
	bool_t Seek_EffectivePreview(
		const VALTAN_PATTERN_VIEW& Pattern,
		uint32_t iPositionMs,
		bool_t bPause,
		std::string& status);
	bool_t Play_EffectivePreview(
		const VALTAN_PATTERN_VIEW& Pattern,
		std::string& status);
	void Render_AnimationStageDetails(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bMutationAdmitted);
	void Render_Preview(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted);
	void Render_Details(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_GameplayStageDetails(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bMutationAdmitted);
	void Render_Timeline(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_PatternDurationControl(
		const VALTAN_PATTERN_VIEW& Pattern,
		bool_t bPatternMutationAdmitted);
	void Render_SelectedStageGapControl(
		const VALTAN_PATTERN_VIEW& Pattern,
		bool_t bPatternMutationAdmitted);
	bool_t Validate_TimelineDependencyWindows(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pEffectOverride,
		const VALTAN_PATTERN_SOUND_CUE* pSoundOverride,
		std::string& strOutStatus) const;
	bool_t Apply_EffectOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
		uint32_t iSourceStartMs,
		uint32_t iSourceEndMs,
		std::string& strOutStatus);
	bool_t Apply_PatternSoundOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PATTERN_SOUND_CUE& Current,
		uint32_t iSourceStartMs,
		std::string& strOutStatus);
	void Render_DataFiles(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_SemanticLinkedRows(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage);
	void Request_EffectOwner(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue);
	void Reload_SemanticValtanEffects();
	void Reset_EffectCueEditor();
	void Render_WindowMenu();
	bool_t Render_SessionWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_PatternsWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_PreviewWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted);
	void Render_SequencerWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_DetailsWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_ResourcesWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_BossPatternWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		std::uint64_t iPatternViewDraftGeneration,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	bool_t Render_BossPatternStageTopologyControls(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bPatternMutationAdmitted);
	void Request_LaneAuthoring(
		TIMELINE_LANE eLane,
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);

	const char_t* Admission_Label() const;
	static const char_t* Owner_Label(DETAIL_OWNER eOwner);
	static const char_t* Lane_Label(TIMELINE_LANE eLane);
	static uint32_t Lane_Color(TIMELINE_LANE eLane);

private:
	CAnimation_Tool* m_pAnimationTool = nullptr;
	CBalanceTool* m_pBalanceTool = nullptr;
	CBossTool* m_pBossTool = nullptr;

	VALTAN_PATTERN_TREE_VIEW m_CanonicalView;
	/* The Browser is the same admitted Product inventory used by Boss Tool
	   Complete Play.  Reference/legacy compatibility rows remain available to
	   the canonical loader for validation, but never become authoring choices. */
	VALTAN_TOOL_AUDITION_INVENTORY m_PlayableInventory;
	std::vector<SOURCE_SEQUENCE_OWNER_INDEX_ENTRY>
		m_SourceSequenceOwnerIndex;
	std::uint64_t m_iCanonicalDisplayGeneration = 0u;
	std::uint64_t m_iSourceSequenceOwnerIndexGeneration =
		~std::uint64_t{ 0u };
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT m_PatternShakes;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT m_CombatObjectSounds;
	ADMISSION_STATE m_eAdmission = ADMISSION_STATE::UNLOADED;
	DETAIL_OWNER m_eDetailOwner = DETAIL_OWNER::PATTERN;
	bool_t m_bLoadAttempted = false;
	bool_t m_bPatternShakesReady = false;
	bool_t m_bCombatObjectSoundsReady = false;
	bool_t m_bResetLayoutRequested = false;
	bool_t m_bApplyResetLayoutThisFrame = false;
	bool_t m_bPatternsWindowVisible = true;
	bool_t m_bPreviewWindowVisible = true;
	bool_t m_bSequencerWindowVisible = true;
	bool_t m_bDetailsWindowVisible = true;
	bool_t m_bResourcesWindowVisible = true;
	bool_t m_bSessionWindowVisible = true;
	bool_t m_bBossPatternWindowVisible = false;
	bool_t m_bBossPatternFocusRequested = false;
	bool_t m_bBossPatternFitRequested = false;
	bool_t m_bLoopPreview = true;
	VALTAN_PATTERN_PREVIEW_PATH m_ePreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	VALTAN_PATTERN_PREVIEW_PATH m_eStagedPreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::END;
	std::uint64_t m_iPreviewDraftGeneration = 0u;
	bool_t m_bPreviewOwnerClaimRequested = false;
	bool_t m_bPreviewOwnerActive = false;
	bool_t m_bOpenAnimationToolRequested = false;
	bool_t m_bEffectToolOpenRequested = false;
	bool_t m_bCameraToolOpenRequested = false;
	bool_t m_bTimelineTrimActive = false;
	std::string m_strTimelineTrimPatternId;
	std::string m_strTimelineTrimStageId;
	std::string m_strTimelineTrimStableId;
	bool_t m_bTimelineMoveActive = false;
	std::string m_strTimelineMovePatternId;
	std::string m_strTimelineMoveStageId;
	std::string m_strTimelineMoveStableId;
	uint32_t m_iTimelineMoveMouseStartMs = 0u;
	uint32_t m_iTimelineMoveSourceStartMs = 0u;
	uint32_t m_iTimelineMoveSourceEndMs = 0u;

	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;
	std::string m_strSelectedStableId;
	bool_t m_bDetailFocusRequested = false;
	RESOURCE_DOMAIN m_eRequestedResourceDomain = RESOURCE_DOMAIN::ANIMATION;
	bool_t m_bResourceDomainSelectionRequested = false;
	std::string m_strPinnedAuthoringSourceRevision;
	std::string m_strPinnedCanonicalSourceRevision;
	std::string m_strDisplayProvenance =
		"No canonical Product display has been admitted.";
	std::string m_strStatus =
		"Load the canonical Valtan graph to begin composition.";
	std::string m_strPatternSaveStatus;
	std::string m_strSoundStatus;
	std::vector<std::string> m_PatternSoundEvents;
	std::string m_strSoundAddClipOccurrenceId;
	std::string m_strSoundAddEvent;
	uint32_t m_iSoundAddStartMs = 0u;
	VALTAN_PATTERN_SOUND_REPEAT_POLICY m_eSoundAddRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	std::string m_strShakeStatus;
	std::string m_strCombatObjectSoundStatus;
	std::array<char_t, 160u> m_PatternSearch{};
	std::array<char_t, 160u> m_ResourceSearch{};
	std::array<char_t, 160u> m_EffectSearch{};
	std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>
		m_AnimationSequences;
	std::vector<std::size_t> m_FilteredAnimationSequenceIndices;
	std::string m_strAnimationSequenceFilterQuery;
	bool_t m_bAnimationSequenceFilterDirty = true;
	bool_t m_bAnimationSequenceLoadAttempted = false;
	int32_t m_iSelectedSequenceSkillId = -1;
	int32_t m_iSelectedSequenceIndex = -1;
	std::string m_strSourceSequenceServerPatternId;
	int32_t m_iDamageProfileSelection = 0;
	uint32_t m_iManualStageInsertDurationMs = 1000u;
	std::string m_strAnimationSequenceStatus;
	bool_t m_bAuthoringDraftDirty = false;
	bool_t m_bPatternSoundDependencyDirty = false;
	bool_t m_bConfirmDiscardPatternSoundDraft = false;
	bool_t m_bSavePatternRequested = false;
	bool_t m_bPatternSaveResultAvailable = false;
	bool_t m_bPatternSaveSucceeded = false;

	std::vector<TIMELINE_ITEM> m_TimelineItems;
	std::array<std::size_t, 7u> m_TimelineLaneSubrowCounts{};
	std::string m_strTimelineCachePatternId;
	std::uint64_t m_iTimelineCacheDraftGeneration = ~std::uint64_t{ 0u };
	std::uint64_t m_iTimelineCacheSoundGeneration = ~std::uint64_t{ 0u };
	uint32_t m_iTimelineDurationMs = 0u;
	uint32_t m_iPlayheadMs = 0u;
	f32_t m_fTimelinePixelsPerSecond = 120.f;

	/* Materializing a typed Pattern draft copies string/vector ownership.  Keep
	   one immutable selected snapshot per draft generation instead of repeating
	   that work in every rendered frame. */
	VALTAN_PATTERN_VIEW m_EffectivePatternCache;
	std::string m_strEffectivePatternCachePatternId;
	std::string m_strEffectivePatternCacheCanonicalRevision;
	std::uint64_t m_iEffectivePatternCacheDraftGeneration =
		~std::uint64_t{ 0u };
	bool_t m_bEffectivePatternCacheReady = false;

	ACTION_COMPOSITION_GRAPH_SNAPSHOT m_BossPatternGraphSnapshot;
	ACTION_COMPOSITION_GRAPH_ERROR m_BossPatternGraphError;
	std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>
		m_BossPatternOutcomeOverrides;
	std::string m_strBossPatternRoutePatternId;
	std::string m_strBossPatternGraphAttemptPatternId;
	std::string m_strBossPatternGraphAttemptCanonicalRevision;
	std::uint64_t m_iBossPatternGraphAttemptDraftGeneration =
		~std::uint64_t{ 0u };
	std::uint64_t m_iBossPatternGraphAttemptRouteGeneration =
		~std::uint64_t{ 0u };
	std::uint64_t m_iBossPatternRouteGeneration = 0u;
	bool_t m_bBossPatternGraphAttempted = false;
	bool_t m_bBossPatternGraphReady = false;
	f32_t m_fBossPatternPanX = 0.f;
	f32_t m_fBossPatternPanY = 0.f;
	f32_t m_fBossPatternZoom = 1.f;

	std::string m_strEffectPatternId;
	std::string m_strEffectStageId;
	std::string m_strEffectOccurrenceId;
	std::string m_strEffectAssetId;
	std::vector<std::string> m_SemanticValtanEffectAssetIds;
	std::vector<std::size_t> m_FilteredEffectAssetIndices;
	std::string m_strEffectFilterQuery;
	bool_t m_bEffectFilterDirty = true;
	bool_t m_bSemanticValtanEffectLoadAttempted = false;
	std::string m_strEffectAddAssetId;
	std::string m_strEffectAddClipOccurrenceId;
	std::string m_strEffectEditIdentity;
	VALTAN_PRODUCT_EFFECT_CUE_VIEW m_EffectCueEditDraft;
	std::string m_strCameraCueId;
};

NS_END
