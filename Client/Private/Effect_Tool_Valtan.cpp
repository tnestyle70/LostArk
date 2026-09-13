#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "ValtanPatternAuditionService.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "Logic_LanceMaster.h"
#include "MainApp.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include "NetworkManager.h"
#include "Valtan.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Transform.h"
#include "CharacterPreviewPanel.h"
#include "BalanceTool.h"

bool_t Client::CEffect_Tool::Play_ValtanClipOccurrence(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
{
	return Play_ValtanStageSequence({ Clip });
}

bool_t Client::CEffect_Tool::Play_ValtanProductCue(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	VALTAN_PRODUCT_PREVIEW Preview;
	Preview.Clip = Clip;
	Preview.Cue = Cue;
	VALTAN_CLIP_OCCURRENCE_VIEW TimelineClip = Clip;
	/* The compatibility overload historically owned exactly one semantic
	   stage, even when its caller did not carry the derived authoring budget. */
	TimelineClip.iAuthoringWallMs = Cue.iStageDurationMs;
	Preview.TimelineClips = { std::move(TimelineClip) };
	Preview.iTimelineDurationMs = Cue.iStageDurationMs;
	return Play_ValtanProductCue(Preview);
}

bool_t Client::CEffect_Tool::Play_ValtanProductCue(
	const VALTAN_PRODUCT_PREVIEW& SourcePreview)
{
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip = SourcePreview.Clip;
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue = SourcePreview.Cue;
	if (Clip.strClipOccurrenceId.empty() ||
		Cue.strClipOccurrenceId != Clip.strClipOccurrenceId ||
		Cue.strEffectAssetId.empty() || 0u == Cue.iStageDurationMs ||
		SourcePreview.TimelineClips.empty() ||
		0u == SourcePreview.iTimelineDurationMs ||
		SourcePreview.iOwningClipTimelineOffsetMs >=
			SourcePreview.iTimelineDurationMs ||
		SourcePreview.iOwningStageTimelineOffsetMs >
			SourcePreview.iOwningClipTimelineOffsetMs ||
		static_cast<uint64_t>(
			SourcePreview.iOwningStageTimelineOffsetMs) +
			Cue.iStageDurationMs > SourcePreview.iTimelineDurationMs)
	{
		m_strPreviewAnimationStatus =
			"Valtan Product cue rejected a stale full-timeline occurrence join.";
		return false;
	}
	uint64_t iDerivedClipOffsetMs = 0u;
	size_t iOwnerMatchCount = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& TimelineClip :
		SourcePreview.TimelineClips)
	{
		if (TimelineClip.strClipOccurrenceId == Clip.strClipOccurrenceId)
		{
			++iOwnerMatchCount;
			if (1u == iOwnerMatchCount &&
				iDerivedClipOffsetMs !=
					SourcePreview.iOwningClipTimelineOffsetMs)
			{
				m_strPreviewAnimationStatus =
					"Valtan Product cue rejected a stale global clip offset.";
				return false;
			}
		}
		iDerivedClipOffsetMs += TimelineClip.iAuthoringWallMs;
	}
	if (1u != iOwnerMatchCount ||
		iDerivedClipOffsetMs != SourcePreview.iTimelineDurationMs)
	{
		m_strPreviewAnimationStatus =
			"Valtan Product cue rejected a non-unique or incomplete owner timeline.";
		return false;
	}
	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = Clip.fPlayRate;
	Timing.fCueSourceStartSeconds =
		static_cast<f32_t>(Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds =
		static_cast<f32_t>(Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd = Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE InitialSample;
	if (!CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, 0.f, InitialSample))
	{
		m_strPreviewAnimationStatus =
			"Valtan Product cue rejected an invalid source-local preview window.";
		return false;
	}
	if (!Play_ValtanStageSequence(SourcePreview.TimelineClips))
		return false;

	Clear_ProductCuePreview();
	m_ValtanProductPreview = SourcePreview;
	Reset_ProductCueSnapshot();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration();
	m_strPreviewAnimationStatus =
		"Valtan Product cue bound to the full authoring timeline: " +
		Clip.strClipName + " | cue " + Cue.strOccurrenceId + " @ source " +
		std::to_string(Cue.iSourceStartMs) + " ms | stage+" +
		std::to_string(SourcePreview.iOwningStageTimelineOffsetMs) +
		" ms | clip+" +
		std::to_string(SourcePreview.iOwningClipTimelineOffsetMs) +
		" ms | timeline " +
		std::to_string(SourcePreview.iTimelineDurationMs) + " ms";
	return true;
}

bool_t Client::CEffect_Tool::Play_ValtanStageSequence(
	const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips)
{
	if (Clips.empty() || std::any_of(Clips.begin(), Clips.end(),
			[](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId.empty() ||
					Clip.strClipName.empty() ||
					!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f;
			}))
	{
		return false;
	}
	if (nullptr == m_pCharacterPreviewPanel ||
		(CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(
			VALTAN_ANIMATION_ASSET_NAME)))
	{
		m_strPreviewAnimationStatus =
			"Valtan model could not be staged for the ordered clip sequence.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Valtan model is not staged; the ordered clip sequence was not started.";
		return false;
	}

	std::vector<SYNCHRONIZED_ANIMATION_CLIP> Staged;
	Staged.reserve(Clips.size());
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Source : Clips)
	{
		SYNCHRONIZED_ANIMATION_CLIP Clip;
		Clip.strClipName = Source.strClipName;
		Clip.iPlayMs = Source.iPlayMs;
		Clip.fPlayRate = Source.fPlayRate;
		Clip.iSourceStartMs = Source.iSourceStartMs;
		Clip.iAuthoringWallMs = Source.iAuthoringWallMs;
		Clip.bLoop = Source.bLoop;
		Clip.bHasExplicitLoopPolicy = true;
		Staged.push_back(std::move(Clip));
	}
	for (const SYNCHRONIZED_ANIMATION_CLIP& Clip : Staged)
	{
		uint32_t iAnimationIndex = UINT32_MAX;
		f32_t fSourceStartTicks = 0.f;
		if (!Resolve_SynchronizedAnimationClipStart(
				pModel, Clip, iAnimationIndex, fSourceStartTicks))
		{
			m_strPreviewAnimationStatus =
				"Valtan clip occurrence has an invalid model/source segment: " +
				Clip.strClipName;
			return false;
		}
	}
	std::vector<SYNCHRONIZED_ANIMATION_CLIP> PreviousClips =
		m_SynchronizedAnimationClips;
	const size_t iPreviousClipIndex = m_iSynchronizedAnimationClipIndex;
	const uint64_t iPreviousLoopEpoch = m_iSynchronizedAnimationLoopEpoch;
	const uint64_t iPreviousTargetGeneration =
		m_iSynchronizedAnimationTargetGeneration;
	m_SynchronizedAnimationClips = std::move(Staged);
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	m_iSynchronizedAnimationTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	if (!Start_SynchronizedAnimationClip(0u, false))
	{
		const std::string FailedClip = Clips.front().strClipName;
		m_SynchronizedAnimationClips = std::move(PreviousClips);
		m_iSynchronizedAnimationClipIndex = iPreviousClipIndex;
		m_iSynchronizedAnimationLoopEpoch = iPreviousLoopEpoch;
		m_iSynchronizedAnimationTargetGeneration =
			iPreviousTargetGeneration;
		m_strPreviewAnimationStatus =
			"Valtan clip occurrence could not be started: " + FailedClip;
		return false;
	}
	m_strPreviewAnimationStatus = "Valtan ordered clip sequence synced to the Effect clock: " +
		Clips.front().strClipName + " (1/" +
		std::to_string(Clips.size()) + ")";
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	VALTAN_PRODUCT_PREVIEW Preview;
	Preview.Clip = Clip;
	Preview.Cue = Cue;
	VALTAN_CLIP_OCCURRENCE_VIEW TimelineClip = Clip;
	TimelineClip.iAuthoringWallMs = Cue.iStageDurationMs;
	Preview.TimelineClips = { std::move(TimelineClip) };
	Preview.iTimelineDurationMs = Cue.iStageDurationMs;
	return Try_PlayValtanSavedUnifiedEffect(
		Path, strEffectAssetId, Preview);
}

bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_PRODUCT_PREVIEW& Preview)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid || !Cache.bDrawable)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	const bool_t bTargetReady = Is_UnifiedEffectActive(Cache) ?
		Play_ValtanProductCue(Preview) :
		Try_OpenValtanAuthoredEffect(
			Path, strEffectAssetId, Preview, true);
	return bTargetReady && Try_PlayUnifiedEffect(Cache);
}

bool_t Client::CEffect_Tool::Try_SnapshotValtanWorldPreviewRoot()
{
	float4x4_t TargetRoot{};
	if (!CAnimationTargetService::Resolve_RootTransform(&TargetRoot))
	{
		m_strPreviewStatus =
			"Valtan world-owned preview could not resolve the current target root.";
		return false;
	}

	vector_t Scale{};
	vector_t Rotation{};
	vector_t Translation{};
	if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMLoadFloat4x4(&TargetRoot)) ||
		!std::isfinite(XMVectorGetX(Rotation)) ||
		!std::isfinite(XMVectorGetY(Rotation)) ||
		!std::isfinite(XMVectorGetZ(Rotation)) ||
		!std::isfinite(XMVectorGetW(Rotation)) ||
		!std::isfinite(XMVectorGetX(Translation)) ||
		!std::isfinite(XMVectorGetY(Translation)) ||
		!std::isfinite(XMVectorGetZ(Translation)))
	{
		m_strPreviewStatus =
			"Valtan world-owned preview rejected an invalid target root.";
		return false;
	}

	/* A Server combat object owns a fixed world transform. Snapshot the staged
	   Valtan pose so the local preview stays in the visible arena, but remove
	   actor presentation scale: the authored combat-object Effect owns its own
	   geometry and gameplay-footprint sizes. */
	Rotation = XMQuaternionNormalize(Rotation);
	XMStoreFloat4x4(&m_PreviewWorldRoot,
		XMMatrixRotationQuaternion(Rotation) *
		XMMatrixTranslationFromVector(Translation));
	m_vPickedWorldPosition = {
		XMVectorGetX(Translation),
		XMVectorGetY(Translation),
		XMVectorGetZ(Translation) };
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips,
	const uint32_t iWorldOwnerStageDurationMs,
	const bool_t bQueuePlayCompleteAfterLoad,
	const uint32_t iReferenceEffectStartMs)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid ||
		(bQueuePlayCompleteAfterLoad && !Cache.bDrawable))
	{
		/* Open admits a structurally valid partial authoring document so missing
		   resources can be repaired. Play alone requires drawable readiness. A
		   malformed or identity-mismatched shell never replaces Current Effect. */
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId;
	if (!bAlreadyActive && !Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path)
		{
			m_PendingDocumentLoad->ValtanReferenceClips = Clips;
			m_PendingDocumentLoad->iValtanWorldOwnerStageDurationMs =
				iWorldOwnerStageDurationMs;
			m_PendingDocumentLoad->iValtanReferenceEffectStartMs =
				iReferenceEffectStartMs;
			m_PendingDocumentLoad->ValtanClip.reset();
			m_PendingDocumentLoad->ValtanCue.reset();
			m_PendingDocumentLoad->ValtanProductPreview.reset();
			m_PendingDocumentLoad->bPlayCompleteAfterLoad =
				bQueuePlayCompleteAfterLoad;
		}
		return false;
	}
	Clear_ProductCuePreview();
	m_iValtanWorldOwnerStageDurationMs = iWorldOwnerStageDurationMs;
	m_iValtanReferenceEffectStartMs = iReferenceEffectStartMs;
	if (0u != iWorldOwnerStageDurationMs)
	{
		/* Server combat objects own a replicated world transform.  Never let
		   an old Player Root selection silently re-parent that visual. */
		m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
		m_strPreviewAnchorSlotId.clear();
		Copy_Buffer(m_PreviewAnchorBuffer.data(),
			m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
	}
	Recalculate_PreviewDuration();
	if (!Clips.empty())
	{
		if (!Play_ValtanStageSequence(Clips))
			return false;
	}
	else if (nullptr == m_pCharacterPreviewPanel ||
		(CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(
			VALTAN_ANIMATION_ASSET_NAME)))
	{
		m_strPreviewAnimationStatus =
			"Valtan model could not be staged for this saved Effect.";
		return false;
	}
	if (0u != iWorldOwnerStageDurationMs &&
		!Try_SnapshotValtanWorldPreviewRoot())
	{
		/* Editing remains available when a scene root cannot be staged. The
		   explicit Play command is fail-closed instead of silently drawing at
		   the world origin. */
		if (bQueuePlayCompleteAfterLoad)
			return false;
	}
	if (!bQueuePlayCompleteAfterLoad)
	{
		/* Open Editor stages the exact owner pose and timeline at zero without
		   consuming it. Only the explicit Play action advances animation. */
		Set_SynchronizedAnimationPaused(true);
	}
	if (bQueuePlayCompleteAfterLoad && !Try_PlayActiveUnifiedEffect())
		return false;
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const bool_t bQueuePlayCompleteAfterLoad)
{
	VALTAN_PRODUCT_PREVIEW Preview;
	Preview.Clip = Clip;
	Preview.Cue = Cue;
	VALTAN_CLIP_OCCURRENCE_VIEW TimelineClip = Clip;
	TimelineClip.iAuthoringWallMs = Cue.iStageDurationMs;
	Preview.TimelineClips = { std::move(TimelineClip) };
	Preview.iTimelineDurationMs = Cue.iStageDurationMs;
	return Try_OpenValtanAuthoredEffect(
		Path, strEffectAssetId, Preview, bQueuePlayCompleteAfterLoad);
}

bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_PRODUCT_PREVIEW& Preview,
	const bool_t bQueuePlayCompleteAfterLoad)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	if (!Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId))
	{
		m_strPreviewStatus = m_strDocumentStatus;
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path)
		{
			m_PendingDocumentLoad->ValtanClip = Preview.Clip;
			m_PendingDocumentLoad->ValtanCue = Preview.Cue;
			m_PendingDocumentLoad->ValtanProductPreview = Preview;
			m_PendingDocumentLoad->bPlayCompleteAfterLoad =
				bQueuePlayCompleteAfterLoad;
		}
		return false;
	}
	/* Do not switch the Model View target before the unsaved-document guard
	   decides whether this load will commit.  Play_ValtanProductCue owns the
	   target pattern timeline after a successful load, so Cancel preserves the
	   previous Character Product preview exactly. */
	const bool_t bAnimationReady = Play_ValtanProductCue(Preview);
	if (!bAnimationReady)
		m_strPreviewStatus = m_strPreviewAnimationStatus;
	if (bAnimationReady && !bQueuePlayCompleteAfterLoad)
	{
		/* Keep Open Editor distinct from Play Effect + Animation. */
		Set_SynchronizedAnimationPaused(true);
	}
	if (bAnimationReady && !Cache.bDrawable)
	{
		m_strPreviewStatus =
			"Opened an empty Product Effect for authoring; create its first Element before Play.";
	}
	return bAnimationReady;
}

void Client::CEffect_Tool::Request_ValtanGraphRefresh(
	const std::string& strExpectedSourceRevision)
{
	/* Latest committed Save wins if the Effect window stayed closed across
	   multiple Saves. The exact receipt remains mandatory; an unavailable
	   receipt is consumed as a typed stale-revision failure instead of silently
	   reopening whatever happens to be current later. */
	m_strPendingValtanGraphRefreshRevision = strExpectedSourceRevision;
	m_bValtanGraphRefreshRequested = true;
	m_eValtanGraphRefreshState = VALTAN_GRAPH_REFRESH_STATE::PENDING;
}

void Client::CEffect_Tool::Process_PendingValtanGraphRefresh()
{
	if (!m_bValtanGraphRefreshRequested)
		return;
	const std::string ExpectedRevision =
		std::move(m_strPendingValtanGraphRefreshRevision);
	m_strPendingValtanGraphRefreshRevision.clear();
	m_bValtanGraphRefreshRequested = false;

	/* The canonical tree is the revision gate. Do not mutate any of the related
	   All Effects indexes until its staged read has committed the requested
	   Save receipt. */
	if (!Refresh_ValtanPatternTreeForRevision(ExpectedRevision))
		return;
	const bool_t bAllEffectsReady = Refresh_AllEffects(true);
	const bool_t bDataFilesReady = Refresh_DataFiles();
	const bool_t bResourceSnapshotReady =
		Refresh_ValtanEffectResourceSnapshot();
	if (!bAllEffectsReady || !bDataFilesReady || !bResourceSnapshotReady)
	{
		m_eValtanGraphRefreshState = VALTAN_GRAPH_REFRESH_STATE::FAILED;
		m_strValtanPatternTreeStatus +=
			" | RELATED_REFRESH_FAILED: the exact Pattern tree remains admitted, but one or more Effect indexes preserved their previous snapshot.";
	}
}

bool_t Client::CEffect_Tool::Observe_ExpectedValtanSourceRevision(
	const std::string& strExpectedSourceRevision,
	const char_t* const pPhase)
{
	std::string CurrentRevision;
	std::string RevisionStatus;
	const bool_t bMatches = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Get_ValtanPublishSourceRevision(
			CurrentRevision, RevisionStatus) &&
		CurrentRevision == strExpectedSourceRevision;
	if (bMatches)
		return true;

	const std::string Diagnostic =
		"Observed " +
		(CurrentRevision.empty() ? std::string("UNAVAILABLE") :
			CurrentRevision) + ". " +
		(nullptr == m_pBalanceTool ?
			std::string("Balance revision owner is unavailable.") :
			RevisionStatus);
	Preserve_ValtanGraphForStaleRevision(
		strExpectedSourceRevision, pPhase, Diagnostic);
	return false;
}

void Client::CEffect_Tool::Preserve_ValtanGraphForStaleRevision(
	const std::string& strExpectedSourceRevision,
	const char_t* const pPhase,
	const std::string& strDiagnostic)
{
	m_bValtanPatternTreeLoadAttempted = true;
	m_bValtanPatternTreeLastRefreshSucceeded = false;
	m_bValtanPatternTreeReloadRetryPending = false;
	m_eValtanGraphRefreshState =
		VALTAN_GRAPH_REFRESH_STATE::STALE_REVISION;
	m_eValtanPatternTreeAdmission =
		(m_bValtanPatternTreeLoaded || m_bValtanProductFallbackReady) ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
	m_strValtanPatternTreeStatus =
		"STALE_REVISION: All Effects refresh preserved the previous Pattern tree at " +
		std::string(nullptr == pPhase ? "revision check" : pPhase) +
		"; expected Save receipt " +
		(strExpectedSourceRevision.empty() ? std::string("NONE") :
			strExpectedSourceRevision) + ". " + strDiagnostic;
}

bool_t Client::CEffect_Tool::Refresh_ValtanPatternTreeForRevision(
	const std::string& strExpectedSourceRevision)
{
	if (!Observe_ExpectedValtanSourceRevision(
			strExpectedSourceRevision, "before parse"))
	{
		return false;
	}
	m_strActiveValtanGraphRefreshRevision = strExpectedSourceRevision;
	const bool_t bRefreshed = Refresh_ValtanPatternTree();
	m_strActiveValtanGraphRefreshRevision.clear();
	if (!bRefreshed)
	{
		/* A guarded Save receipt must never degrade into the ordinary unpinned
		   automatic retry, which could later commit a different generation. */
		m_bValtanPatternTreeReloadRetryPending = false;
		if (VALTAN_GRAPH_REFRESH_STATE::STALE_REVISION !=
				m_eValtanGraphRefreshState)
		{
			m_eValtanGraphRefreshState = VALTAN_GRAPH_REFRESH_STATE::FAILED;
		}
		return false;
	}
	m_strCommittedValtanGraphRevision = strExpectedSourceRevision;
	m_eValtanGraphRefreshState = VALTAN_GRAPH_REFRESH_STATE::ADMITTED;
	m_strValtanPatternTreeStatus +=
		" | EXACT_SAVE_REVISION " + strExpectedSourceRevision + " admitted.";
	return true;
}

bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()
{
	/* parse -> validate -> stage -> commit. A failed reload keeps whatever the
	   window is already showing so a transient read error never empties it. */
	Initialize_CatalogMetadataView();
	m_bValtanPatternTreeLoadAttempted = true;
	m_bValtanPatternTreeLastRefreshSucceeded = false;
	m_bValtanPatternTreeReloadRetryPending = false;
	/* Revoke command authority before staging. A previously committed strict
	   tree or Product fallback remains one immutable display snapshot. */
	m_eValtanPatternTreeAdmission =
		Can_DisplayValtanView(m_eValtanPatternTreeAdmission) ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::UNLOADED;
	VALTAN_PATTERN_TREE_VIEW Staged;
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	if (!CanonicalAdmission.Acquire(Diagnostic))
	{
		if (Diagnostic.Is_AutomaticRetryable())
			Schedule_ValtanPatternTreeReloadRetry();
		const std::string RetryStatus = m_bValtanPatternTreeReloadRetryPending ?
			" Automatic retry is scheduled." : std::string{};
		m_eValtanPatternTreeAdmission =
			(m_bValtanPatternTreeLoaded || m_bValtanProductFallbackReady) ?
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
				VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanPatternTreeStatus = m_bValtanPatternTreeLoaded ?
			("Valtan tree reload preserved the previous tree; canonical Product read admission failed: " +
				Diagnostic.strStatus + RetryStatus) :
			(m_bValtanProductFallbackReady ?
				"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; admission failed, so no unpinned Product files were reopened: " + Diagnostic.strStatus + RetryStatus :
				"Valtan canonical Product read admission failed; no unpinned fallback read was attempted: " + Diagnostic.strStatus + RetryStatus);
		return false;
	}
	if (!CValtanPatternTree::Load_WhileAdmitted(
			CanonicalAdmission, Staged, Diagnostic))
	{
		std::string Status = Diagnostic.strStatus;
		if (Diagnostic.Requires_ProductProjection())
		{
			Status = "REPROJECTION_REQUIRED: the generated Product is older than "
				"the joined Valtan source; pattern browsing falls back to the pinned "
				"Product until Save/Project succeeds. Recovery command: " +
				std::string{
					VALTAN_CANONICAL_READ_DIAGNOSTIC::PRODUCT_PROJECTION_COMMAND } +
				" | " + Status;
			if (!Diagnostic.strRejectedPatternId.empty())
			{
				Status += " | quarantined source owner=" +
					Diagnostic.strRejectedPatternId;
				if (!Diagnostic.strRejectedStageId.empty())
					Status += "/" + Diagnostic.strRejectedStageId;
			}
		}
		if (m_bValtanPatternTreeLoaded)
		{
			m_eValtanPatternTreeAdmission =
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
			m_strValtanPatternTreeStatus =
				"Valtan tree reload preserved the previous tree: " + Status;
		}
		else
		{
			(void)Stage_ValtanProductFallback(CanonicalAdmission, Status);
		}
		return false;
	}
	VALTAN_TOOL_AUDITION_INVENTORY StagedAuditionInventory;
	std::string InventoryError;
	if (!CValtanPatternTree::Build_PlayablePatternInventory(
			Staged, StagedAuditionInventory, InventoryError))
	{
		if (m_bValtanPatternTreeLoaded)
		{
			m_eValtanPatternTreeAdmission =
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
			m_strValtanPatternTreeStatus =
				"Valtan tree reload preserved the previous tree: " +
				InventoryError;
		}
		else
		{
			(void)Stage_ValtanProductFallback(
				CanonicalAdmission, InventoryError);
		}
		return false;
	}
	VALTAN_CANONICAL_READ_DIAGNOSTIC CurrentDiagnostic;
	if (!CanonicalAdmission.Validate_StillCurrent(CurrentDiagnostic))
	{
		if (CurrentDiagnostic.Is_AutomaticRetryable())
			Schedule_ValtanPatternTreeReloadRetry();
		m_eValtanPatternTreeAdmission =
			(m_bValtanPatternTreeLoaded || m_bValtanProductFallbackReady) ?
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
				VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanPatternTreeStatus = m_bValtanPatternTreeLoaded ?
			"Valtan tree generation changed before commit; previous tree was preserved: " + CurrentDiagnostic.strStatus :
			(m_bValtanProductFallbackReady ?
				"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; strict tree generation changed before commit: " + CurrentDiagnostic.strStatus :
				"Valtan tree generation changed before commit: " + CurrentDiagnostic.strStatus);
		return false;
	}
	std::string ExactRevisionStatus;
	if (!m_strActiveValtanGraphRefreshRevision.empty() &&
		(nullptr == m_pBalanceTool ||
		 !m_pBalanceTool->Verify_ValtanCanonicalSourceRevision_WhileAdmitted(
			CanonicalAdmission, m_strActiveValtanGraphRefreshRevision,
			ExactRevisionStatus)))
	{
		/* Staged and validated locals fall out of scope. The previously committed
		   tree, inventory, selection and caches remain byte-for-byte untouched. */
		Preserve_ValtanGraphForStaleRevision(
			m_strActiveValtanGraphRefreshRevision, "before commit",
			nullptr == m_pBalanceTool ?
				"Balance revision owner is unavailable." : ExactRevisionStatus);
		return false;
	}
	const std::string Status = Diagnostic.strStatus;
	m_ValtanPatternTree = std::move(Staged);
	m_ValtanToolAuditionInventory = std::move(StagedAuditionInventory);
	m_bValtanPatternTreeLoaded = true;
	m_eValtanPatternTreeAdmission = VALTAN_VIEW_ADMISSION::ADMITTED;
	m_ValtanProductFallbackEncounter.Clear();
	m_bValtanProductFallbackReady = false;
	m_strValtanPatternTreeStatus = Status;
	if (!Refresh_ValtanPatternAuthoringEffects())
	{
		/* Draft attachment ownership is a Tool-only sidecar, not a Product
		   admission dependency.  Keep the valid Server Pattern inventory visible
		   and disable only the unavailable Draft authoring surface. */
		m_strValtanPatternTreeStatus = Status +
			" | Draft Effect index unavailable: " +
			m_strValtanPatternAuthoringEffectsStatus;
	}
	/* Explicit Refresh is also the retry boundary for a repaired authored
	   document that was previously observed as invalid or non-drawable. */
	m_ValtanUnifiedEffectCaches.clear();
	if (!m_strSelectedValtanPatternId.empty())
	{
		const VALTAN_PATTERN_VIEW* pSelected = Find_ValtanPattern(
			m_strSelectedValtanPatternId);
		if (nullptr == pSelected || !Is_ValtanAllEffectsPattern(*pSelected))
			m_strSelectedValtanPatternId.clear();
	}
	m_bValtanPatternTreeLastRefreshSucceeded = true;
	return true;
}

void Client::CEffect_Tool::Schedule_ValtanPatternTreeReloadRetry()
{
	m_bValtanPatternTreeReloadRetryPending = true;
	m_dNextValtanPatternTreeReloadRetrySeconds =
		ImGui::GetTime() + VALTAN_PATTERN_TREE_RELOAD_RETRY_SECONDS;
}

bool_t Client::CEffect_Tool::Stage_ValtanProductFallback(
	const CValtanCanonicalProductReadAdmission& Admission,
	const std::string& strStrictFailure)
{
	CEncounterPatternReference Staged;
	std::string ProductStatus;
	if (!Staged.Load(
			CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json"),
			ProductStatus))
	{
		m_eValtanPatternTreeAdmission = m_bValtanProductFallbackReady ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanPatternTreeStatus = m_bValtanProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; strict join and fallback refresh failed. Strict failure: " +
				strStrictFailure + " | Product failure: " + ProductStatus :
			"Valtan strict join failed and generated Product fallback could not load: " +
				strStrictFailure + " | " + ProductStatus;
		return false;
	}
	std::string CurrentStatus;
	if (!Admission.Validate_StillCurrent(CurrentStatus))
	{
		Schedule_ValtanPatternTreeReloadRetry();
		m_eValtanPatternTreeAdmission = m_bValtanProductFallbackReady ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		m_strValtanPatternTreeStatus = m_bValtanProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; Product generation changed before fallback commit: " + CurrentStatus :
			"Generated Product fallback generation changed before commit: " +
				CurrentStatus;
		return false;
	}
	m_ValtanProductFallbackEncounter = std::move(Staged);
	m_bValtanProductFallbackReady = true;
	m_eValtanPatternTreeAdmission =
		VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
	m_strValtanPatternTreeStatus =
		"READ-ONLY PRODUCT FALLBACK: " +
		std::to_string(
			m_ValtanProductFallbackEncounter.Get_Patterns().size()) +
		" generated Product patterns remain visible. Product-linked Pattern editing and Server playback stay blocked. Strict failure: " +
		strStrictFailure;
	return true;
}

bool_t Client::CEffect_Tool::Refresh_ValtanPatternAuthoringEffects()
{
	m_bValtanPatternAuthoringEffectsLastRefreshSucceeded = false;
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT Staged;
	std::string Baseline;
	std::string Status;
	if (!CValtanPatternAuthoringEffectDocument::Load(
			Staged, Baseline, Status))
	{
		m_strValtanPatternAuthoringEffectsStatus =
			m_bValtanPatternAuthoringEffectsLoaded ?
				("Valtan pattern Effect binding reload preserved the previous view: " +
				 Status) : Status;
		return false;
	}

	for (const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding :
		Staged.Bindings)
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPattern(
			Binding.strPatternId);
		const bool_t bIndependentCollision = std::any_of(
			m_ValtanPatternTree.IndependentEffects.begin(),
			m_ValtanPatternTree.IndependentEffects.end(),
			[&Binding](const VALTAN_INDEPENDENT_EFFECT_VIEW& Independent)
			{
				return Independent.strEffectAssetId == Binding.strEffectAssetId;
			});
		const std::filesystem::path AuthoredPath =
			CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(
				Binding);
		if (nullptr == pPattern || !Is_ValtanAllEffectsPattern(*pPattern) ||
			bIndependentCollision || AuthoredPath.empty() ||
			Binding.strState != "DRAFT_ATTACHED" ||
			Binding.strAuthoringPath !=
				CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
					Binding.strEffectAssetId) ||
			CEffectCatalog::Contains(Binding.strEffectAssetId))
		{
			m_strValtanPatternAuthoringEffectsStatus =
				"Valtan Draft Effect binding is outside the playable inventory, collides with Product ownership, or has no exact Draft path: " +
				Binding.strPatternId + " / " + Binding.strEffectAssetId;
			return false;
		}

		EFFECT_DOCUMENT_DESC DraftDocument;
		std::string DraftError;
		if (!CEffectDocumentCodec::Load(
				AuthoredPath, DraftDocument, DraftError) ||
			DraftDocument.strEffectAssetId != Binding.strEffectAssetId)
		{
			m_strValtanPatternAuthoringEffectsStatus =
				"Valtan Draft Effect file does not match its exact sidecar ID/path: " +
				Binding.strPatternId + " / " + Binding.strEffectAssetId +
				(DraftError.empty() ? std::string{} : " | " + DraftError);
			return false;
		}
	}

	m_ValtanPatternAuthoringEffects = std::move(Staged);
	m_strValtanPatternAuthoringEffectsBaseline = std::move(Baseline);
	m_strValtanPatternAuthoringEffectsStatus =
		"Loaded " + std::to_string(Staged.Bindings.size()) +
		" exact Draft Effect binding(s); Product Catalog ownership is not required.";
	m_bValtanPatternAuthoringEffectsLoaded = true;
	m_bValtanPatternAuthoringEffectsLastRefreshSucceeded = true;
	return true;
}

const Client::VALTAN_PATTERN_AUTHORING_EFFECT_BINDING*
Client::CEffect_Tool::Find_ValtanPatternAuthoringEffect(
	const std::string& strPatternId) const
{
	const auto Found = std::find_if(
		m_ValtanPatternAuthoringEffects.Bindings.begin(),
		m_ValtanPatternAuthoringEffects.Bindings.end(),
		[&strPatternId](
			const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Binding)
		{
			return Binding.strPatternId == strPatternId;
		});
	return Found == m_ValtanPatternAuthoringEffects.Bindings.end() ?
		nullptr : &*Found;
}

const Client::VALTAN_PATTERN_VIEW* Client::CEffect_Tool::Find_ValtanPattern(
	const std::string& strPatternId) const
{
	for (const std::vector<VALTAN_PATTERN_VIEW>* pGroup :
		{ &m_ValtanPatternTree.Gimmicks, &m_ValtanPatternTree.Rotation })
	{
		const auto Found = std::find_if(
			pGroup->begin(), pGroup->end(),
			[&strPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strPatternId;
			});
		if (Found != pGroup->end())
			return &*Found;
	}
	return nullptr;
}

bool_t Client::CEffect_Tool::Is_ValtanAllEffectsPattern(
	const VALTAN_PATTERN_VIEW& Pattern) const
{
	return m_ValtanToolAuditionInventory.Contains(Pattern.strPatternId);
}

std::string Client::CEffect_Tool::Build_ValtanPatternAggregateEffectAssetId(
	const VALTAN_PATTERN_VIEW& Pattern) const
{
	return Pattern.strActionId.empty() ?
		std::string{} : "effect." + Pattern.strActionId;
}

bool_t Client::CEffect_Tool::Try_OpenExistingValtanPatternEffect(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	const auto Reject = [this](std::string Reason)
	{
		m_strValtanPatternEffectStatus =
			"Open Existing Effect preserved the current Effect: " +
			std::move(Reason);
		return false;
	};
	if (m_ValtanPatternProductUnlinkOperation.has_value())
		return Reject("wait for the current Product unlink transaction.");
	const VALTAN_PATTERN_VIEW* pCurrent =
		Find_ValtanPattern(Pattern.strPatternId);
	if (nullptr == pCurrent || !Is_ValtanAllEffectsPattern(*pCurrent) ||
		m_strSelectedValtanPatternId != Pattern.strPatternId)
	{
		return Reject("select one current Pattern from the All Effects inventory.");
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_BINDING Aggregate;
	Aggregate.strPatternId = pCurrent->strPatternId;
	Aggregate.strEffectAssetId = Build_ValtanPatternAggregateEffectAssetId(*pCurrent);
	Aggregate.strAuthoringPath =
		CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
			Aggregate.strEffectAssetId);
	const std::filesystem::path Path =
		CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(Aggregate);
	if (Path.empty())
		return Reject("the Pattern has no safe canonical authored Effect path.");
	const auto Indexed =
		m_DirectAuthoredEditableEntries.find(Aggregate.strEffectAssetId);
	if (Indexed != m_DirectAuthoredEditableEntries.end() &&
		Indexed->second.Path.lexically_normal() != Path.lexically_normal())
	{
		return Reject("the registered source path differs from the Pattern aggregate; use its exact Product row.");
	}
	EFFECT_DOCUMENT_DESC ExistingDocument;
	std::string Error;
	if (!CEffectDocumentCodec::Load(Path, ExistingDocument, Error))
		return Reject(std::move(Error));
	if (ExistingDocument.strEffectAssetId != Aggregate.strEffectAssetId)
		return Reject("the existing file does not contain the exact Pattern aggregate Effect ID.");

	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST Request;
	size_t iProductCueCount = 0u;
	for (const VALTAN_STAGE_VIEW& Stage : pCurrent->Stages)
	{
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		{
			if (Cue.strEffectAssetId != Aggregate.strEffectAssetId)
				continue;
			++iProductCueCount;
			Request.strPatternId = Aggregate.strPatternId;
			Request.strStageId = Stage.strStageId;
			Request.strCueOccurrenceId = Cue.strOccurrenceId;
			Request.strEffectAssetId = Aggregate.strEffectAssetId;
		}
	}
	if (iProductCueCount > 1u)
	{
		return Reject("this Effect has multiple Product occurrences; select its exact occurrence before opening.");
	}
	if (1u == iProductCueCount)
	{
		/* The existing Product opener refreshes and re-resolves the complete stable
		   tuple. Do not retain any Pattern-tree pointer across this call. */
		return Open_ValtanProductEffect(Request);
	}

	/* Unlink removes only the cue. The preserved authored file remains the one
	   aggregate; opening it must not recreate Product or DRAFT_ATTACHED ownership. */
	if (!Try_OpenValtanStandaloneEffect(Path, Aggregate.strEffectAssetId))
	{
		const bool_t bPending = m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path &&
			m_PendingDocumentLoad->strSelectionId == Aggregate.strEffectAssetId;
		if (bPending)
			m_PendingDocumentLoad->strValtanPatternId = Aggregate.strPatternId;
		m_strValtanPatternEffectStatus = bPending ?
			"Open Existing Effect is waiting for Save, Discard, or Cancel; the current Effect is unchanged." :
			("Open Existing Effect failed: " + m_strPreviewStatus);
		return false;
	}
	m_SelectedValtanPatternEffect.reset();
	m_strValtanPatternEffectStatus =
		"Opened the existing aggregate Effect for authoring only: " +
		Aggregate.strEffectAssetId +
		". This Pattern remains unlinked; no Product cue, catalog row, or draft ownership was created.";
	return true;
}

bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::string& strSearch) const
{
	if (strSearch.empty())
		return true;
	if (Contains_NoCase(Pattern.strPatternId, strSearch) ||
		Contains_NoCase(Pattern.strDisplayName, strSearch) ||
		Contains_NoCase(Pattern.strActionId, strSearch))
	{
		return true;
	}
	const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING* pBinding =
		Find_ValtanPatternAuthoringEffect(Pattern.strPatternId);
	if (nullptr != pBinding)
	{
		if (Contains_NoCase(pBinding->strEffectAssetId, strSearch) ||
			Contains_NoCase(pBinding->strAuthoringPath, strSearch))
		{
			return true;
		}
	}
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		if (Contains_NoCase(Stage.strStageId, strSearch) ||
			Contains_NoCase(Stage.strActionId, strSearch) ||
			Contains_NoCase(Stage.strHitShape, strSearch))
		{
			return true;
		}
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			Stage.ClipOccurrences)
		{
			if (Contains_NoCase(Clip.strClipOccurrenceId, strSearch) ||
				Contains_NoCase(Clip.strClipName, strSearch) ||
				Contains_NoCase(Clip.strMappingBasis, strSearch))
			{
				return true;
			}
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				Clip.ProductCues)
			{
				if (Contains_NoCase(Cue.strBindingId, strSearch) ||
					Contains_NoCase(Cue.strOccurrenceId, strSearch) ||
					Contains_NoCase(Cue.strEffectAssetId, strSearch) ||
					Contains_NoCase(Cue.strV1EffectAssetId, strSearch))
				{
					return true;
				}
			}
		}
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
			Stage.ProductCues)
		{
			if (Contains_NoCase(Cue.strBindingId, strSearch) ||
				Contains_NoCase(Cue.strOccurrenceId, strSearch) ||
				Contains_NoCase(Cue.strEffectAssetId, strSearch) ||
				Contains_NoCase(Cue.strV1EffectAssetId, strSearch))
			{
				return true;
			}
		}
		for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
		{
			if (Contains_NoCase(Effect.strEffectAssetId, strSearch))
				return true;
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
			Stage.CombatObjectEffects)
		{
			if (Contains_NoCase(Effect.strCombatObjectArchetypeId, strSearch) ||
				Contains_NoCase(Effect.strClientVisualId, strSearch) ||
				Contains_NoCase(Effect.strEffectAssetId, strSearch))
			{
				return true;
			}
		}
	}
	return false;
}

bool_t Client::CEffect_Tool::Matches_ValtanIndependentEffectSearch(
	const VALTAN_INDEPENDENT_EFFECT_VIEW& Effect,
	const std::string& strSearch) const
{
	return strSearch.empty() ||
		Contains_NoCase(Effect.strIndependentEffectId, strSearch) ||
		Contains_NoCase(Effect.strDisplayName, strSearch) ||
		Contains_NoCase(Effect.strEffectAssetId, strSearch) ||
		Contains_NoCase(Effect.strOwnership, strSearch) ||
		Contains_NoCase(Effect.strOwnerPatternId, strSearch) ||
		Contains_NoCase(Effect.strOwnerStageId, strSearch) ||
		Contains_NoCase(Effect.strTriggerPolicy, strSearch) ||
		Contains_NoCase(Effect.strCombatObjectArchetypeId, strSearch) ||
		Contains_NoCase(Effect.strClientVisualId, strSearch) ||
		Contains_NoCase(Effect.strEffectCueBindingId, strSearch);
}

bool_t Client::CEffect_Tool::Build_ValtanAuthoringTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& OutClips,
	uint32_t& iOutDurationMs,
	std::string& strOutError) const
{
	OutClips.clear();
	iOutDurationMs = 0u;
	strOutError.clear();
	if (!Pattern.bAuthoringMasterManaged || Pattern.Stages.empty())
	{
		strOutError =
			"This pattern is not managed by the joined Valtan gameplay/presentation sources.";
		return false;
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagePath;
	if (!CValtanPatternTree::Build_PreviewStagePath(
			Pattern, ePath, StagePath, strOutError))
	{
		return false;
	}

	uint64_t iTimelineDurationMs = 0u;
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> StagedClips;
	for (const VALTAN_STAGE_VIEW* pStage : StagePath)
	{
		if (nullptr == pStage || pStage->ClipOccurrences.empty())
		{
			strOutError =
				"Authoring path has no ordered animation occurrence.";
			return false;
		}
		const size_t iPlayableOccurrenceCount =
			pStage->iAuthoringRepeatCount > 1u ?
				static_cast<size_t>(pStage->iAuthoringRepeatCount) :
				pStage->ClipOccurrences.size();
		if (0u == iPlayableOccurrenceCount ||
			iPlayableOccurrenceCount != pStage->ClipOccurrences.size())
		{
			strOutError =
				"Authoring repeatCount does not own exactly its explicit occurrences for " +
				pStage->strActionId + ".";
			return false;
		}
		uint64_t iStageAnimationWallMs = 0u;
		for (size_t iClip = 0u; iClip < iPlayableOccurrenceCount; ++iClip)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				pStage->ClipOccurrences[iClip];
			if (0u == Clip.iAuthoringWallMs)
			{
				strOutError =
					"Joined presentation source did not derive a wall budget for " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}
			iStageAnimationWallMs += Clip.iAuthoringWallMs;
			StagedClips.push_back(Clip);
		}
		if (iStageAnimationWallMs != pStage->iDurationMs)
		{
			strOutError = "Authoring animation does not fill Server stage " +
				pStage->strStageId + ".";
			return false;
		}
		iTimelineDurationMs += pStage->iDurationMs;
		if (iTimelineDurationMs > static_cast<uint64_t>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			strOutError = "Authoring timeline duration overflowed.";
			return false;
		}
	}
	if (StagedClips.empty() || 0u == iTimelineDurationMs)
	{
		strOutError = "Authoring timeline is empty.";
		return false;
	}
	OutClips = std::move(StagedClips);
	iOutDurationMs = static_cast<uint32_t>(iTimelineDurationMs);
	return true;
}

bool_t Client::CEffect_Tool::Build_ValtanProductPreview(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	VALTAN_PRODUCT_PREVIEW& OutPreview,
	std::string& strOutError) const
{
	OutPreview = {};
	strOutError.clear();
	if (!Pattern.bAuthoringMasterManaged ||
		Cue.strPatternId != Pattern.strPatternId ||
		Cue.strClipOccurrenceId != Clip.strClipOccurrenceId ||
		Cue.strStageId.empty() || Cue.strActionId.empty() ||
		Cue.strEffectAssetId.empty() || 0u == Cue.iStageDurationMs)
	{
		strOutError =
			"Product cue no longer owns this managed pattern occurrence.";
		return false;
	}

	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> TimelineClips;
	uint32_t iTimelineDurationMs = 0u;
	if (!Build_ValtanAuthoringTimeline(
			Pattern, ePath, TimelineClips, iTimelineDurationMs, strOutError))
	{
		return false;
	}

	uint64_t iClipTimelineOffsetMs = 0u;
	size_t iTimelineOwnerCount = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& TimelineClip : TimelineClips)
	{
		if (TimelineClip.strClipOccurrenceId == Clip.strClipOccurrenceId)
		{
			++iTimelineOwnerCount;
			if (1u == iTimelineOwnerCount)
			{
				/* Preserve the offset before this occurrence; every master
				   occurrence carries its exact wall budget. */
				OutPreview.iOwningClipTimelineOffsetMs =
					static_cast<uint32_t>(iClipTimelineOffsetMs);
			}
		}
		iClipTimelineOffsetMs += TimelineClip.iAuthoringWallMs;
	}
	if (1u != iTimelineOwnerCount ||
		iClipTimelineOffsetMs != iTimelineDurationMs)
	{
		strOutError =
			"Chosen pattern branch does not contain one exact cue owner occurrence.";
		return false;
	}

	const auto OwnerStage = std::find_if(
		Pattern.Stages.begin(), Pattern.Stages.end(),
		[&Cue](const VALTAN_STAGE_VIEW& Stage)
		{
			return Stage.strStageId == Cue.strStageId &&
				Stage.strActionId == Cue.strActionId;
		});
	if (OwnerStage == Pattern.Stages.end() ||
		OwnerStage->iDurationMs != Cue.iStageDurationMs)
	{
		strOutError =
			"Product cue stage identity or wall window drifted from the master.";
		return false;
	}

	uint64_t iClipOffsetWithinStageMs = 0u;
	size_t iStageOwnerCount = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& StageClip :
		OwnerStage->ClipOccurrences)
	{
		if (StageClip.strClipOccurrenceId == Clip.strClipOccurrenceId)
		{
			++iStageOwnerCount;
			continue;
		}
		if (0u == iStageOwnerCount)
			iClipOffsetWithinStageMs += StageClip.iAuthoringWallMs;
	}
	if (1u != iStageOwnerCount ||
		iClipOffsetWithinStageMs >
			OutPreview.iOwningClipTimelineOffsetMs)
	{
		strOutError =
			"Product cue clip occurrence left its declared semantic stage.";
		return false;
	}
	OutPreview.iOwningStageTimelineOffsetMs =
		OutPreview.iOwningClipTimelineOffsetMs -
		static_cast<uint32_t>(iClipOffsetWithinStageMs);
	if (static_cast<uint64_t>(OutPreview.iOwningStageTimelineOffsetMs) +
		OwnerStage->iDurationMs > iTimelineDurationMs)
	{
		strOutError =
			"Product cue semantic stage exceeds the chosen pattern branch.";
		return false;
	}

	OutPreview.Clip = Clip;
	OutPreview.Cue = Cue;
	OutPreview.TimelineClips = std::move(TimelineClips);
	OutPreview.iTimelineDurationMs = iTimelineDurationMs;
	return true;
}

bool_t Client::CEffect_Tool::Play_ValtanAuthoringTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> Clips;
	uint32_t iDurationMs = 0u;
	std::string Error;
	if (!Build_ValtanAuthoringTimeline(
			Pattern, ePath, Clips, iDurationMs, Error))
	{
		m_strPreviewAnimationStatus = std::move(Error);
		return false;
	}

	/* One EffectObject can represent one exact cue occurrence.  When the
	   Current Effect owns that occurrence, bind it to the same complete branch
	   that the animation author is scrubbing.  Multiple candidates are not
	   silently collapsed into a fake "play all effects" preview. */
	struct ACTIVE_CUE_OWNER final
	{
		const VALTAN_CLIP_OCCURRENCE_VIEW* pClip = nullptr;
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCue = nullptr;
	};
	std::vector<ACTIVE_CUE_OWNER> ActiveCueOwners;
	const bool_t bHasActiveAuthoredEffect = m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource;
	if (bHasActiveAuthoredEffect)
	{
		const std::string& strActiveEffectAssetId =
			m_ActiveDocument->strEffectAssetId;
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Clips)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Clip.ProductCues)
			{
				if (Cue.strEffectAssetId == strActiveEffectAssetId ||
					Cue.strV1EffectAssetId == strActiveEffectAssetId)
				{
					ActiveCueOwners.push_back({ &Clip, &Cue });
				}
			}
		}
	}

	if (1u == ActiveCueOwners.size())
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue =
			*ActiveCueOwners.front().pCue;
		PlaybackCue.strEffectAssetId = m_ActiveDocument->strEffectAssetId;
		VALTAN_PRODUCT_PREVIEW Preview;
		if (!Build_ValtanProductPreview(Pattern, ePath,
				*ActiveCueOwners.front().pClip, PlaybackCue, Preview, Error))
		{
			m_strPreviewAnimationStatus =
				"Active authored Effect cue could not join the full timeline: " +
				Error;
			return false;
		}
		if (!Play_ValtanProductCue(Preview))
			return false;
		m_bPreviewPlaying = true;
		Set_SynchronizedAnimationPaused(false);
		const bool_t bEffectStarted = Try_PlayActiveUnifiedEffect();
		m_strPreviewAnimationStatus =
			"Valtan joined authoring timeline + active exact cue: " +
			Pattern.strPatternId + " | " + std::to_string(Clips.size()) +
			" occurrences | " + std::to_string(iDurationMs) + " ms" +
			(bEffectStarted ? std::string{} :
				" | Effect preview unavailable; animation timeline remains bound");
		return true;
	}

	/* No exact active cue means animation-only by design.  Clear an old cue
	   only in this explicit branch, hide its EffectObject, and report why. */
	Clear_ProductCuePreview();
	Hide_WorldPreview();
	if (!Play_ValtanStageSequence(Clips))
		return false;
	m_iValtanWorldOwnerStageDurationMs = iDurationMs;
	m_fPreviewTimeSeconds = 0.f;
	m_fPreviewDurationSeconds = static_cast<f32_t>(iDurationMs) * 0.001f;
	m_bPreviewPlaying = true;
	Set_SynchronizedAnimationPaused(false);
	m_strPreviewAnimationStatus = "Valtan joined authoring animation-only timeline: " +
		Pattern.strPatternId + " | " + std::to_string(Clips.size()) +
		" occurrences | " + std::to_string(iDurationMs) + " ms | " +
		(ActiveCueOwners.empty() ?
			"no exact cue for the current authored Effect" :
			"current authored Effect maps to multiple cues; select one saved occurrence");
	return true;
}

void Client::CEffect_Tool::Render_ValtanStageRow(
	const VALTAN_STAGE_VIEW& Stage)
{
	/* One line states what the Server does in this window, because that is the
	   window the Effect has to fill. The numbers are read, never written. */
	std::string Shape = Stage.strHitShape.empty() ? "NONE" : Stage.strHitShape;
	if (Stage.Has_HitShape())
	{
		char_t Detail[128]{};
		if ("CIRCLE" == Stage.strHitShape)
			sprintf_s(Detail, " r=%.1f", Stage.fHitOuterRadius);
		else if ("RING" == Stage.strHitShape)
			sprintf_s(Detail, " in=%.1f out=%.1f",
				Stage.fHitInnerRadius, Stage.fHitOuterRadius);
		else if ("CONE" == Stage.strHitShape)
			sprintf_s(Detail, " %.0fdeg L=%.1f",
				Stage.fHitAngleDegrees, Stage.fHitLength);
		else if ("BOX" == Stage.strHitShape || "CROSS" == Stage.strHitShape ||
			"SIX_DIRECTIONS" == Stage.strHitShape)
			sprintf_s(Detail, " L=%.1f W=%.1f",
				Stage.fHitLength, Stage.fHitHalfWidth);
		Shape += Detail;
		if (Stage.iHitCount > 1u)
		{
			Shape += " x" + std::to_string(Stage.iHitCount) + " @";
			if (!Stage.HitOffsetsMs.empty())
			{
				Shape += "[";
				for (size_t iOffset = 0u;
					iOffset < Stage.HitOffsetsMs.size(); ++iOffset)
				{
					if (iOffset > 0u)
						Shape += ",";
					Shape += std::to_string(Stage.HitOffsetsMs[iOffset]);
				}
				Shape += "]ms";
			}
			else
			{
				Shape += std::to_string(Stage.iHitDelayMs) + "+k*" +
					std::to_string(Stage.iHitIntervalMs) + "ms";
			}
		}
	}

	ImGui::PushID(Stage.strActionId.c_str());
	const std::string StageLabel = Stage.strStageId +
		(Stage.strSequenceRole.empty() ? std::string{} :
			(" / " + Stage.strSequenceRole)) + " | " +
		Stage.strStageKind + " | " + std::to_string(Stage.iDurationMs) +
		" ms | " + Shape + " | " +
		std::to_string(Stage.ClipOccurrences.size()) + " clips" +
		(1u < Stage.iAuthoringRepeatCount ?
			(" | repeat " + std::to_string(Stage.iAuthoringRepeatCount)) :
			std::string{}) + " | " +
		std::to_string(Stage.ProductCues.size()) + " cues | " +
		std::to_string(Stage.CombatObjectEffects.size()) + " moving fx";
	const bool_t bStageOpen = ImGui::TreeNodeEx(StageLabel.c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow);
	if (bStageOpen)
	{
		ImGui::TextDisabled("action %s", Stage.strActionId.c_str());
		if (!Stage.ClipOccurrences.empty())
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Replay Sequence"))
				Play_ValtanStageSequence(Stage.ClipOccurrences);
		}
		if (!Stage.strServerDamageProfileId.empty())
		{
			ImGui::TextDisabled("Server damage %s",
				Stage.strServerDamageProfileId.c_str());
		}

		for (size_t iClip = 0u; iClip < Stage.ClipOccurrences.size(); ++iClip)
		{
			Render_ValtanClipOccurrence(
				Stage, Stage.ClipOccurrences[iClip], iClip + 1u);
		}
		if (Stage.ClipOccurrences.empty())
			ImGui::TextDisabled("(no ordered animation clip occurrence)");

		if (!Stage.CombatObjectEffects.empty())
		{
			ImGui::SeparatorText("World-root Effect Occurrences");
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
				Stage.CombatObjectEffects)
			{
				ImGui::PushID(Effect.strCombatObjectArchetypeId.c_str());
				ImGui::TextDisabled("%s | %s | %s x%u | %s",
					Effect.strCombatObjectArchetypeId.c_str(),
					Effect.strClientVisualId.c_str(),
					Effect.strTrigger.c_str(), Effect.iSpawnValue,
					Effect.strEffectAssetId.c_str());
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanClipOccurrence(
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const size_t iClipOrdinal)
{
	ImGui::PushID(Clip.strClipOccurrenceId.c_str());
	std::string Label = "Clip " + std::to_string(iClipOrdinal) + " | " +
		Clip.strClipName + " | " + Clip.strClipOccurrenceId + " | " +
		std::to_string(Clip.ProductCues.size()) + " cues";
	if (Clip.iSourceStartMs > 0u || Clip.iPlayMs > 0u ||
		std::abs(Clip.fPlayRate - 1.f) > 0.0001f || Clip.bLoop)
	{
		Label += " | src+" + std::to_string(Clip.iSourceStartMs) +
			" play=" + (0u == Clip.iPlayMs ? std::string("natural") :
				std::to_string(Clip.iPlayMs) + "ms") +
			" rate=" + std::to_string(Clip.fPlayRate) +
			(Clip.bLoop ? " loop" : "");
	}
	const bool_t bOpen = ImGui::TreeNodeEx(
		Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
	ImGui::SameLine();
	if (ImGui::SmallButton("Replay"))
		Play_ValtanClipOccurrence(Clip);
	if (bOpen)
	{
		ImGui::TextDisabled("mapping %s | stage wall %u ms",
			Clip.strMappingBasis.empty() ? "(unspecified)" :
				Clip.strMappingBasis.c_str(),
			Stage.iDurationMs);
		for (size_t iCue = 0u; iCue < Clip.ProductCues.size(); ++iCue)
		{
			Render_ValtanProductCue(
				Clip, Clip.ProductCues[iCue], iCue + 1u);
		}
		if (Clip.ProductCues.empty())
			ImGui::TextDisabled("(no Product cue occurrence)");
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanProductCue(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const size_t iCueOrdinal)
{
	(void)Clip;
	ImGui::PushID(Cue.strOccurrenceId.c_str());
	const std::string SourceWindow = Cue.bHasSourceEnd ?
		(std::to_string(Cue.iSourceStartMs) + "-" +
		 std::to_string(Cue.iSourceEndMs) + " ms") :
		(std::to_string(Cue.iSourceStartMs) + "-natural");
	const std::string Label = "Cue " + std::to_string(iCueOrdinal) + " | " +
		Cue.strOccurrenceId + " | src " + SourceWindow + " | " +
		Cue.strEffectAssetId;
	const bool_t bOpen = ImGui::TreeNodeEx(
		Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::TextDisabled("binding %s | anchor %s | %s / %s | repeat %s | stage %u ms | mapped %zu times",
		Cue.strBindingId.c_str(), Cue.strAnchorSlotId.c_str(),
		Cue.strFollowPolicy.c_str(), Cue.strStopPolicy.c_str(),
		Cue.strRepeatPolicy.c_str(), Cue.iStageDurationMs,
		Count_ProductCueMappings(Cue.strEffectAssetId));
	ImGui::TextDisabled(
		"Open/Play controls are listed once in Saved Unified Effects above.");
	ImGui::TreePop();
	ImGui::PopID();
}

bool_t Client::CEffect_Tool::Prepare_ActiveValtanPatternDraftTimeline(
	const bool_t bPaused)
{
	if (!m_ActiveDocument.has_value() ||
		EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT !=
			m_eActiveDocumentPreviewIntent ||
		m_strActiveValtanPatternDraftId.empty())
	{
		m_strPreviewAnimationStatus =
			"Valtan Pattern Draft has no preserved pattern identity.";
		return false;
	}

	const VALTAN_PATTERN_VIEW* pPattern =
		Find_ValtanPattern(m_strActiveValtanPatternDraftId);
	if (nullptr == pPattern || !Is_ValtanAllEffectsPattern(*pPattern))
	{
		m_strPreviewAnimationStatus =
			"Valtan Pattern Draft owner is no longer in the current split-owned playable inventory: " +
			m_strActiveValtanPatternDraftId;
		return false;
	}

	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> Clips;
	uint32_t iTimelineDurationMs = 0u;
	std::string Error;
	if (!Build_ValtanAuthoringTimeline(
			*pPattern, m_eActiveValtanPatternDraftPreviewPath,
			Clips, iTimelineDurationMs, Error))
	{
		m_strPreviewAnimationStatus =
			"Valtan Pattern Draft timeline could not be built: " + Error;
		return false;
	}

	/* An aggregate Pattern Draft is not a published cue occurrence. It owns the
	   complete selected Pattern clock and starts its authored Effect at t=0 while
	   reusing the same ordered occurrence sampler as Product previews. */
	Clear_ProductCuePreview();
	Reset_ValtanBossPatternTransformHistory();
	m_iValtanWorldOwnerStageDurationMs = iTimelineDurationMs;
	m_iValtanReferenceEffectStartMs = 0u;
	m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration();
	if (!Play_ValtanStageSequence(Clips))
	{
		if (m_strPreviewAnimationStatus.empty())
		{
			m_strPreviewAnimationStatus =
				"Valtan Pattern Draft could not stage its ordered animation sequence.";
		}
		return false;
	}
	Set_SynchronizedAnimationPaused(bPaused);
	m_strPreviewAnimationStatus =
		"Valtan Pattern Draft bound at t=0: " + pPattern->strPatternId +
		" | " + std::to_string(Clips.size()) + " occurrences | " +
		std::to_string(iTimelineDurationMs) + " ms";
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanPatternDraftEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bPlayAfterOpen)
{
	if (Path.empty() || strEffectAssetId.empty() ||
		!Is_ValtanAllEffectsPattern(Pattern))
	{
		m_strValtanPatternEffectStatus =
			"Pattern Draft requires one exact Pattern and authored Effect path.";
		return false;
	}

	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId;
	const bool_t bPreserveAuthoringPivot = bAlreadyActive &&
		EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT ==
			m_eActiveDocumentPreviewIntent &&
		m_strActiveValtanPatternDraftId == Pattern.strPatternId;
	if (!bAlreadyActive)
	{
		UNIFIED_EFFECT_CACHE& Cache =
			m_ValtanUnifiedEffectCaches[strEffectAssetId];
		if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
			!Cache.bValid || (bPlayAfterOpen && !Cache.bDrawable))
		{
			m_strValtanPatternEffectStatus = Cache.strStatus.empty() ?
				"Pattern Draft Effect is not ready for this operation." :
				Cache.strStatus;
			return false;
		}
		if (!Try_LoadDocumentPath(Path, EFFECT_DOCUMENT_SOURCE::AUTHORED,
				strEffectAssetId,
				EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == Path &&
				m_PendingDocumentLoad->strSelectionId == strEffectAssetId)
			{
				m_PendingDocumentLoad->strValtanPatternId = Pattern.strPatternId;
				m_PendingDocumentLoad->eValtanPatternPreviewPath =
					"VALTAN_DASH_CHARGE" == Pattern.strPatternId ?
						m_eValtanDashAuthoringTimelinePath :
						VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
				m_PendingDocumentLoad->bPlayCompleteAfterLoad = bPlayAfterOpen;
			}
			return false;
		}
	}
	else if (bPlayAfterOpen && !m_bActiveDocumentDrawable)
	{
		m_strValtanPatternEffectStatus =
			"Create and bind at least one drawable Element before Pattern Draft Play.";
		return false;
	}

	if (bAlreadyActive)
		Release_WorldPreview(true);
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT;
	m_strActiveValtanPatternDraftId = Pattern.strPatternId;
	m_eActiveValtanPatternDraftPreviewPath =
		"VALTAN_DASH_CHARGE" == Pattern.strPatternId ?
			m_eValtanDashAuthoringTimelinePath :
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	if (!bPreserveAuthoringPivot)
	{
		m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
		m_strPreviewAnchorSlotId.clear();
		Copy_Buffer(m_PreviewAnchorBuffer.data(),
			m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
	}
	if (!Prepare_ActiveValtanPatternDraftTimeline(true))
	{
		m_strValtanPatternEffectStatus = m_strPreviewAnimationStatus;
		return false;
	}
	if (bPlayAfterOpen && !Try_PlayActiveUnifiedEffect())
	{
		Set_SynchronizedAnimationPaused(true);
		m_strValtanPatternEffectStatus = m_strPreviewStatus.empty() ?
			"Pattern Draft Effect preview could not start." :
			m_strPreviewStatus;
		return false;
	}

	m_strValtanPatternEffectStatus = bPlayAfterOpen ?
		("Playing Pattern Draft Effect + animation from t=0: " +
		 Pattern.strPatternId) :
		("Opened Pattern Draft with its animation paused at t=0: " +
		 Pattern.strPatternId);
	return true;
}

bool_t Client::CEffect_Tool::Prepare_ValtanStandaloneEffectTarget()
{
	if (nullptr == m_pCharacterPreviewPanel)
	{
		m_strPreviewAnimationStatus =
			"Standalone Valtan Effect requires the Model View panel.";
		return false;
	}
	if ((nullptr == CAnimationTargetService::Resolve_Boss() ||
		 CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME) &&
		!m_pCharacterPreviewPanel->Select_TargetAsset(
			VALTAN_ANIMATION_ASSET_NAME))
	{
		m_strPreviewAnimationStatus =
			"Standalone Valtan Effect could not stage its dedicated Model View target.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == CAnimationTargetService::Resolve_Boss() ||
		nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Standalone Valtan Effect target did not expose a boss model.";
		return false;
	}

	/* Pause the concrete model before clearing sequence ownership. The generic
	   pause helper intentionally ignores an empty/stale sequence generation. */
	if (!pModel->Set_Animation(VALTAN_STANDALONE_STATIC_CLIP, true))
		pModel->Set_Animation(0u, true);
	pModel->Play_Animation(0.f);
	pModel->Set_AnimationSpeed(1.f);
	pModel->Set_AnimPaused(true);
	Reset_SynchronizedAnimationSequence();
	Reset_ValtanBossPatternTransformHistory();
	Clear_ProductCuePreview();
	m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
	m_strPreviewAnchorSlotId.clear();
	Copy_Buffer(m_PreviewAnchorBuffer.data(),
		m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
	m_bPreviewVisibleRequested = false;
	m_bPreviewPlaying = false;
	m_strPreviewAnimationStatus =
		"Standalone Valtan Effect mode: static model root, no animation timeline.";
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanStandaloneEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId)
{
	if (Path.empty() || strEffectAssetId.empty())
	{
		m_strPreviewStatus =
			"Standalone Valtan Effect has no exact authored source path.";
		return false;
	}

	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}

	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId;
	if (!bAlreadyActive)
	{
		const bool_t bLoaded = Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId,
			EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT);
		if (!bLoaded)
			m_strPreviewStatus = m_strDocumentStatus;
		return bLoaded;
	}

	/* Active drafts are previewed without reloading the disk document. Stage
	   the dedicated static target before changing active preview ownership. */
	if (!Prepare_ValtanStandaloneEffectTarget())
	{
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return false;
	}
	Release_WorldPreview(true);
	m_eActiveDocumentPreviewIntent =
		EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT;
	m_strActiveValtanPatternDraftId.clear();
	m_eActiveValtanPatternDraftPreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayValtanStandaloneEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId)
{
	if (!Try_OpenValtanStandaloneEffect(Path, strEffectAssetId))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path &&
			m_PendingDocumentLoad->strSelectionId == strEffectAssetId &&
			m_PendingDocumentLoad->ePreviewIntent ==
				EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT)
		{
			m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
		}
		return false;
	}
	return Try_PlayActiveUnifiedEffect();
}

bool_t Client::CEffect_Tool::Try_PlayValtanCombatObjectIndependentEffect(
	const std::filesystem::path& Path,
	const VALTAN_INDEPENDENT_EFFECT_VIEW& Effect,
	const VALTAN_PATTERN_VIEW& OwnerPattern)
{
	if (Path.empty() || Effect.strEffectAssetId.empty() ||
		"SERVER_COMBAT_OBJECT" != Effect.strOwnership ||
		Effect.strOwnerPatternId != OwnerPattern.strPatternId)
	{
		m_strPreviewStatus =
			"Independent combat-object preview lost its exact Product owner.";
		return false;
	}

	const VALTAN_STAGE_VIEW* pOwnerStage = nullptr;
	size_t iOwnerStageMatches = 0u;
	for (const VALTAN_STAGE_VIEW& Stage : OwnerPattern.Stages)
	{
		if (Stage.strStageId == Effect.strOwnerStageId)
		{
			pOwnerStage = &Stage;
			++iOwnerStageMatches;
		}
	}
	const VALTAN_COMBAT_OBJECT_EFFECT_VIEW* pCombatObject = nullptr;
	size_t iCombatObjectMatches = 0u;
	if (nullptr != pOwnerStage)
	{
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Candidate :
			pOwnerStage->CombatObjectEffects)
		{
			if (Candidate.strCombatObjectArchetypeId ==
					Effect.strCombatObjectArchetypeId &&
				Candidate.strClientVisualId == Effect.strClientVisualId &&
				Candidate.strEffectAssetId == Effect.strEffectAssetId)
			{
				pCombatObject = &Candidate;
				++iCombatObjectMatches;
			}
		}
	}
	if (1u != iOwnerStageMatches || nullptr == pOwnerStage ||
		1u != iCombatObjectMatches || nullptr == pCombatObject ||
		"ENTER" != pCombatObject->strTrigger ||
		("BOSS_RELATIVE" != pCombatObject->strVolleyPolicy &&
		 "ARENA_CENTER" != pCombatObject->strVolleyPolicy) ||
		"RADIAL" != pCombatObject->strVolleyLayout ||
		0u == pCombatObject->iSpawnValue ||
		0u == pCombatObject->iLifetimeMs ||
		pCombatObject->iLifetimeMs > pOwnerStage->iDurationMs)
	{
		m_strPreviewStatus =
			"Independent combat-object preview requires one exact boss-relative radial Product lifecycle.";
		return false;
	}

	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[Effect.strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(
			Cache, Path, Effect.strEffectAssetId) ||
		!Cache.bValid || !Cache.bDrawable)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument->strEffectAssetId == Effect.strEffectAssetId;
	if (bAlreadyActive && Has_UnsavedWork())
	{
		m_strPreviewStatus =
			"Save or discard Current Effect changes before Product-owned combat-object lifecycle preview.";
		return false;
	}
	if (!bAlreadyActive && !Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED,
			Effect.strEffectAssetId,
			EFFECT_DOCUMENT_PREVIEW_INTENT::STANDALONE_EFFECT))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path &&
			m_PendingDocumentLoad->strSelectionId == Effect.strEffectAssetId)
		{
			m_PendingDocumentLoad->strValtanPatternId =
				OwnerPattern.strPatternId;
			m_PendingDocumentLoad->strValtanIndependentEffectId =
				Effect.strIndependentEffectId;
			m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
		}
		return false;
	}

	Release_WorldPreview(true);
	if (!Prepare_ValtanStandaloneEffectTarget())
	{
		m_strPreviewStatus = m_strPreviewAnimationStatus;
		return false;
	}
	const shared_ptr<CValtan> pBoss =
		CAnimationTargetService::Resolve_Boss();
	if (nullptr == pBoss)
	{
		m_strPreviewStatus =
			"Independent combat-object preview target did not expose Valtan.";
		return false;
	}
	std::string StageStatus;
	if (!pBoss->Stage_LocalPatternAuthoringPreview(
			OwnerPattern, StageStatus))
	{
		m_strPreviewStatus =
			"Independent combat-object Product staging failed: " + StageStatus;
		return false;
	}

	VALTAN_COMBAT_OBJECT_INDEPENDENT_PREVIEW Preview;
	Preview.strIndependentEffectId = Effect.strIndependentEffectId;
	Preview.strEffectAssetId = Effect.strEffectAssetId;
	Preview.strOwnerPatternId = OwnerPattern.strPatternId;
	Preview.strOwnerStageId = pOwnerStage->strStageId;
	Preview.strOwnerActionId = pOwnerStage->strActionId;
	Preview.iInstanceCount = pCombatObject->iSpawnValue;
	Preview.iLifetimeMs = pCombatObject->iLifetimeMs;
	Preview.iTimelineDurationMs = (std::max)(
		pOwnerStage->iDurationMs, pCombatObject->iLifetimeMs);
	Preview.iTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	Preview.pBoss = pBoss;
	m_ValtanCombatObjectIndependentPreview = std::move(Preview);
	m_pCharacterPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, true,
		"Stop the independent combat-object preview before another Tool changes the locked Valtan target.");
	m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
	m_strPreviewAnchorSlotId.clear();
	Copy_Buffer(m_PreviewAnchorBuffer.data(),
		m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
	m_iValtanWorldOwnerStageDurationMs =
		m_ValtanCombatObjectIndependentPreview->iTimelineDurationMs;
	m_iValtanReferenceEffectStartMs = 0u;
	m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration();
	m_bPreviewVisibleRequested = true;
	m_bPreviewPlaying = true;
	if (!Sync_ValtanCombatObjectIndependentPreview(true))
	{
		const std::string Failure = m_strPreviewStatus;
		Clear_ValtanCombatObjectIndependentPreview();
		m_bPreviewVisibleRequested = false;
		m_bPreviewPlaying = false;
		m_strPreviewStatus = Failure;
		return false;
	}
	m_strPreviewAnimationStatus =
		"Valtan IDLE is paused; only the Server-owned combat-object lifecycle is sampled.";
	return true;
}

bool_t Client::CEffect_Tool::Sync_ValtanCombatObjectIndependentPreview(
	const bool_t bResetTransport)
{
	if (!m_ValtanCombatObjectIndependentPreview.has_value())
		return false;
	const shared_ptr<CValtan> pBoss =
		m_ValtanCombatObjectIndependentPreview->pBoss.lock();
	const shared_ptr<CValtan> pCurrentBoss =
		CAnimationTargetService::Resolve_Boss();
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	const char_t* const pCurrentAnimation = nullptr == pModel ? nullptr :
		pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
	if (nullptr == pBoss || pCurrentBoss != pBoss ||
		m_ValtanCombatObjectIndependentPreview->iTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		nullptr == pCurrentAnimation ||
		std::string_view(pCurrentAnimation) !=
			VALTAN_STANDALONE_STATIC_CLIP ||
		!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId !=
			m_ValtanCombatObjectIndependentPreview->strEffectAssetId)
	{
		m_strPreviewStatus =
			"Independent combat-object preview lost its locked Valtan target generation.";
		return false;
	}
	pModel->Set_AnimPaused(true);

	std::string Status;
	if (!pBoss->Apply_LocalCombatObjectAuthoringPreviewSample(
			m_ValtanCombatObjectIndependentPreview->strOwnerActionId,
			m_fPreviewTimeSeconds, bResetTransport, Status))
	{
		m_strPreviewStatus = Status.empty() ?
			"Independent combat-object lifecycle sample failed." : Status;
		return false;
	}
	m_strPreviewStatus = Status + " IDLE owner | authored lifetime " +
		std::to_string(
			m_ValtanCombatObjectIndependentPreview->iLifetimeMs) + " ms.";
	return true;
}

void Client::CEffect_Tool::Clear_ValtanCombatObjectIndependentPreview()
{
	if (!m_ValtanCombatObjectIndependentPreview.has_value())
		return;
	if (const shared_ptr<CValtan> pBoss =
			m_ValtanCombatObjectIndependentPreview->pBoss.lock())
	{
		pBoss->Reset_LocalPatternPresentationSample();
	}
	m_ValtanCombatObjectIndependentPreview.reset();
	m_iValtanWorldOwnerStageDurationMs = 0u;
	m_iValtanReferenceEffectStartMs = 0u;
	if (nullptr != m_pCharacterPreviewPanel)
	{
		m_pCharacterPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, false, {});
	}
}

bool_t Client::CEffect_Tool::Can_PlayValtanServerPattern(
	const VALTAN_PATTERN_VIEW& Pattern,
	std::string& strOutReason) const
{
	strOutReason.clear();
	if (!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		strOutReason =
			"Complete Play requires a freshly ADMITTED Valtan Pattern view; preserved rows are display-only.";
		return false;
	}
	if (Pattern.strPatternId.empty())
	{
		strOutReason = "Complete Play requires one stable Pattern ID.";
		return false;
	}
	if (nullptr == Resolve_ValtanServerPatternBossPlacement(
			CGameInstance::Get().Get_CurrentLevelID()))
	{
		strOutReason = "Complete Play is available only in Valtan Arena.";
		return false;
	}
	if (!CNetworkManager::Get().Is_Connected())
	{
		strOutReason = "Start and connect the Debug Server first.";
		return false;
	}

	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!Boss.isValid)
	{
		strOutReason =
			"Wait for the Server Valtan snapshot before Play Server.";
		return false;
	}
	if (0u == Boss.iCurrentHp)
	{
		strOutReason =
			"The Server Valtan is dead; respawn or re-enter the Arena first.";
		return false;
	}
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	if (!Player.isValid)
	{
		strOutReason =
			"Wait for the replicated player snapshot before Play Server.";
		return false;
	}
	if (0u == Player.iCurrentHp)
	{
		strOutReason =
			"Player is dead. Open Valtan Boss Tool and use Revive Player first.";
		return false;
	}
	if (!Player.isCombatReady)
	{
		strOutReason =
			"The Server player must be combat-ready before Play Server.";
		return false;
	}

	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	if (Audition.Is_InFlight())
	{
		strOutReason = "Complete Play is " +
			std::string(Describe_ValtanPatternAuditionState(Audition.eState)) +
			" for " + Audition.strPatternId + " (owner " +
			Audition.strConsumerId + ").";
		return false;
	}
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	std::string Reason;
	if (!Can_PlayValtanServerPattern(Pattern, Reason))
	{
		m_strValtanServerPatternStatusPatternId = Pattern.strPatternId;
		m_strValtanServerPatternStatus = std::move(Reason);
		return false;
	}

	m_strValtanServerPatternStatusPatternId = Pattern.strPatternId;
#ifdef _DEBUG
	CMainApp* const pApp = CMainApp::Get_Active();
	if (nullptr == pApp)
	{
		m_strValtanServerPatternStatus =
			"Complete Play workspace is unavailable.";
		return false;
	}
	if (!pApp->Debug_SelectCompletePlayPattern(Pattern.strPatternId))
	{
		m_strValtanServerPatternStatus =
			"The selected Effect pattern is not in the shared Server inventory.";
		return false;
	}
	return pApp->Debug_CompletePlaySelected(
		m_strValtanServerPatternStatus);
#else
	m_strValtanServerPatternStatus =
		"Complete Play is available only in a Debug authoring build.";
	return false;
#endif
}

void Client::CEffect_Tool::Update_ValtanServerPatternAudition()
{
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	if (Audition.strPatternId.empty() ||
		Audition.strPatternId != m_strValtanServerPatternStatusPatternId)
	{
		return;
	}
	m_strValtanServerPatternStatusPatternId = Audition.strPatternId;
	m_strValtanServerPatternStatus =
		std::string(Describe_ValtanPatternAuditionState(Audition.eState)) +
		" | " + Audition.strStatus;
}

void Client::CEffect_Tool::Update_ValtanPatternProductEffectUnlink()
{
	if (!m_ValtanPatternProductUnlinkOperation.has_value())
		return;
	VALTAN_PATTERN_PRODUCT_UNLINK_OPERATION& Operation =
		*m_ValtanPatternProductUnlinkOperation;
	if (nullptr == Operation.hProcess)
	{
		m_strValtanPatternEffectStatus =
			"Product Effect unlink lost its process observation handle. All Effects remains locked; restart the Client only after inspecting source/Product state.";
		return;
	}

	const DWORD iWait = WaitForSingleObject(Operation.hProcess, 0u);
	if (WAIT_TIMEOUT == iWait)
	{
		constexpr uint64_t SLOW_UNLINK_NOTICE_MS = 180000u;
		const uint64_t iNow = GetTickCount64();
		if (!Operation.bSlowNoticeShown &&
			iNow - Operation.iStartedTickMs >= SLOW_UNLINK_NOTICE_MS)
		{
			Operation.bSlowNoticeShown = true;
			m_strValtanPatternEffectStatus =
				"Product Effect unlink is still running after 180 seconds. It was not terminated because the source/Product transaction must finish or roll back.";
		}
		return;
	}
	if (WAIT_OBJECT_0 != iWait)
	{
		m_strValtanPatternEffectStatus =
			"Product Effect unlink process observation failed. It was not terminated and All Effects remains locked while the child may still be committing or rolling back.";
		return;
	}

	const VALTAN_PATTERN_EFFECT_SELECTION Selection = Operation.Selection;
	DWORD iExitCode = 1u;
	if (FALSE == GetExitCodeProcess(Operation.hProcess, &iExitCode))
	{
		CloseHandle(Operation.hProcess);
		Operation.hProcess = nullptr;
		(void)Refresh_ValtanPatternTree();
		(void)Refresh_DataFiles();
		m_strValtanPatternEffectStatus =
			"Product Effect unlink exited but its result code could not be read. Disk state was reloaded, preservation is unknown, and All Effects remains locked until Client restart.";
		return;
	}
	CloseHandle(Operation.hProcess);
	Operation.hProcess = nullptr;
	m_ValtanPatternProductUnlinkOperation.reset();
	m_PendingValtanPatternEffectDeletion.reset();
	m_SelectedValtanPatternEffect.reset();

	const bool_t bTreeReloaded = Refresh_ValtanPatternTree();
	const bool_t bDataFilesReloaded = Refresh_DataFiles();
	if (0u != iExitCode)
	{
		m_strValtanPatternEffectStatus =
			"Product Effect unlink failed with exit code " +
			std::to_string(iExitCode) +
			". The Tool reloaded disk state and does not assume that source or Product was preserved.";
		if (!bTreeReloaded || !bDataFilesReloaded)
		{
			m_strValtanPatternEffectStatus +=
				" Disk reload also failed; use Refresh and inspect the first save error before retrying.";
		}
		return;
	}

	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == Selection.strEffectAssetId)
	{
		Discard_ActiveDocument();
	}
	m_ValtanUnifiedEffectCaches.erase(Selection.strEffectAssetId);
	m_strValtanPatternEffectStatus =
		"Unlinked the selected Product Effect from only " +
		Selection.strPatternId +
		". The shared Effect asset, authored file, and other Pattern links were preserved.";
	if (!bTreeReloaded || !bDataFilesReloaded)
	{
		m_strValtanPatternEffectStatus +=
			" Product files committed, but the current view could not fully refresh.";
	}
	m_strValtanPatternEffectStatus +=
		" Restart the Server and re-enter Valtan Arena before judging Play Server.";
}

bool_t Client::CEffect_Tool::Try_CreateValtanPatternEffect(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	if (!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		m_strValtanPatternEffectStatus =
			"Create Effect requires a freshly ADMITTED Valtan Pattern view; preserved rows are display-only.";
		return false;
	}
	if (!m_bValtanPatternAuthoringEffectsLoaded)
	{
		m_strValtanPatternEffectStatus =
			"Refresh the Valtan pattern Effect ownership document before creating.";
		return false;
	}
	if (Has_UnsavedWork())
	{
		m_strValtanPatternEffectStatus =
			"Save or explicitly discard the active Effect changes before New Effect.";
		return false;
	}
	const VALTAN_PATTERN_VIEW* pCurrent = Find_ValtanPattern(
		Pattern.strPatternId);
	if (nullptr == pCurrent || !Is_ValtanAllEffectsPattern(*pCurrent) ||
		m_strSelectedValtanPatternId != Pattern.strPatternId)
	{
		m_strValtanPatternEffectStatus =
			"New Effect requires one selected pattern from the current split-owned playable inventory.";
		return false;
	}
	if (nullptr != Find_ValtanPatternAuthoringEffect(Pattern.strPatternId))
	{
		m_strValtanPatternEffectStatus =
			"This pattern already owns its one aggregate authoring Effect.";
		return false;
	}
	std::string Error;
	CValtanPatternAuthoringEffectTransaction Transaction;
	if (!Transaction.Try_Acquire(Error))
	{
		m_strValtanPatternEffectStatus = Error;
		return false;
	}
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT CurrentOwnership;
	std::string CurrentOwnershipBaseline;
	if (!CValtanPatternAuthoringEffectDocument::Load(
			CurrentOwnership, CurrentOwnershipBaseline, Error) ||
		CurrentOwnershipBaseline !=
			m_strValtanPatternAuthoringEffectsBaseline)
	{
		m_strValtanPatternEffectStatus =
			"Pattern Effect ownership changed after Refresh; refresh before creating. " +
			Error;
		return false;
	}
	if (std::any_of(
			CurrentOwnership.Bindings.begin(), CurrentOwnership.Bindings.end(),
			[&Pattern](
				const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Existing)
			{
				return Existing.strPatternId == Pattern.strPatternId;
			}))
	{
		m_strValtanPatternEffectStatus =
			"This pattern gained an aggregate authoring Effect after Refresh; refresh the view.";
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_BINDING Binding;
	Binding.strPatternId = Pattern.strPatternId;
	Binding.strEffectAssetId =
		Build_ValtanPatternAggregateEffectAssetId(Pattern);
	Binding.strAuthoringPath =
		CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
			Binding.strEffectAssetId);
	Binding.strState = "DRAFT_ATTACHED";
	const std::filesystem::path EffectPath =
		CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(Binding);
	if (Binding.strEffectAssetId.empty() ||
		Binding.strAuthoringPath.empty() || EffectPath.empty())
	{
		m_strValtanPatternEffectStatus =
			"Pattern action identity cannot produce a safe authored Effect path.";
		return false;
	}
	CAuthoritativeProductSourceReadLocks ProductSourceLocks;
	bool_t bFreshProductOwned = false;
	if (!Try_LockAndInspectAuthoritativeProductOwnership(
			Binding.strEffectAssetId, ProductSourceLocks,
			bFreshProductOwned, Error))
	{
		m_strValtanPatternEffectStatus =
			"New Effect could not prove fresh Product non-ownership: " + Error;
		return false;
	}
	if (CEffectCatalog::Contains(Binding.strEffectAssetId) ||
		bFreshProductOwned)
	{
		m_strValtanPatternEffectStatus =
			"New Effect refuses an Effect ID already owned by Product: " +
			Binding.strEffectAssetId + ". Use Open Existing Effect to preserve it.";
		return false;
	}

	std::error_code FileError;
	const bool_t bEffectPathExists = std::filesystem::exists(
		EffectPath, FileError);
	if (FileError || bEffectPathExists)
	{
		m_strValtanPatternEffectStatus = FileError ?
			"New Effect could not inspect its exact authored destination." :
			"New Effect refuses an existing unowned Effect path: " +
				EffectPath.string() + ". Use Open Existing Effect to preserve it.";
		return false;
	}

	EFFECT_DOCUMENT_DESC EffectDocument;
	EffectDocument.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	EffectDocument.iLoadedFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	EffectDocument.strEffectAssetId = Binding.strEffectAssetId;
	EffectDocument.strDisplayName = Pattern.strDisplayName.empty() ?
		(Pattern.strPatternId + " | Pattern Effect") :
		(Pattern.strDisplayName + " | Pattern Effect");
	if (!CEffectDocumentCodec::Validate(EffectDocument, Error))
	{
		m_strValtanPatternEffectStatus =
			"New Effect document validation failed: " + Error;
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT StagedOwnership =
		CurrentOwnership;
	StagedOwnership.Bindings.push_back(Binding);
	if (!CValtanPatternAuthoringEffectDocument::Validate(
			StagedOwnership, Error))
	{
		m_strValtanPatternEffectStatus =
			"Pattern Effect ownership validation failed: " + Error;
		return false;
	}

	const std::string EffectCanonical =
		CEffectDocumentCodec::Serialize(EffectDocument);
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
			EffectPath, EffectDocument, std::string_view{}, Error))
	{
		m_strValtanPatternEffectStatus =
			"New Effect file was not created: " + Error;
		return false;
	}
	if (!CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged(
			StagedOwnership,
			CurrentOwnershipBaseline, Error))
	{
		std::string RollbackStatus;
		const bool_t bRolledBack = Remove_EffectDocumentIfCanonical(
			EffectPath, EffectCanonical, RollbackStatus);
		m_strValtanPatternEffectStatus =
			"Pattern ownership save failed: " + Error +
			(bRolledBack ?
				(" Rollback: " + RollbackStatus) :
				(" Rollback was not applied: " + RollbackStatus));
		return false;
	}

	const bool_t bOwnershipReloaded =
		Refresh_ValtanPatternAuthoringEffects();
	const bool_t bDataFilesReloaded = Refresh_DataFiles();
	m_strSelectedValtanPatternId = Pattern.strPatternId;
	Select_SharedCompletePlayPattern(m_strSelectedValtanPatternId);
	VALTAN_PATTERN_EFFECT_SELECTION CreatedSelection;
	CreatedSelection.eKind =
		VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED;
	CreatedSelection.strPatternId = Pattern.strPatternId;
	CreatedSelection.strEffectAssetId = Binding.strEffectAssetId;
	m_SelectedValtanPatternEffect = std::move(CreatedSelection);
	const std::string CommitStatus =
		"Created one empty aggregate Effect for " + Pattern.strPatternId +
		" and attached it as DRAFT_ATTACHED authoring ownership.";
	m_strValtanPatternEffectStatus = CommitStatus;
	if (!bOwnershipReloaded || !bDataFilesReloaded)
	{
		m_strValtanPatternEffectStatus +=
			" The files committed, but the current view could not fully refresh.";
	}
	const std::string RefreshStatus = m_strValtanPatternEffectStatus;
	if (!Try_OpenValtanPatternDraftEffect(
			EffectPath, Binding.strEffectAssetId, Pattern, false))
	{
		const std::string OpenFailure = m_strValtanPatternEffectStatus;
		m_strValtanPatternEffectStatus = RefreshStatus +
			" Auto-open failed; files remain committed. Use Open Editor after resolving: " +
			(OpenFailure.empty() ? std::string("unknown preview staging failure") :
				OpenFailure);
	}
	else
	{
		m_strValtanPatternEffectStatus = RefreshStatus +
			" Opened the new Effect with its Pattern animation paused at t=0.";
	}
	return true;
}

bool_t Client::CEffect_Tool::Can_DeleteSelectedValtanPatternEffect(
	std::string& strOutReason) const
{
	strOutReason.clear();
	if (!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		strOutReason =
			"Delete/Unlink requires a freshly ADMITTED Valtan Pattern view; preserved rows are display-only.";
		return false;
	}
	if (m_ValtanPatternProductUnlinkOperation.has_value())
	{
		strOutReason =
			"Wait for the current Product Effect unlink transaction to finish.";
		return false;
	}
	if (!m_SelectedValtanPatternEffect.has_value())
	{
		strOutReason = "Select one Effect row under the selected Pattern first.";
		return false;
	}
	if (Has_UnsavedWork())
	{
		strOutReason =
			"Save or discard the active Effect edits before deleting its Pattern link.";
		return false;
	}

	const VALTAN_PATTERN_EFFECT_SELECTION& Selection =
		*m_SelectedValtanPatternEffect;
	const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPattern(
		Selection.strPatternId);
	if (nullptr == pPattern || !Is_ValtanAllEffectsPattern(*pPattern) ||
		m_strSelectedValtanPatternId != Selection.strPatternId)
	{
		strOutReason =
			"The selected Effect no longer belongs to the selected Pattern; select its row again.";
		return false;
	}

	if (VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED ==
		Selection.eKind)
	{
		const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING* pBinding =
			Find_ValtanPatternAuthoringEffect(Selection.strPatternId);
		if (nullptr == pBinding ||
			pBinding->strEffectAssetId != Selection.strEffectAssetId ||
			pBinding->strState != "DRAFT_ATTACHED")
		{
			strOutReason =
				"The selected unsaved Draft changed after selection; Refresh and select it again.";
			return false;
		}
		strOutReason =
			"Delete this unsaved Draft ownership and its exact authored Effect file.";
		return true;
	}

	if (VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK !=
		Selection.eKind)
	{
		strOutReason = "The selected Effect kind cannot be deleted here.";
		return false;
	}
	if (!pPattern->bAuthoringMasterManaged)
	{
		strOutReason =
			"This Product row has no canonical split-authoring owner to unlink.";
		return false;
	}
	if (Selection.bV1Alias)
	{
		strOutReason =
			"Select the source [PRODUCT] row instead of its read-only V1 alias.";
		return false;
	}
	if (Selection.CueIds.empty())
	{
		strOutReason =
			"This row is owned by a Server combat visual, not a removable Product cue link.";
		return false;
	}

	const std::set<std::string> ExpectedCueIds(
		Selection.CueIds.begin(), Selection.CueIds.end());
	if (ExpectedCueIds.size() != Selection.CueIds.size())
	{
		strOutReason =
			"The selected Product cue identity is ambiguous; Refresh and select it again.";
		return false;
	}
	std::set<std::string> CurrentCueIds;
	for (const VALTAN_STAGE_VIEW& Stage : pPattern->Stages)
	{
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		{
			if (Cue.strEffectAssetId == Selection.strEffectAssetId)
				CurrentCueIds.insert(Cue.strBindingId);
		}
	}
	if (CurrentCueIds != ExpectedCueIds)
	{
		strOutReason =
			"The selected Product cue set changed after selection; Refresh and select it again.";
		return false;
	}

	strOutReason =
		"Unlink this Effect from only the selected Pattern. The shared Effect asset and authored file are preserved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_DeleteValtanPatternDraftEffect(
	const VALTAN_PATTERN_EFFECT_SELECTION& Selection)
{
	const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPattern(
		Selection.strPatternId);
	if (nullptr == pPattern ||
		Build_ValtanPatternAggregateEffectAssetId(*pPattern) !=
			Selection.strEffectAssetId)
	{
		m_strValtanPatternEffectStatus =
			"Draft delete refused a non-canonical Pattern aggregate Effect ID.";
		return false;
	}

	std::string Error;
	CValtanPatternAuthoringEffectTransaction Transaction;
	if (!Transaction.Try_Acquire(Error))
	{
		m_strValtanPatternEffectStatus = Error;
		return false;
	}

	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT CurrentOwnership;
	std::string CurrentOwnershipBaseline;
	if (!CValtanPatternAuthoringEffectDocument::Load(
			CurrentOwnership, CurrentOwnershipBaseline, Error) ||
		CurrentOwnershipBaseline != m_strValtanPatternAuthoringEffectsBaseline)
	{
		m_strValtanPatternEffectStatus =
			"Pattern Effect ownership changed after Refresh; the Draft was preserved. " +
			Error;
		return false;
	}

	const auto Binding = std::find_if(
		CurrentOwnership.Bindings.begin(), CurrentOwnership.Bindings.end(),
		[&Selection](const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING& Candidate)
		{
			return Candidate.strPatternId == Selection.strPatternId;
		});
	if (Binding == CurrentOwnership.Bindings.end() ||
		Binding->strEffectAssetId != Selection.strEffectAssetId ||
		Binding->strState != "DRAFT_ATTACHED" ||
		Binding->strAuthoringPath !=
			CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
				Selection.strEffectAssetId))
	{
		m_strValtanPatternEffectStatus =
			"Draft ownership is not the exact deterministic Pattern binding; no file was deleted.";
		return false;
	}

	const std::filesystem::path EffectPath =
		CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(*Binding);
	EFFECT_DOCUMENT_DESC EffectDocument;
	if (EffectPath.empty() || !CEffectDocumentCodec::Load(
			EffectPath, EffectDocument, Error) ||
		EffectDocument.strEffectAssetId != Selection.strEffectAssetId)
	{
		m_strValtanPatternEffectStatus =
			"Draft authored file no longer matches its exact ownership; it was preserved. " +
			Error;
		return false;
	}
	CAuthoritativeProductSourceReadLocks ProductSourceLocks;
	bool_t bFreshProductOwned = false;
	if (!Try_LockAndInspectAuthoritativeProductOwnership(
			Selection.strEffectAssetId, ProductSourceLocks,
			bFreshProductOwned, Error))
	{
		m_strValtanPatternEffectStatus =
			"Draft delete could not prove fresh Product non-ownership; the Effect was preserved. " +
			Error;
		return false;
	}
	if (CEffectCatalog::Contains(Selection.strEffectAssetId) ||
		bFreshProductOwned ||
		0u != Count_ProductCueMappings(Selection.strEffectAssetId))
	{
		m_strValtanPatternEffectStatus =
			"Draft delete refused an Effect that is registered or currently used by a Pattern. Remove its Pattern link first.";
		return false;
	}

	const std::string EffectCanonical =
		CEffectDocumentCodec::Serialize(EffectDocument);
	VALTAN_PATTERN_AUTHORING_EFFECT_DOCUMENT StagedOwnership =
		CurrentOwnership;
	StagedOwnership.Bindings.erase(
		StagedOwnership.Bindings.begin() +
			std::distance(CurrentOwnership.Bindings.begin(), Binding));
	if (!CValtanPatternAuthoringEffectDocument::Validate(
			StagedOwnership, Error))
	{
		m_strValtanPatternEffectStatus =
			"Draft ownership removal did not validate; no file was deleted. " +
			Error;
		return false;
	}
	const std::string StagedOwnershipCanonical =
		CValtanPatternAuthoringEffectDocument::Serialize(StagedOwnership);
	if (!CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged(
			StagedOwnership, CurrentOwnershipBaseline, Error))
	{
		m_strValtanPatternEffectStatus =
			"Draft ownership removal failed; the authored Effect file was preserved. " +
			Error;
		return false;
	}

	std::string FileDeleteStatus;
	if (!Remove_EffectDocumentIfCanonical(
			EffectPath, EffectCanonical, FileDeleteStatus))
	{
		std::string RollbackStatus;
		const bool_t bOwnershipRestored =
			CValtanPatternAuthoringEffectDocument::Save_AtomicIfUnchanged(
				CurrentOwnership, StagedOwnershipCanonical, RollbackStatus);
		(void)Refresh_ValtanPatternAuthoringEffects();
		m_strValtanPatternEffectStatus = bOwnershipRestored ?
			("Draft file delete failed and ownership was restored; the Effect was preserved. " +
				FileDeleteStatus) :
			("CRITICAL: Draft ownership was removed but the authored file was preserved. Refresh before retrying. File: " +
				FileDeleteStatus + " Ownership rollback: " + RollbackStatus);
		return false;
	}

	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == Selection.strEffectAssetId)
	{
		Discard_ActiveDocument();
	}
	m_ValtanUnifiedEffectCaches.erase(Selection.strEffectAssetId);
	m_SelectedValtanPatternEffect.reset();
	const bool_t bOwnershipReloaded =
		Refresh_ValtanPatternAuthoringEffects();
	const bool_t bDataFilesReloaded = Refresh_DataFiles();
	m_strValtanPatternEffectStatus =
		"Deleted the selected unsaved Pattern Draft ownership and its exact authored Effect file.";
	if (!bOwnershipReloaded || !bDataFilesReloaded)
	{
		m_strValtanPatternEffectStatus +=
			" Files committed, but the current view could not fully refresh.";
	}
	return true;
}

bool_t Client::CEffect_Tool::Try_UnlinkValtanPatternProductEffect(
	const VALTAN_PATTERN_EFFECT_SELECTION& Selection)
{
	if (!Can_MutateValtanView(m_eValtanPatternTreeAdmission))
	{
		m_strValtanPatternEffectStatus =
			"Product Effect unlink requires a freshly ADMITTED Valtan Pattern view; no source transaction was started.";
		return false;
	}
	if (m_ValtanPatternProductUnlinkOperation.has_value())
	{
		m_strValtanPatternEffectStatus =
			"A Product Effect unlink transaction is already running.";
		return false;
	}
	const std::filesystem::path ProjectRoot =
		CProjectDataRoot::Get().parent_path();
	const std::filesystem::path ToolPath = ProjectRoot /
		L"Tools" / L"ValtanPipeline" /
		L"Remove-ValtanPatternEffectLink.ps1";
	std::error_code FileError;
	if (ProjectRoot.empty() || !std::filesystem::is_regular_file(
			ToolPath, FileError) || FileError)
	{
		m_strValtanPatternEffectStatus =
			"Product Effect unlink tool is missing; no Product source was changed.";
		return false;
	}

	std::ostringstream CueList;
	for (size_t iCue = 0u; iCue < Selection.CueIds.size(); ++iCue)
	{
		if (0u != iCue)
			CueList << ',';
		CueList << Selection.CueIds[iCue];
	}
	const std::string CueIds = CueList.str();
	const std::wstring WidePattern(
		Selection.strPatternId.begin(), Selection.strPatternId.end());
	const std::wstring WideEffect(
		Selection.strEffectAssetId.begin(), Selection.strEffectAssetId.end());
	const std::wstring WideCueIds(CueIds.begin(), CueIds.end());
	std::wstring Command =
		L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		ToolPath.wstring() + L"\" -Mode Apply -PatternId \"" +
		WidePattern + L"\" -EffectAssetId \"" + WideEffect +
		L"\" -CueIds \"" + WideCueIds + L"\" -RepositoryRoot \"" +
		ProjectRoot.wstring() + L"\"";
	HANDLE hProcess = nullptr;
	std::string ProcessStatus;
	if (!Start_OwnedToolProcess(
			std::move(Command), ProjectRoot,
			"Valtan Pattern Product Effect unlink", hProcess,
			ProcessStatus))
	{
		m_strValtanPatternEffectStatus = ProcessStatus +
			" No unlink process was started.";
		return false;
	}
	VALTAN_PATTERN_PRODUCT_UNLINK_OPERATION Operation;
	Operation.hProcess = hProcess;
	Operation.iStartedTickMs = GetTickCount64();
	Operation.Selection = Selection;
	m_ValtanPatternProductUnlinkOperation = std::move(Operation);
	m_strValtanPatternEffectStatus =
		"Product Effect unlink is running for " + Selection.strPatternId +
		". All Effects editing is paused until the saved Pattern link is reloaded; the process will not be killed on timeout.";
	return true;
}

bool_t Client::CEffect_Tool::Try_DeleteSelectedValtanPatternEffect()
{
	if (m_PendingValtanPatternEffectDeletion.has_value())
	{
		if (!m_SelectedValtanPatternEffect.has_value())
		{
			m_strValtanPatternEffectStatus =
				"The Effect selection changed after the confirmation dialog opened; nothing was deleted.";
			return false;
		}
		const VALTAN_PATTERN_EFFECT_SELECTION& Pending =
			*m_PendingValtanPatternEffectDeletion;
		const VALTAN_PATTERN_EFFECT_SELECTION& Current =
			*m_SelectedValtanPatternEffect;
		if (Pending.eKind != Current.eKind ||
			Pending.strPatternId != Current.strPatternId ||
			Pending.strEffectAssetId != Current.strEffectAssetId ||
			Pending.CueIds != Current.CueIds ||
			Pending.bV1Alias != Current.bV1Alias)
		{
			m_strValtanPatternEffectStatus =
				"The Effect selection changed after the confirmation dialog opened; nothing was deleted.";
			return false;
		}
	}
	std::string Reason;
	if (!Can_DeleteSelectedValtanPatternEffect(Reason))
	{
		m_strValtanPatternEffectStatus = std::move(Reason);
		return false;
	}
	const VALTAN_PATTERN_EFFECT_SELECTION Selection =
		m_PendingValtanPatternEffectDeletion.has_value() ?
			*m_PendingValtanPatternEffectDeletion :
			*m_SelectedValtanPatternEffect;
	if (VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED ==
		Selection.eKind)
	{
		return Try_DeleteValtanPatternDraftEffect(Selection);
	}
	if (VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK ==
		Selection.eKind)
	{
		return Try_UnlinkValtanPatternProductEffect(Selection);
	}
	m_strValtanPatternEffectStatus =
		"The selected Effect kind cannot be deleted here.";
	return false;
}

void Client::CEffect_Tool::Render_ValtanPatternNode(
	const VALTAN_PATTERN_VIEW& Pattern,
	const char_t* pGroupLabel,
	const std::string& strSearch)
{
	if (!Is_ValtanAllEffectsPattern(Pattern) ||
		!Matches_ValtanPatternSearch(Pattern, strSearch))
	{
		return;
	}
	const VALTAN_PATTERN_AUTHORING_EFFECT_BINDING* pBinding =
		Find_ValtanPatternAuthoringEffect(Pattern.strPatternId);
	const bool_t bSelected =
		m_strSelectedValtanPatternId == Pattern.strPatternId;
	const bool_t bActiveDraft = nullptr != pBinding &&
		m_ActiveDocument.has_value() &&
		m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
		m_ActiveDocument->strEffectAssetId == pBinding->strEffectAssetId &&
		m_eActiveDocumentPreviewIntent ==
			EFFECT_DOCUMENT_PREVIEW_INTENT::VALTAN_PATTERN_DRAFT &&
		m_strActiveValtanPatternDraftId == Pattern.strPatternId;
	std::string Label = Pattern.strDisplayName.empty() ?
		Pattern.strPatternId : Pattern.strDisplayName;
	Label += " | " + std::string(pGroupLabel) + " | Effects";
	if (nullptr != pBinding)
		Label += " | Draft 1";

	ImGui::PushID(Pattern.strPatternId.c_str());
	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	const ImGuiTreeNodeFlags PatternFlags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		(bSelected ? ImGuiTreeNodeFlags_Selected : 0);
	const bool_t bPatternOpen = ImGui::TreeNodeEx(
		Label.c_str(), PatternFlags);
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		if (m_strSelectedValtanPatternId != Pattern.strPatternId)
			m_SelectedValtanPatternEffect.reset();
		m_strSelectedValtanPatternId = Pattern.strPatternId;
		Select_SharedCompletePlayPattern(m_strSelectedValtanPatternId);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Pattern ID: %s", Pattern.strPatternId.c_str());
	std::string ServerPlayReason;
	const bool_t bCanPlayServer =
		Can_PlayValtanServerPattern(Pattern, ServerPlayReason);
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanPlayServer);
	if (ImGui::SmallButton("Complete Play (Server/Arena)"))
	{
		if (m_strSelectedValtanPatternId != Pattern.strPatternId)
			m_SelectedValtanPatternEffect.reset();
		m_strSelectedValtanPatternId = Pattern.strPatternId;
		Select_SharedCompletePlayPattern(m_strSelectedValtanPatternId);
		Try_PlayValtanServerPattern(Pattern);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("%s", bCanPlayServer ?
			"Run this complete Pattern through the Valtan Arena Server fixed-tick path." :
			ServerPlayReason.c_str());
	}
	if (!bCanPlayServer && bSelected)
	{
		ImGui::TextWrapped("Server: %s", ServerPlayReason.c_str());
	}
	else if (m_strValtanServerPatternStatusPatternId == Pattern.strPatternId &&
		!m_strValtanServerPatternStatus.empty())
	{
		ImGui::TextWrapped("Server: %s",
			m_strValtanServerPatternStatus.c_str());
	}
	if (!bPatternOpen)
	{
		ImGui::PopID();
		return;
	}

	struct RUNTIME_VALTAN_PRODUCT_SOURCE final
	{
		const VALTAN_STAGE_VIEW* pStage = nullptr;
		const VALTAN_CLIP_OCCURRENCE_VIEW* pClip = nullptr;
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCue = nullptr;
	};
	struct RUNTIME_VALTAN_EFFECT_ROW final
	{
		std::string strEffectAssetId;
		std::filesystem::path Path;
		std::vector<RUNTIME_VALTAN_PRODUCT_SOURCE> ProductSources;
		std::vector<const VALTAN_STAGE_VIEW*> ReferenceStages;
		std::vector<const VALTAN_STAGE_VIEW*> CombatObjectStages;
		bool_t bV1Alias = false;
	};

	std::vector<RUNTIME_VALTAN_EFFECT_ROW> RuntimeRows;
	std::unordered_map<std::string, size_t> RuntimeRowIndices;
	const auto ResolveRuntimeRow = [&RuntimeRows, &RuntimeRowIndices](
		const std::string& strEffectAssetId) -> RUNTIME_VALTAN_EFFECT_ROW&
	{
		const auto [Found, bInserted] = RuntimeRowIndices.try_emplace(
			strEffectAssetId, RuntimeRows.size());
		if (bInserted)
		{
			RUNTIME_VALTAN_EFFECT_ROW Row;
			Row.strEffectAssetId = strEffectAssetId;
			RuntimeRows.push_back(std::move(Row));
		}
		return RuntimeRows[Found->second];
	};
	const auto AppendStageOnce = [](auto& Stages,
		const VALTAN_STAGE_VIEW& Stage)
	{
		if (std::find(Stages.begin(), Stages.end(), &Stage) == Stages.end())
			Stages.push_back(&Stage);
	};
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
			Stage.ProductCues)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW* pOwnerClip = nullptr;
			if (!Cue.bUsesStageClock)
			{
				const auto OwnerClip = std::find_if(
					Stage.ClipOccurrences.begin(),
					Stage.ClipOccurrences.end(),
					[&Cue](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
					{
						return Clip.strClipOccurrenceId ==
							Cue.strClipOccurrenceId;
					});
				if (OwnerClip != Stage.ClipOccurrences.end())
					pOwnerClip = &*OwnerClip;
			}
			RUNTIME_VALTAN_EFFECT_ROW& Row =
				ResolveRuntimeRow(Cue.strEffectAssetId);
			Row.ProductSources.push_back({ &Stage, pOwnerClip, &Cue });
			if (!Cue.strV1EffectAssetId.empty())
			{
				RUNTIME_VALTAN_EFFECT_ROW& V1Row =
					ResolveRuntimeRow(Cue.strV1EffectAssetId);
				V1Row.ProductSources.push_back({ &Stage, pOwnerClip, &Cue });
				V1Row.bV1Alias = true;
			}
		}
		for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
		{
			RUNTIME_VALTAN_EFFECT_ROW& Row =
				ResolveRuntimeRow(Effect.strEffectAssetId);
			if (Row.Path.empty() && !Effect.DocumentPath.empty())
				Row.Path = Effect.DocumentPath;
			AppendStageOnce(Row.ReferenceStages, Stage);
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
			Stage.CombatObjectEffects)
		{
			RUNTIME_VALTAN_EFFECT_ROW& Row =
				ResolveRuntimeRow(Effect.strEffectAssetId);
			AppendStageOnce(Row.CombatObjectStages, Stage);
		}
	}
	for (RUNTIME_VALTAN_EFFECT_ROW& Row : RuntimeRows)
	{
		const auto Editable = m_DirectAuthoredEditableEntries.find(
			Row.strEffectAssetId);
		if (Editable != m_DirectAuthoredEditableEntries.end())
			Row.Path = Editable->second.Path;
	}
	/* The independent library and the pattern projection answer different
	   questions.  Keep the same canonical document in both places when needed:
	   the root row is the direct editing entry, while this row proves that the
	   selected Server pattern actually consumes it. */
	std::erase_if(RuntimeRows,
		[](const RUNTIME_VALTAN_EFFECT_ROW& Row)
		{
			return Row.ProductSources.empty() &&
				Row.CombatObjectStages.empty();
		});

	if (bPatternOpen)
	{
		ImGui::SeparatorText("Saved Pattern Effects");
		if (RuntimeRows.empty())
		{
			ImGui::TextDisabled(
				"This pattern currently has no saved Effect cue or combat-object visual.");
		}
		for (const RUNTIME_VALTAN_EFFECT_ROW& Row : RuntimeRows)
		{
			std::vector<const VALTAN_STAGE_VIEW*> NonProductStages =
				Row.ReferenceStages;
			for (const VALTAN_STAGE_VIEW* pStage : Row.CombatObjectStages)
			{
				if (std::find(NonProductStages.begin(),
						NonProductStages.end(), pStage) ==
					NonProductStages.end())
				{
					NonProductStages.push_back(pStage);
				}
			}
			const RUNTIME_VALTAN_PRODUCT_SOURCE* pProductSource =
				1u == Row.ProductSources.size() ?
					&Row.ProductSources.front() : nullptr;
			const RUNTIME_VALTAN_PRODUCT_SOURCE* pStageClockSource =
				nullptr != pProductSource &&
				pProductSource->pCue->bUsesStageClock ?
					pProductSource : nullptr;
			optional<VALTAN_PRODUCT_PREVIEW> ProductPlaybackPreview;
			std::string ProductPlaybackError;
			if (nullptr != pProductSource && nullptr == pStageClockSource)
			{
				if (nullptr == pProductSource->pClip)
				{
					ProductPlaybackError =
						"Clip-bound Product cue lost its exact animation occurrence.";
				}
				else
				{
					VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue =
						*pProductSource->pCue;
					PlaybackCue.strEffectAssetId = Row.strEffectAssetId;
					VALTAN_PRODUCT_PREVIEW Preview;
					const VALTAN_PATTERN_PREVIEW_PATH eProductPath =
						"VALTAN_DASH_CHARGE" == Pattern.strPatternId ?
							m_eValtanDashAuthoringTimelinePath :
							VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
					if (Pattern.bAuthoringMasterManaged &&
						Build_ValtanProductPreview(Pattern, eProductPath,
							*pProductSource->pClip, PlaybackCue, Preview,
							ProductPlaybackError))
					{
						ProductPlaybackPreview = std::move(Preview);
					}
					else if (!Pattern.bAuthoringMasterManaged)
					{
						const VALTAN_STAGE_VIEW& OwnerStage =
							*pProductSource->pStage;
						const VALTAN_CLIP_OCCURRENCE_VIEW& OwnerClip =
							*pProductSource->pClip;
						const VALTAN_PRODUCT_EFFECT_CUE_VIEW& SourceCue =
							*pProductSource->pCue;
						const bool_t bExactEffectIdentity =
							SourceCue.strEffectAssetId == Row.strEffectAssetId ||
							SourceCue.strV1EffectAssetId == Row.strEffectAssetId;
						if (!bExactEffectIdentity ||
							SourceCue.strPatternId != Pattern.strPatternId ||
							SourceCue.strStageId != OwnerStage.strStageId ||
							SourceCue.strActionId != OwnerStage.strActionId ||
							SourceCue.strClipOccurrenceId !=
								OwnerClip.strClipOccurrenceId ||
							OwnerStage.iDurationMs != SourceCue.iStageDurationMs ||
							OwnerClip.strClipOccurrenceId.empty() ||
							OwnerClip.strClipName.empty() ||
							0u == SourceCue.iStageDurationMs)
						{
							ProductPlaybackError =
								"Product cue no longer owns one exact stage-local occurrence.";
						}
						else
						{
							Preview.Clip = OwnerClip;
							Preview.Cue = std::move(PlaybackCue);
							VALTAN_CLIP_OCCURRENCE_VIEW TimelineClip = OwnerClip;
							TimelineClip.iAuthoringWallMs =
								Preview.Cue.iStageDurationMs;
							Preview.TimelineClips = { std::move(TimelineClip) };
							Preview.iTimelineDurationMs =
								Preview.Cue.iStageDurationMs;
							ProductPlaybackPreview = std::move(Preview);
						}
					}
				}
			}

			const VALTAN_STAGE_VIEW* pNonProductStage =
				Row.ProductSources.empty() && 1u == NonProductStages.size() ?
					NonProductStages.front() : nullptr;
			const bool_t bCombatObjectPlaybackOwner =
				nullptr != pNonProductStage &&
				std::find(Row.CombatObjectStages.begin(),
					Row.CombatObjectStages.end(), pNonProductStage) !=
					Row.CombatObjectStages.end();
			std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> ReferenceTimeline;
			uint32_t iReferenceTimelineDurationMs = 0u;
			uint32_t iReferenceEffectStartMs = 0u;
			std::string ReferenceTimelineError;
			bool_t bReferenceTimelineReady = false;
			if (bCombatObjectPlaybackOwner)
			{
				const VALTAN_PATTERN_PREVIEW_PATH eReferencePath =
					"VALTAN_DASH_CHARGE" == Pattern.strPatternId ?
						m_eValtanDashAuthoringTimelinePath :
						VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
				bReferenceTimelineReady = Build_ValtanAuthoringTimeline(
					Pattern, eReferencePath, ReferenceTimeline,
					iReferenceTimelineDurationMs, ReferenceTimelineError);
				if (bReferenceTimelineReady)
				{
					for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
					{
						if (&Stage == pNonProductStage)
							break;
						iReferenceEffectStartMs += Stage.iDurationMs;
					}
				}
			}
			const bool_t bAmbiguousOccurrence =
				1u < Row.ProductSources.size() ||
				(!Row.ProductSources.empty() && !NonProductStages.empty()) ||
				(Row.ProductSources.empty() && 1u < NonProductStages.size());
			const bool_t bHasExactPlaybackOwner =
				ProductPlaybackPreview.has_value() ||
				nullptr != pStageClockSource || bReferenceTimelineReady;
			const bool_t bActive = m_ActiveDocument.has_value() &&
				m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
				m_ActiveDocument->strEffectAssetId == Row.strEffectAssetId;
			const auto ObservedCache = m_ValtanUnifiedEffectCaches.find(
				Row.strEffectAssetId);
			const bool_t bKnownInvalid =
				ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
				ObservedCache->second.bObserved &&
				!ObservedCache->second.bValid;
			const bool_t bKnownNonDrawable =
				ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
				ObservedCache->second.bObserved &&
				ObservedCache->second.bValid &&
				!ObservedCache->second.bDrawable;
			const std::string RuntimeLabel =
				(Row.bV1Alias ? "[V1] " :
					(Row.ProductSources.empty() ? "[WORLD] " : "[PRODUCT] ")) +
				Row.strEffectAssetId;
			const bool_t bRuntimeRowSelected =
				m_SelectedValtanPatternEffect.has_value() &&
				m_SelectedValtanPatternEffect->eKind ==
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK &&
				m_SelectedValtanPatternEffect->strPatternId ==
					Pattern.strPatternId &&
				m_SelectedValtanPatternEffect->strEffectAssetId ==
					Row.strEffectAssetId;
			const auto SelectRuntimeRow = [this, &Pattern, &Row]()
			{
				VALTAN_PATTERN_EFFECT_SELECTION Selection;
				Selection.eKind =
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::PRODUCT_CUE_LINK;
				Selection.strPatternId = Pattern.strPatternId;
				Selection.strEffectAssetId = Row.strEffectAssetId;
				Selection.bV1Alias = Row.bV1Alias;
				for (const RUNTIME_VALTAN_PRODUCT_SOURCE& Source :
					Row.ProductSources)
				{
					Selection.CueIds.push_back(Source.pCue->strBindingId);
				}
				std::ranges::sort(Selection.CueIds);
				const auto UniqueEnd = std::ranges::unique(
					Selection.CueIds).begin();
				Selection.CueIds.erase(UniqueEnd, Selection.CueIds.end());
				m_strSelectedValtanPatternId = Pattern.strPatternId;
				Select_SharedCompletePlayPattern(
					m_strSelectedValtanPatternId);
				m_SelectedValtanPatternEffect = std::move(Selection);
			};
			ImGui::PushID(Row.strEffectAssetId.c_str());
			const bool_t bRuntimeOpen = ImGui::TreeNodeEx(
				RuntimeLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow |
					(bRuntimeRowSelected || bActive ?
						ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				SelectRuntimeRow();
			if (bRuntimeOpen)
			{
				for (const RUNTIME_VALTAN_PRODUCT_SOURCE& Source :
					Row.ProductSources)
				{
					if (Source.pCue->bUsesStageClock)
					{
						ImGui::TextDisabled(
							"Stage %s | Server stage clock +%u ms | Cue %s | no body-animation owner",
							Source.pStage->strStageId.c_str(),
							Source.pCue->iStageOffsetMs,
							Source.pCue->strOccurrenceId.c_str());
					}
					else
					{
						ImGui::TextDisabled("Stage %s | Clip %s | Cue %s",
							Source.pStage->strStageId.c_str(),
							nullptr == Source.pClip ? "(missing)" :
								Source.pClip->strClipName.c_str(),
							Source.pCue->strOccurrenceId.c_str());
					}
				}
				if (!Row.CombatObjectStages.empty())
				{
					ImGui::TextDisabled(
						"Server combat-object visual | %zu owner stage(s)",
						Row.CombatObjectStages.size());
				}
				if (bAmbiguousOccurrence)
				{
					ImGui::TextDisabled(
						"Multiple runtime owners exist; Open Editor is available, but local Play requires one exact occurrence.");
				}
				else if (nullptr != pProductSource &&
					!ProductPlaybackPreview.has_value())
				{
					ImGui::TextWrapped("Joined animation unavailable: %s",
						ProductPlaybackError.c_str());
				}
				else if (bCombatObjectPlaybackOwner &&
					!bReferenceTimelineReady)
				{
					ImGui::TextWrapped("Owner animation unavailable: %s",
						ReferenceTimelineError.c_str());
				}
				ImGui::TextWrapped("Path: %s", Row.Path.empty() ?
					"(unavailable)" : Row.Path.generic_string().c_str());

				ImGui::BeginDisabled(bActive || Row.Path.empty());
				if (ImGui::SmallButton("Open Editor"))
				{
					SelectRuntimeRow();
					if (ProductPlaybackPreview.has_value())
					{
						Try_OpenValtanAuthoredEffect(Row.Path,
							Row.strEffectAssetId, *ProductPlaybackPreview);
					}
					else if (nullptr != pStageClockSource)
					{
						Try_OpenValtanStandaloneEffect(Row.Path,
							Row.strEffectAssetId);
					}
					else if (bReferenceTimelineReady)
					{
						Try_OpenValtanSavedReferenceEffect(Row.Path,
							Row.strEffectAssetId,
							ReferenceTimeline,
							iReferenceTimelineDurationMs, false,
							iReferenceEffectStartMs);
					}
					else
					{
						Try_LoadDocumentPath(Row.Path,
							EFFECT_DOCUMENT_SOURCE::AUTHORED,
							Row.strEffectAssetId);
					}
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(Row.Path.empty() ||
					!bHasExactPlaybackOwner || bAmbiguousOccurrence ||
					bKnownInvalid || bKnownNonDrawable);
				const char_t* pRuntimePlayLabel = nullptr != pStageClockSource ?
					"Play Effect" : "Play Effect + Animation";
				if (ImGui::SmallButton(pRuntimePlayLabel))
				{
					SelectRuntimeRow();
					if (ProductPlaybackPreview.has_value())
					{
						Try_PlayValtanSavedUnifiedEffect(Row.Path,
							Row.strEffectAssetId, *ProductPlaybackPreview);
					}
					else if (nullptr != pStageClockSource)
					{
						Try_PlayValtanStandaloneEffect(Row.Path,
							Row.strEffectAssetId);
					}
					else if (bReferenceTimelineReady)
					{
						Try_OpenValtanSavedReferenceEffect(Row.Path,
							Row.strEffectAssetId,
							ReferenceTimeline,
							iReferenceTimelineDurationMs, true,
							iReferenceEffectStartMs);
					}
				}
				ImGui::EndDisabled();

				const auto RefreshedCache =
					m_ValtanUnifiedEffectCaches.find(Row.strEffectAssetId);
				if (RefreshedCache != m_ValtanUnifiedEffectCaches.end() &&
					RefreshedCache->second.bObserved &&
					!RefreshedCache->second.strStatus.empty())
				{
					ImGui::TextWrapped("Last validation: %s",
						RefreshedCache->second.strStatus.c_str());
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		if (nullptr != pBinding)
		{
			ImGui::SeparatorText("Unsaved Pattern Draft");
			const std::filesystem::path DraftPath =
				CValtanPatternAuthoringEffectDocument::Resolve_AuthoringPath(
					*pBinding);
			ImGui::TextDisabled(
				"DRAFT_ATTACHED | save the Pattern link before Server gameplay uses it");
			const bool_t bDraftSelected =
				m_SelectedValtanPatternEffect.has_value() &&
				m_SelectedValtanPatternEffect->eKind ==
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED &&
				m_SelectedValtanPatternEffect->strPatternId ==
					Pattern.strPatternId &&
				m_SelectedValtanPatternEffect->strEffectAssetId ==
					pBinding->strEffectAssetId;
			if (ImGui::Selectable(
					pBinding->strEffectAssetId.c_str(), bDraftSelected))
			{
				VALTAN_PATTERN_EFFECT_SELECTION Selection;
				Selection.eKind =
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED;
				Selection.strPatternId = Pattern.strPatternId;
				Selection.strEffectAssetId = pBinding->strEffectAssetId;
				m_strSelectedValtanPatternId = Pattern.strPatternId;
				Select_SharedCompletePlayPattern(
					m_strSelectedValtanPatternId);
				m_SelectedValtanPatternEffect = std::move(Selection);
			}
			ImGui::BeginDisabled(DraftPath.empty() || bActiveDraft);
			if (ImGui::SmallButton("Open Editor"))
			{
				VALTAN_PATTERN_EFFECT_SELECTION Selection;
				Selection.eKind =
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED;
				Selection.strPatternId = Pattern.strPatternId;
				Selection.strEffectAssetId = pBinding->strEffectAssetId;
				m_SelectedValtanPatternEffect = std::move(Selection);
				Try_OpenValtanPatternDraftEffect(
					DraftPath, pBinding->strEffectAssetId, Pattern, false);
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(DraftPath.empty());
			if (ImGui::SmallButton("Local Effect + Pattern Preview"))
			{
				VALTAN_PATTERN_EFFECT_SELECTION Selection;
				Selection.eKind =
					VALTAN_PATTERN_EFFECT_SELECTION_KIND::DRAFT_ATTACHED;
				Selection.strPatternId = Pattern.strPatternId;
				Selection.strEffectAssetId = pBinding->strEffectAssetId;
				m_SelectedValtanPatternEffect = std::move(Selection);
				Try_OpenValtanPatternDraftEffect(
					DraftPath, pBinding->strEffectAssetId, Pattern, true);
			}
			ImGui::EndDisabled();
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(
	const VALTAN_INDEPENDENT_EFFECT_VIEW& Effect,
	const std::string& strSearch)
{
	if (!Matches_ValtanIndependentEffectSearch(Effect, strSearch))
		return;
	const bool_t bActive = m_ActiveDocument.has_value() &&
		m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
		m_ActiveDocument->strEffectAssetId == Effect.strEffectAssetId;
	std::string Label = Effect.strIndependentEffectId + " | " +
		Effect.strDisplayName + " | " + Effect.strEffectAssetId;
	ImGui::PushID(Effect.strIndependentEffectId.c_str());
	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	const bool_t bOpen = ImGui::TreeNodeEx(
		Label.c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		(bActive ? ImGuiTreeNodeFlags_Selected : 0));
	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	const VALTAN_PATTERN_VIEW* pOwnerPattern = Find_ValtanPattern(
		Effect.strOwnerPatternId);
	std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> OwnerTimeline;
	uint32_t iOwnerTimelineDurationMs = 0u;
	std::string TimelineError;
	const bool_t bTimelineReady = nullptr != pOwnerPattern &&
		Build_ValtanAuthoringTimeline(*pOwnerPattern,
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL, OwnerTimeline,
			iOwnerTimelineDurationMs, TimelineError);
	uint32_t iEffectStartMs = 0u;
	const VALTAN_STAGE_VIEW* pOwnerStage = nullptr;
	if (nullptr != pOwnerPattern)
	{
		for (const VALTAN_STAGE_VIEW& Stage : pOwnerPattern->Stages)
		{
			if (Stage.strStageId == Effect.strOwnerStageId)
			{
				pOwnerStage = &Stage;
				break;
			}
			iEffectStartMs += Stage.iDurationMs;
		}
	}
	const bool_t bOwnerStageFound = nullptr != pOwnerStage;
	if (nullptr != pOwnerPattern && !bOwnerStageFound)
		TimelineError = "Independent Effect owner stage left its pattern.";

	const bool_t bPatternStageOwner =
		"SERVER_PATTERN_STAGE" == Effect.strOwnership;
	const bool_t bCombatObjectOwner =
		"SERVER_COMBAT_OBJECT" == Effect.strOwnership;
	const VALTAN_COMBAT_OBJECT_EFFECT_VIEW* pCombatObjectLifecycle = nullptr;
	size_t iCombatObjectLifecycleMatches = 0u;
	if (bOwnerStageFound && bCombatObjectOwner)
	{
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Candidate :
			pOwnerStage->CombatObjectEffects)
		{
			if (Candidate.strCombatObjectArchetypeId ==
					Effect.strCombatObjectArchetypeId &&
				Candidate.strClientVisualId == Effect.strClientVisualId &&
				Candidate.strEffectAssetId == Effect.strEffectAssetId)
			{
				pCombatObjectLifecycle = &Candidate;
				++iCombatObjectLifecycleMatches;
			}
		}
		if (1u != iCombatObjectLifecycleMatches)
		{
			TimelineError =
				"SERVER_COMBAT_OBJECT did not resolve one exact Product lifecycle.";
		}
	}
	const bool_t bLocalCombatObjectLifecycleReady =
		1u == iCombatObjectLifecycleMatches &&
		nullptr != pCombatObjectLifecycle &&
		"ENTER" == pCombatObjectLifecycle->strTrigger &&
		("BOSS_RELATIVE" == pCombatObjectLifecycle->strVolleyPolicy ||
		 "ARENA_CENTER" == pCombatObjectLifecycle->strVolleyPolicy) &&
		"RADIAL" == pCombatObjectLifecycle->strVolleyLayout &&
		0u != pCombatObjectLifecycle->iSpawnValue &&
		0u != pCombatObjectLifecycle->iLifetimeMs;
	optional<VALTAN_PRODUCT_PREVIEW> PatternStagePreview;
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pStageClockCue = nullptr;
	if (bOwnerStageFound && bPatternStageOwner &&
		Effect.bHasCueProjection && Effect.bUsesCueStageClock)
	{
		const auto Cue = std::find_if(
			pOwnerStage->ProductCues.begin(), pOwnerStage->ProductCues.end(),
			[&Effect](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
			{
				return Candidate.strEffectAssetId == Effect.strEffectAssetId &&
					Candidate.strBindingId == Effect.strEffectCueBindingId &&
					Candidate.bUsesStageClock &&
					Candidate.iStageOffsetMs == Effect.iCueStageOffsetMs;
			});
		if (Cue != pOwnerStage->ProductCues.end())
			pStageClockCue = &*Cue;
		else
			TimelineError =
				"SERVER_PATTERN_STAGE stage-clock projection did not resolve one exact Product cue owner.";
	}
	else if (bTimelineReady && bOwnerStageFound && bPatternStageOwner)
	{
		if (!Effect.bHasCueProjection ||
			Effect.strEffectCueBindingId.empty() ||
			Effect.strCueClipOccurrenceId.empty())
		{
			TimelineError =
				"SERVER_PATTERN_STAGE requires one validated cueProjection tuple.";
		}
		else
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW* pProjectedClip = nullptr;
			const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pProjectedCue = nullptr;
			size_t iProjectionMatches = 0u;
			for (const VALTAN_STAGE_VIEW& Stage : pOwnerPattern->Stages)
			{
				if (Stage.strStageId != Effect.strOwnerStageId)
					continue;
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					Stage.ClipOccurrences)
				{
					if (Clip.strClipOccurrenceId !=
							Effect.strCueClipOccurrenceId ||
						Clip.strMappingBasis != Effect.strCueMappingBasis)
					{
						continue;
					}
					for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
						Clip.ProductCues)
					{
						if (Cue.strBindingId !=
								Effect.strEffectCueBindingId ||
							Cue.strEffectAssetId != Effect.strEffectAssetId ||
							Cue.iSourceStartMs != Effect.iCueSourceStartMs ||
							Cue.bHasSourceEnd != Effect.bHasCueSourceEnd ||
							(Cue.bHasSourceEnd && Cue.iSourceEndMs !=
								Effect.iCueSourceEndMs))
						{
							continue;
						}
						++iProjectionMatches;
						pProjectedClip = &Clip;
						pProjectedCue = &Cue;
					}
				}
			}
			if (1u != iProjectionMatches || nullptr == pProjectedClip ||
				nullptr == pProjectedCue)
			{
				TimelineError =
					"SERVER_PATTERN_STAGE cueProjection did not resolve one exact Product cue owner.";
			}
			else
			{
				VALTAN_PRODUCT_PREVIEW Preview;
				if (Build_ValtanProductPreview(*pOwnerPattern,
						VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
						*pProjectedClip, *pProjectedCue, Preview,
						TimelineError))
				{
					PatternStagePreview = std::move(Preview);
				}
			}
		}
	}
	else if (!bPatternStageOwner && !bCombatObjectOwner)
	{
		TimelineError = "Independent Effect ownership is unsupported.";
	}
	const bool_t bIndependentTimelineReady = bCombatObjectOwner ?
		(bTimelineReady && bOwnerStageFound) :
		(nullptr != pStageClockCue || PatternStagePreview.has_value());

	std::filesystem::path Path;
	const auto Editable = m_DirectAuthoredEditableEntries.find(
		Effect.strEffectAssetId);
	if (Editable != m_DirectAuthoredEditableEntries.end())
	{
		Path = Editable->second.Path;
	}
	else
	{
		const std::string AuthoringPath =
			CValtanPatternAuthoringEffectDocument::Build_AuthoringPath(
				Effect.strEffectAssetId);
		if (!AuthoringPath.empty())
			Path = CProjectDataRoot::Resolve(AuthoringPath);
	}
	if (Path.empty())
		Path.clear();

	const auto ObservedCache = m_ValtanUnifiedEffectCaches.find(
		Effect.strEffectAssetId);
	const bool_t bKnownInvalid =
		ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
		ObservedCache->second.bObserved && !ObservedCache->second.bValid;
	const bool_t bKnownNonDrawable =
		ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
		ObservedCache->second.bObserved && ObservedCache->second.bValid &&
		!ObservedCache->second.bDrawable;
	if (bOpen)
	{
		ImGui::TextDisabled(bLocalCombatObjectLifecycleReady ?
			"Independent authoring view | Valtan IDLE stays paused while the exact combat-object presentation lifecycle is sampled; gameplay remains Server-owned" :
			"Independent authoring view | Product-owned animation is replayed when present; gameplay remains Server-owned");
		ImGui::TextDisabled("Runtime owner: %s / %s | %s",
			Effect.strOwnerPatternId.c_str(),
			Effect.strOwnerStageId.c_str(),
			Effect.strOwnership.c_str());
		ImGui::TextWrapped("Path: %s",
			Path.empty() ? "(unavailable)" : Path.generic_string().c_str());
		/* The V1 document is the authoring reference. When the catalog overrides
		   the spawned Product presentation with a V2 group, say so here so nobody
		   edits this file expecting Server play to change. */
		if (bCombatObjectOwner && nullptr != pCombatObjectLifecycle &&
			!pCombatObjectLifecycle->strEffectV2GroupId.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f),
				"Product lane: EFFECT_V2_GROUP %s | Server play ignores this V1 document",
				pCombatObjectLifecycle->strEffectV2GroupId.c_str());
		}
		if (nullptr != pStageClockCue)
		{
			ImGui::TextDisabled(
				"Server stage clock +%u ms | Product owns no body animation for this cue | local Play starts the Effect immediately",
				pStageClockCue->iStageOffsetMs);
		}
		else if (!bIndependentTimelineReady)
		{
			ImGui::TextWrapped("Owner animation unavailable: %s",
				TimelineError.c_str());
		}
		else if (PatternStagePreview.has_value())
		{
			ImGui::TextDisabled(
				"Joined pattern timeline: %zu occurrences | %u ms | cue stage+%u ms",
				PatternStagePreview->TimelineClips.size(),
				PatternStagePreview->iTimelineDurationMs,
				PatternStagePreview->iOwningStageTimelineOffsetMs);
		}
		else if (bLocalCombatObjectLifecycleReady)
		{
			ImGui::TextDisabled(
				"Local lifecycle: Valtan IDLE | %u world roots | %s | yaw %.0f + n*%.0f deg | Server lifetime %u ms",
				pCombatObjectLifecycle->iSpawnValue,
				"ARENA_CENTER" == pCombatObjectLifecycle->strVolleyPolicy ?
					"arena-center origin, world yaw" : "boss origin, boss-relative yaw",
				pCombatObjectLifecycle->fVolleyStartAngleDegrees,
				pCombatObjectLifecycle->fVolleyAngleStepDegrees,
				pCombatObjectLifecycle->iLifetimeMs);
			ImGui::TextDisabled(
				"At authored presentation time the terminal hit/explode Effect is sampled; active roots keep their authored lifetime past the Server despawn, and boss animation and Product cues stay suppressed.");
		}
		else
		{
			ImGui::TextDisabled(
				"Owner animation timeline: %zu occurrences | %u ms | local Effect starts at owner stage +%u ms",
				OwnerTimeline.size(), iOwnerTimelineDurationMs,
				iEffectStartMs);
		}

		ImGui::BeginDisabled(Path.empty() || bActive);
		if (ImGui::SmallButton("Open Editor"))
		{
			if (bLocalCombatObjectLifecycleReady)
			{
				Try_OpenValtanStandaloneEffect(Path,
					Effect.strEffectAssetId);
			}
			else if (PatternStagePreview.has_value())
			{
				Try_OpenValtanAuthoredEffect(Path,
					Effect.strEffectAssetId, *PatternStagePreview);
			}
			else if (nullptr != pStageClockCue)
			{
				Try_OpenValtanStandaloneEffect(Path,
					Effect.strEffectAssetId);
			}
			else if (bTimelineReady && bOwnerStageFound)
			{
				Try_OpenValtanSavedReferenceEffect(Path,
					Effect.strEffectAssetId, OwnerTimeline,
					iOwnerTimelineDurationMs, false, iEffectStartMs);
			}
			else
			{
				Try_LoadDocumentPath(Path,
					EFFECT_DOCUMENT_SOURCE::AUTHORED,
					Effect.strEffectAssetId);
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(Path.empty() || !bIndependentTimelineReady ||
			bKnownInvalid || bKnownNonDrawable);
		const char_t* pIndependentPlayLabel =
			bLocalCombatObjectLifecycleReady ?
				"Play Combat Object Lifecycle" :
				(nullptr != pStageClockCue ?
					"Play Effect" : "Play Effect + Owner Animation");
		if (ImGui::SmallButton(pIndependentPlayLabel))
		{
			if (bLocalCombatObjectLifecycleReady &&
				nullptr != pOwnerPattern)
			{
				Try_PlayValtanCombatObjectIndependentEffect(
					Path, Effect, *pOwnerPattern);
			}
			else if (PatternStagePreview.has_value())
			{
				Try_PlayValtanSavedUnifiedEffect(Path,
					Effect.strEffectAssetId, *PatternStagePreview);
			}
			else if (nullptr != pStageClockCue)
			{
				Try_PlayValtanStandaloneEffect(Path,
					Effect.strEffectAssetId);
			}
			else
			{
				Try_OpenValtanSavedReferenceEffect(Path,
					Effect.strEffectAssetId, OwnerTimeline,
					iOwnerTimelineDurationMs, true, iEffectStartMs);
			}
		}
		ImGui::EndDisabled();
		std::string OwnerServerPlayReason;
		const bool_t bCanPlayServerOwner = nullptr != pOwnerPattern &&
			Can_PlayValtanServerPattern(
				*pOwnerPattern, OwnerServerPlayReason);
		if (nullptr == pOwnerPattern)
			OwnerServerPlayReason =
				"The independent Effect lost its stable owner Pattern.";
		ImGui::SameLine();
		ImGui::BeginDisabled(!bCanPlayServerOwner);
		if (ImGui::SmallButton("Complete Play Owner") &&
			nullptr != pOwnerPattern)
			Try_PlayValtanServerPattern(*pOwnerPattern);
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("%s", bCanPlayServerOwner ?
				"Run the complete owner Pattern through the Valtan Arena Server." :
				OwnerServerPlayReason.c_str());
		}
		if (!bCanPlayServerOwner)
		{
			ImGui::TextWrapped("Server: %s",
				OwnerServerPlayReason.c_str());
		}
		else if (nullptr != pOwnerPattern &&
			m_strValtanServerPatternStatusPatternId ==
				pOwnerPattern->strPatternId &&
			!m_strValtanServerPatternStatus.empty())
		{
			ImGui::TextWrapped("Server: %s",
				m_strValtanServerPatternStatus.c_str());
		}
		if (bCombatObjectOwner && bActive)
		{
			ImGui::TextDisabled(
				"World preview root: (%.2f, %.2f, %.2f) | actual target tracking and hit timing require Complete Play Owner",
				m_PreviewWorldRoot._41, m_PreviewWorldRoot._42,
				m_PreviewWorldRoot._43);
		}

		const auto Cache = m_ValtanUnifiedEffectCaches.find(
			Effect.strEffectAssetId);
		if (Cache != m_ValtanUnifiedEffectCaches.end() &&
			Cache->second.bObserved &&
			!Cache->second.strStatus.empty())
		{
			ImGui::TextWrapped("Last validation: %s",
				Cache->second.strStatus.c_str());
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}
